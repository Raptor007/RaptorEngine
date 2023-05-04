/*
 *  MouseState.h
 */

#pragma once
class MouseState;

#include "PlatformSpecific.h"

#include <stdint.h>
#include <string>
#include <map>
#include <SDL/SDL.h>

#include "Animation.h"


class MouseState
{
public:
	int X, Y;
	std::map<Uint8, bool> ButtonsDown;
	
	bool ShowCursor;
	Animation Cursor;
	int Size;
	int PointX, PointY;
	int OffsetX, OffsetY;
	
	MouseState( void );
	MouseState( Animation *cursor, int size );
	virtual ~MouseState();
	
	void TrackEvent( SDL_Event *event );
	bool ButtonDown( Uint8 button ) const;
	void SetOffset( int x, int y );
	
	void SetCursor( Animation *cursor, int size );
	void SetCursor( Animation *cursor, int size, int point_x, int point_y );
	void Draw( void );
	
	std::string Status( void ) const;
};
