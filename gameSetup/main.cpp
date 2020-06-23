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

	return true;
}

extern "C" __declspec(dllexport) void gameLogic(GameInput* input, GameMemory* memory, GameWindowBuffer* windowBuffer)
{

	for(int y=0; y<windowBuffer->h; y++)
		for (int x = 0; x < windowBuffer->w; x++)
		{
			windowBuffer->memory[4 * (x + y * windowBuffer->w)+0] = 220 * (x%200);
			windowBuffer->memory[4 * (x + y * windowBuffer->w)+1] = 254 * (y%102);
			windowBuffer->memory[4 * (x + y * windowBuffer->w)+2] = 0;
			windowBuffer->memory[4 * (x + y * windowBuffer->w)+3] = 0;
				
		}
	//do game logic

}