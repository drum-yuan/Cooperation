#include <stdio.h>
#include <time.h>
#include <string.h>
#include "Video.h"
#include "libyuv.h"
#ifdef WIN32
#include "D3DRenderAPI.h"
#else
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#endif

#ifdef USE_D3D
typedef struct DXVA2DevicePriv {
	HMODULE d3dlib;
	HMODULE dxva2lib;

	HANDLE device_handle;

	IDirect3D9       *d3d9;
	IDirect3DDevice9 *d3d9device;
} DXVA2DevicePriv;
#endif

Video::Video():m_iFrameW(0),
				m_iFrameH(0),
				m_iFrameRate(30),
				m_Bitrate(ENCODER_BITRATE),
				m_bPublisher(false),
				m_bResetSequence(false),
				m_bForceKeyframe(false),
				m_bLockScreen(false),
				m_bShow(true)
{
#ifdef HW_ENCODE
	m_pNVFBCLib = NULL;
	m_pNvFBCDX9 = NULL;
	m_pD3DEx = NULL;
	m_pD3D9Device = NULL;
	for (int i = 0; i < MAX_BUF_QUEUE; i++)
	{
		m_apD3D9RGB8Surf[i] = NULL;
	}
	m_pEncoder = NULL;
	m_pCaptureID = NULL;
	m_bQuit = false;
	m_bPause = false;
	m_maxDisplayW = -1;
	m_maxDisplayH = -1;
	m_uCaptureSeq = 0;
	m_uAckSeq = 0;
	InitNvfbcEncoder();
#else
	m_pEncoder = NULL;
	onEncoded = NULL;
	m_pYUVData = NULL;
#endif
	onLockScreen = NULL;
	m_hRenderWin = NULL;
#ifdef HW_DECODE
	m_pAVDecoder = NULL;
	m_pAVDecoderContext = NULL;
	m_Hwctx = NULL;
	m_AVVideoFrame = NULL;
	m_HwVideoFrame = NULL;
	m_HwPixFmt = AV_PIX_FMT_NONE;
#ifdef USE_D3D
	InitializeCriticalSection(&m_Cs);
#endif
#else
	m_pDecoder = NULL;
#endif
#ifdef WIN32
	m_hD3DHandle = NULL;
#endif
	m_pDecData[0] = NULL;
	m_pDecData[1] = NULL;
	m_pDecData[2] = NULL;
	m_pRenderData = new unsigned char[MAX_FRAME_WIDTH * MAX_FRAME_HEIGHT * 3 / 2];
	OpenDecoder();
}

Video::~Video()
{
#ifdef HW_ENCODE
	CleanupNvfbcEncoder();
#else
	CloseEncoder();
#endif
	CloseDecoder();
	delete[] m_pRenderData;
}

void Video::SetRenderWin(void* hWnd)
{
	m_hRenderWin = hWnd;
	m_bLockScreen = false;
#ifndef WIN32
    if (m_hRenderWin != NULL) {
        printf("init Xv image\n");
        m_Display = GDK_WINDOW_XDISPLAY(gtk_widget_get_window((GtkWidget*)m_hRenderWin));
        m_Xwindow = GDK_WINDOW_XID(gtk_widget_get_window((GtkWidget*)m_hRenderWin));
        Drawable d = GDK_WINDOW_XID(gtk_widget_get_window((GtkWidget*)m_hRenderWin));
        m_XvGC = XCreateGC(m_Display, d, 0, NULL);
        printf("XVAutoDetectPort\n");
        XVAutoDetectPort();
    }
#endif
}

bool Video::show(unsigned char* buffer, unsigned int len, bool is_show)
{
#ifdef HW_DECODE
	int status = 0;

	av_init_packet(&m_AVPacket);
	m_AVPacket.data = (uint8_t*)buffer;
	m_AVPacket.size = len;
	status = avcodec_send_packet(m_pAVDecoderContext, &m_AVPacket);
	if (status < 0) {
		printf("avcodec send packet failed! %d\n", status);
		return false;
	}
	m_AVVideoFrame->format = AV_PIX_FMT_NV12;
	do {
		status = avcodec_receive_frame(m_pAVDecoderContext, m_Hwctx? m_HwVideoFrame : m_AVVideoFrame);
	} while (status == AVERROR(EAGAIN));
	if (status < 0) {
		printf("avcodec receive frame failed! %d\n", status);
		return false;
	}

	if (is_show && m_bShow) {
		DXVA2Render();
	}
	return true;
#else
	SBufferInfo tDstInfo;
	DECODING_STATE state = m_pDecoder->DecodeFrameNoDelay(buffer, len, m_pDecData, &tDstInfo);
	if (state == 0) {
		if (is_show && m_bShow) {
			Render(&tDstInfo);
		}
		return true;
	}
	else {
		printf("decode frame failed 0x%x\n", state);
		return false;
	}
#endif
}

void Video::yuv_show(unsigned char* buffer, int w, int h)
{
	if (buffer == NULL) {
		return;
	}
#ifdef WIN32
	if (m_hD3DHandle == NULL) {
		D3D_Initial(&m_hD3DHandle, (HWND)m_hRenderWin, w, h, 0, 1, D3D_FORMAT_YV12);
	}
	RECT rcSrc = { 0, 0, w, h };
	D3D_UpdateData(m_hD3DHandle, 0, buffer, w, h, &rcSrc, NULL);
	RECT rcDst;
	GetClientRect((HWND)m_hRenderWin, &rcDst);
	D3D_Render(m_hD3DHandle, (HWND)m_hRenderWin, 1, &rcDst);
#endif
}

