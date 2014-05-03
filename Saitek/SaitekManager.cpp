#ifdef WIN32

#include "SaitekManager.h"
#include <cstdio>
#include <cstdlib>
#include <map>
#include "SaitekDevice.h"
#include "SaitekX52Pro.h"
#include "SaitekFIP.h"
#include <SDL/SDL.h>


namespace Saitek
{
	SaitekManager *Global = NULL;
}


SaitekManager::SaitekManager( void )
{
}


SaitekManager::~SaitekManager()
{
}


void SaitekManager::Initialize( void )
{
	Saitek::Global = this;
	
	if( ! Initialized )
	{
		HRESULT result = DO.Initialize( L"RaptorEngine" );
		if( SUCCEEDED(result) )
			Initialized = true;
	}
	
	if( Initialized )
	{
		if( ! Devices.size() )
		{
			// Find out what devices are attached.
			DO.Enumerate( &(SaitekManager::EnumerateCallback), NULL );
		}

		// Configure the callback function for Saitek device add/remove.
		DO.RegisterDeviceCallback( &(SaitekManager::SaitekDeviceChange), NULL );
	}
}


void SaitekManager::Deinitialize( void )
{
	if( Initialized )
	{
		DevicesLock.Lock();
		for( std::vector<SaitekDevice*>::iterator device_iter = Devices.begin(); device_iter != Devices.end(); device_iter ++ )
		{
			(*device_iter)->End();
			delete *device_iter;
		}
		Devices.clear();
		DevicesLock.Unlock();
		
		DO.Deinitialize();
		Initialized = false;
	}
}


void SaitekManager::RestartService( void )
{
	Deinitialize();
	system( "net stop SaiDOutput" );
	system( "net start SaiDOutput" );
	Initialize();
}


void __stdcall SaitekManager::EnumerateCallback( void *device_handle, void *ctxt )
{
	SaitekDeviceChange( device_handle, true, ctxt );
}


void __stdcall SaitekManager::SaitekDeviceChange( void *device_handle, bool added, void *ctxt )
{
	// See if the device was already in our list.
	std::vector<SaitekDevice*>::iterator found = Saitek::Global->Devices.end();
	for( std::vector<SaitekDevice*>::iterator device_iter = Saitek::Global->Devices.begin(); device_iter != Saitek::Global->Devices.end(); device_iter ++ )
		if( (*device_iter)->DeviceHandle == device_handle )
		{
			found = device_iter;
			break;
		}
	
	if( added )
	{
		// Just added a DirectOutput device.

		if( found == Saitek::Global->Devices.end() )
		{
			SaitekDevice *dev_inst = NULL;
			GUID guid;
			Saitek::Global->DO.GetDeviceType( device_handle, &guid );
			
			if( guid == DeviceType_X52Pro )
				dev_inst = new SaitekX52Pro( device_handle );
			else if( guid == DeviceType_Fip )
				dev_inst = new SaitekFIP( device_handle );
			else
				dev_inst = new SaitekDevice( device_handle, guid );
			
			Saitek::Global->DevicesLock.Lock();
			Saitek::Global->Devices.push_back( dev_inst );
			dev_inst->Begin();
			Saitek::Global->DevicesLock.Unlock();
		}
	}
	else if( found != Saitek::Global->Devices.end() )
	{
		// Just removed a DirectOutput device.

		Saitek::Global->DevicesLock.Lock();
		delete *found;
		Saitek::Global->Devices.erase( found );
		Saitek::Global->DevicesLock.Unlock();
	}
}


void __stdcall SaitekManager::PageChange( void *device_handle, DWORD page_num, bool active, void *device_ptr )
{
}


void __stdcall SaitekManager::SoftButtonChange( void *device_handle, DWORD buttons, void *device_ptr )
{
	SDL_Event event;
	memset( &event, 0, sizeof(SDL_Event) );
	event.type = SDL_USEREVENT;
	event.user.type = SDL_USEREVENT;
	event.user.code = buttons;
	event.user.data1 = device_ptr;
	event.user.data2 = NULL;
	SDL_PushEvent(&event);
}


int SaitekManager::SetFIPImage( Framebuffer *fb, int fip_num )
{
	int updated = 0;
	
	int num = 0;
	for( std::vector<SaitekDevice*>::iterator device_iter = Devices.begin(); device_iter != Devices.end(); device_iter ++ )
	{
		if( (*device_iter)->Guid == DeviceType_Fip )
		{
			SaitekFIP *fip = (SaitekFIP*) *device_iter;
			
			if( (fip_num == num) || (fip_num < 0) )
			{
				if( fip->SetImage( fb ) )
					updated ++;
			}
			
			num ++;
		}
	}
	
	return updated;
}


int SaitekManager::ClearFIPImage( int fip_num )
{
	int updated = 0;
	
	int num = 0;
	for( std::vector<SaitekDevice*>::iterator device_iter = Devices.begin(); device_iter != Devices.end(); device_iter ++ )
	{
		if( (*device_iter)->Guid == DeviceType_Fip )
		{
			SaitekFIP *fip = (SaitekFIP*) *device_iter;
			
			if( (fip_num == num) || (fip_num < 0) )
			{
				if( fip->ClearImage() )
					updated ++;
			}
			
			num ++;
		}
	}
	
	return updated;
}


int SaitekManager::SetFIPLED( int led, bool value, int fip_num )
{
	int updated = 0;
	
	int num = 0;
	for( std::vector<SaitekDevice*>::iterator device_iter = Devices.begin(); device_iter != Devices.end(); device_iter ++ )
	{
		if( (*device_iter)->Guid == DeviceType_Fip )
		{
			SaitekFIP *fip = (SaitekFIP*) *device_iter;
			
			if( (fip_num == num) || (fip_num < 0) )
			{
				if( fip->SetLED( led, value ) )
					updated ++;
			}
			
			num ++;
		}
	}
	
	return updated;
}


int SaitekManager::SetX52ProLED( int led, bool value, int x52pro_num )
{
	int updated = 0;
	
	int num = 0;
	for( std::vector<SaitekDevice*>::iterator device_iter = Devices.begin(); device_iter != Devices.end(); device_iter ++ )
	{
		if( (*device_iter)->Guid == DeviceType_X52Pro )
		{
			SaitekX52Pro *x52pro = (SaitekX52Pro*) *device_iter;
			
			if( (x52pro_num == num) || (x52pro_num < 0) )
			{
				if( x52pro->SetLED( led, value ) )
					updated ++;
			}
			
			num ++;
		}
	}
	
	return updated;
}


int SaitekManager::SetX52ProMFD( int line, const char *text, int x52pro_num )
{
	int updated = 0;
	
	int num = 0;
	for( std::vector<SaitekDevice*>::iterator device_iter = Devices.begin(); device_iter != Devices.end(); device_iter ++ )
	{
		if( (*device_iter)->Guid == DeviceType_X52Pro )
		{
			SaitekX52Pro *x52pro = (SaitekX52Pro*) *device_iter;
			
			if( (x52pro_num == num) || (x52pro_num < 0) )
			{
				if( x52pro->SetMFD( line, text ) )
					updated ++;
			}
			
			num ++;
		}
	}
	
	return updated;
}


#endif
