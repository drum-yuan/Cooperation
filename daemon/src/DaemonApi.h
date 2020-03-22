#pragma once
#include <string>
#include <vector>

#ifdef WIN32
#define DAEMON_API __declspec(dllexport)
#else
#define DAEMON_API
#endif

/* 当前用户信息
*/
typedef struct tagUsersInfo {
	unsigned int user_num;
	std::vector<std::string> user_list;
	std::string publisher;
	std::string operater;
} UsersInfo;

/* 收到直播通知后的回调
*/
typedef void(*StartStreamCallback)(int id);
/* 收到停播通知后的回调
*/
typedef void(*StopStreamCallback)(int id);
/* 收完图片文件后的回调
	file_path：图片文件路径名
*/
typedef void (*PictureCallback)(int id, const char* file_path);
/* 操作权限设置成功后的回调
	is_operater：true——有权限；false——无权限
*/
typedef void (*OperaterCallback)(int id, bool is_operater);
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
typedef void (*CursorShapeCallback)(int id, int x, int y, int w, int h, const std::string& color_bytes, const std::string& mask_bytes);

//启动daemon(包含网络初始连接)
DAEMON_API int daemon_start(const std::string& url);
//停止daemon(包含断开连接)
DAEMON_API void daemon_stop(int id);
//开始直播
DAEMON_API void daemon_start_stream(int id, bool is_desktop);
//停止直播
DAEMON_API void daemon_stop_stream(int id);
//显示直播画面
DAEMON_API void daemon_show_stream(int id, void* hwnd);
//获取直播画面的原始宽高
DAEMON_API void daemon_get_stream_size(int id, int* width, int* height);
//设置接收直播通知后的回调
DAEMON_API void daemon_set_start_stream_callback(int id, StartStreamCallback on_stream);
//设置接收停播通知后的回调
DAEMON_API void daemon_set_stop_stream_callback(int id, StopStreamCallback on_stream);
//锁屏后发送无损图片
DAEMON_API void daemon_send_picture(int id);
//设置接收图片完成后的回调
DAEMON_API void daemon_set_picture_callback(int id, PictureCallback on_picture);
//获取操作权限
DAEMON_API void daemon_start_operate(int id);
//设置操作权限更新后的回调
DAEMON_API void daemon_set_operater_callback(int id, OperaterCallback on_operater);
//发送鼠标事件
DAEMON_API void daemon_send_mouse_event(int id, unsigned int x, unsigned int y, unsigned int button_mask);
//设置收到鼠标事件后的回调
DAEMON_API void daemon_set_mouse_callback(int id, MouseCallback on_mouse);
//发送键盘事件
DAEMON_API void daemon_send_keyboard_event(int id, unsigned int key_val, bool is_pressed);
//设置收到键盘事件后的回调
DAEMON_API void daemon_set_keyboard_callback(int id, KeyboardCallback on_keyboard);
//发送鼠标形状
DAEMON_API void daemon_send_cursor_shape(int id, int x, int y, int w, int h, const std::string& color_bytes, const std::string& mask_bytes);
//设置收到鼠标形状后的回调
DAEMON_API void daemon_set_cursor_shape_callback(int id, CursorShapeCallback on_cursor_shape);
//获取播放参与者信息
DAEMON_API void daemon_get_users_info(int id, UsersInfo* info);


