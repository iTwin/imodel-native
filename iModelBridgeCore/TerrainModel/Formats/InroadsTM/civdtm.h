//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
// civdtm.h : main header file for the CIVDTM DLL
//

#if !defined(AFX_CIVDTM_H__39F5F508_9342_11D1_89B1_080036D6F502__INCLUDED_)
#define AFX_CIVDTM_H__39F5F508_9342_11D1_89B1_080036D6F502__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CCivdtmApp
// See civdtm.cpp for the implementation of this class
//

class CCivdtmApp : public CWinApp
{
public:
	CCivdtmApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCivdtmApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CCivdtmApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CIVDTM_H__39F5F508_9342_11D1_89B1_080036D6F502__INCLUDED_)
