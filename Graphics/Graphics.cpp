/*
 *  Graphics.cpp
 */

#include "Graphics.h"

#include <cstddef>
#include <cmath>
#include <SDL/SDL.h>
#include "Endian.h"
#include "Num.h"
#include "ResourceManager.h"
#include "ClientConfig.h"
#include "RaptorGame.h"


Graphics::Graphics( void )
{
	Initialized = false;
	
	W = 640;
	H = 480;
	RealW = W;
	RealH = H;
	DesktopW = 0;
	DesktopH = 0;
	AspectRatio = ((float)( W )) / ((float)( H ));
	BPP = 32;
	ZNear = 0.125;
	ZFar = 15000.;
	Fullscreen = false;
	VSync = false;
	FSAA = 0;
	AF = 16;
	Framebuffers = true;
	
	Screen = NULL;
	DrawTo = NULL;
}


Graphics::~Graphics()
{
	// Don't free this, because it causes EXC_BAD_ACCESS.
	Screen = NULL;
	
	if( Initialized )
	{
		SDL_ShowCursor( SDL_ENABLE );
		TTF_Quit();
	}
	
	Initialized = false;
}


void Graphics::Initialize( void )
{
	// Initialize SDL.
	if( SDL_Init( SDL_INIT_VIDEO ) != 0 )
	{
		fprintf( stderr, "Unable to initialize SDL: %s\n", SDL_GetError() );
		return;
	};
	
	// Get the desktop resolution.
	const SDL_VideoInfo *info = SDL_GetVideoInfo();
	if( info )
	{
		DesktopW = info->current_w;
		DesktopH = info->current_h;
	}
	
	// Default window position should be centered.
	char sdl_env[] = "SDL_VIDEO_WINDOW_POS=center";
	SDL_putenv( sdl_env );
	
	// Initialize SDL_ttf for text rendering.
	TTF_Init();
	
	Initialized = true;
	
	// Configure mode.
	Restart();
}


void Graphics::SetMode( int x, int y )
{
	SetMode( x, y, BPP, Fullscreen, FSAA, AF, ZBits );
}


