/*
 *  DropDown.h
 */

#pragma once
class DropDown;
class DropDownListBox;

#include "PlatformSpecific.h"

#include "LabelledButton.h"
#include "ListBox.h"


class DropDown : public LabelledButton
{
public:
	std::vector<ListBoxItem> Items;
	std::string Value;
	int ScrollBarSize;
	
	DropDown( Layer *container, SDL_Rect *rect, Font *font, uint8_t align, int scroll_bar_size, Animation *normal, Animation *mouse_down = NULL, Animation *mouse_over = NULL );
	virtual ~DropDown();
	
	void Update( void );
	void Clicked( Uint8 button = SDL_BUTTON_LEFT );
	virtual void Changed( void );
};

class DropDownListBox : public ListBox
{
public:
	DropDown *CalledBy;
	
	DropDownListBox( DropDown *dropdown );
	virtual ~DropDownListBox();
	
	void Changed( void );
	bool KeyDown( SDLKey key );
	bool KeyUp( SDLKey key );
};
