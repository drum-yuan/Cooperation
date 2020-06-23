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
	unsigned int option;
} VideoDataHeader;

typedef struct tagAudioDataHeader {
	unsigned int timestamp;
	unsigned int numFrames;
} AudioDataHeader;

typedef enum tagMsgType
{
	kMsgTypeConnect = 1000,
	kMsgTypePublish = 1001,
	kMsgTypePublishAck = 1002,
	kMsgTypeVideoData = 1003,
	kMsgTypeVideoAck = 1004,
	kMsgTypeRequestKeyFrame = 1005,
	kMsgTypePicture = 1006,
	kMsgTypeOperate = 1007,
	kMsgTypeOperateAck = 1008,
	kMsgTypeMouseEvent = 1009,
	kMsgTypeKeyboardEvent = 1010,
	kMsgTypeStopStream = 1011,
	kMsgTypeStopStreamAck = 1012,
	kMsgTypeHeartbeat = 1013,
	kMsgTypeCursorShape = 1014,
	kMsgTypeAudioData = 1015,
	kMsgTypeStreamOnly = 1016,
	kMsgTypeStreamQuit = 1017,
	kMsgTypeClipboardData = 1018
} MsgType;

#define Swap32IfLE(l) \
	((unsigned int)((((l)& 0xff000000) >> 24) | \
	(((l)& 0x00ff0000) >> 8) | \
	(((l)& 0x0000ff00) << 8) | \
	(((l)& 0x000000ff) << 24)))

#define Swap16IfLE(s) \
	((unsigned short)((((s)& 0xff) << 8) | (((s) >> 8) & 0xff)))

