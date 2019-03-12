#include "Protocol.h"
#include "Daemon.h"

#define DEFAULT_BUFFER_SIZE (1024 * 1024)

Daemon::Daemon()
{
	m_SendBuf = new Buffer(DEFAULT_BUFFER_SIZE);
	m_RecvBuf = new Buffer(DEFAULT_BUFFER_SIZE);
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

void Daemon::show_stream()
{
	m_Video.setonDecoded(std::bind(&Daemon::onVideoDecoded, this, std::placeholders::_1));
	m_PlayID = new thread(&Daemon::playStreamProc, this);
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
	header.version = 1;
	header.magic = 0;
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

void Daemon::playStreamProc(void* param)
{
	Daemon* daemon = (Daemon*)param;
	Buffer* buffer = daemon->m_RecvBuf;
	int ret = 0;
	while (ret >= 0) {
		ret = daemon->m_McuClient.recv_msg((unsigned char*)buffer->getbuf(), sizeof(WebSocketHeader));
		if (ret > 0) {
			WebSocketHeader* pWebHeader = (WebSocketHeader*)(buffer->getbuf());
			unsigned int nPayloadLen = Swap32IfLE(pWebHeader->length);
			ret = daemon->m_McuClient.recv_msg((unsigned char*)buffer->getbuf(), nPayloadLen);
			buffer->setdatapos(ret);
			VideoDataHeader* pVideoHeader = (VideoDataHeader*)(buffer->getbuf());
			unsigned int sequence = Swap32IfLE(pVideoHeader->sequence);
			buffer->popfront(sizeof(VideoDataHeader));
			daemon->m_Video.show((unsigned char*)buffer->getbuf(), buffer->getdatalength());
		}
		else {
			Sleep(10);
		}
	}
}

void Daemon::onVideoDecoded(void* data)
{
	//fullscreen render
}