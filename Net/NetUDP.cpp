/*
 *  NetUDP.cpp
 */

#include "NetUDP.h"

#ifdef WIN32
#include <windows.h>
#include <winsock.h>
#include <iphlpapi.h>
#pragma comment( lib, "iphlpapi.lib" )
#endif


NetUDP::NetUDP( void )
{
	Listening = false;
	Port = 7000;
	Socket = NULL;
	SDLPacket = NULL;
}


NetUDP::~NetUDP()
{
	StopListening();

	if( SDLPacket )
		SDLNet_FreePacket( SDLPacket );
	SDLPacket = NULL;
}


int NetUDP::Initialize( void )
{
	// NOTE: SDL_net must be initialized prior to this!

	// Initialize a packet buffer that will be utilized for both incoming and outgoing packets.
	SDLPacket = SDLNet_AllocPacket( PACKET_BUFFER_SIZE );
	if( ! SDLPacket )
	{
		fprintf( stderr, "NetUDP::Initialize: SDLNet_AllocPacket: %s\n", SDLNet_GetError() );
		return -1;
	}
	
	return 0;
}


int NetUDP::StartListening( int port )
{
	if( Listening )
		StopListening();

	Port = port;

	// Open the listening port.
	Socket = SDLNet_UDP_Open( Port );
	if( ! Socket )
	{
		fprintf( stderr, "NetUDP::Initialize: SDLNet_UDP_Open: %s\n", SDLNet_GetError() );
		return -1;
	}

	Listening = true;
	return 0;
}


void NetUDP::StopListening( void )
{
	Listening = false;

	if( Socket )
		SDLNet_UDP_Close( Socket );
	Socket = NULL;
}


NetUDPPacket *NetUDP::GetPacket( void )
{
	if( ! Listening )
		return NULL;
	if( ! SDLPacket )
		return NULL;

	// Return one packet from the UDP input buffer.
	if( SDLNet_UDP_Recv( Socket, SDLPacket ) )
		// The Packet constructor copies the data buffer, so pass it the raw pointer and size.
		return new NetUDPPacket( SDLPacket );

	// Return NULL if there are no packets to process.
	return NULL;
}


void NetUDP::Broadcast( Packet *packet, int port )
{
#ifndef WIN32
	// Everything that isn't Windows does this the easy way.

	// Send to 255.255.255.255 for LAN broadcast.
	IPaddress ip;
	ip.host = INADDR_BROADCAST;
	Endian::WriteBig16( port, &(ip.port) );
	Send( packet, &ip );

#else
	// Fuck you Windows.

	// Adapted from: http://developerweb.net/viewtopic.php?pid=21773#p21773
	// We call GetIpAddrTable() multiple times in order to make the appropriate size of buffer,
	// and deal with any potential race conditions.

	MIB_IPADDRTABLE *ip_table = NULL;
	{
		ULONG buffer_size = 0;
		for( int i = 0; i < 5; i ++ )
		{
			// Attempt to fill the buffer, and get the needed size if buffer too small.
			DWORD ip_table_result = GetIpAddrTable( ip_table, &buffer_size, false );
			if( ip_table_result == ERROR_INSUFFICIENT_BUFFER )
			{
				// Free the buffer, in case we had previously allocated it too small.
				free( ip_table );
				// GetIpAddrTable updated buffer_length with the required size, so allocate for our next try.
				ip_table = (MIB_IPADDRTABLE *) malloc( buffer_size );
			}
			else if( ip_table_result == NO_ERROR )
				// Success!  We got ip_table filled.
				break;
			else
			{
				// Something unexpected went wrong, so give up.
				free( ip_table );
				ip_table = NULL;
				break;
			}
		}
	}
	
	int broadcast_count = 0;

	if( ip_table )
	{
		for( int i = 0; i < ip_table->dwNumEntries; i ++ )
		{
			const MIB_IPADDRROW &row = ip_table->table[ i ];

			// Apply the netmask to the IP to get the broadcast address.
			uint32_t broadcast_address = row.dwAddr & row.dwMask;
			if( row.dwBCastAddr )
				broadcast_address |= ~(row.dwMask);

			// Build a network-endian IPaddress.
			IPaddress ip;
			ip.host = broadcast_address;
			Endian::WriteBig16( port, &(ip.port) );

			// Finally we get to send!
			Send( packet, &ip );
			
			broadcast_count ++;
		}

		free( ip_table );
		ip_table = NULL;
	}
	else
		fprintf( stderr, "NetUDP::Broadcast: GetIpAddrTable failed.\n" );
	
	if( ! broadcast_count )
	{
		// If we failed to send any broadcasts the hard way, try sending to 255.255.255.255 for LAN broadcast.
		IPaddress ip;
		ip.host = INADDR_BROADCAST;
		Endian::WriteBig16( port, &(ip.port) );
		Send( packet, &ip );
	}
	
#endif
}


void NetUDP::Send( Packet *packet, const char *hostname, int port )
{
	// Resolve the host we are connecting to.
	IPaddress ip;
	if( SDLNet_ResolveHost( &ip, hostname, port ) < 0 )
	{
		fprintf( stderr, "NetUDP::Send: SDLNet_ResolveHost: %s\n", SDLNet_GetError() );
		return;
	}

	// Send!
	Send( packet, &ip );
}


void NetUDP::Send( Packet *packet, IPaddress *ip )
{
	if( ! SDLPacket )
		return;

	// Get the size we'll be sending from our outgoing packet.
	SDLPacket->len = packet->Size();
	if( SDLPacket->len <= SDLPacket->maxlen )
	{
		// Open a temporary UDP socket from a random port.
		if( UDPsocket socket = SDLNet_UDP_Open( 0 ) )
		{
			// Set the packet's destination.
			SDLPacket->address.host = ip->host;
			SDLPacket->address.port = ip->port;
			SDLPacket->channel = -1;

			// Copy the packet data.
			memcpy( SDLPacket->data, packet->Data, packet->Size() );

			// Send to packet address.
			if( ! SDLNet_UDP_Send( socket, -1, SDLPacket ) )
				fprintf( stderr, "NetUDP::Send: SDLNet_UDP_Send: %s\n", SDLNet_GetError() );

			// Close our temporary UDP socket.
			SDLNet_UDP_Close( socket );
		}
		else
			fprintf( stderr, "NetUDP::Send: SDLNet_UDP_Open: %s\n", SDLNet_GetError() );
	}
	else
	{
		fprintf( stderr, "NetUDP::Send: Packet too large for outgoing buffer size.\n" );
		SDLPacket->len = 0;
	}
}


// ---------------------------------------------------------------------------


NetUDPPacket::NetUDPPacket( UDPpacket *sdl_packet ) : Packet( sdl_packet->data, sdl_packet->len )
{
	IP = sdl_packet->address.host;
	Port = sdl_packet->address.port;
}
