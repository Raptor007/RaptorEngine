#ifdef WIN32

#include "SaitekFIP.h"
#include <cstdlib>
#include <cmath>
#include "RaptorGL.h"
#include "SaitekManager.h"


SaitekFIP::SaitekFIP( void *device_handle ) : SaitekDevice( device_handle, DeviceType_Fip )
{
	Thread = NULL;
	Running = false;
	Drawing = false;
	memset( Buffer, 0, 320*240*3 );
}

const char *SaitekFIP::TypeString( void )
{
	return "FIP";
}

void SaitekFIP::Begin( void )
{
	Saitek::Global->DO.AddPage( DeviceHandle, 0, NULL, FLAG_SET_AS_ACTIVE );
	Saitek::Global->DO.SetImage( DeviceHandle, 0, 0, 0, NULL );
	
	for( int led = 0; led < 7; led ++ )
	{
		SetLED( led, true );
		SetLED( led, false );
	}
	
	LEDRateLimit = 0.05;
	
	Running = true;
	Thread = SDL_CreateThread( UpdateThread, this );
	
	RegisterCallbacks();
}

void SaitekFIP::End( void )
{
	Running = false;
	if( Thread )
		SDL_WaitThread( Thread, NULL );
	Thread = NULL;
	
	Saitek::Global->DO.SetImage( DeviceHandle, 0, 0, 0, NULL );
	Saitek::Global->DO.RemovePage( DeviceHandle, 0 );
	
	UnregisterCallbacks();
}

bool SaitekFIP::SetImage( Framebuffer *fb )
{
	if( ! Drawing )
	{
		// Read the bottom-left 320x240 of the framebuffer.
		if( fb )
			fb->Select();
		glReadPixels( 0, 0, 320, 240, GL_BGR, GL_UNSIGNED_BYTE, Buffer );
		
		// Let the thread update the device so we don't lag the game.
		Drawing = true;
		return true;
	}
	
	return false;
}

bool SaitekFIP::ClearImage( void )
{
	if( ! Drawing )
	{
		// Set the buffer to black.
		memset( Buffer, 0, 320*240*3 );
		
		// Let the thread update the device so we don't lag the game.
		Drawing = true;
		return true;
	}
	
	return false;
}


int SaitekFIP::UpdateThread( void *saitek_fip )
{
	SaitekFIP *fip = (SaitekFIP*) saitek_fip;
	
	while( fip->Running )
	{
		if( fip->Drawing )
		{
			Saitek::Global->DO.SetImage( fip->DeviceHandle, 0, 0, 320*240*3, fip->Buffer );
			fip->Drawing = false;
		}
		
		// Let the thread rest a bit.
		SDL_Delay( 1 );
	}
	
	return 0;
}


#endif