void Video::SetOnEncoded(onEncode_fp fp)
{
#ifdef HW_ENCODE
	if (m_pEncoder) {
		m_pEncoder->SetOnEncoded(fp);
	}
#else
	onEncoded = fp;
#endif
}

void Video::SetOnLockScreen(onLockScreen_fp fp)
{
	onLockScreen = fp;
	m_bLockScreen = true;
	cap_reset_sequence();
}

void Video::start()
{
	m_bPublisher = true;
#ifdef HW_ENCODE
	m_bQuit = false;
	if (m_pNvFBCDX9) {
		m_pCaptureID = new std::thread(&Video::CaptureLoopProc, this);
	}
#else
	cap_start_capture_screen(1, Video::onFrame, this);
	cap_set_drop_interval(30);
	cap_set_frame_rate(m_iFrameRate);
#endif
}

void Video::stop()
{
	printf("video stop\n");
	m_bPublisher = false;
#ifdef HW_ENCODE
	m_bQuit = true;
	if (m_pCaptureID) {
		if (m_pCaptureID->joinable()) {
			m_pCaptureID->join();
			delete m_pCaptureID;
		}
		m_pCaptureID = NULL;
	}
#else
	cap_stop_capture_screen();
#endif
}

void Video::pause()
{
#ifdef HW_ENCODE
	m_bPause = true;
#else
	cap_pause_capture_screen();
#endif
	m_bShow = false;
}

void Video::resume()
{
#ifdef HW_ENCODE
	m_bPause = false;
#else
	cap_resume_capture_screen();
#endif
	m_bShow = true;
}

void Video::reset_keyframe(bool reset_ack)
{
	m_bForceKeyframe = true;
	if (reset_ack) {
#ifdef HW_ENCODE
		m_uAckSeq = m_uCaptureSeq;
#else
		cap_reset_sequence();
#endif
	}
}

#ifdef HW_ENCODE
void Video::set_ack_seq(unsigned int seq)
{
	if (seq > m_uAckSeq) {
		m_uAckSeq = seq;
	}
}

unsigned int Video::get_capture_seq()
{
	return m_uCaptureSeq;
}

unsigned int Video::get_frame_type(NV_ENC_PIC_TYPE type)
{
	unsigned int frame_type = 0;
	switch (type)
	{
	case NV_ENC_PIC_TYPE_P:
		frame_type = 3;
		break;
	case NV_ENC_PIC_TYPE_B:
		frame_type = 6;
		break;
	case NV_ENC_PIC_TYPE_I:
		frame_type = 2;
		break;
	case NV_ENC_PIC_TYPE_IDR:
		frame_type = 1;
		break;
	case NV_ENC_PIC_TYPE_SKIPPED:
		frame_type = 4;
		break;
	default:
		break;
	}
	return frame_type;
}

