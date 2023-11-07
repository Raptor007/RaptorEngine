/*
 *  InputManager.h
 */

#pragma once
class InputManager;

#include "PlatformSpecific.h"

#include <string>
#include <map>
#include <set>
#include <stdint.h>
#include "JoystickState.h"


class InputManager
{
public:
	std::map<SDLKey,std::string> KeyNames;
	std::map<Uint8,std::string> MouseNames;
	std::set<std::string> DeviceTypes;
	
	std::map<uint8_t,std::string> ControlNames;
	std::map<uint8_t,double> ControlValues;
	std::map<uint8_t,std::string> PreferredDevices;
	std::map<uint8_t,std::pair<double,double> > ControlRanges;
	
	
	InputManager( void );
	virtual ~InputManager();
	
	void ResetDeviceTypes( void );
	
	std::string KeyName( SDLKey key ) const;
	SDLKey KeyID( std::string name ) const;
	
	std::string MouseName( Uint8 button ) const;
	Uint8 MouseID( std::string name ) const;
	
	std::string JoyAxisName( std::string dev, Uint8 axis ) const;
	std::string JoyButtonName( std::string dev, Uint8 button ) const;
	std::string JoyHatName( std::string dev, Uint8 hat, Uint8 dir ) const;
	bool ParseJoyAxis( std::string input, std::string *dev_ptr = NULL, Uint8 *axis_ptr = NULL ) const;
	bool ParseJoyButton( std::string input, std::string *dev_ptr = NULL, Uint8 *button_ptr = NULL ) const;
	bool ParseJoyHat( std::string input, std::string *dev_ptr = NULL, Uint8 *hat_ptr = NULL, Uint8 *dir_ptr = NULL ) const;
	
	bool ValidInput( std::string input ) const;
	
	uint8_t AddControl( std::string name, std::string preferred_device = "", double low = 0., double high = 1. );
	std::string ControlName( uint8_t control ) const;
	std::string GenericControlName( uint8_t control ) const;
	uint8_t ControlID( std::string name ) const;
	
	bool IsPreferredDevice( std::string joy_name, uint8_t control ) const;
	std::string DeviceType( std::string joy_name ) const;
	
	void Update( void );
	
	bool HasControlAxis( uint8_t control ) const;
	double ControlTotal( uint8_t control ) const;
	double ControlTotal( std::string name ) const;
	double ControlValue( uint8_t control ) const;
	double ControlValue( std::string name ) const;
	int8_t ControlPressed( uint8_t control, double threshold = 0.5 ) const;
	int8_t ControlPressed( std::string name, double threshold = 0.5 ) const;
	uint8_t EventBound( const SDL_Event *event ) const;
};
