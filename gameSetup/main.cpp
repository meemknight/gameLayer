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
Console* console = nullptr;

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
	new(mem) GameMemory; // *mem = GameMemory();
	console = &platformFunctions->console;

	platformFunctions->makeContext();
	glewExperimental = true;
	if (glewInit() != GLEW_OK)
	{
		MessageBoxA(0, "glewInit", "Error from glew", MB_ICONERROR);
	}

	gl2d::setErrorFuncCallback([](const char* c) {console->elog(c); });
	gl2d::init();
#pragma endregion


	//set the size of the window
	windowSettings->w = 640;
	windowSettings->h = 360;
	windowSettings->drawWithOpenGl = true;
	windowSettings->lockTo60fps = false;
	gl2d::setVsync(1);


	mem->renderer.create();

	mem->background.loadFromFile("resources//background.png");
	mem->dot.loadFromFile("resources//dot.png");
	mem->characterTexture.loadFromFile("resources//character.png");

	mem->ps.initParticleSystem(300);

	const char* mapShape = 
	"      X  X"
	"      X  X"
	" XXXX     "
	"    X     "
	"       X  "
	" XX XXXX  "
	"          "
	" XX X X   "
	"  X   X  X"
	"  X X   XX";

	mem->mapData.create(10, 10, mapShape);

	mem->player.pos = {};
	mem->player.dimensions = {60,60};

	//mem->musicPlayer.openFromFile("resources//rainForest.wav");
	//mem->musicPlayer.setVolume(50);
	//mem->musicPlayer.play();
	//mem->musicPlayer.setLoop(1);

}

//this might be usefull to change variables on runtime
extern "C" __declspec(dllexport) void onReload(GameMemory * mem, HeapMemory * heapMemory,
	WindowSettings * windowSettings, PlatformFunctions * platformFunctions)
{
#pragma region necesary setup
	allocator = &heapMemory->allocator;
	console = &platformFunctions->console;

	platformFunctions->makeContext();
	glewExperimental = true;
	if (glewInit() != GLEW_OK)
	{
		MessageBoxA(0, "glewInit", "Error from glew", MB_ICONERROR);
	}

	gl2d::setErrorFuncCallback([](const char* c) {console->elog(c); });
	gl2d::init();
#pragma endregion

	platformFunctions->console.log("reloaded...");
	gl2d::setVsync(1);


}


