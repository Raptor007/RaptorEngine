/*
 *  ResourceManager.cpp
 */

#include "ResourceManager.h"

#include <cstddef>
#include "Str.h"
#include "File.h"
#include "Endian.h"
#include "RaptorGame.h"


ResourceManager::ResourceManager( void )
{
	SearchPath.push_back( "." );
	SearchPath.push_back( "Data" );
	SearchPath.push_back( "Textures" );
	SearchPath.push_back( "Models" );
	SearchPath.push_back( "Sounds" );
	SearchPath.push_back( "Music" );
	SearchPath.push_back( "Fonts" );
	SearchPath.push_back( "Shaders" );
}


ResourceManager::~ResourceManager()
{
	DeleteAll();
}


GLuint ResourceManager::GetTexture( const std::string &name )
{
	GLuint texture = 0;
	Lock.Lock();
	
	if( ! name.empty() )
	{
		if( name[ 0 ] == '*' )
		{
			std::map<std::string, Framebuffer*>::iterator it = Framebuffers.find( std::string(name.c_str()+1) );
			if( it != Framebuffers.end() )
				texture = it->second->Texture;
		}
		else
		{
			std::map<std::string, GLuint>::iterator it = Textures.find( name );
			if( it != Textures.end() )
				texture = it->second;
			else
				// If the texture wasn't already loaded and isn't for a framebuffer, load it now.
				texture = LoadTexture( name );
		}
	}
	
	Lock.Unlock();
	return texture;
}


Framebuffer *ResourceManager::GetFramebuffer( const std::string &name, int x, int y )
{
	Framebuffer *framebuffer = NULL;
	Lock.Lock();
	
	if( ! name.empty() )
	{
		std::map<std::string, Framebuffer*>::iterator it = Framebuffers.find( name );
		if( it != Framebuffers.end() )
			framebuffer = it->second;
		else
			// If the framebuffer wasn't already created, create it now.
			framebuffer = CreateFramebuffer( name, x, y );
	}
	
	Lock.Unlock();
	return framebuffer;
}


Animation *ResourceManager::GetAnimation( const std::string &name )
{
	Animation *animation = NULL;
	Lock.Lock();
	
	if( ! name.empty() )
	{
		std::map<std::string, Animation*>::iterator it = Animations.find( name );
		if( it != Animations.end() )
			animation = it->second;
		else
			// If the animation wasn't already loaded, load it now.
			animation = LoadAnimation( name );
	}
	
	Lock.Unlock();
	return animation;
}


Model *ResourceManager::GetModel( const std::string &name )
{
	Model *model = NULL;
	
	if( ! name.empty() )
	{
		Lock.Lock();
		
		std::map<std::string, Model*>::iterator it = Models.find( name );
		if( it != Models.end() )
			model = it->second;
		else
			// If the texture wasn't already loaded, load it now.
			model = LoadModel( name );
		
		Lock.Unlock();
	}
	
	return model;
}


Mix_Chunk *ResourceManager::GetSound( const std::string &name )
{
	if( name.empty() )
		return NULL;
	
	std::map<std::string, Mix_Chunk*>::iterator it = Sounds.find( name );
	if( it != Sounds.end() )
		return it->second;
	
	// If the sound wasn't already loaded, load it now.
	return LoadSound( name );
}


Mix_Music *ResourceManager::GetMusic( const std::string &name )
{
	if( name.empty() )
		return NULL;
	
	std::map<std::string, Mix_Music*>::iterator it = Music.find( name );
	if( it != Music.end() )
		return it->second;
	
	// If the music wasn't already loaded, load it now.
	return LoadMusic( name );
}


Font *ResourceManager::GetFont( const std::string &name, int point_size )
{
	if( name.empty() )
		return NULL;
	
	std::map<FontID, Font*>::iterator it = Fonts.find( FontID( name, point_size ) );
	if( it != Fonts.end() )
		return it->second;
	
	// If the font wasn't already loaded, load it now.
	return LoadFont( name, point_size );
}


