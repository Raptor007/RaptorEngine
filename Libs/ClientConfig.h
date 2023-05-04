/*
 *  ClientConfig.h
 */

#pragma once
class ClientConfig;

#include "PlatformSpecific.h"
#include <cstddef>
#include <map>
#include <string>
#include <vector>
#include <SDL/SDL.h>


class ClientConfig
{
public:
	std::map<std::string, std::string> Settings;
	
	std::map<SDLKey,uint8_t> KeyBinds;
	std::map<Uint8,uint8_t> MouseBinds;
	std::map<std::string,std::map<Uint8,uint8_t> > JoyAxisBinds;
	std::map<std::string,std::map<Uint8,uint8_t> > JoyButtonBinds;
	std::map<std::string,std::map<Uint8,std::map<Uint8,uint8_t> > > JoyHatBinds;
	
	
	ClientConfig( void );
	virtual ~ClientConfig();
	
	void SetDefaults( void );
	
	void Command( std::string str, bool show_in_console = false );

	void Load( std::string filename );
	void Save( std::string filename ) const;
	
	bool HasSetting( std::string name ) const;
	std::string SettingAsString( std::string name, const char *ifndef = NULL ) const;
	double SettingAsDouble( std::string name, double ifndef = 0. ) const;
	int SettingAsInt( std::string name, int ifndef = 0 ) const;
	bool SettingAsBool( std::string name, bool ifndef = false ) const;
	std::vector<double> SettingAsDoubles( std::string name ) const;
	std::vector<int> SettingAsInts( std::string name ) const;
	
	int8_t Bind( SDL_Event *event, uint8_t control );
	bool Bind( std::string input, uint8_t control );
	bool Unbind( std::string input );
	void Unbind( uint8_t control );
	void UnbindAll( void );
	size_t SwapBinds( uint8_t control_a, uint8_t control_b );
	
	uint8_t BoundControl( const SDL_Event *event, bool down_only = false ) const;
	uint8_t BoundControl( std::string input ) const;
	
	std::string ControlName( uint8_t control ) const;
	std::string KeyName( SDLKey key ) const;
	std::string MouseName( Uint8 mouse ) const;
	uint8_t ControlID( std::string name ) const;
	SDLKey KeyID( std::string name ) const;
	Uint8 MouseID( std::string name ) const;
	
	uint8_t KeyBind( SDLKey key ) const;
	uint8_t MouseBind( Uint8 button ) const;
	uint8_t JoyAxisBind( std::string dev, Uint8 axis ) const;
	uint8_t JoyButtonBind( std::string dev, Uint8 button ) const;
	uint8_t JoyHatBind( std::string dev, Uint8 hat, Uint8 dir ) const;
	
	bool HasControlBound( uint8_t control ) const;
	std::vector<std::string> ControlBoundTo( uint8_t control ) const;
};
