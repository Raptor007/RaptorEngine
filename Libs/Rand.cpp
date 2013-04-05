/*
 *  Rand.cpp
 */

#include "Rand.h"
#include <stdlib.h>


void Rand::Seed( unsigned int seed )
{
	srand( seed );
}


int Rand::Int( void )
{
	return rand();
}


int Rand::Int( int min, int max )
{
	return ( rand() % (1 + max - min) ) + min;
}


double Rand::Double( void )
{
	return ((double)( rand() )) / ((double)( RAND_MAX ));
}


double Rand::Double( double max )
{
	return ( ((double)( rand() )) / ((double)( RAND_MAX )) ) * max;
}


double Rand::Double( double min, double max )
{
	return ( ((double)( rand() )) / ((double)( RAND_MAX )) ) * (max - min) + min;
}


bool Rand::Bool( double chance )
{
	return Rand::Double() <= chance;
}
