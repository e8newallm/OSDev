
//TODO: ADD DESTRUCTOR WITH FREE

class String
{
public:
	String();
	~String();
	String(char*);
	String(const char*);
	String operator=(String Equal);
	char& operator[](long position);
	String operator+(String Add);
	String operator+(long);
	String operator+(char*);
	String operator+(const char*);
//private:
	long Length;
	long AllocSize;
	char* Address;
};
