/*
 *  File.cpp
 */

#include "File.h"

#include <fstream>
#include <sys/stat.h>


bool File::Exists( const char *filename )
{
	struct stat buffer;
	return (stat( filename, &buffer ) == 0);
}


std::string File::AsString( const char *filename )
{
	std::string return_string;
	char buffer[ 1024*128 ] = "";
	
	std::ifstream input( filename );
	if( input.is_open() )
	{
		while( ! input.eof() )
		{
			buffer[ 0 ] = '\0';
			input.getline( buffer, 1024*128 );
			return_string += std::string(buffer) + std::string("\n");
		}
		
		input.close();
	}
	
	return return_string;
}
