#pragma once
#ifdef WIN32

class SaitekFIP;

#include <string>
#include <vector>
#include "SaitekDevice.h"
#include "Framebuffer.h"

#ifdef SDL2
	#include <SDL2/SDL.h>
	#include <SDL2/SDL_thread.h>
#else
	#include <SDL/SDL.h>
	#include <SDL/SDL_thread.h>
#endif


class SaitekFIP : public SaitekDevice
{
public:
	size_t IntermediateSize;
	unsigned char *IntermediateBuffer, Buffer[ 320*240*3 ];
	volatile bool Running, Drawing;
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
