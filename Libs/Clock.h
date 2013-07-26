/*
 *  Clock.h
 */

#pragma once
class Clock;

#include "PlatformSpecific.h"

#ifndef WIN32
#include <sys/time.h>
#else
#include <time.h>
#include "gettimeofday.h"
#endif


class Clock
{
public:
	struct timeval TimeVal;
	
	Clock( void );
	Clock( const Clock &c );
	~Clock();
	
	void Reset( void );
	double ElapsedSeconds( void ) const;
	double ElapsedMilliseconds( void ) const;
	double ElapsedMicroseconds( void ) const;
};
