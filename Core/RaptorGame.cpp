/*
 *  RaptorGame.cpp
 */

#define RAPTORGAME_CPP

#include "PlatformSpecific.h"

#include <cstdio>

#ifdef WIN32
#include <tchar.h>
#include <sys/stat.h>
#include <float.h>
#include <winbase.h>
#endif

#if defined(__APPLE__) && (__GNUC__ >= 4) && ((__GNUC__ > 4) || (__GNUC_MINOR__ > 0))
#include <libproc.h>
#include <unistd.h>
#include <string>
#endif

class RaptorGame;

namespace Raptor
{
	// Global pointer to the game engine object.
	RaptorGame *Game = NULL;
	
	void Terminate( int arg );
	void BrokenPipe( int arg );
	
	bool PreMain( int bits );
}

bool Raptor::PreMain( int bits )
{
	#ifdef WIN32
		TCHAR path[MAX_PATH+20] = L"";
		TCHAR *path_append = path;
		
		#ifndef _DEBUG
			// Get the path to the executable.
			GetModuleFileName( 0, path, MAX_PATH+20 );
			path_append = _tcsrchr( path, L'\\' );
			if( path_append )
				path_append ++;
			else
				path_append = path;
			path_append[ 0 ] = L'\0';
			
			// Set executable path as the working directory (fixes drag-and-drop).
			SetCurrentDirectory( path );
		#endif
		
		if( bits )
		{
			// Set the DLL directory to Bin32/Bin64 (or ..\BinXX if that exists instead).
			TCHAR bin_dir[ 16 ] = L"Bin32";
			_stprintf( bin_dir, L"Bin%i", bits );
			_stprintf( path_append, L"%ls", bin_dir );
			struct _stat stat_buffer;
			if( (_tstat( path, &stat_buffer ) == 0) && (stat_buffer.st_mode & S_IFDIR) )
				SetDllDirectory( path );
			else
			{
				_stprintf( path_append, L"..\\%ls", bin_dir );
				if( (_tstat( path, &stat_buffer ) == 0) && (stat_buffer.st_mode & S_IFDIR) )
					SetDllDirectory( path );
			}
		}
		
		#ifdef _DEBUG
			// Enable floating-point exceptions in debug mode.
			_clearfp();
			unsigned int state = _controlfp( 0, 0 );
			_controlfp( state & ~(_EM_INVALID|_EM_ZERODIVIDE), _MCW_EM );
		#endif
		
		// Prevent Windows DPI scaling from stupidly stretching things off-screen.
		#ifndef _MSC_VER
			FARPROC SetProcessDPIAware = GetProcAddress( LoadLibraryA("user32.dll"), "SetProcessDPIAware" );
			if( SetProcessDPIAware )
		#endif
		SetProcessDPIAware();
	#endif
	
	#if defined(__APPLE__) && defined(PROC_PIDPATHINFO_MAXSIZE) && !defined(_DEBUG)
		// Fix the working directory for Mac OS X Intel64.
		char exe_path[ PROC_PIDPATHINFO_MAXSIZE ] = "";
		pid_t pid = getpid();
		if( proc_pidpath( pid, exe_path, sizeof(exe_path) ) > 0 )
		{
			char *real_path = realpath( exe_path, NULL );
			std::string path;
			if( real_path )
			{
				path = real_path;
				free( real_path );
				real_path = NULL;
			}
			else
				path = exe_path;
			
			size_t index = path.rfind( ".app/Contents/MacOS/" );
			if( index != std::string::npos )
			{
				path = path.substr( 0, index );
				index = path.rfind( "/" );
				if( index != std::string::npos )
				{
					path = path.substr( 0, index );
					chdir( path.c_str() );
				}
			}
		}
	#endif
	
	return true;
}

// ---------------------------------------------------------------------------

#include "RaptorGame.h"

#include <cmath>
#include "RaptorDefs.h"
#include "Rand.h"
#include "Num.h"
#include "Notification.h"


RaptorGame::RaptorGame( std::string game, std::string version, RaptorServer *server )
{
	Game = game;
	Version = version;
	DefaultPort = 7000;
	
	SetServer( server );
	
	// This arbitrary large value allows us to use Data.AddObject for client-side non-networked objects, if we want to.
	Data.GameObjectIDs.Clear( 0x10000000 );
	
	MaxFPS = 0.;
	FrameTime = 0.;
	State = Raptor::State::DISCONNECTED;
	PlayerID = 0;
	WaitFont = NULL;
	ReadKeyboard = true;
	ReadMouse = true;
}


