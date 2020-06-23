#pragma once
#include <string>
#include <vector>

#ifdef WIN32
#define DAEMON_API __declspec(dllexport)
#else
#define DAEMON_API
#endif

/* ��ǰ�û���Ϣ
*/
typedef struct tagUsersInfo {
	unsigned int user_num;
	std::vector<std::string> user_list;
	std::string publisher;
	std::string operater;
} UsersInfo;

/* �յ�ֱ��֪ͨ��Ļص�
*/
typedef void(*StartStreamCallback)(int id);
/* �յ�ͣ��֪ͨ��Ļص�
*/
typedef void(*StopStreamCallback)(int id);
/* �յ�����Ӧ��֪ͨ��Ļص�
*/
typedef void(*VappStartCallback)();
/* �յ��ر���Ӧ��֪ͨ��Ļص�
*/
typedef void(*VappStopCallback)();
/* ����ͼƬ�ļ���Ļص�
	file_path��ͼƬ�ļ�·����
*/
typedef void (*PictureCallback)(int id, const char* file_path);
/* ����Ȩ�����óɹ���Ļص�
	is_operater��true������Ȩ�ޣ�false������Ȩ��
*/
typedef void (*OperaterCallback)(int id, bool is_operater);
/* �յ�����¼���Ļص�
	x��λ��x
	y��λ��y
	button_mask���������� ���ο�WinUser.h�еĶ��壬��#define MOUSEEVENTF_MOVE        0x0001 ��
*/
typedef void (*MouseCallback)(unsigned int x, unsigned int y, unsigned int button_mask);
/* �յ������¼���Ļص�
	key_val����ֵ
	is_pressed��true�������£�false����̧��
*/
typedef void (*KeyboardCallback)(unsigned int key_val, bool is_pressed);
/* �յ������״��Ļص�
	x: ���λ�ú�����
	y: ���λ��������
	w: �����״ͼ��
	h: �����״ͼ��
	color_bytes: �����״ͼ��ɫ�ռ�RGB����
	mask_bytes: �����״ͼ������ͨ������
*/
typedef void (*CursorShapeCallback)(int id, int x, int y, int w, int h, const std::string& color_bytes, const std::string& mask_bytes);
/* �յ����������ݺ�Ļص�
	data: ����������
*/
typedef void(*ClipboardDataCallback)(int id, int data_type, const std::string& data);

//����daemon
DAEMON_API int daemon_create();
//����daemon(���������ʼ����)
DAEMON_API bool daemon_start(int id, const std::string& url);
//ֹͣdaemon(�����Ͽ�����)
DAEMON_API void daemon_stop(int id);
//��ʼ����
DAEMON_API void daemon_start_publish(int id);
//��ʼֱ��
DAEMON_API void daemon_start_stream(int id);
//ֱֹͣ��
DAEMON_API void daemon_stop_stream(int id);
//��ʾֱ������
DAEMON_API void daemon_show_stream(int id, void* hwnd);
//��ȡֱ�������ԭʼ���
DAEMON_API void daemon_get_stream_size(int id, int* width, int* height);
//��ͣ��ʾֱ������
DAEMON_API void daemon_pause_show(int id);
//�ָ���ʾֱ������
DAEMON_API void daemon_resume_show(int id);
//���ý���ֱ��֪ͨ��Ļص�
DAEMON_API void daemon_set_start_stream_callback(int id, StartStreamCallback on_stream);
//���ý���ͣ��֪ͨ��Ļص�
DAEMON_API void daemon_set_stop_stream_callback(int id, StopStreamCallback on_stop);
//���ô���Ӧ�õ�֪ͨ�ص�
DAEMON_API void daemon_set_vapp_start_callback(int id, VappStartCallback on_vapp);
//���ùر���Ӧ�õ�֪ͨ�ص�
DAEMON_API void daemon_set_vapp_stop_callback(int id, VappStopCallback on_vapp_stop);
//������������ͼƬ
DAEMON_API void daemon_send_picture(int id);
//���ý���ͼƬ��ɺ�Ļص�
DAEMON_API void daemon_set_picture_callback(int id, PictureCallback on_picture);
//��ȡ����Ȩ��
DAEMON_API void daemon_start_operate(int id);
//���ò���Ȩ�޸��º�Ļص�
DAEMON_API void daemon_set_operater_callback(int id, OperaterCallback on_operater);
//��������¼�
DAEMON_API void daemon_send_mouse_event(int id, unsigned int x, unsigned int y, unsigned int button_mask);
//�����յ�����¼���Ļص�
DAEMON_API void daemon_set_mouse_callback(int id, MouseCallback on_mouse);
//���ͼ����¼�
DAEMON_API void daemon_send_keyboard_event(int id, unsigned int key_val, bool is_pressed);
//�����յ������¼���Ļص�
DAEMON_API void daemon_set_keyboard_callback(int id, KeyboardCallback on_keyboard);
//���������״
DAEMON_API void daemon_send_cursor_shape(int id, int x, int y, int w, int h, const std::string& color_bytes, const std::string& mask_bytes);
//�����յ������״��Ļص�
DAEMON_API void daemon_set_cursor_shape_callback(int id, CursorShapeCallback on_cursor_shape);
//��ȡ���Ų�������Ϣ
DAEMON_API void daemon_get_users_info(int id, UsersInfo* info);
//���ͼ���������
DAEMON_API void daemon_send_clipboard_data(int id, int data_type, const std::string& data);
//�����յ����������ݺ�Ļص�
DAEMON_API void daemon_set_clipboard_data_callback(int id, ClipboardDataCallback on_clipboard_data);


