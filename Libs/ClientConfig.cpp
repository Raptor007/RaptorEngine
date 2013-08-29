/*
 *  ClientConfig.cpp
 */

#include "ClientConfig.h"

#include <cstring>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include "Str.h"
#include "Num.h"
#include "RaptorGame.h"


#define ORIGINAL "\"/\n\r\t"
#define ESCAPED  "\"/nrt"


ClientConfig::ClientConfig( void )
{
	// Configure key and mouse button names.
	
	KeyNames[ SDLK_BACKSPACE ] = "Backspace";
	KeyNames[ SDLK_TAB ] = "Tab";
	KeyNames[ SDLK_CLEAR ] = "Clear";
	KeyNames[ SDLK_RETURN ] = "Return";
	KeyNames[ SDLK_PAUSE ] = "Pause";
	KeyNames[ SDLK_ESCAPE ] = "Esc";
	KeyNames[ SDLK_SPACE ] = "Space";
	KeyNames[ SDLK_EXCLAIM ] = "!";
	KeyNames[ SDLK_QUOTEDBL ] = "\"";
	KeyNames[ SDLK_HASH ] = "#";
	KeyNames[ SDLK_DOLLAR ] = "$";
	KeyNames[ SDLK_AMPERSAND ] = "&";
	KeyNames[ SDLK_QUOTE ] = "'";
	KeyNames[ SDLK_LEFTPAREN ] = "(";
	KeyNames[ SDLK_RIGHTPAREN ] = ")";
	KeyNames[ SDLK_ASTERISK ] = "*";
	KeyNames[ SDLK_PLUS ] = "+";
	KeyNames[ SDLK_COMMA ] = ",";
	KeyNames[ SDLK_MINUS ] = "-";
	KeyNames[ SDLK_PERIOD ] = ".";
	KeyNames[ SDLK_SLASH ] = "/";
	KeyNames[ SDLK_0 ] = "0";
	KeyNames[ SDLK_1 ] = "1";
	KeyNames[ SDLK_2 ] = "2";
	KeyNames[ SDLK_3 ] = "3";
	KeyNames[ SDLK_4 ] = "4";
	KeyNames[ SDLK_5 ] = "5";
	KeyNames[ SDLK_6 ] = "6";
	KeyNames[ SDLK_7 ] = "7";
	KeyNames[ SDLK_8 ] = "8";
	KeyNames[ SDLK_9 ] = "9";
	KeyNames[ SDLK_COLON ] = ":";
	KeyNames[ SDLK_SEMICOLON ] = ";";
	KeyNames[ SDLK_LESS ] = "<";
	KeyNames[ SDLK_EQUALS ] = "=";
	KeyNames[ SDLK_GREATER ] = ">";
	KeyNames[ SDLK_QUESTION ] = "?";
	KeyNames[ SDLK_AT ] = "@";
	KeyNames[ SDLK_LEFTBRACKET ] = "[";
	KeyNames[ SDLK_BACKSLASH ] = "\\";
	KeyNames[ SDLK_RIGHTBRACKET ] = "]";
	KeyNames[ SDLK_CARET ] = "^";
	KeyNames[ SDLK_UNDERSCORE ] = "_";
	KeyNames[ SDLK_BACKQUOTE ] = "`";
	KeyNames[ SDLK_a ] = "A";
	KeyNames[ SDLK_b ] = "B";
	KeyNames[ SDLK_c ] = "C";
	KeyNames[ SDLK_d ] = "D";
	KeyNames[ SDLK_e ] = "E";
	KeyNames[ SDLK_f ] = "F";
	KeyNames[ SDLK_g ] = "G";
	KeyNames[ SDLK_h ] = "H";
	KeyNames[ SDLK_i ] = "I";
	KeyNames[ SDLK_j ] = "J";
	KeyNames[ SDLK_k ] = "K";
	KeyNames[ SDLK_l ] = "L";
	KeyNames[ SDLK_m ] = "M";
	KeyNames[ SDLK_n ] = "N";
	KeyNames[ SDLK_o ] = "O";
	KeyNames[ SDLK_p ] = "P";
	KeyNames[ SDLK_q ] = "Q";
	KeyNames[ SDLK_r ] = "R";
	KeyNames[ SDLK_s ] = "S";
	KeyNames[ SDLK_t ] = "T";
	KeyNames[ SDLK_u ] = "U";
	KeyNames[ SDLK_v ] = "V";
	KeyNames[ SDLK_w ] = "W";
	KeyNames[ SDLK_x ] = "X";
	KeyNames[ SDLK_y ] = "Y";
	KeyNames[ SDLK_z ] = "Z";
	KeyNames[ SDLK_DELETE ] = "Delete";
	KeyNames[ SDLK_KP0 ] = "KP0";
	KeyNames[ SDLK_KP1 ] = "KP1";
	KeyNames[ SDLK_KP2 ] = "KP2";
	KeyNames[ SDLK_KP3 ] = "KP3";
	KeyNames[ SDLK_KP4 ] = "KP4";
	KeyNames[ SDLK_KP5 ] = "KP5";
	KeyNames[ SDLK_KP6 ] = "KP6";
	KeyNames[ SDLK_KP7 ] = "KP7";
	KeyNames[ SDLK_KP8 ] = "KP8";
	KeyNames[ SDLK_KP9 ] = "KP9";
	KeyNames[ SDLK_KP_PERIOD ] = "KP .";
	KeyNames[ SDLK_KP_DIVIDE ] = "KP /";
	KeyNames[ SDLK_KP_MULTIPLY ] = "KP *";
	KeyNames[ SDLK_KP_MINUS ] = "KP -";
	KeyNames[ SDLK_KP_PLUS ] = "KP +";
	KeyNames[ SDLK_KP_ENTER	] = "KP Enter";
	KeyNames[ SDLK_KP_EQUALS ] = "KP =";
	KeyNames[ SDLK_UP ] = "Up";
	KeyNames[ SDLK_DOWN ] = "Down";
	KeyNames[ SDLK_RIGHT ] = "Right";
	KeyNames[ SDLK_LEFT ] = "Left";
	KeyNames[ SDLK_INSERT ] = "Insert";
	KeyNames[ SDLK_HOME ] = "Home";
	KeyNames[ SDLK_END ] = "End";
	KeyNames[ SDLK_PAGEUP ] = "PgUp";
	KeyNames[ SDLK_PAGEDOWN ] = "PgDn";
	KeyNames[ SDLK_F1 ] = "F1";
	KeyNames[ SDLK_F2 ] = "F2";
	KeyNames[ SDLK_F3 ] = "F3";
	KeyNames[ SDLK_F4 ] = "F4";
	KeyNames[ SDLK_F5 ] = "F5";
	KeyNames[ SDLK_F6 ] = "F6";
	KeyNames[ SDLK_F7 ] = "F7";
	KeyNames[ SDLK_F8 ] = "F8";
	KeyNames[ SDLK_F9 ] = "F9";
	KeyNames[ SDLK_F10 ] = "F10";
	KeyNames[ SDLK_F11 ] = "F11";
	KeyNames[ SDLK_F12 ] = "F12";
	KeyNames[ SDLK_F13 ] = "F13";
	KeyNames[ SDLK_F14 ] = "F14";
	KeyNames[ SDLK_F15 ] = "F15";
	KeyNames[ SDLK_NUMLOCK ] = "NumLock";
	KeyNames[ SDLK_CAPSLOCK ] = "CapsLock";
	KeyNames[ SDLK_SCROLLOCK ] = "ScrollLock";
	KeyNames[ SDLK_RSHIFT ] = "RShift";
	KeyNames[ SDLK_LSHIFT ] = "LShift";
	KeyNames[ SDLK_RCTRL ] = "RCtrl";
	KeyNames[ SDLK_LCTRL ] = "LCtrl";
	#ifdef __MACOSX__
	KeyNames[ SDLK_RALT ] = "ROption";
	KeyNames[ SDLK_LALT ] = "LOption";
	KeyNames[ SDLK_RMETA ] = "RCmd";
	KeyNames[ SDLK_LMETA ] = "LCmd";
	#else
	KeyNames[ SDLK_RALT ] = "RAlt";
	KeyNames[ SDLK_LALT ] = "LAlt";
	KeyNames[ SDLK_RMETA ] = "RMeta";
	KeyNames[ SDLK_LMETA ] = "LMeta";
	#endif
	KeyNames[ SDLK_LSUPER ] = "LWin";
	KeyNames[ SDLK_RSUPER ] = "RWin";
	KeyNames[ SDLK_MODE ] = "Mode";
	KeyNames[ SDLK_COMPOSE ] = "Compose";
	KeyNames[ SDLK_HELP ] = "Help";
	KeyNames[ SDLK_PRINT ] = "Print";
	KeyNames[ SDLK_SYSREQ ] = "SysReq";
	KeyNames[ SDLK_BREAK ] = "Break";
	KeyNames[ SDLK_MENU ] = "Menu";
	KeyNames[ SDLK_POWER ] = "Power";
	KeyNames[ SDLK_EURO ] = "Euro";
	KeyNames[ SDLK_UNDO ] = "Undo";
	
	MouseNames[ SDL_BUTTON_LEFT ] = "Mouse1";
	MouseNames[ SDL_BUTTON_RIGHT ] = "Mouse2";
	MouseNames[ SDL_BUTTON_MIDDLE ] = "Mouse3";
	MouseNames[ SDL_BUTTON_WHEELUP ] = "MWheelUp";
	MouseNames[ SDL_BUTTON_WHEELDOWN ] = "MWheelDown";
	MouseNames[ SDL_BUTTON_X1 ] = "Mouse4";
	MouseNames[ SDL_BUTTON_X2 ] = "Mouse5";
	
	
	// Define command names (only UNBOUND is included in the engine).
	
	ControlNames[ UNBOUND ] = "Unbound";


	// Set basic default configuration.

	SetDefaults();
}


