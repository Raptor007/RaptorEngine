/*
 *  Str.h
 */

#pragma once

#include "PlatformSpecific.h"
#include <cstddef>
#include <string>
#include <vector>
#include <list>


namespace CStr
{
	char *Copy( const char *str );
	void Delete( char *str );
	int Count( const char *str, const char *sub );
	
	double AsDouble( const char *str );
	int AsInt( const char *str );
	bool AsBool( const char *str );
	
	bool EqualsInsensitive( const char *a, const char *b );
	bool ContainsInsensitive( const char *str, const char *search_for );
	int FindInsensitive( const char *str, const char *search_for );
	int FindLastInsensitive( const char *str, const char *search_for );
	bool BeginsWith( const char *str, const char *search_for );
	bool EndsWith( const char *str, const char *search_for );
	
	class Less
	{
	public:
		bool operator()( const char *a, const char *b ) const;
	};
	
	std::string Escape( const char *str, const char *original, const char *escaped );
	std::string Unescape( const char *str, const char *original, const char *escaped );
	
	std::string CapitalizedCopy( const char *str );
	std::string LowercaseCopy( const char *str );
	
	void ReplaceChars( char *str, const char *find, const char *replace );
	
	std::vector<std::string> SplitToVector( const char *str, const char *delimiters );
	std::list<std::string> SplitToList( const char *str, const char *delimiters );
	std::string Join( std::vector<char*> container, const char *delimiter );
	std::string Join( std::list<char*> container, const char *delimiter );
	
	std::list<std::string> ParseCommand( const char *cmd, const char *original = NULL, const char *escaped = NULL, const char *comment_marker = "//" );
}


namespace Str
{
	int Count( std::string str, std::string sub );
	
	double AsDouble( std::string str );
	int AsInt( std::string str );
	bool AsBool( std::string str );
	
	bool EqualsInsensitive( std::string a, std::string b );
	bool ContainsInsensitive( std::string str, std::string search_for );
	int FindInsensitive( std::string str, std::string search_for );
	int FindLastInsensitive( std::string str, std::string search_for );
	bool BeginsWith( std::string str, std::string search_for );
	bool EndsWith( std::string str, std::string search_for );
	
	std::string Escape( std::string str, const char *original, const char *escaped );
	std::string Unescape( std::string str, const char *original, const char *escaped );
	
	std::string CapitalizedCopy( std::string str );
	std::string LowercaseCopy( std::string str );
	
	std::vector<std::string> SplitToVector( std::string str, const char *delimiters );
	std::list<std::string> SplitToList( std::string str, const char *delimiters );
	std::string Join( std::vector<std::string> container, const char *delimiter );
	std::string Join( std::list<std::string> container, const char *delimiter );
	
	std::list<std::string> ParseCommand( std::string cmd, const char *original = NULL, const char *escaped = NULL, const char *comment_marker = "//" );
}
