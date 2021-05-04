/*
 *  Effect.cpp
 */

#include "Effect.h"

#include <cmath>
#include "Math2D.h"
#include "SoundOut.h"
#include "RaptorGame.h"


Effect::Effect( Animation *anim, double size, Mix_Chunk *sound, double loudness, const Pos3D *pos, const Vec3D *motion_vec, double rotation_speed, double speed_scale, double seconds_to_live )
: Pos3D( pos )
{
	Anim.BecomeInstance( anim );
	Anim.Speed = speed_scale;
	
	AudioChannel = -1;
	if( sound )
		AudioChannel = Raptor::Game->Snd.PlayPanned( sound, X, Y, Z, loudness );
	
	Size = size;
	Width = size;
	Rotation = 0.;
	
	if( motion_vec )
		MotionVector = *motion_vec;
	RotationSpeed = rotation_speed;
	
	SecondsToLive = seconds_to_live;
	
	Red = 1.0f;
	Green = 1.0f;
	Blue = 1.0f;
	Alpha = 1.0f;
}


Effect::Effect( Animation *anim, double length, double width, Mix_Chunk *sound, double loudness, const Pos3D *pos, const Vec3D *motion_vec, double rotation_speed, double speed_scale, double seconds_to_live )
: Pos3D( pos )
{
	Anim.BecomeInstance( anim );
	Anim.Speed = speed_scale;
	
	AudioChannel = -1;
	if( sound )
		AudioChannel = Raptor::Game->Snd.PlayPanned( sound, X, Y, Z, loudness );
	
	Size = length;
	Width = width;
	Rotation = 0.;
	
	if( motion_vec )
		MotionVector = *motion_vec;
	RotationSpeed = rotation_speed;
	
	SecondsToLive = seconds_to_live;
	
	Red = 1.0f;
	Green = 1.0f;
	Blue = 1.0f;
	Alpha = 1.0f;
}


Effect::~Effect()
{
}


void Effect::Update( double dt )
{
	X += MotionVector.X * dt;
	Y += MotionVector.Y * dt;
	Z += MotionVector.Z * dt;
	Rotation += RotationSpeed * dt;
	
	UpdateAudioPos();
	
	// Don't start the animation until Lifetime.CountUpToSecs is complete.
	if( Lifetime.Progress() < 1. )
		Anim.Start();
}


void Effect::UpdateAudioPos( void )
{
	if( AudioChannel != -1 )
		AudioChannel = Raptor::Game->Snd.SetPos( AudioChannel, X, Y, Z );
}


bool Effect::Finished( void )
{
	if( Anim.Finished() )
		return true;
	if( (SecondsToLive > 0.) && (Lifetime.ElapsedSeconds() > SecondsToLive + Lifetime.CountUpToSecs) )
		return true;
	return false;
}


void Effect::Draw( void )
{
	// Allow using Lifetime.CountUpToSecs to queue a future effect.
	if( Lifetime.Progress() < 1. )
		return;
	
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, Anim.CurrentFrame() );
	glColor4f( Red, Green, Blue, Alpha );
	
	// Calculate corners.
	Vec3D tl = Raptor::Game->Cam.Up * Size/2. - Raptor::Game->Cam.Right * Width/2.;
	Vec3D tr = tl + Raptor::Game->Cam.Right * Width;
	Vec3D bl = tr;
	Vec3D br = tl;
	tl.RotateAround( &(Raptor::Game->Cam.Fwd), Rotation );
	tr.RotateAround( &(Raptor::Game->Cam.Fwd), Rotation );
	br.RotateAround( &(Raptor::Game->Cam.Fwd), Rotation + 180. );
	bl.RotateAround( &(Raptor::Game->Cam.Fwd), Rotation + 180. );
	
	glBegin( GL_QUADS );
		
		// Top-left
		glTexCoord2i( 0, 0 );
		glVertex3d( X + tl.X, Y + tl.Y, Z + tl.Z );
		
		// Bottom-left
		glTexCoord2i( 0, 1 );
		glVertex3d( X + bl.X, Y + bl.Y, Z + bl.Z );
		
		// Bottom-right
		glTexCoord2i( 1, 1 );
		glVertex3d( X + br.X, Y + br.Y, Z + br.Z );
		
		// Top-right
		glTexCoord2i( 1, 0 );
		glVertex3d( X + tr.X, Y + tr.Y, Z + tr.Z );
		
	glEnd();
	
	glDisable( GL_TEXTURE_2D );
	glColor4f( 1.f, 1.f, 1.f, 1.f );
}
