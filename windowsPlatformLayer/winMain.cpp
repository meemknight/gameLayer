#include <Windows.h>
#include "gameStructs.h"
#include "utility.h"
#include <string>

static bool running = 1;
static BITMAPINFO bitmapInfo = {};
static GameWindowBuffer gameWindowBuffer = {};

LRESULT windProc(HWND wind, UINT msg, WPARAM wp, LPARAM lp)
{
	LRESULT rez = 0;


	switch (msg)
	{
	case WM_CLOSE:
		running = 0;
		//PostQuitMessage(0);
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT Paint;
		HDC DeviceContext = BeginPaint(wind, &Paint);
		
		HDC hdc = GetDC(wind);
		
		StretchDIBits(hdc,
			0, 0, gameWindowBuffer.w, gameWindowBuffer.h,
			0, 0, gameWindowBuffer.w, gameWindowBuffer.h,
			gameWindowBuffer.memory,
			&bitmapInfo,
			DIB_RGB_COLORS,
			SRCCOPY
		);

		ReleaseDC(wind, hdc);
		
		EndPaint(wind, &Paint);
	} break;
	default:
		rez = DefWindowProc(wind, msg, wp, lp);
		break;
	}


	return rez;
}



int WINAPI WinMain(HINSTANCE h, HINSTANCE, LPSTR cmd, int show)
{
	WNDCLASS wc = {};
	
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hInstance = h;
	wc.lpfnWndProc = windProc;
	wc.lpszClassName = "MainWindowClass";
	wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;

	RegisterClass(&wc);

	HWND wind = CreateWindow
	(
		wc.lpszClassName,
		"Geam",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		620,
		420,
		0,
		0,
		h,
		0
	);

	GameMemory* gameMemory = (GameMemory*)VirtualAlloc(0, sizeof(gameMemory),
		MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	GameInput gameInput = {};
	RECT rect;
	GetClientRect(wind, &rect);
	gameWindowBuffer.h = rect.bottom;
	gameWindowBuffer.w = rect.right;
	gameWindowBuffer.memory = 
		(char*)VirtualAlloc(0, 4 * gameWindowBuffer.w * gameWindowBuffer.h,
			MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	
	gameLogic_t* gameLogic_ptr;

	bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFO);
	bitmapInfo.bmiHeader.biWidth = gameWindowBuffer.w;
	bitmapInfo.bmiHeader.biHeight = -gameWindowBuffer.h;
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;


	{


		HMODULE dllHand = LoadLibrary("gameSetup.dll");
		
		int a = GetLastError();

		assert(dllHand);

		gameLogic_ptr = (gameLogic_t*)GetProcAddress(dllHand, "gameLogic");

		assert(gameLogic_ptr);
			
	}


	while (running)
	{
		MSG msg = {};
		while(PeekMessage(&msg, wind, 0, 0, PM_REMOVE) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		
		}

		//execute game logic
		gameLogic_ptr(&gameInput, gameMemory, &gameWindowBuffer);

		//draw screen
		{
			HDC hdc = GetDC(wind);

			StretchDIBits(hdc,
				0, 0, gameWindowBuffer.w, gameWindowBuffer.h,
				0, 0, gameWindowBuffer.w, gameWindowBuffer.h,
				gameWindowBuffer.memory,
				&bitmapInfo,
				DIB_RGB_COLORS,
				SRCCOPY
			);

			ReleaseDC(wind, hdc);
		}
	}

	CloseWindow(wind);

	return 0;
}