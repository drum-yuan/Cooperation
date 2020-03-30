#include "DaemonApi.h"
#include "Daemon.h"

static vector<Daemon*> vecDaemon;

int daemon_create()
{
	Daemon* pDaemon = new Daemon();
	if (pDaemon != NULL) {
		vecDaemon.push_back(pDaemon);
		int ins_id = vecDaemon.size() - 1;
		pDaemon->set_instance_id(ins_id);
		return ins_id;
	}
	else {
		return -1;
	}
}

void daemon_start(int id, const string& url)
{
	Daemon* pDaemon = vecDaemon[id];
	if (pDaemon != NULL) {
		if (!pDaemon->connect_mcu(url)) {
			delete pDaemon;
		}
	}
}

void daemon_stop(int id)
{
	Daemon* pDaemon = vecDaemon[id];
	if (pDaemon != NULL) {
		delete pDaemon;
	}
}

void daemon_start_publish(int id)
{
	Daemon* pDaemon = vecDaemon[id];
	if (pDaemon != NULL) {
		pDaemon->start_publish();
	}
}

void daemon_start_stream(int id)
{
	Daemon* pDaemon = vecDaemon[id];
	if (pDaemon != NULL) {
		pDaemon->start_stream();
	}
}

void daemon_stop_stream(int id)
{
	Daemon* pDaemon = vecDaemon[id];
	if (pDaemon != NULL) {
		pDaemon->stop_stream();
	}
}

void daemon_show_stream(int id, void* hwnd)
{
	Daemon* pDaemon = vecDaemon[id];
	if (pDaemon != NULL) {
		pDaemon->show_stream(hwnd);
	}
}

void daemon_get_stream_size(int id, int* width, int* height)
{
	Daemon* pDaemon = vecDaemon[id];
	if (pDaemon != NULL) {
		pDaemon->get_stream_size(width, height);
	}
}

void daemon_set_start_stream_callback(int id, StartStreamCallback on_stream)
{
	Daemon* pDaemon = vecDaemon[id];
	if (pDaemon != NULL) {
		pDaemon->set_start_stream_callback(on_stream);
	}
}

void daemon_set_stop_stream_callback(int id, StopStreamCallback on_stop)
{
	Daemon* pDaemon = vecDaemon[id];
	if (pDaemon != NULL) {
		pDaemon->set_stop_stream_callback(on_stop);
	}
}

void daemon_set_vapp_start_callback(int id, VappStartCallback on_vapp)
{
	Daemon* pDaemon = vecDaemon[id];
	if (pDaemon != NULL) {
		pDaemon->set_vapp_start_callback(on_vapp);
	}
}

void daemon_set_vapp_stop_callback(int id, VappStopCallback on_vapp_stop)
{
	Daemon* pDaemon = vecDaemon[id];
	if (pDaemon != NULL) {
		pDaemon->set_vapp_stop_callback(on_vapp_stop);
	}
}

void daemon_send_picture(int id)
{
	Daemon* pDaemon = vecDaemon[id];
	if (pDaemon != NULL) {
		pDaemon->send_picture();
	}
}

void daemon_set_picture_callback(int id, PictureCallback on_picture)
{
	Daemon* pDaemon = vecDaemon[id];
	if (pDaemon != NULL) {
		pDaemon->set_picture_callback(on_picture);
	}
}

void daemon_start_operate(int id)
{
	Daemon* pDaemon = vecDaemon[id];
	if (pDaemon != NULL) {
		pDaemon->start_operate();
	}
}

void daemon_set_operater_callback(int id, OperaterCallback on_operater)
{
	Daemon* pDaemon = vecDaemon[id];
	if (pDaemon != NULL) {
		pDaemon->set_operater_callback(on_operater);
	}
}

void daemon_send_mouse_event(int id, unsigned int x, unsigned int y, unsigned int button_mask)
{
	Daemon* pDaemon = vecDaemon[id];
	if (pDaemon != NULL) {
		pDaemon->send_mouse_event(x, y, button_mask);
	}
}

void daemon_set_mouse_callback(int id, MouseCallback on_mouse)
{
	Daemon* pDaemon = vecDaemon[id];
	if (pDaemon != NULL) {
		pDaemon->set_mouse_callback(on_mouse);
	}
}

void daemon_send_keyboard_event(int id, unsigned int key_val, bool is_pressed)
{
	Daemon* pDaemon = vecDaemon[id];
	if (pDaemon != NULL) {
		pDaemon->send_keyboard_event(key_val, is_pressed);
	}
}

void daemon_set_keyboard_callback(int id, KeyboardCallback on_keyboard)
{
	Daemon* pDaemon = vecDaemon[id];
	if (pDaemon != NULL) {
		pDaemon->set_keyboard_callback(on_keyboard);
	}
}

void daemon_send_cursor_shape(int id, int x, int y, int w, int h, const string& color_bytes, const string& mask_bytes)
{
	Daemon* pDaemon = vecDaemon[id];
	if (pDaemon != NULL) {
		pDaemon->send_cursor_shape(x, y, w, h, color_bytes, mask_bytes);
	}
}

void daemon_set_cursor_shape_callback(int id, CursorShapeCallback on_cursor_shape)
{
	Daemon* pDaemon = vecDaemon[id];
	if (pDaemon != NULL) {
		pDaemon->set_cursor_shape_callback(on_cursor_shape);
	}
}

void daemon_get_users_info(int id, UsersInfo* info)
{
	if (!info) {
		return;
	}
	Daemon* pDaemon = vecDaemon[id];
	if (pDaemon != NULL) {
		UsersInfoInternal info_internal = pDaemon->get_users_info();
		info->user_num = info_internal.user_num;
		info->user_list = info_internal.user_list;
		info->publisher = info_internal.publisher;
		info->operater = info_internal.operater;
	}
}