#include <Windows.h>
#include "gameStructs.h"
#include "utility.h"
#include <string>
#include <iostream>
#include <stdio.h>
#include "windowsFunctions.h"
#include "buildConfig.h"

static bool running = 1;
static bool active = 0;
static BITMAPINFO bitmapInfo = {};
static GameWindowBuffer gameWindowBuffer = {};
static GameMemory* gameMemory = nullptr;
static char dllName[260];
static GameInput gameInput = {};
static LARGE_INTEGER performanceFrequency;

#define NOT_RECORDING 0
#define RECORDING 1
#define PLAYING 2

static int recordingState = NOT_RECORDING;
static int recordingSlot = 0;
static HANDLE recordingFileHand = 0;
static HANDLE fileMapping = 0;
void* fileMappingPointer = nullptr;
size_t fileMappingSize = 0;
size_t fileMappingCursor = 0;

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
		bool altWasDown = lp & (1 << 29);

		if(wp == 'W')
		{
			gameInput.up = isDown;
		}
		if(wp == 'S')
		{
			gameInput.down = isDown;
		}
		
		if (wp == 'A')
		{
			gameInput.left = isDown;
		}
		if (wp == 'D')
		{
			gameInput.right = isDown;
		}
		if (wp == VK_F4)
		{
			if(altWasDown)
			{
				running = 0;
			}
		}
		if(wp == 'R' && altWasDown && (recordingState == NOT_RECORDING))
		{ 	
			int slot = 0;

			//start recording
			recordingState = RECORDING;
			recordingSlot = slot;

			saveGameState(slot, gameMemory);

			char c[20] = {};
			strcpy(c, "input0");
			c[strlen(c) - 1] += slot;
			strcat(c, ".save");

			//todo check errors

			recordingFileHand = CreateFile(c, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);

		
		}
		if(wp == 'S' && altWasDown)

		{
			if(recordingState == RECORDING)
			{
				CloseHandle(recordingFileHand);

			}else if(recordingState == PLAYING)
			{
				UnmapViewOfFile(fileMappingPointer);
				CloseHandle(fileMapping);
				CloseHandle(recordingFileHand);
			}

			recordingState = NOT_RECORDING;
			
			recordingFileHand = 0;

		}
		
		if (wp == 'P' && altWasDown && (recordingState == NOT_RECORDING))
		{
			//play recording
			recordingState = PLAYING;

			int slot = 0;

			loadGameState(slot, gameMemory);

			recordingSlot = slot;

			char c[20] = {};
			strcpy(c, "input0");
			c[strlen(c) - 1] += slot;
			strcat(c, ".save");

			//todo check errors

			recordingFileHand = CreateFile(c, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_ALWAYS, 0, 0);
			fileMapping = CreateFileMapping(recordingFileHand, 0, PAGE_READWRITE, 0, 0, 0);
			
			fileMappingPointer = MapViewOfFile(fileMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
			
			DWORD high = 0;
			DWORD low = 0;

			low = GetFileSize(recordingFileHand, &high);

			LARGE_INTEGER la = {};
			la.LowPart = low;
			la.HighPart = high;

			fileMappingSize = la.QuadPart;
			fileMappingCursor = 0;
		}

	}break;
	default:
		rez = DefWindowProc(wind, msg, wp, lp);
		break;
	}


	return rez;
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
#if 1 	
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

	size_t gameMemoryBaseAdress = 0;
	size_t gameMemorySize = MB(10);

	assert(sizeof(GameMemory) <= gameMemorySize)

#if INTERNAL_BUILD
	gameMemoryBaseAdress = TB(2);
#endif



	gameMemory = (GameMemory*)VirtualAlloc((LPVOID)gameMemoryBaseAdress, gameMemorySize,
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

	win32LoadDll(&gameLogic_ptr, dllName);

#pragma region time

	QueryPerformanceFrequency(&performanceFrequency);
	LARGE_INTEGER time1;
	LARGE_INTEGER time2;
	LARGE_INTEGER time3;
	LARGE_INTEGER time4;

	QueryPerformanceCounter(&time1);
	QueryPerformanceCounter(&time3);

#pragma endregion


	while (running)
	{
		QueryPerformanceCounter(&time4);
		LARGE_INTEGER deltaTimeInteger;
		deltaTimeInteger.QuadPart = time4.QuadPart - time3.QuadPart;
		
		float deltaTime = (float)deltaTimeInteger.QuadPart / (float)performanceFrequency.QuadPart;
		QueryPerformanceCounter(&time3);

		if (timeBeginPeriod(1) == TIMERR_NOERROR)
		{
			QueryPerformanceCounter(&time2);
			LARGE_INTEGER deltaTimeInteger;
			deltaTimeInteger.QuadPart = time2.QuadPart - time1.QuadPart;
			double dDeltaTime2 = (double)deltaTimeInteger.QuadPart / (double)performanceFrequency.QuadPart;

			int sleep = (1000.0 / 60.0) - (dDeltaTime2 * 1000.0);
			if (sleep > 0) { Sleep(sleep); }
			timeEndPeriod(1);
		}
		else
		{
			int sleep = 0;
			do
			{
				QueryPerformanceCounter(&time2);
				int deltaTime2 = time2.QuadPart - time1.QuadPart;
				double dDeltaTime2 = (double)deltaTime2 / (double)performanceFrequency.QuadPart;

				sleep = (1000.0 / 60.0) - (dDeltaTime2 * 1000.0);
			} while (sleep > 0);
		}


		MSG msg = {};
		while(PeekMessage(&msg, wind, 0, 0, PM_REMOVE) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		
		}

		volatileMemory->reset();

		gameInput.deltaTime = deltaTime;

		if(recordingState == PLAYING)
		{
			memcpy(&gameInput, (char*)fileMappingPointer + fileMappingCursor, sizeof(GameInput));
		
			fileMappingCursor += sizeof(GameInput);
			if (fileMappingCursor >= fileMappingSize)
			{
				fileMappingCursor = 0;
				loadGameState(recordingSlot, gameMemory);

			}

		}

		if(recordingState == RECORDING)
		{
			DWORD nrOfBytes = 0;
			WriteFile(recordingFileHand, &gameInput, sizeof(GameInput), &nrOfBytes, 0);
		}


		//execute game logic
		gameLogic_ptr(&gameInput, gameMemory, volatileMemory, &gameWindowBuffer);

		//draw screen
		RECT r;
		GetClientRect(wind, &r);

		int w = r.left;
		int h = r.bottom;
		//todo repair
		HDC hdc = GetDC(wind);
		PatBlt(hdc, 0, 0, w, h, BLACKNESS);
		ReleaseDC(wind, hdc);
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

			win32LoadDll(&gameLogic_ptr, dllName);

		}

	}

	CloseWindow(wind);

	return 0;
}