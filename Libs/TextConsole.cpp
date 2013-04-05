/*
 *  TextConsole.cpp
 */


#include "TextConsole.h"


TextConsole::TextConsole( void )
{
	OutFile = NULL;
	
	Lock = SDL_CreateMutex();
}


TextConsole::~TextConsole()
{
	SDL_DestroyMutex( Lock );
	Lock = NULL;

	Clear();
}


void TextConsole::Print( std::string text, uint32_t type )
{
	if( Lock )
	{
		if( SDL_mutexP( Lock ) )
			fprintf( stderr, "TextConsole::Print: SDL_mutexP(Lock): %s\n", SDL_GetError() );
	}
	
	if( OutFile )
	{
		const char *cstr = text.c_str();
		fwrite( cstr, 1, strlen(cstr), OutFile );
		fprintf( OutFile, "\n" );
		fflush( OutFile );
	}
	
	Messages.push_back( new TextConsoleMessage( text, type ) );
	
	if( Lock )
	{
		if( SDL_mutexV( Lock ) )
			fprintf( stderr, "TextConsole::Print: SDL_mutexV(Lock): %s\n", SDL_GetError() );
	}
}


void TextConsole::Clear( void )
{
	while( Messages.size() )
	{
		delete Messages.front();
		Messages.pop_front();
	}
}


// ---------------------------------------------------------------------------


TextConsoleMessage::TextConsoleMessage( std::string text, uint32_t type )
{
	Text = text;
	Type = type;
}
