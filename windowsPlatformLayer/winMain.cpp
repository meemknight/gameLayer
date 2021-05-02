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
#include <algorithm>
#include "Audio.h"

#pragma region globals

static bool running = 1;
static bool active = 0;
static BITMAPINFO bitmapInfo = {};
static GameWindowBuffer gameWindowBuffer = {};
static GameWindowBuffer fpsCounterBuffer = {};

static GameMemory* gameMemory = nullptr;
static HeapMemory* heapMemory = nullptr;
PlatformFunctions platformFunctions;
static char dllName[260];	//todo refactor
static GameInput gameInput = {};
static LARGE_INTEGER performanceFrequency;
static WindowSettings windowSettings;

static Win32ReplayBufferData replayBufferData;
static Win32XinputData xinputData;

static bool consoleRunning = false;

extern HWND globalWind;
extern HGLRC globalHGLRC;

const char* windowName = "Geam";

Audio audio = {};

#pragma endregion

#pragma region gpu
extern "C"
{
	__declspec(dllexport) unsigned long NvOptimusEnablement = USE_GPU_ENGINE;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = USE_GPU_ENGINE;
}
#pragma endregion


#pragma region audio functions

void winSetMasterVolume(float volume) 
{
	audio.setMasterVolume(volume);
}


void winPlaySound(const char* name, float volume) 
{
	audio.playSound(name, volume);
}


void winKeepPlayingMusic(const char* name, float volume)
{
	audio.keepPlayingMusic(name, volume);
}

#pragma endregion


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

#pragma region fullScreen

WINDOWPLACEMENT windowPlacementPrev = { sizeof(windowPlacementPrev) };
static bool fullscreen = false;
static float fullScreenZoom = 1;

void setupFullscreen()
{

	DISPLAY_DEVICE displayDevice = {};
	displayDevice.cb = sizeof(displayDevice);
	bool foundPrimaryDevice = false;

	for (int i = 0; EnumDisplayDevices(nullptr, i, &displayDevice, EDD_GET_DEVICE_INTERFACE_NAME); i++)
	{
		if ((displayDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) != 0)
		{
			foundPrimaryDevice = true;
			break;
		}

	}

	const char* monitorName = nullptr;

	if (foundPrimaryDevice)
	{
		monitorName = displayDevice.DeviceName;
	}

	DEVMODE monitorSettings = { };
	monitorSettings.dmSize = sizeof(monitorSettings);
	monitorSettings.dmDriverExtra = 0;
	bool found = 0;

	//for (int i = 0; EnumDisplaySettings(monitorName, i, &monitorSettings); i++)
	//{
	//
	//	const char* c = (monitorSettings.dmDisplayFixedOutput == DMDFO_DEFAULT) ? "DMDFO_DEFAULT" :
	//		((monitorSettings.dmDisplayFixedOutput == DMDFO_STRETCH)? "DMDFO_STRETCH" : "DMDFO_CENTER");
	//
	//
	//	std::cout << c << " " << monitorSettings.dmPelsWidth << " " << monitorSettings.dmPelsHeight << "\n";
	//
	//}

	std::vector<std::pair<DEVMODE, float>> validMonitorSettings;
	validMonitorSettings.reserve(10);

	//https://en.wikipedia.org/wiki/Display_resolution

	//16:9
	glm::vec4 sizes16_9[] =
	{
		{3840, 2160, 8.294, 1},	//4K UHD
		{2560, 1440, 3.686, 1},	//QHD
		{2048, 1152, 2.359, 1},	//QWXGA
		{1920, 1080, 2.074, 1},	//FHD
		{1600, 900,  1.440, 1},	//HD+
		{1536, 864,  1.327, 1},	//Other
		{1366, 768,  1.049, 1},	//HD ~16:9
		{1360, 768,  1.044, 1},	//HD ~16:9
		{1280, 720,  0.922, 1},	//WXGA
		{640, 360,   0.230, 1},	//nHD
	};

	for (int j = 0; j < sizeof(sizes16_9) / sizeof(sizes16_9[0]); j++) 
	{
		sizes16_9[j].w = sizes16_9[j].x / 1920.f;
	
	}

	for (int i = 0; EnumDisplaySettings(monitorName, i, &monitorSettings); i++)
	{

		for(int j=0;j<sizeof(sizes16_9)/sizeof(sizes16_9[0]);j++)
		{
			if (monitorSettings.dmDisplayFixedOutput == DMDFO_DEFAULT
				&& monitorSettings.dmPelsWidth == sizes16_9[j].x
				&& monitorSettings.dmPelsHeight == sizes16_9[j].y)
			{
				validMonitorSettings.push_back({ monitorSettings, sizes16_9[j].w });
			}
		
		}


	}

	std::sort(validMonitorSettings.begin(), validMonitorSettings.end(), 
		[](const std::pair<DEVMODE, float> &a, std::pair<DEVMODE, float>&b) {return a.second > b.second; });

	int i = 0;
	if(validMonitorSettings.size() > i)
	{
		//std::cout << "Current mode: " << validMonitorSettings[i].first.dmPelsWidth <<
		//	" " << validMonitorSettings[i].first.dmPelsHeight << "  zoom: " << validMonitorSettings[i].second << "\n";

		ChangeDisplaySettings(&validMonitorSettings[i].first, CDS_FULLSCREEN);
		fullScreenZoom = validMonitorSettings[i].second;
	}

	
	//to get the zoom use this: actualScreenW / 1920.f;
	//todo implement

}


