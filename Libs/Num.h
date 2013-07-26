/*
 *  Num.h
 */

#pragma once

#include "PlatformSpecific.h"

#include <SDL/SDL.h>
#include <string>
#include <list>
#include <vector>


namespace Num
{
	std::string ToString( int num );
	std::string ToString( double num );
	std::string ToHexString( int num );
	
	int8_t Sign( double num );
	int8_t Sign( int num );
	double IPart( double num );
	double FPart( double num );
	double NearestWhole( double num, double step = 1. );
	bool EveryOther( int num, int scale = 1 );
	bool EveryOther( double num, double scale = 1. );
	double DegToRad( double num );
	double RadToDeg( double num );
	int NextPower( int num, int base );
	int NextPowerOfTwo( int input );
	
	double Avg( const std::list<double> &nums );
	double Avg( const std::vector<double> &nums );
	double Med( const std::list<double> &nums );
	double Med( const std::vector<double> &nums );
	
	int8_t UnitFloatTo8( double num );
	int16_t UnitFloatTo16( double num );
	double UnitFloatFrom8( int8_t approx );
	double UnitFloatFrom16( int16_t approx );
}
