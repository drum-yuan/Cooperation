#ifdef WIN32
#include <windows.h>
#include <mmeapi.h>
#endif
#include <thread>
#include <functional>

struct OpusEncoder;
#ifdef WIN32
struct IAudioClient;
struct IAudioCaptureClient;

#ifndef REFERENCE_TIME
typedef long long REFERENCE_TIME;
#endif
#endif

typedef std::function<void(unsigned char* data, int len, unsigned int frame_num)> onAudioEncode_fp;
class Audio
{
public:
	Audio();
	~Audio();

	void SetOnEncoded(onAudioEncode_fp fp);
	void start();
	void stop();
	void pause();
	void resume();

private:
	int InitCapture();
	int InitRender();
#ifdef WIN32
	int GetSupportFormat(WAVEFORMATEX* pwfx);
#endif
	void AudioCaptureThread();

	OpusEncoder* m_pOpusEncoder;
#ifdef WIN32
	REFERENCE_TIME m_nhnsActualDuration;
	IAudioClient* m_pAudioClient;
	IAudioCaptureClient* m_pCaptureClient;
	IAudioRenderClient* m_pRenderClient;
#endif

	bool m_bQuit;
	bool m_bPause;
	onAudioEncode_fp onEncoded;
	unsigned char* m_pAudioBuf;
	std::thread* m_pCaptureID;
};