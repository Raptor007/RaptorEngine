/*
 *  RaptorDefs.h
 */

#pragma once

#include "PlatformSpecific.h"


namespace Raptor
{
	namespace State
	{
		enum
		{
			DISCONNECTED = 0,
			CONNECTING,
			CONNECTED,
			GAME_SPECIFIC
		};
	}
	
	namespace Packet
	{
		enum
		{
			PADDING = '    ',
			
			LOGIN = 'Helo',
			DISCONNECT = 'Gbye',
			RECONNECT = 'Rejn',
			
			INFO = 'Info',
			CHANGE_STATE = 'Mode',
			
			UPDATE = 'Updt',
			
			OBJECTS_ADD = 'Obj+',
			OBJECTS_REMOVE = 'Obj-',
			OBJECTS_CLEAR = 'ObjC',
			
			PLAYER_LIST = 'Plrs',
			PLAYER_ADD = 'Plr+',
			PLAYER_REMOVE = 'Plr-',
			PLAYER_PROPERTIES = 'PlrP',
			
			PING = 'Ping',
			PONG = 'Pong',
			RESYNC = 'Sync',
			
			MESSAGE = 'Note',
			PLAY_SOUND = 'PSnd',
			PLAY_MUSIC = 'PMus'
		};
	}
}
