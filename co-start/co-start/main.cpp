#include <windows.h>
#include <stdio.h>

#pragma comment( linker, "/subsystem:windows /entry:mainCRTStartup" )

int main(int argc, char* argv[])
{
	char buffer[256];
	GetModuleFileName(NULL, buffer, sizeof(buffer));
	char* p = strrchr(buffer, '\\');
	*p = '\0';
	printf("module path %s\n", buffer);
	SetCurrentDirectory(buffer);
	char exeFile[256 + 64];
	sprintf_s(exeFile, sizeof(exeFile), "-i -d -s \"%s\\tool-win.exe\"", buffer);
	ShellExecute(NULL, "open", "PsExec.exe", exeFile, buffer, SW_HIDE);
	return 0;
}