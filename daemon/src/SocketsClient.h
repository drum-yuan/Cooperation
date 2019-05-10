#pragma once
#include <thread>
#include <mutex>
#include <string>
#include "libwebsockets.h"
#include "Video.h"
#include "Buffer.h"

using namespace std;

typedef void(*StartStreamCallback)(void);
typedef void(*StopStreamCallback)(void);
typedef void(*PictureCallback)(const char* file_path);
typedef void(*OperaterCallback)(bool is_operater);
typedef void(*MouseCallback)(unsigned int x, unsigned int y, unsigned int button_mask);
typedef void(*KeyboardCallback)(unsigned int key_val, bool is_pressed);
class SocketsClient
{
public:
	SocketsClient();
	~SocketsClient();

	void set_video_event(Video* pEvent);
	bool connect(std::string url, bool blocking, bool ssl);
	void stop();
	void reset();
	bool is_connected();
	unsigned int get_video_ack_seq();

	int send_msg(unsigned char* payload, unsigned int msglen);
	void set_start_stream_callback(StartStreamCallback on_stream);
	void set_picture_callback(PictureCallback on_recv);
	void set_operater_callback(OperaterCallback on_operater);
	void set_mouse_callback(MouseCallback on_mouse);
	void set_keyboard_callback(KeyboardCallback on_keyboard);
	void handle_in(struct lws *wsi, const void* data, size_t len);

	void send_connect();
	void send_publish();
	void send_video_data(void* data);
	void send_video_ack(unsigned int sequence);
	void send_keyframe_request(bool reset_seq);
	void send_picture_data(unsigned char* data, int len);
	void send_operate();
	void send_mouse_event(unsigned int x, unsigned int y, unsigned int button_mask);
	void send_keyboard_event(unsigned int key_val, bool is_pressed);

	static int callback_client(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

private:
	void NotifyForceExit();
	void SetConnectState(int connstate);
	int RunWebSocketClient();
	unsigned int CalcFrameSize(void* data);

	bool m_Exit;
	int  m_State;
	bool   m_UseSSL;
	string m_ServerUrl;
	char m_FilePath[256];
	FILE* m_PicFile;

	lws* m_wsi;
	thread* m_wsthread;
	condition_variable m_cv;
	struct lws_protocols* m_protocols;
	struct lws_extension* m_exts;

	Buffer* m_SendBuf;
	Video* m_pVideo;
	StartStreamCallback m_CallbackStream;
	PictureCallback m_CallbackPicture;
	OperaterCallback m_CallbackOperater;
	MouseCallback m_CallbackMouse;
	KeyboardCallback m_CallbackKeyboard;
	unsigned int m_CurVideoAckSeq;
};