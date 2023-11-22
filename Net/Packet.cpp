/*
 *  Packet.cpp
 */

#include "Packet.h"

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>


// Allocate for outgoing data.
Packet::Packet( PacketType packet_type )
{
	AllocationChunkSize = PACKET_OUTGOING_ALLOCATION_CHUNK_SIZE;
	ThrowExceptions = false;
	
	Data = NULL;
	Allocated = 0;
	Offset = 0;
	MakeRoom( PACKET_HEADER_SIZE );
	
	SetSize( PACKET_HEADER_SIZE );
	SetType( packet_type );
	Offset = PACKET_HEADER_SIZE;
}


// Allocate for incoming data.
Packet::Packet( const void *data, int size )
{
	AllocationChunkSize = PACKET_INCOMING_ALLOCATION_CHUNK_SIZE;
	ThrowExceptions = false;
	
	Data = NULL;
	Allocated = 0;
	Offset = 0;
	MakeRoom( PACKET_HEADER_SIZE );
	
	SetSize( PACKET_HEADER_SIZE );
	SetType( PACKET_DEFAULT_TYPE );
	Offset = PACKET_HEADER_SIZE;
	
	SetData( data, size );
}


// Copy an existing packet.
Packet::Packet( const Packet *other )
{
	ThrowExceptions = other->ThrowExceptions;
	AllocationChunkSize = 1;
	Data = NULL;
	Allocated = 0;
	Offset = 0;
	
	Data = (uint8_t *) malloc( other->Allocated );
	if( Data )
	{
		Allocated = other->Allocated;
		memcpy( Data, other->Data, Allocated );
	}
	
	Offset = other->Offset;
	AllocationChunkSize = other->AllocationChunkSize;
}


Packet::~Packet()
{
	if( Allocated )
	{
		SetType( PACKET_DEFAULT_TYPE );
		SetSize( 0 );
		free( Data );
		Data = NULL;
		Allocated = 0;
		Offset = 0;
	}
}


void Packet::SetData( const void *data, PacketSize size )
{
	// If we got the full header, allocate enough room for the packet's total data (even if fragmented).
	if( size >= PACKET_HEADER_SIZE )
	{
		if( ! MakeRoom( FirstPacketSize(data) - PACKET_HEADER_SIZE ) )
			return;
	}
	else if( ! Data )
		return;
	
	// Copy the packet type.
	for( size_t i = 0; (i < sizeof(PacketType)) && (i < size); i ++ )
		Data[ i ] = ((uint8_t *)( data ))[ i ];
	
	// Set up the size; use the received size, so fragmented packets can append the rest later.
	SetSize( size );
	
	// Copy the packet's non-header data.
	for( size_t i = PACKET_HEADER_SIZE; i < size; i ++ )
		Data[ i ] = ((uint8_t *)( data ))[ i ];
	
	// Move the offset to the start of the real data.
	Offset = PACKET_HEADER_SIZE;
}


void Packet::AddData( const void *data, PacketSize size )
{
	// Make sure we have enough room for the addition.
	if( ! MakeRoom(size) )
		return;
	
	// Append additional packet data.
	for( size_t i = 0; i < size; i ++ )
		Data[ Size() + i ] = ((uint8_t *)( data ))[ i ];
	
	// Increase the stored size.
	SetSize( Size() + size );
}


// -----------------------------------------------------------------------------


PacketType Packet::Type( void )
{
	if( Data )
		return PACKET_READ_TYPE( Data );
	return PACKET_DEFAULT_TYPE;
}


void Packet::SetType( PacketType type )
{
	if( Data )
		PACKET_WRITE_TYPE( type, Data );
}


PacketSize Packet::Size( void )
{
	if( Data )
		return PACKET_READ_SIZE( Data + sizeof(PacketType) );
	return 0;
}


void Packet::SetSize( PacketSize size )
{
	if( Data )
		PACKET_WRITE_SIZE( size, Data + sizeof(PacketType) );
}


void Packet::Clear( void )
{
	SetSize( PACKET_HEADER_SIZE );
	Rewind();
}


void Packet::Clear( PacketType packet_type )
{
	Clear();
	SetType( packet_type );
}


void Packet::Rewind( void )
{
	Offset = PACKET_HEADER_SIZE;
}


int Packet::Remaining( void )
{
	return Offset - Size();
}


// -----------------------------------------------------------------------------