void Video::CaptureLoopProc(void* param)
{
	Video* video = (Video*)param;

	LARGE_INTEGER counter;
	LARGE_INTEGER frameBegin;
	LARGE_INTEGER frameEnd;
	QueryPerformanceFrequency(&counter);
	LONGLONG sleepMsec = 1000 / video->m_iFrameRate;

	NVFBC_TODX9VID_GRAB_FRAME_PARAMS fbcDX9GrabParams = { 0 };
	NVFBCRESULT fbcRes = NVFBC_ERROR_GENERIC;
	fbcDX9GrabParams.dwVersion = NVFBC_TODX9VID_GRAB_FRAME_PARAMS_V1_VER;
	fbcDX9GrabParams.dwFlags = NVFBC_TODX9VID_NOWAIT;
	fbcDX9GrabParams.eGMode = NVFBC_TODX9VID_SOURCEMODE_SCALE;
	fbcDX9GrabParams.dwTargetWidth = MAX_FRAME_WIDTH;
	fbcDX9GrabParams.dwTargetHeight = MAX_FRAME_HEIGHT;
	fbcDX9GrabParams.pNvFBCFrameGrabInfo = &(video->m_frameGrabInfo);

	while (!video->m_bQuit) {
		QueryPerformanceCounter(&frameBegin);
		if (video->m_bPause) {
			Sleep(50);
			continue;
		}
		if (video->m_uCaptureSeq - video->m_uAckSeq >= 25) {
			Sleep(1);
			continue;
		}
		unsigned int frameIndex = video->m_uCaptureSeq % MAX_BUF_QUEUE;
		if (video->m_pNvFBCDX9) {
			fbcRes = video->m_pNvFBCDX9->NvFBCToDx9VidGrabFrame(&fbcDX9GrabParams);
		}
		if (fbcRes == NVFBC_SUCCESS && video->m_pEncoder)
		{
			if (video->m_iFrameW == 0 && video->m_iFrameH == 0) {
				if (S_OK != video->m_pEncoder->SetupEncoder(MAX_FRAME_WIDTH, MAX_FRAME_HEIGHT, m_Bitrate,
					video->m_maxDisplayW, video->m_maxDisplayH, 0, video->m_apD3D9RGB8Surf, false, false, false, NULL, false, 48))
				{
					printf("Failed when calling Encoder::SetupEncoder()\n");
				}
				printf("Setup encoder\n");
				video->m_iFrameW = MAX_FRAME_WIDTH;
				video->m_iFrameH = MAX_FRAME_HEIGHT;
			}
			if ((video->m_iFrameW != video->m_frameGrabInfo.dwBufferWidth) || (video->m_iFrameH != video->m_frameGrabInfo.dwHeight) || video->m_bForceKeyframe)
			{
				printf("Encoder reset %d\n", video->m_bForceKeyframe);
				video->m_pEncoder->Reconfigure(video->m_frameGrabInfo.dwWidth, video->m_frameGrabInfo.dwHeight, m_Bitrate);
				video->m_iFrameW = video->m_frameGrabInfo.dwBufferWidth;
				video->m_iFrameH = video->m_frameGrabInfo.dwHeight;
				if (video->m_bForceKeyframe) {
					video->m_bForceKeyframe = false;
				}
			}

			if (video->m_bLockScreen) {
				if (video->onLockScreen) {
					D3DLOCKED_RECT lockedRect;
					RECT rect = { 0, 0, 1, 1 };
					if (FAILED(video->m_apD3D9RGB8Surf[frameIndex]->LockRect(&lockedRect, &rect, 0))) {
						printf("LockRect() failed.\n");
					}
					video->m_apD3D9RGB8Surf[frameIndex]->UnlockRect();

					if (FAILED(video->m_apD3D9RGB8Surf[frameIndex]->LockRect(&lockedRect, NULL, D3DLOCK_READONLY))) {
						printf("LockRect() D3DLOCK_READONLY failed.\n");
					}
					D3DSURFACE_DESC desc;
					video->m_apD3D9RGB8Surf[frameIndex]->GetDesc(&desc);
					unsigned char* pData = new unsigned char[video->m_iFrameW * video->m_iFrameH * 4];
					for (int i = 0; i < video->m_iFrameH; i++) {
						memcpy(pData + video->m_iFrameW * 4 * i, (unsigned char*)lockedRect.pBits + lockedRect.Pitch * i, video->m_iFrameW * 4);
					}
					video->m_apD3D9RGB8Surf[frameIndex]->UnlockRect();
					video->onLockScreen(pData, video->m_iFrameW * video->m_iFrameH * 4);
					delete[] pData;
				}
				video->m_bLockScreen = false;
				return;
			}

			HRESULT hr = E_FAIL;
			//! Start encoding
			hr = video->m_pEncoder->LaunchEncode(frameIndex);
			if (hr != S_OK)
			{
				printf("Failed encoding via LaunchEncode frame %d\n", video->m_uCaptureSeq);
				return;
			}

			//! Fetch encoded bitstream
			hr = video->m_pEncoder->GetBitstream(frameIndex);
			if (hr != S_OK)
			{
				printf("Failed encoding via GetBitstream frame %d\n", video->m_uCaptureSeq);
				return;
			}
			video->m_uCaptureSeq++;
			QueryPerformanceCounter(&frameEnd);
			LONGLONG dt = (frameEnd.QuadPart - frameBegin.QuadPart) * 1000 / counter.QuadPart;
			//printf("capture&encode cost %lld ms\n", dt);
			if (dt < sleepMsec) {
				Sleep(sleepMsec - dt);
			}
		}
		else {
			Sleep(1);
			printf("Grab frame failed\n");
		}
	}
	video->m_iFrameW = 0;
	video->m_iFrameH = 0;
	printf("CaptureLoopProc end\n");
}

void Video::InitNvfbcEncoder()
{
	m_pNVFBCLib = new NvFBCLibrary();
	m_pNVFBCLib->load();
	if (!SUCCEEDED(InitD3D9(0)))
	{
		printf("Unable to create a D3D9Ex Device\n");
		CleanupNvfbcEncoder();
		return;
	}
	if (!SUCCEEDED(InitD3D9Surfaces()))
	{
		printf("Unable to create a D3D9Ex Device\n");
		CleanupNvfbcEncoder();
		return;
	}
	m_pNvFBCDX9 = (NvFBCToDx9Vid *)m_pNVFBCLib->create(NVFBC_TO_DX9_VID, &m_maxDisplayW, &m_maxDisplayH, 0, (void *)m_pD3D9Device);
	if (!m_pNvFBCDX9)
	{
		printf("Failed to create an instance of NvFBCToDx9Vid.  Please check the following requirements\n");
		printf("1) A driver R355 or newer with capable Tesla/Quadro/GRID capable product\n");
		printf("2) Run \"NvFBCEnable -enable\" after a new driver installation\n");
		CleanupNvfbcEncoder();
		return;
	}
	printf("NvFBCToDX9Vid CreateEx() success!\n");

	m_pEncoder = new Encoder();
	if (S_OK != m_pEncoder->Init(m_pD3D9Device))
	{
		printf("Failed to initialize NVENC video encoder\n");
		CleanupNvfbcEncoder();
		return;
	}
	NVFBC_TODX9VID_OUT_BUF NvFBC_OutBuf[MAX_BUF_QUEUE];
	for (unsigned int i = 0; i < MAX_BUF_QUEUE; i++)
	{
		NvFBC_OutBuf[i].pPrimary = m_apD3D9RGB8Surf[i];
	}
	DWORD validClassMapSize = (int)(ceil((float)MAX_FRAME_WIDTH / NVFBC_TODX9VID_MIN_CLASSIFICATION_MAP_STAMP_DIM) * ceil((float)MAX_FRAME_HEIGHT / NVFBC_TODX9VID_MIN_CLASSIFICATION_MAP_STAMP_DIM));
	NVFBC_TODX9VID_SETUP_PARAMS DX9SetupParams = {};
	DX9SetupParams.dwVersion = NVFBC_TODX9VID_SETUP_PARAMS_V3_VER;
	DX9SetupParams.bWithHWCursor = 0;
	DX9SetupParams.bStereoGrab = 0;
	DX9SetupParams.bDiffMap = 0;
	DX9SetupParams.ppBuffer = NvFBC_OutBuf;
	DX9SetupParams.eMode = NVFBC_TODX9VID_ARGB;
	DX9SetupParams.dwNumBuffers = MAX_BUF_QUEUE;
	if (NVFBC_SUCCESS != m_pNvFBCDX9->NvFBCToDx9VidSetUp(&DX9SetupParams))
	{
		printf("Failed when calling NvFBCDX9->NvFBCToDX9VidSetup()\n");
		CleanupNvfbcEncoder();
		return;
	}
}

