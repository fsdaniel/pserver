#ifndef _SERVER_H
#define _SERVER_H

#include <set>
#include <vector>

#include "connection.hpp"

// script event mask
enum
{
	PE_SELECT = 1,
	PE_LOCK,
	PE_UNLOCK = 4,
	PE_HIDE = 8,
	PE_SHOW = 16,
	PE_STARTUP = 32,
	PE_ALARM = 64,
	PE_CUSTOM = 128,
	PE_INCHAT = 256,
	PE_PROPCHANGE = 512,
	PE_ENTER = 1024,
	PE_LEAVE = 2048,
	PE_OUTCHAT = 4096,
	PE_SIGNON = 8192,
	PE_SIGNOFF = 0x4000,
	PE_MACRO0 = 0x8000,
	PE_MACRO1 = 0x10000,
	PE_MACRO2 = 0x20000,
	PE_MACRO3 = 0x40000,
	PE_MACRO4 = 0x80000,
	PE_MACRO5 = 0x100000,
	PE_MACRO6 = 0x200000,
	PE_MACRO7 = 0x400000,
	PE_MACRO8 = 0x800000,
	PE_MACRO9 = 0x1000000
};

// hotspot
enum
{
	HS_NORMAL = 0,
	HS_DOOR,
	HS_SHUTABLEDOOR,
	HS_LOCKABLEDOOR,
	HS_BOLT,
	HS_NAVAREA
};

// room status
enum
{
	RF_AUTHORLOCKED = 1,
	RF_PRIVATE,
	RF_NOPAINTING = 4,
	RF_CLOSED = 8,
	RF_CYBORGFREEZONE = 16,
	RF_HIDDEN = 32,
	RF_WIZARDSONLY = 128,
	RF_DROPZONE = 256,
	RF_NOLOOSEPROPS = 512
};

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

struct SpotState final
{
	Point location;
	uint16_t img_id;
	
	std::string Serialise();
};

struct Hotspot final
{
	std::string name, script;
	std::vector<SpotState> states;
	std::vector<Point> points;
	Point location;
	uint32_t script_event_mask, flags;
	uint16_t type, state;
	int16_t id, dest;
	
	std::string Serialise(uint16_t, uint16_t, uint16_t, uint16_t);
};

struct LooseProp final
{
	PropPtr data;
	Point location;
	int32_t refcon;
	
	std::string Serialise();
};

struct Image final
{
	std::string name;
	uint16_t id, alpha;
	
	std::string Serialise(uint16_t);
};

struct Draw final
{
	std::string data;
	uint16_t cmd;
	
	std::string Serialise();
};

class Server;

class Room final
{
public:
	Room(int16_t, Server*);
	void Join(ConnectionPtr);
	void Leave(ConnectionPtr);
	std::string Serialise(bool);
	int TotalPoints() const;
	int TotalStates() const;
	void SendDescription(ConnectionPtr, bool);
	void SendUserList(ConnectionPtr);
	void SendRoomDescEnd(ConnectionPtr, bool);
private:
	std::vector<Image> imgs;
	std::set<ConnectionPtr> users;
	std::vector<Hotspot> spots;
	std::vector<LooseProp> lprops;
	std::vector<Draw> draws;
	std::string name, img_name, artist, pw;
	Server *owner;
	uint32_t faces;
	int16_t id, flags;
};

typedef std::pair<int16_t, Room*> RoomID;

class Server final
{
public:
	Server(boost::asio::io_service&, const tcp::endpoint&);
	void Disconnect(int32_t);
private:
	void Listen();
	void SendID(ConnectionPtr);
	void ReadHeader(ConnectionPtr);
	void ReadLogin(ConnectionPtr);
	void SendLoginReply(ConnectionPtr, char*);
	void SendVersion(ConnectionPtr);
	void SendServerInfo(ConnectionPtr);
	void SendUserStatus(ConnectionPtr);
	void NotifyNewLogin(const ConnectionPtr);
	void SendMediaURL(ConnectionPtr);
	
	std::map<int32_t, ConnectionPtr> users;
	std::map<int16_t, Room*> rooms;
	tcp::acceptor listener;
	tcp::socket socket;
	std::string name, media_url;
	uint32_t opts, ping, pong, perms;
	int32_t last_user_id;
	uint16_t port;
};

#endif // _SERVER_H
