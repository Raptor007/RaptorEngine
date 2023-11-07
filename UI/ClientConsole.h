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

#ifdef SDL2
	#include <SDL2/SDL.h>
#else
	#include <SDL/SDL.h>
#endif


class ClientConsole : public TextConsole, public Layer
{
public:
	bool Initialized;
	SDLKey ToggleKey;
	Font *MessageFont;
	TextBox *Input;
	int Scroll;
	std::vector<std::string> History;
	size_t HistoryIndex;
	
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
