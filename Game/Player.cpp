/*
 *  Player.cpp
 */

#include "Player.h"

#include "Str.h"
#include "RaptorGame.h"
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


// -----------------------------------------------------------------------------


bool Player::PlayingVoice( void ) const
{
	std::map<uint16_t,PlaybackBuffer*>::const_iterator voice_buffer = Raptor::Game->Snd.VoiceBuffers.find( ID );
	if( (voice_buffer != Raptor::Game->Snd.VoiceBuffers.end()) && (voice_buffer->second->AudioChannel >= 0) )
		return true;
	
	if( (ID == Raptor::Game->PlayerID) && Raptor::Game->Mic.Transmitting )
		return true;
	
	return false;
}


uint8_t Player::VoiceChannel( void ) const
{
	std::map<uint16_t,PlaybackBuffer*>::const_iterator voice_buffer = Raptor::Game->Snd.VoiceBuffers.find( ID );
	if( (voice_buffer != Raptor::Game->Snd.VoiceBuffers.end()) && (voice_buffer->second->AudioChannel >= 0) )
		return voice_buffer->second->VoiceChannel;
	
	if( ID == Raptor::Game->PlayerID )
		return Raptor::Game->Mic.Transmitting;
	
	return Raptor::VoiceChannel::NONE;
}
