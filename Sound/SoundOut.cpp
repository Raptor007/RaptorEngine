/*
 *  SoundOut.cpp
 */

#include "SoundOut.h"

#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <dirent.h>
#include "Num.h"
#include "Rand.h"
#include "RaptorGame.h"


SoundOut::SoundOut( void )
{
	Initialized = false;
	Channels = 64;
	
	MasterVolume = 1.f;
	SoundVolume = 1.f;
	MusicVolume = 1.f;
	DistScale = 0.25;
	
	PlayMusic = false;
	CurrentMusic = -1;
	ShuffleMusic = false;
	
	SoundAttenuate = 1.f;
	MusicAttenuate = 1.f;
	AttenuateFor = -1;
}


SoundOut::~SoundOut()
{
	StopMusic();
	
	if( Initialized )
	{
		Mix_CloseAudio();
		SDL_QuitSubSystem( SDL_INIT_AUDIO );
	}
	
	Initialized = false;
}


void SoundOut::Initialize( void )
{
	Initialize( 2, 44100, 16, 4096, Channels );
}


void SoundOut::Initialize( int channels, int rate, int depth, int buffer, int mix_channels )
{
	// Determine the audio format; default to 16-bit if an unsupported depth is passed.
	uint16_t format = AUDIO_S16SYS;
	if( depth == 8 )
		format = AUDIO_U8;
	else if( depth == 16 )
		format = AUDIO_S16SYS;
	
	// NOTE: This must happen after graphics are initialized!
	SDL_InitSubSystem( SDL_INIT_AUDIO );
	
	// Open the audio output.
	if( Mix_OpenAudio( rate, format, channels, buffer ) )
	{
		fprintf( stderr, "Unable to initialize audio: %s\n", Mix_GetError() );
		SDL_QuitSubSystem( SDL_INIT_AUDIO );
		return;
	}
	
	// Try to allocate mixing channels.
	Mix_AllocateChannels( mix_channels );
	
	// Get the number of actual mixing channels available.
	Channels = Mix_AllocateChannels( -1 );
	
	// Done initializing.
	Initialized = true;
}


int SoundOut::Play( Mix_Chunk *sound )
{
	if( ! Initialized )
		return -1;
	if( ! sound )
		return -1;
	
	int channel = Mix_PlayChannelTimed( -1, sound, 0, -1 );
	
	if( channel >= 0 )
		ActiveChannels.insert( channel );
	
	UpdateVolumes();
	
	return channel;
}


int SoundOut::Play( Mix_Chunk *sound, int16_t angle, uint8_t dist )
{
	if( ! Initialized )
		return -1;
	if( ! sound )
		return -1;
	
	// Don't play far-away sounds to keep channels free.
	if( dist == 255 )
		return -1;
	
	int channel = Play( sound );
	if( channel >= 0 )
	{
		if( ! Mix_SetPosition( channel, angle, dist ) )
			fprintf( stderr, "Mix_SetPosition: %s\n", Mix_GetError() );
	}
	
	return channel;
}


int SoundOut::PlayAt( Mix_Chunk *sound, double x, double y, double z, double loudness )
{
	return Pan3D( Play(sound), x, y, z, loudness );
}


void SoundOut::StopSounds( void )
{
	Mix_HaltChannel( -1 );
}


bool SoundOut::IsPlaying( int channel )
{
	if( ! Initialized )
		return false;
	if( channel < 0 )
		return false;
	
	return Mix_Playing( channel );
}


int SoundOut::Pan2D( int channel, int16_t angle, uint8_t dist )
{
	if( ! IsPlaying(channel) )
		return -1;
	
	// Halt far-away sounds to free up channels.
	if( (channel >= 0) && (dist == 255) )
	{
		Mix_HaltChannel( channel );
		return -1;
	}
	
	if( ! Mix_SetPosition( channel, angle, dist ) )
		fprintf( stderr, "Mix_SetPosition: %s\n", Mix_GetError() );
	
	return channel;
}


int SoundOut::Pan3D( int channel, double x, double y, double z, double loudness )
{
	Vec3D cam_to_pt( x - Cam.X, y - Cam.Y, z - Cam.Z );
	double fwd_dot = Cam.Fwd.Dot( &cam_to_pt );
	double right_dot = Cam.Right.Dot( &cam_to_pt );
	
	double angle = Num::RadToDeg( atan2( right_dot, fwd_dot ) );
	if( angle < 0. )
		angle += 360.;
	
	double dist = sqrt( (Cam.X-x)*(Cam.X-x) + (Cam.Y-y)*(Cam.Y-y) + (Cam.Z-z)*(Cam.Z-z) ) * DistScale / loudness;
	if( dist > 255.1 )
		dist = 255.1;
	else if( dist < 0.1 )
		angle = 0.;
	
	return Pan2D( channel, angle, dist );
}


int SoundOut::PlayPanned( Mix_Chunk *sound, double x, double y, double z, double loudness )
{
	int channel = PlayAt( sound, x, y, z, loudness );
	
	if( channel >= 0 )
		ActivePans[ channel ] = PanningSound( channel, x, y, z, loudness );
	
	return channel;
}


