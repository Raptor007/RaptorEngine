/*
 *  ClientConfig.cpp
 */

#include "ClientConfig.h"

#include <cstring>
#include <cmath>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include "Str.h"
#include "Num.h"
#include "RaptorGame.h"

#include "Math3D.h"
#include <cfloat>


#define ORIGINAL "\"/\n\r\t"
#define ESCAPED  "\"/nrt"


ClientConfig::ClientConfig( void )
{
}


ClientConfig::~ClientConfig()
{
}


// ---------------------------------------------------------------------------


void ClientConfig::SetDefaults( void )
{
	// Remove any existing settings.
	Settings.clear();
	
	
	// Default client settings.
	
	Settings[ "g_res_fullscreen_x" ] = Num::ToString( Raptor::Game->Gfx.DesktopW );
	Settings[ "g_res_fullscreen_y" ] = Num::ToString( Raptor::Game->Gfx.DesktopH );
	Settings[ "g_res_windowed_x" ] = (Raptor::Game->Gfx.DesktopH > 1080) ? "1920" : "1280";
	Settings[ "g_res_windowed_y" ] = (Raptor::Game->Gfx.DesktopH > 1080) ? "1080" : "720";
	Settings[ "g_fullscreen" ] = "true";
	Settings[ "g_vsync" ] = "false";
	Settings[ "g_fsaa" ] = "4";
	Settings[ "g_mipmap" ] = "true";
	Settings[ "g_af" ] = "16";
	Settings[ "g_texture_maxres" ] = "0";
#if SDL_VERSION_ATLEAST(2,0,0)
	Settings[ "g_bpp" ] = "24";
#else
	Settings[ "g_bpp" ] = "32";
#endif
	Settings[ "g_zbits" ] = "24";
	Settings[ "g_znear" ] = "0.125";
	Settings[ "g_zfar" ] = "15000";
	Settings[ "g_fov" ] = "auto";
	Settings[ "g_framebuffers" ] = "true";
	Settings[ "g_framebuffers_anyres" ] = "true";
	Settings[ "g_shader_enable" ] = "true";
	Settings[ "g_shader_version" ] = "110";
	Settings[ "g_shader_light_quality" ] = "2";
	Settings[ "g_shader_point_lights" ] = "4";
	
	Settings[ "s_channels" ] = "2";
	Settings[ "s_rate" ] = "44100";
	Settings[ "s_depth" ] = "16";
	Settings[ "s_buffer" ] = "4096";
	Settings[ "s_mix_channels" ] = "64";
	Settings[ "s_volume" ] = "0.3";
	Settings[ "s_effect_volume" ] = "0.5";
	Settings[ "s_music_volume" ] = "0.9";
	
	Settings[ "vr_enable" ] = "false";
	Settings[ "vr_mirror" ] = "true";
	Settings[ "vr_fov" ] = "-111";
	Settings[ "vr_separation" ] = "0.0625";
	Settings[ "vr_offset" ] = "87";
	
	Settings[ "joy_deadzone" ] = "0.03";
	Settings[ "joy_deadzone_thumbsticks" ] = "0.1";
	Settings[ "joy_deadzone_triggers" ] = "0.02";
	Settings[ "joy_smooth_x" ] = "0";
	Settings[ "joy_smooth_y" ] = "0.125";
	Settings[ "joy_smooth_z" ] = "0.5";
	Settings[ "joy_smooth_pedals" ] = "0";
	Settings[ "joy_smooth_thumbsticks" ] = "2";
	Settings[ "joy_smooth_triggers" ] = "0.5";
	
	#ifdef WIN32
		Settings[ "saitek_enable" ] = "true";
	#endif
	
	const char *name = getenv("USER");
	if( !(name && name[0]) )
		name = getenv("USERNAME");
	if( !(name && name[0]) )
		name = getenv("HOSTNAME");
	if( !(name && name[0]) )
		name = "Name";
	Settings[ "name" ] = name;
	
	Settings[ "netrate" ] = "30";
	Settings[ "maxfps" ] = "60";
	
	#ifdef WIN32
		// Ideal maxfps is the display's refresh rate, especially when using vsync.
		DEVMODE dm;
		memset( &dm, 0, sizeof(dm) );
		dm.dmSize = sizeof(DEVMODE);
		if( EnumDisplaySettings( NULL, ENUM_CURRENT_SETTINGS, &dm )
		&& (dm.dmFields & DM_DISPLAYFREQUENCY)
		&& (dm.dmDisplayFrequency >= 60) )
			Settings[ "maxfps" ] = Num::ToString( (int) dm.dmDisplayFrequency );
	#endif
	
	#ifdef APPLE_POWERPC
		Settings[ "g_fsaa" ] = "2";
		Settings[ "g_af" ] = "2";
		Settings[ "g_framebuffers_anyres" ] = "false";
	#endif
	
	
	// Default server settings.
	
	Settings[ "sv_port" ] = "7000";
	Settings[ "sv_netrate" ] = "30";
	Settings[ "sv_maxfps" ] = "60";
	
	Settings[ "password" ] = "";
}


// ---------------------------------------------------------------------------


