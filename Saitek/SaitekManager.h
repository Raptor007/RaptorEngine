#pragma once
#ifdef WIN32

class SaitekManager;

#include <vector>
#include "DirectOutputImpl.h"
#include "SaitekDevice.h"
#include "Mutex.h"
#include "Framebuffer.h"

namespace Saitek
{
	extern SaitekManager *Global;
}

class SaitekManager
{
public:
	CDirectOutput DO;
	bool Initialized;
	
	std::vector<SaitekDevice*> Devices;
	Mutex DevicesLock;
	
	SaitekManager( void );
	~SaitekManager();
	
	void Initialize( void );
	void Deinitialize( void );
	void RestartService( void );
	
	static bool IsEqualGUID( const GUID guid1, const GUID guid2 );
	static void __stdcall EnumerateCallback( void *device_handle, void *ctxt );
	static void __stdcall SaitekDeviceChange( void *device_handle, bool added, void *ctxt );
	static void __stdcall PageChange( void *device_handle, DWORD page_num, bool active, void *device_ptr );
	static void __stdcall SoftButtonChange( void *device_handle, DWORD buttons, void *device_ptr );
	
	int SetFIPImage( Framebuffer *fb = NULL, int fip_num = -1 );
	int ClearFIPImage( int fip_num = -1 );
	int SetFIPLED( int led, bool value, int fip_num = -1 );
	int SetX52ProLED( int led, bool value, int x52pro_num = -1 );
	int SetX52ProMFD( int line, const char *text, int x52pro_num = -1 );
};

#endif
