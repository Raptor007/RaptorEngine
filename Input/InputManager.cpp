/*
 *  InputManager.cpp
 */

#include "InputManager.h"

#include <cmath>
#include <climits>
#include "RaptorGame.h"
#include "Num.h"


InputManager::InputManager( void )
{
	KeyNames[ SDLK_BACKSPACE ] = "Backspace";
	KeyNames[ SDLK_TAB ] = "Tab";
	KeyNames[ SDLK_CLEAR ] = "Clear";
	KeyNames[ SDLK_RETURN ] = "Return";
	KeyNames[ SDLK_PAUSE ] = "Pause";
	KeyNames[ SDLK_ESCAPE ] = "Esc";
	KeyNames[ SDLK_SPACE ] = "Space";
	KeyNames[ SDLK_EXCLAIM ] = "!";
	KeyNames[ SDLK_QUOTEDBL ] = "''";
	KeyNames[ SDLK_HASH ] = "#";
	KeyNames[ SDLK_DOLLAR ] = "$";
	KeyNames[ SDLK_AMPERSAND ] = "&";
	KeyNames[ SDLK_QUOTE ] = "'";
	KeyNames[ SDLK_LEFTPAREN ] = "(";
	KeyNames[ SDLK_RIGHTPAREN ] = ")";
	KeyNames[ SDLK_ASTERISK ] = "*";
	KeyNames[ SDLK_PLUS ] = "+";
	KeyNames[ SDLK_COMMA ] = ",";
	KeyNames[ SDLK_MINUS ] = "-";
	KeyNames[ SDLK_PERIOD ] = ".";
	KeyNames[ SDLK_SLASH ] = "/";
	KeyNames[ SDLK_0 ] = "0";
	KeyNames[ SDLK_1 ] = "1";
	KeyNames[ SDLK_2 ] = "2";
	KeyNames[ SDLK_3 ] = "3";
	KeyNames[ SDLK_4 ] = "4";
	KeyNames[ SDLK_5 ] = "5";
	KeyNames[ SDLK_6 ] = "6";
	KeyNames[ SDLK_7 ] = "7";
	KeyNames[ SDLK_8 ] = "8";
	KeyNames[ SDLK_9 ] = "9";
	KeyNames[ SDLK_COLON ] = ":";
	KeyNames[ SDLK_SEMICOLON ] = ";";
	KeyNames[ SDLK_LESS ] = "<";
	KeyNames[ SDLK_EQUALS ] = "=";
	KeyNames[ SDLK_GREATER ] = ">";
	KeyNames[ SDLK_QUESTION ] = "?";
	KeyNames[ SDLK_AT ] = "@";
	KeyNames[ SDLK_LEFTBRACKET ] = "[";
	KeyNames[ SDLK_BACKSLASH ] = "\\";
	KeyNames[ SDLK_RIGHTBRACKET ] = "]";
	KeyNames[ SDLK_CARET ] = "^";
	KeyNames[ SDLK_UNDERSCORE ] = "_";
	KeyNames[ SDLK_BACKQUOTE ] = "`";
	KeyNames[ SDLK_a ] = "A";
	KeyNames[ SDLK_b ] = "B";
	KeyNames[ SDLK_c ] = "C";
	KeyNames[ SDLK_d ] = "D";
	KeyNames[ SDLK_e ] = "E";
	KeyNames[ SDLK_f ] = "F";
	KeyNames[ SDLK_g ] = "G";
	KeyNames[ SDLK_h ] = "H";
	KeyNames[ SDLK_i ] = "I";
	KeyNames[ SDLK_j ] = "J";
	KeyNames[ SDLK_k ] = "K";
	KeyNames[ SDLK_l ] = "L";
	KeyNames[ SDLK_m ] = "M";
	KeyNames[ SDLK_n ] = "N";
	KeyNames[ SDLK_o ] = "O";
	KeyNames[ SDLK_p ] = "P";
	KeyNames[ SDLK_q ] = "Q";
	KeyNames[ SDLK_r ] = "R";
	KeyNames[ SDLK_s ] = "S";
	KeyNames[ SDLK_t ] = "T";
	KeyNames[ SDLK_u ] = "U";
	KeyNames[ SDLK_v ] = "V";
	KeyNames[ SDLK_w ] = "W";
	KeyNames[ SDLK_x ] = "X";
	KeyNames[ SDLK_y ] = "Y";
	KeyNames[ SDLK_z ] = "Z";
	KeyNames[ SDLK_DELETE ] = "Delete";
	KeyNames[ SDLK_KP0 ] = "KP0";
	KeyNames[ SDLK_KP1 ] = "KP1";
	KeyNames[ SDLK_KP2 ] = "KP2";
	KeyNames[ SDLK_KP3 ] = "KP3";
	KeyNames[ SDLK_KP4 ] = "KP4";
	KeyNames[ SDLK_KP5 ] = "KP5";
	KeyNames[ SDLK_KP6 ] = "KP6";
	KeyNames[ SDLK_KP7 ] = "KP7";
	KeyNames[ SDLK_KP8 ] = "KP8";
	KeyNames[ SDLK_KP9 ] = "KP9";
	KeyNames[ SDLK_KP_PERIOD ] = "KP.";
	KeyNames[ SDLK_KP_DIVIDE ] = "KP/";
	KeyNames[ SDLK_KP_MULTIPLY ] = "KP*";
	KeyNames[ SDLK_KP_MINUS ] = "KP-";
	KeyNames[ SDLK_KP_PLUS ] = "KP+";
	KeyNames[ SDLK_KP_ENTER	] = "KPEnter";
	KeyNames[ SDLK_KP_EQUALS ] = "KP=";
	KeyNames[ SDLK_UP ] = "Up";
	KeyNames[ SDLK_DOWN ] = "Down";
	KeyNames[ SDLK_RIGHT ] = "Right";
	KeyNames[ SDLK_LEFT ] = "Left";
	KeyNames[ SDLK_INSERT ] = "Insert";
	KeyNames[ SDLK_HOME ] = "Home";
	KeyNames[ SDLK_END ] = "End";
	KeyNames[ SDLK_PAGEUP ] = "PageUp";
	KeyNames[ SDLK_PAGEDOWN ] = "PageDown";
	KeyNames[ SDLK_F1 ] = "F1";
	KeyNames[ SDLK_F2 ] = "F2";
	KeyNames[ SDLK_F3 ] = "F3";
	KeyNames[ SDLK_F4 ] = "F4";
	KeyNames[ SDLK_F5 ] = "F5";
	KeyNames[ SDLK_F6 ] = "F6";
	KeyNames[ SDLK_F7 ] = "F7";
	KeyNames[ SDLK_F8 ] = "F8";
	KeyNames[ SDLK_F9 ] = "F9";
	KeyNames[ SDLK_F10 ] = "F10";
	KeyNames[ SDLK_F11 ] = "F11";
	KeyNames[ SDLK_F12 ] = "F12";
	KeyNames[ SDLK_F13 ] = "F13";
	KeyNames[ SDLK_F14 ] = "F14";
	KeyNames[ SDLK_F15 ] = "F15";
	KeyNames[ SDLK_NUMLOCK ] = "NumLock";
	KeyNames[ SDLK_CAPSLOCK ] = "CapsLock";
	KeyNames[ SDLK_SCROLLOCK ] = "ScrollLock";
	KeyNames[ SDLK_RSHIFT ] = "RShift";
	KeyNames[ SDLK_LSHIFT ] = "LShift";
	KeyNames[ SDLK_RCTRL ] = "RCtrl";
	KeyNames[ SDLK_LCTRL ] = "LCtrl";
	KeyNames[ SDLK_RALT ] = "RAlt";
	KeyNames[ SDLK_LALT ] = "LAlt";
	KeyNames[ SDLK_RMETA ] = "RCmd";
	KeyNames[ SDLK_LMETA ] = "LCmd";
	KeyNames[ SDLK_LSUPER ] = "LWin";
	KeyNames[ SDLK_RSUPER ] = "RWin";
	KeyNames[ SDLK_MODE ] = "Mode";
	KeyNames[ SDLK_COMPOSE ] = "Compose";
	KeyNames[ SDLK_HELP ] = "Help";
	KeyNames[ SDLK_PRINT ] = "Print";
	KeyNames[ SDLK_SYSREQ ] = "SysReq";
	KeyNames[ SDLK_BREAK ] = "Break";
	KeyNames[ SDLK_MENU ] = "Menu";
	KeyNames[ SDLK_POWER ] = "Power";
	KeyNames[ SDLK_EURO ] = "Euro";
	KeyNames[ SDLK_UNDO ] = "Undo";
	
	MouseNames[ SDL_BUTTON_LEFT ] = "Mouse1";
	MouseNames[ SDL_BUTTON_RIGHT ] = "Mouse2";
	MouseNames[ SDL_BUTTON_MIDDLE ] = "Mouse3";
	MouseNames[ SDL_BUTTON_WHEELUP ] = "MWheelUp";
	MouseNames[ SDL_BUTTON_WHEELDOWN ] = "MWheelDown";
	MouseNames[ SDL_BUTTON_X1 ] = "Mouse4";
	MouseNames[ SDL_BUTTON_X2 ] = "Mouse5";
	
	ResetDeviceTypes();
	
	ControlNames[ 0 ] = "Unbound";
}


