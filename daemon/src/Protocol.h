#pragma once

typedef struct tagWebSocketHeader
{
	unsigned char	version;
	unsigned char	magic;
	unsigned short	type;
	unsigned int	length;
} WebSocketHeader;

typedef struct tagVideoDataHeader {
	unsigned int eFrameType;
	unsigned int sequence;
} VideoDataHeader;

typedef enum tagMsgType
{
	kMsgTypeVideoData = 1000
} MsgType;

#define Swap32IfLE(l) \
	((unsigned int)((((l)& 0xff000000) >> 24) | \
	(((l)& 0x00ff0000) >> 8) | \
	(((l)& 0x0000ff00) << 8) | \
	(((l)& 0x000000ff) << 24)))

#define Swap16IfLE(s) \
	((unsigned short)((((s)& 0xff) << 8) | (((s) >> 8) & 0xff)))

