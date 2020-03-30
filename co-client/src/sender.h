#include <string>
#include <thread>
#ifdef WIN32
#include <windows.h>
#endif

using namespace std;

struct RDSHInfo {
	string rdsh_ip;
	string domain;
	string user;
};

class Sender
{
public:
	Sender(const string& url);
	~Sender();

	bool register_compute_node(const string& app_name, const RDSHInfo& rdsh_info, string& app_guid);
	void unregister_compute_node(const string& app_guid);

	static void recv_mouse_event_callback(unsigned int x, unsigned int y, unsigned int button_mask);
	static void recv_keyboard_event_callback(unsigned int key_val, bool is_pressed);
	static void recv_vapp_start_callback();
	static void recv_vapp_stop_callback();

private:
	void monitor_thread();

	string m_Url;
	string m_ServerIP;
	string m_HostName;
	int m_DaemonId;
	thread* m_MonitorThread;
	bool m_Quit;
	HCURSOR m_hCursor;
	RDSHInfo m_RDSHInfo;
	string m_AppName;
	string m_SiriusUrl;
};