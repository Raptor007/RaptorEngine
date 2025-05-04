/*
 *  Num.cpp
 */

#include "Num.h"

#include <cstdio>
#include <cstring>
#include <cmath>
#include <algorithm>

#ifdef SDL2
	#include <SDL2/SDL_stdinc.h>
#endif


std::string Num::ToString( int num )
{
	char cstr[ 1024 ] = "";
	snprintf( cstr, 1024, "%i", num );
	return std::string(cstr);
}


std::string Num::ToString( double num, int max_precision )
{
	char cstr[ 1024 ] = "";
	char format[ 32 ] = "%f";
	if( max_precision >= 0 )
		snprintf( format, sizeof(format), "%%.%if", max_precision );
	snprintf( cstr, 1024, format, num );
	
	// Trim trailing zeros after decimal point.
	char *dot = strchr( cstr, '.' );
	if( dot )
	{
		for( char *last = dot + strlen(dot) - 1; last >= dot; last -- )
		{
			if( last[ 0 ] == '0' )
				last[ 0 ] = '\0';
			else if( last == dot )
			{
				last[ 0 ] = '\0';
				break;
			}
			else
				break;
		}
	}
	
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


uint16_t Num::Mantissa16( double num )
{
	uint64_t *raw = (uint64_t*) &num;
	return (*raw & 0x000FFFF000000000ULL) >> 36;
}


uint32_t Num::Mantissa32( double num )
{
	uint64_t *raw = (uint64_t*) &num;
	return (*raw & 0x000FFFFFFFF00000ULL) >> 20;
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


double Num::SignedPow( double num, double exponent )
{
	if( num < 0. )
		return pow( num * -1., exponent ) * -1.;
	return pow( num, exponent );
}


double Num::Clamp( double num, double min, double max )
{
	if( num > max )
		return max;
	if( num < min )
		return min;
	return num;
}


int16_t Num::ScaleInt16( int16_t num, double scale )
{
	double scaled = num * scale;
	if( scaled <= -32768. )
		return -32768;
	if( scaled >= 32767. )
		return 32767;
	return scaled + Num::Sign(scaled) * 0.5;
}


bool Num::Valid( double num )
{
	return (num == num);  // Detect floating-point NaN/IND.
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
