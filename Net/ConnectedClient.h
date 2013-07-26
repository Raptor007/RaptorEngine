/*
 *  ConnectedClient.h
 */

#pragma once
class ConnectedClient;

#include "PlatformSpecific.h"

#include <queue>
#include <map>
#include <stdexcept>
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include <SDL_net/SDL_net.h>
#include "Packet.h"
#include "Clock.h"
#include "Identifier.h"
#include "NetServer.h"
#include "Mutex.h"


class ConnectedClient
{
public:
	volatile bool Connected;
	SDL_Thread *InThread, *OutThread;
	Mutex InLock, OutLock;
	TCPsocket Socket;
	unsigned int IP;
	unsigned short Port;
	std::queue< Packet*, std::list<Packet*> > InBuffer, OutBuffer;
	bool Synchronized;
	Clock NetClock;
	double NetRate;
	int8_t Precision;
	uint64_t BytesSent;
	uint64_t BytesReceived;
	std::list<double> PingTimes;
	std::map<uint8_t,Clock> SentPings;
	bool UseOutThread;
	
	uint16_t PlayerID;
	
	
	ConnectedClient( TCPsocket socket, bool use_out_thread = false, double net_rate = 30., int8_t precision = 0 );
	~ConnectedClient();
	
	void DisconnectNice( const char *message = NULL );
	void Disconnect( void );
	void Cleanup( void );
	
	void ProcessIn( void );
	void ProcessTop( void );
	bool ProcessPacket( Packet *packet );
	
	bool Send( Packet *packet );
	void SendOthers( Packet *packet );
	
	// This should ONLY be called by Send() or ConnectedClientOutThread!
	bool SendNow( Packet *packet );
	
	void Login( std::string name, std::string password );
	
	static int ConnectedClientInThread( void *client );
	static int ConnectedClientOutThread( void *client );
	
private:
	void SendToOutBuffer( Packet *packet );
};
