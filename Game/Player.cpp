/*
 *  Player.cpp
 */

#include "Player.h"

#include "Str.h"
#include <cstdlib>
#include <cstring>


Player::Player( uint16_t id )
{
	ID = id;
}


Player::~Player()
{
}


// -----------------------------------------------------------------------------


bool Player::HasProperty( std::string name ) const
{
	std::map<std::string, std::string>::const_iterator found = Properties.find( name );
	return ( found != Properties.end() );
}


std::string Player::PropertyAsString( std::string name, const char *ifndef, const char *ifempty ) const
{
	std::map<std::string, std::string>::const_iterator found = Properties.find( name );
	if( found != Properties.end() )
	{
		if( found->second.empty() && ifempty )
			return std::string(ifempty);
		return found->second;
	}
	if( ifndef )
		return std::string(ifndef);
	return "";
}


double Player::PropertyAsDouble( std::string name, double ifndef, double ifempty ) const
{
	std::map<std::string, std::string>::const_iterator found = Properties.find( name );
	if( found != Properties.end() )
	{
		if( found->second.empty() )
			return ifempty;
		return Str::AsDouble( found->second );
	}
	return ifndef;
}


int Player::PropertyAsInt( std::string name, int ifndef, int ifempty ) const
{
	std::map<std::string, std::string>::const_iterator found = Properties.find( name );
	if( found != Properties.end() )
	{
		if( found->second.empty() )
			return ifempty;
		return Str::AsInt( found->second );
	}
	return ifndef;
}


bool Player::PropertyAsBool( std::string name, bool ifndef, bool ifempty ) const
{
	std::map<std::string, std::string>::const_iterator found = Properties.find( name );
	if( found != Properties.end() )
	{
		if( found->second.empty() )
			return ifempty;
		return Str::AsBool( found->second );
	}
	return ifndef;
}


std::vector<int> Player::PropertyAsInts( std::string name ) const
{
	std::vector<int> results;
	
	std::map<std::string, std::string>::const_iterator found = Properties.find( name );
	if( found != Properties.end() )
	{
		bool seeking = true;
		for( const char *buffer_ptr = found->second.c_str(); *buffer_ptr; buffer_ptr ++ )
		{
			bool numeric_char = strchr( "0123456789.", *buffer_ptr );
			if( seeking && numeric_char )
			{
				results.push_back( atoi(buffer_ptr) );
				seeking = false;
			}
			else if( ! numeric_char )
				seeking = true;
		}
	}
	
	return results;
}
