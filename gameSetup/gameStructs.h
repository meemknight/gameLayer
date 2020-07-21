#pragma once
#include "utility.h"


struct GameMemory
{
	
	float posX;
	float posY;
	char isInitialized = 0;


	char memory[1000];
};

struct VolatileMemory
{
	static const size_t maxSize = 4000;

	char memory[maxSize];
	char* stackTop = 0;

	void *allocate(size_t size)
	{
		if(stackTop == 0)
		{
			stackTop = memory;
		}

		void* beginBlock = stackTop;
		stackTop += size;

		assert(stackTop < memory + maxSize);

		return beginBlock;
	}

	void *allocateAndClear(size_t size)
	{
		auto ptr = allocate(size);
		clearMemory(ptr, size);
	}

	void reset()
	{
		stackTop = 0;
	}
};


struct Button
{
	char pressed = 0;
	char held = 0;
	char released = 0;

	void process(bool newState)
	{
		if (newState)
		{
			if (!held)
			{
				held = true;
				pressed = true;
				released = false;

			}
			else
			{
				held = true;
				pressed = false;
				released = false;
			}

		}
		else
		{
			if (held)
			{
				held = false;
				released = true;
				pressed = false;

			}
			else
			{
				held = false;
				released = false;
				pressed = false;

			}

		}
	}
};


struct GameInput
{
	float deltaTime;

	Button up;
	Button down;
	Button left;
	Button right;

};

struct GameWindowBuffer
{
	char *memory;
	int w;
	int h;

};

#define GAMELOGIC(x) void x(GameInput *input, GameMemory* memory, VolatileMemory *volatileMemory,\
 GameWindowBuffer *windowBuffer)
typedef GAMELOGIC(gameLogic_t);
extern "C" __declspec(dllexport) GAMELOGIC(gameLogic);


