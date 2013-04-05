/*
 *  Screensaver.h
 */

#pragma once
class Screensaver;

#include "platforms.h"

#include "Layer.h"


class Screensaver : public Layer
{
public:
	Screensaver( void );
	virtual ~Screensaver();
	
	bool HandleEvent( SDL_Event *event, bool already_handled = false );
};
