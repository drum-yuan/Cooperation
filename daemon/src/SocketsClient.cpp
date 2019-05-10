#include "SocketsClient.h"
#include "Protocol.h"
#include "proto.hpp"
#include "autojsoncxx/autojsoncxx.hpp"

#define WEBSOCKET_MAX_BUFFER_SIZE (1024*1024)
#define CONNECT_TIMEOUT 5000  //ms

using namespace MCUProtocol;

enum ClientConnState {
	ConnectStateUnConnected = 0,
	ConnectStateConnecting = 1,
	ConnectStateConnectError = 2,
	ConnectStateEstablished = 3,
	ConnectStateClosed = 4
};

SocketsClient::SocketsClient() : m_Exit(false),
								m_State(ConnectStateUnConnected),
								m_UseSSL(false),
								m_PicFile(NULL),
								m_wsi(NULL),
								m_wsthread(NULL),
								m_protocols(NULL),
								m_exts(NULL)
{
	m_protocols = (struct lws_protocols*)malloc(sizeof(struct lws_protocols) * 2);
	if (m_protocols)
	{
		memset(m_protocols, 0, sizeof(struct lws_protocols) * 2);
		m_protocols[0].name = "binary";
		m_protocols[0].callback = SocketsClient::callback_client;
		m_protocols[0].per_session_data_size = sizeof(SocketsClient*);
		m_protocols[0].rx_buffer_size = WEBSOCKET_MAX_BUFFER_SIZE;
		m_protocols[0].user = this;

		m_protocols[1].name = NULL;
		m_protocols[1].callback = NULL;
		m_protocols[1].per_session_data_size = 0;
		m_protocols[1].rx_buffer_size = 0;
		m_protocols[1].user = NULL;
	}

	m_exts = (struct lws_extension*)malloc(sizeof(struct lws_extension));
	if (m_exts)
	{
		memset(m_exts, 0, sizeof(struct lws_extension));
		m_exts[0].name = NULL;
		m_exts[0].callback = NULL;
		m_exts[0].client_offer = NULL;
	}

	m_SendBuf = new Buffer(WEBSOCKET_MAX_BUFFER_SIZE);
	m_CallbackPicture = NULL;
	m_CallbackOperater = NULL;
	memset(m_FilePath, 0, sizeof(m_FilePath));
}

SocketsClient::~SocketsClient()
{
	if (m_protocols)
	{
		free(m_protocols);
		m_protocols = NULL;
	}

	if (m_exts)
	{
		free(m_exts);
		m_exts = NULL;
	}

	delete m_SendBuf;
}

void SocketsClient::NotifyForceExit()
{
	m_Exit = true;
}

void SocketsClient::SetConnectState(int connstate)
{
	m_State = connstate;
	m_cv.notify_all();
}

