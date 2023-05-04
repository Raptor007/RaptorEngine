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
}


Screensaver::~Screensaver()
{
}


bool Screensaver::HandleEvent( SDL_Event *event )
{
	if( Raptor::Game->Console.IsActive() )
		return false;
	
#ifndef DEBUG
	if( (event->type == SDL_MOUSEMOTION) || (event->type == SDL_MOUSEBUTTONDOWN) || ((event->type == SDL_KEYDOWN) && (IgnoreKeys.find( event->key.keysym.sym ) == IgnoreKeys.end())) )
	{
		Raptor::Game->Quit();
		return true;
	}
#endif
	
	return false;
}
