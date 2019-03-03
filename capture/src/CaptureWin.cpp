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
	grab_type = 0;
	monitor_id = id;
	onFrame = on_frame;
	onframe_param = param;

	quit = FALSE;
	pause_grab = FALSE;

	memset(&mirror, 0, sizeof(mirror));
	memset(&directx, 0, sizeof(directx));
	memset(&gdi, 0, sizeof(gdi));
	
	sleep_msec = 25;
	hMessageWnd = NULL;
	hEvt = CreateEvent(NULL, TRUE, FALSE, NULL);
	memset(t_arr, 0, sizeof(t_arr));
	h_thread = NULL;

	interval_count = 5;
	ack_seq = 0;
	capture_seq = 0;
}

CCapture::~CCapture()
{
}

void CCapture::start()
{
	DWORD tid;
	h_thread = CreateThread(NULL, 10 * 1024 * 1024, __loop_msg, this, 0, &tid);

	if (!h_thread) {
		CloseHandle(hEvt);
	}
	::SetThreadPriority(h_thread, THREAD_PRIORITY_HIGHEST); //提升优先级
	::WaitForSingleObject(hEvt, INFINITE);
	::ResetEvent(hEvt); 
	if (!hMessageWnd) {
		CloseHandle(hEvt); 
		CloseHandle(h_thread);
	}
}

void CCapture::stop()
{
	hMessageWnd = NULL;
	quit = TRUE;
}

void CCapture::pause()
{
	pause_grab = TRUE;
}

void CCapture::resume()
{
	pause_grab = FALSE;
}

