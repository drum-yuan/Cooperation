#include "DaemonApi.h"
#include "Daemon.h"

static Daemon *s_pDaemon = NULL;

bool daemon_start(const char* url)
{
	if (s_pDaemon != NULL) {
		delete s_pDaemon;
	}
	s_pDaemon = new Daemon();
	return s_pDaemon->connect_mcu(string(url));
}

void daemon_stop()
{
	if (s_pDaemon != NULL) {
		delete s_pDaemon;
		s_pDaemon = NULL;
	}
}

void daemon_start_stream()
{
	if (s_pDaemon != NULL) {
		s_pDaemon->start_stream();
	}
}

void daemon_stop_stream()
{
	if (s_pDaemon != NULL) {
		s_pDaemon->stop_stream();
	}
}

void daemon_show_stream(void* hwnd)
{
	if (s_pDaemon != NULL) {
		s_pDaemon->show_stream(hwnd);
	}
}

void daemon_get_stream_size(int* width, int* height)
{
	if (s_pDaemon != NULL) {
		s_pDaemon->get_stream_size(width, height);
	}
}

void daemon_set_start_stream_callback(StartStreamCallback on_stream)
{
	if (s_pDaemon != NULL) {
		s_pDaemon->set_start_stream_callback(on_stream);
	}
}

void daemon_set_stop_stream_callback(StopStreamCallback on_stop)
{
	if (s_pDaemon != NULL) {
		s_pDaemon->set_stop_stream_callback(on_stop);
	}
}

void daemon_send_picture()
{
	if (s_pDaemon != NULL) {
		s_pDaemon->send_picture();
	}
}

void daemon_set_picture_callback(PictureCallback on_picture)
{
	if (s_pDaemon != NULL) {
		s_pDaemon->set_picture_callback(on_picture);
	}
}

void daemon_start_operate()
{
	if (s_pDaemon != NULL) {
		s_pDaemon->start_operate();
	}
}

void daemon_set_operater_callback(OperaterCallback on_operater)
{
	if (s_pDaemon != NULL) {
		s_pDaemon->set_operater_callback(on_operater);
	}
}

void daemon_send_mouse_event(unsigned int x, unsigned int y, unsigned int button_mask)
{
	if (s_pDaemon != NULL) {
		s_pDaemon->send_mouse_event(x, y, button_mask);
	}
}

void daemon_set_mouse_callback(MouseCallback on_mouse)
{
	if (s_pDaemon != NULL) {
		s_pDaemon->set_mouse_callback(on_mouse);
	}
}

void daemon_send_keyboard_event(unsigned int key_val, bool is_pressed)
{
	if (s_pDaemon != NULL) {
		s_pDaemon->send_keyboard_event(key_val, is_pressed);
	}
}

void daemon_set_keyboard_callback(KeyboardCallback on_keyboard)
{
	if (s_pDaemon != NULL) {
		s_pDaemon->set_keyboard_callback(on_keyboard);
	}
}