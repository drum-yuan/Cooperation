#include "receiver.h"
#include "DaemonApi.h"
#include "restbed"
#include "cjson.h"
#ifdef WIN32
#include "floatbar.h"
#include "imm.h"
#pragma comment(lib, "imm32.lib")
#endif

using namespace restbed;

#define AGENT_LBUTTON_MASK (1 << 1)
#define AGENT_MBUTTON_MASK (1 << 2)
#define AGENT_RBUTTON_MASK (1 << 3)
#define AGENT_UBUTTON_MASK (1 << 4)
#define AGENT_DBUTTON_MASK (1 << 5)

typedef unsigned long  DWORD;

static DWORD _dwButtonState = 0;
static uint64_t _lastMouseMove = 0;
static Receiver* _instance = NULL;
Receiver::Receiver(const string& url)
{
	m_Url = url;

	m_ServerIP = m_Url;
	string::size_type pos = m_Url.find(':');
	if (pos != string::npos) {
		m_ServerIP = m_Url.substr(0, pos);
	}
#ifdef WIN32
	m_hCursor = NULL;
#endif
	_instance = this;
}

Receiver::~Receiver()
{

}

int Receiver::get_compute_node_list(vector<NodeInfo>& node_list)
{
	node_list.clear();
	string strUrl = "http://" + m_Url + "/nodelist";
	LOG_INFO("str url %s", strUrl.c_str());
	auto request = std::make_shared< Request >(Uri(strUrl));
	request->set_method("GET");
	request->set_header("Host", m_ServerIP);
	auto response = Http::sync(request);
	if (response->get_status_code() != OK) {
		LOG_ERROR("response status code %d", response->get_status_code());
		return -1;
	}
	int body_len = stoi(response->get_header("Content-Length"));
	LOG_INFO("response body len %d", body_len);
	if (body_len == 0) {
		return 0;
	}
	Http::fetch(body_len, response);
	std::string content((char*)response->get_body().data(), body_len);
	LOG_INFO("get node list request succeed %s", content.c_str());

	cJSON *root = cJSON_Parse(content.c_str());
	int node_num = cJSON_GetArraySize(root);
	cJSON *item = NULL;
	for (int i = 0; i < node_num; i++) {
		item = cJSON_GetArrayItem(root, i);
		NodeInfo info;
		cJSON *sub_item = cJSON_GetObjectItem(item, "app_guid");
		if (sub_item != NULL) {
			info.app_guid = sub_item->valuestring;
		}
		sub_item = cJSON_GetObjectItem(item, "app_name");
		if (sub_item != NULL) {
			info.app_name = sub_item->valuestring;
		}
		sub_item = cJSON_GetObjectItem(item, "sirius_port");
		if (sub_item != NULL) {
			info.sirius_url = m_ServerIP + ":" + to_string(sub_item->valueint);
		}
		sub_item = cJSON_GetObjectItem(item, "host_name");
		if (sub_item != NULL) {
			info.host_name = sub_item->valuestring;
		}
		sub_item = cJSON_GetObjectItem(item, "status");
		if (sub_item != NULL) {
			info.status = sub_item->valueint;
		}
		LOG_INFO("node %s name %s sirius url %s", info.app_guid.c_str(), info.app_name.c_str(), info.sirius_url.c_str());
		node_list.push_back(info);
	}
	return node_list.size();
}

int Receiver::start(const NodeInfo& node)
{
	int ins_id = daemon_create();
	if (ins_id < 0) {
		LOG_ERROR("daemon start fail");
		return -1;
	}
	m_WinThreadMap.insert(pair<int, thread*>(ins_id, NULL));
	daemon_set_start_stream_callback(ins_id, start_stream_callback);
	daemon_set_stop_stream_callback(ins_id, stop_stream_callback);
	daemon_set_operater_callback(ins_id, start_operate_callback);
	daemon_set_cursor_shape_callback(ins_id, recv_cursor_shape_callback);
	daemon_set_picture_callback(ins_id, recv_picture_callback);
	daemon_start(ins_id, node.sirius_url);
	DaemonInfo info;
	info.app_guid = node.app_guid;
	info.hwnd = NULL;
	info.can_operate = false;
	info.is_fullscreen = false;
	m_DaemonMap.insert(pair<int, DaemonInfo>(ins_id, info));
	LOG_INFO("daemon start %d guid %s name %s", ins_id, node.app_guid.c_str(), node.app_name.c_str());
	return ins_id;
}

