#include <cstdlib>
#include <sstream>

#include "server.hpp"

#include "room.cpp"

// extended info flags
enum
{
	SI_AVATAR_URL = 1,
	SI_SERVER_INFO,
	SI_SERVER_TYPE = 4,
	SI_SERVER_FLAGS = 8,
	SI_NUM_USERS = 16,
	SI_SERVER_NAME = 32,
	SI_HTTP_URL = 64
};

// extended info response IDs
enum
{
	INF_AURL = 0x4155524C,
	ERR_AUTH = 0x41555448,
	INF_FLAG = 0x464C4147,
	INF_HURL = 0x4855524C,
	EXT_NAME = 0x4E414D45,
	INF_NUM_USERS = 0x4E555352,
	EXT_PASS = 0x50415353,
	EXT_TYPE = 0x54595045,
	ERR_UNKN = 0x554E4B4E,
	INF_VERS = 0x56455253
};

// misc and duplicates
enum
{
	VERSION = 0x10016,
	INF_NAME = EXT_NAME,
	INF_TYPE = EXT_TYPE
};

// INF_FLAG
enum
{
	FF_CLOSEDSERVER = 2,
	FF_GUESTSAREMEMBERS = 4, // should always be set for legacy purposes
	FF_INSTANTPALACE = 16
};

// INF_TYPE
enum
{
	PLAT_MACINTOSH = 0,
	PLAT_WINDOWSNT = 2,
	PLAT_UNIX
};

static const char *CLIENT_TYPE[] = {
	"32Bit Windows",
	"Java",
	"Unknown"
};

static Message SERVER_VERSION = { "", MSG_VERSION, 0, VERSION };

Server::Server(boost::asio::io_service &io, const tcp::endpoint &ep):
	listener(io, ep),
	socket(io),
	last_user_id(0),
	perms(PM_DEFAULTS),
	opts(SO_DEFAULTS),
	name("test"),
	media_url("http://chat.epalacesmedia.com/")
{
	rooms[2] = new Room(2, this);
	Listen();
}

void Server::Listen()
{
	listener.async_accept(socket,
		[this](boost::system::error_code ec)
		{
			if (ec)
				Log(ec.message());
			else
			{
				auto c = std::make_shared<Connection>(std::move(socket));
				SendID(c);
			}
			
			Listen();
		});
}

void Server::Disconnect(int32_t uid)
{
	Log(users[uid]->name + " disconnected.");

	// NOTE: should we care about socket errors here?
	users[uid]->socket.cancel();
	users[uid]->socket.close();
	users[uid].reset();
	users.erase(uid);
}

void Server::SendID(ConnectionPtr c)
{
	Message msg = { "", MSG_TIYID, 0, ++last_user_id };
	
	boost::asio::async_write(c->socket, boost::asio::buffer(msg.Serialise(), 12),
		[this, c](boost::system::error_code ec, size_t)
		{
			if (ec)
			{
				Disconnect(c->id);
				Log(ec.message());
			}
			else
			{
				Log("Incoming connection from " + c->socket.remote_endpoint().address().to_string());
				users[last_user_id] = c;
				c->id = last_user_id;
				ReadHeader(c);
			}
		});
}

void Server::ReadHeader(ConnectionPtr c)
{
	char data[12];
	
	boost::asio::async_read(c->socket, boost::asio::buffer(data, 12),
		[this, c, &data](boost::system::error_code ec, size_t)
		{
			if (ec)
			{
				Disconnect(c->id);
				Log(ec.message());
			}
			else
			{
				uint32_t type = *(reinterpret_cast<uint32_t*>(data));
				uint32_t size = *(reinterpret_cast<uint32_t*>(data)+4);
				int32_t refnum = *(reinterpret_cast<int32_t*>(data)+8);
				
				switch (type)
				{
					case MSG_LOGON: ReadLogin(c); break;
				}
			}
		});
}

