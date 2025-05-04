/*
 *  LabelledButton.cpp
 */

#include "LabelledButton.h"


LabelledButton::LabelledButton( SDL_Rect *rect, Font *label_font, std::string text, uint8_t align, Animation *normal, Animation *mouse_down, Animation *mouse_over ) : Button( rect, normal, mouse_down, mouse_over )
{
	LabelText = text;
	LabelAlign = align;
	
	LabelFont = label_font;
	
	RedNormal = 1.0;
	GreenNormal = 1.0;
	BlueNormal = 1.0;
	AlphaNormal = 0.8;
	AlphaDisabled = 0.5;
	
	RedDown = 1.0;
	GreenDown = 1.0;
	BlueDown = 0.0;
	AlphaDown = 1.0;
	
	RedOver = 1.0;
	GreenOver = 1.0;
	BlueOver = 1.0;
	AlphaOver = 1.0;
}


LabelledButton::~LabelledButton()
{
	LabelFont = NULL;
}


void LabelledButton::Draw( void )
{
	Button::Draw();
	
	if( LabelFont )
	{
		if( MouseIsWithin && Enabled )
		{
			if( MouseIsDown )
				LabelFont->DrawText( LabelText, 0, 0, Rect.w, Rect.h, LabelAlign, RedDown, GreenDown, BlueDown, AlphaDown );
			else 
				LabelFont->DrawText( LabelText, 0, 0, Rect.w, Rect.h, LabelAlign, RedOver, GreenOver, BlueOver, AlphaOver );
		}
		else
			LabelFont->DrawText( LabelText, 0, 0, Rect.w, Rect.h, LabelAlign, RedNormal, GreenNormal, BlueNormal, Enabled ? AlphaNormal : AlphaDisabled );
	}
}


void LabelledButton::SizeToText( void )
{
	if( LabelFont )
	{
		SDL_Rect rect = {0,0,0,0};
		LabelFont->TextSize( LabelText, &rect );
		Rect.w = rect.w;
	}
}
