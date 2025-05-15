/*
 *  MessageOverlay.cpp
 */


#include "MessageOverlay.h"

#include "RaptorGame.h"


MessageOverlay::MessageOverlay( Font *message_font ) : Layer()
{
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 0.f;
	
	MessageFont = message_font;
	
	MessageLifetime = 5.;
	MaxMessages = 10;
	ScrollTime = 0.;
	
	UpdateRects();
}


MessageOverlay::~MessageOverlay()
{
}


bool MessageOverlay::DrawsType( uint32_t type ) const
{
	return (DoNotDraw.find( type ) == DoNotDraw.end());
}


void MessageOverlay::SetTypeToDraw( uint32_t type, bool draw )
{
	if( draw )
	{
		std::set<uint32_t>::iterator type_iter = DoNotDraw.find( type );
		if( type_iter != DoNotDraw.end() )
			DoNotDraw.erase( type_iter );
	}
	else
		DoNotDraw.insert( type );
}


void MessageOverlay::UpdateRects( void )
{
	// Set up the window size.
	Rect.x = 0;
	Rect.y = 0;
	Rect.w = Raptor::Game->Gfx.W;
	Rect.h = Raptor::Game->Gfx.H;
	
	if( Raptor::Game->Head.VR && Raptor::Game->Gfx.DrawTo )
	{
		Rect.h = 360;
		if( Container )
		{
			Rect.w = Container->Rect.w;
			Rect.y = Container->Rect.h/2 - Rect.h/2;
		}
		else
			Rect.y = Raptor::Game->Gfx.H/2 - Rect.h/2;
	}
	
	// Let the Layer code clean things up.
	UpdateCalcRects();
}


void MessageOverlay::Draw( void )
{
	glPushMatrix();
	
	UpdateRects();
	DrawSetup();
	
	bool vr = Raptor::Game->Head.VR && Raptor::Game->Gfx.DrawTo;
	int x = vr ? (Rect.w / 2) : 5;
	int y = Rect.h - 5;
	
	if( Raptor::Game->Msg.Lock )
	{
		if( SDL_mutexP( Raptor::Game->Msg.Lock ) )
			fprintf( stderr, "MessageOverlay::Draw: SDL_mutexP(Lock): %s\n", SDL_GetError() );
	}
	
	int count = 0;
	for( std::deque<TextConsoleMessage*>::reverse_iterator message_iter = Raptor::Game->Msg.Messages.rbegin(); message_iter != Raptor::Game->Msg.Messages.rend(); message_iter ++ )
	{
		if( ! DrawsType( (*message_iter)->Type ) )
			continue;
		double elapsed_seconds = (*message_iter)->TimeStamp.ElapsedSeconds();
		double a = 1.;
		if( elapsed_seconds > MessageLifetime )
			break;
		if( ScrollTime && (elapsed_seconds < ScrollTime) && (message_iter == Raptor::Game->Msg.Messages.rbegin()) )
			y += MessageFont->GetHeight() * (ScrollTime - elapsed_seconds) / ScrollTime;
		if( elapsed_seconds > (MessageLifetime - 1.) )
			a = MessageLifetime - elapsed_seconds;
		
		float r = 1.f, g = 1.f, b = 1.f;
		switch( (*message_iter)->Type )
		{
			case( TextConsole::MSG_INPUT ):
			{
				r = 0.f;
				g = 1.f;
				b = 1.f;
				break;
			}
			case( TextConsole::MSG_ERROR ):
			{
				r = 1.f;
				g = 0.f;
				b = 0.f;
				break;
			}
			case( TextConsole::MSG_CHAT ):
			{
				r = 1.f;
				g = 1.f;
				b = 0.f;
				break;
			}
			case( TextConsole::MSG_TEAM ):
			{
				r = 0.25f;
				g = 1.f;
				b = 0.f;
				break;
			}
		}
		
		y -= MessageFont->TextHeight( (*message_iter)->Text );
		count ++;
		
		uint8_t align = vr ? Font::ALIGN_TOP_CENTER : Font::ALIGN_TOP_LEFT;
		MessageFont->DrawText( (*message_iter)->Text, x+1, y+1, align, 0.f, 0.f, 0.f, a * 0.8f );
		MessageFont->DrawText( (*message_iter)->Text, x, y, align, r, g, b, a );
		
		if( y < 0 )
			break;
		if( count >= MaxMessages )
			break;
	}
	
	if( Raptor::Game->Msg.Lock )
	{
		if( SDL_mutexV( Raptor::Game->Msg.Lock ) )
			fprintf( stderr, "MessageOverlay::Draw: SDL_mutexP(Lock): %s\n", SDL_GetError() );
	}
	
	glPopMatrix();
}