InputManager::~InputManager()
{
}


// ---------------------------------------------------------------------------


void InputManager::ResetDeviceTypes( void )
{
	DeviceTypes.clear();
	
	DeviceTypes.insert( "Joy" );
	DeviceTypes.insert( "Xbox" );
	DeviceTypes.insert( "Pedal" );
	DeviceTypes.insert( "Throttle" );
	DeviceTypes.insert( "Wheel" );
	
	#if SDL_VERSION_ATLEAST(2,0,0)
		// SDL2 does not normalize joystick axis mappings across devices; use separate binds for different layouts.
		DeviceTypes.insert( "X52" );
		DeviceTypes.insert( "SideWinder" );
	#else
		// SDL2 does not detect F16 MFD button panels as joysticks, but SDL1 does.
		DeviceTypes.insert( "MFD" );
	#endif
}


// ---------------------------------------------------------------------------


std::string InputManager::KeyName( SDLKey key ) const
{
	std::map<SDLKey, std::string>::const_iterator iter = KeyNames.find( key );
	if( iter != KeyNames.end() )
		return iter->second;
	
	char key_string[ 32 ] = "";
	snprintf( key_string, sizeof(key_string), "Key%i", key );
	return std::string( key_string );
}


SDLKey InputManager::KeyID( std::string name ) const
{
	for( std::map<SDLKey, std::string>::const_iterator iter = KeyNames.begin(); iter != KeyNames.end(); iter ++ )
	{
		// First see if this key has an assigned name.
		if( Str::EqualsInsensitive( iter->second, name ) )
			return iter->first;
	}
	
	if( Str::BeginsWith( name, "Key" ) )
		return (SDLKey) atoi( name.c_str() + 3 );
	
	return SDLK_UNKNOWN;
}


