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

extern "C" __declspec(dllexport) void gameLogic(GameInput* input, GameMemory* memory, 
	VolatileMemory *volatileMemory, GameWindowBuffer* windowBuffer)
{
	//do game logic
	float deltaTime = input->deltaTime;

	if(memory->isInitialized==0)
	{
		memory->posX = 0;
		memory->posY = 0;
		memory->isInitialized = true;
	}


	char* c = (char*)volatileMemory->allocate(100);
	char* c1 = (char*)volatileMemory->allocate(100);

	c[10] = 10;
	c1[10] = 10;

	for(int y=0; y<windowBuffer->h; y++)
		for (int x = 0; x < windowBuffer->w; x++)
		{
			windowBuffer->memory[4 * (x + y * windowBuffer->w) + 0] = 0 * ((y + x)%9);	//blue
			windowBuffer->memory[4 * (x + y * windowBuffer->w) + 1] = 1 * ((x + y*x ) % 17);	//green
			windowBuffer->memory[4 * (x + y * windowBuffer->w) + 2] = 1 * ((x*x*x+y)%20);	//red
			windowBuffer->memory[4 * (x + y * windowBuffer->w) + 3] = 0;
				
		}

	//draw player
	for(int y=0; y<20; y++)
		for(int x=0; x<20; x++)
		{
			int newX = memory->posX + x;
			int newY = memory->posY + y;

			if (newX < 0) { continue; }
			if (newY < 0) { continue; }

			if (newX >= windowBuffer->w) { continue; }
			if (newY >= windowBuffer->h) { continue; }

			windowBuffer->memory[4 * (newX + newY * windowBuffer->w) + 0] = 0;//blue
			windowBuffer->memory[4 * (newX + newY * windowBuffer->w) + 1] = 255; //green
			windowBuffer->memory[4 * (newX + newY * windowBuffer->w) + 2] = 255; //red
		}

	//move player

	float spped = 400 * deltaTime;
		
		if(input->up.held)
		{
			memory->posY -= spped;
		}
		if (input->down.held)
		{
			memory->posY += spped;
		}
		if (input->left.held)
		{
			memory->posX -= spped;
		}
		if (input->right.held)
		{
			memory->posX += spped;
		}

}