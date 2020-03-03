//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/ActiveImage/ActiveImage.cpp $
//:>    $RCSfile: ActiveImage.cpp,v $
//:>   $Revision: 1.13 $
//:>       $Date: 2011/07/18 21:10:35 $
//:>     $Author: Donald.Morissette $
//:>
//:>  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class: ActiveImage
// ----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Header Files
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include <afxsock.h>		// MFC socket extensions

#include "ActiveImage.h"
#include "ActiveImageFrame.h"
#include "ActiveImageDoc.h"
#include "ActiveImageView.h"
#include "ActiveImageSplash.h"
#include "ActiveImageFileDialog.h"
#include "HMRStress.h"
#include "iostream"
#include "fstream"

#include "OpenInternetFileDialog.h"

#include <crtdbg.h>

#include <ImagePP/all/h/HRFFileFormats.h>
#include <Imagepp/all/h/ImageppLib.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HRFRasterFile.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HRFException.h>

#include <Imagepp/all/h/HCDCodecHMRCCITT.h>
#include <Imagepp/all/h/HPAException.h>
#include <Imagepp/all/h/HRFException.h>

#include <stdexcpt.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HCDException.h>
#include <Imagepp/all/h/HFCMath.h>
#include <BeSQLite/L10N.h>
#include <BeSQLite/BeSQLite.h>

#include <Windows.h>

#ifdef __IPP_EXTERNAL_THIRD_PARTY_SUPPORTED     // IPP_HAVE_PDF_SUPPORT
#include <PdfLibInitializer/PdfLibInitializer.h>
#endif
//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#ifdef _DEBUG
static CMemoryState oldstate, newstate, diffstate;

//#define new DEBUG_NEW
//#undef THIS_FILE
//static char THIS_FILE[] = __FILE__;
#endif

#define BUILD_ID    110
#define BUILD_MONTH "September"


#define  SECTION_MEMORY             _TEXT("MemorySetting")
#define  MEMORY_OBJECTSTORE_MEM     _TEXT("ObjectStoreLog_Mem")
#define  DEF_OBJECTSTORE_MEM        50*1024
#define  MEMORY_BUFFERIMG_MEM       _TEXT("BufferImgLog_Mem")
#define  DEF_BUFFERIMG_MEM          30*1024


#define  DEFAULT_ERR_FILENAME		_TEXT("C:\\ActiveImage.err")
#define  DEFAULT_LOG_FILENAME		_TEXT("C:\\ActiveImage.log")
#define  DEFAULT_OUT_FILENAME		_TEXT("C:\\ActiveImage.out")

#ifndef ACTIVEIMAGE_OUT_FILENAME
#define ACTIVEIMAGE_OUT_FILENAME	DEFAULT_OUT_FILENAME
#endif

#ifndef ACTIVEIMAGE_LOG_FILENAME
#define ACTIVEIMAGE_LOG_FILENAME	DEFAULT_LOG_FILENAME
#endif

#ifndef ACTIVEIMAGE_ERR_FILENAME
#define ACTIVEIMAGE_ERR_FILENAME    DEFAULT_ERR_FILENAME
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ActiveImage_BeAssertHandler (wchar_t const* _Message, wchar_t const*_File, unsigned _Line, BentleyApi::BeAssertFunctions::AssertType atype)
    {
    #ifndef NDEBUG
        _wassert (_Message, _File, _Line);
    #endif
    }

//-----------------------------------------------------------------------------
// CAboutDlg dialog used for App About
//-----------------------------------------------------------------------------
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};



//-----------------------------------------------------------------------------
// Message Map and other MFC Macros
//-----------------------------------------------------------------------------
BEGIN_MESSAGE_MAP(CActiveImageApp, CWinApp)
	//{{AFX_MSG_MAP(CActiveImageApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_OPENINTERNETFILE, OnInternetFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

