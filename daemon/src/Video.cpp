#include <stdio.h>
#include <time.h>
#include "Video.h"
#include "libyuv.h"
#include "D3DRenderAPI.h"

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
				m_bPublisher(false),
				m_bResetSequence(false),
				m_bForceKeyframe(false),
				m_bLockScreen(false),
				m_iFrameRate(60)
{
	m_pEncoder = NULL;
	onEncoded = NULL;
	onLockScreen = NULL;
	m_pYUVData = NULL;
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
	m_hD3DHandle = NULL;
	m_pDecData[0] = NULL;
	m_pDecData[1] = NULL;
	m_pDecData[2] = NULL;
	m_pRenderData = new unsigned char[MAX_FRAME_WIDTH * MAX_FRAME_HEIGHT * 3 / 2];
	OpenDecoder();
}

Video::~Video()
{
	CloseEncoder();
	CloseDecoder();
#ifndef HW_DECODE
	delete[] m_pRenderData;
#endif
}

void Video::SetRenderWin(void* hWnd)
{
	m_hRenderWin = hWnd;
	m_bLockScreen = false;
}

bool Video::show(unsigned char* buffer, unsigned int len)
{
#ifdef HW_DECODE
	int status = 0;

	av_init_packet(&m_AVPacket);
	m_AVPacket.data = (BYTE*)buffer;
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

	DXVA2Render();
	return true;
#else
	SBufferInfo tDstInfo;
	DECODING_STATE state = m_pDecoder->DecodeFrameNoDelay(buffer, len, m_pDecData, &tDstInfo);
	if (state == 0) {
		Render(&tDstInfo);
		return true;
	}
	else {
		printf("decode frame failed 0x%x\n", state);
		return false;
	}
#endif
}

void Video::SetOnEncoded(onEncode_fp fp)
{
	onEncoded = fp;
}

void Video::SetOnLockScreen(onLockScreen_fp fp)
{
	onLockScreen = fp;
	m_bLockScreen = true;
}

void Video::start()
{
	m_bPublisher = true;
	cap_start_capture_screen(0, Video::onFrame, this);
	cap_set_drop_interval(25);
	cap_set_frame_rate(m_iFrameRate);
}

void Video::stop()
{
	m_bPublisher = false;
	cap_stop_capture_screen();
}

void Video::pause()
{
	cap_pause_capture_screen();
}

void Video::resume()
{
	cap_resume_capture_screen();
}

void Video::reset_keyframe(bool reset_ack)
{
	m_bForceKeyframe = true;
	if (reset_ack) {
		cap_reset_sequence();
	}
}

void Video::onFrame(CallbackFrameInfo* frame, void* param)
{
	Video* video = (Video*)param;

	if (video->m_bLockScreen) {
		if (video->onLockScreen) {
			video->onLockScreen((unsigned char*)frame->buffer, frame->length);
		}
		video->m_bLockScreen = false;
		return;
	}

	if (video->m_iFrameW != frame->width || video->m_iFrameH != frame->height || video->m_pYUVData == NULL) {
		printf("onFrame change size: seq %d w %d h %d\n", cap_get_capture_sequence(), frame->width, frame->height);
		video->CloseEncoder();
		if (video->m_pYUVData)
			delete[] video->m_pYUVData;
		video->m_pYUVData = new unsigned char[frame->width * frame->height * 3 / 2];
		video->OpenEncoder(frame->width, frame->height);
		cap_reset_sequence();
		return;
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
		int uv_stride = (video->m_iFrameW + 1) / 2;
		uint8_t* y = video->m_pYUVData;
		uint8_t* u = video->m_pYUVData + video->m_iFrameW * video->m_iFrameH;
		uint8_t* v = video->m_pYUVData + video->m_iFrameW * video->m_iFrameH + (video->m_iFrameH + 1) / 2 * uv_stride;
		libyuv::ARGBToI420((uint8_t*)frame->buffer, frame->line_stride, y, video->m_iFrameW, u, uv_stride, v, uv_stride, video->m_iFrameW, video->m_iFrameH);
		/*static int cnt = 0;
		if (cnt < 10) {
		m_pFile = fopen("yuv_data.yuv", "ab");
		fwrite(m_pYUVData, 1, video->m_iFrameW * video->m_iFrameH * 3 / 2, m_pFile);
		fclose(m_pFile);
		cnt++;
		}*/
	}
	video->Encode();
}

