#include "windowsFunctions.h"
#include "utility.h"
#include <gl/glew.h>
#include <gl/wglew.h>
#include <wingdi.h>
#include <string>

using namespace std::string_literals;

//#pragma comment(lib,"opengl32.lib")
extern PlatformFunctions platformFunctions;

static HMODULE dllHand;

HWND globalWind;
HGLRC globalHGLRC;

void win32LoadDll(gameLogic_t** gameLogicPtr, onCreate_t** onCreatePtr, onReload_t** onReloadPtr,
	onClose_t** onClosePtr,
	const char *dllName)
{
#if INTERNAL_BUILD
	std::string newName = dllName;

	int lastLine = 0;

	for(int i=0;i <newName.size();i++)
	{
		if(dllName[i] == '\\')
		{
			lastLine = i;
		}

	}
	
	newName = std::string( newName.begin(), newName.begin() + lastLine);
	newName += "\\gameSetupCopy.dll";


	if (!CopyFile(dllName, newName.c_str(), FALSE) )
	{
		volatile auto err = GetLastError();
	}


	dllHand = LoadLibrary(newName.c_str());
#else
	dllHand = LoadLibrary(dllName);
#endif

	winAssert(dllHand);

	*gameLogicPtr = (gameLogic_t*)GetProcAddress(dllHand, "gameLogic");
	winAssert(*gameLogicPtr);

	*onCreatePtr = (onCreate_t*)GetProcAddress(dllHand, "onCreate");
	winAssert(*onCreatePtr);

	*onReloadPtr = (onReload_t*)GetProcAddress(dllHand, "onReload");
	winAssert(*onReloadPtr);

	*onClosePtr = (onReload_t*)GetProcAddress(dllHand, "onClose");
	winAssert(*onClosePtr);

	OutputDebugString("RELOADED DLL");
}


void win32UnloadDll()
{

	FreeLibrary(dllHand);

}

FILETIME win32GetLastWriteFile(const char* name)
{
	FILETIME time = {};

	WIN32_FILE_ATTRIBUTE_DATA Data;
	if (GetFileAttributesEx(name, GetFileExInfoStandard, &Data))
	{
		time = Data.ftLastWriteTime;
	}
	else
	{
		//	assert(0);
	}

	return(time);

}

