#pragma once
#include "wels/codec_def.h"
#include "wels/codec_app_def.h"
#include "wels/codec_api.h"
#include "CaptureApi.h"
#include <functional>

typedef std::function<void(void* data)> onCodec_fp;
class Video
{
public:
	Video();
	~Video();

	void setonEncoded(onCodec_fp fp);
	void setonDecoded(onCodec_fp fp);
	void start();
	void stop();
	void show(unsigned char* buffer, unsigned int len);
	unsigned char* get_yuvdata();
	void pause();
	void resume();
	void reset_keyframe(bool reset_ack);
	static void onFrame(CallbackFrameInfo* frame, void* param);

private:
	bool initEncoder(int w, int h);
	bool initDecoder();
	void closeEncoder();
	void FillSpecificParameters(SEncParamExt &sParam);
	void Encode();
	void Decode(unsigned char* buffer, unsigned int len);

	unsigned char* m_pYUVData;
	int m_iFrameW;
	int m_iFrameH;

	bool m_bRunning;
	bool m_bResetSequence;
	bool m_bForceKeyframe;
	int m_iFrameRate;
	ISVCEncoder* m_pEncoder;
	ISVCDecoder* m_pDecoder;
	onCodec_fp onEncoded;
	onCodec_fp onDecoded;
};