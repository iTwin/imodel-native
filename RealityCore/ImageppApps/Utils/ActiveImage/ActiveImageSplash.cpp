/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ActiveImageSplash.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// ----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/ActiveImage/ActiveImageSplash.cpp,v 1.3 2011/07/18 21:10:42 Donald.Morissette Exp $
//
// Class: CActiveImageSplashWnd
// ----------------------------------------------------------------------------
// CG: This file was added by the Splash Screen component.
// ActiveImageSplash.cpp : implementation file
// ----------------------------------------------------------------------------

#include "stdafx.h"
#include "resource.h"
#include "ActiveImageSplash.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------------------
//   Splash Screen class
//-----------------------------------------------------------------------------

BOOL CActiveImageSplashWnd::c_bShowSplashWnd;
CActiveImageSplashWnd* CActiveImageSplashWnd::c_pSplashWnd;
CActiveImageSplashWnd::CActiveImageSplashWnd()
{
}


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

CActiveImageSplashWnd::~CActiveImageSplashWnd()
{
	// Clear the static window pointer.
	ASSERT(c_pSplashWnd == this);
	c_pSplashWnd = NULL;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CActiveImageSplashWnd, CWnd)
	//{{AFX_MSG_MAP(CActiveImageSplashWnd)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

void CActiveImageSplashWnd::EnableSplashScreen(BOOL bEnable /*= true*/)
{
	c_bShowSplashWnd = bEnable;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

void CActiveImageSplashWnd::ShowSplashScreen(CWnd* pParentWnd /*= NULL*/)
{
	if (!c_bShowSplashWnd || c_pSplashWnd != NULL)
		return;

	// Allocate a new splash screen, and create the window.
	c_pSplashWnd = new CActiveImageSplashWnd;
	if (!c_pSplashWnd->Create(pParentWnd))
		delete c_pSplashWnd;
	else
		c_pSplashWnd->UpdateWindow();
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------


BOOL CActiveImageSplashWnd::PreTranslateAppMessage(MSG* pMsg)
{
	if (c_pSplashWnd == NULL)
		return false;

	// If we get a keyboard or mouse message, hide the splash screen.
	if (pMsg->message == WM_KEYDOWN ||
	    pMsg->message == WM_SYSKEYDOWN ||
	    pMsg->message == WM_LBUTTONDOWN ||
	    pMsg->message == WM_RBUTTONDOWN ||
	    pMsg->message == WM_MBUTTONDOWN ||
	    pMsg->message == WM_NCLBUTTONDOWN ||
	    pMsg->message == WM_NCRBUTTONDOWN ||
	    pMsg->message == WM_NCMBUTTONDOWN)
	{
		c_pSplashWnd->HideSplashScreen();
		return true;	// message handled here
	}

	return false;	// message not handled
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

BOOL CActiveImageSplashWnd::Create(CWnd* pParentWnd /*= NULL*/)
{
	BOOL IsCreationSucess = false;
    
    if (m_bitmap.LoadBitmap(IDB_SPLASH))
    {
	
	    BITMAP bm;
	    m_bitmap.GetBitmap(&bm);

	    IsCreationSucess = CreateEx(0,
		                           AfxRegisterWndClass(0, AfxGetApp()->LoadStandardCursor(IDC_ARROW)),
		                           NULL, WS_POPUP | WS_VISIBLE, 
                                   0, 
                                   0, 
                                   bm.bmWidth, 
                                   bm.bmHeight, 
                                   pParentWnd->GetSafeHwnd(), 
                                   NULL);
    }
    return IsCreationSucess;
}

//-----------------------------------------------------------------------------
// protected: PostNcDestroy
//-----------------------------------------------------------------------------

void CActiveImageSplashWnd::HideSplashScreen()
{
	// Destroy the window, and update the mainframe.
	DestroyWindow();
	AfxGetMainWnd()->UpdateWindow();
}

//-----------------------------------------------------------------------------
// public: PostNcDestroy
//-----------------------------------------------------------------------------

void CActiveImageSplashWnd::PostNcDestroy()
{
	// Free the C++ class.
	delete this;
}

//-----------------------------------------------------------------------------
// protected: OnCreate
//-----------------------------------------------------------------------------

int CActiveImageSplashWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	int ReturnStatus = 0;
    
    if (CWnd::OnCreate(lpCreateStruct) == -1)
		ReturnStatus = -1;
    else
    {
	    // Center the window.
	    CenterWindow();

	    // Set a timer to destroy the splash screen.
	    SetTimer(1, 1100, NULL);
    }
    return ReturnStatus;
}

//-----------------------------------------------------------------------------
// protected: OnPaint
//-----------------------------------------------------------------------------

void CActiveImageSplashWnd::OnPaint()
{
	CPaintDC dc(this);

	CDC dcImage;
	if (dcImage.CreateCompatibleDC(&dc))
    {
        CRgn   MainDialogRgn;
        CRect  MyWindowRect;
        BITMAP bm;
    
        // Change the window shape...
        GetClientRect(MyWindowRect);
        MainDialogRgn.CreateRoundRectRgn( MyWindowRect.TopLeft().x + 1, MyWindowRect.TopLeft().y + 1, MyWindowRect.BottomRight().x, MyWindowRect.BottomRight().y, 50, 50);
        SetWindowRgn( MainDialogRgn, true);
        	    
	    m_bitmap.GetBitmap(&bm);

	    // Paint the image.
	    CBitmap* pOldBitmap = dcImage.SelectObject(&m_bitmap);
	    dc.BitBlt(0, 0, bm.bmWidth, bm.bmHeight, &dcImage, 0, 0, SRCCOPY);
	    dcImage.SelectObject(pOldBitmap);
    }
}

//-----------------------------------------------------------------------------
// protected: OnTimer
//-----------------------------------------------------------------------------

void CActiveImageSplashWnd::OnTimer(UINT_PTR nIDEvent)
{
	// Destroy the splash screen window.
	HideSplashScreen();
}
