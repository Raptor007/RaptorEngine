/*
 *  JoystickManager.h
 */

#pragma once
class JoystickManager;

#include "PlatformSpecific.h"

#include <string>
#include <map>
#include <SDL/SDL.h>
#include "JoystickState.h"


class JoystickManager
{
public:
	std::map<Uint8, JoystickState> Joysticks;
	bool Initialized;
	
	JoystickManager( void );
	virtual ~JoystickManager();
	
	void Initialize( void );
	void Refresh( void );
	void FindJoysticks( void );
	void ReleaseJoysticks( void );
	bool DeviceTypeFound( std::string dev ) const;
	
	void TrackEvent( SDL_Event *event );
	bool HasAxis( int joystick_id, Uint8 axis ) const;
	double Axis( int joystick_id, Uint8 axis, double deadzone = 0., double deadzone_at_ends = 0. ) const;
	double AxisScaled( int joystick_id, Uint8 axis, double low, double high, double deadzone = 0., double deadzone_at_ends = 0. ) const;
	bool ButtonDown( int joystick_id, Uint8 button ) const;
	Uint8 Hat( int joystick_id, Uint8 hat ) const;
	bool HatDir( int joystick_id, Uint8 hat, Uint8 dir ) const;
	std::string Name( int joystick_id ) const;
	
	std::string Status( void ) const;
};
