/*
 *  Num.h
 */

#pragma once

#include "PlatformSpecific.h"

#include <string>
#include <list>
#include <vector>
#include <stdint.h>


namespace Num
{
	std::string ToString( int num );
	std::string ToString( double num, int max_precision = -1 );
	std::string ToHexString( int num );
	
	int8_t Sign( double num );
	int8_t Sign( int num );
	double IPart( double num );
	double FPart( double num );
	uint16_t Mantissa16( double num );
	uint32_t Mantissa32( double num );
	double NearestWhole( double num, double step = 1. );
	bool EveryOther( int num, int scale = 1 );
	bool EveryOther( double num, double scale = 1. );
	double DegToRad( double num );
	double RadToDeg( double num );
	int NextPower( int num, int base );
	int NextPowerOfTwo( int input );
	double SignedPow( double num, double exponent );
	double Clamp( double num, double min, double max );
	bool Valid( double num );
	
	double Avg( const std::list<double> &nums );
	double Avg( const std::vector<double> &nums );
	double Med( const std::list<double> &nums );
	double Med( const std::vector<double> &nums );
	
	int8_t UnitFloatTo8( double num );
	int16_t UnitFloatTo16( double num );
	double UnitFloatFrom8( int8_t approx );
	double UnitFloatFrom16( int16_t approx );
}
