#pragma once
#include <string>

#ifdef WIN32
#define DAEMON_API __declspec(dllexport)
#else
#define DAEMON_API
#endif

/* ��ǰ�û���Ϣ
*/
typedef struct tagUsersInfo {
	unsigned int user_num;
	char user_list[20][64];
	char publisher[64];
	char operater[64];
} UsersInfo;

/* �յ�ֱ��֪ͨ��Ļص�
*/
typedef void(*StartStreamCallback)(void);
/* �յ�ͣ��֪ͨ��Ļص�
*/
typedef void(*StopStreamCallback)(void);
/* ����ͼƬ�ļ���Ļص�
	file_path��ͼƬ�ļ�·����
*/
typedef void (*PictureCallback)(const char* file_path);
/* ����Ȩ�����óɹ���Ļص�
	is_operater��true������Ȩ�ޣ�false������Ȩ��
*/
typedef void (*OperaterCallback)(bool is_operater);
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
typedef void (*CursorShapeCallback)(int x, int y, int w, int h, const std::string& color_bytes, const std::string& mask_bytes);

//����daemon(���������ʼ����)
DAEMON_API bool daemon_start(const char* url);
//ֹͣdaemon(�����Ͽ�����)
DAEMON_API void daemon_stop();
//��ʼֱ��
DAEMON_API void daemon_start_stream();
//ֱֹͣ��
DAEMON_API void daemon_stop_stream();
//��ʾֱ������
DAEMON_API void daemon_show_stream(void* hwnd);
//��ȡֱ�������ԭʼ���
DAEMON_API void daemon_get_stream_size(int* width, int* height);
//���ý���ֱ��֪ͨ��Ļص�
DAEMON_API void daemon_set_start_stream_callback(StartStreamCallback on_stream);
//���ý���ͣ��֪ͨ��Ļص�
DAEMON_API void daemon_set_stop_stream_callback(StopStreamCallback on_stream);
//������������ͼƬ
DAEMON_API void daemon_send_picture();
//���ý���ͼƬ��ɺ�Ļص�
DAEMON_API void daemon_set_picture_callback(PictureCallback on_picture);
//��ȡ����Ȩ��
DAEMON_API void daemon_start_operate();
//���ò���Ȩ�޸��º�Ļص�
DAEMON_API void daemon_set_operater_callback(OperaterCallback on_operater);
//��������¼�
DAEMON_API void daemon_send_mouse_event(unsigned int x, unsigned int y, unsigned int button_mask);
//�����յ�����¼���Ļص�
DAEMON_API void daemon_set_mouse_callback(MouseCallback on_mouse);
//���ͼ����¼�
DAEMON_API void daemon_send_keyboard_event(unsigned int key_val, bool is_pressed);
//�����յ������¼���Ļص�
DAEMON_API void daemon_set_keyboard_callback(KeyboardCallback on_keyboard);
//���������״
DAEMON_API void daemon_send_cursor_shape(int x, int y, int w, int h, const std::string& color_bytes, const std::string& mask_bytes);
//�����յ������״��Ļص�
DAEMON_API void daemon_set_cursor_shape_callback(CursorShapeCallback on_cursor_shape);
//��ȡ���Ų�������Ϣ
DAEMON_API void daemon_get_users_info(UsersInfo* info);


