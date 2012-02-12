/////////////////////////////////////////////////////////////////////////////
//
// This file is part of ResizableLib
// http://sourceforge.net/projects/resizablelib
//
// Copyright (C) 2000-2004 by Paolo Messina
// http://www.geocities.com/ppescher - mailto:ppescher@hotmail.com
//
// The contents of this file are subject to the Artistic License (the "License").
// You may not use this file except in compliance with the License. 
// You may obtain a copy of the License at:
// http://www.opensource.org/licenses/artistic-license.html
//
// If you find this code useful, credits would be nice!
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// WTL ResizingLib implementation
// Copyright (C) 2002-2005 by Alexander D. Alexeev
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include <Windows.h>
// DLL Version support
#include <shlwapi.h>

// global variables that hold actual version numbers
// retrieved and adapted at run-time to be equivalent
// to preprocessor macros that set the target platform

__declspec(selectany) DWORD realWINVER;
__declspec(selectany) DWORD real_WIN32_WINDOWS;
__declspec(selectany) DWORD real_WIN32_WINNT;
__declspec(selectany) DWORD real_WIN32_IE;


void InitRealVersions();
DWORD GetRealWINVER();
DWORD Get_real_WIN32_WINDOWS();
DWORD Get_real_WIN32_WINNT();
DWORD Get_real_WIN32_IE();


struct _VersionInitializer
{
	_VersionInitializer()
	{
		InitRealVersions();
	};
};

// The one and only version-check object
__declspec(selectany) _VersionInitializer g_version;

// called automatically by a static initializer
// (if not appropriate can be called later)
// to setup global version numbers

__declspec(selectany) DLLVERSIONINFO g_dviCommCtrls;
__declspec(selectany) OSVERSIONINFOEX g_osviWindows;

inline void CheckOsVersion()
{
	// Try calling GetVersionEx using the OSVERSIONINFOEX structure.
	ZeroMemory(&g_osviWindows, sizeof(OSVERSIONINFOEX));
	g_osviWindows.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if (GetVersionEx((LPOSVERSIONINFO)&g_osviWindows))
		return;
	
	// If that fails, try using the OSVERSIONINFO structure.
	g_osviWindows.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
	if (GetVersionEx((LPOSVERSIONINFO)&g_osviWindows))
		return;

	// When all the above fails, set values for the worst case
	g_osviWindows.dwMajorVersion = 4;
	g_osviWindows.dwMinorVersion = 0;
	g_osviWindows.dwBuildNumber = 0;
	g_osviWindows.dwPlatformId = VER_PLATFORM_WIN32_WINDOWS;
	g_osviWindows.szCSDVersion[0] = TEXT('\0');
}

inline void CheckCommCtrlsVersion()
{
	// Check Common Controls version
	ZeroMemory(&g_dviCommCtrls, sizeof(DLLVERSIONINFO));
	HMODULE hMod = ::LoadLibrary(TEXT("comctl32.dll"));
	if (hMod != NULL)
	{
		// Get the version function
		DLLGETVERSIONPROC pfnDllGetVersion;
		pfnDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hMod, "DllGetVersion");

		if (pfnDllGetVersion != NULL)
		{
			// Obtain version information
			g_dviCommCtrls.cbSize = sizeof(DLLVERSIONINFO);
			if (SUCCEEDED(pfnDllGetVersion(&g_dviCommCtrls)))
			{
				::FreeLibrary(hMod);
				return;
			}
		}

		::FreeLibrary(hMod);
	}

	// Set values for the worst case
	g_dviCommCtrls.dwMajorVersion = 4;
	g_dviCommCtrls.dwMinorVersion = 0;
	g_dviCommCtrls.dwBuildNumber = 0;
	g_dviCommCtrls.dwPlatformID = DLLVER_PLATFORM_WINDOWS;
}


// macro to convert version numbers to hex format
#define CNV_OS_VER(x) ((BYTE)(((BYTE)(x) / 10 * 16) | ((BYTE)(x) % 10)))

inline void InitRealVersions()
{
	CheckCommCtrlsVersion();
	CheckOsVersion();

	// set real version values

	realWINVER = MAKEWORD(CNV_OS_VER(g_osviWindows.dwMinorVersion),
		CNV_OS_VER(g_osviWindows.dwMajorVersion));

	if (g_osviWindows.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		real_WIN32_WINDOWS = realWINVER;
	else
		real_WIN32_WINDOWS = 0;
	if (g_osviWindows.dwPlatformId == VER_PLATFORM_WIN32_NT)
		real_WIN32_WINNT = realWINVER;
	else
		real_WIN32_WINNT = 0;
	switch (g_dviCommCtrls.dwMajorVersion)
	{
	case 4:
		switch (g_dviCommCtrls.dwMinorVersion)
		{
		case 70:
			real_WIN32_IE = 0x0300;
			break;
		case 71:
			real_WIN32_IE = 0x0400;
			break;
		case 72:
			real_WIN32_IE = 0x0401;
			break;
		default:
			real_WIN32_IE = 0x0200;
		}
		break;
	case 5:
		if (g_dviCommCtrls.dwMinorVersion > 80)
			real_WIN32_IE = 0x0501;
		else
			real_WIN32_IE = 0x0500;
		break;
	case 6:
		real_WIN32_IE = 0x0600;	// includes checks for 0x0560 (IE6)
		break;
	default:
		real_WIN32_IE = 0;
	}
}

inline DWORD GetRealWINVER()
{
	return realWINVER;
}

inline DWORD Get_real_WIN32_WINDOWS()
{
	return real_WIN32_WINDOWS;
}
inline DWORD Get_real_WIN32_WINNT()
{
	return real_WIN32_WINNT;
}
inline DWORD Get_real_WIN32_IE()
{
	return real_WIN32_IE;
}
