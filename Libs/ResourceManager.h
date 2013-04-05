/*
 *  ResourceManager.h
 */

#pragma once
class ResourceManager;

#include "platforms.h"

#include <map>
#include <string>
#include <SDL/SDL.h>
#include <GLEW/glew.h>
#include <OpenGL/gl.h>
#include <SDL/SDL_thread.h>
#include <SDL_image/SDL_image.h>
#include <SDL_mixer/SDL_mixer.h>

#include "Framebuffer.h"
#include "Animation.h"
#include "Model.h"
#include "Font.h"
#include "Clock.h"
#include "Mutex.h"


class ResourceManager
{
public:
	Clock ResetTime;
	std::string TextureDir;
	std::string ModelDir;
	std::string SoundDir;
	std::string MusicDir;
	std::string FontDir;
	Mutex Lock;
	
	ResourceManager( void );
	virtual ~ResourceManager();
	
	GLuint GetTexture( std::string name );
	Framebuffer *GetFramebuffer( std::string name, int x = 0, int y = 0 );
	Animation *GetAnimation( std::string name );
	Model *GetModel( std::string name );
	Mix_Chunk *GetSound( std::string name );
	Mix_Music *GetMusic( std::string name );
	Font *GetFont( std::string name, int point_size );
	
	void DeleteGraphics( void );
	void ReloadGraphics( void );
	
	void DeleteAll( void );
	void DeleteFramebuffers( void );
	void DeleteTextures( void );
	void DeleteAnimations( void );
	void DeleteSounds( void );
	void DeleteMusic( void );

private:
	std::map<std::string, GLuint> Textures;
	std::map<std::string, Framebuffer*> Framebuffers;
	std::map<std::string, Animation*> Animations;
	std::map<std::string, Model*> Models;
	std::map<std::string, Mix_Chunk*> Sounds;
	std::map<std::string, Mix_Music*> Music;
	std::map<FontID, Font*> Fonts;
	
	GLuint LoadTexture( std::string name );
	Framebuffer *CreateFramebuffer( std::string name, int x, int y );
	Animation *LoadAnimation( std::string name );
	Model *LoadModel( std::string name );
	Mix_Chunk *LoadSound( std::string name );
	Mix_Music *LoadMusic( std::string name );
	Font *LoadFont( std::string name, int point_size );
};