void Graphics::SetMode( int x, int y, int bpp, bool fullscreen, int fsaa, int af, int zbits )
{
	// Make sure we've initialized SDL.
	if( ! Initialized )
		Initialize();
	if( ! Initialized )
		return;
	
	// Going to 0x0 should select resolution automatically.
	if( !(x || y) )
	{
		if( fullscreen )
		{
			x = DesktopW;
			y = DesktopH;
		}
		else if( DesktopW && DesktopH )
		{
			x = DesktopW - 20;
			y = DesktopH - 100;
			if( x > y * AspectRatio )
				x = y * AspectRatio;
			else
				y = x / AspectRatio;
		}
	}
	
	// Set up the video properties.
	W = x;
	H = y;
	BPP = bpp;
	AspectRatio = (W && H) ? ((float)( W )) / ((float)( H )) : 1.f;
	Fullscreen = fullscreen;
	FSAA = fsaa;
	AF = af;
	ZBits = zbits;
	
	// Make sure we use hardware-accelerated double-buffered OpenGL.
	SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	
	// Set vsync.
	SDL_GL_SetAttribute( SDL_GL_SWAP_CONTROL, VSync ? 1 : 0 );
	
	// Set minimum depth buffer.
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, ZBits );
	
	// Enable multi-sample anti-aliasing.
	if( FSAA > 1 )
	{
		SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 );
		SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, FSAA );
	}
	else
	{
		FSAA = 0;
		SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 0 );
		SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 1 );
	}
	
	// The current OpenGL context will be invalid, so clear all textures from the ResourceManager first.
	Raptor::Game->Res.DeleteGraphics();
	
	// Free the current screen.
	if( Screen )
		SDL_FreeSurface( Screen );
	
	#ifndef __APPLE__
		// Set the titlebar icon.
		SDL_Surface *icon = SDL_LoadBMP( Raptor::Game->Res.Find("icon.bmp").c_str() );
		if( icon )
		{
			SDL_WM_SetIcon( icon, NULL );
			SDL_FreeSurface( icon );
		}
	#endif
	
	// Create a new screen.
	Screen = SDL_SetVideoMode( x, y, bpp, SDL_OPENGL | SDL_ANYFORMAT | (Fullscreen ? SDL_FULLSCREEN : SDL_RESIZABLE) );
	
	if( ! Screen )
	{
		fprintf( stderr, "Unable to set %s video mode %ix%ix%i: %s\n", (fullscreen ? "fullscreen" : "windowed"), x, y, bpp, SDL_GetError() );
		
		// We couldn't set that video mode, so try a few other options.
		if( DesktopW && DesktopH && ((x > DesktopW) || (y > DesktopH)) )
			SetMode( DesktopW, DesktopH, bpp, fullscreen, fsaa, af, 24 ); // Desktop Resolution
		else if( zbits > 24 )
			SetMode( x,   y,   bpp, fullscreen, fsaa, af, 24 );     // 24-bit Z-Depth
		else if( fullscreen && ((x != 640) || (y != 480)) )
			SetMode( 640, 480, bpp, fullscreen, fsaa, af, zbits );  // 640x480 Fullscreen
		else if( zbits > 16 )
			SetMode( x,   y,   bpp, fullscreen, fsaa, af, 16 );     // 16-bit Z-Depth
		else if( fullscreen )
			SetMode( 640, 480, bpp, false,      fsaa, af, zbits );  // 640x480 Windowed
		else if( bpp > 16 )
			SetMode( x,   y,   16,  fullscreen, fsaa, af, zbits );  // 16-bit Color
		else
		{
			fflush( stderr );
			exit( -1 );
		}
		return;
	}
	
	// We successfully set the video mode, so pull the resolution from the SDL_Screen.
	W = Screen->w;
	H = Screen->h;
	RealW = W;
	RealH = H;
	AspectRatio = (W && H) ? ((float)( W )) / ((float)( H )) : 1.f;
	BPP = Screen->format->BitsPerPixel;
	
	// If we weren't able to use the requested anti-aliasing samples, update the setting.
	int max_fsaa = 0;
	SDL_GL_GetAttribute( SDL_GL_MULTISAMPLESAMPLES, &max_fsaa );
	if( FSAA > max_fsaa )
		FSAA = max_fsaa;
	if( FSAA == 1 )
		FSAA = 0;
	
	// Determine maximum anisotropic filtering.
	if( AF < 1 )
		AF = 1;
	GLint max_af = 1;
	glGetIntegerv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_af );
	if( AF > max_af )
		AF = max_af;
	
	// Set hints to nicest.
	glHint( GL_GENERATE_MIPMAP_HINT, GL_NICEST );
	glHint( GL_TEXTURE_COMPRESSION_HINT, GL_NICEST );
	glHint( GL_FRAGMENT_SHADER_DERIVATIVE_HINT, GL_NICEST );
	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
	glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
	glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
	glHint( GL_POINT_SMOOTH_HINT, GL_NICEST );
	glHint( GL_FOG_HINT, GL_NICEST );
	
	#ifdef GL_PROGRAM_POINT_SIZE
		// Allow fixed-function point sizes.
		glDisable( GL_PROGRAM_POINT_SIZE );
	#endif
	
	// Tell the ResourceManager to reload any previously-loaded textures and shaders.
	Raptor::Game->Res.ReloadGraphics();
	
	// Set the titlebar name.
	SDL_WM_SetCaption( Raptor::Game->Game.c_str(), NULL );
	
	// Set the OpenGL state after creating the context with SDL_SetVideoMode.
	glViewport( 0, 0, W, H );
	
	// Enable alpha blending.
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	
	// Disable the system cursor.
	SDL_ShowCursor( SDL_DISABLE );
	
	// Update the client config.
	if( Fullscreen )
	{
		Raptor::Game->Cfg.Settings[ "g_fullscreen" ] = "true";
		
		// We use x,y instead of W,H so 0,0 (native res) can be retained.
		Raptor::Game->Cfg.Settings[ "g_res_fullscreen_x" ] = Num::ToString(x);
		Raptor::Game->Cfg.Settings[ "g_res_fullscreen_y" ] = Num::ToString(y);
	}
	else
	{
		Raptor::Game->Cfg.Settings[ "g_fullscreen" ] = "false";
		
		// We use x,y instead of W,H so 0,0 (native res) can be retained.
		Raptor::Game->Cfg.Settings[ "g_res_windowed_x" ] = Num::ToString(x);
		Raptor::Game->Cfg.Settings[ "g_res_windowed_y" ] = Num::ToString(y);
	}
	Raptor::Game->Cfg.Settings[ "g_bpp" ] = Num::ToString(BPP);
	Raptor::Game->Cfg.Settings[ "g_fsaa" ] = Num::ToString(FSAA);
	Raptor::Game->Cfg.Settings[ "g_af" ] = Num::ToString(AF);
	Raptor::Game->Cfg.Settings[ "g_znear" ] = Num::ToString(ZNear);
	Raptor::Game->Cfg.Settings[ "g_zfar" ] = Num::ToString(ZFar);
	Raptor::Game->Cfg.Settings[ "g_vsync" ] = VSync ? "true" : "false";
	Raptor::Game->Cfg.Settings[ "g_framebuffers" ] = Framebuffers ? "true" : "false";
}


