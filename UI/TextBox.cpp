/*
 *  TextBox.cpp
 */

#include "TextBox.h"

#include <cstddef>


TextBox::TextBox( SDL_Rect *rect, Font *font, uint8_t align ) : Layer( rect )
{
	Text = "";
	Cursor = 0;
	ShiftIsDown = false;
	TextFont = font;
	Align = align;
	
	ReturnDeselects = false;
	PassReturn = true;
	EscDeselects = true;
	PassEsc = false;
	TabDeselects = true;
	PassTab = false;
	PassExtendedKeys = true;
	ClickOutDeselects = true;
	
	CursorAppearance = "|";
	CenterCursor = true;
	
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 0.75f;
	
	SelectedRed = 0.f;
	SelectedGreen = 1.f;
	SelectedBlue = 1.f;
	SelectedAlpha = 1.f;
	
	TextRed = 1.f;
	TextGreen = 1.f;
	TextBlue = 1.f;
	TextAlpha = 1.f;
	
	SelectedTextRed = 0.f;
	SelectedTextGreen = 0.f;
	SelectedTextBlue = 0.f;
	SelectedTextAlpha = 1.f;
	
	if( TextFont )
		Rect.h = TextFont->GetHeight();
}


TextBox::TextBox( SDL_Rect *rect, Font *font, uint8_t align, std::string text ) : Layer( rect )
{
	Text = text;
	Cursor = Text.length();
	ShiftIsDown = false;
	TextFont = font;
	Align = align;
	
	ReturnDeselects = false;
	PassReturn = true;
	EscDeselects = true;
	PassEsc = false;
	TabDeselects = true;
	PassTab = false;
	PassExtendedKeys = true;
	ClickOutDeselects = true;
	
	CursorAppearance = "|";
	CenterCursor = true;
	
	Red = 0.f;
	Green = 0.f;
	Blue = 0.f;
	Alpha = 0.75f;
	
	SelectedRed = 1.f;
	SelectedGreen = 1.f;
	SelectedBlue = 0.f;
	SelectedAlpha = 1.f;
	
	TextRed = 1.f;
	TextGreen = 1.f;
	TextBlue = 1.f;
	TextAlpha = 1.f;
	
	SelectedTextRed = 0.f;
	SelectedTextGreen = 0.f;
	SelectedTextBlue = 0.f;
	SelectedTextAlpha = 1.f;
	
	if( TextFont )
		Rect.h = TextFont->GetHeight();
}


TextBox::~TextBox()
{
}


void TextBox::Draw( void )
{
	if( IsSelected() )
		glColor4f( SelectedRed, SelectedGreen, SelectedBlue, SelectedAlpha );
	else
		glColor4f( Red, Green, Blue, Alpha );
	
	glBegin( GL_QUADS );
		
		glVertex2i( 0, 0 );
		glVertex2i( Rect.w, 0 );
		glVertex2i( Rect.w, Rect.h );
		glVertex2i( 0, Rect.h );
		
	glEnd();
	
	if( TextFont )
	{
		SDL_Rect rect = Rect;
		rect.x = 0;
		rect.y = 0;
		
		if( IsSelected() )
			TextFont->DrawText( Text, &rect, Align, SelectedTextRed, SelectedTextGreen, SelectedTextBlue, SelectedTextAlpha );
		else
			TextFont->DrawText( Text, &rect, Align, TextRed, TextGreen, TextBlue, TextAlpha );
		
		if( IsSelected() )
		{
			bool show_cursor = ((int)(CursorTimer.ElapsedSeconds() * 4.0))%2;
			if( show_cursor )
			{
				UpdateCursor();
				
				SDL_Rect size = {0,0,0,0};
				TextFont->TextSize( Text.substr( 0, Cursor ), &size );
				int x = size.w, y = 0;
				
				switch( Align )
				{
					case Font::ALIGN_TOP_LEFT:
					case Font::ALIGN_TOP_CENTER:
					case Font::ALIGN_TOP_RIGHT:
						y = 0;
						break;
					case Font::ALIGN_MIDDLE_LEFT:
					case Font::ALIGN_MIDDLE_CENTER:
					case Font::ALIGN_MIDDLE_RIGHT:
						y = Rect.h / 2;
						break;
					case Font::ALIGN_BASELINE_LEFT:
					case Font::ALIGN_BASELINE_CENTER:
					case Font::ALIGN_BASELINE_RIGHT:
						y = TextFont->GetAscent();
						break;
					case Font::ALIGN_BOTTOM_LEFT:
					case Font::ALIGN_BOTTOM_CENTER:
					case Font::ALIGN_BOTTOM_RIGHT:
						y = Rect.h;
						break;
				}
				
				switch( Align )
				{
					case Font::ALIGN_TOP_LEFT:
					case Font::ALIGN_MIDDLE_LEFT:
					case Font::ALIGN_BASELINE_LEFT:
					case Font::ALIGN_BOTTOM_LEFT:
						x = size.w;
						break;
					case Font::ALIGN_TOP_CENTER:
					case Font::ALIGN_MIDDLE_CENTER:
					case Font::ALIGN_BASELINE_CENTER:
					case Font::ALIGN_BOTTOM_CENTER:
						x = (size.w + Rect.w) / 2;
						break;
					case Font::ALIGN_TOP_RIGHT:
					case Font::ALIGN_MIDDLE_RIGHT:
					case Font::ALIGN_BASELINE_RIGHT:
					case Font::ALIGN_BOTTOM_RIGHT:
						x = Rect.w;
						break;
				}
				
				if( CenterCursor )
					x -= TextFont->LineWidth( CursorAppearance ) / 2;
				
				TextFont->DrawText( CursorAppearance, x, y, Align, SelectedTextRed, SelectedTextGreen, SelectedTextBlue, SelectedTextAlpha );
			}
		}
	}
}


