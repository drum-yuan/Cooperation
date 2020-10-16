#pragma once

#ifdef HW_DECODE
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}
#ifdef USE_D3D
#include <d3d9.h>
#endif
#endif
#include "CaptureApi.h"
#include "wels/codec_def.h"
#include "wels/codec_app_def.h"
#include "wels/codec_api.h"
#ifdef WIN32
#include "NvFBCLibrary.h"
#include "NvFBC/NvFBCToDx9vid.h"
#include "NvEncoder.h"
#else
#include <X11/Xlib.h>
#include <X11/extensions/Xvlib.h>
#endif
#include <functional>
#include <thread>

#define MAX_FRAME_WIDTH		1920
#define MAX_FRAME_HEIGHT	1080

#define ENCODER_BITRATE		2000000

typedef std::function<void(void* data)> onEncode_fp;
typedef std::function<void(unsigned char* data, int len)> onLockScreen_fp;
class Video
{
public:
	Video();
	~Video();

	void SetRenderWin(void* hWnd);
	bool show(unsigned char* buffer, unsigned int len, bool is_show);

	void SetOnEncoded(onEncode_fp fp);
	void SetOnLockScreen(onLockScreen_fp fp);
	void WriteBmpHeader(FILE* fp);
	bool IsPublisher();
	void SetPublisher(bool is_publisher);
	bool IsOperater();
	void SetOperater(bool is_operater);
	void GetStreamSize(int* width, int* height);
	void start();
	void stop();
	void pause();
	void resume();
	void reset_keyframe(bool reset_ack);
	bool IsUseNvEnc();
#ifdef WIN32
	void set_ack_seq(unsigned int seq);
	unsigned int get_capture_seq();
	unsigned int get_frame_type(NV_ENC_PIC_TYPE type);
#endif
	void increase_encoder_bitrate(int delta_bitrate);
	static void onFrame(CallbackFrameInfo* frame, void* param);
#ifdef HW_DECODE
	static enum AVPixelFormat get_hw_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts);
#endif

private:
#ifdef WIN32
	void InitNvfbcEncoder();
	void CaptureLoopProc();
	HRESULT InitD3D9(unsigned int deviceID);
	HRESULT InitD3D9Surfaces();
	void CleanupNvfbcEncoder();
#else
	int XVAutoDetectPort();
#endif
	void FillSpecificParameters(SEncParamExt &sParam);
	bool OpenEncoder(int w, int h);
	void CloseEncoder();
	void Encode();
	bool OpenDecoder();
	void CloseDecoder();
	void WriteYUVBuffer(int iStride[2], int iWidth, int iHeight, int iFormat);

#ifdef HW_DECODE
	void DXVA2Render();
#else
	void Render(SBufferInfo* pInfo);
#endif

#ifdef WIN32
	NvFBCFrameGrabInfo m_frameGrabInfo;
	NvFBCLibrary *m_pNVFBCLib;
	NvFBCToDx9Vid *m_pNvFBCDX9;
	IDirect3D9Ex *m_pD3DEx;
	IDirect3DDevice9Ex *m_pD3D9Device;
	IDirect3DSurface9 *m_apD3D9RGB8Surf[MAX_BUF_QUEUE];
	Encoder *m_pEncoder;
	std::thread *m_pCaptureID;
	bool m_bQuit;
	bool m_bPause;
	unsigned long m_maxDisplayW;
	unsigned long m_maxDisplayH;
	unsigned int m_uCaptureSeq;
	unsigned int m_uAckSeq;
#endif
	ISVCEncoder* m_pSVCEncoder;
	onEncode_fp onEncoded;
	unsigned char* m_pYUVData;

	onLockScreen_fp onLockScreen;
	int m_iFrameW;
	int m_iFrameH;
	int m_iFrameRate;
	int m_iFrameInterval;
	int m_Bitrate;
	bool m_bPublisher;
	bool m_bOperater;
	bool m_bForceKeyframe;
	bool m_bLockScreen;
	bool m_bShow;
	bool m_bUseNvEnc;

	void* m_hRenderWin;
#ifdef HW_DECODE
	AVCodec* m_pAVDecoder;
	AVCodecContext* m_pAVDecoderContext;
	AVPacket m_AVPacket;
	AVBufferRef* m_Hwctx;
	AVFrame* m_AVVideoFrame;
	AVFrame* m_HwVideoFrame;
	enum AVPixelFormat m_HwPixFmt;
#ifdef USE_D3D
	CRITICAL_SECTION m_Cs;
#endif
#else
	ISVCDecoder* m_pDecoder;
#endif
#ifdef WIN32
	void* m_hD3DHandle;
#else
    Display* m_Display;
    Window m_Xwindow;
    GC m_XvGC;
    XvPortID m_XvPortID;
#endif
	unsigned char* m_pDecData[3];
	unsigned char* m_pRenderData;
};
