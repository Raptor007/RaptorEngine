/*
 *  Camera.cpp
 */

#include "Camera.h"


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


void Camera::SetupGraphics( Graphics *gfx )
{
	gfx->Setup3D( FOV, X, Y, Z, X+Fwd.X, Y+Fwd.Y, Z+Fwd.Z, X+Up.X, Y+Up.Y, Z+Up.Z );
}