void TextBox::TrackEvent( SDL_Event *event )
{
	Layer::TrackEvent( event );
	
	if( ClickOutDeselects && IsSelected() && (event->type == SDL_MOUSEBUTTONUP) && (event->button.button == SDL_BUTTON_LEFT) && ! MouseIsWithin )
	{
		Container->Selected = NULL;
		Deselected();
	}
}


void TextBox::MouseEnter( void )
{
}


void TextBox::MouseLeave( void )
{
}


bool TextBox::MouseDown( Uint8 button )
{
	if( button != SDL_BUTTON_LEFT )
		return false;
	
	return true;
}


bool TextBox::MouseUp( Uint8 button )
{
	if( button != SDL_BUTTON_LEFT )
		return false;
	
	Container->Selected = this;
	Cursor = Text.length();
	return true;
}


bool TextBox::KeyDown( SDLKey key )
{
	if( IsSelected() )
	{
		if( (key == SDLK_RETURN) || (key == SDLK_KP_ENTER) )
		{
			if( PassReturn )
				return false;
		}
		else if( key == SDLK_ESCAPE )
		{
			if( PassEsc )
				return false;
		}
		else if( key == SDLK_TAB )
		{
			if( PassTab )
				return false;
		}
		else if( key == SDLK_BACKSPACE )
		{
			UpdateCursor();
			if( Cursor > 0 )
			{
				Text.erase( Cursor - 1, 1 );
				Cursor --;
				Changed();
			}
		}
		else if( key == SDLK_DELETE )
		{
			UpdateCursor();
			if( (size_t) Cursor < Text.length() )
			{
				Text.erase( Cursor, 1 );
				Changed();
			}
		}
		else if( key == SDLK_LEFT )
		{
			if( Cursor > 0 )
				Cursor --;
		}
		else if( key == SDLK_RIGHT )
		{
			if( (size_t) Cursor < Text.length() )
				Cursor ++;
		}
		else if( (key == SDLK_UP) || (key == SDLK_HOME) )
			Cursor = 0;
		else if( (key == SDLK_DOWN) || (key == SDLK_END) )
			Cursor = Text.length();
		else if( (key == SDLK_LSHIFT) || (key == SDLK_RSHIFT) || (key == SDLK_CAPSLOCK) )
			ShiftIsDown = true;
		
		else if( (key >= SDLK_KP0) && (key <= SDLK_KP9) )
			InsertAtCursor( ((char) key) + '0'-SDLK_KP0 );
		else if( key == SDLK_KP_PERIOD )
			InsertAtCursor( '.' );
		else if( key == SDLK_KP_MINUS )
			InsertAtCursor( '-' );
		else if( key == SDLK_KP_PLUS )
			InsertAtCursor( '+' );
		else if( key == SDLK_KP_MULTIPLY )
			InsertAtCursor( '*' );
		else if( key == SDLK_KP_DIVIDE )
			InsertAtCursor( '/' );
		else if( key == SDLK_KP_EQUALS )
			InsertAtCursor( '=' );
		else if( ShiftIsDown && (key >= 'a') && (key <= 'z') )
			InsertAtCursor( ((char) key) + 'A'-'a' );
		else if( ShiftIsDown && (key == '`') )
			InsertAtCursor( '~' );
		else if( ShiftIsDown && (key == '1') )
			InsertAtCursor( '!' );
		else if( ShiftIsDown && (key == '2') )
			InsertAtCursor( '@' );
		else if( ShiftIsDown && (key == '3') )
			InsertAtCursor( '#' );
		else if( ShiftIsDown && (key == '4') )
			InsertAtCursor( '$' );
		else if( ShiftIsDown && (key == '5') )
			InsertAtCursor( '%' );
		else if( ShiftIsDown && (key == '6') )
			InsertAtCursor( '^' );
		else if( ShiftIsDown && (key == '7') )
			InsertAtCursor( '&' );
		else if( ShiftIsDown && (key == '8') )
			InsertAtCursor( '*' );
		else if( ShiftIsDown && (key == '9') )
			InsertAtCursor( '(' );
		else if( ShiftIsDown && (key == '0') )
			InsertAtCursor( ')' );
		else if( ShiftIsDown && (key == '-') )
			InsertAtCursor( '_' );
		else if( ShiftIsDown && (key == '=') )
			InsertAtCursor( '+' );
		else if( ShiftIsDown && (key == '[') )
			InsertAtCursor( '{' );
		else if( ShiftIsDown && (key == ']') )
			InsertAtCursor( '}' );
		else if( ShiftIsDown && (key == '\\') )
			InsertAtCursor( '|' );
		else if( ShiftIsDown && (key == ';') )
			InsertAtCursor( ':' );
		else if( ShiftIsDown && (key == '\'') )
			InsertAtCursor( '\"' );
		else if( ShiftIsDown && (key == ',') )
			InsertAtCursor( '<' );
		else if( ShiftIsDown && (key == '.') )
			InsertAtCursor( '>' );
		else if( ShiftIsDown && (key == '/') )
			InsertAtCursor( '?' );
		else if( key <= 255 )
			InsertAtCursor( (char) key );
		else if( PassExtendedKeys && (key >= SDLK_F1) )
			return false;
		
		return true;
	}
	
	return false;
}


