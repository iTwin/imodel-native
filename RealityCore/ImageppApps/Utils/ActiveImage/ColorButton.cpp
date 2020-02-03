/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ColorButton.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// ColorButton.cpp : implementation file
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "ActiveImage.h"
#include "ColorButton.h"
#include ".\colorbutton.h"

IMPLEMENT_DYNAMIC(CColorButton, CButton)

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

CColorButton::CColorButton()
{
    m_clrBkgnd = GetSysColor(COLOR_WINDOW);

   m_brBkgnd = new CBrush;
   m_brBkgnd->CreateSolidBrush (m_clrBkgnd);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

CColorButton::~CColorButton()
{
    m_brBkgnd->DeleteObject();
    delete m_brBkgnd;
}

//-----------------------------------------------------------------------------
// CColorButton message handlers
//-----------------------------------------------------------------------------

BEGIN_MESSAGE_MAP(CColorButton, CButton)
    ON_WM_CTLCOLOR_REFLECT()
    ON_WM_DRAWITEM()
END_MESSAGE_MAP()


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

void CColorButton::SetBGColor (COLORREF clrBkgnd)
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

HBRUSH CColorButton::CtlColor(CDC* pDC, UINT nCtlColor) 
{
   // pDC->SetTextColor(m_clrText);
   pDC->SetBkColor(m_clrBkgnd);
   
   return (HBRUSH)*m_brBkgnd;
}

