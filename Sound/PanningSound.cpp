/*
 *  PanningSound.cpp
 */

#include "PanningSound.h"
#include <cmath>
#include "Num.h"
#include "Math2D.h"
#include "RaptorGame.h"


PanningSound::PanningSound( void ) : Vec3D( 0., 0., 0. )
{
	AudioChannel = -1;
	ObjectID = 0;
	Loudness = 1.;
}


PanningSound::PanningSound( int audio_channel, double x, double y, double z, double loudness ) : Vec3D( x, y, z )
{
	AudioChannel = audio_channel;
	ObjectID = 0;
	Loudness = loudness;
}


PanningSound::PanningSound( int audio_channel, uint32_t object_id, double loudness )
{
	AudioChannel = audio_channel;
	ObjectID = object_id;
	Loudness = loudness;
	Update();
}


PanningSound::~PanningSound()
{
}


void PanningSound::Update( void )
{
	if( ObjectID )
	{
		GameObject *obj = Raptor::Game->Data.GetObject( ObjectID );
		if( obj )
			Set( obj->X, obj->Y, obj->Z );
	}
}
