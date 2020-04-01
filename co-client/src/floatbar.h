#pragma once
#ifdef WIN32
#include <windows.h>

/* TIMERs */
#define TIMER_HIDE          1
#define TIMER_ANIMAT_SHOW   2
#define TIMER_ANIMAT_HIDE   3

/* Button Type */
#define BUTTON_MINIMIZE     0
#define BUTTON_CLOSE        1
#define BTN_MAX             2

/* bmp size */
#define BACKGROUND_W        581
#define BACKGROUND_H        29
#define LOCK_X              13
#define MINIMIZE_X          (BACKGROUND_W - 91)
#define CLOSE_X             (BACKGROUND_W - 37)
#define RESTORE_X           (BACKGROUND_W - 64)

#define BUTTON_Y            2
#define BUTTON_WIDTH        24
#define BUTTON_HEIGHT       24

#define MSG_PAUSE_RENDER		1
#define MSG_RESUME_RENDER		2

typedef struct _Button
{
	int type;
	int x, y, h, w;
	int active;
	HBITMAP bmp;
	HBITMAP bmp_act;

	/* Lock Specified */
	HBITMAP locked_bmp;
	HBITMAP locked_bmp_act;
	HBITMAP unlocked_bmp;
	HBITMAP unlocked_bmp_act;
} Button;


class FloatBar
{
public:
	FloatBar(void* parent_win);
	~FloatBar();

	void floatbar_window_create();
	void floatbar_show();
	void floatbar_hide();

private:
	void button_hit(Button* button);
	void button_paint(Button* button, HDC hdc);
	Button* floatbar_create_button(int type, char* resid_path, char* resid_act_path, int x, int y, int h, int w);
	Button* floatbar_get_button(int x, int y);
	void floatbar_paint(HDC hdc);
	void floatbar_animation(BOOL show);

	static LRESULT CALLBACK floatbar_proc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

	HWND m_Parent;
	HWND m_Hwnd;
	RECT m_Rect;
	Button* m_Buttons[BTN_MAX];
	BOOL m_Shown;
	HDC m_HdcMem;
	HBITMAP m_Background;
};
#endif