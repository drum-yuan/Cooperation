#pragma once
#include "SocketsClient.h"
#include <string>

class Daemon
{
public:
	Daemon();
	~Daemon();

	void set_instance_id(int id);
	void start_publish();
	void start_stream();
	void stop_stream();
	void show_stream(void* hWnd);
	void get_stream_size(int* width, int* height);
	void pause_show();
	void resume_show();
	bool connect_mcu(const string& url);
	void set_start_stream_callback(StartStreamCallback on_stream);
	void set_stop_stream_callback(StopStreamCallback on_stop);
	void set_vapp_start_callback(VappStartCallback on_vapp);
	void set_vapp_stop_callback(VappStopCallback on_vapp_stop);
	void send_picture();
	void set_picture_callback(PictureCallback on_picture);
	void start_operate();
	void set_operater_callback(OperaterCallback on_operater);
	void send_mouse_event(unsigned int x, unsigned int y, unsigned int button_mask);
	void set_mouse_callback(MouseCallback on_mouse);
	void send_keyboard_event(unsigned int key_val, bool is_pressed);
	void set_keyboard_callback(KeyboardCallback on_keyboard);
	void send_cursor_shape(int x, int y, int w, int h, const string& color_bytes, const string& mask_bytes);
	void set_cursor_shape_callback(CursorShapeCallback on_cursor_shape);
	void send_clipboard_data(int data_type, const string& data);
	void set_clipboard_data_callback(ClipboardDataCallback on_clipboard_data);
	UsersInfoInternal get_users_info();

private:
	void OnVideoEncoded(void* data);
	void OnAudioEncoded(unsigned char* data, int len, unsigned int frams_num);
	void OnLockScreen(unsigned char* data, int len);
	void HeartbeatThread();

	SocketsClient m_McuClient;
	Video m_Video;
	Audio m_Audio;
	string m_McuUrl;
	thread* m_pHeartbeatID;
	bool m_IsPublisher;
	bool m_bQuit;
};