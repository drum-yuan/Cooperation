#include "CaptureWin.h"

static LPSTR dispCode[7] = {
	"Change Successful",
	"Must Restart",
	"Bad Flags",
	"Bad Parameters",
	"Failed",
	"Bad Mode",
	"Not Updated" };

CCapture::CCapture(int id, FrameCallback on_frame, void* param)
{
	m_GrabType = 0;
	m_MonitorID = id;
	onFrame = on_frame;
	onframe_param = param;

	m_Quit = FALSE;
	m_Pause = FALSE;

	memset(&m_Mirror, 0, sizeof(m_Mirror));
	memset(&m_Directx, 0, sizeof(m_Directx));
	memset(&m_GDI, 0, sizeof(m_GDI));
	
	m_SleepMsec = 25;
	m_hMessageWnd = NULL;
	m_hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hThread = NULL;

	m_IntervalCnt = 5;
	m_AckSeq = 0;
	m_CaptureSeq = 0;
}

CCapture::~CCapture()
{
}

void CCapture::start()
{
	DWORD tid;
	m_hThread = CreateThread(NULL, 10 * 1024 * 1024, __loop_msg, this, 0, &tid);

	if (!m_hThread) {
		CloseHandle(m_hEvent);
	}
	::SetThreadPriority(m_hThread, THREAD_PRIORITY_HIGHEST); //�������ȼ�
	::WaitForSingleObject(m_hEvent, INFINITE);
	::ResetEvent(m_hEvent); 
	if (!m_hMessageWnd) {
		CloseHandle(m_hEvent); 
		CloseHandle(m_hThread);
	}
}

void CCapture::stop()
{
	m_hMessageWnd = NULL;
	m_Quit = TRUE;
}

void CCapture::pause()
{
	m_Pause = TRUE;
}

void CCapture::resume()
{
	m_Pause = FALSE;
}

DWORD CALLBACK CCapture::__loop_msg(void* _p)
{
	CCapture* dp = (CCapture*)_p;
	::CoInitialize(0);

	int m_GrabType = dp->m_GrabType;
	BOOL is_ok = FALSE;
	if (m_GrabType == GRAB_TYPE_AUTO) {
		is_ok = dp->__init_mirror(TRUE);
		if (is_ok) m_GrabType = GRAB_TYPE_MIRROR;
		else {
			is_ok = dp->__init_gdi(TRUE);
			if (is_ok) m_GrabType = GRAB_TYPE_GDI;
		}
		dp->m_GrabType = m_GrabType;
	}
	else if (m_GrabType == GRAB_TYPE_MIRROR) {
		is_ok = dp->__init_mirror(TRUE);
	}
	else if (m_GrabType == GRAB_TYPE_DIRECTX) {
		is_ok = dp->__init_directx(TRUE);
	}
	else if (m_GrabType == GRAB_TYPE_GDI) {
		is_ok = dp->__init_gdi(TRUE);
	}

	if (!is_ok) {
		printf("Can not init capture screen type=%d\n", m_GrabType);
		dp->m_hMessageWnd = NULL;
		SetEvent(dp->m_hEvent);
		CoUninitialize();
		return 0;
	}

	HWND hwnd = dp->CreateMsgWnd();
	HANDLE hEvt = dp->m_hEvent;

	if (!hwnd) {
		switch (m_GrabType) {
		case GRAB_TYPE_MIRROR:  dp->__init_mirror(FALSE);  break;
		case GRAB_TYPE_DIRECTX: dp->__init_directx(FALSE); break;
		case GRAB_TYPE_GDI:     dp->__init_gdi(FALSE);     break;
		}
		dp->m_hMessageWnd = NULL;
		SetEvent(hEvt);
		CoUninitialize();
		return 0;
	}
	SetEvent(hEvt);

	long sleep_msec = dp->m_SleepMsec;

	MSG msg;
	BOOL bQuit = FALSE;
	QueryPerformanceFrequency(&dp->counter);
	while (!dp->m_Quit) {	
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				bQuit = TRUE;
				break;
			}
			else {
				DispatchMessage(&msg);
			}
		}
		if (bQuit)break;

		sleep_msec = dp->m_SleepMsec;
		if (!dp->m_Pause) {
			while ((!dp->m_Quit) && (dp->m_CaptureSeq - dp->m_AckSeq > dp->m_IntervalCnt)) {
				//printf("Sequence D-value %d\n", dp->m_CaptureSeq - dp->m_AckSeq);
				SleepEx(10, TRUE);
			}

			if (dp->m_Quit) {
				goto End;
			}
			QueryPerformanceCounter(&dp->frame_begin);
			if (m_GrabType == GRAB_TYPE_MIRROR) {
				dp->capture_mirror();
			}
			else if (m_GrabType == GRAB_TYPE_DIRECTX) {
				dp->capture_dxgi();
			}
			else if (m_GrabType == GRAB_TYPE_GDI) {
				dp->capture_gdi();
			}
			dp->m_CaptureSeq++;
			QueryPerformanceCounter(&dp->frame_end);

			LONGLONG dt = (dp->frame_end.QuadPart - dp->frame_begin.QuadPart) * 1000 / dp->counter.QuadPart;
			if (dt >= dp->m_SleepMsec) {
				sleep_msec = 0;
			}
			else {
				sleep_msec = dp->m_SleepMsec - (long)dt;
			}
		}
		SleepEx(sleep_msec, TRUE);
	}

