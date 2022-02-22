/*
 *  TextBox.h
 */

#pragma once
class TextBox;

#include "PlatformSpecific.h"

#include "Layer.h"
#include <string>
#include <SDL/SDL.h>
#include "Font.h"


class TextBox : public Layer
{
public:
	std::string Text;
	int Cursor;
	bool ShiftIsDown;
	Font *TextFont;
	uint8_t Align;
	bool ReturnDeselects, PassReturn, EscDeselects, PassEsc, TabDeselects, PassTab;
	float SelectedRed, SelectedGreen, SelectedBlue, SelectedAlpha;
	float TextRed, TextGreen, TextBlue, TextAlpha;
	float SelectedTextRed, SelectedTextGreen, SelectedTextBlue, SelectedTextAlpha;
	std::string CursorAppearance;
	bool CenterCursor;
	
	TextBox( SDL_Rect *rect, Font *font, uint8_t align );
	TextBox( SDL_Rect *rect, Font *font, uint8_t align, std::string text );
	virtual ~TextBox();
	
	void Draw( void );
	void MouseEnter( void );
	void MouseLeave( void );
	bool MouseDown( Uint8 button = SDL_BUTTON_LEFT );
	bool MouseUp( Uint8 button = SDL_BUTTON_LEFT );
	bool KeyDown( SDLKey key );
	bool KeyUp( SDLKey key );
	void InsertAtCursor( std::string str );
	void InsertAtCursor( char character );
	void UpdateCursor( void );
	virtual void Changed( void );

private:
	Clock CursorTimer;
};
