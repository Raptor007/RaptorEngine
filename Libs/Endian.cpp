/*
 *  Endian.cpp
 */

#include "Endian.h"
#include <cstring>


bool Endian::Big( void )
{
	#ifdef ENDIAN_BIG
		return true;
	#else
		return false;
	#endif
}

bool Endian::Little( void )
{
	#ifdef ENDIAN_BIG
		return false;
	#else
		return true;
	#endif
}


void Endian::ByteSwap( void *ptr, int bytes )
{
	for( int i = 0; i < bytes/2; i ++ )
	{
		uint8_t swap = ((uint8_t*)( ptr ))[ i ];
		((uint8_t*)( ptr ))[ i ] = ((uint8_t*)( ptr ))[ bytes - i - 1 ];
		((uint8_t*)( ptr ))[ bytes - i - 1 ] = swap;
	}
}

void Endian::ByteSwap( void *ptr, int bytes, int count )
{
	for( int word = 0; word < count; word ++ )
	{
		for( int i = 0; i < bytes/2; i ++ )
		{
			uint8_t swap = ((uint8_t*)( ptr ))[ i + (bytes * word) ];
			((uint8_t*)( ptr ))[ i + (bytes * word) ] = ((uint8_t*)( ptr ))[ (bytes - i - 1) + (bytes * word) ];
			((uint8_t*)( ptr ))[ (bytes - i - 1) + (bytes * word) ] = swap;
		}
	}
}

void Endian::ByteSwapCopy( const void *src, void *dest, int bytes )
{
	for( int i = 0; i < bytes; i ++ )
		((uint8_t*)( dest ))[ bytes - i - 1 ] = ((const uint8_t*)( src ))[ i ];
}

void Endian::ByteSwapCopy( const void *src, void *dest, int bytes, int count )
{
	for( int word = 0; word < count; word ++ )
	{
		for( int i = 0; i < bytes; i ++ )
			((uint8_t*)( dest ))[ (bytes - i - 1) + (bytes * word) ] = ((const uint8_t*)( src ))[ i + (bytes * word) ];
	}
}

void Endian::CopyBig( const void *src, void *dest, int bytes )
{
	#ifdef ENDIAN_BIG
		memmove( dest, src, bytes );
	#else
		ByteSwapCopy( src, dest, bytes );
	#endif
}

void Endian::CopyLittle( const void *src, void *dest, int bytes )
{
	#ifdef ENDIAN_BIG
		ByteSwapCopy( src, dest, bytes );
	#else
		memmove( dest, src, bytes );
	#endif
}


uint16_t Endian::ReadBig16( const void *src )
{
	#ifdef ENDIAN_BIG
		return *((const uint16_t*) src);
	#else
		uint16_t value;
		ByteSwapCopy( src, &value, 2 );
		return value;
	#endif
}

uint32_t Endian::ReadBig32( const void *src )
{
	#ifdef ENDIAN_BIG
		return *((const uint32_t*) src);
	#else
		uint32_t value;
		ByteSwapCopy( src, &value, 4 );
		return value;
	#endif
}

uint64_t Endian::ReadBig64( const void *src )
{
	#ifdef ENDIAN_BIG
		return *((const uint64_t*) src);
	#else
		uint64_t value;
		ByteSwapCopy( src, &value, 8 );
		return value;
	#endif
}


uint16_t Endian::ReadLittle16( const void *src )
{
	#ifdef ENDIAN_LITTLE
		return *((const uint16_t*) src);
	#else
		uint16_t value;
		ByteSwapCopy( src, &value, 2 );
		return value;
	#endif
}

uint32_t Endian::ReadLittle32( const void *src )
{
	#ifdef ENDIAN_LITTLE
		return *((const uint32_t*) src);
	#else
		uint32_t value;
		ByteSwapCopy( src, &value, 4 );
		return value;
	#endif
}

uint64_t Endian::ReadLittle64( const void *src )
{
	#ifdef ENDIAN_LITTLE
		return *((const uint64_t*) src);
	#else
		uint64_t value;
		ByteSwapCopy( src, &value, 8 );
		return value;
	#endif
}


void Endian::WriteBig16( uint16_t value, void *dest )
{
	#ifdef ENDIAN_BIG
		memmove( dest, &value, 2 );
	#else
		ByteSwapCopy( &value, dest, 2 );
	#endif
}

void Endian::WriteBig32( uint32_t value, void *dest )
{
	#ifdef ENDIAN_BIG
		memmove( dest, &value, 4 );
	#else
		ByteSwapCopy( &value, dest, 4 );
	#endif
}

void Endian::WriteBig64( uint64_t value, void *dest )
{
	#ifdef ENDIAN_BIG
		memmove( dest, &value, 8 );
	#else
		ByteSwapCopy( &value, dest, 8 );
	#endif
}


void Endian::WriteLittle16( uint16_t value, void *dest )
{
	#ifdef ENDIAN_LITTLE
		memmove( dest, &value, 2 );
	#else
		ByteSwapCopy( &value, dest, 2 );
	#endif
}

void Endian::WriteLittle32( uint32_t value, void *dest )
{
	#ifdef ENDIAN_LITTLE
		memmove( dest, &value, 4 );
	#else
		ByteSwapCopy( &value, dest, 4 );
	#endif
}

void Endian::WriteLittle64( uint64_t value, void *dest )
{
	#ifdef ENDIAN_LITTLE
		memmove( dest, &value, 8 );
	#else
		ByteSwapCopy( &value, dest, 8 );
	#endif
}
