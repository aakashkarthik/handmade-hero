#include <windows.h>
#include <stdint.h>

#pragma warning (disable:28251)

static LPVOID BitmapMemory;
static BOOL Running;
static int Bitmap_width;
static int Bitmap_height;
BITMAPINFO bitmap;

static void FillBuffer(int height,
					   int width,
					   int x_offset,
					   int y_offset)
{
		uint8_t* rgb = (uint8_t*)BitmapMemory;
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				*rgb = (uint8_t)(x + x_offset);//Blue
				rgb++;

				*rgb = (uint8_t)(y + y_offset);//Green
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
	if(BitmapMemory)
		VirtualFree(BitmapMemory, 0, MEM_RELEASE);
	BitmapMemory = VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);
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
		int x = paint.rcPaint.top;
		int y = paint.rcPaint.left;
		int width = paint.rcPaint.right - paint.rcPaint.left;
		int height = paint.rcPaint.bottom - paint.rcPaint.top;
		Bitmap_width = width;
		Bitmap_height = height;
		bitmap.bmiHeader.biSize = sizeof(bitmap.bmiHeader);
		bitmap.bmiHeader.biWidth = width;
		bitmap.bmiHeader.biHeight = -height;
		bitmap.bmiHeader.biPlanes = 1;
		bitmap.bmiHeader.biBitCount = 32;
		bitmap.bmiHeader.biCompression = BI_RGB;
		CreateBuffer(height, width);
		FillBuffer(height, width, 0, 0);
		StretchDIBits(hdc,
					  x, y, width, height,
					  x, y, width, height,
					  BitmapMemory,
					  &bitmap,
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
	
	int x = 0, y = 0;
	Running = true;
	while(Running)
	{
		MSG message;
		while(PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
		StretchDIBits(GetDC(Handle),
			0, 0, Bitmap_width, Bitmap_height,
			0, 0, Bitmap_width, Bitmap_height,
			BitmapMemory,
			&bitmap,
			DIB_RGB_COLORS,
			SRCCOPY);

		FillBuffer(Bitmap_height, Bitmap_width, x, y);
		x += 0;
		y += 0;
	}
	
	return 0;
}