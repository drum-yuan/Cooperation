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
	RECT   rc;             ///�����仯�ľ��ο�

	char*  line_buffer;    ///���ο�������ʼ��ַ
	int    line_bytes;     ///ÿ�У����ο�width��Ӧ�������ݳ���
	int    line_nextpos;   ///��0��ʼ����N�е����ݵ�ַ: line_buffer + N*line_nextpos ��
	int    line_count;     ///���ھ��ο�߶� height
};

//��������ץ��
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
// DXץ��
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
//GDI ץ��
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

typedef HRESULT(WINAPI *fnD3D11CreateDevice)(
	_In_opt_ IDXGIAdapter* pAdapter,
	D3D_DRIVER_TYPE DriverType,
	HMODULE Software,
	UINT Flags,
	_In_reads_opt_(FeatureLevels) CONST D3D_FEATURE_LEVEL* pFeatureLevels,
	UINT FeatureLevels,
	UINT SDKVersion,
	_Out_opt_ ID3D11Device** ppDevice,
	_Out_opt_ D3D_FEATURE_LEVEL* pFeatureLevel,
	_Out_opt_ ID3D11DeviceContext** ppImmediateContext);

class CCapture
{
public:
	CCapture(int id, FrameCallback on_frame, void* param);
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
	BOOL __init_dxgi();
	BOOL __init_directx(BOOL is_init);
	BOOL __init_gdi(BOOL is_init);

	HWND CreateMsgWnd();
	void change_display(int w, int h, int bits);
#ifdef DMF_MIRROR
	void map_and_unmap_buffer(const char* dev_name, BOOL is_map, GETCHANGESBUF* p_buf);
#else
	void map_and_unmap_buffer(const char* dev_name, BOOL is_map, draw_buffer_t** p_buf);
#endif
	BOOL find_display_device(PDISPLAY_DEVICE disp, PDEVMODE mode, BOOL is_primary, int id);
	BOOL active_mirror_driver(BOOL is_active, PDISPLAY_DEVICE dp);
	LPSTR GetDispCode(INT code);

	int              grab_type;   // ץ������: 0 �Զ�ѡ��1 mirror��2 DXץ����3 GDIץ��
	int				 monitor_id;
	char			 disp_name[32];
	BOOL             quit;
	BOOL             pause_grab;
	__mirror_cap_t   mirror;
	__d3d_cap_t      directx;
	__gdi_cap_t      gdi;
	long             sleep_msec;
	HWND             hMessageWnd;
	HANDLE           h_thread;
	HANDLE           hEvt;
	__tbuf_t         t_arr[2];

	unsigned int	 interval_count;
	unsigned int	 ack_seq;
	unsigned int	 capture_seq;
	FrameCallback	 onFrame;
	void*			 onframe_param;

	LARGE_INTEGER	 counter;
	LARGE_INTEGER	 frame_begin;
	LARGE_INTEGER	 frame_end;
};

#define USE_MIRROR_DIRTY_RECT   1   //ʹ��mirror���������Լ����ɵ����������

#define USE_DXGI_DIRTY_RECT     1   //ʹ��DXGI�� GetFrameDirtyRect��ȡ���µ�����


//����С��������
#define MIRROR_SMALL_RECT_WIDTH    256
#define MIRROR_SMALL_RECT_HEIGHT   256

//С���ο�
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
