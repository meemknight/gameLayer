#pragma once
#include "gameStructs.h"
#include <string>

void drawConsole(GameWindowBuffer* window, Console *console);

void processCommand(std::string msg);