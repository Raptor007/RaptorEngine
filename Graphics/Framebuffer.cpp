/*
 *  Framebuffer.cpp
 */

#include "Framebuffer.h"

#include <cstddef>
#include <cmath>
#include <cfloat>
#include <string>
#include "Num.h"
#include "RaptorGame.h"


Framebuffer::Framebuffer( int x, int y, GLint texture_filter )
{
	Initialized = false;
	FramebufferHandle = 0;
	Texture = 0;
	Depthbuffer = 0;
	AllocW = 0;
	AllocH = 0;
	OffsetX = 0;
	
	W = x ? x : FRAMEBUFFER_DEFAULT_RES;
	H = y ? y : W;
	
	TextureFilter = texture_filter;
	
	AspectRatio = ((float)( W )) / ((float)( H ));

	#if GL_ARB_texture_non_power_of_two
		ForcePowerOfTwo = ! Raptor::Game->Cfg.SettingAsBool( "g_framebuffers_anyres", true );
	#else
		ForcePowerOfTwo = true;
	#endif
	
	Initialize();
}


Framebuffer::~Framebuffer()
{
	Clear();
}


void Framebuffer::Clear( void )
{
	if( Initialized && (LoadedTime.ElapsedSeconds() > Raptor::Game->Res.ResetTime.ElapsedSeconds()) )
	{
		// No OpenGL cleanup if we already lost the context.
		FramebufferHandle = 0;
		Texture = 0;
		Depthbuffer = 0;
	}
	
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
	
	// If framebuffers are disabled or unavailable, create a blank 2x2 texture instead.
	if( ! (Raptor::Game->Gfx.Framebuffers && glGenFramebuffers) )
	{
		glGenTextures( 1, &Texture );
		glBindTexture( GL_TEXTURE_2D, Texture );
		glEnable( GL_TEXTURE_2D );
		unsigned char raw[ 16 ] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, raw );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TextureFilter );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glBindTexture( GL_TEXTURE_2D, 0 );
		LoadedTime.Reset();
		return;
	}
	
	// Some systems require power-of-two dimensions.
	if( ForcePowerOfTwo )
	{
		AllocW = Num::NextPowerOfTwo(W);
		AllocH = Num::NextPowerOfTwo(H);
	}
	else
	{
		AllocW = W;
		AllocH = H;
	}
	
	// Limit framebuffers to maximum texture resolution.
	GLint tex_max = Raptor::Game->Cfg.SettingAsInt( "g_texture_maxres", 0 );
	if( ! tex_max )
		glGetIntegerv( GL_MAX_TEXTURE_SIZE, &tex_max );
	if( tex_max > 0 )
	{
		if( ForcePowerOfTwo )
		{
			int tex_max_pow2 = Num::NextPowerOfTwo(tex_max);
			tex_max = (tex_max_pow2 > tex_max) ? (tex_max_pow2 / 2) : tex_max_pow2;
		}
		if( AllocW > tex_max )
			AllocW = tex_max;
		if( AllocH > tex_max )
			AllocH = tex_max;
	}
	
	// Framebuffer
	glGenFramebuffers( 1, &FramebufferHandle );
	glBindFramebuffer( GL_FRAMEBUFFER, FramebufferHandle );
	
	// Renderbuffer
	glGenTextures( 1, &Texture );
	glBindTexture( GL_TEXTURE_2D, Texture );
	void *raw = malloc( AllocW * AllocH * 4 );
	memset( raw, 0, AllocW * AllocH * 4 );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, AllocW, AllocH, 0, GL_RGBA, GL_UNSIGNED_BYTE, raw );
	free( raw );
	raw = NULL;
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TextureFilter );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Texture, 0 );
	GLenum draw_buffers[ 1 ] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers( 1, draw_buffers );
	
	// Depthbuffer
	glGenRenderbuffers( 1, &Depthbuffer );
	glBindRenderbuffer( GL_RENDERBUFFER, Depthbuffer );
	glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, AllocW, AllocH );
	glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, Depthbuffer );
	
	// Make sure everything worked.
	GLenum framebuffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if( framebuffer_status == GL_FRAMEBUFFER_COMPLETE )
		Initialized = true;
#ifdef GL_FRAMEBUFFER_UNSUPPORTED
	else if( (framebuffer_status == GL_FRAMEBUFFER_UNSUPPORTED) && ! ForcePowerOfTwo )
	{
		ForcePowerOfTwo = true;
		Clear();
		Reload();
	}
#endif
	else
	{
		std::string status_string = "UNKNOWN ERROR";
		if( framebuffer_status == GL_INVALID_ENUM )
			status_string = "GL_INVALID_ENUM";
#ifdef GL_FRAMEBUFFER_UNDEFINED
		else if( framebuffer_status == GL_FRAMEBUFFER_UNDEFINED )
			status_string = "GL_FRAMEBUFFER_UNDEFINED";
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT
		else if( framebuffer_status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT )
			status_string = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT
		else if( framebuffer_status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT )
			status_string = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER
		else if( framebuffer_status == GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER )
			status_string = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER
		else if( framebuffer_status == GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER )
			status_string = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
#endif
#ifdef GL_FRAMEBUFFER_UNSUPPORTED
		else if( framebuffer_status == GL_FRAMEBUFFER_UNSUPPORTED )
			status_string = "GL_FRAMEBUFFER_UNSUPPORTED";
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE
		else if( framebuffer_status == GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE )
			status_string = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
#endif
#ifdef GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS
		else if( framebuffer_status == GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS )
			status_string = "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
#endif
		else
			status_string = std::string("0x") + Num::ToHexString( framebuffer_status );
		
		Raptor::Game->Console.Print( std::string("Framebuffer::Initialize: OpenGL Error ") + status_string, TextConsole::MSG_ERROR );
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
		SetViewport();
		return true;
	}
	
	return false;
}


void Framebuffer::SetViewport( void )
{
	// Correct for VR offset.
	if( OffsetX >= 0 )
		glViewport( 0, 0, AllocW + OffsetX, AllocH );
	else
		glViewport( OffsetX, 0, AllocW - OffsetX, AllocH );
}


void Framebuffer::SetViewport( int x, int y, int w, int h )
{
	// Correct for forced power-of-two framebuffers.
	if( W != AllocW )
	{
		x = (x / (float) W) * AllocW + 0.5f;
		w = (w / (float) W) * AllocW + 0.5f;
	}
	if( H != AllocH )
	{
		y = (y / (float) H) * AllocH + 0.5f;
		h = (h / (float) H) * AllocH + 0.5f;
	}
	
	// Correct for VR offset.
	glViewport( x + OffsetX / 2, AllocH - y - h, w, h );  // 0,0 is top left for RaptorEngine but bottom left for OpenGL.
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
	float aspect_ratio = (W + abs(OffsetX)) / (float) H;
	
	// If we pass FOV=0, calculate a good default.  4:3 is FOV 80, widescreen is scaled appropriately.
	if( fov_w == 0. )
		fov_w = 60. * aspect_ratio;
	// If we pass FOV<0, treat its absolute value as fov_h.
	else if( fov_w < 0. )
		fov_w *= -aspect_ratio;
	
	glEnable( GL_DEPTH_TEST );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( fov_w / aspect_ratio, aspect_ratio, Raptor::Game->Gfx.ZNear, Raptor::Game->Gfx.ZFar );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	gluLookAt( cam_x, cam_y, cam_z,  cam_look_x, cam_look_y, cam_look_z,  cam_up_x, cam_up_y, cam_up_z );
}
