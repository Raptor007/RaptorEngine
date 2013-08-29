/*
 *  Notification.h
 */

#pragma once
class Notification;

#include "PlatformSpecific.h"
#include "WaitScreen.h"
#include <cstddef>


class Notification : public WaitScreen
{
public:
	Clock Lifetime;
	
	Notification( std::string text, Font *font = NULL, SDL_Rect *rect = NULL, Color *color = NULL );
	virtual ~Notification();
	
	void Draw( void );
	bool KeyDown( SDLKey key );
	bool KeyUp( SDLKey key );
	bool MouseDown( Uint8 button = SDL_BUTTON_LEFT );
	bool MouseUp( Uint8 button = SDL_BUTTON_LEFT );
};