int SoundOut::PlayFromObject( Mix_Chunk *sound, uint32_t object_id, double loudness )
{
	int channel = -1;
	
	GameObject *obj = Raptor::Game->Data.GetObject( object_id );
	if( obj )
	{
		channel = PlayAt( sound, obj->X, obj->Y, obj->Z, loudness );
		
		if( channel >= 0 )
		{
			ActivePans[ channel ] = PanningSound( channel, object_id, loudness );
			ObjectPans[ object_id ] = channel;
			RecentPans[ object_id ].Reset();
		}
	}
	
	return channel;
}


int SoundOut::SetPos( int channel, double x, double y, double z )
{
	if( ! Initialized )
		return -1;
	
	if( channel >= 0 )
	{
		if( ! IsPlaying(channel) )
			return -1;
		
		std::map<int, PanningSound>::iterator this_pan = ActivePans.find(channel);
		if( this_pan == ActivePans.end() )
			return -1;
		else
			this_pan->second.Set( x, y, z );
	}
	
	return channel;
}


void SoundOut::Update( const Pos3D *cam )
{
	for( std::set<int>::const_iterator channel_iter = ActiveChannels.begin(); channel_iter != ActiveChannels.end(); )
	{
		std::set<int>::const_iterator channel_next = channel_iter;
		channel_next ++;
		
		if( ! IsPlaying( *channel_iter ) )
			ActiveChannels.erase( channel_iter );
		
		channel_iter = channel_next;
	}
	
	UpdateVolumes();
	
	
	if( cam )
	{
		// Pan sound effects.
		
		Cam = *cam;
		
		for( std::map<int, PanningSound>::iterator this_pan = ActivePans.begin(); this_pan != ActivePans.end(); )
		{
			std::map<int, PanningSound>::iterator next_pan = this_pan;
			next_pan ++;
			
			this_pan->second.Update();
			
			if( IsPlaying( this_pan->first ) )
				Pan3D( this_pan->first, this_pan->second.X, this_pan->second.Y, this_pan->second.Z, this_pan->second.Loudness );
			else
			{
				if( this_pan->second.ObjectID )
				{
					std::map<uint32_t, int>::iterator obj_pan_iter = ObjectPans.find( this_pan->second.ObjectID );
					if( obj_pan_iter != ObjectPans.end() )
						ObjectPans.erase( obj_pan_iter );
				}
				
				ActivePans.erase( this_pan );
				// Remember: Don't delete from RecentPans here so we can know how long ago a sound last played for any object.
			}
			
			this_pan = next_pan;
		}
	}
	
	
	// Forget last sound time of any objects that no longer exist.
	
	for( std::map<uint32_t, Clock>::iterator this_pan = RecentPans.begin(); this_pan != RecentPans.end(); )
	{
		std::map<uint32_t, Clock>::iterator next_pan = this_pan;
		next_pan ++;
		
		if( ! Raptor::Game->Data.GetObject( this_pan->first ) )
			RecentPans.erase( this_pan );
		
		this_pan = next_pan;
	}
	
	
	// Update active music.
	
	if( Initialized )
	{
		if( ! Mix_PlayingMusic() )
		{
			if( PlayMusic && MusicStream.size() )
			{
				// The stream allows context-specific music to play smoothly within the background music playlist.
				PlayMusicWithRetries( MusicStream.front() );
				MusicStream.pop_front();
			}
			else if( PlayMusic && MusicList.size() )
			{
				int next_track = CurrentMusic + 1;
				
				if( ShuffleMusic && (MusicList.size() > 1) )
				{
					// A non-negative value for CurrentMusic means the previous track was in the playlist, so try not to repeat it.
					if( CurrentMusic >= 0 )
					{
						// Subtract 2 because Rand::Int(min,max) includes the endpoints, and we want to avoid repeating the same track.
						next_track = Rand::Int( 0, MusicList.size() - 2 );
						if( next_track >= CurrentMusic )
							next_track ++;
					}
					else
						// Subtract 1 because Rand::Int(min,max) includes the endpoints.
						next_track = Rand::Int( 0, MusicList.size() - 1 );
				}
				
				// If we hit the end of the playlist, start again at the beginning.
				if( (size_t) next_track >= MusicList.size() )
					next_track = 0;
				
				try
				{
					PlayMusicWithRetries( MusicList.at( next_track ) );
					CurrentMusic = next_track;
				}
				catch( std::out_of_range &exception )
				{
					fprintf( stderr, "SoundOut::Update: std::out_of_range\n" );
				}
			}
		}
		else if( ! PlayMusic )
		{
			Mix_HaltMusic();
			SDL_Delay( 1 );
		}
	}
}