Shader *ResourceManager::GetShader( const std::string &name )
{
	Shader *shader = NULL;
	
	if( ! name.empty() )
	{
		Lock.Lock();
		
		std::map<std::string, Shader*>::iterator it = Shaders.find( name );
		if( it != Shaders.end() )
			shader = it->second;
		else
			// If the shader wasn't already loaded, load it now.
			shader = LoadShader( name );
		
		Lock.Unlock();
	}
	
	return shader;
}


void ResourceManager::DeleteGraphics( void )
{
	Lock.Lock();
	
	DeleteTextures();
	
	for( std::map<std::string, Framebuffer*>::iterator iter = Framebuffers.begin(); iter != Framebuffers.end(); iter ++ )
	{
		if( iter->second )
			iter->second->Clear();
	}
	
	for( std::map<std::string, Shader*>::iterator iter = Shaders.begin(); iter != Shaders.end(); iter ++ )
	{
		if( iter->second )
			iter->second->Clear();
	}
	
	Lock.Unlock();
}


void ResourceManager::ReloadGraphics( void )
{
	Lock.Lock();
	
	DeleteTextures();
	ResetTime.Reset();
	
	for( std::map<std::string, Framebuffer*>::iterator iter = Framebuffers.begin(); iter != Framebuffers.end(); iter ++ )
	{
		if( iter->second )
			iter->second->Reload();
	}
	
	for( std::map<std::string, Shader*>::iterator iter = Shaders.begin(); iter != Shaders.end(); iter ++ )
	{
		if( iter->second )
		{
			std::string filename = Find( iter->second->Name + std::string(".frag") );
			filename = filename.substr( 0, filename.find_last_of(".") );
			
			std::map<std::string,std::string> defs;
			for( std::map<std::string,std::string>::iterator setting_iter = Raptor::Game->Cfg.Settings.begin(); setting_iter != Raptor::Game->Cfg.Settings.end(); setting_iter ++ )
			{
				if( strncasecmp( setting_iter->first.c_str(), "g_shader_", strlen("g_shader_") ) == 0 )
					defs[ CStr::CapitalizedCopy( setting_iter->first.c_str() + strlen("g_shader_") ) ] = setting_iter->second;
			}
			
			iter->second->Load( filename, defs );
		}
	}
	
	for( std::map<std::string, Animation*>::iterator iter = Animations.begin(); iter != Animations.end(); iter ++ )
	{
		if( iter->second )
		{
			std::string filename = Find( iter->second->Name );
			iter->second->Load( filename );
		}
	}
	
	Lock.Unlock();
}


void ResourceManager::DeleteAll( void )
{
	Lock.Lock();
	
	DeleteTextures();
	DeleteFramebuffers();
	DeleteAnimations();
	DeleteSounds();
	DeleteMusic();
	DeleteShaders();
	
	ResetTime.Reset();
	Lock.Unlock();
}


void ResourceManager::DeleteFramebuffers( void )
{
	Lock.Lock();
	
	for( std::map<std::string, Framebuffer*>::iterator iter = Framebuffers.begin(); iter != Framebuffers.end(); iter ++ )
	{
		if( iter->second )
			delete iter->second;
		iter->second = NULL;
	}
	Framebuffers.clear();
	
	Lock.Unlock();
}


void ResourceManager::DeleteTextures( void )
{
	for( std::map<std::string, GLuint>::iterator iter = Textures.begin(); iter != Textures.end(); iter ++ )
	{
		if( iter->second )
			glDeleteTextures( 1, &(iter->second) );
		iter->second = 0;
	}
	Textures.clear();
}


void ResourceManager::DeleteAnimations( void )
{
	for( std::map<std::string, Animation*>::iterator iter = Animations.begin(); iter != Animations.end(); iter ++ )
	{
		if( iter->second )
			delete iter->second;
		iter->second = NULL;
	}
	Animations.clear();
}


void ResourceManager::DeleteSounds( void )
{
	for( std::map<std::string, Mix_Chunk*>::iterator iter = Sounds.begin(); iter != Sounds.end(); iter ++ )
	{
		if( iter->second )
			Mix_FreeChunk( iter->second );
		iter->second = NULL;
	}
	Sounds.clear();
}


