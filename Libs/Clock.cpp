/*
 *  Clock.cpp
 */

#include "Clock.h"


Clock::Clock( void )
{
	TimeVal.tv_sec = 0;
	TimeVal.tv_usec = 0;
	Reset();
}


Clock::Clock( const Clock &c )
{
	TimeVal.tv_sec = c.TimeVal.tv_sec;
	TimeVal.tv_usec = c.TimeVal.tv_usec;
}


Clock::~Clock()
{
}


void Clock::Reset( void )
{
	gettimeofday( &TimeVal, NULL );
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
