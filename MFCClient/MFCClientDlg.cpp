
// MFCClientDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "MFCClient.h"
#include "MFCClientDlg.h"
#include "afxdialogex.h"
#include "DaemonApi.h"
#include "imm.h"
#include "io.h"
#pragma comment(lib, "imm32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMFCClientDlg 对话框

#define AGENT_LBUTTON_MASK (1 << 1)
#define AGENT_MBUTTON_MASK (1 << 2)
#define AGENT_RBUTTON_MASK (1 << 3)
#define AGENT_UBUTTON_MASK (1 << 4)
#define AGENT_DBUTTON_MASK (1 << 5)

#define MYWM_NOTIFYICON WM_USER+1

static bool m_bOperater = false;
static HWND m_hShowWnd = NULL;
static DWORD m_dwButtonState = 0;
static DWORD m_dwInputTime = 0;
static CRect m_rectOrigin;
CMFCClientDlg::CMFCClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCCLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMFCClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEHWHEEL()
	ON_WM_LBUTTONDBLCLK()
	ON_BN_CLICKED(IDC_CHECK1, &CMFCClientDlg::OnBnClickedSaveUrl)
	ON_BN_CLICKED(IDC_CHECK2, &CMFCClientDlg::OnBnClickedAutoConnect)
	ON_BN_CLICKED(IDC_BUTTON1, &CMFCClientDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CMFCClientDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CMFCClientDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CMFCClientDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CMFCClientDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, &CMFCClientDlg::OnBnClickedButton6)
	ON_MESSAGE(MYWM_NOTIFYICON, OnNotifyIcon)
	ON_BN_CLICKED(IDC_BUTTON7, &CMFCClientDlg::OnBnClickedButton7)
END_MESSAGE_MAP()


// CMFCClientDlg 消息处理程序

BOOL CMFCClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_uButtonMask = 0;
	m_bOperater = false;
	m_dwButtonState = 0;
	m_dwInputTime = 0;
	ImmDisableIME(0);
	m_hShowWnd = GetSafeHwnd();
	FILE* fp = fopen("addr", "rb");
	if (fp) {
		char url[64] = { 0 };
		fread(url, 1, sizeof(url), fp);
		fclose(fp);
		SetDlgItemText(IDC_EDIT1, url);
		((CButton *)GetDlgItem(IDC_CHECK1))->SetCheck(1);
		if (_access("auto", 0) == 0) {
			((CButton *)GetDlgItem(IDC_CHECK2))->SetCheck(1);
			OnBnClickedButton1();
		}
	}

	//CenterWindow();
	GetWindowRect(&m_rectOrigin);

	CreateNotifyIcon();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMFCClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMFCClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
		GetClientRect(&m_rectClient);
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMFCClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMFCClientDlg::OnClose()
{
	daemon_stop();
	Shell_NotifyIcon(NIM_DELETE, &m_nd);
	CDialogEx::OnClose();
}

void CMFCClientDlg::CreateNotifyIcon()
{
	m_nd.cbSize = sizeof(NOTIFYICONDATA);
	m_nd.hWnd = m_hShowWnd;
	m_nd.uID = IDR_MAINFRAME;
	m_nd.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	m_nd.uCallbackMessage = MYWM_NOTIFYICON;
	HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MAINFRAME));
	m_nd.hIcon = hIcon;
	strcpy(m_nd.szTip, "");

	Shell_NotifyIcon(NIM_ADD, &m_nd);
}


void CMFCClientDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bOperater) {
		if (point.x != m_poCursorPos.x || point.y != m_poCursorPos.y) {
			m_poCursorPos.x = point.x;
			m_poCursorPos.y = point.y;
			scale_to_screen(point);
			daemon_send_mouse_event(point.x, point.y, m_uButtonMask);
		}
	}
	else {
		CDialogEx::OnMouseMove(nFlags, point);
	}
}

void CMFCClientDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_bOperater) {
		m_uButtonMask |= AGENT_LBUTTON_MASK;
		scale_to_screen(point);
		daemon_send_mouse_event(point.x, point.y, m_uButtonMask);
	}
	else {
		CDialogEx::OnLButtonDown(nFlags, point);
	}
}

void CMFCClientDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bOperater) {
		m_uButtonMask &= ~AGENT_LBUTTON_MASK;
		scale_to_screen(point);
		daemon_send_mouse_event(point.x, point.y, m_uButtonMask);
	}
	else {
		CDialogEx::OnLButtonUp(nFlags, point);
	}
}

void CMFCClientDlg::OnMButtonDown(UINT nFlags, CPoint point)
{
	if (m_bOperater) {
		m_uButtonMask |= AGENT_MBUTTON_MASK;
		scale_to_screen(point);
		daemon_send_mouse_event(point.x, point.y, m_uButtonMask);
	}
	else {
		CDialogEx::OnMButtonDown(nFlags, point);
	}
}

void CMFCClientDlg::OnMButtonUp(UINT nFlags, CPoint point)
{
	if (m_bOperater) {
		m_uButtonMask &= ~AGENT_MBUTTON_MASK;
		scale_to_screen(point);
		daemon_send_mouse_event(point.x, point.y, m_uButtonMask);
	}
	else {
		CDialogEx::OnMButtonUp(nFlags, point);
	}
}

void CMFCClientDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (m_bOperater) {
		m_uButtonMask |= AGENT_RBUTTON_MASK;
		scale_to_screen(point);
		daemon_send_mouse_event(point.x, point.y, m_uButtonMask);
	}
	else {
		CDialogEx::OnRButtonDown(nFlags, point);
	}
}

void CMFCClientDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (m_bOperater) {
		m_uButtonMask &= ~AGENT_RBUTTON_MASK;
		scale_to_screen(point);
		daemon_send_mouse_event(point.x, point.y, m_uButtonMask);
	}
	else {
		CDialogEx::OnRButtonUp(nFlags, point);
	}
}

void CMFCClientDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint point)
{
	if (m_bOperater) {
		if (zDelta > 0) {
			m_uButtonMask |= AGENT_UBUTTON_MASK;
		}
		else {
			m_uButtonMask |= AGENT_DBUTTON_MASK;
		}
		daemon_send_mouse_event(point.x, point.y, m_uButtonMask);
		if (zDelta > 0) {
			m_uButtonMask &= ~AGENT_UBUTTON_MASK;
		}
		else {
			m_uButtonMask &= ~AGENT_DBUTTON_MASK;
		}
	}
	else {
		CDialogEx::OnMouseWheel(nFlags, zDelta, point);
	}
}

void CMFCClientDlg::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (m_bOperater) {
		scale_to_screen(point);
		m_uButtonMask |= AGENT_LBUTTON_MASK;
		daemon_send_mouse_event(point.x, point.y, m_uButtonMask);
	}
	else {
		CDialogEx::OnLButtonDblClk(nFlags, point);
	}
}

void CMFCClientDlg::OnBnClickedButton1()
{
	char url[64];
	GetDlgItemText(IDC_EDIT1, url, sizeof(url));
	if (strlen(url) == 0) {
		return;
	}
	bool ret = daemon_start(url);
	if (ret) {
		daemon_set_start_stream_callback(start_stream_callback);
		daemon_set_stop_stream_callback(stop_stream_callback);
		daemon_set_picture_callback(recv_picture_callback);
		daemon_set_operater_callback(start_operate_callback);
		daemon_set_mouse_callback(recv_mouse_event_callback);
		daemon_set_keyboard_callback(recv_keyboard_event_callback);
		GetDlgItem(IDC_BUTTON1)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON6)->EnableWindow(TRUE);
	}
	else {
		MessageBox("连接失败！");
		return;
	}
	printf("connect succeed\n");
}


void CMFCClientDlg::OnBnClickedButton2()
{
	daemon_start_stream();
}


void CMFCClientDlg::OnBnClickedButton3()
{
	daemon_stop_stream();
}


void CMFCClientDlg::OnBnClickedButton4()
{
	daemon_send_picture();
}


