/*
 *  ClientConsole.cpp
 */

#include "ClientConsole.h"

#include <cstddef>
#include <SDL/SDL.h>
#include "RaptorGame.h"
#include "Str.h"


ClientConsole::ClientConsole( void ) : Layer( &Rect )
{
	Initialized = false;
	
	ToggleKey = SDLK_BACKQUOTE;
	Active = false;
	
	Red = 0.f;
	Green = 0.f;
	Blue = 0.5f;
	Alpha = 0.5f;
	
	Scroll = -1;
	
	HistoryIndex = 0;
}


ClientConsole::~ClientConsole()
{
}


void ClientConsole::Initialize( void )
{
	MessageFont = Raptor::Game->Res.GetFont( "ProFont.ttf", 12 );
	
	Input = new TextBox( &Rect, Raptor::Game->Res.GetFont( "ProFont.ttf", 12 ), Font::ALIGN_TOP_LEFT );
	Input->ReturnDeselects = false;
	Input->Visible = true;
	Input->SelectedTextRed = 0.f;
	Input->SelectedTextGreen = 1.f;
	Input->SelectedTextBlue = 1.f;
	Input->SelectedTextAlpha = 1.f;
	Input->SelectedAlpha = 0.f;
	AddElement( Input );
	
	UpdateRects();
	
	Initialized = true;
}


void ClientConsole::UpdateRects( void )
{
	// Set up the window size.
	Rect.x = 0;
	Rect.y = 0;
	Rect.w = Raptor::Game->Gfx.W;
	Rect.h = Raptor::Game->Gfx.H / 2;
	
	if( Raptor::Game->Head.VR && Raptor::Game->Gfx.DrawTo )
	{
		Rect.w = 512;
		Rect.h = 512;
		Rect.x = Raptor::Game->Gfx.W/2 - Rect.w/2;
		Rect.y = Raptor::Game->Gfx.H/2 - Rect.h;
	}
	
	// Set up the input text box size.
	Input->Rect.w = Rect.w - 4;
	Input->Rect.h = Input->TextFont->GetHeight();
	Input->Rect.x = 2;
	Input->Rect.y = Rect.h - Input->Rect.h - 2;
	
	// Let the Layer code clean things up.
	UpdateCalcRects();
}


void ClientConsole::Draw( void )
{
	if( Active )
	{
		glPushMatrix();
		
		UpdateRects();
		DrawSetup();
		
		Raptor::Game->Gfx.DrawRect2D( 0, 0, Rect.w, Rect.h, 0, Red, Green, Blue, Alpha );
		
		int x = Input->Rect.x;
		int y = Input->Rect.y;
		
		if( Lock )
		{
			if( SDL_mutexP( Lock ) )
				fprintf( stderr, "ClientConsole::Draw: SDL_mutexP(Lock): %s\n", SDL_GetError() );
		}
		
		int skip = 0;
		if( Scroll >= 0 )
			skip = Messages.size() - Scroll;
		
		for( std::deque<TextConsoleMessage*>::reverse_iterator message_iter = Messages.rbegin(); message_iter != Messages.rend(); message_iter ++ )
		{
			if( skip )
			{
				skip --;
				continue;
			}
			
			float r = 1.f, g = 1.f, b = 1.f;
			switch( (*message_iter)->Type )
			{
				case( MSG_INPUT ):
				{
					r = 0.f;
					g = 1.f;
					b = 1.f;
					break;
				}
				case( MSG_ERROR ):
				{
					r = 1.f;
					g = 0.f;
					b = 0.f;
					break;
				}
				case( MSG_CHAT ):
				{
					r = 1.f;
					g = 1.f;
					b = 0.f;
					break;
				}
			}
			
			y -= MessageFont->TextHeight( (*message_iter)->Text );
			
			MessageFont->DrawText( (*message_iter)->Text, x, y, Font::ALIGN_TOP_LEFT, r, g, b, 1.f );
			
			if( y < 0 )
				break;
		}
		
		if( Lock )
		{
			if( SDL_mutexV( Lock ) )
				fprintf( stderr, "ClientConsole::Draw: SDL_mutexP(Lock): %s\n", SDL_GetError() );
		}
		
		DrawElements();
		
		glPopMatrix();
	}
}