void Graphics::Restart( void )
{
	int x = 640, y = 480;
	int bpp = Raptor::Game->Cfg.SettingAsInt( "g_bpp", 32 );
	bool fullscreen = Raptor::Game->Cfg.SettingAsBool( "g_fullscreen", true );
	int fsaa = Raptor::Game->Cfg.SettingAsInt( "g_fsaa", 0 );
	int af = Raptor::Game->Cfg.SettingAsInt( "g_af", 1 );
	if( fullscreen )
	{
		x = Raptor::Game->Cfg.SettingAsInt( "g_res_fullscreen_x", 640 );
		y = Raptor::Game->Cfg.SettingAsInt( "g_res_fullscreen_y", 480 );
	}
	else
	{
		x = Raptor::Game->Cfg.SettingAsInt( "g_res_windowed_x", 640 );
		y = Raptor::Game->Cfg.SettingAsInt( "g_res_windowed_y", 480 );
	}
	int zbits = Raptor::Game->Cfg.SettingAsInt( "g_zbits", 24 );
	
	ZNear = Raptor::Game->Cfg.SettingAsDouble( "g_znear", 0.125 );
	ZFar = Raptor::Game->Cfg.SettingAsDouble( "g_zfar", 15000 );
	VSync = Raptor::Game->Cfg.SettingAsBool( "g_vsync", false );
	Framebuffers = Raptor::Game->Cfg.SettingAsBool( "g_framebuffers", true );
	
	SetMode( x, y, bpp, fullscreen, fsaa, af, zbits );
}


bool Graphics::SelectDefaultFramebuffer( void )
{
	if( DrawTo && DrawTo->Select() )
	{
		W = DrawTo->W;
		H = DrawTo->H;
		AspectRatio = DrawTo->AspectRatio;
	}
	else
	{
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
		W = RealW;
		H = RealH;
		AspectRatio = (W && H) ? ((float)( W )) / ((float)( H )) : 1.f;
		glViewport( 0, 0, W, H );
	}
	return true;
}


bool Graphics::SelectFramebuffer( Framebuffer *fb )
{
	if( fb )
		return fb->Select();
	return false;
}


