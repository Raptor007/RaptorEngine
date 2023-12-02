/*
 *  PacketBuffer.h
 */

#pragma once
class PacketBuffer;

#include <queue>
#include "Packet.h"
#include "Mutex.h"


class PacketBuffer
{
public:
	size_t MaxPacketSize;
	
	PacketBuffer( void );
	virtual ~PacketBuffer();
	
	void AddData( void *data, PacketSize size );
	void Push( Packet *packet );
	Packet *Pop( void );
	
private:
	std::queue< Packet*, std::list<Packet*> > Complete;
	Mutex Lock;
	Packet *Unfinished;
	PacketSize UnfinishedSizeRemaining, PartialHeaderSize;
	uint8_t PartialHeaderData[ PACKET_HEADER_SIZE ];
};
