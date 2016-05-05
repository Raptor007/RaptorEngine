/*
 *  RaptorGame.cpp
 */

#define RAPTORGAME_CPP

#include "RaptorGame.h"

#include "RaptorDefs.h"
#include "Rand.h"
#include "Num.h"


namespace Raptor
{
	// Global pointer to the game engine object.
	RaptorGame *Game = NULL;
	
	void Terminate( int arg );
	
#ifdef WIN32
	BOOL WindowsInit( void );
#endif
}


RaptorGame::RaptorGame( std::string game, std::string version, RaptorServer *server )
{
	Game = game;
	Version = version;
	
	SetServer( server );
	
	// This arbitrary large value allows us to use Data.AddObject for client-side non-networked objects, if we want to.
	Data.GameObjectIDs.Clear( 0x10000000 );
	
	MaxFPS = 0.;
	FrameTime = 0.;
	State = Raptor::State::DISCONNECTED;
	PlayerID = 0;
}


RaptorGame::~RaptorGame()
{
}


void RaptorGame::SetServer( RaptorServer *server )
{
	Server = server;
	if( Server )
		Server->Console = &Console;
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
		else if( strcmp( argv[ i ], "-safe" ) == 0 )
		{
			Cfg.Settings[ "g_framebuffers" ] = "false";
			Cfg.Settings[ "g_shader_enable" ] = "false";
			Cfg.Settings[ "g_legacy_mipmap" ] = "true";
		}
		else if( strcmp( argv[ i ], "-screensaver" ) == 0 )
		{
			screensaver = true;
			Cfg.Settings[ "screensaver" ] = "true";
			Cfg.Settings[ "g_fullscreen" ] = "true";
			Cfg.Settings[ "g_res_fullscreen_x" ] = "0";
			Cfg.Settings[ "g_res_fullscreen_y" ] = "0";
			Cfg.Settings[ "s_volume" ] = "0";
		}
	}
	
	
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
	
	
	// Configure callback for SIGTERM.
	
	signal( SIGTERM, &(Raptor::Terminate) );
	
	
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
		if( (elapsed > 0.0) && ( (MaxFPS <= 0.0) || ((1.0 / elapsed) <= MaxFPS) ) )
		{
			FrameTime = GameClock.ElapsedSeconds();
			GameClock.Reset();
			
			// Keep the list of joysticks up-to-date.
			Joy.FindJoysticks();
			
			// Handle all user input.
			SDL_Event event;
			while( SDL_PollEvent( &event ) )
			{
				// Keep track of input and mouse position events.
				Mouse.TrackEvent( &event );
				Keys.TrackEvent( &event );
				Joy.TrackEvent( &event );
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
					else if( event.type == SDL_VIDEORESIZE )
						Gfx.SetMode( event.resize.w, event.resize.h );
					else if( (event.type == SDL_KEYUP) && (event.key.keysym.sym == Console.ToggleKey) )
						Console.ToggleActive();
				}
			}
			
			// Update!
			Update( FrameTime );
			
			// Update active panning sounds and continue music playlist if applicable.
			Snd.MasterVolume = Cfg.SettingAsDouble( "s_volume", 0.5 );
			Snd.SoundVolume = Cfg.SettingAsDouble( "s_effect_volume", 0.5 );
			Snd.MusicVolume = Cfg.SettingAsDouble( "s_music_volume", 1. );
			Snd.Update( &Cam );
			
			// If we're disconnected, make sure we clean up variables after the thread finishes.
			Net.Cleanup();
			
			// Send periodic updates to server.
			Net.NetRate = Raptor::Game->Cfg.SettingAsDouble( "netrate", 30. );
			Net.SendUpdates();
			
			// Honor the maxfps variable.
			MaxFPS = Cfg.SettingAsDouble("maxfps");
			
			// If we're waiting to reconnect, try when it's time.
			if( (Net.ReconnectTime > 0) && (Net.ReconnectClock.ElapsedSeconds() > Net.ReconnectTime) )
				Net.Reconnect( Cfg.SettingAsString("name").c_str(), Cfg.SettingAsString("password").c_str() );
			
			// Draw and swap buffers.
			Draw();
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
	
	// Swap front and back framebuffers to update the screen.
	Gfx.SwapBuffers();
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
	return false;
}


bool RaptorGame::HandleCommand( std::string cmd, std::vector<std::string> *elements )
{
	return false;
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
				Console.Print( "Sync error: UPDATE", TextConsole::MSG_ERROR );
				return true;
			}
		}
		
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
			Data.AddObject( obj );
			obj->ReadFromInitPacket( packet );
			AddedObject( obj );
		}
		
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
		char *name = packet->NextString();
		
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
				Console.Print( "Sync error: PLAYER_PROPERTIES", TextConsole::MSG_ERROR );
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
			char *name = packet->NextString();
			
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
			
			Data.Properties[ name ] = value;
		}
		
		return true;
	}
	
	else if( type == Raptor::Packet::PLAY_SOUND )
	{
		Mix_Chunk *sound = Res.GetSound( packet->NextString() );
		float music_volume = 1.f;
		float sound_volume = 1.f;
		
		if( packet->Remaining() )
			music_volume = Num::UnitFloatFrom8( packet->NextChar() );
		if( packet->Remaining() )
			sound_volume = Num::UnitFloatFrom8( packet->NextChar() );
		
		if( sound )
		{
			int channel = Snd.Play( sound );
			Snd.AttenuateFor = channel;
			Snd.SoundAttenuate = sound_volume;
			Snd.MusicAttenuate = music_volume;
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


void RaptorGame::ChangeState( int state )
{
	State = state;
	
	if( state < Raptor::State::CONNECTED )
	{
		// Clear all game data.
		Data.Clear();
		
		// Clear the message list, since it's session-specific; copies remain in the console.
		Raptor::Game->Msg.Clear();
	}
}


void RaptorGame::AddedObject( GameObject *obj )
{
}


void RaptorGame::RemovedObject( GameObject *obj )
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
		Server->Port = Cfg.SettingAsInt( "sv_port", 7000 );
		Server->MaxFPS = Cfg.SettingAsDouble( "sv_maxfps", 60. );
		Server->NetRate = Cfg.SettingAsDouble( "sv_netrate", 30. );
		Server->UseOutThreads = Cfg.SettingAsBool( "sv_use_out_threads", true );
		Server->Start( Cfg.SettingAsString( "name" , Raptor::Server->Game.c_str() ) );
		
		Clock wait_for_start;
		while( ! Server->IsRunning() )
		{
			if( wait_for_start.ElapsedSeconds() > 3.0 )
				break;
		}
		
		Net.Connect( "localhost", Server->Port, Cfg.SettingAsString("name").c_str(), Cfg.SettingAsString("password").c_str() );
	}
}


void RaptorGame::Quit( void )
{
	// This quickly removes all layers without deleting them.
	Layers.Layers.clear();
}


// ---------------------------------------------------------------------------


void Raptor::Terminate( int arg )
{
	Raptor::Game->Quit();
}


// ---------------------------------------------------------------------------


#ifdef WIN32
#include <tchar.h>
#include <sys/stat.h>
#endif

#if defined(__APPLE__) && (__GNUC__ >= 4) && ((__GNUC__ > 4) || (__GNUC_MINOR__ > 0))
#include <libproc.h>
#include <unistd.h>
#endif

bool Raptor::PreMain( void )
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
		
		// Set the DLL directory to Bin32/Bin64 (or ..\BinXX if that exists instead).
		TCHAR bin_dir[ 16 ] = L"Bin32";
		_stprintf( bin_dir, L"Bin%i", sizeof(void*) * 8 );
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