End:
	CloseHandle(hEvt);
	switch (dp->m_GrabType) {
	case GRAB_TYPE_MIRROR:  dp->__init_mirror(FALSE);  break;
	case GRAB_TYPE_DIRECTX: dp->__init_directx(FALSE); break;
	case GRAB_TYPE_GDI:     dp->__init_gdi(FALSE);     break;
	}

	CoUninitialize();
	return 0;
}

LRESULT CALLBACK CCapture::xDispWindowProc(HWND hwnd, UINT uMsg, WPARAM  wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DISPLAYCHANGE:
	{
		printf("WM_DISPLAYCHANGE\n");
		CCapture* dp = (CCapture*)GetProp(hwnd, "xDispData");
		if (dp) {
			if (dp->m_GrabType == GRAB_TYPE_MIRROR) {
				dp->change_display(LOWORD(lParam), HIWORD(lParam), wParam);
			}
			else if (dp->m_GrabType == GRAB_TYPE_DIRECTX) {
				dp->__init_directx(FALSE);
				dp->__init_directx(TRUE);
				printf("directx restart\n");
			}
			else if (dp->m_GrabType == GRAB_TYPE_GDI) {
				dp->__init_gdi(FALSE);
				dp->__init_gdi(TRUE);
			}
		}
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

HWND CCapture::CreateMsgWnd()
{
	HMODULE hmod = GetModuleHandle(NULL);

	const char* cls_name = "xdisp_window_class";
	WNDCLASS wndClass = { 0,
		xDispWindowProc,
		0, 0,
		hmod,
		NULL,NULL,NULL,NULL,
		cls_name
	};
	RegisterClass(&wndClass);
	HWND hwnd = CreateWindow(cls_name, "", WS_OVERLAPPEDWINDOW, 0, 0, 1, 1, NULL, NULL, hmod, NULL);
	m_hMessageWnd = hwnd;
	if (!hwnd) {
		printf("NULL Message Window.\n");
	}
	SetProp(hwnd, "xDispData", this);

	return hwnd;
}

void CCapture::change_display(int w, int h, int bits)
{
	printf("change new width=%d, heigt=%d, bitcount=%d\n", w, h, bits);
#ifdef DMF_MIRROR
	if (m_Mirror.buffer.Userbuffer) {
#else
	if (m_Mirror.buffer) {
#endif
		map_and_unmap_buffer(m_Mirror.disp.DeviceName, FALSE, &m_Mirror.buffer);
		m_Mirror.last_check_drawbuf_time = 0;
#ifdef DMF_MIRROR
		m_Mirror.buffer.Userbuffer = NULL;
#else
		m_Mirror.buffer = NULL;
#endif
	}

	DEVMODE devmode;
	memset(&devmode, 0, sizeof(DEVMODE));
	devmode.dmSize = sizeof(DEVMODE);
	devmode.dmDriverExtra = 0;
	BOOL bRet = EnumDisplaySettings(m_DispName, ENUM_CURRENT_SETTINGS, &devmode);
	devmode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	strcpy((char*)devmode.dmDeviceName, "xietong_mirror");
	devmode.dmPelsWidth = w;
	devmode.dmPelsHeight = h;
	devmode.dmBitsPerPel = bits;

	INT code = ChangeDisplaySettingsEx(m_Mirror.disp.DeviceName, &devmode, NULL, CDS_UPDATEREGISTRY, NULL);
	printf("Change_display Update Registry on device mode: %s\n", GetDispCode(code));
}

void CCapture::set_drop_interval(unsigned int count)
{
	m_IntervalCnt = count;
}

void CCapture::set_ack_sequence(unsigned int seq)
{
	if (seq > m_AckSeq) {
		m_AckSeq = seq;
	}
}

unsigned int CCapture::get_capture_sequence()
{
	return m_CaptureSeq;
}

void CCapture::reset_sequence()
{
	m_AckSeq = m_CaptureSeq;
}

void CCapture::set_frame_rate(unsigned int rate)
{
	if (rate > 100) {
		return;
	}
	m_SleepMsec = 1000 / rate;
}

#ifdef DMF_MIRROR
void CCapture::map_and_unmap_buffer(const char* dev_name, BOOL is_map, GETCHANGESBUF* p_buf)
{
	printf("buffer map %d\n", is_map);
	DWORD code = dmf_esc_usm_pipe_unmap;
	if (is_map) {
		code = dmf_esc_usm_pipe_map;
	}
#else
void CCapture::map_and_unmap_buffer(const char* dev_name, BOOL is_map, draw_buffer_t** p_buf)
{
	DWORD code = ESCAPE_CODE_UNMAP_USERBUFFER;
	if (is_map) {
		code = ESCAPE_CODE_MAP_USERBUFFER;
	}
#endif

	HDC hdc = CreateDC(NULL, dev_name, NULL, NULL);
	if (!hdc) {
		printf("CreateDC err=%d\n", GetLastError());
		return;
	}

#ifdef DMF_MIRROR
	int ret;
	if (is_map) {
		ret = ExtEscape(hdc, code, 0, NULL, sizeof(GETCHANGESBUF), (LPSTR)p_buf);
		printf("Map ExtEscape ret=%d, err=%d\n", ret, GetLastError());
		if (ret <= 0) {
			
			DeleteDC(hdc);
			return;
		}
	}
	else {
		ret = ExtEscape(hdc, code, sizeof(GETCHANGESBUF), (LPCSTR)p_buf, 0, NULL);
		printf("UnMap ExtEscape ret=%d, err=%d\n", ret, GetLastError());
		if (ret <= 0) {
			
			DeleteDC(hdc);
			return;
		}
	}
#else
	unsigned __int64  u_addr = 0;
	int ret;
	if (is_map) {
		ret = ExtEscape(hdc, code, 0, NULL, sizeof(u_addr), (LPSTR)&u_addr);
		if (ret <= 0) {
			printf("Map ExtEscape ret=%d, err=%d\n", ret, GetLastError());
			DeleteDC(hdc);
			return;
		}
		*p_buf = (draw_buffer_t*)(ULONG_PTR)u_addr;
		printf("-- map user=%p\n", *p_buf);
	}
	else {
		u_addr = (ULONG_PTR)*p_buf;
		ret = ExtEscape(hdc, code, sizeof(u_addr), (LPCSTR)&u_addr, 0, NULL);
		if (ret <= 0) {
			printf("UnMap ExtEscape err=%d\n", GetLastError());
			DeleteDC(hdc);
			return;
		}
	}
#endif
	DeleteDC(hdc);
}

BOOL CCapture::find_display_device(PDISPLAY_DEVICE disp, PDEVMODE mode, BOOL is_primary, int id)
{
	DISPLAY_DEVICE dispDevice;
	memset(&dispDevice, 0, sizeof(DISPLAY_DEVICE));
	dispDevice.cb = sizeof(DISPLAY_DEVICE);
	int num = 0; 
	BOOL find = FALSE;

	if (id >= 0) {
		if (EnumDisplayDevices(NULL, id, &dispDevice, NULL)) {
			find = TRUE;
		}
	}
	else {
		while (EnumDisplayDevices(NULL, num, &dispDevice, NULL)) {
			/*printf("devName=[%s], desc=[%s], id=[%s], key[%s],flags=0x%X\n",
				dispDevice.DeviceName, dispDevice.DeviceString, dispDevice.DeviceID, dispDevice.DeviceKey, dispDevice.StateFlags);*/
			if (is_primary) {
				if (dispDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) {//����ʾ��
					find = TRUE;
					break;
				}
			}
			else {
				const char* desc = MIRROR_DRIVER;
#if (WINVER < 0x0601)
				desc = VIRTUAL_DRIVER;
#endif
				if (_stricmp(dispDevice.DeviceString, desc) == 0) {
					find = TRUE;
					break;
				}
			}
			++num;
		}
	}
	if (!find) {
		return FALSE;
	}
	BOOL bRet = FALSE;
	if (id >= 0) {
		strcpy_s(m_DispName, dispDevice.DeviceName);
		bRet = EnumDisplaySettings(m_DispName, ENUM_CURRENT_SETTINGS, mode);
	}
	else {
		bRet = EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, mode);
	}
	if (!bRet) {
		printf("not get current display information\n");
		return FALSE;
	}

	*disp = dispDevice;
	printf("Found devName=[%s], desc=[%s], id=[%s], key[%s], flags=0x%X\n", 
		dispDevice.DeviceName, dispDevice.DeviceString, dispDevice.DeviceID, dispDevice.DeviceKey, dispDevice.StateFlags);
	return TRUE;
}

//�������ȡ����������
BOOL CCapture::active_mirror_driver(BOOL is_active, PDISPLAY_DEVICE dp)
{
	DISPLAY_DEVICE dispDevice;
	PDISPLAY_DEVICE disp;
	DEVMODE devmode;

	memset(&devmode, 0, sizeof(DEVMODE));
	devmode.dmSize = sizeof(DEVMODE);
	devmode.dmDriverExtra = 0;
	if (is_active || !dp) {
		BOOL b = find_display_device(&dispDevice, &devmode, FALSE, -1);
		if (!b) {
			printf("not found Mirror Driver maybe not install.\n");
			return FALSE;
		}
		disp = &dispDevice;
	}
	else {
		disp = dp;
	}

	DWORD attach = 0;
	if (is_active) attach = 1;
	HKEY hkey;
	if (RegOpenKey(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\xdisp_mirror\\Device0", &hkey) == ERROR_SUCCESS) {
		RegSetValueEx(hkey, "Attach.ToDesktop", 0, REG_DWORD, (BYTE*)&attach, sizeof(DWORD));
		RegCloseKey(hkey);
	}
	char keystr[1024];
	char* sep = strstr(disp->DeviceKey, "\\{");
	if (!sep) {
		printf("invalid EnumDisplaySettings param.\n");
		return FALSE;
	}

	sprintf(keystr, "SYSTEM\\CurrentControlSet\\Hardware Profiles\\Current\\System\\CurrentControlSet\\Control\\VIDEO%s", sep);
	if (RegOpenKey(HKEY_LOCAL_MACHINE, keystr, &hkey) == ERROR_SUCCESS) {
		RegSetValueEx(hkey, "Attach.ToDesktop", 0, REG_DWORD, (BYTE*)&attach, sizeof(DWORD));
		RegCloseKey(hkey);
	}

	if (!is_active) {
		EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode);
		devmode.dmPelsWidth = 0;
		devmode.dmPelsHeight = 0;
	}
	else {
		strcpy((char*)devmode.dmDeviceName, "xietong_mirror");
		devmode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	}

	const char* DeviceName = disp->DeviceName;
	INT code = ChangeDisplaySettingsEx(DeviceName, &devmode, NULL, CDS_UPDATEREGISTRY, NULL);
	printf("Active_mirror Update Registry on device mode: %s\n", GetDispCode(code));

	if (dp && disp != dp) {
		*dp = *disp;
	}
	return TRUE;
}

LPSTR CCapture::GetDispCode(INT code)
{
	switch (code) {
	case DISP_CHANGE_SUCCESSFUL: return dispCode[0];
	case DISP_CHANGE_RESTART: return dispCode[1];
	case DISP_CHANGE_BADFLAGS: return dispCode[2];
	case DISP_CHANGE_BADPARAM: return dispCode[3];
	case DISP_CHANGE_FAILED: return dispCode[4];
	case DISP_CHANGE_BADMODE: return dispCode[5];
	case DISP_CHANGE_NOTUPDATED: return dispCode[6];
	default:
		static char tmp[MAX_PATH];
		sprintf(&tmp[0], "Unknown code: %08x\n", code);
		return (LPTSTR)&tmp[0];
	}
}

void CCapture::capture_mirror()
{
#ifdef DMF_MIRROR
	if (!m_Mirror.buffer.Userbuffer && abs(time(0) - m_Mirror.last_check_drawbuf_time) > 5) {
		m_Mirror.last_check_drawbuf_time = time(0);
		map_and_unmap_buffer(m_Mirror.disp.DeviceName, TRUE, &m_Mirror.buffer);
	}
	if (!m_Mirror.buffer.Userbuffer) {
		return;
	}

	CallbackFrameInfo frm;

	frm.width = GetSystemMetrics(SM_CXSCREEN);
	frm.height = GetSystemMetrics(SM_CYSCREEN);
	frm.line_bytes = frm.width * 4;
	frm.line_stride = frm.width * 4;
	frm.bitcount = 32;
	frm.buffer = (char*)m_Mirror.buffer.Userbuffer;
	frm.length = frm.width * frm.height * 4;
#else
	if (!m_Mirror.buffer && abs(time(0) - m_Mirror.last_check_drawbuf_time) > 5) {
		m_Mirror.last_check_drawbuf_time = time(0);
		map_and_unmap_buffer(m_Mirror.disp.DeviceName, TRUE, &m_Mirror.buffer);
	}
	if (!m_Mirror.buffer) {
		return;
	}

	CallbackFrameInfo frm;

	frm.width = m_Mirror.buffer->cx;
	frm.height = m_Mirror.buffer->cy;
	frm.line_bytes = m_Mirror.buffer->line_bytes;
	frm.line_stride = m_Mirror.buffer->line_stride;
	frm.bitcount = m_Mirror.buffer->bitcount;
	frm.buffer = (char*)m_Mirror.buffer->data_buffer;
	frm.length = m_Mirror.buffer->data_length;
#endif
	onFrame(&frm, onframe_param);
}

void CCapture::capture_dxgi()
{
	if (!m_Directx.dxgi_dup) {
		return;
	}

	IDXGIResource* DesktopResource = 0;
	DXGI_OUTDUPL_FRAME_INFO FrameInfo;
	HRESULT hr;
	ID3D11Texture2D* image2d = NULL;

	if (m_Directx.is_acquire_frame) {
		m_Directx.dxgi_dup->ReleaseFrame();
		m_Directx.is_acquire_frame = 0;
	}
	hr = m_Directx.dxgi_dup->AcquireNextFrame(0, &FrameInfo, &DesktopResource);
	if (FAILED(hr)) {   
		if (hr == _HRESULT_TYPEDEF_(0x887A0026L) || hr == _HRESULT_TYPEDEF_(0x887A0001L)) {
			__init_directx(FALSE);
			__init_directx(TRUE);
			return;
		}
		if (!m_Directx.buffer)
			return;
		//printf("acquire frame fail 0x%X\n", hr);
		goto L;
	}
	m_Directx.is_acquire_frame = 1;

	SAFE_RELEASE(image2d);
	hr = DesktopResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&image2d);
	SAFE_RELEASE(DesktopResource);
	if (FAILED(hr)) {
		return;
	}

	DXGI_MAPPED_RECT mappedRect;
	if (!m_Directx.dxgi_text2d) {
		D3D11_TEXTURE2D_DESC frameDescriptor;
		image2d->GetDesc(&frameDescriptor);
		frameDescriptor.Usage = D3D11_USAGE_STAGING;
		frameDescriptor.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		frameDescriptor.BindFlags = 0;
		frameDescriptor.MiscFlags = 0;
		frameDescriptor.MipLevels = 1;
		frameDescriptor.ArraySize = 1;
		frameDescriptor.SampleDesc.Count = 1;
		hr = m_Directx.d11dev->CreateTexture2D(&frameDescriptor, NULL, &m_Directx.dxgi_text2d);
		if (FAILED(hr)) {
			m_Directx.dxgi_dup->ReleaseFrame(); 
			printf("CreateTexture2D err=0x%X\n", hr);
			return;
		}
		hr = m_Directx.dxgi_text2d->QueryInterface(__uuidof(IDXGISurface), (void**)&m_Directx.dxgi_surf);
		if (FAILED(hr)) {
			SAFE_RELEASE(m_Directx.dxgi_text2d);
			m_Directx.dxgi_dup->ReleaseFrame(); 
			printf("CreateTexture2D 2 err=0x%X\n", hr);
			return;
		}
		hr = m_Directx.dxgi_surf->Map(&mappedRect, DXGI_MAP_READ);
		if (FAILED(hr)) {
			printf("Map Data buffer error\n");
			SAFE_RELEASE(m_Directx.dxgi_text2d);
			SAFE_RELEASE(m_Directx.dxgi_surf);
			m_Directx.dxgi_dup->ReleaseFrame();
			return;
		}

		if (!m_Directx.buffer) 
			m_Directx.buffer = (byte*)mappedRect.pBits;
	}

	//��ȡ����֡����
	m_Directx.d11ctx->CopyResource(m_Directx.dxgi_text2d, image2d);

L:
	CallbackFrameInfo frm;

	frm.width = m_Directx.cx;
	frm.height = m_Directx.cy;
	frm.line_bytes = m_Directx.line_bytes;
	frm.line_stride = m_Directx.line_stride;
	frm.bitcount = m_Directx.bitcount;
	frm.buffer = (char*)m_Directx.buffer;
	frm.length = m_Directx.line_stride * m_Directx.cy;
	onFrame(&frm, onframe_param);
}

void CCapture::capture_gdi()
{
	if (!m_GDI.buffer || !m_GDI.hbmp)return;

	HDC hdc = CreateDC(NULL, m_DispName, NULL, NULL);
	BitBlt(m_GDI.memdc, 0, 0, m_GDI.cx, m_GDI.cy, hdc, 0, 0, SRCCOPY | CAPTUREBLT);
	ReleaseDC(NULL, hdc);

	CallbackFrameInfo frm;

	frm.width = m_GDI.cx;
	frm.height = m_GDI.cy;
	frm.line_bytes = m_GDI.line_bytes;
	frm.line_stride = m_GDI.line_stride;
	frm.bitcount = m_GDI.bitcount;
	frm.buffer = (char*)m_GDI.buffer;
	frm.length = m_GDI.line_stride * m_GDI.cy;
	onFrame(&frm, onframe_param);
}

BOOL CCapture::__init_mirror(BOOL is_init)
{
	BOOL r = FALSE;
	if (is_init) {
#ifdef DMF_MIRROR
		m_Mirror.buffer.Userbuffer = NULL;
#else
		m_Mirror.buffer = NULL;
#endif
		m_Mirror.is_active = FALSE;
		r = active_mirror_driver(TRUE, &m_Mirror.disp);
		if (!r) {
			printf("Active Mirror Driver Error\n");
		}
		else {
			m_Mirror.is_active = TRUE;
		}
		return m_Mirror.is_active;
	}
	else { // deinit
#ifdef DMF_MIRROR
		if (m_Mirror.buffer.Userbuffer) {
			m_Mirror.buffer.Userbuffer = NULL;
#else
		if (m_Mirror.buffer) {
			m_Mirror.buffer = NULL;
#endif
			map_and_unmap_buffer(m_Mirror.disp.DeviceName, FALSE, &m_Mirror.buffer);
		}

		m_Mirror.is_active = FALSE;
		r = active_mirror_driver(FALSE, &m_Mirror.disp);

		printf("unload mirror driver ret=%d\n", r);
		return TRUE;
	}
}

BOOL CCapture::__init_dxgi()
{
	HRESULT hr;
	static HMODULE hmod = 0; 
	if (!hmod)hmod = LoadLibrary("d3d11.dll");
	fnD3D11CreateDevice fn = (fnD3D11CreateDevice)GetProcAddress(hmod, "D3D11CreateDevice");
	if (!fn) {
		printf("DXGI init :Not Load [D3D11CreateDevice] function \n");
		return FALSE;
	}
	D3D_DRIVER_TYPE DriverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT NumDriverTypes = ARRAYSIZE(DriverTypes);
	D3D_FEATURE_LEVEL FeatureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_1
	};
	UINT NumFeatureLevels = ARRAYSIZE(FeatureLevels);
	D3D_FEATURE_LEVEL FeatureLevel;
	for (UINT DriverTypeIndex = 0; DriverTypeIndex < NumDriverTypes; ++DriverTypeIndex) {
		hr = fn(nullptr, DriverTypes[DriverTypeIndex], nullptr, 0, FeatureLevels, NumFeatureLevels,
			D3D11_SDK_VERSION, &m_Directx.d11dev, &FeatureLevel, &m_Directx.d11ctx);
		if (SUCCEEDED(hr))break;
	}
	if (FAILED(hr)) {
		printf("DXGI Init: D3D11CreateDevice Return hr=0x%X\n", hr);
		return FALSE;
	}
	IDXGIDevice* dxdev = 0;
	hr = m_Directx.d11dev->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxdev);
	if (FAILED(hr)) {
		__init_directx(FALSE); 
		printf("DXGI init: IDXGIDevice Error hr=0x%X\n", hr);
		return FALSE;
	}
	IDXGIAdapter* DxgiAdapter = 0;
	hr = dxdev->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&DxgiAdapter));
	SAFE_RELEASE(dxdev);
	if (FAILED(hr)) {
		__init_directx(FALSE); 
		printf("DXGI init: IDXGIAdapter Error hr=0x%X\n", hr);
		return FALSE;
	}
	INT nOutput = m_MonitorID;
	IDXGIOutput* DxgiOutput = 0;
	hr = DxgiAdapter->EnumOutputs(nOutput, &DxgiOutput);
	SAFE_RELEASE(DxgiAdapter);
	if (FAILED(hr)) {
		__init_directx(FALSE);
		printf("DXGI init: IDXGIOutput Error hr=0x%X\n", hr);
		return FALSE;
	}
	DxgiOutput->GetDesc(&m_Directx.dxgi_desc);
	IDXGIOutput1* DxgiOutput1 = 0;
	hr = DxgiOutput->QueryInterface(__uuidof(IDXGIOutput1), reinterpret_cast<void**>(&DxgiOutput1));
	SAFE_RELEASE(DxgiOutput);
	if (FAILED(hr)) {
		__init_directx(FALSE); 
		printf("DXGI init: IDXGIOutput1 Error hr=0x%X\n", hr);
		return FALSE;
	}
	hr = DxgiOutput1->DuplicateOutput(m_Directx.d11dev, &m_Directx.dxgi_dup);
	SAFE_RELEASE(DxgiOutput1);
	if (FAILED(hr)) {
		__init_directx(FALSE);
		printf("DXGI init: IDXGIOutputDuplication Error hr=0x%X\n", hr);
		return FALSE;
	}
	return TRUE;
}