DWORD CALLBACK CCapture::__loop_msg(void* _p)
{
	CCapture* dp = (CCapture*)_p;
	::CoInitialize(0);

	int grab_type = dp->grab_type;
	BOOL is_ok = FALSE;
	if (grab_type == GRAB_TYPE_AUTO) {
		is_ok = dp->__init_mirror(TRUE);
		if (is_ok) grab_type = GRAB_TYPE_MIRROR;
		else {
			is_ok = dp->__init_directx(TRUE);
			if (is_ok) grab_type = GRAB_TYPE_DIRECTX;
			else {
				is_ok = dp->__init_gdi(TRUE);
				if (is_ok) grab_type = GRAB_TYPE_GDI;
			}
		}
		dp->grab_type = grab_type;
	}
	else if (grab_type == GRAB_TYPE_MIRROR) {
		is_ok = dp->__init_mirror(TRUE);
	}
	else if (grab_type == GRAB_TYPE_DIRECTX) {
		is_ok = dp->__init_directx(TRUE);
		if (!is_ok) {
			is_ok = dp->__init_mirror(TRUE);
			if (is_ok) {
				grab_type = GRAB_TYPE_MIRROR;
				dp->grab_type = grab_type;
			}
			else {
				is_ok = dp->__init_gdi(TRUE);
				if (is_ok) {
					grab_type = GRAB_TYPE_GDI;
					dp->grab_type = grab_type;
				}
			}
		}
	}
	else if (grab_type == GRAB_TYPE_GDI) {
		is_ok = dp->__init_gdi(TRUE);
	}

	if (!is_ok) {
		printf("Can not init capture screen type=%d\n", grab_type);
		dp->hMessageWnd = NULL;
		SetEvent(dp->hEvt);
		CoUninitialize();
		return 0;
	}

	HWND hwnd = dp->CreateMsgWnd();
	HANDLE hEvt = dp->hEvt;

	if (!hwnd) {
		switch (grab_type) {
		case GRAB_TYPE_MIRROR:  dp->__init_mirror(FALSE);  break;
		case GRAB_TYPE_DIRECTX: dp->__init_directx(FALSE); break;
		case GRAB_TYPE_GDI:     dp->__init_gdi(FALSE);     break;
		}
		dp->hMessageWnd = NULL;
		SetEvent(hEvt);
		CoUninitialize();
		return 0;
	}
	SetEvent(hEvt);

	long sleep_msec = dp->sleep_msec;

	MSG msg;
	BOOL bQuit = FALSE;
	QueryPerformanceFrequency(&dp->counter);
	while (!dp->quit) {	
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

		sleep_msec = dp->sleep_msec;
		if (!dp->pause_grab) {
			while ((!dp->quit) && (dp->capture_seq - dp->ack_seq > dp->interval_count)) {
				//printf("Sequence D-value %d\n", dp->capture_seq - dp->ack_seq);
				SleepEx(10, TRUE);
			}

			if (dp->quit) {
				goto End;
			}
			QueryPerformanceCounter(&dp->frame_begin);
			if (grab_type == GRAB_TYPE_MIRROR) {
				dp->capture_mirror();
			}
			else if (grab_type == GRAB_TYPE_DIRECTX) {
				dp->capture_dxgi();
			}
			else if (grab_type == GRAB_TYPE_GDI) {
				dp->capture_gdi();
			}
			dp->capture_seq++;
			QueryPerformanceCounter(&dp->frame_end);

			LONGLONG dt = (dp->frame_end.QuadPart - dp->frame_begin.QuadPart) * 1000 / dp->counter.QuadPart;
			if (dt >= dp->sleep_msec) {
				sleep_msec = 0;
			}
			else {
				sleep_msec = dp->sleep_msec - (long)dt;
			}
		}
		SleepEx(sleep_msec, TRUE);
	}

End:
	CloseHandle(hEvt);
	switch (dp->grab_type) {
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
			if (dp->grab_type == GRAB_TYPE_MIRROR) {
				dp->change_display(LOWORD(lParam), HIWORD(lParam), wParam);
			}
			else if (dp->grab_type == GRAB_TYPE_DIRECTX) {
				dp->__init_directx(FALSE);
				dp->__init_directx(TRUE);
				printf("directx restart\n");
			}
			else if (dp->grab_type == GRAB_TYPE_GDI) {
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
	hMessageWnd = hwnd;
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
	if (mirror.buffer.Userbuffer) {
#else
	if (mirror.buffer) {
#endif
		map_and_unmap_buffer(mirror.disp.DeviceName, FALSE, &mirror.buffer);
		mirror.last_check_drawbuf_time = 0;
#ifdef DMF_MIRROR
		mirror.buffer.Userbuffer = NULL;
#else
		mirror.buffer = NULL;
#endif
	}

	DEVMODE devmode;
	memset(&devmode, 0, sizeof(DEVMODE));
	devmode.dmSize = sizeof(DEVMODE);
	devmode.dmDriverExtra = 0;
	BOOL bRet = EnumDisplaySettings(disp_name, ENUM_CURRENT_SETTINGS, &devmode);
	devmode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	strcpy((char*)devmode.dmDeviceName, "xietong_mirror");
	devmode.dmPelsWidth = w;
	devmode.dmPelsHeight = h;
	devmode.dmBitsPerPel = bits;

	INT code = ChangeDisplaySettingsEx(mirror.disp.DeviceName, &devmode, NULL, CDS_UPDATEREGISTRY, NULL);
	printf("Change_display Update Registry on device mode: %s\n", GetDispCode(code));
}

void CCapture::set_drop_interval(unsigned int count)
{
	interval_count = count;
}

void CCapture::set_ack_sequence(unsigned int seq)
{
	if (seq > ack_seq) {
		ack_seq = seq;
	}
}

unsigned int CCapture::get_capture_sequence()
{
	return capture_seq;
}

void CCapture::reset_sequence()
{
	ack_seq = capture_seq;
}

void CCapture::set_frame_rate(unsigned int rate)
{
	if (rate > 100) {
		return;
	}
	sleep_msec = 1000 / rate;
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
				if (dispDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) {//主显示器
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
		strcpy_s(disp_name, dispDevice.DeviceName);
		bRet = EnumDisplaySettings(disp_name, ENUM_CURRENT_SETTINGS, mode);
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

//激活或者取消镜像驱动
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
	if (!mirror.buffer.Userbuffer && abs(time(0) - mirror.last_check_drawbuf_time) > 5) {
		mirror.last_check_drawbuf_time = time(0);
		map_and_unmap_buffer(mirror.disp.DeviceName, TRUE, &mirror.buffer);
	}
	if (!mirror.buffer.Userbuffer) {
		return;
	}

	CallbackFrameInfo frm;

	frm.grab_type = 1;
	frm.width = GetSystemMetrics(SM_CXSCREEN);
	frm.height = GetSystemMetrics(SM_CYSCREEN);
	frm.line_bytes = frm.width * 4;
	frm.line_stride = frm.width * 4;
	frm.bitcount = 32;
	frm.buffer = (char*)mirror.buffer.Userbuffer;
	frm.length = frm.width * frm.height * 4;
#else
	if (!mirror.buffer && abs(time(0) - mirror.last_check_drawbuf_time) > 5) {
		mirror.last_check_drawbuf_time = time(0);
		map_and_unmap_buffer(mirror.disp.DeviceName, TRUE, &mirror.buffer);
	}
	if (!mirror.buffer) {
		return;
	}

	CallbackFrameInfo frm;

	frm.grab_type = 1;
	frm.width = mirror.buffer->cx;
	frm.height = mirror.buffer->cy;
	frm.line_bytes = mirror.buffer->line_bytes;
	frm.line_stride = mirror.buffer->line_stride;
	frm.bitcount = mirror.buffer->bitcount;
	frm.buffer = (char*)mirror.buffer->data_buffer;
	frm.length = mirror.buffer->data_length;
#endif
	onFrame(&frm, onframe_param);
}

void CCapture::capture_dxgi()
{
	if (!directx.dxgi_dup) {
		return;
	}

	IDXGIResource* DesktopResource = 0;
	DXGI_OUTDUPL_FRAME_INFO FrameInfo;
	HRESULT hr;
	ID3D11Texture2D* image2d = NULL;

	if (directx.is_acquire_frame) {
		directx.dxgi_dup->ReleaseFrame();
		directx.is_acquire_frame = 0;
	}
	hr = directx.dxgi_dup->AcquireNextFrame(0, &FrameInfo, &DesktopResource);
	if (FAILED(hr)) {   
		if (hr == _HRESULT_TYPEDEF_(0x887A0026L) || hr == _HRESULT_TYPEDEF_(0x887A0001L)) {
			__init_directx(FALSE);
			__init_directx(TRUE);
			return;
		}
		if (!directx.buffer)
			return;
		//printf("acquire frame fail 0x%X\n", hr);
		goto L;
	}
	directx.is_acquire_frame = 1;

	SAFE_RELEASE(image2d);
	hr = DesktopResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&image2d);
	SAFE_RELEASE(DesktopResource);
	if (FAILED(hr)) {
		return;
	}

	DXGI_MAPPED_RECT mappedRect;
	if (!directx.dxgi_text2d) {
		D3D11_TEXTURE2D_DESC frameDescriptor;
		image2d->GetDesc(&frameDescriptor);
		frameDescriptor.Usage = D3D11_USAGE_STAGING;
		frameDescriptor.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		frameDescriptor.BindFlags = 0;
		frameDescriptor.MiscFlags = 0;
		frameDescriptor.MipLevels = 1;
		frameDescriptor.ArraySize = 1;
		frameDescriptor.SampleDesc.Count = 1;
		hr = directx.d11dev->CreateTexture2D(&frameDescriptor, NULL, &directx.dxgi_text2d);
		if (FAILED(hr)) {
			directx.dxgi_dup->ReleaseFrame(); 
			printf("CreateTexture2D err=0x%X\n", hr);
			return;
		}
		hr = directx.dxgi_text2d->QueryInterface(__uuidof(IDXGISurface), (void**)&directx.dxgi_surf);
		if (FAILED(hr)) {
			SAFE_RELEASE(directx.dxgi_text2d);
			directx.dxgi_dup->ReleaseFrame(); 
			printf("CreateTexture2D 2 err=0x%X\n", hr);
			return;
		}
		hr = directx.dxgi_surf->Map(&mappedRect, DXGI_MAP_READ);
		if (FAILED(hr)) {
			printf("Map Data buffer error\n");
			SAFE_RELEASE(directx.dxgi_text2d);
			SAFE_RELEASE(directx.dxgi_surf);
			directx.dxgi_dup->ReleaseFrame();
			return;
		}

		if (!directx.buffer) 
			directx.buffer = (byte*)mappedRect.pBits;
	}

	//获取整个帧数据
	directx.d11ctx->CopyResource(directx.dxgi_text2d, image2d);

L:
	CallbackFrameInfo frm;

	frm.grab_type = 2;
	frm.width = directx.cx;
	frm.height = directx.cy;
	frm.line_bytes = directx.line_bytes;
	frm.line_stride = directx.line_stride;
	frm.bitcount = directx.bitcount;
	frm.buffer = (char*)directx.buffer;
	frm.length = directx.line_stride * directx.cy;
	onFrame(&frm, onframe_param);
}

void CCapture::capture_gdi()
{
	if (!gdi.buffer || !gdi.hbmp)return;

	HDC hdc = CreateDC(NULL, disp_name, NULL, NULL);
	BitBlt(gdi.memdc, 0, 0, gdi.cx, gdi.cy, hdc, 0, 0, SRCCOPY | CAPTUREBLT);
	ReleaseDC(NULL, hdc);

	CallbackFrameInfo frm;

	frm.grab_type = 3;
	frm.width = gdi.cx;
	frm.height = gdi.cy;
	frm.line_bytes = gdi.line_bytes;
	frm.line_stride = gdi.line_stride;
	frm.bitcount = gdi.bitcount;
	frm.buffer = (char*)gdi.buffer;
	frm.length = gdi.line_stride * gdi.cy;
	onFrame(&frm, onframe_param);
}

BOOL CCapture::__init_mirror(BOOL is_init)
{
	BOOL r = FALSE;
	if (is_init) {
#ifdef DMF_MIRROR
		mirror.buffer.Userbuffer = NULL;
#else
		mirror.buffer = NULL;
#endif
		mirror.is_active = FALSE;
		r = active_mirror_driver(TRUE, &mirror.disp);
		if (!r) {
			printf("Active Mirror Driver Error\n");
		}
		else {
			mirror.is_active = TRUE;
		}
		return mirror.is_active;
	}
	else { // deinit
#ifdef DMF_MIRROR
		if (mirror.buffer.Userbuffer) {
			mirror.buffer.Userbuffer = NULL;
#else
		if (mirror.buffer) {
			mirror.buffer = NULL;
#endif
			map_and_unmap_buffer(mirror.disp.DeviceName, FALSE, &mirror.buffer);
		}

		mirror.is_active = FALSE;
		r = active_mirror_driver(FALSE, &mirror.disp);

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
			D3D11_SDK_VERSION, &directx.d11dev, &FeatureLevel, &directx.d11ctx);
		if (SUCCEEDED(hr))break;
	}
	if (FAILED(hr)) {
		printf("DXGI Init: D3D11CreateDevice Return hr=0x%X\n", hr);
		return FALSE;
	}
	IDXGIDevice* dxdev = 0;
	hr = directx.d11dev->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxdev);
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
	INT nOutput = monitor_id;
	IDXGIOutput* DxgiOutput = 0;
	hr = DxgiAdapter->EnumOutputs(nOutput, &DxgiOutput);
	SAFE_RELEASE(DxgiAdapter);
	if (FAILED(hr)) {
		__init_directx(FALSE);
		printf("DXGI init: IDXGIOutput Error hr=0x%X\n", hr);
		return FALSE;
	}
	DxgiOutput->GetDesc(&directx.dxgi_desc);
	IDXGIOutput1* DxgiOutput1 = 0;
	hr = DxgiOutput->QueryInterface(__uuidof(IDXGIOutput1), reinterpret_cast<void**>(&DxgiOutput1));
	SAFE_RELEASE(DxgiOutput);
	if (FAILED(hr)) {
		__init_directx(FALSE); 
		printf("DXGI init: IDXGIOutput1 Error hr=0x%X\n", hr);
		return FALSE;
	}
	hr = DxgiOutput1->DuplicateOutput(directx.d11dev, &directx.dxgi_dup);
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
		BOOL bRet = find_display_device(&dispDevice, &devmode, FALSE, monitor_id);
		if (!bRet)return FALSE;

		directx.cx = devmode.dmPelsWidth;
		directx.cy = devmode.dmPelsHeight;
		directx.bitcount = devmode.dmBitsPerPel;
		if (directx.bitcount != 8 && directx.bitcount != 16 && directx.bitcount != 32 && directx.bitcount != 24) {
			return FALSE;
		}
		directx.line_bytes = directx.cx * directx.bitcount / 8;
		directx.line_stride = (directx.line_bytes + 3) / 4 * 4;

		//DXGI above win8
		if (__init_dxgi()) {
			return TRUE;
		}

		return FALSE;
	}
	else {
		SAFE_RELEASE(directx.dxgi_dup);
		SAFE_RELEASE(directx.d11dev);
		SAFE_RELEASE(directx.d11ctx);
		SAFE_RELEASE(directx.dxgi_text2d);
		if (directx.dxgi_surf) {
			directx.dxgi_surf->Unmap();
		}
		SAFE_RELEASE(directx.dxgi_surf);
		if (directx.buffer) {
			directx.buffer = NULL;
		}
		directx.is_acquire_frame = 0;
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
		BOOL bRet = find_display_device(&dispDevice, &devmode, FALSE, monitor_id);
		if (!bRet)return FALSE;

		gdi.cx = devmode.dmPelsWidth;
		gdi.cy = devmode.dmPelsHeight;
		gdi.bitcount = devmode.dmBitsPerPel;
		if (gdi.bitcount != 8 && gdi.bitcount != 16 && gdi.bitcount != 32 && gdi.bitcount != 24) {
			return FALSE;
		}
		gdi.line_bytes = gdi.cx * gdi.bitcount / 8;
		gdi.line_stride = (gdi.line_bytes + 3) / 4 * 4;

		BITMAPINFOHEADER bi; memset(&bi, 0, sizeof(bi));
		bi.biSize = sizeof(bi);
		bi.biWidth = gdi.cx;
		bi.biHeight = -gdi.cy; //从上朝下扫描
		bi.biPlanes = 1;
		bi.biBitCount = gdi.bitcount; //RGB
		bi.biCompression = BI_RGB;
		bi.biSizeImage = 0;
		BYTE bb[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256];
		memcpy(bb, &bi, sizeof(bi));
		HDC hdc = CreateDC(NULL, disp_name, NULL, NULL); //屏幕DC
		if (gdi.bitcount == 8) {//系统调色板
			PALETTEENTRY pal[256]; memset(pal, 0, sizeof(pal));
			GetSystemPaletteEntries(hdc, 0, 256, pal); //获取系统调色板
			RGBQUAD* PalColor = (RGBQUAD*)(bb + sizeof(bi));
			for (int i = 0; i < 256; ++i) {
				PalColor[i].rgbReserved = 0;
				PalColor[i].rgbRed = pal[i].peRed;
				PalColor[i].rgbGreen = pal[i].peGreen;
				PalColor[i].rgbBlue = pal[i].peBlue;
			}
		}

		gdi.memdc = CreateCompatibleDC(hdc);
		gdi.buffer = NULL;
		gdi.hbmp = CreateDIBSection(hdc, (BITMAPINFO*)bb, DIB_RGB_COLORS, (void**)&gdi.buffer, NULL, 0);
		ReleaseDC(NULL, hdc);
		if (!gdi.buffer) {
			DeleteDC(gdi.memdc); gdi.memdc = 0;
			return FALSE;
		}
		SelectObject(gdi.memdc, gdi.hbmp);
		return TRUE;
	}
	else {
		gdi.buffer = 0;
		DeleteDC(gdi.memdc); gdi.memdc = NULL;
		DeleteObject(gdi.hbmp); gdi.hbmp = NULL;
	}

	return FALSE;
}
