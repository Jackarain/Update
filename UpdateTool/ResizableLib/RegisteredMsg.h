#pragma once

///////////////////////////////////////////////////////////////////////////////
// Registered Messages
#define DECLARE_REGISTERED_MESSAGE(message) \
static UINT GetRegistered##message() \
{ \
	static UINT uRegisterd##message = 0; \
	if (uRegisterd##message == 0) { \
		CComCritSecLock<CComCriticalSection> lock(_pAtlModule->m_csStaticDataInitAndTypeInfo, false);\
		HRESULT hRes = lock.Lock();\
		uRegisterd##message = ::RegisterWindowMessage(_T(#message)); \
	} \
	ATLASSERT(uRegisterd##message != 0); \
	return uRegisterd##message; \
}

#define GET_REGISTERED_MESSAGE(message) GetRegistered##message()
///////////////////////////////////////////////////////////////////////////////
