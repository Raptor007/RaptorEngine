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
	double CountUpToSecs;
	double TimeScale;
	
	Clock( void );
	Clock( double count_up_to_secs );
	Clock( const Clock &c );
	~Clock();
	
	void Reset( void );
	void Reset( double count_up_to_secs );
	void Sync( const Clock *c );
	void Advance( double secs );
	void SetTimeScale( double time_scale );
	
	double ElapsedSeconds( void ) const;
	double ElapsedMilliseconds( void ) const;
	double ElapsedMicroseconds( void ) const;
	
	double RemainingSeconds( void ) const;
	double Progress( void ) const;
	
	Clock &operator = ( const Clock &other );
};
