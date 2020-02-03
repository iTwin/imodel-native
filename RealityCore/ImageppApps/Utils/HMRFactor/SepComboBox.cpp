/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/HMRFactor/SepComboBox.cpp $
|    $RCSfile: SepComboBox.cpp,v $
|   $Revision: 1.1 $
|       $Date: 2006/02/20 15:57:08 $
|     $Author: BENTLEY\Simon.Normand $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "stdafx.h"
#include "SepComboBox.h"

// SepComboBox

IMPLEMENT_DYNAMIC(SepComboBox, CComboBox)
SepComboBox::SepComboBox(): 
   m_nPenStyle(PS_DOT), 
   m_crColor(RGB(64, 64, 64)),
   m_nBottomMargin(2),
   m_nSepWidth(1),
   m_nHorizontalMargin(2)
{
}

SepComboBox::~SepComboBox()
{
}


BEGIN_MESSAGE_MAP(SepComboBox, CComboBox)
   ON_WM_CTLCOLOR()
   ON_WM_DESTROY()
END_MESSAGE_MAP()

// SepComboBox message handlers

HBRUSH SepComboBox::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
   if (nCtlColor == CTLCOLOR_LISTBOX)
   {
      if (m_listbox.GetSafeHwnd() ==NULL)
      {
         m_listbox.SubclassWindow(pWnd->GetSafeHwnd());
      }

      CRect r;
      int   nIndex, n = m_listbox.GetCount();

      CPen pen(m_nPenStyle, m_nSepWidth, m_crColor), *pOldPen;
      pOldPen = pDC->SelectObject(&pen);

      for (int i=0; i< m_arySeparators.GetSize(); i++)
      {
         nIndex = m_arySeparators[i];
         if (nIndex<0) nIndex += n-1;

         if (nIndex < n-1)
         {
            m_listbox.GetItemRect(nIndex, &r );
            pDC->MoveTo(r.left+m_nHorizontalMargin, r.bottom-m_nBottomMargin);
            pDC->LineTo(r.right-m_nHorizontalMargin, r.bottom-m_nBottomMargin);
         }
      }

      pDC->SelectObject(pOldPen);
   }

   // TODO:  Return a different brush if the default is not desired
   //HBRUSH hbr = CComboBox::OnCtlColor(pDC, pWnd, nCtlColor);
   //return hbr;      // (HBRUSH)COLOR_WINDOW;   
   return CComboBox::OnCtlColor(pDC, pWnd, nCtlColor);
}

void SepComboBox::OnDestroy()
{
   if (m_listbox.GetSafeHwnd() !=NULL)
      m_listbox.UnsubclassWindow();

   CComboBox::OnDestroy();
}

   
void SepComboBox::SetSeparator(int iSep) 
{ 
   if (!m_arySeparators.GetSize())
      AdjustItemHeight();

   m_arySeparators.Add(iSep); 
}


void SepComboBox::AdjustItemHeight(int nInc/*=3*/)
{
   SetItemHeight(0, nInc+ GetItemHeight(0));
}
