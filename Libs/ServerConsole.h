/*
 *  ServerConsole.h
 */

#pragma once

#include "PlatformSpecific.h"
class ServerConsole;

#include "TextConsole.h"


class ServerConsole : public TextConsole
{
public:
	virtual ~ServerConsole();
	
	void Run( void );
};
