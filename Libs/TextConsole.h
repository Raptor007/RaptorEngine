/*
 *  TextConsole.h
 */

#pragma once

#include "PlatformSpecific.h"
class TextConsole;
class TextConsoleMessage;

#include <stdint.h>
#include <deque>
#include "Layer.h"
#include "Clock.h"
#include "Font.h"
#include "TextBox.h"


class TextConsole
{
public:
	std::deque<TextConsoleMessage*> Messages;
	SDL_mutex *Lock;
	FILE *OutFile;
	
	TextConsole( void );
	virtual ~TextConsole();
	
	virtual void Print( std::string text, uint32_t type = MSG_NORMAL );
	void Clear( void );
	
	enum
	{
		MSG_NORMAL = 0,
		MSG_INPUT = 'Inpt',
		MSG_ERROR = 'Err ',
		MSG_CHAT = 'Chat',
		MSG_TEAM = 'Team'
	};
};


class TextConsoleMessage
{
public:
	std::string Text;
	uint32_t Type;
	Clock TimeStamp;
	
	TextConsoleMessage( std::string text, uint32_t type );
	virtual ~TextConsoleMessage();
};