bool Video::OpenEncoder(int w, int h)
{
	m_iFrameW = w;
	m_iFrameH = h;
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
	sParam.fMaxFrameRate = 60;    // input frame rate
	sParam.iPicWidth = m_iFrameW;         // width of picture in samples
	sParam.iPicHeight = m_iFrameH;         // height of picture in samples
	sParam.iTargetBitrate = 1500000; // target bitrate desired
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
	sParam.iMinQp = 0;
	sParam.uiMaxNalSize = 0;
	int iIndexLayer = 0;
	sParam.sSpatialLayers[iIndexLayer].uiProfileIdc = PRO_BASELINE;
	sParam.sSpatialLayers[iIndexLayer].iVideoWidth = 160;
	sParam.sSpatialLayers[iIndexLayer].iVideoHeight = 90;
	sParam.sSpatialLayers[iIndexLayer].fFrameRate = 7.5f;
	sParam.sSpatialLayers[iIndexLayer].iSpatialBitrate = 64000;
	sParam.sSpatialLayers[iIndexLayer].iMaxSpatialBitrate = UNSPECIFIED_BIT_RATE;

	++iIndexLayer;
	sParam.sSpatialLayers[iIndexLayer].uiProfileIdc = PRO_SCALABLE_BASELINE;
	sParam.sSpatialLayers[iIndexLayer].iVideoWidth = 320;
	sParam.sSpatialLayers[iIndexLayer].iVideoHeight = 180;
	sParam.sSpatialLayers[iIndexLayer].fFrameRate = 15.0f;
	sParam.sSpatialLayers[iIndexLayer].iSpatialBitrate = 160000;
	sParam.sSpatialLayers[iIndexLayer].iMaxSpatialBitrate = UNSPECIFIED_BIT_RATE;

	++iIndexLayer;
	sParam.sSpatialLayers[iIndexLayer].uiProfileIdc = PRO_SCALABLE_BASELINE;
	sParam.sSpatialLayers[iIndexLayer].iVideoWidth = 640;
	sParam.sSpatialLayers[iIndexLayer].iVideoHeight = 360;
	sParam.sSpatialLayers[iIndexLayer].fFrameRate = 30.0f;
	sParam.sSpatialLayers[iIndexLayer].iSpatialBitrate = 512000;
	sParam.sSpatialLayers[iIndexLayer].iMaxSpatialBitrate = UNSPECIFIED_BIT_RATE;

	++iIndexLayer;
	sParam.sSpatialLayers[iIndexLayer].uiProfileIdc = PRO_SCALABLE_BASELINE;
	sParam.sSpatialLayers[iIndexLayer].iVideoWidth = 1280;
	sParam.sSpatialLayers[iIndexLayer].iVideoHeight = 720;
	sParam.sSpatialLayers[iIndexLayer].fFrameRate = 30.0f;
	sParam.sSpatialLayers[iIndexLayer].iSpatialBitrate = 1500000;
	sParam.sSpatialLayers[iIndexLayer].iMaxSpatialBitrate = UNSPECIFIED_BIT_RATE;

	//layer cfg
	SSpatialLayerConfig *pDLayer = &sParam.sSpatialLayers[0];
	pDLayer->iVideoWidth = m_iFrameW;
	pDLayer->iVideoHeight = m_iFrameH;
	pDLayer->fFrameRate = (float)m_iFrameRate;
	pDLayer->iSpatialBitrate = 1500000;
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

bool Video::OpenDecoder()
{
#ifdef HW_DECODE
	m_pAVDecoder = avcodec_find_decoder(AV_CODEC_ID_H264);
	if (m_pAVDecoder) {
		m_pAVDecoderContext = avcodec_alloc_context3(m_pAVDecoder);
	}
	if (m_pAVDecoderContext == NULL) {
		return false;
	}
	if (m_pAVDecoder->capabilities & AV_CODEC_CAP_TRUNCATED) {
		m_pAVDecoderContext->flags |= AV_CODEC_FLAG_TRUNCATED;
	}
	if (av_hwdevice_ctx_create(&m_Hwctx, AV_HWDEVICE_TYPE_DXVA2, NULL, NULL, 0) < 0) {
		avcodec_free_context(&m_pAVDecoderContext);
		return false;
	}
	m_pAVDecoderContext->hw_device_ctx = av_buffer_ref(m_Hwctx);
	m_pAVDecoderContext->get_format = get_hw_format;
	m_HwPixFmt = AV_PIX_FMT_DXVA2_VLD;
	m_pAVDecoderContext->opaque = this;

	if (avcodec_open2(m_pAVDecoderContext, m_pAVDecoder, NULL) < 0) {
		avcodec_free_context(&m_pAVDecoderContext);
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
	//RECT rcDst;
	//GetClientRect((HWND)m_hRenderWin, &rcDst);
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
	if (avVideoFrame->width != m_iFrameW || avVideoFrame->height != m_iFrameH) {
		m_iFrameW = avVideoFrame->width;
		m_iFrameH = avVideoFrame->height;
		if (m_hD3DHandle) {
			D3D_Release(&m_hD3DHandle);
		}
		D3D_Initial(&m_hD3DHandle, (HWND)m_hRenderWin, avVideoFrame->width, avVideoFrame->height, 0, 1, D3D_FORMAT_YV12);
	}
	RECT rcSrc = { 0, 0, avVideoFrame->width, avVideoFrame->height };
	D3D_UpdateData(m_hD3DHandle, 0, m_pRenderData, avVideoFrame->width, avVideoFrame->height, &rcSrc, NULL);
	RECT rcDst;
	GetClientRect((HWND)m_hRenderWin, &rcDst);
	D3D_Render(m_hD3DHandle, (HWND)m_hRenderWin, 1, &rcDst);
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

	int width = pInfo->UsrData.sSystemBuffer.iWidth;
	int height = pInfo->UsrData.sSystemBuffer.iHeight;
	WriteYUVBuffer(pInfo->UsrData.sSystemBuffer.iStride, width, height, 0);
	if (width != m_iFrameW || height != m_iFrameH) {
		m_iFrameW = width;
		m_iFrameH = height;
		if (m_hD3DHandle) {
			D3D_Release(&m_hD3DHandle);
		}
		D3D_Initial(&m_hD3DHandle, (HWND)m_hRenderWin, width, height, 0, 1, D3D_FORMAT_YV12);
	}
	RECT rcSrc = { 0, 0, width, height };
	D3D_UpdateData(m_hD3DHandle, 0, m_pRenderData, width, height, &rcSrc, NULL);
	RECT rcDst;
	GetClientRect((HWND)m_hRenderWin, &rcDst);
	D3D_Render(m_hD3DHandle, (HWND)m_hRenderWin, 1, &rcDst);
}
#endif

void Video::WriteBmpHeader(FILE* fp)
{
	if (fp == NULL || m_iFrameW == 0 || m_iFrameH == 0) {
		return;
	}

	unsigned char header[54] = {
	  0x42, 0x4d, 0, 0, 0, 0, 0, 0, 0, 0,
		54, 0, 0, 0, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 32, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0
	};

	long file_size = (long)m_iFrameW * (long)m_iFrameH * 4 + 54;
	header[2] = (unsigned char)(file_size & 0x000000ff);
	header[3] = (file_size >> 8) & 0x000000ff;
	header[4] = (file_size >> 16) & 0x000000ff;
	header[5] = (file_size >> 24) & 0x000000ff;

	long width = m_iFrameW;
	header[18] = width & 0x000000ff;
	header[19] = (width >> 8) & 0x000000ff;
	header[20] = (width >> 16) & 0x000000ff;
	header[21] = (width >> 24) & 0x000000ff;

	long height = -m_iFrameH;
	header[22] = height & 0x000000ff;
	header[23] = (height >> 8) & 0x000000ff;
	header[24] = (height >> 16) & 0x000000ff;
	header[25] = (height >> 24) & 0x000000ff;

	fwrite(header, sizeof(unsigned char), 54, fp);
}

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
	*width = m_iFrameW;
	*height = m_iFrameH;
}