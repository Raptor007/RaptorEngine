/*
 *  LabelledButton.cpp
 */

#include "LabelledButton.h"

#include "RaptorGame.h"


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
	
	PadX = 0;
	PadY = 0;
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
		float ui_scale = UIScaleMode ? Raptor::Game->UIScale : 1.f;
		int pad_x = PadX * ui_scale;
		int pad_y = PadY * ui_scale;
		
		if( MouseIsWithin && Enabled )
		{
			if( MouseIsDown )
				LabelFont->DrawText( LabelText, pad_x, pad_y, CalcRect.w - pad_x * 2, CalcRect.h - pad_y * 2, LabelAlign, RedDown, GreenDown, BlueDown, AlphaDown, ui_scale );
			else 
				LabelFont->DrawText( LabelText, pad_x, pad_y, CalcRect.w - pad_x * 2, CalcRect.h - pad_y * 2, LabelAlign, RedOver, GreenOver, BlueOver, AlphaOver, ui_scale );
		}
		else
			LabelFont->DrawText( LabelText, pad_x, pad_y, CalcRect.w - pad_x * 2, CalcRect.h - pad_y * 2, LabelAlign, RedNormal, GreenNormal, BlueNormal, Enabled ? AlphaNormal : AlphaDisabled, ui_scale );
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
