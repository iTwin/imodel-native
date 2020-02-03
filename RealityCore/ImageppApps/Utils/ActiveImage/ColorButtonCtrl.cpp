/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ColorButtonCtrl.cpp $
|    $RCSfile: ColorButtonCtrl.cpp,v $
|   $Revision: 1.1 $
|       $Date: 2005/02/08 18:35:28 $
|     $Author: SebastienGosselin $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// ColorButtonCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "ColorButtonCtrl.h"
#include ".\ColorButtonCtrl.h"


// ColorButtonCtrl dialog

IMPLEMENT_DYNAMIC(ColorButtonCtrl, CButton)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  07/2004
+---------------+---------------+---------------+---------------+---------------+------*/
ColorButtonCtrl::ColorButtonCtrl()
:m_bMouseOver (false),
m_clrText(0x00000000),
m_clrBkgnd (GetSysColor(COLOR_WINDOW))
   
    {
    m_brBkgnd = new CBrush;
    m_brBkgnd->CreateSolidBrush (m_clrBkgnd);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  07/2004
+---------------+---------------+---------------+---------------+---------------+------*/
ColorButtonCtrl::~ColorButtonCtrl()
    {
    m_brBkgnd->DeleteObject();
    delete m_brBkgnd;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  07/2004
+---------------+---------------+---------------+---------------+---------------+------*/
void ColorButtonCtrl::SetBGColor (COLORREF clrBkgnd)
    {
    m_clrBkgnd = clrBkgnd;

    m_brBkgnd->DeleteObject();
    delete m_brBkgnd;

    m_brBkgnd = new CBrush;
    m_brBkgnd->CreateSolidBrush(m_clrBkgnd);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  07/2004
+---------------+---------------+---------------+---------------+---------------+------*/
void ColorButtonCtrl::SetTXColor (COLORREF clrText)
    {
    m_clrText = clrText;
    GetDC()->SetTextColor(m_clrText);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  09/2004
+---------------+---------------+---------------+---------------+---------------+------*/
void    ColorButtonCtrl::DrawButton ()
    {
    CDC* pDC = GetDC();
    pDC->SetTextColor(m_clrText);
    pDC->SetBkColor(m_clrBkgnd);

    CRect MyCtrlRect;
    GetClientRect(MyCtrlRect);

    COLORREF topleft (GetSysColor (COLOR_BTNFACE));
    COLORREF bottomright (RGB(0,0,0));
    if (m_bMouseOver)
        {
        topleft = GetSysColor (COLOR_BTNHIGHLIGHT);
        }

    pDC->Draw3dRect( MyCtrlRect, topleft, bottomright);
    }


BEGIN_MESSAGE_MAP(ColorButtonCtrl, CButton)
        //{{AFX_MSG_MAP(ColorButtonCtrl)
        ON_WM_CTLCOLOR_REFLECT()
        ON_WM_MOUSEMOVE ()
        ON_MESSAGE(WM_MOUSELEAVE,OnMouseLeave)
        ON_WM_RBUTTONDOWN()
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

// ColorButtonCtrl message handlers
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  07/2004
+---------------+---------------+---------------+---------------+---------------+------*/
HBRUSH ColorButtonCtrl::CtlColor(CDC* pDC, UINT nCtlColor) 
    {
    DrawButton ();
    
    return (HBRUSH)*m_brBkgnd;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  09/2004
+---------------+---------------+---------------+---------------+---------------+------*/
void ColorButtonCtrl::OnMouseMove
(
UINT nFlags,
CPoint point 
)
    {
    m_bMouseOver = true;
    DrawButton ();
    
    TRACKMOUSEEVENT track; 
    track.cbSize = sizeof(track);
    track.dwFlags = TME_LEAVE; //Notify us when the mouse leaves
    track.hwndTrack = m_hWnd; //Assigns this window's hwnd
    TrackMouseEvent(&track); //Tracks the events like WM_MOUSELEAVE
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    SimonNormand  09/2004
+---------------+---------------+---------------+---------------+---------------+------*/
LRESULT ColorButtonCtrl::OnMouseLeave
(
WPARAM wParam,
LPARAM lParam
)
    {
    m_bMouseOver = false;
    DrawButton();

    return 0;
    }

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

void ColorButtonCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
   // This code only works with buttons.
   ASSERT(lpDrawItemStruct->CtlType == ODT_BUTTON);

   /*
   // If drawing selected, add the pushed style to DrawFrameControl.
   if (lpDrawItemStruct->itemState & ODS_SELECTED)
      uStyle |= DFCS_PUSHED;

   // Draw the button frame.
   ::DrawFrameControl(lpDrawItemStruct->hDC, &lpDrawItemStruct->rcItem, 
      DFC_BUTTON, uStyle);

   // Get the button's text.
   CString strText;
   GetWindowText(strText);

   // Draw the button text using the text color red.
   COLORREF crOldColor = ::SetTextColor(lpDrawItemStruct->hDC, RGB(255,0,0));
   ::DrawText(lpDrawItemStruct->hDC, strText, strText.GetLength(), 
      &lpDrawItemStruct->rcItem, DT_SINGLELINE|DT_VCENTER|DT_CENTER);
   ::SetTextColor(lpDrawItemStruct->hDC, crOldColor);
   */
}

//------------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------------

void ColorButtonCtrl::OnRButtonDown( UINT, CPoint )
{
    GetParent()->SendMessage(WM_BUTTON_MOUSE_RCLICKED, 0, 0);
}

//------------------------------------------------------------------------------------