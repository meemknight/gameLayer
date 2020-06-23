#pragma once



struct GameMemory
{
	char memory[1000];

};




__declspec(dllexport) void gameLogic(GameMemory* memory);
