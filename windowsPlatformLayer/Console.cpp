#include "Console.h"
#include "font8x8_basic.h"
#include <algorithm>
#include "iostream"

constexpr int magW = 2;
constexpr int magH = 2;
int Ypadding = 0; //todo move ypadding in console
int resetNetxTime = 0;


void resetConsole(GameWindowBuffer* window, Console* console)
{
	int celPosX = 0; // pixel pos
	int maxDownPadding = 0;

	int cellXnumber = window->w / (8 * magW);		//nr of cells
	int cellYnumber = window->h / (8 * magH);

	int i = 0; 

	while(console->buffer[i].c)
	{
		
		celPosX += 8 * magW;
		if (console->buffer[i].c == '\t')
		{
			celPosX += 8 * magW * 2;
		}

		if (celPosX / (8 * magW) >= cellXnumber)
		{
			celPosX = 0;
			maxDownPadding--;
		}

		if(console->buffer[i].c == '\n' 
			||
			console->buffer[i].c == '\v'
			)
		{
			celPosX = 0;
			maxDownPadding--;
		}
	
		i++;
	}

	Ypadding = maxDownPadding + cellYnumber - 1;
	Ypadding = std::min(Ypadding, 0);

}

void drawConsole(GameWindowBuffer* window, Console* console, GameInput* input, SerializedVariabels* serializedVars)
{

	if (resetNetxTime)
	{
		resetConsole(window, console);
	}
	resetNetxTime = false;

	window->clear(15,15,18);

	auto w = window->w;
	auto h = window->h;
	auto mem = window->memory;

	int celPosX=0; // pixel pos
	int celPosY=Ypadding * 8 * magH;
	int maxDownPadding = 0;

	int cellXnumber = w / (8*magW);		//nr of cells
	int cellYnumber = h / (8*magH);

	auto drawChar = [&](char c, int x, int y, char color) 
	{
		if(c >= 128)
		{
			return;
		}

		if(c == '\n')
		{
			celPosX = 0;
			celPosY += 8 * magH;
			maxDownPadding--;
			return;
		}

		if(c == '\t')
		{
			celPosX += 8 * magW * 3;
			if (celPosX / (8 * magW) >= cellXnumber)
			{
				celPosX = 0;
				celPosY += 8 * magH;
				maxDownPadding--;
			}
			
		}

		if(y>=0 && y <= h - (magH * 8 - 2))
		{
			char letter[8][8] = {};
			char* font = font8x8_basic[c];

			for (int j = 0; j < 8; j++)
			{
				char line = font[j];
				for (int i = 0; i < 8; i++)
				{
					if ((line & 0b1) == 1)
					{
						letter[i][j] = 1;
					}

					line >>= 1;
				}

			}

			for (int yy = 0; yy < 8 * magH; yy++)
				for (int xx = 0; xx < 8 * magH; xx++)
				{
					if (letter[xx / magW][yy / magH])
					{
						if (color == 1)
						{
							window->drawAt(x + xx, y + yy, 255, 150, 150);
						}
						else if (color == 2)
						{
							window->drawAt(x + xx, y + yy, 250, 250, 100);
						}
						else if (color == 3)
						{
							window->drawAt(x + xx, y + yy, 150, 255, 150);
						}
						else if (color == 4)
						{
							window->drawAt(x + xx, y + yy, 150, 150, 255);
						}
						else if (color == 5)
						{
							window->drawAt(x + xx, y + yy, 15, 15, 18);
						}
						else
						{
							window->drawAt(x + xx, y + yy, 200, 200, 200);
						}
					}
					else
					{

					}
				}
		}

		celPosX += 8 * magW;
		if (celPosX / (8 * magW) >= cellXnumber)
		{
			celPosX = 0;
			celPosY += 8 * magH;
			maxDownPadding--;
		}

	};

	for (int i = console->bufferBeginPos;
		i < console->BUFFER_SIZE; i++)
	{
		if (console->buffer[i].c)
		{
			drawChar(console->buffer[i].c, celPosX, celPosY, console->buffer[i].color);
			
		}
	}

	for (int i = 0;
		i < console->bufferBeginPos; i++)
	{
		if (console->buffer[i].c)
		{
			drawChar(console->buffer[i].c, celPosX, celPosY, console->buffer[i].color);
		}
	}

#pragma region handle input

	for (int i = 0; input->typedCharacters[i] != 0; i++)
	{
		if (input->typedCharacters[i] == input->BACK_SPACE)
		{
			if (console->writeBufferPos != 0)
			{
				console->writeBufferPos--;
				console->writeBuffer[console->writeBufferPos] = 0;
			}
		}
		else if (console->writeBufferPos < console->WRITE_BUFFER_SIZE)
		{
			console->writeBuffer[console->writeBufferPos++] = input->typedCharacters[i];
		}
	}

	for(int i=8; i<255; i++)
	{
		if (GetAsyncKeyState(i) == -32767)
		{
		
			if (i == VK_UP)
			{
				Ypadding++;
			}
			else if (i == VK_DOWN)
			{
				Ypadding--;

			}
			else if (i == VK_RETURN)
			{
				console->writeText(">", 2);
				console->writeText(console->writeBuffer);
				console->writeText("\n");
				tokenizeCommand(console->writeBuffer, console, serializedVars);
				memset(console->writeBuffer, 0, sizeof(console->writeBuffer));
				console->writeBufferPos = 0;

				maxDownPadding--;
				Ypadding = maxDownPadding + cellYnumber - 1;

				resetNetxTime = true;

			}

		}


	}

	Ypadding = std::max(Ypadding, maxDownPadding+cellYnumber-1);
	Ypadding = std::min(Ypadding, 0);

#pragma endregion


#pragma region draw input
	{
		//clear bottom part
		for (int j = 0; j < magH * 8; j++)
			for (int i = 0; i < w; i++)
			{

				window->drawAt(i, h - j, 200, 200, 200);

			}

		int yPos = h - (magH * 8 - 2);
		int xPos = 8 * magW;
		drawChar('>', 0, yPos, 5);
		
		char* c = console->writeBuffer;

		if (console->writeBufferPos > cellXnumber)
		{
			c += console->writeBufferPos - cellXnumber;
		}

		while(*c)
		{
			drawChar(*c, xPos, yPos, 5);
			c++;
			xPos += 8 * magW;
		}
	
	}
#pragma endregion


}