HRESULT Video::InitD3D9(unsigned int deviceID)
{
	HRESULT hr = S_OK;
	D3DPRESENT_PARAMETERS d3dpp;
	D3DADAPTER_IDENTIFIER9 adapterId;
	unsigned int iAdapter = NULL;

	Direct3DCreate9Ex(D3D_SDK_VERSION, &m_pD3DEx);
	if (deviceID >= m_pD3DEx->GetAdapterCount())
	{
		printf("Error:(deviceID=%d) is not a valid GPU device. Headless video devices will not be detected\n", deviceID);
		return E_FAIL;
	}
	hr = m_pD3DEx->GetAdapterIdentifier(deviceID, 0, &adapterId);
	if (hr != S_OK)
	{
		printf("Error:(deviceID=%d) is not a valid GPU device\n", deviceID);
		return E_FAIL;
	}

	// Create the Direct3D9 device and the swap chain. In this example, the swap
	// chain is the same size as the current display mode. The format is RGB-32.
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = true;
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;

	d3dpp.BackBufferWidth = MAX_FRAME_WIDTH;
	d3dpp.BackBufferHeight = MAX_FRAME_HEIGHT;
	d3dpp.BackBufferCount = 1;
	d3dpp.SwapEffect = D3DSWAPEFFECT_COPY;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	d3dpp.Flags = D3DPRESENTFLAG_VIDEO;
	DWORD dwBehaviorFlags = D3DCREATE_FPU_PRESERVE | D3DCREATE_MULTITHREADED | D3DCREATE_HARDWARE_VERTEXPROCESSING;
	hr = m_pD3DEx->CreateDeviceEx(
		deviceID,
		D3DDEVTYPE_HAL,
		NULL,
		dwBehaviorFlags,
		&d3dpp,
		NULL,
		&m_pD3D9Device);

	return hr;
}

HRESULT Video::InitD3D9Surfaces()
{
	HRESULT hr = E_FAIL;
	if (m_pD3D9Device) {
		for (int i = 0; i < MAX_BUF_QUEUE; i++)
		{
			hr = m_pD3D9Device->CreateOffscreenPlainSurface(MAX_FRAME_WIDTH, MAX_FRAME_HEIGHT, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_apD3D9RGB8Surf[i], NULL);
			if (FAILED(hr))
			{
				printf("Failed to create D3D9 surfaces D3DFMT_A8R8G8B8 for output. Error 0x%08x\n", hr);
			}
		}
	}
	return S_OK;
}

void Video::CleanupNvfbcEncoder()
{
	if (m_pEncoder)
	{
		m_pEncoder->TearDown();
		delete m_pEncoder;
		m_pEncoder = NULL;
	}

	if (m_pNvFBCDX9)
	{
		m_pNvFBCDX9->NvFBCToDx9VidRelease();
		m_pNvFBCDX9 = NULL;
	}

	for (int i = 0; i < MAX_BUF_QUEUE; i++)
	{
		if (m_apD3D9RGB8Surf[i])
		{
			m_apD3D9RGB8Surf[i]->Release();
			m_apD3D9RGB8Surf[i] = NULL;
		}
	}

	if (m_pD3D9Device)
	{
		m_pD3D9Device->Release();
		m_pD3D9Device = NULL;
	}

	if (m_pD3DEx)
	{
		m_pD3DEx->Release();
		m_pD3DEx = NULL;
	}

	if (m_pNVFBCLib)
	{
		m_pNVFBCLib->close();
		delete m_pNVFBCLib;
		m_pNVFBCLib = NULL;
	}
}

#else
void Video::increase_encoder_bitrate(int delta_bitrate)
{
	if (m_pEncoder && ((m_Bitrate > 500000 && delta_bitrate < 0) || (m_Bitrate < 4000000 && delta_bitrate > 0))) {
		SBitrateInfo info;
		info.iLayer = SPATIAL_LAYER_ALL;
		info.iBitrate = m_Bitrate + delta_bitrate;
		m_pEncoder->SetOption(ENCODER_OPTION_BITRATE, &info);
	}
}

