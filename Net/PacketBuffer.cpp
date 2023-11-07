/*
 *  PacketBuffer.cpp
 */

#include "PacketBuffer.h"

#include "RaptorDefs.h"
#include <cstddef>


PacketBuffer::PacketBuffer( void )
{
	MaxPacketSize = 0x00FFFFFF;
	Unfinished = NULL;
	UnfinishedSizeRemaining = 0;
}


PacketBuffer::~PacketBuffer()
{
	Packet *packet = NULL;
	while( (packet = Pop()) )
		delete packet;
	
	if( Unfinished )
		delete Unfinished;
	Unfinished = NULL;
}


void PacketBuffer::AddData( void *data, PacketSize size )
{
	uint8_t *data_unprocessed = (uint8_t*) data;
	size_t size_unprocessed = size;
	
	if( Unfinished )
	{
		if( size_unprocessed >= UnfinishedSizeRemaining )
		{
			Unfinished->AddData( data_unprocessed, UnfinishedSizeRemaining );
			Complete.push( Unfinished );
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
		size_t packet_size = 0;
		if( size_unprocessed >= PACKET_HEADER_SIZE )
		{
			packet_size = Packet::FirstPacketSize( data_unprocessed );
			
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
				Complete.push( packet );
				return;
			}
		}
		else
		{
			// FIXME: What if a partial packet doesn't include the entire header?
		}
		
		if( (size_unprocessed >= packet_size) && (size_unprocessed >= PACKET_HEADER_SIZE) )
		{
			Packet *packet = new Packet( data_unprocessed, packet_size );
			Complete.push( packet );
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


Packet *PacketBuffer::Pop( void )
{
	if( ! Complete.size() )
		return NULL;
	
	Packet *packet = Complete.front();
	Complete.pop();
	return packet;
}
