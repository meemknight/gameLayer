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

	void merge(const Button &b)
	{
		this->pressed |= b.pressed;
		this->released |= b.released;
		this->held |= b.held;
	}
};


struct ControllerButtons
{

	Button X;
	Button Y;
	Button A;
	Button B;

	Button Up;
	Button Down;
	Button Left;
	Button Riight;

	Button LB;
	Button RB;

	Button LPress;
	Button RPress;

	Button Back;
	Button Menu;

	float LT;
	float RT;

	struct
	{
		float x, y;
	}LThumb, RThumb;

	void merge(const ControllerButtons &b)
	{
		X.merge(b.X);
		Y.merge(b.Y);
		A.merge(b.A);
		B.merge(b.B);
		Up.merge(b.Up);
		Down.merge(b.Down);
		Left.merge(b.Left);
		Riight.merge(b.Riight);
		LB.merge(b.LB);
		RB.merge(b.RB);
		LPress.merge(b.LPress);
		RPress.merge(b.RPress);
		Back.merge(b.Back);
		Menu.merge(b.Menu);

		if (!LT) { LT = b.LT; }
		if (!RT) { RT = b.RT; }

		if (!LThumb.x) { LThumb.x = b.LThumb.x; }
		if (!LThumb.y) { LThumb.y = b.LThumb.y; }
	
		if (!RThumb.x) { RThumb.x = b.RThumb.x; }
		if (!RThumb.y) { RThumb.y = b.RThumb.y; }

	}
};

struct GameInput
{
	float deltaTime;

	Button keyBoard[Button::BUTTONS_COUNT];

	Button leftMouse;
	Button rightMouse;

	ControllerButtons controllers[4] = {};
	ControllerButtons anyController = {};

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
	bool drawWithOpenGl = true;
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

