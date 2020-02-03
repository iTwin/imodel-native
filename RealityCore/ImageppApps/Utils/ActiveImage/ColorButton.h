/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ColorButton.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once


// CColorButton

class CColorButton : public CButton
{
	DECLARE_DYNAMIC(CColorButton)

public:
	CColorButton();
	virtual ~CColorButton();

    void SetBGColor (COLORREF clrBkgnd);

protected:
	DECLARE_MESSAGE_MAP()

    //{{AFX_MSG(ColorEditBox)
    afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	//}}AFX_MSG

private:
    COLORREF m_clrBkgnd;
    CBrush   *m_brBkgnd;

};


