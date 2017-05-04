/*
 *  Graphics.h
 */

#pragma once
class Graphics;

#include "PlatformSpecific.h"

#include <SDL/SDL.h>
#include "RaptorGL.h"

#ifdef __APPLE__
	#include <SDL_ttf/SDL_ttf.h>
#else
	#include <SDL/SDL_ttf.h>
#endif

#include <string>
#include "Camera.h"
#include "Framebuffer.h"


class Graphics
{
public:
	SDL_Surface *Screen;
	bool Initialized;
	int W, H, RealW, RealH, BPP;
	float AspectRatio;
	bool Fullscreen;
	int FSAA, AF;
	double ZNear, ZFar;
	Framebuffer *DrawTo;
	
	Graphics( void );
	~Graphics();
	
	void Initialize( void );
	void SetMode( int x, int y );
	void SetMode( int x, int y, int bpp, bool fullscreen, int fsaa, int af, double z_near, double z_far );
	void Restart( void );
	
	bool SelectDefaultFramebuffer( void );
	bool SelectFramebuffer( Framebuffer *fb );
	
	void Clear( void );
	void Clear( GLfloat red, GLfloat green, GLfloat blue );
	void Clear( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha );
	void SwapBuffers( void );
	
	void SetViewport( void );
	void SetViewport( int x, int y, int w, int h );
	
	void Setup2D( void );
	void Setup2D( double y1, double y2 );
	void Setup2D( double x1, double y1, double x2, double y2 );
	void Setup3D( Camera *cam );
	void Setup3D( double fov_w, double cam_x, double cam_y, double cam_z, double yaw );
	void Setup3D( double fov_w, double cam_x, double cam_y, double cam_z, double yaw, double pitch );
	void Setup3D( double fov_w, double cam_x, double cam_y, double cam_z, double cam_look_x, double cam_look_y, double cam_look_z, double cam_up_x, double cam_up_y, double cam_up_z );
	
	void DrawRect2D( int x1, int y1, int x2, int y2, GLuint texture = 0 );
	void DrawRect2D( double x1, double y1, double x2, double y2, GLuint texture = 0 );
	void DrawRect2D( int x1, int y1, int x2, int y2, GLuint texture, float r, float g, float b, float a );
	void DrawRect2D( double x1, double y1, double x2, double y2, GLuint texture, float r, float g, float b, float a );
	
	void DrawBox2D( double x1, double y1, double x2, double y2, float line_width, float r, float g, float b, float a );
	
	void DrawCircle2D( double x, double y, double r, int res, GLuint texture = 0 );
	void DrawCircle2D( double x, double y, double r, int res, GLuint texture, float red, float green, float blue, float alpha );
	
	void DrawCircleOutline2D( double x, double y, double r, int res, float line_width );
	void DrawCircleOutline2D( double x, double y, double r, int res, float line_width, float red, float green, float blue, float alpha );
	
	void DrawLine2D( double x1, double y1, double x2, double y2, float line_width, float r, float g, float b, float a );
	
	void DrawSphere3D( double x, double y, double z, double r, int res, GLuint texture = 0, uint32_t texture_mode = TEXTURE_MODE_Y_ASIN );
	void DrawSphere3D( double x, double y, double z, double r, int res, GLuint texture, float red, float green, float blue, float alpha );
	void DrawSphere3D( double x, double y, double z, double r, int res, GLuint texture, uint32_t texture_mode, float red, float green, float blue, float alpha );
	
	void DrawLine3D( double x1, double y1, double z1, double x2, double y2, double z2, float line_width, float r, float g, float b, float a );
	
	void DrawBox3D( const Pos3D *corner, const Vec3D *fwd, const Vec3D *up, const Vec3D *right, float line_width, float r, float g, float b, float a );
	
	GLuint MakeTexture( SDL_Surface *surface, GLint texture_filter = GL_LINEAR_MIPMAP_LINEAR, GLint texture_wrap = GL_REPEAT, GLfloat *texcoord = NULL );
	
	enum
	{
		TEXTURE_MODE_Y_ASIN = 0x01,
		TEXTURE_MODE_X_DIV_R = 0x02
	};
};
