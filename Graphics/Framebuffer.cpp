/*
 *  Framebuffer.cpp
 */

#include "Framebuffer.h"
#include <cmath>
#include <cfloat>
#include "RaptorGame.h"


Framebuffer::Framebuffer( int x, int y, GLint texture_filter )
{
	Initialized = false;
	FramebufferHandle = 0;
	Texture = 0;
	Depthbuffer = 0;
	
	W = FRAMEBUFFER_DEFAULT_RES;
	if( x )
		W = x;
	H = W;
	if( y )
		H = y;
	
	TextureFilter = texture_filter;
	
	AspectRatio = ((float)( W )) / ((float)( H ));
	
	Initialize();
}


Framebuffer::~Framebuffer()
{
	Clear();
}


void Framebuffer::Clear( void )
{
	Initialized = false;
	
	if( FramebufferHandle )
		glDeleteFramebuffers( 1, &FramebufferHandle );
	FramebufferHandle = 0;
	
	if( Texture )
		glDeleteTextures( 1, &Texture );
	Texture = 0;
	
	if( Depthbuffer )
		glDeleteRenderbuffers( 1, &Depthbuffer );
	Depthbuffer = 0;
}


void Framebuffer::Initialize( void )
{
	// Don't attempt to reload if we've already tried to (unless we use Clear first).
	if( FramebufferHandle || Texture || Depthbuffer )
		return;
	
	// Framebuffer
	glGenFramebuffers( 1, &FramebufferHandle );
	glBindFramebuffer( GL_FRAMEBUFFER, FramebufferHandle );
	
	// Renderbuffer
	glGenTextures( 1, &Texture );
	glBindTexture( GL_TEXTURE_2D, Texture );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TextureFilter );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, Raptor::Game->Gfx.AF );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, W, H, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Texture, 0 );
	GLenum draw_buffers[ 1 ] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers( 1, draw_buffers );
	
	// Depthbuffer
	glGenRenderbuffers( 1, &Depthbuffer );
	glBindRenderbuffer( GL_RENDERBUFFER, Depthbuffer );
	glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, W, H );
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, Depthbuffer );
	
	// Make sure everything worked.
	if( glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE )
	{
		Initialized = true;
		
		// Just to be tidy, make the framebuffer transparent initially.
		// NOTE: This may not clear the entire thing if the framebuffer is larger than the currently selected viewport.
		glClearColor( 0.f, 0.f, 0.f, 0.f );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	}
	
	// Unbind things so we don't accidentally use them when we don't mean to.
	glBindTexture( GL_TEXTURE_2D, 0 );
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	
	LoadedTime.Reset();
}


void Framebuffer::Reload( void )
{
	Initialized = false;
	FramebufferHandle = 0;
	Texture = 0;
	Depthbuffer = 0;
	
	Initialize();
}


bool Framebuffer::Select( void )
{
	if( Initialized && (LoadedTime.ElapsedSeconds() > Raptor::Game->Res.ResetTime.ElapsedSeconds()) )
		Reload();
	
	if( Initialized )
	{
		glBindFramebuffer( GL_FRAMEBUFFER, FramebufferHandle );
		glViewport( 0, 0, W, H );
		return true;
	}
	
	return false;
}


void Framebuffer::Setup2D( void )
{
	Setup2D( 0, 0, W, H );
}


void Framebuffer::Setup2D( double y1, double y2 )
{
	double h = y2 - y1;
	double w = h * (double) W / (double) H;
	double extra = (w - h) / 2.;
	double x1 = y1 - extra;
	double x2 = y2 + extra;
	
	Setup2D( x1, y1, x2, y2 );
}


void Framebuffer::Setup2D( double x1, double y1, double x2, double y2 )
{
	glDisable( GL_DEPTH_TEST );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( x1, x2, y2, y1, -1, 1 );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
}


void Framebuffer::Setup3D( Camera *cam )
{
	Setup3D( cam->FOV,  cam->X, cam->Y, cam->Z,  cam->X + cam->Fwd.X, cam->Y + cam->Fwd.Y, cam->Z + cam->Fwd.Z,  cam->Up.X, cam->Up.Y, cam->Up.Z );
}


void Framebuffer::Setup3D( double fov_w, double cam_x, double cam_y, double cam_z, double yaw )
{
	Setup3D( fov_w,  cam_x, cam_y, cam_z,  cam_x + cos(yaw), cam_y + sin(yaw), cam_z,  0, 0, 1 );
}


void Framebuffer::Setup3D( double fov_w, double cam_x, double cam_y, double cam_z, double yaw, double pitch )
{
	Setup3D( fov_w,  cam_x, cam_y, cam_z,  cam_x + cos(yaw)*cos(pitch), cam_y + sin(yaw)*cos(pitch), cam_z + sin(pitch),  0, 0, 1 );
}


void Framebuffer::Setup3D( double fov_w, double cam_x, double cam_y, double cam_z, double cam_look_x, double cam_look_y, double cam_look_z, double cam_up_x, double cam_up_y, double cam_up_z )
{
	// If we pass FOV=0, calculate a good default.  4:3 is FOV 90, widescreen is scaled appropriately.
	if( fov_w == 0. )
		fov_w = 67.5 * AspectRatio;
	// If we pass FOV<0, treat its absolute value as fov_h.
	else if( fov_w < 0. )
		fov_w *= -AspectRatio;
	
	glEnable( GL_DEPTH_TEST );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( fov_w / AspectRatio, AspectRatio, 1., FLT_MAX );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	gluLookAt( cam_x, cam_y, cam_z,  cam_look_x, cam_look_y, cam_look_z,  cam_up_x, cam_up_y, cam_up_z );
}
