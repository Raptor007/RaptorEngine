/*
 *  Camera.h
 */

#pragma once
class Camera;

#include "PlatformSpecific.h"

#include "Pos.h"
#include "Graphics.h"


class Camera : public Pos3D
{
public:
	double FOV;
	
	Camera( void );
	Camera( const Camera &other );
	virtual ~Camera();
	
	double FOVW( void ) const;
	double FOVH( void ) const;
	
	void SetupGraphics( Graphics *gfx ) const;
};