// ---------------------------------------------------------------------------


std::string InputManager::MouseName( Uint8 button ) const
{
	std::map<Uint8, std::string>::const_iterator iter = MouseNames.find( button );
	if( iter != MouseNames.end() )
		return iter->second;
	
	#if SDL_VERSION_ATLEAST(2,0,0)
		return std::string("Mouse") + Num::ToString(button);
	#else
		return std::string("Mouse") + Num::ToString( (button >= SDL_BUTTON_X1) ? (button - 2) : button );
	#endif
}


Uint8 InputManager::MouseID( std::string name ) const
{
	for( std::map<Uint8, std::string>::const_iterator iter = MouseNames.begin(); iter != MouseNames.end(); iter ++ )
	{
		// First see if this mouse button has an assigned name.
		if( Str::EqualsInsensitive( iter->second, name ) )
			return iter->first;
	}
	
	if( Str::BeginsWith( name, "Mouse" ) )
	{
		int button = atoi( name.c_str() + 5 );
		#if SDL_VERSION_ATLEAST(2,0,0)
			return button;
		#else
			return (button >= 4) ? (button + 2) : button;
		#endif
	}
	
	return 0;
}


// ---------------------------------------------------------------------------


std::string InputManager::JoyAxisName( std::string dev, Uint8 axis ) const
{
	return dev + std::string("Axis") + Num::ToString( axis + 1 );
}


