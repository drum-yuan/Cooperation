#pragma once
#include "SocketsClient.h"
#include <string>

class Daemon
{
public:
	Daemon();
	~Daemon();

	void start_stream();
	void stop_stream();
	void show_stream(HWND hWnd);
	bool connect_mcu(const string& url);
	void set_recv_callback(RecvCallback on_recv);
	void send_picture();
	void start_operate();

private:
	void OnVideoEncoded(void* data);
	void OnLockScreen(unsigned char* data, int len);

	SocketsClient m_McuClient;
	Video m_Video;
	string m_McuUrl;
};