#pragma endregion

LRESULT CALLBACK tempWindProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, message, wParam, lParam);
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
	case WM_ACTIVATE:
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
			SetLayeredWindowAttributes(wind, RGB(0, 0, 0), 200, LWA_ALPHA);
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

			if (consoleRunning) 
			{
				resetConsole(&gameWindowBuffer, &platformFunctions.console);
			}
		}
		
#if INTERNAL_BUILD
		if(wp == 'R' && altWasDown && (replayBufferData.recordingState == NOT_RECORDING))
		{ 	
			int slot = 0;

			//start recording
			replayBufferData.recordingState = RECORDING;
			replayBufferData.recordingSlot = slot;

			if(!saveGameState(slot, gameMemory, heapMemory))
				winAssertComment(0, "Error saving game state");

			char c[20] = {};
			strcpy(c, "input0");
			c[strlen(c) - 1] += slot;
			strcat(c, ".save");


			replayBufferData.recordingFileHand = CreateFile(c, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
			
			winAssertComment(replayBufferData.recordingFileHand != 0
				&& replayBufferData.recordingFileHand != INVALID_HANDLE_VALUE
			,"Error creating replay file");
		
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
			
			winAssert(replayBufferData.fileMappingPointer != 0);

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
		//so altf4 works
		rez = DefWindowProc(wind, msg, wp, lp);

	}break;
	default:
		rez = DefWindowProc(wind, msg, wp, lp);
		break;
	}


	return rez;
}

#pragma region signal

void SignalHandler(int signal)
{
	if (signal == SIGABRT)
	{

		

		_exit(3);
	}
	
}

#pragma endregion

int WINAPI WinMain(HINSTANCE h, HINSTANCE, LPSTR cmd, int show)
{

#if ALLOW_ONLY_ONE_INSTANCE 
//global mutex that lets only one instance of this app run
	CreateMutex(NULL, TRUE, "Global\\gameLayerMutex");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		return 0;
	}
#endif

#pragma region console
	
#if ENABLE_CONSOLE
	AllocConsole();
	freopen("conin$", "r", stdin);
	freopen("conout$", "w", stdout);
	freopen("conout$", "w", stderr);
	std::cout.sync_with_stdio();

#endif

	platformFunctions.console.glog("- Platform layer console -");
	platformFunctions.console.glog("----- Luta Vlad (c) ------");
	platformFunctions.console.glog("press ALT + ~ or ` to exit");
	platformFunctions.console.writeText("\n");
#pragma endregion

#pragma region set platform functions pointers

	platformFunctions.readEntirFile = readEntireFile;
	platformFunctions.writeEntireFile = writeEntireFile;
	platformFunctions.makeContext = makeContext;

#pragma endregion

#pragma region enable audio

	audio.initAudioDrivers();

#pragma endregion

#pragma region signal

	signal(SIGABRT, SignalHandler);

