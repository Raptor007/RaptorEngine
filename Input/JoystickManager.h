/*
 *  JoystickManager.h
 */

#pragma once
class JoystickManager;

#include "PlatformSpecific.h"

#include <string>
#include <map>

#ifdef SDL2
	#include <SDL2/SDL.h>
#else
	#include <SDL/SDL.h>
#endif

#include "JoystickState.h"


class JoystickManager
{
public:
	std::map<Sint32,JoystickState> Joysticks;
	bool Initialized;
	
	JoystickManager( void );
	virtual ~JoystickManager();
	
	void Initialize( void );
	void Refresh( void );
	void FindJoysticks( void );
	void ReleaseJoysticks( void );
	bool DeviceTypeFound( std::string dev ) const;
	
	void TrackEvent( SDL_Event *event );
	bool HasAxis( Sint32 joystick_id, Uint8 axis ) const;
	double Axis( Sint32 joystick_id, Uint8 axis, double deadzone = 0., double deadzone_at_ends = 0. ) const;
	double AxisScaled( Sint32 joystick_id, Uint8 axis, double low, double high, double deadzone = 0., double deadzone_at_ends = 0. ) const;
	bool ButtonDown( Sint32 joystick_id, Uint8 button ) const;
	Uint8 Hat( Sint32 joystick_id, Uint8 hat ) const;
	bool HatDir( Sint32 joystick_id, Uint8 hat, Uint8 dir ) const;
	std::string Name( Sint32 joystick_id ) const;
	
	std::string Status( void ) const;
};
