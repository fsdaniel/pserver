#include <sstream>

#include "connection.hpp"

std::string Connection::Serialise(bool full)
{
	std::ostringstream oss;
	char slen = name.size();
	
	oss.seekp(0, oss.beg);
	
	if (full)
	{
		uint32_t point = pos.Serialise();
		uint16_t nprops = 0;
		
		oss.write(reinterpret_cast<char*>(&id), 4);
		oss.write(reinterpret_cast<char*>(&point), 4);
		
		for (int i = 0; i < 9; i++)
		{
			if (props[i])
			{
				uint64_t spec = props[i]->SerialiseSpec();
				oss.write(reinterpret_cast<char*>(&spec), 8);
				nprops++;
			}
			else
				oss.put(0).put(0).put(0).put(0).put(0).put(0).put(0).put(0);
		}
		
		oss.write(reinterpret_cast<char*>(&room), 2);
		oss.write(reinterpret_cast<char*>(&face), 2);
		oss.write(reinterpret_cast<char*>(&colour), 2);
		oss.put(0).put(0).put(0).put(0); // away flag + open to msgs
		oss.write(reinterpret_cast<char*>(&nprops), 2);
		oss.write(&slen, 1);
		oss.write(name.data(), slen);
	}
	else
	{
		oss.write(reinterpret_cast<char*>(&id), 4);
		oss.write(reinterpret_cast<char*>(&status), 2);
		oss.write(reinterpret_cast<char*>(&room), 2);
		oss.write(&slen, 1);
		oss.write(name.data(), slen);
		if (slen % 2) oss.put(0);
	}
	
	return oss.str();
}
