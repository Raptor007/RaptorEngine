/*
 *  Rand.h
 */

#pragma once

#include "PlatformSpecific.h"


namespace Rand
{
	void Seed( unsigned int seed );
	int Int( void );
	int Int( int min, int max );
	double Double( void );
	double Double( double max );
	double Double( double min, double max );
	bool Bool( double chance = 0.5 );
}
