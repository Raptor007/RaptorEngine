/*
 *  Camera.cpp
 */

#include "Camera.h"

#include "RaptorGame.h"


Camera::Camera( void )
{
	// Negative values indicate FOVH instead of FOVW.
	FOV = -60.;
	
	SetPos( 0., 0., 0. );
	SetFwdVec( 0., 1., 0. );
	SetUpVec( 0., 0., 1. );
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
