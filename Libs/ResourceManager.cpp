/*
 *  ResourceManager.cpp
 */

#include "ResourceManager.h"

#include "Str.h"
#include "RaptorGame.h"
#include "Endian.h"


ResourceManager::ResourceManager( void )
{
	TextureDir = "Textures";
	ModelDir = "Models";
	SoundDir = "Sounds";
	MusicDir = "Music";
	FontDir = "Fonts";
}


ResourceManager::~ResourceManager()
{
	DeleteAll();
}


GLuint ResourceManager::GetTexture( std::string name )
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


Framebuffer *ResourceManager::GetFramebuffer( std::string name, int x, int y )
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


Animation *ResourceManager::GetAnimation( std::string name )
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


Model *ResourceManager::GetModel( std::string name )
{
	Model *model = NULL;
	Lock.Lock();
	
	if( ! name.empty() )
	{
		std::map<std::string, Model*>::iterator it = Models.find( name );
		if( it != Models.end() )
			model = it->second;
		else
			// If the texture wasn't already loaded, load it now.
			model = LoadModel( name );
	}
	
	Lock.Unlock();
	return model;
}


Mix_Chunk *ResourceManager::GetSound( std::string name )
{
	if( name.empty() )
		return NULL;
	
	std::map<std::string, Mix_Chunk*>::iterator it = Sounds.find( name );
	if( it != Sounds.end() )
		return it->second;
	
	// If the animation wasn't already loaded, load it now.
	return LoadSound( name );
}


Mix_Music *ResourceManager::GetMusic( std::string name )
{
	if( name.empty() )
		return NULL;
	
	std::map<std::string, Mix_Music*>::iterator it = Music.find( name );
	if( it != Music.end() )
		return it->second;
	
	// If the animation wasn't already loaded, load it now.
	return LoadMusic( name );
}


Font *ResourceManager::GetFont( std::string name, int point_size )
{
	if( name.empty() )
		return NULL;
	
	std::map<FontID, Font*>::iterator it = Fonts.find( FontID( name, point_size ) );
	if( it != Fonts.end() )
		return it->second;
	
	// If the font wasn't already loaded, load it now.
	return LoadFont( name, point_size );
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
	
	for( std::map<std::string, Animation*>::iterator iter = Animations.begin(); iter != Animations.end(); iter ++ )
	{
		if( iter->second )
		{
			std::string filename = TextureDir + "/" + iter->second->Name;
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


GLuint ResourceManager::LoadTexture( std::string name )
{
	if( name.empty() )
		return 0;
	
	std::string filename = TextureDir + "/" + name;
	
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


Framebuffer *ResourceManager::CreateFramebuffer( std::string name, int x, int y )
{
	if( name.empty() )
		return NULL;
	
	// Create a framebuffer.
	Framebuffer *framebuffer = new Framebuffer( x, y );
	
	// Regardless of initialization success, add it to the list to prevent more attempts.
	Framebuffers[ name ] = framebuffer;
	return framebuffer;
}


Animation *ResourceManager::LoadAnimation( std::string name )
{
	if( name.empty() )
		return NULL;
	
	std::string filename = TextureDir + "/" + name;
	
	Animation *anim = new Animation( name, filename );
	
	if( ! anim )
	{
		anim = new Animation();
		fprintf( stderr, "Couldn't load %s: %s\n", filename.c_str(), "File not found" );
	}
		
	// Even if it's empty (file not found), we'll add it to prevent multiple lookups.
	Animations[ name ] = anim;
	return anim;
}


Model *ResourceManager::LoadModel( std::string name )
{
	if( name.empty() )
		return NULL;
	
	std::string filename = ModelDir + "/" + name;
	
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


Mix_Chunk *ResourceManager::LoadSound( std::string name )
{
	if( name.empty() )
		return NULL;
	
	std::string filename = SoundDir + "/" + name;
	
	// This loads any supported sound format (not just WAV).
	Mix_Chunk *sound = Mix_LoadWAV( filename.c_str() );
	if( ! sound )
		fprintf( stderr, "Couldn't load %s: %s\n", filename.c_str(), Mix_GetError() );
	
	// Even if it's empty (file not found), we'll add it to prevent multiple lookups.
	Sounds[ name ] = sound;
	return sound;
}


Mix_Music *ResourceManager::LoadMusic( std::string name )
{
	if( name.empty() )
		return NULL;
	
	std::string filename = MusicDir + "/" + name;
	
	// This loads any supported sound format (not just WAV).
	Mix_Music *music = Mix_LoadMUS( filename.c_str() );
	if( ! music )
		fprintf( stderr, "Couldn't load %s: %s\n", filename.c_str(), Mix_GetError() );
	
	// Even if it's empty (file not found), we'll add it to prevent multiple lookups.
	Music[ name ] = music;
	return music;
}


Font *ResourceManager::LoadFont( std::string name, int point_size )
{
	if( name.empty() )
		return NULL;
	
	std::string filename = FontDir + "/" + name;
	
	Font *font = new Font( filename, point_size );
	
	// Even if it's empty (file not found), we'll add it to prevent multiple lookups.
	Fonts[ FontID( name, point_size ) ] = font;
	return font;
}
