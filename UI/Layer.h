/*
 *  Layer.h
 */

#pragma once
class Layer;

#include "platforms.h"

#include <vector>
#include <SDL/SDL.h>


class Layer
{
public:
	SDL_Rect Rect, CalcRect;
	Layer *Container;
	std::vector<Layer*> Elements;
	bool Removed;
	bool Dirty;
	bool Enabled;
	bool Visible;
	
	bool MouseIsDown;
	bool MouseIsWithin;
	Layer *Selected;
	
	float Red, Green, Blue, Alpha;
	
	
	Layer( Layer *container = NULL, SDL_Rect *rect = NULL );
	virtual ~Layer();
	
	virtual void Cleanup( void );
	virtual void UpdateCalcRects( int offset_x = 0, int offset_y = 0 );
	virtual void DrawSetup( void );
	virtual void Draw( void );
	virtual void DrawElements( void );
	
	virtual bool WithinCalcRect( int x, int y );
	virtual bool IsSelected( void );
	
	virtual bool HandleEvent( SDL_Event *event, bool already_handled = false );
	virtual void MouseEnter( void );
	virtual void MouseLeave( void );
	virtual bool MouseDown( Uint8 button = SDL_BUTTON_LEFT );
	virtual bool MouseUp( Uint8 button = SDL_BUTTON_LEFT );
	virtual bool KeyDown( SDLKey key );
	virtual bool KeyUp( SDLKey key );
	
	void Remove( void );
	void SetDirty( void );
	
	void AddElement( Layer *element );
	void RemoveElement( Layer *element );
	void RemoveElement( int index );
	
	virtual bool IsTop( void );
	virtual Layer *TopElement( void );
};
