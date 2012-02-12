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

#include <RegisteredMsg.h>

// horizontal line in wizard mode
#define ID_WIZLINE		ID_WIZFINISH+1
#define ID_WIZLINEHDR	ID_WIZFINISH+2
#define AFX_IDC_TAB_CONTROL             0x3020


__declspec(selectany) UINT _propButtons[] = 
																{
																	IDOK, IDCANCEL, ID_APPLY_NOW, IDHELP,
																	ID_WIZBACK, ID_WIZNEXT, ID_WIZFINISH
																};

__declspec(selectany) LPCTSTR _propButtonsNames[] = 
																{
																	_T("&A"), _T("&B"), _T("&C"), _T("&D"),
																	_T("&E"), _T("&F"), _T("&G")
																};

template <class T, class TBase = CPropertySheetWindow, class baseSheet = CPropertySheetImpl<T, TBase> >
class ATL_NO_VTABLE CResizablePropertySheetImpl : 
	public baseSheet,
	public CResizableLayout<T>, 
	public CResizableSheetState<T>,
	public CResizableGrip<T>,
	public CResizableMinMax<T>
{
public:
	typedef CResizablePropertySheetImpl<T, TBase>	thisClass;
	typedef baseSheet baseClass;

	CResizablePropertySheetImpl(_U_STRINGorID title = (LPCTSTR)NULL, UINT uStartPage = 0, HWND hWndParent = NULL, BOOL bChild = FALSE):
		baseSheet(title, uStartPage, hWndParent)
	{
		m_bChild = bChild;
		m_bEnableSaveRestore = FALSE;
		m_bSavePage = FALSE;
		m_dwGripTempState = 1;
		m_bLayoutDone = FALSE;
	}

	void EnableSaveRestore(HKEY hKey, LPCTSTR pszAppSection, LPCTSTR pszSection, LPCTSTR pszWndName, BOOL bRectOnly = FALSE, BOOL bWithPage = TRUE)
	{
		T* pT = static_cast<T*>(this);
		m_bSavePage = bWithPage;
		m_bEnableSaveRestore = TRUE;
		m_bRectOnly = bRectOnly;
		// restore immediately
		pT->LoadWindowRect(hKey, pszAppSection, pszSection, pszWndName, bRectOnly);
		pT->LoadPage();
		pT->SetActivePage(pT->m_psh.nStartPage);
	}
	int GetMinWidth()	// minimum width to display all buttons
	{
		T* pT = static_cast<T*>(this);

		HWND hWnd = NULL;
		CRect rectWnd, rectSheet;
		pT->GetTotalClientRect(&rectSheet);

		int max = 0, min = rectSheet.Width();
		// search for leftmost and rightmost button margins
		for (int i = 0; i < 7; i++)
		{
			hWnd = pT->GetDlgItem(_propButtons[i]);
			// exclude not present or hidden buttons
			if (hWnd == NULL || !(::GetWindowLong(hWnd, GWL_STYLE) & WS_VISIBLE))
				continue;

			// left position is relative to the right border
			// of the parent window (negative value)
			::GetWindowRect(hWnd, &rectWnd);
			::MapWindowPoints(NULL, pT->m_hWnd, (LPPOINT)&rectWnd, 2);
			int left = rectSheet.right - rectWnd.left;
			int right = rectSheet.right - rectWnd.right;
			
			if (left > max)
				max = left;
			if (right < min)
				min = right;
		}

		// sizing border width
		int border = GetSystemMetrics(SM_CXSIZEFRAME);
		
		// compute total width
		return max + min + 2*border;
	}

	
	DECLARE_REGISTERED_MESSAGE(WMU_INIT_PROPSHEET)
	BEGIN_MSG_MAP(thisClass)
		if (uMsg == GET_REGISTERED_MESSAGE(WMU_INIT_PROPSHEET))
		{
			T* pT = static_cast<T*>(this);
			pT->PresetLayout();
			pT->OnInitDialog();
			bHandled = FALSE;
			return 0;
		}
		MSG_WM_GETMINMAXINFO(OnGetMinMaxInfo)
		MSG_WM_SIZE(OnSize)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
		REFLECTED_NOTIFY_CODE_HANDLER(PSN_SETACTIVE, OnPageSetActive)
		CHAIN_MSG_MAP(baseSheet)
		if (m_bLayoutDone)
			CHAIN_MSG_MAP(CResizableLayout<T>)
	END_MSG_MAP()

	void OnInitDialog()
	{
	}
	void OnGetMinMaxInfo(LPMINMAXINFO lpMMI)
	{
		T* pT = static_cast<T*>(this);
		pT->MinMaxInfo(lpMMI);

		CTabCtrl wndTabCtrl(pT->GetTabControl());
		int nCount = pT->GetPageCount();
		for (int idx = 0; idx < nCount; ++idx)
		{
			HPROPSHEETPAGE hPage = pT->IndexToPage(idx);
			HWND hWndPage = pT->IndexToHwnd(pT->PageToIndex(hPage));
			if (pT->IsWizard())	// wizard mode
			{
				// use pre-calculated margins
				CRect rectExtra(-CPoint(m_sizePageTL), -CPoint(m_sizePageBR));
				// add non-client size
				::AdjustWindowRectEx(&rectExtra, pT->GetStyle(), !(pT->GetStyle() & WS_CHILD) &&
					::IsMenu(pT->GetMenu()), pT->GetExStyle());
				pT->ChainMinMaxInfo(lpMMI, hWndPage, rectExtra.Size());
			}
			else	// tab mode
			{
				pT->ChainMinMaxInfoCB(lpMMI, hWndPage);
			}
		}
	}
	void OnSize(UINT nType, CPoint /*point*/)
	{
		T* pT = static_cast<T*>(this);
		LRESULT lRes = pT->DefWindowProc();
		if (nType == SIZE_MAXHIDE || nType == SIZE_MAXSHOW)
			return;		// arrangement not needed
		if (!m_bLayoutDone)
			return;		// arrangement not needed
		// update size-grip
		if (nType == SIZE_MAXIMIZED)
			pT->HideSizeGrip(&m_dwGripTempState);
		else
			pT->ShowSizeGrip(&m_dwGripTempState);
		// update grip and layout
		pT->UpdateSizeGrip();
		pT->ArrangeLayout();
		return;
	}
	void OnDestroy()
	{
		if (m_bEnableSaveRestore)
		{
			SaveWindowRect(m_hRegKey, m_sAppSection, m_sSection, m_sWndName, m_bRectOnly);
			SavePage();
		}
		// remove child windows
		RemoveAllAnchors();
		SetMsgHandled(FALSE);
	}
	BOOL OnEraseBkgnd(HDC hDC)
	{
		T* pT = static_cast<T*>(this);
		pT->ClipChildren(hDC, FALSE);
		BOOL bRet = pT->DefWindowProc();
		pT->ClipChildren(hDC, TRUE);
		return bRet;
	}
	LRESULT OnPageSetActive(int nCode, LPNMHDR pNMHDR, BOOL &bHandled)
	{
		// update new wizard page
		// active page changes after this notification
		PostMessage(WM_SIZE);
		bHandled = FALSE;
		return FALSE;	// continue routing
	}
// used internally
	void PresetLayout()
	{
		T* pT = static_cast<T*>(this);

		// set the initial size as the min track size
		CRect rc;
		pT->GetWindowRect(&rc);
		pT->SetMinTrackSize(rc.Size());

		CWindow wndTabControl(pT->GetTabControl());
		if (pT->GetStyle() & WS_CHILD)
		{
			pT->GetClientRect(&rc);

			wndTabControl.MoveWindow(&rc);
		}
		if (pT->IsWizard())	// wizard mode
		{
			// hide tab control
			wndTabControl.ShowWindow(SW_HIDE);
			pT->AddAnchor(ID_WIZLINE, BOTTOM_LEFT, BOTTOM_RIGHT);
		}
		else	// tab mode
		{
			pT->AddAnchor(wndTabControl, TOP_LEFT, BOTTOM_RIGHT);
		}

		// add a callback for active page (which can change at run-time)
		m_nCallbackID = pT->AddAnchorCallback(-1);

		// use *total* parent size to have correct margins
		CRect rectPage, rectSheet;
		pT->GetTotalClientRect(&rectSheet);

		::GetWindowRect(pT->GetActivePage(), &rectPage);
		pT->ScreenToClient(&rectPage);

		// pre-calculate margins
		m_sizePageTL = rectPage.TopLeft() - rectSheet.TopLeft();
		m_sizePageBR = rectPage.BottomRight() - rectSheet.BottomRight();

		// add all possible buttons, if they exist
		for (int i = 0; i < 7; i++)
		{
			if (pT->GetDlgItem(_propButtons[i]) != NULL)
				pT->AddAnchor(_propButtons[i], BOTTOM_RIGHT);
		}
		wndTabControl.ModifyStyle(0, WS_CLIPSIBLINGS);
		pT->m_bLayoutDone = TRUE;
	}
	void RefreshLayout()
	{
		T* pT = static_cast<T*>(this);
		pT->SendMessage(WM_SIZE);
	}
	BOOL ArrangeLayoutCallback(LayoutInfo& layout)
	{
		T* pT = static_cast<T*>(this);
		if (layout.nCallbackID != m_nCallbackID)	// we only added 1 callback
			return CResizableLayout<T>::ArrangeLayoutCallback(layout);

		// set layout info for active page
		layout.hWnd = (HWND)::SendMessage(m_hWnd, PSM_GETCURRENTPAGEHWND, 0, 0);
		if (!::IsWindow(layout.hWnd))
			return FALSE;

		// set margins
		if (pT->IsWizard())	// wizard mode
		{
			// use pre-calculated margins
			layout.marginTL = m_sizePageTL;
			layout.marginBR = m_sizePageBR;
		}
		else	// tab mode
		{
			CTabCtrl wndTabCtrl(pT->GetTabControl());
			// get tab position after resizing and calc page rect
			CRect rectPage, rectSheet;
			pT->GetTotalClientRect(&rectSheet);

			if (!pT->GetAnchorPosition(wndTabCtrl, rectSheet, rectPage))
				return FALSE; // no page yet

			// temporarily resize the tab control to calc page size
			CRect rectSave;
			wndTabCtrl.GetWindowRect(rectSave);
			::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&rectSave, 2);
			wndTabCtrl.SetRedraw(FALSE);
			wndTabCtrl.MoveWindow(rectPage, FALSE);
			wndTabCtrl.AdjustRect(FALSE, &rectPage);
			wndTabCtrl.MoveWindow(rectSave, FALSE);
			wndTabCtrl.SetRedraw(TRUE);

			// set margins
			layout.marginTL = rectPage.TopLeft() - rectSheet.TopLeft();
			layout.marginBR = rectPage.BottomRight() - rectSheet.BottomRight();
		}

		// set anchor types
		layout.anchorTL = TOP_LEFT;
		layout.anchorBR = BOTTOM_RIGHT;

		// use this layout info
		return TRUE;
	}
	BOOL CalcSizeExtra(HWND /*hWndChild*/, CSize sizeChild, CSize &sizeExtra)
	{
		T* pT = static_cast<T*>(this);
		CTabCtrl wndTabCtrl(pT->GetTabControl());
		if (!wndTabCtrl)
			return FALSE;

		// get margins of tabcontrol
		CRect rectMargins;
		if (!pT->GetAnchorMargins(wndTabCtrl, sizeChild, rectMargins))
			return FALSE;

		// get margin caused by tabcontrol
		CRect rectTabMargins(0,0,0,0);

		// get tab position after resizing and calc page rect
		CRect rectPage, rectSheet;
		pT->GetTotalClientRect(&rectSheet);

		if (!pT->GetAnchorPosition(wndTabCtrl, rectSheet, rectPage))
			return FALSE; // no page yet

		// temporarily resize the tab control to calc page size
		CRect rectSave;
		wndTabCtrl.GetWindowRect(rectSave);
		::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&rectSave, 2);
		wndTabCtrl.SetRedraw(FALSE);
		wndTabCtrl.MoveWindow(rectPage, FALSE);
		wndTabCtrl.AdjustRect(TRUE, &rectTabMargins);
		wndTabCtrl.MoveWindow(rectSave, FALSE);
		wndTabCtrl.SetRedraw(TRUE);

		// add non-client size
		::AdjustWindowRectEx(&rectTabMargins, pT->GetStyle(), !(pT->GetStyle() & WS_CHILD) &&
			::IsMenu(pT->GetMenu()), pT->GetExStyle());

		// compute extra size
		sizeExtra = rectMargins.TopLeft() + rectMargins.BottomRight() + rectTabMargins.Size();
		return TRUE;
	}
	// Attributes
	// flags
	BOOL m_bEnableSaveRestore;
	BOOL m_bRectOnly;

	// layout vars
	CSize m_sizePageTL, m_sizePageBR;
	UINT m_nCallbackID;

	// support for temporarily hiding the grip
	DWORD m_dwGripTempState;

	// internal status
	BOOL m_bChild;
	BOOL m_bLayoutDone;
	BOOL IsWizard() { return (m_psh.dwFlags & PSH_WIZARD); }


	// Data
	HICON	m_hIcon;
