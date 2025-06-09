/*
 *  RaptorServer.cpp
 */

#define GAMESERVER_CPP

#include "RaptorServer.h"
#include "RaptorGame.h"

#include <cstddef>
#include <cmath>
#include <string>
#include <map>
#include <signal.h>

#include "RaptorDefs.h"
#include "NetServer.h"
#include "NetUDP.h"
#include "Clock.h"
#include "Rand.h"
#include "Str.h"
#include "Num.h"
#include "IMA.h"


namespace Raptor
{
	// Global pointer to the game server object.
	RaptorServer *Server = NULL;
}


RaptorServer::RaptorServer( std::string game, std::string version )
{
	Game = game;
	Version = version;
	
	Thread = NULL;
	Port = 7000;
	MaxFPS = 60.;
	NetRate = 30.;
	Announce = true;
	AnnouncePort = 7000;
	AnnounceInterval = 3.;
	
	Console = NULL;
	
	FrameTime = 0.;
	State = Raptor::State::DISCONNECTED;
}


RaptorServer::~RaptorServer()
{
	if( IsRunning() )
		StopAndWait( 5. );
	
	if( Thread )
	{
		#if SDL_VERSION_ATLEAST(2,0,0)
			SDL_DetachThread( Thread );
		#else
			// FIXME: This is causing crashing sometimes!
			//        Probably when SDLNet_Quit() has already been called.
			SDL_KillThread( Thread );
		#endif
		Thread = NULL;
	}
}


bool RaptorServer::Start( std::string name )
{
	Data.Properties["name"] = name;
	
	if( IsRunning() )
	{
		Net.SendAllReconnect();
		StopAndWait();
		SDL_Delay( 200 );
	}
	
	Net.NetRate = NetRate;
	
	State = Raptor::State::CONNECTING;
	
	#if SDL_VERSION_ATLEAST(2,0,0)
		Thread = SDL_CreateThread( RaptorServerThread, "RaptorServer", this );
	#else
		Thread = SDL_CreateThread( RaptorServerThread, this );
	#endif
	
	if( ! Thread )
	{
		State = Raptor::State::DISCONNECTED;
		fprintf( stderr, "SDL_CreateThread: %s\n", SDLNet_GetError() );
		return false;
	}
	
	Clock start_time;
	while( (State == Raptor::State::CONNECTING) && (start_time.ElapsedSeconds() < 3.) )
		SDL_Delay( 100 );
	
	Data.GameTime.Reset();
	
	return true;
}


void RaptorServer::StopAndWait( double max_wait_seconds )
{
	Net.DisconnectNice( NULL );
	
	Clock wait_for_stop;
	while( IsRunning() )
	{
		if( (max_wait_seconds > 0) && (wait_for_stop.ElapsedSeconds() > max_wait_seconds) )
			break;
		SDL_Delay( 1 );
	}
	
	Data.Clear();
	Stopped();
}


bool RaptorServer::IsRunning( void )
{
	return (State >= Raptor::State::CONNECTING);
}


// ---------------------------------------------------------------------------


void RaptorServer::ConsolePrint( std::string text, uint32_t type )
{
	if( Console )
		Console->Print( text, type );
}


// ---------------------------------------------------------------------------


void RaptorServer::Started( void )
{
}


void RaptorServer::Stopped( void )
{
}


void RaptorServer::Update( double dt )
{
	Data.CheckCollisions( dt );
	Data.Update( dt );
}


bool RaptorServer::HandleCommand( std::string cmd, std::vector<std::string> *params )
{
	return false;
}


