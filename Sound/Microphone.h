/*
 *  Microphone.h
 */

#pragma once
class Microphone;

#include "PlatformSpecific.h"
#include <cstddef>
#include <string>
#include <stdint.h>
#include "Packet.h"
#include "PlaybackBuffer.h"

#ifdef SDL2
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_audio.h>
#else
	#include <SDL/SDL.h>
#endif

// AudioQueue requires Mac OS X 10.6+, so we only use it in 64-bit builds.
#if defined(__APPLE__) && defined(__x86_64__) && ! SDL_VERSION_ATLEAST(2,0,0)
	#include <AudioToolbox/AudioToolbox.h>
	#define MACOSX_MICROPHONE 1
#endif

class Microphone
{
public:
#if SDL_VERSION_ATLEAST(2,0,0)
	SDL_AudioDeviceID Device;
#elif defined(MACOSX_MICROPHONE)
	AudioQueueRef Device;
	AudioQueueBufferRef Buffers[ 2 ];
	PlaybackBuffer Recorded;
#else
	bool Device;
#endif
	SDL_AudioSpec Spec;
	uint8_t VoiceChannel, Transmitting;
	uint32_t FromObject;
	
	Microphone( void );
	virtual ~Microphone();
	
	void Initialize( const std::string &mic_dev, int buffer = 0 );
	void Initialize( const char *mic_dev = NULL, int buffer = 0 );
	bool Start( void );
	bool Start( uint8_t voice_channel );
	void Stop( void );
	bool Transmit( void );
	bool Transmit( double min_secs, double max_secs = 2., uint8_t flags = 0x00 );
};
