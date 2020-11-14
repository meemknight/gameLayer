#pragma once
#include "utility.h"
#include "freeListAllocator.h"
#include <string>
#include <Windows.h>
#include "opengl2Dlib.h"
#include "Phisics.h"

//here you add the memory of the game like so
struct GameMemory
{
#pragma region internal things
	SerializedVariabels serializedVariables = {};
#pragma endregion


	SERIALIZE(int, test, 0, "test");

	gl2d::Renderer2D renderer;


	gl2d::Texture background;
	gl2d::Texture dot;
	gl2d::Texture characterTexture;

	gl2d::ParticleSystem ps;


	phisics::MapData mapData;
	phisics::Entity player;


	//sf::Music musicPlayer;

	gl2d::ParticleSettings deathParticle;
	gl2d::ParticleSettings particleSettings;
	gl2d::ParticleSettings emitPart;
	gl2d::ParticleSettings firePart;
	gl2d::ParticleSettings smokePart;

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
	Button Right;

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
		Right.merge(b.Right);
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

	int mouseX = 0;
	int mouseY = 0;
	bool windowActive = 0;
};


struct HeapMemory
{

	FreeListAllocator allocator;
	char memory[MB(10)];
		
};

struct VolatileMemory
{
	static const size_t maxSize = KB(10);

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
	int w = 400;
	int h = 400;
	bool drawWithOpenGl = true;
	bool lockTo60fps = true;

	//not implemented
	bool fullScreen = false;
	float fullScreenZoon = 1;

};

struct Console
{
	static constexpr int BUFFER_SIZE = 560;
	static constexpr int WRITE_BUFFER_SIZE = 25;

	struct Letter
	{
		Letter() = default;
		Letter(char c) :c(c) { color = 0; }
		Letter(char c, char color):c(c), color(color) {}

		char c;
		char color;
	};

	Letter buffer[BUFFER_SIZE] = {};
	char writeBuffer[WRITE_BUFFER_SIZE+1] = {};
	int writeBufferPos = 0;
	int bufferBeginPos = 0;

	void writeLetter(Letter l)
	{
		buffer[bufferBeginPos] = l;
		bufferBeginPos++;

		if (bufferBeginPos >= BUFFER_SIZE)
		{
			bufferBeginPos = 0;
		}
	}

	void writeText(const char *c, char color = 0)
	{
		while(*c)
		{
			writeLetter({ *c, color });
			c++;
		}
	}

	///normal log
	void log(const char* c)
	{
		writeText(c, 0);
		writeText("\n");
	}

	///error log
	void elog(const char* c)
	{
		writeText(c, 1);
		writeText("\n");
	}

	///warn log (yellow)
	void wlog(const char* c)
	{
		writeText(c, 2);
		writeText("\n");
	}

	///good log (green)
	void glog(const char* c)
	{
		writeText(c, 3);
		writeText("\n");
	}

	///blue log (blue)
	void blog(const char* c)
	{
		writeText(c, 4);
		writeText("\n");
	}
};

#define WRITEENTIREFILE(x) bool x(const char* name, void* buffer, size_t size)
typedef WRITEENTIREFILE(writeEntireFile_t);
WRITEENTIREFILE(writeEntireFile);

#define READENTIREFILE(x) bool x(const char* name, void* buffer, size_t size)
typedef READENTIREFILE(readEntireFile_t);
READENTIREFILE(readEntireFile);

#define MAKECONTEXT(x) void x()
typedef MAKECONTEXT(makeContext_t);
MAKECONTEXT(makeContext);

struct PlatformFunctions
{
	Console console;

	writeEntireFile_t* writeEntireFile = 0;
	readEntireFile_t* readEntirFile = 0;
	makeContext_t* makeContext = 0;
};

struct GameWindowBuffer
{
	char *memory;
	int w;
	int h;

	void drawAt(int x, int y, char r, char g, char b)
	{
		if (x >= w || y >= h || x < 0 || y < 0) 
			{ return; }

		memory[4 * (x + y * w) + 0] = b; //blue
		memory[4 * (x + y * w) + 1] = g; //green
		memory[4 * (x + y * w) + 2] = r; //red
		memory[4 * (x + y * w) + 3] = 0; //reserved for alignment
	}

	void clear(char r = 0, char g = 0, char b = 0)
	{
		for (int y = 0; y < h; y++)
			for (int x = 0; x < w; x++)
			{
				memory[4 * (x + y * w) + 0] = b; //blue
				memory[4 * (x + y * w) + 1] = g; //green
				memory[4 * (x + y * w) + 2] = r; //red
				memory[4 * (x + y * w) + 3] = 0; //reserved for alignment

			}
	}
};

#define GAMELOGIC(x) void x(GameInput *input, GameMemory* memory, \
HeapMemory *heapMemory, VolatileMemory *volatileMemory,\
 GameWindowBuffer *windowBuffer, WindowSettings* windowSettings,\
PlatformFunctions *platformFunctions)
typedef GAMELOGIC(gameLogic_t);
extern "C" __declspec(dllexport) GAMELOGIC(gameLogic);

#define ONCREATE(x) void x(GameMemory* memory, HeapMemory *heapMemory, \
WindowSettings *windowSettings, PlatformFunctions *platformFunctions)
typedef ONCREATE(onCreate_t);
extern "C" __declspec(dllexport) ONCREATE(onCreate);

#define ONRELOAD(x) void x(GameMemory* memory, HeapMemory *heapMemory, \
WindowSettings *windowSettings, PlatformFunctions *platformFunctions)
typedef ONRELOAD(onReload_t);
extern "C" __declspec(dllexport) ONRELOAD(onReload);

#define ONCLOSE(x) void x(GameMemory* memory, HeapMemory *heapMemory, \
WindowSettings *windowSettings, PlatformFunctions *platformFunctions)
typedef ONCLOSE(onClose_t);
extern "C" __declspec(dllexport) ONCLOSE(onClose);