void ClientConfig::Command( std::string str, bool show_in_console )
{
	if( show_in_console )
		Raptor::Game->Console.Print( str, TextConsole::MSG_INPUT );
	
	std::list<std::string> all_elements = Str::ParseCommand( str, ORIGINAL, ESCAPED );
	
	try
	{
		while( all_elements.size() )
		{
			// Pull all data relevant to one command.
			
			std::vector<std::string> elements;
			
			while( all_elements.size() && (all_elements.front() != ";") )
			{
				elements.push_back( all_elements.front() );
				all_elements.pop_front();
			}

			if( all_elements.size() && (all_elements.front() == ";") )
				all_elements.pop_front();
			
			
			// Execute one command.
			
			if( elements.size() )
			{
				std::string cmd = elements.at(0);
				elements.erase( elements.begin() );
				std::transform( cmd.begin(), cmd.end(), cmd.begin(), tolower );
				
				if( Raptor::Game->HandleCommand( cmd, &elements ) )
					;
				
				else if( cmd == "echo" )
				{
					if( elements.size() >= 1 )
					{
						std::string print_me = elements.at(0);
						for( size_t i = 1; i < elements.size(); i ++ )
							print_me += std::string(" ") + elements.at(i);
						Raptor::Game->Console.Print( print_me );
					}
					else
						Raptor::Game->Console.Print( "" );
				}
				
				else if( cmd == "show" )
				{
					if( elements.size() >= 1 )
					{
						int count = 0;
						const char *cstr = elements.at(0).c_str();
						int len = strlen(cstr);
						for( std::map<std::string, std::string>::iterator setting_iter = Settings.begin(); setting_iter != Settings.end(); setting_iter ++ )
						{
							if( strncmp( setting_iter->first.c_str(), cstr, len ) == 0 )
							{
								Raptor::Game->Console.Print( setting_iter->first + ": " + setting_iter->second );
								count ++;
							}
						}
						if( ! count )
							Raptor::Game->Console.Print( elements.at(0) + " is not defined." );
					}
					else
					{
						for( std::map<std::string, std::string>::iterator setting_iter = Settings.begin(); setting_iter != Settings.end(); setting_iter ++ )
							Raptor::Game->Console.Print( setting_iter->first + ": " + setting_iter->second );
					}
				}
				
				else if( cmd == "set" )
				{
					if( elements.size() >= 2 )
						Settings[ elements.at(0) ] = elements.at(1);
					else
						Raptor::Game->Console.Print( "Usage: set <variable> <value>", TextConsole::MSG_ERROR );
				}

				else if( cmd == "unset" )
				{
					if( elements.size() >= 1 )
					{
						std::map<std::string, std::string>::iterator setting_iter = Settings.find( elements.at(0) );
						if( setting_iter != Settings.end() )
							Settings.erase( setting_iter );
						else
							Raptor::Game->Console.Print( elements.at(0) + " is not defined." );
					}
					else
						Raptor::Game->Console.Print( "Usage: unset <variable>", TextConsole::MSG_ERROR );
				}
				
				else if( cmd == "set_defaults" )
				{
					Raptor::Game->SetDefaults();
				}
				
				else if( cmd == "bind" )
				{
					if( elements.size() >= 2 )
					{
						uint8_t control = ControlID( elements.at(1) );
						if( control || (elements.at(1) == ControlName(0)) )
						{
							if( ! Bind( elements.at(0), control ) )
								Raptor::Game->Console.Print( "Unknown key/button/axis: " + elements.at(0), TextConsole::MSG_ERROR );
						}
						else
							Raptor::Game->Console.Print( "Unknown command: " + elements.at(1), TextConsole::MSG_ERROR );
					}
					else if( elements.size() == 1 )
					{
						uint8_t control = BoundControl( elements.at(0) );
						if( control )
							Raptor::Game->Console.Print( elements.at(0) + std::string(" is bound to: ") + ControlName(control) );
						else if( Raptor::Game->Input.ValidInput( elements.at(0) ) )
							Raptor::Game->Console.Print( elements.at(0) + std::string(" is unbound.") );
						else
							Raptor::Game->Console.Print( "Unknown key/button/axis: " + elements.at(0), TextConsole::MSG_ERROR );
					}
					else
						Raptor::Game->Console.Print( "Usage: bind <input> <command>", TextConsole::MSG_ERROR );
				}
				
				else if( cmd == "unbind" )
				{
					if( elements.size() >= 1 )
					{
						if( ! Unbind( elements.at(0) ) && ! Raptor::Game->Input.ValidInput( elements.at(0) ) )
							Raptor::Game->Console.Print( "Unknown key/button/axis: " + elements.at(0), TextConsole::MSG_ERROR );
					}
					else
						Raptor::Game->Console.Print( "Usage: unbind <input>", TextConsole::MSG_ERROR );
				}
				
				else if( cmd == "unbindall" )
				{
					if( elements.size() >= 1 )
					{
						for( size_t i = 0; i < elements.size(); i ++ )
						{
							if( elements.at(i) == "keys" )
								KeyBinds.clear();
							else if( elements.at(i) == "mouse" )
								MouseBinds.clear();
							else
							{
								std::map<std::string,std::map<Uint8,uint8_t> >::iterator axes = JoyAxisBinds.find( elements.at(i) );
								if( axes != JoyAxisBinds.end() )
									JoyAxisBinds.erase( axes );
								std::map<std::string,std::map<Uint8,uint8_t> >::iterator buttons = JoyButtonBinds.find( elements.at(i) );
								if( buttons != JoyButtonBinds.end() )
									JoyButtonBinds.erase( buttons );
								std::map<std::string,std::map<Uint8,std::map<Uint8,uint8_t> > >::iterator hats = JoyHatBinds.find( elements.at(i) );
								if( hats != JoyHatBinds.end() )
									JoyHatBinds.erase( hats );
							}
						}
					}
					else
						UnbindAll();
				}
				
				else if( cmd == "bind_defaults" )
				{
					Raptor::Game->SetDefaultControls();
				}
				
				else if( cmd == "exec" )
				{
					if( elements.size() >= 1 )
						Load( elements.at(0) );
					else
						Raptor::Game->Console.Print( "Usage: exec <file>", TextConsole::MSG_ERROR );
				}
				
				else if( cmd == "export" )
				{
					if( elements.size() >= 1 )
						Save( elements.at(0) );
					else
						Raptor::Game->Console.Print( "Usage: export <file>", TextConsole::MSG_ERROR );
				}
				
				else if( cmd == "g_restart" )
				{
					Raptor::Game->Gfx.Restart();
				}
				
				else if( cmd == "s_reload" )
				{
					Raptor::Game->Snd.StopSounds();
					Raptor::Game->Res.DeleteSounds();
				}
				
				else if( cmd == "joy_refresh" )
				{
					Raptor::Game->Joy.Refresh();
				}
				
				else if( cmd == "joy_class" )
				{
					if( elements.size() >= 1 )
						Raptor::Game->Input.DeviceTypes.insert( elements.at(0) );
					else
						Raptor::Game->Console.Print( "Usage: joy_class <name>", TextConsole::MSG_ERROR );
				}
				
				else if( cmd == "joy_delete" )
				{
					if( elements.size() >= 1 )
					{
						std::set<std::string>::iterator dev = Raptor::Game->Input.DeviceTypes.find( elements.at(0) );
						if( dev != Raptor::Game->Input.DeviceTypes.end() )
							Raptor::Game->Input.DeviceTypes.erase( dev );
						std::map<std::string,std::map<Uint8,uint8_t> >::iterator axes = JoyAxisBinds.find( elements.at(0) );
						if( axes != JoyAxisBinds.end() )
							JoyAxisBinds.erase( axes );
						std::map<std::string,std::map<Uint8,uint8_t> >::iterator buttons = JoyButtonBinds.find( elements.at(0) );
						if( buttons != JoyButtonBinds.end() )
							JoyButtonBinds.erase( buttons );
						std::map<std::string,std::map<Uint8,std::map<Uint8,uint8_t> > >::iterator hats = JoyHatBinds.find( elements.at(0) );
						if( hats != JoyHatBinds.end() )
							JoyHatBinds.erase( hats );
					}
					else
						Raptor::Game->Console.Print( "Usage: joy_delete <name>", TextConsole::MSG_ERROR );
				}
				
			#ifdef WIN32
				else if( cmd == "saitek_restart" )
				{
					Raptor::Game->Saitek.RestartService();
				}
			#endif
				
				else if( cmd == "vr_restart" )
				{
					Raptor::Game->Head.RestartVR();
				}
				
				else if( cmd == "vr_recenter" )
				{
					Raptor::Game->Head.Recenter();
				}
				
				else if( cmd == "music" )
				{
					if( elements.size() >= 1 )
					{
						Mix_Music *music = Raptor::Game->Res.GetMusic(elements.at(1));
						if( music )
							Raptor::Game->Snd.PlayMusicOnce( music );
						else
							Raptor::Game->Console.Print( "Couldn't find music: " + elements.at(0), TextConsole::MSG_ERROR );
					}
				}
				
				else if( cmd == "sound" )
				{
					if( elements.size() >= 1 )
					{
						Mix_Chunk *sound = Raptor::Game->Res.GetSound(elements.at(1));
						if( sound )
							Raptor::Game->Snd.Play( sound );
						else
							Raptor::Game->Console.Print( "Couldn't find sound: " + elements.at(0), TextConsole::MSG_ERROR );
					}
				}
				
				else if( cmd == "connect" )
				{
					if( elements.size() >= 1 )
						Raptor::Game->Net.Connect( elements.at(0).c_str(), SettingAsString("name").c_str(), SettingAsString("password").c_str() );
					else
						Raptor::Game->Console.Print( "Usage: connect <ip>", TextConsole::MSG_ERROR );
				}
				
				else if( cmd == "disconnect" )
				{
					if( Raptor::Game->Net.Connected )
						Raptor::Game->Net.Disconnect();
					else
						Raptor::Game->Console.Print( "Not connected.", TextConsole::MSG_ERROR );
				}
				
				else if( cmd == "reconnect" )
				{
					if( Raptor::Server->IsRunning() )
						Raptor::Game->Console.Print( "You are the server.", TextConsole::MSG_ERROR );
					else if( HasSetting("host_address") )
						Raptor::Game->Net.Connect( SettingAsString("host_address").c_str(), SettingAsString("name").c_str(), SettingAsString("password").c_str() );
					else
						Raptor::Game->Console.Print( "No recent server to connect to.", TextConsole::MSG_ERROR );
				}
				
				else if( cmd == "resync" )
				{
					uint16_t player_id = Raptor::Game->PlayerID;
					
					if( (! Raptor::Game->Net.Connected) && Raptor::Game->Net.Host.length() && ! Raptor::Server->IsRunning() )
						Raptor::Game->Net.Reconnect();
					
					if( Raptor::Game->Net.Connected )
					{
						Packet local_resync_request( Raptor::Packet::RESYNC );
						local_resync_request.AddUShort( player_id );
						Raptor::Game->Net.ProcessPacket( &local_resync_request );
					}
					else
						Raptor::Game->Console.Print( "Not connected.", TextConsole::MSG_ERROR );
				}
				
				else if( cmd == "host" )
				{
					Raptor::Game->Host();
				}
				
				else if( cmd == "version" )
				{
					Raptor::Game->Console.Print( Raptor::Game->Game + " " + Raptor::Game->Version );
				}
				
				else if( cmd == "status" )
				{
					char cstr[ 1024 ] = "";
					
					Raptor::Game->Console.Print( Raptor::Game->Game + " " + Raptor::Game->Version );
					#if SDL_VERSION_ATLEAST(2,0,0)
						const char *sdl_version = "SDL2";
					#else
						const char *sdl_version = "SDL1";
					#endif
					snprintf( cstr, sizeof(cstr), "Architecture: %i-bit %s Endian, %s", (int) sizeof(void*) * 8, Endian::Big() ? "Big" : "Little", sdl_version );
					Raptor::Game->Console.Print( cstr );
					
					Raptor::Game->Console.Print( Raptor::Game->Keys.Status() );
					Raptor::Game->Console.Print( Raptor::Game->Mouse.Status() );
					Raptor::Game->Console.Print( Raptor::Game->Joy.Status() );
					Raptor::Game->Console.Print( Raptor::Game->Net.Status() );
					
					snprintf( cstr, sizeof(cstr), "Players: %i", (int) Raptor::Game->Data.Players.size() );
					Raptor::Game->Console.Print( cstr );
					snprintf( cstr, sizeof(cstr), "Objects: %i", (int) Raptor::Game->Data.GameObjects.size() );
					Raptor::Game->Console.Print( cstr );
					snprintf( cstr, sizeof(cstr), "Shaders: %s", Raptor::Game->ShaderMgr.Ready() ? "OK" : "Failed" );
					Raptor::Game->Console.Print( cstr );
					snprintf( cstr, sizeof(cstr), "VR: %s", Raptor::Game->Head.Initialized ? (Raptor::Game->Head.VR ? "Available" : "Unavailable") : "Disabled" );
					Raptor::Game->Console.Print( cstr );
					snprintf( cstr, sizeof(cstr), "Camera Facing: %f %f %f", Raptor::Game->Cam.Fwd.X, Raptor::Game->Cam.Fwd.Y, Raptor::Game->Cam.Fwd.Z );
					Raptor::Game->Console.Print( cstr );
					snprintf( cstr, sizeof(cstr), "FPS: %.0f", 1. / Raptor::Game->FrameTime );
					Raptor::Game->Console.Print( cstr );
					
					if( Raptor::Server->IsRunning() )
					{
						snprintf( cstr, sizeof(cstr), "Server FPS: %.0f", 1. / Raptor::Server->FrameTime );
						Raptor::Game->Console.Print( cstr );
					}
				}
				
				else if( cmd == "model_info" )
				{
					if( elements.size() >= 1 )
					{
						Model *model = Raptor::Game->Res.GetModel(elements.at(0));
						char cstr[ 1024 ] = "";
						snprintf( cstr, sizeof(cstr), "%s: %i materials, %i objects, %i arrays, %i triangles, %.3f x %.3f x %.3f, radius %.3f", elements.at(0).c_str(), (int) model->Materials.size(), (int) model->Objects.size(), (int) model->ArrayCount(), (int) model->TriangleCount(), model->GetLength(), model->GetWidth(), model->GetHeight(), model->GetMaxRadius() );
						Raptor::Game->Console.Print( cstr );
					}
					else
						Raptor::Game->Console.Print( "Usage: model_info <file>", TextConsole::MSG_ERROR );
				}
				
				else if( cmd == "name" )
				{
					if( elements.size() >= 1 )
					{
						std::string name = elements.at(0);
						for( size_t i = 1; i < elements.size(); i ++ )
							name += std::string(" ") + elements.at(i);
						
						Raptor::Game->SetPlayerProperty( "name", name );
					}
					else
					{
						std::map<uint16_t,Player*>::const_iterator player_iter = Raptor::Game->Data.Players.find( Raptor::Game->PlayerID );
						std::string name = (player_iter != Raptor::Game->Data.Players.end()) ? player_iter->second->Name : SettingAsString("name");
						Raptor::Game->Console.Print( std::string("name: ") + name );
					}
				}
				
				else if( cmd == "who" )
				{
					if( Raptor::Game->Net.Connected )
					{
						for( std::map<uint16_t,Player*>::const_iterator player_iter = Raptor::Game->Data.Players.begin(); player_iter != Raptor::Game->Data.Players.end(); player_iter ++ )
						{
							std::string name = player_iter->second->Name;
							std::string version = player_iter->second->PropertyAsString( "version", Raptor::Game->Version.c_str(), Raptor::Game->Version.c_str() );
							if( version != Raptor::Game->Version )
								name += std::string(" [v") + version + std::string("]");
							Raptor::Game->Console.Print( name );
						}
					}
					else
						Raptor::Game->Console.Print( "Not connected.", TextConsole::MSG_ERROR );
				}
				
				else if( cmd == "say" )
				{
					if( !(Raptor::Game->Net.Connected && Raptor::Game->PlayerID) )
						Raptor::Game->Console.Print( "Must be connected to chat.", TextConsole::MSG_ERROR );
					else if( elements.size() >= 1 )
					{
						Player *player = Raptor::Game->Data.GetPlayer( Raptor::Game->PlayerID );
						Packet message = Packet( Raptor::Packet::MESSAGE );
						message.AddString( ((player ? player->Name : std::string("Anonymous")) + std::string(": ") + Str::Join( elements, " " )).c_str() );
						message.AddUInt( TextConsole::MSG_CHAT );
						Raptor::Game->Net.Send( &message );
					}
					else
						Raptor::Game->Console.Print( "Usage: say <message>", TextConsole::MSG_ERROR );
				}
				
				else if( cmd == "rcon" )
				{
					if( ! Raptor::Game->Net.Connected )
						Raptor::Game->Console.Print( "Must be connected to use rcon.", TextConsole::MSG_ERROR );
					else if( elements.size() >= 1 )
					{
						std::string sv_cmd = elements.at(0);
						
						if( sv_cmd == "set" )
						{
							if( elements.size() >= 3 )
							{
								Packet info = Packet( Raptor::Packet::INFO );
								info.AddUShort( 1 );
								info.AddString( elements.at(1) );
								info.AddString( elements.at(2) );
								Raptor::Game->Net.Send( &info );
							}
							else
								Raptor::Game->Console.Print( "Usage: rcon set <variable> <value>", TextConsole::MSG_ERROR );
						}
						else if( sv_cmd == "show" )
						{
							if( elements.size() >= 2 )
							{
								int count = 0;
								const char *cstr = elements.at(1).c_str();
								int len = strlen(cstr);
								for( std::map<std::string, std::string>::iterator property_iter = Raptor::Game->Data.Properties.begin(); property_iter != Raptor::Game->Data.Properties.end(); property_iter ++ )
								{
									if( strncmp( property_iter->first.c_str(), cstr, len ) == 0 )
									{
										Raptor::Game->Console.Print( property_iter->first + ": " + property_iter->second );
										count ++;
									}
								}
								if( ! count )
									Raptor::Game->Console.Print( elements.at(1) + " is not defined." );
							}
							else
							{
								for( std::map<std::string, std::string>::iterator setting_iter = Raptor::Game->Data.Properties.begin(); setting_iter != Raptor::Game->Data.Properties.end(); setting_iter ++ )
									Raptor::Game->Console.Print( setting_iter->first + ": " + setting_iter->second );
							}
						}
						else if( Raptor::Game->Data.HasProperty(sv_cmd) )
						{
							if( elements.size() >= 2 )
							{
								Packet info = Packet( Raptor::Packet::INFO );
								info.AddUShort( 1 );
								info.AddString( sv_cmd );
								info.AddString( elements.at(1) );
								Raptor::Game->Net.Send( &info );
							}
							else
								Raptor::Game->Console.Print( sv_cmd + ": " + Raptor::Game->Data.PropertyAsString(sv_cmd) );
						}
						else
							Raptor::Game->Console.Print( "Unknown rcon command: " + sv_cmd, TextConsole::MSG_ERROR );
					}
					else
						Raptor::Game->Console.Print( "Usage: rcon <command>", TextConsole::MSG_ERROR );
				}
				
				else if( cmd == "sv" )
				{
					if( !( Raptor::Server && Raptor::Server->IsRunning() ) )
						Raptor::Game->Console.Print( "Server is not currently running.", TextConsole::MSG_ERROR );
					else if( elements.size() >= 1 )
					{
						std::string sv_cmd = elements.at(0);
						std::vector<std::string> sv_params = elements;
						while( sv_params.at(0) == "sv" )
							sv_params.erase( sv_params.begin() );
						
						if( Raptor::Server->HandleCommand( sv_cmd, &sv_params ) )
							;
						
						else if( sv_cmd == "set" )
						{
							if( elements.size() >= 3 )
							{
								Raptor::Server->Data.SetProperty( elements.at(1), elements.at(2) );
								
								Packet info = Packet( Raptor::Packet::INFO );
								info.AddUShort( 1 );
								info.AddString( elements.at(1) );
								info.AddString( elements.at(2) );
								Raptor::Server->Net.SendAll( &info );
							}
							else
								Raptor::Game->Console.Print( "Usage: sv set <variable> <value>", TextConsole::MSG_ERROR );
						}
						else if( sv_cmd == "unset" )
						{
							if( elements.size() >= 2 )
							{
								// FIXME: Remove the variable on clients instead of just setting to an empty string.
								Packet info = Packet( Raptor::Packet::INFO );
								info.AddUShort( 1 );
								info.AddString( elements.at(1) );
								info.AddString( "" );
								Raptor::Server->Net.SendAll( &info );
								
								Raptor::Server->Data.Lock.Lock();
								
								std::map<std::string, std::string>::iterator setting_iter = Raptor::Server->Data.Properties.find( elements.at(1) );
								if( setting_iter != Raptor::Server->Data.Properties.end() )
									Raptor::Server->Data.Properties.erase( setting_iter );
								else
									Raptor::Game->Console.Print( elements.at(1) + " is not defined." );
								
								Raptor::Server->Data.Lock.Unlock();
							}
							else
								Raptor::Game->Console.Print( "Usage: sv unset <variable>", TextConsole::MSG_ERROR );
						}
						else if( sv_cmd == "show" )
						{
							if( elements.size() >= 2 )
							{
								int count = 0;
								const char *cstr = elements.at(1).c_str();
								int len = strlen(cstr);
								
								Raptor::Server->Data.Lock.Lock();
								
								for( std::map<std::string, std::string>::iterator property_iter = Raptor::Server->Data.Properties.begin(); property_iter != Raptor::Server->Data.Properties.end(); property_iter ++ )
								{
									if( strncmp( property_iter->first.c_str(), cstr, len ) == 0 )
									{
										Raptor::Game->Console.Print( property_iter->first + ": " + property_iter->second );
										count ++;
									}
								}
								
								Raptor::Server->Data.Lock.Unlock();
								
								if( ! count )
									Raptor::Game->Console.Print( elements.at(1) + " is not defined." );
							}
							else
							{
								Raptor::Server->Data.Lock.Lock();
								
								for( std::map<std::string, std::string>::iterator setting_iter = Raptor::Server->Data.Properties.begin(); setting_iter != Raptor::Server->Data.Properties.end(); setting_iter ++ )
									Raptor::Game->Console.Print( setting_iter->first + ": " + setting_iter->second );
								
								Raptor::Server->Data.Lock.Unlock();
							}
						}
						else if( sv_cmd == "port" )
						{
							if( elements.size() >= 2 )
								Settings["sv_port"] = elements.at(1);
							else
							{
								Raptor::Game->Console.Print( std::string("Server port: ") + Num::ToString( Raptor::Game->Server->Port ) );
								int sv_port = SettingAsInt( "sv_port", Raptor::Game->DefaultPort );
								if( sv_port != Raptor::Game->Server->Port )
									Raptor::Game->Console.Print( std::string("(Will be ") + Num::ToString(sv_port) + std::string(" after restart.)") );
							}
						}
						else if( sv_cmd == "netrate" )
						{
							if( elements.size() >= 2 )
							{
								Settings["sv_netrate"] = elements.at(1);
								Raptor::Server->NetRate = SettingAsDouble( "sv_netrate", 30. );
								Raptor::Server->Net.SetNetRate( Raptor::Server->NetRate );
							}
							else
								Raptor::Game->Console.Print( std::string("Server netrate: ") + Num::ToString( (int)( Raptor::Game->Server->Net.NetRate + 0.5 ) ) );
						}
						else if( sv_cmd == "maxfps" )
						{
							if( elements.size() >= 2 )
							{
								Settings["sv_maxfps"] = elements.at(1);
								Raptor::Server->MaxFPS = SettingAsDouble( "sv_maxfps", 60. );
							}
							else
								Raptor::Game->Console.Print( std::string("Server maxfps: ") + Num::ToString( (int)( Raptor::Game->Server->MaxFPS + 0.5 ) ) );
						}
						else if( sv_cmd == "restart" )
						{
							Raptor::Server->Port = SettingAsInt( "sv_port", Raptor::Game->DefaultPort );
							Raptor::Server->NetRate = SettingAsDouble( "sv_netrate", 30. );
							Raptor::Server->MaxFPS = SettingAsDouble( "sv_maxfps", 60. );
							
							Raptor::Server->Start( SettingAsString("name") );
						}
						else if( sv_cmd == "status" )
						{
							char cstr[ 1024 ] = "";
							
							Raptor::Game->Console.Print( Raptor::Game->Game + " " + Raptor::Game->Version );
							snprintf( cstr, sizeof(cstr), "Architecture: %i-bit %s Endian", (int) sizeof(void*) * 8, Endian::Big() ? "Big" : "Little" );
							Raptor::Game->Console.Print( cstr );
							
							snprintf( cstr, sizeof(cstr), "Players: %i", (int) Raptor::Server->Data.Players.size() );
							Raptor::Game->Console.Print( cstr );
							snprintf( cstr, sizeof(cstr), "Objects: %i", (int) Raptor::Server->Data.GameObjects.size() );
							Raptor::Game->Console.Print( cstr );
							snprintf( cstr, sizeof(cstr), "Server FPS: %.0f", 1. / Raptor::Server->FrameTime );
							Raptor::Game->Console.Print( cstr );
						}
						else if( sv_cmd == "who" )
						{
							for( std::map<uint16_t,Player*>::const_iterator player_iter = Raptor::Server->Data.Players.begin(); player_iter != Raptor::Server->Data.Players.end(); player_iter ++ )
							{
								std::string name = player_iter->second->Name;
								std::string version = player_iter->second->PropertyAsString( "version", Raptor::Server->Version.c_str(), Raptor::Server->Version.c_str() );
								if( version != Raptor::Server->Version )
									name += std::string(" [v") + version + std::string("]");
								Raptor::Game->Console.Print( name );
							}
						}
						else if( sv_cmd == "say" )
						{
							if( elements.size() >= 2 )
							{
								std::vector<std::string> msg_vec = elements;
								msg_vec[0] = "";
								std::string msg = Str::Join( msg_vec, " " );
								
								Packet message = Packet( Raptor::Packet::MESSAGE );
								message.AddString( (std::string("Server:") + msg).c_str() );
								message.AddUInt( TextConsole::MSG_CHAT );
								Raptor::Server->Net.SendAll( &message );
							}
							else
								Raptor::Game->Console.Print( "Usage: sv say <message>", TextConsole::MSG_ERROR );
						}
						else if( sv_cmd == "state" )
						{
							if( elements.size() >= 2 )
							{
								int new_state = Raptor::Server->State;
								if( ((elements.at(1) == "+=") || (elements.at(1) == "+")) && (elements.size() >= 3) )
									new_state += atoi( elements.at(2).c_str() );
								else if( ((elements.at(1) == "-=") || (elements.at(1) == "-")) && (elements.size() >= 3) )
									new_state -= atoi( elements.at(2).c_str() );
								else if( (elements.at(1) == "++") || (elements.at(1) == "+") )
									new_state ++;
								else if( (elements.at(1) == "--") || (elements.at(1) == "-") )
									new_state --;
								else
									new_state = atoi( elements.at(1).c_str() );
								
								if( new_state >= Raptor::State::CONNECTED )
								{
									char cstr[ 1024 ] = "";
									snprintf( cstr, sizeof(cstr), "Changing server from state %i to %i.", Raptor::Server->State, new_state );
									Raptor::Game->Console.Print( cstr );
									
									Raptor::Server->ChangeState( new_state );
								}
								else
									Raptor::Game->Console.Print( "Invalid state requested: " + Num::ToString(new_state), TextConsole::MSG_ERROR );
							}
							else
							{
								char cstr[ 1024 ] = "";
								snprintf( cstr, sizeof(cstr), "Current server state: %i", Raptor::Server->State );
								Raptor::Game->Console.Print( cstr );
							}
						}
						else if( Raptor::Server->Data.HasProperty(sv_cmd) )
						{
							if( elements.size() >= 2 )
							{
								Raptor::Server->Data.SetProperty( sv_cmd, elements.at(1) );
								
								Packet info = Packet( Raptor::Packet::INFO );
								info.AddUShort( 1 );
								info.AddString( sv_cmd );
								info.AddString( elements.at(1) );
								Raptor::Server->Net.SendAll( &info );
							}
							else
								Raptor::Game->Console.Print( sv_cmd + ": " + Raptor::Server->Data.PropertyAsString(sv_cmd) );
						}
						else
							Raptor::Game->Console.Print( "Unknown sv command: " + sv_cmd, TextConsole::MSG_ERROR );
					}
					else
						Raptor::Game->Console.Print( "Usage: sv <command>", TextConsole::MSG_ERROR );
				}
				
				else if( cmd == "quit" )
				{
					Raptor::Game->Quit();
				}
				
				else if( cmd.length() )
				{
					if( HasSetting(cmd) )
					{
						if( elements.size() >= 1 )
							Settings[ cmd ] = elements.at(0);
						else
							Raptor::Game->Console.Print( cmd + ": " + Settings[ cmd ] );
					}
					else
						Raptor::Game->Console.Print( "Unknown command: " + cmd, TextConsole::MSG_ERROR );
				}
			}
		}
	}
	catch( std::out_of_range &exception )
	{
		Raptor::Game->Console.Print( "ClientConfig::Command: std::out_of_range", TextConsole::MSG_ERROR );
	}
}