bool TextBox::KeyUp( SDLKey key )
{
	if( IsSelected() )
	{
		if( (key == SDLK_RETURN) || (key == SDLK_KP_ENTER) )
		{
			if( ReturnDeselects )
			{
				Container->Selected = NULL;
				Deselected();
			}
			if( PassReturn )
				return false;
		}
		else if( key == SDLK_ESCAPE )
		{
			if( EscDeselects )
			{
				Container->Selected = NULL;
				Deselected();
			}
			if( PassEsc )
				return false;
		}
		else if( key == SDLK_TAB )
		{
			if( TabDeselects )
			{
				Container->Selected = NULL;
				Deselected();
			}
			if( PassTab )
				return false;
		}
		else if( (key == SDLK_LSHIFT) || (key == SDLK_RSHIFT) || (key == SDLK_CAPSLOCK) )
			ShiftIsDown = false;
		else if( PassExtendedKeys && (key >= SDLK_F1) )
			return false;
		
		return true;
	}
	
	return false;
}


void TextBox::InsertAtCursor( std::string str )
{
	UpdateCursor();
	Text.insert( Cursor, str );
	Cursor += str.length();
	Changed();
}


void TextBox::InsertAtCursor( char character )
{
	std::string str = "";
	str += character;
	InsertAtCursor( str );
}


void TextBox::UpdateCursor( void )
{
	if( Cursor < 0 )
		Cursor = 0;
	else if( (size_t) Cursor > Text.length() )
		Cursor = Text.length();
}


void TextBox::Changed( void )
{
}


void TextBox::Deselected( void )
{
}