enum Commands
{
	VAR = 0,
	VARS,
	HELP,
	CLS,
	CommandsCount,
};

const char* commandNames[CommandsCount] =
{
	"var",
	"vars",
	"help",
	"cls",
};

const char* commandDescription[CommandsCount] =
{
	"Set a variable or display its content\nexample:\nvar a //displays content\nvar b = 5//sets variable",
	"Displays variabels.",
	"Gives info about all functions",
	"Clears the console screen",

};

void tokenizeCommand(std::string msg, Console* console, SerializedVariabels* serializedVars)
{
	//std::transform(msg.begin(), msg.end(), msg.begin(),
	//	[](char c) { return std::tolower(c); });
	
	//trim next or sthing
	int index = 0;

	auto isSpace = [&]()
	{

		return (msg[index] == ' ' || msg[index] == '\n' ||
			msg[index] == '\t' || msg[index] == '\v');
	};

	auto isDelimiterChar = [&]()
	{
		return (
				msg[index] == '='
			|| msg[index] == '+'
			|| msg[index] == '('
			|| msg[index] == ')'
			|| msg[index] == '-'
			|| msg[index] == '/'
			|| msg[index] == '\\'
			|| msg[index] == '{'
			|| msg[index] == '}'
			|| msg[index] == '['
			|| msg[index] == ']'
			|| msg[index] == '~'
			|| msg[index] == '`'
			|| msg[index] == '*'
			|| msg[index] == '^'
			|| msg[index] == '!'
			|| msg[index] == '@'
			|| msg[index] == '#'
			|| msg[index] == '$'
			|| msg[index] == '%'
			|| msg[index] == '&'
			|| msg[index] == '_'
			|| msg[index] == '\''
			|| msg[index] == '\"'
			|| msg[index] == '|'
			|| msg[index] == '<'
			|| msg[index] == ','
			|| msg[index] == '>'
			//|| msg[index] == '.'
			|| msg[index] == '?'
			|| msg[index] == ';'
			|| msg[index] == ':'
			);
	};

	auto skipSpaces = [&]()
	{
		for (; index < msg.size(); index++)
		{
			if (!isSpace())
			{
				break;
			}

		}
	};

	skipSpaces();

	std::string firstToken;
	firstToken.reserve(20);

	for (; index < msg.size(); index++)
	{

		if (isSpace() || isDelimiterChar()) { break; }

		firstToken += msg[index];
	}
	

	std::vector<std::string> params;

	{
		std::string currentToken;
		for (; index < msg.size(); index++)
		{

			if (isSpace() || isDelimiterChar()) 
			{
				if (!currentToken.empty())
				{
					params.push_back(currentToken);
					currentToken = {};
				}

				if (isDelimiterChar())
				{
					std::string c = std::string(msg.begin() + index, msg.begin() + index + 1);
					params.push_back(c);
				}

				continue;

			}

			currentToken += msg[index];
		}

		if (!currentToken.empty())
		{
			params.push_back(currentToken);
			currentToken = {};
		}
	}
	

	std::cout << firstToken << " -> ";
	for (auto& i : params)
	{
		std::cout << i << ", ";
	}
	std::cout << "\n";

	processCommand(firstToken, params, console, serializedVars);
}


