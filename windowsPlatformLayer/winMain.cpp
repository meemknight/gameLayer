#include <Windows.h>
#include "gameStructs.h"

bool running = 1;

LRESULT windProc(HWND wind, UINT msg, WPARAM wp, LPARAM lp)
{
	LRESULT rez = 0;


	switch (msg)
	{
	case WM_CLOSE:
		running = 0;
		//PostQuitMessage(0);
		break;


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

	GameMemory* gameMemory = (GameMemory*)VirtualAlloc(0, sizeof(gameMemory), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	//load dll test
	{
		HMODULE dllHand = LoadLibrary("gameSetup.dll");
		if(dllHand)
		{
			OutputDebugString("Windows: found dll");
			FreeLibrary(dllHand);
		}

	}


	while (running)
	{
		MSG msg = {};
		while(PeekMessage(&msg, wind, 0, 0, PM_REMOVE) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		
		}

	}

	CloseWindow(wind);

	return 0;
}