RaptorGame::~RaptorGame()
{
}


void RaptorGame::SetServer( RaptorServer *server )
{
	Server = server;
	if( Server )
	{
		Server->Console = &Console;
		Server->Port = DefaultPort;
	}
}


void RaptorGame::SetDefaultJoyTypes( void )
{
	Input.ResetDeviceTypes();
}


void RaptorGame::SetDefaultControls( void )
{
	Cfg.UnbindAll();
}


void RaptorGame::SetDefaults( void )
{
	Cfg.SetDefaults();
}


void RaptorGame::Initialize( int argc, char **argv )
{
	// Seed the random number generator.
	Rand::Seed( time(NULL) );
	
	
	// Set defaults, including anything game-specific, then load cfg files.
	
	Cfg.Command( "version" );
	
	SetDefaults();
	SetDefaultControls();
	
	Cfg.Load( "settings.cfg" );
	Cfg.Load( "autoexec.cfg" );
	
	
	// Check for command-line options.
	
	char *connect = NULL;
	bool host = false;
	bool screensaver = false;
	
	for( int i = 1; i < argc; i ++ )
	{
		if( (i + 2 < argc) && (strcmp( argv[ i ], "-set" ) == 0) )
		{
			Cfg.Settings[ argv[ i + 1 ] ] = argv[ i + 2 ];
			i += 2;
		}
		else if( (i + 1 < argc) && (strcmp( argv[ i ], "-name" ) == 0) )
		{
			Cfg.Settings[ "name" ] = argv[ i + 1 ];
			i ++;
		}
		else if( (i + 1 < argc) && (strcmp( argv[ i ], "-password" ) == 0) )
		{
			Cfg.Settings[ "password" ] = argv[ i + 1 ];
			i ++;
		}
		else if( (i + 1 < argc) && (strcmp( argv[ i ], "-sv_port" ) == 0) )
		{
			Cfg.Settings[ "sv_port" ] = argv[ i + 1 ];
			if( Server )
				Server->Port = atoi( argv[ i + 1 ] );
			i ++;
		}
		else if( (i + 1 < argc) && (strcmp( argv[ i ], "-connect" ) == 0) )
		{
			connect = argv[ i + 1 ];
			i ++;
		}
		else if( strcmp( argv[ i ], "-host" ) == 0 )
		{
			host = true;
		}
		else if( strcmp( argv[ i ], "-windowed" ) == 0 )
		{
			Cfg.Settings[ "g_fullscreen" ] = "false";
		}
		else if( strcmp( argv[ i ], "-safe" ) == 0 )
		{
			Cfg.Settings[ "g_framebuffers" ] = "false";
			Cfg.Settings[ "g_shader_enable" ] = "false";
			Cfg.Settings[ "g_texture_maxres" ] = "128";
			Cfg.Settings[ "g_res_fullscreen_x" ] = "640";
			Cfg.Settings[ "g_res_fullscreen_y" ] = "480";
			Cfg.Settings[ "vr_enable" ] = "false";
			#ifdef WIN32
				Cfg.Settings[ "saitek_enable" ] = "false";
			#endif
		}
		else if( strcmp( argv[ i ], "-screensaver" ) == 0 )
		{
			screensaver = true;
			Cfg.Settings[ "screensaver" ] = "true";
			Cfg.Settings[ "g_fullscreen" ] = "true";
			Cfg.Settings[ "g_res_fullscreen_x" ] = "0";
			Cfg.Settings[ "g_res_fullscreen_y" ] = "0";
			Cfg.Settings[ "s_volume" ] = "0";
			Cfg.Settings[ "vr_enable" ] = "false";
		}
	}
	
	
	if( Server )
	{
		int sv_port = Cfg.SettingAsInt( "sv_port", DefaultPort, DefaultPort );
		if( sv_port > 0 )
			Server->Port = sv_port;
		Server->NetRate = Cfg.SettingAsInt( "sv_netrate", 30 );
		Server->MaxFPS  = Cfg.SettingAsInt( "sv_maxfps",  60 );
		Server->Data.ThreadCount = Cfg.SettingAsInt("sv_threads");
	}
	
	// Dedicated servers bail out here.
	if( (argc >= 2) && (strcmp( argv[ 1 ], "-dedicated" ) == 0) )
		return;
	
	
	// Initialize SDL and subsystems.
	
	Gfx.Initialize();
	
	Console.Initialize();
	
	Snd.Initialize( Cfg.SettingAsInt("s_channels",2), Cfg.SettingAsInt("s_rate",44100), Cfg.SettingAsInt("s_depth",16), Cfg.SettingAsInt("s_buffer",4096), Cfg.SettingAsInt("s_mix_channels",64) );
	Snd.MasterVolume = Cfg.SettingAsDouble( "s_volume", 0.5 );
	Snd.SoundVolume = Cfg.SettingAsDouble( "s_effect_volume", 0.5 );
	Snd.MusicVolume = Cfg.SettingAsDouble( "s_music_volume", 1. );
	
	ShaderMgr.Initialize();
	
	Joy.Initialize();
	
	Net.Initialize( 30.0 );
	
	Mouse.SetCursor( Res.GetAnimation("cursor.ani"), 16, 1, 1 );
	
	
	// Configure signal callbacks.
	
	signal( SIGTERM, &(Raptor::Terminate) );
#ifdef SIGPIPE
	signal( SIGPIPE, &(Raptor::BrokenPipe) );
#endif
	
	
	// Run game-specific Setup method.
	
	Setup( argc, argv );
	
	
	// Start Saitek DirectOutput if enabled.
	
	#ifdef WIN32
		if( Cfg.SettingAsBool( "saitek_enable", true ) )
			Saitek.Initialize();
	#endif
	
	
	// Finish handling command-line options that had to wait for initialization.
	
	if( connect )
		Net.Connect( connect, Cfg.Settings[ "name" ].c_str(), Cfg.Settings[ "password" ].c_str() );
	if( host )
		Host();
	if( screensaver )
	{
		if( Server )
			Server->Announce = false;
		Host();
	}
}


