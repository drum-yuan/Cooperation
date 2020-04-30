#pragma once
#include <fstream>
#include <string>
#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

using namespace std;

uint64_t get_cur_timestamp();
void util_sleep(int msecond);

extern ofstream log_file;

#define LOG_LEVEL_ERROR 	0
#define LOG_LEVEL_WARN 		1
#define LOG_LEVEL_INFO 		2
#define LOG_LEVEL_DEBUG		3

#define LEVEL 3

#define LOG_DEBUG(...)			if(LEVEL >= LOG_LEVEL_DEBUG){ char buf[1024]; snprintf(buf, 1024, ##__VA_ARGS__); log_file << buf << std::endl; }
#ifdef WIN32
#define LOG_INFO(...)			if(LEVEL >= LOG_LEVEL_INFO){ char buf[1024]; char time[64]; SYSTEMTIME sys;\
									snprintf(buf, 1024, ##__VA_ARGS__); GetLocalTime(&sys); snprintf(time, 64, "[INFO](%02d:%02d:%02d.%03d)", sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds); log_file << time << buf << std::endl; }

#define LOG_WARN(...)			if(LEVEL >= LOG_LEVEL_WARN){ char buf[1024]; char time[64]; SYSTEMTIME sys;\
									snprintf(buf, 1024, ##__VA_ARGS__); GetLocalTime(&sys); snprintf(time, 64, "[WARN](%02d:%02d:%02d.%03d)", sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds); log_file << time << buf << std::endl; }

#define LOG_ERROR(...)			if(LEVEL >= LOG_LEVEL_ERROR){ char buf[1024]; char time[64]; SYSTEMTIME sys;\
									snprintf(buf, 1024, ##__VA_ARGS__); GetLocalTime(&sys); snprintf(time, 64, "[ERROR](%02d:%02d:%02d.%03d)", sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds); log_file << time << buf << std::endl; }
#else
#define LOG_INFO(...)			if(LEVEL >= LOG_LEVEL_INFO){ char buf[1024]; char time[64]; struct timeval tv; struct timezone tz; struct tm *p;\
									snprintf(buf, 1024, ##__VA_ARGS__); gettimeofday(&tv, &tz); p = localtime(&tv.tv_sec); snprintf(time, 64, "[INFO](%02d:%02d:%02d.%03ld)", p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec); log_file << time << buf << std::endl; }

#define LOG_WARN(...)			if(LEVEL >= LOG_LEVEL_WARN){ char buf[1024]; char time[64]; struct timeval tv; struct timezone tz; struct tm *p;\
									snprintf(buf, 1024, ##__VA_ARGS__); gettimeofday(&tv, &tz); p = localtime(&tv.tv_sec); snprintf(time, 64, "[INFO](%02d:%02d:%02d.%03ld)", p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec); log_file << time << buf << std::endl; }

#define LOG_ERROR(...)			if(LEVEL >= LOG_LEVEL_ERROR){ char buf[1024]; char time[64]; struct timeval tv; struct timezone tz; struct tm *p;\
									snprintf(buf, 1024, ##__VA_ARGS__); gettimeofday(&tv, &tz); p = localtime(&tv.tv_sec); snprintf(time, 64, "[INFO](%02d:%02d:%02d.%03ld)", p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec); log_file << time << buf << std::endl; }
#endif
