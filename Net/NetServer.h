/*
 *  NetServer.h
 */

#pragma once
class NetServer;

#include "PlatformSpecific.h"

#include <list>
#include <queue>
#include <stdexcept>

#ifdef SDL2
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_thread.h>
	#include <SDL2/SDL_net.h>
#else
	#include <SDL/SDL.h>
	#include <SDL/SDL_thread.h>
	#ifdef __APPLE__
		#include <SDL_net/SDL_net.h>
	#else
		#include <SDL/SDL_net.h>
	#endif
#endif

#include "Packet.h"
#include "ConnectedClient.h"
#include "Mutex.h"


class NetServer
{
public:
	bool Initialized;
	volatile bool Listening;
	SDL_Thread *Thread;
	Mutex Lock;
	TCPsocket Socket;
	std::list<ConnectedClient*> Clients, DisconnectedClients;
	double NetRate;
	double ResyncTime, DisconnectTime;
	int8_t Precision;
	
	
	NetServer( void );
	virtual ~NetServer();
	
	bool Initialize( int port = 0 );
	void DisconnectNice( const char *message );
	void Disconnect( void );
	
	void RemoveDisconnectedClients( void );
	void ProcessIn( void );
	void ProcessTop( void );
	
	void SendToPlayer( Packet *packet, uint32_t player_id );
	void SendAll( Packet *packet );
	void SendAllExcept( Packet *packet, ConnectedClient *except );
	void SendAllReconnect( uint8_t seconds_to_wait = 3 );
	void SendUpdates( void );
	
	void SetNetRate( double netrate );
	
	static int NetServerThread( void *server );
};
