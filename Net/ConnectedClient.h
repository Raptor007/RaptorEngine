/*
 *  ConnectedClient.h
 */

#pragma once
class ConnectedClient;

#include "PlatformSpecific.h"
#include <cstddef>
#include <queue>
#include <map>
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
#include "Clock.h"
#include "Identifier.h"
#include "NetServer.h"
#include "Mutex.h"


class ConnectedClient
{
public:
	volatile bool Connected;
	std::string Version;
	SDL_Thread *InThread, *OutThread, *CleanupThread;
	Mutex InLock, OutLock;
	TCPsocket Socket;
	unsigned int IP;
	unsigned short Port;
	std::queue< Packet*, std::list<Packet*> > InBuffer, OutBuffer;
	bool Synchronized;
	Clock NetClock, PingClock;
	double NetRate, PingRate;
	int8_t Precision;
	uint64_t BytesSent;
	uint64_t BytesReceived;
	std::list<double> PingTimes;
	std::map<uint8_t,Clock> SentPings;
	Clock ResyncClock;
	uint16_t PlayerID, DropPlayerID;
	
	
	ConnectedClient( TCPsocket socket, double net_rate = 30., int8_t precision = 0 );
	virtual ~ConnectedClient();
	
	void Cleanup( double wait_for_threads = 0.5 );
	
	void DisconnectNice( const char *message = NULL );
	void Disconnect( void );
	
	void ProcessIn( void );
	void ProcessTop( void );
	bool ProcessPacket( Packet *packet );
	
	void Login( std::string name, std::string password );
	
	bool Send( Packet *packet );
	
	// This should ONLY be called by Send() or ConnectedClientOutThread!
	bool SendNow( Packet *packet );
	
	void SendOthers( Packet *packet );
	
	bool SendPing( void );
	bool SendResync( void );
	double LatestPing( void );
	double AveragePing( void );
	double MedianPing( void );
	
	static int ConnectedClientInThread( void *client );
	static int ConnectedClientOutThread( void *client );
	static int ConnectedClientCleanupThread( void *client );
	
private:
	void SendToOutBuffer( Packet *packet );
};
