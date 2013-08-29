/*
 *  GroupBox.h
 */

#pragma once
class GroupBox;

#include "PlatformSpecific.h"
#include "Layer.h"
#include "Font.h"


class GroupBox : public Layer
{
public:
	std::string TitleText;
	uint8_t TitleAlign;
	Font *TitleFont;
	float BorderWidth;
	
	GroupBox( SDL_Rect *rect, std::string text, Font *font, float border_width = 1.f, uint8_t align = Font::ALIGN_BASELINE_LEFT );
	virtual ~GroupBox();
	
	void Draw( void );
};