// The one and only CActiveImageApp object
CActiveImageApp theApp;
HDEBUGCODE(filebuf s_TheOutFile);
HDEBUGCODE(filebuf s_TheLogFile);
HDEBUGCODE(filebuf s_TheErrFile);


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     10/2010
//---------------------------------------------------------------------------------------
static WString getBaseDirOfExecutingModule ()
    {
    WChar fullExePath[_MAX_PATH];
    ::GetModuleFileNameW (NULL, fullExePath, _MAX_PATH);

    WChar fullExeDrive[_MAX_DRIVE];
    WChar fullExeDir[_MAX_DIR];
    _wsplitpath_s (fullExePath, fullExeDrive, _MAX_DRIVE, fullExeDir, _MAX_DIR, NULL, 0, NULL, 0);

    WChar baseDirW[_MAX_PATH];
    _wmakepath_s (baseDirW, fullExeDrive, fullExeDir, NULL, NULL);

    return baseDirW;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
#ifdef __IPP_EXTERNAL_THIRD_PARTY_SUPPORTED     // IPP_HAVE_PDF_SUPPORT
struct ActiveImagePdfLibInitializerAdmin : Bentley::PdfLibInitializerAdmin
    {
protected:
    virtual BentleyStatus _GetPDFDataPath
        (
        WStringR cMapDir,
        WStringR unicodeDir,
        WStringR fontDir,
        WStringR colorDir,
        WStringR pluginDir
        ) const override
        {
        // Setup location to find PDF resource files (was created under exe location)...

        WString baseDir = getBaseDirOfExecutingModule ();

        cMapDir = baseDir + WString(L"System\\Data\\PDFL\\CMap");
        fontDir = baseDir + WString(L"System\\Data\\PDFL\\Font");

        // Note: TR 311887 results if the PDFL/Unicode directory is unspecified.
        unicodeDir = baseDir + WString(L"System\\Data\\PDFL\\Unicode");

        return BSISUCCESS;
        }

    virtual bool   _IsPDFLibAvailable() const override  {return true;}

    };// ViewDemoPdfLibInitializerAdmin


/*=================================================================================**//**
* @bsiclass                                     		Marc.Bedard     02/2011
+===============+===============+===============+===============+===============+======*/
struct ActiveImagePdfLibInitializerHost : Bentley::PdfLibInitializer::Host 
{
    virtual PdfLibInitializerAdmin& _SupplyPdfLibInitializerAdmin() override { return *new ActiveImagePdfLibInitializerAdmin(); }
}; // ViewDemoPdfLibInitializerHost
#endif
/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ActiveImageImageppLibAdmin : ImagePP::ImageppLibAdmin
    { 
    DEFINE_T_SUPER(ImagePP::ImageppLibAdmin)

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                            05/2012
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual ~ActiveImageImageppLibAdmin()
        {
        }

    virtual BentleyStatus _GetGDalDataPath(BeFileNameR path) const 
        { 
        BeFileName GdalPath(getBaseDirOfExecutingModule());
        GdalPath.AppendToPath(L"Assets/gdal");

        path = GdalPath;
        return BSISUCCESS; 
        }
    };


/*=================================================================================**//**
* @bsiclass                                     		Marc.Bedard     02/2011
+===============+===============+===============+===============+===============+======*/
struct ActiveImageImageppLibHost : ImagePP::ImageppLib::Host 
{
    virtual ImagePP::ImageppLibAdmin& _SupplyImageppLibAdmin() override
        {
        return *new ActiveImageImageppLibAdmin();
        }
    virtual void _RegisterFileFormat() override 
        {
        REGISTER_SUPPORTED_FILEFORMAT 
        }

}; // ActiveImageImageppLibHost



//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
CActiveImageApp::CActiveImageApp()
{	
#ifdef _DEBUG
#if 0 //DMx
	HRFRasterFileFactory::GetInstance()->SetFileSpecificProperty(ERMAPPER_CUSTOM_DATUM_FILE_PATH,
																 _TEXT("d:\\t\\"));
#endif

    oldstate.Checkpoint();
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );

static bool CheckHeap = false;
//HASSERT(_CrtCheckMemory());
static long MemCount;
_CrtSetBreakAlloc(MemCount);
    if (CheckHeap)
    {
        int tmpFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
        // _CRTDBG_CHECK_ALWAYS_DF |
        _CrtSetDbgFlag(tmpFlag | _CRTDBG_CHECK_CRT_DF | _CRTDBG_DELAY_FREE_MEM_DF);
        //  | checkAlwaysMemDF
        afxMemDF = allocMemDF | delayFreeMemDF   ;
    }
#endif

#ifdef __IPP_EXTERNAL_THIRD_PARTY_SUPPORTED     // IPP_HAVE_PDF_SUPPORT
    //Application needs to initialize PdfLibInitializer dll if it wants support for PDF raster attachment.
    Bentley::PdfLibInitializer::Initialize(*new ActiveImagePdfLibInitializerHost());
#endif

    // BeSQLiteLib initialization. Must be called only once. Needed by L10N
    WChar tempPath[MAX_PATH] = L"";
    GetTempPathW(MAX_PATH, tempPath);
    BeFileName tempDir(tempPath);
    BeAssert (tempDir.DoesPathExist());
    BeSQLite::BeSQLiteLib::Initialize (tempDir);

    // L10N initialization
    BeFileName defaultFile (getBaseDirOfExecutingModule());
    defaultFile.AppendToPath (L"Assets/imageppApps_en.sqlang.db3");
    BeAssert (defaultFile.DoesPathExist());
    BentleyStatus l10nStatus = BeSQLite::L10N::Initialize( BeSQLite::L10N::SqlangFiles (defaultFile)); 
    BeAssert(BentleyStatus::SUCCESS == l10nStatus);

    //Initialize ImagePP host
    ImagePP::ImageppLib::Initialize(*new ActiveImageImageppLibHost());

    InitializeGCoord();
}

