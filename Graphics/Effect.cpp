/*
 *  Effect.cpp
 */

#include "Effect.h"

#include <cmath>
#include "Math2D.h"
#include "SoundOut.h"
#include "RaptorGame.h"


Effect::Effect( Animation *anim, double size, Mix_Chunk *sound, double loudness, const Pos3D *pos, const Vec3D *motion_vec, double rotation_speed, double speed_scale, double seconds_to_live ) : Pos3D( pos )
{
	Anim.BecomeInstance( anim );
	Anim.Speed = speed_scale;
	
	AudioChannel = -1;
	if( sound )
		AudioChannel = Raptor::Game->Snd.PlayPanned( sound, X, Y, Z, loudness );
	
	Size = size;
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
	if( (SecondsToLive > 0.) && (Lifetime.ElapsedSeconds() > SecondsToLive) )
		return true;
	return false;
}


void Effect::Draw( void )
{
	// Need this to display a texture.
	glEnable( GL_TEXTURE_2D );
	
	glColor4f( Red, Green, Blue, Alpha );
	
	// Bind the texture to which subsequent calls refer to.
	glBindTexture( GL_TEXTURE_2D, Anim.CurrentFrame() );
	
	// Calculate corners.
	Vec3D tl( Raptor::Game->Cam.Up.X - Raptor::Game->Cam.Right.X, Raptor::Game->Cam.Up.Y - Raptor::Game->Cam.Right.Y, Raptor::Game->Cam.Up.Z - Raptor::Game->Cam.Right.Z );
	tl.ScaleTo( Size * sqrt(0.5) );
	tl.RotateAround( &(Raptor::Game->Cam.Fwd), Rotation );
	Vec3D tr = tl;
	tr.RotateAround( &(Raptor::Game->Cam.Fwd), 90. );
	Vec3D br = tl;
	br.RotateAround( &(Raptor::Game->Cam.Fwd), 180. );
	Vec3D bl = tl;
	bl.RotateAround( &(Raptor::Game->Cam.Fwd), 270. );
	
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
}

