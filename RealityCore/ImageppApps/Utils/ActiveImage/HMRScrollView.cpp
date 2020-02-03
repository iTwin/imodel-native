/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/HMRScrollView.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1997 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

//-----------------------------------------------------------------------------
// Header Files
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include <afxpriv.h>
#include <limits.h>
#include "afximpl.h"
#include "HMRScrollView.h"
#include <math.h>


//-----------------------------------------------------------------------------
// Compiler commands
//-----------------------------------------------------------------------------
#ifdef AFX_CORE2_SEG
#pragma code_seg(AFX_CORE2_SEG)
#endif


//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Special mapping modes just for CHMRScrollView implementation
#define MM_NONE             0

//-----------------------------------------------------------------------------
// Static Attributes
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Message Maps and Other MFC Macros
//-----------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CHMRScrollView, CView)
	//{{AFX_MSG_MAP(CHMRScrollView)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP

    // Respond to WM_HSCROLL and WM_VSCROLL without ClassWizard because
    // the standard MFC implementation of these handlers uses UINT when
    // the scroll message sends signed integers.
    ON_MESSAGE(WM_HSCROLL, OnHMRScrollH)
    ON_MESSAGE(WM_VSCROLL, OnHMRScrollV)
END_MESSAGE_MAP()



//-----------------------------------------------------------------------------
// Protected
// 
//-----------------------------------------------------------------------------
CHMRScrollView::CHMRScrollView()
{
    // set the maximum range at almost the max of the physical scroll bar
    m_MaxRange = 32000.0;

    // initialize the INT2DOUBLE ratio
    m_RatioX = 1.0;
    m_RatioY = 1.0;

    // set the position
    m_PositionX = 0.0;
    m_PositionY = 0.0;

    // setup the page and line size
	m_pageHMR.cx = 0.0;
    m_pageHMR.cy = 0.0;
	m_lineHMR.cx = 0.0;
    m_lineHMR.cy = 0.0;

    // Total range in HMR units
    m_TotalHMRMinX   = 0.0;
    m_TotalHMRMaxX   = 0.0;
    m_TotalHMRMinY   = 0.0;
    m_TotalHMRMaxY   = 0.0;
    m_TotalHMRWidth  = 0.0;
    m_TotalHMRHeight  = 0.0;

    // Total range in device units
    m_TotalDevMinX   = 0;
    m_TotalDevMaxX   = 0;
    m_TotalDevMinY   = 0;
    m_TotalDevMaxY   = 0;
    m_TotalDevWidth  = 0;
    m_TotalDevHeight = 0;

	m_bInsideUpdate = false;
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
CHMRScrollView::~CHMRScrollView()
{
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CHMRScrollView::SetScrollSizes(double pi_MinX, double pi_MaxX,
                                    double pi_MinY, double pi_MaxY,
                                    double pi_SizePageX, double pi_SizePageY,
                                    double pi_SizeLineX, double pi_SizeLineY)
{
	ASSERT((pi_MaxX - pi_MinX) >= 0.0 && (pi_MaxY - pi_MinY) >= 0.0);

    // Compute the X ratio from INT to DOUBLE based on the range
    m_RatioX = 1.0;
    if ((pi_MaxX - pi_MinX) > m_MaxRange)
    {
        // We have to scale the range to fit the maximum allowed by
        // the windows scroll range
        m_RatioX = (pi_MaxX - pi_MinX) / m_MaxRange;
    }

    // Compute the Y ratio from INT to DOUBLE based on the range
    m_RatioY = 1.0;
    if ((pi_MaxY - pi_MinY) > m_MaxRange)
    {
        // We have to scale the range to fit the maximum allowed by
        // the windows scroll range
        m_RatioY = (pi_MaxY - pi_MinY) / m_MaxRange;
    }

    // save the total range in HMR units
    m_TotalHMRMinX = pi_MinX;
    m_TotalHMRMaxX = pi_MaxX;
    m_TotalHMRMinY = pi_MinY;
    m_TotalHMRMaxY = pi_MaxY;
    m_TotalHMRWidth  = m_TotalHMRMaxX - m_TotalHMRMinX;
    m_TotalHMRHeight = m_TotalHMRMaxY - m_TotalHMRMinY;

    // save the total range in device (ints) units
    m_TotalDevMinX = (int32_t )(m_TotalHMRMinX / m_RatioX);
    m_TotalDevMaxX = (int32_t )(m_TotalHMRMaxX / m_RatioX);
    m_TotalDevMinY = (int32_t )(m_TotalHMRMinY / m_RatioY);
    m_TotalDevMaxY = (int32_t )(m_TotalHMRMaxY / m_RatioY);
    m_TotalDevWidth  = m_TotalDevMaxX - m_TotalDevMinX;
    m_TotalDevHeight = m_TotalDevMaxY - m_TotalDevMinY;

    // set the page size in device units
    m_pageHMR.cx = floor(pi_SizePageX);
    m_pageHMR.cy = floor(pi_SizePageY);
//    m_pageHMR.cx = pi_SizePageX;
//    m_pageHMR.cy = pi_SizePageY;

    // set hte line size in device units
    m_lineHMR.cx = floor(pi_SizeLineX);
    m_lineHMR.cy = floor(pi_SizeLineY);
//    m_lineHMR.cx = pi_SizeLineX;
//    m_lineHMR.cy = pi_SizeLineY;

	// now adjust device specific sizes
	if (m_pageHMR.cx == 0.0)
		m_pageHMR.cx = floor((pi_MaxX - pi_MaxX) / 10.0);
	if (m_pageHMR.cy == 0.0)
		m_pageHMR.cy = floor((pi_MaxY - pi_MaxY) / 10.0);
	if (m_lineHMR.cx == 0.0)
		m_lineHMR.cx = floor(m_pageHMR.cx / 10.0);
	if (m_lineHMR.cy == 0.0)
		m_lineHMR.cy = floor(m_pageHMR.cy / 10.0);

	if (m_hWnd != NULL)
	{
		// window has been created, invalidate now
		UpdateBars();
    	Invalidate(true);
	}
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
HMRPOINT CHMRScrollView::GetScrollPosition() const   // logical coordinates
{
	return HMRPOINT(HMRGetScrollPos(SB_HORZ), HMRGetScrollPos(SB_VERT));
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CHMRScrollView::GetScrollSizes(double& pi_rMinX, double& pi_rMaxX,
                                    double& pi_rMinY, double& pi_rMaxY,
                                    double& pi_rSizePageX, double& pi_rSizePageY,
                                    double& pi_rSizeLineX, double& pi_rSizeLineY) const
{
    // set the total size
    pi_rMinX = m_TotalHMRMinX;
    pi_rMaxX = m_TotalHMRMaxX;
    pi_rMinY = m_TotalHMRMinY;
    pi_rMaxY = m_TotalHMRMaxY;

    // set the size page
    pi_rSizePageX = m_pageHMR.cx;
    pi_rSizePageY = m_pageHMR.cy;

    // set the line page
    pi_rSizeLineX = m_lineHMR.cx;
    pi_rSizeLineY = m_lineHMR.cy;
}



//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CHMRScrollView::ScrollToPosition(HMRPOINT pi_Point)
{
    double xMin, xMax, xLimit;
    double yMin, yMax, yLimit;

    // get scroll info
    HMRGetScrollRange(SB_HORZ, &xMin, &xMax);
    HMRGetScrollRange(SB_VERT, &yMin, &yMax);
    xLimit = HMRGetScrollLimit(SB_HORZ);
    yLimit = HMRGetScrollLimit(SB_VERT);

    // Limit if out of range
    if (pi_Point.x < xMin)
        pi_Point.x = xMin;
    else if (pi_Point.x > xLimit)
		pi_Point.x = xLimit;
    if (pi_Point.y < yMin)
        pi_Point.y = yMin;
    else if (pi_Point.y > yLimit)
		pi_Point.y = yLimit;

    // set the position
    m_PositionX = pi_Point.x;
    m_PositionY = pi_Point.y;

	// Note: ScrollToDevicePosition can and is used to scroll out-of-range
	//  areas as far as CHMRScrollView is concerned -- specifically in
	//  the print-preview code.  Since OnScrollBy makes sure the range is
	//  valid, ScrollToDevicePosition does not vector through OnScrollBy.
	double xOrig = HMRGetScrollPos(SB_HORZ);
	HMRSetScrollPos(SB_HORZ, pi_Point.x);
	double yOrig = HMRGetScrollPos(SB_VERT);
	HMRSetScrollPos(SB_VERT, pi_Point.y);
	HMRScrollWindow(xOrig - pi_Point.x, yOrig - pi_Point.y);
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
CPoint CHMRScrollView::GetDeviceScrollPosition() const
{
	return CPoint(GetScrollPos(SB_HORZ), GetScrollPos(SB_VERT));
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CHMRScrollView::GetDeviceScrollSizes(SIZE& sizeTotal, 
                                          SIZE& sizePage, 
                                          SIZE& sizeLine) const
{
	sizeTotal.cx = m_TotalDevWidth;
    sizeTotal.cy = m_TotalDevHeight;
    sizePage.cx = (int)(m_pageHMR.cx / m_RatioX);
    sizePage.cy = (int)(m_pageHMR.cy / m_RatioY);
    sizeLine.cx = (int)(m_lineHMR.cx / m_RatioX);
    sizeLine.cy = (int)(m_lineHMR.cy / m_RatioY);
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CHMRScrollView::ScrollToDevicePosition(POINT ptDev)
{
	// Note: ScrollToDevicePosition can and is used to scroll out-of-range
	//  areas as far as CHMRScrollView is concerned -- specifically in
	//  the print-preview code.  Since OnScrollBy makes sure the range is
	//  valid, ScrollToDevicePosition does not vector through OnScrollBy.
	int xOrig = GetScrollPos(SB_HORZ);
	SetScrollPos(SB_HORZ, ptDev.x);
	int yOrig = GetScrollPos(SB_VERT);
	SetScrollPos(SB_VERT, ptDev.y);
	ScrollWindow(xOrig - ptDev.x, yOrig - ptDev.y);
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CHMRScrollView::FillOutsideRect(CDC* pDC, CBrush* pBrush)
{
	ASSERT_VALID(pDC);
	ASSERT_VALID(pBrush);

	// fill rect outside the image
	CRect rect;
	GetClientRect(rect);
	ASSERT(rect.left == 0 && rect.top == 0);
	rect.left = m_TotalDevWidth;
	if (!rect.IsRectEmpty())
		pDC->FillRect(rect, pBrush);    // vertical strip along the side
	rect.left = 0;
	rect.right = m_TotalDevWidth;
	rect.top = m_TotalDevHeight;
	if (!rect.IsRectEmpty())
		pDC->FillRect(rect, pBrush);    // horizontal strip along the bottom
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CHMRScrollView::ResizeParentToFit(BOOL bShrinkOnly)
{
	// determine current size of the client area as if no scrollbars present
	CRect rectClient;
	GetWindowRect(rectClient);
	CRect rect = rectClient;
	CalcWindowRect(rect);
	rectClient.left += rectClient.left - rect.left;
	rectClient.top += rectClient.top - rect.top;
	rectClient.right -= rect.right - rectClient.right;
	rectClient.bottom -= rect.bottom - rectClient.bottom;
	rectClient.OffsetRect(-rectClient.left, -rectClient.top);
	ASSERT(rectClient.left == 0 && rectClient.top == 0);

	// determine desired size of the view
	CRect rectView(0, 0, m_TotalDevWidth, m_TotalDevHeight);
	if (bShrinkOnly)
	{
		if (rectClient.right <= m_TotalDevWidth)
			rectView.right = rectClient.right;
		if (rectClient.bottom <= m_TotalDevHeight)
			rectView.bottom = rectClient.bottom;
	}
	CalcWindowRect(rectView, CWnd::adjustOutside);
	rectView.OffsetRect(-rectView.left, -rectView.top);
	ASSERT(rectView.left == 0 && rectView.top == 0);
	if (bShrinkOnly)
	{
		if (rectClient.right <= m_TotalDevWidth)
			rectView.right = rectClient.right;
		if (rectClient.bottom <= m_TotalDevHeight)
			rectView.bottom = rectClient.bottom;
	}

	// dermine and set size of frame based on desired size of view
	CRect rectFrame;
	CFrameWnd* pFrame = GetParentFrame();
	ASSERT_VALID(pFrame);
	pFrame->GetWindowRect(rectFrame);
	CSize size = rectFrame.Size();
	size.cx += rectView.right - rectClient.right;
	size.cy += rectView.bottom - rectClient.bottom;
	pFrame->SetWindowPos(NULL, 0, 0, size.cx, size.cy,
		SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CHMRScrollView::OnSize(UINT nType, int cx, int cy)
{
    // Call the base class
	CView::OnSize(nType, cx, cy);

    // UpdateBars() handles locking out recursion
    UpdateBars();
}


//-----------------------------------------------------------------------------
// Protected
// 
//-----------------------------------------------------------------------------
void CHMRScrollView::GetScrollBarSizes(CSize& sizeSb)
{
	sizeSb.cx = sizeSb.cy = 0;
	DWORD dwStyle = GetStyle();

	if (GetScrollBarCtrl(SB_VERT) == NULL)
	{
		// vert scrollbars will impact client area of this window
		sizeSb.cx = afxData.cxVScroll;
		if (dwStyle & WS_BORDER)
			sizeSb.cx -= CX_BORDER;
	}
	if (GetScrollBarCtrl(SB_HORZ) == NULL)
	{
		// horz scrollbars will impact client area of this window
		sizeSb.cy = afxData.cyHScroll;
		if (dwStyle & WS_BORDER)
			sizeSb.cy -= CY_BORDER;
	}
}


//-----------------------------------------------------------------------------
// Protected
// 
//-----------------------------------------------------------------------------
BOOL CHMRScrollView::GetTrueClientSize(CSize& size, CSize& sizeSb)
	// return true if enough room to add scrollbars if needed
{
	CRect rect;
	GetClientRect(&rect);
	ASSERT(rect.top == 0 && rect.left == 0);
	size.cx = rect.right;
	size.cy = rect.bottom;
	DWORD dwStyle = GetStyle();

	// first get the size of the scrollbars for this window
	GetScrollBarSizes(sizeSb);

	// first calculate the size of a potential scrollbar
	// (scroll bar controls do not get turned on/off)
	if (sizeSb.cx != 0 && (dwStyle & WS_VSCROLL))
	{
		// vert scrollbars will impact client area of this window
		size.cx += sizeSb.cx;   // currently on - adjust now
	}
	if (sizeSb.cy != 0 && (dwStyle & WS_HSCROLL))
	{
		// horz scrollbars will impact client area of this window
		size.cy += sizeSb.cy;   // currently on - adjust now
	}

	// return true if enough room
	return (size.cx > sizeSb.cx && size.cy > sizeSb.cy);
}


//-----------------------------------------------------------------------------
// Protected
// 
// helper to return the state of the scrollbars without actually changing
//  the state of the scrollbars
//-----------------------------------------------------------------------------
void CHMRScrollView::GetScrollBarState(CSize sizeClient, CSize& needSb,
	CSize& sizeRange, HMRPOINT& ptMove, BOOL bInsideClient)
{
	// get scroll bar sizes (the part that is in the client area)
	CSize sizeSb;
	GetScrollBarSizes(sizeSb);

	// enough room to add scrollbars
    sizeRange.cx = m_TotalDevWidth - sizeClient.cx;
    sizeRange.cy = m_TotalDevHeight - sizeClient.cy;
		// > 0 => need to scroll
	ptMove = GetScrollPosition();
		// point to move to (start at current scroll pos)

	BOOL bNeedH = sizeRange.cx > 0;
	if (!bNeedH)
		ptMove.x = 0.0;                       // jump back to origin
	else if (bInsideClient)
		sizeRange.cy += sizeSb.cy;          // need room for a scroll bar

	BOOL bNeedV = sizeRange.cy > 0;
	if (!bNeedV)
		ptMove.y = 0.0;                       // jump back to origin
	else if (bInsideClient)
		sizeRange.cx += sizeSb.cx;          // need room for a scroll bar

	if (bNeedV && !bNeedH && sizeRange.cx > 0)
	{
		ASSERT(bInsideClient);
		// need a horizontal scrollbar after all
		bNeedH = true;
		sizeRange.cy += sizeSb.cy;
	}

	// if current scroll position will be past the limit, scroll to limit
//	if (sizeRange.cx > 0 && ptMove.x >= sizeRange.cx)
//		ptMove.x = sizeRange.cx;
//	if (sizeRange.cy > 0 && ptMove.y >= sizeRange.cy)
//		ptMove.y = sizeRange.cy;

	// now update the bars as appropriate
	needSb.cx = bNeedH;
	needSb.cy = bNeedV;

	// needSb, sizeRange, and ptMove area now all updated
}


//-----------------------------------------------------------------------------
// Protected
// 
//-----------------------------------------------------------------------------
void CHMRScrollView::UpdateBars()
{
	// UpdateBars may cause window to be resized - ignore those resizings
	if (m_bInsideUpdate)
		return;         // Do not allow recursive calls

	// Lock out recursion
	m_bInsideUpdate = true;

	// update the horizontal to reflect reality
	// NOTE: turning on/off the scrollbars will cause 'OnSize' callbacks
	ASSERT(m_TotalDevWidth >= 0.0 && m_TotalDevHeight >= 0.0);

	CRect rectClient;
	BOOL bCalcClient = true;

	// allow parent to do inside-out layout first
	CWnd* pParentWnd = GetParent();
	if (pParentWnd != NULL)
	{
		// if parent window responds to this message, use just
		//  client area for scroll bar calc -- not "true" client area
		if ((BOOL)pParentWnd->SendMessage(WM_RECALCPARENT, 0,
			(LPARAM)(LPCRECT)&rectClient) != 0)
		{
			// use rectClient instead of GetTrueClientSize for
			//  client size calculation.
			bCalcClient = false;
		}
	}

	CSize sizeClient;
	CSize sizeSb;

	if (bCalcClient)
	{
		// get client rect
		if (!GetTrueClientSize(sizeClient, sizeSb))
		{
			// no room for scroll bars (common for zero sized elements)
			CRect rect;
			GetClientRect(&rect);
			if (rect.right > 0 && rect.bottom > 0)
			{
				// if entire client area is not invisible, assume we have
				//  control over our scrollbars
				EnableScrollBarCtrl(SB_BOTH, false);
			}
			m_bInsideUpdate = false;
			return;
		}
	}
	else
	{
		// let parent window determine the "client" rect
		GetScrollBarSizes(sizeSb);
		sizeClient.cx = rectClient.right - rectClient.left;
		sizeClient.cy = rectClient.bottom - rectClient.top;
	}

	// enough room to add scrollbars
	CSize sizeRange;
	HMRPOINT ptMove;
	CSize needSb;

	// get the current scroll bar state given the true client area
	GetScrollBarState(sizeClient, needSb, sizeRange, ptMove, bCalcClient);
	if (needSb.cx)
		sizeClient.cy -= sizeSb.cy;
	if (needSb.cy)
		sizeClient.cx -= sizeSb.cx;

	// this structure needed to update the scrollbar page range
	SCROLLINFO info;
	info.fMask = SIF_PAGE|SIF_RANGE;
	info.nMin = 0;

	// now update the bars as appropriate
	EnableScrollBarCtrl(SB_HORZ, needSb.cx);
	if (needSb.cx)
	{
		info.nPage = sizeClient.cx;
        info.nMin = m_TotalDevMinX;
		info.nMax = m_TotalDevMaxX;
		if (!SetScrollInfo(SB_HORZ, &info, true))
			SetScrollRange(SB_HORZ, 0, sizeRange.cx, true);
	}
	EnableScrollBarCtrl(SB_VERT, needSb.cy);
	if (needSb.cy)
	{
		info.nPage = sizeClient.cy;
        info.nMin = m_TotalDevMinY;
		info.nMax = m_TotalDevMaxY;
		if (!SetScrollInfo(SB_VERT, &info, true))
			SetScrollRange(SB_VERT, 0, sizeRange.cy, true);
	}

	// first scroll the window as needed
	ScrollToPosition(ptMove); // will set the scroll bar positions too

	// remove recursion lockout
	m_bInsideUpdate = false;
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CHMRScrollView::CalcWindowRect(LPRECT lpClientRect, UINT nAdjustType)
{
	if (nAdjustType == adjustOutside)
	{
		// allow for special client-edge style
		::AdjustWindowRectEx(lpClientRect, 0, false, GetExStyle());


		// if the view is being used in-place, add scrollbar sizes
		//  (scollbars should appear on the outside when in-place editing)
		CSize sizeClient(
			lpClientRect->right - lpClientRect->left,
			lpClientRect->bottom - lpClientRect->top);

        CSize sizeRange(m_TotalDevWidth - sizeClient.cx, m_TotalDevHeight - sizeClient.cy);
			// > 0 => need to scroll

		// get scroll bar sizes (used to adjust the window)
		CSize sizeSb;
		GetScrollBarSizes(sizeSb);

		// adjust the window size based on the state
		if (sizeRange.cy > 0)
		{   // vertical scroll bars take up horizontal space
			lpClientRect->right += sizeSb.cx;
		}
		if (sizeRange.cx > 0)
		{   // horizontal scroll bars take up vertical space
			lpClientRect->bottom += sizeSb.cy;
		}
	}
	else
	{
		// call default to handle other non-client areas
		::AdjustWindowRectEx(lpClientRect, GetStyle(), false,
			GetExStyle() & ~(WS_EX_CLIENTEDGE));
	}
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
LRESULT CHMRScrollView::OnHMRScrollH(WPARAM wParam, LPARAM lParam)
{
    uint32_t     nSBCode;
    int32_t      nPos;
    CScrollBar* pScrollBar;
    double     ScrollPos;

    // convert message
    nSBCode = LOWORD(wParam);
    nPos    = (short)HIWORD(wParam); // message sends a short int so cast to short
    pScrollBar = (CScrollBar*)CScrollBar::FromHandle((HWND)lParam);

	if (pScrollBar != NULL && pScrollBar->SendChildNotifyLastMsg())
		return (LONG)0;     // eat it

	// ignore scroll bar msgs from other controls
	if (pScrollBar != GetScrollBarCtrl(SB_HORZ))
		return (LONG)0;

    // WARNING: If the given position is the scroll limit (all in device units),
    // force the scroll position to the scroll limit in double.
    if (nPos == GetScrollLimit(SB_HORZ))
        ScrollPos = HMRGetScrollLimit(SB_HORZ);
    else
        ScrollPos = floor((double)nPos * m_RatioX);

	OnScroll(MAKEWORD(nSBCode, -1), ScrollPos);

    return (LONG)0;
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
LRESULT CHMRScrollView::OnHMRScrollV(WPARAM wParam, LPARAM lParam)
{
    uint32_t     nSBCode;
    int32_t      nPos;
    CScrollBar* pScrollBar;
    double     ScrollPos;

    // convert message
    nSBCode = LOWORD(wParam);
    nPos    = (short)HIWORD(wParam); // message sends a short int so cast to short
    pScrollBar = (CScrollBar*)CScrollBar::FromHandle((HWND)lParam);

	if (pScrollBar != NULL && pScrollBar->SendChildNotifyLastMsg())
		return (LONG)0;     // eat it

	// ignore scroll bar msgs from other controls
	if (pScrollBar != GetScrollBarCtrl(SB_VERT))
		return (LONG)0;

    // WARNING: If the given position is the scroll limit (all in device units),
    // force the scroll position to the scroll limit in double.
    if (nPos == GetScrollLimit(SB_VERT))
        ScrollPos = HMRGetScrollLimit(SB_VERT);
    else
        ScrollPos = floor((double)nPos * m_RatioY);

	OnScroll(MAKEWORD(-1, nSBCode), ScrollPos);

    return (LONG)0;
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
BOOL CHMRScrollView::OnScroll(UINT nScrollCode, double nPos, BOOL bDoScroll)
{
    double MinPos, MaxPos;
    double Limit;

	// calc new x position
    HMRGetScrollRange(SB_HORZ, &MinPos, &MaxPos);
    Limit = HMRGetScrollLimit(SB_HORZ);
	double x = HMRGetScrollPos(SB_HORZ);
	double xOrig = x;

	switch (LOBYTE(nScrollCode))
	{
	case SB_TOP:
		x = MinPos;
		break;
	case SB_BOTTOM:
		x = Limit;
		break;
	case SB_LINEUP:
		x -= m_lineHMR.cx;

        // Insure that position is not lower than minpos
        // Only if the original position is not minPos
        if ((xOrig != MinPos) && (x < MinPos))
            x = MinPos;
		break;
	case SB_LINEDOWN:
		x += m_lineHMR.cx;

        // Insure that position is not greater than limit
        // Only of the original position is not Limit
        if ((xOrig != Limit) && (x > Limit))
            x = Limit;
        break;
	case SB_PAGEUP:
		x -= m_pageHMR.cx;

        // Insure that position is not lower than minpos
        if (x < MinPos)
            x = MinPos;
		break;
	case SB_PAGEDOWN:
		x += m_pageHMR.cx;

        // Insure that position is not greater than limit
        if (x > Limit)
            x = Limit;
		break;
	case SB_THUMBTRACK:
    case SB_THUMBPOSITION:
        {
            SCROLLINFO ScrollInfo;
            GetScrollInfo(SB_HORZ, &ScrollInfo);
		    x = ScrollInfo.nTrackPos;;
        }
		break;
	}

	// calc new y position
    HMRGetScrollRange(SB_VERT, &MinPos, &MaxPos);
    Limit = HMRGetScrollLimit(SB_VERT);
	double y = HMRGetScrollPos(SB_VERT);
	double yOrig = y;

	switch (HIBYTE(nScrollCode))
	{
	case SB_TOP:
		y = MinPos;
		break;
	case SB_BOTTOM:
		y = Limit;
		break;
	case SB_LINEUP:
		y -= m_lineHMR.cy;

        // Insure that position is not lower than minpos
        // Only if the original position is not minPos
        if ((yOrig != MinPos) && (y < MinPos))
            y = MinPos;
        break;
	case SB_LINEDOWN:
		y += m_lineHMR.cy;

        // Insure that position is not greater than limit
        // Only of the original position is not Limit
        if ((yOrig != Limit) && (y > Limit))
            y = Limit;
        break;
	case SB_PAGEUP:
		y -= m_pageHMR.cy;

        // Insure that position is not lower than minpos
        if (y < MinPos)
            y = MinPos;
		break;
	case SB_PAGEDOWN:
		y += m_pageHMR.cy;

        // Insure that position is not greater than limit
        if (y > Limit)
            y = Limit;
        break;
	case SB_THUMBTRACK:
    case SB_THUMBPOSITION:
        {
            SCROLLINFO ScrollInfo;
            GetScrollInfo(SB_VERT, &ScrollInfo);
		    y = ScrollInfo.nTrackPos;;
        }
		break;
	}

	return  OnScrollBy(HMRSIZE(x - xOrig, y - yOrig), bDoScroll);
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
BOOL CHMRScrollView::OnScrollBy(HMRSIZE sizeScroll, BOOL bDoScroll)
{
    BOOL bRangeChange = false;
	double xOrig, x, xMin, xMax, xLimit;
	double yOrig, y, yMin, yMax, yLimit;

	// don't scroll if there is no valid scroll range (ie. no scroll bar)
	CScrollBar* pBar;
	DWORD dwStyle = GetStyle();
	pBar = GetScrollBarCtrl(SB_VERT);
	if ((pBar != NULL && !pBar->IsWindowEnabled()) ||
		(pBar == NULL && !(dwStyle & WS_VSCROLL)))
	{
		// vertical scroll bar not enabled
		sizeScroll.cy = 0.0;
	}
	pBar = GetScrollBarCtrl(SB_HORZ);
	if ((pBar != NULL && !pBar->IsWindowEnabled()) ||
		(pBar == NULL && !(dwStyle & WS_HSCROLL)))
	{
		// horizontal scroll bar not enabled
		sizeScroll.cx = 0.0;
	}

    // Obtain current x scroll info
    xOrig = x = HMRGetScrollPos(SB_HORZ);
    HMRGetScrollRange(SB_HORZ, &xMin, &xMax);
    xLimit = HMRGetScrollLimit(SB_HORZ);
    x += sizeScroll.cx;

    // adjust current x position
    if (x < xMin)
    {
        bRangeChange = true;
        xMin = x;
    }
    else if (x > xLimit)
    {
        bRangeChange = true;
        xMax += (x - xLimit);
    }

    // Obtain current y scroll info
    yOrig = y = HMRGetScrollPos(SB_VERT);
    HMRGetScrollRange(SB_VERT, &yMin, &yMax);
    yLimit = HMRGetScrollLimit(SB_VERT);
    y += sizeScroll.cy;

    // adjust current y position
    if (y < yMin)
    {
        bRangeChange = true;
        yMin = y;
    }
    else if (y > yLimit)
    {
        bRangeChange = true;
        yMax += (y - yLimit);
    }

	// did anything change?
	if ((!bRangeChange) && (x == xOrig) && (y == yOrig))
		return false;

    if (bDoScroll)
    {
        double xScroll = -(x-xOrig);
        double yScroll = -(y-yOrig);

        // change the scroll range
        if (bRangeChange)
        {
            HMRSetScrollRange(SB_HORZ, xMin, xMax);
            HMRSetScrollRange(SB_VERT, yMin, yMax);
        }

        // verify if we actually scroll the window
        if ((x != xOrig) || (y != yOrig))
        {
            // update scroll positions
		    if (x != xOrig)
			    HMRSetScrollPos(SB_HORZ, x);
		    if (y != yOrig)
			    HMRSetScrollPos(SB_VERT, y);

            // Obtain the client area of the view
            CRect rect;
            GetClientRect(&rect);

            // verify if the scroll amount is greater than the 
            // client area.  If so, do not scroll the window,
            // invalidate the entire window.
            if ((fabs(xScroll) <= (double)rect.Width()) &&
                (fabs(yScroll) <= (double)rect.Height()) )
            {
		        HMRScrollWindow(-(x-xOrig), -(y-yOrig));
                UpdateWindow();
            }
            else
                RedrawWindow();
        }
	}
	return true;
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
double CHMRScrollView::HMRGetScrollPos(int nBar) const
{
    double ScrollPos;

    // return the wanted information
    if (nBar == SB_HORZ)
        ScrollPos = m_PositionX;
    else
        ScrollPos = m_PositionY;

    return (ScrollPos);
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CHMRScrollView::HMRGetScrollRange(int nBar, double* lpMinPos, double* lpMaxPos) const
{
    int MinPos, MaxPos;

    // get the actual value from the scroll bar
    GetScrollRange(nBar, &MinPos, &MaxPos);

    // convert to HDOUBLEs
    if (nBar == SB_HORZ)
    {
        *lpMinPos = m_TotalHMRMinX;
        *lpMaxPos = m_TotalHMRMaxX;
    }
    else
    {
        *lpMinPos = m_TotalHMRMinY;
        *lpMaxPos = m_TotalHMRMaxY;
    }
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
double CHMRScrollView::HMRSetScrollPos(int nBar, double nPos, BOOL bRedraw)
{
    double PreviousPosition = HMRGetScrollPos(nBar);

    // set the scroll bar
    if (nBar == SB_HORZ)
    {
        // Keep sure that position is in range
        if (nPos < m_TotalHMRMinX)
            nPos = m_TotalHMRMinX;
        if (nPos > m_TotalHMRMaxX)
            nPos = m_TotalHMRMaxX;

        // set the position
        m_PositionX = nPos;
        SetScrollPos(nBar, (int)(m_PositionX / m_RatioX), bRedraw);
    }
    else
    {
        // Keep sure that position is in range
        if (nPos < m_TotalHMRMinY)
            nPos = m_TotalHMRMinY;
        if (nPos > m_TotalHMRMaxY)
            nPos = m_TotalHMRMaxY;

        // set the position
        m_PositionY = nPos;
        SetScrollPos(nBar, (int)(m_PositionY / m_RatioY), bRedraw);
    }

    // return the previous position
    return PreviousPosition;
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CHMRScrollView::HMRSetScrollRange(int nBar, double nMinPos, double nMaxPos,
			                           BOOL bRedraw)
{
    // compute the ratio int to double
    if (nBar == SB_HORZ)
    {
        // Compute the X ratio from INT to DOUBLE based on the range
        m_RatioX = 1.0;
        if ((nMaxPos - nMinPos) > m_MaxRange)
        {
            // We have to scale the range to fit the maximum allowed by
            // the windows scroll range
            m_RatioX = (nMaxPos - nMinPos) / m_MaxRange;
        }

        // set the X total range in HMR units
        m_TotalHMRMinX = nMinPos;
        m_TotalHMRMaxX = nMaxPos;
        m_TotalHMRWidth  = m_TotalHMRMaxX - m_TotalHMRMinX;
        m_TotalDevMinX = (int32_t )(m_TotalHMRMinX / m_RatioX);
        m_TotalDevMaxX = (int32_t )(m_TotalHMRMaxX / m_RatioX);
        m_TotalDevWidth  = m_TotalDevMaxX - m_TotalDevMinX;
        SetScrollRange(SB_HORZ, m_TotalDevMinX, m_TotalDevMaxX);
    }
    else
    {
        // Compute the Y ratio from INT to DOUBLE based on the range
        m_RatioY = 1.0;
        if ((nMaxPos - nMinPos) > m_MaxRange)
        {
            // We have to scale the range to fit the maximum allowed by
            // the windows scroll range
            m_RatioY = (nMaxPos - nMinPos) / m_MaxRange;
        }
        m_TotalHMRMinY = nMinPos;
        m_TotalHMRMaxY = nMaxPos;
        m_TotalHMRHeight = m_TotalHMRMaxY - m_TotalHMRMinY;
        m_TotalDevMinY = (int32_t )(m_TotalHMRMinY / m_RatioY);
        m_TotalDevMaxY = (int32_t )(m_TotalHMRMaxY / m_RatioY);
        m_TotalDevHeight = m_TotalDevMaxY - m_TotalDevMinY;
        SetScrollRange(SB_VERT, m_TotalDevMinY, m_TotalDevMaxY);
    }
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
double CHMRScrollView::HMRGetScrollLimit(int nBar)
{
    SCROLLINFO info;
	double nMin, nMax;

    // get the scroll range
	HMRGetScrollRange(nBar, &nMin, &nMax);

    // if scroll info is available, substract the page size
    // set in the scroll bar to the maximum value of the 
    // scroll range.
	if (GetScrollInfo(nBar, &info, SIF_PAGE))
		nMax -= __max((double)info.nPage - 1.0, 0.0);

	return nMax;
}


//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CHMRScrollView::HMRScrollWindow(double pi_xAmount, 
                                     double pi_yAmount, 
                                     LPCRECT lpRect, 
                                     LPCRECT lpClipRect)
{
    HPRECONDITION(fabs(pi_xAmount) < m_MaxRange);
    HPRECONDITION(fabs(pi_yAmount) < m_MaxRange);

    // call the standard function
    ScrollWindow((int32_t )pi_xAmount, (int32_t )pi_yAmount, lpRect, lpClipRect);
}


#ifdef _DEBUG
//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CHMRScrollView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

//-----------------------------------------------------------------------------
// 
// 
//-----------------------------------------------------------------------------
void CHMRScrollView::AssertValid() const
{
	CView::AssertValid();
}
#endif //_DEBUG

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

IMPLEMENT_DYNAMIC(CHMRScrollView, CView)

/////////////////////////////////////////////////////////////////////////////
