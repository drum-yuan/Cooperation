#pragma once

#define DAEMON_API __declspec(dllexport)

typedef void (*RecvCallback)(const void* data, int len);

DAEMON_API void daemon_start(const char* url);

DAEMON_API void daemon_stop();

DAEMON_API void daemon_set_recv_callback(RecvCallback on_recv);

DAEMON_API void daemon_start_stream();

DAEMON_API void daemon_stop_stream();

DAEMON_API void daemon_show_stream();

DAEMON_API void daemon_send_picture();