ClientConfig::~ClientConfig()
{
}


// ---------------------------------------------------------------------------


void ClientConfig::SetDefaults( void )
{
	// Remove any existing settings and binds.
	
	Settings.clear();
	UnbindAll();
	
	
	// Default client settings.
	
	Settings[ "g_res_fullscreen_x" ] = "0";
	Settings[ "g_res_fullscreen_y" ] = "0";
	Settings[ "g_res_windowed_x" ] = "1280";
	Settings[ "g_res_windowed_y" ] = "720";
	Settings[ "g_fullscreen" ] = "true";
	Settings[ "g_fsaa" ] = "4";
	Settings[ "g_af" ] = "16";
	Settings[ "g_znear" ] = Num::ToString(Z_NEAR);
	Settings[ "g_shader_enable" ] = "true";
	Settings[ "g_shader_file" ] = "model";
	
	Settings[ "s_channels" ] = "2";
	Settings[ "s_rate" ] = "44100";
	Settings[ "s_depth" ] = "16";
	Settings[ "s_buffer" ] = "4096";
	Settings[ "s_mix_channels" ] = "64";
	Settings[ "s_volume" ] = "0.5";
	Settings[ "s_effect_volume" ] = "0.5";
	Settings[ "s_music_volume" ] = "1.0";
	
	Settings[ "name" ] = "Name";
	
	Settings[ "netrate" ] = "30";
	Settings[ "maxfps" ] = "120";
	
	#ifdef APPLE_POWERPC
		Settings[ "g_fsaa" ] = "2";
		Settings[ "g_af" ] = "2";
	#endif
	
	
	// Default server settings.
	
	Settings[ "sv_port" ] = "7000";
	Settings[ "sv_netrate" ] = "30";
	Settings[ "sv_maxfps" ] = "120";
	Settings[ "sv_use_out_threads" ] = "true";
	
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
				std::transform( cmd.begin(), cmd.end(), cmd.begin(), tolower );
				
				if( cmd == "echo" )
				{
					if( elements.size() >= 2 )
						Raptor::Game->Console.Print( elements.at(1) );
					else
						Raptor::Game->Console.Print( "" );
				}
				
				else if( cmd == "show" )
				{
					if( elements.size() >= 2 )
					{
						int count = 0;
						const char *cstr = elements.at(1).c_str();
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
							Raptor::Game->Console.Print( elements.at(1) + " is not defined." );
					}
					else
					{
						for( std::map<std::string, std::string>::iterator setting_iter = Settings.begin(); setting_iter != Settings.end(); setting_iter ++ )
							Raptor::Game->Console.Print( setting_iter->first + ": " + setting_iter->second );
					}
				}
				
				else if( cmd == "set" )
				{
					if( elements.size() >= 3 )
						Settings[ elements.at(1) ] = elements.at(2);
					else
						Raptor::Game->Console.Print( "Usage: set <variable> <value>", TextConsole::MSG_ERROR );
				}

				else if( cmd == "unset" )
				{
					if( elements.size() >= 2 )
					{
						std::map<std::string, std::string>::iterator setting_iter = Settings.find( elements.at(1) );
						if( setting_iter != Settings.end() )
							Settings.erase( setting_iter );
						else
							Raptor::Game->Console.Print( elements.at(1) + " is not defined." );
					}
					else
						Raptor::Game->Console.Print( "Usage: unset <variable>", TextConsole::MSG_ERROR );
				}
				
				else if( cmd == "bind" )
				{
					if( elements.size() >= 3 )
					{
						uint8_t control = ControlID( elements.at(2) );
						if( control != UNBOUND )
						{
							SDLKey key = KeyID( elements.at(1) );
							Uint8 mouse = MouseID( elements.at(1) );
							
							if( key != SDLK_UNKNOWN )
								KeyBinds[ key ] = control;
							else if( mouse )
								MouseBinds[ mouse ] = control;
							else
								Raptor::Game->Console.Print( "Unknown key: " + elements.at(1), TextConsole::MSG_ERROR );
						}
						else
							Raptor::Game->Console.Print( "Unknown command: " + elements.at(2), TextConsole::MSG_ERROR );
					}
					else
						Raptor::Game->Console.Print( "Usage: bind <input> <command>", TextConsole::MSG_ERROR );
				}
				
				else if( cmd == "unbind" )
				{
					if( elements.size() >= 2 )
					{
						SDLKey key = KeyID( elements.at(1) );
						if( key != SDLK_UNKNOWN )
							KeyBinds[ key ] = UNBOUND;
						else
						{
							Uint8 mouse = MouseID( elements.at(1) );
							if( mouse )
								MouseBinds[ mouse ] = UNBOUND;
						}
					}
					else
						Raptor::Game->Console.Print( "Usage: unbind <input>", TextConsole::MSG_ERROR );
				}
				
				else if( cmd == "unbindall" )
				{
					UnbindAll();
				}
				
				else if( cmd == "exec" )
				{
					if( elements.size() >= 2 )
						Load( elements.at(1) );
					else
						Raptor::Game->Console.Print( "Usage: exec <file>", TextConsole::MSG_ERROR );
				}
				
				else if( cmd == "export" )
				{
					if( elements.size() >= 2 )
						Save( elements.at(1) );
					else
						Raptor::Game->Console.Print( "Usage: export <file>", TextConsole::MSG_ERROR );
				}
				
				else if( cmd == "g_restart" )
				{
					Raptor::Game->Gfx.Restart();
				}
				
				else if( cmd == "g_shader_restart" )
				{
					Raptor::Game->ShaderMgr.LoadShaders( SettingAsString("g_shader_file") );
				}
				
				else if( cmd == "s_reload" )
				{
					Raptor::Game->Snd.StopSounds();
					Raptor::Game->Res.DeleteSounds();
				}
				
				else if( cmd == "music" )
				{
					if( elements.size() >= 2 )
					{
						Mix_Music *music = Raptor::Game->Res.GetMusic(elements.at(1));
						if( music )
							Raptor::Game->Snd.PlayMusicOnce( music );
						else
							Raptor::Game->Console.Print( "Couldn't find music: " + elements.at(1), TextConsole::MSG_ERROR );
					}
				}
				
				else if( cmd == "sound" )
				{
					if( elements.size() >= 2 )
					{
						Mix_Chunk *sound = Raptor::Game->Res.GetSound(elements.at(1));
						if( sound )
							Raptor::Game->Snd.Play( sound );
						else
							Raptor::Game->Console.Print( "Couldn't find sound: " + elements.at(1), TextConsole::MSG_ERROR );
					}
				}
				
				else if( cmd == "connect" )
				{
					if( elements.size() >= 2 )
						Raptor::Game->Net.Connect( elements.at(1).c_str(), SettingAsString("name").c_str(), SettingAsString("password").c_str() );
					else
						Raptor::Game->Console.Print( "Usage: connect <ip>", TextConsole::MSG_ERROR );
				}
				
				else if( cmd == "disconnect" )
				{
					if( Raptor::Game->Net.Connected )
						Raptor::Game->Net.Disconnect();
					else
						Raptor::Game->Console.Print( "Not connected." );
				}
				
				else if( cmd == "reconnect" )
				{
					if( HasSetting("host_address") )
						Raptor::Game->Net.Connect( SettingAsString("host_address").c_str(), SettingAsString("name").c_str(), SettingAsString("password").c_str() );
					else
						Raptor::Game->Console.Print( "No recent server to connect to." );
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
					snprintf( cstr, 1024, "Architecture: %i-bit %s Endian", (int) sizeof(void*) * 8, Endian::Big() ? "Big" : "Little" );
					Raptor::Game->Console.Print( cstr );
					
					Raptor::Game->Console.Print( Raptor::Game->Keys.Status() );
					Raptor::Game->Console.Print( Raptor::Game->Mouse.Status() );
					Raptor::Game->Console.Print( Raptor::Game->Joy.Status() );
					Raptor::Game->Console.Print( Raptor::Game->Net.Status() );
					
					snprintf( cstr, 1024, "Players: %i", (int) Raptor::Game->Data.Players.size() );
					Raptor::Game->Console.Print( cstr );
					snprintf( cstr, 1024, "Objects: %i", (int) Raptor::Game->Data.GameObjects.size() );
					Raptor::Game->Console.Print( cstr );
					snprintf( cstr, 1024, "Shaders: %s", Raptor::Game->ShaderMgr.Ready() ? "OK" : "FAILED" );
					Raptor::Game->Console.Print( cstr );
					snprintf( cstr, 1024, "Camera Facing: %f %f %f", Raptor::Game->Cam.Fwd.X, Raptor::Game->Cam.Fwd.Y, Raptor::Game->Cam.Fwd.Z );
					Raptor::Game->Console.Print( cstr );
					snprintf( cstr, 1024, "FPS: %.0f", 1. / Raptor::Game->FrameTime );
					Raptor::Game->Console.Print( cstr );
					
					if( Raptor::Server->IsRunning() )
					{
						snprintf( cstr, 1024, "Server FPS: %.0f", 1. / Raptor::Server->FrameTime );
						Raptor::Game->Console.Print( cstr );
					}
				}
				
				else if( cmd == "model_info" )
				{
					if( elements.size() >= 2 )
					{
						Model *model = Raptor::Game->Res.GetModel(elements.at(1));
						char cstr[ 1024 ] = "";
						snprintf( cstr, 1024, "%s: %i materials, %i objects, %i arrays, %i triangles, %.1f x %.1f x %.1f", elements.at(1).c_str(), (int) model->Materials.size(), (int) model->Objects.size(), model->ArrayCount(), model->TriangleCount(), model->GetLength(), model->GetWidth(), model->GetHeight() );
						Raptor::Game->Console.Print( cstr );
					}
					else
						Raptor::Game->Console.Print( "Usage: model_info <file>", TextConsole::MSG_ERROR );
				}
				
				else if( cmd == "name" )
				{
					if( elements.size() >= 2 )
					{
						Settings[ "name" ] = elements.at(1);
						
						Packet player_properties = Packet( Raptor::Packet::PLAYER_PROPERTIES );
						player_properties.AddUShort( Raptor::Game->PlayerID );
						player_properties.AddUInt( 1 );
						player_properties.AddString( "name" );
						player_properties.AddString( elements.at(1) );
						Raptor::Game->Net.Send( &player_properties );
					}
					else
						Raptor::Game->Console.Print( "Usage: name <name>", TextConsole::MSG_ERROR );
				}
				
				else if( cmd == "say" )
				{
					if( !( Raptor::Game->Net.Connected && Raptor::Game->PlayerID) )
						Raptor::Game->Console.Print( "Must be connected to chat.", TextConsole::MSG_ERROR );
					else if( elements.size() >= 2 )
					{
						Player *player = Raptor::Game->Data.GetPlayer( Raptor::Game->PlayerID );
						
						std::vector<std::string> msg_vec = elements;
						msg_vec[0] = "";
						std::string msg = Str::Join( msg_vec, " " );
						
						Packet message = Packet( Raptor::Packet::MESSAGE );
						message.AddString( ((player ? player->Name : std::string("Anonymous")) + std::string(": ") + msg).c_str() );
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
					else if( elements.size() >= 2 )
					{
						std::string sv_cmd = elements.at(1);
						
						if( sv_cmd == "set" )
						{
							if( elements.size() >= 4 )
							{
								Packet info = Packet( Raptor::Packet::INFO );
								info.AddUShort( 1 );
								info.AddString( elements.at(2) );
								info.AddString( elements.at(3) );
								Raptor::Game->Net.Send( &info );
							}
							else
								Raptor::Game->Console.Print( "Usage: rcon set <variable> <value>", TextConsole::MSG_ERROR );
						}
						else if( sv_cmd == "show" )
						{
							if( elements.size() >= 3 )
							{
								int count = 0;
								const char *cstr = elements.at(2).c_str();
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
									Raptor::Game->Console.Print( elements.at(2) + " is not defined." );
							}
							else
							{
								for( std::map<std::string, std::string>::iterator setting_iter = Raptor::Game->Data.Properties.begin(); setting_iter != Raptor::Game->Data.Properties.end(); setting_iter ++ )
									Raptor::Game->Console.Print( setting_iter->first + ": " + setting_iter->second );
							}
						}
						else if( Raptor::Game->Data.Properties.find( sv_cmd ) != Raptor::Game->Data.Properties.end() )
						{
							if( elements.size() >= 3 )
							{
								Packet info = Packet( Raptor::Packet::INFO );
								info.AddUShort( 1 );
								info.AddString( sv_cmd );
								info.AddString( elements.at(2) );
								Raptor::Game->Net.Send( &info );
							}
							else
								Raptor::Game->Console.Print( sv_cmd + ": " + Raptor::Server->Data.Properties[ sv_cmd ] );
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
					else if( elements.size() >= 2 )
					{
						std::string sv_cmd = elements.at(1);
						
						if( sv_cmd == "set" )
						{
							if( elements.size() >= 4 )
							{
								Raptor::Server->Data.Properties[ elements.at(2) ] = elements.at(3);
								
								Packet info = Packet( Raptor::Packet::INFO );
								info.AddUShort( 1 );
								info.AddString( elements.at(2) );
								info.AddString( elements.at(3) );
								Raptor::Server->Net.SendAll( &info );
							}
							else
								Raptor::Game->Console.Print( "Usage: sv set <variable> <value>", TextConsole::MSG_ERROR );
						}
						else if( sv_cmd == "show" )
						{
							if( elements.size() >= 3 )
							{
								int count = 0;
								const char *cstr = elements.at(2).c_str();
								int len = strlen(cstr);
								for( std::map<std::string, std::string>::iterator property_iter = Raptor::Server->Data.Properties.begin(); property_iter != Raptor::Server->Data.Properties.end(); property_iter ++ )
								{
									if( strncmp( property_iter->first.c_str(), cstr, len ) == 0 )
									{
										Raptor::Game->Console.Print( property_iter->first + ": " + property_iter->second );
										count ++;
									}
								}
								if( ! count )
									Raptor::Game->Console.Print( elements.at(2) + " is not defined." );
							}
							else
							{
								for( std::map<std::string, std::string>::iterator setting_iter = Raptor::Server->Data.Properties.begin(); setting_iter != Raptor::Server->Data.Properties.end(); setting_iter ++ )
									Raptor::Game->Console.Print( setting_iter->first + ": " + setting_iter->second );
							}
						}
						else if( sv_cmd == "port" )
						{
							if( elements.size() >= 3 )
								Settings["sv_port"] = elements.at(2);
							else
							{
								Raptor::Game->Console.Print( std::string("Server port: ") + Num::ToString( Raptor::Game->Server->Port ) );
								int sv_port = Raptor::Game->Cfg.SettingAsInt( "sv_port", 7000 );
								if( sv_port != Raptor::Game->Server->Port )
									Raptor::Game->Console.Print( std::string("(Will be ") + Num::ToString(sv_port) + std::string(" after restart.)") );
							}
						}
						else if( sv_cmd == "netrate" )
						{
							if( elements.size() >= 3 )
							{
								Settings["sv_netrate"] = elements.at(2);
								Raptor::Server->NetRate = SettingAsDouble( "sv_netrate", 30. );
								Raptor::Server->Net.SetNetRate( Raptor::Server->NetRate );
							}
							else
								Raptor::Game->Console.Print( std::string("Server netrate: ") + Num::ToString( (int)( Raptor::Game->Server->Net.NetRate + 0.5 ) ) );
						}
						else if( sv_cmd == "maxfps" )
						{
							if( elements.size() >= 3 )
							{
								Settings["sv_maxfps"] = elements.at(2);
								Raptor::Server->MaxFPS = SettingAsDouble( "sv_maxfps", 120. );
							}
							else
								Raptor::Game->Console.Print( std::string("Server maxfps: ") + Num::ToString( (int)( Raptor::Game->Server->MaxFPS + 0.5 ) ) );
						}
						else if( sv_cmd == "use_out_threads" )
						{
							if( elements.size() >= 3 )
							{
								Settings["sv_use_out_threads"] = elements.at(2);
								Raptor::Server->UseOutThreads = SettingAsBool( "use_out_threads", true );
								Raptor::Server->Net.UseOutThreads = Raptor::Server->UseOutThreads;
							}
							else
								Raptor::Game->Console.Print( std::string("Server use_out_threads: ") + (Raptor::Game->Server->Net.UseOutThreads ? "true" : "false") );
						}
						else if( sv_cmd == "restart" )
						{
							Raptor::Server->Port = Raptor::Game->Cfg.SettingAsInt( "sv_port", 7000 );
							Raptor::Server->NetRate = Raptor::Game->Cfg.SettingAsDouble( "sv_netrate", 30. );
							Raptor::Server->MaxFPS = Raptor::Game->Cfg.SettingAsDouble( "sv_maxfps", 120. );
							Raptor::Server->UseOutThreads = Raptor::Game->Cfg.SettingAsBool( "sv_out_threads", true );
							
							Raptor::Server->Start( Raptor::Game->Cfg.SettingAsString("name") );
						}
						else if( sv_cmd == "status" )
						{
							char cstr[ 1024 ] = "";
							
							Raptor::Game->Console.Print( Raptor::Game->Game + " " + Raptor::Game->Version );
							snprintf( cstr, 1024, "Architecture: %i-bit %s Endian", (int) sizeof(void*) * 8, Endian::Big() ? "Big" : "Little" );
							Raptor::Game->Console.Print( cstr );
							
							snprintf( cstr, 1024, "Players: %i", (int) Raptor::Server->Data.Players.size() );
							Raptor::Game->Console.Print( cstr );
							snprintf( cstr, 1024, "Objects: %i", (int) Raptor::Server->Data.GameObjects.size() );
							Raptor::Game->Console.Print( cstr );
							snprintf( cstr, 1024, "Server FPS: %.0f", 1. / Raptor::Server->FrameTime );
							Raptor::Game->Console.Print( cstr );
						}
						else if( sv_cmd == "say" )
						{
							if( elements.size() >= 3 )
							{
								std::vector<std::string> msg_vec = elements;
								msg_vec[0] = "";
								msg_vec[1] = "";
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
							if( elements.size() >= 3 )
							{
								int new_state = Raptor::Server->State;
								if( elements.at(2) == "++" )
									new_state ++;
								else if( elements.at(2) == "--" )
									new_state --;
								else if( (elements.at(2) == "+=") && (elements.size() >= 4) )
									new_state += atoi( elements.at(3).c_str() );
								else if( (elements.at(2) == "-=") && (elements.size() >= 4) )
									new_state -= atoi( elements.at(3).c_str() );
								else
									new_state = atoi( elements.at(2).c_str() );
								
								char cstr[ 1024 ] = "";
								snprintf( cstr, 1024, "Changing server from state %i to %i.", Raptor::Server->State, new_state );
								Raptor::Game->Console.Print( cstr );
								
								Raptor::Server->ChangeState( new_state );
							}
							else
							{
								char cstr[ 1024 ] = "";
								snprintf( cstr, 1024, "Current server state: %i", Raptor::Server->State );
								Raptor::Game->Console.Print( cstr );
							}
						}
						else if( Raptor::Server->Data.Properties.find( sv_cmd ) != Raptor::Server->Data.Properties.end() )
						{
							if( elements.size() >= 3 )
							{
								Raptor::Server->Data.Properties[ sv_cmd ] = elements.at(2);
								
								Packet info = Packet( Raptor::Packet::INFO );
								info.AddUShort( 1 );
								info.AddString( sv_cmd );
								info.AddString( elements.at(2) );
								Raptor::Server->Net.SendAll( &info );
							}
							else
								Raptor::Game->Console.Print( sv_cmd + ": " + Raptor::Server->Data.Properties[ sv_cmd ] );
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
						if( elements.size() >= 2 )
							Settings[ cmd ] = elements.at(1);
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
		fprintf( stderr, "ClientConfig::Command: std::out_of_range\n" );
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
			input.getline( buffer, 1024 );
			Command( std::string(buffer) );
		}
		
		input.close();
	}
}


void ClientConfig::Save( std::string filename )
{
	FILE *output = fopen( filename.c_str(), "wt" );
	if( output )
	{
		for( std::map<std::string, std::string>::iterator setting_iter = Settings.begin(); setting_iter != Settings.end(); setting_iter ++ )
			fprintf( output, "set \"%s\" \"%s\"\n", Str::Escape( setting_iter->first, ORIGINAL, ESCAPED ).c_str(), Str::Escape( setting_iter->second, ORIGINAL, ESCAPED ).c_str() );
		
		fprintf( output, "\nunbindall\n" );
		for( std::map<Uint8, uint8_t>::iterator mouse_iter = MouseBinds.begin(); mouse_iter != MouseBinds.end(); mouse_iter ++ )
			fprintf( output, "bind \"%s\" \"%s\"\n", Str::Escape( MouseName(mouse_iter->first), ORIGINAL, ESCAPED ).c_str(), Str::Escape( ControlName(mouse_iter->second), ORIGINAL, ESCAPED ).c_str() );
		for( std::map<SDLKey, uint8_t>::iterator key_iter = KeyBinds.begin(); key_iter != KeyBinds.end(); key_iter ++ )
			fprintf( output, "bind \"%s\" \"%s\"\n", Str::Escape( KeyName(key_iter->first), ORIGINAL, ESCAPED ).c_str(), Str::Escape( ControlName(key_iter->second), ORIGINAL, ESCAPED ).c_str() );
		
		
		fflush( output );
		fclose( output );
		output = NULL;
	}
}


// ---------------------------------------------------------------------------


bool ClientConfig::HasSetting( std::string name )
{
	std::map<std::string, std::string>::iterator found = Settings.find( name );
	return ( found != Settings.end() );
}


std::string ClientConfig::SettingAsString( std::string name, const char *ifndef )
{
	std::map<std::string, std::string>::iterator found = Settings.find( name );
	if( found != Settings.end() )
		return found->second;
	if( ifndef )
		return std::string(ifndef);
	return "";
}


double ClientConfig::SettingAsDouble( std::string name, double ifndef )
{
	std::map<std::string, std::string>::iterator found = Settings.find( name );
	if( found != Settings.end() )
		return Str::AsDouble( found->second );
	return ifndef;
}


int ClientConfig::SettingAsInt( std::string name, int ifndef )
{
	std::map<std::string, std::string>::iterator found = Settings.find( name );
	if( found != Settings.end() )
		return Str::AsInt( found->second );
	return ifndef;
}


bool ClientConfig::SettingAsBool( std::string name, bool ifndef )
{
	std::map<std::string, std::string>::iterator found = Settings.find( name );
	if( found != Settings.end() )
		return Str::AsBool( found->second );
	return ifndef;
}


// ---------------------------------------------------------------------------


bool ClientConfig::Bind( SDL_Event *event, uint8_t control )
{
	if( event->type == SDL_KEYDOWN )
	{
		KeyBinds[ event->key.keysym.sym ] = control;
		return true;
	}
	else if( event->type == SDL_MOUSEBUTTONDOWN )
	{
		MouseBinds[ event->button.button ] = control;
		return true;
	}
	
	// Return false if the event type could not be bound.
	return false;
}


void ClientConfig::Unbind( uint8_t control )
{
	const std::map<SDLKey, uint8_t>::iterator key_binds_end = KeyBinds.end();
	for( std::map<SDLKey, uint8_t>::iterator iter = KeyBinds.begin(); iter != key_binds_end; )
	{
		std::map<SDLKey, uint8_t>::iterator next = iter;
		next ++;
		
		if( iter->second == control )
			KeyBinds.erase( iter );
		
		iter = next;
	}
	
	const std::map<Uint8, uint8_t>::iterator mouse_binds_end = MouseBinds.end();
	for( std::map<Uint8, uint8_t>::iterator iter = MouseBinds.begin(); iter != mouse_binds_end; )
	{
		std::map<Uint8, uint8_t>::iterator next = iter;
		next ++;
		
		if( iter->second == control )
			MouseBinds.erase( iter );
		
		iter = next;
	}
}


void ClientConfig::UnbindAll( void )
{
	KeyBinds.clear();
	MouseBinds.clear();
}


// ---------------------------------------------------------------------------


uint8_t ClientConfig::GetControl( SDL_Event *event )
{
	if( (event->type == SDL_KEYDOWN) || (event->type == SDL_KEYUP) )
	{
		std::map<SDLKey, uint8_t>::iterator iter = KeyBinds.find( event->key.keysym.sym );
		if( iter != KeyBinds.end() )
			return iter->second;
	}
	
	else if( (event->type == SDL_MOUSEBUTTONDOWN) || (event->type == SDL_MOUSEBUTTONUP) )
	{
		std::map<Uint8, uint8_t>::iterator iter = MouseBinds.find( event->button.button );
		if( iter != MouseBinds.end() )
			return iter->second;
	}
	
	// Return 0 if the event was not bound.
	return UNBOUND;
}


// ---------------------------------------------------------------------------


std::string ClientConfig::ControlName( uint8_t control )
{
	std::map<uint8_t, std::string>::iterator iter = ControlNames.find( control );
	if( iter != ControlNames.end() )
		return iter->second;
	
	char control_string[ 32 ] = "";
	snprintf( control_string, 32, "Control%i", control );
	return std::string( control_string );
}


std::string ClientConfig::KeyName( SDLKey key )
{
	std::map<SDLKey, std::string>::iterator iter = KeyNames.find( key );
	if( iter != KeyNames.end() )
		return iter->second;
	
	char key_string[ 32 ] = "";
	snprintf( key_string, 32, "Key%i", key );
	return std::string( key_string );
}


std::string ClientConfig::MouseName( Uint8 mouse )
{
	std::map<Uint8, std::string>::iterator iter = MouseNames.find( mouse );
	if( iter != MouseNames.end() )
		return iter->second;
	
	char mouse_string[ 32 ] = "";
	if( mouse > 5 )
		snprintf( mouse_string, 32, "Mouse%i", mouse - 2 );
	else
		snprintf( mouse_string, 32, "MouseUnknown" );
	return std::string( mouse_string );
}


// ---------------------------------------------------------------------------


uint8_t ClientConfig::ControlID( std::string name )
{
	for( std::map<uint8_t, std::string>::iterator iter = ControlNames.begin(); iter != ControlNames.end(); iter ++ )
	{
		if( iter->second == name )
			return iter->first;
	}
	
	for( std::map<uint8_t, std::string>::iterator iter = ControlNames.begin(); iter != ControlNames.end(); iter ++ )
	{
		if( ControlName(iter->first) == name )
			return iter->first;
	}
	
	return UNBOUND;
}


SDLKey ClientConfig::KeyID( std::string name )
{
	for( std::map<SDLKey, std::string>::iterator iter = KeyNames.begin(); iter != KeyNames.end(); iter ++ )
	{
		if( iter->second == name )
			return iter->first;
	}
	
	for( std::map<SDLKey, std::string>::iterator iter = KeyNames.begin(); iter != KeyNames.end(); iter ++ )
	{
		if( KeyName(iter->first) == name )
			return iter->first;
	}
	
	return SDLK_UNKNOWN;
}


Uint8 ClientConfig::MouseID( std::string name )
{
	for( std::map<Uint8, std::string>::iterator iter = MouseNames.begin(); iter != MouseNames.end(); iter ++ )
	{
		if( iter->second == name )
			return iter->first;
	}
	
	for( std::map<Uint8, std::string>::iterator iter = MouseNames.begin(); iter != MouseNames.end(); iter ++ )
	{
		if( MouseName(iter->first) == name )
			return iter->first;
	}
	
	return 0;
}

