/*
 *  File.h
 */

#pragma once

#include "PlatformSpecific.h"

#include <string>


namespace File
{
	bool Exists( const char *filename );
	std::string AsString( const char *filename );
}
