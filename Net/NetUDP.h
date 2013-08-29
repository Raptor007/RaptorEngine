/*
 *  NetUDP.h
 */

#pragma once
class NetUDP;
class NetUDPPacket;

#include "PlatformSpecific.h"

#include <SDL_net/SDL_net.h>
#include "Packet.h"


class NetUDP
{
public:
	bool Listening;

	NetUDP( void );
	virtual ~NetUDP();
	
	int Initialize( void );
	int StartListening( int port );
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
