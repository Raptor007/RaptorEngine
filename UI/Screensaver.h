/*
 *  Screensaver.h
 */

#pragma once
class Screensaver;
class ScreensaverFoundServer;

#include "PlatformSpecific.h"

#include "Layer.h"
#include "Clock.h"
#include "NetUDP.h"
#include <set>
#include <map>

#ifdef SDL2
	#include <SDL2/SDL.h>
#else
	#include <SDL/SDL.h>
#endif


class Screensaver : public Layer
{
public:
	std::set<SDLKey> IgnoreKeys;
	int MouseMoves;
	Clock MouseMoved;
	
	NetUDP ServerFinder;
	std::map<std::string,ScreensaverFoundServer> FoundServers;
	
	Screensaver( void );
	virtual ~Screensaver();
	
	bool HandleEvent( SDL_Event *event );
	void Draw( void );
};


class ScreensaverFoundServer
{
public:
	int Players, Screensavers;
	bool SSConnect;
	float Uptime;
};
