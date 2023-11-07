/*
 *  Screensaver.h
 */

#pragma once
class Screensaver;

#include "PlatformSpecific.h"

#include "Layer.h"
#include "Clock.h"
#include <set>

#ifdef SDL2
	#include <SDL2/SDL.h>
#else
	#include <SDL/SDL.h>
#endif


class Screensaver : public Layer
{
public:
	std::set<SDLKey> IgnoreKeys;
	int MouseMoves;
	Clock MouseMoved;
	
	Screensaver( void );
	virtual ~Screensaver();
	
	bool HandleEvent( SDL_Event *event );
};
