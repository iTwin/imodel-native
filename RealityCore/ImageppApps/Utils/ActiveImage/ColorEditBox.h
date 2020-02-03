/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ColorEditBox.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

#ifndef __ColorEditBox_H__
#define __ColorEditBox_H__

#if _MSC_VER >= 1000
    #pragma once
#endif // _MSC_VER >= 1000

class ColorEditBox : public CEdit
{
public:
    
    ColorEditBox();
    virtual ~ColorEditBox();

    void SetBGColor (COLORREF clrBkgnd);
    void SetTXColor (COLORREF clrBkgnd);

    void ObligatoryField ();
    void OptionalField   ();

    void SetCursorAsArrow(BOOL pi_DisplayArrowCursor = true);

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(ColorEditBox)
    //}}AFX_VIRTUAL

protected:
   // COLORREF SetColorRef (UINT theColor);
   
    //{{AFX_MSG(ColorEditBox)
    afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG

    DECLARE_MESSAGE_MAP()
private:

    COLORREF m_clrText;
    COLORREF m_clrBkgnd;
    CBrush   *m_brBkgnd;

    BOOL m_DisplayArrowCursor;
};

#endif // __ColorEditBox_H__