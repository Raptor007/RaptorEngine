/*
 *  Player.h
 */

#pragma once
class Player;

#include "PlatformSpecific.h"

#include <stdint.h>
#include <string>
#include <map>
#include <vector>


class Player
{
public:
	uint16_t ID;
	std::string Name;
	std::map<std::string,std::string> Properties;
	
	Player( uint16_t id = 0 );
	virtual ~Player();
	
	bool HasProperty( std::string name ) const;
	std::string PropertyAsString( std::string name, const char *ifndef = NULL, const char *ifempty = NULL ) const;
	double PropertyAsDouble( std::string name, double ifndef = 0., double ifempty = 0. ) const;
	int PropertyAsInt( std::string name, int ifndef = 0, int ifempty = 0 ) const;
	bool PropertyAsBool( std::string name, bool ifndef = false, bool ifempty = false ) const;
	std::vector<int> PropertyAsInts( std::string name ) const;
};