void RaptorGame::Run( void )
{
	Clock GameClock;
	Clock NetClock;
	
	while( Layers.Layers.size() )
	{
		// Process network input buffer.
		Net.ProcessIn();
		
		// Calculate the time elapsed for the frame.
		double elapsed = GameClock.ElapsedSeconds();
		if( MaxFPS && (elapsed > 0.) && (elapsed < 1. / MaxFPS) )
			elapsed = 1. / MaxFPS;
		
		if( elapsed > 0. )
		{
			FrameTime = elapsed;
			GameClock.Advance( FrameTime );
			
			// Keep the list of joysticks up-to-date.
			Joy.FindJoysticks();
			
			// Handle all user input.
			SDL_Event event;
			memset( &event, 0, sizeof(event) );
			while( SDL_PollEvent( &event ) )
			{
				// Keep track of input and mouse position events.
				Mouse.TrackEvent( &event );
				Keys.TrackEvent( &event );
				Joy.TrackEvent( &event );
				Input.Update();
				Console.TrackEvent( &event );
				Layers.TrackEvent( &event );
				
				// Pass the event through the layer stack.
				bool handled = false;
				if( ! handled )
					handled = Console.HandleEvent( &event );
				if( ! handled )
					handled = Layers.HandleEvent( &event );
				if( ! handled )
					handled = HandleEvent( &event );
				if( ! handled )
				{
					if( event.type == SDL_QUIT )
						Quit();
#if SDL_VERSION_ATLEAST(2,0,0)
					else if( event.type == SDL_WINDOWEVENT_RESIZED )
						Gfx.SetMode( event.window.data1, event.window.data2 );
#else
					else if( event.type == SDL_VIDEORESIZE )
						Gfx.SetMode( event.resize.w, event.resize.h );
#endif
					else if( (event.type == SDL_KEYUP) && (event.key.keysym.sym == Console.ToggleKey) )
						Console.ToggleActive();
				}
			}
			
			// Update active panning sounds and continue music playlist if applicable.
			Snd.MasterVolume = Cfg.SettingAsDouble( "s_volume", 0.5 );
			Snd.SoundVolume = Cfg.SettingAsDouble( "s_effect_volume", 0.5 );
			Snd.MusicVolume = Cfg.SettingAsDouble( "s_music_volume", 1. );
			Snd.Update( &Cam );
			
			// Honor the maxfps variable.
			MaxFPS = Cfg.SettingAsDouble("maxfps");
			
			// If we're waiting to reconnect, try when it's time.
			if( (Net.ReconnectTime > 0) && (Net.ReconnectClock.ElapsedSeconds() > Net.ReconnectTime) )
				Net.Reconnect( Cfg.SettingAsString("name").c_str(), Cfg.SettingAsString("password").c_str() );
			
			// Draw to all viewports.
			bool vr_enable = Cfg.SettingAsBool("vr_enable");
			if( vr_enable && ! Head.Initialized )
				Head.Initialize();
			if( Head.VR && vr_enable )
			{
				// If the VR viewport is larger than the window, center the mouse area.
				int x = (Head.EyeR->W - Raptor::Game->Gfx.RealW) / 2;
				int y = (Head.EyeR->H - Raptor::Game->Gfx.RealH) / 2;
				Mouse.SetOffset( std::max<int>(0,x), std::max<int>(0,y) );
				
				// Draw all layers to each eye.
				Head.Draw();
				
				if( Cfg.SettingAsBool("vr_mirror",true) )
				{
					// Mirror the right eye to the screen.
					// This can be disabled to avoid monitor vsync slowdown.
					Gfx.Setup2D();
					Gfx.Clear();
					float x1 = x / (float) Head.EyeR->W;
					float y1 = y / (float) Head.EyeR->H;
					float x2 = (Head.EyeR->W - x) / (float) Head.EyeR->W;
					float y2 = (Head.EyeR->H - y) / (float) Head.EyeR->H;
					glColor4f( 1.f, 1.f, 1.f, 1.f );
					glEnable( GL_TEXTURE_2D );
					glBindTexture( GL_TEXTURE_2D, Head.EyeR->Texture );
					glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
					glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
					glBegin( GL_QUADS );
						glTexCoord2f( x1, y2 );
						glVertex2d( 0, 0 );
						glTexCoord2f( x1, y1 );
						glVertex2d( 0, Gfx.RealH );
						glTexCoord2f( x2, y1 );
						glVertex2d( Gfx.RealW, Gfx.RealH );
						glTexCoord2f( x2, y2 );
						glVertex2d( Gfx.RealW, 0 );
					glEnd();
					glDisable( GL_TEXTURE_2D );
					Gfx.SwapBuffers();
				}
			}
			else
			{
				// VR failed to start.
				if( vr_enable )
					Cfg.Settings[ "vr_enable" ] = "false";
				
				// Mouse is aligned with the screen.
				Mouse.SetOffset(0,0);
				
				// Draw all layers.
				Draw();
				
				// Swap front and back framebuffers to update the screen.
				Gfx.SwapBuffers();
			}
			
			// Update!  (Extrapolate object motion, etc.)
			Update( FrameTime );
			
			// If we're disconnected, make sure we clean up variables after the thread finishes.
			Net.Cleanup();
			
			// Send periodic updates to server.
			Net.NetRate = Raptor::Game->Cfg.SettingAsDouble( "netrate", 30. );
			Net.SendUpdates();
		}
		
		// Let the thread rest a bit.
		SDL_Delay( 1 );
	}
	
	// Save our settings.
	if( ! Cfg.SettingAsBool("screensaver") )
		Cfg.Save( "settings.cfg" );
	
	// Let the server know we're quitting.
	Net.DisconnectNice( NULL );
	
	// If we were hosting, stop that too.
	if( Server && Server->IsRunning() )
		Server->StopAndWait( 2. );
	
	// Cleanup for Saitek DirectOutput.
	#ifdef WIN32
		Saitek.Deinitialize();
	#endif
}


