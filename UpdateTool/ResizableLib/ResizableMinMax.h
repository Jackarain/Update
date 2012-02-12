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

template <class T>
class CResizableMinMax  
{
	typedef CResizableMinMax<T> thisClass;
// Attributes
private:
	// flags
	BOOL m_bUseMaxTrack;
	BOOL m_bUseMinTrack;
	BOOL m_bUseMaxRect;

	POINT m_ptMinTrackSize;		// min tracking size
	POINT m_ptMaxTrackSize;		// max tracking size
	POINT m_ptMaxPos;			// maximized position
	POINT m_ptMaxSize;			// maximized size

public:
	CResizableMinMax();
	virtual ~CResizableMinMax();

	void MinMaxInfo(LPMINMAXINFO lpMMI);

	void ChainMinMaxInfo(LPMINMAXINFO lpMMI, HWND hWndParentFrame, HWND hWnd);

	void ChainMinMaxInfo(LPMINMAXINFO lpMMI, HWND hWndChild, CSize sizeExtra);
	
	void ChainMinMaxInfo(LPMINMAXINFO lpMMI, HWND hWndParent, UINT nID, CSize sizeExtra)
	{
		T* pT = static_cast<T*>(this);
		pT->ChainMinMaxInfo(lpMMI, ::GetDlgItem(hWndParent, nID), sizeExtra);
	}

	void ChainMinMaxInfoCB(LPMINMAXINFO lpMMI, HWND hWndChild);
	virtual BOOL CalcSizeExtra(HWND hWndChild, CSize sizeChild, CSize& sizeExtra);

	void SetMaximizedRect(const CRect& rc);		// set window rect when maximized
	void ResetMaximizedRect();					// reset to default maximized rect
	void SetMinTrackSize(const CSize& size);	// set minimum tracking size
	void ResetMinTrackSize();					// reset to default minimum tracking size
	void SetMaxTrackSize(const CSize& size);	// set maximum tracking size
	void ResetMaxTrackSize();					// reset to default maximum tracking size
};

template <class T>
inline CResizableMinMax<T>::CResizableMinMax()
{
	m_bUseMinTrack = FALSE;
	m_bUseMaxTrack = FALSE;
	m_bUseMaxRect = FALSE;
}

template <class T>
inline CResizableMinMax<T>::~CResizableMinMax()
{
}

template <class T>
inline void CResizableMinMax<T>::MinMaxInfo(LPMINMAXINFO lpMMI)
{
	if (m_bUseMinTrack)
		lpMMI->ptMinTrackSize = m_ptMinTrackSize;

	if (m_bUseMaxTrack)
		lpMMI->ptMaxTrackSize = m_ptMaxTrackSize;

	if (m_bUseMaxRect)
	{
		lpMMI->ptMaxPosition = m_ptMaxPos;
		lpMMI->ptMaxSize = m_ptMaxSize;
	}
}

template <class T>
inline void CResizableMinMax<T>::ChainMinMaxInfo(LPMINMAXINFO lpMMI, HWND hWndParentFrame, HWND hWnd)
{
	T* pT = static_cast<T*>(this);
	// get the extra size from child to parent
	CRect rectClient, rectWnd;
	if ((::GetWindowLong(hWndParentFrame, GWL_STYLE) & WS_CHILD) && ::IsZoomed(hWndParentFrame))
		::GetClientRect(hWndParentFrame, rectWnd);
	else
		::GetWindowRect(hWndParentFrame, rectWnd);
	rectClient = rectWnd;
	//pParentFrame->RepositionBars(0, 0xFFFF,
	//	AFX_IDW_PANE_FIRST, CWnd::reposQuery, rectClient);
	CSize sizeExtra = rectWnd.Size() - rectClient.Size();

	pT->ChainMinMaxInfo(lpMMI, hWnd, sizeExtra);
}

