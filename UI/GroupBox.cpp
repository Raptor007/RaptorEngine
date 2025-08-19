/*
 *  GroupBox.cpp
 */

#include "GroupBox.h"
#include "RaptorGame.h"


GroupBox::GroupBox( SDL_Rect *rect, std::string text, Font *font, float border_width, uint8_t align ) : Layer( rect )
{
	TitleText = text;
	TitleAlign = align;
	TitleFont = font;
	
	BorderWidth = border_width;
	
	Red = 1.f;
	Green = 1.f;
	Blue = 1.f;
	Alpha = 1.f;
}


GroupBox::~GroupBox()
{
}


void GroupBox::Draw( void )
{
	if( TitleFont && ! TitleText.empty() )
	{
		float ui_scale = UIScaleMode ? Raptor::Game->UIScale : 1.f;
		SDL_Rect text_rect = {0,0,0,0};
		TitleFont->TextSize( TitleText, &text_rect );
		text_rect.w *= ui_scale;
		text_rect.h *= ui_scale;
		
		int x = 0, y = 0;
		
		switch( TitleAlign )
		{
			case Font::ALIGN_TOP_LEFT:
			case Font::ALIGN_TOP_CENTER:
			case Font::ALIGN_TOP_RIGHT:
				y = 0;
				break;
			case Font::ALIGN_MIDDLE_LEFT:
			case Font::ALIGN_MIDDLE_CENTER:
			case Font::ALIGN_MIDDLE_RIGHT:
				y = text_rect.h / 2;
				break;
			case Font::ALIGN_BASELINE_LEFT:
			case Font::ALIGN_BASELINE_CENTER:
			case Font::ALIGN_BASELINE_RIGHT:
				y = TitleFont->GetAscent() * ui_scale + 0.5f;
				break;
			case Font::ALIGN_BOTTOM_LEFT:
			case Font::ALIGN_BOTTOM_CENTER:
			case Font::ALIGN_BOTTOM_RIGHT:
				y = text_rect.h;
				break;
		}
		
		switch( TitleAlign )
		{
			case Font::ALIGN_TOP_LEFT:
			case Font::ALIGN_MIDDLE_LEFT:
			case Font::ALIGN_BASELINE_LEFT:
			case Font::ALIGN_BOTTOM_LEFT:
				x = 5 * ui_scale;
				break;
			case Font::ALIGN_TOP_CENTER:
			case Font::ALIGN_MIDDLE_CENTER:
			case Font::ALIGN_BASELINE_CENTER:
			case Font::ALIGN_BOTTOM_CENTER:
				x = (Rect.w - text_rect.w) / 2;
				break;
			case Font::ALIGN_TOP_RIGHT:
			case Font::ALIGN_MIDDLE_RIGHT:
			case Font::ALIGN_BASELINE_RIGHT:
			case Font::ALIGN_BOTTOM_RIGHT:
				x = Rect.w - text_rect.w - 5 * ui_scale;
				break;
		}
		
		Raptor::Game->Gfx.DrawLine2D( 0, y, 0, CalcRect.h, BorderWidth, Red, Green, Blue, Alpha );
		Raptor::Game->Gfx.DrawLine2D( CalcRect.w, y, CalcRect.w, CalcRect.h, BorderWidth, Red, Green, Blue, Alpha );
		Raptor::Game->Gfx.DrawLine2D( 0, CalcRect.h, CalcRect.w, CalcRect.h, BorderWidth, Red, Green, Blue, Alpha );
		Raptor::Game->Gfx.DrawLine2D( 0, y, x, y, BorderWidth, Red, Green, Blue, Alpha );
		Raptor::Game->Gfx.DrawLine2D( x + text_rect.w + 10 * ui_scale, y, CalcRect.w, y, BorderWidth, Red, Green, Blue, Alpha );
		
		TitleFont->DrawText( TitleText, x + 5.f * ui_scale, y, TitleAlign, Red, Green, Blue, Alpha, ui_scale );
	}
	else
	{
		Raptor::Game->Gfx.DrawBox2D( 0, 0, CalcRect.x, CalcRect.y, BorderWidth, Red, Green, Blue, Alpha );
	}
}
