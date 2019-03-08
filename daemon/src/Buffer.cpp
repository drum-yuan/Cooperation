#include "Buffer.h"

#define LWS_PRE 16  //reserve 16 bytes for libwebsockets write protocol header data

Buffer::Buffer(unsigned int size):m_buf(NULL),m_size(0), m_pos(LWS_PRE)
{
	m_size = LWS_PRE + size;
	m_buf = allocate(m_size);
}

Buffer::~Buffer()
{
	if (m_buf)
	{
		delete m_buf;
		m_buf = NULL;
	}
}

void Buffer::reset()
{
	memset(m_buf, 0, m_pos);
	m_pos = LWS_PRE;
}

void* Buffer::getbuf()
{
	return (unsigned char*)m_buf + LWS_PRE;
}

unsigned int Buffer::getlength()
{
	return m_size;
}

unsigned int Buffer::getdatalength()
{
	return m_pos - LWS_PRE;
}

void* Buffer::allocate(unsigned int size)
{
	return malloc(size * sizeof(char));
}

unsigned int Buffer::append(void* buf, unsigned int size)
{
	if (m_size < m_pos + size)
		return 0;

	memcpy((unsigned char*)m_buf + m_pos, buf, size);
	m_pos += size;

	return size;
}