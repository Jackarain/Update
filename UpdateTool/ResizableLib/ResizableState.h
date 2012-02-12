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
// WTL port implementation
// Copyright (C) 2002-2005 by Alexander D. Alexeev
/////////////////////////////////////////////////////////////////////////////

#pragma once

#define PLACEMENT_ENT	_T("Windows Placement")

template < class T >
class ATL_NO_VTABLE CResizableState  
{
public:
	CResizableState()
	{
	}
	virtual ~CResizableState()
	{
	}

	// non-zero if successful
	BOOL LoadWindowRect(HKEY hKey, LPCTSTR pszAppSection, LPCTSTR pszSection, LPCTSTR pszWndName, BOOL bRectOnly=FALSE)
	{
		T* pT = static_cast<T*>(this);

		m_hRegKey = hKey;
		m_sAppSection = pszAppSection;
		m_sSection = pszSection;
		m_sWndName = pszWndName;

		CString sSection(pszAppSection);
		sSection += _T("\\");
		sSection += pszSection;

		CRegKey key;
		WINDOWPLACEMENT wp;
		DWORD dwSize = sizeof(WINDOWPLACEMENT);
		if ( (key.Open(m_hRegKey, sSection, KEY_READ) != ERROR_SUCCESS) || 
				 (key.QueryBinaryValue(m_sWndName, &wp, &dwSize) != ERROR_SUCCESS) )
		{
			pT->ShowWindow(SW_SHOW);
			return FALSE;
		}
		return pT->LoadWindowRectIndirect(wp, bRectOnly);
	}
	BOOL SaveWindowRect(HKEY hKey, LPCTSTR pszAppSection, LPCTSTR pszSection, LPCTSTR pszWndName, BOOL bRectOnly=FALSE)
	{
		T* pT = static_cast<T*>(this);
		CRegKey key;

		CString sSection(pszAppSection);
		sSection += _T("\\");
		sSection += pszSection;
		if (key.Open(hKey, sSection, KEY_WRITE) != ERROR_SUCCESS)
			key.Create(hKey, sSection);
		if (key.Open(hKey, sSection, KEY_WRITE) != ERROR_SUCCESS)
			return FALSE;

		WINDOWPLACEMENT wp;
		if (!pT->SaveWindowRectIndirect(wp, bRectOnly))
			return FALSE;
		return key.SetBinaryValue(pszWndName, &wp, sizeof(WINDOWPLACEMENT)) == ERROR_SUCCESS;
	}
	BOOL LoadWindowRectIndirect(WINDOWPLACEMENT &wp, BOOL bRectOnly)
	{
		T* pT = static_cast<T*>(this);

//		RECT rcOld = wp.rcNormalPosition;
		if (bRectOnly)	// restore size/pos only
		{
			wp.showCmd = SW_NORMAL;
			wp.flags = 0;
			CRect rect(wp.rcNormalPosition);
			return pT->SetWindowPos(NULL, rect.left, rect.top,
				rect.Width(), rect.Height(), SWP_NOACTIVATE | SWP_NOZORDER |
				SWP_NOREPOSITION);
		}
		else	// restore also min/max state
			return pT->SetWindowPlacement(&wp);
	}
	BOOL SaveWindowRectIndirect(WINDOWPLACEMENT &wp, BOOL bRectOnly)
	{
		T* pT = static_cast<T*>(this);
		wp.length = sizeof(WINDOWPLACEMENT);
		if (!pT->GetWindowPlacement(&wp))
			return FALSE;
		RECT& rc = wp.rcNormalPosition;	// alias
		if ( pT->IsIconic() )
			// never restore to Iconic state
			wp.showCmd = SW_SHOW ;
		if ((wp.flags & WPF_RESTORETOMAXIMIZED) != 0)
			// if maximized and maybe iconic restore maximized state
			wp.showCmd = SW_SHOWMAXIMIZED;
		if (bRectOnly)	// save size/pos only (normal state)
		{
			wp.showCmd = SW_NORMAL;
			pT->GetWindowRect(&rc);
		}
		// save also min/max state
		return TRUE;
	}

	CString m_sAppSection;
	CString m_sSection;
	CString m_sWndName;

	HKEY	m_hRegKey;
};


#define ACTIVEPAGE_ENT 	_T("ActivePage")

template < class T >
class ATL_NO_VTABLE CResizableSheetState :
	public CResizableState<T>
{
public:
	CResizableSheetState()
	{
		m_bSavePage = TRUE;
	}
	~CResizableSheetState()
	{
	}
	BOOL m_bSavePage;
	BOOL SavePage(LPCTSTR pszSuffix = ACTIVEPAGE_ENT)
	{
		T* pT = static_cast<T*>(this);
		if (!m_bSavePage)
			return FALSE;
		if (m_sWndName.IsEmpty())
		{
			ATLASSERT(!"Use SaveWindowRect First!!!");
			return FALSE;
		}
		int page = pT->GetActiveIndex();
		if (page < 0)
			page = pT->m_psh.nStartPage;
		TCHAR pszKey[MAX_PATH];
		wsprintf(pszKey, _T("%s_%s"), m_sWndName, pszSuffix);
		CRegKey key;

		CString sSection(m_sAppSection);
		sSection += _T("\\");
		sSection += m_sSection;
		if (key.Open(m_hRegKey, sSection, KEY_WRITE) != ERROR_SUCCESS)
			key.Create(m_hRegKey, sSection);
		if (key.Open(m_hRegKey, sSection, KEY_WRITE) != ERROR_SUCCESS)
			return FALSE;
		return key.SetDWORDValue(pszKey, (DWORD)page) == ERROR_SUCCESS;
	}
	BOOL LoadPage(LPCTSTR pszSuffix = ACTIVEPAGE_ENT)
	{
		T* pT = static_cast<T*>(this);
		if (!m_bSavePage)
			return FALSE;
		if (m_sWndName.IsEmpty())
		{
			ATLASSERT(!"Use LoadWindowRect First!!!");
			return FALSE;
		}
		TCHAR pszKey[MAX_PATH];
		wsprintf(pszKey, _T("%s_%s"), m_sWndName, pszSuffix);
		UINT &page = pT->m_psh.nStartPage;

		CString sSection(m_sAppSection);
		sSection += _T("\\");
		sSection += m_sSection;

		CRegKey key;
		if (key.Open(m_hRegKey, sSection, KEY_READ) != ERROR_SUCCESS)
			return FALSE;
		return key.QueryDWORDValue(pszKey, (DWORD&)page) == ERROR_SUCCESS;
	}
};
