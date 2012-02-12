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

#define WC_SIZEGRIPA "SizeGrip"
#define WC_SIZEGRIPW	L"SizeGrip"

#ifdef UNICODE
#define WC_SIZEGRIP	WC_SIZEGRIPW
#else
#define WC_SIZEGRIP	WC_SIZEGRIPA
#endif

///////////////////////////////////////////////////////////////////////////////
// Control itself
///////////////////////////////////////////////////////////////////////////////
typedef CWinTraits<WS_CHILD | WS_CLIPSIBLINGS | SBS_SIZEGRIP, 0 > CSizeGripTraits;

class CSizeGrip : 
	public CWindowImpl<CSizeGrip, CScrollBar, CSizeGripTraits>
{
	typedef CWindowImpl<CSizeGrip, CScrollBar, CSizeGripTraits> baseClass;
	typedef CSizeGrip thisClass;
public:
	DECLARE_WND_SUPERCLASS(WC_SIZEGRIP, CScrollBar::GetWndClassName())

	SIZE m_size;			// holds grip size
	CSizeGrip()
	{
		m_bTransparent = FALSE;
		m_bTriangular = FALSE;
		// set window size
		m_size.cx = GetSystemMetrics(SM_CXVSCROLL);
		m_size.cy = GetSystemMetrics(SM_CYHSCROLL);
	}
	void SetTriangularShape(BOOL bEnable)
	{
		m_bTriangular = bEnable;

		if (bEnable)
		{
			// set a triangular window region
			CRect rect;
			GetWindowRect(rect);
			rect.OffsetRect(-rect.TopLeft());
			POINT arrPoints[] =
			{
				{ rect.left, rect.bottom },
				{ rect.right, rect.bottom },
				{ rect.right, rect.top }
			};
			CRgn rgnGrip;
			rgnGrip.CreatePolygonRgn(arrPoints, 3, WINDING);
			SetWindowRgn((HRGN)rgnGrip.Detach(), IsWindowVisible());
		}
		else
		{
			SetWindowRgn((HRGN)NULL, IsWindowVisible());
		}
	}

	void SetTransparency(BOOL bActivate)
	{
		// creates or deletes DCs and Bitmaps used for
		// implementing a transparent size grip

		if (bActivate && !m_bTransparent)
		{
			m_bTransparent = TRUE;

			CClientDC dc(m_hWnd);

			// create memory DCs and bitmaps
			m_dcGrip.CreateCompatibleDC(dc);
			m_bmGrip.CreateCompatibleBitmap(dc, m_size.cx, m_size.cy);

			m_dcMask.CreateCompatibleDC(dc);
			m_bmMask.CreateBitmap(m_size.cx, m_size.cy, 1, 1, NULL);
		}
		else if (!bActivate && m_bTransparent)
		{
			m_bTransparent = FALSE;

			// destroy memory DCs and bitmaps
			m_dcGrip.DeleteDC();
			m_bmGrip.DeleteObject();

			m_dcMask.DeleteDC();
			m_bmMask.DeleteObject();
		}
	}

	BOOL IsRTL(HWND hwnd)
	{
		return (GetExStyle() & WS_EX_LAYOUTRTL);
	}

	HWND Create(HWND hWndParent, CRect rect)
	{
		HWND hWnd = baseClass::Create(hWndParent, rect, NULL, GetWndStyle(0));
		if (hWnd != NULL)
		{
			POINT aPoints[] = {{rect.left,rect.bottom}, {rect.right,rect.bottom}, {rect.right,rect.top}};
			CRgn rgnGrip;
			rgnGrip.CreatePolygonRgn(aPoints, 3, WINDING);
			SetWindowRgn(rgnGrip.Detach(), FALSE);
		}
		return hWnd;
	}

	BEGIN_MSG_MAP(thisClass)
		MSG_WM_SETFOCUS(OnSetFocus)
		MSG_WM_NCHITTEST(OnNcHitTest)
		MSG_WM_GETDLGCODE(OnGetDlgCode)
		MSG_WM_SETTINGCHANGE(OnSettingChange)
		MSG_WM_DESTROY(OnDestroy)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_PRINTCLIENT(OnPrintClient)
	END_MSG_MAP()
	
	void OnSetFocus(HWND hOldWnd)
	{
		// fix to prevent the control to gain focus, if set directly
		// (for example when it's the only one control in a dialog)
		SetMsgHandled(TRUE);
		return;
	}
	LRESULT OnNcHitTest(CPoint pt)
	{
		// choose proper cursor shape
		if (IsRTL(m_hWnd))
			return HTBOTTOMLEFT;
		else
			return HTBOTTOMRIGHT;
	}
	LRESULT OnGetDlgCode(LPMSG pMsg)
	{
		// fix to prevent the control to gain focus, using arrow keys
		// (standard grip returns DLGC_WANTARROWS, like any standard scrollbar)
		return DLGC_STATIC;
	}
	void OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
	{
		// update grip's size
		CSize sizeOld = m_size;
		m_size.cx = GetSystemMetrics(SM_CXVSCROLL);
		m_size.cy = GetSystemMetrics(SM_CYHSCROLL);

		// resize transparency bitmaps
		if (m_bTransparent)
		{
			CClientDC dc(m_hWnd);
			// destroy bitmaps
			m_bmGrip.DeleteObject();
			m_bmMask.DeleteObject();

			// re-create bitmaps
			m_bmGrip.CreateCompatibleBitmap(dc, m_size.cx, m_size.cy);
			m_bmMask.CreateBitmap(m_size.cx, m_size.cy, 1, 1, NULL);
		}

		// re-calc shape
		if (m_bTriangular)
			SetTriangularShape(m_bTriangular);

		// reposition the grip
		CRect rect;
		GetWindowRect(rect);
		rect.InflateRect(m_size.cx - sizeOld.cx, m_size.cy - sizeOld.cy, 0, 0);
		::MapWindowPoints(NULL, GetParent().m_hWnd, (LPPOINT)&rect, 2);
		MoveWindow(rect, TRUE);
	}

	void OnDestroy()
	{
		// perform clean up
		if (m_bTransparent)
			SetTransparency(FALSE);
		SetMsgHandled(FALSE);
	}
	void OnPrintClient(HDC hDC, UINT options)
	{
		if (!(options & PRF_CHECKVISIBLE) && !IsWindowVisible())
			return;
		OnPaint(hDC);
	}
	void DoPaint(HDC hDC)
	{
		CDCHandle dc(hDC);
		// select bitmaps
		HBITMAP hbmpOldGrip, hbmpOldMask;
		hbmpOldGrip = m_dcGrip.SelectBitmap(m_bmGrip);
		hbmpOldMask = m_dcMask.SelectBitmap(m_bmMask);

		// obtain original grip bitmap, make the mask and prepare masked bitmap

		DefWindowProc(WM_PAINT, (WPARAM)m_dcGrip.m_hDC, NULL);
		m_dcGrip.SetBkColor(m_dcGrip.GetPixel(0, 0));
		m_dcMask.BitBlt(0, 0, m_size.cx, m_size.cy, m_dcGrip, 0, 0, SRCCOPY);
		m_dcGrip.BitBlt(0, 0, m_size.cx, m_size.cy, m_dcMask, 0, 0, 0x00220326);
			
		// draw transparently
		dc.BitBlt(0, 0, m_size.cx, m_size.cy, m_dcMask, 0, 0, SRCAND);
		dc.BitBlt(0, 0, m_size.cx, m_size.cy, m_dcGrip, 0, 0, SRCPAINT);

		// unselect bitmaps
		m_dcGrip.SelectBitmap(hbmpOldGrip);
		m_dcMask.SelectBitmap(hbmpOldMask);
	}
	void OnPaint(HDC hDC)
	{
		if (!m_bTransparent)
		{
			SetMsgHandled(FALSE);
			return;
		}
		if (hDC)
			DoPaint(hDC);
		else
		{
			CPaintDC dc(m_hWnd);
			DoPaint(dc);
		}
	}
protected:
	BOOL m_bTriangular;		// triangular shape active
	BOOL m_bTransparent;	// transparency active

	// memory DCs and bitmaps for transparent grip
	CDC m_dcGrip, m_dcMask;
	CBitmap m_bmGrip, m_bmMask;
};
///////////////////////////////////////////////////////////////////////////////
// Control itself
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
// Template for parent
///////////////////////////////////////////////////////////////////////////////
template <class T>
class CResizableGrip
{
public:
	CResizableGrip()
	{
		m_nShowCount = 0;
	}
	virtual ~CResizableGrip()
	{
	};
	// create a size grip, with options
	BOOL CreateSizeGrip(BOOL bVisible = TRUE,
		BOOL bTriangular = TRUE, BOOL bTransparent = FALSE)
	{
		T* pT = static_cast<T*>(this);
		// create grip
		CRect rect(0 , 0, m_wndGrip.m_size.cx, m_wndGrip.m_size.cy);
		BOOL bRet = m_wndGrip.Create(pT->m_hWnd, rect)!=NULL;

		if (bRet)
		{
			// set options
			m_wndGrip.SetTriangularShape(bTriangular);
			m_wndGrip.SetTransparency(bTransparent);
			SetSizeGripVisibility(bVisible);

			// update position
			UpdateSizeGrip();
		}

		return bRet;
	}

