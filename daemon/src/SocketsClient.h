#pragma once
#include <thread>
#include <mutex>
#include <string>
#include "libwebsockets.h"
#include "Video.h"
#include "Buffer.h"

using namespace std;

typedef void(*RecvCallback)(const void* data, int len);

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

	int send_msg(unsigned char* payload, unsigned int msglen);
	void set_recv_callback(RecvCallback on_recv);
	void handle_in(struct lws *wsi, const void* data, size_t len);

	void send_connect();
	void send_video_data(void* data);
	void send_video_ack(unsigned int sequence);
	void send_keyframe_request(bool reset_seq);

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

	lws* m_wsi;
	thread* m_wsthread;
	condition_variable m_cv;
	struct lws_protocols* m_protocols;
	struct lws_extension* m_exts;

	Buffer* m_SendBuf;
	Video* m_pVideo;
	RecvCallback m_CallbackRecv;
};