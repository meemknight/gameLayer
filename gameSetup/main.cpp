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
		OutputDebugString("gameLayer: dll attached");
	}

	return true;
}

extern "C" __declspec(dllexport) void gameLogic(GameInput* input, GameMemory* memory, GameWindowBuffer* windowBuffer)
{
	for(int y=0; y<windowBuffer->h; y++)
		for (int x = 0; x < windowBuffer->w; x++)
		{
			windowBuffer->memory[4 * (x + y * windowBuffer->w) + 0] = 0 * ((y + x)%29);	//blue
			windowBuffer->memory[4 * (x + y * windowBuffer->w) + 1] = 20 * ((x + y*x ) % 17);	//green
<<<<<<< HEAD
			windowBuffer->memory[4 * (x + y * windowBuffer->w) + 2] = 20 * ((x*x*x+y)%20);	//red
=======
			windowBuffer->memory[4 * (x + y * windowBuffer->w) + 2] = 250 * ((x*x*x+y)%20);	//red
>>>>>>> parent of 7140e86... fixed some things
			windowBuffer->memory[4 * (x + y * windowBuffer->w) + 3] = 0;
				
		}

	//do game logic

}