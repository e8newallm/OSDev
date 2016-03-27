extern "C" void* memcpy(void* destination, const void* source, long num)
{
	for(long i = 0; i < num; i++)
		((char*)destination)[i] = ((char*)source)[i];
	return destination;
}

extern "C" void* memset(void* str, int c, long Size)
{
	for(long i = 0; i < Size; i++)
		*(((char*)str)+i) = (unsigned char)c;
	return str;
}

typedef unsigned long size_t;

void* operator new (size_t, void* ptr)
{
	return ptr;
}


int strcmp(const char *s1, const char *s2)
{
  int ret = 0;

  while (!(ret = *(unsigned char *) s1 - *(unsigned char *) s2) && *s2) ++s1, ++s2;

  if (ret < 0)

    ret = -1;
  else if (ret > 0)

    ret = 1 ;

  return ret;
}


int strcmpl(const char *s1, const char *s2, int length)
{
  int ret = 0;
  length--;
  while (!(ret = *(unsigned char *) s1 - *(unsigned char *) s2) && length) ++s1, ++s2, --length;

  if (ret < 0)

    ret = -1;
  else if (ret > 0)

    ret = 1 ;

  return ret;
}
