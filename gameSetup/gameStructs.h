#pragma once



struct GameMemory
{
	char memory[1000];

};

struct GameInput
{


};

struct GameWindowBuffer
{
	char *memory;
	int w;
	int h;

};

#define GAMELOGIC(x) void x(GameInput *input, GameMemory* memory, GameWindowBuffer *windowBuffer);
typedef GAMELOGIC(gameLogic_t);
extern "C" __declspec(dllexport) GAMELOGIC(gameLogic);
