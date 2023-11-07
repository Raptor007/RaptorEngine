/*
 *  KeyboardState.h
 */

#pragma once
class KeyboardState;

#include "PlatformSpecific.h"

#include <string>
#include <map>

#ifdef SDL2
	#include <SDL2/SDL.h>
#else
	#include <SDL/SDL.h>
#endif


class KeyboardState
{
public:
	std::map<SDLKey, bool> KeysDown;

	KeyboardState( void );
	virtual ~KeyboardState();
	
	void TrackEvent( SDL_Event *event );
	bool KeyDown( SDLKey key ) const;

	std::string Status( void ) const;
};
