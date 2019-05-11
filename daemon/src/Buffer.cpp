#include "Buffer.h"

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

void Buffer::setdatapos(unsigned int pos)
{
	m_pos = LWS_PRE + pos;
}

void Buffer::popfront(unsigned int size)
{
	m_buf = (unsigned char*)m_buf + size;
	m_pos -= size;
	m_size -= size;
}