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
	std::map<SDLKey, uint8_t> KeyBinds;
	std::map<Uint8, uint8_t> MouseBinds;
	std::map<std::string, std::string> Settings;
	
	std::map<SDLKey, std::string> KeyNames;
	std::map<Uint8, std::string> MouseNames;
	std::map<uint8_t, std::string> ControlNames;
	
	
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
	
	bool Bind( SDL_Event *event, uint8_t control );
	void Unbind( uint8_t control );
	void UnbindAll( void );
	
	uint8_t GetControl( SDL_Event *event ) const;
	
	std::string ControlName( uint8_t control ) const;
	std::string KeyName( SDLKey key ) const;
	std::string MouseName( Uint8 mouse ) const;
	
	uint8_t ControlID( std::string name ) const;
	SDLKey KeyID( std::string name ) const;
	Uint8 MouseID( std::string name ) const;
	
	
	enum
	{
		UNBOUND = 0
	};
};
