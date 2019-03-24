#include "DaemonApi.h"
#include "Daemon.h"

static Daemon *s_pDaemon = NULL;

void daemon_start(const char* url)
{
	if (s_pDaemon != NULL) {
		delete s_pDaemon;
	}
	s_pDaemon = new Daemon();
	s_pDaemon->connect_mcu(string(url));
}

void daemon_stop()
{
	if (s_pDaemon != NULL) {
		delete s_pDaemon;
		s_pDaemon = NULL;
	}
}

void daemon_set_recv_callback(RecvCallback on_recv)
{
	s_pDaemon->set_recv_callback(on_recv);
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
	s_pDaemon->show_stream(GetDesktopWindow());
}

void daemon_send_picture()
{
	s_pDaemon->send_picture();
}

void daemon_start_operate()
{
	s_pDaemon->start_operate();
}