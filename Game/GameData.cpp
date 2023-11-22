/*
 *  GameData.cpp
 */

#include "GameData.h"

#include <cstddef>
#include "Camera.h"
#include "RaptorGame.h"


#define COMPLEX_THREADS 0


GameData::GameData( void )
:	GameObjectIDs( 1 )
,	PlayerIDs( 1 )
{
	AntiJitter = 0.999;
	MaxFrameTime = 0.125;  // Assume dropping below 8 FPS is a momentary hiccup.
	TimeScale = 1.;
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
	
	// Make this ID available again, unless it is lower than the range we're using for new objects.
	if( id <= GameObjectIDs.Initial )
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
	TimeScale = 1.;
}


void GameData::CheckCollisions( double dt )
{
	Collisions.clear();
	
	std::map< uint32_t, std::list<GameObject*> > moving_can_hit_own_type;
	std::map< uint32_t, std::list<GameObject*> > stationary_can_hit_own_type;
	std::map< uint32_t, std::list<GameObject*> > moving_can_hit_other_types;
	std::map< uint32_t, std::list<GameObject*> > stationary_can_hit_other_types;
	std::list<GameObject*> complex;
	std::list<CollisionDataSet> threads;
	
	// Pre-sort by object type and collidability.
	for( std::map<uint32_t,GameObject*>::iterator obj_iter = GameObjects.begin(); obj_iter != GameObjects.end(); obj_iter ++ )
	{
		#if COMPLEX_THREADS
		if( obj_iter->second->ComplexCollisionDetection() )
			complex.push_back( obj_iter->second );
		else
		#endif
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
	
	#if COMPLEX_THREADS
	// Get threads going for complex collision detection.
	for( std::list<GameObject*>::iterator obj_iter = complex.begin(); obj_iter != complex.end(); obj_iter ++ )
	{
		std::vector<CollisionDataSet*> data_sets;
		
		for( int i = 0; i < COMPLEX_THREADS; i ++ )
		{
			threads.push_back( CollisionDataSet(dt) );
			data_sets.push_back( &(threads.back()) );
			data_sets[ i ]->Objects1.push_back( *obj_iter );
		}
		
		uint32_t type = (*obj_iter)->Type();
		
		int index = 0;
		
		for( std::list<GameObject*>::iterator obj2_iter = moving_can_hit_own_type[ type ].begin(); obj2_iter != moving_can_hit_own_type[ type ].end(); obj2_iter ++ )
		{
			data_sets[ index % COMPLEX_THREADS ]->Objects2.push_back( *obj2_iter );
			index ++;
		}
		for( std::map< uint32_t, std::list<GameObject*> >::iterator list_iter = moving_can_hit_other_types.begin(); list_iter != moving_can_hit_other_types.end(); list_iter ++ )
		{
			if( list_iter->first == type )
				continue;
			
			for( std::list<GameObject*>::iterator obj2_iter = list_iter->second.begin(); obj2_iter != list_iter->second.end(); obj2_iter ++ )
			{
				data_sets[ index % COMPLEX_THREADS ]->Objects2.push_back( *obj2_iter );
				index ++;
			}
		}
		
		if( (*obj_iter)->IsMoving() )
		{
			for( std::list<GameObject*>::iterator obj2_iter = stationary_can_hit_own_type[ type ].begin(); obj2_iter != stationary_can_hit_own_type[ type ].end(); obj2_iter ++ )
			{
				data_sets[ index % COMPLEX_THREADS ]->Objects2.push_back( *obj2_iter );
				index ++;
			}
			for( std::map< uint32_t, std::list<GameObject*> >::iterator list_iter = stationary_can_hit_other_types.begin(); list_iter != stationary_can_hit_other_types.end(); list_iter ++ )
			{
				if( list_iter->first == type )
					continue;
				
				for( std::list<GameObject*>::iterator obj2_iter = list_iter->second.begin(); obj2_iter != list_iter->second.end(); obj2_iter ++ )
				{
					data_sets[ index % COMPLEX_THREADS ]->Objects2.push_back( *obj2_iter );
					index ++;
				}
			}
		}
		
		std::list<GameObject*>::iterator obj2_iter = obj_iter;
		obj2_iter ++;
		for( ; obj2_iter != complex.end(); obj2_iter ++ )
		{
			data_sets[ index % COMPLEX_THREADS ]->Objects2.push_back( *obj2_iter );
			index ++;
		}
		
		for( int i = 0; i < COMPLEX_THREADS; i ++ )
			data_sets[ i ]->Thread = SDL_CreateThread( &FindCollisionsThread, data_sets[ i ] );
	}
	#endif
	
	std::string a_object, b_object;
	
	// Check moving objects that can hit their own type.
	for( std::map< uint32_t, std::list<GameObject*> >::iterator list_iter = moving_can_hit_own_type.begin(); list_iter != moving_can_hit_own_type.end(); list_iter ++ )
	{
		for( std::list<GameObject*>::iterator obj1_iter = list_iter->second.begin(); obj1_iter != list_iter->second.end(); obj1_iter ++ )
		{
			// Check for collisions with other moving objects of the same type.
			std::list<GameObject*>::iterator obj2_iter = obj1_iter;
			obj2_iter ++;
			for( ; obj2_iter != list_iter->second.end(); obj2_iter ++ )
			{
				if( (*obj1_iter)->WillCollide( *obj2_iter, dt, &a_object, &b_object ) )
				{
					Collisions.push_back( Collision( *obj1_iter, *obj2_iter, &a_object, &b_object ) );
					a_object.clear();
					b_object.clear();
				}
			}
			
			// Check for collisions with stationary objects of the same type.
			for( obj2_iter = stationary_can_hit_own_type[ list_iter->first ].begin(); obj2_iter != stationary_can_hit_own_type[ list_iter->first ].end(); obj2_iter ++ )
			{
				if( (*obj1_iter)->WillCollide( *obj2_iter, dt, &a_object, &b_object ) )
				{
					Collisions.push_back( Collision( *obj1_iter, *obj2_iter, &a_object, &b_object ) );
					a_object.clear();
					b_object.clear();
				}
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
			for( std::list<GameObject*>::iterator obj1_iter = list1_iter->second.begin(); obj1_iter != list1_iter->second.end(); obj1_iter ++ )
			{
				for( std::list<GameObject*>::iterator obj2_iter = list2_iter->second.begin(); obj2_iter != list2_iter->second.end(); obj2_iter ++ )
				{
					if( (*obj1_iter)->WillCollide( *obj2_iter, dt, &a_object, &b_object ) )
					{
						Collisions.push_back( Collision( *obj1_iter, *obj2_iter, &a_object, &b_object ) );
						a_object.clear();
						b_object.clear();
					}
				}
			}
		}
		
		// Check for collisions with stationary objects of other types.
		for( list2_iter = stationary_can_hit_other_types.begin(); list2_iter != stationary_can_hit_other_types.end(); list2_iter ++ )
		{
			if( list1_iter->first == list2_iter->first )
				continue;
			
			for( std::list<GameObject*>::iterator obj1_iter = list1_iter->second.begin(); obj1_iter != list1_iter->second.end(); obj1_iter ++ )
			{
				for( std::list<GameObject*>::iterator obj2_iter = list2_iter->second.begin(); obj2_iter != list2_iter->second.end(); obj2_iter ++ )
				{
					if( (*obj1_iter)->WillCollide( *obj2_iter, dt, &a_object, &b_object ) )
					{
						Collisions.push_back( Collision( *obj1_iter, *obj2_iter, &a_object, &b_object ) );
						a_object.clear();
						b_object.clear();
					}
				}
			}
		}
	}
	
	// Wait for threads to end, and gather their results.
	for( std::list< CollisionDataSet >::iterator thread_iter = threads.begin(); thread_iter != threads.end(); thread_iter ++ )
	{
		SDL_WaitThread( thread_iter->Thread, NULL );
		for( std::list<Collision>::iterator collision_iter = thread_iter->Collisions.begin(); collision_iter != thread_iter->Collisions.end(); collision_iter ++ )
			Collisions.push_back( *collision_iter );
	}
}


void GameData::Update( double dt )
{
	AntiJitter = Raptor::Game->Cfg.SettingAsDouble("net_anti_jitter",0.999);
	
	for( std::map<uint32_t,GameObject*>::iterator obj_iter = GameObjects.begin(); obj_iter != GameObjects.end(); obj_iter ++ )
		obj_iter->second->Update( dt );  // NOTE: Do not multiply by TimeScale here; dt will be scaled by the game's Update method.
	
	for( std::list<Effect>::iterator effect_iter = Effects.begin(); effect_iter != Effects.end(); )
	{
		std::list<Effect>::iterator effect_next = effect_iter;
		effect_next ++;
		
		effect_iter->Update( dt );
		if( effect_iter->Finished() )
			Effects.erase( effect_iter );
		
		effect_iter = effect_next;
	}
	
	TimeScale = PropertyAsDouble("time_scale",1.);
	MaxFrameTime = 0.125 * TimeScale;
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


// -----------------------------------------------------------------------------


bool GameData::HasProperty( std::string name ) const
{
	std::map<std::string, std::string>::const_iterator found = Properties.find( name );
	return ( found != Properties.end() );
}


std::string GameData::PropertyAsString( std::string name, const char *ifndef, const char *ifempty ) const
{
	std::map<std::string, std::string>::const_iterator found = Properties.find( name );
	if( found != Properties.end() )
	{
		if( found->second.empty() && ifempty )
			return std::string(ifempty);
		return found->second;
	}
	if( ifndef )
		return std::string(ifndef);
	return "";
}


double GameData::PropertyAsDouble( std::string name, double ifndef, double ifempty ) const
{
	std::map<std::string, std::string>::const_iterator found = Properties.find( name );
	if( found != Properties.end() )
	{
		if( found->second.empty() )
			return ifempty;
		return Str::AsDouble( found->second );
	}
	return ifndef;
}


int GameData::PropertyAsInt( std::string name, int ifndef, int ifempty ) const
{
	std::map<std::string, std::string>::const_iterator found = Properties.find( name );
	if( found != Properties.end() )
	{
		if( found->second.empty() )
			return ifempty;
		return Str::AsInt( found->second );
	}
	return ifndef;
}


bool GameData::PropertyAsBool( std::string name, bool ifndef, bool ifempty ) const
{
	std::map<std::string, std::string>::const_iterator found = Properties.find( name );
	if( found != Properties.end() )
	{
		if( found->second.empty() )
			return ifempty;
		return Str::AsBool( found->second );
	}
	return ifndef;
}


std::vector<double> GameData::PropertyAsDoubles( std::string name ) const
{
	std::vector<double> results;
	
	std::map<std::string, std::string>::const_iterator found = Properties.find( name );
	if( found != Properties.end() )
	{
		bool seeking = true;
		for( const char *buffer_ptr = found->second.c_str(); *buffer_ptr; buffer_ptr ++ )
		{
			bool numeric_char = strchr( "0123456789.-", *buffer_ptr );
			if( seeking && numeric_char )
			{
				results.push_back( atof(buffer_ptr) );
				seeking = false;
			}
			else if( ! numeric_char )
				seeking = true;
		}
	}
	
	return results;
}


std::vector<int> GameData::PropertyAsInts( std::string name ) const
{
	std::vector<int> results;
	
	std::map<std::string, std::string>::const_iterator found = Properties.find( name );
	if( found != Properties.end() )
	{
		bool seeking = true;
		for( const char *buffer_ptr = found->second.c_str(); *buffer_ptr; buffer_ptr ++ )
		{
			bool numeric_char = strchr( "0123456789.-", *buffer_ptr );
			if( seeking && numeric_char )
			{
				results.push_back( atoi(buffer_ptr) );
				seeking = false;
			}
			else if( ! numeric_char )
				seeking = true;
		}
	}
	
	return results;
}


// -----------------------------------------------------------------------------


Collision::Collision( GameObject *a, GameObject *b, std::string *a_object, std::string *b_object )
{
	first = a;
	second = b;
	
	if( a_object )
		FirstObject = *a_object;
	if( b_object )
		SecondObject = *b_object;
}


Collision::~Collision()
{
}


// -----------------------------------------------------------------------------



CollisionDataSet::CollisionDataSet( double dt )
{
	Thread = NULL;
	dT = dt;
}


CollisionDataSet::~CollisionDataSet()
{
}


void CollisionDataSet::DetectCollisions( void )
{
	std::string a_object, b_object;
	
	for( std::list<GameObject*>::const_iterator obj1_iter = Objects1.begin(); obj1_iter != Objects1.end(); obj1_iter ++ )
	{
		for( std::list<GameObject*>::const_iterator obj2_iter = Objects2.begin(); obj2_iter != Objects2.end(); obj2_iter ++ )
		{
			if( (*obj1_iter)->WillCollide( *obj2_iter, dT, &a_object, &b_object ) )
			{
				Collisions.push_back( Collision( *obj1_iter, *obj2_iter, &a_object, &b_object ) );
				a_object.clear();
				b_object.clear();
			}
		}
	}
}


// -----------------------------------------------------------------------------


int FindCollisionsThread( void *data_set_ptr )
{
	CollisionDataSet *data_set = (CollisionDataSet*) data_set_ptr;
	data_set->DetectCollisions();
	
	return 0;
}