void RaptorGame::Draw( void )
{
	// Clear the framebuffer.
	Gfx.Clear();
	
	// Draw all layers.
	Layers.Draw();
	Console.Draw();
	Mouse.Draw();
	
	// Draw text overlay.
	if( WaitText.length() )
	{
		if( ! WaitFont )
			WaitFont = Raptor::Game->Res.GetFont( "Verdana.ttf", 16 );
		
		SDL_Rect rect = {0,0,0,0};
		WaitFont->TextSize( WaitText, &rect );
		Gfx.DrawRect2D( Gfx.W/2 - rect.w/2 - 30, Gfx.H/2 - rect.h/2 - 20, Gfx.W/2 + rect.w/2 + 30, Gfx.H/2 + rect.h/2 + 20, 0, 0.f,0.f,0.f,0.75f );
		
		WaitFont->DrawText( WaitText, Gfx.W/2, Gfx.H/2, Font::ALIGN_MIDDLE_CENTER );
	}
}


void RaptorGame::Setup( int argc, char **argv )
{
}


void RaptorGame::Update( double dt )
{
	Data.Update( dt );
}


bool RaptorGame::HandleEvent( SDL_Event *event )
{
#ifdef WIN32
	// FIXME: Should a signal handler be responsible for this instead?
	if( (event->type == SDL_KEYDOWN) && (event->key.keysym.sym == SDLK_F4) && (Keys.KeyDown(SDLK_LALT) || Keys.KeyDown(SDLK_RALT)) )
		Quit();
#endif
	return false;
}