void ClientConfig::Load( std::string filename )
{
	std::ifstream input( filename.c_str() );
	if( input.is_open() )
	{
		char buffer[ 1024 ] = "";
		while( ! input.eof() )
		{
			buffer[ 0 ] = '\0';
			input.getline( buffer, sizeof(buffer) );
			Command( std::string(buffer) );
		}
		
		input.close();
	}
}


void ClientConfig::Save( std::string filename ) const
{
	FILE *output = fopen( filename.c_str(), "wt" );
	if( output )
	{
		for( std::map<std::string, std::string>::const_iterator setting_iter = Settings.begin(); setting_iter != Settings.end(); setting_iter ++ )
			fprintf( output, "set \"%s\" \"%s\"\n", Str::Escape( setting_iter->first, ORIGINAL, ESCAPED ).c_str(), Str::Escape( setting_iter->second, ORIGINAL, ESCAPED ).c_str() );
		
		if( JoyAxisBinds.size() || JoyButtonBinds.size() || JoyHatBinds.size() )
			for( std::set<std::string>::const_iterator dev_iter = Raptor::Game->Input.DeviceTypes.begin(); dev_iter != Raptor::Game->Input.DeviceTypes.end(); dev_iter ++ )
				fprintf( output, "joy_class \"%s\"\n", dev_iter->c_str() );
		
		if( Raptor::Game->Input.ControlNames.size() > 1 )
			fprintf( output, "\nunbindall\n" );
		
		for( std::map<Uint8,uint8_t>::const_iterator mouse_iter = MouseBinds.begin(); mouse_iter != MouseBinds.end(); mouse_iter ++ )
			fprintf( output, "bind \"%s\" \"%s\"\n", Str::Escape( MouseName(mouse_iter->first), ORIGINAL, ESCAPED ).c_str(), Str::Escape( ControlName(mouse_iter->second), ORIGINAL, ESCAPED ).c_str() );
		
		for( std::map<SDLKey,uint8_t>::const_iterator key_iter = KeyBinds.begin(); key_iter != KeyBinds.end(); key_iter ++ )
			fprintf( output, "bind \"%s\" \"%s\"\n", Str::Escape( KeyName(key_iter->first), ORIGINAL, ESCAPED ).c_str(), Str::Escape( ControlName(key_iter->second), ORIGINAL, ESCAPED ).c_str() );
		
		for( std::map<std::string,std::map<Uint8,uint8_t> >::const_iterator dev_iter = JoyAxisBinds.begin(); dev_iter != JoyAxisBinds.end(); dev_iter ++ )
			for( std::map<Uint8,uint8_t>::const_iterator bind = dev_iter->second.begin(); bind != dev_iter->second.end(); bind ++ )
				fprintf( output, "bind \"%s\" \"%s\"\n", Str::Escape( Raptor::Game->Input.JoyAxisName(dev_iter->first,bind->first), ORIGINAL, ESCAPED ).c_str(), Str::Escape( ControlName(bind->second), ORIGINAL, ESCAPED ).c_str() );
		
		for( std::map<std::string,std::map<Uint8,uint8_t> >::const_iterator dev_iter = JoyButtonBinds.begin(); dev_iter != JoyButtonBinds.end(); dev_iter ++ )
			for( std::map<Uint8,uint8_t>::const_iterator bind = dev_iter->second.begin(); bind != dev_iter->second.end(); bind ++ )
				fprintf( output, "bind \"%s\" \"%s\"\n", Str::Escape( Raptor::Game->Input.JoyButtonName(dev_iter->first,bind->first), ORIGINAL, ESCAPED ).c_str(), Str::Escape( ControlName(bind->second), ORIGINAL, ESCAPED ).c_str() );
		
		for( std::map<std::string,std::map<Uint8,std::map<Uint8,uint8_t> > >::const_iterator dev_iter = JoyHatBinds.begin(); dev_iter != JoyHatBinds.end(); dev_iter ++ )
			for( std::map<Uint8,std::map<Uint8,uint8_t> >::const_iterator hat = dev_iter->second.begin(); hat != dev_iter->second.end(); hat ++ )
				for( std::map<Uint8,uint8_t>::const_iterator bind = hat->second.begin(); bind != hat->second.end(); bind ++ )
					fprintf( output, "bind \"%s\" \"%s\"\n", Str::Escape( Raptor::Game->Input.JoyHatName(dev_iter->first,hat->first,bind->first), ORIGINAL, ESCAPED ).c_str(), Str::Escape( ControlName(bind->second), ORIGINAL, ESCAPED ).c_str() );
		
		fflush( output );
		fclose( output );
		output = NULL;
	}
}


