#ifdef WIN32

#include "DirectOutputImpl.h"

CDirectOutput::CDirectOutput() : 
	m_module(NULL),
	m_initialize(0), m_deinitialize(0), 
	m_registerdevicecallback(0), m_enumerate(0), 
	m_registerpagecallback(0), m_registersoftbuttoncallback(0), 
	m_getdevicetype(0), m_getdeviceinstance(0), 
	m_setprofile(0), m_addpage(0), m_removepage(0),
	m_setled(0), m_setstring(0), m_setimage(0), m_setimagefromfile(0), 
	m_startserver(0), m_closeserver(0), m_sendservermsg(0), m_sendserverfile(0), 
	m_savefile(0), m_displayfile(0), m_deletefile(0)
{
	TCHAR filename[2048] = { 0 };
	if (GetDirectOutputFilename(filename, sizeof(filename)/sizeof(filename[0])))
	{
		m_module = LoadLibrary(filename);
		if (m_module)
		{
			m_initialize =					(Pfn_DirectOutput_Initialize)					GetProcAddress(m_module, "DirectOutput_Initialize");
			m_deinitialize =				(Pfn_DirectOutput_Deinitialize)					GetProcAddress(m_module, "DirectOutput_Deinitialize");
			m_registerdevicecallback =		(Pfn_DirectOutput_RegisterDeviceCallback)		GetProcAddress(m_module, "DirectOutput_RegisterDeviceCallback");
			m_enumerate =					(Pfn_DirectOutput_Enumerate)					GetProcAddress(m_module, "DirectOutput_Enumerate");
			m_registerpagecallback =		(Pfn_DirectOutput_RegisterPageCallback)			GetProcAddress(m_module, "DirectOutput_RegisterPageCallback");
			m_registersoftbuttoncallback =	(Pfn_DirectOutput_RegisterSoftButtonCallback)	GetProcAddress(m_module, "DirectOutput_RegisterSoftButtonCallback");
			m_getdevicetype =				(Pfn_DirectOutput_GetDeviceType)				GetProcAddress(m_module, "DirectOutput_GetDeviceType");
			m_getdeviceinstance =			(Pfn_DirectOutput_GetDeviceInstance)			GetProcAddress(m_module, "DirectOutput_GetDeviceInstance");
			m_setprofile =					(Pfn_DirectOutput_SetProfile)					GetProcAddress(m_module, "DirectOutput_SetProfile");
			m_addpage =						(Pfn_DirectOutput_AddPage)						GetProcAddress(m_module, "DirectOutput_AddPage");
			m_removepage =					(Pfn_DirectOutput_RemovePage)					GetProcAddress(m_module, "DirectOutput_RemovePage");
			m_setled =						(Pfn_DirectOutput_SetLed)						GetProcAddress(m_module, "DirectOutput_SetLed");
			m_setstring =					(Pfn_DirectOutput_SetString)					GetProcAddress(m_module, "DirectOutput_SetString");
			m_setimage =					(Pfn_DirectOutput_SetImage)						GetProcAddress(m_module, "DirectOutput_SetImage");
			m_setimagefromfile =			(Pfn_DirectOutput_SetImageFromFile)				GetProcAddress(m_module, "DirectOutput_SetImageFromFile");
			m_startserver =					(Pfn_DirectOutput_StartServer)					GetProcAddress(m_module, "DirectOutput_StartServer");
			m_closeserver =					(Pfn_DirectOutput_CloseServer)					GetProcAddress(m_module, "DirectOutput_CloseServer");
			m_sendservermsg =				(Pfn_DirectOutput_SendServerMsg)				GetProcAddress(m_module, "DirectOutput_SendServerMsg");
			m_sendserverfile =				(Pfn_DirectOutput_SendServerFile)				GetProcAddress(m_module, "DirectOutput_SendServerFile");
			m_savefile =					(Pfn_DirectOutput_SaveFile)						GetProcAddress(m_module, "DirectOutput_SaveFile");
			m_displayfile =					(Pfn_DirectOutput_DisplayFile)					GetProcAddress(m_module, "DirectOutput_DisplayFile");
			m_deletefile =					(Pfn_DirectOutput_DeleteFile)					GetProcAddress(m_module, "DirectOutput_DeleteFile");
		}
	}
}
CDirectOutput::~CDirectOutput()
{
	if (m_module)
	{
		FreeLibrary(m_module);
	}
}
HRESULT CDirectOutput::Initialize(const wchar_t* wszPluginName)
{
	if (m_module && m_initialize) 
	{
		return m_initialize(wszPluginName);
	}
	return E_NOTIMPL;
}
HRESULT CDirectOutput::Deinitialize()
{
	if (m_module && m_deinitialize) 
	{
		return m_deinitialize();
	}
	return E_NOTIMPL;
}
HRESULT CDirectOutput::RegisterDeviceCallback(Pfn_DirectOutput_DeviceChange pfnCb, void* pCtxt)
{
	if (m_module && m_registerdevicecallback) 
	{
		return m_registerdevicecallback(pfnCb, pCtxt);
	}
	return E_NOTIMPL;
}
//HRESULT CDirectOutput::Enumerate()
//{
//	if (m_module && m_enumerate) 
//	{
//		return m_enumerate();
//	}
//	return E_NOTIMPL;
//}
HRESULT CDirectOutput::Enumerate(Pfn_DirectOutput_EnumerateCallback pfnCb, void* pCtxt)
{
	if (m_module && m_enumerate)
	{
		return m_enumerate(pfnCb, pCtxt);
	}
	return E_NOTIMPL;
}
HRESULT CDirectOutput::RegisterPageCallback(void* hDevice, Pfn_DirectOutput_PageChange pfnCb, void* pCtxt)
{
	if (m_module && m_registerpagecallback) 
	{
		return m_registerpagecallback(hDevice, pfnCb, pCtxt);
	}
	return E_NOTIMPL;
}
HRESULT CDirectOutput::RegisterSoftButtonCallback(void* hDevice, Pfn_DirectOutput_SoftButtonChange pfnCb, void* pCtxt)
{
	if (m_module && m_registersoftbuttoncallback) 
	{
		return m_registersoftbuttoncallback(hDevice, pfnCb, pCtxt);
	}
	return E_NOTIMPL;
}
HRESULT CDirectOutput::GetDeviceType(void* hDevice, LPGUID pGuid)
{
	if (m_module && m_getdevicetype) 
	{
		return m_getdevicetype(hDevice, pGuid);
	}
	return E_NOTIMPL;
}
HRESULT CDirectOutput::GetDeviceInstance(void* hDevice, LPGUID pGuid)
{
	if (m_module && m_getdeviceinstance) 
	{
		return m_getdeviceinstance(hDevice, pGuid);
	}
	return E_NOTIMPL;
}
HRESULT CDirectOutput::SetProfile(void* hDevice, DWORD cchProfile, const wchar_t* wszProfile)
{
	if (m_module && m_setprofile) 
	{
		return m_setprofile(hDevice, cchProfile, wszProfile);
	}
	return E_NOTIMPL;
}
HRESULT CDirectOutput::AddPage(void* hDevice, DWORD dwPage, const wchar_t* wszDebugName, DWORD dwFlags)
{
	if (m_module && m_addpage) 
	{
		return m_addpage(hDevice, dwPage, dwFlags);
	}
	return E_NOTIMPL;
}
HRESULT CDirectOutput::RemovePage(void* hDevice, DWORD dwPage)
{
	if (m_module && m_removepage) 
	{
		return m_removepage(hDevice, dwPage);
	}
	return E_NOTIMPL;
}
HRESULT CDirectOutput::SetLed(void* hDevice, DWORD dwPage, DWORD dwIndex, DWORD dwValue)
{
	if (m_module && m_setled) 
	{
		return m_setled(hDevice, dwPage, dwIndex, dwValue);
	}
	return E_NOTIMPL;
}
HRESULT CDirectOutput::SetString(void* hDevice, DWORD dwPage, DWORD dwIndex, DWORD cchValue, const wchar_t* wszValue)
{
	if (m_module && m_setstring) 
	{
		return m_setstring(hDevice, dwPage, dwIndex, cchValue, wszValue);
	}
	return E_NOTIMPL;
}
HRESULT CDirectOutput::SetImage(void* hDevice, DWORD dwPage, DWORD dwIndex, DWORD cbValue, const void* pvValue)
{
	if (m_module && m_setimage) 
	{
		return m_setimage(hDevice, dwPage, dwIndex, cbValue, pvValue);
	}
	return E_NOTIMPL;
}
HRESULT CDirectOutput::SetImageFromFile(void* hDevice, DWORD dwPage, DWORD dwIndex, DWORD cchFilename, const wchar_t* wszFilename)
{
	if (m_module && m_setimagefromfile) 
	{
		return m_setimagefromfile(hDevice, dwPage, dwIndex, cchFilename, wszFilename);
	}
	return E_NOTIMPL;
}
HRESULT CDirectOutput::StartServer(void* hDevice, DWORD cchFilename, const wchar_t* wszFilename, LPDWORD pdwServerId, PSRequestStatus psStatus)
{
	if (m_module && m_startserver) 
	{
		return m_startserver(hDevice, cchFilename, wszFilename, pdwServerId, psStatus);
	}
	return E_NOTIMPL;
}
HRESULT CDirectOutput::CloseServer(void* hDevice, DWORD dwServerId, PSRequestStatus psStatus)
{
	if (m_module && m_closeserver) 
	{
		return m_closeserver(hDevice, dwServerId, psStatus);
	}
	return E_NOTIMPL;
}
HRESULT CDirectOutput::SendServerMsg(void* hDevice, DWORD dwServerId, DWORD dwRequest, DWORD dwPage, DWORD cbIn, const void* pvIn, DWORD cbOut, void* pvOut, PSRequestStatus psStatus)
{
	if (m_module && m_sendservermsg) 
	{
		return m_sendservermsg(hDevice, dwServerId, dwRequest, dwPage, cbIn, pvIn, cbOut, pvOut, psStatus);
	}
	return E_NOTIMPL;
}
HRESULT CDirectOutput::SendServerFile(void* hDevice, DWORD dwServerId, DWORD dwRequest, DWORD dwPage, DWORD cbInHdr, const void* pvInHdr, DWORD cchFile, const wchar_t* wszFile, DWORD cbOut, void* pvOut, PSRequestStatus psStatus)
{
	if (m_module && m_sendserverfile) 
	{
		return m_sendserverfile(hDevice, dwServerId, dwRequest, dwPage, cbInHdr, pvInHdr, cchFile, wszFile, cbOut, pvOut, psStatus);
	}
	return E_NOTIMPL;
}
HRESULT CDirectOutput::SaveFile(void* hDevice, DWORD dwPage, DWORD dwFile, DWORD cchFilename, const wchar_t* wszFilename, PSRequestStatus psStatus)
{
	if (m_module && m_savefile) 
	{
		return m_savefile(hDevice, dwPage, dwFile, cchFilename, wszFilename, psStatus);
	}
	return E_NOTIMPL;
}
HRESULT CDirectOutput::DisplayFile(void* hDevice, DWORD dwPage, DWORD dwIndex, DWORD dwFile, PSRequestStatus psStatus)
{
	if (m_module && m_displayfile) 
	{
		return m_displayfile(hDevice, dwPage, dwIndex, dwFile, psStatus);
	}
	return E_NOTIMPL;
}
HRESULT CDirectOutput::DeleteFile(void* hDevice, DWORD dwPage, DWORD dwFile, PSRequestStatus psStatus)
{
	if (m_module && m_deletefile) 
	{
		return m_deletefile(hDevice, dwPage, dwFile, psStatus);
	}
	return E_NOTIMPL;
}
bool CDirectOutput::GetDirectOutputFilename(LPTSTR filename, DWORD length)
{
	bool retval(false);
	HKEY hk;
	// Read the Full Path to DirectOutput.dll from the registry
	long lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Saitek\\DirectOutput"), 0, KEY_READ, &hk);
	if (ERROR_SUCCESS == lRet)
	{
		DWORD size( length * sizeof(filename[0]) );
		// Note: this DirectOutput entry will point to the correct version on x86 or x64 systems
		lRet = RegQueryValueEx(hk, _T("DirectOutput"), 0, 0, (LPBYTE)filename, &size);
		if (ERROR_SUCCESS == lRet)
		{
			retval = true;
		}
		RegCloseKey(hk);
	}
	return retval;
}

#endif
