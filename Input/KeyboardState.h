/*
 *  KeyboardState.h
 */

#pragma once
class KeyboardState;

#include "platforms.h"

#include <string>
#include <map>
#include <SDL/SDL.h>


class KeyboardState
{
public:
	std::map<SDLKey, bool> KeysDown;

	KeyboardState( void );
	~KeyboardState();
	
	void TrackEvent( SDL_Event *event );
	bool KeyDown( SDLKey key );

	std::string Status( void );
};