void SoundOut::UpdateVolumes( void )
{
	if( MasterVolume > 1.f )
		MasterVolume = 1.f;
	else if( MasterVolume < 0.f )
		MasterVolume = 0.f;
	
	if( SoundVolume > 1.f )
		SoundVolume = 1.f;
	else if( SoundVolume < 0.f )
		SoundVolume = 0.f;
	
	if( MusicVolume > 1.f )
		MusicVolume = 1.f;
	else if( MusicVolume < 0.f )
		MusicVolume = 0.f;
	
	if( ! IsPlaying(AttenuateFor) )
	{
		SoundAttenuate = 1.f;
		MusicAttenuate = 1.f;
		AttenuateFor = -1;
	}
	else
	{
		if( SoundAttenuate > 1.f )
			SoundAttenuate = 1.f;
		else if( SoundAttenuate < 0.f )
			SoundAttenuate = 0.f;
		
		if( MusicAttenuate > 1.f )
			MusicAttenuate = 1.f;
		else if( MusicAttenuate < 0.f )
			MusicAttenuate = 0.f;
	}
	
	if( Initialized )
	{
		if( AttenuateFor >= 0 )
		{
			for( std::set<int>::const_iterator channel_iter = ActiveChannels.begin(); channel_iter != ActiveChannels.end(); channel_iter ++ )
			{
				if( *channel_iter == AttenuateFor )
					Mix_Volume( AttenuateFor, MasterVolume * MIX_MAX_VOLUME + 0.25f );
				else
					Mix_Volume( *channel_iter, MasterVolume * SoundVolume * SoundAttenuate * MIX_MAX_VOLUME + 0.25f );
			}
		}
		else
			Mix_Volume( -1, MasterVolume * SoundVolume * SoundAttenuate * MIX_MAX_VOLUME + 0.25f );
		
		Mix_VolumeMusic( MasterVolume * MusicVolume * MusicAttenuate * MIX_MAX_VOLUME + 0.25f );
	}
}


void SoundOut::PlayMusicOnce( Mix_Music *music )
{
	// This will not affect the current playlist; it will play a song once and then return to the previous state.
	// However, if there was a queued playlist and playback was stopped, this will resume it.
	
	CurrentMusic = -1;
	
	if( Initialized )
	{
		if( Mix_PlayingMusic() )
		{
			Mix_HaltMusic();
			SDL_Delay( 1 );
		}
		
		if( music )
			PlayMusicWithRetries( music );
	}
	
	PlayMusic = true;
}


void SoundOut::PlayMusicLooped( Mix_Music *music )
{
	// This clears the playlist, adds the designated music as the only item, and plays it.
	// This is more flexible than using SDL's music looping, since we can still add items to our playlist.
	
	StopMusic();
	
	if( music )
	{
		MusicList.push_back( music );
		CurrentMusic = 0;
		
		if( Initialized )
			PlayMusicWithRetries( music );
	}
	
	PlayMusic = true;
}


void SoundOut::QueueMusic( Mix_Music *music )
{
	// This adds an item to the playlist without changing the current playback.
	
	if( music )
		MusicList.push_back( music );
}


void SoundOut::StreamMusic( Mix_Music *music )
{
	// This adds an item to the stream without changing the current playback.
	
	if( music )
		MusicStream.push_back( music );
}


void SoundOut::StreamMusicUnique( Mix_Music *music )
{
	// This adds an item to the stream without changing the current playback.
	
	if( music && (std::find( MusicStream.begin(), MusicStream.end(), music ) == MusicStream.end()) )
		MusicStream.push_back( music );
}


void SoundOut::StopMusic( void )
{
	// This stops music playback and clears the stream and playlist.
	
	PlayMusic = false;
	
	if( Initialized && Mix_PlayingMusic() )
	{
		Mix_HaltMusic();
		SDL_Delay( 1 );
	}
	
	MusicStream.clear();
	MusicSubdir = "";
	MusicList.clear();
	CurrentMusic = -1;
}


void SoundOut::PlayMusicSubdir( std::string dir )
{
	StopMusic();
	
	QueueMusicSubdir( dir );
	MusicSubdir = dir;
	
	PlayMusic = true;
}


void SoundOut::PlayMusicSubdirNext( std::string dir )
{
	if( dir == MusicSubdir )
		return;
	
	MusicList.clear();
	CurrentMusic = -1;
	
	QueueMusicSubdir( dir );
	MusicSubdir = dir;
	
	PlayMusic = true;
}


void SoundOut::QueueMusicSubdir( std::string dir )
{
	if( DIR *dir_p = opendir( Raptor::Game->Res.Find(dir).c_str() ) )
	{
		while( struct dirent *dir_entry_p = readdir(dir_p) )
		{
			if( dir_entry_p->d_name[ 0 ] == '.' )
				continue;
			
			Mix_Music *music = Raptor::Game->Res.GetMusic( dir + "/" + std::string(dir_entry_p->d_name) );
			if( music )
				QueueMusic( music );
		}
		
		closedir( dir_p );
	}
}


void SoundOut::PlayMusicWithRetries( Mix_Music *music )
{
	if( ! music )
		return;
	
	for( int attempt = 0; attempt < 10; attempt ++ )
	{
		int error = Mix_PlayMusic( music, 1 );
		SDL_Delay( 1 );
		
		if( ! error )
		{
			UpdateVolumes();
			break;
		}
	}
}
