#include <windows.h>

#pragma warning (disable:28251)

LRESULT CALLBACK Manage(
	HWND Window,
	UINT Message,
	WPARAM Param1,
	LPARAM Param2
)
{
	LRESULT result = 0;
	PAINTSTRUCT paint;
	switch (Message)
	{
	case WM_PAINT:
	{
		BeginPaint(Window, &paint);
		EndPaint(Window, &paint);
	}break;
	case WM_DESTROY:
	{
		PostQuitMessage(0);
	}break;
	default:
		result = DefWindowProc(Window, Message, Param1, Param2);
	}
	return result;
}

int WinMain(HINSTANCE instance,
			HINSTANCE,
			LPSTR cmdline, 
			int cmdshow)
{
	UNREFERENCED_PARAMETER(cmdline);
	UNREFERENCED_PARAMETER(cmdshow);
	WNDCLASS WindowClass = {};
	WindowClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
	WindowClass.lpfnWndProc = Manage;
	WindowClass.hInstance = instance;
	WindowClass.lpszClassName = TEXT("handmadeherowindow");

	RegisterClass(&WindowClass);
	return 0;
}