void Video::onFrame(CallbackFrameInfo* frame, void* param)
{
	Video* video = (Video*)param;

	bool IsSizeChanging = false;
	if (video->m_iFrameW != frame->width || video->m_iFrameH != frame->height || video->m_pYUVData == NULL) {
		printf("onFrame change size: seq %d w %d h %d\n", cap_get_capture_sequence(), frame->width, frame->height);
		video->CloseEncoder();
		if (video->m_pYUVData)
			delete[] video->m_pYUVData;
		video->m_pYUVData = new unsigned char[frame->width * frame->height * 3 / 2];
		video->OpenEncoder(frame->width, frame->height);
		cap_reset_sequence();
		IsSizeChanging = true;
	}

	if (frame->bitcount == 8) {
		printf("capture 8bit\n");
		return;
	}
	else if (frame->bitcount == 16) {
		printf("capture 16bit\n");
	}
	else if (frame->bitcount == 24) {
		//libyuv::RGB24ToI420();
		printf("capture 24bit\n");
	}
	else if (frame->bitcount == 32) {
		//printf("capture 32bit len %d\n", frame->length);
		int uv_stride = video->m_iFrameW / 2;
		uint8_t* y = video->m_pYUVData;
		uint8_t* u = video->m_pYUVData + video->m_iFrameW * video->m_iFrameH;
		uint8_t* v = video->m_pYUVData + video->m_iFrameW * video->m_iFrameH + video->m_iFrameH / 2 * uv_stride;
		libyuv::ARGBToI420((uint8_t*)frame->buffer, frame->line_stride, y, video->m_iFrameW, u, uv_stride, v, uv_stride, video->m_iFrameW, video->m_iFrameH);
		/*static int cnt = 0;
		if (cnt < 10) {
		FILE* fp = fopen("yuv_data.yuv", "ab");
		fwrite(video->m_pYUVData, 1, video->m_iFrameW * video->m_iFrameH * 3 / 2, fp);
		fclose(fp);
		cnt++;
		}*/
	}
	if (video->m_bLockScreen && !IsSizeChanging) {
		if (video->onLockScreen) {
			video->onLockScreen(video->m_pYUVData, video->m_iFrameW * video->m_iFrameH * 3 / 2);
		}
		video->m_bLockScreen = false;
	}
	else {
		video->Encode();
	}
}

bool Video::OpenEncoder(int w, int h)
{
	m_iFrameW = w;
	m_iFrameH = h;
	printf("OpenEncoder w=%d h=%d\n", m_iFrameW, m_iFrameH);
	if (!m_pEncoder)
	{
		if (WelsCreateSVCEncoder(&m_pEncoder) || (NULL == m_pEncoder))
		{
			printf("Create encoder failed, this = %p", this);
			return false;
		}

		SEncParamExt tSvcParam;
		m_pEncoder->GetDefaultParams(&tSvcParam);
		FillSpecificParameters(tSvcParam);
		int nRet = m_pEncoder->InitializeExt(&tSvcParam);
		if (nRet != 0)
		{
			printf("init encoder failed, this = %p, error = %d", this, nRet);
			return false;
		}
	}
	return true;
}

void Video::CloseEncoder()
{
	if (m_pEncoder)
	{
		WelsDestroySVCEncoder(m_pEncoder);
		m_pEncoder = NULL;
	}
}

void Video::FillSpecificParameters(SEncParamExt &sParam)
{
	sParam.iUsageType = SCREEN_CONTENT_REAL_TIME;
	sParam.fMaxFrameRate = 30;    // input frame rate
	sParam.iPicWidth = m_iFrameW;         // width of picture in samples
	sParam.iPicHeight = m_iFrameH;         // height of picture in samples
	sParam.iTargetBitrate = m_Bitrate; // target bitrate desired
	sParam.iMaxBitrate = UNSPECIFIED_BIT_RATE;
	sParam.iRCMode = RC_BITRATE_MODE;      //  rc mode control
	sParam.iTemporalLayerNum = 1;          // layer number at temporal level
	sParam.iSpatialLayerNum = 1;           // layer number at spatial level
	sParam.iEntropyCodingModeFlag = 0; 	//#0:cavlc 1:cabac;
	sParam.bEnableDenoise = 0;             // denoise control
	sParam.bEnableBackgroundDetection = 0; // background detection control
	sParam.bEnableAdaptiveQuant = 0;       // adaptive quantization control
	sParam.bEnableFrameSkip = 1;           // frame skipping
	sParam.bEnableLongTermReference = 0;   // long term reference control
	sParam.iLtrMarkPeriod = 30;
	//sParam.uiIntraPeriod = 320; // period of Intra frame
	sParam.eSpsPpsIdStrategy = INCREASING_ID;
	sParam.bPrefixNalAddingCtrl = 0;
	sParam.iComplexityMode = LOW_COMPLEXITY;
	sParam.bSimulcastAVC = false;
	sParam.iMaxQp = 48;
	sParam.iMinQp = 24;
	sParam.uiMaxNalSize = 0;

	//layer cfg
	SSpatialLayerConfig *pDLayer = &sParam.sSpatialLayers[0];
	pDLayer->iVideoWidth = m_iFrameW;
	pDLayer->iVideoHeight = m_iFrameH;
	pDLayer->fFrameRate = (float)m_iFrameRate;
	pDLayer->iSpatialBitrate = m_Bitrate;
	pDLayer->iMaxSpatialBitrate = UNSPECIFIED_BIT_RATE;
}

void Video::Encode()
{
	if (m_bForceKeyframe) {
		m_pEncoder->ForceIntraFrame(true);
		m_bForceKeyframe = false;
	}

	if (!m_pEncoder)
		return;
	SSourcePicture tSrcPic;

	unsigned int uTimeStamp = clock();
	tSrcPic.uiTimeStamp = uTimeStamp;
	tSrcPic.iColorFormat = videoFormatI420;
	tSrcPic.iStride[0] = m_iFrameW;
	tSrcPic.iStride[1] = tSrcPic.iStride[0] >> 1;
	tSrcPic.iStride[2] = tSrcPic.iStride[1];
	tSrcPic.iPicWidth = m_iFrameW;
	tSrcPic.iPicHeight = m_iFrameH;

	tSrcPic.pData[0] = m_pYUVData;
	tSrcPic.pData[1] = tSrcPic.pData[0] + (m_iFrameW * m_iFrameH);
	tSrcPic.pData[2] = tSrcPic.pData[1] + (m_iFrameW * m_iFrameH / 4);

	SFrameBSInfo tFbi;
	int iEncFrames = m_pEncoder->EncodeFrame(&tSrcPic, &tFbi);
	if (iEncFrames == cmResultSuccess)
	{
		tFbi.uiTimeStamp = uTimeStamp;
		if (onEncoded) {
			onEncoded(&tFbi);
		}
	}
	else {
		printf("encodeVideo failed\n");
	}
}
#endif

