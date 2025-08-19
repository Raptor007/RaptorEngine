/*
 *  Layer.h
 */

#pragma once
class Layer;

#include "PlatformSpecific.h"
#include <cstddef>
#include <list>
#include <string>

#ifdef SDL2
	#include <SDL2/SDL.h>
	#define SDLKey int
#else
	#include <SDL/SDL.h>
#endif


class Layer
{
public:
	SDL_Rect Rect, CalcRect;
	Layer *Container;
	std::list<Layer*> Elements;
	bool Removed;
	bool Dirty;
	bool Enabled;
	bool Visible;
	bool Draggable;
	uint8_t UIScaleMode;
	std::string Name;
	
	bool MouseIsDown;
	bool MouseIsWithin;
	Layer *Selected;
	
	float Red, Green, Blue, Alpha;
	
	
	Layer( SDL_Rect *rect = NULL );
	virtual ~Layer();
	
	virtual void Cleanup( void );
	virtual void UpdateCalcRects( int offset_x = 0, int offset_y = 0 );
	virtual void DrawSetup( void );
	virtual void Draw( void );
	virtual void DrawElements( void );
	
	virtual bool WithinCalcRect( int x, int y );
	virtual bool IsSelected( void );
	
	virtual void TrackEvent( SDL_Event *event );
	virtual bool HandleEvent( SDL_Event *event );
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
	void RemoveAllElements( void );
	
	bool IsTop( bool and_containers = true );
	Layer *TopElement( void );
	
	void MoveElementToTop( Layer *element );
	void MoveToTop( bool and_containers = true );
	size_t RemoveOthersAbove( bool and_containers = false );
	
	Layer *FindElement( const std::string &name, bool recursive = true );
	Layer *FindParent( const std::string &name ) const;
	
	void SizeToContainer( int buffer = 0 );
	void SizeToElements( int max_buffer = 10 );
	void Center( void );
	void SetElementScaling( uint8_t scale_mode );
};
