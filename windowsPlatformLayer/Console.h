#pragma once
#include "gameStructs.h"
#include <string>

//called when entering console to set the y padding
void resetConsole(GameWindowBuffer* window, Console* console);

void drawConsole(GameWindowBuffer* window, Console *console);

void processCommand(std::string msg);