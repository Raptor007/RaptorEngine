#pragma once
#ifdef WIN32

class SaitekFIP;

#include <string>
#include <vector>
#include "SaitekDevice.h"
#include <SDL/SDL_thread.h>
#include "Framebuffer.h"


class SaitekFIP : public SaitekDevice
{
public:
	size_t IntermediateSize;
	unsigned char *IntermediateBuffer, Buffer[ 320*240*3 ];
	bool Running, Drawing;
	SDL_Thread *Thread;
	
	SaitekFIP( void *device_handle );
	
	const char *TypeString( void );
	
	void Begin( void );
	void End( void );
	
	bool SetImage( Framebuffer *fb = NULL );
	bool ClearImage( void );
	
	static int UpdateThread( void *saitek_fip );
};

#endif
