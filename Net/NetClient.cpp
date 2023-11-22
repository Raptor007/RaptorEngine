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
	NetRate = 30.;
	DisconnectTime = 30.;
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
		#if SDL_VERSION_ATLEAST(2,0,0)
			SDL_DetachThread( Thread );
		#else
			SDL_KillThread( Thread );
		#endif
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


bool NetClient::Initialize( double net_rate, int8_t precision )
{
	NetRate = net_rate;
	Precision = precision;
	
	// Initialize SDL_net.
	if( SDLNet_Init() < 0 )
	{
		fprintf( stderr, "SDLNet_Init: %s\n", SDLNet_GetError() );
		return false;
	}
	
	Initialized = true;
	return true;
}


bool NetClient::Connect( const char *host, const char *name, const char *password )
{
	bool return_value = false;
	
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


bool NetClient::Connect( const char *hostname, int port, const char *name, const char *password )
{
	Host = hostname;
	
	if( port > 0 )
		Port = port;
	else
		Port = Raptor::Game->DefaultPort;
	
	if( ! Initialized )
		return false;
	
	if( Connected )
		DisconnectNice();
	
	// Clean up old data and socket.
	SDL_Delay( 1 );
	Cleanup();
	DisconnectMessage.clear();
	
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
		return false;
	}
	
	// Open a connection with the IP provided.
	if( !( Socket = SDLNet_TCP_Open(&ip) ) )
	{
		Raptor::Game->Console.Print( "Failed to open socket to server.", TextConsole::MSG_ERROR );
		Raptor::Game->ChangeState( Raptor::State::DISCONNECTED );
		return false;
	}
	
	BytesSent = 0;
	BytesReceived = 0;
	
	// Start the listener thread.
	Connected = true;
	#if SDL_VERSION_ATLEAST(2,0,0)
		Thread = SDL_CreateThread( NetClientThread, "NetClient", this );
	#else
		Thread = SDL_CreateThread( NetClientThread, this );
	#endif
	if( ! Thread )
	{
		fprintf( stderr, "SDL_CreateThread: %s\n", SDLNet_GetError() );
		Connected = false;
		SDLNet_TCP_Close( Socket );
		Socket = NULL;
		Raptor::Game->ChangeState( Raptor::State::DISCONNECTED );
		return false;
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
	
	return true;
}


bool NetClient::Reconnect( void )
{
	if( ! ReconnectAttempts )
		ReconnectAttempts = 1;
	
	return Reconnect( Raptor::Game->Cfg.SettingAsString("name").c_str(), Raptor::Game->Cfg.SettingAsString("password").c_str() );
}


bool NetClient::Reconnect( const char *name, const char *password )
{
	// If we've never connected to anything, we cannot reconnect.
	if( Host.empty() )
	{
		ReconnectTime = 0;
		ReconnectAttempts = 0;
		return false;
	}
	
	if( ReconnectAttempts )
	{
		// Reset the time-to-reconnect clock and subtract one attempt.
		ReconnectClock.Reset();
		ReconnectAttempts --;
		
		// Attempt to reconnect.
		return Connect( Host.c_str(), Port, name, password );
	}
	
	return false;
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
	DisconnectMessage.clear();
	Disconnect();
}


void NetClient::Disconnect( void )
{
	if( Connected )
	{
		if( DisconnectMessage.length() )
			Raptor::Game->Console.Print( std::string("Disconnected from server: ") + DisconnectMessage );
		else
			Raptor::Game->Console.Print( "Disconnected from server." );
	}
	
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
		
		// Make sure the old thread is done before we do anything else (like reconnect).
		if( Thread )
			SDL_WaitThread( Thread, NULL );
		
		if( Socket && ! Thread )
		{
			// If the connection is open, close it.
			SDLNet_TCP_Close( Socket );
			Socket = NULL;
		}
		
		// Empty the incoming packet buffer.
		ClearPackets();
		
		PingTimes.clear();
		SentPings.clear();
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
	
	else if( type == Raptor::Packet::RESYNC )
	{
		// The server hasn't received any of our responses for a while; reconnect with same PlayerID and state.
		uint16_t player_id = packet->NextUShort();
		
		int state = Raptor::Game->State;
		if( state < Raptor::State::CONNECTED )
			state = Raptor::State::CONNECTED;
		
		bool reconnected = Raptor::Server->IsRunning();
		if( (! reconnected) && (ReconnectClock.ElapsedSeconds() > 2.) )
		{
			Connected = false; // Prevent DisconnectNice in Connect, so the server keeps our Player.
			reconnected = Reconnect();
		}
		
		if( reconnected )
		{
			Packet resync( Raptor::Packet::RESYNC );
			resync.AddUShort( player_id );
			resync.AddInt( state );
			Send( &resync );
			// Server should respond with LOGIN packet to update PlayerID and restore state.
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
		int state = packet->Remaining() ? packet->NextInt() : Raptor::State::CONNECTED;
		
		if( Raptor::Game->State < Raptor::State::CONNECTING )
			DisconnectNice();
		else if( Raptor::Game->State != state )
			Raptor::Game->ChangeState( state );
	}
	
	else if( type == Raptor::Packet::DISCONNECT )
	{
		DisconnectMessage = packet->NextString();
		Disconnect();
	}
	
	else if( type == Raptor::Packet::RECONNECT )
	{
		uint8_t time = packet->NextUChar();
		
		DisconnectMessage.clear();
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


bool NetClient::Send( Packet *packet )
{
	if( ! Initialized )
		return false;
	if( ! Connected )
		return false;
	
	int sent = 0;
	int retries = 1;
	
	while( sent < (int) packet->Size() )
	{
		int sent_now = SDLNet_TCP_Send( Socket, ((uint8_t*)( packet->Data )) + sent, packet->Size() - sent );
		if( sent_now > 0 )
		{
			sent      += sent_now;
			BytesSent += sent_now;
			retries = 1;
		}
		else if( retries )
			retries --;
		else
		{
			DisconnectMessage = std::string("SDLNet_TCP_Send: ") + std::string(SDLNet_GetError());
			Disconnect();
			return false;
		}
	}
	
	return true;
}


void NetClient::SendUpdates( void )
{
	if( !( Connected && Raptor::Game->PlayerID ) )
		return;
	
	// Reduce update rate temporarily in high-ping situations.
	double temp_netrate = NetRate;
	temp_netrate /= ((int) LatestPing() / 100) + 1;
	
	// Send an update if it's time to do so.
	double elapsed = NetClock.ElapsedSeconds();
	if( elapsed >= (1. / temp_netrate) )
	{
		NetClock.Advance( elapsed );
		SendUpdate();
		
		double ping_elapsed = PingClock.ElapsedSeconds();
		if( (ping_elapsed >= (1. / PingRate)) && SendPing() )
			PingClock.Advance( ping_elapsed );
		else if( DisconnectTime && (ping_elapsed >= DisconnectTime) )  // Disconnect if we have lost connection.
			Disconnect();
	}
}


void NetClient::SendUpdate( void )
{
	Raptor::Game->SendUpdate( Precision );
}


bool NetClient::SendPing( void )
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
	
	if( found_available )
	{
		SentPings[ ping_id ].Reset();
		
		Packet ping( Raptor::Packet::PING );
		ping.AddUChar( ping_id );
		Send( &ping );
	}
	
	return found_available;
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
	int retries = 3;
	
	while( net_client->Connected )
	{
		// Check for packets.
		int size = SDLNet_TCP_Recv( net_client->Socket, data, PACKET_BUFFER_SIZE );
		if( size > 0 )
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
			
			retries = 3;
		}
		else if( (size < 0) && retries )
			retries --;
		else
		{
			// If 0 (disconnect) or multiple -1 (errors), stop listening.
			if( size < 0 )
				net_client->DisconnectMessage = std::string("SDLNet_TCP_Recv: ") + std::string(SDLNet_GetError());
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

