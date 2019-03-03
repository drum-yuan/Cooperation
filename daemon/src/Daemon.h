#pragma once

#include "SocketsClient.h"
#include "Video.h"

class Daemon
{
public:
	Daemon();
	~Daemon();

	void start_stream();
	void stop_stream();

private:
	void onVideoEncoded(void* data);

	SocketsClient m_McuClient;
	SocketsClient m_ProxyClient;
	Video m_Video;
	
};