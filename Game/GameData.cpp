/*
 *  GameData.cpp
 */

#include "GameData.h"

#include "Camera.h"
#include "RaptorGame.h"


GameData::GameData( void )
:	GameObjectIDs( 1 )
,	PlayerIDs( 1 )
{
}


GameData::~GameData()
{
}


uint32_t GameData::AddObject( GameObject *obj )
{
	if( ! obj )
		return 0;
	
	// ID 0 means no ID has been assigned yet.
	if( ! obj->ID )
		obj->ID = GameObjectIDs.NextAvailable();
	
	GameObjects[ obj->ID ] = obj;
	
	obj->Data = this;
	if( this == &(Raptor::Game->Data) )
		obj->ClientInit();
	
	return obj->ID;
}


uint16_t GameData::AddPlayer( Player *player )
{
	if( ! player )
		return 0;
	
	// ID 0 means no ID has been assigned yet.
	if( ! player->ID )
		player->ID = PlayerIDs.NextAvailable();
	
	Players[ player->ID ] = player;

	return player->ID;
}


void GameData::RemoveObject( uint32_t id )
{
	std::map<uint32_t,GameObject*>::iterator obj_iter = GameObjects.find( id );
	if( obj_iter != GameObjects.end() )
	{
		delete obj_iter->second;
		obj_iter->second = NULL;
		GameObjects.erase( obj_iter );
	}
	
	GameObjectIDs.Remove( id );
}


void GameData::RemovePlayer( uint16_t id )
{
	std::map<uint16_t,Player*>::iterator player_iter = Players.find( id );
	if( player_iter != Players.end() )
	{
		delete player_iter->second;
		player_iter->second = NULL;
		Players.erase( player_iter );
	}
	
	PlayerIDs.Remove( id );
}


void GameData::ClearObjects( void )
{
	for( std::map<uint32_t,GameObject*>::iterator obj_iter = GameObjects.begin(); obj_iter != GameObjects.end(); obj_iter ++ )
		delete obj_iter->second;
	
	GameObjects.clear();
	GameObjectIDs.Clear();
	ObjectIDsToRemove.clear();
	Collisions.clear();
	Effects.clear();
}


void GameData::Clear( void )
{
	ClearObjects();
	
	for( std::map<uint16_t,Player*>::iterator player_iter = Players.begin(); player_iter != Players.end(); player_iter ++ )
		delete player_iter->second;
	
	Players.clear();
	PlayerIDs.Clear();
	Properties.clear();
}


