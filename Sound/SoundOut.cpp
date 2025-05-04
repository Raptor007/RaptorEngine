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
	VoiceVolume = 1.f;
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
	Initialize( Raptor::Game->Cfg.SettingAsInt("s_channels",2), Raptor::Game->Cfg.SettingAsInt("s_rate",44100), Raptor::Game->Cfg.SettingAsInt("s_depth",16), Raptor::Game->Cfg.SettingAsInt("s_buffer",4096), Raptor::Game->Cfg.SettingAsInt("s_mix_channels",64) );
}


void SoundOut::Initialize( int channels, int rate, int depth, int buffer, int mix_channels )
{
	bool had_mic = Raptor::Game->Mic.Device;
	std::string music_subdir;
	
	// Determine the audio format; default to 16-bit if an unsupported depth is passed.
	Uint16 format = AUDIO_S16SYS;
	if( depth == 8 )
		format = AUDIO_U8;
	else if( depth == 16 )
		format = AUDIO_S16SYS;
#if SDL_VERSION_ATLEAST(2,0,0)
	else if( depth == 24 )
		format = AUDIO_S32SYS;
	else if( depth == 32 )
		format = AUDIO_F32SYS;
#else
	// SDL1 can't handle more than 16-bit stereo.
	if( channels > 2 )
		channels = 2;
	// SDL1 stretches the music at sample rates above 44.1KHz.
	if( rate > 44100 )
		rate = 44100;
#endif
	
	if( Initialized )
	{
		// Remember the music we were playing so we can restart it.
		if( Mix_PlayingMusic() )
			music_subdir = MusicSubdir;
		
		StopSounds();
		StopMusic();
		
		SDL_Delay( 1 );
		
		Raptor::Game->Res.DeleteSounds();
		Raptor::Game->Res.DeleteMusic();
		Mix_CloseAudio();
		
#if SDL_VERSION_ATLEAST(2,0,0)
		if( had_mic )
		{
			// FIXME: Move this into Microphone?
			SDL_CloseAudioDevice( Raptor::Game->Mic.Device );
			Raptor::Game->Mic.Device = 0;
		}
		else
			had_mic = Raptor::Game->Cfg.SettingAsBool("s_mic_init");
#endif
		
		SDL_QuitSubSystem( SDL_INIT_AUDIO );
		Initialized = false;
	}
	
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
	
	// Restart music if it was playing before.
	if( music_subdir.length() )
		PlayMusicSubdir( music_subdir );
	
	// If we had initialized a microphone before, do that again.
	if( had_mic )
		Raptor::Game->Mic.Initialize( Raptor::Game->Cfg.SettingAsString("s_mic_device"), Raptor::Game->Cfg.SettingAsInt("s_mic_buffer",2048) );
}


