#define SerialData 0x0
#define SerialIntEnable 0x1
#define SerialIntID 0x2
#define SerialLineControl 0x3
#define SerialModemControl 0x4
#define SerialLineStatus 0x5
#define SerialModemStatus 0x6
#define SerialScratch 0x7

class SerialController// : protected Mutex
{
	public:
	short COM1, COM2, COM3, COM4;
	
	void Setup(char*);
	bool WriteChar(short, char);
	bool WriteString(short, char*);
	bool WriteString(short, const char*);
	bool WriteLongHex(short, long);
	bool WriteLong(short, long);
	int PortAvail(short);
	short GetPort(int);
};

SerialController Serial;

void YieldCPU();
#define SerialQueueSize 30

class SerialQueue : protected Mutex
{
	private:
	volatile char Queue[SerialQueueSize][256];
	unsigned int Head = 0, Tail = 0;
	bool SameCycle = true;
	public:
	SerialQueue();
	void WriteToLog(char*);
	void WriteToLog(const char*);
	void WriteToLog(long);
	void ReadFromLog(char*);
};

SerialQueue SerialLog;
