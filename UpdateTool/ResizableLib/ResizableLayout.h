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

#include "ResizableMsgSupport.h"
#include "atlcoll.h"

#define _BS_TYPEMASK 0x0000000FL


// useful compatibility constants (the only one required is NOANCHOR)

const CSize NOANCHOR(-1,-1),
	TOP_LEFT(0,0), TOP_CENTER(50,0), TOP_RIGHT(100,0),
	MIDDLE_LEFT(0,50), MIDDLE_CENTER(50,50), MIDDLE_RIGHT(100,50),
	BOTTOM_LEFT(0,100), BOTTOM_CENTER(50,100), BOTTOM_RIGHT(100,100);


class LayoutInfo
{
public:
	HWND hWnd;
	UINT nCallbackID;

	TCHAR sWndClass[MAX_PATH];

	// upper-left corner
	SIZE anchorTL;
	SIZE marginTL;
	SIZE marginStartupTL;

	// bottom-right corner
	SIZE anchorBR;
	SIZE marginBR;
	SIZE marginStartupBR;

	// custom window support
	BOOL bMsgSupport;
	RESIZEPROPERTIES properties;

public:
	LayoutInfo() : hWnd(NULL), nCallbackID(0), bMsgSupport(FALSE)
	{ 
		sWndClass[0] = 0;
	}

	LayoutInfo(HWND hwnd, SIZE tl_t, SIZE tl_m, 
		SIZE br_t, SIZE br_m)
		: hWnd(hwnd), nCallbackID(0),
		bMsgSupport(FALSE),
		anchorTL(tl_t), marginTL(tl_m),
		anchorBR(br_t), marginBR(br_m)
	{ 
		sWndClass[0] = 0;
	}
};


template <class T>
class CResizableLayout
{
private:
	// list of repositionable controls
	CAtlMap<HWND, POSITION> m_mapLayout;
	CAtlMap<HWND, POSITION> m_mapLayoutCB;
	CAtlList<LayoutInfo> m_listLayout;
	CAtlList<LayoutInfo> m_listLayoutCB;

	HRGN m_hOldClipRgn;
	int m_nOldClipRgn;

	RECT m_rectClientBefore;
	BOOL m_bNoRecursion;

	void ClipChildWindow(const LayoutInfo &layout, CRgn &Region);
	void CalcNewChildPosition(const LayoutInfo &layout,
		const CRect &rectParent, CRect &rectChild, UINT& uFlags);
public:
	// override to initialize resize properties (clipping, refresh)
	void InitResizeProperties(LayoutInfo& layout);

	// override to specify clipping for unsupported windows
	BOOL LikesClipping(const LayoutInfo &layout);

	// override to specify refresh for unsupported windows
	BOOL NeedsRefresh(const LayoutInfo &layout,
		const CRect &rectOld, const CRect &rectNew);

	// clip out child windows from the given DC (support old code)
	BOOL ClipChildren(HDC hDC, BOOL bUndo);

	// get the clipping region (without clipped child windows)
	void GetClippingRegion(CRgn &Region);

	// override for scrollable or expanding parent windows
	void GetTotalClientRect(LPRECT lpRect);

	// add anchors to a control, given its HWND
	void AddAnchor(HWND hWnd, CSize anchorTL, CSize anchorBR = NOANCHOR);

	// add anchors to a control, given its ID
	void AddAnchor(UINT nID, CSize anchorTL, CSize anchorBR = NOANCHOR)
	{
		T* pT = static_cast<T*>(this);
		AddAnchor(pT->GetDlgItem(nID), anchorTL, anchorBR);
	}
	// add anchors for All other controls
	void AddOtherChildAnchors() 
	{
		T* pT = static_cast<T*>(this);
		HWND hWndChild = ::GetWindow(pT->m_hWnd, GW_CHILD);    
		while (hWndChild != NULL)    
		{        
			__if_exists(T::m_wndGrip)
			{
				if (hWndChild != pT->m_wndGrip.m_hWnd) // check window class/name?            
			}
				AddAnchor(hWndChild, TOP_LEFT);        
				hWndChild = ::GetWindow(hWndChild, GW_HWNDNEXT);    
		}
	}
	// add a callback (control ID or HWND is unknown or may change)
	UINT AddAnchorCallback(UINT nCallbackID = -1);
	UINT AddAnchorCallback(UINT nCallbackID, CSize anchorTL, CSize anchorBR = NOANCHOR);
	void RemoveAnchorCallback(UINT nCallbackID)
	{
		T* pT = static_cast<T*>(this);
		HWND hWnd = pT->GetDlgItem(nCallbackID);
		POSITION pos;
		if (m_mapLayoutCB.Lookup(hWnd, pos))
		{
			m_listLayoutCB.RemoveAt(pos);
			m_mapLayoutCB.RemoveKey(hWnd);
		}
		else
		{
			pos = m_listLayoutCB.GetHeadPosition();
			while (pos)
			{
				LayoutInfo &info = m_listLayoutCB.GetAt(pos);
				if (info.nCallbackID == nCallbackID)
				{
					m_listLayoutCB.RemoveAt(pos);
					return;
				}
				m_listLayoutCB.GetNext(pos);
			}
		}
	}

