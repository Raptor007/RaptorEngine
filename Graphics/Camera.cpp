/*
 *  Camera.cpp
 */

#include "Camera.h"

#include "RaptorGame.h"


Camera::Camera( void )
{
	// Negative values indicate FOVH instead of FOVW.
	FOV = -60.;
	
	// Worldspace
	SetPos(0,0,0);
	SetFwdVec(0,1,0);
	SetUpVec(0,0,1);
	
	// Modelspace
	Offset.SetPos(0,0,0);
	Offset.SetFwdVec(1,0,0);
	Offset.SetUpVec(0,1,0);
}


Camera::Camera( const Camera &other ) : Pos3D( other )
{
	FOV = other.FOV;
}


Camera::~Camera()
{
}


double Camera::FOVW( void ) const
{
	if( FOV > 0 )
		return FOV;
	else if( FOV < 0 )
		return -FOV * Raptor::Game->Gfx.AspectRatio;
	else
		return 60. * Raptor::Game->Gfx.AspectRatio;
}


double Camera::FOVH( void ) const
{
	if( FOV > 0 )
		return FOV / Raptor::Game->Gfx.AspectRatio;
	else if( FOV < 0 )
		return -FOV;
	else
		return 60.;
}


void Camera::SetupGraphics( Graphics *gfx ) const
{
	gfx->Setup3D( FOV, X, Y, Z, X+Fwd.X, Y+Fwd.Y, Z+Fwd.Z, Up.X, Up.Y, Up.Z );
}


void Camera::Copy( const Pos3D *pos )
{
	Pos3D::Copy( pos );
	
	// In model space, X = fwd, Y = up, Z = right.
	MoveAlong( &Fwd,   Offset.X );
	MoveAlong( &Up,    Offset.Y );
	MoveAlong( &Right, Offset.Z );
	Vec3D x_vec( pos->Fwd.X, pos->Up.X, pos->Right.X );
	Vec3D y_vec( pos->Fwd.Y, pos->Up.Y, pos->Right.Y );
	Vec3D z_vec( pos->Fwd.Z, pos->Up.Z, pos->Right.Z );
	SetUpVec(  Offset.Up.Dot( &x_vec), Offset.Up.Dot( &y_vec), Offset.Up.Dot( &z_vec) );
	SetFwdVec( Offset.Fwd.Dot(&x_vec), Offset.Fwd.Dot(&y_vec), Offset.Fwd.Dot(&z_vec) );
}
