/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageInsider/ImageInsider.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// ImageInsider.h : main header file for the IMAGEINSIDER application
//
#if !defined(AFX_IMAGEINSIDER_H__70E2D9A8_ABEB_11D3_B587_0060082BD950__INCLUDED_)
#define AFX_IMAGEINSIDER_H__70E2D9A8_ABEB_11D3_B587_0060082BD950__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CImageInsiderApp:
// See ImageInsider.cpp for the implementation of this class
//
class CImageInsiderApp : public CWinApp
{
public:
	void OnProperties();
	CImageInsiderApp();
    ~CImageInsiderApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImageInsiderApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CImageInsiderApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
DECLARE_MESSAGE_MAP()
  

public :
    void HandleIppException(const HFCException& pi_rObj, 
                            Utf8String&            po_rErrMsg) const;

    //Static method  
    static const CImageInsiderApp* GetInstance();

};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMAGEINSIDER_H__70E2D9A8_ABEB_11D3_B587_0060082BD950__INCLUDED_)
