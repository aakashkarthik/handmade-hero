#include <windows.h>
#include <stdint.h>
#include <stdbool.h>
#include <Xinput.h>
#include <stdio.h>
#include <dsound.h>
#pragma warning (disable:28251)

DWORD (*DynXInputGetState)(DWORD dwUserIndex, XINPUT_STATE* pState);
DWORD XInputGetStateStub(DWORD, XINPUT_STATE*) { return ERROR_DEVICE_NOT_CONNECTED; }
#define XInputGetState DynXInputGetState

DWORD (*DynXInputSetState)(DWORD dwUserIndex,XINPUT_VIBRATION* pVibration);
DWORD XInputSetStateStub(DWORD, XINPUT_VIBRATION*) { return ERROR_DEVICE_NOT_CONNECTED; }
#define XInputSetState DynXInputSetState

typedef HRESULT WINAPI DynDirectSoundCreate(LPCGUID pcGuidDevice,LPDIRECTSOUND* ppDS,LPUNKNOWN pUnkOuter);
DynDirectSoundCreate *directSoundCreate;

static void LoadDsoundLibrary(HWND Window)
{
	HMODULE Library = LoadLibrary(TEXT("dsound.dll"));
	if (Library) {
		directSoundCreate = (DynDirectSoundCreate*) GetProcAddress(Library, "DirectSoundCreate");
		LPDIRECTSOUND DirectSoundObj;
		if (directSoundCreate && SUCCEEDED(directSoundCreate(NULL, &DirectSoundObj, 0)))
		{
			//Set cooperative Level
			if(SUCCEEDED(DirectSoundObj->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
			{
				DSBUFFERDESC BufferDescription = {};
				LPDIRECTSOUNDBUFFER dsPrimaryBuffer;
				if (SUCCEEDED(DirectSoundObj->CreateSoundBuffer(&BufferDescription, &dsPrimaryBuffer, 0)))
				{

				}
			}
			else
			{ }
		}
	}
}
static void LoadXInputLibrary(void)
{
	HMODULE Library = LoadLibraryA("XInput1_4.dll");
	if (Library == NULL)
		Library = LoadLibraryA("XInput1_3.dll");
	if (Library == NULL)
		Library = LoadLibraryA("XInput9_1_0");
	if (Library)
	{
		DynXInputGetState = (DWORD(*)(DWORD, XINPUT_STATE*))
			GetProcAddress(Library, "XInputGetState");
		DynXInputSetState = (DWORD(*)(DWORD, XINPUT_VIBRATION*))
			GetProcAddress(Library, "XInputSetState");
	}
	else
	{
		DynXInputGetState = &XInputGetStateStub;
		DynXInputSetState = &XInputSetStateStub;
	}
}


void LoadLibraries(HWND Window)
{
	LoadXInputLibrary();
	LoadDsoundLibrary(Window);
}

static int x = 0, y = 0, incrementX, incrementY;
static BOOL animate = true;
static BOOL Running;

static void KeyboardInput(WPARAM wparam, LPARAM lparam)
{
	WPARAM VKcode = wparam;
	//BOOL PrevKey = ((lparam & (1ULL << 30)) != 0);
	BOOL IsKey = ((lparam & (1ULL << 31)) == 0);

	if (IsKey)
	{
		switch (VKcode)
		{
			case VK_SPACE:
			{
				if (animate)
				{
					animate = false;
				}
				else
				{
					animate = true;
				}
			}break;
			case VK_UP:
			{
				if (animate)
				{
					incrementX = 0;
					incrementY = -5;
				}
			}break;
			case VK_DOWN:
			{
				if (animate)
				{
					incrementX = 0;
					incrementY = 5;
				}
			}break;
			case VK_RIGHT:
			{
				if (animate)
				{
					incrementY = 0;
					incrementX = 5;
				}
			}break;
			case VK_LEFT:
			{
				if (animate)
				{
					incrementY = 0;
					incrementX = -5;
				}
			}break;
			case VK_ESCAPE:
			{
				Running = false;
			}break;
		}
	}
}

static unsigned int OldPacket = 0;

struct Buffer
{
	BITMAPINFO info;
	LPVOID Memory;
	int width;
	int height;
}Bitmap;

static void FillBuffer(int height,
					   int width)
{
		uint8_t* rgb = (uint8_t*)Bitmap.Memory;
		for (int Y = 0; Y < height; Y++)
		{
			for (int X = 0; X < width; X++)
			{
				*rgb = (uint8_t)(X + x);//Blue
				rgb++;

				*rgb = (uint8_t)(Y + y);//Green
				rgb++;

				*rgb = 0;//Red
				rgb++;

				*rgb = 0;//Padding actually alpha
				rgb++;
			}
		}
}

static void CreateBuffer(int height,
						 int width)
{
	int size = height * width * 4;//bytes per pixel for me 32/8
	Bitmap.info.bmiHeader.biSize = sizeof(Bitmap.info.bmiHeader);
	Bitmap.info.bmiHeader.biBitCount = 32;
	Bitmap.width = width;
	Bitmap.height = height;
	Bitmap.info.bmiHeader.biWidth = width;
	Bitmap.info.bmiHeader.biHeight = -height;
	Bitmap.info.bmiHeader.biPlanes = 1;
	Bitmap.info.bmiHeader.biCompression = BI_RGB;

	if(Bitmap.Memory)
		VirtualFree(Bitmap.Memory, 0, MEM_RELEASE);
	Bitmap.Memory = VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);
}

static void Render(HWND Handle, RECT *ClientRect)
{
	if (animate)
	{
		GetClientRect(Handle, ClientRect);
		int width = ClientRect->right - ClientRect->left;
		int height = ClientRect->bottom - ClientRect->top;
		FillBuffer(Bitmap.height, Bitmap.width);
		HDC hdc = GetDC(Handle);
		StretchDIBits(hdc,
			0, 0, width, height,
			0, 0, Bitmap.width, Bitmap.height,
			Bitmap.Memory,
			&Bitmap.info,
			DIB_RGB_COLORS,
			SRCCOPY);
		ReleaseDC(Handle, hdc);
		x += incrementX;
		y += incrementY;
	}
}

LRESULT CALLBACK Manage(
	HWND Window,
	UINT Message,
	WPARAM wparam,
	LPARAM lparam
)
{
	LRESULT result = 0;
	switch (Message)
	{
		case WM_SYSKEYUP:
		case WM_SYSKEYDOWN :
		case WM_KEYUP :
		case WM_KEYDOWN:
		{
			KeyboardInput(wparam, lparam);
		}break;

		case WM_DESTROY:
		{
			Running = false;
		}break;

		default:
			result = DefWindowProc(Window, Message, wparam, lparam);
	}
	return result;
}

static void ReadMessage(void)
{
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

}

static void LoadXboxInput(void)
{
	XINPUT_STATE State;
	for (DWORD ControlIndex = 0; ControlIndex < XUSER_MAX_COUNT; ControlIndex++)
	{
		ZeroMemory(&State, sizeof(XINPUT_STATE));

		DWORD dwResult = XInputGetState(ControlIndex, &State);
		
		if (dwResult == ERROR_SUCCESS)
		{
			if (OldPacket != State.dwPacketNumber)
			{
				OldPacket = State.dwPacketNumber;

				BOOL up = State.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP;
				BOOL down = State.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
				BOOL left = State.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
				BOOL right = State.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
				BOOL start = State.Gamepad.wButtons & XINPUT_GAMEPAD_START;
				BOOL back = State.Gamepad.wButtons & XINPUT_GAMEPAD_BACK;
				BOOL LShoulder = State.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER;
				BOOL RShoulder = State.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER;
				BOOL ButtonA = State.Gamepad.wButtons & XINPUT_GAMEPAD_A;
				BOOL ButtonB = State.Gamepad.wButtons & XINPUT_GAMEPAD_B;
				BOOL ButtonX = State.Gamepad.wButtons & XINPUT_GAMEPAD_X;
				BOOL ButtonY = State.Gamepad.wButtons & XINPUT_GAMEPAD_Y;


				//short ThumbLX = State.Gamepad.sThumbLX;
				//short ThumbLY = State.Gamepad.sThumbLY;
				//short ThumbRX = State.Gamepad.sThumbRX;
				//short ThumbRY = State.Gamepad.sThumbRY;

				//if (ThumbLX > 10 || ThumbLY > 10)
				//{
				//	incrementX = ThumbLX >> 12;
				//	incrementY = -ThumbLY >> 12;
				//}

				if (up || ButtonY && animate)
				{
					incrementX = 0;
					incrementY = -5;
				}
				if (down || ButtonA && animate)
				{
					incrementX = 0;
					incrementY = 5;
				}
				if (right || ButtonB && animate)
				{
					incrementY = 0;
					incrementX = 5;
				}
				if (left || ButtonX && animate)
				{
					incrementY = 0;
					incrementX = -5;
				}
				if (start || back || LShoulder || RShoulder)
					animate = !animate;
			}
		}
		else
		{
			//IF controller is not connected.
		}
	}
}

static HWND MakeWindow(WNDCLASS& WindowClass, HINSTANCE instance)
{
	WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	WindowClass.lpfnWndProc = Manage;
	WindowClass.hInstance = instance;
	WindowClass.lpszClassName = TEXT("handmadeherowindow");

	RegisterClass(&WindowClass);
	HWND handle = CreateWindowEx(0, WindowClass.lpszClassName,
		TEXT("Aakash's Window"),
		WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_MAXIMIZE,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		0, 0, instance, NULL);
	return handle;
}

int WinMain(HINSTANCE instance,
			HINSTANCE,
			LPSTR cmdline, 
			int cmdshow)
{
	UNREFERENCED_PARAMETER(cmdline);
	UNREFERENCED_PARAMETER(cmdshow);
	WNDCLASS WindowClass = {};
	HWND Handle = MakeWindow(WindowClass, instance);
	CreateBuffer(1920, 1080);
	LoadLibraries(Handle);
	
	Running = true;
	while(Running)
	{
		ReadMessage();
		LoadXboxInput();
		RECT ClientRect;
		Render(Handle, &ClientRect);
	}
	
	return 0;
}