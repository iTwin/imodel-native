/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ColorEditBox.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "ColorEditBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
#endif

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

ColorEditBox::ColorEditBox()
{
   m_clrText  = 0x00000000;
   m_clrBkgnd = GetSysColor(COLOR_WINDOW);

   m_brBkgnd = new CBrush;
   m_brBkgnd->CreateSolidBrush (m_clrBkgnd);

   m_DisplayArrowCursor = false;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

ColorEditBox::~ColorEditBox()
{
    m_brBkgnd->DeleteObject();
    delete m_brBkgnd;
}

//-----------------------------------------------------------------------------
// ColorEditBox message handlers
//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(ColorEditBox, CEdit)
	//{{AFX_MSG_MAP(ColorEditBox)
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_LBUTTONUP()
	ON_WM_SETCURSOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HBRUSH ColorEditBox::CtlColor(CDC* pDC, UINT nCtlColor) 
{
   pDC->SetTextColor(m_clrText);
   pDC->SetBkColor(m_clrBkgnd);
   
   return (HBRUSH)*m_brBkgnd;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void ColorEditBox::SetBGColor (COLORREF clrBkgnd)
{
   m_clrBkgnd = clrBkgnd;
   
   m_brBkgnd->DeleteObject();
   delete m_brBkgnd;

   m_brBkgnd = new CBrush;
   m_brBkgnd->CreateSolidBrush(m_clrBkgnd);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void ColorEditBox::SetTXColor (COLORREF clrText)
{
   m_clrText = clrText;
   GetDC()->SetTextColor(m_clrText);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void ColorEditBox::ObligatoryField ()
{
   SetBGColor (GetSysColor(COLOR_WINDOW));
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void ColorEditBox::OptionalField ()
{
   SetBGColor (GetSysColor(COLOR_3DFACE));
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void ColorEditBox::OnLButtonUp(UINT nFlags, CPoint point) 
{
    // GetParent()->SendMessage(WM_EDIT_SELECTED, GetDlgCtrlID() , 0);
	CEdit::OnLButtonUp(nFlags, point);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

BOOL ColorEditBox::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
    if (m_DisplayArrowCursor)
    {
        ::SetCursor(LoadCursor(NULL, IDC_ARROW));
        return true;
    }
    else
    {
	    return CEdit::OnSetCursor(pWnd, nHitTest, message);
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void ColorEditBox::SetCursorAsArrow(BOOL pi_DisplayArrowCursor)
{
    m_DisplayArrowCursor = pi_DisplayArrowCursor;
}
