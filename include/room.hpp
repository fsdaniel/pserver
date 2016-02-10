#ifndef _ROOM_H
#define _ROOM_H

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

struct SpotState final
{
	Point location;
	std::uint16_t img_id;
};

struct Hotspot final
{
	std::vector<std::uint16_t> scripts_offs, points_offs, states_offs;
	Point location;
	std::uint32_t script_event_mask, flags;
	std::uint16_t dest, type, state, name_offs;
};

struct LooseProp final
{
	std::uint32_t flags;
	std::int32_t refcon;
};

struct Image final
{
	std::int32_t refcon;
	std::uint16_t id, name_offs, alpha;
};

struct Draw final
{
	char *data;
	std::uint16_t cmd, size, data_offs;
};

class Room final
{
public:
	Room(std::uint16_t);
	void Join(ConnectionPtr);
	void Leave(ConnectionPtr);
	void SendDescription(ConnectionPtr, bool) const;
private:
	std::vector<std::string> scripts, img_names, spot_names;
	std::vector<Image> imgs;
	std::vector<ConnectionPtr> users;
	std::vector<Hotspot> spots;
	std::vector<LooseProp> lprops;
	std::vector<Point> points;
	std::vector<SpotState> states;
	std::vector<Draw> draws;
	std::string name, img_name, artist, pw;
	std::uint32_t flags, faces;
	std::uint16_t id;
};

typedef std::pair<std::uint16_t, Room*> RoomID;

#endif // _ROOM_H
