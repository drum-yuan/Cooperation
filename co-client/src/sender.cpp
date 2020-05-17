
#include "sender.h"
#include "DaemonApi.h"
#include "restbed"
#include "cjson.h"

#ifdef WIN32
#include <tchar.h>
#include <Wtsapi32.h>
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

static INPUT _input;
static DWORD _dwButtonState = 0;
static DWORD _dwInputTime = 0;
static Sender* _instance = NULL;
Sender::Sender(const string& url)
{
	m_Url = url;
	m_HostName = "";
	m_DaemonId = -1;
	m_MonitorThread = NULL;
	m_DesktopEventThread = NULL;
	m_VappQuit = true;
	m_EventRunning = false;
	m_DesktopSwitch = false;
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
	m_VappQuit = true;
	m_EventRunning = false;
	if (m_MonitorThread && m_MonitorThread->joinable()) {
		m_MonitorThread->join();
		delete m_MonitorThread;
	}
	m_MonitorThread = NULL;
	if (m_DesktopEventThread && m_DesktopEventThread->joinable()) {
		m_DesktopEventThread->join();
		delete m_DesktopEventThread;
	}
	m_DesktopEventThread = NULL;
}

bool Sender::register_compute_node(const string& app_name, const RDSHInfo& rdsh_info, string& app_guid)
{
	if (m_DaemonId != -1) {
		LOG_ERROR("this compute node is already registered");
		return false;
	}
	m_AppName = app_name;
	m_RDSHInfo = rdsh_info;
	string strUrl = "http://" + m_Url + "/add-node";
	string strPost = string("{\"app_name\": \"") + app_name + string("\",\"host_name\": \"") + m_HostName + string("\"}");
	LOG_INFO("str url %s\npost %s", strUrl.c_str(), strPost.c_str());
	auto request = std::make_shared< Request >(Uri(strUrl));
	request->set_header("Content-Type", "application/json");
	request->set_method("POST");
	request->set_header("Host", m_ServerIP);
	request->set_header("Content-Length", to_string(strPost.length()));
	request->set_body(strPost);
	auto response = Http::sync(request);
	if (response->get_status_code() != OK) {
		LOG_ERROR("response status code %d", response->get_status_code());
		return false;
	}
	int body_len = stoi(response->get_header("Content-Length"));
	Http::fetch(body_len, response);
	std::string content((char*)response->get_body().data(), body_len);
	LOG_INFO("add-node request succeed %s", content.c_str());

	cJSON *root = cJSON_Parse(content.c_str());
	cJSON *item = cJSON_GetObjectItem(root, "app_guid");
	if (item != NULL) {
		app_guid = string(item->valuestring);
	}
	LOG_INFO("app_guid %s", app_guid.c_str());
	item = cJSON_GetObjectItem(root, "sirius_port");
	if (item != NULL) {
		m_SiriusUrl = m_ServerIP + ":" + to_string(item->valueint);
	}
	cJSON_Delete(root);

	int ins_id = daemon_create();
	if (ins_id < 0) {
		LOG_ERROR("daemon create fail");
		return false;
	}
	daemon_set_mouse_callback(ins_id, recv_mouse_event_callback);
	daemon_set_keyboard_callback(ins_id, recv_keyboard_event_callback);
	daemon_set_vapp_start_callback(ins_id, recv_vapp_start_callback);
	daemon_set_vapp_stop_callback(ins_id, recv_vapp_stop_callback);
	LOG_INFO("daemon start %s", m_SiriusUrl.c_str());
	while (!daemon_start(ins_id, m_SiriusUrl)) {
		util_sleep(1000);
	}
	daemon_start_publish(ins_id);
	m_DaemonId = ins_id;
	if (m_DesktopEventThread == NULL) {
		m_DesktopEventThread = new thread(&Sender::desktop_event_thread, _instance);
	}
	return true;
}

void Sender::unregister_compute_node(const string& app_guid)
{
	string strUrl = "http://" + m_Url + "/del-node";
	string strPost = string("{\"app_guid\": \"") + app_guid + string("\"}");
	LOG_INFO("str url %s\npost %s", strUrl.c_str(), strPost.c_str());
	auto request = std::make_shared< Request >(Uri(strUrl));
	request->set_header("Content-Type", "application/json");
	request->set_method("POST");
	request->set_header("Host", m_ServerIP);
	request->set_header("Content-Length", to_string(strPost.length()));
	request->set_body(strPost);
	auto response = std::make_shared< Response >();
	response = Http::sync(request);
	if (response->get_status_code() != OK) {
		LOG_ERROR("response status code %d", response->get_status_code());
		return;
	}

	m_VappQuit = true;
	daemon_stop(m_DaemonId);
	m_DaemonId = -1;
	m_EventRunning = false;
}

