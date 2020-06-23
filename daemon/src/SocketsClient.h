#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <vector>
#include "libwebsockets.h"
#include "Video.h"
#include "Audio.h"
#include "Buffer.h"

using namespace std;

typedef struct tagUsersInfoInternal {
	unsigned int user_num;
	vector<string> user_list;
	string publisher;
	string operater;
} UsersInfoInternal;

typedef void(*StartStreamCallback)(int id);
typedef void(*StopStreamCallback)(int id);
typedef void(*VappStartCallback)();
typedef void(*VappStopCallback)();
typedef void(*PictureCallback)(int id, const char* file_path);
typedef void(*OperaterCallback)(int id, bool is_operater);
typedef void(*MouseCallback)(unsigned int x, unsigned int y, unsigned int button_mask);
typedef void(*KeyboardCallback)(unsigned int key_val, bool is_pressed);
typedef void(*CursorShapeCallback)(int id, int x, int y, int w, int h, const string& color_bytes, const string& mask_bytes);
typedef void(*ClipboardDataCallback)(int id, int data_type, const std::string& data);
class SocketsClient
{
public:
	SocketsClient();
	~SocketsClient();

	void set_video_event(Video* pEvent);
	bool connect(std::string url, bool blocking, bool ssl);
	void stop();
	void reset();
	bool is_connected();
	void set_instance_id(int id);

	int send_msg(unsigned char* payload, unsigned int msglen);
	void continue_show_stream();
	void set_start_stream_callback(StartStreamCallback on_stream);
	void set_stop_stream_callback(StopStreamCallback on_stop);
	void set_vapp_start_callback(VappStartCallback on_vapp);
	void set_vapp_stop_callback(VappStopCallback on_vapp_stop);
	void set_picture_callback(PictureCallback on_recv);
	void set_operater_callback(OperaterCallback on_operater);
	void set_mouse_callback(MouseCallback on_mouse);
	void set_keyboard_callback(KeyboardCallback on_keyboard);
	void set_cursor_shape_callback(CursorShapeCallback on_cursor_shape);
	void set_clipboard_data_callback(ClipboardDataCallback on_clipboard_data);
	void handle_in(struct lws *wsi, const void* data, size_t len);

	void send_connect();
	void send_publish();
	void send_stop_stream();
	void send_video_data(void* data);
	void send_video_ack(unsigned int sequence);
	void send_keyframe_request(bool reset_seq);
	void send_picture_data(unsigned char* data, int len);
	void send_operate();
	void send_mouse_event(unsigned int x, unsigned int y, unsigned int button_mask);
	void send_keyboard_event(unsigned int key_val, bool is_pressed);
	void send_audio_data(unsigned char* data, int len, unsigned int frams_num);
	void send_cursor_shape(int x, int y, int w, int h, const string& color_bytes, const string& mask_bytes);
	void send_clipboard_data(int data_type, const string& data);
	UsersInfoInternal get_users_info();

	static int callback_client(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

private:
	void NotifyForceExit();
	void SetConnectState(int connstate);
	int RunWebSocketClient();
	unsigned int CalcFrameSize(void* data);

	int m_InsId;
	bool m_Exit;
	int  m_State;
	bool m_UseSSL;
	bool m_WaitingKeyframe;
	string m_ServerUrl;
	unsigned char* m_PicBuffer;
	int m_PicPos;

	lws* m_wsi;
	thread* m_wsthread;
	condition_variable m_cv;
	struct lws_protocols* m_protocols;

	Buffer* m_SendBuf;
	Video* m_pVideo;
	StartStreamCallback m_CallbackStream;
	StopStreamCallback m_CallbackStop;
	VappStartCallback m_CallbackVapp;
	VappStopCallback m_CallbackVappStop;
	PictureCallback m_CallbackPicture;
	OperaterCallback m_CallbackOperater;
	MouseCallback m_CallbackMouse;
	KeyboardCallback m_CallbackKeyboard;
	CursorShapeCallback m_CallbackCursorShape;
	ClipboardDataCallback m_CallbackClipboardData;
	UsersInfoInternal m_UsersInfo;
};