bool Video::OpenDecoder()
{
#ifdef HW_DECODE
	m_pAVDecoder = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (m_pAVDecoder) {
		m_pAVDecoderContext = avcodec_alloc_context3(m_pAVDecoder);
	}
	if (m_pAVDecoderContext == NULL) {
        printf("alloc context failed\n");
		return false;
	}
	if (m_pAVDecoder->capabilities & AV_CODEC_CAP_TRUNCATED) {
		m_pAVDecoderContext->flags |= AV_CODEC_FLAG_TRUNCATED;
	}
#ifdef WIN32
	if (av_hwdevice_ctx_create(&m_Hwctx, AV_HWDEVICE_TYPE_DXVA2, NULL, NULL, 0) < 0) {
#else
	if (av_hwdevice_ctx_create(&m_Hwctx, AV_HWDEVICE_TYPE_VAAPI, NULL, NULL, 0) < 0) {
#endif
		avcodec_free_context(&m_pAVDecoderContext);
        printf("create hwdevice ctx failed\n");
		return false;
	}
	m_pAVDecoderContext->hw_device_ctx = av_buffer_ref(m_Hwctx);
	m_pAVDecoderContext->get_format = get_hw_format;
#ifdef WIN32
	m_HwPixFmt = AV_PIX_FMT_DXVA2_VLD;
#else
    m_HwPixFmt = AV_PIX_FMT_VAAPI;
#endif
	m_pAVDecoderContext->opaque = this;

	if (avcodec_open2(m_pAVDecoderContext, m_pAVDecoder, NULL) < 0) {
		avcodec_free_context(&m_pAVDecoderContext);
		printf("avcodec open failed\n");
		return false;
	}
	m_AVVideoFrame = av_frame_alloc();
	m_AVVideoFrame->pts = 0;
	m_HwVideoFrame = av_frame_alloc();
#else
	if (!m_pDecoder)
	{
		if (WelsCreateDecoder(&m_pDecoder) || (NULL == m_pDecoder))
		{
			printf("Create decoder failed, this = %p", this);
			return false;
		}
		SDecodingParam tParam;
		memset(&tParam, 0, sizeof(SDecodingParam));
		tParam.uiTargetDqLayer = UCHAR_MAX;
		tParam.eEcActiveIdc = ERROR_CON_FRAME_COPY_CROSS_IDR;
		tParam.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_DEFAULT;
		int nRet = m_pDecoder->Initialize(&tParam);
		if (nRet != 0)
		{
			printf("init decoder failed, this = %p, error = %d", this, nRet);
			return false;
		}
	}
#endif
	return true;
}

void Video::CloseDecoder()
{
#ifdef HW_DECODE
	if (m_HwVideoFrame) {
		av_frame_free(&m_HwVideoFrame);
	}
	if (m_Hwctx) {
		av_buffer_unref(&m_Hwctx);
	}
	if (m_pAVDecoderContext) {
		avcodec_close(m_pAVDecoderContext);
		avcodec_free_context(&m_pAVDecoderContext);
	}
#else
	if (m_pDecoder)
	{
		WelsDestroyDecoder(m_pDecoder);
		m_pDecoder = NULL;
	}
#endif
}

void Video::WriteYUVBuffer(int iStride[2], int iWidth, int iHeight, int iFormat)
{
	int   i, j;
	unsigned char* pYUV = NULL;
	unsigned char* pData = NULL;
	unsigned char* pData2 = NULL;

	pData = m_pRenderData;
	pYUV = m_pDecData[0];
	if (iFormat == 0) {
		for (i = 0; i < iHeight; i++) {
			memcpy(pData, pYUV, iWidth);
			pYUV += iStride[0];
			pData += iWidth;
		}
		pYUV = m_pDecData[1];
		for (i = 0; i < iHeight / 2; i++) {
			memcpy(pData, pYUV, iWidth / 2);
			pYUV += iStride[1];
			pData += iWidth / 2;
		}
		pYUV = m_pDecData[2];
		for (i = 0; i < iHeight / 2; i++) {
			memcpy(pData, pYUV, iWidth / 2);
			pYUV += iStride[1];
			pData += iWidth / 2;
		}
	}
	else if (iFormat == 1) {
		for (i = 0; i < iHeight; i++) {
			memcpy(pData, pYUV, iWidth);
			pYUV += iStride[0];
			pData += iWidth;
		}
		pYUV = m_pDecData[1];
		pData2 = pData + (iWidth * iHeight) / 4;
		for (i = 0; i < iHeight / 2; i++) {
			for (j = 0; j < iWidth; j++) {
				if (j % 2 == 0) {
					*pData = *(pYUV + j);
					pData++;
				}
				else {
					*pData2 = *(pYUV + j);
					pData2++;
				}
			}
			pYUV += iStride[1];
		}
	}
}

#ifdef HW_DECODE
void Video::DXVA2Render()
{
	if (m_hRenderWin == NULL) {
		return;
	}
	AVFrame* avVideoFrame = m_AVVideoFrame;
	if (!m_Hwctx) {
		goto COMMON_RENDER;
	}

#ifdef USE_D3D
	IDirect3DSurface9* backBuffer = NULL;
	LPDIRECT3DSURFACE9 surface = (LPDIRECT3DSURFACE9)m_HwVideoFrame->data[3];
	AVHWDeviceContext* ctx = (AVHWDeviceContext*)m_Hwctx->data;
	DXVA2DevicePriv* priv = (DXVA2DevicePriv*)ctx->user_opaque;
	HRESULT ret = 0;

	m_iFrameW = m_HwVideoFrame->width;
	m_iFrameH = m_HwVideoFrame->height;

	EnterCriticalSection(&m_Cs);
	ret = priv->d3d9device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	if (ret < 0) {
		printf("Clear failed\n");
	}
	ret = priv->d3d9device->BeginScene();
	if (ret < 0) {
		printf("BeginScene failed\n");
	}
	ret = priv->d3d9device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backBuffer);
	if (ret < 0) {
		printf("GetBackBuffer failed\n");
	}
	//RECT rcRender;
	//GetClientRect((HWND)m_hRenderWin, &rcRender);
	ret = priv->d3d9device->StretchRect(surface, NULL, backBuffer, NULL, D3DTEXF_LINEAR);
	if (ret < 0) {
		printf("StretchRect failed\n");
	}
	ret = priv->d3d9device->EndScene();
	if (ret < 0) {
		printf("EndScene failed\n");
	}
	ret = priv->d3d9device->Present(NULL, NULL, (HWND)m_hRenderWin, NULL);
	if (ret < 0) {
		printf("Present failed\n");
	}
	LeaveCriticalSection(&m_Cs);
	backBuffer->Release();
	return;
#else
	if (m_HwVideoFrame->format == m_HwPixFmt) {
		m_AVVideoFrame->width = m_HwVideoFrame->width;
		m_AVVideoFrame->height = m_HwVideoFrame->height;
		printf("transfer data from gpu to cpu %d-%d\n", m_AVVideoFrame->width, m_AVVideoFrame->height);
		if (av_hwframe_transfer_data(m_AVVideoFrame, m_HwVideoFrame, 0) < 0) {
			printf("transfer data failed!\n");
			return;
		}
	}
	else {
		avVideoFrame = m_HwVideoFrame;
	}
#endif

COMMON_RENDER:
	int iStride[2];
	if (avVideoFrame->format == AV_PIX_FMT_YUV420P) {
		m_pDecData[0] = avVideoFrame->data[0];
		m_pDecData[1] = avVideoFrame->data[1];
		m_pDecData[2] = avVideoFrame->data[2];
		iStride[0] = avVideoFrame->linesize[0];
		iStride[1] = avVideoFrame->linesize[1];
	}
	else if (avVideoFrame->format == AV_PIX_FMT_NV12) {
		m_pDecData[0] = avVideoFrame->data[0];
		m_pDecData[1] = avVideoFrame->data[1];
		iStride[0] = avVideoFrame->linesize[0];
		iStride[1] = avVideoFrame->linesize[1];
	}
	WriteYUVBuffer(iStride, avVideoFrame->width, avVideoFrame->height, 1);
	/*static int cnt = 0;
	if (cnt < 10) {
	FILE* fp = fopen("render_data.yuv", "ab");
	fwrite(m_pRenderData, 1, width * height * 3 / 2, fp);
	fclose(fp);
	cnt++;
	}*/
	int width = avVideoFrame->width;
	int height = avVideoFrame->height;
#ifdef WIN32
	if (width != m_iFrameW || height != m_iFrameH) {
		m_iFrameW = width;
		m_iFrameH = height;
		if (m_hD3DHandle) {
			D3D_Release(&m_hD3DHandle);
		}
		D3D_Initial(&m_hD3DHandle, (HWND)m_hRenderWin, m_iFrameW, m_iFrameH, 0, 1, D3D_FORMAT_YV12);
	}
	RECT rcSrc = { 0, 0, m_iFrameW, m_iFrameH };
	D3D_UpdateData(m_hD3DHandle, 0, m_pRenderData, m_iFrameW, m_iFrameH, &rcSrc, NULL);
	RECT rcDst;
	GetClientRect((HWND)m_hRenderWin, &rcDst);
	D3D_Render(m_hD3DHandle, (HWND)m_hRenderWin, 1, &rcDst);
#else
	if (width != m_iFrameW || height != m_iFrameH) {
		m_iFrameW = width;
		m_iFrameH = height;
	}
    XvImage *xv_image = XvCreateImage(m_Display, m_XvPortID, 0x30323449, (char*)m_pRenderData, width, height);
    int dst_w = 0;
    int dst_h = 0;
    gtk_widget_get_size_request((GtkWidget*)m_hRenderWin, &dst_w, &dst_h);
    XvPutImage(m_Display, m_XvPortID, m_Xwindow, m_XvGC, xv_image, 0, 0, width, height, 0, 0, dst_w, dst_h);
    XFree(xv_image);
#endif
}

