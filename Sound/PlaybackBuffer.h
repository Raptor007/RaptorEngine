/*
 *  PlaybackBuffer.h
 */

#pragma once
class PlaybackBuffer;

#include "PlatformSpecific.h"
#include <stdio.h>
#include <stdint.h>
#include "Mutex.h"


class PlaybackBuffer
{
public:
	uint32_t SampleRate;
	uint8_t Channels;
	uint8_t BytesPerSample;
	size_t Allocated, Filled, AllocationChunkSize;
	uint8_t *Data;
	Mutex Lock;
	int AudioChannel;
	uint8_t VoiceChannel;
	
	PlaybackBuffer( void );
	virtual ~PlaybackBuffer();
	
	void AddBytes( const void *data, size_t bytes );
	size_t ConsumeBytes( void *dest, size_t bytes );
	bool MakeRoom( size_t add_bytes );
	int PlayAvailable( void );
	int PlayAvailable( int snd_channel );
	double ReadySeconds( void ) const;
};
