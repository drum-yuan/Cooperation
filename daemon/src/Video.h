#pragma once

#include "CaptureApi.h"
#include "wels/codec_def.h"
#include "wels/codec_app_def.h"
#include "wels/codec_api.h"
#ifdef HW_DECODE
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}
#ifdef USE_D3D
#include <d3d9.h>
#endif
#endif
#include <functional>

#define MAX_FRAME_WIDTH		4096
#define MAX_FRAME_HEIGHT	2160

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
	void SetPublisher(bool is_Publisher);
	void start();
	void stop();
	void pause();
	void resume();
	void reset_keyframe(bool reset_ack);
	static void onFrame(CallbackFrameInfo* frame, void* param);
#ifdef HW_DECODE
	static enum AVPixelFormat get_hw_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts);
#endif

private:
	bool OpenEncoder(int w, int h);
	void CloseEncoder();
	void FillSpecificParameters(SEncParamExt &sParam);
	void Encode();

	bool OpenDecoder();
	void CloseDecoder();
	void WriteYUVBuffer(int iStride[2], int iWidth, int iHeight, int iFormat);
#ifdef HW_DECODE
	void DXVA2Render();
#else
	void Render(SBufferInfo* pInfo);
#endif

	ISVCEncoder* m_pEncoder;
	onEncode_fp onEncoded;
	onLockScreen_fp onLockScreen;
	unsigned char* m_pYUVData;
	int m_iFrameW;
	int m_iFrameH;
	bool m_bPublisher;
	bool m_bResetSequence;
	bool m_bForceKeyframe;
	bool m_bLockScreen;
	bool m_bSendPic;
	int m_iFrameRate;

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