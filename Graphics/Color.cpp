/*
 *  Color.cpp
 */

#include "Color.h"


Color::Color( void )
{
	Red = 1.f;
	Green = 1.f;
	Blue = 1.f;
	Alpha = 1.f;
}

Color::Color( const Color &other )
{
	Red = other.Red;
	Green = other.Green;
	Blue = other.Blue;
	Alpha = other.Alpha;
}

Color::Color( const Color *other )
{
	Red = other->Red;
	Green = other->Green;
	Blue = other->Blue;
	Alpha = other->Alpha;
}

Color::Color( float red, float green, float blue, float alpha )
{
	Red = red;
	Green = green;
	Blue = blue;
	Alpha = alpha;
}

Color::~Color()
{
}

void Color::Set( float red, float green, float blue, float alpha )
{
	Red = red;
	Green = green;
	Blue = blue;
	Alpha = alpha;
}

Color &Color::operator +=( const Color &other )
{
	Red += other.Red;
	Green += other.Green;
	Blue += other.Blue;
	return *this;
}

Color &Color::operator -=( const Color &other )
{
	Red -= other.Red;
	Green -= other.Green;
	Blue -= other.Blue;
	return *this;
}

Color &Color::operator *=( double scale )
{
	Red *= scale;
	Green *= scale;
	Blue *= scale;
	return *this;
}

Color &Color::operator /=( double scale )
{
	Red /= scale;
	Green /= scale;
	Blue /= scale;
	return *this;
}

const Color Color::operator*( double scale ) const
{
	return Color( Red*scale, Green*scale, Blue*scale, Alpha );
}

const Color Color::operator/( double scale ) const
{
	return Color( Red/scale, Green/scale, Blue/scale, Alpha );
}