	// get rect of an anchored window, given the parent's client area
	BOOL GetAnchorPosition(HWND hWnd, const CRect &rectParent,
		CRect &rectChild, UINT* lpFlags = NULL)
	{
		POSITION pos;
		if (!m_mapLayout.Lookup(hWnd, pos))
			return FALSE;

		UINT uTmpFlags;
		CalcNewChildPosition(m_listLayout.GetAt(pos), rectParent, rectChild,
			(lpFlags != NULL) ? (*lpFlags) : uTmpFlags);
		return TRUE;
	}

	// get rect of an anchored window, given the parent's client area
	BOOL GetAnchorPosition(UINT nID, const CRect &rectParent,
		CRect &rectChild, UINT* lpFlags = NULL)
	{
		T* pT = static_cast<T*>(this);
		return pT->GetAnchorPosition(pT->GetDlgItem(nID),
			rectParent, rectChild, lpFlags);
	}

	BOOL GetAnchorMargins(HWND hWnd, const CSize &sizeChild, CRect &rectMargins);
	BOOL GetAnchorMargins(UINT nID, const CSize &sizeChild, CRect &rectMargins)
	{
		return GetAnchorMargins(::GetDlgItem(GetResizableWnd()->GetSafeHwnd(), nID),
			sizeChild, rectMargins);
	}
	BOOL GetAnchorStartupMargin(HWND hWnd, CSize &marginTL, CSize &marginBR);
	BOOL GetAnchorStartupMargin(UINT nID, CSize &marginTL, CSize &marginBR)
	{
		T* pT = static_cast<T*>(this);
		return GetAnchorStartupMargin(pT->GetDlgItem(nID), marginTL, marginBR);
	}

	// remove an anchored control from the layout, given its HWND
	BOOL RemoveAnchor(HWND hWnd)
	{
		POSITION pos;
		if (!m_mapLayout.Lookup(hWnd, pos))
			return FALSE;

		m_listLayout.RemoveAt(pos);
		return m_mapLayout.RemoveKey(hWnd);
	}

	// remove an anchored control from the layout, given its HWND
	BOOL RemoveAnchor(UINT nID)
	{
		T* pT = static_cast<T*>(this);
		return RemoveAnchor(pT->GetDlgItem(nID));
	}

	// reset layout content
	void RemoveAllAnchors()
	{
		m_mapLayout.RemoveAll();
		m_mapLayoutCB.RemoveAll();
		m_listLayout.RemoveAll();
		m_listLayoutCB.RemoveAll();
	}

	// adjust children's layout, when parent's size changes
	void ArrangeLayout();

	// override to provide dynamic control's layout info
	BOOL ArrangeLayoutCallback(LayoutInfo& layout);

	//! @brief Enhance anti-flickering
	void HandleNcCalcSize(BOOL bAfterDefault, LPNCCALCSIZE_PARAMS lpncsp, LRESULT& lResult);
	
	//! @brief Enable resizable style for top level parent windows
	void MakeResizable(LPCREATESTRUCT lpCreateStruct);
	void MakeResizable();
public:
	CResizableLayout() 
	{ 
		m_bNoRecursion = FALSE;
		m_hOldClipRgn = ::CreateRectRgn(0,0,0,0);
		m_bPrevIsNcCalcSize = FALSE;
	}

	~CResizableLayout()
	{
		if (m_hOldClipRgn)
			::DeleteObject((HGDIOBJ)m_hOldClipRgn);
		// just for safety
		RemoveAllAnchors();
	}

	BOOL m_bPrevIsNcCalcSize;
	LPARAM m_lParamNcCalcSize;
	BEGIN_MSG_MAP(CResizableLayout<T>)
		if (uMsg == WM_NCCALCSIZE && !m_bPrevIsNcCalcSize && wParam!=0)
		{
			T* pT = static_cast<T*>(this);
			lResult = 0;
			m_lParamNcCalcSize = lParam;
			pT->HandleNcCalcSize(FALSE, (LPNCCALCSIZE_PARAMS)m_lParamNcCalcSize, lResult);
			m_bPrevIsNcCalcSize = TRUE;
			bHandled = FALSE;
		}
		if (uMsg == WM_NCCALCSIZE && m_bPrevIsNcCalcSize && wParam!=0)
		{
			T* pT = static_cast<T*>(this);
			pT->HandleNcCalcSize(TRUE, (LPNCCALCSIZE_PARAMS)m_lParamNcCalcSize, lResult);
			m_bPrevIsNcCalcSize = FALSE;
		}
	END_MSG_MAP()
};