void Graphics::Clear( void )
{
	glClearColor( 0.f, 0.f, 0.f, 1.f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}


void Graphics::Clear( GLfloat red, GLfloat green, GLfloat blue )
{
	glClearColor( red, green, blue, 1.f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}


void Graphics::Clear( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha )
{
	glClearColor( red, green, blue, alpha );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}


void Graphics::SwapBuffers( void )
{
	SDL_GL_SwapBuffers();
}


void Graphics::SetViewport( void )
{
	if( DrawTo )
		DrawTo->SetViewport();
	else
		glViewport( 0, 0, W, H );
}


void Graphics::SetViewport( int x, int y, int w, int h )
{
	if( DrawTo )
		DrawTo->SetViewport( x, y, w, h );
	else
		glViewport( x, H - y - h, w, h );  // 0,0 is top left for RaptorEngine but bottom left for OpenGL.
}


void Graphics::Setup2D( void )
{
	Setup2D( 0, 0, W, H );
}


void Graphics::Setup2D( double y1, double y2 )
{
	double h = fabs( y2 - y1 );
	double w = h * (double) W / (double) H;
	double extra = (w - h) / 2.;
	double x1 = std::min<double>( y1, y2 ) - extra;
	double x2 = std::max<double>( y1, y2 ) + extra;
	
	Setup2D( x1, y1, x2, y2 );
}


void Graphics::Setup2D( double x1, double y1, double x2, double y2 )
{
	if( DrawTo )
	{
		DrawTo->Setup2D( x1, y1, x2, y2 );
		return;
	}
	
	glDisable( GL_DEPTH_TEST );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( x1, x2, y2, y1, -1, 1 );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
}


void Graphics::Setup3D( Camera *cam )
{
	Setup3D( cam->FOV,  cam->X, cam->Y, cam->Z,  cam->X + cam->Fwd.X, cam->Y + cam->Fwd.Y, cam->Z + cam->Fwd.Z,  cam->Up.X, cam->Up.Y, cam->Up.Z );
}


void Graphics::Setup3D( double fov_w, double cam_x, double cam_y, double cam_z, double yaw )
{
	Setup3D( fov_w,  cam_x, cam_y, cam_z,  cam_x + cos(yaw), cam_y + sin(yaw), cam_z,  0, 0, 1 );
}


void Graphics::Setup3D( double fov_w, double cam_x, double cam_y, double cam_z, double yaw, double pitch )
{
	Setup3D( fov_w,  cam_x, cam_y, cam_z,  cam_x + cos(yaw)*cos(pitch), cam_y + sin(yaw)*cos(pitch), cam_z + sin(pitch),  0, 0, 1 );
}


void Graphics::Setup3D( double fov_w, double cam_x, double cam_y, double cam_z, double cam_look_x, double cam_look_y, double cam_look_z, double cam_up_x, double cam_up_y, double cam_up_z )
{
	if( DrawTo )
	{
		DrawTo->Setup3D( fov_w, cam_x, cam_y, cam_z, cam_look_x, cam_look_y, cam_look_z, cam_up_x, cam_up_y, cam_up_z );
		return;
	}
	
	// If we pass FOV=0, calculate a good default.  4:3 is FOV 80, widescreen is scaled appropriately.
	if( fov_w == 0. )
		fov_w = 60. * AspectRatio;
	// If we pass FOV<0, treat its absolute value as fov_h.
	else if( fov_w < 0. )
		fov_w *= -AspectRatio;
	
	glEnable( GL_DEPTH_TEST );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( fov_w / AspectRatio, AspectRatio, ZNear, ZFar );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	gluLookAt( cam_x, cam_y, cam_z,  cam_look_x, cam_look_y, cam_look_z,  cam_up_x, cam_up_y, cam_up_z );
}


// ---------------------------------------------------------------------------


void Graphics::DrawRect2D( int x1, int y1, int x2, int y2, GLuint texture )
{
	if( texture )
	{
		glEnable( GL_TEXTURE_2D );
		glBindTexture( GL_TEXTURE_2D, texture );
	}
	
	glBegin( GL_QUADS );
		
		// Top-left
		if( texture )
			glTexCoord2i( 0, 0 );
		glVertex2i( x1, y1 );
		
		// Bottom-left
		if( texture )
			glTexCoord2i( 0, 1 );
		glVertex2i( x1, y2 );
		
		// Bottom-right
		if( texture )
			glTexCoord2i( 1, 1 );
		glVertex2i( x2, y2 );
		
		// Top-right
		if( texture )
			glTexCoord2i( 1, 0 );
		glVertex2i( x2, y1 );
		
	glEnd();
	
	if( texture )
		glDisable( GL_TEXTURE_2D );
}


void Graphics::DrawRect2D( double x1, double y1, double x2, double y2, GLuint texture )
{
	if( texture )
	{
		glEnable( GL_TEXTURE_2D );
		glBindTexture( GL_TEXTURE_2D, texture );
	}
	
	glBegin( GL_QUADS );
		
		// Top-left
		if( texture )
			glTexCoord2i( 0, 0 );
		glVertex2d( x1, y1 );
		
		// Bottom-left
		if( texture )
			glTexCoord2i( 0, 1 );
		glVertex2d( x1, y2 );
		
		// Bottom-right
		if( texture )
			glTexCoord2i( 1, 1 );
		glVertex2d( x2, y2 );
		
		// Top-right
		if( texture )
			glTexCoord2i( 1, 0 );
		glVertex2d( x2, y1 );
		
	glEnd();
	
	if( texture )
		glDisable( GL_TEXTURE_2D );
}


void Graphics::DrawRect2D( int x1, int y1, int x2, int y2, GLuint texture, float r, float g, float b, float a )
{
	glColor4f( r, g, b, a );
	DrawRect2D( x1, y1, x2, y2, texture );
}


void Graphics::DrawRect2D( double x1, double y1, double x2, double y2, GLuint texture, float r, float g, float b, float a )
{
	glColor4f( r, g, b, a );
	DrawRect2D( x1, y1, x2, y2, texture );
}


// ---------------------------------------------------------------------------


void Graphics::DrawBox2D( double x1, double y1, double x2, double y2, float line_width, float r, float g, float b, float a )
{
	glColor4f( r, g, b, a );
	glLineWidth( line_width );
	
	glBegin( GL_LINE_LOOP );
		glVertex2d( x1, y1 );
		glVertex2d( x1, y2 );
		glVertex2d( x2, y2 );
		glVertex2d( x2, y1 );
	glEnd();
}


// ---------------------------------------------------------------------------


void Graphics::DrawCircle2D( double x, double y, double r, int res, GLuint texture )
{
	DrawCircle2D( x, y, r, res, texture, 1.f, 1.f, 1.f, 1.f );
}


void Graphics::DrawCircle2D( double x, double y, double r, int res, GLuint texture, float red, float green, float blue, float alpha )
{
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, texture );
	glColor4f( red, green, blue, alpha );
	
	glBegin( GL_TRIANGLE_FAN );
		
		glTexCoord2d( 0.5, 0.5 );
		glVertex2d( x, y );
		
		for( int i = 0; i <= res; i ++ )
		{
			double percent = ((double) i) / (double) res;
			double unit_x = cos( percent * 2. * M_PI );
			double unit_y = sin( percent * 2. * M_PI );
			double px = x + unit_x * r;
			double py = y + unit_y * r;
			double tx = (unit_x + 1.) / 2.;
			double ty = (unit_y + 1.) / 2.;
			
			glTexCoord2d( tx, ty );
			glVertex2d( px, py );
		}
		
	glEnd();
	
	glDisable( GL_TEXTURE_2D );
}


// ---------------------------------------------------------------------------


void Graphics::DrawCircleOutline2D( double x, double y, double r, int res, float line_width )
{
	DrawCircleOutline2D( x, y, r, res, line_width, 1.f, 1.f, 1.f, 1.f );
}


void Graphics::DrawCircleOutline2D( double x, double y, double r, int res, float line_width, float red, float green, float blue, float alpha )
{
	glColor4f( red, green, blue, alpha );
	glLineWidth( line_width );
	
	glBegin( GL_LINE_STRIP );
		
		for( int i = 0; i <= res; i ++ )
		{
			double percent = ((double) i) / (double) res;
			double unit_x = cos( percent * 2. * M_PI );
			double unit_y = sin( percent * 2. * M_PI );
			double px = x + unit_x * r;
			double py = y + unit_y * r;
			
			glVertex2d( px, py );
		}
		
	glEnd();
}


// ---------------------------------------------------------------------------


void Graphics::DrawLine2D( double x1, double y1, double x2, double y2, float line_width, float r, float g, float b, float a )
{
	glColor4f( r, g, b, a );
	glLineWidth( line_width );
	
	glBegin( GL_LINES );
		glVertex2d( x1, y1 );
		glVertex2d( x2, y2 );
	glEnd();
}


// ---------------------------------------------------------------------------


void Graphics::DrawSphere3D( double x, double y, double z, double r, int res, GLuint texture, uint32_t texture_mode )
{
	DrawSphere3D( x, y, z, r, res, texture, texture_mode, 1.f, 1.f, 1.f, 1.f );
}


void Graphics::DrawSphere3D( double x, double y, double z, double r, int res, GLuint texture, float red, float green, float blue, float alpha )
{
	DrawSphere3D( x, y, z, r, res, texture, TEXTURE_MODE_Y_ASIN, red, green, blue, alpha );
}


void Graphics::DrawSphere3D( double x, double y, double z, double r, int res, GLuint texture, uint32_t texture_mode, float red, float green, float blue, float alpha )
{
	int res_vertical = res;
	int res_around = res * 2;
	double step_vertical = 1. / (double) res_vertical;
	double step_around = 1. / (double) res_around;
	
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, texture );
	glColor4f( red, green, blue, alpha );
	
	glBegin( GL_QUADS );
		
		for( int i = 0; i < res_vertical; i ++ )
		{
			double percent_vertical = ((double) i) / (double) res_vertical;
			double z1 = r * cos( percent_vertical * M_PI );
			double r1 = r * sin( percent_vertical * M_PI );
			double z2 = r * cos( (percent_vertical + step_vertical) * M_PI );
			double r2 = r * sin( (percent_vertical + step_vertical) * M_PI );
			
			for( int j = 0; j < res_around; j ++ )
			{
				double percent_around = ((double) j) / (double) res_around;
				double x1 = cos( percent_around * 2. * M_PI );
				double y1 = sin( percent_around * 2. * M_PI );
				double x2 = cos( (percent_around + step_around) * 2. * M_PI );
				double y2 = sin( (percent_around + step_around) * 2. * M_PI );
				
				// Texture coordinates.
				double tc[ 4 ][ 2 ];
				tc[ 0 ][ 0 ] = percent_around;
				tc[ 0 ][ 1 ] = percent_vertical;
				tc[ 1 ][ 0 ] = percent_around;
				tc[ 1 ][ 1 ] = percent_vertical + step_vertical;
				tc[ 2 ][ 0 ] = percent_around + step_around;
				tc[ 2 ][ 1 ] = percent_vertical + step_vertical;
				tc[ 3 ][ 0 ] = percent_around + step_around;
				tc[ 3 ][ 1 ] = percent_vertical;
				
				// Experimental texture coordinate distortions.
				if( texture_mode & TEXTURE_MODE_Y_ASIN )
				{
					// Scale traversal of Y values to the radius of the current ring.
					tc[ 0 ][ 1 ] = asin( percent_vertical * 2. - 1. ) / M_PI + 0.5;
					tc[ 1 ][ 1 ] = asin( (percent_vertical + step_vertical) * 2. - 1. ) / M_PI + 0.5;
					tc[ 2 ][ 1 ] = tc[ 1 ][ 1 ];
					tc[ 3 ][ 1 ] = tc[ 0 ][ 1 ];
				}
				if( texture_mode & TEXTURE_MODE_X_DIV_R )
				{
					// Remove corners of texture by reducing the X distance traversed at top and bottom.
					double r1scale = r1 / r;
					tc[ 0 ][ 0 ] = percent_around * r1scale + (1. - r1scale) / 2.;
					tc[ 3 ][ 0 ] = tc[ 0 ][ 0 ] + r1scale * step_around;
					double r2scale = r2 / r;
					tc[ 1 ][ 0 ] = percent_around * r2scale + (1. - r2scale) / 2.;
					tc[ 2 ][ 0 ] = tc[ 1 ][ 0 ] + r2scale * step_around;
				}
				
				// Calculate vertices.
				double px[ 4 ] = { x + r1*x1, x + r2*x1, x + r2*x2, x + r1*x2 };
				double py[ 4 ] = { y + r1*y1, y + r2*y1, y + r2*y2, y + r1*y2 };
				double pz[ 2 ] = { z + z1, z + z2 };
				
				#define SMOOTH 1
				#if SMOOTH
					// Calculate per-vertex normals.
					Vec3D normals[ 4 ] = { Vec3D(r1*x1,r1*y1,z1), Vec3D(r2*x1,r2*y1,z2), Vec3D(r2*x2,r2*y2,z2), Vec3D(r1*x2,r1*y2,z1) };
					for( int k = 0; k < 4; k ++ )
						normals[ k ].ScaleTo( 1. );
				#else
					// Set face normal vector.
					Vec3D normal = Vec3D( px[1] - px[0], py[1] - py[0], pz[1] - pz[0] ).Cross( Vec3D( px[3] - px[0], py[3] - py[0], 0. ) );
					if( i == 0 )
						normal = Vec3D( px[3] - px[2], py[3] - py[2], pz[1] - pz[0] ).Cross( Vec3D( px[1] - px[2], py[1] - py[2], 0. ) );
					normal.ScaleTo( 1. );
					glNormal3d( normal.X, normal.Y, normal.Z );
				#endif
				
				// Top-left
				#if SMOOTH
					glNormal3d( normals[ 0 ].X, normals[ 0 ].Y, normals[ 0 ].Z );
				#endif
				glTexCoord2d( tc[ 0 ][ 0 ], tc[ 0 ][ 1 ] );
				glVertex3d( px[ 0 ], py[ 0 ], pz[ 0 ] );
				
				// Bottom-left
				#if SMOOTH
					glNormal3d( normals[ 1 ].X, normals[ 1 ].Y, normals[ 1 ].Z );
				#endif
				glTexCoord2d( tc[ 1 ][ 0 ], tc[ 1 ][ 1 ] );
				glVertex3d( px[ 1 ], py[ 1 ], pz[ 1 ] );
				
				// Bottom-right
				#if SMOOTH
					glNormal3d( normals[ 2 ].X, normals[ 2 ].Y, normals[ 2 ].Z );
				#endif
				glTexCoord2d( tc[ 2 ][ 0 ], tc[ 2 ][ 1 ] );
				glVertex3d( px[ 2 ], py[ 2 ], pz[ 1 ] );
				
				// Top-right
				#if SMOOTH
					glNormal3d( normals[ 3 ].X, normals[ 3 ].Y, normals[ 3 ].Z );
				#endif
				glTexCoord2d( tc[ 3 ][ 0 ], tc[ 3 ][ 1 ] );
				glVertex3d( px[ 3 ], py[ 3 ], pz[ 0 ] );
				
				#undef SMOOTH
			}
		}
		
	glEnd();
	
	glDisable( GL_TEXTURE_2D );
}


