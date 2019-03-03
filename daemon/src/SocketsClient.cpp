#include "SocketsClient.h"

#define WEBSOCKET_MAX_BUFFER_SIZE 1048576
#define CONNECT_TIMEOUT 5000  //ms

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
	const char *interface = NULL;
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
	info.iface = interface;
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

bool SocketsClient::connect(std::string url, bool blocking, bool ssl)
{
	m_ServerUrl = url;
	m_UseSSL = ssl;
	reset();
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

	return lws_write(m_wsi, payload, msglen, LWS_WRITE_BINARY);
}

int SocketsClient::recv_msg(unsigned char* payload, unsigned int msglen)
{
	if (m_State != ConnectStateEstablished)
	{
		return -1;
	}

	return lws_read(m_wsi, payload, msglen);
}

void SocketsClient::handle_in(struct lws *wsi, const void* in, size_t len)
{

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
		lwsl_notice("Client RX %d\n", len);
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