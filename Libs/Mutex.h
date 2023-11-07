/*
 *  Mutex.h
 */

#pragma once
class Mutex;

#include "PlatformSpecific.h"

#ifdef SDL2
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_thread.h>
	#include <SDL2/SDL_mutex.h>
#else
	#include <SDL/SDL.h>
	#include <SDL/SDL_thread.h>
	#include <SDL/SDL_mutex.h>
#endif


class Mutex
{
public:
	Mutex( void );
	~Mutex();
	
	bool Lock( void );
	bool Unlock( void );
	
private:
	SDL_mutex *RawMutex;
};
