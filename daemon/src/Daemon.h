#pragma once

#include "SocketsClient.h"
#include "Video.h"
#include "Buffer.h"
#include <string>

using namespace std;
class Daemon
{
public:
	Daemon();
	~Daemon();

	void connect_mcu(const string& url);
	void connect_proxy(const string& url);
	void start_stream();
	void stop_stream();

private:
	unsigned int CalcFrameSize(void* data);
	void onVideoEncoded(void* data);

	SocketsClient m_McuClient;
	SocketsClient m_ProxyClient;
	Video m_Video;
	Buffer* m_SendBuf;
	int m_McuUserID;
	int m_ProxyUserID;
};