bool RaptorGame::HandleCommand( std::string cmd, std::vector<std::string> *params )
{
	return false;
}


void RaptorGame::MessageReceived( std::string text, uint32_t type )
{
	Raptor::Game->Console.Print( text, type );
	Raptor::Game->Msg.Print( text, type );
}


bool RaptorGame::ProcessPacket( Packet *packet )
{
	packet->Rewind();
	PacketType type = packet->Type();
	
	if( type == Raptor::Packet::UPDATE )
	{
		// First read the precision of the update.
		int8_t precision = packet->NextChar();
		
		// Then read the number of objects.
		uint32_t obj_count = packet->NextUInt();
		
		// Loop through for each object's update data.
		while( obj_count )
		{
			obj_count --;
			
			// First read the ID.
			uint32_t obj_id = packet->NextUInt();
			
			// Look up the ID in the client-side list of objects and update it.
			std::map<uint32_t,GameObject*>::iterator obj_iter = Data.GameObjects.find( obj_id );
			if( obj_iter != Data.GameObjects.end() )
				obj_iter->second->ReadFromUpdatePacketFromServer( packet, precision );
			else
			{
				// The server thinks we should have this object, so we're out of sync!
				// This means the rest of the update packet could be misaligned, so stop trying to parse it.
				std::string sync_error = std::string("Sync error: UPDATE: missing object ") + Num::ToString((int)obj_id);
				if( Server->IsRunning() )
				{
					// If we are running the server, we can determine what kind of object the client is missing.
					const GameObject *obj = Server->Data.GetObject( obj_id );
					if( obj )
					{
						uint32_t type = obj->Type();
						char type_str[ 5 ] = {0};
						Endian::CopyBig( &type, type_str, 4 );
						sync_error += std::string(" type ") + std::string(type_str);
					}
				}
				Console.Print( sync_error, TextConsole::MSG_ERROR );
				return true;
			}
		}
		
		if( packet->Offset < packet->Size() )
			Console.Print( "Sync error: UPDATE: unparsed " + Num::ToString((int)(packet->Size() - packet->Offset)), TextConsole::MSG_ERROR );
		
		return true;
	}
	
	else if( type == Raptor::Packet::OBJECTS_ADD )
	{
		// First read the number of objects being added.
		uint32_t obj_count = packet->NextUInt();
		
		// Loop through for each object's initialization data.
		while( obj_count )
		{
			obj_count --;
			
			uint32_t id = packet->NextUInt();
			uint32_t type = packet->NextUInt();
			GameObject *obj = NewObject( id, type );
			obj->Data = &Data;  // Just in case ReadFromInitPacket checks ClientSide.
			obj->ReadFromInitPacket( packet );
			Data.AddObject( obj );
		}
		
		if( packet->Offset < packet->Size() )
			Console.Print( "Sync error: OBJECTS_ADD: unparsed " + Num::ToString((int)(packet->Size() - packet->Offset)), TextConsole::MSG_ERROR );
		
		return true;
	}
	
	else if( type == Raptor::Packet::OBJECTS_REMOVE )
	{
		// First read the number of objects being removed.
		uint32_t obj_count = packet->NextUInt();
		
		// Loop through for each object's ID and remove them.
		while( obj_count )
		{
			obj_count --;
			
			uint32_t id = packet->NextUInt();
			Data.RemoveObject( id );
		}
		
		return true;
	}
	
	else if( type == Raptor::Packet::OBJECTS_CLEAR )
	{
		Data.ClearObjects();
		
		return true;
	}
	
	else if( type == Raptor::Packet::PLAYER_ADD )
	{
		uint16_t id = packet->NextUShort();
		const char *name = packet->NextString();
		
		Player *player = NewPlayer( id );
		player->Name = name;
		Data.AddPlayer( player );
		
		return true;
	}
	
	else if( type == Raptor::Packet::PLAYER_REMOVE )
	{
		uint16_t id = packet->NextUShort();
		Data.RemovePlayer( id );
		
		return true;
	}
	
	else if( type == Raptor::Packet::PLAYER_PROPERTIES )
	{
		// Read the player ID and look them up.
		uint16_t id = packet->NextUShort();
		Player *player = Data.GetPlayer( id );
		
		// First read the number of properties.
		uint32_t property_count = packet->NextUInt();
		
		// Loop through for each property being changed.
		while( property_count )
		{
			property_count --;
			
			std::string property_name = packet->NextString();
			std::string property_value = packet->NextString();
			
			if( player )
			{
				if( property_name == "name" )
					player->Name = property_value;
				else
					player->Properties[ property_name ] = property_value;
			}
			else
			{
				// The server thinks we should have this player, so we're out of sync!
				Console.Print( "Sync error: PLAYER_PROPERTIES: missing player " + Num::ToString((int)id), TextConsole::MSG_ERROR );
			}
		}
		
		return true;
	}
	
	else if( type == Raptor::Packet::PLAYER_LIST )
	{
		// First read the number of players.
		uint16_t obj_count = packet->NextUShort();
		
		// Loop through for each player's data.
		while( obj_count )
		{
			obj_count --;
			
			uint16_t id = packet->NextUShort();
			const char *name = packet->NextString();
			
			Player *player = NULL;
			std::map<uint16_t,Player*>::iterator player_iter = Data.Players.find( id );
			if( player_iter != Data.Players.end() )
				player = player_iter->second;
			
			if( ! player )
			{
				player = NewPlayer( id );
				Data.AddPlayer( player );
			}
			
			player->Name = name ? name : "?";
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
			
			Data.SetProperty( name, value );
		}
		
		return true;
	}
	
	else if( type == Raptor::Packet::MESSAGE )
	{
		std::string message = packet->NextString();
		uint32_t msg_type = 0;
		if( packet->Remaining() )
			msg_type = packet->NextUInt();
		Raptor::Game->MessageReceived( message, msg_type );
	}
	
	else if( type == Raptor::Packet::PLAY_SOUND )
	{
		Mix_Chunk *sound = Res.GetSound( packet->NextString() );
		float music_volume = 1.f;
		float sound_volume = 1.f;
		bool attenuate = false;
		
		if( packet->Remaining() )
		{
			music_volume = Num::UnitFloatFrom8( packet->NextChar() );
			attenuate = true;
		}
		
		if( packet->Remaining() )
			sound_volume = Num::UnitFloatFrom8( packet->NextChar() );
		
		if( sound )
		{
			int channel = Snd.Play( sound );
			if( attenuate && (channel >= 0) )
			{
				Snd.AttenuateFor = channel;
				Snd.SoundAttenuate = sound_volume;
				Snd.MusicAttenuate = music_volume;
			}
		}
		
		return true;
	}
	
	else if( type == Raptor::Packet::PLAY_MUSIC )
	{
		Mix_Music *music = Res.GetMusic( packet->NextString() );
		if( music )
			Snd.PlayMusicOnce( music );
		
		return true;
	}
	
	return false;
}


