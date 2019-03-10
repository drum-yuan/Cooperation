#include "DaemonApi.h"
#include "Daemon.h"

static Daemon *s_pDaemon = NULL;

void daemon_start()
{
	if (s_pDaemon != NULL) {
		delete s_pDaemon;
	}
	s_pDaemon = new Daemon();
}

void daemon_stop()
{
	if (s_pDaemon != NULL) {
		delete s_pDaemon;
		s_pDaemon = NULL;
	}
}

int daemon_send_proxy_msg(const char* buffer, int size)
{
	return 0;
}

int daemon_recv_proxy_msg(char* buffer, int size)
{
	return 0;
}

void daemon_start_stream()
{
	s_pDaemon->start_stream();
}

void daemon_stop_stream()
{
	s_pDaemon->stop_stream();
}

void daemon_show_stream()
{

}

void daemon_send_picture()
{

}
