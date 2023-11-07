#pragma once
#ifdef WIN32

class SaitekDevice;

#include <string>
#include <vector>
#include "Clock.h"
#include "DirectOutputImpl.h"


class SaitekDevice
{
public:
	GUID Guid;
	void *DeviceHandle;
	bool LEDs[ 20 ];
	Clock LEDsChanged[ 20 ];
	double LEDRateLimit;
	
	SaitekDevice( void *device_handle, GUID guid );
	virtual ~SaitekDevice();
	
	virtual const char *TypeString( void );
	std::string GuidString( void );
	
	virtual void Begin( void );
	virtual void End( void );
	
	void RegisterCallbacks( void );
	void UnregisterCallbacks( void );
	
	bool SetLED( int led, bool value );
};


#endif
