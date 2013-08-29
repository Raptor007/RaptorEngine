/*
 *  Label.cpp
 */

#include "Label.h"


Label::Label( SDL_Rect *rect, std::string text, Font *font, uint8_t align ) : Layer( rect )
{
	LabelText = text;
	LabelAlign = align;
	LabelFont = font;
	
	Red = 1.f;
	Green = 1.f;
	Blue = 1.f;
	Alpha = 1.f;
}


Label::~Label()
{
}


void Label::Draw( void )
{
	if( LabelFont )
	{
		LabelFont->DrawText( LabelText, 0, 0, Rect.w, Rect.h, LabelAlign, Red, Green, Blue, Alpha );
	}
}