void CMFCClientDlg::OnBnClickedButton5()
{
	daemon_start_operate();
}

void CMFCClientDlg::OnBnClickedButton6()
{
	ShowWindow(SW_HIDE);
	daemon_start_stream();
}

void CMFCClientDlg::OnBnClickedButton7()
{
	UsersInfo info;
	daemon_get_users_info(&info);
	printf("user num: %d\n", info.user_num);
	for (int i = 0; i < 20 && i < info.user_num; i++) {
		printf("user %d: %s\n", i, info.user_list[i]);
	}
	printf("publisher: %s\n", info.publisher);
	printf("operater: %s\n", info.operater);
}

LRESULT CMFCClientDlg::OnNotifyIcon(WPARAM wParam, LPARAM lParam)
{
	if (lParam == WM_LBUTTONDBLCLK)
	{
		UINT MODE;
		MODE = IsWindowVisible() ? SW_HIDE : SW_SHOW;
		ShowWindow(MODE);
	}
	return 0;
}

void CMFCClientDlg::scale_to_screen(POINT& point)
{
	int w = 0;
	int h = 0;
	daemon_get_stream_size(&w, &h);

	if (w > 0 && h > 0) {
		point.x = point.x * w / m_rectClient.Width();
		point.y = point.y * h / m_rectClient.Height();
	}
}

void CMFCClientDlg::start_stream_callback()
{
	printf("start_stream_callback\n");
	enable_sub_windows(FALSE);
	//ModifyStyle(m_hShowWnd, WS_CAPTION, 0, 0);
	//::MoveWindow(m_hShowWnd, 0, 0, ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN), TRUE);
	::SetWindowPos(m_hShowWnd, HWND_TOP, 0, 0, ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN), SWP_SHOWWINDOW);
	daemon_show_stream(m_hShowWnd);
}

void CMFCClientDlg::stop_stream_callback()
{
	//::MoveWindow(m_hShowWnd, m_rectOrigin.left, m_rectOrigin.top, m_rectOrigin.Width(), m_rectOrigin.Height() + 20, TRUE);
	enable_sub_windows(TRUE);
	::SetWindowPos(m_hShowWnd, HWND_TOP, m_rectOrigin.left, m_rectOrigin.top, m_rectOrigin.Width(), m_rectOrigin.Height(), SWP_SHOWWINDOW);
	::InvalidateRect(m_hShowWnd, m_rectOrigin, TRUE);
	//ModifyStyle(m_hShowWnd, 0, WS_CAPTION, 0);
}

void CMFCClientDlg::recv_picture_callback(const char* file_path)
{
	char cmd[256];
	sprintf_s(cmd, "mspaint %s", file_path);
	printf("open file cmd: %s\n", cmd);
	system(cmd);
}

void CMFCClientDlg::start_operate_callback(bool is_operater)
{
	if (is_operater) {
		::SetDlgItemText(m_hShowWnd, IDC_STATIC2, "主控");
	}
	else {
		::SetDlgItemText(m_hShowWnd, IDC_STATIC2, "");
	}
	m_bOperater = is_operater;
}

