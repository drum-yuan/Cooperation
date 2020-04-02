
#include "sender.h"
#include "DaemonApi.h"
#include "restbed"
#include "cjson.h"

#ifdef WIN32
#define AGENT_LBUTTON_MASK (1 << 1)
#define AGENT_MBUTTON_MASK (1 << 2)
#define AGENT_RBUTTON_MASK (1 << 3)
#define AGENT_UBUTTON_MASK (1 << 4)
#define AGENT_DBUTTON_MASK (1 << 5)
#else
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

typedef unsigned long  DWORD;
#define sprintf_s snprintf
#endif

using namespace restbed;

static DWORD _dwButtonState = 0;
static DWORD _dwInputTime = 0;
static Sender* _instance = NULL;
Sender::Sender(const string& url)
{
	m_Url = url;
	m_HostName = "";
	m_DaemonId = -1;
	m_MonitorThread = NULL;
	m_Quit = true;
	m_hCursor = NULL;
#ifdef WIN32
	DWORD name_len = 0;
	GetComputerName(NULL, &name_len);
	char* name = new char[name_len];
	GetComputerName(name, &name_len);
	m_HostName += name;
	m_HostName += "|";
	delete[] name;
	name_len = 0;
	GetUserName(NULL, &name_len);
	name = new char[name_len];
	GetUserName(name, &name_len);
	m_HostName += name;
	delete[] name;
#endif
	m_ServerIP = m_Url;
	string::size_type pos = m_Url.find(':');
	if (pos != string::npos) {
		m_ServerIP = m_Url.substr(0, pos);
	}
	_instance = this;
}

Sender::~Sender()
{
	m_Quit = true;
	if (m_MonitorThread && m_MonitorThread->joinable()) {
		m_MonitorThread->join();
		delete m_MonitorThread;
	}
	m_MonitorThread = NULL;
}

bool Sender::register_compute_node(const string& app_name, const RDSHInfo& rdsh_info, string& app_guid)
{
	if (m_DaemonId != -1) {
		printf("this compute node is already registered\n");
		return false;
	}
	m_AppName = app_name;
	m_RDSHInfo = rdsh_info;
	string strUrl = "http://" + m_Url + "/add-node";
	string strPost = string("{\"app_name\": \"") + app_name + string("\",\"host_name\": \"") + m_HostName + string("\"}");
	printf("str url %s\npost %s\n", strUrl.c_str(), strPost.c_str());
	auto request = std::make_shared< Request >(Uri(strUrl));
	request->set_header("Content-Type", "application/json");
	request->set_method("POST");
	request->set_header("Host", m_ServerIP);
	request->set_header("Content-Length", to_string(strPost.length()));
	request->set_body(strPost);
	auto response = Http::sync(request);
	if (response->get_status_code() != OK) {
		printf("response status code %d\n", response->get_status_code());
		return false;
	}
	int body_len = stoi(response->get_header("Content-Length"));
	Http::fetch(body_len, response);
	std::string content((char*)response->get_body().data(), body_len);
	printf("add-node request succeed %s\n", content.c_str());

	cJSON *root = cJSON_Parse(content.c_str());
	cJSON *item = cJSON_GetObjectItem(root, "app_guid");
	if (item != NULL) {
		app_guid = string(item->valuestring);
	}
	printf("app_guid %s\n", app_guid.c_str());
	item = cJSON_GetObjectItem(root, "sirius_port");
	if (item != NULL) {
		m_SiriusUrl = m_ServerIP + ":" + to_string(item->valueint);
	}
	cJSON_Delete(root);

	int ins_id = daemon_create();
	if (ins_id < 0) {
		printf("daemon create fail\n");
		return false;
	}
	daemon_set_mouse_callback(ins_id, recv_mouse_event_callback);
	daemon_set_keyboard_callback(ins_id, recv_keyboard_event_callback);
	daemon_set_vapp_start_callback(ins_id, recv_vapp_start_callback);
	daemon_set_vapp_stop_callback(ins_id, recv_vapp_stop_callback);
	printf("daemon start %s\n", m_SiriusUrl.c_str());
	daemon_start(ins_id, m_SiriusUrl);
	daemon_start_publish(ins_id);
	m_DaemonId = ins_id;
	return true;
}

void Sender::unregister_compute_node(const string& app_guid)
{
	string strUrl = "http://" + m_Url + "/del-node";
	string strPost = string("{\"app_guid\": \"") + app_guid + string("\"}");
	printf("str url %s\npost %s\n", strUrl.c_str(), strPost.c_str());
	auto request = std::make_shared< Request >(Uri(strUrl));
	request->set_header("Content-Type", "application/json");
	request->set_method("POST");
	request->set_header("Host", m_ServerIP);
	request->set_header("Content-Length", to_string(strPost.length()));
	request->set_body(strPost);
	auto response = std::make_shared< Response >();
	response = Http::sync(request);
	if (response->get_status_code() != OK) {
		printf("response status code %d\n", response->get_status_code());
		return;
	}

	m_Quit = true;
	daemon_stop(m_DaemonId);
	m_DaemonId = -1;
}

