class String
{
public:
	String();
	String(char*);
	String operator=(String Equal);
	char& operator[](long position);
	String operator+(String Add);
//private:
	long Length;
	long AllocSize;
	char* Address;
};