BOOL CCapture::__init_directx(BOOL is_init)
{
	if (is_init) {
		DISPLAY_DEVICE dispDevice;
		DEVMODE devmode;
		memset(&devmode, 0, sizeof(DEVMODE));
		devmode.dmSize = sizeof(DEVMODE);
		devmode.dmDriverExtra = 0;
		BOOL bRet = find_display_device(&dispDevice, &devmode, FALSE, m_MonitorID);
		if (!bRet)return FALSE;

		m_Directx.cx = devmode.dmPelsWidth;
		m_Directx.cy = devmode.dmPelsHeight;
		m_Directx.bitcount = devmode.dmBitsPerPel;
		if (m_Directx.bitcount != 8 && m_Directx.bitcount != 16 && m_Directx.bitcount != 32 && m_Directx.bitcount != 24) {
			return FALSE;
		}
		m_Directx.line_bytes = m_Directx.cx * m_Directx.bitcount / 8;
		m_Directx.line_stride = (m_Directx.line_bytes + 3) / 4 * 4;

		//DXGI above win8
		if (__init_dxgi()) {
			return TRUE;
		}

		return FALSE;
	}
	else {
		SAFE_RELEASE(m_Directx.dxgi_dup);
		SAFE_RELEASE(m_Directx.d11dev);
		SAFE_RELEASE(m_Directx.d11ctx);
		SAFE_RELEASE(m_Directx.dxgi_text2d);
		if (m_Directx.dxgi_surf) {
			m_Directx.dxgi_surf->Unmap();
		}
		SAFE_RELEASE(m_Directx.dxgi_surf);
		if (m_Directx.buffer) {
			m_Directx.buffer = NULL;
		}
		m_Directx.is_acquire_frame = 0;
		Sleep(2000);

		return TRUE;
	}

	return FALSE;
}

