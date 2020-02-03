/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/DisplayExtent.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if !defined(AFX_DISPLAYEXTENT_H__EB43AB48_E198_4A97_907D_A031203D445A__INCLUDED_)
#define AFX_DISPLAYEXTENT_H__EB43AB48_E198_4A97_907D_A031203D445A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DisplayExtent.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// DisplayExtent dialog

class DisplayExtent : public CDialog
{
// Construction
public:
	DisplayExtent(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(DisplayExtent)
	enum { IDD = IDD_DISPLAY_EXTENT };
	double	m_Max_X;
	double	m_Max_Y;
	double	m_Min_X;
	double	m_Min_Y;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DisplayExtent)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(DisplayExtent)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DISPLAYEXTENT_H__EB43AB48_E198_4A97_907D_A031203D445A__INCLUDED_)
