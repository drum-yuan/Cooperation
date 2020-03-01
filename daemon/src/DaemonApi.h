#pragma once
#include <string>

#ifdef WIN32
#define DAEMON_API __declspec(dllexport)
#else
#define DAEMON_API
#endif

/* 当前用户信息
*/
typedef struct tagUsersInfo {
	unsigned int user_num;
	char user_list[20][64];
	char publisher[64];
	char operater[64];
} UsersInfo;

/* 收到直播通知后的回调
*/
typedef void(*StartStreamCallback)(void);
/* 收到停播通知后的回调
*/
typedef void(*StopStreamCallback)(void);
/* 收完图片文件后的回调
	file_path：图片文件路径名
*/
typedef void (*PictureCallback)(const char* file_path);
/* 操作权限设置成功后的回调
	is_operater：true——有权限；false——无权限
*/
typedef void (*OperaterCallback)(bool is_operater);
/* 收到鼠标事件后的回调
	x：位置x
	y：位置y
	button_mask：按键动作 （参考WinUser.h中的定义，如#define MOUSEEVENTF_MOVE        0x0001 ）
*/
typedef void (*MouseCallback)(unsigned int x, unsigned int y, unsigned int button_mask);
/* 收到键盘事件后的回调
	key_val：键值
	is_pressed：true——按下；false——抬起
*/
typedef void (*KeyboardCallback)(unsigned int key_val, bool is_pressed);
/* 收到鼠标形状后的回调
	x: 鼠标位置横坐标
	y: 鼠标位置纵坐标
	w: 鼠标形状图宽
	h: 鼠标形状图高
	color_bytes: 鼠标形状图颜色空间RGB数据
	mask_bytes: 鼠标形状图阿尔法通道数据
*/
typedef void (*CursorShapeCallback)(int x, int y, int w, int h, const std::string& color_bytes, const std::string& mask_bytes);

//启动daemon(包含网络初始连接)
DAEMON_API bool daemon_start(const char* url);
//停止daemon(包含断开连接)
DAEMON_API void daemon_stop();
//开始直播
DAEMON_API void daemon_start_stream();
//停止直播
DAEMON_API void daemon_stop_stream();
//显示直播画面
DAEMON_API void daemon_show_stream(void* hwnd);
//获取直播画面的原始宽高
DAEMON_API void daemon_get_stream_size(int* width, int* height);
//设置接收直播通知后的回调
DAEMON_API void daemon_set_start_stream_callback(StartStreamCallback on_stream);
//设置接收停播通知后的回调
DAEMON_API void daemon_set_stop_stream_callback(StopStreamCallback on_stream);
//锁屏后发送无损图片
DAEMON_API void daemon_send_picture();
//设置接收图片完成后的回调
DAEMON_API void daemon_set_picture_callback(PictureCallback on_picture);
//获取操作权限
DAEMON_API void daemon_start_operate();
//设置操作权限更新后的回调
DAEMON_API void daemon_set_operater_callback(OperaterCallback on_operater);
//发送鼠标事件
DAEMON_API void daemon_send_mouse_event(unsigned int x, unsigned int y, unsigned int button_mask);
//设置收到鼠标事件后的回调
DAEMON_API void daemon_set_mouse_callback(MouseCallback on_mouse);
//发送键盘事件
DAEMON_API void daemon_send_keyboard_event(unsigned int key_val, bool is_pressed);
//设置收到键盘事件后的回调
DAEMON_API void daemon_set_keyboard_callback(KeyboardCallback on_keyboard);
//发送鼠标形状
DAEMON_API void daemon_send_cursor_shape(int x, int y, int w, int h, const std::string& color_bytes, const std::string& mask_bytes);
//设置收到鼠标形状后的回调
DAEMON_API void daemon_set_cursor_shape_callback(CursorShapeCallback on_cursor_shape);
//获取播放参与者信息
DAEMON_API void daemon_get_users_info(UsersInfo* info);