int SocketsClient::RunWebSocketClient()
{
	int n = 0;
	int port = 0;
	int use_ssl = 0;
	const char *prot, *p, *address;
	struct lws_context *context;
	struct lws_context_creation_info info;
	char uri[256] = "/";
	char ads_port[256 + 30];
	const char *connect_protocol = "binary";
	struct lws_client_connect_info i;
	int debug_level = 7;

	memset(&info, 0, sizeof info);
	if (m_UseSSL)
		use_ssl = 2;

	if (!m_protocols)
		return -1;

	/* tell the library what debug level to emit and to send it to syslog */
	lws_set_log_level(debug_level, lwsl_emit_syslog);

	if (lws_parse_uri((char*)m_ServerUrl.c_str(), &prot, &address, &port, &p))
		return -1;

	uri[0] = '/';
	strncpy(uri + 1, p, sizeof(uri) - 2);
	uri[sizeof(uri) - 1] = '\0';
	i.path = uri;

	info.port = CONTEXT_PORT_NO_LISTEN;
	info.iface = NULL;
	info.protocols = m_protocols;
	info.keepalive_timeout = PENDING_TIMEOUT_HTTP_KEEPALIVE_IDLE;

	if (use_ssl) {
		info.ssl_cert_filepath = NULL;
		info.ssl_private_key_filepath = NULL;
		info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
	}

	info.gid = -1;
	info.uid = -1;
	info.options |= LWS_SERVER_OPTION_VALIDATE_UTF8;

#ifndef LWS_NO_EXTENSIONS
	info.extensions = m_exts;
#endif

	context = lws_create_context(&info);
	if (context == NULL) {
		lwsl_err("libwebsocket init failed\n");
		return -1;
	}

	n = 0;
	while (n >= 0 && !m_Exit) {
		if (!m_State) {
			m_State = ConnectStateConnecting;
			lwsl_notice("Client connecting to %s:%u....\n", address, port);

			sprintf(ads_port, "%s:%u", address, port & 65535);
			memset(&i, 0, sizeof(i));
			i.context = context;
			i.address = address;
			i.port = port;
			i.ssl_connection = use_ssl;
			i.path = uri;
			i.host = ads_port;
			i.origin = ads_port;
			i.protocol = connect_protocol;
			i.client_exts = m_exts;
			i.userdata = this;

			m_wsi = lws_client_connect_via_info(&i);
			if (!m_wsi) {
				lwsl_err("Client failed to connect to %s:%u\n",
					address, port);
				goto fail;
			}
		}
		lws_callback_on_writable_all_protocol(context, &m_protocols[0]);
		n = lws_service(context, 20);
	}
fail:
	lws_context_destroy(context);
	lwsl_notice("websocket client exited cleanly\n");

	return 0;
}

void SocketsClient::set_video_event(Video* pEvent)
{
	m_pVideo = pEvent;
}

bool SocketsClient::connect(std::string url, bool blocking, bool ssl)
{
	m_ServerUrl = url;
	m_UseSSL = ssl;
	//reset();
	m_wsthread = new std::thread(&SocketsClient::RunWebSocketClient, this);

	if (blocking && m_State != ConnectStateEstablished)
	{
		std::mutex mtx;
		std::unique_lock<std::mutex> lck(mtx);
		while (m_cv.wait_for(lck, std::chrono::milliseconds(CONNECT_TIMEOUT)) == std::cv_status::timeout) {
			printf("connect timeout\n");
			NotifyForceExit();
			return false;
		}
		if (m_State == ConnectStateClosed || m_State == ConnectStateConnectError)
		{
			printf("connect failed\n");
			return false;
		}
	}

	return true;
}

void SocketsClient::reset()
{
	m_Exit = true;
	if (m_wsthread)
	{
		if (m_wsthread->joinable())
		{
			m_wsthread->join();
		}
		delete m_wsthread;
		m_wsthread = NULL;
	}
	m_wsi = NULL;
	m_State = ConnectStateUnConnected;
	m_Exit = false;
}

void SocketsClient::stop()
{
	reset();
}

bool SocketsClient::is_connected()
{
	return m_State == ConnectStateEstablished;
}

int SocketsClient::send_msg(unsigned char* payload, unsigned int msglen)
{
	if (m_State != ConnectStateEstablished)
	{
		return -1;
	}

	int ret = lws_write(m_wsi, payload, msglen, LWS_WRITE_BINARY);
	if (ret < 0) {
		printf("send msg error\n");
	}
	return ret;
}

void SocketsClient::set_start_stream_callback(StartStreamCallback on_stream)
{
	m_CallbackStream = on_stream;
}

void SocketsClient::set_picture_callback(PictureCallback on_recv)
{
	m_CallbackPicture = on_recv;
}

void SocketsClient::set_operater_callback(OperaterCallback on_operater)
{
	m_CallbackOperater = on_operater;
}

void SocketsClient::set_mouse_callback(MouseCallback on_mouse)
{
	m_CallbackMouse = on_mouse;
}

void SocketsClient::set_keyboard_callback(KeyboardCallback on_keyboard)
{
	m_CallbackKeyboard = on_keyboard;
}

