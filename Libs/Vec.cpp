/*
 *  Vec.cpp
 */

#include "Vec.h"

#include <cmath>
#include "Num.h"


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
	if( old_length )
	{
		X *= length / old_length;
		Y *= length / old_length;
	}
}


Vec2D Vec2D::Unit( void ) const
{
	double old_length = sqrt( X*X + Y*Y );
	if( old_length )
		return Vec2D( X / old_length, Y / old_length );
	return *this;
}


void Vec2D::Rotate( double degrees )
{
	double radians = Num::DegToRad(degrees);
	double x = X, y = Y;
	X = x * cos(radians) - y * sin(radians);
	Y = y * cos(radians) + x * sin(radians);
}

void Vec2D::Rotate( double degrees, const Vec2D *anchor )
{
	X -= anchor->X;
	Y -= anchor->Y;
	
	Rotate( degrees );
	
	X += anchor->X;
	Y += anchor->Y;
}


double Vec2D::Dot( const Vec2D &other ) const
{
	return Dot( other.X, other.Y );
}

double Vec2D::Dot( double x, double y ) const
{
	return X*x + Y*y;
}


Vec2D Vec2D::Reflect( const Vec2D *normal ) const
{
	double dp_x2 = Dot(normal) * 2.0;
	return Vec2D( X - dp_x2 * normal->X, Y - dp_x2 * normal->Y );
}


Vec2D Vec2D::ReflectAnySide( const Vec2D *normal ) const
{
	if( Dot(normal) <= 0. )
		return Reflect( normal );
	else
	{
		Vec2D new_normal( normal->X * -1., normal->Y * -1. );
		return Reflect( &new_normal );
	}
}


Vec2D &Vec2D::operator = ( const Vec2D &other )
{
	X = other.X;
	Y = other.Y;
	return *this;
}


Vec2D &Vec2D::operator += ( const Vec2D &other )
{
	X += other.X;
	Y += other.Y;
	return *this;
}


Vec2D &Vec2D::operator -= ( const Vec2D &other )
{
	X -= other.X;
	Y -= other.Y;
	return *this;
}


Vec2D &Vec2D::operator *= ( double scale )
{
	ScaleBy( scale );
	return *this;
}


Vec2D &Vec2D::operator /= ( double scale )
{
	ScaleBy( 1. / scale );
	return *this;
}


const Vec2D Vec2D::operator + ( const Vec2D &other ) const
{
	return Vec2D(this) += other;
}


const Vec2D Vec2D::operator - ( const Vec2D &other ) const
{
	return Vec2D(this) -= other;
}


const Vec2D Vec2D::operator * ( double scale ) const
{
	return Vec2D(this) *= scale;
}


const Vec2D Vec2D::operator / ( double scale ) const
{
	return Vec2D(this) /= scale;
}


bool Vec2D::operator < ( const Vec2D &other ) const
{
	if( X != other.X )
		return (X < other.X);
	if( Y != other.Y )
		return (Y < other.Y);
	return false;
}


bool Vec2D::operator == ( const Vec2D &other ) const
{
	return (X == other.X) && (Y == other.Y);
}


// ---------------------------------------------------------------------------


Vec3D::Vec3D( const Vec3D &other ) : Vec2D( other.X, other.Y )
{
	Z = other.Z;
}


Vec3D::Vec3D( const Vec3D *other ) : Vec2D( other->X, other->Y )
{
	Z = other->Z;
}


Vec3D::Vec3D( const Vec2D &other ) : Vec2D( other.X, other.Y )
{
	Z = 0.;
}


Vec3D::Vec3D( const Vec2D *other ) : Vec2D( other->X, other->Y )
{
	Z = 0.;
}


Vec3D::Vec3D( double x, double y, double z ) : Vec2D( x, y )
{
	Z = z;
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
	X = x;
	Y = y;
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
	if( old_length )
	{
		X *= length / old_length;
		Y *= length / old_length;
		Z *= length / old_length;
	}
}


