#include <Windows.h>
#include "gameStructs.h"
#include "utility.h"
#include <string>
#include <iostream>
#include <stdio.h>
#include "windowsFunctions.h"
#include "buildConfig.h"
#include <Xinput.h>

static bool running = 1;
static bool active = 0;
static BITMAPINFO bitmapInfo = {};
static GameWindowBuffer gameWindowBuffer = {};
static GameMemory* gameMemory = nullptr;
static char dllName[260];
static GameInput gameInput = {};
static LARGE_INTEGER performanceFrequency;

static Win32ReplayBufferData replayBufferData;
static Win32XinputData xinputData;

const int sizeX = 192;
const int sizeY = 40;
const int magnifier = 7;

void processAsynkButton(Button &b, bool newState)
{
	if (newState)
	{
		if (!b.held)
		{
			b.held = true;
			b.pressed = true;
			b.released = false;

		}
		else
		{
			b.held = true;
			b.pressed = false;
			b.released = false;
		}

	}
	else
	{
		if (b.held)
		{
			b.held = false;
			b.released = true;
			b.pressed = false;

		}
		else
		{
			b.held = false;
			b.released = false;
			b.pressed = false;

		}

	}
}
void processEventButton(Button &b, bool newState)
{
	
	if (newState) 
	{
		b.held = true;
		b.pressed = true;
		b.released = false;
	}
	else 
	{
		b.held = false;
		b.pressed = true;
		b.released = true;
	}


}
void asynkButtonClear(Button &b)
{
	b.released = 0;
	b.pressed = 0;
}

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

		RECT rect;
		GetClientRect(wind, &rect);

		StretchDIBits(hdc,
			0, 0, rect.right, rect.bottom,
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
#if INTERNAL_BUILD
			SetLayeredWindowAttributes(wind, RGB(0, 0, 0), 255, LWA_ALPHA);
#endif 

			active = true;
		}
		else
		{
#if INTERNAL_BUILD
			SetLayeredWindowAttributes(wind, RGB(0, 0, 0), 105, LWA_ALPHA);
#endif

			active = false;
		}

	}	break;
	case WM_MENUCHAR:

		rez = MNC_CLOSE<<16;

		break;
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		isDown = 1;
	case WM_SYSKEYUP:
	case WM_KEYUP: 
	{
		bool altWasDown = lp & (1 << 29);

		if(wp == 'W')
		{
			processEventButton(gameInput.up, isDown);
		}
		if(wp == 'S')
		{
			processEventButton(gameInput.down, isDown);
		}
		
		if (wp == 'A')
		{
			processEventButton(gameInput.left, isDown);
		}
		if (wp == 'D')
		{
		 	processEventButton(gameInput.right, isDown);
		}
		if (wp == 'X')
		{
			processEventButton(gameInput.x, isDown);
		}
		if (wp == 'Y')
		{
			processEventButton(gameInput.y, isDown);
		}
		if (wp == VK_ESCAPE)
		{
			processEventButton(gameInput.esc, isDown);
		}
		if (wp == VK_RETURN)
		{
			processEventButton(gameInput.enter, isDown);
		}


#if INTERNAL_BUILD
		if(wp == 'R' && altWasDown && (replayBufferData.recordingState == NOT_RECORDING))
		{ 	
			int slot = 0;

			//start recording
			replayBufferData.recordingState = RECORDING;
			replayBufferData.recordingSlot = slot;

			assert(saveGameState(slot, gameMemory));

			char c[20] = {};
			strcpy(c, "input0");
			c[strlen(c) - 1] += slot;
			strcat(c, ".save");


			replayBufferData.recordingFileHand = CreateFile(c, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
			
			assert(replayBufferData.recordingFileHand != 0
				&& replayBufferData.recordingFileHand != INVALID_HANDLE_VALUE
			);
		
		}
		if(wp == 'S' && altWasDown)

		{
			if(replayBufferData.recordingState == RECORDING)
			{
				CloseHandle(replayBufferData.recordingFileHand);

			}else if(replayBufferData.recordingState == PLAYING)
			{
				UnmapViewOfFile(replayBufferData.fileMappingPointer);
				CloseHandle(replayBufferData.fileMapping);
				CloseHandle(replayBufferData.recordingFileHand);

				gameInput = {};
			}

			replayBufferData.recordingState = NOT_RECORDING;
			
			replayBufferData.recordingFileHand = 0;

			
		}
		
		if (wp == 'P' && altWasDown && (replayBufferData.recordingState == NOT_RECORDING))
		{
			//play recording
			replayBufferData.recordingState = PLAYING;

			int slot = 0;

			loadGameState(slot, gameMemory);

			replayBufferData.recordingSlot = slot;

			char c[20] = {};
			strcpy(c, "input0");
			c[strlen(c) - 1] += slot;
			strcat(c, ".save");


			replayBufferData.recordingFileHand = CreateFile(c, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_ALWAYS, 0, 0);
			replayBufferData.fileMapping = CreateFileMapping(replayBufferData.recordingFileHand, 0, PAGE_READWRITE, 0, 0, 0);

			replayBufferData.fileMappingPointer = MapViewOfFile(replayBufferData.fileMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0);
			
			assert(replayBufferData.fileMappingPointer != 0);

			DWORD high = 0;
			DWORD low = 0;

			low = GetFileSize(replayBufferData.recordingFileHand, &high);

			LARGE_INTEGER la = {};
			la.LowPart = low;
			la.HighPart = high;

			replayBufferData.fileMappingSize = la.QuadPart;
			replayBufferData.fileMappingCursor = 0;
		}
#endif

		rez = DefWindowProc(wind, msg, wp, lp);

	}break;
	default:
		rez = DefWindowProc(wind, msg, wp, lp);
		break;
	}


	return rez;
}


