/*
 *  NetServer.cpp
 */

#include "NetServer.h"

#include <cstddef>
#include "RaptorDefs.h"
#include "RaptorServer.h"
#include "RaptorGame.h"


NetServer::NetServer( void )
{
	Initialized = false;
	Listening = false;
	Thread = NULL;
	Socket = NULL;
	NetRate = 30.;
	DisconnectTime = 30.;
	ResyncTime = 0.;  // Send RESYNC packet after this long without response.  Disabled because the updated netcode should no longer lose sync.
	Precision = 0;
}


NetServer::~NetServer()
{
	Disconnect();
	
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
	
	if( Initialized )
		SDLNet_Quit();
}


bool NetServer::Initialize( int port )
{
	if( ! port )
		port = Raptor::Server->Port;
	
	if( ! ( Initialized || Raptor::Game->Net.Initialized ) )
	{
		// Dedicated server initializes SDL_net here.
		if( SDLNet_Init() < 0 )
		{
			fprintf( stderr, "SDLNet_Init: %s\n", SDLNet_GetError() );
			return false;
		}
		
		Initialized = true;
	}
	
	// Resolving the host using NULL makes the network interface listen.
	IPaddress nonsense_ip;
	if( SDLNet_ResolveHost( &nonsense_ip, NULL, port ) < 0 )
	{
		fprintf( stderr, "SDLNet_ResolveHost: %s\n", SDLNet_GetError() );
		return false;
	}
 	
	// Open a connection with the "IP" provided (listen on the host's port).
	if( !( Socket = SDLNet_TCP_Open(&nonsense_ip) ) )
	{
		fprintf( stderr, "SDLNet_TCP_Open: %s\n", SDLNet_GetError() );
		return false;
	}
	
	// Start the listener thread.
	Listening = true;
	#if SDL_VERSION_ATLEAST(2,0,0)
		Thread = SDL_CreateThread( NetServerThread, "NetServer", this );
	#else
		Thread = SDL_CreateThread( NetServerThread, this );
	#endif
	if( ! Thread )
	{
		fprintf( stderr, "SDL_CreateThread: %s\n", SDLNet_GetError() );
		Listening = false;
		SDLNet_TCP_Close( Socket );
		return false;
	}
	
	return true;
}


void NetServer::DisconnectNice( const char *message )
{
	if( Listening )
	{
		Packet disconnect( Raptor::Packet::DISCONNECT );
		
		if( message )
			disconnect.AddString( message );
		else
			disconnect.AddString( "" );
		
		SendAll( &disconnect );
	}
	
	Disconnect();
}


void NetServer::Disconnect( void )
{
	Listening = false;
	
	// Sleep until the other thread has finished (max 3 sec).
	Clock wait_for_thread;
	while( Thread && (wait_for_thread.ElapsedSeconds() < 3.) )
		SDL_Delay( 1 );
	
	if( Thread )
	{
		#if SDL_VERSION_ATLEAST(2,0,0)
			SDL_DetachThread( Thread );
		#else
			SDL_KillThread( Thread );
		#endif
		Thread = NULL;
	}
	
	if( ! Lock.Lock() )
		fprintf( stderr, "NetServer::Disconnect: Lock.Lock: %s\n", SDL_GetError() );
	
	for( std::list<ConnectedClient*>::iterator iter = Clients.begin(); iter != Clients.end(); )
	{
		std::list<ConnectedClient*>::iterator next = iter;
		next ++;
		
		(*iter)->Disconnect();
		
		iter = next;
	}
	
	if( ! Lock.Unlock() )
		fprintf( stderr, "NetServer::Disconnect: Lock.Unlock: %s\n", SDL_GetError() );
	
	if( Socket )
	{
		SDLNet_TCP_Close( Socket );
		Socket = NULL;
	}
}


void NetServer::RemoveDisconnectedClients( void )
{
	if( ! Lock.Lock() )
		fprintf( stderr, "NetServer::RemoveDisconnectedClients: Lock.Lock: %s\n", SDL_GetError() );
	
	// Move disconnected clients to their own list.
	for( std::list<ConnectedClient*>::iterator iter = Clients.begin(); iter != Clients.end(); )
	{
		std::list<ConnectedClient*>::iterator next = iter;
		next ++;
		
		ConnectedClient *client = *iter;
		
		// Kick connections held for too long without login; these are probably not valid clients.
		if( client->Connected && ! client->Synchronized && (client->NetClock.ElapsedSeconds() >= 10.) )
			client->Disconnect();
		
		if( ! client->Connected )
		{
			DisconnectedClients.push_back( client );
			Clients.erase( iter );
		}
		
		iter = next;
	}
	
	// Clean up any disconnected clients whose threads have finished.
	for( std::list<ConnectedClient*>::iterator iter = DisconnectedClients.begin(); iter != DisconnectedClients.end(); )
	{
		std::list<ConnectedClient*>::iterator next = iter;
		next ++;
		
		ConnectedClient *client = *iter;
		
		if( client && (( (! client->InThread) && (! client->OutThread) && (client->ResyncClock.Progress() >= 1.) ) || (client->ResyncClock.ElapsedSeconds() >= 10.)) )
		{
			// Start the cleanup thread.
			#if SDL_VERSION_ATLEAST(2,0,0)
				client->CleanupThread = SDL_CreateThread( ConnectedClient::ConnectedClientCleanupThread, "ConnectedClientCleanup", client );
			#else
				client->CleanupThread = SDL_CreateThread( ConnectedClient::ConnectedClientCleanupThread, client );
			#endif
			if( ! client->CleanupThread )
			{
				fprintf( stderr, "NetServer::RemoveDisconnectedClients: SDL_CreateThread(ConnectedClientCleanupThread): %s\n", SDLNet_GetError() );
				delete client;
			}
			
			DisconnectedClients.erase( iter );
		}
		
		iter = next;
	}
	
	if( ! Lock.Unlock() )
		fprintf( stderr, "NetServer::RemoveDisconnectedClients: Lock.Unlock: %s\n", SDL_GetError() );
}


