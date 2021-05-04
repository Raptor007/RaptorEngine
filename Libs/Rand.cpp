/*
 *  Rand.cpp
 */

#include "Rand.h"
#include <stdlib.h>


int Rand::Max = RAND_MAX;


void Rand::Seed( unsigned int seed )
{
	srand( seed );
}


int Rand::Int( void )
{
	return rand() & Max;
}


int Rand::Int( int min, int max )
{
	return Int() % (1 + max - min) + min;
}


double Rand::Double( void )
{
	return Int() / (double) Max;
}


double Rand::Double( double max )
{
	return Double() * max;
}


double Rand::Double( double min, double max )
{
	return Double() * (max - min) + min;
}


bool Rand::Bool( double chance )
{
	return Double() <= chance;
}