bool Sender::start_compute_node(const string& app_name, const RDSHInfo& rdsh_info)
{
	string app_guid;
	bool ret = register_compute_node(app_name, rdsh_info, m_AppGuid);
	if (ret) {
		daemon_set_vapp_start_callback(m_DaemonId, NULL);

		start_vapp();
		m_Quit = false;
		if (m_MonitorThread == NULL) {
			m_MonitorThread = new thread(&Sender::monitor_thread, _instance);
		}
	}
	return ret;
}

void Sender::stop_compute_node()
{
	system("taskkill /IM xtapp.exe /F");
	unregister_compute_node(m_AppGuid);
}

void Sender::monitor_thread()
{
#ifdef WIN32
	while (!m_Quit) {
		Sleep(50);
		CURSORINFO cursor_info;
		cursor_info.cbSize = sizeof(CURSORINFO);
		BOOL ret = GetCursorInfo(&cursor_info);
		if (cursor_info.hCursor != m_hCursor) {
			ICONINFO icon_info;
			BITMAP bmMask;
			BITMAP bmColor;
			string mask_bytes;
			string color_bytes;

			ret = GetIconInfo(cursor_info.hCursor, &icon_info);
			if (icon_info.hbmMask == NULL) {
				continue;
			}
			int x = icon_info.xHotspot;
			int y = icon_info.yHotspot;

			GetObject(icon_info.hbmMask, sizeof(bmMask), &bmMask);
			if (bmMask.bmPlanes != 1 || bmMask.bmBitsPixel != 1) {
				continue;
			}
			char* mask_buffer = new char[bmMask.bmWidthBytes * bmMask.bmHeight];
			GetBitmapBits(icon_info.hbmMask, bmMask.bmWidthBytes * bmMask.bmHeight, mask_buffer);
			DeleteObject(icon_info.hbmMask);
			mask_bytes = string(mask_buffer, bmMask.bmWidthBytes * bmMask.bmHeight);
			delete[]mask_buffer;

			if (icon_info.hbmColor != NULL) {
				GetObject(icon_info.hbmColor, sizeof(bmColor), &bmColor);
				char* color_buffer = new char[bmColor.bmWidthBytes * bmColor.bmHeight];
				GetBitmapBits(icon_info.hbmColor, bmColor.bmWidthBytes * bmColor.bmHeight, color_buffer);
				DeleteObject(icon_info.hbmColor);
				color_bytes = string(color_buffer, bmColor.bmWidthBytes * bmColor.bmHeight);
				delete[]color_buffer;
			}
			m_hCursor = cursor_info.hCursor;
			daemon_send_cursor_shape(m_DaemonId, x, y, bmMask.bmWidth, bmMask.bmHeight, color_bytes, mask_bytes);
		}
	}
#endif
}

#ifndef WIN32
static void sigchld_handler(int sig)
{
    if (sig == SIGCHLD) {
        int status = 0;
        int pid = waitpid(-1, &status, WNOHANG);
        printf("recv sigchld pid %d\n", pid);
    }
}
#endif

void Sender::start_vapp()
{
	char param[4096];
	if (m_AppName == "desktop") {
		sprintf_s(param, sizeof(param), "/v:%s /d:%s /u:%s /p:%s /cert-ignore /gfx:avc444 /drive:LOCAL,C:\\ /drive:hotplug,DynamicDrives /f /sirius:\"%s\"",
			m_RDSHInfo.rdsh_ip.c_str(), m_RDSHInfo.domain.c_str(), m_RDSHInfo.user.c_str(), m_RDSHInfo.password.c_str(), m_SiriusUrl.c_str());
	}
	else {
		sprintf_s(param, sizeof(param), "/v:%s /d:%s /u:%s /p:%s /cert-ignore /gfx:avc444 /drive:LOCAL,C:\\ /drive:hotplug,DynamicDrives /f /app:\"||%s\" /sirius:\"%s\"",
			m_RDSHInfo.rdsh_ip.c_str(), m_RDSHInfo.domain.c_str(), m_RDSHInfo.user.c_str(), m_RDSHInfo.password.c_str(), m_AppName.c_str(), m_SiriusUrl.c_str());
	}
	printf("start vapp: %s\n", param);
#ifdef WIN32
	SHELLEXECUTEINFO  ShExecInfo = { 0 };
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = "open";
	ShExecInfo.lpFile = "xtapp";
	ShExecInfo.lpParameters = param;
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_SHOW;
	ShExecInfo.hInstApp = NULL;
	ShellExecuteEx(&ShExecInfo);
#else
    pid_t pid = vfork();
    if (pid == 0) {
        execl("./xtapp", "xtapp", param, NULL);
        exit(0);
    } else {
        printf("pid=%d, getpid=%d\n", pid, getpid());
        signal(SIGCHLD, sigchld_handler);
    }
#endif
}