int WINAPI WinMain(HINSTANCE h, HINSTANCE, LPSTR cmd, int show)
{

#if ALLOW_ONLY_ONE_INSTANCE 
//global mutex that lets only one instance of this app run
	CreateMutex(NULL, TRUE, "gameLayerMutex");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		return 0;
	}
#endif

	//console
#if ENABLE_CONSOLE
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

#pragma region load xinput

	win32LoadXinput(xinputData);

#pragma endregion



#pragma region create window

	WNDCLASS wc = {};
	
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hInstance = h;
	wc.lpfnWndProc = windProc;
	wc.lpszClassName = "MainWindowClass";
	wc.style = CS_HREDRAW | CS_VREDRAW;

	RegisterClass(&wc);


	HWND wind = CreateWindowEx
	(
#if INTERNAL_BUILD
		WS_EX_LAYERED |
		WS_EX_TOPMOST |
#endif
		0
		,
		wc.lpszClassName,
		"Geam",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		sizeX * magnifier,
		sizeY * magnifier + 38,
		0,
		0,
		h,
		0
	);
#pragma endregion


	//todo add a guard

	size_t gameMemoryBaseAdress = 0;
	size_t gameMemorySize = MB(25);

	assert(sizeof(GameMemory) <= gameMemorySize);

#if INTERNAL_BUILD
	gameMemoryBaseAdress = TB(2);
#endif



	//gameMemory = (GameMemory*)VirtualAlloc((LPVOID)gameMemoryBaseAdress, gameMemorySize,
	//	MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	gameMemory = (GameMemory*)allocateWithGuard(gameMemorySize, (void*)gameMemoryBaseAdress);

	VolatileMemory* volatileMemory;
	//volatileMemory = (VolatileMemory*)VirtualAlloc(0, sizeof(VolatileMemory),
	//	MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	volatileMemory = (VolatileMemory*)allocateWithGuard(sizeof(VolatileMemory), 0);

	RECT rect;
	GetClientRect(wind, &rect);
	gameWindowBuffer.h = sizeY;
	gameWindowBuffer.w = sizeX;
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

#pragma region time

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
#pragma endregion


#pragma region process messages

		 asynkButtonClear(gameInput.up);
		 asynkButtonClear(gameInput.down);
		 asynkButtonClear(gameInput.left);
		 asynkButtonClear(gameInput.right);
		 asynkButtonClear(gameInput.x);
		 asynkButtonClear(gameInput.y);
		 asynkButtonClear(gameInput.esc);
		 asynkButtonClear(gameInput.enter);

		MSG msg = {};
		while(PeekMessage(&msg, wind, 0, 0, PM_REMOVE) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
	
		}

		bool up = GetAsyncKeyState('W');
		bool down = GetAsyncKeyState('S');
		bool left = GetAsyncKeyState('A');
		bool right = GetAsyncKeyState('D');

		for(int i=0; i<XUSER_MAX_COUNT; i++)	
		{
			if(xinputData.controllerConnected[i])
			{

				up |= xinputData.controllers[i].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP;
				down |= xinputData.controllers[i].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
				left |= xinputData.controllers[i].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
				right |= xinputData.controllers[i].Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;

			}
		
		}

#pragma endregion

#pragma region process xinput

		if (xinputData.xinputLoaded)
		{
			for (int i = 0; i < XUSER_MAX_COUNT; i++)
			{
				if(xinputData.DynamicXinputGetState(i, &xinputData.controllers[i])
					== ERROR_SUCCESS)
				{
					xinputData.controllerConnected[i] = true;
				}else
				{
					xinputData.controllerConnected[i] = false;
				}
			}
			
		}



#pragma endregion

		volatileMemory->reset();

		gameInput.deltaTime = deltaTime;

#if INTERNAL_BUILD
		if(replayBufferData.recordingState == PLAYING)
		{
			memcpy(&gameInput, (char*)replayBufferData.fileMappingPointer + replayBufferData.fileMappingCursor, sizeof(GameInput));
		
			replayBufferData.fileMappingCursor += sizeof(GameInput);
			if (replayBufferData.fileMappingCursor >= replayBufferData.fileMappingSize)
			{
				replayBufferData.fileMappingCursor = 0;
				loadGameState(replayBufferData.recordingSlot, gameMemory);

			}

#if NOT_RECORD_DELTATIME
			gameInput.deltaTime = deltaTime;
#endif	

		}

		if(replayBufferData.recordingState == RECORDING)
		{
			DWORD nrOfBytes = 0;
			WriteFile(replayBufferData.recordingFileHand, &gameInput, sizeof(GameInput), &nrOfBytes, 0);
		}
#endif


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
		FILETIME fileTime2 = {};
		
		fileTime2 = win32GetLastWriteFile(dllName);

#if INTERNAL_BUILD
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
#endif

	}

	CloseWindow(wind);

	return 0;
}