// ---------------------------------------------------------------------------


bool ClientConfig::HasSetting( std::string name ) const
{
	std::map<std::string, std::string>::const_iterator found = Settings.find( name );
	return ( found != Settings.end() );
}


std::string ClientConfig::SettingAsString( std::string name, const char *ifndef, const char *ifempty ) const
{
	std::map<std::string, std::string>::const_iterator found = Settings.find( name );
	if( found != Settings.end() )
	{
		if( found->second.empty() && ifempty )
			return std::string(ifempty);
		return found->second;
	}
	if( ifndef )
		return std::string(ifndef);
	return "";
}


double ClientConfig::SettingAsDouble( std::string name, double ifndef, double ifempty ) const
{
	std::map<std::string, std::string>::const_iterator found = Settings.find( name );
	if( found != Settings.end() )
	{
		if( found->second.empty() )
			return ifempty;
		return Str::AsDouble( found->second );
	}
	return ifndef;
}


int ClientConfig::SettingAsInt( std::string name, int ifndef, int ifempty ) const
{
	std::map<std::string, std::string>::const_iterator found = Settings.find( name );
	if( found != Settings.end() )
	{
		if( found->second.empty() )
			return ifempty;
		return Str::AsInt( found->second );
	}
	return ifndef;
}


bool ClientConfig::SettingAsBool( std::string name, bool ifndef, bool ifempty ) const
{
	std::map<std::string, std::string>::const_iterator found = Settings.find( name );
	if( found != Settings.end() )
	{
		if( found->second.empty() )
			return ifempty;
		return Str::AsBool( found->second );
	}
	return ifndef;
}


