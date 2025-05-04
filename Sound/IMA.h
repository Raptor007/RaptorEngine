/*
 *  IMA.h
 */

#pragma once

#include "PlatformSpecific.h"
#include <stdio.h>
#include <stdint.h>

namespace IMA
{
	uint8_t Encode( int16_t value, int16_t *prev, int8_t *idx );
	int16_t Decode( uint8_t nibble, int16_t *prev, int8_t *idx );
	void EncodeSegment( int16_t *from, uint8_t *into, size_t samples, int16_t *prev, int8_t *idx );
	void DecodeSegment( uint8_t *from, int16_t *into, size_t samples, int16_t *prev, int8_t *idx );
	void EncodeBuffer( int16_t *from, uint8_t *into, size_t samples );
	void DecodeBuffer( uint8_t *from, int16_t *into, size_t samples );
}
