/*
 *  Window.cpp
 */

#include "Window.h"

#include "RaptorGame.h"


Window::Window( void )
{
	Rect.x = 0;
	Rect.y = 0;
	Rect.w = Raptor::Game->Gfx.W;
	Rect.h = Raptor::Game->Gfx.H;
	
	Red = 0.0f;
	Green = 0.0f;
	Blue = 0.0f;
	Alpha = 0.5f;
}


Window::Window( SDL_Rect *param_rect )
{
	Rect.x = param_rect->x;
	Rect.y = param_rect->y;
	Rect.w = param_rect->w;
	Rect.h = param_rect->h;
	
	Red = 0.0f;
	Green = 0.0f;
	Blue = 0.0f;
	Alpha = 0.5f;
}


Window::Window( SDL_Rect *param_rect, float red, float green, float blue, float alpha )
{
	Rect.x = param_rect->x;
	Rect.y = param_rect->y;
	Rect.w = param_rect->w;
	Rect.h = param_rect->h;
	
	Red = red;
	Green = green;
	Blue = blue;
	Alpha = alpha;
}


Window::Window( SDL_Rect *param_rect, SDL_Color *param_color )
{
	Rect.x = param_rect->x;
	Rect.y = param_rect->y;
	Rect.w = param_rect->w;
	Rect.h = param_rect->h;
	
	Red = ((float)( param_color->r )) / 255.0f;
	Green = ((float)( param_color->r )) / 255.0f;
	Blue = ((float)( param_color->r )) / 255.0f;
	Alpha = ((float)( param_color->unused )) / 255.0f;
}


Window::~Window()
{
}


void Window::Draw( void )
{
	Raptor::Game->Gfx.DrawRect2D( 0, 0, Rect.w, Rect.h, 0, Red, Green, Blue, Alpha );
}
