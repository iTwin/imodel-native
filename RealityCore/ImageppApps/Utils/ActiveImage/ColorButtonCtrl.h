/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ColorButtonCtrl.h $
|    $RCSfile: ColorButtonCtrl.h,v $
|   $Revision: 1.1 $
|       $Date: 2005/02/08 18:35:28 $
|     $Author: SebastienGosselin $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once


// ColorButtonCtrl dialog

class ColorButtonCtrl : public CButton
{
	DECLARE_DYNAMIC(ColorButtonCtrl)

public:
	ColorButtonCtrl();   // standard constructor
	virtual ~ColorButtonCtrl();

    void SetBGColor (COLORREF clrBkgnd);
    void SetTXColor (COLORREF clrTxt);

    virtual void DrawItem( LPDRAWITEMSTRUCT lpDrawItemStruct );

protected:

	DECLARE_MESSAGE_MAP()
    //{{AFX_MSG(ColorButtonCtrl)
    afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
    afx_msg void OnMouseMove(UINT nFlags,CPoint point);
    afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
    afx_msg void OnRButtonDown( UINT, CPoint );
    //}}AFX_MSG
private:
    void    DrawButton ();

    COLORREF m_clrText;
    COLORREF m_clrBkgnd;
    CBrush   *m_brBkgnd;
    bool      m_bMouseOver;

};
