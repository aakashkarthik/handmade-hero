#include <windows.h>

#pragma warning (disable:28251)

int WinMain(HINSTANCE instance,
			HINSTANCE,
			LPSTR cmdline, 
			int cmdshow)
{
	UNREFERENCED_PARAMETER(instance);
	UNREFERENCED_PARAMETER(cmdline);
	UNREFERENCED_PARAMETER(cmdshow);
	MessageBox(0, TEXT("This is NOT a message"), TEXT("heading"), MB_ABORTRETRYIGNORE | MB_ICONERROR);
	return 0;
}