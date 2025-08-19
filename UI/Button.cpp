/*
 *  Button.cpp
 */

#include "Button.h"

#include <cstddef>
#include <cmath>
#include "RaptorGame.h"


Button::Button( SDL_Rect *rect, Animation *normal ) : Layer( rect )
{
	ImageNormal = normal;
	ImageMouseDown = NULL;
	ImageMouseOver = NULL;
	
	Image.BecomeInstance( ImageNormal );

	Red = 1.f;
	Green = 1.f;
	Blue = 1.f;
	Alpha = 1.f;
}


Button::Button( SDL_Rect *rect, Animation *normal, Animation *mouse_down ) : Layer( rect )
{
	ImageNormal = normal;
	ImageMouseDown = mouse_down;
	ImageMouseOver = NULL;
	
	Image.BecomeInstance( ImageNormal );

	Red = 1.f;
	Green = 1.f;
	Blue = 1.f;
	Alpha = 1.f;
}


Button::Button( SDL_Rect *rect, Animation *normal, Animation *mouse_down, Animation *mouse_over ) : Layer( rect )
{
	ImageNormal = normal;
	ImageMouseDown = mouse_down;
	ImageMouseOver = mouse_over;
	
	Image.BecomeInstance( ImageNormal );

	Red = 1.f;
	Green = 1.f;
	Blue = 1.f;
	Alpha = 1.f;
}


Button::~Button()
{
}


void Button::Draw( void )
{
	Raptor::Game->Gfx.DrawRect2D( 0, 0, CalcRect.w, CalcRect.h, Image.CurrentFrame(), Red, Green, Blue, Alpha );
}


void Button::MouseEnter( void )
{
	if( ImageMouseDown && MouseIsDown )
		Image.BecomeInstance( ImageMouseDown );
	else if( ImageMouseOver )
		Image.BecomeInstance( ImageMouseOver );
}


void Button::MouseLeave( void )
{
	if( ImageMouseOver || (ImageMouseDown && MouseIsDown) )
		Image.BecomeInstance( ImageNormal );
}


bool Button::MouseDown( Uint8 button )
{
	if( ImageMouseDown )
		Image.BecomeInstance( ImageMouseDown );
	
	return true;
}


bool Button::MouseUp( Uint8 button )
{
	if( ImageMouseDown )
		Image.BecomeInstance( ImageNormal );
	
	Clicked( button );
	
	return true;
}