std::vector<double> ClientConfig::SettingAsDoubles( std::string name ) const
{
	std::vector<double> results;
	
	std::map<std::string, std::string>::const_iterator found = Settings.find( name );
	if( found != Settings.end() )
	{
		bool seeking = true;
		for( const char *buffer_ptr = found->second.c_str(); *buffer_ptr; buffer_ptr ++ )
		{
			bool numeric_char = strchr( "0123456789.-", *buffer_ptr );
			if( seeking && numeric_char )
			{
				results.push_back( atof(buffer_ptr) );
				seeking = false;
			}
			else if( ! numeric_char )
				seeking = true;
		}
	}
	
	return results;
}


std::vector<int> ClientConfig::SettingAsInts( std::string name ) const
{
	std::vector<int> results;
	
	std::map<std::string, std::string>::const_iterator found = Settings.find( name );
	if( found != Settings.end() )
	{
		bool seeking = true;
		for( const char *buffer_ptr = found->second.c_str(); *buffer_ptr; buffer_ptr ++ )
		{
			bool numeric_char = strchr( "0123456789.-", *buffer_ptr );
			if( seeking && numeric_char )
			{
				results.push_back( atoi(buffer_ptr) );
				seeking = false;
			}
			else if( ! numeric_char )
				seeking = true;
		}
	}
	
	return results;
}


// ---------------------------------------------------------------------------


int8_t ClientConfig::Bind( SDL_Event *event, uint8_t control )
{
	if( event->type == SDL_KEYDOWN )
	{
		KeyBinds[ event->key.keysym.sym ] = control;
		return 1;
	}
	else if( event->type == SDL_MOUSEBUTTONDOWN )
	{
		MouseBinds[ event->button.button ] = control;
		return 1;
	}
#if SDL_VERSION_ATLEAST(2,0,0)
	else if( (event->type == SDL_MOUSEWHEEL) && event->wheel.y )
	{
		Uint8 mouse_button = (event->wheel.y > 0) ? SDL_BUTTON_WHEELUP : SDL_BUTTON_WHEELDOWN;
		MouseBinds[ mouse_button ] = control;
		return 1;
	}
#endif
	else if( (event->type == SDL_JOYAXISMOTION)
	&&       (event->jaxis.value >= 16384)
	&&       (event->jaxis.value > Raptor::Game->Joy.Joysticks[ event->jaxis.which ].PrevAxes[ event->jaxis.axis ]) )
	{
		std::string joy_name = Raptor::Game->Joy.Joysticks[ event->jaxis.which ].Name;
		std::string joy_type = Raptor::Game->Input.DeviceType( joy_name );
		JoyAxisBinds[ joy_type ][ event->jaxis.axis ] = control;
		return 1;
	}
	else if( (event->type == SDL_JOYAXISMOTION)
	&&       (event->jaxis.value <= -16384)
	&&       (event->jaxis.value < Raptor::Game->Joy.Joysticks[ event->jaxis.which ].PrevAxes[ event->jaxis.axis ]) )
	{
		std::string joy_name = Raptor::Game->Joy.Joysticks[ event->jaxis.which ].Name;
		std::string joy_type = Raptor::Game->Input.DeviceType( joy_name );
		JoyAxisBinds[ joy_type ][ event->jaxis.axis ] = control;
		return -1;  // Axis was moved in negative direction.
	}
	else if( event->type == SDL_JOYBUTTONDOWN )
	{
		std::string joy_name = Raptor::Game->Joy.Joysticks[ event->jbutton.which ].Name;
		std::string joy_type = Raptor::Game->Input.DeviceType( joy_name );
		JoyButtonBinds[ joy_type ][ event->jbutton.button ] = control;
		return 1;
	}
	else if( event->type == SDL_JOYHATMOTION )
	{
		std::string joy_name = Raptor::Game->Joy.Joysticks[ event->jhat.which ].Name;
		std::string joy_type = Raptor::Game->Input.DeviceType( joy_name );
		JoyHatBinds[ joy_type ][ event->jhat.hat ][ event->jhat.value ] = control;
		return 1;
	}
	
	return 0;  // Event type could not be bound.
}