void RaptorGame::SendUpdate( int8_t precision )
{
	// Before adding anything to the packet, build a list of the objects we will be sending data for.
	std::vector<GameObject*> objects_to_update;
	for( std::map<uint32_t,GameObject*>::iterator obj_iter = Data.GameObjects.begin(); obj_iter != Data.GameObjects.end(); obj_iter ++ )
	{
		if( (obj_iter->second->PlayerID == PlayerID) && obj_iter->second->PlayerShouldUpdateServer() )
			objects_to_update.push_back( obj_iter->second );
	}
	
	// If precision is auto (-128), the number of objects to update dictates how much detail to send about each.
	if( precision == -128 )
	{
		if( objects_to_update.size() < 32 )
			precision = 127;
		else if( objects_to_update.size() < 1024 )
			precision = 0;
		else
			precision = -127;
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
		(*obj_iter)->AddToUpdatePacketFromClient( &update_packet, precision );
	}
	
	// Send the packet.
	Net.Send( &update_packet );
}


bool RaptorGame::SetPlayerProperty( std::string name, std::string value )
{
	if( name == "name" )
		Cfg.Settings[ "name" ] = value;
	
	Player *player = Data.GetPlayer( PlayerID );
	if( ! player )
		return false;
	
	if( player->Properties[ name ] != value )
	{
		if( name == "name" )
			player->Name = value;
		else
			player->Properties[ name ] = value;
		
		if( State >= Raptor::State::CONNECTED )
		{
			Packet player_properties( Raptor::Packet::PLAYER_PROPERTIES );
			player_properties.AddUShort( player->ID );
			player_properties.AddUInt( 1 );
			player_properties.AddString( name );
			player_properties.AddString( value );
			Net.Send( &player_properties );
		}
		
		return true;
	}
	
	return false;
}


