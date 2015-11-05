#include "Font.h"

class Video
{
public:
	unsigned short Width;
	unsigned short Height;
	unsigned char Depth;
	unsigned short BytesPerLine;
	unsigned char MemoryModel;
	unsigned short ModeAttributes;
	unsigned char RedMaskSize;
	unsigned char RedFieldPos;
	unsigned char GreenMaskSize;
	unsigned char GreenFieldPos;
	unsigned char BlueMaskSize;
	unsigned char BlueFieldPos;
	unsigned char RSVDMaskSize;
	unsigned char RSVDFieldPos;
	
	unsigned char* FrameAddress;
	unsigned char* SecondFrameAddress;
	
	unsigned char* operator()(unsigned int, unsigned int);
	void DrawPixel(unsigned int, unsigned int, unsigned char, unsigned char, unsigned char);
	void DrawRect(unsigned int, unsigned int, unsigned int, unsigned int, unsigned char, unsigned char, unsigned char);
	void Update();
	Video(vbe_mode_info_struct*);
	Video();
};

unsigned char* Video::operator()(unsigned int x, unsigned int y)
{
	return (SecondFrameAddress + ((y * BytesPerLine) + (x * Depth)));
}

void Video::DrawPixel(unsigned int x, unsigned int y, unsigned char r, unsigned char g, unsigned char b)
{
	unsigned char* Pos = (SecondFrameAddress + ((y * BytesPerLine) + (x * Depth)));
	*Pos = r;
	*(Pos + 1) = g;
	*(Pos + 2)= b;
}

void Video::DrawRect(unsigned int x, unsigned int y, unsigned int Width, unsigned int Height, unsigned char r, unsigned char g, unsigned char b)
{
	unsigned char* Start = (SecondFrameAddress + ((y * BytesPerLine) + (x * Depth)));
	for(unsigned int yPos = 0; yPos < Height; yPos++)
	{	
		for(unsigned int xPos = 0; xPos < Width*Depth; xPos += Depth)
		{	
			Start[xPos] = r;
			Start[xPos+1] = g;
			Start[xPos+2] = b;
		}
		Start += BytesPerLine;
	}
}

void Video::Update()
{
	char* NewFrame = (char*)SecondFrameAddress;
	char* MainFrame = (char*)FrameAddress;
	//for(int i = 0; i < End; i++)
	//{
		for(int i = 0; i < Height; i++)
		{
			for(int j = 0; j < Width*3; j++)
			{
				int pos = (i * BytesPerLine) + j;
				MainFrame[pos] = NewFrame[pos];
			}
		}
		//MainFrame[i] = NewFrame[i];
	//}
}

Video::Video()
{}

Video::Video(vbe_mode_info_struct* Video_data)
{
	Width = Video_data->XRes;
	Height = Video_data->YRes;
	Depth = Video_data->BitsPerPixel / 8;
	FrameAddress = (unsigned char*)((long)Video_data->PhysBasePtr);
	BytesPerLine = Video_data->BytesPerScanLine;
	MemoryModel = Video_data->MemoryModel;
	ModeAttributes = Video_data->ModeAttr;
	RedMaskSize = Video_data->RedMaskSize;
	RedFieldPos = Video_data->RedFieldPos;
	GreenMaskSize = Video_data->GreenMaskSize;
	GreenFieldPos = Video_data->GreenFieldPos;
	BlueMaskSize = Video_data->BlueMaskSize;
	BlueFieldPos = Video_data->BlueFieldPos;
	RSVDMaskSize = Video_data->RSVDMaskSize;
	RSVDFieldPos = Video_data->RSVDFieldPos;
}

Video GUI;

void DrawCharacter(char Character, long x, long y)
{
	for(long i = 0; i < 8; i++)
	{
		char Line = Font[Character][i];
		for(long j = 0; j < 8; j++)
		{
			if(Line & (1 << (7 - j)))
			{
				*GUI(x+j, y+i) = 0x0;
				*(GUI(x+j, y+i) + 1) = 0xFF;
				*(GUI(x+j, y+i) + 2)= 0x0;
			}
		}
	}
}

void DrawString(char* String, int Length, long x, long y)
{
	for(int i = 0; i < Length; i++)
	{
		DrawCharacter(String[i], x, y);
		x += 9;
	}
}

void DrawString(const char* String, int Length, long x, long y)
{
	DrawString((char*)String, Length, x, y);
}