void Receiver::stop(int ins_id, bool close_win)
{
	if (close_win) {
		PostMessage(m_DaemonMap[ins_id].hwnd, WM_CLOSE, 0, 0);
	}
	else {
		daemon_stop(ins_id);
		m_DaemonMap.erase(ins_id);
		m_WinThreadMap.erase(ins_id);
		LOG_INFO("daemon stop %d", ins_id);
	}
}

void Receiver::show(int ins_id)
{
	ShowWindow(m_DaemonMap[ins_id].hwnd, SW_SHOW);
}

void Receiver::hide(int ins_id)
{
	ShowWindow(m_DaemonMap[ins_id].hwnd, SW_HIDE);
}

void Receiver::start_operate(int ins_id)
{
	LOG_INFO("daemon start operate %d", ins_id);
	daemon_start_operate(ins_id);
}

void Receiver::set_fullscreen(int ins_id)
{
	if (m_DaemonMap.find(ins_id) != m_DaemonMap.end()) {
		m_DaemonMap[ins_id].is_fullscreen = true;
	}
}

void Receiver::get_users_info(int ins_id, CoUsersInfo& users_info)
{
	LOG_INFO("daemon get users info");
	UsersInfo info;
	daemon_get_users_info(ins_id, &info);
	users_info.user_num = info.user_num;
	users_info.user_list = info.user_list;
	users_info.publisher = info.publisher;
	users_info.operater = info.operater;
}

vector<int> Receiver::get_current_receiver()
{
	vector<int> ins_id_list;
	for (auto &it : m_DaemonMap) {
		ins_id_list.push_back(it.first);
	}
	return ins_id_list;
}

void Receiver::set_cursor_shape()
{
#ifdef WIN32
	SetCursor(m_hCursor);
#endif
}

string Receiver::get_guid_from_daemon_map(HWND hwnd)
{
	string app_guid;
	for (auto &it : m_DaemonMap) {
		if (it.second.hwnd == hwnd) {
			app_guid = it.second.app_guid;
			break;
		}
	}
	return app_guid;
}

int Receiver::get_id_from_daemon_map(HWND hwnd)
{
	int ins_id = -1;
	for (auto &it : m_DaemonMap) {
		if (it.second.hwnd == hwnd) {
			ins_id = it.first;
			break;
		}
	}
	return ins_id;
}

bool Receiver::get_can_operate_from_daemon_map(HWND hwnd)
{
	bool can_operate = false;
	for (auto &it : m_DaemonMap) {
		if (it.second.hwnd == hwnd) {
			can_operate = it.second.can_operate;
			break;
		}
	}
	return can_operate;
}

HWND Receiver::get_hwnd_from_ins_id(int id)
{
	if (m_DaemonMap.find(id) != m_DaemonMap.end()) {
		return m_DaemonMap[id].hwnd;
	}
	else {
		return NULL;
	}
}

void Receiver::start_stream_callback(int id)
{
	LOG_INFO("id %d start stream callback", id);
	if (_instance->m_WinThreadMap[id] == NULL) {
		_instance->m_WinThreadMap[id] = new thread(&Receiver::CreateWndInThread, _instance, id);
	}
}

void Receiver::stop_stream_callback(int id)
{
	LOG_INFO("id %d stop stream callback", id);
	HWND hwnd = _instance->get_hwnd_from_ins_id(id);
	if (hwnd != NULL) {
#ifdef WIN32
		::PostMessage(hwnd, WM_CLOSE, 0, 0);
#else
		g_signal_emit_by_name(G_OBJECT(hwnd), "destroy", NULL);
#endif
	}
}

void Receiver::start_operate_callback(int id, bool is_operater)
{
	LOG_INFO("id %d start operate callback %d", id, is_operater);
	_instance->m_DaemonMap[id].can_operate = is_operater;
}

void Receiver::recv_cursor_shape_callback(int id, int x, int y, int w, int h, const string& color_bytes, const string& mask_bytes)
{
#ifdef WIN32
	ICONINFO info;
	info.fIcon = FALSE;
	info.xHotspot = x;
	info.yHotspot = y;
	info.hbmMask = CreateBitmap(w, h, 1, 1, mask_bytes.c_str());
	info.hbmColor = CreateBitmap(w, h, 1, 32, color_bytes.c_str());
	_instance->m_hCursor = CreateIconIndirect(&info);
#else
	HWND hwnd = _instance->get_hwnd_from_ins_id(id);
	if (hwnd == NULL) {
		return;
	}
	GdkPixbuf *cursor_buf = gdk_pixbuf_new_from_data((guchar*)color_bytes.c_str(), GDK_COLORSPACE_RGB, TRUE, 8, w, h, w * 4, (GdkPixbufDestroyNotify)g_free, NULL);
	GdkCursor *cursor = gdk_cursor_new_from_pixbuf(gtk_widget_get_display(hwnd), cursor_buf, x, y);
	GdkWindow *window = GDK_WINDOW(gtk_widget_get_window(hwnd));
	if (gdk_window_get_cursor(window) != cursor) {
		gdk_window_set_cursor(window, cursor);
	}
#endif
}

