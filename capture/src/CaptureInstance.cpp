#include "CaptureApi.h"
#include "CaptureWin.h"

static CCapture *s_pCapture = NULL;

void xt_start_capture_screen(int monitor_id, FrameCallback on_frame, void* param)
{
	if (s_pCapture != NULL) {
		s_pCapture->stop();
		Sleep(1000);
		delete s_pCapture;
	}
	printf("new Capture\n");
	s_pCapture = new CCapture(monitor_id, on_frame, param);
	s_pCapture->start();
}

void xt_stop_capture_screen()
{
	if (s_pCapture != NULL) {
		s_pCapture->stop();
		Sleep(1000);
		delete s_pCapture;
		s_pCapture = NULL;
	}
}

void xt_pause_capture_screen()
{
	if (s_pCapture != NULL) {
		s_pCapture->pause();
	}
}

void xt_resume_capture_screen()
{
	if (s_pCapture != NULL) {
		s_pCapture->resume();
	}
}

void xt_set_drop_interval(unsigned int count)
{
	if (s_pCapture != NULL) {
		s_pCapture->set_drop_interval(count);
	}
}

void xt_set_ack_sequence(unsigned int seq)
{
	if (s_pCapture != NULL) {
		s_pCapture->set_ack_sequence(seq);
	}
}

unsigned int xt_get_capture_sequence()
{
	if (s_pCapture == NULL) {
		return 0;
	}
	return s_pCapture->get_capture_sequence();
}

void xt_reset_sequence()
{
	if (s_pCapture != NULL) {
		s_pCapture->reset_sequence();
	}
}

void xt_set_frame_rate(unsigned int rate)
{
	if (s_pCapture != NULL) {
		s_pCapture->set_frame_rate(rate);
	}
}