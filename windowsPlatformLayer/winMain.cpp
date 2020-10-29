#include <Windows.h>
#include "gameStructs.h"
#include "utility.h"
#include <string>
#include <iostream>
#include <stdio.h>
#include "windowsFunctions.h"
#include "buildConfig.h"
#include <Xinput.h>
#include "Console.h"

static bool running = 1;
static bool active = 0;
static BITMAPINFO bitmapInfo = {};
static GameWindowBuffer gameWindowBuffer = {};
static GameMemory* gameMemory = nullptr;
static HeapMemory* heapMemory = nullptr;
static PlatformFunctions platformFunctions;
static char dllName[260];
static GameInput gameInput = {};
static LARGE_INTEGER performanceFrequency;
static WindowSettings windowSettings = {};

static Win32ReplayBufferData replayBufferData;
static Win32XinputData xinputData;

static bool consoleRunning = false;

extern "C"
{
//	_declspec(dllexport) DWORD NvOptimusEnablement = 1;
}

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
		if(b.held)
		{
			b.pressed = false;
		}else
		{
			b.pressed = true;
		}

		b.held = true;
		b.released = false;
	}
	else 
	{
		b.held = false;
		b.pressed = false;
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
	bool isUp = 1;

	switch (msg)
	{
	case WM_CLOSE:
		running = 0;
		//PostQuitMessage(0);
		break;
	case WM_SIZE:
	{

		resetWindowBuffer(&gameWindowBuffer, &bitmapInfo, wind, &windowSettings);
		
	}break;
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
		//to remove an annoying sound
		rez = MNC_CLOSE<<16;
		break;
	case WM_LBUTTONDOWN:
		processEventButton(gameInput.leftMouse, 1);
		break;
	case WM_RBUTTONDOWN:
		processEventButton(gameInput.rightMouse, 1);
		break;
	case WM_LBUTTONUP:
		processEventButton(gameInput.leftMouse, 0);
		break;
	case WM_RBUTTONUP:
		processEventButton(gameInput.rightMouse, 0);
		break;

	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		isDown = 1;
		isUp = 0;
	case WM_SYSKEYUP:
	case WM_KEYUP: 
	{
		bool altWasDown = lp & (1 << 29);

		for (int i = 0; i < Button::BUTTONS_COUNT; i++) 
		{
			if(wp == Button::buttonValues[i])
			{
				processEventButton(gameInput.keyBoard[i], isDown);
			}
		}
		
		if (wp == VK_OEM_3 && altWasDown & isUp)
		{
			consoleRunning = !consoleRunning;
		}
		
#if INTERNAL_BUILD
		if(wp == 'R' && altWasDown && (replayBufferData.recordingState == NOT_RECORDING))
		{ 	
			int slot = 0;

			//start recording
			replayBufferData.recordingState = RECORDING;
			replayBufferData.recordingSlot = slot;

			assert(saveGameState(slot, gameMemory, heapMemory));

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

			loadGameState(slot, gameMemory, heapMemory);

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

#pragma region alocateMemory


	//todo add a guard

	size_t gameMemoryBaseAdress = 0;
	size_t gameMemorySize = sizeof(GameMemory);
	size_t heapMemorySize = sizeof(HeapMemory);

#if INTERNAL_BUILD
	gameMemoryBaseAdress = TB(1);
#endif


	//gameMemory = (GameMemory*)VirtualAlloc((LPVOID)gameMemoryBaseAdress, gameMemorySize,
	//	MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	char* memBlock = (char*)allocateWithoutGuard(gameMemorySize + heapMemorySize + 8, (void*)gameMemoryBaseAdress);

	gameMemory = (GameMemory*)memBlock;
	heapMemory = (HeapMemory*)(&memBlock[sizeof(GameMemory)]); //todo align 8

	heapMemory->allocator.init(heapMemory->memory, sizeof(heapMemory->memory));

	VolatileMemory* volatileMemory;
	//volatileMemory = (VolatileMemory*)VirtualAlloc(0, sizeof(VolatileMemory),
	//	MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	volatileMemory = (VolatileMemory*)allocateWithGuard(sizeof(VolatileMemory), 0);

#pragma endregion

#pragma region loadDll
	
	gameLogic_t* gameLogic_ptr;
	onCreate_t* onCreate_ptr;


	FILETIME lastFileTime = win32GetLastWriteFile(dllName);

	win32LoadDll(&gameLogic_ptr, &onCreate_ptr, dllName);

#pragma endregion

#pragma region print to console
	platformFunctions.console.glog("- Platform layer console -");
	platformFunctions.console.glog("----- Luta Vlad (c) ------");
	platformFunctions.console.glog("press ALT + ~ or ` to exit");
	platformFunctions.console.writeText("\n");
#pragma endregion

#pragma region set platform functions pointers

	platformFunctions.readEntirFile = readEntireFile;
	platformFunctions.writeEntireFile = writeEntireFile;

#pragma endregion

#pragma region call game init
	onCreate_ptr(gameMemory, heapMemory, &windowSettings, &platformFunctions);
#pragma endregion

#pragma region create window

	WNDCLASS wc = {};

	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hInstance = h;
	wc.lpfnWndProc = windProc;
	wc.lpszClassName = "MainWindowClass";
	wc.style = CS_HREDRAW | CS_VREDRAW;

	RegisterClass(&wc);

	int multiplier = 1;

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
		windowSettings.w,
		windowSettings.h,
		0,
		0,
		h,
		0
	);

	setWindowSize(wind, windowSettings.h, windowSettings.w);
	resetWindowBuffer(&gameWindowBuffer, &bitmapInfo, wind, &windowSettings);

#pragma endregion

#pragma region enable opengl
	HGLRC hrc;
	enableOpenGL(wind, &hrc);
#pragma endregion


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

		//todo
		//if (replayBufferData.recordingState != PLAYING)
		{
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
		}

	
#pragma endregion


#pragma region process messages & input
		
		gameInput.windowActive = active;
		
		POINT p;
		GetCursorPos(&p);
		ScreenToClient(wind, &p);
		gameInput.mouseX = p.x;
		gameInput.mouseY = p.y;

		if (GetAsyncKeyState(VK_LBUTTON) == 0)
		{
			gameInput.leftMouse = {};
		}
		if (GetAsyncKeyState(VK_RBUTTON) == 0)
		{
			gameInput.rightMouse = {};
		}

		if(!active)
		{
			for (int i = 0; i < Button::BUTTONS_COUNT; i++)
			{
				gameInput.keyBoard[i] = {};
			}

			gameInput.leftMouse = {};
			gameInput.rightMouse = {};

		}else
		{

			for (int i = 0; i < Button::BUTTONS_COUNT; i++)
			{
				asynkButtonClear(gameInput.keyBoard[i]);
			}

			asynkButtonClear(gameInput.leftMouse);
			asynkButtonClear(gameInput.rightMouse);
		
		}


		MSG msg = {};
		while(PeekMessage(&msg, wind, 0, 0, PM_REMOVE) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
	
		}


#pragma endregion

#pragma region process xinput

		if (xinputData.xinputLoaded)
		{
			for (int i = 0; i < XUSER_MAX_COUNT; i++)
			{
				if (xinputData.DynamicXinputGetState(i, &xinputData.controllers[i])
					== ERROR_SUCCESS)
				{
					xinputData.controllerConnected[i] = true;
				}
				else
				{
					xinputData.controllerConnected[i] = false;
				}
			}

			gameInput.anyController = {};

			for (int i = 0; i < 4; i++)
			{
				if (xinputData.controllerConnected[i])
				{

					asynkButtonClear(gameInput.controllers[i].A);
					asynkButtonClear(gameInput.controllers[i].B);
					asynkButtonClear(gameInput.controllers[i].X);
					asynkButtonClear(gameInput.controllers[i].Y);

					asynkButtonClear(gameInput.controllers[i].Up);
					asynkButtonClear(gameInput.controllers[i].Down);
					asynkButtonClear(gameInput.controllers[i].Left);
					asynkButtonClear(gameInput.controllers[i].Riight);

					asynkButtonClear(gameInput.controllers[i].Menu);
					asynkButtonClear(gameInput.controllers[i].Back);

					asynkButtonClear(gameInput.controllers[i].LPress);
					asynkButtonClear(gameInput.controllers[i].RPress);

					asynkButtonClear(gameInput.controllers[i].RB);
					asynkButtonClear(gameInput.controllers[i].LB);


					processAsynkButton(gameInput.controllers[i].A,
						xinputData.controllers[i].Gamepad.wButtons & XINPUT_GAMEPAD_A);

					processAsynkButton(gameInput.controllers[i].B,
						xinputData.controllers[i].Gamepad.wButtons & XINPUT_GAMEPAD_B);
					
					processAsynkButton(gameInput.controllers[i].X,
						xinputData.controllers[i].Gamepad.wButtons & XINPUT_GAMEPAD_X);

					processAsynkButton(gameInput.controllers[i].Y,
						xinputData.controllers[i].Gamepad.wButtons& XINPUT_GAMEPAD_Y);
					//
					processAsynkButton(gameInput.controllers[i].Up,
						xinputData.controllers[i].Gamepad.wButtons& XINPUT_GAMEPAD_DPAD_UP);

					processAsynkButton(gameInput.controllers[i].Down,
						xinputData.controllers[i].Gamepad.wButtons& XINPUT_GAMEPAD_DPAD_DOWN);

					processAsynkButton(gameInput.controllers[i].Left,
						xinputData.controllers[i].Gamepad.wButtons& XINPUT_GAMEPAD_DPAD_LEFT);

					processAsynkButton(gameInput.controllers[i].Riight,
						xinputData.controllers[i].Gamepad.wButtons& XINPUT_GAMEPAD_DPAD_RIGHT);
				
					//
					processAsynkButton(gameInput.controllers[i].Menu,
						xinputData.controllers[i].Gamepad.wButtons& XINPUT_GAMEPAD_START);

					processAsynkButton(gameInput.controllers[i].Back,
						xinputData.controllers[i].Gamepad.wButtons& XINPUT_GAMEPAD_BACK);

					processAsynkButton(gameInput.controllers[i].LPress,
						xinputData.controllers[i].Gamepad.wButtons& XINPUT_GAMEPAD_LEFT_THUMB);

					processAsynkButton(gameInput.controllers[i].RPress,
						xinputData.controllers[i].Gamepad.wButtons& XINPUT_GAMEPAD_RIGHT_THUMB);
				
					//
					processAsynkButton(gameInput.controllers[i].RB,
						xinputData.controllers[i].Gamepad.wButtons& XINPUT_GAMEPAD_RIGHT_SHOULDER);

					processAsynkButton(gameInput.controllers[i].LB,
						xinputData.controllers[i].Gamepad.wButtons& XINPUT_GAMEPAD_RIGHT_SHOULDER);

					gameInput.controllers[i].LT = xinputData.controllers->Gamepad.bLeftTrigger / 255.f;
					gameInput.controllers[i].RT = xinputData.controllers->Gamepad.bRightTrigger / 255.f;
					//todo tresshold here
					
					gameInput.controllers[i].LThumb.x = xinputData.controllers->Gamepad.sThumbLX / (float)0x7f'ff;
					gameInput.controllers[i].LThumb.y = xinputData.controllers->Gamepad.sThumbLY / (float)0x7f'ff;
					
					gameInput.controllers[i].RThumb.x = xinputData.controllers->Gamepad.sThumbRX / (float)0x7f'ff;
					gameInput.controllers[i].RThumb.y = xinputData.controllers->Gamepad.sThumbRY / (float)0x7f'ff;

					//todo tresshold here
					if (abs(gameInput.controllers[i].RThumb.x) < 0.2)
					{
						gameInput.controllers[i].RThumb.x = 0;
					}
					if (abs(gameInput.controllers[i].RThumb.y) < 0.2)
					{
						gameInput.controllers[i].RThumb.y = 0;
					}

					if (abs(gameInput.controllers[i].LThumb.x) < 0.2)
					{
						gameInput.controllers[i].LThumb.x = 0;
					}
					if (abs(gameInput.controllers[i].LThumb.y) < 0.2)
					{
						gameInput.controllers[i].LThumb.y = 0;
					}

					gameInput.anyController.merge(gameInput.controllers[i]);

				}
				else
				{

					gameInput.controllers[i] = {};
				}

			}

		}


#pragma endregion

		if (!consoleRunning) 
		{
#pragma region checkForChanges

			if (((windowSettings.h != gameWindowBuffer.h)
				|| (windowSettings.h != gameWindowBuffer.h)
				) && windowSettings.h != 0
				&& windowSettings.w != 0)
			{
				setWindowSize(wind, windowSettings.h, windowSettings.w);
				resetWindowBuffer(&gameWindowBuffer, &bitmapInfo, wind, &windowSettings);
			}
			else
			{
				windowSettings.h = gameWindowBuffer.h;
				windowSettings.w = gameWindowBuffer.w;
			}


#pragma endregion


			volatileMemory->reset();

			gameInput.deltaTime = deltaTime;

			//playback recording
#if INTERNAL_BUILD 
			if (replayBufferData.recordingState == PLAYING)
			{
				memcpy(&gameInput, (char*)replayBufferData.fileMappingPointer + replayBufferData.fileMappingCursor, sizeof(GameInput));

				replayBufferData.fileMappingCursor += sizeof(GameInput);
				if (replayBufferData.fileMappingCursor >= replayBufferData.fileMappingSize)
				{
					replayBufferData.fileMappingCursor = 0;
					loadGameState(replayBufferData.recordingSlot, gameMemory, heapMemory);

				}

#if NOT_RECORD_DELTATIME
				gameInput.deltaTime = deltaTime;
#endif	

			}

			if (replayBufferData.recordingState == RECORDING)
			{
				DWORD nrOfBytes = 0;
				WriteFile(replayBufferData.recordingFileHand, &gameInput, sizeof(GameInput), &nrOfBytes, 0);
			}
#endif

			//execute game logic
			gameLogic_ptr(&gameInput, gameMemory, heapMemory, volatileMemory, &gameWindowBuffer,
				&windowSettings, &platformFunctions);

		}else // run the console
		{
			drawConsole(&gameWindowBuffer, &platformFunctions.console);
		}


#pragma region draw screen
		
		if(windowSettings.drawWithOpenGl && !consoleRunning)
		{

			HDC hdc = GetDC(wind);
			SwapBuffers(hdc);
			ReleaseDC(wind, hdc);
		}else
		{

			//todo repair
			RECT r;
			GetClientRect(wind, &r);
			int w = r.left;
			int h = r.bottom;
			HDC hdc = GetDC(wind);
			PatBlt(hdc, 0, 0, w, h, BLACKNESS);
			ReleaseDC(wind, hdc);
			
			SendMessage(wind, WM_PAINT, 0, 0);
		}
		

#pragma endregion

	//check if game code changed	
#if INTERNAL_BUILD 
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

			win32LoadDll(&gameLogic_ptr, &onCreate_ptr, dllName);

		}
#endif

#if ENABLE_CONSOLE //write to the console
		auto& console = platformFunctions.console;
	
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { 0,0 });
		
		for(int i = console.bufferBeginPos; 
			i< console.BUFFER_SIZE; i++)
		{
			if(console.buffer[i].c)
				std::cout << console.buffer[i].c;
		}

		for (int i = 0;
			i < console.bufferBeginPos; i++)
		{
			if (console.buffer[i].c)
				std::cout << console.buffer[i].c;
		}

#endif



	}

	CloseWindow(wind);

	return 0;
}