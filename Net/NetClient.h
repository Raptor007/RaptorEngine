/*
 *  NetClient.h
 */

#pragma once
class NetClient;

#include "platforms.h"

#include <stdint.h>
#include <queue>
#include <map>
#include <string>
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include <SDL_net/SDL_net.h>
#include "Packet.h"
#include "Clock.h"


class NetClient
{
public:
	bool Initialized;
	volatile bool Connected;
	SDL_Thread *Thread;
	SDL_mutex *Lock;
	TCPsocket Socket;
	std::queue< Packet*, std::list<Packet*> > InBuffer;
	Clock NetClock;
	double NetRate;
	int8_t Precision;
	uintmax_t BytesSent;
	uintmax_t BytesReceived;
	std::list<double> PingTimes;
	std::map<uint8_t,Clock> SentPings;
	
	int ReconnectAttempts;
	int ReconnectTime;
	Clock ReconnectClock;
	std::string Host;
	int Port;
	
	
	NetClient( void );
	~NetClient();
	
	int Initialize( double net_rate = 30., int8_t precision = 0 );
	int Connect( const char *host, const char *name, const char *password );
	int Connect( const char *hostname, int port, const char *name, const char *password );
	int Reconnect( const char *name, const char *password );
	void DisconnectNice( const char *message = NULL );
	void Disconnect( void );
	void Cleanup( void );
	void ClearPackets( void );
	
	void ProcessIn( void );
	bool ProcessPacket( Packet *packet );
	
	int Send( Packet *packet );
	
	void SendUpdates( void );
	void SendUpdate( void );
	void SendPing( void );
	double LatestPing( void );
	double AveragePing( void );
	double MedianPing( void );
	
	std::string Status( void );
};


int NetClientThread( void *client );
