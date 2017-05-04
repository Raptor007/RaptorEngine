/*
 *  NetClient.cpp
 */

#include "NetClient.h"

#include "RaptorDefs.h"
#include "PacketBuffer.h"
#include "ClientConfig.h"
#include "Str.h"
#include "Num.h"
#include "RaptorGame.h"


NetClient::NetClient( void )
{
	Initialized = false;
	Connected = false;
	Thread = NULL;
	Socket = NULL;
	NetRate = 30.0;
	PingRate = 4.;
	Precision = 0;
	BytesSent = 0;
	BytesReceived = 0;
	
	Lock = SDL_CreateMutex();
	
	ReconnectTime = 0;
	ReconnectAttempts = 0;
}


NetClient::~NetClient()
{
	Connected = false;
	
	// Sleep until the other thread has finished (max 4 sec).
	Clock wait_for_thread;
	while( Thread && (wait_for_thread.ElapsedSeconds() < 4.) )
		SDL_Delay( 1 );
	
	// If the thread didn't finish, kill it.
	if( Thread )
	{
		SDL_KillThread( Thread );
		Thread = NULL;
	}
	
	// Clean up old data and socket.
	Cleanup();
	
	if( Lock )
	{
		SDL_DestroyMutex( Lock );
		Lock = NULL;
	}
	
	if( Initialized )
		SDLNet_Quit();
}


int NetClient::Initialize( double net_rate, int8_t precision )
{
	NetRate = net_rate;
	Precision = precision;
	
	// Initialize SDL_net.
	if( SDLNet_Init() < 0 )
	{
		fprintf( stderr, "SDLNet_Init: %s\n", SDLNet_GetError() );
		return -1;
	}
	
	Initialized = true;
	return 0;
}


int NetClient::Connect( const char *host, const char *name, const char *password )
{
	int return_value = 0;
	
	char *colon = NULL;
	if( host )
		colon = strrchr( (char *) host, ':' );
	
	if( colon )
	{
		char *hostname = CStr::Copy( host );
		hostname[ colon - host ] = '\0';
		
		int port = atoi( colon + 1 );
		
		return_value = Connect( hostname, port, name, password );
		
		CStr::Delete( hostname );
		hostname = NULL;
	}
	else
		return_value = Connect( host, 0, name, password );
	
	return return_value;
}


int NetClient::Connect( const char *hostname, int port, const char *name, const char *password )
{
	Host = hostname;
	
	if( port > 0 )
		Port = port;
	else
		Port = 7000;
	
	if( ! Initialized )
		return -1;
	
	if( Connected )
		DisconnectNice();
	
	// Clean up old data and socket.
	SDL_Delay( 1 );
	Cleanup();
	
	// Show the wait screen.
	char cstr[ 1024 ] = "";
	snprintf( cstr, 1024, "Connecting to %s:%i...", hostname, Port );
	Raptor::Game->Console.Print( cstr );
	Raptor::Game->ChangeState( Raptor::State::CONNECTING );
	
	// Resolve the host we are connecting to.
	IPaddress ip;
	if( SDLNet_ResolveHost( &ip, hostname, Port ) < 0 )
	{
		Raptor::Game->Console.Print( "Failed to resolve server hostname.", TextConsole::MSG_ERROR );
		Raptor::Game->ChangeState( Raptor::State::DISCONNECTED );
		return -1;
	}
	
	// Open a connection with the IP provided.
	if( !( Socket = SDLNet_TCP_Open(&ip) ) )
	{
		Raptor::Game->Console.Print( "Failed to open socket to server.", TextConsole::MSG_ERROR );
		Raptor::Game->ChangeState( Raptor::State::DISCONNECTED );
		return -1;
	}
	
	BytesSent = 0;
	BytesReceived = 0;
	
	// Start the listener thread.
	Connected = true;
	if( !( Thread = SDL_CreateThread( NetClientThread, this ) ) )
	{
		fprintf( stderr, "SDL_CreateThread: %s\n", SDLNet_GetError() );
		Connected = false;
		SDLNet_TCP_Close( Socket );
		Socket = NULL;
		Raptor::Game->ChangeState( Raptor::State::DISCONNECTED );
		return -1;
	}
	
	uint32_t ip_int = Endian::ReadBig32(&(ip.host));
	uint16_t port_int = Endian::ReadBig16(&(ip.port));
	snprintf( cstr, 1024, "Connected to: %i.%i.%i.%i:%i", (ip_int & 0xFF000000) >> 24, (ip_int & 0x00FF0000) >> 16, (ip_int & 0x0000FF00) >> 8, ip_int & 0x000000FF, port_int );
	Raptor::Game->Console.Print( cstr );
	
	// Send login information.
	Packet packet( Raptor::Packet::LOGIN );
	packet.AddString( Raptor::Game->Game );
	packet.AddString( Raptor::Game->Version );
	packet.AddString( name );
	packet.AddString( password );  // FIXME: This assumes every game requires a password!
	Send( &packet );
	
	// If we connected successfully, don't try to reconnect.
	ReconnectTime = 0;
	ReconnectAttempts = 0;
	
	return 0;
}


