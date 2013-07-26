/*
 *  Button.h
 */

#pragma once
class Button;

#include "PlatformSpecific.h"

#include "Layer.h"
#include "Animation.h"


class Button : public Layer
{
public:
	Animation Image;
	Animation *ImageNormal;
	Animation *ImageMouseDown;
	Animation *ImageMouseOver;
	
	Button( Layer *container, SDL_Rect *rect, Animation *normal );
	Button( Layer *container, SDL_Rect *rect, Animation *normal, Animation *mouse_down );
	Button( Layer *container, SDL_Rect *rect, Animation *normal, Animation *mouse_down, Animation *mouse_over );
	virtual ~Button();
	
	virtual void Draw( void );
	virtual void MouseEnter( void );
	virtual void MouseLeave( void );
	virtual bool MouseDown( Uint8 button = SDL_BUTTON_LEFT );
	virtual bool MouseUp( Uint8 button = SDL_BUTTON_LEFT );
	
	virtual void Clicked( Uint8 button = SDL_BUTTON_LEFT ) = 0;
};
