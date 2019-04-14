#include "DaemonApi.h"
#include <stdio.h>
#include <windows.h>

int main(int argc, char* argv)
{
	daemon_start("192.168.13.20:20000");

	//call daemon api
	daemon_start_stream();

	while (getchar() != 'q') {
		Sleep(10);
	}
	daemon_stop();
	return 0;
}