bool writeEntireFile(const char *name, void* buffer, size_t size)
{
	HANDLE file = CreateFile(name, GENERIC_WRITE, NULL, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if(file == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	DWORD sizeWritten = 0;
	int rez = 1;

	if(!WriteFile(file, buffer, size, &sizeWritten, NULL))
	{
		rez = 0;
	}

	winAssertComment(size == sizeWritten, "File not written properly");

	CloseHandle(file);

	return rez;
}

bool clearFileContent(const char* name)
{
	bool b = writeEntireFile(name, nullptr, 0);
	return b;
}

#if 0
bool appendToFille(const char *name, void* buffer, size_t size)
{
	HANDLE file = CreateFile(name, FILE_APPEND_DATA, FILE_SHARE_WRITE, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (file == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	DWORD sizeWritten = 0;
	int rez = 1;

	SetFilePointer(file, 0, 0, FILE_END);

	if (!WriteFile(file, buffer, size, &sizeWritten, NULL))
	{
		rez = 0;
	}

	assert(size == sizeWritten);

	CloseHandle(file);

	return rez;
}
#endif

bool readEntireFile(const char *name, void* buffer, size_t size)
{
	HANDLE file = CreateFile(name, GENERIC_READ, NULL, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if(file == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	DWORD sizeRead = 0;

	int rez = 1;

	if(!ReadFile(file, buffer, size, &sizeRead, NULL))
	{
		rez = 0;
	}

	CloseHandle(file);

	return rez;
}

bool saveGameState(int id, GameMemory *gameMemory, HeapMemory *heapMemory)
{
	char fileName[256] = {};
	strcpy(fileName, "mmemory0");
	fileName[sizeof(fileName) - 1] += id;
	strcat(fileName, ".save");

	bool b = writeEntireFile(fileName, gameMemory, sizeof(GameMemory));

	memset(fileName, 0, sizeof(fileName));
	strcpy(fileName, "heap0");
	fileName[sizeof(fileName) - 1] += id;
	strcat(fileName, ".save");

	b &= writeEntireFile(fileName, heapMemory, sizeof(HeapMemory));


	return b;
}

void makeContext()
{
	auto hDC = GetDC(globalWind);
	wglMakeCurrent(hDC, globalHGLRC);
	ReleaseDC(globalWind, hDC);
}

bool loadGameState(int id, GameMemory* gameMemory, HeapMemory *heapMemory)
{
	char fileName[256] = {};
	strcpy(fileName, "mmemory0");
	fileName[sizeof(fileName) - 1] += id;
	strcat(fileName, ".save");

	bool b = readEntireFile(fileName, gameMemory, sizeof(GameMemory));
	
	memset(fileName, 0, sizeof(fileName));
	strcpy(fileName, "heap0");
	fileName[sizeof(fileName) - 1] += id;
	strcat(fileName, ".save");

	b &= readEntireFile(fileName, heapMemory, sizeof(HeapMemory));
	
	return b;
}

bool win32LoadXinput(Win32XinputData &xinputData)
{
	xinputData.xinputLib = LoadLibrary("xinput1_4.dll");
	if (xinputData.xinputLib == NULL)
	{
		xinputData.xinputLib = LoadLibrary("xinput1_3.dll");
	}

	if (xinputData.xinputLib == NULL)
	{
		xinputData.xinputLoaded = 0;
	}
	else
	{
		xinputData.DynamicXinputGetState = (XInputGetState_t*)
			GetProcAddress(xinputData.xinputLib, "XInputGetState");
		xinputData.DynamicXinputSetState = (XInputSetState_t*)
			GetProcAddress(xinputData.xinputLib, "XInputSetState");
		xinputData.DynamicXInputGetKeystroke = (XInputGetKeystroke_t*)
			GetProcAddress(xinputData.xinputLib, "XInputGetKeystroke");

		if (xinputData.DynamicXInputGetKeystroke &&
			xinputData.DynamicXinputGetState &&
			xinputData.DynamicXinputSetState
			)
		{
			xinputData.xinputLoaded = 1;
		}
		else
		{
			xinputData.xinputLoaded = 0;
		}

	}

	return xinputData.xinputLoaded;
}

void enableOpenGL(HWND hwnd, HGLRC* hRC)
{

	/*
	PIXELFORMATDESCRIPTOR pfd = {};

	int iFormat;

	// get the device context (DC) 
	HDC hDC = GetDC(hwnd);


	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 16;
	pfd.cStencilBits = 8;
	pfd.iLayerType = PFD_MAIN_PLANE;

	iFormat = ChoosePixelFormat(hDC, &pfd);

	SetPixelFormat(hDC, iFormat, &pfd);

	// create and enable the render context (RC)
	*hRC = wglCreateContext(hDC);

	wglMakeCurrent(hDC, *hRC);
	*/

	HDC hDC;

	PIXELFORMATDESCRIPTOR pfd = {};
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 16;
	pfd.cStencilBits = 8;
	pfd.cAlphaBits = 8;
	pfd.iLayerType = PFD_MAIN_PLANE;

	if (
		true
		//wglewIsSupported("WGL_ARB_create_context") == 1
		)
	{


		hDC = GetDC(hwnd);

		const int iPixelFormatAttribList[] = {
			
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_COLOR_BITS_ARB, 32,
			WGL_DEPTH_BITS_ARB, 24,
			WGL_STENCIL_BITS_ARB, 8,
			WGL_ALPHA_BITS_ARB, 8,
			WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
			WGL_SAMPLES_ARB, 4,
			0 // End of attributes list
		};
		int attributes[] = {
			//WGL_CONTEXT_MAJOR_VERSION_ARB, 3
			//, WGL_CONTEXT_MINOR_VERSION_ARB, 2
			//, WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
			0
		};

		int nPixelFormat = 0;
		UINT iNumFormats = 0;

		//https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_pixel_format.txt
		wglChoosePixelFormatARB(hDC, iPixelFormatAttribList, NULL, 1, &nPixelFormat, (UINT*)&iNumFormats);

		SetPixelFormat(hDC, nPixelFormat, &pfd);

		//https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_create_context.txt
		*hRC = wglCreateContextAttribsARB(hDC, 0, attributes);

		wglMakeCurrent(hDC, *hRC);

		ReleaseDC(hwnd, hDC);
	}
	else
	{

		int iFormat;

		/* get the device context (DC) */
		hDC = GetDC(hwnd);

		iFormat = ChoosePixelFormat(hDC, &pfd);

		SetPixelFormat(hDC, iFormat, &pfd);

		/* create and enable the render context (RC) */
		*hRC = wglCreateContext(hDC);

		wglMakeCurrent(hDC, *hRC);

		ReleaseDC(hwnd, hDC);
	}


	int multiSample = 0;
	int samples = 0;
	glGetIntegerv(GL_SAMPLE_BUFFERS, &multiSample);
	glGetIntegerv(GL_SAMPLES, &samples);

	platformFunctions.console.blog(("Multi sample window: "s  + std::to_string(multiSample)).c_str());
	platformFunctions.console.blog(("Samples count: "s + std::to_string(samples)).c_str());
	platformFunctions.console.log("");

}

void* allocateWithoutGuard(size_t size, void* basePointer)
{
	void* pointer = VirtualAlloc(basePointer, size,
		MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	
	DWORD err;
	if (pointer == nullptr)
	{
		err = GetLastError();
	}

	return pointer;
}

void* allocateWithGuard(size_t size, void* basePointer)
{
	SYSTEM_INFO info = {};

	GetSystemInfo(&info);

	size_t pageSize = info.dwAllocationGranularity;

	size_t pages = size / pageSize;

	if(size%pageSize != 0)
	{
		pages++;
	}

	void *pointer = VirtualAlloc(basePointer, (pages + 1) * pageSize,
		MEM_RESERVE, PAGE_READWRITE);

	VirtualAlloc(pointer, pages * pageSize, MEM_COMMIT, PAGE_READWRITE);


	VirtualAlloc((char*)pointer + (pages * pageSize), pageSize, MEM_COMMIT, PAGE_NOACCESS);

	return pointer;
}

void setWindowSize(HWND wind, int w, int h)
{
	//https://cboard.cprogramming.com/windows-programming/80365-adjustwindowrect-setting-client-size.html

	RECT winRC, clientRC;
	GetWindowRect(wind, &winRC);
	GetClientRect(wind, &clientRC);
	int dx = (clientRC.right - clientRC.left) - w;
	int dy = (clientRC.bottom - clientRC.top) - h;

	//adjust the size
	SetWindowPos(wind, 0, 0, 0, winRC.right - winRC.left - dx, \
		winRC.bottom - winRC.top - dy, SWP_NOZORDER | SWP_NOMOVE);

}


void resetWindowBuffer(GameWindowBuffer* gameWindowBuffer, BITMAPINFO* bitmapInfo, HWND wind\
,WindowSettings *windowSettings)
{

	RECT rect;
	GetClientRect(wind, &rect);
	gameWindowBuffer->h = rect.bottom;
	gameWindowBuffer->w = rect.right;

	if(gameWindowBuffer->memory)
	{
		VirtualFree(gameWindowBuffer->memory, 0, MEM_RELEASE);
	}

	gameWindowBuffer->memory =
		(char*)VirtualAlloc(0, 4 * gameWindowBuffer->w * gameWindowBuffer->h,
			MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	bitmapInfo->bmiHeader.biSize = sizeof(BITMAPINFO);
	bitmapInfo->bmiHeader.biWidth = gameWindowBuffer->w;
	bitmapInfo->bmiHeader.biHeight = -gameWindowBuffer->h;
	bitmapInfo->bmiHeader.biPlanes = 1;
	bitmapInfo->bmiHeader.biBitCount = 32;
	bitmapInfo->bmiHeader.biCompression = BI_RGB;

	windowSettings->w = gameWindowBuffer->w;
	windowSettings->h = gameWindowBuffer->h;

}
