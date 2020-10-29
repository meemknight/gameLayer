#include <Windows.h>
#include "gameStructs.h"
#include <iostream>
#include <GL/glew.h>

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
	windowSettings->drawWithOpenGl = false;

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

	char color1 = 255-input->controllers[0].LT * 255;
	char color2 = 255-input->controllers[0].RT * 255;

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

			windowBuffer->memory[4 * (newX + newY * windowBuffer->w) + 0] = 25;//blue
			windowBuffer->memory[4 * (newX + newY * windowBuffer->w) + 1] = color1; //green
			windowBuffer->memory[4 * (newX + newY * windowBuffer->w) + 2] = color2; //red
		}

	//move player

	float speed = 340 * deltaTime;
		
		if(input->keyBoard[Button::W].held || input->anyController.Up.held)
		{
			mem->posY -= speed;
		}
		if (input->keyBoard[Button::S].held || input->anyController.Down.held)
		{
			mem->posY += speed;
		}
		if (input->keyBoard[Button::A].held || input->anyController.Left.held)
		{
			mem->posX -= speed;
		}
		if (input->keyBoard[Button::D].held || input->anyController.Riight.held)
		{
			mem->posX += speed;
		}


		//glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
		glClear(GL_COLOR_BUFFER_BIT);         // Clear the color buffer

		// Draw a Red 1x1 Square centered at origin
		glBegin(GL_QUADS);              // Each set of 4 vertices form a quad
		glColor3f(1.0f, 0.0f, 0.0f); // Red
		glVertex2f(-0.5f, -0.5f);    // x, y
		glVertex2f(0.5f, -0.5f);
		glVertex2f(0.5f, 0.5f);
		glVertex2f(-0.5f, 0.5f);
		glEnd();


		mem->posX += speed * input->anyController.LThumb.x;
		mem->posY -= speed * input->anyController.LThumb.y;

		//todo fix button press
		if(input->keyBoard[Button::Space].pressed)
		{
			windowSettings->drawWithOpenGl = !windowSettings->drawWithOpenGl;
		}

}