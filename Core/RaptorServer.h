/*
 *  RaptorServer.h
 */

#pragma once
class RaptorServer;

#include "PlatformSpecific.h"

#include <string>
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include "NetServer.h"
#include "Packet.h"
#include "GameData.h"
#include "TextConsole.h"


class RaptorServer
{
public:
	std::string Game;
	std::string Version;
	
	NetServer Net;
	SDL_Thread *Thread;
	TextConsole *Console;
	int Port;
	double MaxFPS;
	double NetRate;
	bool Announce;
	double AnnounceInterval;
	
	double FrameTime;
	
	volatile int State;
	GameData Data;
	
	
	RaptorServer( std::string game, std::string version );
	virtual ~RaptorServer();
	
	int Start( std::string name );
	void StopAndWait( double max_wait_seconds = 3. );
	bool IsRunning( void );
	
	void ConsoleStart( void );
	void ConsolePrint( std::string text, uint32_t type = 0 );
	
	virtual void Update( double dt );
	
	virtual void Started( void );
	virtual void Stopped( void );
	virtual bool ProcessPacket( Packet *packet, ConnectedClient *from_client );
	virtual bool ValidateLogin( std::string name, std::string password );
	virtual void AcceptedClient( ConnectedClient *client );
	virtual void DroppedClient( ConnectedClient *client );
	virtual void SendUpdate( ConnectedClient *client, int8_t precision = 0 );
	
	virtual void ChangeState( int state );
	
	static int RaptorServerThread( void *game_server );
};


#ifndef GAMESERVER_CPP
namespace Raptor
{
	extern RaptorServer *Server;
}
#endif
