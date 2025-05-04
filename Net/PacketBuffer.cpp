/*
 *  PacketBuffer.cpp
 */

#include "PacketBuffer.h"

#include "RaptorDefs.h"
#include <cstddef>
#include <cstring>
#include "Num.h"


PacketBuffer::PacketBuffer( void )
{
	MaxPacketSize = 0x00FFFFFF;
	Unfinished = NULL;
	UnfinishedSizeRemaining = 0;
	PartialHeaderSize = 0;
	memset( PartialHeaderData, 0, sizeof(PartialHeaderData) );
}


PacketBuffer::~PacketBuffer()
{
	Packet *packet = NULL;
	while( (packet = Pop()) )
		delete packet;
	
	if( Unfinished )
		delete Unfinished;
	Unfinished = NULL;
	
	UnfinishedSizeRemaining = 0;
	PartialHeaderSize = 0;
}


void PacketBuffer::AddData( void *data, PacketSize size )
{
	uint8_t *data_unprocessed = (uint8_t*) data;
	size_t size_unprocessed = size;
	
	if( PartialHeaderSize )
	{
		PacketSize add_size = std::min<PacketSize>( size_unprocessed, PACKET_HEADER_SIZE - PartialHeaderSize );
		memcpy( PartialHeaderData + PartialHeaderSize, data_unprocessed, add_size );
		data_unprocessed += add_size;
		size_unprocessed -= add_size;
		PartialHeaderSize += add_size;
		
		if( PartialHeaderSize == PACKET_HEADER_SIZE )
		{
			size_t packet_size = Packet::FirstPacketSize( PartialHeaderData );
			
			// Sanity check the incoming packet size.
			if( (packet_size < PACKET_HEADER_SIZE) || (MaxPacketSize && (packet_size > MaxPacketSize)) )
			{
				PartialHeaderSize = 0;
				memset( PartialHeaderData, 0, sizeof(PartialHeaderData) );
				
				Packet *packet = new Packet( Raptor::Packet::DISCONNECT );
				packet->AddString( std::string("Packet size error: ") + Num::ToString((int)packet_size) );
				Push( packet );
				
				return;
			}
			
			Unfinished = new Packet( PartialHeaderData, PACKET_HEADER_SIZE );
			UnfinishedSizeRemaining = packet_size - PACKET_HEADER_SIZE;
			PartialHeaderSize = 0;
			memset( PartialHeaderData, 0, sizeof(PartialHeaderData) );
		}
	}
	
	if( Unfinished )
	{
		if( size_unprocessed >= UnfinishedSizeRemaining )
		{
			Unfinished->AddData( data_unprocessed, UnfinishedSizeRemaining );
			Push( Unfinished );
			Unfinished = NULL;
			data_unprocessed += UnfinishedSizeRemaining;
			size_unprocessed -= UnfinishedSizeRemaining;
			UnfinishedSizeRemaining = 0;
		}
		else
		{
			Unfinished->AddData( data_unprocessed, size_unprocessed );
			data_unprocessed += size_unprocessed;
			UnfinishedSizeRemaining -= size_unprocessed;
			size_unprocessed = 0;
		}
	}
	
	while( size_unprocessed )
	{
		// Over the internet packets may be arbitrarily combined and split, so we need to handle only part of a header coming through.
		if( size_unprocessed < PACKET_HEADER_SIZE )
		{
			PartialHeaderSize = size_unprocessed;
			memcpy( PartialHeaderData, data_unprocessed, size_unprocessed );
			return;
		}
		
		size_t packet_size = Packet::FirstPacketSize( data_unprocessed );
		
		// Sanity check the incoming packet size.
		if( (packet_size < PACKET_HEADER_SIZE) || (MaxPacketSize && (packet_size > MaxPacketSize)) )
		{
			if( Unfinished )
			{
				delete Unfinished;
				Unfinished = NULL;
			}
			
			Packet *packet = new Packet( Raptor::Packet::DISCONNECT );
			packet->AddString( "Packet size error." );
			Push( packet );
			
			return;
		}
		
		if( size_unprocessed >= packet_size )
		{
			Packet *packet = new Packet( data_unprocessed, packet_size );
			Push( packet );
			data_unprocessed += packet_size;
			size_unprocessed -= packet_size;
		}
		else
		{
			Unfinished = new Packet( data_unprocessed, size_unprocessed );
			UnfinishedSizeRemaining = packet_size - size_unprocessed;
			data_unprocessed += size_unprocessed;
			size_unprocessed = 0;
		}
	}
}


void PacketBuffer::Push( Packet *packet )
{
	Lock.Lock();
	
	Complete.push( packet );
	
	Lock.Unlock();
}


Packet *PacketBuffer::Pop( void )
{
	Packet *packet = NULL;
	
	Lock.Lock();
	
	if( Complete.size() )
	{
		packet = Complete.front();
		Complete.pop();
	}
	
	Lock.Unlock();
	
	return packet;
}
