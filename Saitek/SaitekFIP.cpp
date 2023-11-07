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
	IntermediateSize = 0;
	IntermediateBuffer = NULL;
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
	
	#if SDL_VERSION_ATLEAST(2,0,0)
		Thread = SDL_CreateThread( UpdateThread, "SaitekFIP", this );
	#else
		Thread = SDL_CreateThread( UpdateThread, this );
	#endif
	
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
		int w = 320, h = 240;
		
		// If a framebuffer was passed, select it for copying.
		if( fb )
		{
			if( fb->Select() )
			{
				w = fb->AllocW;
				h = fb->AllocH;
			}
			else
			{
				w = 0;
				h = 0;
			}
		}
		
		if( w && h )
		{
			// If the source is 320x240, don't use the intermediate buffer.
			if( (w == 320) && (h = 240) )
				glReadPixels( 0, 0, 320, 240, GL_BGR, GL_UNSIGNED_BYTE, Buffer );
			else if( w && h )
			{
				// Make sure the intermediate buffer exists and is the right size.
				if( ((size_t) w ) * ((size_t) h ) * 3 != IntermediateSize )
				{
					IntermediateSize = w * h * 3;
					free( IntermediateBuffer );
					IntermediateBuffer = (unsigned char*) malloc( IntermediateSize );
				}
				
				if( IntermediateBuffer )
				{
					// Copy to the intermediate buffer and resample to 320x240.
					glReadPixels( 0, 0, w, h, GL_BGR, GL_UNSIGNED_BYTE, IntermediateBuffer );
					double x_scale = w / 320., y_scale = h / 240.;
					for( int x = 0; x < 320; x ++ )
						for( int y = 0; y < 240; y ++ )
						{
							size_t dst = y * 320 + x;
							size_t src = (size_t)(y * y_scale) * w + (size_t)(x * x_scale);
							Buffer[ 3 * dst     ] = IntermediateBuffer[ 3 * src     ];
							Buffer[ 3 * dst + 1 ] = IntermediateBuffer[ 3 * src + 1 ];
							Buffer[ 3 * dst + 2 ] = IntermediateBuffer[ 3 * src + 2 ];
						}
				}
			}
			
			// Let the thread update the device so we don't lag the game.
			Drawing = true;
			return true;
		}
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
