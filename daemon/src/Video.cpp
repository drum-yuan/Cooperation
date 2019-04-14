#include <stdio.h>
#include <time.h>
#include "Video.h"
#include "libyuv.h"

Video::Video():m_iFrameW(0),
				m_iFrameH(0),
				m_bRunning(false),
				m_bResetSequence(false),
				m_bForceKeyframe(false),
				m_bLockScreen(false),
				m_bSendPic(false),
				m_iFrameRate(30)
{
	m_pEncoder = NULL;
	onEncoded = NULL;
	onLockScreen = NULL;
	m_pYUVData = NULL;
	m_pDecoder = NULL;
	m_hRenderWin = NULL;
	m_hD3DHandle = NULL;
	m_pDecData[0] = NULL;
	m_pDecData[1] = NULL;
	m_pDecData[2] = NULL;
	m_pRenderData = new unsigned char[4096 * 2160 * 3 / 2];
	OpenDecoder();
}

Video::~Video()
{
	CloseEncoder();
	CloseDecoder();
	delete[] m_pRenderData;
}

void Video::SetRenderWin(HWND hWnd)
{
	m_hRenderWin = hWnd;
	m_bSendPic = false;
	m_bLockScreen = false;
}

void Video::show(unsigned char* buffer, unsigned int len)
{
	SBufferInfo tDstInfo;
	DECODING_STATE state = m_pDecoder->DecodeFrame2(buffer, len, m_pDecData, &tDstInfo);
	if (state == 0) {
		Render(&tDstInfo);
	}
	else {
		printf("decode frame failed 0x%x\n", state);
	}
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
	cap_start_capture_screen(0, Video::onFrame, this);
	cap_set_drop_interval(25);
	cap_set_frame_rate(m_iFrameRate);
}

void Video::stop()
{
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

void Video::set_ackseq(unsigned int sequence)
{
	cap_set_ack_sequence(sequence);
}

void Video::onFrame(CallbackFrameInfo* frame, void* param)
{
	Video* video = (Video*)param;

	if (!video->m_bSendPic && video->m_bLockScreen) {
		if (video->onLockScreen) {
			video->onLockScreen((unsigned char*)frame->buffer, frame->length);
		}
		video->m_bSendPic = true;
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
	sParam.fMaxFrameRate = 30;    // input frame rate
	sParam.iPicWidth = m_iFrameW;         // width of picture in samples
	sParam.iPicHeight = m_iFrameH;         // height of picture in samples
	sParam.iTargetBitrate = 2500000; // target bitrate desired
	sParam.iMaxBitrate = UNSPECIFIED_BIT_RATE;
	sParam.iRCMode = RC_BITRATE_MODE;      //  rc mode control
	sParam.iTemporalLayerNum = 1;          // layer number at temporal level
	sParam.iSpatialLayerNum = 1;           // layer number at spatial level
	sParam.iEntropyCodingModeFlag = 0; 	//#0:cavlc 1:cabac;
	sParam.bEnableDenoise = 0;             // denoise control
	sParam.bEnableBackgroundDetection = 0; // background detection control
	sParam.bEnableAdaptiveQuant = 0;       // adaptive quantization control
	sParam.bEnableFrameSkip = 0;           // frame skipping
	sParam.bEnableLongTermReference = 0;   // long term reference control
	sParam.bEnableSceneChangeDetect = 1;
	sParam.iLtrMarkPeriod = 30;
	sParam.uiIntraPeriod = 320; // period of Intra frame
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
	pDLayer->iSpatialBitrate = 2500000;
	pDLayer->iMaxSpatialBitrate = 0;
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
	return true;
}

void Video::CloseDecoder()
{
	if (m_pDecoder)
	{
		WelsDestroyDecoder(m_pDecoder);
		m_pDecoder = NULL;
	}
}

void Video::WriteYUVBuffer(int iStride[2], int iWidth, int iHeight)
{
	int   i;
	unsigned char* pYUV = NULL;
	unsigned char* pData = NULL;

	pData = m_pRenderData;
	pYUV = m_pDecData[0];
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

void Video::Render(SBufferInfo* pInfo)
{
	if (pInfo == NULL || pInfo->iBufferStatus != 1 || m_hRenderWin == NULL) {
		return;
	}

	int width = pInfo->UsrData.sSystemBuffer.iWidth;
	int height = pInfo->UsrData.sSystemBuffer.iHeight;
	WriteYUVBuffer(pInfo->UsrData.sSystemBuffer.iStride, width, height);
	if (width != m_iFrameW || height != m_iFrameH) {
		m_iFrameW = width;
		m_iFrameH = height;
		if (m_hD3DHandle) {
			D3D_Release(&m_hD3DHandle);
		}
		D3D_Initial(&m_hD3DHandle, m_hRenderWin, width, height, 0, 1, D3D_FORMAT_YV12);
	}
	RECT rcSrc = { 0, 0, width, height };
	D3D_UpdateData(m_hD3DHandle, 0, m_pRenderData, width, height, &rcSrc, NULL);
	RECT rcDst;
	GetClientRect(m_hRenderWin, &rcDst);
	D3D_Render(m_hD3DHandle, m_hRenderWin, 1, &rcDst);
}

