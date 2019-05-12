
// MFCClientDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "MFCClient.h"
#include "MFCClientDlg.h"
#include "afxdialogex.h"
#include "DaemonApi.h"

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

static bool m_bOperater = false;
static HWND m_hShowWnd = NULL;
static DWORD m_dwButtonState = 0;
static DWORD m_dwInputTime = 0;
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
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEHWHEEL()
	ON_BN_CLICKED(IDC_BUTTON1, &CMFCClientDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CMFCClientDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CMFCClientDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CMFCClientDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CMFCClientDlg::OnBnClickedButton5)
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
	CDialogEx::OnClose();
}


void CMFCClientDlg::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bOperater) {
		if (point != m_poCursorPos) {
			m_poCursorPos = point;
			scale_to_screen(point);
			daemon_send_mouse_event(point.x, point.y, m_uButtonMask);
		}
	}

	CDialogEx::OnMouseMove(nFlags, point);
}

void CMFCClientDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (m_bOperater) {
		m_uButtonMask |= AGENT_LBUTTON_MASK;
		scale_to_screen(point);
		daemon_send_mouse_event(point.x, point.y, m_uButtonMask);
	}

	CDialogEx::OnLButtonDown(nFlags, point);
}

void CMFCClientDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bOperater) {
		m_uButtonMask &= ~AGENT_LBUTTON_MASK;
		scale_to_screen(point);
		daemon_send_mouse_event(point.x, point.y, m_uButtonMask);
	}

	CDialogEx::OnLButtonUp(nFlags, point);
}

void CMFCClientDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	if (m_bOperater) {
		m_uButtonMask |= AGENT_RBUTTON_MASK;
		scale_to_screen(point);
		daemon_send_mouse_event(point.x, point.y, m_uButtonMask);
	}

	CDialogEx::OnRButtonDown(nFlags, point);
}

void CMFCClientDlg::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (m_bOperater) {
		m_uButtonMask &= ~AGENT_RBUTTON_MASK;
		scale_to_screen(point);
		daemon_send_mouse_event(point.x, point.y, m_uButtonMask);
	}

	CDialogEx::OnRButtonUp(nFlags, point);
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

	CDialogEx::OnMouseWheel(nFlags, zDelta, point);
}


void CMFCClientDlg::OnBnClickedButton1()
{
	char url[64];
	GetDlgItemText(IDC_EDIT1, url, sizeof(url));
	daemon_start(url);
	m_hShowWnd = GetSafeHwnd();
	daemon_set_start_stream_callback(start_stream_callback);
	daemon_set_picture_callback(recv_picture_callback);
	daemon_set_operater_callback(start_operate_callback);
	daemon_set_mouse_callback(recv_mouse_event_callback);
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

void CMFCClientDlg::scale_to_screen(POINT& point)
{
	DWORD w = ::GetSystemMetrics(SM_CXSCREEN);
	DWORD h = ::GetSystemMetrics(SM_CYSCREEN);

	point.x = point.x * w / m_rectClient.Width();
	point.y = point.y * h / m_rectClient.Height();
}

void CMFCClientDlg::start_stream_callback()
{
	daemon_show_stream(m_hShowWnd);
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
	m_bOperater = is_operater;
}

void CMFCClientDlg::recv_mouse_event_callback(unsigned int x, unsigned int y, unsigned int button_mask)
{
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