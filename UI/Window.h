/*
 *  Window.h
 */

#pragma once
class Window;

#include "PlatformSpecific.h"

#include <stdexcept>

#ifdef SDL2
	#include <SDL2/SDL.h>
#else
	#include <SDL/SDL.h>
#endif

#include "Layer.h"
#include "Graphics.h"


class Window : public Layer
{
public:
	Window( void );
	Window( SDL_Rect *param_rect );
	Window( SDL_Rect *param_rect, float red, float green, float blue, float alpha );
	Window( SDL_Rect *param_rect, SDL_Color *param_color );
	virtual ~Window();
	
	virtual void Draw( void );
};
