/*
 *  Screensaver.cpp
 */

#include "Screensaver.h"

#include "RaptorGame.h"


Screensaver::Screensaver( void )
{
	Rect.x = 0;
	Rect.y = 0;
	Rect.w = Raptor::Game->Gfx.W;
	Rect.h = Raptor::Game->Gfx.H;
	
	MouseMoves = 0;
}


Screensaver::~Screensaver()
{
}


bool Screensaver::HandleEvent( SDL_Event *event )
{
#ifndef DEBUG
	if( Raptor::Game->Console.IsActive() )
		return false;
	
	if( event->type == SDL_MOUSEMOTION )
		MouseMoved.Reset();
	else if( MouseMoved.ElapsedSeconds() > 10. )
		MouseMoves = 0;
	
	if( ((event->type == SDL_MOUSEMOTION) && (++MouseMoves >= 2)) || (event->type == SDL_MOUSEBUTTONDOWN) || ((event->type == SDL_KEYDOWN) && (IgnoreKeys.find( event->key.keysym.sym ) == IgnoreKeys.end())) )
	{
		Raptor::Game->Quit();
		return true;
	}
#endif
	
	return false;
}