void ResourceManager::DeleteMusic( void )
{
	for( std::map<std::string, Mix_Music*>::iterator iter = Music.begin(); iter != Music.end(); iter ++ )
	{
		if( iter->second )
			Mix_FreeMusic( iter->second );
		iter->second = NULL;
	}
	Music.clear();
}


void ResourceManager::DeleteShaders( void )
{
	for( std::map<std::string, Shader*>::iterator iter = Shaders.begin(); iter != Shaders.end(); iter ++ )
	{
		if( iter->second )
			delete iter->second;
		iter->second = NULL;
	}
	Shaders.clear();
}


GLuint ResourceManager::LoadTexture( const std::string &name )
{
	if( name.empty() )
		return 0;
	
	std::string filename = Find( name );
	
	GLuint texture = 0; // Texture object handle.
	SDL_Surface *surface = NULL; // Gives us the information to make the texture.
	
	if( (surface = IMG_Load( filename.c_str() )) )
	{
		// Check that the image's width is a power of 2.
		if( (surface->w & (surface->w - 1)) != 0 )
			fprintf( stderr, "%s: width is not a power of 2.\n", name.c_str() );
		
		// Also check if the height is a power of 2.
		if( (surface->h & (surface->h - 1)) != 0 )
			fprintf( stderr, "%s: height is not a power of 2.\n", name.c_str() );
		
		// Convert to RGBA pixel format (required for SDL_image 1.2.8+).
		switch( surface->format->BitsPerPixel )
		{
			case 8:
				// It's 8 bit, so always convert to RGBA.
			case 16:
				// It's 16 bit, so always convert to RGBA.
			case 24:
				// It's 24 bit, so always convert to RGBA.
				{
					#ifdef ENDIAN_BIG
						SDL_PixelFormat format = { NULL, 32, 4, 0, 0, 0, 0, 0, 8, 16, 24, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF, 0, 255 };
					#else
						SDL_PixelFormat format = { NULL, 32, 4, 0, 0, 0, 0, 0, 8, 16, 24, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000, 0, 255 };
					#endif
					SDL_Surface *temp = SDL_ConvertSurface( surface, &format, SDL_SWSURFACE );
					SDL_FreeSurface( surface );
					surface = temp;
				}
				break;
			case 32:
				// It's 32 bit, so convert to RGBA only if it's ABGR.
				if( surface->format->Rshift > surface->format->Bshift )
				{
					#ifdef ENDIAN_BIG
						SDL_PixelFormat format = { NULL, 32, 4, 0, 0, 0, 0, 0, 8, 16, 24, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF, 0, 255 };
					#else
						SDL_PixelFormat format = { NULL, 32, 4, 0, 0, 0, 0, 0, 8, 16, 24, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000, 0, 255 };
					#endif
					SDL_Surface *temp = SDL_ConvertSurface( surface, &format, SDL_SWSURFACE );
					SDL_FreeSurface( surface );
					surface = temp;
				}
				break;
			default:
				fprintf( stderr, "%s: unknown pixel format.\n", name.c_str() );
		}
		
		// Have OpenGL generate a texture object handle for us.
		glGenTextures( 1, &texture );
		
		// Bind the texture object.
		glBindTexture( GL_TEXTURE_2D, texture );
		
		// Set the texture's stretching properties.
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		
		// Enable anisotropic filtering.
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, Raptor::Game->Gfx.AF );
		
		// Edit the texture object's image data using the information SDL_Surface gives us.
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels );
		
		// Generate mipmaps.
		#ifndef LEGACY_MIPMAP
			glGenerateMipmap( GL_TEXTURE_2D );
		#else
			gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGBA, surface->w, surface->h, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels );
		#endif
		
		// Free the SDL_Surface only if it was successfully created.
		SDL_FreeSurface( surface );
	}
	else
		fprintf( stderr, "Couldn't load %s: %s\n", filename.c_str(), SDL_GetError() );
	
	// Even if it's empty (file not found), we'll add it to prevent multiple lookups.
	Textures[ name ] = texture;
	return texture;
}


