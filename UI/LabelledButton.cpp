/*
 *  LabelledButton.cpp
 */

#include "LabelledButton.h"


LabelledButton::LabelledButton( Layer *container, SDL_Rect *rect, Font *label_font, std::string text, uint8_t align, Animation *normal, Animation *mouse_down, Animation *mouse_over ) : Button( container, rect, normal, mouse_down, mouse_over )
{
	LabelText = text;
	LabelAlign = align;
	
	LabelFont = label_font;
	
	RedNormal = 1.0;
	GreenNormal = 1.0;
	BlueNormal = 1.0;
	AlphaNormal = 0.8;
	
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
		if( MouseIsWithin )
		{
			if( MouseIsDown )
				LabelFont->DrawText( LabelText, &Rect, LabelAlign, RedDown, GreenDown, BlueDown, AlphaDown );
			else 
				LabelFont->DrawText( LabelText, &Rect, LabelAlign, RedOver, GreenOver, BlueOver, AlphaOver );
		}
		else
			LabelFont->DrawText( LabelText, &Rect, LabelAlign, RedNormal, GreenNormal, BlueNormal, AlphaNormal );
	}
}
