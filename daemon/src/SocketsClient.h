#pragma once
#include <thread>
#include <mutex>
#include <string>
#include "libwebsockets.h"

class SocketsClient
{
public:
	SocketsClient();
	~SocketsClient();

	bool connect(std::string url, bool blocking, bool ssl);
	void stop();
	void reset();
	bool is_connected();

	int send_msg(unsigned char* payload, unsigned int msglen);
	int recv_msg(unsigned char* payload, unsigned int msglen);

	void handle_in(struct lws *wsi, const void* data, size_t len);
	static int callback_client(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

private:
	void NotifyForceExit();
	void SetConnectState(int connstate);
	int RunWebSocketClient();

	bool m_Exit;
	int  m_State;
	bool   m_UseSSL;
	std::string m_ServerUrl;

	lws* m_wsi;
	std::thread* m_wsthread;
	std::condition_variable m_cv;
	struct lws_protocols* m_protocols;
	struct lws_extension* m_exts;
};