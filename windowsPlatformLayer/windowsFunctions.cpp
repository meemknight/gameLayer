#include "windowsFunctions.h"
#include "utility.h"

static HMODULE dllHand;

void win32LoadDll(gameLogic_t** gameLogicPtr, const char *dllName)
{
#if INTERNAL_BUILD
	assert(CopyFile(dllName, "gameSetupCopy.dll", FALSE));

	dllHand = LoadLibrary("gameSetupCopy.dll");
#else
	dllHand = LoadLibrary(dllName);
#endif

	assert(dllHand);

	*gameLogicPtr = (gameLogic_t*)GetProcAddress(dllHand, "gameLogic");

	assert(*gameLogicPtr);

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

	assert(size == sizeWritten);

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

bool readEntireFile(const char *name, void* buffer, size_t size, size_t *sizeRead)
{
	if(sizeRead)
	{
		*sizeRead = 0;
	}

	HANDLE file = CreateFile(name, GENERIC_READ, NULL, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if(file == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	DWORD sizeRead1 = 0;

	int rez = 1;

	if(!ReadFile(file, buffer, size, &sizeRead1, NULL))
	{
		rez = 0;
	}

	if (sizeRead)
	{
		*sizeRead = sizeRead1;
	}

	CloseHandle(file);


	return rez;
}

bool saveGameState(int id, GameMemory *gameMemory)
{
	char fileName[256] = {};
	strcpy(fileName, "mmemory0");
	fileName[sizeof(fileName) - 1] += id;
	strcat(fileName, ".save");

	bool b = writeEntireFile(fileName, gameMemory, sizeof(GameMemory));
	return b;
}

bool loadGameState(int id, GameMemory* gameMemory)
{
	char fileName[256] = {};
	strcpy(fileName, "mmemory0");
	fileName[sizeof(fileName) - 1] += id;
	strcat(fileName, ".save");

	bool b = readEntireFile(fileName, gameMemory, sizeof(GameMemory));
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