bool Packet::MakeRoom( PacketSize addition_size )
{
	// If we've never allocated, do so!
	if( ! Data )
	{
		// Determine how much to allocate.
		size_t add_mem = addition_size;
		if( AllocationChunkSize > 1 )
		{
			add_mem = AllocationChunkSize;
			while( add_mem < addition_size )
				add_mem += AllocationChunkSize;
		}
		
		// Create data allocation for this packet.
		Data = (uint8_t *) malloc( add_mem );
		if( Data )
			Allocated = add_mem;
		else
		{
			Allocated = 0;
			return false;
		}
	}
	
	// If we don't have enough memory yet, allocate more!
	else if( Size() + addition_size > Allocated )
	{
		// Determine how much to add.
		int add_mem = addition_size;
		if( AllocationChunkSize > 1 )
		{
			add_mem = AllocationChunkSize;
			while( Allocated + add_mem < Size() + addition_size )
				add_mem += AllocationChunkSize;
		}
		
		// Increase allocation for this packet.
		uint8_t *new_data = (uint8_t *) realloc( Data, Allocated + add_mem );
		if( new_data )
		{
			Data = new_data;
			Allocated += add_mem;
		}
		else
			return false;
	}
	
	return true;
}


void Packet::AddChar( int8_t addition )
{
	if( ! MakeRoom(1) )
		return;
	Data[ Size() ] = addition;
	SetSize( Size() + 1 );
}


void Packet::AddUChar( uint8_t addition )
{
	if( ! MakeRoom(1) )
		return;
	Data[ Size() ] = addition;
	SetSize( Size() + 1 );
}


void Packet::AddShort( int16_t addition )
{
	if( ! MakeRoom(2) )
		return;
	// Store as network-endian.
	Endian::WriteBig16( addition, Data + Size() );
	SetSize( Size() + 2 );
}


void Packet::AddUShort( uint16_t addition )
{
	if( ! MakeRoom(2) )
		return;
	// Store as network-endian.
	Endian::WriteBig16( addition, Data + Size() );
	SetSize( Size() + 2 );
}


void Packet::AddInt( int32_t addition )
{
	if( ! MakeRoom(4) )
		return;
	// Store as network-endian.
	Endian::WriteBig32( addition, Data + Size() );
	SetSize( Size() + 4 );
}


void Packet::AddUInt( uint32_t addition )
{
	if( ! MakeRoom(4) )
		return;
	// Store as network-endian.
	Endian::WriteBig32( addition, Data + Size() );
	SetSize( Size() + 4 );
}


void Packet::AddInt64( int64_t addition )
{
	if( ! MakeRoom(8) )
		return;
	// Store as network-endian.
	Endian::WriteBig64( addition, Data + Size() );
	SetSize( Size() + 8 );
}


void Packet::AddUInt64( uint64_t addition )
{
	if( ! MakeRoom(8) )
		return;
	// Store as network-endian.
	Endian::WriteBig64( addition, Data + Size() );
	SetSize( Size() + 8 );
}


void Packet::AddFloat( float addition )
{
	if( ! MakeRoom(4) )
		return;
	#ifdef ENDIAN_BIG
		*((float *)( Data + Size() )) = addition;
	#else
		Endian::ByteSwapCopy( &addition, Data + Size(), 4 );
	#endif
	SetSize( Size() + 4 );
}


void Packet::AddDouble( double addition )
{
	if( ! MakeRoom(8) )
		return;
	#ifdef ENDIAN_BIG
		*((double *)( Data + Size() )) = addition;
	#else
		Endian::ByteSwapCopy( &addition, Data + Size(), 8 );
	#endif
	SetSize( Size() + 8 );
}


void Packet::AddString( const char *addition )
{
	if( addition )
	{
		if( ! MakeRoom( (strlen(addition) + 1) * sizeof(char) ) )
			return;
		sprintf( ((char *)( Data + Size() )), "%s", addition );
		SetSize( Size() + (strlen(addition) + 1) * sizeof(char) );
	}
	else
		AddString( "" );
}


void Packet::AddString( const std::string &addition )
{
	AddString( addition.c_str() );
}


// -----------------------------------------------------------------------------


int8_t Packet::NextChar( void )
{
	int8_t value = '\0';
	if( Offset + 1 <= Size() )
	{
		value = Data[ Offset ];
		Offset ++;
	}
	else
	{
		Offset = Size();
		if( ThrowExceptions )
			throw PacketSmall();
	}
	return value;
}


