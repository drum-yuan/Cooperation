#include <string>

#define VDI_API __declspec(dllexport)

VDI_API void sender_create();

VDI_API void sender_destroy();

VDI_API void sender_start(int server_port);

VDI_API void sender_stop();