#include "Protocol.h"
#include "proto.hpp"
#include "autojsoncxx/autojsoncxx.hpp"
#include "lz4.h"
#include "libyuv.h"
#include "SocketsClient.h"

#define WEBSOCKET_MAX_BUFFER_SIZE (1024 * 1024 * 2)
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
								m_WaitingKeyframe(false),
								m_PicBuffer(NULL),
								m_PicPos(0),
								m_wsi(NULL),
								m_wsthread(NULL),
								m_protocols(NULL)
{
	m_protocols = (struct lws_protocols*)malloc(sizeof(struct lws_protocols) * 2);
	if (m_protocols)
	{
		memset(m_protocols, 0, sizeof(struct lws_protocols) * 2);
		m_protocols[0].name = "binary";
		m_protocols[0].callback = SocketsClient::callback_client;
		m_protocols[0].per_session_data_size = sizeof(SocketsClient*);
		m_protocols[0].rx_buffer_size = WEBSOCKET_MAX_BUFFER_SIZE + LWS_PRE;
		m_protocols[0].user = this;

		m_protocols[1].name = NULL;
		m_protocols[1].callback = NULL;
		m_protocols[1].per_session_data_size = 0;
		m_protocols[1].rx_buffer_size = 0;
		m_protocols[1].user = NULL;
	}

	m_SendBuf = new Buffer(WEBSOCKET_MAX_BUFFER_SIZE);
	m_pVideo = NULL;
	m_CallbackStream = NULL;
	m_CallbackStop = NULL;
	m_CallbackPicture = NULL;
	m_CallbackOperater = NULL;
	m_CallbackMouse = NULL;
	m_CallbackKeyboard = NULL;
	m_CallbackCursorShape = NULL;
}

SocketsClient::~SocketsClient()
{
	stop();
	if (m_protocols)
	{
		free(m_protocols);
		m_protocols = NULL;
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
	struct lws_client_connect_info i;
	int debug_level = 7;

	memset(&info, 0, sizeof info);
	if (m_UseSSL)
		use_ssl = 1;

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
			i.protocol = "binary";
			i.userdata = this;

			m_wsi = lws_client_connect_via_info(&i);
			if (!m_wsi) {
				lwsl_err("Client failed to connect to %s:%u\n", address, port);
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
	reset();
	m_Exit = false;
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
			delete m_wsthread;
		}
		m_wsthread = NULL;
	}
	m_wsi = NULL;
	m_State = ConnectStateUnConnected;
}

void SocketsClient::stop()
{
	reset();
}

bool SocketsClient::is_connected()
{
	return m_State == ConnectStateEstablished;
}