std::string InputManager::JoyButtonName( std::string dev, Uint8 button ) const
{
	return dev + std::string("Button") + Num::ToString( button + 1 );
}


std::string InputManager::JoyHatName( std::string dev, Uint8 hat, Uint8 dir ) const
{
	std::string str = dev + std::string("Hat") + Num::ToString( hat + 1 );
	if( dir & SDL_HAT_UP )
		str += std::string("Up");
	if( dir & SDL_HAT_DOWN )
		str += std::string("Down");
	if( dir & SDL_HAT_LEFT )
		str += std::string("Left");
	if( dir & SDL_HAT_RIGHT )
		str += std::string("Right");
	if( dir == SDL_HAT_CENTERED )
		str += std::string("Center");
	return str;
}


bool InputManager::ParseJoyAxis( std::string input, std::string *dev_ptr, Uint8 *axis_ptr ) const
{
	int axis_index = Str::FindLastInsensitive( input, "Axis" );
	if( axis_index >= 1 )
	{
		std::string dev = input.substr( 0, axis_index );
		int axis = atoi( input.c_str() + axis_index + 4 ) - 1;
		if( axis >= 0 )
		{
			if( dev_ptr )
				*dev_ptr = dev;
			if( axis_ptr )
				*axis_ptr = axis;
			return true;
		}
	}
	return false;
}


bool InputManager::ParseJoyButton( std::string input, std::string *dev_ptr, Uint8 *button_ptr ) const
{
	int button_index = Str::FindLastInsensitive( input, "Button" );
	if( button_index >= 1 )
	{
		std::string dev = input.substr( 0, button_index );
		int button = atoi( input.c_str() + button_index + 6 ) - 1;
		if( button >= 0 )
		{
			if( dev_ptr )
				*dev_ptr = dev;
			if( button_ptr )
				*button_ptr = button;
			return true;
		}
	}
	return false;
}


bool InputManager::ParseJoyHat( std::string input, std::string *dev_ptr, Uint8 *hat_ptr, Uint8 *dir_ptr ) const
{
	int hat_index = Str::FindLastInsensitive( input, "Hat" );
	if( hat_index >= 1 )
	{
		std::string dev = input.substr( 0, hat_index );
		int hat = atoi( input.c_str() + hat_index + 3 ) - 1;
		if( hat >= 0 )
		{
			const char *dir_str = input.c_str() + hat_index + 4;
			while( (dir_str[0] >= '0') && (dir_str[0] <= '9') )
				dir_str ++;
			
			int dir = SDL_HAT_CENTERED;
			if( CStr::FindInsensitive( dir_str, "Up" ) >= 0 )
				dir |= SDL_HAT_UP;
			if( CStr::FindInsensitive( dir_str, "Down" ) >= 0 )
				dir |= SDL_HAT_DOWN;
			if( CStr::FindInsensitive( dir_str, "Left" ) >= 0 )
				dir |= SDL_HAT_LEFT;
			if( CStr::FindInsensitive( dir_str, "Right" ) >= 0 )
				dir |= SDL_HAT_RIGHT;
			
			if( dir || (strcasecmp( dir_str, "Center" ) == 0) )
			{
				if( dev_ptr )
					*dev_ptr = dev;
				if( hat_ptr )
					*hat_ptr = hat;
				if( dir_ptr )
					*dir_ptr = dir;
				return true;
			}
		}
	}
	return false;
}


// ---------------------------------------------------------------------------


bool InputManager::ValidInput( std::string input ) const
{
	return (KeyID(input) != SDLK_UNKNOWN) || MouseID(input) || ParseJoyAxis(input) || ParseJoyButton(input) || ParseJoyHat(input);
}


// ---------------------------------------------------------------------------


uint8_t InputManager::AddControl( std::string name, std::string preferred_device, double low, double high )
{
	uint8_t control = ControlID( name );
	if( ! control )
	{
		control = ControlNames.size() + 1;
		ControlNames[ control ] = name;
	}
	
	if( preferred_device.length() )
		PreferredDevices[ control ] = preferred_device;
	
	if( (low != -1.) || (high != 1.) )
		ControlRanges[ control ] = std::pair<double,double>( low, high );
	
	return control;
}


