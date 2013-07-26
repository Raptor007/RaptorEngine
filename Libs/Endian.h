/*
 *  Endian.h
 */

#pragma once

#include "PlatformSpecific.h"

#include <stdint.h>


#if defined(_ppc_) || defined(_ppc64_) || defined(__ppc__) || defined(__ppc64__) || defined(__POWERPC__) || defined(_M_PPC)
#define ENDIAN_BIG true
#else
#define ENDIAN_LITTLE true
#endif


namespace Endian
{
	bool Big( void );
	bool Little( void );
	
	void ByteSwap( void *ptr, int bytes );
	void ByteSwap( void *ptr, int bytes, int count );
	void ByteSwapCopy( void *src, void *dest, int bytes );
	void ByteSwapCopy( void *src, void *dest, int bytes, int count );
	void CopyBig( void *src, void *dest, int bytes );
	void CopyLittle( void *src, void *dest, int bytes );
	
	uint16_t ReadBig16( void *src );
	uint32_t ReadBig32( void *src );
	uint64_t ReadBig64( void *src );
	uint16_t ReadLittle16( void *src );
	uint32_t ReadLittle32( void *src );
	uint64_t ReadLittle64( void *src );
	
	void WriteBig16( uint16_t value, void *dest );
	void WriteBig32( uint32_t value, void *dest );
	void WriteBig64( uint64_t value, void *dest );
	void WriteLittle16( uint16_t value, void *dest );
	void WriteLittle32( uint32_t value, void *dest );
	void WriteLittle64( uint64_t value, void *dest );
}
