#ifdef WIN32

#include "SaitekX52Pro.h"
#include <fstream>
#include "SaitekManager.h"

#ifndef _MSC_VER
#define swprintf_s snwprintf
#endif


SaitekX52Pro::SaitekX52Pro( void *device_handle ) : SaitekDevice( device_handle, DeviceType_X52Pro )
{
	TextRateLimit = 0.25;
	
	memset( Text[ 0 ], 0, 32 );
	memset( Text[ 1 ], 0, 32 );
	memset( Text[ 2 ], 0, 32 );
}

const char *SaitekX52Pro::TypeString( void )
{
	return "X52 Pro";
}

void SaitekX52Pro::Begin( void )
{
	Saitek::Global->DO.AddPage( DeviceHandle, 0, NULL, FLAG_SET_AS_ACTIVE );
	
	for( int led = 0; led < 20; led ++ )
	{
		SetLED( led, true );
		SetLED( led, false );
	}
	
	SetLED( SaitekX52ProLED::AGreen, true );
	SetLED( SaitekX52ProLED::BGreen, true );
	SetLED( SaitekX52ProLED::ClutchGreen, true );
	SetLED( SaitekX52ProLED::DGreen, true );
	SetLED( SaitekX52ProLED::EGreen, true );
	SetLED( SaitekX52ProLED::Fire, true );
	SetLED( SaitekX52ProLED::Hat2Green, true );
	SetLED( SaitekX52ProLED::T1Green, true );
	SetLED( SaitekX52ProLED::T3Green, true );
	SetLED( SaitekX52ProLED::T5Green, true );
	SetLED( SaitekX52ProLED::Throttle, true );
	
	LEDRateLimit = 0.05;
	
	RegisterCallbacks();
}

void SaitekX52Pro::End( void )
{
	for( int page_num = 0; page_num >= 0; page_num -- )
		Saitek::Global->DO.RemovePage( DeviceHandle, page_num );
	
	UnregisterCallbacks();
}

bool SaitekX52Pro::SetMFD( int line, const char *text )
{
	if( (strcmp( Text[line], text ) != 0) && (TextChanged[line].ElapsedSeconds() >= TextRateLimit) )
	{
		TextChanged[line].Reset();
		snprintf( Text[line], 32, "%s", text );
		
		// Convert to UTF-16 string for Saitek MFD.
		wchar_t buffer[ 32 ] = L"";
		memset( buffer, 0, 32*sizeof(wchar_t) );
		swprintf_s( buffer, 32, L"%hs", text );
		
		Saitek::Global->DO.SetString( DeviceHandle, 0, line, wcslen(buffer), buffer );
	}
	
	return true;
}


#endif