std::string InputManager::ControlName( uint8_t control ) const
{
	std::map<uint8_t,std::string>::const_iterator iter = ControlNames.find( control );
	if( iter != ControlNames.end() )
		return iter->second;
	
	return GenericControlName( control );
}


std::string InputManager::GenericControlName( uint8_t control ) const
{
	char control_string[ 32 ] = "";
	snprintf( control_string, sizeof(control_string), "Control%i", control );
	return std::string( control_string );
}


uint8_t InputManager::ControlID( std::string name ) const
{
	for( std::map<uint8_t,std::string>::const_iterator iter = ControlNames.begin(); iter != ControlNames.end(); iter ++ )
	{
		// First see if this control ID has an assigned name.
		if( Str::EqualsInsensitive( iter->second, name ) )
			return iter->first;
	}
	
	for( std::map<uint8_t,std::string>::const_iterator iter = ControlNames.begin(); iter != ControlNames.end(); iter ++ )
	{
		// If it matched no assigned name, see if it matches an auto-generated name like "Control66".
		if( Str::EqualsInsensitive( GenericControlName(iter->first), name ) )
			return iter->first;
	}
	
	return 0;
}


// ---------------------------------------------------------------------------


bool InputManager::IsPreferredDevice( std::string joy_name, uint8_t control ) const
{
	std::map<uint8_t,std::string>::const_iterator preferred_device = PreferredDevices.find( control );
	if( preferred_device != PreferredDevices.end() )
		return DeviceType(joy_name) == preferred_device->second;
	return false;
}


std::string InputManager::DeviceType( std::string joy_name ) const
{
	std::string dev = "Joy";
	int best_index = INT_MAX;
	for( std::set<std::string>::const_iterator dev_iter = DeviceTypes.begin(); dev_iter != DeviceTypes.end(); dev_iter ++ )
	{
		int index = Str::FindInsensitive( joy_name, *dev_iter );
		if( (index >= 0) && (index < best_index) )
		{
			dev = *dev_iter;
			best_index = index;
		}
	}
	return dev;
}


// ---------------------------------------------------------------------------


