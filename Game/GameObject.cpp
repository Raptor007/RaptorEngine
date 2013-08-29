/*
 *  GameObject.cpp
 */

#include "GameObject.h"

#include <cstddef>
#include "Num.h"
#include "RaptorGame.h"


#define SMOOTH_RADIUS 128.


GameObject::GameObject( uint32_t id, uint32_t type_code, uint16_t player_id )
{
	ID = id;
	TypeCode = type_code;
	PlayerID = player_id;
	Data = NULL;
	MotionVector.Set( 0., 0., 0. );
	RollRate = 0.;
	PitchRate = 0.;
	YawRate = 0.;
	NextUpdateTimeTweak = 0.;
	SmoothPos = true;
}


GameObject::GameObject( const GameObject &other )
{
	ID = other.ID;
	TypeCode = other.TypeCode;
	PlayerID = other.PlayerID;
	Data = other.Data;
	MotionVector = other.MotionVector;
	RollRate = other.RollRate;
	PitchRate = other.PitchRate;
	YawRate = other.YawRate;
	Lifetime = other.Lifetime;
	NextUpdateTimeTweak = 0.;
	SmoothPos = true;
}


GameObject::~GameObject()
{
}


void GameObject::ClientInit( void )
{
}


uint32_t GameObject::Type( void ) const
{
	return TypeCode;
}


bool GameObject::PlayerShouldUpdateServer( void ) const
{
	return true;
}

bool GameObject::ServerShouldUpdatePlayer( void ) const
{
	return false;
}

bool GameObject::ServerShouldUpdateOthers( void ) const
{
	return true;
}

bool GameObject::CanCollideWithOwnType( void ) const
{
	return false;
}

bool GameObject::CanCollideWithOtherTypes( void ) const
{
	return false;
}


bool GameObject::IsMoving( void ) const
{
	return ( RollRate || PitchRate || YawRate || MotionVector.Length() );
}


void GameObject::AddToInitPacket( Packet *packet, int8_t precision )
{
	AddToUpdatePacketFromServer( packet, precision );
}


void GameObject::ReadFromInitPacket( Packet *packet, int8_t precision )
{
	bool smooth_pos = SmoothPos;
	SmoothPos = false;
	ReadFromUpdatePacketFromServer( packet, precision );
	SmoothPos = smooth_pos;
}


void GameObject::AddToUpdatePacket( Packet *packet, int8_t precision )
{
	packet->AddDouble( X );
	packet->AddDouble( Y );
	packet->AddDouble( Z );
	if( precision >= 0 )
	{
		packet->AddFloat( Fwd.X );
		packet->AddFloat( Fwd.Y );
		packet->AddFloat( Fwd.Z );
	}
	else
	{
		packet->AddShort( Num::UnitFloatTo16(Fwd.X) );
		packet->AddShort( Num::UnitFloatTo16(Fwd.Y) );
		packet->AddShort( Num::UnitFloatTo16(Fwd.Z) );
	}
	if( precision >= 1 )
	{
		packet->AddFloat( Up.X );
		packet->AddFloat( Up.Y );
		packet->AddFloat( Up.Z );
	}
	else if( precision >= 0 )
	{
		packet->AddShort( Num::UnitFloatTo16(Up.X) );
		packet->AddShort( Num::UnitFloatTo16(Up.Y) );
		packet->AddShort( Num::UnitFloatTo16(Up.Z) );
	}
	else
	{
		packet->AddChar( Num::UnitFloatTo8(Up.X) );
		packet->AddChar( Num::UnitFloatTo8(Up.Y) );
		packet->AddChar( Num::UnitFloatTo8(Up.Z) );
	}
	packet->AddFloat( MotionVector.X );
	packet->AddFloat( MotionVector.Y );
	packet->AddFloat( MotionVector.Z );
}


void GameObject::ReadFromUpdatePacket( Packet *packet, int8_t precision )
{
	PrevPos.Copy( this );
	
	X = packet->NextDouble();
	Y = packet->NextDouble();
	Z = packet->NextDouble();
	if( precision >= 0 )
	{
		Fwd.X = packet->NextFloat();
		Fwd.Y = packet->NextFloat();
		Fwd.Z = packet->NextFloat();
	}
	else
	{
		Fwd.X = Num::UnitFloatFrom16( packet->NextShort() );
		Fwd.Y = Num::UnitFloatFrom16( packet->NextShort() );
		Fwd.Z = Num::UnitFloatFrom16( packet->NextShort() );
	}
	if( precision >= 1 )
	{
		Up.X = packet->NextFloat();
		Up.Y = packet->NextFloat();
		Up.Z = packet->NextFloat();
	}
	else if( precision >= 0 )
	{
		Up.X = Num::UnitFloatFrom16( packet->NextShort() );
		Up.Y = Num::UnitFloatFrom16( packet->NextShort() );
		Up.Z = Num::UnitFloatFrom16( packet->NextShort() );
	}
	else
	{
		Up.X = Num::UnitFloatFrom8( packet->NextChar() );
		Up.Y = Num::UnitFloatFrom8( packet->NextChar() );
		Up.Z = Num::UnitFloatFrom8( packet->NextChar() );
	}
	MotionVector.X = packet->NextFloat();
	MotionVector.Y = packet->NextFloat();
	MotionVector.Z = packet->NextFloat();
	
	if( SmoothPos && (Dist(&PrevPos) < SMOOTH_RADIUS) )
	{
		// Average with the previously-calculated position to reduce jitter.
		
		X += PrevPos.X;
		Y += PrevPos.Y;
		Z += PrevPos.Z;
		X /= 2.;
		Y /= 2.;
		Z /= 2.;
		
		Fwd += PrevPos.Fwd;
		Fwd /= 2.;
		Up += PrevPos.Up;
		Up /= 2.;
	}
	
	FixVectors();
}


void GameObject::AddToUpdatePacketFromServer( Packet *packet, int8_t precision )
{
	AddToUpdatePacket( packet, precision );
	packet->AddUShort( PlayerID );
}


void GameObject::ReadFromUpdatePacketFromServer( Packet *packet, int8_t precision )
{
	// Client-side anti-lag.
	NextUpdateTimeTweak -= Raptor::Game->FrameTime / 2.;
	NextUpdateTimeTweak += Raptor::Game->Net.MedianPing() / 4000.;
	
	ReadFromUpdatePacket( packet, precision );
	PlayerID = packet->NextUShort();
}


void GameObject::AddToUpdatePacketFromClient( Packet *packet, int8_t precision )
{
	AddToUpdatePacket( packet, precision );
}


void GameObject::ReadFromUpdatePacketFromClient( Packet *packet, int8_t precision )
{
	ReadFromUpdatePacket( packet, precision );
}


bool GameObject::WillCollide( const GameObject *other, double dt ) const
{
	return false;
}


void GameObject::Update( double dt )
{
	PrevPos.Copy( this );
	
	dt += NextUpdateTimeTweak;
	NextUpdateTimeTweak = 0.;
	
	Roll( dt * RollRate );
	Pitch( dt * PitchRate );
	Yaw( dt * YawRate );
	Move( MotionVector.X * dt, MotionVector.Y * dt, MotionVector.Z * dt );
}


void GameObject::Draw( void )
{
}
