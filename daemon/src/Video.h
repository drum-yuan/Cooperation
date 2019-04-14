#pragma once
#include "wels/codec_def.h"
#include "wels/codec_app_def.h"
#include "wels/codec_api.h"
#include "CaptureApi.h"
#include "D3DRenderAPI.h"
#include <functional>

typedef std::function<void(void* data)> onEncode_fp;
typedef std::function<void(unsigned char* data, int len)> onLockScreen_fp;
class Video
{
public:
	Video();
	~Video();

	void SetRenderWin(HWND hWnd);
	bool show(unsigned char* buffer, unsigned int len);

	void SetOnEncoded(onEncode_fp fp);
	void SetOnLockScreen(onLockScreen_fp fp);
	void start();
	void stop();
	void pause();
	void resume();
	void reset_keyframe(bool reset_ack);
	void set_ackseq(unsigned int sequence);
	static void onFrame(CallbackFrameInfo* frame, void* param);

private:
	bool OpenEncoder(int w, int h);
	void CloseEncoder();
	void FillSpecificParameters(SEncParamExt &sParam);
	void Encode();

	bool OpenDecoder();
	void CloseDecoder();
	void WriteYUVBuffer(int iStride[2], int iWidth, int iHeight);
	void Render(SBufferInfo* pInfo);

	ISVCEncoder* m_pEncoder;
	onEncode_fp onEncoded;
	onLockScreen_fp onLockScreen;
	unsigned char* m_pYUVData;
	int m_iFrameW;
	int m_iFrameH;
	bool m_bRunning;
	bool m_bResetSequence;
	bool m_bForceKeyframe;
	bool m_bLockScreen;
	bool m_bSendPic;
	int m_iFrameRate;

	ISVCDecoder* m_pDecoder;
	HWND m_hRenderWin;
	D3D_HANDLE	m_hD3DHandle;
	unsigned char* m_pDecData[3];
	unsigned char* m_pRenderData;
};