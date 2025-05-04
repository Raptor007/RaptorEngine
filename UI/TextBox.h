/*
 *  TextBox.h
 */

#pragma once
class TextBox;

#include "PlatformSpecific.h"

#include "Layer.h"
#include <string>

#ifdef SDL2
	#include <SDL2/SDL.h>
#else
	#include <SDL/SDL.h>
#endif

#include "Font.h"


class TextBox : public Layer
{
public:
	std::string Text;
	int Cursor;
	bool ShiftIsDown;
	Font *TextFont;
	uint8_t Align;
	bool ReturnDeselects, PassReturn, EscDeselects, PassEsc, TabDeselects, PassTab, PassExtendedKeys, ClickOutDeselects;
	float SelectedRed, SelectedGreen, SelectedBlue, SelectedAlpha;
	float TextRed, TextGreen, TextBlue, TextAlpha;
	float SelectedTextRed, SelectedTextGreen, SelectedTextBlue, SelectedTextAlpha;
	std::string CursorAppearance;
	bool CenterCursor;
	char IgnoreNext;
	
	TextBox( SDL_Rect *rect, Font *font, uint8_t align );
	TextBox( SDL_Rect *rect, Font *font, uint8_t align, std::string text );
	virtual ~TextBox();
	
	void Draw( void );
	void TrackEvent( SDL_Event *event );
	virtual bool HandleEvent( SDL_Event *event );
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
	virtual void Deselected( void );
	
private:
	Clock CursorTimer;
};
