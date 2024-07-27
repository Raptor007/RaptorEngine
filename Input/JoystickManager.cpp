/*
 *  JoystickManager.cpp
 */

#include "JoystickManager.h"

#include <cstddef>


JoystickManager::JoystickManager( void )
{
	Initialized = false;
}


JoystickManager::~JoystickManager()
{
	if( Initialized )
	{
		ReleaseJoysticks();
		SDL_JoystickEventState( SDL_DISABLE );
		SDL_QuitSubSystem( SDL_INIT_JOYSTICK );
	}
	
	Initialized = false;
}


void JoystickManager::Initialize( void )
{
	if( ! Initialized )
		SDL_InitSubSystem( SDL_INIT_JOYSTICK );
	
	SDL_JoystickEventState( SDL_ENABLE );
	
	Initialized = true;
}


void JoystickManager::Refresh( void )
{
	#if SDL_VERSION_ATLEAST(2,0,0)
		if( Initialized )
			ReleaseJoysticks();
		else
			Initialize();
	#else
		if( Initialized )
		{
			ReleaseJoysticks();
			SDL_JoystickEventState( SDL_DISABLE );
			SDL_QuitSubSystem( SDL_INIT_JOYSTICK );
			Initialized = false;
		}
		Initialize();
	#endif
	FindJoysticks();
}


void JoystickManager::FindJoysticks( void )
{
	if( ! Initialized )
		return;
	
	for( int i = 0; i < SDL_NumJoysticks(); i ++ )
	{
		SDL_Joystick *joystick = SDL_JoystickOpen( i );
		if( joystick )
		{
			#if SDL_VERSION_ATLEAST(2,0,0)
				Sint32 id = SDL_JoystickInstanceID( joystick );
			#else
				Sint32 id = i;
			#endif
			Joysticks[ id ].ID = id;
			
			Joysticks[ id ].Joystick = joystick;
			
			#if SDL_VERSION_ATLEAST(2,0,0)
				const char *joystick_name = SDL_JoystickName( joystick );
			#else
				const char *joystick_name = SDL_JoystickName( i );
			#endif
			Joysticks[ id ].Name = joystick_name ? joystick_name : "Joystick";
		}
	}
}


bool JoystickManager::DeviceTypeFound( std::string dev ) const
{
	for( std::map<Sint32, JoystickState>::const_iterator joy_iter = Joysticks.begin(); joy_iter != Joysticks.end(); joy_iter ++ )
	{
		if( joy_iter->second.DeviceType() == dev )
			return true;
	}
	
	return false;
}


void JoystickManager::ReleaseJoysticks( void )
{
	if( ! Initialized )
		return;
	
	for( std::map<Sint32, JoystickState>::iterator joy_iter = Joysticks.begin(); joy_iter != Joysticks.end(); joy_iter ++ )
	{
		if( joy_iter->second.Joystick )
			SDL_JoystickClose( joy_iter->second.Joystick );
	}
	
	Joysticks.clear();
}


void JoystickManager::TrackEvent( SDL_Event *event )
{
	Sint32 id = -1;
	
	if( event->type == SDL_JOYAXISMOTION )
		id = event->jaxis.which;
	else if( (event->type == SDL_JOYBUTTONDOWN) || (event->type == SDL_JOYBUTTONUP) )
		id = event->jbutton.which;
	else if( event->type == SDL_JOYHATMOTION )
		id = event->jhat.which;
	else if( event->type == SDL_JOYBALLMOTION )
		id = event->jball.which;
	else
		return;
	
	Joysticks[ id ].ID = id;  // In case somehow we get an event for a joystick we didn't initialize, I guess.
	Joysticks[ id ].TrackEvent( event );
}


bool JoystickManager::HasAxis( Sint32 joystick_id, Uint8 axis ) const
{
	// Return true if this joystick has ever seen motion on this axis.
	
	std::map<Sint32, JoystickState>::const_iterator joy_iter = Joysticks.find( joystick_id );
	if( joy_iter != Joysticks.end() )
		return joy_iter->second.HasAxis( axis );
	
	return 0.;
}


double JoystickManager::Axis( Sint32 joystick_id, Uint8 axis, double deadzone_min, double deadzone_max, double range_min, double range_max ) const
{
	// Return the value of a joystick's axis.
	// Note that these values are scaled to the (-1,1) range when tracked.
	
	std::map<Sint32, JoystickState>::const_iterator joy_iter = Joysticks.find( joystick_id );
	if( joy_iter != Joysticks.end() )
		return joy_iter->second.Axis( axis, deadzone_min, deadzone_max, range_min, range_max );
	
	return 0.;
}


double JoystickManager::AxisScaled( Sint32 joystick_id, Uint8 axis, double low, double high, double deadzone_min, double deadzone_max, double range_min, double range_max ) const
{
	// Return the value of a joystick's axis, scaled to (low,high).
	
	std::map<Sint32, JoystickState>::const_iterator joy_iter = Joysticks.find( joystick_id );
	if( joy_iter != Joysticks.end() )
		return joy_iter->second.AxisScaled( axis, low, high, deadzone_min, deadzone_max, range_min, range_max );

	return low + (high - low) / 2.;
}


bool JoystickManager::ButtonDown( Sint32 joystick_id, Uint8 button ) const
{
	std::map<Sint32, JoystickState>::const_iterator joy_iter = Joysticks.find( joystick_id );
	if( joy_iter != Joysticks.end() )
		return joy_iter->second.ButtonDown( button );

	return false;
}


Uint8 JoystickManager::Hat( Sint32 joystick_id, Uint8 hat ) const
{
	std::map<Sint32, JoystickState>::const_iterator joy_iter = Joysticks.find( joystick_id );
	if( joy_iter != Joysticks.end() )
		return joy_iter->second.Hat( hat );

	return 0;
}


bool JoystickManager::HatDir( Sint32 joystick_id, Uint8 hat, Uint8 dir ) const
{
	// See if the hat switch is in this cardinal direction.
	// This matches straight directions even if the hat is being pushed diagonally.

	std::map<Sint32, JoystickState>::const_iterator joy_iter = Joysticks.find( joystick_id );
	if( joy_iter != Joysticks.end() )
		return joy_iter->second.HatDir( hat, dir );

	return false;
}


std::string JoystickManager::Name( Sint32 joystick_id ) const
{
	std::map<Sint32, JoystickState>::const_iterator joy_iter = Joysticks.find( joystick_id );
	if( joy_iter != Joysticks.end() )
		return joy_iter->second.Name;
	return "";
}


std::string JoystickManager::Status( void ) const
{
	// Create a status string for all joysticks.
	
	std::string return_string;
	char cstr[ 1024 ] = "";
	
	snprintf( cstr, 1024, "Joysticks: %i", (int) Joysticks.size() );
	return_string += cstr;
	
	for( std::map<Sint32, JoystickState>::const_iterator joy_iter = Joysticks.begin(); joy_iter != Joysticks.end(); joy_iter ++ )
		return_string += "\n" + joy_iter->second.Status();

	return return_string;
}
