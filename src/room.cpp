#include <sstream>

#include "server.hpp"

static Message DESC_END = { "", MSG_ROOMDESCEND, 0, 0 };

Room::Room(int16_t id, Server *owner):
	name("test"),
	img_name("dropzone.gif"),
	owner(owner),
	faces(0),
	id(id),
	flags(RF_DROPZONE)
{
}

void Room::Join(ConnectionPtr c)
{
	std::string info = c->Serialise(true);
	
	Message msg = { info, MSG_USERNEW, static_cast<uint32_t>(info.size()), c->id };
	
	for (auto u: users)
	{
		boost::asio::async_write(u->socket, boost::asio::buffer(msg.Serialise(), msg.size),
			[this, u](boost::system::error_code ec, size_t)
			{
				if (ec)
				{
					owner->Disconnect(u->id);
					Log(ec.message());
				}
			});
	}
	
	users.insert(c);
}

void Room::SendDescription(ConnectionPtr c, bool updated)
{
	std::ostringstream oss, ulists;

	std::string desc = Serialise(true);
	Message rdesc = { desc, MSG_ROOMDESC, static_cast<uint32_t>(desc.size()), 0 };
	if (updated) rdesc.type = MSG_ROOMSETDESC;
	
	ulists.seekp(0, ulists.beg);
	for (auto u: users)
	{
		std::string s = u->Serialise(true);
		ulists.write(s.data(), s.size());
	}
	Message rprs = { ulists.str(), MSG_USERLIST, static_cast<uint32_t>(ulists.str().size()),
		static_cast<int32_t>(users.size()) };
	
	oss.seekp(0, oss.beg);
	oss.write(rdesc.Serialise().data(), rdesc.size+12);
	oss.write(rprs.Serialise().data(), rprs.size+12);
	oss.write(DESC_END.Serialise().data(), 12);
	
	if (updated)
		for (auto u: users)
		{
			boost::asio::async_write(u->socket, boost::asio::buffer(oss.str(), oss.tellp()),
				[this, u](boost::system::error_code ec, size_t)
				{
					if (ec)
					{
						owner->Disconnect(u->id);
						Log(ec.message());
					}
				});
		}
	else
	{
		boost::asio::async_write(c->socket, boost::asio::buffer(oss.str(), oss.tellp()),
			[this, c](boost::system::error_code ec, size_t)
			{
				if (ec)
				{
					owner->Disconnect(c->id);
					Log(ec.message());
				}
				else
					Join(c);
			});
	}
}

std::string SpotState::Serialise()
{
	std::ostringstream oss;
	uint32_t point = location.Serialise();
	
	oss.seekp(0, oss.beg);
	oss.write(reinterpret_cast<char*>(&img_id), 2);
	oss.put(0).put(0); // alignment
	oss.write(reinterpret_cast<char*>(&point), 4);
	
	return oss.str();
}

std::string Image::Serialise(uint16_t name_offs)
{
	std::ostringstream oss;
	
	oss.seekp(0, oss.beg);
	oss.put(0).put(0).put(0).put(0); // refcon
	oss.write(reinterpret_cast<char*>(&id), 2);
	oss.write(reinterpret_cast<char*>(&name_offs), 2);
	oss.write(reinterpret_cast<char*>(&alpha), 2);
	oss.put(0).put(0); // alignment
	
	return oss.str();
}

std::string Hotspot::Serialise(uint16_t points_offs, uint16_t states_offs,
	uint16_t name_offs, uint16_t script_offs)
{
	std::ostringstream oss;
	uint32_t point = location.Serialise();
	uint16_t n;
	
	oss.seekp(0, oss.beg);
	oss.write(reinterpret_cast<char*>(&script_event_mask), 4);
	oss.write(reinterpret_cast<char*>(&flags), 4);
	oss.put(0).put(0).put(0).put(0).put(0).put(0).put(0).put(0); // secure info + refcon
	oss.write(reinterpret_cast<char*>(&point), 4);
	oss.write(reinterpret_cast<char*>(&id), 2);
	oss.write(reinterpret_cast<char*>(&dest), 2);
	n = points.size();
	oss.write(reinterpret_cast<char*>(&n), 2);
	oss.write(reinterpret_cast<char*>(&points_offs), 2);
	oss.write(reinterpret_cast<char*>(&type), 2);
	oss.put(0).put(0); // group id
	n = script.empty() ? 0 : 1; // NOTE: multiple scripts possible?
	oss.write(reinterpret_cast<char*>(&n), 2);
	oss.put(0).put(0); // script rec offset
	oss.write(reinterpret_cast<char*>(&state), 2);
	n = states.size();
	oss.write(reinterpret_cast<char*>(&n), 2);
	oss.write(reinterpret_cast<char*>(&states_offs), 2);
	oss.write(reinterpret_cast<char*>(&name_offs), 2);
	oss.write(reinterpret_cast<char*>(&script_offs), 2);
	oss.put(0).put(0); // alignment
	
	return oss.str();
}