void InputManager::Update( void )
{
	std::set<uint8_t> digital_pressed, found_preferred_device;
	
	for( std::map<uint8_t,double>::iterator value = ControlValues.begin(); value != ControlValues.end(); value ++ )
	{
		ControlValues[ value->first ] = 0.;
		value = ControlValues.find( value->first );
	}
	
	if( Raptor::Game->ReadKeyboard && ! Raptor::Game->Console.IsActive() )
	{
		for( std::map<SDLKey,uint8_t>::const_iterator bind = Raptor::Game->Cfg.KeyBinds.begin(); bind != Raptor::Game->Cfg.KeyBinds.end(); bind ++ )
		{
			if( Raptor::Game->Keys.KeyDown( bind->first ) )
			{
				ControlValues[ bind->second ] += 1.;
				digital_pressed.insert( bind->second );
			}
		}
	}
	
	if( Raptor::Game->ReadMouse )
	{
		for( std::map<Uint8,uint8_t>::const_iterator bind = Raptor::Game->Cfg.MouseBinds.begin(); bind != Raptor::Game->Cfg.MouseBinds.end(); bind ++ )
		{
			if( Raptor::Game->Mouse.ButtonDown( bind->first ) )
			{
				ControlValues[ bind->second ] += 1.;
				digital_pressed.insert( bind->second );
			}
		}
	}
	
	for( std::map<Sint32,JoystickState>::const_iterator joy = Raptor::Game->Joy.Joysticks.begin(); joy != Raptor::Game->Joy.Joysticks.end(); joy ++ )
	{
		for( std::map<std::string,std::map<Uint8,uint8_t> >::const_iterator dev = Raptor::Game->Cfg.JoyButtonBinds.begin(); dev != Raptor::Game->Cfg.JoyButtonBinds.end(); dev ++ )
		{
			if( dev->first != DeviceType(joy->second.Name) )
				continue;
			for( std::map<Uint8,uint8_t>::const_iterator bind = dev->second.begin(); bind != dev->second.end(); bind ++ )
			{
				if( joy->second.ButtonDown( bind->first ) )
				{
					ControlValues[ bind->second ] += 1.;
					digital_pressed.insert( bind->second );
				}
			}
		}
		
		for( std::map<std::string,std::map<Uint8,std::map<Uint8,uint8_t> > >::const_iterator dev = Raptor::Game->Cfg.JoyHatBinds.begin(); dev != Raptor::Game->Cfg.JoyHatBinds.end(); dev ++ )
		{
			if( dev->first != DeviceType(joy->second.Name) )
				continue;
			for( std::map<Uint8,std::map<Uint8,uint8_t> >::const_iterator hat = dev->second.begin(); hat != dev->second.end(); hat ++ )
			{
				for( std::map<Uint8,uint8_t>::const_iterator bind = hat->second.begin(); bind != hat->second.end(); bind ++ )
				{
					if( joy->second.HatDir( hat->first, bind->first ) )
					{
						ControlValues[ bind->second ] += 1.;
						digital_pressed.insert( bind->second );
					}
				}
			}
		}
	}
	
	for( std::map<Sint32,JoystickState>::const_iterator joy = Raptor::Game->Joy.Joysticks.begin(); joy != Raptor::Game->Joy.Joysticks.end(); joy ++ )
	{
		std::string joy_device_type = DeviceType( joy->second.Name );
		for( std::map<std::string,std::map<Uint8,uint8_t> >::const_iterator dev = Raptor::Game->Cfg.JoyAxisBinds.begin(); dev != Raptor::Game->Cfg.JoyAxisBinds.end(); dev ++ )
		{
			if( dev->first != joy_device_type )
				continue;
			for( std::map<Uint8,uint8_t>::const_iterator bind = dev->second.begin(); bind != dev->second.end(); bind ++ )
			{
				if( joy->second.HasAxis( bind->first ) && ! digital_pressed.count( bind->second ) )
				{
					bool joy_preferred = IsPreferredDevice( joy->second.Name, bind->second );
					bool found_preferred = found_preferred_device.count( bind->second );
					if( found_preferred && ! joy_preferred )
						continue;
					if( joy_preferred && ! found_preferred )
					{
						found_preferred_device.insert( bind->second );
						ControlValues[ bind->second ] = 0.;
					}
					
					double low = -1., high = 1.;
					std::map<uint8_t,std::pair<double,double> >::const_iterator range = ControlRanges.find( bind->second );
					if( range != ControlRanges.end() )
					{
						low = range->second.first;
						high = range->second.second;
					}
					
					double deadzone = 0., deadedge = 0., exponent = 1.;
					if( joy_device_type == "Xbox" )
					{
#if SDL_VERSION_ATLEAST(2,0,0)
						if( bind->first >= 4 ) // Triggers
						{
							deadedge = Raptor::Game->Cfg.SettingAsDouble( "joy_deadzone_triggers", 0.02 );
							exponent = 1. + Raptor::Game->Cfg.SettingAsDouble( "joy_smooth_triggers", 0.75 );
						}
#else
						if( bind->first == 2 ) // Triggers (Combined)
						{
							deadzone = Raptor::Game->Cfg.SettingAsDouble( "joy_deadzone_triggers", 0.02 );
							exponent = 1. + Raptor::Game->Cfg.SettingAsDouble( "joy_smooth_triggers", 0.75 );
						}
#endif
						else
						{
							deadzone = Raptor::Game->Cfg.SettingAsDouble( "joy_deadzone_thumbsticks", 0.1 );
							exponent = 1. + Raptor::Game->Cfg.SettingAsDouble( "joy_smooth_thumbsticks", 2. );
						}
					}
					else if( joy_device_type == "Pedal" )
					{
						deadzone = Raptor::Game->Cfg.SettingAsDouble( "joy_deadzone", 0.03 );
						exponent = 1. + Raptor::Game->Cfg.SettingAsDouble( "joy_smooth_pedals" );
					}
					else if( joy_device_type == "Throttle" )
						deadedge = Raptor::Game->Cfg.SettingAsDouble( "joy_deadzone", 0.03 );
					else if( joy_device_type == "Wheel" )
					{
						if( bind->first == 0 ) // Steering Wheel
						{
							deadzone = Raptor::Game->Cfg.SettingAsDouble( "joy_deadzone", 0.03 );
							exponent = 1. + Raptor::Game->Cfg.SettingAsDouble( "joy_smooth" );
						}
						else // Pedals
						{
							deadedge = Raptor::Game->Cfg.SettingAsDouble( "joy_deadzone", 0.03 );
							exponent = 1. + Raptor::Game->Cfg.SettingAsDouble( "joy_smooth_pedals" );
						}
					}
					else // Joy
					{
						#if SDL_VERSION_ATLEAST(2,0,0)
							Uint8 throttle_axis = (joy_device_type == "SideWinder") ? 3 : 2;
						#else
							Uint8 throttle_axis = 2;
						#endif
						if( bind->first == throttle_axis ) // Throttle
							deadedge = Raptor::Game->Cfg.SettingAsDouble( "joy_deadzone", 0.03 );
						else
							deadzone = Raptor::Game->Cfg.SettingAsDouble( "joy_deadzone", 0.03 );
						
						if( bind->first == 0 )
							exponent = 1. + Raptor::Game->Cfg.SettingAsDouble( "joy_smooth_x" );
						else if( bind->first == 1 )
							exponent = 1. + Raptor::Game->Cfg.SettingAsDouble( "joy_smooth_y", 0.125 );
						else if( bind->first != throttle_axis ) // Twist
							exponent = 1. + Raptor::Game->Cfg.SettingAsDouble( "joy_smooth_z", 0.5 );
					}
					
					ControlValues[ bind->second ] += Num::SignedPow( joy->second.AxisScaled( bind->first, low, high, deadzone, deadedge ), exponent );
				}
			}
		}
	}
}