extern "C" __declspec(dllexport) void gameLogic(GameInput * input, GameMemory * mem,
	HeapMemory * heapMemory, VolatileMemory * volatileMemory, GameWindowBuffer * windowBuffer,
	WindowSettings * windowSettings, PlatformFunctions * platformFunctions)
{

#pragma region per frame setup
	allocator = &heapMemory->allocator;
	float deltaTime = input->deltaTime;
	console = &platformFunctions->console;

	glClear(GL_COLOR_BUFFER_BIT);
	//glViewport(0, 0, windowBuffer->w, windowBuffer->h);
	mem->renderer.updateWindowMetrics(windowBuffer->w, windowBuffer->h);

	if(windowSettings->fullScreen)
	{
		mem->renderer.currentCamera.zoom = windowSettings->fullScreenZoon;
	}else
	{
		windowSettings->w = 640;
		windowSettings->h = 360;
		mem->renderer.currentCamera.zoom = 640.f / 1920.f;
	}
	//mem->renderer.currentCamera.zoom = 0.5;
	//mem->renderer.currentCamera.target = { mem->posX, mem->posY };

	float w = windowBuffer->w;
	float h = windowBuffer->h;

	auto& renderer = mem->renderer;
#pragma endregion



	mem->ps.pixelateFactor = 2;

	//do game logic
	//all the global variabels will be stored in "mem"
	// mem->positionX
	//if you want to add any you can do so in gameStructs.h
#pragma region part1


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
	mem->emitPart.createApearence.color1 = { 0, 0.3, 0.5, 0.6 };
	mem->emitPart.createApearence.color2 = { 0.1, 0.4, 0.6, 0.7 };
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
	mem->particleSettings.createApearence.color1 = { 0, 0.2, 0.4, 0.7 };
	mem->particleSettings.createApearence.color2 = { 0.1, 0.4, 0.5, 0.8 };
	mem->particleSettings.createEndApearence.color1 = { 1,0.5,0.5,0.6 };
	mem->particleSettings.createEndApearence.size = {25,25};
	mem->particleSettings.tranzitionType = gl2d::TRANZITION_TYPES::wave;
	mem->particleSettings.texturePtr = &mem->dot;
	mem->particleSettings.deathRattle = &mem->deathParticle;
	//mem->particleSettings.subemitParticle = &mem->emitPart;
	mem->particleSettings.positionX = { -20,20 };
	mem->particleSettings.positionY = { -20,20 };

#pragma endregion

	mem->firePart.onCreateCount = 2;
	mem->firePart.subemitParticleTime = {};		
	mem->firePart.particleLifeTime = {0.9, 1.5};
	mem->firePart.directionX = { -8,8 };
	mem->firePart.directionY = { -4,-6 };
	mem->firePart.createApearence.size = { 30, 30 };
	mem->firePart.dragX = { -5,5 };
	mem->firePart.dragY = { -50,-80 };
	mem->firePart.rotation = { 0, 360 };
	mem->firePart.rotationSpeed = { 0, 10 };
	mem->firePart.rotationDrag = { 0, 100 };
	mem->firePart.createApearence.color1 = { 0.1, 0.3, 0.8, 0.6 };
	mem->firePart.createApearence.color2 = { 0.2, 0.4, 0.9, 0.8 };
	mem->firePart.createEndApearence.color1 = { 0.6,0.4,0.7,0.1 };
	mem->firePart.createEndApearence.size = { 10,15 };
	mem->firePart.tranzitionType = gl2d::TRANZITION_TYPES::wave;
	//mem->firePart.texturePtr = &mem->dot;
	mem->firePart.deathRattle = &mem->smokePart;
	mem->firePart.subemitParticle = nullptr;
	mem->firePart.positionX = { -20,20 };
	mem->firePart.positionY = { -20,20 };

	mem->smokePart.onCreateCount = 1;
	mem->smokePart.subemitParticle = nullptr;
	mem->smokePart.subemitParticleTime = {};
	mem->smokePart.particleLifeTime = { 0.9, 1 };
	mem->smokePart.directionX = { -8,8 };
	mem->smokePart.directionY = { -4,-6 };
	mem->smokePart.createApearence.size = { 10, 15 };
	mem->smokePart.dragX = { -5,5 };
	mem->smokePart.dragY = { -50,-80 };
	mem->smokePart.rotation = { 0, 360 };
	mem->smokePart.rotationSpeed = { 0, 10 };
	mem->smokePart.rotationDrag = { 0, 100 };
	mem->smokePart.createApearence.color1 = { 0.3, 0.1, 0.1, 0.5 };
	mem->smokePart.createApearence.color2 = { 0.5, 0.2, 0.2, 0.6 };
	mem->smokePart.createEndApearence.color1 = { 0.1,0.1,0.1,0.2 };
	mem->smokePart.createEndApearence.size = { 2,5 };
	mem->smokePart.tranzitionType = gl2d::TRANZITION_TYPES::curbe;
	//mem->smokePart.texturePtr = &mem->dot;
	//mem->smokePart.deathRattle = &mem->emitPart;
	//mem->smokePart.subemitParticle = &mem->emitPart;
	mem->smokePart.positionX = { -20,20 };
	mem->smokePart.positionY = { -20,20 };

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

	mem->test = 0;

	char color1 = 255-input->controllers[0].LT * 254;
	char color2 = 255-input->controllers[0].RT * 254;

	renderer.renderRectangle({ 0,0, 1500, 800 }, {}, 0, mem->background);
	


	//move player

	float speed = 620 * deltaTime;
		
	glm::vec2 dir = {};

		if(input->keyBoard[Button::W].held || input->anyController.Up.held)
		{
			dir.y -= speed;
		}
		if (input->keyBoard[Button::S].held || input->anyController.Down.held)
		{
			dir.y += speed;
		}
		if (input->keyBoard[Button::A].held || input->anyController.Left.held)
		{
			dir.x -= speed;
		}
		if (input->keyBoard[Button::D].held || input->anyController.Right.held)
		{
			dir.x += speed;
		}

		dir.x += speed * input->anyController.LThumb.x;
		dir.y -= speed * input->anyController.LThumb.y;

		//todo normalize dir;
		

		if (input->keyBoard[Button::Q].held )
		{
			mem->renderer.currentCamera.rotation -= speed;
		}
		if (input->keyBoard[Button::E].held )
		{
			mem->renderer.currentCamera.rotation += speed;
		}

		


		if (input->keyBoard[Button::Enter].released)
		{
			windowSettings->fullScreen = !windowSettings->fullScreen;
		}

		if (input->keyBoard[Button::Space].released)
		{
			mem->ps.postProcessing = !mem->ps.postProcessing;
		}

		if (input->leftMouse.held)
		{

			glm::vec2 p = {
				mem->player.pos.x + (input->mouseX - w / 2.f) / mem->renderer.currentCamera.zoom,
				mem->player.pos.y + (input->mouseY - h / 2.f) / mem->renderer.currentCamera.zoom
			};

			p = { input->mouseX , input->mouseY };

			p = mem->renderer.currentCamera.convertPoint(p, w, h);

			mem->ps.emitParticleWave(&mem->firePart, p);

		}


#pragma region drawMap
		auto& mapData = mem->mapData;
		for (int y = 0; y < mapData.h; y++)
			for (int x = 0; x < mapData.w; x++) 
			{
				if(mapData.get(x,y).type == 'X')
				{
					auto s = mapData.BLOCK_SIZE;
					renderer.renderRectangle({ x * s, y * s, s, s }, Colors_Green);
				}
			}

#pragma endregion



		mem->player.move(dir);


		mem->player.resolveConstrains(mem->mapData);

		mem->player.updateMove();

		mem->renderer.currentCamera.follow({ mem->player.pos }, deltaTime * 100, 30, windowBuffer->w, windowBuffer->h);

		mem->player.draw(renderer, deltaTime, mem->characterTexture);
		


		mem->ps.applyMovement(deltaTime);
		mem->ps.draw(mem->renderer);


		mem->renderer.flush();


	//mem->renderer.renderRectangle({ 10,10,100,100 }, Colors_Green);
	//mem->renderer.flush();

}

extern "C" __declspec(dllexport) void onClose(GameMemory * mem, HeapMemory * heapMemory,
	WindowSettings * windowSettings, PlatformFunctions * platformFunctions)
{
#pragma region necesary setup
	allocator = &heapMemory->allocator;
	console = &platformFunctions->console;


#pragma endregion




}