static int get_buttons_change(unsigned int last_buttons_state, unsigned int new_buttons_state,
								unsigned int mask, unsigned int down_flag, unsigned int up_flag)
{
	int ret = 0;
	if (!(last_buttons_state & mask) && (new_buttons_state & mask)) {
		ret = down_flag;
	}
	else if ((last_buttons_state & mask) && !(new_buttons_state & mask)) {
		ret = up_flag;
	}
	return ret;
}

void Sender::recv_mouse_event_callback(unsigned int x, unsigned int y, unsigned int button_mask)
{
#ifdef WIN32
	INPUT _input;
	DWORD mouse_move = 0;
	DWORD buttons_change = 0;
	DWORD mouse_wheel = 0;

	ZeroMemory(&_input, sizeof(INPUT));
	_input.type = INPUT_MOUSE;

	DWORD w = ::GetSystemMetrics(SM_CXSCREEN);
	DWORD h = ::GetSystemMetrics(SM_CYSCREEN);
	w = (w > 1) ? w - 1 : 1; /* coordinates are 0..w-1, protect w==0 */
	h = (h > 1) ? h - 1 : 1; /* coordinates are 0..h-1, protect h==0 */
	mouse_move = MOUSEEVENTF_MOVE;
	_input.mi.dx = x * 0xffff / w;
	_input.mi.dy = y * 0xffff / h;

	if (button_mask != _dwButtonState) {
		buttons_change = get_buttons_change(_dwButtonState, button_mask, AGENT_LBUTTON_MASK,
			MOUSEEVENTF_LEFTDOWN, MOUSEEVENTF_LEFTUP) |
			get_buttons_change(_dwButtonState, button_mask, AGENT_MBUTTON_MASK,
				MOUSEEVENTF_MIDDLEDOWN, MOUSEEVENTF_MIDDLEUP) |
			get_buttons_change(_dwButtonState, button_mask, AGENT_RBUTTON_MASK,
				MOUSEEVENTF_RIGHTDOWN, MOUSEEVENTF_RIGHTUP);
		_dwButtonState = button_mask;
	}
	if (button_mask & (AGENT_UBUTTON_MASK | AGENT_DBUTTON_MASK)) {
		mouse_wheel = MOUSEEVENTF_WHEEL;
		if (button_mask & AGENT_UBUTTON_MASK) {
			_input.mi.mouseData = WHEEL_DELTA;
		}
		else if (button_mask & AGENT_DBUTTON_MASK) {
			_input.mi.mouseData = (DWORD)(-WHEEL_DELTA);
		}
	}
	else {
		mouse_wheel = 0;
		_input.mi.mouseData = 0;
	}

	_input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | mouse_move |
		mouse_wheel | buttons_change;

	if ((mouse_move && GetTickCount() - _dwInputTime > 10) || buttons_change || mouse_wheel) {
		SendInput(1, &_input, sizeof(INPUT));
		_dwInputTime = GetTickCount();
	}
#endif
}

void Sender::recv_keyboard_event_callback(unsigned int key_val, bool is_pressed)
{
#ifdef WIN32
	/*printf("recv key event %u %d\n", key_val, is_pressed);
	if (is_pressed) {
		keybd_event(key_val, 0, 0, 0);
	}
	else {
		keybd_event(key_val, 0, KEYEVENTF_KEYUP, 0);
	}*/
	INPUT inp[1];
	ZeroMemory(inp, sizeof(inp));

	inp[0].type = INPUT_KEYBOARD;
	inp[0].ki.wVk = key_val;
	inp[0].ki.wScan = MapVirtualKey(key_val, 0);

	if (!is_pressed)
	{
		inp[0].ki.dwFlags |= KEYEVENTF_KEYUP;
	}

	switch (key_val)
	{
	case VK_UP:
	case VK_DOWN:
	case VK_LEFT:
	case VK_RIGHT:
		inp[0].ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
		break;
	}

	UINT hr = SendInput(1, inp, sizeof(INPUT));
#endif
}

void Sender::recv_vapp_start_callback()
{
#ifdef WIN32
	if (_instance->m_AppName == "") {
		daemon_start_stream(_instance->m_DaemonId);
	}
	else {
		system("taskkill /IM xtapp.exe /F");

		_instance->start_vapp();
	}
	printf("daemon start stream %d\n", _instance->m_DaemonId);
	_instance->m_Quit = false;
	if (_instance->m_MonitorThread == NULL) {
		_instance->m_MonitorThread = new thread(&Sender::monitor_thread, _instance);
	}
#endif
}

void Sender::recv_vapp_stop_callback()
{
#ifdef WIN32
	if (_instance->m_AppName != "") {
		system("taskkill /IM xtapp.exe /F");
	}
	printf("daemon stop stream %d\n", _instance->m_DaemonId);
	_instance->m_Quit = true;
	if (_instance->m_MonitorThread && _instance->m_MonitorThread->joinable()) {
		_instance->m_MonitorThread->join();
		delete _instance->m_MonitorThread;
	}
	_instance->m_MonitorThread = NULL;
#endif
}