bool RaptorServer::ProcessPacket( Packet *packet, ConnectedClient *from_client )
{
	PacketType type = packet->Type();
	
	if( type == Raptor::Packet::UPDATE )
	{
		// First read the precision of this update packet.
		int8_t precision = packet->NextChar();
		
		// Then read the number of objects.
		uint32_t obj_count = packet->NextUInt();
		
		// Loop through for each object's update data.
		while( obj_count )
		{
			obj_count --;
			
			// First read the ID.
			uint32_t obj_id = packet->NextUInt();
			
			// Look up the ID in the server-side list of objects and update it.
			std::map<uint32_t,GameObject*>::iterator obj_iter = Data.GameObjects.find( obj_id );
			if( obj_iter != Data.GameObjects.end() )
			{
				// FIXME: Make sure the client is authorized to update this object?
				obj_iter->second->ReadFromUpdatePacketFromClient( packet, precision );
			}
			else
			{
				// The client thinks we should have this object, so we're out of sync!
				// This means the rest of the update packet could be misaligned, so stop trying to parse it.
				//ConsolePrint( "Sync error: UPDATE", TextConsole::MSG_ERROR );
				return true;
			}
		}
		
		return true;
	}
	
	else if( type == Raptor::Packet::VOICE )
	{
		uint16_t player_id   = packet->NextUShort();
		/*uint32_t object_id = */ packet->NextUInt();
		uint8_t  channel     = packet->NextUChar() & 0x7F;
		uint32_t sample_rate = packet->NextUInt();
		uint32_t samples     = packet->NextUInt();
		
		if( (samples > 0x00080000) || ! (sample_rate && samples) )
			return true;
		
		const Player *from_player = from_client ? Data.GetPlayer( from_client->PlayerID ) : NULL;
		if( (! from_player) || (from_player->ID != player_id) )
			return true;
		
		std::string match_team = (channel == Raptor::VoiceChannel::TEAM) ? from_player->PropertyAsString("team") : "";
		
		for( std::map<uint16_t,Player*>::const_iterator player_iter = Data.Players.begin(); player_iter != Data.Players.end(); player_iter ++ )
		{
			if( (player_iter->second == from_player) && ! Data.PropertyAsBool("echo") )
				continue;
			if( match_team.length() && (player_iter->second->PropertyAsString("team") != match_team) )
				continue;
			Net.SendToPlayer( packet, player_iter->first );
		}
		
		return true;
	}
	
	else if( type == Raptor::Packet::PLAYER_PROPERTIES )
	{
		uint16_t id = packet->NextUShort();
		Player *player = Data.GetPlayer( id );
		// FIXME: Make sure the sending client is authorized to update this player?
		
		uint32_t property_count = packet->NextUInt();
		
		while( property_count )
		{
			property_count --;
			
			std::string property_name = packet->NextString();
			std::string property_value = packet->NextString();
			
			// Set property and retransmit if changed (unless intercepted by game-specific code).
			SetPlayerProperty( player, property_name, property_value );
		}
		
		return true;
	}
	
	else if( type == Raptor::Packet::INFO )
	{
		// First read the number of properties.
		uint16_t property_count = packet->NextUShort();
		
		// Loop through for each name/value pair.
		for( int i = 0; i < property_count; i ++ )
		{
			std::string name = packet->NextString();
			std::string value = packet->NextString();
			
			// FIXME: Make sure the client is authorized to update this data?
			SetProperty( name, value );
		}
		
		// Lazy solution: Just resend this packet back to everyone.
		// FIXME: Should SetProperty do this instead?
		Net.SendAll( packet );
		
		return true;
	}
	
	else if( type == Raptor::Packet::MESSAGE )
	{
		std::string msg = packet->NextString();
		uint32_t msg_type = 0;
		if( packet->Remaining() )
			msg_type = packet->NextUInt();
		
		if( msg_type == TextConsole::MSG_TEAM )
		{
			const Player *from_player = from_client ? Data.GetPlayer( from_client->PlayerID ) : NULL;
			std::string match_team = from_player ? from_player->PropertyAsString("team") : "";
			if( match_team.empty() )
			{
				// Convert to regular chat when not on a team.
				Packet message( Raptor::Packet::MESSAGE );
				message.AddString( msg );
				message.AddUInt( TextConsole::MSG_CHAT );
				Net.SendAll( &message );
			}
			else
			{
				// Send only to players on the same team.
				for( std::map<uint16_t,Player*>::const_iterator player_iter = Data.Players.begin(); player_iter != Data.Players.end(); player_iter ++ )
				{
					if( player_iter->second->PropertyAsString("team") == match_team )
						Net.SendToPlayer( packet, player_iter->first );
				}
			}
		}
		else
			// Lazy solution: Just resend this packet back to everyone.
			Net.SendAll( packet );
		
		// Dedicated servers print chat to console.
		if( Console != &(Raptor::Game->Console) )
			ConsolePrint( msg, msg_type );
	}
	
	return false;
}


bool RaptorServer::CompatibleVersion( std::string version ) const
{
	// FIXME: There are more efficient ways to get the first word (but it's probably not worth the effort).
	std::vector<std::string> client_version_words = Str::SplitToVector( version, " " );
	std::vector<std::string> server_version_words = Str::SplitToVector( Version, " " );
	return (client_version_words[ 0 ] == server_version_words[ 0 ]);
}


