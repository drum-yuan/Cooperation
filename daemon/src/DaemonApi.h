#pragma once

#define DAEMON_API __declspec(dllexport)

DAEMON_API void daemon_start();

DAEMON_API void daemon_stop();

DAEMON_API int daemon_send_proxy_msg();

DAEMON_API int daemon_recv_proxy_msg();

DAEMON_API void daemon_start_stream();

DAEMON_API void daemon_stop_stream();

DAEMON_API void daemon_show_stream();

DAEMON_API void daemon_send_picture();

DAEMON_API void daemon_get_picture();

