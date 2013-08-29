/*
 *  TextFileViewer.cpp
 */

#include "TextFileViewer.h"

#include <fstream>
#include "Num.h"
#include "RaptorGame.h"


TextFileViewer::TextFileViewer( SDL_Rect *window_rect, const char *filename, Font *font, const char *title )
{
	if( window_rect )
	{
		Rect.x = window_rect->x;
		Rect.y = window_rect->y;
		Rect.w = window_rect->w;
		Rect.h = window_rect->h;
	}
	else
	{
		Rect.w = 640;
		Rect.h = Raptor::Game->Gfx.H - 20;
		Rect.x = (Raptor::Game->Gfx.W - Rect.w) / 2;
		Rect.y = (Raptor::Game->Gfx.H - Rect.h) / 2;
	}
	
	Red = 0.0f;
	Green = 0.0f;
	Blue = 1.0f;
	Alpha = 0.5f;
	
	if( title )
		Title = std::string(title);
	else
		Title = std::string(filename);
	
	TitleFont = Raptor::Game->Res.GetFont( "Verdana.ttf", 30 );
	
	SDL_Rect rect;
	
	rect.w = 150;
	rect.h = 50;
	rect.x = Rect.w - rect.w - 10;
	rect.y = Rect.h - rect.h - 10;
	AddElement( new TextFileViewerCloseButton( &rect, Raptor::Game->Res.GetFont( "Verdana.ttf", 30 ) ) );
	
	rect.w = Rect.w - 20;
	rect.h = rect.y - 70;
	rect.x = 10;
	rect.y = 60;
	Contents = new ListBox( &rect, font ? font : Raptor::Game->Res.GetFont( "ProFont.ttf", 14 ), 16 );
	Contents->SelectedRed = Contents->TextRed;
	Contents->SelectedGreen = Contents->TextGreen;
	Contents->SelectedBlue = Contents->TextBlue;
	Contents->SelectedAlpha = Contents->TextAlpha;
	AddElement( Contents );
	
	std::ifstream input( filename );
	if( input.is_open() )
	{
		int line = 0;
		char buffer[ 1024 ] = "";
		while( ! input.eof() )
		{
			buffer[ 0 ] = '\0';
			input.getline( buffer, 1024 );
			CStr::ReplaceChars( buffer, "\r\n\t", "   " );
			Contents->AddItem( Num::ToString(line), std::string(buffer) );
			line ++;
		}
		
		input.close();
	}
}


TextFileViewer::~TextFileViewer()
{
}


bool TextFileViewer::KeyUp( SDLKey key )
{
	if( key == SDLK_DOWN )
	{
		Contents->ScrollDown();
		return true;
	}
	else if( key == SDLK_UP )
	{
		Contents->ScrollUp();
		return true;
	}
	else if( key == SDLK_PAGEDOWN )
	{
		int lines = Contents->Rect.h / Contents->TextFont->GetLineSkip();
		for( int i = 0; i < lines; i ++ )
			Contents->ScrollDown();
		return true;
	}
	else if( key == SDLK_PAGEUP )
	{
		int lines = Contents->Rect.h / Contents->TextFont->GetLineSkip();
		for( int i = 0; i < lines; i ++ )
			Contents->ScrollUp();
		return true;
	}
	else if( key == SDLK_ESCAPE )
	{
		Remove();
		return true;
	}
	
	return false;
}


void TextFileViewer::Draw( void )
{
	Window::Draw();
	TitleFont->DrawText( Title, Rect.w/2 + 2, 12, Font::ALIGN_TOP_CENTER, 0,0,0,0.8f );
	TitleFont->DrawText( Title, Rect.w/2, 10, Font::ALIGN_TOP_CENTER );
}


// ---------------------------------------------------------------------------


TextFileViewerCloseButton::TextFileViewerCloseButton( SDL_Rect *rect, Font *button_font ) : LabelledButton( rect, button_font, "Close", Font::ALIGN_MIDDLE_CENTER, Raptor::Game->Res.GetAnimation("button.ani"), Raptor::Game->Res.GetAnimation("button_mdown.ani") )
{
	Red = 1.f;
	Green = 1.f;
	Blue = 1.f;
	Alpha = 0.75f;
}


TextFileViewerCloseButton::~TextFileViewerCloseButton()
{
}


void TextFileViewerCloseButton::Clicked( Uint8 button )
{
	if( button != SDL_BUTTON_LEFT )
		return;
	
	Container->Remove();
}