void Receiver::recv_picture_callback(int id, const char* file_path)
{
#ifdef WIN32
	LOG_INFO("recv picture %s", file_path);
	ShellExecute(NULL, "open", "mspaint", file_path, NULL, SW_SHOW);
#else
#endif
}

static void scale_to_video(int id, HWND hwnd, short x, short y, unsigned int& scale_x, unsigned int& scale_y)
{
	if (id < 0) {
		scale_x = 0;
		scale_y = 0;
		return;
	}

	int video_w = 1366;
	int video_h = 768;
	daemon_get_stream_size(id, &video_w, &video_h);
#ifdef WIN32
	RECT rect;
	GetClientRect(hwnd, &rect);
	int win_w = rect.right - rect.left;
	int win_h = rect.bottom - rect.top;
#else
	int win_w = 0;
	int win_h = 0;
	gtk_window_get_size(GTK_WINDOW(hwnd), &win_w, &win_h);
#endif

	if (video_w > 0 && video_h > 0 && win_w > 0 && win_h > 0) {
		scale_x = x * video_w / win_w;
		scale_y = y * video_h / win_h;
	}
}

#ifdef WIN32
LRESULT CALLBACK WndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	unsigned int x = 0;
	unsigned int y = 0;

	switch (wMsg) {
	case WM_SETCURSOR:
	{
		if (_instance->m_hCursor) {
			_instance->set_cursor_shape();
			return 0;
		}
	}
	break;
	case WM_CLOSE:
	{
		_instance->stop(_instance->get_id_from_daemon_map(hWnd));
	}
	break;
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	case WM_USER + MSG_PAUSE_RENDER:
	{
		daemon_pause_show(_instance->get_id_from_daemon_map(hWnd));
	}
		break;
	case WM_USER + MSG_RESUME_RENDER:
	{
		daemon_resume_show(_instance->get_id_from_daemon_map(hWnd));
	}
		break;
	case WM_MOUSEWHEEL:
	{
		if (_instance->get_can_operate_from_daemon_map(hWnd)) {
			int ins_id = _instance->get_id_from_daemon_map(hWnd);
			scale_to_video(ins_id, hWnd, LOWORD(lParam), HIWORD(lParam), x, y);
			short offset = HIWORD(wParam);
			if (offset > 0) {
				_dwButtonState |= AGENT_UBUTTON_MASK;
			}
			else {
				_dwButtonState |= AGENT_DBUTTON_MASK;
			}
			daemon_send_mouse_event(ins_id, x, y, _dwButtonState);
			if (offset > 0) {
				_dwButtonState &= ~AGENT_UBUTTON_MASK;
			}
			else {
				_dwButtonState &= ~AGENT_DBUTTON_MASK;
			}
			return 0;
		}
	}
	break;
	case WM_MOUSEMOVE:
	{
		if (_instance->get_can_operate_from_daemon_map(hWnd)) {
			int ins_id = _instance->get_id_from_daemon_map(hWnd);
			uint64_t cur_timestamp = get_cur_timestamp();
			if (cur_timestamp > _lastMouseMove + 20) {
				_lastMouseMove = cur_timestamp;
				scale_to_video(ins_id, hWnd, LOWORD(lParam), HIWORD(lParam), x, y);
				daemon_send_mouse_event(ins_id, x, y, _dwButtonState);
			}
			return 0;
		}
	}
	break;
	case WM_LBUTTONDOWN:
	{
		if (_instance->get_can_operate_from_daemon_map(hWnd)) {
			int ins_id = _instance->get_id_from_daemon_map(hWnd);
			scale_to_video(ins_id, hWnd, LOWORD(lParam), HIWORD(lParam), x, y);
			_dwButtonState |= AGENT_LBUTTON_MASK;
			daemon_send_mouse_event(ins_id, x, y, _dwButtonState);
			return 0;
		}
	}
	break;
	case WM_MBUTTONDOWN:
	{
		if (_instance->get_can_operate_from_daemon_map(hWnd)) {
			int ins_id = _instance->get_id_from_daemon_map(hWnd);
			scale_to_video(ins_id, hWnd, LOWORD(lParam), HIWORD(lParam), x, y);
			_dwButtonState |= AGENT_MBUTTON_MASK;
			daemon_send_mouse_event(ins_id, x, y, _dwButtonState);
			return 0;
		}
	}
	break;
	case WM_RBUTTONDOWN:
	{
		if (_instance->get_can_operate_from_daemon_map(hWnd)) {
			int ins_id = _instance->get_id_from_daemon_map(hWnd);
			scale_to_video(ins_id, hWnd, LOWORD(lParam), HIWORD(lParam), x, y);
			_dwButtonState |= AGENT_RBUTTON_MASK;
			daemon_send_mouse_event(ins_id, x, y, _dwButtonState);
			return 0;
		}
	}
	break;
	case WM_LBUTTONUP:
	{
		if (_instance->get_can_operate_from_daemon_map(hWnd)) {
			int ins_id = _instance->get_id_from_daemon_map(hWnd);
			scale_to_video(ins_id, hWnd, LOWORD(lParam), HIWORD(lParam), x, y);
			_dwButtonState &= ~AGENT_LBUTTON_MASK;
			daemon_send_mouse_event(ins_id, x, y, _dwButtonState);
			return 0;
		}
	}
	break;
	case WM_MBUTTONUP:
	{
		if (_instance->get_can_operate_from_daemon_map(hWnd)) {
			int ins_id = _instance->get_id_from_daemon_map(hWnd);
			scale_to_video(ins_id, hWnd, LOWORD(lParam), HIWORD(lParam), x, y);
			_dwButtonState &= ~AGENT_MBUTTON_MASK;
			daemon_send_mouse_event(ins_id, x, y, _dwButtonState);
			return 0;
		}
	}
	break;
	case WM_RBUTTONUP:
	{
		if (_instance->get_can_operate_from_daemon_map(hWnd)) {
			int ins_id = _instance->get_id_from_daemon_map(hWnd);
			scale_to_video(ins_id, hWnd, LOWORD(lParam), HIWORD(lParam), x, y);
			_dwButtonState &= ~AGENT_RBUTTON_MASK;
			daemon_send_mouse_event(ins_id, x, y, _dwButtonState);
			return 0;
		}
	}
	break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	{
		if (_instance->get_can_operate_from_daemon_map(hWnd)) {
			int ins_id = _instance->get_id_from_daemon_map(hWnd);
			daemon_send_keyboard_event(ins_id, wParam, true);
			return 0;
		}
	}
	break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
	{
		if (_instance->get_can_operate_from_daemon_map(hWnd)) {
			int ins_id = _instance->get_id_from_daemon_map(hWnd);
			daemon_send_keyboard_event(ins_id, wParam, false);
			return 0;
		}
	}
	break;
	default:
		break;
	}
	return DefWindowProc(hWnd, wMsg, wParam, lParam);
}
#else
static void on_window_closed(GtkWidget *widget, gpointer *data)
{
	gtk_main_quit();
	_instance->stop(_instance->get_id_from_daemon_map(widget));
}

