/*
 *  Microphone.cpp
 */

#include "Microphone.h"

#include "IMA.h"
#include "Num.h"
#include "RaptorGame.h"


// =============================================================================
#if defined(MACOSX_MICROPHONE) && ! SDL_VERSION_ATLEAST(2,0,0)

void MicrophoneCallback
(
	void *microphone,
	AudioQueueRef aq,
	AudioQueueBufferRef buf,
	const AudioTimeStamp *start,
	UInt32 packet_desc_num,
	const AudioStreamPacketDescription *packet_desc
)
{
	Microphone *mic = (Microphone*) microphone;
	uint8_t *source = (uint8_t*) buf->mAudioData;
	size_t bytes = buf->mAudioDataByteSize;
	
	mic->Recorded.AddBytes( source, bytes );
	
	// Use this microphone buffer again.
	AudioQueueEnqueueBuffer( mic->Device, buf, 0, NULL );
}

#endif
// =============================================================================


Microphone::Microphone( void )
{
#if SDL_VERSION_ATLEAST(2,0,0)
	Device = 0;
#elif defined(MACOSX_MICROPHONE)
	Device = NULL;
	memset( Buffers, 0, sizeof(Buffers) );
#else
	Device = false;
#endif
	memset( &Spec, 0, sizeof(Spec) );
	Spec.format = AUDIO_S16SYS;
	Spec.channels = 1;
	Spec.freq = 22050;
	Spec.samples = 2048;
	Spec.size = Spec.samples * Spec.channels * 2;
	VoiceChannel = Raptor::VoiceChannel::ALL;
	Transmitting = Raptor::VoiceChannel::NONE;
	FromObject = 0;
}


Microphone::~Microphone()
{
#if SDL_VERSION_ATLEAST(2,0,0)
	if( Device )
	{
		//SDL_CloseAudioDevice( Device );  // FIXME: Make sure there is no race condition between ~Microphone and ~SoundOut when quitting.
		Device = 0;
	}
#elif defined(MACOSX_MICROPHONE)
	if( Device )
	{
		AudioQueueDispose( Device, true );
		Device = NULL;
	}
#endif
}


void Microphone::Initialize( const std::string &mic_dev, int buffer )
{
	Initialize( mic_dev.empty() ? NULL : mic_dev.c_str(), buffer );
}


void Microphone::Initialize( const char *mic_dev, int buffer )
{
	if( buffer )
		Spec.samples = buffer;
	
	Spec.size = Spec.samples * Spec.channels * 2;
	
#if SDL_VERSION_ATLEAST(2,0,0)
	
	if( Device )
	{
		SDL_CloseAudioDevice( Device );
		Device = 0;
	}
	
	SDL_AudioSpec want = Spec;
	
	Device = SDL_OpenAudioDevice( mic_dev, 1, &want, &Spec, 0 );
	
#elif defined(MACOSX_MICROPHONE)
	
	AudioStreamBasicDescription format;
	memset( &format, 0, sizeof(format) );
	format.mFormatID = kAudioFormatLinearPCM;
	format.mSampleRate = Spec.freq;
	format.mChannelsPerFrame = Spec.channels;
	format.mBitsPerChannel = 16;
	format.mFormatFlags = kAudioFormatFlagIsPacked | kAudioFormatFlagIsSignedInteger;
	format.mBytesPerFrame = (format.mBitsPerChannel/8) * format.mChannelsPerFrame;
	format.mFramesPerPacket = 1;
	format.mBytesPerPacket = format.mBytesPerFrame * format.mFramesPerPacket;
	
	AudioQueueNewInput( &format, &MicrophoneCallback, this, NULL, kCFRunLoopCommonModes, 0, &Device );
	if( Device )
	{
		// Allocate and queue multiple buffers so we can record while processing.
		for( size_t i = 0; i < 2; i ++ )
		{
			AudioQueueAllocateBuffer( Device, format.mBytesPerFrame * Spec.samples, &(Buffers[ i ]) );
			AudioQueueEnqueueBuffer( Device, Buffers[ i ], 0, NULL );
		}
	}
#endif
}


