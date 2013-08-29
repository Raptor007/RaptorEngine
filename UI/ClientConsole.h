/*
 *  ClientConsole.h
 */

#pragma once

#include "PlatformSpecific.h"
class ClientConsole;

#include "TextConsole.h"
#include "Clock.h"
#include "Font.h"
#include "TextBox.h"
#include <SDL/SDL.h>


class ClientConsole : public TextConsole, public Layer
{
public:
	bool Initialized;
	SDLKey ToggleKey;
	Font *MessageFont;
	TextBox *Input;
	std::deque<std::string> History;
	int Scroll;
	
	ClientConsole( void );
	virtual ~ClientConsole();
	
	void Initialize( void );
	void UpdateRects( void );
	
	void Draw( void );
	bool HandleEvent( SDL_Event *event );
	
	bool IsActive( void );
	void ToggleActive( void );
	
	void ScrollUp( int lines = 1 );
	void ScrollDown( int lines = 1 );
	
private:
	bool Active;
};