	BOOL IsSizeGripVisible()	// TRUE if grip is set to be visible
	{
		// NB: visibility is effective only after an update
		return (m_nShowCount > 0);
	}

	void SetSizeGripVisibility(BOOL bVisible)	// set default visibility
	{
		if (bVisible)
			m_nShowCount = 1;
		else
			m_nShowCount = 0;
	}

	void UpdateSizeGrip()		// update the grip's visibility and position
	{
		T* pT = static_cast<T*>(this);
		ATLASSERT(pT->IsWindow());
		// size-grip goes bottom right in the client area
		// (any right-to-left adjustment should go here)
		CRect rect;
		pT->GetClientRect(&rect);

		rect.left = rect.right - m_wndGrip.m_size.cx;
		rect.top = rect.bottom - m_wndGrip.m_size.cy;

		// must stay below other children
		m_wndGrip.SetWindowPos(HWND_BOTTOM, rect.left, rect.top, 0, 0,
			SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREPOSITION
			| (IsSizeGripVisible() ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));
	}

// pbStatus points to a variable, maintained by the caller, that
// holds its visibility status. Initialize the variable with 1
// to allow to temporarily hide the grip, 0 to allow to
// temporarily show the grip (with respect to the dwMask bit).

// NB: visibility is effective only after an update
	void ShowSizeGrip(DWORD* pStatus, DWORD dwMask = 1)	// temp show the size grip
	{
		ATLASSERT(pStatus != NULL);
		if (!(*pStatus & dwMask))
		{
			m_nShowCount++;
			(*pStatus) |= dwMask;
		}
	}
	void HideSizeGrip(DWORD* pStatus, DWORD dwMask = 1)	// temp hide the size grip
	{
		ATLASSERT(pStatus != NULL);

		if (*pStatus & dwMask)
		{
			m_nShowCount--;
			(*pStatus) &= ~dwMask;
		}
	}

	BOOL SetSizeGripBkMode(int nBkMode)		// like CDC::SetBkMode
	{
		if (::IsWindow(m_wndGrip.m_hWnd))
		{
			if (nBkMode == OPAQUE)
				m_wndGrip.SetTransparency(FALSE);
			else if (nBkMode == TRANSPARENT)
				m_wndGrip.SetTransparency(TRUE);
			else
				return FALSE;
			return TRUE;
		}
		return FALSE;
	}

	void SetSizeGripShape(BOOL bTriangular)
	{
		m_wndGrip.SetTriangularShape(bTriangular);
	}

	CSizeGrip m_wndGrip;		// grip control
	int m_nShowCount;			// support for hiding the grip
};
///////////////////////////////////////////////////////////////////////////////
// Template for parent
///////////////////////////////////////////////////////////////////////////////
