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
	gl2d::init();
#pragma endregion


	//set the size of the window
	windowSettings->w = 600;
	windowSettings->h = 400;
	windowSettings->drawWithOpenGl = true;
	windowSettings->lockTo60fps = false;
	
	gl2d::setVsync(1);

	GLuint id = 0;

	//glActiveTexture(GL_TEXTURE0);


	mem->renderer.create();

	mem->background.loadFromFile("resources//background.png");
	mem->dot.loadFromFile("resources//dot.png");


	mem->ps.initParticleSystem(500);


	console.blog("serialized variables:");
	console.log(mem->serializedVariables.var[0].name);

}

//this might be usefull to change variables on runtime
extern "C" __declspec(dllexport) void onReload(GameMemory * mem, HeapMemory * heapMemory,
	WindowSettings * windowSettings, PlatformFunctions * platformFunctions)
{
#pragma region necesary setup
	allocator = &heapMemory->allocator;
	auto& console = platformFunctions->console;

	platformFunctions->makeContext();
	glewExperimental = true;
	if (glewInit() != GLEW_OK)
	{
		MessageBoxA(0, "glewInit", "Error from glew", MB_ICONERROR);
	}
	gl2d::init();
#pragma endregion


	platformFunctions->console.log("reloaded...");
	gl2d::setVsync(1);

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

	mem->deathParticle.positionX = { -40,40 };
	mem->deathParticle.positionY = { -40,40 };
	mem->deathParticle.particleLifeTime = { 1,1.1 };
	mem->deathParticle.directionX = { -30,30 };
	mem->deathParticle.directionY = { -30,30 };
	mem->deathParticle.createApearence.size = { 40, 40 };
	mem->deathParticle.dragX = { 0,0 };
	mem->deathParticle.dragY = { 0,0 };
	mem->deathParticle.rotation = { 0, 360 };
	mem->deathParticle.rotationSpeed = { -50, 50 };
	mem->deathParticle.rotationDrag = { 0, 0 };
	mem->deathParticle.createApearence.color1 = { 0.9, 0.9, 0.9, 0.9 };
	mem->deathParticle.createApearence.color2 = { 1, 1, 1, 1 };
	mem->deathParticle.createEndApearence.color1 = { 0.9, 0.4, 0.5, 1 };
	mem->deathParticle.createEndApearence.size = { 1,1 };
	mem->deathParticle.tranzitionType = gl2d::TRANZITION_TYPES::abruptCurbe;
	//mem->deathParticle.deathRattle = &mem->deathParticle;
	mem->deathParticle.onCreateCount = 20;

	mem->emitPart.onCreateCount = 2;
	mem->emitPart.particleLifeTime = { 1, 1 };
	mem->emitPart.directionX = { -20,200 };
	mem->emitPart.directionY = { 30, 60 };
	mem->emitPart.createApearence.size = { 10, 10 };
	mem->emitPart.dragX = { -10,10 };
	mem->emitPart.dragY = { 200,250 };
	mem->emitPart.rotation = { 0, 0 };
	mem->emitPart.rotationSpeed = { 0, 0 };
	mem->emitPart.rotationDrag = { 0, 0 };
	mem->emitPart.createApearence.color1 = { 0, 0.3, 0.5, 0.8 };
	mem->emitPart.createApearence.color2 = { 0.1, 0.4, 0.6, 1 };
	mem->emitPart.createEndApearence.color1 = { 0.5,0.5,0.5,0.6 };
	mem->emitPart.createEndApearence.size = { 2,2 };
	mem->emitPart.tranzitionType = gl2d::TRANZITION_TYPES::curbe;
	//mem->emitPart.texturePtr = &mem->dot;
	mem->emitPart.deathRattle = nullptr;
	mem->emitPart.positionX = { -2,2 };
	mem->emitPart.positionY = { -10,0 };


	mem->particleSettings.onCreateCount = 5;
	mem->particleSettings.subemitParticleTime = {0.1, 0.2};
	mem->particleSettings.particleLifeTime = { 1, 1 };
	mem->particleSettings.directionX = { -300,300 };
	mem->particleSettings.directionY = {-300,300};
	mem->particleSettings.createApearence.size = { 40, 40 };
	mem->particleSettings.dragX = { -50,50 };
	mem->particleSettings.dragY = { -50,50 };
	mem->particleSettings.rotation = { 0, 360 };
	mem->particleSettings.rotationSpeed = { 0, 10 };
	mem->particleSettings.rotationDrag = { 0, 100 };
	mem->particleSettings.createApearence.color1 = { 0, 0.2, 0.4, 0.9 };
	mem->particleSettings.createApearence.color2 = { 0.1, 0.4, 0.5, 1 };
	mem->particleSettings.createEndApearence.color1 = { 1,0.5,0.5,0.6 };
	mem->particleSettings.createEndApearence.size = {25,25};
	mem->particleSettings.tranzitionType = gl2d::TRANZITION_TYPES::wave;
	mem->particleSettings.texturePtr = &mem->dot;
	mem->particleSettings.deathRattle = &mem->deathParticle;
	mem->particleSettings.subemitParticle = &mem->emitPart;
	mem->particleSettings.positionX = { -20,20 };
	mem->particleSettings.positionY = { -20,20 };

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
			windowSettings->fullScreen = !windowSettings->fullScreen;
		}

		if (input->leftMouse.pressed)
		{
			//mem->ps.emitParticles = true;

			mem->ps.emitParticleWave(&mem->particleSettings, {input->mouseX, input->mouseY});

		}
		
		
		mem->ps.applyMovement(deltaTime);

		mem->ps.draw(mem->renderer);


		mem->renderer.flush();



}