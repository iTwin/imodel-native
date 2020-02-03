/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/HMRFactor/SepComboBox.h $
|    $RCSfile: SepComboBox.h,v $
|   $Revision: 1.1 $
|       $Date: 2006/02/20 15:57:08 $
|     $Author: BENTLEY\Simon.Normand $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/****************************************************************************************************************
Usage:   if SepComboBox cb; you have to call:
      For example,
         cb.SetSeparator(0);                 // Set Separator after the first one
         cb.SetSeparator(-2);                // Set Separator before last two items

      These methods are optional:
         cb.AdjustItemHeight();              // Adjust item height for Separators, Def: Inc=3
         cb.SetSepLineStyle(PS_DASH);        // Set Separator to dash Lines, Def: dot line
         cb.SetSepLineColor(RGB(0, 0, 128)); // Set Separator Color to blue, Def: dark gray 64
         cb.SetSepLineWidth(2);              // Set Separator Line Width, Def: 1 pixel
         cb.SetBottomMargin(3);              // Set Separator Bottom Margin, Def: 2 pixels
         cb.SetHorizontalMargin(1);          // Set Separator Horizontal Margin, Def: 2 pixels
****************************************************************************************************************/
#pragma once

#include <afxtempl.h>

// SepComboBox
class SepComboBox : public CComboBox
{
   DECLARE_DYNAMIC(SepComboBox)

   CListBox    m_listbox;
   CArray<int> m_arySeparators;

   int         m_nHorizontalMargin;
   int         m_nBottomMargin;
   int         m_nSepWidth;
   int         m_nPenStyle;
   COLORREF    m_crColor;

public:
   SepComboBox();
   virtual ~SepComboBox();

   void SetSeparator(int iSep);
   void AdjustItemHeight(int nInc=3);

   void SetSepLineStyle(int iSep) { m_nPenStyle = iSep; }
   void SetSepLineColor(COLORREF crColor) { m_crColor = crColor; }
   void SetSepLineWidth(int iWidth) { m_nSepWidth = iWidth; }
   void SetBottomMargin(int iMargin) { m_nBottomMargin = iMargin; }
   void SetHorizontalMargin(int iMargin) { m_nHorizontalMargin = iMargin; }

protected:
   DECLARE_MESSAGE_MAP()

public:
   afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
   afx_msg void OnDestroy();
};


