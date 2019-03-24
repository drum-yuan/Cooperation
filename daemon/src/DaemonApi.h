#pragma once

#define DAEMON_API __declspec(dllexport)

typedef void (*PictureCallback)(const char* file_path);

typedef void (*OperaterCallback)(bool is_operater);

typedef void (*MouseCallback)(unsigned int x, unsigned int y, unsigned int button_mask);

typedef void (*KeyboardCallback)(unsigned int key_val, bool is_pressed);

DAEMON_API void daemon_start(const char* url);

DAEMON_API void daemon_stop();

DAEMON_API void daemon_start_stream();

DAEMON_API void daemon_stop_stream();

DAEMON_API void daemon_show_stream();

DAEMON_API void daemon_send_picture();

DAEMON_API void daemon_set_picture_callback(PictureCallback on_picture);

DAEMON_API void daemon_start_operate();

DAEMON_API void daemon_set_operater_callback(OperaterCallback on_operater);

DAEMON_API void daemon_send_mouse_event(unsigned int x, unsigned int y, unsigned int button_mask);

DAEMON_API void daemon_set_mouse_callback(MouseCallback on_mouse);

DAEMON_API void daemon_send_keyboard_event(unsigned int key_val, bool is_pressed);

DAEMON_API void daemon_set_keyboard_callback(KeyboardCallback on_keyboard);


