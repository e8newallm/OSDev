const char* VGAText_Start = (char*)0xB8000;
char* VGAPos = (char*)0xB8000;

bool ScrollUp();

////////////////////////Print String///////////////////////////////////
bool PrintString(char* String, char* Position, char Colour)
{
	for(int i = 0; String[i] != 0; i++)
	{
		*((char*)Position) = String[i];
		*((char*)Position+1) = Colour;
		Position += 2;
	}
	return 0;
}

bool PrintString(char* String, char Colour)
{
	for(int i = 0; String[i] != 0; i++)
	{
		if(String[i] == (char)10)
		{
			VGAPos += 80*2;
		}
		else if(String[i] == (char)13)
		{
			VGAPos -= ((long)VGAPos % 160) - 64; // - 64 because 0xB8000 % 160 = 64
		}
		else if(String[i] == '\b')
		{
			VGAPos -= 1;
			*((char*)VGAPos) = 0;
		}
		else
		{
			*((char*)VGAPos) = String[i];
			*((char*)VGAPos+1) = Colour;
			VGAPos += 2;
		}
		if(VGAPos > VGAText_Start+(80*25*2))
		{
			ScrollUp();
			VGAPos-= 80*2;
		}
	}
	return 0;
}

bool PrintChar(char Char, char Colour)
{
	if(Char == (char)10)
	{
		VGAPos += 80*2;
	}
	else if(Char == (char)13)
	{
		VGAPos -= ((long)VGAPos % 160) - 64; // - 64 because 0xB8000 % 160 = 64
	}
	else if(Char == '\b')
	{
		VGAPos -= 1;
		*((char*)VGAPos) = 0;
	}
	else
	{
		*((char*)VGAPos) = Char;
		*((char*)VGAPos+1) = Colour;
		VGAPos += 2;
	}
	if(VGAPos > VGAText_Start+(80*25*2))
	{
		ScrollUp();
		VGAPos-= 80*2;
	}
	return 0;
}

bool PrintString(const char* String, char Colour)
{
	return PrintString((char*)String, Colour);
}

bool PrintChar(char* Char, char colour)
{
	return PrintChar(*Char, colour);
}

bool PrintString(String string, char Colour)
{
	for(long i = 0; i < string.Length; i++)
	{
		PrintChar(string[i], Colour);
	}
	return 0;
}

bool ScrollUp()
{
	for(char* i = (char*)VGAText_Start; i < VGAText_Start + (80*25*2); i++)
	{
		*i = *(i+80*2);
	}
	return 0;
}