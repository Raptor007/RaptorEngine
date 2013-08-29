/*
 *  FirstLoadScreen.cpp
 */

#include "FirstLoadScreen.h"

#include "RaptorGame.h"


FirstLoadScreen::FirstLoadScreen( const char *bg_filename, Font *font )
{
	Rect.x = 0;
	Rect.y = 0;
	Rect.w = Raptor::Game->Gfx.W;
	Rect.h = Raptor::Game->Gfx.H;
	
	if( font )
		TextFont = font;
	else
		TextFont = Raptor::Game->Res.GetFont( "Verdana.ttf", 30 );
	
	Background.BecomeInstance( Raptor::Game->Res.GetAnimation(bg_filename) );
}


FirstLoadScreen::~FirstLoadScreen()
{
}


void FirstLoadScreen::Draw( void )
{
	Raptor::Game->Gfx.DrawRect2D( Rect.w / 2 - Rect.h, 0, Rect.w / 2 + Rect.h, Rect.h, Background.CurrentFrame(), 1.f, 1.f, 1.f, 1.f );
	
	TextFont->DrawText( "Loading...", Rect.w / 2, Rect.h / 2, Font::ALIGN_MIDDLE_CENTER );
}