uint8_t Packet::NextUChar( void )
{
	uint8_t value = '\0';
	if( Offset + 1 <= Size() )
	{
		value = Data[ Offset ];
		Offset ++;
	}
	else
	{
		Offset = Size();
		if( ThrowExceptions )
			throw PacketSmall();
	}
	return value;
}


int16_t Packet::NextShort( void )
{
	int16_t value = 0;
	if( Offset + 2 <= Size() )
	{
		// Retrieve from network-endian.
		value = Endian::ReadBig16( Data + Offset );
		Offset += 2;
	}
	else
	{
		Offset = Size();
		if( ThrowExceptions )
			throw PacketSmall();
	}
	return value;
}


uint16_t Packet::NextUShort( void )
{
	uint16_t value = 0;
	if( Offset + 2 <= Size() )
	{
		// Retrieve from network-endian.
		value = Endian::ReadBig16( Data + Offset );
		Offset += 2;
	}
	else
	{
		Offset = Size();
		if( ThrowExceptions )
			throw PacketSmall();
	}
	return value;
}


int32_t Packet::NextInt( void )
{
	int32_t value = 0;
	if( Offset + 4 <= Size() )
	{
		// Retrieve from network-endian.
		value = Endian::ReadBig32( Data + Offset );
		Offset += 4;
	}
	else
	{
		Offset = Size();
		if( ThrowExceptions )
			throw PacketSmall();
	}
	return value;
}


uint32_t Packet::NextUInt( void )
{
	uint32_t value = 0;
	if( Offset + 4 <= Size() )
	{
		// Retrieve from network-endian.
		value = Endian::ReadBig32( Data + Offset );
		Offset += 4;
	}
	else
	{
		Offset = Size();
		if( ThrowExceptions )
			throw PacketSmall();
	}
	return value;
}


int64_t Packet::NextInt64( void )
{
	int64_t value = 0;
	if( Offset + 8 <= Size() )
	{
		// Retrieve from network-endian.
		value = Endian::ReadBig64( Data + Offset );
		Offset += 8;
	}
	else
	{
		Offset = Size();
		if( ThrowExceptions )
			throw PacketSmall();
	}
	return value;
}


uint64_t Packet::NextUInt64( void )
{
	uint64_t value = 0;
	if( Offset + 8 <= Size() )
	{
		// Retrieve from network-endian.
		value = Endian::ReadBig64( Data + Offset );
		Offset += 8;
	}
	else
	{
		Offset = Size();
		if( ThrowExceptions )
			throw PacketSmall();
	}
	return value;
}


float Packet::NextFloat( void )
{
	float value = 0.f;
	if( Offset + 4 <= Size() )
	{
		// Retrieve from network-endian.
		#ifdef ENDIAN_BIG
			value = *((float *)( Data + Offset ));
		#else
			Endian::ByteSwapCopy( Data + Offset, &value, 4 );
		#endif
		Offset += 4;
	}
	else
	{
		Offset = Size();
		if( ThrowExceptions )
			throw PacketSmall();
	}
	return value;
}


double Packet::NextDouble( void )
{
	double value = 0.;
	if( Offset + 8 <= Size() )
	{
		// Retrieve from network-endian.
		#ifdef ENDIAN_BIG
			value = *((double *)( Data + Offset ));
		#else
			Endian::ByteSwapCopy( Data + Offset, &value, 8 );
		#endif
		Offset += 8;
	}
	else
	{
		Offset = Size();
		if( ThrowExceptions )
			throw PacketSmall();
	}
	return value;
}


const char *Packet::NextString( void )
{
	const char *value = "";
	if( Offset < Size() )
	{
		value = ((char *)( Data + Offset ));
		Offset += (strlen(value) + 1) * sizeof(char);
	}
	else
	{
		Offset = Size();
		if( ThrowExceptions )
			throw PacketSmall();
	}
	return value;
}


// -----------------------------------------------------------------------------


PacketSize Packet::FirstPacketSize( const void *data )
{
	return PACKET_READ_SIZE( ((uint8_t*)( data )) + sizeof(PacketType) );
}


// -----------------------------------------------------------------------------


const char *PacketSmall::what() const throw()
{
	return "Packet contains less data than expected.";
}
