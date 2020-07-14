#include <Windows.h>
#include "gameStructs.h"
#include "utility.h"
#include <string>
#include <iostream>
#include <stdio.h>

static bool running = 1;
static bool active = 0;
static BITMAPINFO bitmapInfo = {};
static GameWindowBuffer gameWindowBuffer = {};
static char dllName[260];
static GameInput gameInput = {};

LRESULT windProc(HWND wind, UINT msg, WPARAM wp, LPARAM lp)
{
	LRESULT rez = 0;

	//if((msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST)
	//	||(msg >= WM_KEYFIRST && msg <= WM_KEYLAST)
	//	)
	//{
	//	HWND w = GetTopWindow(0);	
	//	HWND secondW = GetNextWindow(w, GW_HWNDNEXT);
	//
	//	char n[260];
	//
	//	GetWindowModuleFileName(secondW, n, 260);
	//	
	//	std::cout << n << "\n";
	//	SendMessage(secondW, msg, wp, lp);
	//}

	bool isDown = 0;

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
		
		RECT r;
		GetClientRect(wind, &r);

		int w = r.left;
		int h = r.bottom;
		//todo repair
		PatBlt(hdc, 0, 0, w, h, BLACKNESS);

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
	case WM_ACTIVATEAPP:
	{
		if (wp)
		{
			SetLayeredWindowAttributes(wind, RGB(0, 0, 0), 255, LWA_ALPHA);
			active = true;
		}
		else
		{
			SetLayeredWindowAttributes(wind, RGB(0, 0, 0), 105, LWA_ALPHA);
			active = false;
		}

	}	break;

	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		isDown = 1;
	case WM_SYSKEYUP:
	case WM_KEYUP: 
	{


		if(wp == 'W')
		{
			gameInput.up = isDown;
		}else if(wp == 'S')
		{
			gameInput.down = isDown;
		}
		else if (wp == 'A')
		{
			gameInput.left = isDown;
		}
		else if (wp == 'D')
		{
			gameInput.right = isDown;
		}

	}break;
	default:
		rez = DefWindowProc(wind, msg, wp, lp);
		break;
	}


	return rez;
}

static FILETIME win32GetLastWriteFile(const char *name)
{
	FILETIME time = {};

	WIN32_FILE_ATTRIBUTE_DATA Data;
	if (GetFileAttributesEx(name, GetFileExInfoStandard, &Data))
	{
		time = Data.ftLastWriteTime;
	}else
	{
	//	assert(0);
	}

	return(time);

}

static HMODULE dllHand;

static void win32LoadDll(gameLogic_t **gameLogicPtr)
{

	assert(CopyFile(dllName, "gameSetupCopy.dll", FALSE));

	dllHand = LoadLibrary("gameSetupCopy.dll");

	assert(dllHand);

	*gameLogicPtr = (gameLogic_t*)GetProcAddress(dllHand, "gameLogic");

	assert(*gameLogicPtr);

	OutputDebugString("RELOADED DLL");
}

static void win32UnloadDll()
{
	
	FreeLibrary(dllHand);

}

int WINAPI WinMain(HINSTANCE h, HINSTANCE, LPSTR cmd, int show)
{

#pragma region global mutex that lets only one instance of this app run
	CreateMutex(NULL, TRUE, "gameLayerMutex");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		return 0;
	}
#pragma endregion


	//console
#if 0 	
	AllocConsole();
	freopen("conin$", "r", stdin);
	freopen("conout$", "w", stdout);
	freopen("conout$", "w", stderr);
	std::cout.sync_with_stdio();
#endif

#pragma region dllName


	GetModuleFileName(GetModuleHandle(0), dllName, 260);

	//OutputDebugString(c);
	int s = strlen(dllName);
	for (int i = s - 1; i > 0; i--)
	{
		if (dllName[i] == '\\')
		{
			dllName[i] = 0;
			break;
		}
	}

	strcat_s(dllName, 260, "\\gameSetup.dll");

#pragma endregion


	WNDCLASS wc = {};
	
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hInstance = h;
	wc.lpfnWndProc = windProc;
	wc.lpszClassName = "MainWindowClass";
	wc.style = CS_HREDRAW | CS_VREDRAW;

	RegisterClass(&wc);

	HWND wind = CreateWindowEx
	(
		WS_EX_TOPMOST | WS_EX_LAYERED,
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

	//todo add a guard
	//todo add a base pointer
	//todo add compile macro settings
	GameMemory *gameMemory = (GameMemory*)VirtualAlloc(0, sizeof(GameMemory),
		MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	VolatileMemory* volatileMemory = (VolatileMemory*)VirtualAlloc(0, sizeof(VolatileMemory),
		MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

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

	FILETIME lastFileTime = win32GetLastWriteFile(dllName);

	win32LoadDll(&gameLogic_ptr);

	while (running)
	{

		MSG msg = {};
		while(PeekMessage(&msg, wind, 0, 0, PM_REMOVE) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		
		}


		volatileMemory->reset();

		//execute game logic
		gameLogic_ptr(&gameInput, gameMemory, volatileMemory, &gameWindowBuffer);

		//draw screen
		SendMessage(wind, WM_PAINT, 0, 0);


		//check if game code changed
		//todo change into a string macro
		//make utility char with path name
		FILETIME fileTime2 = {};
		
		
		fileTime2 = win32GetLastWriteFile(dllName);
		

		if(CompareFileTime(&lastFileTime, &fileTime2) != 0)
		{
			lastFileTime = fileTime2;
			win32UnloadDll();
			HANDLE file;
			while ((file = CreateFile(dllName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL,
				OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE)
			{
				//check if dll can be opened
			}

			CloseHandle(file);

			win32LoadDll(&gameLogic_ptr);

		}

	}

	CloseWindow(wind);

	return 0;
}