public:
	static int CALLBACK PropSheetCallback(HWND hWnd, UINT uMsg, LPARAM lParam)
	{
		if(uMsg == PSCB_PRECREATE)
		{
			int nResult = baseClass::PropSheetCallback(hWnd, uMsg, lParam);
			T* pT = (T*)ATL::_AtlWinModule.ExtractCreateWndData();
			ATL::_AtlWinModule.AddCreateWndData(&pT->m_thunk.cd, pT);

			DLGTEMPLATE* dlg = (DLGTEMPLATE*) lParam;
			if (pT->m_bChild)
			{
				dlg->style &= ~(WS_POPUP|WS_CAPTION|WS_SYSMENU|DS_MODALFRAME|DS_NOIDLEMSG);
				dlg->style |= (WS_CHILD|WS_VISIBLE|WS_TABSTOP|DS_CONTROL|DS_3DLOOK);
				dlg->dwExtendedStyle |= WS_EX_CONTROLPARENT;
			}
			else
			{
				dlg->style &= ~(WS_CHILD|DS_MODALFRAME|WS_CLIPCHILDREN|WS_VISIBLE|DS_NOIDLEMSG);
				dlg->style |= (WS_THICKFRAME);
			}
#if (_WIN32_WINNT >= 0x0501 && !defined(RSZLIB_NO_XP_DOUBLE_BUFFER))
			dlg->dwExtendedStyle |= WS_EX_COMPOSITED;
#endif
			return nResult;
		}
		else if (uMsg == PSCB_INITIALIZED)
		{
			T* pT = (T*)ATL::_AtlWinModule.ExtractCreateWndData();
			ATL::_AtlWinModule.AddCreateWndData(&pT->m_thunk.cd, pT);
			int nResult = baseClass::PropSheetCallback(hWnd, uMsg, lParam);
			// child dialogs don't want resizable border or size grip,
			// nor they can handle the min/max size constraints
			BOOL bChild = pT->GetStyle() & WS_CHILD;

			// create and init the size-grip
			if (!pT->CreateSizeGrip(!bChild))
				return FALSE;

			pT->MakeResizable();
			// set the initial size as the min track size
			CRect rc;
			pT->GetWindowRect(&rc);
			pT->SetMinTrackSize(rc.Size());

			// initialize layout
			pT->PostMessage(GET_REGISTERED_MESSAGE(WMU_INIT_PROPSHEET));
			//pT->PresetLayout();
			//pT->OnInitDialog();
			return nResult;
		}
		return 0;
	}
};
// for non-customized sheets
class CResizablePropertySheet : 
	public CResizablePropertySheetImpl<CResizablePropertySheet>
{
	typedef CResizablePropertySheetImpl<CResizablePropertySheet> baseClass;
	typedef CResizablePropertySheet thisClass;
public:
	CResizablePropertySheet(_U_STRINGorID title = (LPCTSTR)NULL, UINT uStartPage = 0, HWND hWndParent = NULL)
		: baseClass(title, uStartPage, hWndParent)
	{ }
	BEGIN_MSG_MAP(thisClass)
		CHAIN_MSG_MAP(baseClass)
	END_MSG_MAP()
	void OnInitDialog()
	{
		// set the initial size as the min track size
		CRect rc;
		GetWindowRect(&rc);
		SetMinTrackSize(rc.Size());
		CenterWindow(GetParent());
		ShowWindow(SW_SHOW);
	}
};