// ---------------------------------------------------------------------------


bool InputManager::HasControlAxis( uint8_t control ) const
{
	const std::map<std::string, std::map<Uint8, uint8_t> >::const_iterator joy_axis_binds_end = Raptor::Game->Cfg.JoyAxisBinds.end();
	for(  std::map<std::string, std::map<Uint8, uint8_t> >::const_iterator dev_iter = Raptor::Game->Cfg.JoyAxisBinds.begin(); dev_iter != joy_axis_binds_end; dev_iter ++ )
	{
		const std::map<Uint8, uint8_t>::const_iterator dev_iter_second_end = dev_iter->second.end();
		for(  std::map<Uint8, uint8_t>::const_iterator bind = dev_iter->second.begin(); bind != dev_iter_second_end; bind ++ )
		{
			if( bind->second == control )
			{
				for( std::map<Sint32,JoystickState>::const_iterator joy = Raptor::Game->Joy.Joysticks.begin(); joy != Raptor::Game->Joy.Joysticks.end(); joy ++ )
				{
					if( DeviceType( joy->second.Name ) != dev_iter->first )
						continue;
					if( joy->second.HasAxis( bind->first ) )
						return true;
				}
			}
		}
	}
	
	return false;
}


double InputManager::ControlTotal( uint8_t control ) const
{
	std::map<uint8_t,double>::const_iterator control_iter = ControlValues.find( control );
	return (control_iter != ControlValues.end()) ? control_iter->second : 0.;
}


double InputManager::ControlTotal( std::string name ) const
{
	return ControlTotal( ControlID(name) );
}


double InputManager::ControlValue( uint8_t control ) const
{
	double total = ControlTotal( control );
	if( total > 1. )
		return 1.;
	if( total < -1. )
		return -1.;
	return total;
}


double InputManager::ControlValue( std::string name ) const
{
	return ControlValue( ControlID(name) );
}


int8_t InputManager::ControlPressed( uint8_t control, double threshold ) const
{
	double value = ControlTotal( control );
	if( value >= threshold )
		return 1;
	if( value <= -threshold )
		return -1;
	return 0;
}


int8_t InputManager::ControlPressed( std::string name, double threshold ) const
{
	return ControlPressed( ControlID(name) );
}


uint8_t InputManager::EventBound( const SDL_Event *event ) const
{
	return Raptor::Game->Cfg.BoundControl( event, true );
}
