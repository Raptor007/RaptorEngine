/*
 *  Screensaver.cpp
 */

#include "Screensaver.h"

#include "RaptorGame.h"
#include "Num.h"


Screensaver::Screensaver( void )
{
	Rect.x = 0;
	Rect.y = 0;
	Rect.w = Raptor::Game->Gfx.W;
	Rect.h = Raptor::Game->Gfx.H;
	
	MouseMoves = 0;
}


Screensaver::~Screensaver()
{
	ServerFinder.StopListening();
}


bool Screensaver::HandleEvent( SDL_Event *event )
{
#ifndef DEBUG
	if( Raptor::Game->Console.IsActive() )
		return false;
	
	if( event->type == SDL_MOUSEMOTION )
		MouseMoved.Reset();
	else if( MouseMoved.ElapsedSeconds() > 10. )
		MouseMoves = 0;
	
	if( ((event->type == SDL_MOUSEMOTION) && (++MouseMoves >= 2)) || (event->type == SDL_MOUSEBUTTONDOWN) || ((event->type == SDL_KEYDOWN) && (IgnoreKeys.find( event->key.keysym.sym ) == IgnoreKeys.end())) )
	{
		Raptor::Game->Quit();
		return true;
	}
#endif
	
	return false;
}


void Screensaver::Draw( void )
{
	Layer::Draw();
	
	// Everything below here is for Screensaver Connect, so check if it's enabled.
	if( ! Raptor::Game->Cfg.SettingAsBool("screensaver_connect") )
	{
		if( ServerFinder.Listening )
			ServerFinder.StopListening();
		return;
	}
	
	// Start the UDP listener after a short delay.
	if( (! ServerFinder.Listening) && (MouseMoved.ElapsedSeconds() > Raptor::Server->AnnounceInterval) )
	{
		MouseMoved.Reset();
		ServerFinder.StartListening( Raptor::Server->AnnouncePort );
		if( ServerFinder.Listening )
			Raptor::Game->Console.Print( std::string("Screensaver Connect is listening on UDP port ") + Num::ToString(Raptor::Server->AnnouncePort) + std::string(".") );
		else
			Raptor::Game->Console.Print( std::string("Screensaver Connect could not open UDP port ") + Num::ToString(Raptor::Server->AnnouncePort) + std::string("!"), TextConsole::MSG_ERROR );
	}
	
	// Look for server announcements.
	if( ServerFinder.Listening )
	{
		NetUDPPacket *packet = ServerFinder.GetPacket();
		if( packet )
		{
			if( packet->Type() == Raptor::Packet::INFO )
			{
				std::map<std::string,std::string> properties;
				uint16_t property_count = packet->NextUShort();
				for( int i = 0; i < property_count; i ++ )
				{
					std::string name  = packet->NextString();
					std::string value = packet->NextString();
					properties[ name ] = value;
				}
				
				int real_players = 0, screensaver_players = 0;
				uint16_t player_count = packet->NextUShort();
				for( int i = 0; i < player_count; i ++ )
				{
					if( Str::ContainsInsensitive( packet->NextString(), "Screensaver" ) )
						screensaver_players ++;
					else
						real_players ++;
				}
				
				char host_str[ 128 ] = "";
				int host_ip = Endian::ReadBig32( &(packet->IP) );
				snprintf( host_str, 128, "%i.%i.%i.%i:%s", (host_ip & 0xFF000000) >> 24, (host_ip & 0x00FF0000) >> 16, (host_ip & 0x0000FF00) >> 8, host_ip & 0x000000FF, properties["port"].c_str() );
				
				if( (properties["game"] == Raptor::Game->Game)                                                                     // Make sure they are announcing the same game.
				&&  Raptor::Server->CompatibleVersion( properties["version"] )                                                     // Make sure the version seems compatible.
				&&  (std::string(host_str) != (Raptor::Game->Net.Host + std::string(":") + Num::ToString(Raptor::Game->Net.Port))) // Make sure we're not already connected to this server.
				&&  ((properties["name"] != Raptor::Server->Data.PropertyAsString("name")) || ! Raptor::Server->IsRunning())       // Make sure it isn't our own server.
				&&  ((Str::AsInt(properties["state"]) >= Raptor::Game->State) || properties["state"].empty())                      // Make sure the state is gameplay (not lobby).
				&&  (properties["campaign"].empty() || (Raptor::Game->Cfg.SettingAsInt("screensaver_connect") >= 2)) )             // Typically do not spectate singleplayer campaigns.
				{
					if( FoundServers.empty() )
						MouseMoved.Reset();  // Make sure we wait to see all servers.
					
					FoundServers[ host_str ].Players      = real_players;
					FoundServers[ host_str ].Screensavers = screensaver_players;
					FoundServers[ host_str ].SSConnect    = Str::AsBool( properties["screensaver_connect"] );
					FoundServers[ host_str ].Uptime       = Str::AsDouble( properties["uptime"] );
				}
				else
				{
					std::map<std::string,ScreensaverFoundServer>::iterator server_iter = FoundServers.find( host_str );
					if( server_iter != FoundServers.end() )
						FoundServers.erase( server_iter );
				}
			}
			
			delete packet;
		}
	}
	
	// Decide if we should connect to a different server.
	if( FoundServers.size()
	&& (MouseMoved.ElapsedSeconds() > (Raptor::Server->AnnounceInterval * 1.5))         // Wait until we've had time to see what's out there.
	&& ((Raptor::Server->Data.Players.size() <= 1) || ! Raptor::Server->IsRunning()) )  // Don't join another server if we are hosting anyone else.
	{
		std::string best_server;
		float best_uptime = Raptor::Game->Data.GameTime.ElapsedSeconds();
		bool best_ssconnect = true;
		int best_players = 0, best_screensavers = 0;
		for( std::map<uint16_t,Player*>::const_iterator player_iter = Raptor::Game->Data.Players.begin(); player_iter != Raptor::Game->Data.Players.end(); player_iter ++ )
		{
			if( Str::ContainsInsensitive( player_iter->second->Name, "Screensaver" ) )
				best_screensavers ++;
			else
				best_players ++;
		}
		
		// Prioritize the server with the most real players, then most screensavers, then not using screensaver_connect (more likely to stay up), and finally highest uptime.
		for( std::map<std::string,ScreensaverFoundServer>::const_iterator server_iter = FoundServers.begin(); server_iter != FoundServers.end(); server_iter ++ )
		{
			if( (server_iter->second.Players > best_players)
			|| ((server_iter->second.Players == best_players) && (server_iter->second.Screensavers > best_screensavers))
			|| ((server_iter->second.Players == best_players) && (server_iter->second.Screensavers == best_screensavers) && best_ssconnect && ! server_iter->second.SSConnect)
			|| ((server_iter->second.Players == best_players) && (server_iter->second.Screensavers == best_screensavers) && (best_ssconnect == server_iter->second.SSConnect) && (server_iter->second.Uptime > best_uptime)) )
			{
				best_server       = server_iter->first;
				best_players      = server_iter->second.Players;
				best_screensavers = server_iter->second.Screensavers;
				best_ssconnect    = server_iter->second.SSConnect;
				best_uptime       = server_iter->second.Uptime;
			}
		}
		
		if( ! best_server.empty() )
		{
			MouseMoved.Reset();
			FoundServers.clear();
			
			std::string join_message;
			if( best_players )
				join_message = std::string("Screensaver Connect found ") + Num::ToString(best_players) + std::string((best_players == 1) ? " player to spectate" : " players to spectate");
			else
				join_message = std::string("Screensaver Connect is joining another screensaver");
			Raptor::Game->MessageReceived( join_message + std::string(" at: ") + best_server, TextConsole::MSG_CHAT );
			
			Raptor::Game->Net.ConnectWhenSafe( best_server, join_message + std::string("...") );
		}
	}
}
