#pragma once

#define DAEMON_API __declspec(dllexport)
/* �յ�ֱ��֪ͨ��Ļص�
*/
typedef void(*StartStreamCallback)(void);
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

//����daemon(���������ʼ����)
DAEMON_API void daemon_start(const char* url);
//ֹͣdaemon(�����Ͽ�����)
DAEMON_API void daemon_stop();
//��ʼֱ��
DAEMON_API void daemon_start_stream();
//ֱֹͣ��
DAEMON_API void daemon_stop_stream();
//��ʾֱ������
DAEMON_API void daemon_show_stream(void* hwnd);
//���ý���ֱ��֪ͨ��Ļص�
DAEMON_API void daemon_set_start_stream_callback(StartStreamCallback on_stream);
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


