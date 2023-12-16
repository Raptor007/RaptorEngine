/*
 *  GameObject.cpp
 */

#include "GameObject.h"

#include <cstddef>
#include <cmath>
#include "Num.h"
#include "RaptorGame.h"


#define SMOOTH_RADIUS 128.


GameObject::GameObject( uint32_t id, uint32_t type_code, uint16_t player_id ) : Pos3D()
{
	ID = id;
	TypeCode = type_code;
	PlayerID = player_id;
	Data = NULL;
	Fwd.Set( 1., 0., 0. );
	Up.Set( 0., 0., 1. );
	FixVectors();
	MotionVector.Set( 0., 0., 0. );
	RollRate = 0.;
	PitchRate = 0.;
	YawRate = 0.;
	SmoothPos = true;
}


GameObject::GameObject( const GameObject &other ) : Pos3D( other )
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
	SmoothPos = other.SmoothPos;
}


GameObject::~GameObject()
{
}


bool GameObject::ClientSide( void ) const
{
	return (Data == &(Raptor::Game->Data));
}


void GameObject::ClientInit( void )
{
}


uint32_t GameObject::Type( void ) const
{
	return TypeCode;
}

Player *GameObject::Owner( void ) const
{
	return Data ? Data->GetPlayer( PlayerID ) : NULL;
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


bool GameObject::ComplexCollisionDetection( void ) const
{
	return false;
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
	if( precision >= 0 )
	{
		packet->AddFloat( RollRate );
		packet->AddFloat( PitchRate );
		packet->AddFloat( YawRate );
	}
}


void GameObject::ReadFromUpdatePacket( Packet *packet, int8_t precision )
{
	PrevPos.Copy( this );
	PrevMotionVector.Copy( &MotionVector );
	
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
	if( precision >= 0 )
	{
		RollRate  = packet->NextFloat();
		PitchRate = packet->NextFloat();
		YawRate   = packet->NextFloat();
	}
	FixVectors();
	
	// Remove jitter from delayed position updates by moving object forward to predicted position.
	double speed = MotionVector.Length();
	if( SmoothPos && Data && Data->AntiJitter && (Dist(&PrevPos) < SMOOTH_RADIUS) && speed )
	{
		Vec3D unit_motion = MotionVector.Unit();
		double dist_behind = PrevPos.DistAlong( &unit_motion, this );
		if( dist_behind > 0. )
		{
			if( speed * 1.001 < PrevMotionVector.Length() )
				dist_behind *= std::min<double>( 0.75, speed / 100. );  // Reduce rubber-banding when stopping.
			MoveAlong( &unit_motion, dist_behind * Data->AntiJitter );
		}
	}
}


void GameObject::AddToUpdatePacketFromServer( Packet *packet, int8_t precision )
{
	AddToUpdatePacket( packet, precision );
	packet->AddUShort( PlayerID );
}


void GameObject::ReadFromUpdatePacketFromServer( Packet *packet, int8_t precision )
{
	ReadFromUpdatePacket( packet, precision );
	PlayerID = packet->NextUShort();
	
	// Further reduce jitter by averaging anti-jittered received position with previous predicted position.
	if( SmoothPos && Data && Data->AntiJitter && (Dist(&PrevPos) < SMOOTH_RADIUS) )
	{
		X   = (X   + PrevPos.X   * Data->AntiJitter) / (1. + Data->AntiJitter);
		Y   = (Y   + PrevPos.Y   * Data->AntiJitter) / (1. + Data->AntiJitter);
		Z   = (Z   + PrevPos.Z   * Data->AntiJitter) / (1. + Data->AntiJitter);
		Fwd = (Fwd + PrevPos.Fwd * Data->AntiJitter) / (1. + Data->AntiJitter);
		Up  = (Up  + PrevPos.Up  * Data->AntiJitter) / (1. + Data->AntiJitter);
		FixVectors();
	}
}


void GameObject::AddToUpdatePacketFromClient( Packet *packet, int8_t precision )
{
	AddToUpdatePacket( packet, precision );
}


void GameObject::ReadFromUpdatePacketFromClient( Packet *packet, int8_t precision )
{
	ReadFromUpdatePacket( packet, precision );
}


bool GameObject::WillCollide( const GameObject *other, double dt, std::string *this_object, std::string *other_object ) const
{
	return false;
}


GameObject *GameObject::Trace( double dt, double precision ) const
{
	if( ! Data )
		return NULL;
	
	std::set<GameObject*> will_hit;
	
	// First find every object that this will collide with.
	for( std::map<uint32_t,GameObject*>::iterator obj_iter = Data->GameObjects.begin(); obj_iter != Data->GameObjects.end(); obj_iter ++ )
	{
		if( obj_iter->second == this )
			continue;
		if( obj_iter->second->Type() == Type() )
		{
			if( ! CanCollideWithOwnType() )
				continue;
		}
		else if( ! CanCollideWithOtherTypes() )
			continue;
		else if( ! obj_iter->second->CanCollideWithOtherTypes() )
			continue;
		if( WillCollide( obj_iter->second, dt ) )
			will_hit.insert( obj_iter->second );
	}
	
	// Binary search slices of time until we find the first collision by itself or hit the precision limit.
	double ddt = dt / -2.;
	while( (will_hit.size() > 1) && (fabs(ddt) >= precision) )
	{
		dt += ddt;
		ddt = fabs(ddt) / 2.;
		std::set<GameObject*> these_hits;
		for( std::set<GameObject*>::iterator hit_iter = will_hit.begin(); hit_iter != will_hit.end(); hit_iter ++ )
		{
			if( WillCollide( *hit_iter, dt ) )
				these_hits.insert( *hit_iter );
		}
		if( these_hits.size() )
		{
			will_hit = these_hits;
			ddt *= -1.;
		}
	}
	
	return will_hit.size() ? *(will_hit.begin()) : NULL;
}


void GameObject::Update( double dt )
{
	PrevPos.Copy( this );
	PrevMotionVector.Copy( &MotionVector );
	
	// Limit over-prediction from momentary hiccups, such as when loading assets mid-game.
	if( (dt > Data->MaxFrameTime) && (Data->MaxFrameTime > 0.) )
		dt = Data->MaxFrameTime;
	
	Roll( dt * RollRate );
	Pitch( dt * PitchRate );
	Yaw( dt * YawRate );
	Move( MotionVector.X * dt, MotionVector.Y * dt, MotionVector.Z * dt );
}


void GameObject::SendUpdate( int8_t precision )
{
	Packet update( Raptor::Packet::UPDATE );
	update.AddChar( precision );
	update.AddUInt( 1 );
	update.AddUInt( ID );
	if( ClientSide() )
	{
		AddToUpdatePacketFromClient( &update, precision );
		Raptor::Game->Net.Send( &update );
	}
	else
	{
		AddToUpdatePacketFromServer( &update, precision );
		Raptor::Server->Net.SendAll( &update );
	}
}


void GameObject::Draw( void )
{
}
