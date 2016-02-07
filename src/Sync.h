class Mutex
{
	protected:
	bool InUse; // 0 if free or ID of current thread using it
	long CurrentThreadID;
	public:
	void Lock();
	void Unlock();
};

class CriticalRegion
{
	protected:
	Mutex RegionMutex;
	public:
	void Lock();
	void Unlock();
};