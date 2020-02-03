/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageInsider/MyPropertySheet.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// MyPropertySheet.h : header file
//
// This class defines custom modal property sheet 
// CMyPropertySheet.
 
#ifndef __MYPROPERTYSHEET_H__
#define __MYPROPERTYSHEET_H__

#include "MyPropertyPage1.h"
#include "MyPropertyPage4.h"

/////////////////////////////////////////////////////////////////////////////
// CMyPropertySheet

class CMyPropertySheet : public CPropertySheet
{
	DECLARE_DYNAMIC(CMyPropertySheet)

// Construction
public:
	CMyPropertySheet(CWnd* pWndParent = NULL);

// Attributes
public:
	CMyPropertyPage1 m_Page1;
	CMyPropertyPage2 m_Page2;
	CMyPropertyPage3 m_Page3;
	CMyPropertyPage4 m_Page4;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMyPropertySheet)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMyPropertySheet();

// Generated message map functions
protected:
	//{{AFX_MSG(CMyPropertySheet)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

#endif	// __MYPROPERTYSHEET_H__
