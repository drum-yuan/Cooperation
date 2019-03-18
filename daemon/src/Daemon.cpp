#include "Protocol.h"
#include "Daemon.h"


Daemon::Daemon()
{
}

Daemon::~Daemon()
{
	stop_stream();
}

void Daemon::start_stream()
{
	m_Video.SetOnEncoded(std::bind(&Daemon::OnVideoEncoded, this, std::placeholders::_1));
	m_Video.start();
}

void Daemon::stop_stream()
{
	m_Video.stop();
}

void Daemon::show_stream(HWND hWnd)
{
	m_Video.SetRenderWin(hWnd);
	m_McuClient.send_keyframe_request(true);
}

bool Daemon::connect_mcu(const string& url)
{
	m_McuUrl = url;
	if (!m_McuClient.connect(url, true, false)) {
		return false;
	}

	m_McuClient.set_video_event(&m_Video);
	m_McuClient.send_connect();
	return true;
}

void Daemon::set_recv_callback(RecvCallback on_recv)
{
	m_McuClient.set_recv_callback(on_recv);
}

void Daemon::OnVideoEncoded(void* data)
{
	m_McuClient.send_video_data(data);
}
