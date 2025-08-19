/*
 *  ListBox.h
 */

#pragma once
class ListBox;
class ListBoxItem;
class ListBoxButton;

#include "PlatformSpecific.h"

#include "Layer.h"
#include <string>
#include <vector>

#ifdef SDL2
	#include <SDL2/SDL.h>
#else
	#include <SDL/SDL.h>
#endif

#include "Font.h"
#include "Animation.h"
#include "Button.h"
#include "Color.h"


class ListBox : public Layer
{
public:
	std::vector<ListBoxItem> Items;
	ListBoxItem *Selected;
	bool AllowDeselect;
	uint8_t TextAlign;
	Font *TextFont;
	float TextRed, TextGreen, TextBlue, TextAlpha;
	float SelectedRed, SelectedGreen, SelectedBlue, SelectedAlpha;
	int ScrollBarSize;
	float ScrollBarRed, ScrollBarGreen, ScrollBarBlue, ScrollBarAlpha;
	int VRHeight;
	int PadX;
	
	int Scroll;
	bool ClickedScrollBar;
	
	ListBox( SDL_Rect *rect, Font *font, int scroll_bar_size );
	ListBox( SDL_Rect *rect, Font *font, int scroll_bar_size, std::vector<ListBoxItem> items );
	virtual ~ListBox();
	
	void AddItem( std::string value, std::string text, const Color *color = NULL );
	int FindItem( std::string value );
	void RemoveItem( std::string value );
	void RemoveItem( int index );
	void Clear( void );
	
	int LineScroll( void );
	int MaxScroll( void );
	void ScrollUp( int lines = 1 );
	void ScrollDown( int lines = 1 );
	void ScrollTo( std::string value, int at = 0 );
	void ScrollTo( int index, int at = 0 );
	void ScrollToSelected( int at = 0 );
	
	void UpdateRects( void );
	void Draw( void );
	virtual void DrawItem( const ListBoxItem *item, const SDL_Rect *rect );
	
	void TrackEvent( SDL_Event *event );
	virtual bool HandleEvent( SDL_Event *event );
	void MouseEnter( void );
	void MouseLeave( void );
	bool MouseDown( Uint8 button = SDL_BUTTON_LEFT );
	bool MouseUp( Uint8 button = SDL_BUTTON_LEFT );
	virtual void Changed( void );
	
	std::string SelectedValue( void );
	std::string SelectedText( void );
	void Select( std::string value );
	void Select( int index );
	
	enum
	{
		SCROLL_UP = 1,
		SCROLL_DOWN
	};
};


class ListBoxItem
{
public:
	std::string Value;
	std::string Text;
	bool HasCustomColor;
	Color CustomColor;
	
	ListBoxItem( std::string value, std::string text );
	void SetColor( float r, float g, float b, float a = 1.f );
};


class ListBoxButton : public Button
{
public:
	uint8_t Action;
	
	ListBoxButton( uint8_t action, Animation *normal, Animation *mouse_down );
	void UpdateRect( void );
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};
