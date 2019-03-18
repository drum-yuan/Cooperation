#pragma once
#include <memory>

class Buffer 
{
public:
	Buffer(unsigned int size);
	virtual ~Buffer();
		
	void  reset();
	void* getbuf();
	unsigned int getlength();
	unsigned int getdatalength();
	unsigned int append(void* buf, unsigned int size);
	void setdatapos(unsigned int pos);
	void popfront(unsigned int size);
private:
	void* allocate(unsigned int size);
private:
	void* m_buf;
	unsigned int   m_pos;
	unsigned int   m_size;
};