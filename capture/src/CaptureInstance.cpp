#include "CaptureApi.h"
#ifdef WIN32
#include "CaptureWin.h"
#else
#include "CaptureLinux.h"
#include <unistd.h>
#endif

static CCapture *s_pCapture = NULL;

void cap_start_capture_screen(int monitor_id, FrameCallback on_frame, void* param)
{
	if (s_pCapture != NULL) {
		s_pCapture->stop();
#ifdef WIN32
		Sleep(1000);
#else
		usleep(1000 * 1000);
#endif
		delete s_pCapture;
	}
	printf("new Capture\n");
	s_pCapture = new CCapture(monitor_id, on_frame, param);
	s_pCapture->start();
}

void cap_stop_capture_screen()
{
	if (s_pCapture != NULL) {
		s_pCapture->stop();
#ifdef WIN32
                Sleep(1000);
#else
                usleep(1000 * 1000);
#endif
		delete s_pCapture;
		s_pCapture = NULL;
	}
}

void cap_pause_capture_screen()
{
	if (s_pCapture != NULL) {
		s_pCapture->pause();
	}
}

void cap_resume_capture_screen()
{
	if (s_pCapture != NULL) {
		s_pCapture->resume();
	}
}

void cap_set_drop_interval(unsigned int count)
{
	if (s_pCapture != NULL) {
		s_pCapture->set_drop_interval(count);
	}
}

unsigned int cap_get_ack_sequence()
{
	if (s_pCapture == NULL) {
		return 0;
	}
	return s_pCapture->get_ack_sequence();
}

void cap_set_ack_sequence(unsigned int seq)
{
	if (s_pCapture != NULL) {
		s_pCapture->set_ack_sequence(seq);
	}
}

unsigned int cap_get_capture_sequence()
{
	if (s_pCapture == NULL) {
		return 0;
	}
	return s_pCapture->get_capture_sequence();
}

void cap_set_capture_sequence(unsigned int seq)
{
	if (s_pCapture != NULL) {
		s_pCapture->set_capture_sequence(seq);
	}
}

void cap_reset_sequence()
{
	if (s_pCapture != NULL) {
		s_pCapture->reset_sequence();
	}
}

void cap_set_frame_rate(unsigned int rate)
{
	if (s_pCapture != NULL) {
		s_pCapture->set_frame_rate(rate);
	}
}