static void button_press_event(GtkWidget *widget, GdkEventButton *button)
{
	if (!_instance->get_can_operate_from_daemon_map(widget)) {
		return;
	}
	unsigned int x = 0;
	unsigned int y = 0;
	int ins_id = _instance->get_id_from_daemon_map(widget);
	scale_to_video(ins_id, widget, button->x, button->y, x, y);
	if (button->button == 1) {
		_dwButtonState |= AGENT_LBUTTON_MASK;
	}
	else if (button->button == 2) {
		_dwButtonState |= AGENT_MBUTTON_MASK;
	}
	else if (button->button == 3) {
		_dwButtonState |= AGENT_RBUTTON_MASK;
	}
	daemon_send_mouse_event(ins_id, x, y, _dwButtonState);
}

static void button_release_event(GtkWidget *widget, GdkEventButton *button)
{
	if (!_instance->get_can_operate_from_daemon_map(widget)) {
		return;
	}
	unsigned int x = 0;
	unsigned int y = 0;
	int ins_id = _instance->get_id_from_daemon_map(widget);
	scale_to_video(ins_id, widget, button->x, button->y, x, y);
	if (button->button == 1) {
		_dwButtonState &= ~AGENT_LBUTTON_MASK;
	}
	else if (button->button == 2) {
		_dwButtonState &= ~AGENT_MBUTTON_MASK;
	}
	else if (button->button == 3) {
		_dwButtonState &= ~AGENT_RBUTTON_MASK;
	}
	daemon_send_mouse_event(ins_id, x, y, _dwButtonState);
}

