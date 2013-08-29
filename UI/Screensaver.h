/*
 *  Screensaver.h
 */

#pragma once
class Screensaver;

#include "PlatformSpecific.h"

#include "Layer.h"
#include <set>
#include <SDL/SDL.h>


class Screensaver : public Layer
{
public:
	std::set<SDLKey> IgnoreKeys;
	
	Screensaver( void );
	virtual ~Screensaver();
	
	bool HandleEvent( SDL_Event *event );
};
