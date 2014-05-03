#ifdef WIN32

#include "SaitekDevice.h"
#include <cmath>
#include <fstream>
#include "SaitekManager.h"


SaitekDevice::SaitekDevice( void *device_handle, GUID guid )
{
	Guid = guid;
	DeviceHandle = device_handle;
	LEDRateLimit = 0.;
	memset( LEDs, 0, sizeof(LEDs) );
}

const char *SaitekDevice::TypeString( void )
{
	return "Unknown";
}

std::string SaitekDevice::GuidString( void )
{
	char guid_string[ 128 ] = "";
	sprintf_s( guid_string, 128, "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X", Guid.Data1, Guid.Data2, Guid.Data3, Guid.Data4[ 0 ], Guid.Data4[ 1 ], Guid.Data4[ 2 ], Guid.Data4[ 3 ], Guid.Data4[ 4 ], Guid.Data4[ 5 ], Guid.Data4[ 6 ], Guid.Data4[ 7 ] );
	return std::string( guid_string );
}

void SaitekDevice::Begin( void ){}
void SaitekDevice::End( void ){}

void SaitekDevice::RegisterCallbacks( void )
{
	Saitek::Global->DO.RegisterSoftButtonCallback( DeviceHandle, &(SaitekManager::SoftButtonChange), this );
}

void SaitekDevice::UnregisterCallbacks( void )
{
	Saitek::Global->DO.RegisterSoftButtonCallback( DeviceHandle, NULL, this );
}

bool SaitekDevice::SetLED( int led, bool value )
{
	if( (LEDs[led] != value) && (LEDsChanged[led].ElapsedSeconds() >= LEDRateLimit) )
	{
		LEDsChanged[led].Reset();
		LEDs[led] = value;
		
		Saitek::Global->DO.SetLed( DeviceHandle, 0, led, value );
	}
	
	return true;
}


#endif
