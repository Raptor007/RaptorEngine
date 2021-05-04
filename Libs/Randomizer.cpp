/*
 *  Randomizer.cpp
 */

#include "Randomizer.h"

#include <cmath>
#include "Num.h"


Randomizer::Randomizer( unsigned int seed )
{
	Seed( seed );
}


Randomizer::Randomizer( const Randomizer &other )
{
	Internal = other.Internal;
}


Randomizer::~Randomizer()
{
}


void Randomizer::Seed( unsigned int seed )
{
	Internal = seed;
}


int Randomizer::Int( void )
{
	double next = 1.49 + cos( Internal );
	Internal = (fabs( next - Internal ) < 0.1) ? (next + 1.51) : next;
	uint32_t mantissa = Num::Mantissa32( Internal );
	return (mantissa >> 9) & Max;
}


int Randomizer::Int( int min, int max )
{
	return Int() % (1 + max - min) + min;
}


double Randomizer::Double( void )
{
	return Int() / (double) Max;
}


double Randomizer::Double( double max )
{
	return Double() * max;
}


double Randomizer::Double( double min, double max )
{
	return Double() * (max - min) + min;
}


bool Randomizer::Bool( double chance )
{
	return Double() <= chance;
}
