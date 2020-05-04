/*
 *  ResourceManager.h
 */

#pragma once
class ResourceManager;
class DecryptedResource;

#include "PlatformSpecific.h"

#include <map>
#include <deque>
#include <string>
#include "RaptorGL.h"
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>

#ifdef __APPLE__
	#include <SDL_image/SDL_image.h>
	#include <SDL_mixer/SDL_mixer.h>
#else
	#include <SDL/SDL_image.h>
	#include <SDL/SDL_mixer.h>
#endif

#include "Framebuffer.h"
#include "Animation.h"
#include "Model.h"
#include "Font.h"
#include "Shader.h"
#include "Clock.h"
#include "Mutex.h"


class ResourceManager
{
public:
	Clock ResetTime;
	std::deque<std::string> SearchPath;
	Mutex Lock;
	
	ResourceManager( void );
	virtual ~ResourceManager();
	
	std::string Find( const std::string &name ) const;
	
	GLuint GetTexture( const std::string &name );
	Framebuffer *GetFramebuffer( const std::string &name, int x = 0, int y = 0 );
	Animation *GetAnimation( const std::string &name );
	Model *GetModel( const std::string &name, double scale = 1. );
	Mix_Chunk *GetSound( const std::string &name );
	Mix_Music *GetMusic( const std::string &name );
	Font *GetFont( const std::string &name, int point_size );
	Shader *GetShader( const std::string &name );
	
	void DeleteGraphics( void );
	void ReloadGraphics( void );
	
	void DeleteAll( void );
	void DeleteFramebuffers( void );
	void DeleteTextures( void );
	void DeleteAnimations( void );
	void DeleteSounds( void );
	void DeleteMusic( void );
	void DeleteShaders( void );

private:
	std::map<std::string, GLuint> Textures;
	std::map<std::string, Framebuffer*> Framebuffers;
	std::map<std::string, Animation*> Animations;
	std::map<std::string, Model*> Models;
	std::map<std::string, Mix_Chunk*> Sounds;
	std::map<std::string, Mix_Music*> Music;
	std::map<FontID, Font*> Fonts;
	std::map<std::string, Shader*> Shaders;
	
	GLuint LoadTexture( const std::string &name );
	Framebuffer *CreateFramebuffer( const std::string &name, int x, int y );
	Animation *LoadAnimation( const std::string &name );
	Model *LoadModel( const std::string &name, double scale = 1. );
	Mix_Chunk *LoadSound( const std::string &name );
	Mix_Music *LoadMusic( const std::string &name );
	Font *LoadFont( const std::string &name, int point_size );
	Shader *LoadShader( const std::string &name );
};
