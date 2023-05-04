/*
 *  DropDown.h
 */

#pragma once
class DropDown;
class DropDownListBox;

#include "PlatformSpecific.h"
#include "LabelledButton.h"
#include <cstddef>
#include "ListBox.h"


class DropDown : public LabelledButton
{
public:
	std::vector<ListBoxItem> Items;
	std::string Value;
	int ScrollBarSize;
	DropDownListBox *MyListBox;
	
	bool ClickOutEventHandle;
	std::set<const SDL_Event*> EventsHandled;
	
	DropDown( SDL_Rect *rect, Font *font, uint8_t align, int scroll_bar_size, Animation *normal, Animation *mouse_down = NULL, Animation *mouse_over = NULL );
	virtual ~DropDown();
	
	void AddItem( std::string value, std::string text );
	int FindItem( std::string value );
	void RemoveItem( std::string value );
	void RemoveItem( int index );
	void Clear( void );
	virtual void Update( void );
	
	virtual void SizeToText( void );
	void TrackEvent( SDL_Event *event );
	bool HandleEvent( SDL_Event *event );
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
	void Close( void );
	bool Select( std::string value );
	bool Select( int index );
	virtual void Changed( void );
};

class DropDownListBox : public ListBox
{
public:
	DropDown *CalledBy;
	
	DropDownListBox( DropDown *dropdown );
	virtual ~DropDownListBox();
	
	void AutoSize( void );
	void Changed( void );
	bool KeyDown( SDLKey key );
	bool KeyUp( SDLKey key );
};
