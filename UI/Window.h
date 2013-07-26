/*
 *  Window.h
 */

#pragma once
class Window;

#include "PlatformSpecific.h"

#include <stdexcept>
#include <SDL/SDL.h>
#include "Layer.h"
#include "Graphics.h"


class Window : public Layer
{
public:
	std::string Title;
	
	Window( void );
	Window( SDL_Rect *param_rect );
	Window( SDL_Rect *param_rect, float red, float green, float blue, float alpha );
	Window( SDL_Rect *param_rect, SDL_Color *param_color );
	virtual ~Window();
	
	void Draw( void );
};
