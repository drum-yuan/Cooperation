#include "Daemon.h"

Daemon::Daemon()
{
	
}

Daemon::~Daemon()
{

}

void Daemon::start_stream()
{
	m_Video.setonEncoded(std::bind(&Daemon::onVideoEncoded, this, std::placeholders::_1));
	m_Video.start();
}

void Daemon::stop_stream()
{
	m_Video.stop();
}

void Daemon::onVideoEncoded(void* data)
{

}