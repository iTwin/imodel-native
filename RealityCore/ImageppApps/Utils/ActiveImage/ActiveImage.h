/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ActiveImage/ActiveImage.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// ----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/ActiveImage/ActiveImage.h,v 1.2 2006/12/11 16:17:17 Mathieu.St-Pierre Exp $
//
// Class: ActiveImageDoc
// ----------------------------------------------------------------------------

#ifndef __ACTIVEIMAGE_H__
#define __ACTIVEIMAGE_H__

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

//-----------------------------------------------------------------------------
// Header Files
//-----------------------------------------------------------------------------
#include "resource.h"       // main symbols

#include <Imagepp/all/h/HFCException.h>

// Exception Handling 
void  ExceptionHandler(const HFCException& pi_rObj);

class CActiveImageFileDialog;

class CActiveImageApp : public CWinApp
{
    ////////////////////////////////////////
    // Constructor / Destructor
    ////////////////////////////////////////
    public:
        CActiveImageApp();
        virtual ~CActiveImageApp();

   
    ////////////////////////////////////////
    // Utility Methods
    ////////////////////////////////////////
    public:
        //const HRFHMRRasterFileRegistry* GetRegistry() const;
        CActiveImageFileDialog* GetFileDialog(BOOL pi_bOpenFileDialog,
                                              BOOL pi_bShowExtra);
        const OSVERSIONINFO* GetOSVersion() const;


        int GetObjectStore_Mem      () const {return m_ObjectStore_Mem;}
        int GetBufferImg_Mem        () const {return m_BufferImg_Mem;}

    private:
        void InitializeGCoord();




    ////////////////////////////////////////
    // Overrides
    ////////////////////////////////////////
    public:
        virtual BOOL PreTranslateMessage(MSG* pMsg);
	    // ClassWizard generated virtual function overrides
	    //{{AFX_VIRTUAL(CActiveImageApp)
	public:
	    virtual BOOL InitInstance();
	//}}AFX_VIRTUAL


    ////////////////////////////////////////
    // Message Map
    ////////////////////////////////////////

        afx_msg void OnFileOpen();
		afx_msg void OnInternetFileOpen();
	    //{{AFX_MSG(CActiveImageApp)
	    afx_msg void OnAppAbout();
	    //}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    ////////////////////////////////////////
    // Attributes
    ////////////////////////////////////////
    private:

        // The registry used to open raster files
        //HRFHMRRasterFileRegistry    m_FileFormatRegistry;
        OSVERSIONINFO               m_OSInfo;


        // Memory Settings
        int     m_ObjectStore_SelectMem; 
        int     m_ObjectStore_Mem;
        int     m_ObjectStore_Tiles;
        int     m_BufferImg_SelectMem;
        int     m_BufferImg_Mem;
        int     m_BufferImg_Tiles;
         

};

#include "ActiveImage.hpp"

extern CActiveImageApp NEAR theApp;

#endif
