#include <vector>
#include <map>
#include <thread>
#ifdef WIN32
#include <windows.h>
#else
#include <gdk/gdkx.h>
#include <gtk/gtk.h>

typedef GtkWidget  *HWND;
#endif

using namespace std;

struct NodeInfo {
	string app_guid;
	string app_name;
	string sirius_url;
	int status;
};

struct DaemonInfo {
	string app_guid;
	HWND hwnd;
	bool can_operate;
	bool is_fullscreen;
};

class Receiver
{
public:
	Receiver(const string& url);
	~Receiver();

	int get_compute_node_list(vector<NodeInfo>& node_list);
	int start(const NodeInfo& node);
	void stop(int ins_id);
	void start_operate(int ins_id);
	void set_fullscreen(int ins_id);

	void set_cursor_shape();
	string get_guid_from_daemon_map(HWND hwnd);
	int get_id_from_daemon_map(HWND hwnd);
	bool get_can_operate_from_daemon_map(HWND hwnd);
	HWND get_hwnd_from_ins_id(int id);

	static void start_stream_callback(int id);
	static void stop_stream_callback(int id);
	static void start_operate_callback(int id, bool is_operater);
	static void recv_cursor_shape_callback(int id, int x, int y, int w, int h, const std::string& color_bytes, const std::string& mask_bytes);
#ifdef WIN32
	HCURSOR m_hCursor;
#endif
private:
	void CreateWndInThread(int id);

	string m_Url;
	string m_ServerIP;
	map<int, DaemonInfo> m_DaemonMap;
	map<int, thread*> m_WinThreadMap;
};
