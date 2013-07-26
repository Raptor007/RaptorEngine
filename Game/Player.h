/*
 *  Player.h
 */

#pragma once
class Player;

#include "PlatformSpecific.h"

#include <stdint.h>
#include <string>
#include <map>


class Player
{
public:
	uint16_t ID;
	std::string Name;
	std::map<std::string,std::string> Properties;
	
	Player( uint16_t id = 0 );
	~Player();
};
