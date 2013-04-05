/*
 *  Color.h
 */

#pragma once
class Color;

#include "platforms.h"


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
};