void SocketsClient::handle_in(struct lws *wsi, const void* in, size_t len)
{
	WebSocketHeader* pWebHeader = (WebSocketHeader*)in;
	unsigned char uMagic = pWebHeader->magic;
	unsigned short uType = Swap16IfLE(pWebHeader->type);
	unsigned int uPayloadLen = Swap32IfLE(pWebHeader->length);

	printf("handle in type %u\n", uType);
	switch (uType)
	{
	case kMsgTypePublishAck:
	{
		if (m_pVideo && m_pVideo->IsPublisher()) {
			m_pVideo->SetPublisher(false);
			reset();
			m_pVideo->stop();
		}
		if (m_CallbackStream) {
			m_CallbackStream();
		}
	}
		break;
	case kMsgTypeVideoData:
	{
		VideoDataHeader* pVideoHeader = (VideoDataHeader*)((uint8_t*)in + sizeof(WebSocketHeader));
		unsigned int sequence = Swap32IfLE(pVideoHeader->sequence);
		unsigned int len = uPayloadLen - sizeof(VideoDataHeader);
		if (m_pVideo) {
			if (!(m_pVideo->show((uint8_t*)in + sizeof(WebSocketHeader) + sizeof(VideoDataHeader), len))) {
				send_keyframe_request(false);
			}
		}
		send_video_ack(sequence);
	}
		break;
	case kMsgTypeVideoAck:
	{
		VideoAck_C2S tVideoAck;
		const char* pPayload = (const char*)in + sizeof(WebSocketHeader);
		autojsoncxx::ParsingResult result;
		if (!autojsoncxx::from_json_string(pPayload, tVideoAck, result)) {
			printf("parser json string fail\n");
			break;
		}
		unsigned int sequence = tVideoAck.sequence;
		if (m_pVideo) {
			m_pVideo->set_ackseq(sequence);
		}
	}
		break;
	case kMsgTypeRequestKeyFrame:
	{
		RequestKeyFrame tRequestKeyFrame;
		const char* pPayload = (const char*)in + sizeof(WebSocketHeader);
		autojsoncxx::ParsingResult result;
		if (!autojsoncxx::from_json_string(pPayload, tRequestKeyFrame, result)) {
			printf("parser json string fail\n");
			break;
		}
		bool is_reset = tRequestKeyFrame.resetSequence;
		if (m_pVideo) {
			m_pVideo->reset_keyframe(is_reset);
		}
	}
		break;
	case kMsgTypePicture:
	{
		unsigned char* pData = (uint8_t*)in + sizeof(WebSocketHeader);
		if (m_PicFile == NULL) {
			SYSTEMTIME t;
			GetLocalTime(&t);
			sprintf(m_FilePath, "%%USERPROFILE%%\\Pictures\\pic%04d%02d%02d%02d%02d%02d.bmp", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
			m_PicFile = fopen(m_FilePath, "wb");
			if (m_pVideo && m_PicFile) {
				m_pVideo->WriteBmpHeader(m_PicFile);
			}
		}
		fwrite(pData, 1, uPayloadLen, m_PicFile);
		if (uMagic == 1) {
			fclose(m_PicFile);
			m_PicFile = NULL;
			if (m_CallbackPicture) {
				m_CallbackPicture(m_FilePath);
			}
			memset(m_FilePath, 0, sizeof(m_FilePath));
		}
	}
		break;
	case kMsgTypeOperate:
		if (m_CallbackOperater) {
			m_CallbackOperater(true);
		}
		break;
	case kMsgTypeOperateAck:
		if (m_CallbackOperater) {
			m_CallbackOperater(false);
		}
		break;
	case kMsgTypeMouseEvent:
	{
		MouseInput_C2S tMouseInput;
		const char* pPayload = (const char*)in + sizeof(WebSocketHeader);
		autojsoncxx::ParsingResult result;
		if (!autojsoncxx::from_json_string(pPayload, tMouseInput, result)) {
			printf("parser json string fail\n");
			break;
		}
		if (m_CallbackMouse) {
			m_CallbackMouse(tMouseInput.posX, tMouseInput.posY, tMouseInput.buttonMask);
		}
	}
		break;
	case kMsgTypeKeyboardEvent:
	{
		KeyboardInput_C2S tKeyboardInput;
		const char* pPayload = (const char*)in + sizeof(WebSocketHeader);
		autojsoncxx::ParsingResult result;
		if (!autojsoncxx::from_json_string(pPayload, tKeyboardInput, result)) {
			printf("parser json string fail\n");
			break;
		}
		if (m_CallbackKeyboard) {
			m_CallbackKeyboard(tKeyboardInput.keyValue, tKeyboardInput.isPressed);
		}
	}
		break;
	default:
		printf("handle_in unknown msg type %d\n", uType);
		break;
	}
}

void SocketsClient::send_connect()
{
	WebSocketHeader header;
	header.version = 1;
	header.magic = 0;
	header.type = Swap16IfLE(kMsgTypeConnect);
	header.length = 0;
	m_SendBuf->append(&header, sizeof(WebSocketHeader));
	send_msg((unsigned char*)m_SendBuf->getbuf(), m_SendBuf->getdatalength());
	m_SendBuf->reset();
}

void SocketsClient::send_publish()
{
	WebSocketHeader header;
	header.version = 1;
	header.magic = 0;
	header.type = Swap16IfLE(kMsgTypePublish);
	header.length = 0;
	m_SendBuf->append(&header, sizeof(WebSocketHeader));
	send_msg((unsigned char*)m_SendBuf->getbuf(), m_SendBuf->getdatalength());
	m_SendBuf->reset();
}

void SocketsClient::send_video_data(void* data)
{
	if (data == NULL)
		return;
	int iFrameSize = CalcFrameSize(data);
	if (!iFrameSize) {
		printf("frame size 0\n");
		return;
	}

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
	send_msg((unsigned char*)m_SendBuf->getbuf(), m_SendBuf->getdatalength());
	m_SendBuf->reset();
	printf("send video data seq %d\n", sequence);
}

void SocketsClient::send_video_ack(unsigned int sequence)
{
	VideoAck_C2S tVideoAck;
	tVideoAck.sequence = sequence;
	string str = autojsoncxx::to_json_string(tVideoAck);
	WebSocketHeader header;
	header.version = 1;
	header.magic = 0;
	header.type = Swap16IfLE(kMsgTypeVideoAck);
	header.length = Swap32IfLE(str.size());
	m_SendBuf->append(&header, sizeof(WebSocketHeader));
	m_SendBuf->append((unsigned char*)str.c_str(), str.size());
	send_msg((unsigned char*)m_SendBuf->getbuf(), m_SendBuf->getdatalength());
	m_SendBuf->reset();
}

void SocketsClient::send_keyframe_request(bool reset_seq)
{
	RequestKeyFrame tRequest;
	tRequest.resetMcuBuf = false;
	tRequest.resetSequence = reset_seq;
	string str = autojsoncxx::to_json_string(tRequest);
	WebSocketHeader header;
	header.version = 1;
	header.magic = 0;
	header.type = Swap16IfLE(kMsgTypeRequestKeyFrame);
	header.length = Swap32IfLE(str.size());
	m_SendBuf->append(&header, sizeof(WebSocketHeader));
	m_SendBuf->append((unsigned char*)str.c_str(), str.size());
	send_msg((unsigned char*)m_SendBuf->getbuf(), m_SendBuf->getdatalength());
	m_SendBuf->reset();
}

void SocketsClient::send_picture_data(unsigned char* data, int len)
{
	WebSocketHeader header;
	header.version = 1;
	header.magic = 0;
	header.type = Swap16IfLE(kMsgTypePicture);
	header.length = Swap32IfLE(len);

	int send_len = 0;
	while (send_len + WEBSOCKET_MAX_BUFFER_SIZE < len) {
		m_SendBuf->append(&header, sizeof(WebSocketHeader));
		m_SendBuf->append(data + send_len, WEBSOCKET_MAX_BUFFER_SIZE);
		send_len += send_msg((unsigned char*)m_SendBuf->getbuf(), m_SendBuf->getdatalength());
		m_SendBuf->reset();
	}
	header.magic = 1;
	m_SendBuf->append(&header, sizeof(WebSocketHeader));
	m_SendBuf->append(data + send_len, len - send_len);
	send_msg((unsigned char*)m_SendBuf->getbuf(), m_SendBuf->getdatalength());
	m_SendBuf->reset();
}

void SocketsClient::send_operate()
{
	WebSocketHeader header;
	header.version = 1;
	header.magic = 0;
	header.type = Swap16IfLE(kMsgTypeOperate);
	header.length = 0;
	m_SendBuf->append(&header, sizeof(WebSocketHeader));
	send_msg((unsigned char*)m_SendBuf->getbuf(), m_SendBuf->getdatalength());
	m_SendBuf->reset();
}

void SocketsClient::send_mouse_event(unsigned int x, unsigned int y, unsigned int button_mask)
{
	MouseInput_C2S tMouseInput;
	tMouseInput.posX = x;
	tMouseInput.posY = y;
	tMouseInput.buttonMask = button_mask;
	string str = autojsoncxx::to_json_string(tMouseInput);
	WebSocketHeader header;
	header.version = 1;
	header.magic = 0;
	header.type = Swap16IfLE(kMsgTypeMouseEvent);
	header.length = Swap32IfLE(str.size());
	m_SendBuf->append(&header, sizeof(WebSocketHeader));
	m_SendBuf->append((unsigned char*)str.c_str(), str.size());
	send_msg((unsigned char*)m_SendBuf->getbuf(), m_SendBuf->getdatalength());
	m_SendBuf->reset();
}

void SocketsClient::send_keyboard_event(unsigned int key_val, bool is_pressed)
{
	KeyboardInput_C2S tKeyInput;
	tKeyInput.keyValue = key_val;
	tKeyInput.isPressed = is_pressed;
	string str = autojsoncxx::to_json_string(tKeyInput);
	WebSocketHeader header;
	header.version = 1;
	header.magic = 0;
	header.type = Swap16IfLE(kMsgTypeKeyboardEvent);
	header.length = Swap32IfLE(str.size());
	m_SendBuf->append(&header, sizeof(WebSocketHeader));
	m_SendBuf->append((unsigned char*)str.c_str(), str.size());
	send_msg((unsigned char*)m_SendBuf->getbuf(), m_SendBuf->getdatalength());
	m_SendBuf->reset();
}

int SocketsClient::callback_client(struct lws *wsi, enum lws_callback_reasons reason, void *user,
									void *in, size_t len)
{
	SocketsClient* wsclient = (SocketsClient*)user;

	switch (reason) {
		/* when the callback is used for client operations --> */

	case LWS_CALLBACK_CLOSED:
		lwsl_debug("closed\n");
		wsclient->SetConnectState(ConnectStateClosed);
		wsclient->NotifyForceExit();
		break;
	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		lwsl_debug("close with error, reconnect\n");
		wsclient->SetConnectState(ConnectStateConnectError);
		wsclient->NotifyForceExit();
		break;
	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		lwsl_debug("Client has connected\n");
		wsclient->SetConnectState(ConnectStateEstablished);
		break;
	case LWS_CALLBACK_CLIENT_RECEIVE:
		//lwsl_notice("Client RX %d\n", len);
		wsclient->handle_in(wsi, in, len);
		break;
	case LWS_CALLBACK_CLIENT_WRITEABLE:
		break;

	case LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED:
		/* reject everything else except permessage-deflate */
		if (strcmp((const char*)in, "permessage-deflate"))
			return 1;
		break;

	default:
		break;
	}

	return 0;
}

unsigned int SocketsClient::CalcFrameSize(void* data)
{
	int iLayer = 0;
	int iFrameSize = 0;

	if (!data) {
		printf("encoded data null\n");
		return 0;
	}
	SFrameBSInfo* sFbi = (SFrameBSInfo*)data;
	//printf("Fbi layernum %d\n", sFbi->iLayerNum);
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
