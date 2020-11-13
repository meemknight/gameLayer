#pragma once
#include <Windows.h>
#include "gameStructs.h"
#include <Xinput.h>

#if ENABLE_ASSERT

#define win32Assert(x) if(!(x)){std::cout<< __LINE__ << " " << GetLastError(); DebugBreak();}

#else 

#define win32Assert(x) 

#endif


void win32LoadDll(gameLogic_t * *gameLogicPtr, onCreate_t * *onCreatePtr, onReload_t * *onReloadPtr,
	onClose_t * *onClosePtr, const char* dllName);
void win32UnloadDll();

FILETIME win32GetLastWriteFile(const char* name);



bool clearFileContent(const char* name);
//bool appendToFille(const char* name, void* buffer, size_t size);

bool saveGameState(int id, GameMemory* gameMemory, HeapMemory *heapMemory);
bool loadGameState(int id, GameMemory* gameMemory, HeapMemory *heapMemory);

void* allocateWithoutGuard(size_t size, void* basePointer);
void* allocateWithGuard(size_t size, void *basePointer);

void setWindowSize(HWND wind, int w, int h);
void resetWindowBuffer(GameWindowBuffer * gameWindowBuffer, \
	BITMAPINFO * bitmapInfo, HWND wind, WindowSettings * windowSettings);


#define NOT_RECORDING 0
#define RECORDING 1
#define PLAYING 2


struct Win32ReplayBufferData
{

	int recordingState = NOT_RECORDING;
	int recordingSlot = 0;
	HANDLE recordingFileHand = 0;
	HANDLE fileMapping = 0;
	void* fileMappingPointer = nullptr;
	size_t fileMappingSize = 0;
	size_t fileMappingCursor = 0;

};

typedef DWORD WINAPI XInputGetState_t(DWORD dwUserIndex, XINPUT_STATE* pState);
typedef DWORD WINAPI XInputSetState_t(DWORD dwUserIndex, XINPUT_VIBRATION* pState);
typedef DWORD WINAPI XInputGetKeystroke_t(DWORD dwUserIndex, DWORD dwReserved, PXINPUT_KEYSTROKE pKeystroke);

struct Win32XinputData
{
	bool xinputLoaded = 0;
	HMODULE xinputLib = 0;
	XInputGetState_t* DynamicXinputGetState;
	XInputSetState_t* DynamicXinputSetState;
	XInputGetKeystroke_t* DynamicXInputGetKeystroke;

	XINPUT_STATE controllers[XUSER_MAX_COUNT] = {};
	bool controllerConnected[XUSER_MAX_COUNT] = {};

};

bool win32LoadXinput(Win32XinputData& xinputData);

void enableOpenGL(HWND hwnd, HGLRC* hRC);

