/*
 *  ServerConsole.cpp
 */


#include "ServerConsole.h"

#include <iostream>
#include "RaptorGame.h"


ServerConsole::~ServerConsole()
{
}


void ServerConsole::Run( void )
{
	char input[ 10240 ] = "";
	bool running = true;
	
	while( running )
	{
		std::cin.getline( input, 10240 );
		
		if( strcmp( input, "quit" ) == 0 )
			running = false;
		else if( strlen(input) )
			// FIXME
			Raptor::Game->Cfg.Command( std::string("sv ") + input );
	}
}
