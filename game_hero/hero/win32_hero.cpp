#include <windows.h>
#include <stdint.h>

#pragma warning (disable:28251)
static BOOL Running;

struct Buffer
{
	BITMAPINFO info;
	LPVOID Memory;
	int width;
	int height;
}Bitmap;

static void FillBuffer(int height,
					   int width,
					   int x_offset,
					   int y_offset)
{
	UNREFERENCED_PARAMETER(y_offset);
		uint8_t* rgb = (uint8_t*)Bitmap.Memory;
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				*rgb = (uint8_t)(x + y + x_offset);//Blue
				rgb++;

				*rgb = (uint8_t)(y + x_offset);//Green
				rgb++;

				*rgb = 0;//Red
				rgb++;

				*rgb = 0;//Padding(always 0)
				rgb++;
			}
		}
}

static void CreateBuffer(int height,
						 int width)
{
	int size = height * width * 4;//bytes per pixel for me 32/8
	Bitmap.width = width;
	Bitmap.height = height;
	Bitmap.info.bmiHeader.biWidth = width;
	Bitmap.info.bmiHeader.biHeight = -height;

	if(Bitmap.Memory)
		VirtualFree(Bitmap.Memory, 0, MEM_RELEASE);
	Bitmap.Memory = VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);
}

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
		int width = paint.rcPaint.right - paint.rcPaint.left;
		int height = paint.rcPaint.bottom - paint.rcPaint.top;
		Bitmap.info.bmiHeader.biSize = sizeof(Bitmap.info.bmiHeader);
		Bitmap.info.bmiHeader.biPlanes = 1;
		Bitmap.info.bmiHeader.biBitCount = 32;
		Bitmap.info.bmiHeader.biCompression = BI_RGB;
		FillBuffer(Bitmap.height, Bitmap.width, 0, 0);
		StretchDIBits(hdc,
					  0, 0, width, height,
					  0, 0, Bitmap.width, Bitmap.height,
					  Bitmap.Memory,
					  &Bitmap.info,
					  DIB_RGB_COLORS,
					  SRCCOPY);
		EndPaint(Window, &paint);
	}break;
	case WM_DESTROY:
	{
		Running = false;
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
	HWND Handle = CreateWindowEx(0, WindowClass.lpszClassName,
				   TEXT("Aakash's Window"),
				   WS_OVERLAPPEDWINDOW | WS_VISIBLE,
				   CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
				   0, 0, instance, NULL);
	CreateBuffer(1920, 1080);
	
	int x = 0, y = 0;
	Running = true;
	while(Running)
	{
		MSG msg;
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
				Running = false;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		RECT ClientRect;
		GetClientRect(Handle, &ClientRect);
		int width = ClientRect.right - ClientRect.left;
		int height = ClientRect.bottom - ClientRect.top;
		StretchDIBits(GetDC(Handle),
			0, 0, width, height,
			0, 0, Bitmap.width, Bitmap.height,
			Bitmap.Memory,
			&Bitmap.info,
			DIB_RGB_COLORS,
			SRCCOPY);

		FillBuffer(Bitmap.height, Bitmap.width, x, y);
		x += 10;
		y += 10;
	}
	
	return 0;
}