int NetClient::Reconnect( const char *name, const char *password )
{
	if( ReconnectAttempts )
	{
		// Reset the time-to-reconnect clock and subtract one attempt.
		ReconnectClock.Reset();
		ReconnectAttempts --;
		
		// Attempt to reconnect.
		return Connect( Host.c_str(), Port, name, password );
	}
	
	return -1;
}


void NetClient::DisconnectNice( const char *message )
{
	if( Connected )
	{
		// Tell the server we're leaving.

		Packet packet( Raptor::Packet::DISCONNECT );
		
		if( message )
			packet.AddString( message );
		else
			packet.AddString( "" );
		
		Send( &packet );
	}
	
	// Disconnect from the server.
	Disconnect();
}


void NetClient::Disconnect( void )
{
	if( Connected )
		Raptor::Game->Console.Print( "Disconnected from server." );
	
	// Set disconnected state.
	Connected = false;
	
	ReconnectClock.Reset();
	
	// If we're hosting the game, stop the server.
	if( Raptor::Server->IsRunning() )
		Raptor::Server->StopAndWait();
}


void NetClient::Cleanup( void )
{
	if( ! Connected )
	{
		Raptor::Game->ChangeState( Raptor::State::DISCONNECTED );
		
		if( ! Thread )
		{
			// If the connection is open, close it.
			if( Socket )
			{
				SDLNet_TCP_Close( Socket );
				Socket = NULL;
			}
			
			// This empties the incoming packet buffer.
			ClearPackets();
			
			PingTimes.clear();
			SentPings.clear();
		}
	}
}


void NetClient::ClearPackets( void )
{
	if( Lock )
		SDL_mutexP( Lock );
	
	while( ! InBuffer.empty() )
	{
		Packet *packet = InBuffer.front();
		delete packet;
		InBuffer.pop();
	}
	
	if( Lock )
		SDL_mutexV( Lock );
}


void NetClient::ProcessIn( void )
{
	SDL_mutexP( Lock );
	
	// Process the entire incoming packet buffer.
	while( ! InBuffer.empty() )
	{
		Packet *packet = InBuffer.front();
		ProcessPacket( packet );
		delete packet;
		InBuffer.pop();
	}
	
	SDL_mutexV( Lock );
}


bool NetClient::ProcessPacket( Packet *packet )
{
	packet->Rewind();
	PacketType type = packet->Type();
	
	if( Raptor::Game->ProcessPacket( packet ) )
		return true;
	
	else if( type == Raptor::Packet::PING )
	{
		uint8_t ping_id = packet->NextUChar();
		Packet pong( Raptor::Packet::PONG );
		pong.AddUChar( ping_id );
		Send( &pong );
	}
	
	else if( type == Raptor::Packet::PONG )
	{
		uint8_t ping_id = packet->NextUChar();
		std::map<uint8_t,Clock>::iterator ping_iter = SentPings.find( ping_id );
		if( ping_iter != SentPings.end() )
		{
			double ms = ping_iter->second.ElapsedMilliseconds();
			SentPings.erase( ping_iter );
			PingTimes.push_back( ms );
			while( PingTimes.size() > 120 )
				PingTimes.pop_front();
		}
	}
	
	else if( type == Raptor::Packet::PADDING )
	{
		// Always ignore padding packets.
		packet->Offset = packet->Size();
	}
	
	else if( type == Raptor::Packet::CHANGE_STATE )
	{
		int state = packet->NextInt();
		Raptor::Game->ChangeState( state );
	}
	
	else if( type == Raptor::Packet::LOGIN )
	{
		Raptor::Game->PlayerID = packet->NextUShort();
		if( Raptor::Game->State >= Raptor::State::CONNECTING )
			Raptor::Game->ChangeState( Raptor::State::CONNECTED );
		else
			DisconnectNice();
	}
	
	else if( type == Raptor::Packet::DISCONNECT )
	{
		std::string message = packet->NextString();
		
		Disconnect();
	}
	
	else if( type == Raptor::Packet::RECONNECT )
	{
		uint8_t time = packet->NextUChar();
		
		Disconnect();
		
		ReconnectTime = time;
		ReconnectAttempts = 3;
		ReconnectClock.Reset();
	}
	
	else
		return false;
	
	// This means one of the if statements above matched, so NetClient did handle the packet.
	return true;
}


