#include "Protocol.h"
#include "Daemon.h"


Daemon::Daemon()
{
	m_pHeartbeatID = NULL;
	m_bQuit = true;
}

Daemon::~Daemon()
{
	m_bQuit = true;
	if (m_pHeartbeatID) {
		if (m_pHeartbeatID->joinable()) {
			m_pHeartbeatID->join();
		}
		delete m_pHeartbeatID;
		m_pHeartbeatID = NULL;
	}
	if (m_Video.IsPublisher()) {
		stop_stream();
	}
	m_McuClient.stop();
}

void Daemon::start_stream()
{
	m_Video.SetOnEncoded(std::bind(&Daemon::OnVideoEncoded, this, std::placeholders::_1));
	m_Audio.SetOnEncoded(std::bind(&Daemon::OnAudioEncoded, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	m_McuClient.send_publish();
	m_Video.start();
}

void Daemon::stop_stream()
{
	m_Video.stop();
	m_McuClient.send_stop_stream();
}

void Daemon::show_stream(void* hWnd)
{
	m_Video.SetRenderWin(hWnd);
	m_McuClient.send_keyframe_request(true);
}

void Daemon::get_stream_size(int* width, int* height)
{
	m_Video.GetStreamSize(width, height);
}

bool Daemon::connect_mcu(const string& url)
{
	m_McuUrl = url;
	if (!m_McuClient.connect(url, true, false)) {
		printf("connect mcu failed\n");
		return false;
	}

	printf("send connect msg\n");
	m_McuClient.set_video_event(&m_Video);
	m_McuClient.send_connect();
	m_bQuit = false;
	m_pHeartbeatID = new thread(&Daemon::HeartbeatThread, this);
	return true;
}

void Daemon::set_start_stream_callback(StartStreamCallback on_stream)
{
	m_McuClient.set_start_stream_callback(on_stream);
}

void Daemon::set_stop_stream_callback(StopStreamCallback on_stop)
{
	m_McuClient.set_stop_stream_callback(on_stop);
}

void Daemon::send_picture()
{
	m_Video.SetOnLockScreen(std::bind(&Daemon::OnLockScreen, this, std::placeholders::_1, std::placeholders::_2));
}

void Daemon::set_picture_callback(PictureCallback on_picture)
{
	m_McuClient.set_picture_callback(on_picture);
}

void Daemon::start_operate()
{
	m_McuClient.send_operate();
	m_Video.SetOperater(true);
}

void Daemon::set_operater_callback(OperaterCallback on_operater)
{
	m_McuClient.set_operater_callback(on_operater);
}

void Daemon::send_mouse_event(unsigned int x, unsigned int y, unsigned int button_mask)
{
	m_McuClient.send_mouse_event(x, y, button_mask);
}

void Daemon::set_mouse_callback(MouseCallback on_mouse)
{
	m_McuClient.set_mouse_callback(on_mouse);
}

void Daemon::send_keyboard_event(unsigned int key_val, bool is_pressed)
{
	m_McuClient.send_keyboard_event(key_val, is_pressed);
}

void Daemon::set_keyboard_callback(KeyboardCallback on_keyboard)
{
	m_McuClient.set_keyboard_callback(on_keyboard);
}

void Daemon::send_cursor_shape(int x, int y, int w, int h, const string& color_bytes, const string& mask_bytes)
{
	m_McuClient.send_cursor_shape(x, y, w, h, color_bytes, mask_bytes);
}

void Daemon::set_cursor_shape_callback(CursorShapeCallback on_cursor_shape)
{
	m_McuClient.set_cursor_shape_callback(on_cursor_shape);
}

UsersInfoInternal Daemon::get_users_info()
{
	return m_McuClient.get_users_info();
}

void Daemon::OnVideoEncoded(void* data)
{
	m_McuClient.send_video_data(data);
}

void Daemon::OnAudioEncoded(unsigned char* data, int len, unsigned int frame_num)
{
	m_McuClient.send_audio_data(data, len, frame_num);
}

void Daemon::OnLockScreen(unsigned char* data, int len)
{
	m_McuClient.send_picture_data(data, len);
	stop_stream();
}

void Daemon::HeartbeatThread()
{
	int retry = 3;

	while (!m_bQuit)
	{
		if (!m_McuClient.is_connected() && retry)
		{
			if (!m_McuClient.connect(m_McuUrl, true, false))
			{
				printf("reconnect mcu failed\n");
				if (!--retry)
				{
					break;
				}
			}
			else
			{
				m_McuClient.send_connect();
				if (m_Video.IsPublisher()) {
					m_McuClient.send_publish();
				}
				else {
					m_McuClient.continue_show_stream();
				}
				if (m_Video.IsOperater()) {
					m_McuClient.send_operate();
				}
				retry = 3;
			}
		}
#ifdef WIN32
		Sleep(100);
#else
        usleep(100 * 1000);
#endif
	}
}
