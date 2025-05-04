/*
 *  MouseState.cpp
 */

#include "MouseState.h"

#include "RaptorGame.h"


MouseState::MouseState( void )
{
	ShowCursor = true;
	X = 0;
	Y = 0;
	PointX = 0;
	PointY = 0;
	Size = 0;
	OffsetX = 0;
	OffsetY = 0;
}


MouseState::MouseState( Animation *cursor, int size )
{
	ShowCursor = true;
	X = 0;
	Y = 0;
	PointX = 0;
	PointY = 0;
	OffsetX = 0;
	OffsetY = 0;
	SetCursor( cursor, size );
}


MouseState::~MouseState()
{
}


void MouseState::TrackEvent( SDL_Event *event )
{
	// Update the mouse cursor position and button status.
	
	if( event->type == SDL_MOUSEMOTION )
	{
		X = (event->motion.x += OffsetX);
		Y = (event->motion.y += OffsetY);
	}
	else if( event->type == SDL_MOUSEBUTTONDOWN )
	{
		X = (event->button.x += OffsetX);
		Y = (event->button.y += OffsetY);
		ButtonsDown[ event->button.button ] = true;
	}
	else if( event->type == SDL_MOUSEBUTTONUP )
	{
		X = (event->button.x += OffsetX);
		Y = (event->button.y += OffsetY);
		ButtonsDown[ event->button.button ] = false;
	}
#if SDL_VERSION_ATLEAST(2,26,0)
	else if( event->type == SDL_MOUSEWHEEL )
	{
		X = (event->wheel.mouseX + OffsetX);
		Y = (event->wheel.mouseY + OffsetY);
	}
#endif
}


bool MouseState::ButtonDown( Uint8 button ) const
{
	// Check if a mouse button is down.
	// Assume it is not down if its state has never been recorded.
	
	std::map<Uint8, bool>::const_iterator button_iter = ButtonsDown.find( button );
	if( button_iter != ButtonsDown.end() )
		return button_iter->second;
	else
		return false;
}


void MouseState::SetOffset( int x, int y )
{
	OffsetX = x;
	OffsetY = y;
}


void MouseState::SetCursor( Animation *cursor, int size )
{
	SetCursor( cursor, size, size/2, size/2 );
}


void MouseState::SetCursor( Animation *cursor, int size, int point_x, int point_y )
{
	Cursor.BecomeInstance( cursor );
	Size = size;
	PointX = point_x;
	PointY = point_y;
}


void MouseState::Draw( void )
{
	if( ShowCursor )
	{
		glPushMatrix();
		
		// Set up UI rendering output.
		Raptor::Game->Gfx.Setup2D();
		
		// Clamp the cursor texture.
		GLuint texture = Cursor.CurrentFrame();
		glBindTexture( GL_TEXTURE_2D, texture );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
		
		// Draw the cursor.
		Raptor::Game->Gfx.DrawRect2D( X - PointX, Y - PointY, X + Size - PointX, Y + Size - PointY, texture, 1.f, 1.f, 1.f, 1.f );
		
		glPopMatrix();
	}
}


std::string MouseState::Status( void ) const
{
	// Create a status string for the mouse.
	
	std::string return_string;
	char cstr[ 1024 ] = "";

	snprintf( cstr, 1024, "Mouse at: %i, %i\n", X, Y );
	return_string += cstr;

	return_string += "Mouse buttons down:";
	for( std::map<Uint8, bool>::const_iterator button_iter = ButtonsDown.begin(); button_iter != ButtonsDown.end(); button_iter ++ )
	{
		if( button_iter->second )
		{
			snprintf( cstr, 1024, " %s", Raptor::Game->Cfg.MouseName(button_iter->first).c_str() );
			return_string += cstr;
		}
	}
	
	return return_string;
}
