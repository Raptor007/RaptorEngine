/*
 *  Framebuffer.h
 */

#pragma once
class Framebuffer;

#include "PlatformSpecific.h"

#include "RaptorGL.h"
#include "Camera.h"
#include "Clock.h"

#define FRAMEBUFFER_DEFAULT_RES 512


class Framebuffer
{
public:
	bool Initialized;
	GLuint FramebufferHandle;
	GLuint Texture;
	GLuint Depthbuffer;
	int W, H, AllocW, AllocH;
	GLint TextureFilter;
	float AspectRatio;
	bool ForcePowerOfTwo;
	Clock LoadedTime;
	
	Framebuffer( int x = FRAMEBUFFER_DEFAULT_RES, int y = 0, GLint texture_filter = GL_LINEAR );
	~Framebuffer();
	
	void Clear( void );
	void Initialize( void );
	void Reload( void );
	
	bool Select( void );
	
	void Setup2D( void );
	void Setup2D( double y1, double y2 );
	void Setup2D( double x1, double y1, double x2, double y2 );
	void Setup3D( Camera *cam );
	void Setup3D( double fov_w, double cam_x, double cam_y, double cam_z, double yaw );
	void Setup3D( double fov_w, double cam_x, double cam_y, double cam_z, double yaw, double pitch );
	void Setup3D( double fov_w, double cam_x, double cam_y, double cam_z, double cam_look_x, double cam_look_y, double cam_look_z, double cam_up_x, double cam_up_y, double cam_up_z );
};
