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
	Pos3D Offset;
	
	Camera( void );
	Camera( const Camera &other );
	virtual ~Camera();
	
	double FOVW( void ) const;
	double FOVH( void ) const;
	
	void SetupGraphics( Graphics *gfx ) const;
	
	void Copy( const Pos3D *pos );
};