enum AVPixelFormat Video::get_hw_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts)
{
	Video* pVideo = (Video*)ctx->opaque;
	const enum AVPixelFormat *p;
	for (p = pix_fmts; *p != -1; p++) {
		if (*p == pVideo->m_HwPixFmt)
			return *p;
	}

	printf("Failed to get HW surface format.\n");
	return AV_PIX_FMT_NONE;
}

#else
void Video::Render(SBufferInfo* pInfo)
{
	if (pInfo == NULL || pInfo->iBufferStatus != 1 || m_hRenderWin == NULL) {
		return;
	}

	WriteYUVBuffer(pInfo->UsrData.sSystemBuffer.iStride, pInfo->UsrData.sSystemBuffer.iWidth, pInfo->UsrData.sSystemBuffer.iHeight, 0);
	int width = pInfo->UsrData.sSystemBuffer.iWidth;
	int height = pInfo->UsrData.sSystemBuffer.iHeight;
#ifdef WIN32
	if (width != m_iFrameW || height != m_iFrameH) {
		m_iFrameW = width;
		m_iFrameH = height;
		if (m_hD3DHandle) {
			D3D_Release(&m_hD3DHandle);
		}
		D3D_Initial(&m_hD3DHandle, (HWND)m_hRenderWin, m_iFrameW, m_iFrameH, 0, 1, D3D_FORMAT_YV12);
	}
	RECT rcSrc = { 0, 0, m_iFrameW, m_iFrameH };
	D3D_UpdateData(m_hD3DHandle, 0, m_pRenderData, m_iFrameW, m_iFrameH, &rcSrc, NULL);
	RECT rcDst;
	GetClientRect((HWND)m_hRenderWin, &rcDst);
	D3D_Render(m_hD3DHandle, (HWND)m_hRenderWin, 1, &rcDst);
#else
	if (width != m_iFrameW || height != m_iFrameH) {
		m_iFrameW = width;
		m_iFrameH = height;
	}
    XvImage *xv_image = XvCreateImage(m_Display, m_XvPortID, 0x30323449, (char*)m_pRenderData, width, height);
    int dst_w = 0;
    int dst_h = 0;
    gtk_widget_get_size_request((GtkWidget*)m_hRenderWin, &dst_w, &dst_h);
    XvPutImage(m_Display, m_XvPortID, m_Xwindow, m_XvGC, xv_image, 0, 0, width, height, 0, 0, dst_w, dst_h);
    XFree(xv_image);
#endif
}
#endif

