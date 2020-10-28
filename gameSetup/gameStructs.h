#pragma once
#include "utility.h"
#include "freeListAllocator.h"
#include <string>


//here you add the memory of the game like so
struct GameMemory
{
	
	float posX = 0;
	float posY = 0;

	std::string test = "test1";

};



struct Button
{
	char pressed = 0;
	char held = 0;
	char released = 0;

	enum
	{
		A = 0,
		B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
		NR0, NR1, NR2, NR3, NR4, NR5, NR6, NR7, NR8, NR9,
		Space,
		Enter,
		Escape,
		Up,
		Down,
		Left,
		Right,
		BUTTONS_COUNT, //
	};

	static constexpr int buttonValues[BUTTONS_COUNT] =
	{
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 
		'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		VK_SPACE, VK_RETURN, VK_ESCAPE, VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT
	};
};

//todo change to suit every input

struct GameInput
{
	float deltaTime;

	Button keyBoard[Button::BUTTONS_COUNT];

	Button leftMouse;
	Button rightMouse;

};

struct HeapMemory
{

	FreeListAllocator allocator;
	char memory[MB(10)];
		
};

struct VolatileMemory
{
	static const size_t maxSize = KB(5);

	char memory[maxSize];
	char* stackTop = 0;

	///allocates and the memory is not initialized to 0
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

	///allocates and clears the memory to 0
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

struct WindowSettings
{
	int w;
	int h;

};

struct GameWindowBuffer
{
	char *memory;
	int w;
	int h;

};

#define GAMELOGIC(x) void x(GameInput *input, GameMemory* memory, \
HeapMemory *heapMemory, VolatileMemory *volatileMemory,\
 GameWindowBuffer *windowBuffer, WindowSettings* windowSettings)
typedef GAMELOGIC(gameLogic_t);
extern "C" __declspec(dllexport) GAMELOGIC(gameLogic);

#define ONCREATE(x) void x(GameMemory* memory, HeapMemory *heapMemory, \
WindowSettings *windowSettings)
typedef ONCREATE(onCreate_t);
extern "C" __declspec(dllexport) ONCREATE(onCreate);

