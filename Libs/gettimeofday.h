/*
 *  gettimeofday.h
 */

#pragma once

#ifdef WIN32

#include <time.h>
#include <winsock.h>

struct timezone_win32
{
	int tz_minuteswest; // minutes W of Greenwich
	int tz_dsttime;     // type of dst correction
};

int gettimeofday( struct timeval *tv, struct timezone_win32 *tz );

#endif
