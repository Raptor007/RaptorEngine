/*
 *  Randomizer.h
 */

#pragma once
class Randomizer;

#include "PlatformSpecific.h"

#include <ctime>


class Randomizer
{
public:
	double Internal;
	const static int Max = 0x7fff;
	
	Randomizer( unsigned int seed = time(NULL) );
	Randomizer( const Randomizer &other );
	virtual ~Randomizer();
	
	void Seed( unsigned int seed = time(NULL) );
	int Int( void );
	int Int( int min, int max );
	double Double( void );
	double Double( double max );
	double Double( double min, double max );
	bool Bool( double chance = 0.5 );
};
