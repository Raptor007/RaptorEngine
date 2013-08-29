/*
 *  Clock.cpp
 */

#include "Clock.h"

#include <cstddef>


Clock::Clock( void )
{
	TimeVal.tv_sec = 0;
	TimeVal.tv_usec = 0;
	CountUpToSecs = 0.;
	Reset();
}


Clock::Clock( double count_up_to_secs )
{
	TimeVal.tv_sec = 0;
	TimeVal.tv_usec = 0;
	CountUpToSecs = count_up_to_secs;
	Reset();
}


Clock::Clock( const Clock &c )
{
	TimeVal.tv_sec = c.TimeVal.tv_sec;
	TimeVal.tv_usec = c.TimeVal.tv_usec;
	CountUpToSecs = c.CountUpToSecs;
}


Clock::~Clock()
{
}


void Clock::Reset( void )
{
	gettimeofday( &TimeVal, NULL );
}


void Clock::Reset( double count_up_to_secs )
{
	gettimeofday( &TimeVal, NULL );
	CountUpToSecs = count_up_to_secs;
}


double Clock::ElapsedSeconds( void ) const
{
	struct timeval current_time;
	gettimeofday( &current_time, NULL );
	
	return ((double)( current_time.tv_sec - TimeVal.tv_sec )) + ((double)( current_time.tv_usec - TimeVal.tv_usec )) / 1000000.0;
}


double Clock::ElapsedMilliseconds( void ) const
{
	struct timeval current_time;
	gettimeofday( &current_time, NULL );
	
	return ((double)( current_time.tv_sec - TimeVal.tv_sec )) * 1000.0 + ((double)( current_time.tv_usec - TimeVal.tv_usec )) / 1000.0;
}


double Clock::ElapsedMicroseconds( void ) const
{
	struct timeval current_time;
	gettimeofday( &current_time, NULL );
	
	return ((double)( current_time.tv_sec - TimeVal.tv_sec )) * 1000000.0 + ((double)( current_time.tv_usec - TimeVal.tv_usec ));
}


double Clock::RemainingSeconds( void ) const
{
	return CountUpToSecs - ElapsedSeconds();
}
