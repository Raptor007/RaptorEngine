/*
 *  Vec.cpp
 */

#include "Vec.h"

#include <cmath>
#include "Num.h"


Vec::~Vec()
{
}


Vec2D::Vec2D( const Vec2D &other )
{
	Set( other.X, other.Y );
}


Vec2D::Vec2D( const Vec2D *other )
{
	Set( other->X, other->Y );
}


Vec2D::Vec2D( double x, double y )
{
	Set( x, y );
}


Vec2D::~Vec2D()
{
}


void Vec2D::Copy( const Vec2D &other )
{
	X = other.X;
	Y = other.Y;
}


void Vec2D::Set( double x, double y )
{
	X = x;
	Y = y;
}


double Vec2D::Length( void ) const
{
	return sqrt( X*X + Y*Y );
}


void Vec2D::ScaleBy( double factor )
{
	X *= factor;
	Y *= factor;
}


void Vec2D::ScaleTo( double length )
{
	double old_length = sqrt( X*X + Y*Y );
	X *= length / old_length;
	Y *= length / old_length;
}


double Vec2D::Dot( const Vec2D &other ) const
{
	return Dot( other.X, other.Y );
}

double Vec2D::Dot( double x, double y ) const
{
	return X*x + Y*y;
}


Vec2D &Vec2D::operator +=( const Vec2D &other )
{
	X += other.X;
	Y += other.Y;
	return *this;
}


Vec2D &Vec2D::operator -=( const Vec2D &other )
{
	X -= other.X;
	Y -= other.Y;
	return *this;
}


Vec2D &Vec2D::operator *=( double scale )
{
	ScaleBy( scale );
	return *this;
}


Vec2D &Vec2D::operator /=( double scale )
{
	ScaleBy( 1. / scale );
	return *this;
}


const Vec2D Vec2D::operator+( const Vec2D &other ) const
{
	return Vec2D(this) += other;
}


const Vec2D Vec2D::operator-( const Vec2D &other ) const
{
	return Vec2D(this) -= other;
}


const Vec2D Vec2D::operator*( double scale ) const
{
	return Vec2D(this) *= scale;
}


const Vec2D Vec2D::operator/( double scale ) const
{
	return Vec2D(this) /= scale;
}


// ---------------------------------------------------------------------------


Vec3D::Vec3D( const Vec3D &other )
{
	Set( other.X, other.Y, other.Z );
}


Vec3D::Vec3D( const Vec3D *other )
{
	Set( other->X, other->Y, other->Z );
}


Vec3D::Vec3D( double x, double y, double z )
{
	Set( x, y, z );
}


Vec3D::~Vec3D()
{
}


void Vec3D::Copy( const Vec3D &other )
{
	X = other.X;
	Y = other.Y;
	Z = other.Z;
}


void Vec3D::Set( double x, double y, double z )
{
	Vec2D::Set( x, y );
	Z = z;
}


double Vec3D::Length( void ) const
{
	return sqrt( X*X + Y*Y + Z*Z );
}


void Vec3D::ScaleBy( double factor )
{
	X *= factor;
	Y *= factor;
	Z *= factor;
}


void Vec3D::ScaleTo( double length )
{
	double old_length = sqrt( X*X + Y*Y + Z*Z );
	X *= length / old_length;
	Y *= length / old_length;
	Z *= length / old_length;
}


void Vec3D::RotateAround( const Vec3D *axis, double degrees )
{
	// http://inside.mines.edu/~gmurray/ArbitraryAxisRotation/

	// Convert to radians for the maths.
	double radians = Num::DegToRad(degrees);

	// Keep old X,Y,Z values to use them as inputs for the new values.
	double x = X, y = Y, z = Z;

	// Convert the axis to a unit vector (scale to length 1).
	double axis_length = sqrt( axis->X * axis->X + axis->Y * axis->Y + axis->Z * axis->Z );
	double u = axis->X / axis_length, v = axis->Y / axis_length, w = axis->Z / axis_length;

	// Calculate new X,Y,Z values.
	X = u * ( u*x + v*y + w*z ) * ( 1 - cos(radians) ) + x * cos(radians) + ( v*z - w*y ) * sin(radians);
	Y = v * ( u*x + v*y + w*z ) * ( 1 - cos(radians) ) + y * cos(radians) + ( w*x - u*z ) * sin(radians);
	Z = w * ( u*x + v*y + w*z ) * ( 1 - cos(radians) ) + z * cos(radians) + ( u*y - v*x ) * sin(radians);
}


void Vec3D::RotateAround( const Vec3D *axis, double degrees, const Vec3D *anchor )
{
	X -= anchor->X;
	Y -= anchor->Y;
	Z -= anchor->Z;
	
	RotateAround( axis, degrees );
	
	X += anchor->X;
	Y += anchor->Y;
	Z += anchor->Z;
}


double Vec3D::Dot( const Vec3D &other ) const
{
	return Dot( other.X, other.Y, other.Z );
}


double Vec3D::Dot( double x, double y, double z ) const
{
	return X*x + Y*y + Z*z;
}


Vec3D Vec3D::Cross( const Vec3D &other ) const
{
	Vec3D cross;
	cross.X = Y * other.Z - Z * other.Y;
	cross.Y = Z * other.X - X * other.Z;
	cross.Z = X * other.Y - Y * other.X;
	return cross;
}


Vec3D &Vec3D::operator +=( const Vec3D &other )
{
	X += other.X;
	Y += other.Y;
	Z += other.Z;
	return *this;
}


Vec3D &Vec3D::operator -=( const Vec3D &other )
{
	X -= other.X;
	Y -= other.Y;
	Z -= other.Z;
	return *this;
}


Vec3D &Vec3D::operator *=( double scale )
{
	ScaleBy( scale );
	return *this;
}


Vec3D &Vec3D::operator /=( double scale )
{
	ScaleBy( 1. / scale );
	return *this;
}


const Vec3D Vec3D::operator+( const Vec3D &other ) const
{
	return Vec3D(this) += other;
}


const Vec3D Vec3D::operator-( const Vec3D &other ) const
{
	return Vec3D(this) -= other;
}


const Vec3D Vec3D::operator*( double scale ) const
{
	return Vec3D(this) *= scale;
}


const Vec3D Vec3D::operator/( double scale ) const
{
	return Vec3D(this) /= scale;
}
