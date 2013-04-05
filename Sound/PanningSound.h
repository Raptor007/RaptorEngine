/*
 *  PanningSound.h
 */

#pragma once
class PanningSound;

#include "platforms.h"

#include <stdint.h>
#include "Vec.h"


class PanningSound : public Vec3D
{
public:
	int AudioChannel;
	uint32_t ObjectID;
	double Loudness;
	
	PanningSound( void );
	PanningSound( int audio_channel, double x, double y, double z, double loudness = 1. );
	PanningSound( int audio_channel, uint32_t object_id, double loudness = 1. );
	~PanningSound();
	
	void Update( void );
};