void CActiveImageApp::InitializeGCoord()
    {
    //Get .exe path
    wchar_t buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    WString s(buffer);
    size_t index = s.find_last_of(L"/\\");
    WString path = s.substr(0, index).append(L"\\Assets\\DgnGeoCoord\\");

    //Initialize
    GeoCoordinates::BaseGCS::Initialize(path.c_str());
    }


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
CActiveImageApp::~CActiveImageApp()
{
#ifdef _DEBUG
    newstate.Checkpoint();
    if (diffstate.Difference(oldstate, newstate))
    {
        TRACE("Memory leaked\n");
        diffstate.DumpStatistics();
    }
#endif
    //Terminate ImagePP lib host
    ImagePP::ImageppLib::GetHost().Terminate(true);

#ifdef __IPP_EXTERNAL_THIRD_PARTY_SUPPORTED     // IPP_HAVE_PDF_SUPPORT
    //Terminate PdfLibInitializer lib host
    Bentley::PdfLibInitializer::GetHost().Terminate(true);
#endif

    BeSQLite::L10N::Shutdown();
    }


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
BOOL CActiveImageApp::InitInstance()
{
    // Fix crash when passing an image as the first param on the cmd line.
    //AfxSetAmbientActCtx(FALSE); No longer supported in visual studio 2012

    AfxEnableControlContainer();
	// Initialize Winsock
	if (!AfxSocketInit())
	{
		AfxMessageBox(_TEXT("Winsock Initialization Error"));
		return false;
	}

	// CG: The following block was added by the Splash Screen component.
	{
		CCommandLineInfo cmdInfo;
		ParseCommandLine(cmdInfo);

#ifdef _DEBUG
		cmdInfo.m_bShowSplash = false;
#endif
		CActiveImageSplashWnd::EnableSplashScreen(cmdInfo.m_bShowSplash);
	}
	// Standard initialization

	LoadStdProfileSettings(8);  // Load standard INI file options (including MRU)

    // Profile ...
    m_ObjectStore_Mem = GetProfileInt (SECTION_MEMORY, MEMORY_OBJECTSTORE_MEM, DEF_OBJECTSTORE_MEM);
    WriteProfileInt (SECTION_MEMORY, MEMORY_OBJECTSTORE_MEM, m_ObjectStore_Mem);

    m_BufferImg_Mem = GetProfileInt (SECTION_MEMORY, MEMORY_BUFFERIMG_MEM, DEF_BUFFERIMG_MEM);
    WriteProfileInt (SECTION_MEMORY, MEMORY_BUFFERIMG_MEM, m_BufferImg_Mem);



	// Register document templates

	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CActiveImageDoc),
		RUNTIME_CLASS(CActiveImageFrame),       // main SDI frame window
		RUNTIME_CLASS(CActiveImageView));
	AddDocTemplate(pDocTemplate);

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	// Dispatch commands specified on the command line
	if (!ProcessShellCommand(cmdInfo))
		return false;

    BOOL allocResult = AllocConsole();
    FILE* pFile;
    freopen_s(&pFile, "CONOUT$", "w", stdout);
    printf("ActiveImage console \n");

    m_pMainWnd->DragAcceptFiles();

    // Add our assert handler that will avoid the call to 'DebugBreak()' when an assert occurs.
    BentleyApi::BeAssertFunctions::SetBeAssertHandler(ActiveImage_BeAssertHandler);

