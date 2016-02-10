#ifndef _SERVER_H
#define _SERVER_H

#include "room.hpp"

// perms
enum
{
	PM_ALLOWGUESTS = 1, // should always be on for legacy purposes
	PM_ALLOWCYBORGS,
	PM_ALLOWPAINTING = 4,
	PM_ALLOWCUSTOMPROPS = 8,
	PM_ALLOWWIZARDS = 16,
	PM_WIZARDSMAYKILL = 32,
	PM_WIZARDSMAYAUTHOR = 64,
	PM_PLAYERSMAYKILL = 128,
	PM_CYBORGSMAYKILL = 256,
	PM_DEATHPENALTY = 512,
	PM_PURGEINACTIVEPROPS = 1024,
	PM_KILLFLOODERS = 2048,
	PM_NOSPOOFING = 4096,
	PM_MEMBERCREATEDROOMS = 8192,
	PM_DEFAULTS = (PM_ALLOWGUESTS | PM_ALLOWCYBORGS | PM_ALLOWPAINTING |
		PM_ALLOWCUSTOMPROPS | PM_ALLOWWIZARDS | PM_WIZARDSMAYKILL | PM_WIZARDSMAYAUTHOR |
		PM_DEATHPENALTY | PM_PURGEINACTIVEPROPS | PM_KILLFLOODERS | PM_MEMBERCREATEDROOMS)
};

// opts
enum
{
	SO_PASSWORDSECURITY = 2,
	SO_CHATLOG = 4,
	SO_NOWHISPER = 8,
	SO_AUTHENTICATE = 32,
	SO_POUNDPROTECT = 64,
	SO_SORTOPTIONS = 128,
	SO_AUTHTRACKLOGOFF = 256,
	SO_JAVASECURE = 512,
	SO_DEFAULTS = (SO_CHATLOG | SO_AUTHTRACKLOGOFF)
};

class Server final
{
public:
	Server(boost::asio::io_service&, const tcp::endpoint&);
	void Disconnect(std::uint32_t);
private:
	void Listen();
	void SendID(ConnectionPtr);
	void ReadHeader(ConnectionPtr);
	void ReadLogin(ConnectionPtr);
	void SendLoginReply(ConnectionPtr, char*);
	void SendVersion(ConnectionPtr);
	void SendServerInfo(ConnectionPtr);
	void SendUserStatus(ConnectionPtr);
	void NotifyNewLogin(std::uint32_t);
	
	std::map<std::uint32_t, ConnectionPtr> users;
	std::map<std::uint16_t, Room*> rooms;
	tcp::acceptor listener;
	tcp::socket socket;
	std::string name, media_url;
	std::uint32_t opts, ping, pong, perms, last_user_id;
	std::uint16_t port;
};

#endif // _SERVER_H
