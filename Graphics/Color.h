/*
 *  Color.h
 */

#pragma once
class Color;

#include "PlatformSpecific.h"


class Color
{
public:
	float Red, Green, Blue, Alpha;
	
	Color( void );
	Color( const Color &other );
	Color( const Color *other );
	Color( float red, float green, float blue, float alpha = 1.f );
	virtual ~Color();
	
	void Set( float red, float green, float blue, float alpha );
	
	Color &operator = ( const Color &other );
	Color &operator += ( const Color &other );
	Color &operator -= ( const Color &other );
	Color &operator *= ( double scale );
	Color &operator /= ( double scale );
	const Color operator * ( double scale ) const;
	const Color operator / ( double scale ) const;
};
