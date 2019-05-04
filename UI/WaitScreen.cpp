/*
 *  WaitScreen.cpp
 */

#include "WaitScreen.h"

#include "RaptorGame.h"


WaitScreen::WaitScreen( std::string text, Font *font, SDL_Rect *rect, Color *color )
{
	Text = text;
	
	if( font )
		TextFont = font;
	else
		TextFont = Raptor::Game->Res.GetFont( "Verdana.ttf", 16 );
	
	if( rect )
	{
		Rect.w = rect->w;
		Rect.h = rect->h;
		Rect.x = rect->x;
		Rect.y = rect->y;
		AutoPosition = false;
	}
	else
	{
		TextFont->TextSize( Text, &Rect );
		Rect.w += 60;
		Rect.h += 40;
		AutoPosition = true;
	}
	
	if( color )
	{
		Red = color->Red;
		Green = color->Green;
		Blue = color->Blue;
		Alpha = color->Alpha;
	}
}


WaitScreen::~WaitScreen()
{
}


void WaitScreen::UpdateRects( void )
{
	if( AutoPosition )
	{
		Rect.x = Raptor::Game->Gfx.W / 2 - Rect.w / 2;
		Rect.y = Raptor::Game->Gfx.H / 2 - Rect.h / 2;
	}
}


void WaitScreen::Draw( void )
{
	UpdateRects();
	Window::Draw();
	TextFont->DrawText( Text, Rect.w / 2, Rect.h / 2, Font::ALIGN_MIDDLE_CENTER );
}
