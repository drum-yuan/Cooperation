#include "Audio.h"
#ifdef WIN32
#include "opus.h"
#include <wmcodecdsp.h>      // CLSID_CWMAudioAEC
// (must be before audioclient.h)
#include <Audioclient.h>     // WASAPI
#include <Audiopolicy.h>
#include <Mmdeviceapi.h>     // MMDevice
#include <endpointvolume.h>
#include <mediaobj.h>        // IMediaObject
#include <avrt.h>            // Avrt
#endif

#define AUDIO_CAPTURE_BUF_SIZE 1024
#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000

Audio::Audio() :m_bQuit(false), m_bPause(false)
{
	m_pOpusEncoder = NULL;
#ifdef WIN32
	m_nhnsActualDuration = 0;
	m_pAudioClient = NULL;
	m_pCaptureClient = NULL;
	m_pRenderClient = NULL;
#endif
	onEncoded = NULL;
	m_pAudioBuf = new unsigned char[AUDIO_CAPTURE_BUF_SIZE];
	m_pCaptureID = NULL;
}

Audio::~Audio()
{
	if (m_pAudioBuf) {
		delete m_pAudioBuf;
		m_pAudioBuf = NULL;
	}
}

void Audio::SetOnEncoded(onAudioEncode_fp fp)
{
	onEncoded = fp;
}

void Audio::start()
{
	m_bQuit = false;
	m_pCaptureID = new std::thread(&Audio::AudioCaptureThread, this);
}

void Audio::stop()
{
	m_bQuit = true;
	if (m_pCaptureID) {
		if (m_pCaptureID->joinable()) {
			m_pCaptureID->join();
			delete m_pCaptureID;
		}
		m_pCaptureID = NULL;
	}
}

void Audio::pause()
{
	m_bPause = true;
#ifdef WIN32
	m_pAudioClient->Stop();
#endif
}

void Audio::resume()
{
#ifdef WIN32
	m_pAudioClient->Start();
#endif
	m_bPause = true;
}

int Audio::InitCapture()
{
#ifdef WIN32
	int result;
	IMMDeviceEnumerator* pEnumerator = NULL;
	IMMDevice* pDevice = NULL;
	WAVEFORMATEX* pwfx = NULL;
	UINT32 bufferFrameCount;
	REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;

	m_pOpusEncoder = opus_encoder_create(16000, 1, OPUS_APPLICATION_VOIP, &result);

	if (CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator) < 0) {
		printf("CoCreateInstance failed");
		return -1;
	}

	if (pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice) < 0) {
		printf("GetDefaultAudioEndpoint failed");
		return -1;
	}

	if (pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&m_pAudioClient) < 0) {
		printf("Activate failed");
		return -1;
	}

	if (m_pAudioClient->GetMixFormat(&pwfx) < 0) {
		printf("GetMixFormat failed");
		return -1;
	}

	if (GetSupportFormat(pwfx) < 0) {
		printf("GetSupportFormat failed");
		return -1;
	}

	if (m_pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, hnsRequestedDuration, 0, pwfx, NULL) < 0) {
		printf("AudioClient Initialize failed");
		return -1;
	}

	if (m_pAudioClient->GetBufferSize(&bufferFrameCount) < 0) {
		printf("AudioClient GetBufferSize failed");
		return -1;
	}

	if (m_pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&m_pCaptureClient) < 0) {
		printf("AudioClient GetService failed");
		return -1;
	}

	if (m_pAudioClient->Start() < 0) {
		printf("AudioClient Start failed");
		return -1;
	}

	// Calculate the actual duration of the allocated buffer.
	m_nhnsActualDuration = (double)REFTIMES_PER_SEC * bufferFrameCount / pwfx->nSamplesPerSec;
#endif
	return 0;
}

int Audio::InitRender()
{
#ifdef WIN32

#endif
	return 0;
}

#ifdef WIN32
int Audio::GetSupportFormat(WAVEFORMATEX* pwfx)
{
	WAVEFORMATEX* pCLosetMatch = NULL;
	//opus encoder only support 16K sample rate
	pwfx->nChannels = 2;
	pwfx->wBitsPerSample = 32;
	pwfx->nSamplesPerSec = 16000;
	pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
	pwfx->nAvgBytesPerSec = pwfx->nSamplesPerSec * pwfx->nBlockAlign;

	return m_pAudioClient->IsFormatSupported(AUDCLNT_SHAREMODE_SHARED, pwfx, &pCLosetMatch);
}
#endif

void Audio::AudioCaptureThread()
{
#ifdef WIN32
	UINT32 numFramesAvailable;
	BYTE* pData;
	DWORD flags;
	UINT32 packetLength = 0;

	CoInitialize(NULL);

	// Start recording.
	if (InitCapture() < 0) {
		CoUninitialize();
		printf("audio init failed");
		return;
	}
	while (!m_bQuit)
	{
		Sleep(m_nhnsActualDuration / REFTIMES_PER_SEC / 2);
		if (m_bPause) {
			continue;
		}
		
		while (m_pCaptureClient->GetNextPacketSize(&packetLength) >= 0 && packetLength != 0)
		{
			if (m_pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL) < 0) {
				printf("get audio buffer failed");
				break;
			}
			if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
				pData = NULL;
			}
			if (pData) {
				memset(m_pAudioBuf, 0, AUDIO_CAPTURE_BUF_SIZE);
				float* p32BitPcmStereo = (float*)pData;
				short* p16BitPcmMono = new short[numFramesAvailable];

				for (unsigned int nIdx = 0; nIdx < numFramesAvailable; nIdx++) {
					float fSample = *p32BitPcmStereo;
					p16BitPcmMono[nIdx] = fSample * 32767.0;
					p32BitPcmStereo += 2;
				}

				opus_int32 nRetCode = opus_encode(m_pOpusEncoder, (opus_int16*)p16BitPcmMono, numFramesAvailable, m_pAudioBuf, AUDIO_CAPTURE_BUF_SIZE);
				if (nRetCode > 0) {
					onEncoded(m_pAudioBuf, nRetCode, numFramesAvailable);
				}
				delete[] p16BitPcmMono;
			}

			if (m_pCaptureClient->ReleaseBuffer(numFramesAvailable) < 0) {
				printf("release buffer failed");
				break;
			}
		}
		m_bQuit = true;
		if (m_pAudioClient->Stop() < 0) {
			printf("stop audio failed");
		}
		CoUninitialize();
		printf("stop audio capture thread");
	}
#endif
}
