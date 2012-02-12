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

template <class T, class TBase = CWindow, class DlgBase = CAxDialogImpl<T, TBase> >
class ATL_NO_VTABLE CResizableDialogImplT : 
	public DlgBase, 
	public CResizableLayout<T>, 
	public CResizableState<T>,
	public CResizableGrip<T>,
	public CResizableMinMax<T>
{
	typedef CResizableDialogImplT<T, TBase, DlgBase> thisClass;
	typedef DlgBase baseClass;
public:
	CResizableDialogImplT()
	{
		m_bEnableSaveRestore = FALSE;
		m_bRectOnly = FALSE;
		m_dwGripTempState = 1;
		m_bPrevIsEraseBkGnd = FALSE;
		m_bPrevIsNcCalcSize = FALSE;
	}

	BOOL m_bPrevIsEraseBkGnd;
	HDC	m_hDCBkGnd;
	BEGIN_MSG_MAP(thisClass)
		// 亦觇?蝈腩溻桄屙? 镱蝾祗 黩?礤忸珈铈眍 恹玮囹?BaseDlgProc
		if (uMsg == WM_ERASEBKGND && !m_bPrevIsEraseBkGnd)
		{
			T* pT = static_cast<T*>(this);
			m_hDCBkGnd = (HDC)wParam;
			pT->ClipChildren(m_hDCBkGnd, FALSE);
			m_bPrevIsEraseBkGnd = TRUE;
			bHandled = FALSE;
			return FALSE;
		}
		if (uMsg != WM_ERASEBKGND && m_bPrevIsEraseBkGnd)
		{
			T* pT = static_cast<T*>(this);
			pT->ClipChildren(m_hDCBkGnd, TRUE);
			m_bPrevIsEraseBkGnd = FALSE;
			bHandled = FALSE;
		}
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_SIZE(OnSize)
		MSG_WM_GETMINMAXINFO(OnGetMinMaxInfo)
		MSG_WM_DESTROY(OnDestroy)
		CHAIN_MSG_MAP(CResizableLayout<T>)
	END_MSG_MAP()

	void OnGetMinMaxInfo(LPMINMAXINFO lpMinMaxInfo)
	{
		T* pT = static_cast<T*>(this);
		pT->MinMaxInfo(lpMinMaxInfo);
	}
	void OnSize(UINT nType, CPoint /*point*/)
	{
		T* pT = static_cast<T*>(this);
		SetMsgHandled (FALSE);
		if (nType == SIZE_MAXHIDE || nType == SIZE_MAXSHOW)
			return;		// arrangement not needed
		if (nType == SIZE_MAXIMIZED)
			pT->HideSizeGrip(&m_dwGripTempState);
		else
			pT->ShowSizeGrip(&m_dwGripTempState);

		// update grip and layout
		pT->UpdateSizeGrip();
		pT->ArrangeLayout();
	}
	void OnDestroy()
	{
		T* pT =static_cast<T*>(this);
		if (m_bEnableSaveRestore)
			pT->SaveWindowRect(m_hRegKey, m_sAppSection, m_sSection, m_sWndName, m_bRectOnly);
		// remove child windows
		RemoveAllAnchors();
		SetMsgHandled(FALSE);
	}

	BOOL OnInitDialog(HWND hWnd, LPARAM lParam)
	{
		T* pT =static_cast<T*>(this);
		// child dialogs don't want resizable border or size grip,
		// nor they can handle the min/max size constraints
		BOOL bChild = pT->GetStyle()& WS_CHILD;
		// create and init the size-grip
		if (!pT->CreateSizeGrip(!bChild))
			return FALSE;
		// We need for all dialogs
		//if (bChild) 
		pT->AddOtherChildAnchors();
		// set the initial size as the min track size
		CRect rc;
		pT->GetWindowRect(&rc);
		pT->SetMinTrackSize(rc.Size());

		pT->MakeResizable();
		SetMsgHandled(FALSE);
		return TRUE;
	}

	// Operations
protected:
	// section to use in app's profile
	void EnableSaveRestore(HKEY hKey, LPCTSTR pszAppSection, LPCTSTR pszSection, LPCTSTR pszWndName, BOOL bRectOnly = FALSE)
	{
		m_bEnableSaveRestore = TRUE;
		m_bRectOnly = bRectOnly;
		LoadWindowRect(hKey, pszAppSection, pszSection, pszWndName, bRectOnly);
	}
// Attributes
private:
	// support for temporarily hiding the grip
	DWORD m_dwGripTempState;
	// flags
	BOOL m_bEnableSaveRestore;
	BOOL m_bRectOnly;
};


template <class T>
class ATL_NO_VTABLE CResizableDialogImpl : public CResizableDialogImplT<T>
{
public:
	CResizableDialogImpl():CResizableDialogImplT<T>()
	{	
	}
};
