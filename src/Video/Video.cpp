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
	
	char* FrameAddress;
	char* SecondFrameAddress;
	
	char* operator()(unsigned int, unsigned int);
	void Update();
	Video(vbe_mode_info_struct*);
	Video();
};

char* Video::operator()(unsigned int x, unsigned int y)
{
	return (SecondFrameAddress + ((y * BytesPerLine) + (x * Depth/8)));
}

void Video::Update()
{
	long End = BytesPerLine * Height / 8;
	long* MainFrame = (long*)FrameAddress;
	long* NewFrame = (long*)SecondFrameAddress;
	for(int i = 0; i < End; i++)
	{
		MainFrame[i] = NewFrame[i];
	}
}

Video::Video()
{}

Video::Video(vbe_mode_info_struct* Video_data)
{
	Width = Video_data->XRes;
	Height = Video_data->YRes;
	Depth = Video_data->BitsPerPixel;
	FrameAddress = (char*)((long)Video_data->PhysBasePtr);
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