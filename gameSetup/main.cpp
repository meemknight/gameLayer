#include <Windows.h>
#include "gameStructs.h"
#include <iostream>

BOOL WINAPI DllMain(
	HINSTANCE hinstDLL,
	DWORD     fdwReason,
	LPVOID    lpvReserved
)
{

	if(fdwReason == DLL_PROCESS_ATTACH)
	{
	//	OutputDebugString("gameLayer: dll attached");
	}

	return true;
}

FreeListAllocator* allocator = nullptr;

#pragma region allocator

void* operator new  (std::size_t count)
{
	auto a = allocator->allocate(count);


	return a;
}

void* operator new[](std::size_t count)
{
	auto a = allocator->allocate(count);


	return a;
}

void operator delete  (void* ptr)
{

	allocator->free(ptr);
}

void operator delete[](void* ptr)
{

	allocator->free(ptr);
}

#pragma endregion


//here you initialize the game memory for the first time
extern "C" __declspec(dllexport) void onCreate(GameMemory* memory, HeapMemory * heapMemory,
	WindowSettings *windowSettings)
{
	allocator = &heapMemory->allocator;
	*memory = GameMemory();

	//set the size of the window
	windowSettings->w = 450;
	windowSettings->h = 450;


}

extern "C" __declspec(dllexport) void gameLogic(GameInput* input, GameMemory* mem, HeapMemory * heapMemory,
	VolatileMemory *volatileMemory, GameWindowBuffer* windowBuffer, WindowSettings * windowSettings)
{

#pragma region per frame setup
	allocator = &heapMemory->allocator;
	float deltaTime = input->deltaTime;
#pragma endregion

	//do game logic
	//all the global variabels will be stored in "mem"
	// mem->positionX
	//if you want to add any you can do so in gameStructs.h


	//the volatile memory persists only for one frame
	char* c = (char*)volatileMemory->allocate(100);
	char* c1 = (char*)volatileMemory->allocate(100);
	c[10] = 10;
	c1[10] = 10;


	//you can change the window settings
	//!they will take place next frame
	//windowSettings->w = 450;
	//windowSettings->h = 450;


	//this is how you draw to the screen for now
	//this clears the screen to black
	for(int y=0; y<windowBuffer->h; y++)
		for (int x = 0; x < windowBuffer->w; x++)
		{
			windowBuffer->memory[4 * (x + y * windowBuffer->w) + 0] = 0; //blue
			windowBuffer->memory[4 * (x + y * windowBuffer->w) + 1] = 0; //green
			windowBuffer->memory[4 * (x + y * windowBuffer->w) + 2] = 0; //red
			windowBuffer->memory[4 * (x + y * windowBuffer->w) + 3] = 0; //reserved for alignment
				
		}

	//draw player
	for(int y=0; y<20; y++)
		for(int x=0; x<20; x++)
		{
			int newX = mem->posX + x;
			int newY = mem->posY + y;

			if (newX < 0) { continue; }
			if (newY < 0) { continue; }

			if (newX >= windowBuffer->w) { continue; }
			if (newY >= windowBuffer->h) { continue; }

			windowBuffer->memory[4 * (newX + newY * windowBuffer->w) + 0] = 0;//blue
			windowBuffer->memory[4 * (newX + newY * windowBuffer->w) + 1] = 255; //green
			windowBuffer->memory[4 * (newX + newY * windowBuffer->w) + 2] = 255; //red
		}

	//move player

	float spped = 340 * deltaTime;
		
		if(input->up.held)
		{
			mem->posY -= spped;
		}
		if (input->down.held)
		{
			mem->posY += spped;
		}
		if (input->left.held)
		{
			mem->posX -= spped;
		}
		if (input->right.held)
		{
			mem->posX += spped;
		}



	if(input->left.released)
	{
		mem->test += ".";
	}

	if (input->right.released)
	{
		if(mem->test.size())
			mem->test.pop_back();
	}
	
	std::cout << mem->test + "\n";

}