#ifdef STRESS_APPS
	LoadImageList(gImageList);
	LoadServerInfo();
#endif

#if 0
//DMx Testing
//-----------------------------------------------------------------------------------------
#define ForCount    0x5fffffff

    srand((unsigned)time( NULL ));

    uint32_t aResult_0[1000];
    clock_t StartTime0 = clock();
    for(int32_t Index = 0; Index < ForCount; Index++)
    {
//        double Number = rand() / (double)RAND_MAX;
//        Byte Character = (char)(Number * 255.0);

        aResult_0[Index % 1000] = CONVERT_8BIT_TO_32BITx(Index & 0x000000ff);
    }    
    clock_t StopTime0 = clock();  

    
    uint32_t aResult_1[1000];
    clock_t StartTime1 = clock();
    for(int32_t Index = 0; Index < ForCount; Index++)
    {
        aResult_1[Index % 1000] = CONVERT_8BIT_TO_32BITxx(Index & 0x000000ff);
    }    
    clock_t StopTime1 = clock();  

    uint32_t aResult_2[1000];
    clock_t StartTime2 = clock();
    for(int32_t Index = 0; Index < ForCount; Index++)
    {
        aResult_2[Index % 1000] = CONVERT_8BIT_TO_32BIT(Index & 0x000000ff);
    }    
    clock_t StopTime2 = clock();  

    HFCMath (*pMath) (HFCMath::GetInstance());

    uint32_t aResult_3[1000];
    clock_t StartTime3 = clock();
    for(int32_t Index = 0; Index < ForCount; Index++)
    {
        aResult_3[Index % 1000] = pMath->MultiplyBy0X01010101(Index & 0x000000ff);
    }    
    clock_t StopTime3 = clock();  

// Division
    Byte aResultUINT_1[1000];
    clock_t StartTime4 = clock();
    for(int32_t Index = 0; Index < ForCount; Index++)
    {
//        double Number = rand() / (double)RAND_MAX;
//        UInt32 Val = (UInt32)(Number * UINT_MAX);

        aResultUINT_1[Index % 1000] = CONVERT_32BIT_TO_8BIT(Index);
    }    
    clock_t StopTime4 = clock();  
    
    Byte aResultUINT_2[1000];
    clock_t StartTime5 = clock();
    for(int32_t Index = 0; Index < ForCount; Index++)
    {
        aResultUINT_2[Index % 1000] = (Byte)(Index / 0X01010101);
    }    
    clock_t StopTime5 = clock();  


    WChar Msg[2048];
    _stprintf(Msg,_TEXT("Computetime:\n[<<24]         Time(sec):%.3f \
                                     \n[* 0x01010101] Time(sec):%.3f \
                                     \n[or and shift] Time(sec):%.3f \
                                     \n[Table       ] Time(sec):%.3f \
                                     \n[>>24]         Time(sec):%.3f \
                                     \n[\\ 0x01010101] Time(sec):%.3f"), 
                        (double)(StopTime0 - StartTime0) / CLOCKS_PER_SEC,
                        (double)(StopTime1 - StartTime1) / CLOCKS_PER_SEC,
                        (double)(StopTime2 - StartTime2) / CLOCKS_PER_SEC,
                        (double)(StopTime3 - StartTime3) / CLOCKS_PER_SEC,
                        (double)(StopTime4 - StartTime4) / CLOCKS_PER_SEC,
                        (double)(StopTime5 - StartTime5) / CLOCKS_PER_SEC);
    AfxMessageBox(Msg);


    for(uint32_t i=0; i<1000; ++i)
    {
        if (aResultUINT_1[i] == aResultUINT_2[i])
        {
            AfxMessageBox(_TEXT("aResultUINT =="));
            break;
        }

        if (aResult_0[i] == aResult_1[i])
        {
            AfxMessageBox(_TEXT("aResultUINT =="));
            break;
        }
        if (aResult_2[i] == aResult_3[i])
        {
            AfxMessageBox(_TEXT("aResultUINT =="));
            break;
        }

    }
    
