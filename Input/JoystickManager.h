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
	~JoystickManager();
	
	void Initialize( void );
	void FindJoysticks( void );
	void ReleaseJoysticks( void );
	
	void TrackEvent( SDL_Event *event );
	double Axis( int joystick_id, Uint8 axis, double deadzone = 0., double deadzone_at_ends = 0. );
	double AxisScaled( int joystick_id, Uint8 axis, double low, double high, double deadzone = 0., double deadzone_at_ends = 0. );
	bool ButtonDown( int joystick_id, Uint8 button );
	Uint8 Hat( int joystick_id, Uint8 hat );
	bool HatDir( int joystick_id, Uint8 hat, Uint8 dir );

	std::string Status( void );
};
