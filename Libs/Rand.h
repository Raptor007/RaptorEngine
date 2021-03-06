/*
 *  Rand.h
 */

#pragma once

#include "PlatformSpecific.h"

#include <ctime>


namespace Rand
{
	extern int Max;
	void Seed( unsigned int seed = time(NULL) );
	int Int( void );
	int Int( int min, int max );
	double Double( void );
	double Double( double max );
	double Double( double min, double max );
	bool Bool( double chance = 0.5 );
}
