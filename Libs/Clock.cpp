/*
 *  Clock.cpp
 */

#include "Clock.h"

#include <cstddef>
#include <cmath>


Clock::Clock( void )
{
	TimeVal.tv_sec = 0;
	TimeVal.tv_usec = 0;
	CountUpToSecs = 0.;
	TimeScale = 1.;
	Reset();
}


Clock::Clock( double count_up_to_secs )
{
	TimeVal.tv_sec = 0;
	TimeVal.tv_usec = 0;
	CountUpToSecs = count_up_to_secs;
	TimeScale = 1.;
	Reset();
}


Clock::Clock( const Clock &c )
{
	TimeVal.tv_sec = c.TimeVal.tv_sec;
	TimeVal.tv_usec = c.TimeVal.tv_usec;
	CountUpToSecs = c.CountUpToSecs;
	TimeScale = c.TimeScale;
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


void Clock::Sync( const Clock *c )
{
	TimeVal.tv_sec = c->TimeVal.tv_sec;
	TimeVal.tv_usec = c->TimeVal.tv_usec;
	CountUpToSecs = c->CountUpToSecs;
	TimeScale = c->TimeScale;
}


void Clock::Advance( double secs )
{
	double s = 0.;
	double us = modf( secs, &s );
	TimeVal.tv_sec += s;
	TimeVal.tv_usec += us * 1000000. + 0.5;
	if( TimeVal.tv_usec > 1000000 )
	{
		TimeVal.tv_sec ++;
		TimeVal.tv_usec -= 1000000;
	}
	else if( TimeVal.tv_usec < 0 )
	{
		TimeVal.tv_sec --;
		TimeVal.tv_usec += 1000000;
	}
}


void Clock::SetTimeScale( double time_scale )
{
	if( (TimeScale != time_scale) && time_scale && TimeScale )
	{
		double elapsed_seconds = ElapsedSeconds();  // NOTE: This is scaled by current TimeScale.
		double desired_elapsed = elapsed_seconds / time_scale;
		Reset();
		Advance( -desired_elapsed );
	}
	
	TimeScale = time_scale;
}


double Clock::ElapsedSeconds( void ) const
{
	double scale = TimeScale;
	double scale2 = scale / 1000000.;
	
	struct timeval current_time;
	gettimeofday( &current_time, NULL );
	
	return ((double)( current_time.tv_sec - TimeVal.tv_sec )) * scale + ((double)( current_time.tv_usec - TimeVal.tv_usec )) * scale2;
}


double Clock::ElapsedMilliseconds( void ) const
{
	double scale = TimeScale * 1000.;
	double scale2 = scale / 1000000.;
	
	struct timeval current_time;
	gettimeofday( &current_time, NULL );
	
	return ((double)( current_time.tv_sec - TimeVal.tv_sec )) * scale + ((double)( current_time.tv_usec - TimeVal.tv_usec )) * scale2;
}


double Clock::ElapsedMicroseconds( void ) const
{
	double scale = TimeScale * 1000000.;
	double scale2 = scale / 1000000.;
	
	struct timeval current_time;
	gettimeofday( &current_time, NULL );
	
	return ((double)( current_time.tv_sec - TimeVal.tv_sec )) * scale + ((double)( current_time.tv_usec - TimeVal.tv_usec )) * scale2;
}


double Clock::RemainingSeconds( void ) const
{
	return CountUpToSecs - ElapsedSeconds();
}


double Clock::Progress( void ) const
{
	if( CountUpToSecs )
		return ElapsedSeconds() / CountUpToSecs;
	
	return 1.;  // Counting up to 0 is always 100% done.
}


Clock &Clock::operator = ( const Clock &other )
{
	TimeVal.tv_sec  = other.TimeVal.tv_sec;
	TimeVal.tv_usec = other.TimeVal.tv_usec;
	CountUpToSecs   = other.CountUpToSecs;
	TimeScale       = other.TimeScale;
	return *this;
}
