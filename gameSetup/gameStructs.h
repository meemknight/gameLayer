#pragma once
#include "utility.h"


struct GameMemory
{
	char memory[1000];
	float posX;
	float posY;
	char isInitialized = 0;

};

struct VolatileMemory
{
	static const size_t maxSize = 1000;

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

struct GameInput
{
	char up;
	char down;
	char left;
	char right;

};

struct GameWindowBuffer
{
	char *memory;
	int w;
	int h;

};

#define GAMELOGIC(x) void x(GameInput *input, GameMemory* memory, VolatileMemory *volatileMemory, GameWindowBuffer *windowBuffer);
typedef GAMELOGIC(gameLogic_t);
extern "C" __declspec(dllexport) GAMELOGIC(gameLogic);


