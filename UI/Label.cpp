/*
 *  Label.cpp
 */

#include "Label.h"


Label::Label( Layer *container, SDL_Rect *rect, std::string text, Font *font, uint8_t align ) : Layer( container, rect )
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
		LabelFont->DrawText( LabelText, &Rect, LabelAlign, Red, Green, Blue, Alpha );
}