bool RaptorServer::ValidateLogin( std::string name, std::string password )
{
	return true;
}


void RaptorServer::AcceptedClient( ConnectedClient *client )
{
	if( ! client )
		return;
	
	Player *player = Data.GetPlayer( client->PlayerID );
	
	Raptor::Server->Data.Lock.Lock();
	
	// Send list of server properties to the new client.
	Packet info( Raptor::Packet::INFO );
	info.AddUShort( Data.Properties.size() );
	for( std::map<std::string,std::string>::iterator property_iter = Data.Properties.begin(); property_iter != Data.Properties.end(); property_iter ++ )
	{
		info.AddString( property_iter->first.c_str() );
		info.AddString( property_iter->second.c_str() );
	}
	client->Send( &info );
	
	// Send list of all players to the new client.
	Packet player_list( Raptor::Packet::PLAYER_LIST );
	player_list.AddUShort( Raptor::Server->Data.Players.size() );
	for( std::map<uint16_t,Player*>::iterator player_iter = Raptor::Server->Data.Players.begin(); player_iter != Raptor::Server->Data.Players.end(); player_iter ++ )
	{
		player_list.AddUShort( player_iter->second->ID );
		player_list.AddString( player_iter->second->Name );
	}
	client->Send( &player_list );
	
	// Send all existing players' properties to the new client.
	for( std::map<uint16_t,Player*>::iterator player_iter = Raptor::Server->Data.Players.begin(); player_iter != Raptor::Server->Data.Players.end(); player_iter ++ )
	{
		Packet player_properties( Raptor::Packet::PLAYER_PROPERTIES );
		player_properties.AddUShort( player_iter->second->ID );
		player_properties.AddUInt( player_iter->second->Properties.size() );
		for( std::map<std::string,std::string>::iterator property_iter = player_iter->second->Properties.begin(); property_iter != player_iter->second->Properties.end(); property_iter ++ )
		{
			player_properties.AddString( property_iter->first.c_str() );
			player_properties.AddString( property_iter->second.c_str() );
		}
		client->Send( &player_properties );
	}
	
	// Tell other clients about the new player.
	Packet player_add( Raptor::Packet::PLAYER_ADD );
	player_add.AddUShort( client->PlayerID );
	player_add.AddString( player ? player->Name.c_str() : "" );
	Net.SendAllExcept( &player_add, client );
	
	// Tell other clients about the new player's properties.
	if( player )
	{
		Packet player_properties( Raptor::Packet::PLAYER_PROPERTIES );
		player_properties.AddUShort( client->PlayerID );
		player_properties.AddUInt( player->Properties.size() );
		for( std::map<std::string,std::string>::iterator property_iter = player->Properties.begin(); property_iter != player->Properties.end(); property_iter ++ )
		{
			player_properties.AddString( property_iter->first.c_str() );
			player_properties.AddString( property_iter->second.c_str() );
		}
		Net.SendAllExcept( &player_properties, client );
	}
	
	// Send list of existing objects to the new client.
	std::vector<GameObject*> objects_to_send;
	for( std::map<uint32_t,GameObject*>::iterator obj_iter = Data.GameObjects.begin(); obj_iter != Data.GameObjects.end(); obj_iter ++ )
	{
		if( obj_iter->second->ServerShouldSend() )
			objects_to_send.push_back( obj_iter->second );
	}
	Packet obj_list = Packet( Raptor::Packet::OBJECTS_ADD );
	obj_list.AddUInt( objects_to_send.size() );
	for( std::vector<GameObject*>::iterator obj_iter = objects_to_send.begin(); obj_iter != objects_to_send.end(); obj_iter ++ )
	{
		obj_list.AddUInt( (*obj_iter)->ID );
		obj_list.AddUInt( (*obj_iter)->Type() );
		(*obj_iter)->AddToInitPacket( &obj_list );
	}
	client->Send( &obj_list );
	
	// The client is now synchronized, so it should receive updates.
	client->Synchronized = true;
	client->NetClock.Reset();
	
	if( player ) // FIXME: Don't display this message on resync.
	{
		// Tell other players to display a message about the new player.
		Packet message( Raptor::Packet::MESSAGE );
		std::string join_message = player->Name + std::string(" has joined the game (v") + client->Version + std::string(").");
		message.AddString( join_message.c_str() );
		if( (Raptor::Game->State >= Raptor::State::CONNECTING) && (((client->IP & 0xFF000000) >> 24) == 127) )
		{
			// Self-hosted game prints localhost join messages to console, but not to messages.
			ConsolePrint( join_message );
			Net.SendAllExcept( &message, client );
		}
		else
		{
			// Show join messages on dedicated server console (player hosted servers will see the message packet).
			if( Console != &(Raptor::Game->Console) )
				ConsolePrint( join_message );
			Net.SendAll( &message );
		}
	}
	
	Raptor::Server->Data.Lock.Unlock();
}