void Server::ReadLogin(ConnectionPtr c)
{
	char *data = new char[128];
	
	boost::asio::async_read(c->socket, boost::asio::buffer(data, 128),
		[this, c, data](boost::system::error_code ec, size_t)
		{
			if (ec)
			{
				Disconnect(c->id);
				Log(ec.message());
			}
			else
			{
				std::istringstream iss(data);
				char str31[31];
				char slen;
				
				memset(&str31[0], 0, 31);
				iss.seekg(8, iss.beg);  // skipping reg code
				iss.read(&slen, 1);
				iss.read(&str31[0], slen);
				iss.seekg(31-slen, iss.cur);
				c->name = str31;
				iss.read(&slen, 1);
				iss.read(&str31[0], slen);
				iss.seekg(31-slen, iss.cur);
				c->pw = std::string(str31, slen);
				iss.read(reinterpret_cast<char*>(&c->aux), 4);
				iss.seekg(20, iss.cur); // skip pseudo reg and demo stuff
				//iss.read(reinterpret_cast<char*>(&c->room), 2); // skip rest
				// TODO: read reserved[6] as 6-character client ID
				c->room = 2;
				c->face = rand() % NUM_FACES;
				c->colour = rand() % NUM_COLORS;
				c->status = 0; // TODO: determine from banlist and set appropriately
				c->draw_state = DrawState::NONE;
				
				Log(c->name + " logged in.");
				SendLoginReply(c, data);
			}
		});
}

void Server::SendLoginReply(ConnectionPtr c, char *data)
{
	Message msg = { std::string(data, 128), MSG_ALTLOGONREPLY, 128, c->id };
	delete[] data;
	
	boost::asio::async_write(c->socket, boost::asio::buffer(msg.Serialise(), 140),
		[this, c](boost::system::error_code ec, size_t)
		{
			if (ec)
			{
				Disconnect(c->id);
				Log(ec.message());
			}
			else
				SendVersion(c);
		});
}

void Server::SendVersion(ConnectionPtr c)
{
	boost::asio::async_write(c->socket, boost::asio::buffer(SERVER_VERSION.Serialise(), 12),
		[this, c](boost::system::error_code ec, size_t)
		{
			if (ec)
			{
				Disconnect(c->id);
				Log(ec.message());
			}
			else
				SendServerInfo(c);
		});
}

void Server::SendServerInfo(ConnectionPtr c)
{
	std::ostringstream oss;
	char l = name.size();
	
	oss.seekp(0, oss.beg);
	oss.write(reinterpret_cast<char*>(&perms), 4);
	oss.write(&l, 1);
	oss.write(name.data(), l);
	// NOTE: according to packet sniffing, opts + dl/ul caps not sent?
	
	Message msg = { oss.str(), MSG_SERVERINFO, static_cast<uint32_t>(oss.str().size()), c->id };
	
	boost::asio::async_write(c->socket, boost::asio::buffer(msg.Serialise(), msg.size+12),
		[this, c](boost::system::error_code ec, size_t)
		{
			if (ec)
			{
				Disconnect(c->id);
				Log(ec.message());
			}
			else
				SendUserStatus(c);
		});
}

void Server::SendUserStatus(ConnectionPtr c)
{
	Message msg = { std::string(reinterpret_cast<char*>(&c->status), 2), MSG_USERSTATUS, 2, c->id };
	
	boost::asio::async_write(c->socket, boost::asio::buffer(msg.Serialise(), 14),
		[this, c](boost::system::error_code ec, size_t)
		{
			if (ec)
			{
				Disconnect(c->id);
				Log(ec.message());
			}
			else
				NotifyNewLogin(c);
		});
}

void Server::NotifyNewLogin(const ConnectionPtr c)
{
	uint32_t nusers = users.size();
	Message msg = { std::string(reinterpret_cast<char*>(&nusers), 4), MSG_USERLOG, 4, c->id };
	
	for (auto u: users)
	{
		auto user = std::get<1>(u);
		boost::asio::async_write(user->socket, boost::asio::buffer(msg.Serialise(), 16),
			[this, user, c](boost::system::error_code ec, size_t)
			{
				if (ec)
				{
					Disconnect(user->id);
					Log(ec.message());
				}
				else
					if (user->id == c->id)
						SendMediaURL(user);
			});
	}
}

void Server::SendMediaURL(ConnectionPtr c)
{
	Message msg = { media_url, MSG_HTTPSERVER, static_cast<uint32_t>(media_url.size())+1, c->id };
	
	boost::asio::async_write(c->socket, boost::asio::buffer(msg.Serialise(), msg.size),
		[this, c](boost::system::error_code ec, size_t)
		{
			if (ec)
			{
				Disconnect(c->id);
				Log(ec.message());
			}
		else
			rooms[c->room]->SendDescription(c, false);
		});
}