void GameData::CheckCollisions( double dt )
{
	Collisions.clear();
	
	std::map< uint32_t, std::list<GameObject*> > moving_can_hit_own_type;
	std::map< uint32_t, std::list<GameObject*> > stationary_can_hit_own_type;
	std::map< uint32_t, std::list<GameObject*> > moving_can_hit_other_types;
	std::map< uint32_t, std::list<GameObject*> > stationary_can_hit_other_types;
	
	for( std::map<uint32_t,GameObject*>::iterator obj_iter = GameObjects.begin(); obj_iter != GameObjects.end(); obj_iter ++ )
	{
		if( obj_iter->second->IsMoving() )
		{
			if( obj_iter->second->CanCollideWithOwnType() )
				moving_can_hit_own_type[ obj_iter->second->Type() ].push_back( obj_iter->second );
			if( obj_iter->second->CanCollideWithOtherTypes() )
				moving_can_hit_other_types[ obj_iter->second->Type() ].push_back( obj_iter->second );
		}
		else
		{
			if( obj_iter->second->CanCollideWithOwnType() )
				stationary_can_hit_own_type[ obj_iter->second->Type() ].push_back( obj_iter->second );
			if( obj_iter->second->CanCollideWithOtherTypes() )
				stationary_can_hit_other_types[ obj_iter->second->Type() ].push_back( obj_iter->second );
		}
	}
	
	// Check moving objects that can hit their own type.
	for( std::map< uint32_t, std::list<GameObject*> >::iterator list_iter = moving_can_hit_own_type.begin(); list_iter != moving_can_hit_own_type.end(); list_iter ++ )
	{
		for( std::list<GameObject *>::iterator obj1_iter = list_iter->second.begin(); obj1_iter != list_iter->second.end(); obj1_iter ++ )
		{
			// Check for collisions with other moving objects of the same type.
			std::list<GameObject *>::iterator obj2_iter = obj1_iter;
			obj2_iter ++;
			for( ; obj2_iter != list_iter->second.end(); obj2_iter ++ )
			{
				if( (*obj1_iter)->WillCollide( *obj2_iter, dt ) )
					Collisions.push_back( Collision(*obj1_iter, *obj2_iter) );
			}
			
			// Check for collisions with stationary objects of the same type.
			for( obj2_iter = stationary_can_hit_own_type[ list_iter->first ].begin(); obj2_iter != stationary_can_hit_own_type[ list_iter->first ].end(); obj2_iter ++ )
			{
				if( (*obj1_iter)->WillCollide( *obj2_iter, dt ) )
					Collisions.push_back( Collision(*obj1_iter, *obj2_iter) );
			}
		}
	}
	
	// Check moving objects that can hit other types.
	for( std::map< uint32_t, std::list<GameObject*> >::iterator list1_iter = moving_can_hit_other_types.begin(); list1_iter != moving_can_hit_other_types.end(); list1_iter ++ )
	{
		// Check for collisions with moving objects of other types.
		std::map< uint32_t, std::list<GameObject*> >::iterator list2_iter = list1_iter;
		list2_iter ++;
		for( ; list2_iter != moving_can_hit_other_types.end(); list2_iter ++ )
		{
			for( std::list<GameObject *>::iterator obj1_iter = list1_iter->second.begin(); obj1_iter != list1_iter->second.end(); obj1_iter ++ )
			{
				for( std::list<GameObject *>::iterator obj2_iter = list2_iter->second.begin(); obj2_iter != list2_iter->second.end(); obj2_iter ++ )
				{
					if( (*obj1_iter)->WillCollide( *obj2_iter, dt ) )
						Collisions.push_back( Collision(*obj1_iter, *obj2_iter) );
				}
			}
		}
		
		// Check for collisions with stationary objects of other types.
		for( list2_iter = stationary_can_hit_other_types.begin(); list2_iter != stationary_can_hit_other_types.end(); list2_iter ++ )
		{
			if( list1_iter->first == list2_iter->first )
				continue;
			
			for( std::list<GameObject *>::iterator obj1_iter = list1_iter->second.begin(); obj1_iter != list1_iter->second.end(); obj1_iter ++ )
			{
				for( std::list<GameObject *>::iterator obj2_iter = list2_iter->second.begin(); obj2_iter != list2_iter->second.end(); obj2_iter ++ )
				{
					if( (*obj1_iter)->WillCollide( *obj2_iter, dt ) )
						Collisions.push_back( Collision(*obj1_iter, *obj2_iter) );
				}
			}
		}
	}
}


void GameData::Update( double dt )
{
	for( std::map<uint32_t,GameObject*>::iterator obj_iter = GameObjects.begin(); obj_iter != GameObjects.end(); obj_iter ++ )
		obj_iter->second->Update( dt );
	
	for( std::list<Effect>::iterator effect_iter = Effects.begin(); effect_iter != Effects.end(); )
	{
		std::list<Effect>::iterator effect_next = effect_iter;
		effect_next ++;
		
		effect_iter->Update( dt );
		if( effect_iter->Finished() )
			Effects.erase( effect_iter );
		
		effect_iter = effect_next;
	}
}


GameObject *GameData::GetObject( uint32_t id )
{
	std::map<uint32_t,GameObject*>::iterator obj_iter = GameObjects.find( id );
	if( obj_iter != GameObjects.end() )
		return obj_iter->second;
	
	return NULL;
}


Player *GameData::GetPlayer( uint16_t id )
{
	std::map<uint16_t,Player*>::iterator player_iter = Players.find( id );
	if( player_iter != Players.end() )
		return player_iter->second;
	
	return NULL;
}
