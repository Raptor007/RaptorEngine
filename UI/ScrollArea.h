/*
 *  ScrollArea.h
 */

#pragma once
class ScrollArea;
class ScrollAreaButton;

#include "PlatformSpecific.h"

#include "Layer.h"
#include <string>
#include <vector>

#ifdef SDL2
	#include <SDL2/SDL.h>
#else
	#include <SDL/SDL.h>
#endif

#include "Font.h"
#include "Animation.h"
#include "Button.h"


class ScrollArea : public Layer
{
public:
	int ScrollBarSize;
	float ScrollBarRed, ScrollBarGreen, ScrollBarBlue, ScrollBarAlpha;
	int ScrollX, ScrollY;
	int ScrollBy;
	bool ClickedScrollBar;
	
	ScrollArea( SDL_Rect *rect, int scroll_bar_size );
	virtual ~ScrollArea();
	
	int MaxY( void ) const;
	int MaxScroll( void ) const;
	void ScrollUp( int dy = 0 );
	void ScrollDown( int dy = 0 );
	void ScrollTo( int y );
	
	void UpdateRects( void );
	void UpdateCalcRects( int offset_x = 0, int offset_y = 0 );
	void Draw( void );
	void DrawElements( void );
	
	void TrackEvent( SDL_Event *event );
	bool HandleEvent( SDL_Event *event );
	void MouseEnter( void );
	void MouseLeave( void );
	bool MouseDown( Uint8 button = SDL_BUTTON_LEFT );
	bool MouseUp( Uint8 button = SDL_BUTTON_LEFT );
	
	enum
	{
		SCROLL_UP = 1,
		SCROLL_DOWN
	};
};


class ScrollAreaButton : public Button
{
public:
	uint8_t Action;
	
	ScrollAreaButton( uint8_t action, Animation *normal, Animation *mouse_down );
	void UpdateRect( void );
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};