bool ClientConsole::HandleEvent( SDL_Event *event )
{
	if( ! Initialized )
		return false;
	
	// First check for the console toggle key.
	if( ((event->type == SDL_KEYDOWN) || (event->type == SDL_KEYUP)) && (event->key.keysym.sym == ToggleKey) )
	{
		if( event->type == SDL_KEYDOWN )
			ToggleActive();
		
		return true;
	}
	
	// If the console is active, we might handle the event.
	if( Active )
	{
		if( ((event->type == SDL_KEYDOWN) || (event->type == SDL_KEYUP)) && ((event->key.keysym.sym == SDLK_RETURN) || (event->key.keysym.sym == SDLK_KP_ENTER)) )
		{
			if( event->type == SDL_KEYDOWN )
			{
				std::string cmd = Input->Text;
				Input->Text = "";
				Input->Cursor = 0;
				Raptor::Game->Cfg.Command( cmd, true );
				if( History.empty() || (History.back() != cmd) )
					History.push_back( cmd );
				HistoryIndex = History.size();
			}
			
			return true;
		}
		
		else if( ((event->type == SDL_KEYDOWN) || (event->type == SDL_KEYUP)) && (event->key.keysym.sym == SDLK_UP) )
		{
			if( event->type == SDL_KEYDOWN )
			{
				if( HistoryIndex )
					HistoryIndex --;
				else
					HistoryIndex = History.size();
				
				if( HistoryIndex < History.size() )
				{
					Input->Text = History[ HistoryIndex ];
					Input->Cursor = Input->Text.length();
				}
				else
				{
					Input->Text = "";
					Input->Cursor = 0;
				}
			}
			
			return true;
		}
		
		else if( ((event->type == SDL_KEYDOWN) || (event->type == SDL_KEYUP)) && (event->key.keysym.sym == SDLK_DOWN) )
		{
			if( event->type == SDL_KEYDOWN )
			{
				if( HistoryIndex < History.size() )
					HistoryIndex ++;
				else
					HistoryIndex = 0;
				
				if( HistoryIndex < History.size() )
				{
					Input->Text = History[ HistoryIndex ];
					Input->Cursor = Input->Text.length();
				}
				else
				{
					Input->Text = "";
					Input->Cursor = 0;
				}
			}
			
			return true;
		}
		
		else if( ((event->type == SDL_KEYDOWN) || (event->type == SDL_KEYUP)) && (event->key.keysym.sym == SDLK_TAB) )
		{
			if( event->type == SDL_KEYDOWN )
			{
				size_t match = History.size();
				for( size_t i = 0; i < History.size(); i ++ )
				{
					if( strncasecmp( History[ i ].c_str(), Input->Text.c_str(), Input->Text.length() ) == 0 )
						match = i;
				}
				
				if( match < History.size() )
				{
					HistoryIndex = match;
					Input->Text = History[ HistoryIndex ];
					Input->Cursor = Input->Text.length();
				}
			}
			
			return true;
		}
		
		else if( ((event->type == SDL_MOUSEBUTTONDOWN) || (event->type == SDL_MOUSEBUTTONUP)) && (event->button.button == SDL_BUTTON_WHEELUP) )
		{
			if( event->type == SDL_MOUSEBUTTONDOWN )
				ScrollUp();
			
			return true;
		}
		
		else if( ((event->type == SDL_MOUSEBUTTONDOWN) || (event->type == SDL_MOUSEBUTTONUP)) && (event->button.button == SDL_BUTTON_WHEELDOWN) )
		{
			if( event->type == SDL_MOUSEBUTTONDOWN )
				ScrollDown();
			
			return true;
		}
		
		else if( ((event->type == SDL_KEYDOWN) || (event->type == SDL_KEYUP)) && (event->key.keysym.sym == SDLK_PAGEUP) )
		{
			if( event->type == SDL_KEYDOWN )
				ScrollUp( Rect.h / MessageFont->TextHeight("Q") );
			
			return true;
		}
		
		else if( ((event->type == SDL_KEYDOWN) || (event->type == SDL_KEYUP)) && (event->key.keysym.sym == SDLK_PAGEDOWN) )
		{
			if( event->type == SDL_KEYDOWN )
				ScrollDown( Rect.h / MessageFont->TextHeight("Q") );
			
			return true;
		}
		
		else if( ((event->type == SDL_KEYDOWN) || (event->type == SDL_KEYUP)) && (event->key.keysym.sym == SDLK_HOME) )
		{
			if( event->type == SDL_KEYDOWN )
				Scroll = std::min<int>( Rect.h / MessageFont->TextHeight("Q") - 1, Messages.size() );
			
			return true;
		}
		
		else if( ((event->type == SDL_KEYDOWN) || (event->type == SDL_KEYUP)) && (event->key.keysym.sym == SDLK_END) )
		{
			if( event->type == SDL_KEYDOWN )
				Scroll = -1;
			
			return true;
		}
		
		// If the console is active, let its TextBox handle most events.
		bool handled = Input->HandleEvent( event );
		
		// If the event caused the console input to be deselected, hide the console.
		if( ! Input->IsSelected() )
			Active = false;
		
		return handled;
	}
	
	// Return 0 if the event was not handled.
	return false;
}


bool ClientConsole::IsActive( void )
{
	return Active;
}


void ClientConsole::ToggleActive( void )
{
	Active = ! Active;
	Selected = Active ? Input : NULL;
}


void ClientConsole::ScrollUp( int lines )
{
	if( Scroll < 0 )
		Scroll = Messages.size();
	
	Scroll -= lines;
}


void ClientConsole::ScrollDown( int lines )
{
	if( Scroll >= 0 )
	{
		Scroll += lines;
		
		if( (size_t) Scroll > Messages.size() )
			Scroll = -1;
	}
}
