/*
 *	strtok_r.cpp
 */

#ifdef WIN32
#ifndef strtok_r

#include "strtok_r.h"
#include <cstring>
extern "C" {

char *strtok_r( char *str, const char *delim, char **saveptr )
{
	if( !str && !(str = *saveptr) )
		return NULL;

	str += strspn( str, delim );
	if( !*str )
		return *saveptr = 0;
	*saveptr = str + strcspn( str, delim );

	if( **saveptr )
		*(*saveptr)++ = 0;
	else
		*saveptr = 0;
	
	return str;
}

}
#endif
#endif
