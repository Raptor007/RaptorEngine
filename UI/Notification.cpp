/*
 *  Notification.cpp
 */

#include "Notification.h"

#include "RaptorGame.h"


Notification::Notification( std::string text, Font *font, SDL_Rect *rect, Color *color )
: WaitScreen( text, font, rect, color )
{
}


Notification::~Notification()
{
}


void Notification::Draw( void )
{
	double seconds = Lifetime.ElapsedSeconds();
	float text_alpha = 1.f;
	
	if( seconds >= 3. )
	{
		Remove();
		return;
	}
	else if( seconds >= 2. )
	{
		Alpha -= Raptor::Game->FrameTime;
		text_alpha = 1.f - (seconds - 2.f);
	}
	
	UpdateRects();
	Window::Draw();
	TextFont->DrawText( Text, Rect.w / 2, Rect.h / 2, Font::ALIGN_MIDDLE_CENTER, 1.f, 1.f, 1.f, text_alpha );
}


bool Notification::KeyDown( SDLKey key )
{
	return true;
}


bool Notification::KeyUp( SDLKey key )
{
	if( (key == SDLK_ESCAPE) || (key == SDLK_RETURN) || (key == SDLK_KP_ENTER) )
		Remove();
	
	return true;
}


bool Notification::MouseDown( Uint8 button )
{
	return true;
}


bool Notification::MouseUp( Uint8 button )
{
	Remove();
	return true;
}