Framebuffer *ResourceManager::CreateFramebuffer( const std::string &name, int x, int y )
{
	if( name.empty() )
		return NULL;
	
	// Create a framebuffer.
	Framebuffer *framebuffer = new Framebuffer( x, y );
	
	// Regardless of initialization success, add it to the list to prevent more attempts.
	Framebuffers[ name ] = framebuffer;
	return framebuffer;
}


Animation *ResourceManager::LoadAnimation( const std::string &name )
{
	if( name.empty() )
		return NULL;
	
	std::string filename = Find( name );
	
	Animation *anim = new Animation( name, filename );
	
	// Even if it's empty (file not found), we'll add it to prevent multiple lookups.
	Animations[ name ] = anim;
	return anim;
}


Model *ResourceManager::LoadModel( const std::string &name )
{
	if( name.empty() )
		return NULL;
	
	std::string filename = Find( name );
	
	Model *model = new Model();
	bool success = model->LoadOBJ( filename );
	if( ! success )
	{
		char cstr[ 1024 ] = "";
		snprintf( cstr, 1024, "Couldn't load %s: %s", filename.c_str(), "File not found" );
		Raptor::Game->Console.Print( cstr, TextConsole::MSG_ERROR );
	}
	
	// Even if it's empty (file not found), we'll add it to prevent multiple lookups.
	Models[ name ] = model;
	return model;
}


Mix_Chunk *ResourceManager::LoadSound( const std::string &name )
{
	if( name.empty() )
		return NULL;
	
	std::string filename = Find( name );
	
	// This loads any supported sound format (not just WAV).
	Mix_Chunk *sound = Mix_LoadWAV( filename.c_str() );
	if( ! sound )
		fprintf( stderr, "Couldn't load %s: %s\n", filename.c_str(), Mix_GetError() );
	
	// Even if it's empty (file not found), we'll add it to prevent multiple lookups.
	Sounds[ name ] = sound;
	return sound;
}


Mix_Music *ResourceManager::LoadMusic( const std::string &name )
{
	if( name.empty() )
		return NULL;
	
	std::string filename = Find( name );
	
	// This loads any supported sound format (not just WAV).
	Mix_Music *music = Mix_LoadMUS( filename.c_str() );
	if( ! music )
		fprintf( stderr, "Couldn't load %s: %s\n", filename.c_str(), Mix_GetError() );
	
	// Even if it's empty (file not found), we'll add it to prevent multiple lookups.
	Music[ name ] = music;
	return music;
}


Font *ResourceManager::LoadFont( const std::string &name, int point_size )
{
	if( name.empty() )
		return NULL;
	
	std::string filename = Find( name );
	
	Font *font = new Font( filename, point_size );
	
	// Even if it's empty (file not found), we'll add it to prevent multiple lookups.
	Fonts[ FontID( name, point_size ) ] = font;
	return font;
}


Shader *ResourceManager::LoadShader( const std::string &name )
{
	if( name.empty() )
		return NULL;
	
	std::string filename = Find( name + std::string(".frag") );
	filename = filename.substr( 0, filename.find_last_of(".") );
	
	std::map<std::string,std::string> defs;
	for( std::map<std::string,std::string>::iterator setting_iter = Raptor::Game->Cfg.Settings.begin(); setting_iter != Raptor::Game->Cfg.Settings.end(); setting_iter ++ )
	{
		if( strncasecmp( setting_iter->first.c_str(), "g_shader_", strlen("g_shader_") ) == 0 )
			defs[ CStr::CapitalizedCopy( setting_iter->first.c_str() + strlen("g_shader_") ) ] = setting_iter->second;
	}
	
	Shader *shader = new Shader( name, filename, defs );
	
	// Even if it's empty (file not found), we'll add it to prevent multiple lookups.
	Shaders[ name ] = shader;
	return shader;
}


std::string ResourceManager::Find( const std::string &name ) const
{
	if( name[ 0 ] == '/' )
		return name.substr( 1 );
	
	for( std::deque<std::string>::const_iterator path_iter = SearchPath.begin(); path_iter != SearchPath.end(); path_iter ++ )
	{
		std::string path = *path_iter + "/" + name;
		if( File::Exists(path.c_str()) )
			return path;
	}
	
	return name;
}
