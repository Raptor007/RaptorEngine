/*
 *  Label.h
 */

#pragma once
class Label;

#include "Layer.h"
#include "Font.h"


class Label : public Layer
{
public:
	std::string LabelText;
	uint8_t LabelAlign;
	Font *LabelFont;
	
	Label( Layer *container, SDL_Rect *rect, std::string text, Font *font, uint8_t align );
	virtual ~Label();
	
	void Draw( void );
};
