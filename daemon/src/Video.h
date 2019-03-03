#pragma once
#include "wels/codec_def.h"
#include "wels/codec_app_def.h"
#include "wels/codec_api.h"
#include "CaptureApi.h"
#include <functional>

typedef std::function<void(void* data)> onEncoded_fp;
class Video
{
public:
	Video();
	~Video();

	void setonEncoded(onEncoded_fp fp);
	void start();
	void stop();
	void pause();
	void resume();
	void reset_keyframe(bool reset_ack);
	static void onFrame(CallbackFrameInfo* frame, void* param);

private:
	bool initEncoder(int w, int h);
	void closeEncoder();
	void FillSpecificParameters(SEncParamExt &sParam);
	void Encode();

	unsigned char* m_pYUVData;
	int m_iFrameW;
	int m_iFrameH;

	bool m_bRunning;
	bool m_bResetSequence;
	bool m_bForceKeyframe;
	int m_iFrameRate;
	ISVCEncoder* m_pEncoder;
	onEncoded_fp onEncoded;
};