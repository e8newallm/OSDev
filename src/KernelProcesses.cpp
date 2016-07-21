/////////////////////////TEST PROGRAMS////////////////////////////////////////////
///////TODO: COMPILE SEPARATELY AND LOAD USING GRUB MODULES///////////////////////

__attribute__((noinline)) void Graphics()
{
	long* MainFrame = (long*)ProcessMemStart;
	long* NewFrame = (long*)0xA00000;
	long Size = (GUI.Height * GUI.BytesPerLine)/8;
	MapVideoMemory((void*)ProcessMemStart);
	while(1)
	{
		for(long i = 0; i < Size; i++)
		{
			(MainFrame)[i] = (NewFrame)[i];
		}
		YieldCPU();
	}
}

__attribute__((noinline)) void SerialWrite()
{
	char NextLog[256];
	while(1)
	{
		SerialLog.ReadFromLog(NextLog);
		Serial.WriteString(0x1, NextLog);
	}
}

__attribute__((noinline)) void SystemIdle()
{
	while(1)
	{
		asm("HLT");
	}
}

Draw Window;
Mutex WindowMutex;
int TextHeight, TextWidth;
char** Text;

void Newline()
{
	char* Temp = Text[0];
	for(int i = 0; i < TextHeight-1; i++)
	{
		Text[i] = Text[i+1];
	}
	Text[TextHeight-1] = Temp;
	for(int i = 0; i < TextWidth; i++)
	{
		Text[TextHeight-1][i] = 0;
	}
}

void WriteLine(int Line, char* Message)
{
	int i = 0;
	while(Message[i])
	{
		Text[Line][i] = Message[i];
		i++;
	}
}

void WriteLine(int Line, const char* Message)
{
	WriteLine(Line, (char*)Message);
}

void WriteLine(char* Message)
{
	WriteLine(TextHeight-1, (char*)Message);
}

void WriteLine(const char* Message)
{
	WriteLine(TextHeight-1, (char*)Message);
}

void WriteLine(int Line, String Message)
{
	for(int i = 0; i < Message.Length; i++)
	{
		Text[Line][i] = Message[i];
	}
}

void WriteLine(String Message)
{
	WriteLine(TextHeight-1, Message);
}

__attribute__((noinline)) void TextboxUpdate()
{
	while(1)
	{
		Window.DrawRect(1, 1, GUI.Width - 2, GUI.Height - 2, 128, 128, 128);
		for(int i = 0; i < TextHeight; i++)
		{
			Window.DrawString(Text[i], 4, 4+(9*i));
		}
		Window.Update();
		YieldCPU();
	}
}

void Tasklist()
{
	int CurProcessID = 0;
	while(CurProcessID < 256)
	{
		Process* CurProcess = GetProcess(CurProcessID);
		if(CurProcess->Available == false)
		{
			WriteLine(CurProcess->ProcessName);
			Newline();
		}
		CurProcessID++;
	}
}

__attribute__((noinline)) void Textbox()
{
	int Pointer = 2;
	long CursorTimer = GetMilli();
	TextHeight = (GUI.Height-4) / 9, TextWidth = GUI.Width / 9;
	Window = Draw(0, 0, GUI.Width, GUI.Height, GUI.Width, GUI.Height);
	Text = (char**)Malloc(8 * TextHeight);
	
	StartThread(CreateThread(TextboxUpdate, 2));
	for(int i = 0; i < TextHeight; i++)
	{
		Text[i] = (char*)Malloc(TextWidth);
	}
	Text[TextHeight-1][0] = '>';
	Text[TextHeight-1][1] = ' ';
	while(1)
	{
		unsigned char Key = GetKeyPress();
		if(Key == Key_Backspace)
		{
			if(Pointer > 2)
			{
				Text[TextHeight-1][Pointer--] = 0;
				Text[TextHeight-1][Pointer] = '_';
			}
		}
		else if(Key == Key_Enter)
		{
			Text[TextHeight-1][Pointer] = 0;
			
			if(strcmp(Text[TextHeight-1]+2, "crash") == 0)
			{
				asm("hlt");
			}
			
			if(strcmp(Text[TextHeight-1]+2, "tasklist") == 0)
			{
				Newline();
				Tasklist();
			}
			
			if(strcmp(Text[TextHeight-1]+2, "pcitest") == 0)
			{
				Newline();
				for(int Bus = 0; Bus < 256; Bus++)
				{
					for(int Device = 0; Device < 32; Device++)
					{
						if(ReadFromPCI(Bus, Device, 0, 0) != 0xFFFFFFFF)
						{
							WriteLine("Device found on Bus ");
							Newline();
						}
					}
				}
			}
			
			if(strcmp(Text[TextHeight-1]+2, "stringtest") == 0)
			{
				Newline();
				WriteLine("Testing");
				Newline();
				String Test("lol");
				WriteLine(Test);
				Test = Test + "lol";
				Newline();
				WriteLine(Test);
				Newline();
				WriteLine(Test + "lol");
				Newline();
				WriteLine(Test);
				Newline();
				WriteLine(Test+Test);
				Newline();
				Test = "test";
				WriteLine(Test);
				Newline();
				Test[1] = 'a';
				WriteLine(Test);
				Newline();
				Test[0] = Test[1];
				WriteLine(Test);
				Newline();
				WriteLine(Test + " " + 1234567);
				Newline();
				Test = Test + " " + 1234567;
				Test = Test + " " + 10;
				WriteLine(Test);
				Newline();
			}
			
			Newline();
			Text[TextHeight-1][0] = '>';
			Text[TextHeight-1][1] = ' ';
			Pointer = 2;
		}
		else if(IsCharacter(Key) && Pointer < 200)
		{
			Text[TextHeight-1][Pointer++] = Key;
			Text[TextHeight-1][Pointer] = '_';
		}
		
		if(GetMilli() - CursorTimer > 500)
		{
			CursorTimer = GetMilli();
			Text[TextHeight-1][Pointer] ^= '_';
		}
	}
}