void Video::WriteBmpHeader(FILE* fp)
{
	if (fp == NULL || m_iFrameW == 0 || m_iFrameH == 0) {
		return;
	}

	int width = m_iFrameW;
	int height = m_iFrameH;
	unsigned char header[54] = {
	  0x42, 0x4d, 0, 0, 0, 0, 0, 0, 0, 0,
		54, 0, 0, 0, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 32, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0
	};

	long file_size = (long)width * (long)height * 4 + 54;
	header[2] = (unsigned char)(file_size & 0x000000ff);
	header[3] = (file_size >> 8) & 0x000000ff;
	header[4] = (file_size >> 16) & 0x000000ff;
	header[5] = (file_size >> 24) & 0x000000ff;

	header[18] = width & 0x000000ff;
	header[19] = (width >> 8) & 0x000000ff;
	header[20] = (width >> 16) & 0x000000ff;
	header[21] = (width >> 24) & 0x000000ff;

	height = -height;
	header[22] = height & 0x000000ff;
	header[23] = (height >> 8) & 0x000000ff;
	header[24] = (height >> 16) & 0x000000ff;
	header[25] = (height >> 24) & 0x000000ff;

	fwrite(header, sizeof(unsigned char), 54, fp);
}

#ifndef WIN32
int Video::XVAutoDetectPort()
{
    int ret = Success;
	unsigned int i, j, k;
	unsigned int ver,rel,req,ev,err;
	XvAdaptorInfo* adaptor_info = NULL;
	unsigned int adaptor_num;
	XvImageFormatValues *formatValues;
	int formats;
	XvPortID port;

	XvQueryExtension(m_Display, &ver, &rel, &req, &ev, &err);
	if (ret != Success)
	{
	    printf("XvQueryExtension failed,xv is not present.\n");
	    goto OnError;
	}
	printf("xv version %u,release %u,request_base %u,event_base %u,error_base=%u\n", ver,rel,req,ev,err);
	ret = XvQueryAdaptors(m_Display, m_Xwindow, &adaptor_num, &adaptor_info);
	if (ret != Success)
	{
	    printf("XvQueryAdaptors failed.\n");
	    goto OnError;
	}
	for (i = 0; i < adaptor_num; i++)
	{
	    for (j = 0; j < adaptor_info[i].num_ports; j++)
	    {
            port = adaptor_info[i].base_id + j;
	        formatValues = XvListImageFormats(m_Display, port, &formats);
            for (k = 0; k < formats; k++)
            {
                if (formatValues[k].type == XvYUV && (strcmp(formatValues[k].guid, "I420") == 0) &&
                    XvGrabPort(m_Display, port, 0) == Success)
                {
                    m_XvPortID = port;
                    goto OnError;
                }
            }
	    }
	}
	ret = -1;
    printf("Grab port failed\n");
OnError:
	if (adaptor_info)
        XvFreeAdaptorInfo(adaptor_info);
	return ret;
}
#endif

bool Video::IsPublisher()
{
	return m_bPublisher;
}

void Video::SetPublisher(bool is_publisher)
{
	m_bPublisher = is_publisher;
}

bool Video::IsOperater()
{
	return m_bOperater;
}

void Video::SetOperater(bool is_operater)
{
	m_bOperater = is_operater;
}

void Video::GetStreamSize(int* width, int* height)
{
	if (m_iFrameW > 0 && m_iFrameH > 0) {
		*width = m_iFrameW;
		*height = m_iFrameH;
	}
}
