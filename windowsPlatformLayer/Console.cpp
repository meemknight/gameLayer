#include "Console.h"
#include "font8x8_basic.h"
#include <algorithm>
#include "iostream"

constexpr int magW = 3;
constexpr int magH = 3;
int Ypadding = 0;

void resetConsole(GameWindowBuffer* window, Console* console)
{
	int lineCount = 1;
	int i = 0; 

	while(console->buffer[i].c)
	{
		
		if(console->buffer[i].c == '\n' 
			||
			console->buffer[i].c == '\v'
			)
		{
			lineCount++;
		}
	
		i++;
	}

	int ySize = window->h;
	int yCellCount = ySize / (8 * magH) - 3;

	int diff = lineCount - yCellCount;

	if(diff > 0)
	{
		Ypadding = -diff;
	}else
	{
		Ypadding = 0;
	}

}

void drawConsole(GameWindowBuffer* window, Console* console, GameInput* input)
{
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
			celPosX = 8 * magW;
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
				processCommand(console->writeBuffer);
				memset(console->writeBuffer, 0, sizeof(console->writeBuffer));
				console->writeBufferPos = 0;

				maxDownPadding--;
				Ypadding = maxDownPadding + cellYnumber - 1;
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



const char* commandNames[] =
{
	"var",
	"vars",
	"help",
};

void processCommand(std::string msg)
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
			|| msg[index] == '.'
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
}