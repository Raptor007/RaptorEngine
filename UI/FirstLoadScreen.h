/*
 *  FirstLoadScreen.h
 */

#pragma once
class FirstLoadScreen;

#include "PlatformSpecific.h"
#include "Layer.h"
#include <cstddef>
#include "Animation.h"
#include "Font.h"


class FirstLoadScreen : public Layer
{
public:
	Animation Background;
	Font *TextFont;
	
	FirstLoadScreen( const char *bg_filename = "bg_menu_small.ani", Font *font = NULL );
	virtual ~FirstLoadScreen();
	
	void Draw( void );
};
