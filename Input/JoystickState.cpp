/*
 *  JoystickState.cpp
 */

#include "JoystickState.h"

#include <cmath>


JoystickState::JoystickState( Uint8 id, SDL_Joystick *joystick, std::string name )
{
	ID = id;
	Joystick = joystick;
	Name = name;
}


JoystickState::~JoystickState()
{
}


void JoystickState::TrackEvent( SDL_Event *event )
{
	// Update joystick axis and button status.
	
	if( event->type == SDL_JOYAXISMOTION )
		Axes[ event->jaxis.axis ] = AxisScale( event->jaxis.value );
	else if( event->type == SDL_JOYBUTTONDOWN )
		ButtonsDown[ event->jbutton.button ] = true;
	else if( event->type == SDL_JOYBUTTONUP )
		ButtonsDown[ event->jbutton.button ] = false;
	else if( event->type == SDL_JOYHATMOTION )
		Hats[ event->jhat.hat ] = event->jhat.value;
	else if( event->type == SDL_JOYBALLMOTION )
	{
		BallsX[ event->jball.ball ] += event->jball.xrel;
		BallsY[ event->jball.ball ] += event->jball.yrel;
	}
}


double JoystickState::AxisScale( Sint16 value )
{
	// Convert axis values from (-32768,32767) to (-1,1) range.
	
	if( value >= 0 )
		return ((double)value) / 32767.;
	else
		return ((double)value) / 32768.;
}


double JoystickState::Axis( Uint8 axis, double deadzone, double deadzone_at_ends )
{
	// Check an axis value.
	// Assume it is 0 if its state has never been recorded.
	
	std::map<Uint8, double>::iterator axis_iter = Axes.find( axis );
	if( axis_iter != Axes.end() )
	{
		double value = axis_iter->second;
		
		if( fabs(value) < deadzone )
			value = 0.;
		else if( value + deadzone_at_ends > 1. )
			value = 1.;
		else if( value - deadzone_at_ends < -1. )
			value = -1.;
		else
		{
			// Reclaim the range of values lost to the deadzones.
			if( value > 0. )
				value -= deadzone;
			else
				value += deadzone;
			value /= (1. - deadzone - deadzone_at_ends);
		}
		
		return value;
	}
	
	return 0.;
}


double JoystickState::AxisScaled( Uint8 axis, double low, double high, double deadzone, double deadzone_at_ends )
{
	// Scale output to (low,high) range.
	// This can also be used to invert an axis with low>high.

	return low + (high - low) * (Axis( axis, deadzone, deadzone_at_ends ) + 1.) / 2.;
}


bool JoystickState::ButtonDown( Uint8 button )
{
	// Check if a key is down.
	// Assume it is not down if its state has never been recorded.
	
	std::map<Uint8, bool>::iterator button_iter = ButtonsDown.find( button );
	if( button_iter != ButtonsDown.end() )
		return button_iter->second;
	
	return false;
}


Uint8 JoystickState::Hat( Uint8 hat )
{
	// Check the direction of a hat switch.
	
	std::map<Uint8, Uint8>::iterator hat_iter = Hats.find( hat );
	if( hat_iter != Hats.end() )
		return hat_iter->second;

	return 0;
}


bool JoystickState::HatDir( Uint8 hat, Uint8 dir )
{
	// See if the hat switch is in this cardinal direction.
	// This matches straight directions even if the hat is being pushed diagonally.

	return Hat( hat ) & dir;
}


std::string JoystickState::Status( void )
{
	// Create a status string for this joystick.
	
	std::string return_string;
	char cstr[ 1024 ] = "";

	snprintf( cstr, 1024, "Joystick %i: %s\n", ID, Name.c_str() );
	return_string += cstr;

	return_string += "Joy axes:";
	bool first_axis = true;
	for( std::map<Uint8, double>::iterator axis_iter = Axes.begin(); axis_iter != Axes.end(); axis_iter ++ )
	{
		if( first_axis )
			first_axis = false;
		else
			return_string += ",";

		snprintf( cstr, 1024, " %i=%.4f", axis_iter->first, axis_iter->second );
		return_string += cstr;
	}
	return_string += "\n";

	return_string += "Joy buttons down:";
	for( std::map<Uint8, bool>::iterator button_iter = ButtonsDown.begin(); button_iter != ButtonsDown.end(); button_iter ++ )
	{
		if( button_iter->second )
		{
			snprintf( cstr, 1024, " %i", button_iter->first );
			return_string += cstr;
		}
	}
	return_string += "\n";

	return_string += "Joy hats:";
	bool first_hat = true;
	for( std::map<Uint8, Uint8>::iterator hat_iter = Hats.begin(); hat_iter != Hats.end(); hat_iter ++ )
	{
		if( first_hat )
			first_hat = false;
		else
			return_string += ",";

		snprintf( cstr, 1024, " %i=%i%s%s%s%s%s%s", hat_iter->first, hat_iter->second,
		          hat_iter->second ? "(" : "",
		          hat_iter->second & SDL_HAT_UP ? "U" : "",
		          hat_iter->second & SDL_HAT_DOWN ? "D" : "",
		          hat_iter->second & SDL_HAT_LEFT ? "L" : "",
		          hat_iter->second & SDL_HAT_RIGHT ? "R" : "",
		          hat_iter->second ? ")" : "" );
		return_string += cstr;
	}
	
	return return_string;
}