#endif
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
#if 0
//#include <windows.h>
//#include <tchar.h> 

    typedef HRESULT (*DllReg)(void); 

//    int _tmain(int argc, _TCHAR* argv[])
    {
        DWORD err = 0;
//        LPCWSTR dll = argv[1]; 

        HMODULE module = LoadLibrary("J:\\output\\I++ Debug\\bin\\HMRImageCtrlPro.ocx");
        if ( module == NULL ) {
            err = GetLastError();
        } else {
            DllReg dllReg = (DllReg)GetProcAddress(module,"DllRegisterServer");
            if ( dllReg == NULL ) {
                err = GetLastError();
            } else {
                dllReg();
            }
        }
        if ( err ) {
            LPTSTR msg;
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,0,err,0,(LPTSTR)&msg,0,NULL);
            _tprintf(msg);
            LocalFree(msg);
        }

    }
#endif

#if 0
    const IID BASED_CODE IID_Test = {0x31a2b283, 0x8c11, 0x11cf, 0xb3, 0x7f, 0, 0xaa, 0, 0xb9, 0x2b, 0x50};
    const IID BASED_CODE IID_HMRControl = { 0xd83e0111, 0xe9a4, 0x11d1, 0xa5, 0xb7, 0x0, 0x60, 0x8, 0x2b, 0xd9, 0x7a };
    const IID BASED_CODE IID_DSpitfire  = { 0x40ab0ae6, 0xe9a0, 0x11d1, 0xa5, 0xb7, 0x0, 0x60, 0x8, 0x2b, 0xd9, 0x7a };
    HRESULT R1=CoInitialize(NULL);
    
    IUnknown* pIUnknown = 0;
    // Create COM object instance to get specifications and capabilities of this file format
    HRESULT R2=CoCreateInstance(IID_HMRControl, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (void**)&pIUnknown);
        
    CComPtr<IClassFactory> spClassFactory;        
//     HRESULT R3=CoGetClassObject(IID_HMRControl, CLSCTX_INPROC_SERVER  , NULL, IID_IClassFactory, (void**)&spClassFactory); 
//     HRESULT R4= = spClassFactory->CreateInstance(pUnkOuter, riid, ppvObj) 
//     spClassFactory->Release();         
        
//    TCHAR Msg[256];
//    _stprintf(Msg, "CoCreateInstance code: %d", R2);
//    AfxMessageBox(Msg);             
        