bool ClientConfig::Bind( std::string input, uint8_t control )
{
	if( ! control )
		return Unbind( input );
	
	SDLKey key = KeyID( input );
	if( key != SDLK_UNKNOWN )
	{
		KeyBinds[ key ] = control;
		return true;
	}
	
	Uint8 mouse = MouseID( input );
	if( mouse )
	{
		MouseBinds[ mouse ] = control;
		return true;
	}
	
	std::string dev;
	Uint8 index = 0, dir = 0;
	if( Raptor::Game->Input.ParseJoyAxis( input, &dev, &index ) )
	{
		JoyAxisBinds[ dev ][ index ] = control;
		Raptor::Game->Input.DeviceTypes.insert( dev );
		return true;
	}
	else if( Raptor::Game->Input.ParseJoyButton( input, &dev, &index ) )
	{
		JoyButtonBinds[ dev ][ index ] = control;
		Raptor::Game->Input.DeviceTypes.insert( dev );
		return true;
	}
	else if( Raptor::Game->Input.ParseJoyHat( input, &dev, &index, &dir ) )
	{
		JoyHatBinds[ dev ][ index ][ dir ] = control;
		Raptor::Game->Input.DeviceTypes.insert( dev );
		return true;
	}
	
	return false;
}


bool ClientConfig::Unbind( std::string input )
{
	SDLKey key = KeyID( input );
	if( key != SDLK_UNKNOWN )
	{
		std::map<SDLKey,uint8_t>::iterator bind = KeyBinds.find( key );
		if( bind != KeyBinds.end() )
		{
			KeyBinds.erase( bind );
			return true;
		}
		return false;
	}
	
	Uint8 mouse = MouseID( input );
	if( mouse )
	{
		std::map<Uint8,uint8_t>::iterator bind = MouseBinds.find( mouse );
		if( bind != MouseBinds.end() )
		{
			MouseBinds.erase( bind );
			return true;
		}
		return false;
	}
	
	std::string dev;
	Uint8 index = 0, dir = 0;
	if( Raptor::Game->Input.ParseJoyAxis( input, &dev, &index ) )
	{
		std::map<std::string,std::map<Uint8,uint8_t> >::iterator dev_iter = JoyAxisBinds.find( dev );
		if( dev_iter != JoyAxisBinds.end() )
		{
			std::map<Uint8,uint8_t>::iterator bind = dev_iter->second.find( index );
			if( bind != dev_iter->second.end() )
			{
				dev_iter->second.erase( bind );
				if( dev_iter->second.empty() )
					JoyAxisBinds.erase( dev_iter );
				return true;
			}
		}
		return false;
	}
	else if( Raptor::Game->Input.ParseJoyButton( input, &dev, &index ) )
	{
		std::map<std::string,std::map<Uint8,uint8_t> >::iterator dev_iter = JoyButtonBinds.find( dev );
		if( dev_iter != JoyButtonBinds.end() )
		{
			std::map<Uint8,uint8_t>::iterator bind = dev_iter->second.find( index );
			if( bind != dev_iter->second.end() )
			{
				dev_iter->second.erase( bind );
				if( dev_iter->second.empty() )
					JoyButtonBinds.erase( dev_iter );
				return true;
			}
		}
		return false;
	}
	else if( Raptor::Game->Input.ParseJoyHat( input, &dev, &index, &dir ) )
	{
		std::map<std::string,std::map<Uint8,std::map<Uint8,uint8_t> > >::iterator dev_iter = JoyHatBinds.find( dev );
		if( dev_iter != JoyHatBinds.end() )
		{
			std::map<Uint8,std::map<Uint8,uint8_t> >::iterator hat = dev_iter->second.find( index );
			if( hat != dev_iter->second.end() )
			{
				std::map<Uint8,uint8_t>::iterator bind = hat->second.find( dir );
				if( bind != hat->second.end() )
				{
					hat->second.erase( bind );
					if( hat->second.empty() )
					{
						dev_iter->second.erase( hat );
						if( dev_iter->second.empty() )
							JoyHatBinds.erase( dev_iter );
					}
					return true;
				}
			}
		}
		return true;
	}
	
	return false;
}


void ClientConfig::Unbind( uint8_t control )
{
	const std::map<SDLKey, uint8_t>::iterator key_binds_end = KeyBinds.end();
	for(  std::map<SDLKey, uint8_t>::iterator bind = KeyBinds.begin(); bind != key_binds_end; )
	{
		std::map<SDLKey, uint8_t>::iterator next = bind;
		next ++;
		
		if( bind->second == control )
			KeyBinds.erase( bind );
		
		bind = next;
	}
	
	const std::map<Uint8, uint8_t>::iterator mouse_binds_end = MouseBinds.end();
	for(  std::map<Uint8, uint8_t>::iterator bind = MouseBinds.begin(); bind != mouse_binds_end; )
	{
		std::map<Uint8, uint8_t>::iterator next = bind;
		next ++;
		
		if( bind->second == control )
			MouseBinds.erase( bind );
		
		bind = next;
	}
	
	const std::map<std::string, std::map<Uint8, uint8_t> >::iterator joy_axis_binds_end = JoyAxisBinds.end();
	for(  std::map<std::string, std::map<Uint8, uint8_t> >::iterator dev_iter = JoyAxisBinds.begin(); dev_iter != joy_axis_binds_end; dev_iter ++ )
	{
		const std::map<Uint8, uint8_t>::iterator dev_iter_second_end = dev_iter->second.end();
		for(  std::map<Uint8, uint8_t>::iterator bind = dev_iter->second.begin(); bind != dev_iter_second_end; )
		{
			std::map<Uint8, uint8_t>::iterator next = bind;
			next ++;
			
			if( bind->second == control )
				dev_iter->second.erase( bind );
			
			bind = next;
		}
	}
	
	const std::map<std::string, std::map<Uint8, uint8_t> >::iterator joy_button_binds_end = JoyButtonBinds.end();
	for(  std::map<std::string, std::map<Uint8, uint8_t> >::iterator dev_iter = JoyButtonBinds.begin(); dev_iter != joy_button_binds_end; dev_iter ++ )
	{
		const std::map<Uint8, uint8_t>::iterator dev_iter_second_end = dev_iter->second.end();
		for(  std::map<Uint8, uint8_t>::iterator bind = dev_iter->second.begin(); bind != dev_iter_second_end; )
		{
			std::map<Uint8, uint8_t>::iterator next = bind;
			next ++;
			
			if( bind->second == control )
				dev_iter->second.erase( bind );
			
			bind = next;
		}
	}
	
	const std::map<std::string, std::map<Uint8, std::map<Uint8, uint8_t> > >::iterator joy_hat_binds_end = JoyHatBinds.end();
	for(  std::map<std::string, std::map<Uint8, std::map<Uint8, uint8_t> > >::iterator dev_iter = JoyHatBinds.begin(); dev_iter != joy_hat_binds_end; dev_iter ++ )
	{
		const std::map<Uint8, std::map<Uint8, uint8_t> >::iterator dev_iter_second_end = dev_iter->second.end();
		for(  std::map<Uint8, std::map<Uint8, uint8_t> >::iterator hat = dev_iter->second.begin(); hat != dev_iter_second_end; hat ++ )
		{
			const std::map<Uint8, uint8_t>::iterator hat_second_end = hat->second.end();
			for(  std::map<Uint8, uint8_t>::iterator bind = hat->second.begin(); bind != hat_second_end; )
			{
				std::map<Uint8, uint8_t>::iterator next = bind;
				next ++;
				
				if( bind->second == control )
					hat->second.erase( bind );
				
				bind = next;
			}
		}
	}
}


void ClientConfig::UnbindAll( void )
{
	KeyBinds.clear();
	MouseBinds.clear();
	JoyAxisBinds.clear();
	JoyButtonBinds.clear();
	JoyHatBinds.clear();
}