// ---------------------------------------------------------------------------


void Graphics::DrawLine3D( double x1, double y1, double z1, double x2, double y2, double z2, float line_width, float r, float g, float b, float a )
{
	glColor4f( r, g, b, a );
	glLineWidth( line_width );
	
	glBegin( GL_LINES );
		glVertex3d( x1, y1, z1 );
		glVertex3d( x2, y2, z2 );
	glEnd();
}


// ---------------------------------------------------------------------------


void Graphics::DrawBox3D( const Pos3D *corner, const Vec3D *fwd, const Vec3D *up, const Vec3D *right, float line_width, float r, float g, float b, float a )
{
	Vec3D rbl = Vec3D( corner->X, corner->Y, corner->Z );
	Vec3D fbl = rbl + *fwd;
	Vec3D rbr = rbl + *right;
	Vec3D fbr = fbl + *right;
	Vec3D rtl = rbl + *up;
	Vec3D ftl = fbl + *up;
	Vec3D rtr = rbr + *up;
	Vec3D ftr = fbr + *up;
	
	glColor4f( r, g, b, a );
	glLineWidth( line_width );
	
	glBegin( GL_LINE_LOOP );
		glVertex3d( rbl.X, rbl.Y, rbl.Z );
		glVertex3d( fbl.X, fbl.Y, fbl.Z );
		glVertex3d( fbr.X, fbr.Y, fbr.Z );
		glVertex3d( rbr.X, rbr.Y, rbr.Z );
	glEnd();
	
	glBegin( GL_LINE_LOOP );
		glVertex3d( rtl.X, rtl.Y, rtl.Z );
		glVertex3d( ftl.X, ftl.Y, ftl.Z );
		glVertex3d( ftr.X, ftr.Y, ftr.Z );
		glVertex3d( rtr.X, rtr.Y, rtr.Z );
	glEnd();
	
	glBegin( GL_LINES );
		glVertex3d( rtl.X, rtl.Y, rtl.Z );
		glVertex3d( rbl.X, rbl.Y, rbl.Z );
		glVertex3d( ftl.X, ftl.Y, ftl.Z );
		glVertex3d( fbl.X, fbl.Y, fbl.Z );
		glVertex3d( ftr.X, ftr.Y, ftr.Z );
		glVertex3d( fbr.X, fbr.Y, fbr.Z );
		glVertex3d( rtr.X, rtr.Y, rtr.Z );
		glVertex3d( rbr.X, rbr.Y, rbr.Z );
	glEnd();
}


