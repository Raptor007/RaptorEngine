/*
 *  KeyboardState.cpp
 */

#include "KeyboardState.h"

#include "RaptorGame.h"


KeyboardState::KeyboardState( void )
{
}


KeyboardState::~KeyboardState()
{
}


void KeyboardState::TrackEvent( SDL_Event *event )
{
	// Update key status.
	
	if( event->type == SDL_KEYDOWN )
		KeysDown[ event->key.keysym.sym ] = true;
	else if( event->type == SDL_KEYUP )
		KeysDown[ event->key.keysym.sym ] = false;
}


bool KeyboardState::KeyDown( SDLKey key ) const
{
	// Check if a key is down.
	// Assume it is not down if its state has never been recorded.
	
	std::map<SDLKey, bool>::const_iterator key_iter = KeysDown.find( key );
	if( key_iter != KeysDown.end() )
		return key_iter->second;
	else
		return false;
}


std::string KeyboardState::Status( void ) const
{
	// Create a status string for the keyboard.
	
	std::string return_string;
	char cstr[ 1024 ] = "";
	
	return_string += "Keys down:";
	for( std::map<SDLKey, bool>::const_iterator key_iter = KeysDown.begin(); key_iter != KeysDown.end(); key_iter ++ )
	{
		if( key_iter->second )
		{
			snprintf( cstr, 1024, " %s", Raptor::Game->Cfg.KeyName(key_iter->first).c_str() );
			return_string += cstr;
		}
	}
	
	return return_string;
}
