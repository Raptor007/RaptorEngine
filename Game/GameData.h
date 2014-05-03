/*
 *  GameData.h
 */

#pragma once
class GameData;
class Collision;
class CollisionDataSet;

#include "PlatformSpecific.h"

#include <stdint.h>
#include <map>
#include <list>
#include <set>
#include "Identifier.h"
#include "GameObject.h"
#include "Player.h"
#include "Effect.h"
#include "Clock.h"


class GameData
{
public:
	Identifier<uint32_t> GameObjectIDs;
	std::map<uint32_t,GameObject*> GameObjects;
	
	Identifier<uint16_t> PlayerIDs;
	std::map<uint16_t,Player*> Players;
	
	Clock GameTime;
	
	std::list<Collision> Collisions;
	std::set<uint32_t> ObjectIDsToRemove;
	
	std::list<Effect> Effects;
	
	std::map<std::string,std::string> Properties;
	
	
	GameData( void );
	virtual ~GameData();
	
	uint32_t AddObject( GameObject *obj );
	uint16_t AddPlayer( Player *player );
	void RemoveObject( uint32_t id );
	void RemovePlayer( uint16_t id );
	void ClearObjects( void );
	void Clear( void );
	GameObject *GetObject( uint32_t id );
	Player *GetPlayer( uint16_t id );
	
	void CheckCollisions( double dt );
	void Update( double dt );
};


class Collision
{
public:
	// These names mimic std::pair<GameObject*,GameObject*> that was used before.
	GameObject *first, *second;
	std::string FirstObject, SecondObject;
	
	Collision( GameObject *a, GameObject *b, std::string *a_object = NULL, std::string *b_object = NULL );
	virtual ~Collision();
};


class CollisionDataSet
{
public:
	SDL_Thread *Thread;
	std::list<GameObject*> Objects1, Objects2;
	double dT;
	std::list<Collision> Collisions;
	
	CollisionDataSet( double dt );
	virtual ~CollisionDataSet();
	
	void DetectCollisions( void );
};


int FindCollisionsThread( void *data_set_ptr );
