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
#ifdef HW_ENCODE
#include "NvFBCLibrary.h"
#include "NvFBC/NvFBCToDx9vid.h"
#include "NvEncoder.h"
#include <thread>
#else
#include "CaptureApi.h"
#include "wels/codec_def.h"
#include "wels/codec_app_def.h"
#include "wels/codec_api.h"
#endif
#include <functional>

#ifdef HW_ENCODE
#define MAX_FRAME_WIDTH		1920
#define MAX_FRAME_HEIGHT	1080
#else
#define MAX_FRAME_WIDTH		4096
#define MAX_FRAME_HEIGHT	2160
#endif
#define ENCODER_BITRATE		4000000

typedef std::function<void(void* data)> onEncode_fp;
typedef std::function<void(unsigned char* data, int len)> onLockScreen_fp;
class Video
{
public:
	Video();
	~Video();

	void SetRenderWin(void* hWnd);
	bool show(unsigned char* buffer, unsigned int len);

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
#ifdef HW_ENCODE
	void set_ack_seq(unsigned int seq);
	unsigned int get_capture_seq();
	unsigned int get_frame_type(NV_ENC_PIC_TYPE type);
	static void CaptureLoopProc(void* param);
#else
	static void onFrame(CallbackFrameInfo* frame, void* param);
#endif
#ifdef HW_DECODE
	static enum AVPixelFormat get_hw_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts);
#endif

private:
#ifdef HW_ENCODE
	void InitNvfbcEncoder();
	HRESULT InitD3D9(unsigned int deviceID);
	HRESULT InitD3D9Surfaces();
	void CleanupNvfbcEncoder();
#else
	void FillSpecificParameters(SEncParamExt &sParam);
	bool OpenEncoder(int w, int h);
	void CloseEncoder();
	void Encode();
#endif
	bool OpenDecoder();
	void CloseDecoder();
	void WriteYUVBuffer(int iStride[2], int iWidth, int iHeight, int iFormat);
#ifdef HW_DECODE
	void DXVA2Render();
#else
	void Render(SBufferInfo* pInfo);
#endif

#ifdef HW_ENCODE
	NvFBCFrameGrabInfo m_frameGrabInfo;
	NvFBCLibrary *m_pNVFBCLib;
	NvFBCToDx9Vid *m_pNvFBCDX9;
	IDirect3D9Ex *m_pD3DEx;
	IDirect3DDevice9Ex *m_pD3D9Device;
	IDirect3DSurface9 *m_apD3D9RGB8Surf[MAX_BUF_QUEUE];
	Encoder *m_pEncoder;
	std::thread *m_captureID;
	bool m_bQuit;
	bool m_bPause;
	unsigned long m_maxDisplayW;
	unsigned long m_maxDisplayH;
	unsigned int m_uCaptureSeq;
	unsigned int m_uAckSeq;
#else
	ISVCEncoder* m_pEncoder;
	onEncode_fp onEncoded;
	unsigned char* m_pYUVData;
#endif
	onLockScreen_fp onLockScreen;
	int m_iFrameW;
	int m_iFrameH;
	int m_iFrameRate;
	bool m_bPublisher;
	bool m_bOperater;
	bool m_bResetSequence;
	bool m_bForceKeyframe;
	bool m_bLockScreen;

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
	void* m_hD3DHandle;
	unsigned char* m_pDecData[3];
	unsigned char* m_pRenderData;
};