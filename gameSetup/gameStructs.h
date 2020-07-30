#pragma once
#include "utility.h"


struct GameMemory
{
	
	float posX;
	float posY;
	char isInitialized = 0;

	union
	{
		struct
		{
			char r1;
			char r2;
			char r3;
			char r4;
			char r5;
			char r6;

		};
		char r[6];
	};

	char memory[1000];
};

struct VolatileMemory
{
	static const size_t maxSize = 8000;

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


};


struct GameInput
{
	float deltaTime;

	Button up;
	Button down;
	Button left;
	Button right;
	Button x;
	Button y;
	Button esc;
	Button enter;

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