int SoundOut::Play( Mix_Chunk *sound, int channel )
{
	if( ! Initialized )
		return -1;
	if( ! sound )
		return -1;
	
	channel = Mix_PlayChannelTimed( channel, sound, 0, -1 );
	
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


int SoundOut::PlayBuffer( PlaybackBuffer *buffer, int channel )
{
	if( ! buffer )
		return -1;
	
	return buffer->PlayAvailable( channel );
}


int SoundOut::PlayBuffer( PlaybackBuffer *buffer, int16_t angle, uint8_t dist )
{
	if( ! buffer )
		return -1;
	
	int channel = buffer->PlayAvailable();
	if( channel >= 0 )
		Pan2D( channel, angle, dist );
	
	return channel;
}


int SoundOut::PlayAt( Mix_Chunk *sound, double x, double y, double z, double loudness )
{
	if( loudness <= 0. )
		return -1;
	return Pan3D( Play(sound), x, y, z, loudness );
}


Mix_Chunk *SoundOut::AllocatePCM( const void *data, size_t samples, uint32_t sample_rate, uint8_t bytes_per_sample, uint8_t channels )
{
	if( ! (data && bytes_per_sample && sample_rate && channels) )
		return NULL;
	
	uint8_t  bytes_per_frame = channels * bytes_per_sample;
	uint32_t byte_rate = sample_rate * bytes_per_frame;
	uint32_t size2 = samples * bytes_per_frame;
	uint32_t size1 = 36 + size2;
	
	unsigned char wave_header[ 44 ] = { 'R','I','F','F',
		(unsigned char)(size1&0xFF),(unsigned char)((size1>>8)&0xFF),(unsigned char)((size1>>16)&0xFF),(unsigned char)((size1>>24)&0xFF),
		'W','A','V','E', 'f','m','t',' ', 16,0,0,0, 1,0, channels,0,
		(unsigned char)(sample_rate&0xFF),(unsigned char)((sample_rate>>8)&0xFF),(unsigned char)((sample_rate>>16)&0xFF),(unsigned char)((sample_rate>>24)&0xFF),
		(unsigned char)(  byte_rate&0xFF),(unsigned char)((  byte_rate>>8)&0xFF),(unsigned char)((  byte_rate>>16)&0xFF),(unsigned char)((  byte_rate>>24)&0xFF),
		bytes_per_frame,0,
		(unsigned char)(8*bytes_per_sample),0,
		'd','a','t','a',
		(unsigned char)(size2&0xFF),(unsigned char)((size2>>8)&0xFF),(unsigned char)((size2>>16)&0xFF),(unsigned char)((size2>>24)&0xFF) };
	
	size_t buffer_size = sizeof(wave_header) + size2;
	uint8_t *buffer = (uint8_t*) malloc( buffer_size );
	if( ! buffer )
	{
		Raptor::Game->Console.Print( "SoundOut::AllocateChunk: malloc failed!", TextConsole::MSG_ERROR );
		return NULL;
	}
	
	memcpy( buffer, wave_header, sizeof(wave_header) );  // FIXME: Get rid of wave_header and just write straight into buffer?
	memcpy( buffer + sizeof(wave_header), data, size2 );
	
	SDL_RWops *rw = SDL_RWFromConstMem( buffer, buffer_size );
	if( ! rw )
		Raptor::Game->Console.Print( "SoundOut::AllocateChunk: SDL_RWFromMem failed!", TextConsole::MSG_ERROR );
	Mix_Chunk *chunk = Mix_LoadWAV_RW( rw, 1 );  // 1 means auto-free the SDL_RWops.
	
	free( buffer );
	return chunk;
}


void SoundOut::StopSounds( void )
{
	Mix_HaltChannel( -1 );
	Delayed.clear();
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
	double fwd_dot   = Cam.Fwd.Dot(   &cam_to_pt );
	double right_dot = Cam.Right.Dot( &cam_to_pt );
	
	double angle = Num::RadToDeg( atan2( right_dot, fwd_dot ) );
	if( angle < 0. )
		angle += 360.;
	
	double real_dist = sqrt( (Cam.X-x)*(Cam.X-x) + (Cam.Y-y)*(Cam.Y-y) + (Cam.Z-z)*(Cam.Z-z) ) * DistScale;
	double dist = loudness ? (real_dist / loudness) : 255.1;
	if( dist > 255.1 )
		dist = 255.1;
	else if( real_dist < 0.1 )  // Prevent jittery sound direction when extremely close to camera.
		angle = 0.;
	
	return Pan2D( channel, angle, dist );
}


int SoundOut::PanWithObject( int channel, uint32_t object_id, double loudness )
{
	if( channel >= 0 )
	{
		ActivePans[ channel ] = PanningSound( channel, object_id, loudness );
		ObjectPans[ object_id ] = channel;
		RecentPans[ object_id ].Reset();
	}
	
	return channel;
}


int SoundOut::PlayPanned( Mix_Chunk *sound, double x, double y, double z, double loudness )
{
	int channel = PlayAt( sound, x, y, z, loudness );
	
	if( channel >= 0 )
		ActivePans[ channel ] = PanningSound( channel, x, y, z, loudness );
	
	return channel;
}


int SoundOut::PlayFromObject( Mix_Chunk *sound, const GameObject *obj, double loudness )
{
	int channel = -1;
	
	if( obj )
	{
		channel = PlayAt( sound, obj->X, obj->Y, obj->Z, loudness );
		
		if( channel >= 0 )
		{
			ActivePans[ channel ] = PanningSound( channel, obj->ID, loudness );
			ObjectPans[ obj->ID ] = channel;
			RecentPans[ obj->ID ].Reset();
		}
	}
	
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


void SoundOut::PlayDelayedFromObject( Mix_Chunk *sound, double delay, uint32_t object_id, double loudness )
{
	Delayed.push_back( SoundOutDelayed( sound, delay, object_id, loudness ) );
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
		
		int channel = *channel_iter;
		
		if( ! IsPlaying( channel ) )
		{
			bool ended = true;
			
			std::map<int, Mix_Chunk*>::iterator alloc_iter = Allocated.find( channel );
			if( alloc_iter != Allocated.end() )
			{
				Mix_FreeChunk( alloc_iter->second );
				Allocated.erase( alloc_iter );
			}
			
			std::map<int, PlaybackBuffer*>::iterator buffer_iter = PlayingBuffers.find( channel );
			if( buffer_iter != PlayingBuffers.end() )
			{
				if( buffer_iter->second->Filled )
					ended = (buffer_iter->second->PlayAvailable() < 0);
				
				if( ended )
				{
					buffer_iter->second->AudioChannel = -1;
					PlayingBuffers.erase( buffer_iter );
				}
			}
			
			if( ended )
				ActiveChannels.erase( channel_iter );
		}
		
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
	
	
	// Play any delayed sounds that are ready.
	
	for( std::list<SoundOutDelayed>::iterator this_delayed = Delayed.begin(); this_delayed != Delayed.end(); )
	{
		std::list<SoundOutDelayed>::iterator next_delayed = this_delayed;
		next_delayed ++;
		
		if( this_delayed->Delay.Progress() >= 1. )
		{
			if( this_delayed->ObjectID )
				PlayFromObject( this_delayed->Sound, this_delayed->ObjectID, this_delayed->Loudness );
			Delayed.erase( this_delayed );
		}
		
		this_delayed = next_delayed;
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
	
	if( VoiceVolume > 1.f )
		VoiceVolume = 1.f;
	else if( VoiceVolume < 0.f )
		VoiceVolume = 0.f;
	
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
		if( (AttenuateFor < 0) && PlayingBuffers.empty() )
			Mix_Volume( -1, MasterVolume * SoundVolume * MIX_MAX_VOLUME + 0.25f );
		else
		{
			for( std::set<int>::const_iterator channel_iter = ActiveChannels.begin(); channel_iter != ActiveChannels.end(); channel_iter ++ )
			{
				int channel = *channel_iter;
				if( PlayingBuffers.find(channel) != PlayingBuffers.end() )  // FIXME: Check if this is in VoiceBuffers?
					Mix_Volume( channel, MasterVolume * VoiceVolume * MIX_MAX_VOLUME + 0.25f );
				else if( channel == AttenuateFor )
					Mix_Volume( channel, MasterVolume * MIX_MAX_VOLUME + 0.25f );
				else
					Mix_Volume( channel, MasterVolume * SoundVolume * SoundAttenuate * MIX_MAX_VOLUME + 0.25f );
			}
		}
		
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


// -----------------------------------------------------------------------------


SoundOutDelayed::SoundOutDelayed( Mix_Chunk *sound, double delay, uint32_t object_id, double loudness )
{
	Sound = sound;
	Delay.CountUpToSecs = std::max<double>( 0., delay );
	ObjectID = object_id;
	Loudness = loudness;
}


SoundOutDelayed::SoundOutDelayed( const SoundOutDelayed &other )
{
	Sound = other.Sound;
	Delay.Sync( &(other.Delay) );
	ObjectID = other.ObjectID;
	Loudness = other.Loudness;
}


SoundOutDelayed::~SoundOutDelayed()
{
}