///////////////////////////////////////////////////////////////////////////////
// CResizableLayout<T>::AddAnchor
///////////////////////////////////////////////////////////////////////////////
template <class T>
inline void CResizableLayout<T>::AddAnchor(HWND hWnd, CSize anchorTL, CSize anchorBR)
{
	T* pT = static_cast<T*>(this);

	// child window must be valid
	ATLASSERT(::IsWindow(hWnd));
	// must be child of parent window
	ATLASSERT(::IsChild(pT->m_hWnd, hWnd));
	// top-left anchor must be valid
	ATLASSERT(anchorTL != NOANCHOR);
	// get parent window's rect
	CRect rectParent;
	pT->GetTotalClientRect(&rectParent);
	// and child control's rect
	CRect rectChild;
	::GetWindowRect(hWnd, &rectChild);
	::MapWindowPoints(NULL, pT->m_hWnd, (LPPOINT)&rectChild, 2);

	// adjust position, if client area has been scrolled
	rectChild.OffsetRect(-rectParent.TopLeft());

	// go calculate margins
	CSize marginTL, marginBR;

	if (anchorBR == NOANCHOR)
		anchorBR = anchorTL;
	
	// calculate margin for the top-left corner

	marginTL.cx = rectChild.left - rectParent.Width() * anchorTL.cx / 100;
	marginTL.cy = rectChild.top - rectParent.Height() * anchorTL.cy / 100;
	
	// calculate margin for the bottom-right corner

	marginBR.cx = rectChild.right - rectParent.Width() * anchorBR.cx / 100;
	marginBR.cy = rectChild.bottom - rectParent.Height() * anchorBR.cy / 100;

	// prepare the structure
	LayoutInfo layout(hWnd, anchorTL, marginTL,
		anchorBR, marginBR);
	// get control's window class
	::GetClassName(hWnd, layout.sWndClass, MAX_PATH);

	// initialize resize properties (overridable)
	pT->InitResizeProperties(layout);

	// must not be already there!
	// (this is probably due to a duplicate call to AddAnchor)
	POSITION pos;
	if (m_mapLayout.Lookup(hWnd, pos))
		m_listLayout.SetAt(pos, layout);
	else
	{
		// add to the list and the map
		pos = m_listLayout.AddTail(layout);
		m_mapLayout.SetAt(hWnd, pos);
	}
}
///////////////////////////////////////////////////////////////////////////////
// CResizableLayout<T>::AddAnchor
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// CResizableLayout<T>::AddAnchorCallback
///////////////////////////////////////////////////////////////////////////////


template <class T>
inline UINT CResizableLayout<T>::AddAnchorCallback(UINT nCallbackID, CSize anchorTL, CSize anchorBR )
{
	T* pT = static_cast<T*>(this);
	HWND hWnd = pT->GetDlgItem(nCallbackID);
	// child window must be valid
	ATLASSERT(::IsWindow(hWnd));
	// must be child of parent window
	ATLASSERT(::IsChild(pT->m_hWnd, hWnd));
	// top-left anchor must be valid
	// get parent window's rect
	CRect rectParent;
	pT->GetTotalClientRect(&rectParent);
	// and child control's rect
	CRect rectChild;
	::GetWindowRect(hWnd, &rectChild);
	::MapWindowPoints(NULL, pT->m_hWnd, (LPPOINT)&rectChild, 2);

	// adjust position, if client area has been scrolled
	rectChild.OffsetRect(-rectParent.TopLeft());

	if (anchorBR == NOANCHOR)
		anchorBR = anchorTL;
	
	LayoutInfo layout;
	if (nCallbackID == -1)
		nCallbackID = m_listLayoutCB.GetCount()+1;
	layout.nCallbackID = nCallbackID;
	layout.anchorTL = anchorTL;
	layout.anchorBR = anchorBR;

	// calculate margin 
	layout.marginStartupTL.cx = rectChild.left - rectParent.Width() * anchorTL.cx / 100;
	layout.marginStartupTL.cy = rectChild.top - rectParent.Height() * anchorTL.cy / 100;
	layout.marginStartupBR.cx = rectChild.right - rectParent.Width() * anchorBR.cx / 100;
	layout.marginStartupBR.cy = rectChild.bottom - rectParent.Height() * anchorBR.cy / 100;

	// one callback control cannot rely upon another callback control's
	// size and/or position (they're updated all together at the end)
	// it can however use a non-callback control, which is updated before
	// add to the list
	RemoveAnchor(nCallbackID);
	RemoveAnchorCallback(nCallbackID);

	POSITION pos;
	ATLASSERT(!m_mapLayoutCB.Lookup(hWnd, pos));
	pos = m_listLayoutCB.AddTail(layout);
	m_mapLayoutCB.SetAt(hWnd, pos);
	return layout.nCallbackID;
}





