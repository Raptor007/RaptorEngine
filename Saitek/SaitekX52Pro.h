#pragma once
#ifdef WIN32

class SaitekX52Pro;

#include "SaitekDevice.h"
#include <vector>
#include "Clock.h"


namespace SaitekX52ProLED
{
	enum
	{
		Fire = 0,
		ARed,
		AGreen,
		BRed,
		BGreen,
		DRed,
		DGreen,
		ERed,
		EGreen,
		T1Red,
		T1Green,
		T3Red,
		T3Green,
		T5Red,
		T5Green,
		Hat2Red,
		Hat2Green,
		ClutchRed,
		ClutchGreen,
		Throttle
	};
}


class SaitekX52Pro : public SaitekDevice
{
public:
	char Text[ 3 ][ 32 ];
	Clock TextChanged[ 3 ];
	double TextRateLimit;
	
	SaitekX52Pro( void *device_handle );
	
	const char *TypeString( void );
	
	void Begin( void );
	void End( void );
	
	bool SetMFD( int line, const char *text );
};


#endif
