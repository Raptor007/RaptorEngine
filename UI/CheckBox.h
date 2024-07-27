/*
 *  CheckBox.h
 */

#pragma once
class CheckBox;

#include "PlatformSpecific.h"

#include "LabelledButton.h"
#include "Font.h"
#include "RaptorGame.h"


class CheckBox : public LabelledButton
{
public:
	bool Checked;
	Animation *ImageNormalChecked;
	Animation *ImageMouseDownChecked;
	Animation *ImageMouseOverChecked;
	
	CheckBox( SDL_Rect *rect, Font *font, std::string text, bool checked = false, Animation *u_normal = Raptor::Game->Res.GetAnimation("box_unchecked.ani"), Animation *u_down = Raptor::Game->Res.GetAnimation("box_unchecked_mdown.ani"), Animation *u_over = NULL, Animation *c_normal = Raptor::Game->Res.GetAnimation("box_checked.ani"), Animation *c_down = Raptor::Game->Res.GetAnimation("box_checked_mdown.ani"), Animation *c_over = NULL );
	virtual ~CheckBox();
	
	virtual void Draw( void );
	virtual void MouseEnter( void );
	virtual void MouseLeave( void );
	virtual bool MouseDown( Uint8 button = SDL_BUTTON_LEFT );
	virtual bool MouseUp( Uint8 button = SDL_BUTTON_LEFT );
	
	virtual void Clicked( Uint8 button = SDL_BUTTON_LEFT );
	virtual void Changed( void );
	
	virtual void SizeToText( void );
};
