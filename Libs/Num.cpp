/*
 *  Num.cpp
 */

#include "Num.h"
#include <cmath>
#include <algorithm>


std::string Num::ToString( int num )
{
	char cstr[ 1024 ] = "";
	snprintf( cstr, 1024, "%i", num );
	return std::string(cstr);
}


std::string Num::ToString( double num )
{
	char cstr[ 1024 ] = "";
	snprintf( cstr, 1024, "%f", num );
	return std::string(cstr);
}


std::string Num::ToHexString( int num )
{
	char cstr[ 1024 ] = "";
	snprintf( cstr, 1024, "%X", num );
	return std::string(cstr);
}


int8_t Num::Sign( double num )
{
	if( num > 0.0 )
		return 1;
	if( num < 0.0 )
		return -1;
	return 0;
}


int8_t Num::Sign( int num )
{
	if( num > 0 )
		return 1;
	if( num < 0 )
		return -1;
	return 0;
}


double Num::IPart( double num )
{
	double ipart = 0.;
	modf( num, &ipart );
	return ipart;
}


double Num::FPart( double num )
{
	return fmod( num, 1. );
}


double Num::NearestWhole( double num, double step )
{
	return (floor( num / step + 0.5 ) * step);
}


bool Num::EveryOther( int num, int scale )
{
	return (num%(scale*2) >= scale);
}


bool Num::EveryOther( double num, double scale )
{
	return (fmod( num, scale * 2. ) >= scale);
}


double Num::DegToRad( double num )
{
	return num * M_PI / 180.0;
}


double Num::RadToDeg( double num )
{
	return num * 180.0 / M_PI;
}


int Num::NextPower( int num, int base )
{
	return ((int)( pow( base, ceil( log( (double)num ) / log( (double)base ) ) ) + 0.5 ));
}


int Num::NextPowerOfTwo( int input )
{
	int value = 1;

	while( value < input )
		value <<= 1;
	
	return value;
}


double Num::Avg( const std::list<double> &nums )
{
	double avg = 0.;
	
	int count = nums.size();
	if( count )
	{
		for( std::list<double>::const_iterator num_iter = nums.begin(); num_iter != nums.end(); num_iter ++ )
			avg += *num_iter;
		
		avg /= count;
	}
	
	return avg;
}


double Num::Avg( const std::vector<double> &nums )
{
	double avg = 0.;
	
	int count = nums.size();
	if( count )
	{
		for( std::vector<double>::const_iterator num_iter = nums.begin(); num_iter != nums.end(); num_iter ++ )
			avg += *num_iter;
		
		avg /= count;
	}
	
	return avg;
}


double Num::Med( const std::list<double> &nums )
{
	std::vector<double> nums_vec;
	for( std::list<double>::const_iterator num_iter = nums.begin(); num_iter != nums.end(); num_iter ++ )
		nums_vec.push_back( *num_iter );
	
	return Med( nums_vec );
}


double Num::Med( const std::vector<double> &nums )
{
	std::vector<double> sorted = nums;
	std::sort( sorted.begin(), sorted.end() );
	
	int count = sorted.size();
	if( count % 2 )
		return sorted[ count / 2 ];
	else if( count )
		return (sorted[ count / 2 ] + sorted[ count / 2 - 1 ]) / 2.;
	else
		return 0.;
}


int8_t Num::UnitFloatTo8( double num )
{
	// Convert from (-1,1) to (-128,127) range.
	
	if( num >= 0. )
		return num * 127.1;
	else
		return num * 128.1;
}


int16_t Num::UnitFloatTo16( double num )
{
	// Convert from (-1,1) to (-32768,32767) range.
	
	if( num >= 0. )
		return num * 32767.1;
	else
		return num * 32768.1;
}


double Num::UnitFloatFrom8( int8_t approx )
{
	// Convert from (-128,127) to (-1,1) range.
	
	if( approx >= 0 )
		return ((double) approx) / 127.;
	else
		return ((double) approx) / 128.;
}


double Num::UnitFloatFrom16( int16_t approx )
{
	// Convert from (-32768,32767) to (-1,1) range.
	
	if( approx >= 0 )
		return ((double) approx) / 32767.;
	else
		return ((double) approx) / 32768.;
}
