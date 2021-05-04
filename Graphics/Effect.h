/*
 *  Effect.h
 */

#pragma once
class Effect;

#include "PlatformSpecific.h"
#include "Pos.h"
#include <cstddef>
#include "Animation.h"
#include "SoundOut.h"

class Effect : public Pos3D
{
public:
	double Rotation;
	double Size, Width;
	Vec3D MotionVector;
	double RotationSpeed;
	Animation Anim;
	float Red, Green, Blue, Alpha;
	int AudioChannel;
	Clock Lifetime;
	double SecondsToLive;
	
	Effect( Animation *anim, double size, Mix_Chunk *sound, double loudness, const Pos3D *pos, const Vec3D *motion_vec = NULL, double rotation_speed = 0, double speed_scale = 1., double seconds_to_live = -1. );
	Effect( Animation *anim, double length, double width, Mix_Chunk *sound, double loudness, const Pos3D *pos, const Vec3D *motion_vec = NULL, double rotation_speed = 0, double speed_scale = 1., double seconds_to_live = -1. );
	virtual ~Effect();
	
	void Update( double dt );
	void UpdateAudioPos( void );
	bool Finished( void );
	void Draw( void );
};