// ---------------------------------------------------------------------------


GLuint Graphics::MakeTexture( SDL_Surface *surface, GLint texture_filter, GLint texture_wrap, GLfloat *texcoord )
{
	int w = surface->w;
	int h = surface->h;
	
	// If the caller (ex: Font) wants to know the texture coordinates of the in-use area, pass that back.
	if( texcoord )
	{
		w = Num::NextPowerOfTwo(w);
		h = Num::NextPowerOfTwo(h);
		texcoord[0] = 0.f;                                          // Min X
		texcoord[1] = 0.f;                                          // Min Y
		texcoord[2] = ((GLfloat)( surface->w )) / ((GLfloat)( w )); // Max X
		texcoord[3] = ((GLfloat)( surface->h )) / ((GLfloat)( h ));	// Max Y
	}
	else if( ! Raptor::Game->Cfg.SettingAsBool( "g_texture_anyres", true ) )
	{
		w = Num::NextPowerOfTwo(w);
		h = Num::NextPowerOfTwo(h);
	}
	
	std::vector<SDL_Surface*> allocated;
	
	// Convert to RGBA pixel format (required for SDL_image 1.2.8+).
	if( (surface->format->BitsPerPixel != 32) || (surface->format->Rshift > surface->format->Bshift) )
	{
		#ifdef ENDIAN_BIG
			SDL_PixelFormat format = { NULL, 32, 4, 0, 0, 0, 0, 0, 8, 16, 24, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF, 0, 255 };
		#else
			SDL_PixelFormat format = { NULL, 32, 4, 0, 0, 0, 0, 0, 8, 16, 24, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000, 0, 255 };
		#endif
		SDL_Surface *temp = SDL_ConvertSurface( surface, &format, SDL_SWSURFACE );
		if( temp )
		{
			allocated.push_back( temp );
			surface = temp;
		}
	}
	
	// See if we have a texture dimension limit.
	GLint tex_max = Raptor::Game->Cfg.SettingAsInt( "g_texture_maxres", 0 );
	if( ! tex_max )
		glGetIntegerv( GL_MAX_TEXTURE_SIZE, &tex_max );
	if( tex_max > 0 )
	{
		if( w > tex_max )
			w = tex_max;
		if( h > tex_max )
			h = tex_max;
	}
	
	// Create a new image of the final size to be used for OpenGL (power of 2 no greater than maxres).
	if( (surface->w != w) || (surface->h != h) )
	{
		SDL_Surface *temp = SDL_CreateRGBSurface( surface->flags, w, h, surface->format->BitsPerPixel, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask );
		if( temp )
		{
			// Resample with nearest-neighbor algorithm.
			memset( temp->pixels, 0, w*h*4 );
			int w_reduce = Num::NextPowerOfTwo(surface->w) / w;
			int h_reduce = Num::NextPowerOfTwo(surface->h) / h;
			int x_max = (surface->w + w_reduce - 1) / w_reduce;
			int y_max = (surface->h + h_reduce - 1) / h_reduce;
			for( int x = 0; x < x_max; x ++ )
				for( int y = 0; y < y_max; y ++ )
					((uint32_t*)( temp->pixels ))[ y*w + x ] = ((const uint32_t*)( surface->pixels ))[ y*h_reduce * surface->w + x*w_reduce ];
			
			allocated.push_back( temp );
			surface = temp;
		}
	}
	
	// Determine best texture modes and if we need to generate mipmaps.
	GLint texture_filter_mag = texture_filter;
	GLint texture_filter_min = texture_filter;
	bool mipmap = Raptor::Game->Cfg.SettingAsBool( "g_mipmap", true );;
	if( (texture_filter == GL_LINEAR_MIPMAP_LINEAR) || (texture_filter == GL_LINEAR_MIPMAP_NEAREST) )
	{
		texture_filter_mag = GL_LINEAR;
		if( ! mipmap )
			texture_filter_min = GL_LINEAR;
	}
	else if( (texture_filter == GL_NEAREST_MIPMAP_LINEAR) || (texture_filter == GL_NEAREST_MIPMAP_NEAREST) )
	{
		texture_filter_mag = GL_NEAREST;
		if( ! mipmap )
			texture_filter_min = GL_LINEAR;
	}
	else
	{
		mipmap = false;
		if( texture_filter == GL_NEAREST )
			texture_filter_min = GL_LINEAR;
	}
	
	// Create an OpenGL texture handle.
	GLuint texture = 0;
	glGenTextures( 1, &texture );
	glBindTexture( GL_TEXTURE_2D, texture );
	glEnable( GL_TEXTURE_2D );
	
	// Set the texture's wrapping option (repeat/clamp).
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture_wrap );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture_wrap );
	
	// Set the texture's stretching properties.
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture_filter_mag );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture_filter_min );
	
	// Enable anisotropic filtering.
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, Raptor::Game->Gfx.AF );
	
	// Create an OpenGL texture from the surface.
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels );
	
	if( mipmap )
		// This is the most compatible mipmap mode and seems to have no downsides.
		gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGBA, surface->w, surface->h, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels );
	
	// Free any temporary surfaces we had to create.
	for( std::vector<SDL_Surface*>::iterator surf_iter = allocated.begin(); surf_iter != allocated.end(); surf_iter ++ )
		SDL_FreeSurface( *surf_iter );
	
	return texture;
}