bool Microphone::Start( void )
{
#if SDL_VERSION_ATLEAST(2,0,0)
	
	if( ! Device )
		Initialize();
	if( Device )
	{
		SDL_PauseAudioDevice( Device, 0 );
		Transmitting = VoiceChannel;
		return true;
	}
	
#elif defined(MACOSX_MICROPHONE)
	
	if( ! Device )
		Initialize();
	if( Device )
	{
		for( size_t i = 0; i < 2; i ++ )
			AudioQueueEnqueueBuffer( Device, Buffers[ i ], 0, NULL );
		if( ! AudioQueueStart( Device, NULL ) )
		{
			Transmitting = VoiceChannel;
			return true;
		}
	}
	
#endif
	return false;
}


bool Microphone::Start( uint8_t voice_channel )
{
	VoiceChannel = voice_channel;
	return Start();
}


void Microphone::Stop( void )
{
#if SDL_VERSION_ATLEAST(2,0,0)
	
	if( Device )
		SDL_PauseAudioDevice( Device, 1 );
	
#elif defined(MACOSX_MICROPHONE)
	
	if( Device )
		AudioQueueStop( Device, true );
	
#endif
	Transmit( 0., 0., 0x80 );
	Transmitting = Raptor::VoiceChannel::NONE;
}


bool Microphone::Transmit( void )
{
	return Transmit( Raptor::Game->Cfg.SettingAsDouble("s_mic_delay") );
}


bool Microphone::Transmit( double min_secs, double max_secs, uint8_t flags )
{
#if SDL_VERSION_ATLEAST(2,0,0)
	Uint32 ready_bytes = SDL_GetQueuedAudioSize( Device );
#elif defined(MACOSX_MICROPHONE)
	Uint32 ready_bytes = Recorded.Filled;
#else
	Uint32 ready_bytes = 0;
#endif
	if( ! ready_bytes )
		return false;
	
	Uint32 bpf = Spec.channels * 2;
	if( min_secs && ((ready_bytes / (double)(Spec.freq * bpf)) < min_secs) )
		return false;
	
	Uint32 max_samples = 0x00080000;
	if( max_secs )
		max_samples = std::min<Uint32>( max_samples, Spec.freq * max_secs );
	
	ready_bytes = std::min<Uint32>( ready_bytes, max_samples * bpf );
	
	Packet voice( Raptor::Packet::VOICE );
	voice.AddUShort( Raptor::Game->PlayerID );
	voice.AddUInt( FromObject );
	voice.AddUChar( VoiceChannel | flags );
	voice.AddUInt( 22050 );  // Sample Rate
	voice.AddUInt( ready_bytes / bpf );  // Samples
	
	int16_t buffer[ 2048 ] = {0};
	uint8_t imabuf[ 1024 ] = {0};
	int16_t ima_prev = 0;
	int8_t ima_index = 0;
	Uint32 total_samples = 0;
	Uint32 loops = (ready_bytes + sizeof(buffer) - 1) / sizeof(buffer);
	double volume = Raptor::Game->Cfg.SettingAsDouble( "s_mic_volume", 1., 1. );
	
	for( Uint32 loop = 0; loop < loops; loop ++ )
	{
#if SDL_VERSION_ATLEAST(2,0,0)
		Uint32 bytes = SDL_DequeueAudio( Device, buffer, sizeof(buffer) );
#elif defined(MACOSX_MICROPHONE)
		Uint32 bytes = Recorded.ConsumeBytes( buffer, sizeof(buffer) );
#else
		Uint32 bytes = 0;
#endif
		Uint32 samples = std::min<Uint32>( bytes, sizeof(buffer) ) / bpf;
		if( ! samples )
			break;
		if( (volume > 0.) && (volume < 1.) )  // IMA doesn't handle clipping well, so only volume reduction is allowed before encoding.
		{
			for( Uint32 sample = 0; sample < samples; sample ++ )
				buffer[ sample ] = Num::ScaleInt16( buffer[ sample ], volume );
		}
		IMA::EncodeSegment( buffer, imabuf, samples, &ima_prev, &ima_index );
		voice.AddData( imabuf, (samples + 1) / 2 );  // IMA encodes 2 samples per byte.
		total_samples += samples;
	}
	
	if( (volume > 1.) || ! volume )
		voice.AddFloat( volume );  // Send volume scale for clients to apply after IMA decoding.  0 means auto-gain (experimental).
	
	if( total_samples )
		return Raptor::Game->Net.Send( &voice );
	
	return false;
}
