/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageInsider/MyPropertyPage4.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if !defined(AFX_MYPROPERTYPAGE4_H__70E2D9B2_ABEB_11D3_B587_0060082BD950__INCLUDED_)
#define AFX_MYPROPERTYPAGE4_H__70E2D9B2_ABEB_11D3_B587_0060082BD950__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// MyPropertyPage4.h : header file
//

//-------------------------------------------------------------------------------------------------
// CMyPropertyPage4 dialog
//-------------------------------------------------------------------------------------------------

class CMyPropertyPage4 : public CPropertyPage
{
	DECLARE_DYNCREATE(CMyPropertyPage4)

// Construction
public:
	CMyPropertyPage4();
	~CMyPropertyPage4();

// Dialog Data
	//{{AFX_DATA(CMyPropertyPage4)
	enum { IDD = IDD_PROPPAGE4 };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CMyPropertyPage4)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CMyPropertyPage4)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//-------------------------------------------------------------------------------------------------

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYPROPERTYPAGE4_H__70E2D9B2_ABEB_11D3_B587_0060082BD950__INCLUDED_)