void SocketsClient::set_instance_id(int id)
{
	m_InsId = id;
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

void SocketsClient::continue_show_stream()
{
	if (m_CallbackStream) {
		m_CallbackStream(m_InsId);
	}
}

void SocketsClient::set_start_stream_callback(StartStreamCallback on_stream)
{
	m_CallbackStream = on_stream;
}

void SocketsClient::set_stop_stream_callback(StopStreamCallback on_stop)
{
	m_CallbackStop = on_stop;
}

void SocketsClient::set_vapp_start_callback(VappStartCallback on_vapp)
{
	m_CallbackVapp = on_vapp;
}

void SocketsClient::set_vapp_stop_callback(VappStopCallback on_vapp_stop)
{
	m_CallbackVappStop = on_vapp_stop;
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

void SocketsClient::set_cursor_shape_callback(CursorShapeCallback on_cursor_shape)
{
	m_CallbackCursorShape = on_cursor_shape;
}

void SocketsClient::set_clipboard_data_callback(ClipboardDataCallback on_clipboard_data)
{
	m_CallbackClipboardData = on_clipboard_data;
}

void SocketsClient::handle_in(struct lws *wsi, const void* in, size_t len)
{
	WebSocketHeader* pWebHeader = (WebSocketHeader*)in;
	unsigned char uMagic = pWebHeader->magic;
	unsigned short uType = Swap16IfLE(pWebHeader->type);
	unsigned int uPayloadLen = Swap32IfLE(pWebHeader->length);

	//printf("handle in type %u\n", uType);
	switch (uType)
	{
	case kMsgTypePublishAck:
	{
		printf("receive stream start %d\n", m_InsId);
		if (m_pVideo && m_pVideo->IsPublisher()) {
			m_pVideo->stop();
		}
		if (m_CallbackStream) {
			m_CallbackStream(m_InsId);
		}
	}
		break;
	case kMsgTypeVideoData:
	{
		if (m_pVideo == NULL) {
			break;
		}
		VideoDataHeader* pVideoHeader = (VideoDataHeader*)((uint8_t*)in + sizeof(WebSocketHeader));
		unsigned int sequence = Swap32IfLE(pVideoHeader->sequence);
		unsigned int option = Swap32IfLE(pVideoHeader->option);
		unsigned int len1 = uPayloadLen - sizeof(VideoDataHeader);
		unsigned int len2 = 0;
		//printf("recv length %u\n", uPayloadLen + sizeof(WebSocketHeader));

		if (option > 2) {
			len2 = len1 - option;
			len1 = option;
		}
		else if (option == 2) {
			len2 = len1;
			len1 = 0;
		}
		uint8_t* p = (uint8_t*)in + sizeof(WebSocketHeader) + sizeof(VideoDataHeader);
		unsigned int offset = 0;
		unsigned int rect_num = 0;
		if (sequence != 0xffffffff) {
			send_video_ack(sequence);
		}
		if (len1 > 0) {
			if (option != 0) {
				rect_num = *(unsigned int*)p;
				offset = rect_num * 8 + 4;
			}
			printf("decode 1 offset=%d len=%d\n", offset, len1-offset);
			if (!m_pVideo->show(p + offset, len1 - offset, true)) {
				if (!m_WaitingKeyframe) {
					send_keyframe_request(false);
					m_WaitingKeyframe = true;
				}
			}
			else {
				m_WaitingKeyframe = false;
			}
		}
		if (len2 > 0) {
			rect_num = *(unsigned int*)(p + len1);
			offset = rect_num * 8 + 4;
			printf("decode 2 offset=%d len=%d\n", offset, len2 - offset);
			m_pVideo->show(p + len1 + offset, len2 - offset, false);
		}
	}
		break;
	case kMsgTypeVideoAck:
	{
		if (m_pVideo == NULL) {
			break;
		}
		VideoAck_C2S tVideoAck;
		const char* pPayload = (const char*)in + sizeof(WebSocketHeader);
		autojsoncxx::ParsingResult result;
		if (!autojsoncxx::from_json_string(pPayload, tVideoAck, result)) {
			printf("parser json string fail\n");
			break;
		}
		unsigned int sequence = tVideoAck.sequence;

		if (m_pVideo->IsUseNvEnc()) {
#ifdef WIN32
			m_pVideo->set_ack_seq(sequence);
#endif
		}
		else {
			cap_set_ack_sequence(sequence);
		}
	}
		break;
	case kMsgTypeRequestKeyFrame:
	{
		if (m_pVideo && m_pVideo->IsPublisher()) {
			RequestKeyFrame tRequestKeyFrame;
			const char* pPayload = (const char*)in + sizeof(WebSocketHeader);
			autojsoncxx::ParsingResult result;
			if (!autojsoncxx::from_json_string(pPayload, tRequestKeyFrame, result)) {
				printf("parser json string fail\n");
				break;
			}
			bool is_reset = tRequestKeyFrame.resetSequence;
			m_pVideo->reset_keyframe(is_reset);
		}
		else if (m_CallbackVapp) {
			m_CallbackVapp();
		}
	}
		break;
	case kMsgTypePicture:
	{
		unsigned char* pData = (uint8_t*)in + sizeof(WebSocketHeader);
		int w = 0;
		int h = 0;
		m_pVideo->GetStreamSize(&w, &h);
		int bound = LZ4_compressBound(w * h * 3 / 2);
		if (m_PicBuffer == NULL) {
			m_PicBuffer = (unsigned char*)malloc(bound);
			m_PicPos = 0;
		}
		if (m_PicBuffer && m_PicPos + uPayloadLen <= bound) {
			memcpy(m_PicBuffer + m_PicPos, pData, uPayloadLen);
			m_PicPos += uPayloadLen;
		}
		else {
			free(m_PicBuffer);
			m_PicBuffer = NULL;
			printf("receive picture data out of memory\n");
			break;
		}
		if (uMagic == 1) {
			unsigned char* decompressed = (unsigned char*)malloc(w * h * 3 / 2);
			int decompressed_len = LZ4_decompress_safe((char*)m_PicBuffer, (char*)decompressed, m_PicPos, w * h * 3 / 2);
			free(m_PicBuffer);
			m_PicBuffer = NULL;
			/*if (m_pVideo) {
				m_pVideo->yuv_show(decompressed, w, h);
			}*/

            char file_path[256];
#ifdef WIN32
			SYSTEMTIME t;
			GetLocalTime(&t);
			char buffer[256];
			GetTempPath(sizeof(buffer), buffer);
			sprintf(file_path, "%s/pic%04d%02d%02d%02d%02d%02d.bmp", buffer, t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);
#else
            time_t now;
            struct tm* tm_now;
            time(&now);
            tm_now = localtime(&now);
            sprintf(file_path, "pic%04d%02d%02d%02d%02d%02d.bmp",
                    tm_now->tm_year+1900, tm_now->tm_mon+1, tm_now->tm_mday, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
#endif
			FILE* fp = fopen(file_path, "wb");
			if (fp && m_pVideo) {
				m_pVideo->WriteBmpHeader(fp);
				unsigned char* buffer = (unsigned char*)malloc(w * h * 4);
				libyuv::I420ToARGB(decompressed, w, decompressed + w * h, w / 2, decompressed + w * h + w * h / 4, w / 2, buffer, w * 4, w, h);
				fwrite(buffer, 1, w * h * 4, fp);
				fclose(fp);
				free(buffer);
			}
			if (m_CallbackPicture) {
				m_CallbackPicture(m_InsId, file_path);
			}
			free(decompressed);
		}
	}
		break;
	case kMsgTypeOperate:
	{
		if (m_CallbackOperater) {
			m_CallbackOperater(m_InsId, true);
		}
	}
		break;
	case kMsgTypeOperateAck:
	{
		if (m_CallbackOperater) {
			m_CallbackOperater(m_InsId, false);
		}
		if (m_pVideo) {
			m_pVideo->SetOperater(false);
		}
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
	case kMsgTypeStopStream:
	{
		if (m_pVideo && m_pVideo->IsPublisher()) {
			m_pVideo->stop();
		}
		else if (m_CallbackVappStop) {
			m_CallbackVappStop();
		}
	}
		break;
	case kMsgTypeStopStreamAck:
	case kMsgTypeStreamQuit:
	{
		if (m_CallbackStop) {
			m_CallbackStop(m_InsId);
		}
	}
		break;
	case kMsgTypeHeartbeat:
	{
		HeartbeatInfo tHeartbeat;
		const char* pPayload = (const char*)in + sizeof(WebSocketHeader);
		autojsoncxx::ParsingResult result;
		if (!autojsoncxx::from_json_string(pPayload, tHeartbeat, result)) {
			printf("parser json string fail %s\n", pPayload);
			break;
		}
		m_UsersInfo.user_num = tHeartbeat.UserNum;
		m_UsersInfo.user_list = tHeartbeat.UserList;
		m_UsersInfo.publisher = tHeartbeat.Publisher;
		m_UsersInfo.operater = tHeartbeat.Operater;
	}
		break;
	case kMsgTypeCursorShape:
	{
		CursorShapeUpdate_S2C tCursorShape;
		const char* pPayload = (const char*)in + sizeof(WebSocketHeader);
		autojsoncxx::ParsingResult result;
		if (!autojsoncxx::from_json_string(pPayload, tCursorShape, result)) {
			printf("parser json string fail %s\n", pPayload);
			break;
		}
		if (m_CallbackCursorShape) {
			m_CallbackCursorShape(m_InsId, tCursorShape.xhot, tCursorShape.yhot, tCursorShape.width, tCursorShape.height, tCursorShape.color_bytes_base64, tCursorShape.mask_bytes_base64);
		}
	}
		break;
	case kMsgTypeClipboardData:
	{
		ClipboardData tData;
		const char* pPayload = (const char*)in + sizeof(WebSocketHeader);
		autojsoncxx::ParsingResult result;
		if (!autojsoncxx::from_json_string(pPayload, tData, result)) {
			printf("parser json string fail %s\n", pPayload);
			break;
		}
		if (m_CallbackClipboardData) {
			m_CallbackClipboardData(m_InsId, tData.data_type, tData.data);
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
	string str = "{\"UserName\":\"";
#ifdef WIN32
	DWORD name_len = 0;
	GetComputerName(NULL, &name_len);
	char* name = new char[name_len];
	GetComputerName(name, &name_len);
	str += name;
	str += "|";
	delete[] name;
	name_len = 0;
	GetUserName(NULL, &name_len);
	name = new char[name_len];
	GetUserName(name, &name_len);
	str += name;
	delete[] name;
#endif
	str += "\"}";
	WebSocketHeader header;
	header.version = 1;
	header.magic = 0;
	header.type = Swap16IfLE(kMsgTypeConnect);
	header.length = Swap32IfLE(str.size());
	m_SendBuf->append(&header, sizeof(WebSocketHeader));
	m_SendBuf->append((unsigned char*)str.c_str(), str.size());

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

void SocketsClient::send_stop_stream()
{
	WebSocketHeader header;
	header.version = 1;
	header.magic = 0;
	header.type = Swap16IfLE(kMsgTypeStopStream);
	header.length = 0;
	m_SendBuf->append(&header, sizeof(WebSocketHeader));
	send_msg((unsigned char*)m_SendBuf->getbuf(), m_SendBuf->getdatalength());
	m_SendBuf->reset();
}

void SocketsClient::send_video_data(void* data)
{
	if (data == NULL) {
		printf("send data null\n");
		return;
	}
	if (m_pVideo->IsUseNvEnc()) {
#ifdef WIN32
		NV_ENC_LOCK_BITSTREAM* lockBitstreamData = (NV_ENC_LOCK_BITSTREAM*)data;
		printf("bitstreamdata len %d\n", lockBitstreamData->bitstreamSizeInBytes);

		WebSocketHeader header;
		header.version = 1;
		header.magic = 0;
		header.length = Swap32IfLE(sizeof(VideoDataHeader) + lockBitstreamData->bitstreamSizeInBytes);
		header.type = Swap16IfLE(kMsgTypeVideoData);
		m_SendBuf->append((void*)&header, sizeof(WebSocketHeader));

		VideoDataHeader videoheader;
		unsigned int frame_type = m_pVideo->get_frame_type(lockBitstreamData->pictureType);
		videoheader.eFrameType = Swap32IfLE(frame_type);
		unsigned int sequence = m_pVideo->get_capture_seq();
		videoheader.sequence = Swap32IfLE(sequence);
		videoheader.option = 0;
		m_SendBuf->append((unsigned char*)&videoheader, sizeof(VideoDataHeader));
		m_SendBuf->append((unsigned char*)lockBitstreamData->bitstreamBufferPtr, lockBitstreamData->bitstreamSizeInBytes);
#endif
	}
	else {
		int iFrameSize = CalcSVCFrameSize(data);
		if (!iFrameSize) {
			printf("frame size 0\n");
			cap_set_capture_sequence(cap_get_capture_sequence() - 1);
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
		videoheader.option = 0;
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
	}
	send_msg((unsigned char*)m_SendBuf->getbuf(), m_SendBuf->getdatalength());
	m_SendBuf->reset();
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

	int bound = LZ4_compressBound(len);
	unsigned char* compressed = (unsigned char*)malloc(bound);
	int compressed_len = LZ4_compress_default((char*)data, (char*)compressed, len, bound);

	int send_len = 0;
	int once_len = WEBSOCKET_MAX_BUFFER_SIZE - sizeof(WebSocketHeader);
	while (send_len + once_len < compressed_len) {
		header.length = Swap32IfLE(once_len);
		m_SendBuf->append(&header, sizeof(WebSocketHeader));
		m_SendBuf->append(compressed + send_len, once_len);
		send_len += once_len;
		send_msg((unsigned char*)m_SendBuf->getbuf(), m_SendBuf->getdatalength());
		m_SendBuf->reset();
#ifdef WIN32
		Sleep(10000);
#else
        usleep(10000 * 1000);
#endif
	}
	once_len = compressed_len - send_len;
	header.magic = 1;
	header.length = Swap32IfLE(once_len);
	m_SendBuf->append(&header, sizeof(WebSocketHeader));
	m_SendBuf->append(compressed + send_len, once_len);
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

void SocketsClient::send_audio_data(unsigned char* data, int len, unsigned int frams_num)
{
	WebSocketHeader header;
	header.version = 1;
	header.magic = 0;
	header.type = Swap16IfLE(kMsgTypeAudioData);
	header.length = sizeof(AudioDataHeader) + len;

	AudioDataHeader audioheader;
	audioheader.timestamp = Swap32IfLE((unsigned int)clock());
	audioheader.numFrames = Swap32IfLE(frams_num);
}

void SocketsClient::send_cursor_shape(int x, int y, int w, int h, const string& color_bytes, const string& mask_bytes)
{
	CursorShapeUpdate_S2C tUpdate;
	tUpdate.xhot = x;
	tUpdate.yhot = y;
	tUpdate.width = w;
	tUpdate.height = h;
	tUpdate.color_bytes_base64 = color_bytes;
	tUpdate.color_size = color_bytes.size();
	tUpdate.mask_bytes_base64 = mask_bytes;
	tUpdate.mask_size = mask_bytes.size();
	string str = autojsoncxx::to_json_string(tUpdate);
	WebSocketHeader header;
	header.version = 1;
	header.magic = 0;
	header.type = Swap16IfLE(kMsgTypeCursorShape);
	header.length = Swap32IfLE(str.size());
	m_SendBuf->append(&header, sizeof(WebSocketHeader));
	m_SendBuf->append((unsigned char*)str.c_str(), str.size());
	send_msg((unsigned char*)m_SendBuf->getbuf(), m_SendBuf->getdatalength());
	m_SendBuf->reset();
}

void SocketsClient::send_clipboard_data(int data_type, const string& data)
{
	ClipboardData tData;
	tData.data_type = data_type;
	tData.data = data;
	string str = autojsoncxx::to_json_string(tData);
	WebSocketHeader header;
	header.version = 1;
	header.magic = 0;
	header.type = Swap16IfLE(kMsgTypeClipboardData);
	header.length = Swap32IfLE(str.size());
	m_SendBuf->append(&header, sizeof(WebSocketHeader));
	m_SendBuf->append((unsigned char*)str.c_str(), str.size());
	send_msg((unsigned char*)m_SendBuf->getbuf(), m_SendBuf->getdatalength());
	m_SendBuf->reset();
}

UsersInfoInternal SocketsClient::get_users_info()
{
	return m_UsersInfo;
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

unsigned int SocketsClient::CalcSVCFrameSize(void* data)
{
	int iFrameSize = 0;
	int iLayer = 0;

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
