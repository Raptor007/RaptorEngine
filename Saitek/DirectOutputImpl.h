#pragma once
#ifdef WIN32

#ifdef _MSC_VER
#pragma comment( lib, "advapi32.lib" )
#endif

class CDirectOutput;

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include "DirectOutput.h"

///
/// CDirectOutput
///
/// Simplifies the LoadLibrary interface to DirectOutput
///
class CDirectOutput
{
public:
	CDirectOutput();
	~CDirectOutput();

	HRESULT Initialize(const wchar_t* wszPluginName);
	HRESULT Deinitialize();
	HRESULT RegisterDeviceCallback(Pfn_DirectOutput_DeviceChange pfnCb, void* pCtxt);
	HRESULT Enumerate(Pfn_DirectOutput_EnumerateCallback pfnCb, void* pCtxt);
	HRESULT RegisterPageCallback(void* hDevice, Pfn_DirectOutput_PageChange pfnCb, void* pCtxt);
	HRESULT RegisterSoftButtonCallback(void* hDevice, Pfn_DirectOutput_SoftButtonChange pfnCb, void* pCtxt);
	HRESULT GetDeviceType(void* hDevice, LPGUID pGuid);
	HRESULT GetDeviceInstance(void* hDevice, LPGUID pGuid);
	HRESULT SetProfile(void* hDevice, DWORD cchProfile, const wchar_t* wszProfile);
	HRESULT AddPage(void* hDevice, DWORD dwPage, const wchar_t* wszDebugName, DWORD dwFlags);
	HRESULT RemovePage(void* hDevice, DWORD dwPage);
	HRESULT SetLed(void* hDevice, DWORD dwPage, DWORD dwIndex, DWORD dwValue);
	HRESULT SetString(void* hDevice, DWORD dwPage, DWORD dwIndex, DWORD cchValue, const wchar_t* wszValue);
	HRESULT SetImage(void* hDevice, DWORD dwPage, DWORD dwIndex, DWORD cbValue, const void* pvValue);
	HRESULT SetImageFromFile(void* hDevice, DWORD dwPage, DWORD dwIndex, DWORD cchFilename, const wchar_t* wszFilename);
	HRESULT StartServer(void* hDevice, DWORD cchFilename, const wchar_t* wszFilename, LPDWORD pdwServerId, PSRequestStatus psStatus);
	HRESULT CloseServer(void* hDevice, DWORD dwServerId, PSRequestStatus psStatus);
	HRESULT SendServerMsg(void* hDevice, DWORD dwServerId, DWORD dwRequest, DWORD dwPage, DWORD cbIn, const void* pvIn, DWORD cbOut, void* pvOut, PSRequestStatus psStatus);
	HRESULT SendServerFile(void* hDevice, DWORD dwServerId, DWORD dwRequest, DWORD dwPage, DWORD cbInHdr, const void* pvInHdr, DWORD cchFile, const wchar_t* wszFile, DWORD cbOut, void* pvOut, PSRequestStatus psStatus);
	HRESULT SaveFile(void* hDevice, DWORD dwPage, DWORD dwFile, DWORD cchFilename, const wchar_t* wszFilename, PSRequestStatus psStatus);
	HRESULT DisplayFile(void* hDevice, DWORD dwPage, DWORD dwIndex, DWORD dwFile, PSRequestStatus psStatus);
	HRESULT DeleteFile(void* hDevice, DWORD dwPage, DWORD dwFile, PSRequestStatus psStatus);
private:
	HMODULE										m_module;

	Pfn_DirectOutput_Initialize					m_initialize;
	Pfn_DirectOutput_Deinitialize				m_deinitialize;
	Pfn_DirectOutput_RegisterDeviceCallback		m_registerdevicecallback;
	Pfn_DirectOutput_Enumerate					m_enumerate;
	Pfn_DirectOutput_RegisterPageCallback		m_registerpagecallback;
	Pfn_DirectOutput_RegisterSoftButtonCallback	m_registersoftbuttoncallback;
	Pfn_DirectOutput_GetDeviceType				m_getdevicetype;
	Pfn_DirectOutput_GetDeviceInstance			m_getdeviceinstance;
	Pfn_DirectOutput_SetProfile					m_setprofile;
	Pfn_DirectOutput_AddPage					m_addpage;
	Pfn_DirectOutput_RemovePage					m_removepage;
	Pfn_DirectOutput_SetLed						m_setled;
	Pfn_DirectOutput_SetString					m_setstring;
	Pfn_DirectOutput_SetImage					m_setimage;
	Pfn_DirectOutput_SetImageFromFile			m_setimagefromfile;
	Pfn_DirectOutput_StartServer				m_startserver;
	Pfn_DirectOutput_CloseServer				m_closeserver;
	Pfn_DirectOutput_SendServerMsg				m_sendservermsg;
	Pfn_DirectOutput_SendServerFile				m_sendserverfile;
	Pfn_DirectOutput_SaveFile					m_savefile;
	Pfn_DirectOutput_DisplayFile				m_displayfile;
	Pfn_DirectOutput_DeleteFile					m_deletefile;

	bool GetDirectOutputFilename(LPTSTR filename, DWORD length);
};

#endif
