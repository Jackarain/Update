// ResizablePage.h: interface for the CResizablePropertyPage class.
//
//////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2000-2004 by Paolo Messina
// (http://www.geocities.com/ppescher - ppescher@yahoo.com)
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

template <class T, class TBase = CPropertyPageWindow, class basePage = CPropertyPageImpl<T, TBase> >
class ATL_NO_VTABLE CResizablePropertyPageImpl : 
	public basePage, 
	public CResizableLayout<T>,
	public CResizableMinMax<T>
{
public:
	typedef CResizablePropertyPageImpl<T, TBase, basePage> thisClass;
	typedef basePage baseClass;

	CResizablePropertyPageImpl(_U_STRINGorID title = (LPCTSTR)NULL) : basePage(title)
	{
		m_bPrevIsEraseBkGnd = FALSE;
		m_bPrevIsNcCalcSize = FALSE;
	}

	BOOL m_bPrevIsEraseBkGnd;
	HDC	m_hDCBkGnd;
	BOOL m_bPrevIsNcCalcSize;
	LPARAM m_lParamNcCalcSize;

	BEGIN_MSG_MAP(thisClass)
		if (uMsg == WM_NOTIFY)
		{
			T* pT = static_cast<T*>(this);
			NMHDR* pNMHDR = (NMHDR*)lParam;
			if (pNMHDR->code == PSN_SETACTIVE)
				lResult = pT->ReflectNotifications(uMsg, wParam, lParam, bHandled);
		}
		// 亦觇?蝈腩溻桄屙? 镱蝾祗 黩?礤忸珈铈眍 恹玮囹?BaseDlgProc
		if (uMsg == WM_ERASEBKGND && !m_bPrevIsEraseBkGnd)
		{
			T* pT = static_cast<T*>(this);
			m_bPrevIsEraseBkGnd = TRUE;
			m_hDCBkGnd = (HDC)wParam;
			pT->ClipChildren(m_hDCBkGnd, FALSE);
			return FALSE;
		}
		if (uMsg != WM_ERASEBKGND && m_bPrevIsEraseBkGnd)
		{
			T* pT = static_cast<T*>(this);
			pT->ClipChildren(m_hDCBkGnd, TRUE);
			m_bPrevIsEraseBkGnd = FALSE;
		}
		if (uMsg == WM_ERASEBKGND && m_bPrevIsEraseBkGnd)
		{
			ATLASSERT(FALSE);
		}
		MSG_WM_SIZE(OnSize)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_DESTROY(OnDestroy)
		if (uMsg == WM_NCCALCSIZE && !m_bPrevIsNcCalcSize)
		{
			T* pT = static_cast<T*>(this);
			lResult = 0;
			m_lParamNcCalcSize = lParam;
			pT->HandleNcCalcSize(FALSE, (LPNCCALCSIZE_PARAMS)m_lParamNcCalcSize, lResult);
			m_bPrevIsNcCalcSize = TRUE;
			bHandled = FALSE;
		}
		if (uMsg == WM_NCCALCSIZE && m_bPrevIsNcCalcSize)
		{
			T* pT = static_cast<T*>(this);
			pT->HandleNcCalcSize(TRUE, (LPNCCALCSIZE_PARAMS)m_lParamNcCalcSize, lResult);
			m_bPrevIsNcCalcSize = FALSE;
		}
		MSG_WM_GETMINMAXINFO(OnGetMinMaxInfo)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()
	BOOL OnInitDialog(HWND hDlg, LPARAM lParam)
	{
		T* pT = static_cast<T*>(this);
		// set the initial size as the min track size
		CRect rc;
		pT->GetWindowRect(&rc);
		pT->SetMinTrackSize(rc.Size());
		AddOtherChildAnchors();
		SetMsgHandled(FALSE);
		return TRUE;
	}
	void OnDestroy()
	{
		T* pT = static_cast<T*>(this);
		// remove child windows
		pT->RemoveAllAnchors();
		SetMsgHandled(FALSE);
	}
	void OnSize(UINT /*nType*/, CPoint /*point*/)
	{
		T* pT = static_cast<T*>(this);
		ArrangeLayout();
	}
	void OnGetMinMaxInfo(LPMINMAXINFO lpMinMaxInfo)
	{
		T* pT = static_cast<T*>(this);
		pT->MinMaxInfo(lpMinMaxInfo);
	}
};


// for non-customized pages
template <WORD t_wDlgTemplateID>
class CResizablePropertyPage : public CResizablePropertyPageImpl<CResizablePropertyPage<t_wDlgTemplateID> >
{
public:
	enum { IDD = t_wDlgTemplateID };

	CResizablePropertyPage(_U_STRINGorID title = (LPCTSTR)NULL) : CResizablePropertyPageImpl<CResizablePropertyPage>(title)
	{ }
	BEGIN_MSG_MAP(CResizablePropertyPage<t_wDlgTemplateID>)
		CHAIN_MSG_MAP(CResizablePropertyPageImpl<CResizablePropertyPage<t_wDlgTemplateID> >)
	END_MSG_MAP()
};