size_t ClientConfig::SwapBinds( uint8_t control_a, uint8_t control_b )
{
	size_t count = 0;
	
	const std::map<SDLKey, uint8_t>::iterator key_binds_end = KeyBinds.end();
	for(  std::map<SDLKey, uint8_t>::iterator bind = KeyBinds.begin(); bind != key_binds_end; )
	{
		std::map<SDLKey, uint8_t>::iterator next = bind;
		next ++;
		
		if( bind->second == control_a )
		{
			KeyBinds[ bind->first ] = control_b;
			count ++;
		}
		else if( bind->second == control_b )
		{
			KeyBinds[ bind->first ] = control_a;
			count ++;
		}
		
		bind = next;
	}
	
	const std::map<Uint8, uint8_t>::iterator mouse_binds_end = MouseBinds.end();
	for(  std::map<Uint8, uint8_t>::iterator bind = MouseBinds.begin(); bind != mouse_binds_end; )
	{
		std::map<Uint8, uint8_t>::iterator next = bind;
		next ++;
		
		if( bind->second == control_a )
		{
			MouseBinds[ bind->first ] = control_b;
			count ++;
		}
		else if( bind->second == control_b )
		{
			MouseBinds[ bind->first ] = control_a;
			count ++;
		}
		
		bind = next;
	}
	
	const std::map<std::string, std::map<Uint8, uint8_t> >::iterator joy_axis_binds_end = JoyAxisBinds.end();
	for(  std::map<std::string, std::map<Uint8, uint8_t> >::iterator dev_iter = JoyAxisBinds.begin(); dev_iter != joy_axis_binds_end; dev_iter ++ )
	{
		const std::map<Uint8, uint8_t>::iterator dev_iter_second_end = dev_iter->second.end();
		for(  std::map<Uint8, uint8_t>::iterator bind = dev_iter->second.begin(); bind != dev_iter_second_end; )
		{
			std::map<Uint8, uint8_t>::iterator next = bind;
			next ++;
			
			if( bind->second == control_a )
			{
				JoyAxisBinds[ dev_iter->first ][ bind->first ] = control_b;
				count ++;
			}
			else if( bind->second == control_b )
			{
				JoyAxisBinds[ dev_iter->first ][ bind->first ] = control_a;
				count ++;
			}
			
			bind = next;
		}
	}
	
	const std::map<std::string, std::map<Uint8, uint8_t> >::iterator joy_button_binds_end = JoyButtonBinds.end();
	for(  std::map<std::string, std::map<Uint8, uint8_t> >::iterator dev_iter = JoyButtonBinds.begin(); dev_iter != joy_button_binds_end; dev_iter ++ )
	{
		const std::map<Uint8, uint8_t>::iterator dev_iter_second_end = dev_iter->second.end();
		for(  std::map<Uint8, uint8_t>::iterator bind = dev_iter->second.begin(); bind != dev_iter_second_end; )
		{
			std::map<Uint8, uint8_t>::iterator next = bind;
			next ++;
			
			if( bind->second == control_a )
			{
				JoyButtonBinds[ dev_iter->first ][ bind->first ] = control_b;
				count ++;
			}
			else if( bind->second == control_b )
			{
				JoyButtonBinds[ dev_iter->first ][ bind->first ] = control_a;
				count ++;
			}
			
			bind = next;
		}
	}
	
	const std::map<std::string, std::map<Uint8, std::map<Uint8, uint8_t> > >::iterator joy_hat_binds_end = JoyHatBinds.end();
	for(  std::map<std::string, std::map<Uint8, std::map<Uint8, uint8_t> > >::iterator dev_iter = JoyHatBinds.begin(); dev_iter != joy_hat_binds_end; dev_iter ++ )
	{
		const std::map<Uint8, std::map<Uint8, uint8_t> >::iterator dev_iter_second_end = dev_iter->second.end();
		for(  std::map<Uint8, std::map<Uint8, uint8_t> >::iterator hat = dev_iter->second.begin(); hat != dev_iter_second_end; hat ++ )
		{
			const std::map<Uint8, uint8_t>::iterator hat_second_end = hat->second.end();
			for(  std::map<Uint8, uint8_t>::iterator bind = hat->second.begin(); bind != hat_second_end; )
			{
				std::map<Uint8, uint8_t>::iterator next = bind;
				next ++;
				
				if( bind->second == control_a )
				{
					JoyHatBinds[ dev_iter->first ][ hat->first ][ bind->first ] = control_b;
					count ++;
				}
				else if( bind->second == control_b )
				{
					JoyHatBinds[ dev_iter->first ][ hat->first ][ bind->first ] = control_a;
					count ++;
				}
				
				bind = next;
			}
		}
	}
	
	return count;
}


// ---------------------------------------------------------------------------


uint8_t ClientConfig::BoundControl( const SDL_Event *event, bool down_only ) const
{
	if( (event->type == SDL_KEYDOWN) || ((event->type == SDL_KEYUP) && ! down_only) )
	{
#if SDL_VERSION_ATLEAST(2,0,0)
		if( event->key.repeat )
			return 0;
#endif
		std::map<SDLKey, uint8_t>::const_iterator bind = KeyBinds.find( event->key.keysym.sym );
		if( bind != KeyBinds.end() )
			return bind->second;
	}
	
	else if( (event->type == SDL_MOUSEBUTTONDOWN) || ((event->type == SDL_MOUSEBUTTONUP) && ! down_only) )
	{
		std::map<Uint8, uint8_t>::const_iterator bind = MouseBinds.find( event->button.button );
		if( bind != MouseBinds.end() )
			return bind->second;
	}
	
#if SDL_VERSION_ATLEAST(2,0,0)
	else if( (event->type == SDL_MOUSEWHEEL) && event->wheel.y )
	{
		Uint8 mouse_button = (event->wheel.y > 0) ? SDL_BUTTON_WHEELUP : SDL_BUTTON_WHEELDOWN;
		std::map<Uint8, uint8_t>::const_iterator bind = MouseBinds.find( mouse_button );
		if( bind != MouseBinds.end() )
			return bind->second;
	}
#endif
	
	else if( event->type == SDL_JOYAXISMOTION )
	{
		std::string dev = Raptor::Game->Input.DeviceType( Raptor::Game->Joy.Name( event->jaxis.which ) );
		std::map<std::string, std::map<Uint8, uint8_t> >::const_iterator dev_iter = JoyAxisBinds.find( dev );
		if( dev_iter != JoyAxisBinds.end() )
		{
			std::map<Uint8, uint8_t>::const_iterator bind = dev_iter->second.find( event->jaxis.axis );
			if( bind != dev_iter->second.end() )
				return bind->second;
		}
	}
	
	else if( (event->type == SDL_JOYBUTTONDOWN) || ((event->type == SDL_JOYBUTTONUP) && ! down_only) )
	{
		std::string dev = Raptor::Game->Input.DeviceType( Raptor::Game->Joy.Name( event->jbutton.which ) );
		std::map<std::string, std::map<Uint8, uint8_t> >::const_iterator dev_iter = JoyButtonBinds.find( dev );
		if( dev_iter != JoyButtonBinds.end() )
		{
			std::map<Uint8, uint8_t>::const_iterator bind = dev_iter->second.find( event->jbutton.button );
			if( bind != dev_iter->second.end() )
				return bind->second;
		}
	}
	
	else if( event->type == SDL_JOYHATMOTION )
	{
		std::string dev = Raptor::Game->Input.DeviceType( Raptor::Game->Joy.Name( event->jhat.which ) );
		std::map<std::string, std::map<Uint8, std::map<Uint8, uint8_t> > >::const_iterator dev_iter = JoyHatBinds.find( dev );
		if( dev_iter != JoyHatBinds.end() )
		{
			std::map<Uint8, std::map<Uint8, uint8_t> >::const_iterator hat = dev_iter->second.find( event->jhat.hat );
			if( hat != dev_iter->second.end() )
			{
				std::map<Uint8, uint8_t>::const_iterator bind = hat->second.find( event->jhat.value );
				if( bind != hat->second.end() )
					return bind->second;
			}
		}
	}
	
	return 0;
}


uint8_t ClientConfig::BoundControl( std::string input ) const
{
	SDLKey key = KeyID( input );
	if( key != SDLK_UNKNOWN )
		return KeyBind( key );
	
	Uint8 mouse = MouseID( input );
	if( mouse )
		return MouseBind( mouse );
	
	std::string dev;
	Uint8 index = 0, dir = 0;
	if( Raptor::Game->Input.ParseJoyAxis( input, &dev, &index ) )
		return JoyAxisBind( dev, index );
	else if( Raptor::Game->Input.ParseJoyButton( input, &dev, &index ) )
		return JoyButtonBind( dev, index );
	else if( Raptor::Game->Input.ParseJoyHat( input, &dev, &index, &dir ) )
		return JoyHatBind( dev, index, dir );
	
	return 0;
}


// ---------------------------------------------------------------------------


std::string ClientConfig::ControlName( uint8_t control ) const
{
	return Raptor::Game->Input.ControlName( control );
}


std::string ClientConfig::KeyName( SDLKey key ) const
{
	return Raptor::Game->Input.KeyName( key );
}


std::string ClientConfig::MouseName( Uint8 mouse ) const
{
	return Raptor::Game->Input.MouseName( mouse );
}


uint8_t ClientConfig::ControlID( std::string name ) const
{
	return Raptor::Game->Input.ControlID( name );
}


SDLKey ClientConfig::KeyID( std::string name ) const
{
	return Raptor::Game->Input.KeyID( name );
}


