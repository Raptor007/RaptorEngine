/*
 *  Str.cpp
 */

#include "Str.h"

#include <cstdlib>
#include <cstring>
#include <algorithm>
#ifdef WIN32
#ifndef _MSC_VER
#include "strtok_r.h"
#endif
#endif


char *CStr::Copy( const char *str )
{
	if( str )
		return strdup(str);
	
	return NULL;
}


void CStr::Delete( char *str )
{
	if( str )
		free( str );
}


int CStr::Count( const char *str, const char *sub )
{
	int count = 0;
	size_t sub_len = strlen(sub);
	while( (str = strstr( str, sub )) )
	{
		count ++;
		str += sub_len;
	}
	return count;
}


double CStr::AsDouble( const char *str )
{
	if( ! str )
		return 0.0;
	if( strcasecmp( str, "true" ) == 0 )
		return 1.0;
	return atof( str );
}


int CStr::AsInt( const char *str )
{
	if( ! str )
		return 0;
	if( strcasecmp( str, "true" ) == 0 )
		return 1;
	return atoi( str );
}


bool CStr::AsBool( const char *str )
{
	return AsInt( str );
}


bool CStr::EqualsInsensitive( const char *a, const char *b )
{
	return (strcasecmp( a, b ) == 0);
}


int CStr::FindInsensitive( const char *str, const char *search_for )
{
	size_t search_len = strlen(search_for);
	int count = strlen(str) + 1 - search_len;
	for( int i = 0; i < count; i ++ )
	{
		if( strncasecmp( str + i, search_for, search_len ) == 0 )
			return i;
	}
	return -1;
}


int CStr::FindLastInsensitive( const char *str, const char *search_for )
{
	size_t search_len = strlen(search_for);
	int count = strlen(str) + 1 - search_len;
	for( int i = count - 1; i >= 0; i -- )
	{
		if( strncasecmp( str + i, search_for, search_len ) == 0 )
			return i;
	}
	return -1;
}


bool CStr::BeginsWith( const char *str, const char *search_for )
{
	return (strncasecmp( str, search_for, strlen(search_for) ) == 0);
}


bool CStr::EndsWith( const char *str, const char *search_for )
{
	size_t str_len = strlen(str);
	size_t search_len = strlen(search_for);
	if( search_len > str_len )
		return false;
	return (strncasecmp( str + str_len - search_len, search_for, search_len ) == 0);
}


bool CStr::Less::operator()( const char *a, const char *b ) const
{
	if( ! b )
		return false;
	if( ! a )
		return true;
	
	return (strcmp( a, b ) < 0);
}


std::string CStr::Escape( const char *str, const char *original, const char *escaped )
{
	std::string return_str;
	
	if( str && original && escaped && (strlen(original) == strlen(escaped)) )
	{
		const char *str_ptr = str;
		while( *str_ptr != '\0' )
		{
			if( (*str_ptr == '\\') || strchr( original, *str_ptr ) )
			{
				return_str += '\\';
				
				const char *found_char = strchr( original, *str_ptr );
				if( found_char )
					return_str += escaped[ found_char - original ];
				else
					return_str += *str_ptr;
			}
			else
				return_str += *str_ptr;
			
			str_ptr ++;
		}
	}
	
	return return_str;
}


std::string CStr::Unescape( const char *str, const char *original, const char *escaped )
{
	std::string return_str;
	
	if( str && original && escaped && (strlen(original) == strlen(escaped)) )
	{
		const char *str_ptr = str;
		while( *str_ptr != '\0' )
		{
			if( *str_ptr == '\\' )
			{
				str_ptr ++;
				
				const char *found_char = strchr( escaped, *str_ptr );
				if( found_char )
					return_str += original[ found_char - escaped ];
				else
					return_str += *str_ptr;
			}
			else
				return_str += *str_ptr;
			
			str_ptr ++;
		}
	}
	
	return return_str;
}


std::string CStr::CapitalizedCopy( const char *str )
{
	return Str::CapitalizedCopy( std::string(str) );
}