template <class T>
inline void CResizableMinMax<T>::ChainMinMaxInfo(LPMINMAXINFO lpMMI, HWND hWndChild, CSize sizeExtra)
{
	// ask the child window for track size
	MINMAXINFO mmiChild = *lpMMI;
	::SendMessage(hWndChild, WM_GETMINMAXINFO, 0, (LPARAM)&mmiChild);
	BOOL bRetMax = (lpMMI->ptMaxTrackSize.x != mmiChild.ptMaxTrackSize.x
		|| lpMMI->ptMaxTrackSize.y != mmiChild.ptMaxTrackSize.y);
	BOOL bRetMin = (lpMMI->ptMinTrackSize.x != mmiChild.ptMinTrackSize.x
		|| lpMMI->ptMinTrackSize.y != mmiChild.ptMinTrackSize.y);

	// add static extra size
	mmiChild.ptMaxTrackSize = sizeExtra + mmiChild.ptMaxTrackSize;
	mmiChild.ptMinTrackSize = sizeExtra + mmiChild.ptMinTrackSize;

	// min size is the largest
	if (bRetMin)
	{
		lpMMI->ptMinTrackSize.x = __max(lpMMI->ptMinTrackSize.x,
			mmiChild.ptMinTrackSize.x);
		lpMMI->ptMinTrackSize.y = __max(lpMMI->ptMinTrackSize.y,
			mmiChild.ptMinTrackSize.y);
	}
	// max size is the shortest
	if (bRetMax)
	{
		lpMMI->ptMaxTrackSize.x = __min(lpMMI->ptMaxTrackSize.x,
			mmiChild.ptMaxTrackSize.x);
		lpMMI->ptMaxTrackSize.y = __min(lpMMI->ptMaxTrackSize.y,
			mmiChild.ptMaxTrackSize.y);
	}
}

template <class T>
inline BOOL CResizableMinMax<T>::CalcSizeExtra(HWND /*hWndChild*/, CSize /*sizeChild*/, CSize& /*sizeExtra*/)
{
	// should be overridden if you use ChainMinMaxInfoCB
	ATLASSERT(FALSE);
	return FALSE;
}

template <class T>
inline void CResizableMinMax<T>::ChainMinMaxInfoCB(LPMINMAXINFO lpMMI, HWND hWndChild)
{
	// ask the child window for track size
	MINMAXINFO mmiChild = *lpMMI;
	::SendMessage(hWndChild, WM_GETMINMAXINFO, 0, (LPARAM)&mmiChild);
	BOOL bRetMax = (lpMMI->ptMaxTrackSize.x != mmiChild.ptMaxTrackSize.x
		|| lpMMI->ptMaxTrackSize.y != mmiChild.ptMaxTrackSize.y);
	BOOL bRetMin = (lpMMI->ptMinTrackSize.x != mmiChild.ptMinTrackSize.x
		|| lpMMI->ptMinTrackSize.y != mmiChild.ptMinTrackSize.y);

	// use a callback to determine extra size
	CSize sizeExtra(0, 0);
	bRetMax = bRetMax && CalcSizeExtra(hWndChild, mmiChild.ptMaxTrackSize, sizeExtra);
	mmiChild.ptMaxTrackSize = sizeExtra + mmiChild.ptMaxTrackSize;
	bRetMin = bRetMin && CalcSizeExtra(hWndChild, mmiChild.ptMinTrackSize, sizeExtra);
	mmiChild.ptMinTrackSize = sizeExtra + mmiChild.ptMinTrackSize;

	// min size is the largest
	if (bRetMin)
	{
		lpMMI->ptMinTrackSize.x = __max(lpMMI->ptMinTrackSize.x,
			mmiChild.ptMinTrackSize.x);
		lpMMI->ptMinTrackSize.y = __max(lpMMI->ptMinTrackSize.y,
			mmiChild.ptMinTrackSize.y);
	}
	// max size is the shortest
	if (bRetMax)
	{
		lpMMI->ptMaxTrackSize.x = __min(lpMMI->ptMaxTrackSize.x,
			mmiChild.ptMaxTrackSize.x);
		lpMMI->ptMaxTrackSize.y = __min(lpMMI->ptMaxTrackSize.y,
			mmiChild.ptMaxTrackSize.y);
	}
}

template <class T>
inline void CResizableMinMax<T>::SetMaximizedRect(const CRect& rc)
{
	m_bUseMaxRect = TRUE;

	m_ptMaxPos = rc.TopLeft();
	m_ptMaxSize.x = rc.Width();
	m_ptMaxSize.y = rc.Height();
}

template <class T>
inline void CResizableMinMax<T>::ResetMaximizedRect()
{
	m_bUseMaxRect = FALSE;
}

template <class T>
inline void CResizableMinMax<T>::SetMinTrackSize(const CSize& size)
{
	m_bUseMinTrack = TRUE;

	m_ptMinTrackSize.x = size.cx;
	m_ptMinTrackSize.y = size.cy;
}

template <class T>
inline void CResizableMinMax<T>::ResetMinTrackSize()
{
	m_bUseMinTrack = FALSE;
}

template <class T>
inline void CResizableMinMax<T>::SetMaxTrackSize(const CSize& size)
{
	m_bUseMaxTrack = TRUE;

	m_ptMaxTrackSize.x = size.cx;
	m_ptMaxTrackSize.y = size.cy;
}

template <class T>
inline void CResizableMinMax<T>::ResetMaxTrackSize()
{
	m_bUseMaxTrack = FALSE;
}

