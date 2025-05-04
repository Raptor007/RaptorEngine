/*
 *  IMA.cpp
 */

#include "IMA.h"

static int8_t IndexTable[] = {
	-1, -1, -1, -1, 2, 4, 6, 8,
	-1, -1, -1, -1, 2, 4, 6, 8,
};

static int16_t StepSizeTable[] = {
	7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
	19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
	50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
	130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
	337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
	876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
	2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
	5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
	15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};


uint8_t IMA::Encode( int16_t value, int16_t *prev, int8_t *idx )
{
	int index = *idx;
	int step = StepSizeTable[ index ];
	int valpred = *prev;
	int delta = value;
	uint8_t nibble = 0;
	
	int diff = delta - valpred;
	if( diff < 0 )
	{
		nibble = 8;
		diff *= -1;
	}
	
	int tempstep = step;
	if( diff >= tempstep )
	{
		nibble |= 4;
		diff -= tempstep;
	}
	
	tempstep >>= 1;
	if( diff >= tempstep )
	{
		nibble |= 2;
		diff -= tempstep;
	}
	
	tempstep >>= 1;
	if( diff >= tempstep )
		nibble |= 1;
	
	int diffq = step >> 3;
	if( nibble & 1 )
		diffq += step >> 2;
	if( nibble & 2 )
		diffq += step >> 1;
	if( nibble & 4 )
		diffq += step;
	
	if( nibble & 8 )
		valpred -= diffq;
	else
		valpred += diffq;
	
	if( valpred > 32767 )
		valpred = 32767;
	else if( valpred < -32768 )
		valpred = -32768;
	
	index += IndexTable[ nibble ];
	if( index < 0 )
		index = 0;
	else if( index > 88 )
		index = 88;
	
	*prev = valpred;
	*idx = index;
	return nibble;
}


int16_t IMA::Decode( uint8_t nibble, int16_t *prev, int8_t *idx )
{
	int index = *idx;
	int step = StepSizeTable[ index ];
	int valpred = *prev;
	
	index += IndexTable[ nibble ];
	if( index < 0 )
		index = 0;
	else if( index > 88 )
		index = 88;
	
	int vpdiff = step >> 3;
	if( nibble & 1 )
		vpdiff += step >> 2;
	if( nibble & 2 )
		vpdiff += step >> 1;
	if( nibble & 4 )
		vpdiff += step;
	
	if( nibble & 8 )
		valpred -= vpdiff;
	else
		valpred += vpdiff;
	
	if( valpred > 32767 )
		valpred = 32767;
	else if( valpred < -32768 )
		valpred = -32768;
	
	*prev = valpred;
	*idx = index;
	return valpred;
}


void IMA::EncodeSegment( int16_t *from, uint8_t *into, size_t samples, int16_t *prev, int8_t *idx )
{
	for( size_t i = 0; i < samples; i += 2 )
	{
		uint8_t encoded = Encode( from[ i ], prev, idx ) << 4;
		
		if( (i + 1) < samples )
			encoded |= Encode( from[ i + 1 ], prev, idx );
		
		into[ i / 2 ] = encoded;
	}
}


void IMA::DecodeSegment( uint8_t *from, int16_t *into, size_t samples, int16_t *prev, int8_t *idx )
{
	for( size_t i = 0; i < samples; i += 2 )
	{
		uint8_t encoded = from[ i / 2 ];
		into[ i ] = Decode( (encoded & 0xF0) << 4, prev, idx );
		
		if( (i + 1) < samples )
			into[ i + 1 ] = Decode( encoded & 0x0F, prev, idx );
	}
}



void IMA::EncodeBuffer( int16_t *from, uint8_t *into, size_t samples )
{
	int16_t prev = 0;
	int8_t index = 0;
	EncodeSegment( from, into, samples, &prev, &index );
}


void IMA::DecodeBuffer( uint8_t *from, int16_t *into, size_t samples )
{
	int16_t prev = 0;
	int8_t index = 0;
	DecodeSegment( from, into, samples, &prev, &index );
}