std::string CStr::LowercaseCopy( const char *str )
{
	return Str::LowercaseCopy( std::string(str) );
}


void CStr::ReplaceChars( char *str, const char *find, const char *replace )
{
	if( str && find && replace && (strlen(find) == strlen(replace)) )
	{
		char *str_ptr = str;
		while( *str_ptr != '\0' )
		{
			const char *found_char = strchr( find, *str_ptr );
			if( found_char )
				*str_ptr = replace[ found_char - find ];
			
			str_ptr ++;
		}
	}
}


std::vector<std::string> CStr::SplitToVector( const char *str, const char *delimiters )
{
	std::vector<std::string> return_vec;
	
	char *copy = CStr::Copy(str);
	char *pch = NULL;
	char *save = NULL;
	pch = strtok_r( copy, delimiters, &save );
	while( pch )
	{
		return_vec.push_back( std::string(pch) );
		pch = strtok_r( NULL, delimiters, &save );
	}
	CStr::Delete(copy);
	
	return return_vec;
}


std::list<std::string> CStr::SplitToList( const char *str, const char *delimiters )
{
	std::list<std::string> return_list;
	
	char *copy = CStr::Copy(str);
	char *pch = NULL;
	char *save = NULL;
	pch = strtok_r( copy, delimiters, &save );
	while( pch )
	{
		return_list.push_back( std::string(pch) );
		pch = strtok_r( NULL, delimiters, &save );
	}
	CStr::Delete(copy);
	
	return return_list;
}


std::string CStr::Join( std::vector<char*> container, const char *delimiter )
{
	std::string return_string;
	
	for( std::vector<char*>::iterator str_iter = container.begin(); str_iter != container.end(); str_iter ++ )
	{
		if( str_iter != container.begin() )
			return_string += std::string(delimiter);

		return_string += std::string(*str_iter);
	}
	
	return return_string;
}


std::string CStr::Join( std::list<char*> container, const char *delimiter )
{
	std::string return_string;
	
	for( std::list<char*>::iterator str_iter = container.begin(); str_iter != container.end(); str_iter ++ )
	{
		if( str_iter != container.begin() )
			return_string += std::string(delimiter);

		return_string += std::string(*str_iter);
	}
	
	return return_string;
}


std::list<std::string> CStr::ParseCommand( const char *cmd, const char *original, const char *escaped, const char *comment_marker )
{
	std::list<std::string> return_list;
	
	if( cmd )
	{
		char *buffer = CStr::Copy(cmd);
		const char *DELIMITERS = "; \t";
		std::string item;
		
		// Ignore comments.
		char *comment = strstr( buffer, comment_marker );
		if( comment )
			*comment = '\0';
		
		// Parse the command.
		char *buffer_ptr = buffer;
		while( *buffer_ptr )
		{
			// Clear the item buffer.
			item = "";
			
			// Skip the spaces, tabs, semicolons, etc.
			while( *buffer_ptr && strchr( DELIMITERS, *buffer_ptr ) )
			{
				// Check for semicolons (not within quotes) to delimit multiple commands.
				if( *buffer_ptr == ';' )
					return_list.push_back( ";;" );  // Turn ; into ;; so ";" can be used to bind semicolon.
				
				buffer_ptr ++;
			}
			
			// Split into items if we have an applicable command.
			char *end_ptr = NULL;
			
			// In case we have an item like "", make sure it doesn't get discarded for zero-length.
			bool must_add = false;
			
			// Find the end of this item.
			if( *buffer_ptr == '\"' )
			{
				must_add = true;
				
				buffer_ptr ++;
				end_ptr = strchr( buffer_ptr, '\"' );
				
				// Don't end the item at escaped quotes.
				// FIXME: Some situations will make this parsing incorrect!
				while( end_ptr && (*(end_ptr-1) == '\\') && (*(end_ptr-2) != '\\') )
					end_ptr = strchr( end_ptr+1, '\"' );
			}
			else
			{
				for( size_t i = 0; i < strlen(DELIMITERS); i ++ )
				{
					char *this_end_ptr = strchr( buffer_ptr, DELIMITERS[ i ] );
					if( this_end_ptr && ((! end_ptr) || (this_end_ptr < end_ptr)) )
						end_ptr = this_end_ptr;
				}
			}
			
			if( end_ptr )
			{
				char swap = *end_ptr;
				*end_ptr = '\0';
				item = std::string( buffer_ptr );
				*end_ptr = swap;
				
				// There are more items to go after this one.
				// Don't skip past semicolons, because we parse those out above.
				if( swap == ';' )
					buffer_ptr = end_ptr;
				else
					buffer_ptr = end_ptr + 1;
			}
			else
			{
				item = std::string( buffer_ptr );
				
				// Final item.
				buffer_ptr = buffer + strlen(buffer);
			}
			
			// Build a list of the command and its parameters.
			if( must_add || item.length() )
			{
				if( original && escaped )
					return_list.push_back( Str::Unescape( item, original, escaped ) );
				else
					return_list.push_back( item );
			}
		}
		
		
		CStr::Delete(buffer);
		buffer = NULL;
	}
	
	return return_list;
}


