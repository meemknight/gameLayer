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
extern "C" __declspec(dllexport) void onCreate(GameMemory* mem, HeapMemory * heapMemory,
	WindowSettings *windowSettings, PlatformFunctions * platformFunctions)
{
#pragma region necesary setup
	allocator = &heapMemory->allocator;
	*mem = GameMemory();
	auto& console = platformFunctions->console;

	platformFunctions->makeContext();
	glewExperimental = true;
	if (glewInit() != GLEW_OK)
	{
		MessageBoxA(0, "glewInit", "Error from glew", MB_ICONERROR);
	}

#pragma endregion


	//set the size of the window
	windowSettings->w = 600;
	windowSettings->h = 400;
	windowSettings->drawWithOpenGl = true;
	
	GLuint id = 0;

	//glActiveTexture(GL_TEXTURE0);

	//glGenTextures(1, &id);

	gl2d::init();

	mem->renderer.create();

	mem->background.loadFromFile("resources//background.png");


	console.blog("serialized variables:");
	console.log(mem->serializedVariables.var[0].name);

}

extern "C" __declspec(dllexport) void gameLogic(GameInput* input, GameMemory* mem,
	HeapMemory * heapMemory, VolatileMemory *volatileMemory, GameWindowBuffer* windowBuffer, 
	WindowSettings * windowSettings, PlatformFunctions * platformFunctions)
{

#pragma region per frame setup
	allocator = &heapMemory->allocator;
	float deltaTime = input->deltaTime;
	auto& console = platformFunctions->console;

	glClear(GL_COLOR_BUFFER_BIT);
	//glViewport(0, 0, windowBuffer->w, windowBuffer->h);
	mem->renderer.updateWindowMetrics(windowBuffer->w, windowBuffer->h);

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
	//windowSettings->w = 600;
	//windowSettings->h = 400;
	//windowSettings->drawWithOpenGl = true;


	char color1 = 255-input->controllers[0].LT * 255;
	char color2 = 255-input->controllers[0].RT * 255;

	mem->renderer.renderRectangle({ 0,0, 600, 400 }, {}, 0, mem->background);
	
	//draw player
	mem->renderer.renderRectangle({ mem->posX , mem->posY, 20, 20 }, { color2, color1, 25, 255 });


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

		

		mem->posX += speed * input->anyController.LThumb.x;
		mem->posY -= speed * input->anyController.LThumb.y;


		if (input->keyBoard[Button::Enter].released)
		{
			console.writeText("test aicisa");
		}
		
		for(int i=0; i<10; i++)
			if (input->keyBoard[Button::NR0 + i].pressed)
			{
				console.writeLetter('0' + i);
			}


		mem->renderer.flush();
}