void NetServer::ProcessIn( void )
{
	if( ! Lock.Lock() )
		fprintf( stderr, "NetServer::ProcessIn: Lock.Lock: %s\n", SDL_GetError() );
	
	for( std::list<ConnectedClient*>::iterator iter = Clients.begin(); iter != Clients.end(); )
	{
		std::list<ConnectedClient*>::iterator next = iter;
		next ++;
		
		(*iter)->ProcessIn();
		
		iter = next;
	}
	
	if( ! Lock.Unlock() )
		fprintf( stderr, "NetServer::ProcessIn: Lock.Unlock: %s\n", SDL_GetError() );
}


void NetServer::ProcessTop( void )
{
	if( ! Lock.Lock() )
		fprintf( stderr, "NetServer::ProcessTop: Lock.Lock: %s\n", SDL_GetError() );
	
	for( std::list<ConnectedClient*>::iterator iter = Clients.begin(); iter != Clients.end(); )
	{
		std::list<ConnectedClient*>::iterator next = iter;
		next ++;
		
		(*iter)->ProcessTop();
		
		iter = next;
	}
	
	if( ! Lock.Unlock() )
		fprintf( stderr, "NetServer::ProcessTop: Lock.Unlock: %s\n", SDL_GetError() );
}


void NetServer::SendToPlayer( Packet *packet, uint32_t player_id )
{
	if( ! Lock.Lock() )
		fprintf( stderr, "NetServer::SendAll: Lock.Lock: %s\n", SDL_GetError() );
	
	for( std::list<ConnectedClient*>::iterator iter = Clients.begin(); iter != Clients.end(); )
	{
		std::list<ConnectedClient*>::iterator next = iter;
		next ++;
		
		if( (*iter)->PlayerID == player_id )
			(*iter)->Send( packet );
		
		iter = next;
	}
	
	if( ! Lock.Unlock() )
		fprintf( stderr, "NetServer::SendAll: Lock.Unlock: %s\n", SDL_GetError() );
}


void NetServer::SendAll( Packet *packet, bool send_to_unsynced )
{
	if( ! Lock.Lock() )
		fprintf( stderr, "NetServer::SendAll: Lock.Lock: %s\n", SDL_GetError() );
	
	for( std::list<ConnectedClient*>::iterator iter = Clients.begin(); iter != Clients.end(); )
	{
		std::list<ConnectedClient*>::iterator next = iter;
		next ++;
		
		if( (*iter)->Synchronized || send_to_unsynced )
			(*iter)->Send( packet );
		
		iter = next;
	}
	
	if( ! Lock.Unlock() )
		fprintf( stderr, "NetServer::SendAll: Lock.Unlock: %s\n", SDL_GetError() );
}


void NetServer::SendAllExcept( Packet *packet, ConnectedClient *except, bool send_to_unsynced )
{
	if( ! Lock.Lock() )
		fprintf( stderr, "NetServer::SendAllExcept: Lock.Lock: %s\n", SDL_GetError() );
	
	for( std::list<ConnectedClient*>::iterator iter = Clients.begin(); iter != Clients.end(); )
	{
		std::list<ConnectedClient*>::iterator next = iter;
		next ++;
		
		if( (*iter != except) && ((*iter)->Synchronized || send_to_unsynced) )
			(*iter)->Send( packet );
		
		iter = next;
	}
	
	if( ! Lock.Unlock() )
		fprintf( stderr, "NetServer::SendAllExcept: Lock.Unlock: %s\n", SDL_GetError() );
}


void NetServer::SendAllReconnect( uint8_t seconds_to_wait )
{
	Packet reconnect_packet( Raptor::Packet::RECONNECT );
	reconnect_packet.AddUChar( seconds_to_wait );
	SendAll( &reconnect_packet, true );
}


