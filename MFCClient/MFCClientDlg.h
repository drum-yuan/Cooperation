
// MFCClientDlg.h: 头文件
//

#pragma once


// CMFCClientDlg 对话框
class CMFCClientDlg : public CDialogEx
{
// 构造
public:
	CMFCClientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCCLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseWheel(UINT nFlags, short zDelta, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);

	afx_msg BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedSaveUrl();
	afx_msg void OnBnClickedAutoConnect();
	afx_msg LRESULT OnNotifyIcon(WPARAM wParam, LPARAM lParam);

	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButton7();

	void CreateNotifyIcon();

	static void start_stream_callback();
	static void stop_stream_callback();
	static void recv_picture_callback(const char* file_path);
	static void start_operate_callback(bool is_operater);
	static void recv_mouse_event_callback(unsigned int x, unsigned int y, unsigned int button_mask);
	static void recv_keyboard_event_callback(unsigned int key_val, bool is_pressed);
	static DWORD get_buttons_change(DWORD last_buttons_state, DWORD new_buttons_state,
		DWORD mask, DWORD down_flag, DWORD up_flag);
	static void enable_sub_windows(BOOL enable);

private:
	void scale_to_screen(POINT& point);

	NOTIFYICONDATA m_nd;
	POINT m_poCursorPos;
	CRect m_rectClient;
	unsigned int m_uButtonMask;
};
