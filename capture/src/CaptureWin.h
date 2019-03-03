#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <ddraw.h>
#include <d3d9.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#ifdef DMF_MIRROR
#include "DisplayEsc.h"
#endif
#include "IOCtl.h"
#include "CaptureApi.h"


#define GRAB_TYPE_AUTO      0
#define GRAB_TYPE_MIRROR    1
#define GRAB_TYPE_DIRECTX   2
#define GRAB_TYPE_GDI       3

struct dp_rect_t
{
	RECT   rc;             ///发生变化的矩形框

	char*  line_buffer;    ///矩形框数据起始地址
	int    line_bytes;     ///每行（矩形框width对应）的数据长度
	int    line_nextpos;   ///从0开始，第N行的数据地址: line_buffer + N*line_nextpos 。
	int    line_count;     ///等于矩形框高度 height
};

//镜像驱动抓屏
struct __mirror_cap_t
{
	BOOL             is_active;
	DISPLAY_DEVICE   disp;
	time_t           last_check_drawbuf_time;
#ifdef DMF_MIRROR
	GETCHANGESBUF    buffer;
#else
	draw_buffer_t*   buffer;
#endif
};
// DX抓屏
struct __d3d_cap_t
{
	int                 cx;
	int                 cy;
	int                 bitcount;
	int                 line_bytes;
	int                 line_stride;

	int                       is_acquire_frame;
	ID3D11Device*             d11dev;
	ID3D11DeviceContext*      d11ctx;
	IDXGIOutputDuplication*   dxgi_dup;
	DXGI_OUTPUT_DESC          dxgi_desc;
	ID3D11Texture2D*          dxgi_text2d;
	IDXGISurface*             dxgi_surf;

	byte*                     buffer;
};
///GDI 抓屏
struct __gdi_cap_t
{
	int        cx;
	int        cy;
	int        line_bytes;
	int        line_stride;
	int        bitcount;
	HDC        memdc;
	HBITMAP    hbmp;
	byte*      buffer;
};

struct __tbuf_t
{
	int    size;
	void*  buf;
};

class CCapture
{
public:
	CCapture(FrameCallback on_frame, void* param);
	~CCapture();
	void start();
	void stop();
	void pause();
	void resume();
	void set_drop_interval(unsigned int count);
	void set_ack_sequence(unsigned int seq);
	unsigned int get_capture_sequence();
	void reset_sequence();
	void set_frame_rate(unsigned int rate);

	static DWORD CALLBACK __loop_msg(void* _p);
	static LRESULT CALLBACK xDispWindowProc(HWND hwnd, UINT uMsg, WPARAM  wParam, LPARAM lParam);

private:
	void capture_mirror();
	void capture_dxgi();
	void capture_gdi();
	BOOL __init_mirror(BOOL is_init);
	BOOL __init_directx(BOOL is_init);
	BOOL __init_dxgi();
	BOOL __init_gdi(BOOL is_init);

	HWND CreateMsgWnd();
	void change_display(int w, int h, int bits);
#ifdef DMF_MIRROR
	void map_and_unmap_buffer(const char* dev_name, BOOL is_map, GETCHANGESBUF* p_buf);
#else
	void map_and_unmap_buffer(const char* dev_name, BOOL is_map, draw_buffer_t** p_buf);
#endif
	BOOL find_display_device(PDISPLAY_DEVICE disp, PDEVMODE mode, BOOL is_primary, BOOL is_mirror);
	BOOL active_mirror_driver(BOOL is_active, PDISPLAY_DEVICE dp);
	LPSTR GetDispCode(INT code);

	int              grab_type;   // 抓屏方法: 0 自动选择，1 mirror，2 DX抓屏，3 GDI抓屏

	BOOL             quit;
	BOOL             is_pause_grab;
	__mirror_cap_t   mirror;
	__d3d_cap_t      directx;
	__gdi_cap_t      gdi;
	long             sleep_msec;
	HWND             hMessageWnd;
	HANDLE           h_thread;
	DWORD            id_thread;
	HANDLE           hEvt;
	__tbuf_t         t_arr[2];

	unsigned int	 interval_count;
	unsigned int	 ack_seq;
	unsigned int	 capture_seq;
	FrameCallback	 onFrame;
	void*			 onframe_param;
};

#define USE_MIRROR_DIRTY_RECT   1   //使用mirror镜像驱动自己生成的脏矩形区域

#define USE_DXGI_DIRTY_RECT     1   //使用DXGI的 GetFrameDirtyRect获取更新的区域


//计算小矩形像素
#define MIRROR_SMALL_RECT_WIDTH    256
#define MIRROR_SMALL_RECT_HEIGHT   256

//小矩形块
#define GDI_SMALL_RECT_WIDTH    128
#define GDI_SMALL_RECT_HEIGHT   8

#define SAFE_RELEASE(a) if(a){ (a)->Release(); (a)=NULL; }
#define SAFE_FREE(a) if(a){ free(a); (a)=NULL; }

#ifdef DMF_MIRROR
#define MIRROR_DRIVER   "XieTong Mirror Display Driver"
#else
#define MIRROR_DRIVER   "XietongHF Mirror Display Driver"
#define VIRTUAL_DRIVER  "XietongHF Virtual Display Driver (XPDM for WinXP Only)"
#endif
