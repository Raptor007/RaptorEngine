/*
 *  ClientConfig.h
 */

#pragma once
class ClientConfig;

#include "platforms.h"

#include <map>
#include <string>
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
	~ClientConfig();
	
	void SetDefaults( void );
	
	void Command( std::string str, bool show_in_console = false );

	void Load( std::string filename );
	void Save( std::string filename );
	
	bool HasSetting( std::string name );
	std::string SettingAsString( std::string name, const char *ifndef = NULL );
	double SettingAsDouble( std::string name, double ifndef = 0. );
	int SettingAsInt( std::string name, int ifndef = 0 );
	bool SettingAsBool( std::string name, bool ifndef = false );
	
	bool Bind( SDL_Event *event, uint8_t control );
	void Unbind( uint8_t control );
	void UnbindAll( void );
	
	uint8_t GetControl( SDL_Event *event );
	
	std::string ControlName( uint8_t control );
	std::string KeyName( SDLKey key );
	std::string MouseName( Uint8 mouse );
	
	uint8_t ControlID( std::string name );
	SDLKey KeyID( std::string name );
	Uint8 MouseID( std::string name );
	
	
	enum
	{
		UNBOUND = 0
	};
};
