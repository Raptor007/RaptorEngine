/*
 *  Pos.h
 */

#pragma once
class Pos;

#include "PlatformSpecific.h"

#include <vector>
#include <list>
#include <set>
#include "Vec.h"


class Pos3D
{
public:
	double X, Y, Z;
	Vec3D Fwd;
	Vec3D Up;
	Vec3D Right;
	
	Pos3D( void );
	Pos3D( const Pos3D &other );
	Pos3D( const Pos3D *other );
	Pos3D( double x, double y, double z );
	virtual ~Pos3D();
	
	void Copy( const Pos3D *other );
	
	void SetPos( double x, double y, double z );
	void SetFwdVec( double w, double u, double v );
	void SetUpVec( double w, double u, double v );
	void SetFwdPt( double x, double y, double z );
	void SetUpPt( double x, double y, double z );
	void UpdateRight( void );
	void FixVectors( void );
	
	void Yaw( double degrees );
	void Pitch( double degrees );
	void Roll( double degrees );
	void Move( double dx, double dy, double dz );
	void MoveAlong( const Vec3D *vec, double dist );
	void MoveAlong( double w, double u, double v, double dist );
	
	double AheadX( void ) const;
	double AheadY( void ) const;
	double AheadZ( void ) const;
	
	double AboveX( void ) const;
	double AboveY( void ) const;
	double AboveZ( void ) const;
	
	double Dist( const Pos3D *other ) const;
	double DistAlong( const Vec3D *vec, const Pos3D *from ) const;
	
	Pos3D *Nearest( const std::vector<Pos3D*> *others );
	Pos3D *Nearest( const std::list<Pos3D*> *others );
	Pos3D *Nearest( const std::vector<Pos3D*> *others, const std::set<Pos3D*> *except );
	Pos3D *Nearest( const std::list<Pos3D*> *others, const std::set<Pos3D*> *except );
	
	virtual Pos3D &operator +=( const Vec3D &vec );
	virtual Pos3D &operator -=( const Vec3D &vec );
	virtual Pos3D &operator +=( const Pos3D &other );
	virtual Pos3D &operator -=( const Pos3D &other );
	
	virtual const Pos3D operator+( const Vec3D &other ) const;
	virtual const Pos3D operator-( const Vec3D &other ) const;
};
