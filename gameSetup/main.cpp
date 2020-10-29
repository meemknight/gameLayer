#include <Windows.h>
#include "gameStructs.h"
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
	WindowSettings *windowSettings, PlatformFunctions * platformFunctions)
{
#pragma region necesary setup
	allocator = &heapMemory->allocator;
	*memory = GameMemory();
	auto& console = platformFunctions->console;
#pragma endregion

	//set the size of the window
	windowSettings->w = 450;
	windowSettings->h = 450;
	windowSettings->drawWithOpenGl = false;

	console.log("normal log here");
	console.elog("error log here");
	console.wlog("warn log here");
	console.glog("green log here");
	console.blog("blue log here");
	console.log("log here...");

}

extern "C" __declspec(dllexport) void gameLogic(GameInput* input, GameMemory* mem,
	HeapMemory * heapMemory, VolatileMemory *volatileMemory, GameWindowBuffer* windowBuffer, 
	WindowSettings * windowSettings, PlatformFunctions * platformFunctions)
{

#pragma region per frame setup
	allocator = &heapMemory->allocator;
	float deltaTime = input->deltaTime;
	auto& console = platformFunctions->console;
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


	windowBuffer->clear();

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

			windowBuffer->drawAt(newX, newY, color2, color1, 25);

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

		if(input->keyBoard[Button::Space].pressed)
		{
			windowSettings->drawWithOpenGl = !windowSettings->drawWithOpenGl;
		}

		if (input->keyBoard[Button::Enter].released)
		{
			console.writeText("test aicisa");
		}
		
		for(int i=0; i<10; i++)
			if (input->keyBoard[Button::NR0 + i].pressed)
			{
				console.writeLetter('0' + i);
			}

}