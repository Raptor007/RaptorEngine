/*
 *  LabelledButton.h
 */

#pragma once
class LabelledButton;

#include "Button.h"
#include "Font.h"


class LabelledButton : public Button
{
public:
	std::string LabelText;
	uint8_t LabelAlign;
	Font *LabelFont;
	double RedNormal, GreenNormal, BlueNormal, AlphaNormal;
	double RedDown, GreenDown, BlueDown, AlphaDown;
	double RedOver, GreenOver, BlueOver, AlphaOver;
	
	LabelledButton( Layer *container, SDL_Rect *rect, Font *label_font, std::string text, uint8_t align, Animation *normal, Animation *mouse_down = NULL, Animation *mouse_over = NULL );
	virtual ~LabelledButton();
	
	void Draw( void );
};