void RaptorServer::DroppedClient( ConnectedClient *client )
{
	client->Synchronized = false;
	
	// Before removing player, make sure they haven't resynced.
	Net.Lock.Lock();
	for( std::list<ConnectedClient*>::const_iterator client_iter = Net.Clients.begin(); client_iter != Net.Clients.end(); client_iter ++ )
	{
		if( (*client_iter)->Connected && ((*client_iter)->PlayerID == client->DropPlayerID) )
		{
			Net.Lock.Unlock();
			return;
		}
	}
	Net.Lock.Unlock();
	
	Player *player = Data.GetPlayer( client->DropPlayerID );
	
	// Tell other clients about the removed player.
	Packet player_remove( Raptor::Packet::PLAYER_REMOVE );
	player_remove.AddUShort( client->DropPlayerID );
	Net.SendAllExcept( &player_remove, client );
	
	if( player )
	{
		// Show leave messages on dedicated server console (player hosted servers will see the message packet).
		std::string leave_message = player->Name + std::string(" has left the game.");
		if( Console != &(Raptor::Game->Console) )
			ConsolePrint( leave_message );
		
		// Tell other players to display a message about the removed player.
		Packet message( Raptor::Packet::MESSAGE );
		message.AddString( leave_message.c_str() );
		Net.SendAllExcept( &message, client );
	}
	
	player = NULL;
	Data.RemovePlayer( client->DropPlayerID );
}


void RaptorServer::SendUpdate( ConnectedClient *client )
{
	// Don't attempt to update clients that haven't received the list of objects yet.
	if( ! client->Synchronized )
		return;
	
	std::vector<GameObject*> objects_to_update;
	
	// Before adding anything to the packet, count how many objects we will be sending data for.
	for( std::map<uint32_t,GameObject*>::iterator obj_iter = Data.GameObjects.begin(); obj_iter != Data.GameObjects.end(); obj_iter ++ )
	{
		if( client->PlayerID && (client->PlayerID == obj_iter->second->PlayerID) )
		{
			if( obj_iter->second->ServerShouldUpdatePlayer() )
				objects_to_update.push_back( obj_iter->second );
		}
		else if( obj_iter->second->ServerShouldUpdateOthers() )
			objects_to_update.push_back( obj_iter->second );
	}
	
	int8_t precision = client->Precision;
	
	// If precision is auto (-128), the number of objects to update dictates how much detail to send about each.
	if( precision == -128 )
	{
		if( objects_to_update.size() < 32 )
			precision = 1;
		else if( objects_to_update.size() < 64 )
			precision = 0;
		else
			precision = -1;
	}
	
	Packet update_packet = Packet( Raptor::Packet::UPDATE );
	
	// First specify the update precision.
	update_packet.AddChar( precision );
	
	// Then add the number of objects.
	update_packet.AddUInt( objects_to_update.size() );
	
	for( std::vector<GameObject*>::iterator obj_iter = objects_to_update.begin(); obj_iter != objects_to_update.end(); obj_iter ++ )
	{
		// Add each object ID and then its specific update data.
		update_packet.AddUInt( (*obj_iter)->ID );
		(*obj_iter)->AddToUpdatePacketFromServer( &update_packet, precision );
	}
	
	// Send the packet.
	client->Send( &update_packet );
}


bool RaptorServer::SetPlayerProperty( Player *player, std::string name, std::string value, bool force )
{
	if( ! player )
		return false;
	
	bool is_name = Str::EqualsInsensitive( name, "name" );
	if( force || ((is_name ? player->Name : player->Properties[ name ]) != value) )
	{
		if( is_name )
			player->Name = value;
		else
			player->Properties[ name ] = value;
		
		Packet player_properties( Raptor::Packet::PLAYER_PROPERTIES );
		player_properties.AddUShort( player->ID );
		player_properties.AddUInt( 1 );
		player_properties.AddString( name );
		player_properties.AddString( value );
		Net.SendAll( &player_properties );
		
		return true;
	}
	
	return false;
}