void NetServer::SendUpdates( void )
{
	if( ! Lock.Lock() )
		fprintf( stderr, "NetServer::SendUpdates: Lock.Lock: %s\n", SDL_GetError() );
	
	for( std::list<ConnectedClient*>::iterator iter = Clients.begin(); iter != Clients.end(); )
	{
		std::list<ConnectedClient*>::iterator next = iter;
		next ++;
		
		ConnectedClient *client = *iter;
		
		if( client->Connected && client->Synchronized )
		{
			// Reduce update rate temporarily in high-ping situations.
			double temp_netrate = client->NetRate;
			temp_netrate /= ((int) client->LatestPing() / 100) + 1;
			
			// Send an update if it's time to do so.
			double elapsed = client->NetClock.ElapsedSeconds();
			if( elapsed >= (1. / temp_netrate) )
			{
				client->NetClock.Advance( elapsed );
				Raptor::Server->SendUpdate( client );
				
				double ping_elapsed = client->PingClock.ElapsedSeconds();
				if( (ping_elapsed >= (1. / client->PingRate)) && client->SendPing() )
					client->PingClock.Advance( ping_elapsed );
				else if( DisconnectTime && (ping_elapsed >= DisconnectTime) )  // Kick clients that have lost connection.
					client->Disconnect();
				else if( ResyncTime && (ping_elapsed >= ResyncTime) )
				{
					double resync_elapsed = client->ResyncClock.ElapsedSeconds();
					if( resync_elapsed >= ResyncTime )
					{
						// Sometimes clients with flaky connections can receive data but not send replies; ask them to reconnect and resync.
						client->SendResync();
						client->ResyncClock.Advance( resync_elapsed );
					}
				}
			}
		}
		
		iter = next;
	}
	
	if( ! Lock.Unlock() )
		fprintf( stderr, "NetServer::SendUpdates: Lock.Unlock: %s\n", SDL_GetError() );
}


void NetServer::SetNetRate( double netrate )
{
	NetRate = netrate;
	
	if( ! Lock.Lock() )
		fprintf( stderr, "NetServer::SendUpdates: Lock.Lock: %s\n", SDL_GetError() );
	
	for( std::list<ConnectedClient*>::iterator iter = Clients.begin(); iter != Clients.end(); iter ++ )
		(*iter)->NetRate = NetRate;
	
	if( ! Lock.Unlock() )
		fprintf( stderr, "NetServer::SendUpdates: Lock.Unlock: %s\n", SDL_GetError() );
}


// -----------------------------------------------------------------------------


int NetServer::NetServerThread( void *server )
{
	NetServer *net_server = (NetServer*) server;
	TCPsocket client_socket;
	IPaddress *remote_ip;
	
	SDLNet_SocketSet socket_set = SDLNet_AllocSocketSet( 1 );
	SDLNet_TCP_AddSocket( socket_set, net_server->Socket );
	
	while( net_server->Listening )
	{
		// Avoid indefinite TCP blocking: https://libsdl.org/projects/old/SDL_net/docs/SDL_net_47.html
		SDLNet_CheckSockets( socket_set, 500 );
		if( ! SDLNet_SocketReady(net_server->Socket) )
		{
			SDL_Delay( 1 );
			continue;
		}
		
		// Check for new connections.
		if( (client_socket = SDLNet_TCP_Accept(net_server->Socket)) )
		{
			if( ! net_server->Lock.Lock() )
				fprintf( stderr, "NetServerThread: net_server->Lock.Lock: %s\n", SDL_GetError() );
			
			ConnectedClient *connected_client = new ConnectedClient( client_socket, net_server->NetRate, net_server->Precision );
			
			if( (remote_ip = SDLNet_TCP_GetPeerAddress(client_socket)) )
			{
				connected_client->IP = Endian::ReadBig32(&(remote_ip->host));
				connected_client->Port = Endian::ReadBig16(&(remote_ip->port));
				
				char cstr[ 1024 ] = "";
				snprintf( cstr, 1024, "Client connected: %i.%i.%i.%i:%i", (connected_client->IP & 0xFF000000) >> 24, (connected_client->IP & 0x00FF0000) >> 16, (connected_client->IP & 0x0000FF00) >> 8, connected_client->IP & 0x000000FF, connected_client->Port );
				Raptor::Server->ConsolePrint( cstr );
			}
			else
			{
				fprintf( stderr, "SDLNet_TCP_GetPeerAddress: %s\n", SDLNet_GetError() );
				Raptor::Server->ConsolePrint( "Client connected from unknown IP!\n" );
			}
			
			net_server->Clients.push_back( connected_client );
			
			if( ! net_server->Lock.Unlock() )
				fprintf( stderr, "NetServerThread: net_server->Lock.Unlock: %s\n", SDL_GetError() );
		}
		
		// Let the thread rest a bit.  This thread only handles new connections, so it can be a long delay.
		SDL_Delay( 100 );
	}
	
	// Set the thread pointer to NULL so we can delete the NetServer object.
	net_server->Thread = NULL;
	
	SDLNet_FreeSocketSet( socket_set );
	
	return 0;
}
