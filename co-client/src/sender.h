#pragma once

#include "co-interface.h"
#include <thread>
#include <queue>
#include "util.h"
#ifndef WIN32
typedef void  *HCURSOR;
typedef GtkWidget  *HWND;
#endif

class Sender
{
public:
	Sender(const string& url);
	~Sender();

	bool register_compute_node(const string& app_name, const RDSHInfo& rdsh_info, string& app_guid);
	void unregister_compute_node(const string& app_guid);
	bool start_compute_node(const string& app_name, const RDSHInfo& rdsh_info);
	void stop_compute_node();

	static void recv_mouse_event_callback(unsigned int x, unsigned int y, unsigned int button_mask);
	static void recv_keyboard_event_callback(unsigned int key_val, bool is_pressed);
	static void recv_vapp_start_callback();
	static void recv_vapp_stop_callback();

private:
	void monitor_thread();
#ifdef WIN32
	void desktop_event_thread();
#endif
	void start_vapp();
	void switch_input_desktop();

	string m_Url;
	string m_ServerIP;
	string m_HostName;
	int m_DaemonId;
	thread* m_MonitorThread;
	thread* m_DesktopEventThread;
	bool m_VappQuit;
	bool m_EventRunning;
	bool m_DesktopSwitch;
	HCURSOR m_hCursor;
	RDSHInfo m_RDSHInfo;
	string m_AppGuid;
	string m_AppName;
	string m_SiriusUrl;
};
