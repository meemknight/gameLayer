#include "Console.h"
#include "font8x8_basic.h"

constexpr int magW = 3;
constexpr int magH = 3;

void drawConsole(GameWindowBuffer* window, Console* console)
{
	window->clear(15,15,18);

	auto w = window->w;
	auto h = window->h;
	auto mem = window->memory;

	int celPosX=0; // pixel pos
	int celPosY=0;

	int cellXsize = w / (8*magW);
	int cellYsize = h / (8*magH);

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
			return;
		}

		if(c == '\t')
		{
			celPosX += 8 * magW * 3;
			if (celPosX / (8 * magW) >= cellXsize)
			{
				celPosX = 0;
				celPosY += 8 * magH;
			}
			
		}

		char letter[8][8] = {};
		char *font = font8x8_basic[c];

		for (int j = 0; j < 8; j++)
		{
			char line = font[j];
			for (int i = 0; i < 8; i++)
			{
				if(line & 0b1 == 1)
				{
					letter[i][j] = 1;
				}

				line >>= 1;
			}

		}
			
		for (int yy = 0; yy < 8 * magH; yy++)
			for (int xx = 0; xx < 8 * magH; xx++)
			{
				if(letter[xx / magW][yy / magH])
				{
					if(color == 1)
					{
						window->drawAt(x + xx, y + yy, 255, 150, 150);
					}else if(color == 2)
					{
						window->drawAt(x + xx, y + yy, 250, 250, 100);
					}else if(color == 3)
					{
						window->drawAt(x + xx, y + yy, 150, 255, 150);
					}else if (color == 4)
					{
						window->drawAt(x + xx, y + yy, 150, 150, 255);
					}
					else
					{
						window->drawAt(x + xx, y + yy, 200, 200, 200);
					}
				}else
				{
				
				}
			}
		celPosX += 8 * magW;
		if (celPosX / (8 * magW) >= cellXsize)
		{
			celPosX = 0;
			celPosY += 8 * magH;
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

}