template <class T>
inline UINT CResizableLayout<T>::AddAnchorCallback(UINT nCallbackID)
{
	// one callback control cannot rely upon another callback control's
	// size and/or position (they're updated all together at the end)
	// it can however use a non-callback control, which is updated before
	// add to the list
	LayoutInfo layout;
	if (nCallbackID == -1)
		nCallbackID = m_listLayoutCB.GetCount()+1;
	RemoveAnchor(nCallbackID);
	layout.nCallbackID = nCallbackID;
	m_listLayoutCB.AddTail(layout);
	return layout.nCallbackID;
}
///////////////////////////////////////////////////////////////////////////////
// CResizableLayout<T>::AddAnchorCallback
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// CResizableLayout<T>::GetAnchorMargins
///////////////////////////////////////////////////////////////////////////////
template <class T>
inline BOOL CResizableLayout<T>::GetAnchorMargins(HWND hWnd, const CSize &sizeChild, CRect &rectMargins)
{
	POSITION pos;
	if (!m_mapLayout.Lookup(hWnd, pos))
		return FALSE;

	const LayoutInfo& layout = m_listLayout.GetAt(pos);

	// augmented size, relative to anchor points
	CSize size = sizeChild + layout.marginTL - layout.marginBR;

	// percent of parent size occupied by this control
	CSize percent(layout.anchorBR.cx - layout.anchorTL.cx,
		layout.anchorBR.cy - layout.anchorTL.cy);

	// calculate total margins
	rectMargins.left = size.cx * layout.anchorTL.cx / percent.cx + layout.marginTL.cx;
	rectMargins.top = size.cy * layout.anchorTL.cy / percent.cy + layout.marginTL.cy;
	rectMargins.right = size.cx * (100 - layout.anchorBR.cx) / percent.cx - layout.marginBR.cx;
	rectMargins.bottom = size.cy * (100 - layout.anchorBR.cy) / percent.cy - layout.marginBR.cy;

	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
// CResizableLayout<T>::GetAnchorMargins
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// CResizableLayout<T>::GetAnchorStartupMargin
///////////////////////////////////////////////////////////////////////////////
template <class T>
inline BOOL CResizableLayout<T>::GetAnchorStartupMargin(HWND hWnd, CSize &marginTL, CSize &marginBR)
{
	T* pT = static_cast<T*>(this);
	
	POSITION pos;
	if (!m_mapLayoutCB.Lookup(hWnd, pos))
		return FALSE;

	LayoutInfo &layout = m_listLayout.GetAt(pos);
	marginBR = layout.marginStartupBR;
	marginTL = layout.marginStartupTL;

	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
// CResizableLayout<T>::GetAnchorStartupMargin
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CResizableLayout<T>::ArrangeLayoutCallback
///////////////////////////////////////////////////////////////////////////////
template <class T>
inline BOOL CResizableLayout<T>::ArrangeLayoutCallback(LayoutInfo& /*layout*/)
{
	ATLASSERT(FALSE);
	// must be overridden, if callback is used
	
	return FALSE;	// no output data
}
///////////////////////////////////////////////////////////////////////////////
// CResizableLayout<T>::ArrangeLayoutCallback
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// CResizableLayout<T>::ArrangeLayout
///////////////////////////////////////////////////////////////////////////////
template <class T>
inline void CResizableLayout<T>::ArrangeLayout()
{
	T* pT = static_cast<T*>(this);
	// common vars
	UINT uFlags;
	LayoutInfo layout;
	CRect rectParent, rectChild;
	int count = m_listLayout.GetCount();
	int countCB = m_listLayoutCB.GetCount();

	if (count + countCB == 0)
		return; // nothing to do

	// get parent window's rect
	pT->GetTotalClientRect(&rectParent); 

	// reposition child windows
	HDWP hdwp = ::BeginDeferWindowPos(count + countCB);
	
	POSITION pos = m_listLayout.GetHeadPosition();
	while (pos != NULL)
	{
		// get layout info
		layout = m_listLayout.GetNext(pos);
		
		// calculate new child's position, size and flags for SetWindowPos
		CalcNewChildPosition(layout, rectParent, rectChild, uFlags);

		// only if size or position changed
		if ((uFlags & (SWP_NOMOVE|SWP_NOSIZE)) != (SWP_NOMOVE|SWP_NOSIZE))
		{
			hdwp = ::DeferWindowPos(hdwp, layout.hWnd, NULL, rectChild.left,
				rectChild.top, rectChild.Width(), rectChild.Height(), uFlags);
		}
	}

	// for callback items you may use GetAnchorPosition to know the
	// new position and size of a non-callback item after resizing

	pos = m_listLayoutCB.GetHeadPosition();
	while (pos != NULL)
	{
		// get layout info
		layout = m_listLayoutCB.GetNext(pos);
		// request layout data
		if (!pT->ArrangeLayoutCallback(layout))
			continue;

		// calculate new child's position, size and flags for SetWindowPos
		CalcNewChildPosition(layout, rectParent, rectChild, uFlags);

		// only if size or position changed
		if ((uFlags & (SWP_NOMOVE|SWP_NOSIZE)) != (SWP_NOMOVE|SWP_NOSIZE))
		{
			hdwp = ::DeferWindowPos(hdwp, layout.hWnd, NULL, rectChild.left,
				rectChild.top, rectChild.Width(), rectChild.Height(), uFlags);
		}
	}

	// finally move all the windows at once
	::EndDeferWindowPos(hdwp);
}
///////////////////////////////////////////////////////////////////////////////
// CResizableLayout<T>::ArrangeLayout
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// CResizableLayout<T>::ClipChildWindow
///////////////////////////////////////////////////////////////////////////////
template <class T>
inline void CResizableLayout<T>::ClipChildWindow(const LayoutInfo& layout,
									   CRgn &Region)
{
	T* pT = static_cast<T*>(this);
	// obtain window position
	CRect rect;
	::GetWindowRect(layout.hWnd, &rect);
#if (_WIN32_WINNT >= 0x0501)
	//! @todo decide when to clip client only or non-client too (themes?)
	//! (leave disabled meanwhile, until I find a good solution)
	//! @note wizard97 with watermark bitmap and themes won't look good!
	// if (Get_real_WIN32_WINNT >= 0x501)
	//	::SendMessage(layout.hWnd, WM_NCCALCSIZE, FALSE, (LPARAM)&rect);
#endif
	::MapWindowPoints(NULL, pT->m_hWnd, (LPPOINT)&rect, 2);

	// use window region if any
	CRgn rgn;
	rgn.CreateRectRgn(0,0,0,0);
	switch (::GetWindowRgn(layout.hWnd, rgn))
	{
	case COMPLEXREGION:
	case SIMPLEREGION:
		rgn.OffsetRgn(rect.TopLeft());
		break;

	default:
		rgn.SetRectRgn(&rect);
	}

	// get the clipping property
	BOOL bClipping = layout.properties.bAskClipping ?
		pT->LikesClipping(layout) : layout.properties.bCachedLikesClipping;

	// modify region accordingly
	if (bClipping)
		Region.CombineRgn(Region, rgn, RGN_DIFF);
	else
		Region.CombineRgn(Region, rgn, RGN_OR);
}
///////////////////////////////////////////////////////////////////////////////
// CResizableLayout<T>::ClipChildWindow
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// CResizableLayout<T>::GetClippingRegion
///////////////////////////////////////////////////////////////////////////////
template <class T>
inline void CResizableLayout<T>::GetClippingRegion(CRgn &Region)
{
	T* pT = static_cast<T*>(this);

	// System's default clipping area is screen's size,
	// not enough for max track size, for example:
	// if screen is 1024 x 768 and resizing border is 4 pixels,
	// maximized size is 1024+4*2=1032 x 768+4*2=776,
	// but max track size is 4 pixels bigger 1036 x 780 (don't ask me why!)
	// So, if you resize the window to maximum size, the last 4 pixels
	// are clipped out by the default clipping region, that gets created
	// as soon as you call clipping functions (my guess).

	// reset clipping region to the whole client area
	CRect rect;
	pT->GetClientRect(&rect);
	Region.CreateRectRgnIndirect(&rect);

	// clip only anchored controls
	LayoutInfo layout;
	POSITION pos = m_listLayout.GetHeadPosition();
	while (pos != NULL)
	{
		// get layout info
		layout = m_listLayout.GetNext(pos);
		
		if (::IsWindowVisible(layout.hWnd))
			ClipChildWindow(layout, Region);
	}
	pos = m_listLayoutCB.GetHeadPosition();
	while (pos != NULL)
	{
		// get layout info
		layout = m_listLayoutCB.GetNext(pos);
		// request data
		if (!pT->ArrangeLayoutCallback(layout))
			continue;

		if (::IsWindowVisible(layout.hWnd))
			ClipChildWindow(layout, Region);
	}
//! @todo Has XP changed this??? It doesn't seem correct anymore!
/*
	// fix for RTL layouts (1 pixel of horz offset)
	if (pWnd->GetExStyle() & WS_EX_LAYOUTRTL)
		pRegion->OffsetRgn(-1,0);
*/
}
///////////////////////////////////////////////////////////////////////////////
// CResizableLayout<T>::GetClippingRegion
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// ::GetRootParentWnd
///////////////////////////////////////////////////////////////////////////////
inline HWND GetRootParentWnd(HWND hWnd)
{
	// GetAncestor API not present, emulate
	if (!(::GetWindowLong(hWnd, GWL_STYLE) & WS_CHILD))
		return NULL;
	while (::GetWindowLong(hWnd, GWL_STYLE) & WS_CHILD)
		hWnd = ::GetParent(hWnd);
	return hWnd;
}
///////////////////////////////////////////////////////////////////////////////
// ::GetRootParentWnd
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// CResizableLayout<T>::ClipChildren
///////////////////////////////////////////////////////////////////////////////
// support legacy code (will disappear in future versions)
template <class T>
inline BOOL CResizableLayout<T>::ClipChildren(HDC hDC, BOOL bUndo)
{
	T* pT = static_cast<T*>(this);
#if (_WIN32_WINNT >= 0x0501 && !defined(RSZLIB_NO_XP_DOUBLE_BUFFER))
	// clipping not necessary when double-buffering enabled
	if (Get_real_WIN32_WINNT() >= 0x0501)
	{
		HWND hWnd = GetRootParentWnd(pT->m_hWnd);
		if (hWnd == NULL)
			hWnd = pT->m_hWnd;
		if (::GetWindowLong(hWnd, GWL_EXSTYLE) & WS_EX_COMPOSITED)
			return FALSE;
	}
#endif

	HWND hWnd = pT->m_hWnd;

	m_nOldClipRgn = -1; // invalid region by default

	// Some controls (such as transparent toolbars and standard controls
	// with XP theme enabled) send a WM_ERASEBKGND msg to the parent
	// to draw themselves, in which case we must not enable clipping.

	// We check that the window associated with the DC is the
	// resizable window and not a child control.

	if (!bUndo && (hWnd == ::WindowFromDC(hDC)))
	{
		// save old DC clipping region
		m_nOldClipRgn = ::GetClipRgn(hDC, m_hOldClipRgn);

		// clip out supported child windows
		CRgn rgnClip;
		GetClippingRegion(rgnClip);
		::ExtSelectClipRgn(hDC, rgnClip, RGN_AND);

		return TRUE;
	}

	// restore old clipping region, only if modified and valid
	if (bUndo && m_nOldClipRgn >= 0)
	{
		if (m_nOldClipRgn == 1)
			::SelectClipRgn(hDC, m_hOldClipRgn);
		else
			::SelectClipRgn(hDC, NULL);
		
		return TRUE;
	}

	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////
// CResizableLayout<T>::ClipChildren
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// CResizableLayout<T>::GetTotalClientRect
///////////////////////////////////////////////////////////////////////////////
template <class T>
inline void CResizableLayout<T>::GetTotalClientRect(LPRECT lpRect)
{
	T* pT = static_cast<T*>(this);
	pT->GetClientRect(lpRect);
}
///////////////////////////////////////////////////////////////////////////////
// CResizableLayout<T>::GetTotalClientRect
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// CResizableLayout<T>::NeedsRefresh
///////////////////////////////////////////////////////////////////////////////
template <class T>
inline BOOL CResizableLayout<T>::NeedsRefresh(const LayoutInfo& layout,
									const CRect& rectOld, const CRect& rectNew)
{
	T* pT = static_cast<T*>(this);

	if (layout.bMsgSupport)
	{
		REFRESHPROPERTY refresh;
		refresh.rcOld = rectOld;
		refresh.rcNew = rectNew;
		if (Send_NeedsRefresh(layout.hWnd, &refresh))
			return refresh.bNeedsRefresh;
	}

	int nDiffWidth = (rectNew.Width() - rectOld.Width());
	int nDiffHeight = (rectNew.Height() - rectOld.Height());

	// is the same size?
	if (nDiffWidth == 0 && nDiffHeight == 0)
		return FALSE;

	// optimistic, no need to refresh
	BOOL bRefresh = FALSE;

	// window classes that need refresh when resized
	if (layout.sWndClass == WC_STATIC)
	{
		DWORD style = ::GetWindowLong(layout.hWnd, GWL_STYLE);

		switch (style & SS_TYPEMASK)
		{
		case SS_LEFT:
		case SS_CENTER:
		case SS_RIGHT:
			// word-wrapped text
			bRefresh = bRefresh || (nDiffWidth != 0);
			// vertically centered text
			if (style & SS_CENTERIMAGE)
				bRefresh = bRefresh || (nDiffHeight != 0);
			break;

		case SS_LEFTNOWORDWRAP:
			// text with ellipsis
			if (style & SS_ELLIPSISMASK)
				bRefresh = bRefresh || (nDiffWidth != 0);
			// vertically centered text
			if (style & SS_CENTERIMAGE)
				bRefresh = bRefresh || (nDiffHeight != 0);
			break;

		case SS_ENHMETAFILE:
		case SS_BITMAP:
		case SS_ICON:
			// images
		case SS_BLACKFRAME:
		case SS_GRAYFRAME:
		case SS_WHITEFRAME:
		case SS_ETCHEDFRAME:
			// and frames
			bRefresh = TRUE;
			break;
		}
	}

	// window classes that don't redraw client area correctly
	// when the hor scroll pos changes due to a resizing
	BOOL bHScroll = FALSE;
	if (layout.sWndClass == WC_LISTBOX)
		bHScroll = TRUE;

	// fix for horizontally scrollable windows
	if (bHScroll && (nDiffWidth > 0))
	{
		// get max scroll position
		SCROLLINFO info;
		info.cbSize = sizeof(SCROLLINFO);
		info.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
		if (::GetScrollInfo(layout.hWnd, SB_HORZ, &info))
		{
			// subtract the page size
			info.nMax -= __max(info.nPage-1,0);
		}

		// resizing will cause the text to scroll on the right
		// because the scrollbar is going beyond the right limit
		if ((info.nMax > 0) && (info.nPos + nDiffWidth > info.nMax))
		{
			// needs repainting, due to horiz scrolling
			bRefresh = TRUE;
		}
	}
	return bRefresh;
}
///////////////////////////////////////////////////////////////////////////////
// CResizableLayout<T>::NeedsRefresh
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// CResizableLayout<T>::LikesClipping
///////////////////////////////////////////////////////////////////////////////
template <class T>
inline BOOL CResizableLayout<T>::LikesClipping(const LayoutInfo& layout)
{
	T* pT = static_cast<T*>(this);
	if (layout.bMsgSupport)
	{
		CLIPPINGPROPERTY clipping;
		if (Send_LikesClipping(layout.hWnd, &clipping))
			return clipping.bLikesClipping;
	}

	DWORD style = ::GetWindowLong(layout.hWnd, GWL_STYLE);

	// skip windows that wants background repainted
	if (!lstrcmp(layout.sWndClass, TOOLBARCLASSNAME) && (style & TBSTYLE_TRANSPARENT))
		return FALSE;
	else if (!lstrcmp(layout.sWndClass, WC_BUTTON))
	{
		CRect rect;
		switch (style & _BS_TYPEMASK)
		{
		case BS_GROUPBOX:
			return FALSE;

		case BS_OWNERDRAW:
			// ownerdraw buttons must return correct hittest code
			// to notify their transparency to the system and this library
			::GetWindowRect(layout.hWnd, &rect);
			if ( HTTRANSPARENT == ::SendMessage(layout.hWnd,
				WM_NCHITTEST, 0, MAKELPARAM(rect.left, rect.top)) )
				return FALSE;
			break;
		}
		return TRUE;
	}
	else if (layout.sWndClass == WC_STATIC)
	{
		switch (style & SS_TYPEMASK)
		{
		case SS_LEFT:
		case SS_CENTER:
		case SS_RIGHT:
		case SS_SIMPLE:
		case SS_LEFTNOWORDWRAP:
			// text
		case SS_BLACKRECT:
		case SS_GRAYRECT:
		case SS_WHITERECT:
			// filled rects
		case SS_ETCHEDHORZ:
		case SS_ETCHEDVERT:
			// etched lines
		case SS_BITMAP:
			// bitmaps
			return TRUE;
			break;

		case SS_ICON:
		case SS_ENHMETAFILE:
			if (style & SS_CENTERIMAGE)
				return FALSE;
			return TRUE;
			break;

		default:
			return FALSE;
		}
	}

	// assume the others like clipping
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////
// CResizableLayout<T>::LikesClipping
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// CResizableLayout<T>::CalcNewChildPosition
///////////////////////////////////////////////////////////////////////////////
template <class T>
inline void CResizableLayout<T>::CalcNewChildPosition(const LayoutInfo& layout,
								const CRect &rectParent, CRect &rectChild, UINT& uFlags)
{
	T* pT = static_cast<T*>(this);

	::GetWindowRect(layout.hWnd, &rectChild);
	::MapWindowPoints(NULL, pT->m_hWnd, (LPPOINT)&rectChild, 2);
	
	CRect rectNew;

	// calculate new top-left corner
	rectNew.left = layout.marginTL.cx + rectParent.Width() * layout.anchorTL.cx / 100;
	rectNew.top = layout.marginTL.cy + rectParent.Height() * layout.anchorTL.cy / 100;
	
	// calculate new bottom-right corner
	rectNew.right = layout.marginBR.cx + rectParent.Width() * layout.anchorBR.cx / 100;
	rectNew.bottom = layout.marginBR.cy + rectParent.Height() * layout.anchorBR.cy / 100;

	// adjust position, if client area has been scrolled
	rectNew.OffsetRect(rectParent.TopLeft());

	// get the refresh property
	BOOL bRefresh = layout.properties.bAskRefresh ?
		pT->NeedsRefresh(layout, rectChild, rectNew) : layout.properties.bCachedNeedsRefresh;

	// set flags 
	uFlags = SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREPOSITION;
	if (bRefresh)
		uFlags |= SWP_NOCOPYBITS;
	if (rectNew.TopLeft() == rectChild.TopLeft())
		uFlags |= SWP_NOMOVE;
	if (rectNew.Size() == rectChild.Size())
		uFlags |= SWP_NOSIZE;

	// update rect
	rectChild = rectNew;
}
///////////////////////////////////////////////////////////////////////////////
// CResizableLayout<T>::CalcNewChildPosition
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// CResizableLayout<T>::InitResizeProperties
///////////////////////////////////////////////////////////////////////////////
template <class T>
inline void CResizableLayout<T>::InitResizeProperties(LayoutInfo &layout)
{
	T* pT = static_cast<T*>(this);

	// check if custom window supports this library
	// (properties must be correctly set by the window)
	layout.bMsgSupport = Send_QueryProperties(layout.hWnd, &layout.properties);

	// default properties
	if (!layout.bMsgSupport)
	{
		// clipping property is assumed as static
		layout.properties.bAskClipping = FALSE;
		layout.properties.bCachedLikesClipping = pT->LikesClipping(layout);
		// refresh property is assumed as dynamic
		layout.properties.bAskRefresh = TRUE;
	}
}
///////////////////////////////////////////////////////////////////////////////
// CResizableLayout<T>::InitResizeProperties
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// CResizableLayout<T>::MakeResizable
///////////////////////////////////////////////////////////////////////////////
template <class T>
inline void CResizableLayout<T>::MakeResizable(LPCREATESTRUCT lpCreateStruct)
{
	T* pT = static_cast<T*>(this);
	if (lpCreateStruct->style & WS_CHILD)
		return;

#if (_WIN32_WINNT >= 0x0501 && !defined(RSZLIB_NO_XP_DOUBLE_BUFFER))
	// enable double-buffering on supported platforms
	pT->ModifyStyleEx(0, WS_EX_COMPOSITED);
#endif

	if (!(lpCreateStruct->style & WS_THICKFRAME))
	{
		// set resizable style
		pT->ModifyStyle(DS_MODALFRAME, WS_THICKFRAME);
		// keep client area
		CRect rect(CPoint(lpCreateStruct->x, lpCreateStruct->y),
			CSize(lpCreateStruct->cx, lpCreateStruct->cy));
		pT->SendMessage(WM_NCCALCSIZE, FALSE, (LPARAM)&rect);
		// adjust size to reflect new style
		::AdjustWindowRectEx(&rect, pT->GetStyle(),
			::IsMenu(pT->GetMenu()), pT->GetExStyle());
		pT->SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height(),
			SWP_NOSENDCHANGING|SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREPOSITION);
		// update dimensions
		lpCreateStruct->cx = rect.Width();
		lpCreateStruct->cy = rect.Height();
	}
}
///////////////////////////////////////////////////////////////////////////////
// CResizableLayout<T>::MakeResizable
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// CResizableLayout<T>::MakeResizable
///////////////////////////////////////////////////////////////////////////////
template <class T>
inline void CResizableLayout<T>::MakeResizable()
{
	T* pT = static_cast<T*>(this);
	DWORD dwStyle = pT->GetStyle();
	if (dwStyle & WS_CHILD)
		return;

#if (_WIN32_WINNT >= 0x0501 && !defined(RSZLIB_NO_XP_DOUBLE_BUFFER))
	// enable double-buffering on supported platforms
	pT->ModifyStyleEx(0, WS_EX_COMPOSITED);
#endif

	if (!(dwStyle & WS_THICKFRAME))
	{
		// set resizable style
		pT->ModifyStyle(DS_MODALFRAME, WS_THICKFRAME);
		// keep client area
		RECT rect={0};
		pT->GetClientRect(&rect);
		pT->SendMessage(WM_NCCALCSIZE, FALSE, (LPARAM)&rect);
		// adjust size to reflect new style
		::AdjustWindowRectEx(&rect, pT->GetStyle(),
			::IsMenu(pT->GetMenu()), pT->GetExStyle());
		pT->SetWindowPos(NULL, 0, 0, rect.right-rect.left, rect.bottom-rect.top,
			SWP_NOSENDCHANGING|SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOREPOSITION);
	}
}
///////////////////////////////////////////////////////////////////////////////
// CResizableLayout<T>::MakeResizable
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// CResizableLayout<T>::HandleNcCalcSize
///////////////////////////////////////////////////////////////////////////////
template <class T>
inline void CResizableLayout<T>::HandleNcCalcSize(BOOL bAfterDefault, LPNCCALCSIZE_PARAMS lpncsp, LRESULT &lResult)
{
	T* pT = static_cast<T*>(this);
	// prevent useless complication when size is not changing
	// prevent recursion when resetting the window region (see below)
	if ((lpncsp->lppos->flags & SWP_NOSIZE)
#if (_WIN32_WINNT >= 0x0501)
		|| m_bNoRecursion
#endif
		)
		return;

	if (!bAfterDefault)
	{
		// save a copy before default handler gets called
		m_rectClientBefore = lpncsp->rgrc[2];
	}
	else // after default WM_NCCALCSIZE msg processing
	{
		if (lResult != 0)
		{
			// default handler already uses an advanced validation policy, give up
			return;
		}
		// default calculated client rect
		RECT &rectClientAfter = lpncsp->rgrc[0];

		// intersection between old and new client area is to be preserved
		// set source and destination rects to this intersection
		RECT &rectPreserve = lpncsp->rgrc[1];
		::IntersectRect(&rectPreserve, &rectClientAfter, &m_rectClientBefore);
		lpncsp->rgrc[2] = rectPreserve;

		lResult = WVR_VALIDRECTS;

		// FIX: window region must be updated before the result of the
		//		WM_NCCALCSIZE message gets processed by the system,
		//		otherwise the old window region will clip the client
		//		area during the preservation process.
		//		This is especially evident on WinXP when the non-client
		//		area is rendered with Visual Styles enabled and the
		//		windows have a non rectangular region.
#if (_WIN32_WINNT >= 0x0501)
		//! @todo change rt check to only if themed frame. what about custom wnd region?
		if (Get_real_WIN32_WINNT() >= 0x0501)
		{
			DWORD dwStyle = pT->GetStyle();
			if ((dwStyle & (WS_CAPTION|WS_MAXIMIZE)) == WS_CAPTION)
			{
				m_bNoRecursion = TRUE;
				pT->SetWindowRgn(NULL, FALSE);
				m_bNoRecursion = FALSE;
			}
		}
#endif
	}
}
