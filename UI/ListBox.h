/*
 *  ListBox.h
 */

#pragma once
class ListBox;
class ListBoxItem;
class ListBoxButton;

#include "platforms.h"

#include "Layer.h"
#include <string>
#include <vector>
#include <SDL/SDL.h>
#include "Font.h"
#include "Animation.h"
#include "Button.h"


class ListBox : public Layer
{
public:
	std::vector<ListBoxItem> Items;
	ListBoxItem *Selected;
	Font *TextFont;
	float TextRed, TextGreen, TextBlue, TextAlpha;
	float SelectedRed, SelectedGreen, SelectedBlue, SelectedAlpha;
	int ScrollBarSize;
	float ScrollBarRed, ScrollBarGreen, ScrollBarBlue, ScrollBarAlpha;
	int Scroll;
	
	ListBox( Layer *container, SDL_Rect *rect, Font *font, int scroll_bar_size );
	ListBox( Layer *container, SDL_Rect *rect, Font *font, int scroll_bar_size, std::vector<ListBoxItem> items );
	virtual ~ListBox();
	
	void AddItem( std::string value, std::string text );
	int FindItem( std::string value );
	void RemoveItem( std::string value );
	void RemoveItem( int index );
	void Clear( void );
	
	int LineScroll( void );
	int MaxScroll( void );
	void ScrollUp( void );
	void ScrollDown( void );
	
	void UpdateRects( void );
	void Draw( void );
	void MouseEnter( void );
	void MouseLeave( void );
	bool MouseDown( Uint8 button = SDL_BUTTON_LEFT );
	bool MouseUp( Uint8 button = SDL_BUTTON_LEFT );
	virtual void Changed( void );
	
	std::string SelectedValue( void );
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
	
	ListBoxItem( std::string value, std::string text );
};


class ListBoxButton : public Button
{
public:
	uint8_t Action;
	
	ListBoxButton( ListBox *list_box, uint8_t action, Animation *normal, Animation *mouse_down );
	void UpdateRect( void );
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
};