void RaptorGame::ChangeState( int state )
{
	WaitText.clear();
	
	if( state == Raptor::State::DISCONNECTED )
		Disconnected();
	else if( state == Raptor::State::CONNECTING )
		Connecting();
	else if( state == Raptor::State::CONNECTED )
		Connected();
	
	if( state < Raptor::State::CONNECTED )
	{
		// Clear all game data.
		Data.Clear();
		
		// Clear the message list, since it's session-specific; copies remain in the console.
		Raptor::Game->Msg.Clear();
	}
	
	State = state;
}


void RaptorGame::Disconnected( void )
{
	if( State >= Raptor::State::CONNECTED )
	{
		if( Net.DisconnectMessage.length() )
			Layers.Add( new Notification( std::string("Disconnected from server:\n\n") + Net.DisconnectMessage ) );
		else
			Layers.Add( new Notification( "Disconnected from server." ) );
	}
	else if( State == Raptor::State::CONNECTING )
	{
		if( Net.DisconnectMessage.length() )
			Layers.Add( new Notification( std::string("Failed to connect:\n\n") + Net.DisconnectMessage ) );
		else
			Layers.Add( new Notification( std::string("Failed to connect to ") + Net.Host + std::string(":") + Num::ToString(Net.Port) + std::string(".") ) );
	}
}


void RaptorGame::Connecting( void )
{
	WaitText = std::string("Connecting to ") + Net.Host + std::string(":") + Num::ToString(Net.Port) + std::string("...");
	Draw();
}


void RaptorGame::Connected( void )
{
}


GameObject *RaptorGame::NewObject( uint32_t id, uint32_t type )
{
	return new GameObject( id, type );
}


Player *RaptorGame::NewPlayer( uint16_t id )
{
	return new Player( id );
}


void RaptorGame::Host( void )
{
	if( Net.Connected )
	{
		Net.DisconnectNice( NULL );
		SDL_Delay( 200 );
	}
	
	if( Server )
	{
		Server->Port = Cfg.SettingAsInt( "sv_port", Raptor::Game->DefaultPort );
		Server->MaxFPS = Cfg.SettingAsDouble( "sv_maxfps", 60. );
		Server->NetRate = Cfg.SettingAsDouble( "sv_netrate", 30. );
		Server->Start( Cfg.SettingAsString( "name" , Raptor::Server->Game.c_str() ) );
		
		Net.Connect( "localhost", Server->Port, Cfg.SettingAsString("name").c_str(), Cfg.SettingAsString("password").c_str() );
	}
}


void RaptorGame::Quit( void )
{
	Layers.RemoveAll();
}


// ---------------------------------------------------------------------------


void Raptor::Terminate( int arg )
{
	Raptor::Game->Quit();
}


void Raptor::BrokenPipe( int arg )
{
	fprintf( stderr, "Received signal: SIGPIPE\n" );
	fflush( stderr );
	
	if( ! Raptor::Server->IsRunning() )
	{
		Raptor::Game->Net.DisconnectMessage = "Broken Pipe";
		Raptor::Game->Net.Disconnect();
		
#ifdef SIGPIPE
		// SDLNet_TCP_Send gets stuck in an infinite do/while loop when a broke pipe occurs, so we exit.
		// FIXME: Replace SDLNet_TCP_Send with our own send loop that can handle broken pipes?
		exit( SIGPIPE );
#endif
	}
}

