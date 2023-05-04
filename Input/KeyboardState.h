/*
 *  KeyboardState.h
 */

#pragma once
class KeyboardState;

#include "PlatformSpecific.h"

#include <string>
#include <map>
#include <SDL/SDL.h>


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