#endif
//-----------------------------------------------------------------------------------------

	return true;
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageApp::OnFileOpen( )
{
    CString                 Filters;
    CActiveImageFileDialog* pDialog;

    // Get the dialog
    pDialog = GetFileDialog(true, false);
    
    // display the dialog
    if (pDialog->DoModal() == IDOK)
    {
        POSITION FilePos = pDialog->GetStartPosition();

        // Fill the list with the files infos
        while(FilePos)
            OpenDocumentFile(pDialog->GetNextPathName(FilePos));
     
//        OpenDocumentFile(pDialog->GetPathName());
    }

    // destroy the dialog
    delete pDialog;
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageApp::OnInternetFileOpen( )
{
	CString                 Filters;
	OpenInternetFileDialog	OpenDialog;			
	
	// display the dialog
	if (OpenDialog.DoModal() == IDOK)
	{				
		CSingleDocTemplate* pDocTemplate = 0;
		CActiveImageDoc* pActiveImageDoc = 0;

		POSITION a = GetFirstDocTemplatePosition();

		pDocTemplate = (CSingleDocTemplate*)GetNextDocTemplate(a);
		
		POSITION b = pDocTemplate->GetFirstDocPosition();

		pActiveImageDoc = (CActiveImageDoc*)pDocTemplate->GetNextDoc(b);
		
		pActiveImageDoc->OnOpenInternetDocument(OpenDialog.GetUrl());		
	}	
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
BOOL CActiveImageApp::PreTranslateMessage(MSG* pMsg)
{
	// CG: The following lines were added by the Splash Screen component.
	if (CActiveImageSplashWnd::PreTranslateAppMessage(pMsg))
		return true;

	return CWinApp::PreTranslateMessage(pMsg);
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
CActiveImageFileDialog* CActiveImageApp::GetFileDialog(BOOL pi_bOpenFileDialog,
                                                       BOOL pi_bShowExtra)
{
    CActiveImageFileDialog* pDialog;
    CString                 Filters;
    CString                 ImageExt;   
    uint32_t                 i;
    
////////////////////////////////////////
// Setup the filters for the file dialog
////////////////////////////////////////

    HRFRasterFileFactory::Creators  ListFile = HRFRasterFileFactory::GetInstance()->GetCreators(HFC_READ_ONLY);
    // add all known formats filters
    uint32_t CountFormat=0;
    for (i=0; i < ListFile.size(); i++)
    {
        if (((HRFRasterFileCreator*)ListFile[i])->SupportsScheme(HFCURLFile::s_SchemeName()) &&
            ((HRFRasterFileCreator*)ListFile[i])->GetExtensions().c_str() != 0)
        {
            // Generate the filter entry
            Filters += ListFile[i]->GetLabel().c_str();
            Filters += " Files (";
            Filters += ((HRFRasterFileCreator*)ListFile[i])->GetExtensions().c_str();
            Filters += ")|";
            Filters += ((HRFRasterFileCreator*)ListFile[i])->GetExtensions().c_str();
            Filters += "|";

            // add the current images to the image extensions
            ImageExt += ((HRFRasterFileCreator*)ListFile[i])->GetExtensions().c_str();
            if (i < ListFile.size() - 1)
                ImageExt += ";";

            CountFormat++;
        }
    }

    // Add the PSS format manually
    {
        // Generate the filter entry
        Filters += "PictureScript";
        Filters += " Files (";
        Filters += "*.pss";
        Filters += ")|";
        Filters += "*.pss";
        Filters += "|";

        // add the current images to the image extensions
        ImageExt += ";";
        ImageExt += "*.pss";
    }

    CountFormat++;

    // add the "All Image Files" filter
    Filters += "All Image Files (";
    Filters += ImageExt;
    Filters += ")|";
    Filters += ImageExt;
    Filters += "|";

    // Add the "All files" filter
    Filters += "All Files (";
    Filters += "*.*";
    Filters += ")|";
    Filters += "*.*";
    Filters += "||";


////////////////////////////////////////
// Setup the file open dialog
////////////////////////////////////////

    pDialog = new CActiveImageFileDialog(pi_bOpenFileDialog, pi_bShowExtra, NULL, NULL, 
                                         OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT, 
                                         Filters);
    HASSERT(pDialog != 0);

    // place the current filter
    if (pi_bOpenFileDialog)
        pDialog->m_ofn.nFilterIndex = CountFormat + 2;
    else
        pDialog->m_ofn.nFilterIndex = 1;

    return (pDialog);
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

    return true;  // return true unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return false
}


void ExceptionHandler(const HFCException& pi_rObj)
{
    WString Msg;

    Msg.AssignUtf8(pi_rObj.GetExceptionMessage().c_str());

    // display filename.
    if (dynamic_cast<HFCFileException const*>(&pi_rObj) != 0)
    {
        Msg.append(L" - Filename: ");
        Msg.AppendUtf8(((HFCFileException&)pi_rObj).GetFileName().c_str());
    }


#if 0
    catch(logic_error&)
    {
        AfxMessageBox("Logic Error.");
    }
    catch(runtime_error&)
    {
        AfxMessageBox("Runtime Error.");
    }
    catch(exception&)
    {
        AfxMessageBox("Standard lib. error");
    }
    catch(HODDatabase::Error& pi_rError)
    {
        char Number[5];
        sprintf(Number, "%i", pi_rError.type);
        string Msg = string("HOD Database error ").append(Number);
        AfxMessageBox(Msg.c_str());
    }
    catch(...)
    {
        AfxMessageBox("Unknown error");
    }
#endif

    AfxMessageBox(Msg.c_str());
}

