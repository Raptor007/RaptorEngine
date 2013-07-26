/*
 *  Packet.h
 */

#pragma once
class Packet;

#include "PlatformSpecific.h"

#include <list>
#include <exception>
#include <string>
#include "Endian.h"

#define PACKET_OUTGOING_ALLOCATION_CHUNK_SIZE  1024
#define PACKET_INCOMING_ALLOCATION_CHUNK_SIZE  1
#define PACKET_BUFFER_SIZE                     65536

typedef uint32_t PacketType;
typedef uint32_t PacketSize;
#define PACKET_READ_TYPE    Endian::ReadBig32
#define PACKET_WRITE_TYPE   Endian::WriteBig32
#define PACKET_READ_SIZE    Endian::ReadBig32
#define PACKET_WRITE_SIZE   Endian::WriteBig32
#define PACKET_HEADER_SIZE  (sizeof(PacketType) + sizeof(PacketSize))
#define PACKET_DEFAULT_TYPE '    '


class Packet
{
public:
	uint8_t *Data;
	PacketSize Allocated;
	PacketSize Offset;
	
	int AllocationChunkSize;
	bool ThrowExceptions;
	
	
	Packet( PacketType packet_type );
	Packet( const void *data, int size );
	Packet( const Packet *other );
	~Packet();
	
	void SetData( const void *data, PacketSize size );
	void AddData( const void *data, PacketSize size );
	
	PacketType Type( void );
	void SetType( PacketType type );
	PacketSize Size( void );
	void SetSize( PacketSize size );
	void Clear( void );
	void Clear( PacketType packet_type );
	void Rewind( void );
	int Remaining( void );
	
	bool MakeRoom( PacketSize addition_size );
	void AddChar( int8_t addition );
	void AddUChar( uint8_t addition );
	void AddShort( int16_t addition );
	void AddUShort( uint16_t addition );
	void AddInt( int32_t addition );
	void AddUInt( uint32_t addition );
	void AddInt64( int64_t addition );
	void AddUInt64( uint64_t addition );
	void AddFloat( float addition );
	void AddDouble( double addition );
	void AddString( const char *addition );
	void AddString( std::string &addition );
	
	int8_t NextChar( void );
	uint8_t NextUChar( void );
	int16_t NextShort( void );
	uint16_t NextUShort( void );
	int32_t NextInt( void );
	uint32_t NextUInt( void );
	int64_t NextInt64( void );
	uint64_t NextUInt64( void );
	float NextFloat( void );
	double NextDouble( void );
	char *NextString( void );
	
	static PacketSize FirstPacketSize( const void *data );
};


class PacketSmall : public std::exception
{
	const char *what() const throw();
};