Uint8 ClientConfig::MouseID( std::string name ) const
{
	return Raptor::Game->Input.MouseID( name );
}


// ---------------------------------------------------------------------------


uint8_t ClientConfig::KeyBind( SDLKey key ) const
{
	std::map<SDLKey,uint8_t>::const_iterator bind = KeyBinds.find( key );
	return (bind != KeyBinds.end()) ? bind->second : 0;
}

uint8_t ClientConfig::MouseBind( Uint8 button ) const
{
	std::map<Uint8,uint8_t>::const_iterator bind = MouseBinds.find( button );
	return (bind != MouseBinds.end()) ? bind->second : 0;
}


uint8_t ClientConfig::JoyAxisBind( std::string dev, Uint8 axis ) const
{
	std::map<std::string,std::map<Uint8,uint8_t> >::const_iterator dev_iter = JoyAxisBinds.find( dev );
	if( dev_iter != JoyAxisBinds.end() )
	{
		std::map<Uint8,uint8_t>::const_iterator bind = dev_iter->second.find( axis );
		return (bind != dev_iter->second.end()) ? bind->second : 0;
	}
	return 0;
}


uint8_t ClientConfig::JoyButtonBind( std::string dev, Uint8 button ) const
{
	std::map<std::string,std::map<Uint8,uint8_t> >::const_iterator dev_iter = JoyButtonBinds.find( dev );
	if( dev_iter != JoyButtonBinds.end() )
	{
		std::map<Uint8,uint8_t>::const_iterator bind = dev_iter->second.find( button );
		return (bind != dev_iter->second.end()) ? bind->second : 0;
	}
	return 0;
}


uint8_t ClientConfig::JoyHatBind( std::string dev, Uint8 hat, Uint8 dir ) const
{
	std::map<std::string,std::map<Uint8,std::map<Uint8,uint8_t> > >::const_iterator dev_iter = JoyHatBinds.find( dev );
	if( dev_iter != JoyHatBinds.end() )
	{
		std::map<Uint8,std::map<Uint8,uint8_t> >::const_iterator hat_iter = dev_iter->second.find( hat );
		if( hat_iter != dev_iter->second.end() )
		{
			std::map<Uint8,uint8_t>::const_iterator bind = hat_iter->second.find( dir );
			return (bind != hat_iter->second.end()) ? bind->second : 0;
		}
	}
	return 0;
}


// ---------------------------------------------------------------------------


bool ClientConfig::HasControlBound( uint8_t control ) const
{
	const std::map<SDLKey, uint8_t>::const_iterator key_binds_end = KeyBinds.end();
	for(  std::map<SDLKey, uint8_t>::const_iterator bind = KeyBinds.begin(); bind != key_binds_end; bind ++ )
	{
		if( bind->second == control )
			return true;
	}
	
	const std::map<Uint8, uint8_t>::const_iterator mouse_binds_end = MouseBinds.end();
	for(  std::map<Uint8, uint8_t>::const_iterator bind = MouseBinds.begin(); bind != mouse_binds_end; bind ++ )
	{
		if( bind->second == control )
			return true;
	}
	
	const std::map<std::string, std::map<Uint8, uint8_t> >::const_iterator joy_axis_binds_end = JoyAxisBinds.end();
	for(  std::map<std::string, std::map<Uint8, uint8_t> >::const_iterator dev_iter = JoyAxisBinds.begin(); dev_iter != joy_axis_binds_end; dev_iter ++ )
	{
		const std::map<Uint8, uint8_t>::const_iterator dev_iter_second_end = dev_iter->second.end();
		for(  std::map<Uint8, uint8_t>::const_iterator bind = dev_iter->second.begin(); bind != dev_iter_second_end; bind ++ )
		{
			if( bind->second == control )
				return true;
		}
	}
	
	const std::map<std::string, std::map<Uint8, uint8_t> >::const_iterator joy_button_binds_end = JoyButtonBinds.end();
	for(  std::map<std::string, std::map<Uint8, uint8_t> >::const_iterator dev_iter = JoyButtonBinds.begin(); dev_iter != joy_button_binds_end; dev_iter ++ )
	{
		const std::map<Uint8, uint8_t>::const_iterator dev_iter_second_end = dev_iter->second.end();
		for(  std::map<Uint8, uint8_t>::const_iterator bind = dev_iter->second.begin(); bind != dev_iter_second_end; bind ++ )
		{
			if( bind->second == control )
				return true;
		}
	}
	
	const std::map<std::string, std::map<Uint8, std::map<Uint8, uint8_t> > >::const_iterator joy_hat_binds_end = JoyHatBinds.end();
	for(  std::map<std::string, std::map<Uint8, std::map<Uint8, uint8_t> > >::const_iterator dev_iter = JoyHatBinds.begin(); dev_iter != joy_hat_binds_end; dev_iter ++ )
	{
		const std::map<Uint8, std::map<Uint8, uint8_t> >::const_iterator dev_iter_second_end = dev_iter->second.end();
		for(  std::map<Uint8, std::map<Uint8, uint8_t> >::const_iterator hat = dev_iter->second.begin(); hat != dev_iter_second_end; hat ++ )
		{
			const std::map<Uint8, uint8_t>::const_iterator hat_second_end = hat->second.end();
			for(  std::map<Uint8, uint8_t>::const_iterator bind = hat->second.begin(); bind != hat_second_end; bind ++ )
			{
				if( bind->second == control )
					return true;
			}
		}
	}
	
	return false;
}


std::vector<std::string> ClientConfig::ControlBoundTo( uint8_t control ) const
{
	std::vector<std::string> binds;
	
	const std::map<Uint8, uint8_t>::const_iterator mouse_binds_end = MouseBinds.end();
	for(  std::map<Uint8, uint8_t>::const_iterator bind = MouseBinds.begin(); bind != mouse_binds_end; bind ++ )
	{
		if( bind->second == control )
			binds.push_back( Raptor::Game->Input.MouseName( bind->first ) );
	}
	
	const std::map<SDLKey, uint8_t>::const_iterator key_binds_end = KeyBinds.end();
	for(  std::map<SDLKey, uint8_t>::const_iterator bind = KeyBinds.begin(); bind != key_binds_end; bind ++ )
	{
		if( bind->second == control )
			binds.push_back( Raptor::Game->Input.KeyName( bind->first ) );
	}
	
	const std::map<std::string, std::map<Uint8, uint8_t> >::const_iterator joy_axis_binds_end = JoyAxisBinds.end();
	for(  std::map<std::string, std::map<Uint8, uint8_t> >::const_iterator dev_iter = JoyAxisBinds.begin(); dev_iter != joy_axis_binds_end; dev_iter ++ )
	{
		const std::map<Uint8, uint8_t>::const_iterator dev_iter_second_end = dev_iter->second.end();
		for(  std::map<Uint8, uint8_t>::const_iterator bind = dev_iter->second.begin(); bind != dev_iter_second_end; bind ++ )
		{
			if( bind->second == control )
				binds.push_back( Raptor::Game->Input.JoyAxisName( dev_iter->first, bind->first ) );
		}
	}
	
	const std::map<std::string, std::map<Uint8, uint8_t> >::const_iterator joy_button_binds_end = JoyButtonBinds.end();
	for(  std::map<std::string, std::map<Uint8, uint8_t> >::const_iterator dev_iter = JoyButtonBinds.begin(); dev_iter != joy_button_binds_end; dev_iter ++ )
	{
		const std::map<Uint8, uint8_t>::const_iterator dev_iter_second_end = dev_iter->second.end();
		for(  std::map<Uint8, uint8_t>::const_iterator bind = dev_iter->second.begin(); bind != dev_iter_second_end; bind ++ )
		{
			if( bind->second == control )
				binds.push_back( Raptor::Game->Input.JoyButtonName( dev_iter->first, bind->first ) );
		}
	}
	
	const std::map<std::string, std::map<Uint8, std::map<Uint8, uint8_t> > >::const_iterator joy_hat_binds_end = JoyHatBinds.end();
	for(  std::map<std::string, std::map<Uint8, std::map<Uint8, uint8_t> > >::const_iterator dev_iter = JoyHatBinds.begin(); dev_iter != joy_hat_binds_end; dev_iter ++ )
	{
		const std::map<Uint8, std::map<Uint8, uint8_t> >::const_iterator dev_iter_second_end = dev_iter->second.end();
		for(  std::map<Uint8, std::map<Uint8, uint8_t> >::const_iterator hat = dev_iter->second.begin(); hat != dev_iter_second_end; hat ++ )
		{
			const std::map<Uint8, uint8_t>::const_iterator hat_second_end = hat->second.end();
			for(  std::map<Uint8, uint8_t>::const_iterator bind = hat->second.begin(); bind != hat_second_end; bind ++ )
			{
				if( bind->second == control )
					binds.push_back( Raptor::Game->Input.JoyHatName( dev_iter->first, hat->first, bind->first ) );
			}
		}
	}
	
	return binds;
}
