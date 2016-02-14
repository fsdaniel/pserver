#ifndef _GLOBALS_H
#define _GLOBALS_H

#include <cstdint>
#include <memory>
#include <string>

// error
enum
{
	SE_INTERNALERROR = 0,
	SE_ROOMUNKNOWN,
	SE_ROOMFULL,
	SE_ROOMCLOSED,
	SE_CANTAUTHOR,
	SE_PALACEFULL
};

// prop flags
enum
{
	P_8BIT = 0,
	P_HEAD = 2,
	P_GHOST = 4,
	P_RARE = 8,
	P_ANIMATE = 16,
	P_BOUNCE = 32,
	P_20BIT = 64,
	P_32BIT = 256,
	P_S20BIT = 512,
	P_HIDEF = 1024,
	P_FORMATS = (P_20BIT | P_32BIT | P_S20BIT | P_HIDEF),
	P_LEGACY = 0xFFC1
};

enum
{
	MSG_HTTPSERVER = 0x48545450,
	MSG_AUTHENTICATE = 0x61757468,
	MSG_AUTHRESPONSE = 0x61757472,
	MSG_BLOWTHRU = 0x626C6F77,
	MSG_LOGOFF = 0x62796520,
	MSG_SPOTMOVE = 0x636F4C73,
	MSG_PROPDEL = 0x64507270,
	MSG_SERVERDOWN = 0x646F776E,
	MSG_DRAW = 0x64726177,
	MSG_DISPLAYURL = 0x6475726C,
	MSG_ROOMDESCEND = 0x656E6472,
	MSG_USEREXIT = 0x65707273,
	MSG_FILENOTFND = 0x666E6665,
	MSG_GMSG = 0x676D7367,
	MSG_KILLUSER = 0x6B696C6C,
	MSG_DOORLOCK = 0x6C6F636B,
	MSG_USERLOG = 0x6C6F6720,
	MSG_PROPMOVE = 0x6D507270,
	MSG_PROPNEW = 0x6E507270,
	MSG_ROOMNEW = 0x6E526F6D,
	MSG_ROOMGOTO = 0x6E617652,
	MSG_USERNEW = 0x6E707273,
	MSG_SPOTDEL = 0x6F705364,
	MSG_SPOTNEW = 0x6F70536E,
	MSG_PICTMOVE = 0x704C6F63,
	MSG_PING = 0x70696E67,
	MSG_PONG = 0x706F6E67,
	MSG_ASSETQUERY = 0x71417374,
	MSG_FILEQUERY = 0x7146696C,
	MSG_ASSETREGI = 0x72417374,
	MSG_LISTOFALLROOMS = 0x724C7374,
	MSG_LOGON = 0x72656769,
	MSG_ALTLOGONREPLY = 0x72657032,
	MSG_RMSG = 0x726D7367,
	MSG_ROOMDESC = 0x726F6F6D,
	MSG_USERLIST = 0x72707273,
	MSG_ASSETSEND = 0x73417374,
	MSG_NAVERROR = 0x73457272,
	MSG_FILESEND = 0x7346696C,
	MSG_EXTENDEDINFO = 0x73496E66,
	MSG_SERVERINFO = 0x73696E66,
	MSG_SMSG = 0x736D7367,
	MSG_ROOMSETDESC = 0x73526F6D,
	MSG_SPOTSTATE = 0x73537461,
	MSG_DOORUNLOCK = 0x756E6C6F,
	MSG_SUPERUSER = 0x73757372,
	MSG_TALK = 0x74616C6B,
	MSG_TIYID = 0x74697972,
	MSG_USERMOVE = 0x754C6F63,
	MSG_LISTOFALLUSERS = 0x754C7374,
	MSG_USERSTATUS = 0x75537461,
	MSG_USERCOLOR = 0x75737243,
	MSG_USERDESC,
	MSG_USERFACE = 0x75737246,
	MSG_USERNAME = 0x7573724E,
	MSG_USERPROP = 0x75737250,
	MSG_VERSION = 0x76657273,
	MSG_WHISPER = 0x77686973,
	MSG_WMSG = 0x776D7367,
	MSG_USERENTER = 0x77707273,
	MSG_XTALK = 0x78746C6B,
	MSG_XWHISPER = 0x78776973
};

struct Message final
{
	char *data;
	std::uint32_t type;
	std::uint32_t size;
	std::int32_t refnum;
	
	const char* Serialise() const;
};

struct Point final
{
	std::int16_t x, y;
};

// omit 'type' as the only value applicable is 0x50726F70 ('Prop')
struct Prop final: std::enable_shared_from_this<Prop>
{
	char *data;
	std::string name;
	std::uint32_t id, crc, block_size, block_offs, flags, size;
	std::uint16_t block_num, nblocks;
	
	void ComputeCRC();
};

typedef std::shared_ptr<Prop> PropPtr;

void Encode(char*, short);
void Decode(char*, short);
void Log(std::string);

#endif // _GLOBALS_H
