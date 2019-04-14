#include "DaemonApi.h"
#include "Daemon.h"

static Daemon *s_pDaemon = NULL;

void daemon_start(const char* url)
{
	if (s_pDaemon != NULL) {
		delete s_pDaemon;
	}
	s_pDaemon = new Daemon();
	s_pDaemon->connect_mcu(string(url));
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
	s_pDaemon->start_stream();
}

void daemon_stop_stream()
{
	s_pDaemon->stop_stream();
}

void daemon_show_stream(void* hwnd)
{
	s_pDaemon->show_stream((HWND)hwnd);
}

void daemon_send_picture()
{
	s_pDaemon->send_picture();
}

void daemon_set_picture_callback(PictureCallback on_picture)
{
	s_pDaemon->set_picture_callback(on_picture);
}

void daemon_start_operate()
{
	s_pDaemon->start_operate();
}

void daemon_set_operater_callback(OperaterCallback on_operater)
{
	s_pDaemon->set_operater_callback(on_operater);
}

void daemon_send_mouse_event(unsigned int x, unsigned int y, unsigned int button_mask)
{
	s_pDaemon->send_mouse_event(x, y, button_mask);
}

void daemon_set_mouse_callback(MouseCallback on_mouse)
{
	s_pDaemon->set_mouse_callback(on_mouse);
}

void daemon_send_keyboard_event(unsigned int key_val, bool is_pressed)
{
	s_pDaemon->send_keyboard_event(key_val, is_pressed);
}

void daemon_set_keyboard_callback(KeyboardCallback on_keyboard)
{
	s_pDaemon->set_keyboard_callback(on_keyboard);
}