void CMFCClientDlg::recv_mouse_event_callback(unsigned int x, unsigned int y, unsigned int button_mask)
{
	INPUT _input;
	DWORD mouse_move = 0;
	DWORD buttons_change = 0;
	DWORD mouse_wheel = 0;

	printf("recv mouse x %u y %u\n", x, y);
	ZeroMemory(&_input, sizeof(INPUT));
	_input.type = INPUT_MOUSE;

	DWORD w = ::GetSystemMetrics(SM_CXSCREEN);
	DWORD h = ::GetSystemMetrics(SM_CYSCREEN);
	w = (w > 1) ? w - 1 : 1; /* coordinates are 0..w-1, protect w==0 */
	h = (h > 1) ? h - 1 : 1; /* coordinates are 0..h-1, protect h==0 */
	mouse_move = MOUSEEVENTF_MOVE;
	_input.mi.dx = x * 0xffff / w;
	_input.mi.dy = y * 0xffff / h;

	if (button_mask != m_dwButtonState) {
		buttons_change = get_buttons_change(m_dwButtonState, button_mask, AGENT_LBUTTON_MASK,
			MOUSEEVENTF_LEFTDOWN, MOUSEEVENTF_LEFTUP) |
			get_buttons_change(m_dwButtonState, button_mask, AGENT_MBUTTON_MASK,
				MOUSEEVENTF_MIDDLEDOWN, MOUSEEVENTF_MIDDLEUP) |
			get_buttons_change(m_dwButtonState, button_mask, AGENT_RBUTTON_MASK,
				MOUSEEVENTF_RIGHTDOWN, MOUSEEVENTF_RIGHTUP);
		m_dwButtonState = button_mask;
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

	if ((mouse_move && GetTickCount() - m_dwInputTime > 10) || buttons_change || mouse_wheel) {
		SendInput(1, &_input, sizeof(INPUT));
		m_dwInputTime = GetTickCount();
	}
}

DWORD CMFCClientDlg::get_buttons_change(DWORD last_buttons_state, DWORD new_buttons_state,
	DWORD mask, DWORD down_flag, DWORD up_flag)
{
	DWORD ret = 0;
	if (!(last_buttons_state & mask) && (new_buttons_state & mask)) {
		ret = down_flag;
	}
	else if ((last_buttons_state & mask) && !(new_buttons_state & mask)) {
		ret = up_flag;
	}
	return ret;
}

void CMFCClientDlg::recv_keyboard_event_callback(unsigned int key_val, bool is_pressed)
{
	if (is_pressed) {
		keybd_event(key_val, 0, 0, 0);
	}
	else {
		keybd_event(key_val, 0, KEYEVENTF_KEYUP, 0);
	}
}

BOOL CMFCClientDlg::PreTranslateMessage(MSG* pMsg)
{
	if (m_bOperater) {
		if (pMsg->message == WM_KEYDOWN) {
			daemon_send_keyboard_event(pMsg->wParam, true);
			if (pMsg->wParam == VK_ESCAPE) {
				return TRUE;
			}
		}
		else if (pMsg->message == WM_KEYUP) {
			daemon_send_keyboard_event(pMsg->wParam, false);
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

void CMFCClientDlg::OnBnClickedSaveUrl()
{
	CButton* pBtn = (CButton*)GetDlgItem(IDC_CHECK1);
	int state = pBtn->GetCheck();
	if (state == 1) // 选中
	{
		FILE* fp = fopen("addr", "wb");
		char url[64] = { 0 };
		GetDlgItemText(IDC_EDIT1, url, sizeof(url));
		fwrite(url, 1, sizeof(url), fp);
		fclose(fp);
	}
	else {
		((CButton *)GetDlgItem(IDC_CHECK2))->SetCheck(0);
		remove("auto");
		remove("addr");
	}
}

void CMFCClientDlg::OnBnClickedAutoConnect()
{
	CButton* pBtn = (CButton*)GetDlgItem(IDC_CHECK2);
	int state = pBtn->GetCheck();
	if (state == 1) // 选中
	{
		FILE* fp = fopen("auto", "wb");
		fclose(fp);
	}
	else {
		remove("auto");
	}
}

void CMFCClientDlg::enable_sub_windows(BOOL enable)
{
	CWnd *pWnd = CWnd::FromHandle(m_hShowWnd);
	pWnd->GetDlgItem(IDC_STATIC)->EnableWindow(enable);
	pWnd->GetDlgItem(IDC_EDIT1)->EnableWindow(enable);
	pWnd->GetDlgItem(IDC_BUTTON2)->EnableWindow(enable);
	pWnd->GetDlgItem(IDC_BUTTON3)->EnableWindow(enable);
	pWnd->GetDlgItem(IDC_BUTTON4)->EnableWindow(enable);
	pWnd->GetDlgItem(IDC_BUTTON5)->EnableWindow(enable);
	pWnd->GetDlgItem(IDC_BUTTON6)->EnableWindow(enable);
	pWnd->GetDlgItem(IDC_BUTTON7)->EnableWindow(enable);
}