#pragma endregion

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
	heapMemory = (HeapMemory*)(&memBlock[sizeof(GameMemory)]);

	heapMemory->allocator.init(heapMemory->memory, sizeof(heapMemory->memory));

	VolatileMemory* volatileMemory;
	//volatileMemory = (VolatileMemory*)VirtualAlloc(0, sizeof(VolatileMemory),
	//	MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	volatileMemory = (VolatileMemory*)allocateWithGuard(sizeof(VolatileMemory), 0);

#pragma endregion

#pragma region loadDll
	
	gameLogic_t* gameLogic_ptr;
	onCreate_t* onCreate_ptr;
	onReload_t* onReload_ptr;
	onClose_t* onClose_ptr;

	FILETIME lastFileTime = win32GetLastWriteFile(dllName);

	win32LoadDll(&gameLogic_ptr, &onCreate_ptr, &onReload_ptr, &onClose_ptr, dllName);

#pragma endregion

#pragma region fake window
	{
		//---- fake Window
		WNDCLASSEX wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wcex.lpfnWndProc = tempWindProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = h;
		wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = "coco";
		wcex.hIconSm = NULL;

		if (!RegisterClassEx(&wcex))
		{
			return 0;
		}

		HWND hwnd = CreateWindow(
			"coco",
			"dddd",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT,
			500, 500,
			NULL,
			NULL,
			h,
			NULL
		);

		HDC hdc = GetDC(hwnd);
		PIXELFORMATDESCRIPTOR pfd = {};

		pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.cDepthBits = 32;
		pfd.iLayerType = PFD_MAIN_PLANE;

		int nPixelFormat = ChoosePixelFormat(hdc, &pfd);

		SetPixelFormat(hdc, nPixelFormat, &pfd);

		HGLRC hrc = wglCreateContext(hdc);

		wglMakeCurrent(hdc, hrc);

		glewExperimental = true;
		if (glewInit() != GLEW_OK)
		{
			MessageBoxA(0, "glewInit", "Error from glew", MB_ICONERROR);
			return 1;
		}

		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(hrc);
		ReleaseDC(hwnd, hdc);
		DestroyWindow(hwnd);

	}
#pragma endregion

#pragma region create window

	WNDCLASS wc = {};

	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hInstance = h;
	wc.lpfnWndProc = windProc;
	wc.lpszClassName = "MainWindowClass";
	wc.style = CS_HREDRAW | CS_VREDRAW 
#if !INTERNAL_BUILD
		| CS_OWNDC
