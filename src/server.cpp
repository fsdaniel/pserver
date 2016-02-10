#include <sstream>

#include "server.hpp"

static const char *CLIENT_TYPE[] = {
	"32Bit Windows",
	"Java",
	"Unknown"
};

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
	DLC_FILES_HTTPSRVR = 256,
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

Server::Server(boost::asio::io_service &io, const tcp::endpoint &ep):
	listener(io, ep),
	socket(io),
	last_user_id(0),
	perms(PM_DEFAULTS),
	opts(SO_DEFAULTS),
	name("test")
{
	Listen();
}

void Server::Listen()
{
	listener.async_accept(socket,
		[this](boost::system::error_code ec)
		{
			if (!ec)
			{
				auto c = std::make_shared<Connection>(std::move(socket));
				SendID(c);
			}
			
			Listen();
		});
}

void Server::Disconnect(std::uint32_t uid)
{
	// NOTE: should we care about socket errors?
	users[uid]->socket.cancel();
	users[uid]->socket.close();
	users[uid].reset();
	users.erase(uid);
}

void Server::SendID(ConnectionPtr c)
{
	auto user(c->shared_from_this());
	Message msg = { nullptr, MSG_TIYID, 0, ++last_user_id };
	const char *data = msg.Serialise();
	
	boost::asio::async_write(user->socket, boost::asio::buffer(data, 12),
		[this, user, data](boost::system::error_code ec, std::size_t)
		{
			if (ec)
				Disconnect(user->id);
			else
			{
				users.insert(UserID(last_user_id, user));
				user->id = last_user_id;
				ReadHeader(user);
			}
			delete[] data;
		});
}

void Server::ReadHeader(ConnectionPtr c)
{
	auto user(c->shared_from_this());
	char *data = new char[12];
	
	boost::asio::async_read(user->socket, boost::asio::buffer(data, 12),
		[this, user, data](boost::system::error_code ec, std::size_t)
		{
			if (ec)
				Disconnect(user->id);
			else
			{
				std::uint32_t type = *(std::uint32_t*)data;
				std::uint32_t size = *(std::uint32_t*)(data+4);
				std::int32_t refnum = *(std::int32_t*)(data+8);
				delete[] data;
				
				switch (type)
				{
					case MSG_LOGON: ReadLogin(user); break;
				}
			}
		});
}

void Server::ReadLogin(ConnectionPtr c)
{
	auto user(c->shared_from_this());
	char *data = new char[128];
	
	boost::asio::async_read(user->socket, boost::asio::buffer(data, 128),
		[this, user, data](boost::system::error_code ec, std::size_t)
		{
			if (ec)
				Disconnect(user->id);
			else
			{
				std::istringstream iss(std::string(data, 128));
				char *str31 = new char[31];
				char slen;
				
				iss.seekg(9, iss.beg);  // skipping reg code and name length;
				iss.read(str31, 31);
				user->name = str31;
				iss.read(&slen, 1);
				iss.read(str31, 31);
				user->pw = std::string(str31, slen);
				delete[] str31;
				iss.read((char*)&user->aux, 4);
				iss.seekg(20, iss.cur); // skip pseudo reg and demo stuff
				iss.read((char*)&user->room, 2); // skip rest
				// TODO: read reserved[6] as 6-character client ID
				user->face = std::rand() % NUM_FACES;
				user->colour = std::rand() % NUM_COLORS;
				user->status = 0; // TODO: determine from banlist and set appropriately
				user->draw_state = DrawState::NONE;
				SendLoginReply(user, data);
			}
		});
}

void Server::SendLoginReply(ConnectionPtr c, char *data)
{
	auto user(c->shared_from_this());
	Message msg = { data, MSG_ALTLOGONREPLY, 128, user->id };
	const char *buf = msg.Serialise();
	
	delete[] data;
	boost::asio::async_write(user->socket, boost::asio::buffer(buf, 140),
		[this, user, buf](boost::system::error_code ec, std::size_t)
		{
			if (ec)
				Disconnect(user->id);
			else
				SendVersion(user);
			delete[] buf;
		});
}

void Server::SendVersion(ConnectionPtr c)
{
	auto user(c->shared_from_this());
	Message msg = { nullptr, MSG_VERSION, 0, VERSION };
	const char *data = msg.Serialise();
	
	boost::asio::async_write(user->socket, boost::asio::buffer(data, 12),
		[this, user, data](boost::system::error_code ec, std::size_t)
		{
			if (ec)
				Disconnect(user->id);
			else
				SendServerInfo(user);
			delete[] data;
		});
}

void Server::SendServerInfo(ConnectionPtr c)
{
	auto user(c->shared_from_this());
	std::ostringstream oss;
	char l = name.size();
	
	oss.seekp(0, oss.beg);
	oss.write((char*)&perms, 4);
	oss.write(&l, 1);
	oss.write(name.data(), l);
	// NOTE: according to packet sniffing, opts + dl/ul caps not sent?
	
	char *data = new char[oss.str().size()];
	std::memmove(data, oss.str().data(), oss.str().size());
	Message msg = { data, MSG_SERVERINFO, oss.str().size(), user->id };
	const char *buf = msg.Serialise();
	delete[] data;
	
	boost::asio::async_write(user->socket, boost::asio::buffer(buf, msg.size+12),
		[this, user, buf](boost::system::error_code ec, std::size_t)
		{
			if (ec)
				Disconnect(user->id);
			else
				SendUserStatus(user);
			delete[] buf;
		});
}

void Server::SendUserStatus(ConnectionPtr c)
{
	auto user(c->shared_from_this());
	Message msg = { (char*)&user->status, MSG_USERSTATUS, 2, user->id };
	const char *data = msg.Serialise();
	
	boost::asio::async_write(user->socket, boost::asio::buffer(data, 14),
		[this, user, data](boost::system::error_code ec, std::size_t)
		{
			if (ec)
				Disconnect(user->id);
			else
				NotifyNewLogin(user->id);
			delete[] data;
		});
}

void Server::NotifyNewLogin(std::uint32_t uid)
{
	std::uint32_t nusers = users.size();
	Message msg = { (char*)&nusers, MSG_USERLOG, 4, uid };
	std::string data(msg.Serialise(), 16);
	
	for (auto u: users)
	{
		auto user = std::get<1>(u)->shared_from_this();
		boost::asio::async_write(user->socket, boost::asio::buffer(data, 16),
			[this, user](boost::system::error_code ec, std::size_t)
			{
				if (ec)
					Disconnect(user->id);
			});
	}
}