// ---------------------------------------------------------------------------


int Str::Count( std::string str, std::string sub )
{
	return CStr::Count( str.c_str(), sub.c_str() );
}


double Str::AsDouble( std::string str )
{
	return CStr::AsDouble( str.c_str() );
}


int Str::AsInt( std::string str )
{
	return CStr::AsInt( str.c_str() );
}


bool Str::AsBool( std::string str )
{
	return CStr::AsBool( str.c_str() );
}


bool Str::EqualsInsensitive( std::string a, std::string b )
{
	return CStr::EqualsInsensitive( a.c_str(), b.c_str() );
}


int Str::FindInsensitive( std::string str, std::string search_for )
{
	return CStr::FindInsensitive( str.c_str(), search_for.c_str() );
}


int Str::FindLastInsensitive( std::string str, std::string search_for )
{
	return CStr::FindLastInsensitive( str.c_str(), search_for.c_str() );
}


bool Str::BeginsWith( std::string str, std::string search_for )
{
	return CStr::BeginsWith( str.c_str(), search_for.c_str() );
}


bool Str::EndsWith( std::string str, std::string search_for )
{
	return CStr::EndsWith( str.c_str(), search_for.c_str() );
}


std::string Str::Escape( std::string str, const char *original, const char *escaped )
{
	return CStr::Escape( str.c_str(), original, escaped );
}


std::string Str::Unescape( std::string str, const char *original, const char *escaped )
{
	return CStr::Unescape( str.c_str(), original, escaped );
}


std::vector<std::string> Str::SplitToVector( std::string str, const char *delimiters )
{
	return CStr::SplitToVector( str.c_str(), delimiters );
}


std::string Str::CapitalizedCopy( std::string str )
{
	std::transform( str.begin(), str.end(), str.begin(), toupper );
	return str;
}


std::string Str::LowercaseCopy( std::string str )
{
	std::transform( str.begin(), str.end(), str.begin(), tolower );
	return str;
}


std::list<std::string> Str::SplitToList( std::string str, const char *delimiters )
{
	return CStr::SplitToList( str.c_str(), delimiters );
}


std::string Str::Join( std::vector<std::string> container, const char *delimiter )
{
	std::string return_string;
	
	for( std::vector<std::string>::iterator str_iter = container.begin(); str_iter != container.end(); str_iter ++ )
	{
		if( str_iter != container.begin() )
			return_string += std::string(delimiter);

		return_string += *str_iter;
	}
	
	return return_string;
}


std::string Str::Join( std::list<std::string> container, const char *delimiter )
{
	std::string return_string;
	
	for( std::list<std::string>::iterator str_iter = container.begin(); str_iter != container.end(); str_iter ++ )
	{
		if( str_iter != container.begin() )
			return_string += std::string(delimiter);

		return_string += *str_iter;
	}
	
	return return_string;
}


std::list<std::string> Str::ParseCommand( std::string cmd, const char *original, const char *escaped, const char *comment_marker )
{
	return CStr::ParseCommand( cmd.c_str(), original, escaped, comment_marker );
}
