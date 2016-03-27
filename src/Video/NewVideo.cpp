#include "Font.h"

struct __attribute__((packed)) Pixel
{
	char Red;
	char Green;
	char Blue;
};

class Draw
{
public:
	unsigned short WindowWidth, WindowHeight, ScreenWidth, ScreenHeight;
	unsigned short x, y;
	Pixel* Buffer;
	Pixel* operator()(unsigned int, unsigned int);
	void DrawPixel(unsigned int, unsigned int, unsigned char, unsigned char, unsigned char);
	void DrawRect(unsigned int, unsigned int, unsigned int, unsigned int, unsigned char, unsigned char, unsigned char);
	void Update();
	Draw(unsigned short, unsigned short, unsigned short, unsigned short, unsigned short, unsigned short);
	Draw();
	void DrawCharacter(unsigned char, long, long);
	void DrawString(char*, int, long, long);
	void DrawString(const char*, int, long, long);
	void DrawString(char*, long, long);
	void DrawString(const char*, long, long);
};

Draw::Draw(unsigned short inX, unsigned short inY, unsigned short inWidth, unsigned short inHeight, unsigned short inScreenWidth, unsigned short inScreenHeight)
{
	WindowWidth = inWidth;
	WindowHeight = inHeight;
	ScreenWidth = inScreenWidth;
	ScreenHeight = inScreenHeight;
	x = inX;
	y = inY;
	Buffer = (Pixel*)Malloc(WindowWidth * WindowHeight * sizeof(Pixel));
}

Pixel* Draw::operator()(unsigned int x, unsigned int y)
{
	return (Buffer + ((y * WindowWidth) + x));
}

void Draw::DrawPixel(unsigned int x, unsigned int y, unsigned char r, unsigned char g, unsigned char b)
{
	Pixel* Pos = (Buffer + ((y * WindowWidth) + x));
	Pos->Red = r;
	Pos->Green = g;
	Pos->Blue = b;
}

void Draw::DrawRect(unsigned int x, unsigned int y, unsigned int Width, unsigned int Height, unsigned char r, unsigned char g, unsigned char b)
{
	Pixel* Start = (Buffer + ((y * WindowWidth) + x));
	for(unsigned int yPos = 0; yPos < Height; yPos++)
	{	
		for(unsigned int xPos = 0; xPos < Width; xPos++)
		{	
			Start[xPos].Red = r;
			Start[xPos].Green = g;
			Start[xPos].Blue = b;
		}
		Start += WindowWidth;
	}
}

void Draw::Update() 
{
	Pixel* NewFrame = (Pixel*)Buffer;
	Pixel* MainFrame = (Pixel*)0xA00000;
	//Serial.WriteString(0x1, "\r\nScreenWidth: ");
	//Serial.WriteLongHex(0x1, ScreenWidth);
	for(int i = 0, Maini = y; i < WindowHeight; i++)
	{
		for(int j = 0, Mainj = x; j < WindowWidth; j++)
		{
			//int pos = (i * Width) + j;
			MainFrame[(Maini * ScreenWidth) + Mainj] = NewFrame[(i * WindowWidth) + j];
			Mainj++;
		}
		Maini++;
	}
	
}

Draw::Draw()
{}


void Draw::DrawCharacter(unsigned char Character, long x, long y)
{
	for(unsigned long i = 0; i < 8; i++)
	{
		unsigned char Line = Font[Character][i];
		for(unsigned long j = 0; j < 8; j++)
		{
			if(Line & (1 << (7 - j)))
			{
				(Buffer + (((y+i) * WindowWidth) + x+j))->Red = 0x0;
				(Buffer + (((y+i) * WindowWidth) + x+j))->Green = 0x0;
				(Buffer + (((y+i) * WindowWidth) + x+j))->Blue = 0x0;
			}
		}
	}
}

void Draw::DrawString(char* String, int Length, long x, long y)
{
	for(int i = 0; i < Length; i++)
	{
		DrawCharacter(String[i], x, y);
		x += 9;
	}
}

void Draw::DrawString(const char* String, int Length, long x, long y)
{
	DrawString((char*)String, Length, x, y);
}

void Draw::DrawString(char* String, long x, long y)
{
	for(int i = 0; String[i] != (char)0; i++)
	{
		DrawCharacter(String[i], x, y);
		x += 9;
	}
}

void Draw::DrawString(const char* String, long x, long y)
{
	DrawString((char*)String, x, y);
}
