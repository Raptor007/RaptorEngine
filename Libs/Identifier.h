/*
 *  Identifier.h
 */

#pragma once
template <typename T> class Identifier;
class IdentifierError;

#include "PlatformSpecific.h"

#include <set>
#include <exception>


template <typename T>
class Identifier
{
public:
	std::set<T> Available;
	T Initial, Next;
	bool ThrowExceptions;
	
	
	Identifier( T initial = 1 )
	{
		Initial = initial;
		Next = Initial;
		ThrowExceptions = false;
	}
	
	~Identifier()
	{
	}
	
	T NextAvailable( void )
	{
		T value = 0;
		
		if( Available.size() )
		{
			value = *(Available.begin());
			Available.erase( Available.begin() );
		}
		else if( Next >= Initial )
		{
			value = Next;
			Next ++;
		}
		else if( ThrowExceptions )
			throw IdentifierError();
		
		return value;
	}
	
	void Remove( T value )
	{
		if( value < Initial )
			;
		else if( value == Next - 1 )
			Next --;
		else if( value < Next )
			Available.insert( value );
	}
	
	void Clear( void )
	{
		Next = Initial;
		Available.clear();
	}
	
	void Clear( T initial )
	{
		Initial = initial;
		Clear();
	}
};


class IdentifierError : public std::exception
{
	const char *what() const throw();
};
