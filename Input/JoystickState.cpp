/*
 *  JoystickState.cpp
 */

#include "JoystickState.h"

#include <cmath>
#include "RaptorGame.h"


JoystickState::JoystickState( Sint32 id, SDL_Joystick *joystick, std::string name )
{
	ID = id;
	Joystick = joystick;
	Name = name;
}


JoystickState::~JoystickState()
{
}


void JoystickState::Clear( void )
{
	Axes.clear();
	PrevAxes.clear();
	ButtonsDown.clear();
	Hats.clear();
	BallsX.clear();
	BallsY.clear();
	PrevBallsX.clear();
	PrevBallsY.clear();
}


void JoystickState::TrackEvent( SDL_Event *event )
{
	// Update joystick axis and button status.
	
	if( event->type == SDL_JOYAXISMOTION )
	{
		PrevAxes[ event->jaxis.axis ] = Axes[ event->jaxis.axis ];
		Axes[ event->jaxis.axis ] = AxisScale( event->jaxis.value );
	}
	else if( event->type == SDL_JOYBUTTONDOWN )
		ButtonsDown[ event->jbutton.button ] = true;
	else if( event->type == SDL_JOYBUTTONUP )
		ButtonsDown[ event->jbutton.button ] = false;
	else if( event->type == SDL_JOYHATMOTION )
		Hats[ event->jhat.hat ] = event->jhat.value;
	else if( event->type == SDL_JOYBALLMOTION )
	{
		PrevBallsX[ event->jball.ball ] = BallsX[ event->jball.ball ];
		PrevBallsY[ event->jball.ball ] = BallsY[ event->jball.ball ];
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


bool JoystickState::HasAxis( Uint8 axis ) const
{
	return (Axes.find( axis ) != Axes.end());
}


double JoystickState::Axis( Uint8 axis, double deadzone, double deadedge ) const
{
	// Check an axis value.
	// Assume it is 0 if its state has never been recorded.
	// All other Axis functions eventually call this one.
	
	std::map<Uint8,double>::const_iterator axis_iter = Axes.find( axis );
	if( axis_iter != Axes.end() )
	{
		double value = axis_iter->second;
		
		if( fabs(value) < deadzone )
			value = 0.;
		else if( value + deadedge > 1. )
			value = 1.;
		else if( value - deadedge < -1. )
			value = -1.;
		else
		{
			// Reclaim the range of values lost to the deadzones.
			if( value > 0. )
				value -= deadzone;
			else
				value += deadzone;
			value /= (1. - deadzone - deadedge);
		}
		
		return value;
	}
	
	return 0.;
}


double JoystickState::AxisScaled( Uint8 axis, double low, double high, double deadzone, double deadedge ) const
{
	// Scale output to (low,high) range.
	// This can also be used to invert an axis with low>high.
	
	if( ! HasAxis(axis) )
		return 0.;
	
	return low + (high - low) * (Axis( axis, deadzone, deadedge ) + 1.) / 2.;
}


bool JoystickState::ButtonDown( Uint8 button ) const
{
	// Check if a key is down.
	// Assume it is not down if its state has never been recorded.
	
	std::map<Uint8,bool>::const_iterator button_iter = ButtonsDown.find( button );
	if( button_iter != ButtonsDown.end() )
		return button_iter->second;
	
	return false;
}


Uint8 JoystickState::Hat( Uint8 hat ) const
{
	// Check the direction of a hat switch.
	
	std::map<Uint8,Uint8>::const_iterator hat_iter = Hats.find( hat );
	if( hat_iter != Hats.end() )
		return hat_iter->second;
	
	return 0;
}


bool JoystickState::HatDir( Uint8 hat, Uint8 dir ) const
{
	// See if the hat switch is in this cardinal direction.
	// This matches straight directions even if the hat is being pushed diagonally.
	
	return dir ? (Hat( hat ) & dir) : ! Hat( hat );
}


std::string JoystickState::DeviceType( void ) const
{
	return Raptor::Game->Input.DeviceType( Name );
}


std::string JoystickState::Status( void ) const
{
	// Create a status string for this joystick.
	
	std::string return_string;
	char cstr[ 1024 ] = "";
	
	snprintf( cstr, 1024, "Joystick %i: %s (%s)\n", ID, Name.c_str(), DeviceType().c_str() );
	return_string += cstr;
	
	return_string += "Joy axes:";
	bool first_axis = true;
	for( std::map<Uint8,double>::const_iterator axis_iter = Axes.begin(); axis_iter != Axes.end(); axis_iter ++ )
	{
		if( first_axis )
			first_axis = false;
		else
			return_string += ",";
		
		snprintf( cstr, 1024, " %i=%.4f", axis_iter->first + 1, axis_iter->second );
		return_string += cstr;
	}
	return_string += "\n";
	
	return_string += "Joy buttons down:";
	for( std::map<Uint8,bool>::const_iterator button_iter = ButtonsDown.begin(); button_iter != ButtonsDown.end(); button_iter ++ )
	{
		if( button_iter->second )
		{
			snprintf( cstr, 1024, " %i", button_iter->first + 1 );
			return_string += cstr;
		}
	}
	return_string += "\n";
	
	return_string += "Joy hats:";
	bool first_hat = true;
	for( std::map<Uint8,Uint8>::const_iterator hat_iter = Hats.begin(); hat_iter != Hats.end(); hat_iter ++ )
	{
		if( first_hat )
			first_hat = false;
		else
			return_string += ",";
		
		snprintf( cstr, 1024, " %i=%i%s%s%s%s%s%s", hat_iter->first + 1, hat_iter->second,
		          hat_iter->second ? "(" : "",
		          hat_iter->second & SDL_HAT_UP ? "U" : "",
		          hat_iter->second & SDL_HAT_DOWN ? "D" : "",
		          hat_iter->second & SDL_HAT_LEFT ? "L" : "",
		          hat_iter->second & SDL_HAT_RIGHT ? "R" : "",
		          hat_iter->second ? ")" : "" );
		return_string += cstr;
	}
	
	if( BallsX.size() )
	{
		return_string += "\nJoy trackballs:";
		for( std::map<Uint8,int>::const_iterator ball_x_iter = BallsX.begin(); ball_x_iter != BallsX.end(); ball_x_iter ++ )
		{
			if( ball_x_iter != BallsX.begin() )
				return_string += ",";
			
			std::map<Uint8,int>::const_iterator ball_y_iter = BallsY.find( ball_x_iter->first );
			if( ball_y_iter != BallsY.end() )
				snprintf( cstr, 1024, " %i=%i,%i", ball_x_iter->first + 1, ball_x_iter->second, ball_y_iter->second );
			else
				snprintf( cstr, 1024, " %i=%i,?", ball_x_iter->first + 1, ball_x_iter->second );
			return_string += cstr;
		}
	}
	
	return return_string;
}
