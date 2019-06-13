#ifndef __CAPTURE_API_H__
#define __CAPTURE_API_H__

#ifdef WIN32
#define CAPTURE_API __declspec(dllexport)
#else
#define CAPTURE_API
#endif

struct CallbackFrameInfo
{
	int width;
	int height;
	int line_bytes;
	int line_stride;
	int bitcount;
	int length;
	char* buffer;
};

typedef void (*FrameCallback)(CallbackFrameInfo* frame, void* param);


CAPTURE_API void cap_start_capture_screen(int monitor_id, FrameCallback on_frame, void* param);

CAPTURE_API void cap_stop_capture_screen();

CAPTURE_API void cap_pause_capture_screen();

CAPTURE_API void cap_resume_capture_screen();

CAPTURE_API void cap_set_drop_interval(unsigned int count);

CAPTURE_API void cap_set_ack_sequence(unsigned int seq);

CAPTURE_API unsigned int cap_get_capture_sequence();

CAPTURE_API void cap_set_capture_sequence(unsigned int seq);

CAPTURE_API void cap_reset_sequence();

#endif

CAPTURE_API void cap_set_frame_rate(unsigned int rate);
