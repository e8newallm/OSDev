/////////////////////////////////////////////////////////////////////////
////////////////////////Long Manipulation////////////////////////////////
/////////////////////////////////////////////////////////////////////////

unsigned char Hex[17] = "0123456789ABCDEF";

long LongDigits(long Number) //Calculate amount of Decimal digits
{
	long i = 1;
	if(Number < 0)Number = -Number;
	Number /= 10;
	for(; Number != 0; i++)
	{
		Number /= 10;
	}
	return i;
}

long LongDigitsHex(long Number) //Calculate amount of Hexadecimal digits
{
	long i = 1;
	if(Number < 0)Number = -Number;
	Number /= 16;
	for(; Number != 0; i++)
	{
		Number /= 16;
	}
	return i;
}

char* LongToString(long Number) //Convert Decimal number into string
{
	long Size = LongDigits(Number);
	bool Neg = Number < 0;
	if(Neg)
	{
		Number = -Number;
		Size++;
	}
	char Return[Size+1];
	Return[Size] = (char)0;
	Return[Size-1] = '0';
	for(long i = Size-1; Number != 0 ; i--)
	{
		Return[i] = (Number % 10) + '0';
		Number /= 10;
	}
	if(Neg) Return[0] = '-';
	return Return;
}

char* ULongToString(unsigned long Number) //Convert Decimal number into string
{
	long Size = LongDigits(Number);
	char Return[Size+1];
	for(long i = Size-1; i >= 0; i--)
	{
		Return[i] = '0';
	}
	Return[Size] = (char)0;
	for(long i = Size-1; Number != 0 ; i--)
	{
		Return[i] = (Number % 10) + '0';
		Number /= 10;
	}
	return Return;
}

char* LongToStringHex(long Number) //Convert Hexadecimal number into string
{
	long Size = LongDigitsHex(Number) + 2;
	bool Neg = Number < 0;
	char Return[Size+1];
	if(Neg)
	{
		Number = -Number;
		Size++;
		Return[0] = '-';
		Return[1] = '0';
		Return[2] = 'x';
	}
	else
	{
		Return[0] = '0';
		Return[1] = 'x';
	}
	Return[Size] = (char)0;
	Return[Size-1] = '0';
	long i;
	for(i = Size-1; Number != 0 ; i--)
	{
		Return[i] = Hex[(Number % 16)];
		Number /= 16;
	}
	if(Neg) ;
	return Return;
}

char* ULongToStringHex(unsigned long Number) //Convert Hexadecimal number into string
{
	long Size = LongDigitsHex(Number) + 2;
	char Return[Size+1];
	Return[Size] = (char)0;
	Return[Size-1] = '0';
	long i;
	for(i = Size-1; Number != 0 ; i--)
	{
		Return[i] = Hex[(Number % 16)];
		Number /= 16;
	}
	Return[1] = 'x';
	Return[0] = '0';
	return Return;
}

/////////////////////////////////////////////////////////////////////////
////////////////////////Char Manipulation////////////////////////////////
/////////////////////////////////////////////////////////////////////////

long CharDigits(char Number) //Calculate amount of Decimal digits
{
	long i = 1;
	long LongNum = (long)Number;
	if(LongNum < 0)LongNum = -LongNum;
	LongNum /= 10;
	for(; LongNum != 0; i++)
	{
		LongNum /= 10;
	}
	return i;
}

long CharDigitsHex(char Number) //Calculate amount of Hexadecimal digits
{
	long LongNum = (long)Number;
	long i = 1;
	if(LongNum < 0)LongNum = -LongNum;
	LongNum /= 16;
	for(; LongNum != 0; i++)
	{
		LongNum /= 16;
	}
	return i;
}

long UCharDigits(unsigned char Number) //Calculate amount of Decimal digits
{
	long i = 1;
	Number /= 10;
	for(; Number != 0; i++)
	{
		Number /= 10;
	}
	return i;
}

long UCharDigitsHex(unsigned char Number) //Calculate amount of Hexadecimal digits
{
	long i = 1;
	Number /= 16;
	for(; Number != 0; i++)
	{
		Number /= 16;
	}
	return i;
}

char* CharToString(char Number) //Convert Decimal number into string
{
	char Size = CharDigits(Number);
	bool Neg = Number < 0;
	if(Neg)
	{
		Number = -Number;
		Size++;
	}
	char Return[Size+1];
	Return[Size] = (char)0;
	Return[Size-1] = '0';
	for(char i = Size-1; Number != 0 ; i--)
	{
		Return[i] = (Number % 10) + '0';
		Number /= 10;
	}
	if(Neg) Return[0] = '-';
	return Return;
}

char* UCharToString(unsigned char Number) //Convert Decimal number into string
{
	char Size = UCharDigits(Number);
	char Return[Size+1];
	for(char i = Size-1; i >= 0; i--)
	{
		Return[i] = '0';
	}
	Return[Size] = (char)0;
	for(char i = Size-1; Number != 0 ; i--)
	{
		Return[i] = (Number % 10) + '0';
		Number /= 10;
	}
	return Return;
}

char* CharToStringHex(char Number) //Convert Hexadecimal number into string
{
	char Size = CharDigitsHex(Number) + 2;
	bool Neg = Number < 0;
	if(Neg)
	{
		Number = -Number;
		Size++;
	}
	char Return[Size+1];
	Return[Size] = (char)0;
	Return[Size-1] = '0';
	char i;
	for(i = Size-1; Number != 0 ; i--)
	{
		Return[i] = Hex[(Number % 16)];
		Number /= 16;
	}
	Return[i] = 'x';
	Return[i-1] = '0';
	if(Neg) Return[0] = '-';
	return Return;
}

char* UCharToStringHex(unsigned char Number) //Convert Hexadecimal number into string
{
	char Size = UCharDigitsHex(Number) + 2;
	char Return[Size+1];
	Return[Size] = (char)0;
	Return[Size-1] = '0';
	char i;
	for(i = Size-1; Number != 0 ; i--)
	{
		Return[i] = Hex[(Number % 16)];
		Number /= 16;
	}
	Return[1] = 'x';
	Return[0] = '0';
	return Return;
}