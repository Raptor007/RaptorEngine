/*
 *  GameData.cpp
 */

#include "GameData.h"

#include <cstddef>
#include <cfloat>
#include "Camera.h"
#include "RaptorGame.h"


GameData::GameData( void )
:	GameObjectIDs( 1 )
,	PlayerIDs( 1 )
{
	AntiJitter = 0.999;
	MaxFrameTime = 0.5;  // Assume dropping below 2 FPS is a momentary hiccup.
	TimeScale = 1.;
	ThreadCount = 0;
}


GameData::~GameData()
{
}


uint32_t GameData::AddObject( GameObject *obj )
{
	if( ! obj )
		return 0;
	
	obj->Data = this;
	
	// ID 0 means no ID has been assigned yet.
	if( ! obj->ID )
		obj->ID = GameObjectIDs.NextAvailable();
	
	uint32_t obj_id = obj->ID;
	if( obj_id )
	{
		GameObjects[ obj_id ] = obj;
		
		if( this == &(Raptor::Game->Data) )
			obj->ClientInit();
	}
	
	return obj_id;
}


uint16_t GameData::AddPlayer( Player *player )
{
	if( ! player )
		return 0;
	
	Lock.Lock();
	
	// ID 0 means no ID has been assigned yet.
	if( ! player->ID )
		player->ID = PlayerIDs.NextAvailable();
	
	uint16_t player_id = player->ID;
	if( player_id )
		Players[ player_id ] = player;
	
	Lock.Unlock();
	
	return player_id;
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
	Lock.Lock();
	
	std::map<uint16_t,Player*>::iterator player_iter = Players.find( id );
	if( player_iter != Players.end() )
	{
		delete player_iter->second;
		player_iter->second = NULL;
		Players.erase( player_iter );
	}
	
	PlayerIDs.Remove( id );
	
	Lock.Unlock();
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
	Lock.Lock();
	
	ClearObjects();
	
	for( std::map<uint16_t,Player*>::iterator player_iter = Players.begin(); player_iter != Players.end(); player_iter ++ )
		delete player_iter->second;
	
	Players.clear();
	PlayerIDs.Clear();
	Properties.clear();
	
	GameTime.Reset();
	TimeScale = 1.;
	
	Lock.Unlock();
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
	int thread_count = ThreadCount;
	
	// Pre-sort by object type and collidability.
	for( std::map<uint32_t,GameObject*>::iterator obj_iter = GameObjects.begin(); obj_iter != GameObjects.end(); obj_iter ++ )
	{
		if( (thread_count > 0) && obj_iter->second->ComplexCollisionDetection() )
			complex.push_back( obj_iter->second );
		else if( obj_iter->second->IsMoving() )
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
	
	if( thread_count > 0 )
	{
		// Get threads going for complex collision detection.
		for( std::list<GameObject*>::iterator obj_iter = complex.begin(); obj_iter != complex.end(); obj_iter ++ )
		{
			std::vector<CollisionDataSet*> data_sets;
			
			for( int i = 0; i < thread_count; i ++ )
			{
				threads.push_back( CollisionDataSet(dt) );
				data_sets.push_back( &(threads.back()) );
				data_sets[ i ]->Objects1.push_back( *obj_iter );
			}
			
			uint32_t type = (*obj_iter)->Type();
			
			int index = 0;
			
			for( std::list<GameObject*>::iterator obj2_iter = moving_can_hit_own_type[ type ].begin(); obj2_iter != moving_can_hit_own_type[ type ].end(); obj2_iter ++ )
			{
				data_sets[ index % thread_count ]->Objects2.push_back( *obj2_iter );
				index ++;
			}
			for( std::map< uint32_t, std::list<GameObject*> >::iterator list_iter = moving_can_hit_other_types.begin(); list_iter != moving_can_hit_other_types.end(); list_iter ++ )
			{
				if( list_iter->first == type )
					continue;
				
				for( std::list<GameObject*>::iterator obj2_iter = list_iter->second.begin(); obj2_iter != list_iter->second.end(); obj2_iter ++ )
				{
					data_sets[ index % thread_count ]->Objects2.push_back( *obj2_iter );
					index ++;
				}
			}
			
			if( (*obj_iter)->IsMoving() )
			{
				for( std::list<GameObject*>::iterator obj2_iter = stationary_can_hit_own_type[ type ].begin(); obj2_iter != stationary_can_hit_own_type[ type ].end(); obj2_iter ++ )
				{
					data_sets[ index % thread_count ]->Objects2.push_back( *obj2_iter );
					index ++;
				}
				for( std::map< uint32_t, std::list<GameObject*> >::iterator list_iter = stationary_can_hit_other_types.begin(); list_iter != stationary_can_hit_other_types.end(); list_iter ++ )
				{
					if( list_iter->first == type )
						continue;
					
					for( std::list<GameObject*>::iterator obj2_iter = list_iter->second.begin(); obj2_iter != list_iter->second.end(); obj2_iter ++ )
					{
						data_sets[ index % thread_count ]->Objects2.push_back( *obj2_iter );
						index ++;
					}
				}
			}
			
			std::list<GameObject*>::iterator obj2_iter = obj_iter;
			obj2_iter ++;
			for( ; obj2_iter != complex.end(); obj2_iter ++ )
			{
				data_sets[ index % thread_count ]->Objects2.push_back( *obj2_iter );
				index ++;
			}
			
			for( int i = 0; i < thread_count; i ++ )
				#if SDL_VERSION_ATLEAST(2,0,0)
					data_sets[ i ]->Thread = SDL_CreateThread( &FindCollisionsThread, "GameDataFindCollisions", data_sets[ i ] );
				#else
					data_sets[ i ]->Thread = SDL_CreateThread( &FindCollisionsThread, data_sets[ i ] );
				#endif
		}
	}
	
	std::string a_object, b_object;
	double when = 0.;
	Pos3D loc;
	
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
				if( (*obj1_iter)->WillCollide( *obj2_iter, dt, &a_object, &b_object, &loc, &when ) )
				{
					Collisions.push_back( Collision( *obj1_iter, *obj2_iter, &a_object, &b_object, &loc, &when ) );
					a_object.clear();
					b_object.clear();
					when = 0.;
					loc.SetPos(0,0,0);
				}
			}
			
			// Check for collisions with stationary objects of the same type.
			for( obj2_iter = stationary_can_hit_own_type[ list_iter->first ].begin(); obj2_iter != stationary_can_hit_own_type[ list_iter->first ].end(); obj2_iter ++ )
			{
				if( (*obj1_iter)->WillCollide( *obj2_iter, dt, &a_object, &b_object, &loc, &when ) )
				{
					Collisions.push_back( Collision( *obj1_iter, *obj2_iter, &a_object, &b_object, &loc, &when ) );
					a_object.clear();
					b_object.clear();
					when = 0.;
					loc.SetPos(0,0,0);
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
					if( (*obj1_iter)->WillCollide( *obj2_iter, dt, &a_object, &b_object, &loc, &when ) )
					{
						Collisions.push_back( Collision( *obj1_iter, *obj2_iter, &a_object, &b_object, &loc, &when ) );
						a_object.clear();
						b_object.clear();
						when = 0.;
						loc.SetPos(0,0,0);
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
					if( (*obj1_iter)->WillCollide( *obj2_iter, dt, &a_object, &b_object, &loc, &when ) )
					{
						Collisions.push_back( Collision( *obj1_iter, *obj2_iter, &a_object, &b_object, &loc, &when ) );
						a_object.clear();
						b_object.clear();
						when = 0.;
						loc.SetPos(0,0,0);
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
	MaxFrameTime = 0.5 * TimeScale;
	
	ThreadCount = Raptor::Game->Cfg.SettingAsInt("sv_threads");
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
	Player *player = NULL;
	
	Lock.Lock();
	
	std::map<uint16_t,Player*>::iterator player_iter = Players.find( id );
	if( player_iter != Players.end() )
		player = player_iter->second;
	
	Lock.Unlock();
	
	return player;
}


int GameData::RealPlayers( void )
{
	int count = 0;
	
	Lock.Lock();
	
	for( std::map<uint16_t,Player*>::const_iterator player_iter = Players.begin(); player_iter != Players.end(); player_iter ++ )
	{
		if( ! Str::ContainsInsensitive( player_iter->second->Name, "Screensaver" ) )
			count ++;
	}
	
	Lock.Unlock();
	
	return count;
}


// -----------------------------------------------------------------------------


void GameData::SetProperty( std::string name, std::string value )
{
	Lock.Lock();
	Properties[ name ] = value;
	Lock.Unlock();
}


bool GameData::HasProperty( std::string name )
{
	bool has_property = false;
	
	Lock.Lock();
	
	std::map<std::string, std::string>::const_iterator found = Properties.find( name );
	has_property = (found != Properties.end());
	
	Lock.Unlock();
	
	return has_property;
}


std::string GameData::PropertyAsString( std::string name, const char *ifndef, const char *ifempty )
{
	std::string value;
	
	Lock.Lock();
	
	std::map<std::string, std::string>::const_iterator found = Properties.find( name );
	if( found != Properties.end() )
	{
		if( found->second.empty() && ifempty )
			value = std::string(ifempty);
		else
			value = found->second;
	}
	else if( ifndef )
		value = std::string(ifndef);
	
	Lock.Unlock();
	
	return value;
}


double GameData::PropertyAsDouble( std::string name, double ifndef, double ifempty )
{
	double value = ifndef;
	
	Lock.Lock();
	
	std::map<std::string, std::string>::const_iterator found = Properties.find( name );
	if( found != Properties.end() )
	{
		if( found->second.empty() )
			value = ifempty;
		else
			value = Str::AsDouble( found->second );
	}
	
	Lock.Unlock();
	
	return value;
}


int GameData::PropertyAsInt( std::string name, int ifndef, int ifempty )
{
	int value = ifndef;
	
	Lock.Lock();
	
	std::map<std::string, std::string>::const_iterator found = Properties.find( name );
	if( found != Properties.end() )
	{
		if( found->second.empty() )
			value = ifempty;
		else
			value = Str::AsInt( found->second );
	}
	
	Lock.Unlock();
	
	return value;
}


bool GameData::PropertyAsBool( std::string name, bool ifndef, bool ifempty )
{
	bool value = ifndef;
	
	Lock.Lock();
	
	std::map<std::string, std::string>::const_iterator found = Properties.find( name );
	if( found != Properties.end() )
	{
		if( found->second.empty() )
			value = ifempty;
		else
			value = Str::AsBool( found->second );
	}
	
	Lock.Unlock();
	
	return value;
}


std::vector<double> GameData::PropertyAsDoubles( std::string name )
{
	std::vector<double> results;
	
	Lock.Lock();
	
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
	
	Lock.Unlock();
	
	return results;
}


std::vector<int> GameData::PropertyAsInts( std::string name )
{
	std::vector<int> results;
	
	Lock.Lock();
	
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
	
	Lock.Unlock();
	
	return results;
}


// -----------------------------------------------------------------------------


Collision::Collision( GameObject *a, GameObject *b, std::string *a_object, std::string *b_object, Pos3D *loc, double *when )
{
	// These names mimic std::pair<GameObject*,GameObject*> that was used before.
	first = a;
	second = b;
	
	if( a_object )
		FirstObject = *a_object;
	if( b_object )
		SecondObject = *b_object;
	if( loc )
		Location.Copy( loc );
	if( when )
		Time = *when;
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
	double when = FLT_MAX;
	Pos3D loc;
	
	for( std::list<GameObject*>::const_iterator obj1_iter = Objects1.begin(); obj1_iter != Objects1.end(); obj1_iter ++ )
	{
		for( std::list<GameObject*>::const_iterator obj2_iter = Objects2.begin(); obj2_iter != Objects2.end(); obj2_iter ++ )
		{
			if( (*obj1_iter)->WillCollide( *obj2_iter, dT, &a_object, &b_object, &loc, &when ) )
			{
				Collisions.push_back( Collision( *obj1_iter, *obj2_iter, &a_object, &b_object, &loc, &when ) );
				a_object.clear();
				b_object.clear();
				loc.SetPos(0,0,0);
				when = FLT_MAX;
			}
		}
	}
}


// -----------------------------------------------------------------------------


int FindCollisionsThread( void *data_set_ptr )
{
	#ifdef WIN32
		SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL );
	#endif
	
	CollisionDataSet *data_set = (CollisionDataSet*) data_set_ptr;
	data_set->DetectCollisions();
	
	return 0;
}
