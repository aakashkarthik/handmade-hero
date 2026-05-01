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
	switch (Message)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT paint;
		HDC hdc = BeginPaint(Window, &paint);

		int x = paint.rcPaint.top;
		int y = paint.rcPaint.left;
		int width = paint.rcPaint.right - paint.rcPaint.left;
		int height = paint.rcPaint.bottom - paint.rcPaint.top;
		static DWORD rastorOp = WHITENESS;

		PatBlt(hdc, x, y, width, height, rastorOp);
		if (rastorOp == WHITENESS)
			rastorOp = BLACKNESS;
		else
			rastorOp = WHITENESS;

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
	CreateWindowEx(0, WindowClass.lpszClassName, TEXT("Aakash's Window"), WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, 0, instance, NULL);
	
	for (;;)
	{
		MSG message;
		int return_value = GetMessage(&message, NULL, 0, 0);
		if (return_value > 0)
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
		else if (return_value == 0)
			break;
		else if (return_value == -1)
		{
			MessageBox(NULL, TEXT("Error occured.Closing Window"), NULL, MB_OK|MB_ICONERROR);
			break;
		}

	}
	
	return 0;
}