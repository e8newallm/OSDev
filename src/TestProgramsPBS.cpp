__attribute__((noinline)) void TestOne()
{
	unsigned long Test = 2;
	for(unsigned long i = 1; i < 10000000; i++)
	{
		Test += Test * Test;
	}
	SerialLog.WriteToLog("\t");
	SerialLog.WriteToLog(Test);
	SerialLog.WriteToLog("\r\n");
}

__attribute__((noinline)) void TestTwoUI()
{
	Draw Window = Draw(GUI.Width/2, 0, GUI.Width/2 - 6, GUI.Height*3/4 + 7, GUI.Width, GUI.Height);
	int x = 0, y = 0;
	int Width = Window.WindowWidth - 6, Height = Window.WindowHeight + 4;
	int Colour = 254;
	Window.Update();
	while(x < Width && y < Height)
	{
		Window.DrawRect(x, y, Width,  Height, Colour, Colour, Colour);
		Window.Update();
		x+=10;
		y+=10;
		Width-=20;
		Height-=20;
		Colour-=15;
	}
}

__attribute__((noinline)) void TestTwo()
{
	CurrentThread->OwnerProcess->GetThread(CreateThread(&TestTwoUI, 6))->Start();
	WaitForThread(CurrentThread->OwnerProcess->ProcessID, 1);
}

volatile bool TestThreeReturnOne = false;
volatile bool TestThreeReturnTwo = false;
volatile bool TestThreeReturnThree = false;

__attribute__((noinline)) void TestThreeOne()
{
	//SerialLog.WriteToLog("\r\nTestThreeOne");
	unsigned long Test = 2;
	for(unsigned long i = 1; i < 10000000; i++)
	{
		Test += Test * Test;
	}
	SerialLog.WriteToLog("\r\n\tOne: ");
	SerialLog.WriteToLog(Test);
	TestThreeReturnOne = true;
}
__attribute__((noinline)) void TestThreeTwo()
{
	//SerialLog.WriteToLog("\r\nTestThreeTwo");
	unsigned long Test = 2;
	for(unsigned long i = 1; i < 20000000; i++)
	{
		Test += Test * Test;
	}
	SerialLog.WriteToLog("\r\n\tTwo: ");
	SerialLog.WriteToLog(Test);
	TestThreeReturnTwo = true;
}
__attribute__((noinline)) void TestThreeThree()
{
	//SerialLog.WriteToLog("\r\nTestThreeThree");
	unsigned long Test = 2;
	for(unsigned long i = 1; i < 40000000; i++)
	{
		Test += Test * Test;
	}
	TestThreeReturnThree = true;
	SerialLog.WriteToLog("\r\n\tThree: ");
	SerialLog.WriteToLog(Test);
}


__attribute__((noinline)) void TestThree()
{
	CurrentThread->OwnerProcess->GetThread(CreateThread(&TestThreeOne, 2))->Start();
	CurrentThread->OwnerProcess->GetThread(CreateThread(&TestThreeTwo, 4))->Start();
	CurrentThread->OwnerProcess->GetThread(CreateThread(&TestThreeThree, 8))->Start();
	while(!TestThreeReturnOne || !TestThreeReturnTwo || !TestThreeReturnThree)
	{
		//SerialLog.WriteToLog("\r\nNOPE");
		YieldCPU();
	}
	SerialLog.WriteToLog("\r\nDONE");
	return;
}

volatile int ThreadNum = 0;
Mutex ThreadNumMutex;

volatile long Result = 150;
Mutex ResultMutex;

volatile int ThreadDone = 0;
Mutex ThreadDoneMutex;

__attribute__((noinline)) void TestFourProgram()
{
	ThreadNumMutex.Lock();
	ThreadNum++;
	SerialLog.WriteToLog("\r\nThread Number");
	SerialLog.WriteToLog(ThreadNum);
	ThreadNumMutex.Unlock();
	ResultMutex.Lock();
	long Mod = -Result;
	for(int i = 0; i < 1234567; i++)
	{
		Result += Mod*i;
	}
	ResultMutex.Unlock();
	ThreadDoneMutex.Lock();
	ThreadDone++;
	ThreadDoneMutex.Unlock();
}

__attribute__((noinline)) void TestFour()
{
	for(int j = 0; j < 10; j++)
	{
		for(int i = 0; i < 10; i++)
		{
			//Serial.WriteString(0x1, "\r\nLUL");
			int Test = CreateThread(&TestFourProgram, 4);
			//Serial.WriteString(0x1, "LUL");
			StartThread(Test);
			//Serial.WriteString(0x1, "LUL");
		}
		//Serial.WriteString(0x1, "\r\neLUL");
		YieldCPU();
	}
	
	while(ThreadDone < 100)
	{
		//Serial.WriteString(0x1, "\r\nLUL");
		YieldCPU();
	}
	SerialLog.WriteToLog("DONE\r\n");
	return;
}
