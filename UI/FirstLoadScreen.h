/*
 *  FirstLoadScreen.h
 */

#pragma once
class FirstLoadScreen;

#include "PlatformSpecific.h"

#include "Window.h"
#include "Animation.h"
#include "Font.h"


class FirstLoadScreen : public Window
{
public:
	Animation Background;
	Font *TextFont;
	
	FirstLoadScreen( const char *bg_filename = "bg_menu_small.ani" );
	virtual ~FirstLoadScreen();
	
	void Draw( void );
};
