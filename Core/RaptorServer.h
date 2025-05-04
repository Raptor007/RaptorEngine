/*
 *  RaptorServer.h
 */

#pragma once
class RaptorServer;

#include "PlatformSpecific.h"

#include <string>

#ifdef SDL2
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_thread.h>
#else
	#include <SDL/SDL.h>
	#include <SDL/SDL_thread.h>
#endif

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
	int AnnouncePort;
	double AnnounceInterval;
	
	double FrameTime;
	
	volatile int State;
	GameData Data;
	
	
	RaptorServer( std::string game, std::string version );
	virtual ~RaptorServer();
	
	bool Start( std::string name );
	void StopAndWait( double max_wait_seconds = 3. );
	bool IsRunning( void );
	
	void ConsoleStart( void );
	void ConsolePrint( std::string text, uint32_t type = 0 );
	
	virtual void Update( double dt );
	
	virtual void Started( void );
	virtual void Stopped( void );
	virtual bool HandleCommand( std::string cmd, std::vector<std::string> *params = NULL );
	virtual bool ProcessPacket( Packet *packet, ConnectedClient *from_client );
	virtual bool CompatibleVersion( std::string version ) const;
	virtual bool ValidateLogin( std::string name, std::string password );
	virtual void AcceptedClient( ConnectedClient *client );
	virtual void DroppedClient( ConnectedClient *client );
	virtual void SendUpdate( ConnectedClient *client );
	virtual bool SetPlayerProperty( Player *player, std::string name, std::string value, bool force = false );
	
	virtual void ChangeState( int state );
	virtual void SetProperty( std::string name, std::string value );
	
	static int RaptorServerThread( void *game_server );
};


#ifndef GAMESERVER_CPP
namespace Raptor
{
	extern RaptorServer *Server;
}
#endif