void processCommand(std::string& firstToken, std::vector<std::string>& params, Console* console, SerializedVariabels* serializedVars)
{
	auto printVariable = [&](int i)
	{
		auto& var = serializedVars->var[i];
		std::string text;

		switch (var.type)
		{
		case type::int_type:
		{
			text += "int ";
			text += var.name;
			text += " = ";

			text += std::to_string(*(int*)var.ptr);

			break;
		};
		case type::float_type:
		{
			text += "float ";
			text += var.name;
			text += " = ";

			text += std::to_string(*(float*)var.ptr);

			break;
		};
		case type::bool_type:
		{
			text += "bool ";
			text += var.name;
			text += " = ";

			text += std::to_string(*(bool*)var.ptr);

			break;
		};
		case type::char_type:
		{
			text += "char ";
			text += var.name;
			text += " = ";

			text += std::to_string(*(char*)var.ptr);

			break;
		};
		default:;
		}

		text += ";\n";
		console->writeText(text);
	};


	int command = -1;
	for (int i = 0; i < CommandsCount; i++)
	{
		if (firstToken == commandNames[i])
		{
			command = i;
			break;
		}
	}

	if (command == -1)
	{
		console->writeText(std::string("Invalid command: ") + firstToken + "\nTry help.\n");
	}
	else
	{
		switch (command)
		{
			case Commands::VAR:
			{
				if (params.empty()) 
				{ 
					console->writeText("Invalid sintax\n");
					break; 
				}

				int index = -1;
				for (int i = 0; i < serializedVars->pos; i++)
				{
					auto& var = serializedVars->var[i];

					if (var.name == params[0])
					{
						index = i;
						break;
					}
					
				}

				if (index == -1)
				{
					console->writeText("No such variable:");
					console->writeText(params[0]);
					console->writeText("\n");
					break;
				}
				else
				{
					auto& var = serializedVars->var[index];

					if (params.size() == 1) //var a
					{
						printVariable(index);
					}
					else if(params.size() == 3 && params[1] == "=")// var a = 1
					{ 
						switch (var.type)
						{
							case type::int_type:
							{
								int val =std::atoi(params[2].c_str()); //todo add own shit here probably
								*(int*)var.ptr = val;

							}break;
							case type::float_type:
							{
								float val = std::atof(params[2].c_str());
								*(float*)var.ptr = val;


							}break;
							case type::char_type:
							{
								char val = std::atoi(params[2].c_str()); //todo add own shit here probably
								*(char*)var.ptr = val;

							}break;
							case type::bool_type:
							{
								bool val = std::atoi(params[2].c_str()); //todo add own shit here probably
								*(bool*)var.ptr = val;

							}break;
						}

						printVariable(index);

					}
					else
					{
						console->writeText("Invalid sintax\n");
						break;
					}
				}

				break;
			}
			case Commands::VARS:
			{
				for (int i = 0; i < serializedVars->pos; i++)
				{
					printVariable(i);
				}

				break;
			}
			case Commands::HELP:
			{
				for (int i = 0; i < Commands::CommandsCount; i++)
				{
					std::string text;
					text += commandNames[i];
					text += ": ";
					text += commandDescription[i];
					text += "\n\n";
					console->writeText(text);
				}
				break;
			}
			case Commands::CLS:
			{
				*console = Console{};

			};

			default:;
		}
	}


}
