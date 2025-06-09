/*
 *  NetUDP.h
 */

#pragma once
class NetUDP;
class NetUDPPacket;

#include "PlatformSpecific.h"

#ifdef SDL2
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_net.h>
#else
	#include <SDL/SDL.h>
	#ifdef __APPLE__
		#include <SDL_net/SDL_net.h>
	#else
		#include <SDL/SDL_net.h>
	#endif
#endif

#include "Packet.h"


class NetUDP
{
public:
	bool Listening;
	
	NetUDP( void );
	virtual ~NetUDP();
	
	bool Initialize( void );
	bool StartListening( int port );
	void StopListening( void );
	
	NetUDPPacket *GetPacket( void );
	void Broadcast( Packet *packet, int port );
	void Send( Packet *packet, const char *hostname, int port );
	void Send( Packet *packet, IPaddress *ip );

private:
	int Port;
	UDPsocket Socket;
	UDPpacket *SDLPacket;
};


class NetUDPPacket : public Packet
{
public:
	uint32_t IP;
	uint16_t Port;
	
	NetUDPPacket( UDPpacket *sdl_packet );
	virtual ~NetUDPPacket();
};
