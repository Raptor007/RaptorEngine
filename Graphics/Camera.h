/*
 *  Camera.h
 */

#pragma once
class Camera;

#include "platforms.h"

#include "Pos.h"
#include "Graphics.h"


class Camera : public Pos3D
{
public:
	double FOV;
	
	Camera( void );
	Camera( const Camera &other );
	virtual ~Camera();
	
	void SetupGraphics( Graphics *gfx );
};
