#ifdef WIN32
#include "FloatBar.h"
#include "util.h"
#include <stdio.h>

FloatBar::FloatBar(void* parent_win, Receiver* receiver)
{
	m_Parent = (HWND)parent_win;
	m_Hwnd = NULL;
	m_Shown = TRUE;
	m_HdcMem = NULL;
	m_Receiver = receiver;
}

FloatBar::~FloatBar()
{

}

void FloatBar::button_hit(Button* button)
{
	switch (button->type)
	{
		case BUTTON_CTRL:
		{
			int ins_id = m_Receiver->get_id_from_daemon_map(m_Parent);
			m_Receiver->start_operate(ins_id);
		}
			break;

		case BUTTON_MINIMIZE:
			ShowWindow(m_Parent, SW_MINIMIZE);
			break;

		case BUTTON_CLOSE:
			PostMessage(m_Parent, WM_CLOSE, 0 , 0);
			break;

		default:
			return;
	}
}

void FloatBar::button_paint(Button* button, HDC hdc)
{
	SelectObject(m_HdcMem, button->active ? button->bmp_act : button->bmp);
	StretchBlt(hdc, button->x, button->y, button->w, button->h, m_HdcMem, 0,
	           0, button->w, button->h, SRCCOPY);
}

Button* FloatBar::floatbar_create_button(int type, char* resid_path, char* resid_act_path, int x, int y, int h, int w)
{
	Button* button;
	button = (Button*)malloc(sizeof(Button));

	if (!button)
		return NULL;

	button->type = type;
	button->x = x;
	button->y = y;
	button->w = w;
	button->h = h;
	button->active = FALSE;
	button->bmp = (HBITMAP)LoadImage(GetModuleHandle(NULL), resid_path, IMAGE_BITMAP, w, h, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	button->bmp_act = (HBITMAP)LoadImage(GetModuleHandle(NULL), resid_act_path, IMAGE_BITMAP, w, h, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	return button;
}

Button* FloatBar::floatbar_get_button(int x, int y)
{
	int i;

	if (y > BUTTON_Y && y < BUTTON_Y + BUTTON_HEIGHT)
		for (i = 0; i < BTN_MAX; i++)
			if (x > m_Buttons[i]->x && x < m_Buttons[i]->x + m_Buttons[i]->w)
				return m_Buttons[i];

	return NULL;
}

void FloatBar::floatbar_paint(HDC hdc)
{
	int i;
	/* paint background */
	SelectObject(m_HdcMem, m_Background);
	StretchBlt(hdc, 0, 0, BACKGROUND_W, BACKGROUND_H, m_HdcMem, 0, 0,
	           BACKGROUND_W, BACKGROUND_H, SRCCOPY);

	/* paint buttons */
	for (i = 0; i < BTN_MAX; i++)
		button_paint(m_Buttons[i], hdc);
}

void FloatBar::floatbar_animation(BOOL show)
{
	SetTimer(m_Hwnd, show ? TIMER_ANIMAT_SHOW : TIMER_ANIMAT_HIDE, 10, NULL);
	m_Shown = show;
}

LRESULT CALLBACK FloatBar::floatbar_proc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	static int lbtn_dwn = FALSE;
	static int btn_dwn_x = 0;
	static FloatBar* floatbar = NULL;
	static TRACKMOUSEEVENT tme;
	PAINTSTRUCT ps;
	Button* button;
	HDC hdc;
	int pos_x;
	int pos_y;
	int xScreen = GetSystemMetrics(SM_CXSCREEN);

	switch (Msg)
	{
		case WM_CREATE:
			{
				floatbar = (FloatBar*)((CREATESTRUCT*)lParam)->lpCreateParams;
				floatbar->m_Hwnd = hWnd;
				floatbar->m_Parent = GetParent(hWnd);
				GetWindowRect(floatbar->m_Hwnd, &floatbar->m_Rect);
				hdc = GetDC(hWnd);
				floatbar->m_HdcMem = CreateCompatibleDC(hdc);
				ReleaseDC(hWnd, hdc);
				tme.cbSize = sizeof(TRACKMOUSEEVENT);
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = hWnd;
				tme.dwHoverTime = HOVER_DEFAULT;
				SetTimer(hWnd, TIMER_HIDE, 1000, NULL);
				break;
			}

		case WM_PAINT:
			{
				hdc = BeginPaint(hWnd, &ps);
				floatbar->floatbar_paint(hdc);
				EndPaint(hWnd, &ps);
				break;
			}

		case WM_LBUTTONDOWN:
			{
				pos_x = lParam & 0xffff;
				pos_y = (lParam >> 16) & 0xffff;
				button = floatbar->floatbar_get_button(pos_x, pos_y);

				if (button)
				{
					lbtn_dwn = TRUE;
				}

				break;
			}

		case WM_LBUTTONUP:
			{
				pos_x = lParam & 0xffff;
				pos_y = (lParam >> 16) & 0xffff;

				if (lbtn_dwn)
				{
					button = floatbar->floatbar_get_button(pos_x, pos_y);

					if (button)
						floatbar->button_hit(button);

					lbtn_dwn = FALSE;
				}

				break;
			}

		case WM_MOUSEMOVE:
			{
				KillTimer(hWnd, TIMER_HIDE);
				pos_x = lParam & 0xffff;
				pos_y = (lParam >> 16) & 0xffff;

				if (!floatbar->m_Shown) {
					LOG_INFO("floatbar show\n");
					PostMessage(floatbar->m_Parent, WM_USER + MSG_PAUSE_RENDER, 0, 0);
					floatbar->floatbar_animation(TRUE);
				}

				int i;
				for (i = 0; i < BTN_MAX; i++)
					floatbar->m_Buttons[i]->active = FALSE;

				button = floatbar->floatbar_get_button(pos_x, pos_y);

				if (button)
					button->active = TRUE;

				InvalidateRect(hWnd, NULL, FALSE);
				UpdateWindow(hWnd);

				TrackMouseEvent(&tme);
				break;
			}

		case WM_MOUSELEAVE:
			{
				int i;
				for (i = 0; i < BTN_MAX; i++)
					floatbar->m_Buttons[i]->active = FALSE;

				InvalidateRect(hWnd, NULL, FALSE);
				UpdateWindow(hWnd);
				SetTimer(hWnd, TIMER_HIDE, 10, NULL);
				break;
			}

		case WM_TIMER:
			{
				int width = floatbar->m_Rect.right - floatbar->m_Rect.left;
				int height = floatbar->m_Rect.bottom - floatbar->m_Rect.top;
				switch (wParam)
				{
				case TIMER_HIDE:
				{
					KillTimer(hWnd, TIMER_HIDE);
					floatbar->floatbar_animation(FALSE);
					break;
				}

				case TIMER_ANIMAT_SHOW:
				{
					MoveWindow(floatbar->m_Hwnd, floatbar->m_Rect.left, 0, width, height, TRUE);
					KillTimer(hWnd, wParam);

					break;
				}

				case TIMER_ANIMAT_HIDE:
				{	
					MoveWindow(floatbar->m_Hwnd, floatbar->m_Rect.left, -height + 1, width, height, TRUE);
					KillTimer(hWnd, wParam);
					LOG_INFO("resume render");
					PostMessage(floatbar->m_Parent, WM_USER + MSG_RESUME_RENDER, 0, 0);

					break;
				}

				default:
					break;
				}

				break;
			}

		case WM_DESTROY:
			{
				DeleteDC(floatbar->m_HdcMem);
				PostQuitMessage(0);
				break;
			}

		default:
			return DefWindowProc(hWnd, Msg, wParam, lParam);
	}

	return 0;
}

void FloatBar::floatbar_hide()
{
	KillTimer(m_Hwnd, TIMER_HIDE);
	int width = m_Rect.right - m_Rect.left;
	int height = m_Rect.bottom - m_Rect.top;
	MoveWindow(m_Hwnd, m_Rect.left, -height, width, height, TRUE);
}

void FloatBar::floatbar_show()
{
	SetTimer(m_Hwnd, TIMER_HIDE, 1000, NULL);
	int width = m_Rect.right - m_Rect.left;
	int height = m_Rect.bottom - m_Rect.top;
	MoveWindow(m_Hwnd, m_Rect.left, m_Rect.top, width, height, TRUE);
}

void FloatBar::floatbar_window_create()
{
	WNDCLASSEX wnd_cls;
	HWND barWnd;
	int x = (GetSystemMetrics(SM_CXSCREEN) - BACKGROUND_W) / 2;
	wnd_cls.cbSize        = sizeof(WNDCLASSEX);
	wnd_cls.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wnd_cls.lpfnWndProc   = floatbar_proc;
	wnd_cls.cbClsExtra    = 0;
	wnd_cls.cbWndExtra    = 0;
	wnd_cls.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	wnd_cls.hCursor       = LoadCursor(GetModuleHandle(NULL), IDC_ARROW);
	wnd_cls.hbrBackground = NULL;
	wnd_cls.lpszMenuName  = NULL;
	wnd_cls.lpszClassName = "floatbar";
	wnd_cls.hInstance     = GetModuleHandle(NULL);
	wnd_cls.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&wnd_cls);

	m_Background = (HBITMAP)LoadImage(GetModuleHandle(NULL), "resources/bg.bmp", IMAGE_BITMAP, BACKGROUND_W, BACKGROUND_H, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	m_Buttons[0] = floatbar_create_button(BUTTON_CTRL,
		(char*)"resources/ctrl.bmp", (char*)"resources/ctrl_active.bmp", CTRL_X, BUTTON_Y, BUTTON_HEIGHT, BUTTON_WIDTH);
	m_Buttons[1] = floatbar_create_button(BUTTON_MINIMIZE,
		(char*)"resources/minimize.bmp", (char*)"resources/minimize_active.bmp", MINIMIZE_X, BUTTON_Y, BUTTON_HEIGHT, BUTTON_WIDTH);
	m_Buttons[2] = floatbar_create_button(BUTTON_CLOSE,
		(char*)"resources/close.bmp", (char*)"resources/close_active.bmp", CLOSE_X, BUTTON_Y, BUTTON_HEIGHT, BUTTON_WIDTH);

	barWnd = CreateWindowEx(WS_EX_TOPMOST, "floatbar", "floatbar", WS_CHILD, x, 0,
	                        BACKGROUND_W, BACKGROUND_H, (HWND)m_Parent, NULL, GetModuleHandle(NULL), this);
	LOG_INFO("Create floatbar %p", barWnd);
	if (barWnd == NULL)
		return;
	ShowWindow(barWnd, SW_SHOWNORMAL);
}
#endif