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
	void show_stream(void* hWnd);
	bool connect_mcu(const string& url);
	void set_start_stream_callback(StartStreamCallback on_stream);
	void set_stop_stream_callback(StopStreamCallback on_stop);
	void send_picture();
	void set_picture_callback(PictureCallback on_picture);
	void start_operate();
	void set_operater_callback(OperaterCallback on_operater);
	void send_mouse_event(unsigned int x, unsigned int y, unsigned int button_mask);
	void set_mouse_callback(MouseCallback on_mouse);
	void send_keyboard_event(unsigned int key_val, bool is_pressed);
	void set_keyboard_callback(KeyboardCallback on_keyboard);

private:
	void OnVideoEncoded(void* data);
	void OnLockScreen(unsigned char* data, int len);
	void HeartbeatThread();

	SocketsClient m_McuClient;
	Video m_Video;
	string m_McuUrl;
	thread* m_pHeartbeatID;
	bool m_bQuit;
	StopStreamCallback m_CallbackStop;
	unsigned int m_LastVideoAckSeq;
	bool m_bWait;
};