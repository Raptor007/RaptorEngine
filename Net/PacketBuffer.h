/*
 *  PacketBuffer.h
 */

#pragma once
class PacketBuffer;

#include <queue>
#include "Packet.h"


class PacketBuffer
{
public:
	PacketBuffer( void );
	virtual ~PacketBuffer();
	
	void AddData( void *data, PacketSize size );
	Packet *Pop( void );
	
private:
	std::queue< Packet*, std::list<Packet*> > Complete;
	Packet *Unfinished;
	PacketSize UnfinishedSizeRemaining;
};
