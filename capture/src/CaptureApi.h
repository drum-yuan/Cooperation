#pragma once

#define CAPTURE_API __declspec(dllexport) 

struct CallbackFrameInfo
{
	int grab_type;
	int width;
	int height;
	int line_bytes;
	int line_stride;
	int bitcount;
	int length;
	char* buffer;
};

typedef void (*FrameCallback)(CallbackFrameInfo* frame, void* param);


CAPTURE_API void xt_start_capture_screen(FrameCallback on_frame, void* param);

CAPTURE_API void xt_stop_capture_screen();

CAPTURE_API void xt_pause_capture_screen();

CAPTURE_API void xt_resume_capture_screen();

CAPTURE_API void xt_set_drop_interval(unsigned int count);

CAPTURE_API void xt_set_ack_sequence(unsigned int seq);

CAPTURE_API unsigned int xt_get_capture_sequence();

CAPTURE_API void xt_reset_sequence();

CAPTURE_API void xt_set_frame_rate(unsigned int rate);