// ---------------------------------------------------------------------------


void RaptorServer::ChangeState( int state )
{
	State = state;
}


void RaptorServer::SetProperty( std::string name, std::string value )
{
	Data.SetProperty( name, value );
}


// ---------------------------------------------------------------------------


int RaptorServer::RaptorServerThread( void *game_server )
{
	RaptorServer *server = (RaptorServer*) game_server;
	Rand::Seed( time(NULL) );
	
	#ifdef WIN32
		SYSTEM_INFO system_info;
		GetSystemInfo( &system_info );
		if( system_info.dwNumberOfProcessors >= 3 )
			SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL );
	#endif
	
	server->Net.Initialize( server->Port );
	
	char cstr[ 1024 ] = "";
	
	if( server->Net.Listening )
	{
		server->Started();
		if( server->State == Raptor::State::CONNECTING )
			server->State = Raptor::State::CONNECTED;
		
		snprintf( cstr, 1024, "%s server started on port %i.", server->Game.c_str(), server->Port );
		server->ConsolePrint( cstr );
		
		NetUDP ServerAnnouncer;
		ServerAnnouncer.Initialize();
		
		Clock GameClock;
		Clock AnnounceClock;
		bool sleep_longer = false;
		
		while( server->Net.Listening )
		{
			// Process network input buffers.
			server->Net.ProcessIn();
			
			// Calculate the time elapsed for the "frame".
			double elapsed = GameClock.ElapsedSeconds();
			if( server->MaxFPS && (elapsed > 0.) && (elapsed < 1. / server->MaxFPS) )
				elapsed = 1. / server->MaxFPS;
			
			if( elapsed > 0. )
			{
				server->FrameTime = elapsed;
				GameClock.Advance( elapsed );
				
				// Update location.
				server->Update( server->FrameTime );
				
				// Drop disconnected clients from the list.
				server->Net.RemoveDisconnectedClients();
				
				// Send periodic updates to clients.
				server->Net.SendUpdates();
				
				// Send periodic server announcements over UDP broadcast.
				if( server->Announce && server->AnnouncePort
				&& (AnnounceClock.ElapsedSeconds() > server->AnnounceInterval) )
				{
					AnnounceClock.Reset();
					
					Packet info( Raptor::Packet::INFO );
					
					server->Data.Lock.Lock();
					
					// Properties.
					info.AddUShort( 5 + server->Data.Properties.size() );
					info.AddString( "game" );
					info.AddString( server->Game );
					info.AddString( "version" );
					info.AddString( server->Version );
					info.AddString( "port" );
					info.AddString( Num::ToString( server->Port ) );
					info.AddString( "state" );
					info.AddString( Num::ToString( server->State ) );
					info.AddString( "uptime" );
					info.AddString( Num::ToString( server->Data.GameTime.ElapsedSeconds(), 1 ) );
					for( std::map<std::string,std::string>::iterator property_iter = server->Data.Properties.begin(); property_iter != server->Data.Properties.end(); property_iter ++ )
					{
						info.AddString( property_iter->first.c_str() );
						info.AddString( property_iter->second.c_str() );
					}
					
					// Players.
					info.AddUShort( server->Data.Players.size() );
					for( std::map<uint16_t,Player*>::iterator player_iter = server->Data.Players.begin(); player_iter != server->Data.Players.end(); player_iter ++ )
						info.AddString( player_iter->second->Name.c_str() );
					
					server->Data.Lock.Unlock();
					
					ServerAnnouncer.Broadcast( &info, server->AnnouncePort );
				}
				
				// Don't work very hard if nobody is connected.
				server->Net.Lock.Lock();
				sleep_longer = (server->Net.Clients.size() < 1);
				server->Net.Lock.Unlock();
			}
			
			// Let the thread rest a bit.
			SDL_Delay( sleep_longer ? 100 : 1 );
		}
		
		snprintf( cstr, 1024, "%s server stopped.", server->Game.c_str() );
		server->ConsolePrint( cstr );
	}
	else
	{
		snprintf( cstr, 1024, "%s server failed to start.", server->Game.c_str() );
		server->ConsolePrint( cstr, TextConsole::MSG_ERROR );
	}
	
	server->Thread = NULL;
	server->State = Raptor::State::DISCONNECTED;
	return 0;
}
