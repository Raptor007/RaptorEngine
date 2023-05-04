/*
 *  GameObject.h
 */

#pragma once
class GameObject;

#include "PlatformSpecific.h"

#include <stdint.h>
#include "Pos.h"
#include "Clock.h"
#include "Packet.h"
#include "GameData.h"
#include "Player.h"


class GameObject : public Pos3D
{
public:
	uint32_t ID;
	uint16_t PlayerID;
	
	GameData *Data;
	
	Clock Lifetime;
	
	Vec3D MotionVector;
	double RollRate;
	double PitchRate;
	double YawRate;
	
	Pos3D PrevPos;
	bool SmoothPos;
	
	
	GameObject( uint32_t id = 0, uint32_t type_code = '    ', uint16_t player_id = 0 );
	GameObject( const GameObject &other );
	virtual ~GameObject();
	
	virtual bool ClientSide( void ) const;
	virtual void ClientInit( void );
	
	virtual uint32_t Type( void ) const;
	virtual Player *Owner( void ) const;
	virtual bool PlayerShouldUpdateServer( void ) const;
	virtual bool ServerShouldUpdatePlayer( void ) const;
	virtual bool ServerShouldUpdateOthers( void ) const;
	virtual bool CanCollideWithOwnType( void ) const;
	virtual bool CanCollideWithOtherTypes( void ) const;
	virtual bool IsMoving( void ) const;
	virtual bool ComplexCollisionDetection( void ) const;
	
	virtual void AddToInitPacket( Packet *packet, int8_t precision = 0 );
	virtual void ReadFromInitPacket( Packet *packet, int8_t precision = 0 );
	virtual void AddToUpdatePacket( Packet *packet, int8_t precision = 0 );
	virtual void ReadFromUpdatePacket( Packet *packet, int8_t precision = 0 );
	virtual void AddToUpdatePacketFromServer( Packet *packet, int8_t precision = 0 );
	virtual void ReadFromUpdatePacketFromServer( Packet *packet, int8_t precision = 0 );
	virtual void AddToUpdatePacketFromClient( Packet *packet, int8_t precision = 0 );
	virtual void ReadFromUpdatePacketFromClient( Packet *packet, int8_t precision = 0 );
	
	virtual bool WillCollide( const GameObject *other, double dt, std::string *this_object = NULL, std::string *other_object = NULL ) const;
	GameObject *Trace( double dt, double precision = 0.01 ) const;
	virtual void Update( double dt );
	virtual void SendUpdate( int8_t precision );
	
	virtual void Draw( void );
	
private:
	uint32_t TypeCode;
};
