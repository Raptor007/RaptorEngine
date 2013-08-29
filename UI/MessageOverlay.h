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
	
	MessageOverlay( Font *message_font );
	virtual ~MessageOverlay();
	
	void Initialize( void );
	void UpdateRects( void );
	
	void Draw( void );
};
