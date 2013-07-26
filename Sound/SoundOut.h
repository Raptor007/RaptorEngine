/*
 *  SoundOut.h
 */

#pragma once
class SoundOut;

#include "PlatformSpecific.h"

#include <string>
#include <map>
#include <queue>
#include <stdint.h>
#include <SDL/SDL.h>
#include <SDL_mixer/SDL_mixer.h>
#include "PanningSound.h"
#include "Pos.h"


class SoundOut
{
public:
	bool Initialized;
	int Channels;
	float MasterVolume, SoundVolume, MusicVolume;
	double DistScale;
	Pos3D Cam;
	//std::set<int> ActiveChannels;
	std::map<int, PanningSound> ActivePans;
	std::map<uint32_t, int> ObjectPans;
	float SoundAttenuate, MusicAttenuate;
	int AttenuateFor;
	
	bool PlayMusic;
	std::vector<Mix_Music*> MusicList;
	int CurrentMusic;
	bool ShuffleMusic;
	
	SoundOut( void );
	~SoundOut();
	
	void Initialize( void );
	void Initialize( int channels, int rate, int depth, int buffer, int mix_channels );
	
	int Play( Mix_Chunk *sound );
	int Play( Mix_Chunk *sound, int16_t angle, uint8_t dist );
	int PlayAt( Mix_Chunk *sound, double x, double y, double z, double loudness = 1. );
	
	void StopSounds( void );
	
	bool IsPlaying( int channel );
	
	int Pan2D( int channel, int16_t angle, uint8_t dist );
	int Pan3D( int channel, double x, double y, double z, double loudness = 1. );
	
	int PlayPanned( Mix_Chunk *sound, double x, double y, double z, double loudness = 1. );
	int PlayFromObject( Mix_Chunk *sound, uint32_t object_id, double loudness = 1. );
	int SetPos( int channel, double x, double y, double z );
	
	void Update( const Pos3D *cam = NULL );
	void UpdateVolumes( void );
	
	void PlayMusicOnce( Mix_Music *music );
	void PlayMusicLooped( Mix_Music *music );
	void QueueMusic( Mix_Music *music );
	void StopMusic( void );
	
	void PlayMusicSubdir( std::string dir );
	void QueueMusicSubdir( std::string dir );
	
private:
	void PlayMusicWithRetries( Mix_Music *music );
};