int NetClient::Send( Packet *packet )
{
	if( ! Initialized )
		return -1;
	if( ! Connected )
		return -1;
	
	if( SDLNet_TCP_Send( Socket, (void *) packet->Data, packet->Size() ) < (int) packet->Size() )
	{
		//fprintf( stderr, "SDLNet_TCP_Send: %s\n", SDLNet_GetError() );
		Disconnect();
		return -1;
	}
	else
		BytesSent += packet->Size();
	
	return 0;
}


void NetClient::SendUpdates( void )
{
	// Reduce update rate temporarily in high-ping situations.
	double temp_netrate = NetRate;
	temp_netrate /= ((int) LatestPing() / 100) + 1;
	
	// Send an update if it's time to do so.
	if( Connected && Raptor::Game->PlayerID && (NetClock.ElapsedSeconds() >= (1.0 / temp_netrate)) )
	{
		NetClock.Reset();
		
		if( PingClock.ElapsedSeconds() >= (1.0 / PingRate) )
		{
			PingClock.Reset();
			SendPing();
		}
		
		SendUpdate();
	}
}


void NetClient::SendUpdate( void )
{
	Raptor::Game->SendUpdate( Precision );
}


void NetClient::SendPing( void )
{
	uint8_t ping_id = 0;
	bool found_available = false;
	
	for( int i = 0; i <= 255; i ++ )
	{
		if( SentPings.find( i ) == SentPings.end() )
		{
			ping_id = i;
			found_available = true;
			break;
		}
	}
	
	if( ! found_available )
	{
		for( std::map<uint8_t,Clock>::iterator ping_iter = SentPings.begin(); ping_iter != SentPings.end(); )
		{
			std::map<uint8_t,Clock>::iterator ping_next = ping_iter;
			ping_next ++;
			
			if( ping_iter->second.ElapsedSeconds() > 3. )
			{
				ping_id = ping_iter->first;
				SentPings.erase( ping_iter );
				found_available = true;
				break;
			}
			
			ping_iter = ping_next;
		}
	}
	
	if( found_available )
	{
		SentPings[ ping_id ].Reset();
		
		Packet ping( Raptor::Packet::PING );
		ping.AddUChar( ping_id );
		Send( &ping );
	}
}


double NetClient::LatestPing( void )
{
	return PingTimes.size() ? PingTimes.back() : 0.;
}


double NetClient::AveragePing( void )
{
	return PingTimes.size() ? Num::Avg(PingTimes) : 0.;
}


double NetClient::MedianPing( void )
{
	return PingTimes.size() ? Num::Med(PingTimes) : 0.;
}


std::string NetClient::Status( void )
{
	char cstr[ 1024 ] = "";
	#ifdef WIN32
		snprintf( cstr, 1024, "Bytes sent as client: %I64u\nBytes received as client: %I64u\nPing: %.0f", (unsigned long long) BytesSent, (unsigned long long) BytesReceived, MedianPing() );
	#else
		snprintf( cstr, 1024, "Bytes sent as client: %llu\nBytes received as client: %llu\nPing: %.0f", (unsigned long long) BytesSent, (unsigned long long) BytesReceived, MedianPing() );
	#endif
	return std::string(cstr);
}


// -----------------------------------------------------------------------------


int NetClientThread( void *client )
{
	NetClient *net_client = (NetClient *) client;
	char data[ PACKET_BUFFER_SIZE ] = "";
	PacketBuffer Buffer;
	
	while( net_client->Connected )
	{
		// Check for packets.
		int size = 0;
		if( ( size = SDLNet_TCP_Recv( net_client->Socket, data, PACKET_BUFFER_SIZE ) ) > 0 )
		{
			if( ! net_client->Connected )
				break;
			
			net_client->BytesReceived += size;
			Buffer.AddData( data, size );
			
			while( Packet *packet = Buffer.Pop() )
			{
				SDL_mutexP( net_client->Lock );
				net_client->InBuffer.push( packet );
				SDL_mutexV( net_client->Lock );
			}
		}
		else
		{
			// If 0 (disconnect) or -1 (error), stop listening.
			net_client->Disconnect();
			break;
		}
		
		// Let the thread rest a bit.
		SDL_Delay( 1 );
	}
	
	// Set the thread pointer to NULL when we disconnect.
	net_client->Thread = NULL;
	
	return 0;
}