static void motion_notify_event(GtkWidget *widget, GdkEventMotion *motion)
{
	if (!_instance->get_can_operate_from_daemon_map(widget)) {
		return;
	}
	uint64_t cur_timestamp = get_cur_timestamp();
	if (cur_timestamp < _lastMouseMove + 20) {
		return;
	}
	_lastMouseMove = cur_timestamp;

	unsigned int x = 0;
	unsigned int y = 0;
	int ins_id = _instance->get_id_from_daemon_map(widget);
	scale_to_video(ins_id, widget, motion->x, motion->y, x, y);
	daemon_send_mouse_event(ins_id, x, y, _dwButtonState);
}

static void key_press_event(GtkWidget *widget, GdkEventKey *key)
{
	if (!_instance->get_can_operate_from_daemon_map(widget)) {
		return;
	}
	int ins_id = _instance->get_id_from_daemon_map(widget);
	daemon_send_keyboard_event(ins_id, key->keyval, true);
}

static void key_release_event(GtkWidget *widget, GdkEventKey *key)
{
	if (!_instance->get_can_operate_from_daemon_map(widget)) {
		return;
	}
	int ins_id = _instance->get_id_from_daemon_map(widget);
	daemon_send_keyboard_event(ins_id, key->keyval, false);
}
#endif

void Receiver::CreateWndInThread(int id)
{
#ifdef WIN32
	WNDCLASSEXA wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = GetModuleHandle(nullptr);
	wcex.hIcon = 0;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;

	char cName[MAX_PATH] = { 0 };
	GetModuleFileNameA(wcex.hInstance, cName, sizeof(cName));
	char* szApp = strrchr(cName, '\\') + 1;
	strchr(szApp, '.')[0] = '\0';

	wcex.lpszClassName = szApp;
	wcex.hIconSm = 0;
	RegisterClassExA(&wcex);

	DWORD dwStyle = WS_OVERLAPPEDWINDOW;
	int width = 1366;
	int height = 768;
	if (m_DaemonMap[id].is_fullscreen) {
		dwStyle = WS_POPUP;
		width = GetSystemMetrics(SM_CXSCREEN);
		height = GetSystemMetrics(SM_CYSCREEN);
	}
	HWND hwnd = CreateWindowA(szApp, nullptr, dwStyle, 0, 0, width, height, NULL, NULL, wcex.hInstance, 0);

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);
	ImmDisableIME(0);

	/*if (m_DaemonMap[id].is_fullscreen) {
		FloatBar* floatbar = new FloatBar(hwnd, this);
		floatbar->floatbar_window_create();
	}*/

	m_DaemonMap[id].hwnd = hwnd;
	daemon_show_stream(id, hwnd);

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
#else
	XInitThreads();
	gtk_init(NULL, NULL);
	GtkWidget* hwnd = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	GdkScreen* screen = gdk_screen_get_default();
	int width = gdk_screen_get_width(screen);
	int height = gdk_screen_get_height(screen);
	gtk_window_set_default_size(GTK_WINDOW(hwnd), width, height);
	gtk_window_fullscreen(GTK_WINDOW(hwnd));

	gtk_widget_set_events(hwnd, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_SCROLL_MASK |
		GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);
	g_signal_connect(G_OBJECT(hwnd), "destroy", G_CALLBACK(on_window_closed), NULL);
	g_signal_connect(G_OBJECT(hwnd), "button_press_event", G_CALLBACK(button_press_event), NULL);
	g_signal_connect(G_OBJECT(hwnd), "button_release_event", G_CALLBACK(button_release_event), NULL);
	g_signal_connect(G_OBJECT(hwnd), "motion_notify_event", G_CALLBACK(motion_notify_event), NULL);
	g_signal_connect(G_OBJECT(hwnd), "key_press_event", G_CALLBACK(key_press_event), NULL);
	g_signal_connect(G_OBJECT(hwnd), "key_release_event", G_CALLBACK(key_release_event), NULL);

	gtk_widget_show(hwnd);

	m_DaemonMap[id].hwnd = hwnd;
	daemon_show_stream(id, hwnd);

	gtk_main();
#endif
}
