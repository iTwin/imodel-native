/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/HMRFactor/HMRFactor.cpp $
|    $RCSfile: HMRFactor.cpp,v $
|   $Revision: 1.2 $
|       $Date: 2011/07/18 21:12:28 $
|     $Author: Donald.Morissette $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// HMRFactor.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "HMRFactor.h"
#include "HMRFactorDlg.h"

#include <ImagePP/all/h/HRFFileFormats.h>
#include <Imagepp/all/h/ImageppLib.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// HMRFactorApp

BEGIN_MESSAGE_MAP(HMRFactorApp, CWinApp)
        ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()


// HMRFactorApp construction

HMRFactorApp::HMRFactorApp()
{
        // TODO: add construction code here,
        // Place all significant initialization in InitInstance
}


// The one and only HMRFactorApp object

HMRFactorApp theApp;


// HMRFactorApp initialization
struct MyImageppLibHost : ImagePP::ImageppLib::Host             
    {                                                       
    virtual void _RegisterFileFormat() override             
        {                                                       
        HOST_REGISTER_FILEFORMAT(HRFHMRCreator)
        }                                                       
    };                                      


BOOL HMRFactorApp::InitInstance()
{
        //Initialize ImagePP host
        ImagePP::ImageppLib::Initialize(*new MyImageppLibHost());

        // InitCommonControls() is required on Windows XP if an application
        // manifest specifies use of ComCtl32.dll version 6 or later to enable
        // visual styles.  Otherwise, any window creation will fail.
        InitCommonControls();

        CWinApp::InitInstance();

        AfxEnableControlContainer();

        // Standard initialization
        // If you are not using these features and wish to reduce the size
        // of your final executable, you should remove from the following
        // the specific initialization routines you do not need
        // Change the registry key under which our settings are stored
        // TODO: You should modify this string to be something appropriate
        // such as the name of your company or organization
        SetRegistryKey(_T("Local AppWizard-Generated Applications"));

        HMRFactorDlg dlg;
        m_pMainWnd = &dlg;
        INT_PTR nResponse = dlg.DoModal();
        if (nResponse == IDOK)
        {
                // TODO: Place code here to handle when the dialog is
                //  dismissed with OK
        }
        else if (nResponse == IDCANCEL)
        {
                // TODO: Place code here to handle when the dialog is
                //  dismissed with Cancel
        }

        //Terminate ImagePP lib host
        ImagePP::ImageppLib::GetHost().Terminate(true);

        // Since the dialog has been closed, return false so that we exit the
        //  application, rather than start the application's message pump.
        return false;
}