BOOL CCapture::__init_gdi(BOOL is_init)
{
	if (is_init) {
		DISPLAY_DEVICE dispDevice;
		DEVMODE devmode;
		memset(&devmode, 0, sizeof(DEVMODE));
		devmode.dmSize = sizeof(DEVMODE);
		devmode.dmDriverExtra = 0;
		BOOL bRet = find_display_device(&dispDevice, &devmode, FALSE, m_MonitorID);
		if (!bRet)return FALSE;

		m_GDI.cx = devmode.dmPelsWidth;
		m_GDI.cy = devmode.dmPelsHeight;
		m_GDI.bitcount = devmode.dmBitsPerPel;
		if (m_GDI.bitcount != 8 && m_GDI.bitcount != 16 && m_GDI.bitcount != 32 && m_GDI.bitcount != 24) {
			return FALSE;
		}
		m_GDI.line_bytes = m_GDI.cx * m_GDI.bitcount / 8;
		m_GDI.line_stride = (m_GDI.line_bytes + 3) / 4 * 4;

		BITMAPINFOHEADER bi; memset(&bi, 0, sizeof(bi));
		bi.biSize = sizeof(bi);
		bi.biWidth = m_GDI.cx;
		bi.biHeight = -m_GDI.cy; //���ϳ���ɨ��
		bi.biPlanes = 1;
		bi.biBitCount = m_GDI.bitcount; //RGB
		bi.biCompression = BI_RGB;
		bi.biSizeImage = 0;
		BYTE bb[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256];
		memcpy(bb, &bi, sizeof(bi));
		HDC hdc = CreateDC(NULL, m_DispName, NULL, NULL); //��ĻDC
		if (m_GDI.bitcount == 8) {//ϵͳ��ɫ��
			PALETTEENTRY pal[256]; memset(pal, 0, sizeof(pal));
			GetSystemPaletteEntries(hdc, 0, 256, pal); //��ȡϵͳ��ɫ��
			RGBQUAD* PalColor = (RGBQUAD*)(bb + sizeof(bi));
			for (int i = 0; i < 256; ++i) {
				PalColor[i].rgbReserved = 0;
				PalColor[i].rgbRed = pal[i].peRed;
				PalColor[i].rgbGreen = pal[i].peGreen;
				PalColor[i].rgbBlue = pal[i].peBlue;
			}
		}

		m_GDI.memdc = CreateCompatibleDC(hdc);
		m_GDI.buffer = NULL;
		m_GDI.hbmp = CreateDIBSection(hdc, (BITMAPINFO*)bb, DIB_RGB_COLORS, (void**)&m_GDI.buffer, NULL, 0);
		ReleaseDC(NULL, hdc);
		if (!m_GDI.buffer) {
			DeleteDC(m_GDI.memdc); m_GDI.memdc = 0;
			return FALSE;
		}
		SelectObject(m_GDI.memdc, m_GDI.hbmp);
		return TRUE;
	}
	else {
		m_GDI.buffer = 0;
		DeleteDC(m_GDI.memdc); m_GDI.memdc = NULL;
		DeleteObject(m_GDI.hbmp); m_GDI.hbmp = NULL;
	}

	return FALSE;
}
