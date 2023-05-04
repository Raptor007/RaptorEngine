/*
 *  JoystickState.h
 */

#pragma once
class JoystickState;

#include "PlatformSpecific.h"
#include <cstddef>
#include <string>
#include <map>
#include <SDL/SDL.h>


class JoystickState
{
public:
	uint8_t ID;
	SDL_Joystick *Joystick;
	std::string Name;
	std::map<Uint8, double> Axes, PrevAxes;
	std::map<Uint8, bool> ButtonsDown;
	std::map<Uint8, Uint8> Hats;
	std::map<Uint8, int> BallsX, BallsY, PrevBallsX, PrevBallsY;
	
	JoystickState( uint8_t id = 255, SDL_Joystick *joystick = NULL, std::string name = "Joystick" );
	virtual ~JoystickState();
	
	void TrackEvent( SDL_Event *event );
	static double AxisScale( Sint16 value );
	
	bool HasAxis( Uint8 axis ) const;
	double Axis( Uint8 axis, double deadzone = 0., double deadedge = 0. ) const;
	double AxisScaled( Uint8 axis, double low, double high, double deadzone = 0., double deadedge = 0. ) const;
	bool ButtonDown( Uint8 button ) const;
	Uint8 Hat( Uint8 hat ) const;
	bool HatDir( Uint8 hat, Uint8 dir ) const;
	
	std::string DeviceType( void ) const;
	
	std::string Status( void ) const;
};
