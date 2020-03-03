/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageInsider/ImageInsider.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// ImageInsider.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "ImageInsider.h"
#include "ImageInsiderDlg.h"
#include "MyPropertySheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// exception
#include <Imagepp/all/h/HPAException.h>
#include <Imagepp/all/h/HRFException.h>
#include <stdexcpt.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HCDException.h>

//-----------------------------------------------------------------------------
// Header Files
//-----------------------------------------------------------------------------
//#include <Imagepp/all/h/Hstdcpp.h>
#include <Imagepp/all/h/HFCPtr.h>
#include <Imagepp/all/h/HRFRasterFile.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HFCURL.h>
#include <Imagepp/all/h/HFCURLFile.h>


//-----------------------------------------------------------------------------
// HRF Includes 
//-----------------------------------------------------------------------------
#include <Imagepp/all/h/HRFFileFormats.h>

#include <BeSQLite/L10N.h>
#include <BeSQLite/BeSQLite.h>

/////////////////////////////////////////////////////////////////////////////
// CImageInsiderApp

BEGIN_MESSAGE_MAP(CImageInsiderApp, CWinApp)
	//{{AFX_MSG_MAP(CImageInsiderApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImageInsiderApp construction

CImageInsiderApp::CImageInsiderApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CImageInsiderApp object

CImageInsiderApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CImageInsiderApp destruction

CImageInsiderApp::~CImageInsiderApp()
{
    //Terminate ImagePP lib host
    ImagePP::ImageppLib::GetHost().Terminate(true);

}

IMPLEMENT_DEFAULT_IMAGEPP_LIBHOST(MyImageppLibHost)

/////////////////////////////////////////////////////////////////////////////
// CImageInsiderApp initialization

BOOL CImageInsiderApp::InitInstance()
{
#ifdef IPP_USING_STATIC_LIBRARIES
    #error TODO resource support.
    //HFCResourceLoader::GetInstance()->SetModuleInstance(AfxGetInstanceHandle());
#endif

    // BeSQLiteLib initialization. Must be called only once. Needed by L10N
    WChar tempPath[MAX_PATH] = L"";
    GetTempPathW(MAX_PATH, tempPath);
    BeFileName tempDir(tempPath);
    BeAssert(tempDir.DoesPathExist());
    BeSQLite::BeSQLiteLib::Initialize(tempDir);

    // L10N initialization
    WChar fullExePath[_MAX_PATH];
    ::GetModuleFileNameW(NULL, fullExePath, _MAX_PATH);
    WChar fullExeDrive[_MAX_DRIVE];
    WChar fullExeDir[_MAX_DIR];
    _wsplitpath_s(fullExePath, fullExeDrive, _MAX_DRIVE, fullExeDir, _MAX_DIR, NULL, 0, NULL, 0);
    WChar baseDirW[_MAX_PATH];
    _wmakepath_s(baseDirW, fullExeDrive, fullExeDir, NULL, NULL);

    BeFileName defaultFile(baseDirW);
    defaultFile.AppendToPath(L"Assets/imageppApps_en.sqlang.db3");
    BeAssert(defaultFile.DoesPathExist());
    BentleyStatus l10nStatus = BeSQLite::L10N::Initialize(BeSQLite::L10N::SqlangFiles(defaultFile));
    BeAssert(BentleyStatus::SUCCESS == l10nStatus);

    //Initialize ImagePP host
    ImagePP::ImageppLib::Initialize(*new MyImageppLibHost());

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.
   if (AfxGetApp()->m_lpCmdLine[0] != _TEXT('\0'))
   {

      // Open a file passed as the first command line parameter.
  try 
  {
      // Remove " around the name.
      if (AfxGetApp()->m_lpCmdLine[0] == _TEXT('"'))
      {
          AfxGetApp()->m_lpCmdLine++;
          AfxGetApp()->m_lpCmdLine[_tcsclen(AfxGetApp()->m_lpCmdLine)-1] = 0;
      }
      
      // try to instanciate directly the given argument
      HFCPtr<HFCURL>  SrcFileName(HFCURL::Instanciate(Utf8String(AfxGetApp()->m_lpCmdLine)));
      if (SrcFileName == 0)
      {
          // Open the raster file as a file
          SrcFileName = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + Utf8String(AfxGetApp()->m_lpCmdLine));     
      }

      if (HRFRasterFileFactory::GetInstance()->FindCreator(SrcFileName) != 0) 
      {
	    CMyPropertySheet propSheet;
        m_pMainWnd = &propSheet;

	    INT_PTR nResponse = propSheet.DoModal();

        AfxMessageBox(_TEXT("End"));

	    //CImageInsiderDlg dlg;
	    //m_pMainWnd = &dlg;
	    //int nResponse = dlg.DoModal();
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
      }
  }
  catch (...)
  {
    Utf8String Msg;

    try
    {
        throw;
    }
    catch (HFCException& rException)
    {
        HandleIppException(rException, Msg);
    }    
    catch(logic_error&)
    {
        Msg = "Logic error";
    }
    catch(runtime_error&)
    {
        Msg = "Runtime error";
    }
    catch(exception&)
    {
        Msg = "Standard lib. error";
    }    
    catch(...)
    {
        Msg = "Unknown error";
    }

    AfxMessageBox(WString(Msg.c_str(), BentleyCharEncoding::Utf8).c_str());
  }
  }
  else
  {
        AfxMessageBox(_TEXT("To execute ImageInsider, drag an image over it."));
  }
  // Since the dialog has been closed, return false so that we exit the
	//  application, rather than start the application's message pump.
	return false;
}


/////////////////////////////////////////////////////////////////////////////
// Default handling of Imagepp exceptions

void CImageInsiderApp::HandleIppException(const HFCException& pi_rObj, 
                                          Utf8String&            po_rErrMsg) const
{    
    HPRECONDITION(po_rErrMsg.size() == 0);

    po_rErrMsg = pi_rObj.GetExceptionMessage();

    // display filename.
    if (dynamic_cast<HFCFileException const*>(&pi_rObj) != 0)
    {
        po_rErrMsg += " - Filename: ";
        po_rErrMsg += ((HFCFileException&)pi_rObj).GetFileName();
    }
}


/////////////////////////////////////////////////////////////////////////////
// static - Return the application's instance

const CImageInsiderApp* CImageInsiderApp::GetInstance()
{
    return &theApp;
}
