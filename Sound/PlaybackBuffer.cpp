/*
 *  PlaybackBuffer.cpp
 */

#include "PlaybackBuffer.h"

#include <stdio.h>
#include <algorithm>
#include "RaptorGame.h"


PlaybackBuffer::PlaybackBuffer( void )
{
	SampleRate = 22050;
	Channels = 1;
	BytesPerSample = 2;
	Allocated = Filled = 0;
	Data = NULL;
	AllocationChunkSize = 32768;
	AudioChannel = -1;
	VoiceChannel = Raptor::VoiceChannel::NONE;
}


PlaybackBuffer::~PlaybackBuffer()
{
	if( Allocated )
		free( Data );
	Data = NULL;
	Allocated = 0;
	Filled = 0;
	AudioChannel = -1;
}


void PlaybackBuffer::AddBytes( const void *data, size_t add_bytes )
{
	if( !( data && add_bytes ) )
		return;
	
	Lock.Lock();
	
	if( MakeRoom(add_bytes) )
	{
		memcpy( Data + Filled, data, add_bytes );
		Filled += add_bytes;
	}
	
	Lock.Unlock();
}


size_t PlaybackBuffer::ConsumeBytes( void *dest, size_t bytes )
{
	Lock.Lock();
	
	if( bytes > Filled )
		bytes = Filled;
	
	if( dest )
		memcpy( dest, Data, bytes );
	Filled -= bytes;
	
	// Scoot any audio we still have.
	if( Filled )
		memmove( Data, Data + bytes, Filled );
	
	Lock.Unlock();
	
	return bytes;
}


bool PlaybackBuffer::MakeRoom( size_t add_bytes )
{
	// If we've never allocated, do so!
	if( ! Data )
	{
		// Determine how much to allocate.
		size_t add_mem = add_bytes;
		if( AllocationChunkSize > 1 )
			add_mem = std::max<size_t>( 1, (add_bytes + AllocationChunkSize - 1) / AllocationChunkSize ) * AllocationChunkSize;
		
		// Create data allocation for this packet.
		Data = (uint8_t *) malloc( add_mem );
		if( Data )
			Allocated = add_mem;
		else
		{
			Allocated = 0;
			return false;
		}
	}
	
	// If we don't have enough memory yet, allocate more!
	else if( Filled + add_bytes > Allocated )
	{
		// Determine how much to add.
		size_t add_mem = add_bytes;
		if( AllocationChunkSize > 1 )
			add_mem = std::max<size_t>( 1, (Filled + add_bytes - Allocated + AllocationChunkSize - 1) / AllocationChunkSize ) * AllocationChunkSize;
		
		// Increase allocation for this packet.
		uint8_t *new_data = (uint8_t *) realloc( Data, Allocated + add_mem );
		if( new_data )
		{
			Data = new_data;
			Allocated += add_mem;
		}
		else
			return false;
	}
	
	return true;
}


int PlaybackBuffer::PlayAvailable( void )
{
	if( ! Filled )
		return -1;
	
	Lock.Lock();
	
	Mix_Chunk *chunk = Raptor::Game->Snd.AllocatePCM( Data, Filled / (BytesPerSample * Channels), SampleRate, BytesPerSample, Channels );
	
	AudioChannel = Raptor::Game->Snd.Play( chunk, AudioChannel );
	
	if( AudioChannel >= 0 )
	{
		Mix_Chunk *garbage = Raptor::Game->Snd.Allocated[ AudioChannel ];
		Raptor::Game->Snd.Allocated[ AudioChannel ] = chunk;
		if( garbage )
			Mix_FreeChunk( garbage );
		
		PlaybackBuffer *prev_buffer = Raptor::Game->Snd.PlayingBuffers[ AudioChannel ];
		if( prev_buffer && (prev_buffer != this) )
			prev_buffer->AudioChannel = -1;
		Raptor::Game->Snd.PlayingBuffers[ AudioChannel ] = this;
		
		Filled = 0;
	}
	else
		Mix_FreeChunk( chunk );
	
	Lock.Unlock();
	
	return AudioChannel;
}


int PlaybackBuffer::PlayAvailable( int snd_channel )
{
	AudioChannel = snd_channel;
	return PlayAvailable();
}


double PlaybackBuffer::ReadySeconds( void ) const
{
	return Filled / (double) (SampleRate * BytesPerSample * Channels);
}