Vec3D Vec3D::Unit( void ) const
{
	double old_length = sqrt( X*X + Y*Y + Z*Z );
	if( old_length )
		return Vec3D( X / old_length, Y / old_length, Z / old_length );
	return *this;
}


void Vec3D::RotateAround( const Vec3D *axis, double degrees )
{
	// http://inside.mines.edu/~gmurray/ArbitraryAxisRotation/
	
	// Convert to radians for the maths.
	double radians = Num::DegToRad(degrees);
	
	// Keep old X,Y,Z values to use them as inputs for the new values.
	double x = X, y = Y, z = Z;
	
	double u = 0., v = 0., w = 1.;
	if( axis )
	{
		// Convert the axis to a unit vector (scale to length 1).
		double axis_length = sqrt( axis->X * axis->X + axis->Y * axis->Y + axis->Z * axis->Z );
		u = axis->X / axis_length;
		v = axis->Y / axis_length;
		w = axis->Z / axis_length;
	}
	
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


double Vec3D::DotPlane( const Vec3D &plane_normal ) const
{
	return AlongPlane( plane_normal ).Length();
}


Vec3D Vec3D::Cross( const Vec3D &other ) const
{
	Vec3D cross;
	cross.X = Y * other.Z - Z * other.Y;
	cross.Y = Z * other.X - X * other.Z;
	cross.Z = X * other.Y - Y * other.X;
	return cross;
}


double Vec3D::AngleBetween( const Vec3D &other ) const
{
	Vec3D unit1 = this, unit2 = other;
	unit1.ScaleTo( 1. );
	unit2.ScaleTo( 1. );
	return Num::RadToDeg( acos( Num::Clamp( unit1.Dot(&unit2), -1., 1. ) ) );
}


Vec3D Vec3D::AlongPlane( const Vec3D &plane_normal ) const
{
	return *this - plane_normal * Dot(plane_normal);
}


Vec3D Vec3D::Reflect( const Vec3D *normal ) const
{
	// Note: The general formula is: Out = In - 2*Normal*(In dot Normal)
	
	double dp_x2 = Dot(normal) * 2.;
	return Vec3D( X - dp_x2 * normal->X, Y - dp_x2 * normal->Y, Z - dp_x2 * normal->Z );
}

Vec3D Vec3D::ReflectAnySide( const Vec3D *normal ) const
{
	if( Dot(normal) <= 0. )
		return Reflect( normal );
	else
	{
		Vec3D new_normal( normal->X * -1., normal->Y * -1., normal->Z * -1. );
		return Reflect( &new_normal );
	}
}


Vec3D &Vec3D::operator = ( const Vec3D &other )
{
	X = other.X;
	Y = other.Y;
	Z = other.Z;
	return *this;
}


Vec3D &Vec3D::operator += ( const Vec3D &other )
{
	X += other.X;
	Y += other.Y;
	Z += other.Z;
	return *this;
}


Vec3D &Vec3D::operator -= ( const Vec3D &other )
{
	X -= other.X;
	Y -= other.Y;
	Z -= other.Z;
	return *this;
}


Vec3D &Vec3D::operator *= ( double scale )
{
	ScaleBy( scale );
	return *this;
}


Vec3D &Vec3D::operator /= ( double scale )
{
	ScaleBy( 1. / scale );
	return *this;
}


const Vec3D Vec3D::operator + ( const Vec3D &other ) const
{
	return Vec3D(this) += other;
}


const Vec3D Vec3D::operator - ( const Vec3D &other ) const
{
	return Vec3D(this) -= other;
}


const Vec3D Vec3D::operator * ( double scale ) const
{
	return Vec3D(this) *= scale;
}


const Vec3D Vec3D::operator / ( double scale ) const
{
	return Vec3D(this) /= scale;
}


bool Vec3D::operator < ( const Vec3D &other ) const
{
	if( X != other.X )
		return (X < other.X);
	if( Y != other.Y )
		return (Y < other.Y);
	if( Z != other.Z )
		return (Z < other.Z);
	return false;
}


bool Vec3D::operator == ( const Vec3D &other ) const
{
	return (X == other.X) && (Y == other.Y) && (Z == other.Z);
}
