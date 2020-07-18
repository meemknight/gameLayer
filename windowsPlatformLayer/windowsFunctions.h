#pragma once
#include <Windows.h>
#include "gameStructs.h"

void win32LoadDll(gameLogic_t** gameLogicPtr, const char* dllName);
void win32UnloadDll();

FILETIME win32GetLastWriteFile(const char* name);

bool writeEntireFile(const char* name, void* buffer, size_t size);
bool readEntireFile(const char* name, void* buffer, size_t size);
bool clearFileContent(const char* name);
//bool appendToFille(const char* name, void* buffer, size_t size);

bool saveGameState(int id, GameMemory* gameMemory);
bool loadGameState(int id, GameMemory* gameMemory);