std::string Draw::Serialise()
{
	std::ostringstream oss;
	uint16_t n;
	
	oss.seekp(0, oss.beg);
	oss.put(0).put(0).put(0).put(0); // llrec
	oss.write(reinterpret_cast<char*>(&cmd), 2);
	n = data.size();
	oss.write(reinterpret_cast<char*>(&n), 2);
	n = 10; // data offset is always 10
	oss.write(reinterpret_cast<char*>(&n), 2);
	oss.write(data.data(), data.size());
	
	return oss.str();
}

std::string LooseProp::Serialise()
{
	std::ostringstream oss;
	uint64_t spec = data->SerialiseSpec();
	uint32_t point = location.Serialise();
	
	oss.seekp(0, oss.beg);
	oss.put(0).put(0).put(0).put(0); // llrec
	oss.write(reinterpret_cast<char*>(&spec), 8);
	oss.write(reinterpret_cast<char*>(&data->flags), 4);
	oss.write(reinterpret_cast<char*>(&refcon), 4);
	oss.write(reinterpret_cast<char*>(&point), 4);
	
	return oss.str();
}

int Room::TotalPoints() const
{
	int n = 0;
	
	for (auto s: spots)
		n += s.points.size();
	return n;
}

int Room::TotalStates() const
{
	int n = 0;
	
	for (auto s: spots)
		n += s.states.size();
	return n;
}

