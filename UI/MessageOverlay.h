/*
 *  MessageOverlay.h
 */

#pragma once

#include "PlatformSpecific.h"
class MessageOverlay;

#include "Layer.h"
#include "Font.h"


class MessageOverlay : public Layer
{
public:
	Font *MessageFont;
	double MessageLifetime;
	int MaxMessages;
	double ScrollTime;
	std::set<uint32_t> DoNotDraw;
	
	MessageOverlay( Font *message_font );
	virtual ~MessageOverlay();
	
	bool DrawsType( uint32_t type ) const;
	void SetTypeToDraw( uint32_t type, bool draw );
	
	void UpdateRects( void );
	void Draw( void );
};
