/*
 *  File.h
 */

#pragma once

#include "PlatformSpecific.h"

#include <string>
#include <vector>


namespace File
{
	bool Exists( const char *filename );
	bool Exists( const std::string &filename );
	std::string AsString( const char *filename );
	std::string AsString( const std::string &filename );
	std::vector<std::string> AsLines( const char *filename );
	std::vector<std::string> AsLines( const std::string &filename );
}