#endif
		;

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
		windowName,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		windowSettings.w,
		windowSettings.h,
		0,
		0,
		h,
		0
	);

	setWindowSize(wind, windowSettings.w, windowSettings.h);
	resetWindowBuffer(&gameWindowBuffer, &bitmapInfo, wind, &windowSettings);

	fpsCounterBuffer.w = 8 * 8 * 3;
	fpsCounterBuffer.h = 1 * 8 * 3;
	fpsCounterBuffer.memory =
		(char*)VirtualAlloc(0, 4 * fpsCounterBuffer.w * fpsCounterBuffer.h,
			MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	fpsCounterBuffer.clear(10, 40, 85);

#pragma endregion

#pragma region enable opengl
	HGLRC hrc;
	enableOpenGL(wind, &hrc);
	globalWind = wind;
	globalHGLRC = hrc;
	
#pragma endregion

#pragma region call game init
	onCreate_ptr(gameMemory, heapMemory, &windowSettings, &platformFunctions);
	setWindowSize(wind, windowSettings.w, windowSettings.h);
	resetWindowBuffer(&gameWindowBuffer, &bitmapInfo, wind, &windowSettings);

	ShowWindow(wind, SW_NORMAL);
#pragma endregion


#pragma region time

	QueryPerformanceFrequency(&performanceFrequency);
	LARGE_INTEGER time1;
	LARGE_INTEGER time2;
	LARGE_INTEGER time3;
	LARGE_INTEGER time4;

	int dtCounter = 0;
	int currentFrameCount = 1;
	float dtSecondCounter = 1;

	QueryPerformanceCounter(&time1);
	QueryPerformanceCounter(&time3);

#pragma endregion

	while (running)
	{

#pragma region time

		//todo lock to sthing like 300 fps anyway
		//todo switch gl vendod and load the vsync function
		//check if it synk function works and if not lock manually
		if (windowSettings.lockTo60fps)
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
				QueryPerformanceCounter(&time1);
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
				QueryPerformanceCounter(&time1);
			}
		}

		QueryPerformanceCounter(&time4);
		LARGE_INTEGER deltaTimeInteger;
		deltaTimeInteger.QuadPart = time4.QuadPart - time3.QuadPart;

		float deltaTime = (float)deltaTimeInteger.QuadPart / (float)performanceFrequency.QuadPart;
		QueryPerformanceCounter(&time3);
	
		dtSecondCounter -= deltaTime;
		dtCounter++;

		if (dtSecondCounter < 0)
		{
			dtSecondCounter++;
			currentFrameCount = dtCounter;
			dtCounter = 0;
			//std::cout << currentFrameCount << '\n';

			SetWindowText(wind, (std::string(windowName) + " - " + std::to_string(currentFrameCount)
				+ "fps"
				).c_str());

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
					auto tresshold = [](float& v, float min, float max, float setMin, float setMax)
					{
						if (v <= min) { v = setMin; }
						if (v >= max) { v = setMax; }
					};

					auto tressholdNegAndPoz = [](float& v, float min, float max, float setMin, float setMax)
					{
						if(v > 0)
						{
							if (v <= min) { v = setMin; }
							if (v >= max) { v = setMax; }
						}else
						{
							if (v >= -min) { v = -setMin; }
							if (v <= -max) {v = -setMax;}
						}

					};

					asynkButtonClear(gameInput.controllers[i].A);
					asynkButtonClear(gameInput.controllers[i].B);
					asynkButtonClear(gameInput.controllers[i].X);
					asynkButtonClear(gameInput.controllers[i].Y);

					asynkButtonClear(gameInput.controllers[i].Up);
					asynkButtonClear(gameInput.controllers[i].Down);
					asynkButtonClear(gameInput.controllers[i].Left);
					asynkButtonClear(gameInput.controllers[i].Right);

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

					processAsynkButton(gameInput.controllers[i].Right,
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

					tresshold(gameInput.controllers[i].LT, 0.2f, 0.9f, 0.f, 1.f);
					tresshold(gameInput.controllers[i].RT, 0.2f, 0.9f, 0.f, 1.f);

					
					gameInput.controllers[i].LThumb.x = xinputData.controllers->Gamepad.sThumbLX / (float)0x7f'ff;
					gameInput.controllers[i].LThumb.y = xinputData.controllers->Gamepad.sThumbLY / (float)0x7f'ff;
					
					tressholdNegAndPoz(gameInput.controllers[i].LThumb.x, 0.2, 0.9, 0, 1);
					tressholdNegAndPoz(gameInput.controllers[i].LThumb.y, 0.2, 0.9, 0, 1);


					gameInput.controllers[i].RThumb.x = xinputData.controllers->Gamepad.sThumbRX / (float)0x7f'ff;
					gameInput.controllers[i].RThumb.y = xinputData.controllers->Gamepad.sThumbRY / (float)0x7f'ff;

					tressholdNegAndPoz(gameInput.controllers[i].RThumb.x, 0.2, 0.9, 0, 1);
					tressholdNegAndPoz(gameInput.controllers[i].RThumb.y, 0.2, 0.9, 0, 1);



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

#pragma region music

			audio.updateAudioStream();
			platformFunctions.setMasterVolume = &winSetMasterVolume;
			platformFunctions.playSound = &winPlaySound;
			platformFunctions.keepPlayingMusic = &winKeepPlayingMusic;

#pragma endregion


#pragma region checkForChanges
			
			if(windowSettings.fullScreen)
			{
				
			}else
			{
				if (((windowSettings.h != gameWindowBuffer.h)
					|| (windowSettings.w != gameWindowBuffer.w)
					) && windowSettings.h != 0
					&& windowSettings.w != 0)
				{
					setWindowSize(wind, windowSettings.w, windowSettings.h);
					resetWindowBuffer(&gameWindowBuffer, &bitmapInfo, wind, &windowSettings);
				}
				else
				{
					windowSettings.h = gameWindowBuffer.h;
					windowSettings.w = gameWindowBuffer.w;
				}

			}
			
		

#pragma endregion


#pragma region clamp Delta time

			float clampedDeltaTime = deltaTime;

			if(clampedDeltaTime < 1/10)
			{
				clampedDeltaTime = 1/10.f;
			}
			
			gameInput.deltaTime = clampedDeltaTime;

#pragma endregion

#pragma region playback recording

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

#pragma endregion

#pragma region game logic

			volatileMemory->reset();

			//execute game logic
			gameLogic_ptr(&gameInput, gameMemory, heapMemory, volatileMemory, &gameWindowBuffer,
				&windowSettings, &platformFunctions);

#pragma endregion


		}else // run the console
		{
#pragma region draw console

			drawConsole(&gameWindowBuffer, &platformFunctions.console);

#pragma endregion
		}


#pragma region draw screen
		
		if(windowSettings.drawWithOpenGl && !consoleRunning)
		{

			HDC hdc = GetDC(wind);
			SwapBuffers(hdc);
			//ReleaseDC(wind, hdc);

			//HDC hdc = GetDC(wind);

			//StretchDIBits(hdc,
			//	0, 0, fpsCounterBuffer.w, fpsCounterBuffer.h,
			//	0, 0, fpsCounterBuffer.w, fpsCounterBuffer.h,
			//	fpsCounterBuffer.memory,
			//	&bitmapInfo,
			//	DIB_RGB_COLORS,
			//	SRCCOPY
			//);

			ReleaseDC(wind, hdc);

		}else
		{

			
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

#pragma region check if game code changed	
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

			win32LoadDll(&gameLogic_ptr, &onCreate_ptr, &onReload_ptr, &onClose_ptr, dllName);
			onReload_ptr(gameMemory, heapMemory, &windowSettings, &platformFunctions);
			setWindowSize(wind, windowSettings.w, windowSettings.h);
			resetWindowBuffer(&gameWindowBuffer, &bitmapInfo, wind, &windowSettings);
		}
#endif
#pragma endregion

#pragma region fullScreen

	if(fullscreen != windowSettings.fullScreen)
	{
		DWORD dwStyle = GetWindowLong(wind, GWL_STYLE);
		if (dwStyle & (WS_OVERLAPPEDWINDOW))
		{
			MONITORINFO mi = { sizeof(mi) };
			if (
				GetWindowPlacement(wind, &windowPlacementPrev) &&
				GetMonitorInfo(MonitorFromWindow(wind,
					//MONITOR_DEFAULTTONEAREST
					MONITOR_DEFAULTTOPRIMARY
				), &mi)
				)
			{
				setupFullscreen();

				SetWindowLong(wind, GWL_STYLE,
					dwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(wind, HWND_TOP,
					mi.rcMonitor.left, mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left,
					mi.rcMonitor.bottom - mi.rcMonitor.top,
					SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

				fullscreen = 1;
				windowSettings.fullScreen = 1;

				resetWindowBuffer(&gameWindowBuffer, &bitmapInfo, wind, &windowSettings);
			}
			
		}
		else
		{
			ChangeDisplaySettings(nullptr, 0);


			SetWindowLong(wind, GWL_STYLE,
				dwStyle | (WS_OVERLAPPEDWINDOW));

			SetWindowPlacement(wind, &windowPlacementPrev);
			SetWindowPos(wind, NULL, 0, 0, 0, 0,
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
				SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
			
			fullscreen = 0;
			windowSettings.fullScreen = 0;


			resetWindowBuffer(&gameWindowBuffer, &bitmapInfo, wind, &windowSettings);
		}
	}

	
	if (windowSettings.fullScreen) 
	{
		windowSettings.h = gameWindowBuffer.h;
		windowSettings.w = gameWindowBuffer.w;
		windowSettings.fullScreenZoon = fullScreenZoom;
	}else
	{
		windowSettings.fullScreenZoon = 1;
		fullScreenZoom = 1;
	}


#pragma endregion

	}

#pragma region close game

	onClose_ptr(gameMemory, heapMemory, &windowSettings, &platformFunctions);

	CloseWindow(wind);

#pragma endregion


	return 0;
}