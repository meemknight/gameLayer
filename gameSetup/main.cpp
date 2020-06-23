#include <Windows.h>
#include "gameStructs.h"

BOOL WINAPI DllMain(
	HINSTANCE hinstDLL,
	DWORD     fdwReason,
	LPVOID    lpvReserved
)
{

	if(fdwReason == DLL_PROCESS_ATTACH)
	{
		OutputDebugString("dll attached");
	}

}

__declspec(dllexport) void gameLogic(GameMemory *memory)
{

	//do game logic

}