bool Sender::start_compute_node(const string& app_name, const RDSHInfo& rdsh_info)
{
	if (m_DaemonId >= 0) {
		daemon_set_vapp_start_callback(m_DaemonId, NULL);

		start_vapp();
		m_VappQuit = false;
		if (m_MonitorThread == NULL) {
			m_MonitorThread = new thread(&Sender::monitor_thread, _instance);
		}
		return true;
	}
	else {
		return false;
	}
}

void Sender::stop_compute_node()
{
#ifdef WIN32
	ShellExecute(NULL, "open", "taskkill", "/IM xtapp.exe /F", "", SW_HIDE);
#else

#endif
}

void Sender::monitor_thread()
{
#ifdef WIN32
	while (!m_VappQuit) {
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

#ifdef WIN32
void Sender::desktop_event_thread()
{
	m_EventRunning = true;
	HANDLE desktop_event = OpenEvent(SYNCHRONIZE, FALSE, "WinSta0_DesktopSwitch");
	if (!desktop_event) {
		LOG_ERROR("OpenEvent() failed: %lu", GetLastError());
		return;
	}
	while (m_EventRunning) {
		DWORD wait_ret = WaitForSingleObject(desktop_event, INFINITE);
		switch (wait_ret) {
		case WAIT_OBJECT_0:
		{
			m_DesktopSwitch = true;
		}
		break;
		case WAIT_TIMEOUT:
		default:
			LOG_WARN("WaitForSingleObject(): %lu", wait_ret);
}
	}
	CloseHandle(desktop_event);
}
#else
static void sigchld_handler(int sig)
{
    if (sig == SIGCHLD) {
        int status = 0;
        int pid = waitpid(-1, &status, WNOHANG);
		LOG_INFO("recv sigchld pid %d", pid);
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
	LOG_INFO("start vapp: %s", param);
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
		LOG_INFO("pid=%d, getpid=%d", pid, getpid());
        signal(SIGCHLD, sigchld_handler);
    }
#endif
}

void Sender::switch_input_desktop()
{
#ifdef WIN32
	TCHAR desktop_name[MAX_PATH];
	HDESK hdesk;

	hdesk = OpenInputDesktop(0, FALSE, GENERIC_ALL);
	if (!hdesk) {
		LOG_ERROR("OpenInputDesktop() failed: %lu", GetLastError());
		return;
	}
	if (!SetThreadDesktop(hdesk)) {
		LOG_ERROR("SetThreadDesktop failed %lu", GetLastError());
		CloseDesktop(hdesk);
		return;
	}
	if (GetUserObjectInformation(hdesk, UOI_NAME, desktop_name, sizeof(desktop_name), NULL)) {
		LOG_INFO("Desktop: %s", desktop_name);
	}
	else {
		LOG_ERROR("GetUserObjectInformation failed %lu", GetLastError());
	}
	CloseDesktop(hdesk);
	m_DesktopSwitch = false;
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
	if (_instance->m_DesktopSwitch) {
		_instance->switch_input_desktop();
	}

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

	if ((mouse_move && GetTickCount() - _dwInputTime > 20) || buttons_change || mouse_wheel) {
		UINT hr = SendInput(1, &_input, sizeof(INPUT));
		if (!hr) {
			LOG_INFO("send input mouse fail, %u", GetLastError());

		}
		_dwInputTime = GetTickCount();
	}
#endif
}

void Sender::recv_keyboard_event_callback(unsigned int key_val, bool is_pressed)
{
#ifdef WIN32
	/*if (is_pressed) {
		keybd_event(key_val, 0, 0, 0);
	}
	else {
		keybd_event(key_val, 0, KEYEVENTF_KEYUP, 0);
	}*/
	ZeroMemory(&_input, sizeof(INPUT));

	_input.type = INPUT_KEYBOARD;
	_input.ki.wVk = key_val;
	_input.ki.wScan = MapVirtualKey(key_val, 0);

	if (!is_pressed)
	{
		_input.ki.dwFlags |= KEYEVENTF_KEYUP;
	}

	switch (key_val)
	{
	case VK_UP:
	case VK_DOWN:
	case VK_LEFT:
	case VK_RIGHT:
		_input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
		break;
	}

	UINT hr = SendInput(1, &_input, sizeof(INPUT));
	if (!hr) {
		LOG_INFO("send input key %u-%d fail, %u", key_val, is_pressed, GetLastError());
	}
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
	LOG_INFO("daemon start stream %d", _instance->m_DaemonId);
	_instance->m_VappQuit = false;
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
	LOG_INFO("daemon stop stream %d", _instance->m_DaemonId);
	_instance->m_VappQuit = true;
	if (_instance->m_MonitorThread && _instance->m_MonitorThread->joinable()) {
		_instance->m_MonitorThread->join();
		delete _instance->m_MonitorThread;
	}
	_instance->m_MonitorThread = NULL;
#endif
}

