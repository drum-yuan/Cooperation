#include "Protocol.h"
#include "Daemon.h"

#define DEFAULT_SEND_BUF_SIZE (1024 * 1024)

Daemon::Daemon()
{
	m_SendBuf = new Buffer(DEFAULT_SEND_BUF_SIZE);
}

Daemon::~Daemon()
{

}

void Daemon::connect_mcu(const string& url)
{
	m_McuClient.connect(url, true, false);
}

void Daemon::connect_proxy(const string& url)
{
	m_ProxyClient.connect(url, true, false);
}

void Daemon::start_stream()
{
	m_Video.setonEncoded(std::bind(&Daemon::onVideoEncoded, this, std::placeholders::_1));
	m_Video.start();
}

void Daemon::stop_stream()
{
	m_Video.stop();
}

unsigned int Daemon::CalcFrameSize(void* data)
{
	int iLayer = 0;
	int iFrameSize = 0;

	if (!data)
		return 0;

	SFrameBSInfo* sFbi = (SFrameBSInfo*)data;

	while (iLayer < sFbi->iLayerNum) {
		SLayerBSInfo* pLayerBsInfo = &(sFbi->sLayerInfo[iLayer]);
		if (pLayerBsInfo != NULL) {
			int iLayerSize = 0;
			int iNalIdx = pLayerBsInfo->iNalCount - 1;
			do {
				iLayerSize += pLayerBsInfo->pNalLengthInByte[iNalIdx];
				--iNalIdx;
			} while (iNalIdx >= 0);

			if (iLayerSize)
			{
				iFrameSize += iLayerSize;
			}
		}
		++iLayer;
	}
	return iFrameSize;
}

void Daemon::onVideoEncoded(void* data)
{
	if (data == NULL)
		return;
	int iFrameSize = CalcFrameSize(data);
	if (!iFrameSize)
		return;

	WebSocketHeader header;
	header.major = CONST_ProtocolMajor;
	header.minor = CONST_ProtocolMinor;
	header.length = Swap32IfLE(sizeof(VideoDataHeader) + iFrameSize);
	header.type = Swap16IfLE(kMsgTypeVideoData);
	m_SendBuf->append((void*)&header, sizeof(WebSocketHeader));

	SFrameBSInfo* sFbi = (SFrameBSInfo*)data;
	VideoDataHeader videoheader;
	videoheader.eFrameType = Swap32IfLE(sFbi->eFrameType);
	unsigned int sequence = cap_get_capture_sequence();
	videoheader.sequence = Swap32IfLE(sequence);
	m_SendBuf->append((unsigned char*)&videoheader, sizeof(VideoDataHeader));

	int iLayer = 0;
	while (iLayer < sFbi->iLayerNum) {
		SLayerBSInfo* pLayerBsInfo = &(sFbi->sLayerInfo[iLayer]);
		if (pLayerBsInfo != NULL) {
			int iLayerSize = 0;
			int iNalIdx = pLayerBsInfo->iNalCount - 1;
			do {
				iLayerSize += pLayerBsInfo->pNalLengthInByte[iNalIdx];
				--iNalIdx;
			} while (iNalIdx >= 0);

			if (iLayerSize)
			{
				m_SendBuf->append((unsigned char*)pLayerBsInfo->pBsBuf, iLayerSize);
			}
		}
		++iLayer;
	}

	int ret = m_McuClient.send_msg((unsigned char*)m_SendBuf->getbuf(), m_SendBuf->getdatalength());
	if (ret < 0) {
		printf("send msg failed\n");
	}
	m_SendBuf->reset();
}