/*
 *  WaitScreen.h
 */

#pragma once
class WaitScreen;

#include "PlatformSpecific.h"
#include "Window.h"
#include <cstddef>
#include "Font.h"
#include "Color.h"


class WaitScreen : public Window
{
public:
	Font *TextFont;
	std::string Text;
	bool AutoPosition;
	
	WaitScreen( std::string text, Font *font = NULL, SDL_Rect *rect = NULL, Color *color = NULL );
	virtual ~WaitScreen();
	
	void UpdateRects( void );
	virtual void Draw( void );
};
