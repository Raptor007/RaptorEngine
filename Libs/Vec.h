/*
 *  Vec.h
 */

#pragma once
class Vec2D;
class Vec3D;

#include "PlatformSpecific.h"


class Vec2D
{
public:
	double X, Y;
	
	Vec2D( const Vec2D &other );
	Vec2D( const Vec2D *other );
	Vec2D( double x = 0., double y = 0. );
	virtual ~Vec2D();
	
	virtual void Copy( const Vec2D &other );
	virtual void Set( double x, double y );
	
	virtual double Length( void ) const;
	
	virtual void ScaleBy( double factor );
	virtual void ScaleTo( double length );
	Vec2D Unit( void ) const;
	
	virtual void Rotate( double degrees );
	virtual void Rotate( double degrees, const Vec2D *anchor );
	
	virtual double Dot( const Vec2D &other ) const;
	virtual double Dot( double x, double y ) const;

	virtual Vec2D Reflect( const Vec2D *normal ) const;
	virtual Vec2D ReflectAnySide( const Vec2D *normal ) const;
	
	virtual Vec2D &operator = ( const Vec2D &other );
	virtual Vec2D &operator += ( const Vec2D &other );
	virtual Vec2D &operator -= ( const Vec2D &other );
	virtual Vec2D &operator *= ( double scale );
	virtual Vec2D &operator /= ( double scale );
	virtual const Vec2D operator + ( const Vec2D &other ) const;
	virtual const Vec2D operator - ( const Vec2D &other ) const;
	const Vec2D operator * ( double scale ) const;
	const Vec2D operator / ( double scale ) const;
	bool operator < ( const Vec2D &other ) const;
	bool operator == ( const Vec2D &other ) const;
};


class Vec3D : public Vec2D
{
public:
	double Z;
	
	Vec3D( const Vec3D &other );
	Vec3D( const Vec3D *other );
	Vec3D( const Vec2D &other );
	Vec3D( const Vec2D *other );
	Vec3D( double x = 0., double y = 0., double z = 0. );
	virtual ~Vec3D();
	
	virtual void Copy( const Vec3D &other );
	virtual void Set( double x, double y, double z );
	
	virtual double Length( void ) const;
	
	virtual void ScaleBy( double factor );
	virtual void ScaleTo( double length );
	Vec3D Unit( void ) const;
	
	virtual void RotateAround( const Vec3D *axis, double degrees );
	virtual void RotateAround( const Vec3D *axis, double degrees, const Vec3D *anchor );
	
	virtual double Dot( const Vec3D &other ) const;
	virtual double Dot( double x, double y, double z ) const;
	virtual double DotPlane( const Vec3D &plane_normal ) const;
	virtual Vec3D Cross( const Vec3D &other ) const;
	virtual double AngleBetween( const Vec3D &other ) const;
	
	virtual Vec3D AlongPlane( const Vec3D &plane_normal ) const;
	virtual Vec3D Reflect( const Vec3D *normal ) const;
	virtual Vec3D ReflectAnySide( const Vec3D *normal ) const;
	
	virtual Vec3D &operator = ( const Vec3D &other );
	virtual Vec3D &operator += ( const Vec3D &other );
	virtual Vec3D &operator -= ( const Vec3D &other );
	virtual Vec3D &operator *= ( double scale );
	virtual Vec3D &operator /= ( double scale );
	virtual const Vec3D operator + ( const Vec3D &other ) const;
	virtual const Vec3D operator - ( const Vec3D &other ) const;
	const Vec3D operator * ( double scale ) const;
	const Vec3D operator / ( double scale ) const;
	bool operator < ( const Vec3D &other ) const;
	bool operator == ( const Vec3D &other ) const;
};
