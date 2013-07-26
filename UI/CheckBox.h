/*
 *  CheckBox.h
 */

#pragma once
class CheckBox;

#include "PlatformSpecific.h"

#include "LabelledButton.h"
#include "Font.h"


class CheckBox : public LabelledButton
{
public:
	bool Checked;
	Animation *ImageNormalChecked;
	Animation *ImageMouseDownChecked;
	Animation *ImageMouseOverChecked;
	
	
	CheckBox( Layer *container, SDL_Rect *rect, Font *font, std::string text, bool checked, Animation *u_normal, Animation *u_down, Animation *u_over, Animation *c_normal, Animation *c_down, Animation *c_over );
	virtual ~CheckBox();
	
	virtual void Draw( void );
	virtual void MouseEnter( void );
	virtual void MouseLeave( void );
	virtual bool MouseDown( Uint8 button = SDL_BUTTON_LEFT );
	virtual bool MouseUp( Uint8 button = SDL_BUTTON_LEFT );
	
	virtual void Clicked( Uint8 button = SDL_BUTTON_LEFT );
	virtual void Changed( void );
};
