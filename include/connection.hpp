#ifndef _CONNECTION_H
#define _CONNECTION_H

#include <boost/asio.hpp>
#include <map>

#include "globals.hpp"

using boost::asio::ip::tcp;

// disconnect
enum
{
	K_UNKNOWN = 0,
	K_LOGGEDOFF,
	K_COMMERROR,
	K_FLOOD,
	K_KILLEDBYPLAYER,
	K_SERVERDOWN,
	K_UNRESPONSIVE,
	K_KILLEDBYSYSOP,
	K_SERVERFULL,
	K_DUPLICATEUSER = 10,
	K_DEATHPENALTYACTIVE,
	K_BANISHED,
	K_BANISHKILL,
	K_VERBOSE = 16
};

enum
{
	AUX_UNKNOWNMACH = 0,
	AUX_WIN32 = 4,
	AUX_JAVA,
	AUX_OSMASK = 15,
	AUX_AUTHENTICATE = 0x80000000
};

// user status
enum
{
	U_SUPERUSER = 1,
	U_GOD,
	U_KILL = 4,
	U_COMMERROR = 64,
	U_GAG = 128,
	U_PIN = 256,
	U_HIDE = 512,
	U_REJECTESP = 1024,
	U_REJECTPRIVATE = 2048,
	U_PROPGAG = 4096
};

enum
{
	FACE_CLOSED = 0,
	FACE_SMILE,
	FACE_TILTDOWN,
	FACE_TALK,
	FACE_WINKLEFT,
	FACE_NORMAL,
	FACE_WINKRIGHT,
	FACE_TILTLEFT,
	FACE_TILTUP,
	FACE_TILTRIGHT,
	FACE_SAD,
	FACE_BLOTTO,
	FACE_ANGRY,
	NUM_FACES
};

enum
{
	C_RED = 0,
	C_REDORANGE,
	C_ORANGE,
	C_YELLOW,
	C_LIME,
	C_GREEN,
	C_MINT,
	C_TURQUOISE,
	C_CYAN,
	C_AQUA,
	C_BLUE,
	C_PURPLE,
	C_VIOLET,
	C_MAGENTA,
	C_PINK,
	C_HOTPINK,
	NUM_COLORS
};

enum class DrawState: char
{
	NONE = 0,
	PAINT,
	ERASE
};

struct Connection final: std::enable_shared_from_this<Connection>
{
	PropPtr props[9];
	tcp::socket socket;
	std::string name, pw;
	Point pos;
	uint32_t aux;
	int32_t id;
	uint16_t status, room, face, colour;
	DrawState draw_state;
	
	Connection(tcp::socket s): socket(std::move(s)) {}
	std::string Serialise(bool);
};

typedef std::shared_ptr<Connection> ConnectionPtr;
typedef std::pair<int32_t, ConnectionPtr> UserID;

#endif // _CONNECTION_H