std::string Room::Serialise(bool full)
{
	if (full)
	{
		std::ostringstream meta, body;
		uint16_t i16;
		uint32_t i32;
		char slen;
		
		meta.seekp(0, meta.beg);
		i32 = flags;
		meta.write(reinterpret_cast<char*>(&i32), 4);
		meta.write(reinterpret_cast<char*>(&faces), 4);
		meta.write(reinterpret_cast<char*>(&id), 2);
		
		body.seekp(0, body.beg);
		body.put(0).put(0); // sentinel
		
		slen = name.size();
		i16 = slen ? static_cast<uint16_t>(body.tellp()) : 0;
		meta.write(reinterpret_cast<char*>(&i16), 2);
		if (slen)
		{
			body.write(&slen, 1);
			body.write(name.data(), slen);
		}
		
		slen = img_name.size();
		i16 = slen ? static_cast<uint16_t>(body.tellp()) : 0;
		meta.write(reinterpret_cast<char*>(&i16), 2);
		if (slen)
		{
			body.write(&slen, 1);
			body.write(img_name.data(), slen);
		}
		
		slen = artist.size();
		i16 = slen ? static_cast<uint16_t>(body.tellp()) : 0;
		meta.write(reinterpret_cast<char*>(&i16), 2);
		if (slen)
		{
			body.write(&slen, 1);
			body.write(artist.data(), slen);
		}
		
		slen = pw.size();
		i16 = slen ? static_cast<uint16_t>(body.tellp()) : 0;
		meta.write(reinterpret_cast<char*>(&i16), 2);
		if (slen)
		{
			body.write(&slen, 1);
			body.write(pw.data(), slen);
		}
		
		uint16_t *img_name_offs = imgs.size() ? new uint16_t[imgs.size()] : nullptr;
		if (img_name_offs)
			for (int i = 0; i < imgs.size(); i++)
			{
				img_name_offs[i] = body.tellp();
				slen = imgs[i].name.size();
				body.write(&slen, 1);
				body.write(imgs[i].name.data(), slen);
			}
		
		uint16_t *spot_name_offs = spots.size() ? new uint16_t[spots.size()] : nullptr;
		if (spot_name_offs)
			for (int i = 0; i < spots.size(); i++)
			{
				spot_name_offs[i] = body.tellp();
				slen = spots[i].name.size();
				body.write(&slen, 1);
				body.write(spots[i].name.data(), slen);
			}
		
		uint16_t *state_offs = TotalStates() ? new uint16_t[spots.size()] : nullptr;
		if (state_offs)
			for (int i = 0; i < spots.size(); i++)
			{
				state_offs[i] = body.tellp();
				for (auto s: spots[i].states)
				{
					std::string state = s.Serialise();
					body.write(state.data(), 8);
				}
			}
		
		uint16_t *point_offs = TotalPoints() ? new uint16_t[spots.size()] : nullptr;
		if (point_offs)
			for (int i = 0; i < spots.size(); i++)
			{
				point_offs[i] = body.tellp();
				for (auto p: spots[i].points)
				{
					uint32_t point = p.Serialise();
					body.write(reinterpret_cast<char*>(&point), 4);
				}
			}
		
		uint16_t *script_offs = spots.size() ? new uint16_t[spots.size()] : nullptr;
		if (script_offs)
			for (int i = 0; i < spots.size(); i++)
			{
				script_offs[i] = body.tellp();
				body.write(spots[i].script.data(), spots[i].script.size());
				body.put(0);
			}
		
		uint16_t img_offs = imgs.size() ? static_cast<uint16_t>(body.tellp()) : 0;
		if (img_offs)
			for (int i = 0; i < imgs.size(); i++)
			{
				std::string ims = imgs[i].Serialise(img_name_offs[i]);
				body.write(ims.data(), 12);
			}
		
		i16 = spots.size();
		meta.write(reinterpret_cast<char*>(&i16), 2);
		
		i16 = spots.size() ? static_cast<uint16_t>(body.tellp()) : 0;
		meta.write(reinterpret_cast<char*>(&i16), 2);
		if (i16)
		{
			for (int i = 0; i < spots.size(); i++)
			{
				std::string s = spots[i].Serialise(point_offs[i], state_offs[i], spot_name_offs[i],
					script_offs[i]);
				body.write(s.data(), 48);
			}
		}
		
		i16 = imgs.size();
		meta.write(reinterpret_cast<char*>(&i16), 2);
		meta.write(reinterpret_cast<char*>(&img_offs), 2);
		
		delete[] img_name_offs;
		delete[] spot_name_offs;
		delete[] state_offs;
		delete[] point_offs;
		delete[] script_offs;
		
		i16 = draws.size();
		meta.write(reinterpret_cast<char*>(&i16), 2);
		
		i16 = draws.size() ? static_cast<uint16_t>(body.tellp()) : 0;
		meta.write(reinterpret_cast<char*>(&i16), 2);
		if (i16)
			for (int i = 0; i < draws.size(); i++)
			{
				std::string d = draws[i].Serialise();
				body.write(d.data(), draws[i].data.size()+10);
			}
		
		i16 = users.size();
		meta.write(reinterpret_cast<char*>(&i16), 2);
		
		i16 = lprops.size();
		meta.write(reinterpret_cast<char*>(&i16), 2);
		
		i16 = lprops.size() ? static_cast<uint16_t>(body.tellp()) : 0;
		meta.write(reinterpret_cast<char*>(&i16), 2);
		if (i16)
			for (int i = 0; i < lprops.size(); i++)
			{
				std::string lp = lprops[i].Serialise();
				body.write(lp.data(), 24);
			}
		
		meta.put(0).put(0); // alignment
		
		i16 = body.tellp();
		meta.write(reinterpret_cast<char*>(&i16), 2);
		
		meta.write(body.str().data(), static_cast<uint16_t>(body.tellp()));
		return meta.str();
	}
	else
	{
		std::ostringstream oss;
		uint32_t i32;
		uint16_t i16;
		char slen = name.size();
		
		oss.seekp(0, oss.beg);
		i32 = id;
		oss.write(reinterpret_cast<char*>(&i32), 4);
		oss.write(reinterpret_cast<char*>(&flags), 2);
		i16 = users.size();
		oss.write(reinterpret_cast<char*>(&i16), 2);
		oss.write(&slen, 1);
		oss.write(name.data(), slen);
		if (slen % 2) oss.put(0);
		
		return oss.str();
	}
}
