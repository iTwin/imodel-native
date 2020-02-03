//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Utils/ActiveImage/ActiveImageDoc.cpp $
//:>    $RCSfile: ActiveImageDoc.cpp,v $
//:>   $Revision: 1.76 $
//:>       $Date: 2011/07/18 21:10:37 $
//:>     $Author: Donald.Morissette $
//:>
//:>  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Header Files
//-----------------------------------------------------------------------------
#include "stdafx.h"

#include "ActiveImage.h"
#include "ActiveImageDoc.h"
#include "ActiveImageView.h"
#include "ActiveImageFrame.h"
#include "AuthenticationDialog.h"
#include "ActiveImageFileDialog.h"
#include "InsertObjectDialog.h"
#include <Imagepp/all/h/HFCPtr.h>
#include <Imagepp/all/h/HGF2DCoordSys.h>
#include <Imagepp/all/h/HGF2DLocation.h>
#include <Imagepp/all/h/HRAReferenceToRaster.h>
#include <Imagepp/all/h/HIMFilteredImage.h>
#include <Imagepp/all/h/HIMTranslucentImageCreator.h>
#include <Imagepp/all/h/HRPConvFiltersV24R8G8B8.h>
#include <Imagepp/all/h/HVE2DRectangle.h>
#include <direct.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HPSObjectStore.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HCDCodecImage.h>
#include <Imagepp/h/HAutoPtr.h>
#include <Imagepp/all/h/HFCURL.h>
#include <Imagepp/all/h/HFCURLHTTP.h>
#include <Imagepp/all/h/HVE2DUniverse.h>
#include <Imagepp/all/h/HRFUtility.h>

#include <Imagepp/all/h/HUTImportFromFileExportToFile.h>
#include <Imagepp/all/h/HUTImportFromRasterExportToFile.h>

#include <Imagepp/all/h/HRADrawProgressIndicator.h>
#include <Imagepp/all/h/HUTExportProgressIndicator.h>
#include "CachePreferences.h"
#include "OpenRawFileDialog.h"
#include <Imagepp/all/h/HRPPixelType1BitInterface.h>

#include <ImageppApps/HUTExport/HUTExportDialog.h>
#include <ImageppApps/HUTExport/HUTProgressDialog.h>
#include <ImagePP/all/h/HRPDEMFilter.h>


// Improve 
#include <Imagepp/all/h/HRFiTiffCacheFileCreator.h>
#include <Imagepp/all/h/HRFRasterFileCache.h>
#include <Imagepp/all/h/HRFPageFileFactory.h>
#include <Imagepp/all/h/HRFRasterFilePageDecorator.h>
#include <Imagepp/all/h/HRFRasterFileBlockAdapter.h>
#include <Imagepp/all/h/HRFRasterFileResBooster.h>
#include <Imagepp/all/h/HRFExportOptions.h>
#include <Imagepp/all/h/HRFImportExport.h>
#include <Imagepp/all/h/HUTExportToFile.h>
#include <Imagepp/all/h/HRFSLOStripAdapter.h>
#include <Imagepp/all/h/HGFResolutionDescriptor.h>
#include <Imagepp/all/h/HRAReferenceToStoredRaster.h>
#include <Imagepp/all/h/HRADEMRaster.h>
#include <Imagepp/all/h/HRPContrastStretchFilter16.h>
#include <Imagepp/all/h/HGF2DProjective.h>
#include <Imagepp/all/h/HGF2DProjectiveGrid.h>
#include <Imagepp/all/h/HGF2DLinearModelAdapter.h>

#include <Imagepp/all/h/HFCStat.h>
#include <Imagepp/all/h/HFCExceptionHandler.h>

#include <Imagepp/all/h/HFCCallbackRegistry.h>
#include <Imagepp/all/h/HFCCallbacks.h>

#include <Imagepp/all/h/HRATIFFFileTransactionRecorder.h>

#include <Imagepp/all/h/HCPGCoordUtility.h>
#include <Imagepp/all/h/HUTLandSat8ToRGBA.h>

#define COMPRESS_DEFLATE    32946

#ifdef STRESS_APPS
#include <Imagepp/all/h/HMRStress.h>
#include "streesdlg.h"
#endif

#ifdef __SUPPORT_GEORASTER
// Oracle GeoRaster
#include <Imagepp/clr/h/SDOGeoRasterWrapper.h>
#endif

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Tree List Icons images offset in image list
#define TLI_DOCUMENT        0
#define TLI_RASTER          2
#define TLI_MOSAIC          4
#define TLI_LINK_RASTER     6
#define TLI_LINK_MOSAIC     8
#define TLI_REF_RASTER      10
#define TLI_REF_MOSAIC      12

//-----------------------------------------------------------------------------
// HRF Includes 
//-----------------------------------------------------------------------------
#include <Imagepp/all/h/HRFRasterFileFactory.h>

#include <Imagepp/all/h/HRFFileFormats.h>

#include ".\activeimagedoc.h"

// Conflict with the use of std::numeric_limits<T>::max()
#ifdef max
#undef max
#endif
// Conflict with the use of std::numeric_limits<T>::min()
#ifdef min
#undef min
#endif

//-----------------------------------------------------------------------------
// Callback
//-----------------------------------------------------------------------------
class AuthenticationCallback : public HFCAuthenticationCallback
    {
    public:
        AuthenticationCallback()
            : HFCAuthenticationCallback()
            {};

        virtual ~AuthenticationCallback()
            {};

        bool GetAuthentication(HFCAuthentication* pio_pAuthentication) const
            {
            AuthenticationDialog AuthenticationDlg(pio_pAuthentication);

            if (AuthenticationDlg.DoModal() == IDOK)
                return true;

            return false;
            };

        bool CanAuthenticate(HCLASS_ID pi_AuthenticationType) const
            {
            return true;
            }

        unsigned short RetryCount(HCLASS_ID pi_AuthenticationType) const
            {
            return 3;
            };

        bool IsCancelled() const
            {
            return false;
            };
    };



//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

// Global Reference Coord Sys
HFCPtr<HGF2DCoordSys>       g_pAIRefCoordSys;
AuthenticationCallback      g_AuthenticationCallback;




//-----------------------------------------------------------------------------
// Message Map and other MFC Macros
//-----------------------------------------------------------------------------
IMPLEMENT_DYNCREATE(CActiveImageDoc, CDocument)
BEGIN_MESSAGE_MAP(CActiveImageDoc, CDocument)
    //{{AFX_MSG_MAP(CActiveImageDoc)
    ON_COMMAND(ID_INSERT_FROMEXTERNALDOCUMENT, OnInsertFromExternalDocument)
    ON_UPDATE_COMMAND_UI(ID_INSERT_FROMEXTERNALDOCUMENT, OnUpdateInsertFromExternalDocument)
    ON_COMMAND(ID_INSERT_NEWOBJECT, OnInsertNewObject)
    ON_UPDATE_COMMAND_UI(ID_INSERT_NEWOBJECT, OnUpdateInsertNewObject)
    ON_COMMAND(ID_INSERT_FROMTHISDOCUMENT, OnInsertFromthisdocument)
    ON_UPDATE_COMMAND_UI(ID_INSERT_FROMTHISDOCUMENT, OnUpdateInsertFromthisdocument)
    ON_COMMAND(ID_FILE_EXPORT_HMR, OnFileExportHmr)
    ON_COMMAND(ID_FILE_EXPORT_CHANNELS, OnFileExportChannels)
    ON_UPDATE_COMMAND_UI(ID_VIEW_REMOVETRANSPARENCY, OnUpdateViewRemovetransparency)
    ON_COMMAND(ID_VIEW_REMOVETRANSPARENCY, OnViewRemovetransparency)
    ON_UPDATE_COMMAND_UI(ID_FILE_READONLY, OnUpdateFileReadonly)
    ON_COMMAND(ID_FILE_READONLY, OnFileReadonly)
    ON_COMMAND(ID_FILE_PREFERENCES_CACHECOMPRESSIONS, OnFilePreferencesCachecompressions)
    ON_UPDATE_COMMAND_UI(ID_FILE_EXPORT, OnUpdateFileExport)
    ON_UPDATE_COMMAND_UI(ID_FILE_EXPORT_REGION, OnUpdateFileExportRegion)
    ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateFileSave)
    ON_COMMAND(ID_FILE_OVERWRITEHGRTWF, OnFileOverwritehgrtwf)
    ON_UPDATE_COMMAND_UI(ID_FILE_OVERWRITEHGRTWF, OnUpdateFileOverwritehgrtwf)
    ON_UPDATE_COMMAND_UI(ID_FILE_INFO_TIME, OnUpdateFileInfoTime)
    ON_COMMAND(ID_FILE_INFO_TIME, OnFileInfoTime)
    ON_UPDATE_COMMAND_UI(ID_FILE_PREFERENCES_FITIMAGEONVIEW, OnUpdateFilePreferenceFitImageOnView)
    ON_COMMAND(ID_FILE_PREFERENCES_FITIMAGEONVIEW, OnFilePreferenceFitImageOnView)
    //}}AFX_MSG_MAP

#ifdef STRESS_APPS
    ON_COMMAND(IDM_STRESS, InternetTest)
#endif

    ON_COMMAND_RANGE(ID_VIEW_1BIT_EQUALWEIGHT, ID_VIEW_1BIT_BACKGROUND, OnView1bitForeground)
    ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_1BIT_EQUALWEIGHT, ID_VIEW_1BIT_BACKGROUND, OnUpdateView1bitForeground)

    ON_COMMAND(ID_OPTIONS_BESTCONTRAST, OnOptionsBestContrast)
    ON_COMMAND(ID_OPTIONS_DISABLECACHE, OnOptionsDisableCache)
    ON_UPDATE_COMMAND_UI(ID_CACHEDRESOLUTIONS_16384, OnUpdateCachedResolutions16384)
    ON_UPDATE_COMMAND_UI(ID_CACHEDRESOLUTIONS_8192, OnUpdateCachedResolutions8192)
    ON_UPDATE_COMMAND_UI(ID_CACHEDRESOLUTIONS_4096, OnUpdateCachedResolutions4096)
    ON_UPDATE_COMMAND_UI(ID_CACHEDRESOLUTIONS_1024, OnUpdateCachedResolutions2048)
    ON_UPDATE_COMMAND_UI(ID_CACHEDRESOLUTIONS_512, OnUpdateCachedResolutions1024)
    ON_UPDATE_COMMAND_UI(ID_CACHEDRESOLUTIONS_256, OnUpdateCachedResolutions256)
    ON_UPDATE_COMMAND_UI(ID_CACHEDRESOLUTIONS_128, OnUpdateCachedResolutions128)
    ON_UPDATE_COMMAND_UI(ID_CACHEDRESOLUTIONS_NORMALCACHE, OnUpdateCachedResolutionsNormalCache)

    ON_COMMAND_RANGE(ID_OPTIONS_LOADINMEMORY_DISABLED, ID_OPTIONS_LOADINMEMORY_STRIP, OnOptionsLoadInMemory)
    ON_UPDATE_COMMAND_UI_RANGE(ID_OPTIONS_LOADINMEMORY_DISABLED, ID_OPTIONS_LOADINMEMORY_STRIP, OnUpdateOptionsLoadInMemory)

    ON_UPDATE_COMMAND_UI(ID_OPTIONS_BESTCONTRAST, OnUpdateOptionsBestContrast)
    ON_UPDATE_COMMAND_UI(ID_OPTIONS_DISABLECACHE, OnUpdateOptionsDisableCache)
    ON_COMMAND(ID_CACHEDRESOLUTIONS_16384, OnCachedResolutions16384)
    ON_COMMAND(ID_CACHEDRESOLUTIONS_8192, OnCachedResolutions8192)
    ON_COMMAND(ID_CACHEDRESOLUTIONS_4096, OnCachedResolutions4096)
    ON_COMMAND(ID_CACHEDRESOLUTIONS_2048, OnCachedResolutions2048)
    ON_COMMAND(ID_CACHEDRESOLUTIONS_1024, OnCachedResolutions1024)
    ON_COMMAND(ID_CACHEDRESOLUTIONS_512, OnCachedResolutions512)
    ON_COMMAND(ID_CACHEDRESOLUTIONS_256, OnCachedResolutions256)
    ON_COMMAND(ID_CACHEDRESOLUTIONS_128, OnCachedResolutions128)
    ON_COMMAND(ID_CACHEDRESOLUTIONS_NORMALCACHE, OnCachedResolutionsNormalCache)
    ON_COMMAND(ID_INFO_TEXTLOG, &CActiveImageDoc::OnInfoTextlog)
    ON_COMMAND(ID_EDIT_REPROJECT, &CActiveImageDoc::OnReproject)
    ON_COMMAND(ID_INFO_IMAGEINFO, &CActiveImageDoc::OnInfoImageinfo)
    ON_COMMAND(ID_INFO_REPROJECTIONINFORMATION, &CActiveImageDoc::OnInfoReprojection)

    ON_COMMAND_RANGE(ID_3BAND16BITTORGB_NATIVESCAN, ID_3BAND16BITTORGB_NONE, On3band16bittorgb)
    ON_UPDATE_COMMAND_UI_RANGE(ID_3BAND16BITTORGB_NATIVESCAN, ID_3BAND16BITTORGB_NONE, OnUpdate3band16bittorgb)

    ON_COMMAND(ID_3BAND16BITTORGB_USEDSAMEVALUE, &CActiveImageDoc::On3band16bittorgbUsedsamevalue)
    ON_UPDATE_COMMAND_UI(ID_3BAND16BITTORGB_USEDSAMEVALUE, &CActiveImageDoc::OnUpdate3band16bittorgbUsedsamevalue)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
TreeListOrder::TreeListOrder(CActiveImageDoc* pi_pDoc,
                             CTreeCtrl*       pi_pTree,
                             HTREEITEM        pi_ParentItem)
    {
    m_pDoc = pi_pDoc;
    m_pTree = pi_pTree;
    m_ParentItem = pi_ParentItem;
    }


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
bool TreeListOrder::operator()(const HFCPtr<HRARaster>& pi_rRaster1,
                               const HFCPtr<HRARaster>& pi_rRaster2)
    {
    int32_t    Pos1 = -1;
    int32_t    Pos2 = -1;
    HTREEITEM Item1;
    HTREEITEM Item2;
    HTREEITEM CurrentChild;

    // Obtain the tree item of the rasters
    Item1 = m_pDoc->GetTreeItem(m_ParentItem, pi_rRaster1);
    Item2 = m_pDoc->GetTreeItem(m_ParentItem, pi_rRaster2);

    HASSERT(Item2 != NULL);

    // parse each child of the parent item
    int32_t i = 0;
    for (CurrentChild = m_pTree->GetChildItem(m_ParentItem);
    CurrentChild != NULL;
        CurrentChild = m_pTree->GetNextSiblingItem(CurrentChild))

        {
        // if the current child is the raster1, then set the position
        if ((CurrentChild == Item1) && (Pos1 == -1))
            Pos1 = i;

        // if the current child is the raster2, then set the position
        if ((CurrentChild == Item2) && (Pos2 == -1))
            Pos2 = i;
        i++;
        }

    return (Pos1 < Pos2);
    }

//-----------------------------------------------------------------------------
// protected
// Default constructor.
//-----------------------------------------------------------------------------
CActiveImageDoc::CActiveImageDoc()
    {
#ifdef IPP_USING_STATIC_LIBRARIES
#error TODO resource support.
    //HFCResourceLoader::GetInstance()->SetModuleInstance(AfxGetInstanceHandle());
#endif    
    // Create the reference CoordSys
    m_pWorldCluster = new HGFHMRStdWorldCluster();
    g_pAIRefCoordSys = GetWorldCluster()->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD);
    HASSERT(g_pAIRefCoordSys != 0);


    m_pLog = new HPMPool(theApp.GetObjectStore_Mem(), HPMPool::KeepLastBlock);
    HASSERT(m_pLog.get() != 0);

    // Give the log to the HRURasterFile for caching
    //HRUHPSFileCreator::GetInstance()->SetObjectLog(m_pLog);

    // set up the document
    m_NumMosaic = 0;
    m_NumImages = 0;
    m_DocItem = NULL;


    m_RemoveTransparency = false;
    m_ReadOnlyMode = true;
    m_FileAccessMode = HFC_NO_ACCESS;
    m_PageFileOverwrite = false;
    m_1BitForegroundMode = ID_VIEW_1BIT_EQUALWEIGHT;
    m_FitImageOnView = false;


    m_DisableCache = false;
    m_LoadInMemoryID = ID_OPTIONS_LOADINMEMORY_DISABLED;
    m_BestContrast = false;
    m_3band16bitSameValue = false;
    m_3band16bittorgbID = ID_3BAND16BITTORGB_NONE;

    // Cached resolutions
    m_NormalCache = true;
    m_Cache16384 = false;
    m_Cache8192 = false;
    m_Cache4096 = false;
    m_Cache2048 = false;
    m_Cache1024 = false;
    m_Cache512 = false;
    m_Cache256 = false;
    m_Cache128 = false;

    // Text Log Dialog
    m_pTextLogDlg = nullptr;
    m_pImageInfoDlg = nullptr;

    //Reprojection parameters
    m_reprojectionType = ID_REPROJECTIONMODEL_EXACTREPROJECTION;
    m_pReprojectDlg = nullptr;
    m_pInfoReprojectionDlg = nullptr;
    m_stringInfoReprojection = "No reprojection.";
    m_GcsPtrFromWellKnownText = nullptr;

    // Default cache file.
    HRFCacheFileCreator* pCacheFileCreator = HRFiTiffCacheFileCreator::GetInstance();

    // Reload the Profile ...
    CWinApp* pApp = AfxGetApp();

    for (uint32_t Index = 0; Index < pCacheFileCreator->CountPixelType(); Index++)
        {
        WChar VarName[1024];

        _stprintf(VarName, _TEXT("PixelType(%ld)"), pCacheFileCreator->GetPixelType(Index));

        uint32_t CodecID = pApp->GetProfileInt(_TEXT("CacheSetting"),
                                               VarName,
                                               pCacheFileCreator->GetSelectedCodecFor(pCacheFileCreator->GetPixelType(Index)));

        pCacheFileCreator->SelectCodecFor(pCacheFileCreator->GetPixelType(Index),
                                          CodecID);

        pApp->WriteProfileInt(_TEXT("CacheSetting"),
                              VarName,
                              pCacheFileCreator->GetSelectedCodecFor(pCacheFileCreator->GetPixelType(Index)));
        }


    // Testing
    if (0)
        {
        HFCPtr<HGFHMRStdWorldCluster>  pWorldCluster = new HGFHMRStdWorldCluster();
        HFCPtr<HGF2DCoordSys>  pCoordSys = pWorldCluster->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD);

        size_t      bufferLength = 24;
        HArrayAutoPtr<double>  pBuffer(new double[24]);

        pBuffer[0] = 47785.000000;
        pBuffer[1] = 6605.000000;
        pBuffer[2] = 47775.000000;
        pBuffer[3] = 6765.000000;
        pBuffer[4] = 48035.000000;
        pBuffer[5] = 6855.000000;
        pBuffer[6] = 48045.000000;
        pBuffer[7] = 6755.000000;
        pBuffer[8] = 48055.000000;
        pBuffer[9] = 6625.000000;
        pBuffer[10] = 47975.000000;
        pBuffer[11] = 6515.000000;
        pBuffer[12] = 47895.000000;
        pBuffer[13] = 6505.000000;
        pBuffer[14] = 47845.000000;
        pBuffer[15] = 6515.000000;
        pBuffer[16] = 47785.000000;
        pBuffer[17] = 6535.000000;
        pBuffer[18] = 47785.000000;
        pBuffer[19] = 6565.000000;
        pBuffer[20] = 47785.000000;
        pBuffer[21] = 6605.000000;
        pBuffer[22] = 47785.000000;
        pBuffer[23] = 6605.000000;
        HVEShape* pShape = new HVEShape(&bufferLength, pBuffer, pCoordSys);

        size_t      bufferLength2 = 10;
        HArrayAutoPtr<double>  pBuffer2(new double[bufferLength2]);

        pBuffer2[0] = 47895.000000;
        pBuffer2[1] = 6505.000000;
        pBuffer2[2] = 48055.000000;
        pBuffer2[3] = 6575.000000;
        pBuffer2[4] = 47965.000000;
        pBuffer2[5] = 6655.000000;
        pBuffer2[6] = 47845.000000;
        pBuffer2[7] = 6515.000000;
        pBuffer2[8] = 47895.000000;
        pBuffer2[9] = 6505.000000;

        HVEShape* pShape2 = new HVEShape(&bufferLength2, pBuffer2, pCoordSys);

        pShape->Unify(*pShape2);

        delete pShape2;
        delete pShape;
        }



#if 0   // Tiff file for Undo/Redo

    HAutoPtr<Byte> pData(new Byte[5000]);
    HFCPtr<HCDPacket> pPacket = new HCDPacket();
    pPacket->SetBufferOwnership(true);

    //--------------------------------------------
    // Creation
    HAutoPtr<HTIFFFile> pFile(HTIFFFile_UndoRedoFile("k:\\tmp\\Undo.file", HFC_READ_WRITE_CREATE));

    memset(pData, 1, 100);
    pFile->StripWriteCompress(pData, 100, 0/*index*/);
    memset(pData, 2, 100);
    pFile->StripWriteCompress(pData, 100, 1/*index*/);
    memset(pData, 3, 100);
    pFile->StripWriteCompress(pData, 100, 2/*index*/);
    memset(pData, 4, 100);
    pFile->StripWriteCompress(pData, 100, 3/*index*/);

    pFile->StripRead(pPacket, 2/*index*/);
    pFile->StripRead(pPacket, 3/*index*/);

    pFile = 0;

    HAutoPtr<HTIFFFile> pFile2(HTIFFFile_UndoRedoFile("k:\\tmp\\Undo.file", HFC_READ_WRITE));

    pFile2->StripRead(pPacket, 1/*index*/);

    memset(pData, 40, 100);
    pFile2->StripWriteCompress(pData, 100, 3/*index*/);
    memset(pData, 50, 100);
    pFile2->StripWriteCompress(pData, 100, 1/*index*/);
    memset(pData, 60, 100);
    pFile2->StripWriteCompress(pData, 100, 4/*index*/);

    pFile2 = 0;

#endif

    // set callback
    HFCCallbackRegistry::GetInstance()->AddCallback((const HFCCallback*) &g_AuthenticationCallback);

    // Initially no reprojection
    m_ReprojectionModel = new HGF2DIdentity();
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
CActiveImageDoc::~CActiveImageDoc()
    {
    if (m_pTextLogDlg != nullptr)
        {
        m_pTextLogDlg->DestroyWindow();
        delete m_pTextLogDlg;
        }

    if (m_pImageInfoDlg != nullptr)
        {
        m_pImageInfoDlg->DestroyWindow();
        delete m_pImageInfoDlg;
        }

    if (m_pInfoReprojectionDlg != nullptr)
        {
        m_pInfoReprojectionDlg->DestroyWindow();
        delete m_pInfoReprojectionDlg;
        }

    if (m_pReprojectDlg != nullptr)
        {
        m_pReprojectDlg->DestroyWindow();
        delete m_pReprojectDlg;
        }

#ifdef __SUPPORT_GEORASTER
    HINSTANCE OracleWrapperDll;
    if ((OracleWrapperDll = LoadLibrary(_TEXT("ippOracleWrapperd.dll"))) != 0)
        {
        if (SDOGeoRasterWrapper::IsConnected())
            SDOGeoRasterWrapper::Disconnect();
        FreeLibrary(OracleWrapperDll);
        }
#endif    
    }


//-----------------------------------------------------------------------------
// public
// 
//-----------------------------------------------------------------------------
BOOL CActiveImageDoc::OnNewDocument()
    {
    // call the base-class function
    CDocument::OnNewDocument();

    return (true);
    }


BOOL CActiveImageDoc::OnOpenDocument(LPCTSTR lpszPathName)
    {
    BOOL Result = false;

    // verify if the file exists
    if (_taccess(lpszPathName, 00) != 0)
        {
        AfxMessageBox(_TEXT("File does not exists."));
        }
    else
        {
        // Delete the content of the document
        DeleteContents();
        try
            {
            // Transform the file into a raster (HRA)
            list<HFCPtr<HRARaster> > RasterLst;
            OpenRasterFile(Utf8String(lpszPathName).c_str(), &RasterLst);

            // Insert the raster in the document
            if (!RasterLst.empty())
                {
                if (RasterLst.size() == 1)
                    {
                    // Insert the raster in the document structure
                    InsertRaster(*RasterLst.begin(), true, m_DocItem, lpszPathName);
                    }
                else
                    {
                    // insert all pages in the document structure
                    InsertMultiPageRaster(RasterLst, true, m_DocItem, lpszPathName);
                    }
                Result = true;
                }
            }
        catch (HFCException& pi_rObj)
            {
            ExceptionHandler(pi_rObj);
            }
        catch (logic_error&)
            {
            AfxMessageBox(_TEXT("Logic Error."));
            }
        catch (runtime_error&)
            {
            AfxMessageBox(_TEXT("Runtime Error."));
            }
        catch (exception&)
            {
            AfxMessageBox(_TEXT("Standard lib. error"));
            }
        catch (...)
            {
            AfxMessageBox(_TEXT("Unknown error"));
            }

        // add to MRU
        if (Result)
            {
            m_FileName = lpszPathName;
            AfxGetApp()->AddToRecentFileList(lpszPathName);
            SetModifiedFlag(false);
            }
        }
    return (Result);
    }


//-----------------------------------------------------------------------------
// public
// 
//-----------------------------------------------------------------------------
BOOL CActiveImageDoc::OnSaveDocument(LPCTSTR lpszPathName)
    {
    BOOL Result = true;

    Result = SaveRasterFile(Utf8String(lpszPathName).c_str());

    // Mark as clean is the save was successful
    if (Result)
        {
        m_FileName = lpszPathName;
        AfxGetApp()->AddToRecentFileList(lpszPathName);
        SetModifiedFlag(false);
        }

    return (Result);
    }



void CActiveImageDoc::OnUpdateFileExport(CCmdUI* pCmdUI)
    {
    pCmdUI->Enable(m_RasterMap.find(GetSelectedObject()) != m_RasterMap.end());
    }

void CActiveImageDoc::OnUpdateFileExportRegion(CCmdUI* pCmdUI)
    {
    pCmdUI->Enable(m_RasterMap.find(GetSelectedObject()) != m_RasterMap.end());
    }

void CActiveImageDoc::OnUpdateFileSave(CCmdUI* pCmdUI)
    {
    pCmdUI->Enable(m_RasterMap.find(GetSelectedObject()) != m_RasterMap.end());
    }


void CActiveImageDoc::OnFileExportHmr()
    {
    try
        {
        // Get the dialog
        HAutoPtr<CActiveImageFileDialog> OpenDialog;
        OpenDialog = ((CActiveImageApp*) AfxGetApp())->GetFileDialog(true, false);

        if (OpenDialog->DoModal() == IDOK)
            {
            HAutoPtr<HUTImportFromFileExportToFile> pImportExport;
            pImportExport = new HUTImportFromFileExportToFile(GetWorldCluster());

            HFCPtr<HFCURLFile> SrcFilename = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + Utf8String(OpenDialog->GetPathName()));
            pImportExport->SelectImportFilename((HFCPtr<HFCURL>)SrcFilename);

            POSITION pos = GetFirstViewPosition();
            CActiveImageView* pView = (CActiveImageView*) GetNextView(pos);
            ASSERT_VALID(pView);

            HUTExportDialog SaveAsDialog(pImportExport, pView);

            if (SaveAsDialog.DoModal() == IDOK)
                {
                HFCPtr<HFCURLFile> DstFilename = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + Utf8String(SaveAsDialog.GetExportFileName()));

                bool ListenerAdded = false;
                HUTProgressDialog ProgressDialog(pView);

                try
                    {
                    HUTExportProgressIndicator::GetInstance()->AddListener(&ProgressDialog);
                    HRADrawProgressIndicator::GetInstance()->AddListener(&ProgressDialog);
                    ListenerAdded = true;

                    pImportExport->SelectExportFilename((HFCPtr<HFCURL>)DstFilename);

                    ExportBlockAccessListener ExportBlockAccess; //Auto register listener


                    clock_t StartTime = clock();
                    pImportExport->StartExport();
                    clock_t StopTime = clock();
#if (0)                    
                    FILE* fp = fopen("k:\\ActiveImageExportTimes.txt", "a");
                    if (fp != 0)
                        {
                        char aDate[12];
                        _ftprintf(fp, _T("%s ConvertFile: IN:%s OUT:%s Time(sec):%.3f\n"), _strdate(aDate),
                                  SrcFilename->GetFilename().c_str(), DstFilename->GetFilename().c_str(), (double) (StopTime - StartTime) / CLOCKS_PER_SEC);
                        fclose(fp);
                        }
#endif
                    WString srcfilenameW(SrcFilename->GetFilename().c_str(), BentleyCharEncoding::Utf8);
                    WString destfilenameW(DstFilename->GetFilename().c_str(), BentleyCharEncoding::Utf8);
                    WChar Msg[2048];
                    _stprintf(Msg, _TEXT("ConvertFile: IN:%s OUT:%s\n   Time(sec):%.3f\n"), srcfilenameW.c_str(), destfilenameW.c_str(), (double) (StopTime - StartTime) / CLOCKS_PER_SEC);
                    AfxMessageBox(Msg);


                    HUTExportProgressIndicator::GetInstance()->RemoveListener(&ProgressDialog);
                    HRADrawProgressIndicator::GetInstance()->RemoveListener(&ProgressDialog);

                    //Destroy the progress bar, if any
                    if (pView->GetProgressBar()->GetSafeHwnd() != 0)
                        {
                        pView->GetProgressBar()->DestroyWindow();
                        }

                    CString  PaneText;
                    PaneText += "                         ";
                    pView->GetFrame()->m_wndStatusBar.SetPaneText(INDEX_INDICATOR_PROGRESSION, PaneText, true);
                    pView->GetFrame()->m_wndStatusBar.SetPaneText(INDEX_INDICATOR_EXPORTEDFILESIZE, PaneText, true);
                    }
                catch (...)
                    {
                    if (ListenerAdded)
                        {
                        HUTExportProgressIndicator::GetInstance()->RemoveListener(&ProgressDialog);
                        HRADrawProgressIndicator::GetInstance()->RemoveListener(&ProgressDialog);

                        //Destroy the progress bar, if any
                        if (pView->GetProgressBar()->GetSafeHwnd() != 0)
                            {
                            pView->GetProgressBar()->DestroyWindow();
                            }

                        CString  PaneText;
                        PaneText += "                         ";
                        pView->GetFrame()->m_wndStatusBar.SetPaneText(INDEX_INDICATOR_PROGRESSION, PaneText, true);
                        pView->GetFrame()->m_wndStatusBar.SetPaneText(INDEX_INDICATOR_EXPORTEDFILESIZE, PaneText, true);
                        }
                    throw;
                    }
                }
            }
        }
    catch (HFCException& pi_rObj)
        {
        ExceptionHandler(pi_rObj);
        }
    catch (logic_error&)
        {
        AfxMessageBox(_TEXT("Logic Error."));
        }
    catch (runtime_error&)
        {
        AfxMessageBox(_TEXT("Runtime Error."));
        }
    catch (exception&)
        {
        AfxMessageBox(_TEXT("Standard lib. error"));
        }
    catch (...)
        {
        AfxMessageBox(_TEXT("Unknown error"));
        }
    }

void CActiveImageDoc::OnFileExportChannels()
    {
    try
        {
        // Get the dialog
        HAutoPtr<CActiveImageFileDialog> OpenDialog;
        OpenDialog = ((CActiveImageApp*) AfxGetApp())->GetFileDialog(true, false);

        uint32_t ChannelCount = 3; // Init

        WString RedFileName;
        WString GreenFileName;
        WString BlueFileName;
        WString AlphaFileName;

        // Get all input files
        OpenDialog->m_ofn.lpstrTitle = _TEXT("Open Red Channel File");
        if (OpenDialog->DoModal() == IDOK)
            RedFileName = WString(OpenDialog->GetPathName());
        else
            return;

        OpenDialog->m_ofn.lpstrTitle = _TEXT("Open Green Channel File");
        if (OpenDialog->DoModal() == IDOK)
            GreenFileName = WString(OpenDialog->GetPathName());
        else
            return;

        OpenDialog->m_ofn.lpstrTitle = _TEXT("Open Blue Channel File");
        if (OpenDialog->DoModal() == IDOK)
            BlueFileName = WString(OpenDialog->GetPathName());
        else
            return;

        if (AfxMessageBox(_TEXT("Do you want to add an alpha channel ?"), MB_YESNO) == IDYES)
            {
            ChannelCount = 4;

            OpenDialog->m_ofn.lpstrTitle = _TEXT("Open Alpha Channel File");
            if (OpenDialog->DoModal() == IDOK)
                AlphaFileName = WString(OpenDialog->GetPathName());
            else
                return;
            }

        // Create URLs
        const HFCPtr<HFCURL> pRedFileURL = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + Utf8String(RedFileName));
        const HFCPtr<HFCURL> pGreenFileURL = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + Utf8String(GreenFileName));
        const HFCPtr<HFCURL> pBlueFileURL = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + Utf8String(BlueFileName));

        // Export object creation
        HAutoPtr<HUTImportFromFileExportToFile> pImportExport;
        pImportExport = new HUTImportFromFileExportToFile(GetWorldCluster());

        // Create HRFxChFile
        if (ChannelCount == 4)
            {
            const HFCPtr<HFCURL> pAlphaFileURL = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + Utf8String(AlphaFileName));
            HFCPtr<HRFRasterFile> pxChFile = new HRFxChFile(pRedFileURL, pGreenFileURL, pBlueFileURL, pAlphaFileURL);

            pImportExport->SetImportRasterFile(pxChFile);
            }
        else
            {
            HFCPtr<HRFRasterFile> pxChFile = new HRFxChFile(pRedFileURL, pGreenFileURL, pBlueFileURL, 0);

            pImportExport->SetImportRasterFile(pxChFile);
            }


        POSITION pos = GetFirstViewPosition();
        CActiveImageView* pView = (CActiveImageView*) GetNextView(pos);
        ASSERT_VALID(pView);

        HUTExportDialog SaveAsDialog(pImportExport, pView);

        if (SaveAsDialog.DoModal() == IDOK)
            {
            HFCPtr<HFCURL> DstFilename = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + Utf8String(SaveAsDialog.GetExportFileName()));

            bool ListenerAdded = false;
            HUTProgressDialog ProgressDialog(pView);

            try
                {
                HUTExportProgressIndicator::GetInstance()->AddListener(&ProgressDialog);
                HRADrawProgressIndicator::GetInstance()->AddListener(&ProgressDialog);
                ListenerAdded = true;



                pImportExport->SelectExportFilename(DstFilename);
                pImportExport->StartExport();

                HUTExportProgressIndicator::GetInstance()->RemoveListener(&ProgressDialog);
                HRADrawProgressIndicator::GetInstance()->RemoveListener(&ProgressDialog);
                }

            catch (...)
                {
                if (ListenerAdded)
                    {
                    HUTExportProgressIndicator::GetInstance()->RemoveListener(&ProgressDialog);
                    HRADrawProgressIndicator::GetInstance()->RemoveListener(&ProgressDialog);
                    }
                throw;
                }
            }
        }
    catch (HFCException& pi_rObj)
        {
        ExceptionHandler(pi_rObj);
        }
    catch (logic_error&)
        {
        AfxMessageBox(_TEXT("Logic Error."));
        }
    catch (runtime_error&)
        {
        AfxMessageBox(_TEXT("Runtime Error."));
        }
    catch (exception&)
        {
        AfxMessageBox(_TEXT("Standard lib. error"));
        }
    catch (...)
        {
        AfxMessageBox(_TEXT("Unknown error"));
        }
    }


#ifdef _DEBUG
//-----------------------------------------------------------------------------
// public
// 
//-----------------------------------------------------------------------------
void CActiveImageDoc::AssertValid() const
    {
    CDocument::AssertValid();
    }

//-----------------------------------------------------------------------------
// public
// 
//-----------------------------------------------------------------------------
void CActiveImageDoc::Dump(CDumpContext& dc) const
    {
    CDocument::Dump(dc);
    }
#endif //_DEBUG


void CActiveImageDoc::OnUpdateFileReadonly(CCmdUI* pCmdUI)
    {
    pCmdUI->SetCheck(m_ReadOnlyMode);
    }
void CActiveImageDoc::OnFileReadonly()
    {
    m_ReadOnlyMode = !m_ReadOnlyMode;
    }


void CActiveImageDoc::OnUpdateFileOverwritehgrtwf(CCmdUI* pCmdUI)
    {
    pCmdUI->SetCheck(m_PageFileOverwrite);
    }
void CActiveImageDoc::OnFileOverwritehgrtwf()
    {
    m_PageFileOverwrite = !m_PageFileOverwrite;
    }






//-----------------------------------------------------------------------------
// public
// 
//-----------------------------------------------------------------------------
void CActiveImageDoc::DeleteContents()
    {
    CloseAllFiles();

    // Delete the Raster in the BufferImage
    UpdateAllViews(0, UPDATE_FIRST, 0);


    m_pWorldCluster = new HGFHMRStdWorldCluster();
    g_pAIRefCoordSys = GetWorldCluster()->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD);
    HASSERT(g_pAIRefCoordSys != 0);

    // Clear the file name
    m_FileName = "";

    // set up the document
    m_NumMosaic = 0;
    m_NumImages = 0;

    // call the parent method
    CDocument::DeleteContents();
    }


//-----------------------------------------------------------------------------
// Protected
// 
//-----------------------------------------------------------------------------
void CActiveImageDoc::OnInsertFromExternalDocument()
    {
    CActiveImageFileDialog*     pDialog;
    CString                     Filters;
    CString                     ImageExt;
    list<HFCPtr<HRARaster> >    PagesList;
    bool                       InsertAsLink;
    WChar                      pFileName[_MAX_PATH];

    // get the dialog
    pDialog = ((CActiveImageApp*) AfxGetApp())->GetFileDialog(true, true);

    // Obtain the currently selected Object
    HTREEITEM  SelObject = GetSelectedObject();
    HASSERT(SelObject != NULL);

    try
        {
        // get the file name from the file dialog
        if (pDialog->DoModal() == IDOK)
            {
            POSITION FilePos = pDialog->GetStartPosition();

            list<HFCPtr<HRARaster> > RasterList;

            // Fill the list with the files infos
            while (FilePos)
                {
                // get the file name
                _tcscpy(pFileName, (LPCTSTR) pDialog->GetNextPathName(FilePos));

                // get the raster from a raster file
                try
                    {
                    RasterList.clear();
                    PagesList.clear();
                    OpenRasterFile(Utf8String(pFileName).c_str(), &PagesList);
                    }
                catch (HFCException& pi_rObj)
                    {
                    ExceptionHandler(pi_rObj);
                    }
                catch (logic_error&)
                    {
                    AfxMessageBox(_TEXT("Logic Error."));
                    }
                catch (runtime_error&)
                    {
                    AfxMessageBox(_TEXT("Runtime Error."));
                    }
                catch (exception&)
                    {
                    AfxMessageBox(_TEXT("Standard lib. error"));
                    }
                catch (...)
                    {
                    AfxMessageBox(_TEXT("Unknown error"));
                    }

                if (!RasterList.empty())
                    InsertAsLink = pDialog->IsLinkBoxChecked() != 0;

                // Insert the raster if it was found
                if (PagesList.size() == 1)
                    {
                    InsertRaster(*PagesList.begin(), InsertAsLink, SelObject, pFileName);
                    //                    RedrawView(pRasterToInsert);
                    }
                else if (PagesList.size() > 1)
                    {
                    InsertMultiPageRaster(PagesList, InsertAsLink, SelObject, pFileName);
                    }
                else
                    {
                    AfxMessageBox(_TEXT("Could not obtain the raster"));
                    }
                }
            } // Dialog execution returned IDOK
        }
    catch (HFCException& pi_rObj)
        {
        ExceptionHandler(pi_rObj);
        }
    catch (logic_error&)
        {
        AfxMessageBox(_TEXT("Logic Error."));
        }
    catch (runtime_error&)
        {
        AfxMessageBox(_TEXT("Runtime Error."));
        }
    catch (exception&)
        {
        AfxMessageBox(_TEXT("Standard lib. error"));
        }
    catch (...)
        {
        AfxMessageBox(_TEXT("Unknown error"));
        }

    // destroy the dialog
    delete pDialog;
    }




//-----------------------------------------------------------------------------
// Protected
// 
//-----------------------------------------------------------------------------
void CActiveImageDoc::OnInsertNewObject()
    {
    HFCPtr<HIMMosaic> pMosaic;
    HTREEITEM         SelObject = GetSelectedObject();
    HASSERT(SelObject != NULL);

    //
    // create the mosaic
    //
    pMosaic = new HIMMosaic(GetWorldCluster()->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD));
    HASSERT(pMosaic != 0);

    //
    // Insert the new mosaic in the tree structure
    //
    InsertRasterInTree((HFCPtr<HRARaster>&)pMosaic, SelObject);


    //
    // insert the mosaoc in its parent (if its not the document)
    //
    if (SelObject != m_DocItem)
        {
        HFCPtr<HIMMosaic> pParent = static_cast<HIMMosaic*>(GetRaster(SelObject).GetPtr());
        HASSERT(pParent != 0);
        HASSERT(!pParent->Contains((HFCPtr<HRARaster>&)pMosaic, true));
        pParent->Add((HFCPtr<HRARaster>&)pMosaic);
        }

    SetModifiedFlag(true);
    }


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageDoc::MoveTreeItem(HTREEITEM  pi_Destination,
                                   HTREEITEM  pi_Source,
                                   HTREEITEM  pi_InsertAfterInDestination)
    {
    CTreeCtrl*        pTree = GetTreeCtrl();
    CString           ItemLabel;
    HFCPtr<HRARaster> pRaster;
    HTREEITEM         NewItem;
    int32_t            nImage;
    int32_t            nSelectedImage;

    // Get the source item's information
    //
    //  1. The Label
    //  2. The associated raster
    //  3. Tree list icons
    ItemLabel = pTree->GetItemText(pi_Source);
    pRaster = GetRaster(pi_Source);
    pTree->GetItemImage(pi_Source, nImage, nSelectedImage);
    HASSERT(pRaster != 0);

    // Create the destination
    //
    NewItem = pTree->InsertItem(ItemLabel, nImage, nSelectedImage,
                                pi_Destination, pi_InsertAfterInDestination);
    HASSERT(NewItem != NULL);

    // Move each of the source item's children to the destination
    //
    if (pTree->ItemHasChildren(pi_Source))
        {
        HTREEITEM  CurrentChild = pTree->GetChildItem(pi_Source);
        while (CurrentChild != NULL)
            {
            // Get the current child's next sibling, because after the move
            // we won't be able to get it.
            HTREEITEM NextItem = pTree->GetNextSiblingItem(CurrentChild);

            // move it and proceed to the next sibling
            MoveTreeItem(NewItem, CurrentChild, TVI_LAST);
            CurrentChild = NextItem;
            }
        }

    // Remove the current entry from the map
    RasterMap::iterator Itr = m_RasterMap.find(pi_Source);
    if (Itr != m_RasterMap.end())
        m_RasterMap.erase(Itr);

    // Keep the moved item in the same expand state
    if (pTree->GetItemState(pi_Source, TVIS_EXPANDED) & TVIS_EXPANDED)
        pTree->Expand(NewItem, TVE_EXPAND);

    // Remove from the tree
    pTree->DeleteItem(pi_Source);

    // Re-insert the item in the map
    m_RasterMap.insert(RasterMap::value_type(NewItem, pRaster));
    }


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
HTREEITEM CActiveImageDoc::GetTreeItem(HTREEITEM&        pi_Parent,  // see first comment below
                                       HFCPtr<HRARaster> pi_pRaster)
    {
    CTreeCtrl* pTree = GetTreeCtrl();
    HTREEITEM  Result = NULL;
    HPRECONDITION(pi_Parent != NULL);

    // if the parent points to the given raster, this means
    // that the raster is not a mosaic.  Set the parent to
    // its true parent.  This is why the pi_Parent parameter
    // is a reference.
    if (GetRaster(pi_Parent) == pi_pRaster)
        pi_Parent = pTree->GetParentItem(pi_Parent);

    // Parse each children of the given tree item
    if (pTree->ItemHasChildren(pi_Parent))
        {
        HTREEITEM CurrentChild = pTree->GetChildItem(pi_Parent);
        while (CurrentChild != NULL)
            {
            // Get the raster associated with the item
            HFCPtr<HRARaster> pRaster = GetRaster(CurrentChild);
            HASSERT(pRaster != 0);

            // verify if the current raster is the wanted raster
            // if so set the Result to this one and stop searching
            if (pRaster == pi_pRaster)
                {
                Result = CurrentChild;
                break;
                }

            // go to next child
            CurrentChild = pTree->GetNextSiblingItem(CurrentChild);
            }
        }

    return (Result);
    }


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageDoc::RemoveTreeItem(HTREEITEM pi_ItemToDelete)
    {
    CTreeCtrl* pTree = GetTreeCtrl();
    HPRECONDITION(pi_ItemToDelete != NULL);

    // if the item has children, remove them
    if (pTree->ItemHasChildren(pi_ItemToDelete))
        {
        HTREEITEM CurrentItem = pTree->GetChildItem(pi_ItemToDelete);
        while (CurrentItem != NULL)
            {
            // go to the next child item
            HTREEITEM NextItem = pTree->GetNextSiblingItem(CurrentItem);

            // remove the current child item
            RemoveTreeItem(CurrentItem);

            // go to the next child item
            CurrentItem = NextItem;
            }
        }

    // delete the entry from the map
    m_RasterMap.erase(pi_ItemToDelete);

    // delete the item from the tree
    pTree->DeleteItem(pi_ItemToDelete);
    }

//-----------------------------------------------------------------------------
// Public
// PlaceInFilteredImage
//-----------------------------------------------------------------------------
HFCPtr<HIMFilteredImage>
CActiveImageDoc::PlaceInFilteredImage(const HFCPtr<HRARaster>& pi_pRaster,
                                      const HFCPtr<HRPFilter>& pi_rpFilter)
    {
    HPRECONDITION(pi_pRaster != 0);

    // get the parent item
    HTREEITEM RasterItem = GetSelectedObject();
    HASSERT(RasterItem != NULL);

    // create the filtered image
    HFCPtr<HIMFilteredImage> pFilteredImage = new HIMFilteredImage(pi_pRaster, pi_rpFilter);
    HASSERT(pFilteredImage != 0);

    // remove the map entry for the current tree item and
    // associate the tree item handle with the reference
    RasterMap::iterator Itr = m_RasterMap.find(RasterItem);
    HASSERT(Itr != m_RasterMap.end());
    m_RasterMap.erase(Itr);
    m_RasterMap.insert(RasterMap::value_type(RasterItem, (HFCPtr<HRARaster>&)pFilteredImage));

    return (pFilteredImage);
    }

//-----------------------------------------------------------------------------
// Public
// PlaceInTransparentImage
//-----------------------------------------------------------------------------
HFCPtr<HRARaster>
CActiveImageDoc::PlaceInTransparentImage(const HFCPtr<HRARaster>& pi_pRaster,
                                         Byte pi_Transparence)
    {
    HPRECONDITION(pi_pRaster != 0); 

    // get the parent item
    HTREEITEM RasterItem = GetSelectedObject();
    HASSERT(RasterItem != NULL);

    // create the filtered image
    HIMTranslucentImageCreator TheCreator(pi_pRaster, pi_Transparence);
    HFCPtr<HRARaster> pTransparentImage = TheCreator.CreateTranslucentRaster();
    HASSERT(pTransparentImage != 0);

    if (pTransparentImage != 0)
        {
        // remove the map entry for the current tree item and
        // associate the tree item handle with the reference
        RasterMap::iterator Itr = m_RasterMap.find(RasterItem);
        HASSERT(Itr != m_RasterMap.end());
        m_RasterMap.erase(Itr);
        m_RasterMap.insert(RasterMap::value_type(RasterItem, pTransparentImage));
        }

    return (pTransparentImage);
    }


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
HFCPtr<HRARaster>
CActiveImageDoc::RemoveImageView(const HFCPtr<HRAImageView>& pi_pRaster)
    {
    HPRECONDITION(pi_pRaster != 0);

    // get the raster item
    HTREEITEM Raster = GetSelectedObject();
    HASSERT(Raster != NULL);

    // remove the map entry for the current tree item and
    // associate the tree item handle with the reference
    m_RasterMap.erase(Raster);
    m_RasterMap.insert(RasterMap::value_type(Raster, (HFCPtr<HRARaster>&)pi_pRaster->GetSource()));

    return ((HFCPtr<HRARaster>&)pi_pRaster->GetSource());
    }

//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
HFCPtr<HRAReferenceToRaster>
CActiveImageDoc::PlaceInReference(const HFCPtr<HRARaster>& pi_pRaster)
    {
    CTreeCtrl* pTree = GetTreeCtrl();
    HPRECONDITION(pi_pRaster != 0);

    // get the parent item
    HTREEITEM ParentItem = GetSelectedObject();
    HASSERT(ParentItem != NULL);

    // create the reference
    HFCPtr<HRAReferenceToRaster> pRefRaster(new HRAReferenceToRaster(pi_pRaster));
    HASSERT(pRefRaster != 0);

    // get the tree item handle associated with the raster in the tree list
    HTREEITEM RasterItem = GetTreeItem(ParentItem, pi_pRaster);
    HASSERT(RasterItem != NULL);

    // remove the map entry for the current tree item and
    // associate the tree item handle with the reference
    m_RasterMap.erase(RasterItem);
    m_RasterMap.insert(RasterMap::value_type(RasterItem, (HFCPtr<HRARaster>&)pRefRaster));

    // set the new tree list icons
    if (pi_pRaster->GetClassID() == HIMMosaic::CLASS_ID)
        pTree->SetItemImage(RasterItem, TLI_REF_MOSAIC, TLI_REF_MOSAIC + 1);
    else
        pTree->SetItemImage(RasterItem, TLI_REF_RASTER, TLI_REF_RASTER + 1);

    // if the parent item is a mosaic
    if (ParentItem != m_DocItem)
        {
        // get the mosaic
        HFCPtr<HIMMosaic> pParent = static_cast<HIMMosaic*>(GetRaster(ParentItem).GetPtr());
        HASSERT(pParent != 0);
        HASSERT(pParent->GetClassID() == HIMMosaic::CLASS_ID);

        // remove the raster from the mosaic
        pParent->Remove(pi_pRaster);

        // get the previous item in the mosaic (from the tree list)


        HTREEITEM PreviousItem = pTree->GetPrevSiblingItem(RasterItem);

        // if the item is NULL, it means the current item is the first
        // In that case, add on top
        if (PreviousItem == NULL)
            pParent->Add((HFCPtr<HRARaster>&)pRefRaster);
        else
            {
            // get the previous raster
            HFCPtr<HRARaster> pPrevRaster = GetRaster(PreviousItem);
            HASSERT(pPrevRaster != 0);

            // add after the previous raster in the mosaic
            pParent->AddAfter((HFCPtr<HRARaster>&)pRefRaster, pPrevRaster);
            }
        }

    return (pRefRaster);
    }


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
bool CActiveImageDoc::IsRasterLinked(HFCPtr<HRARaster>& pi_rpRaster) const
    {
    HPRECONDITION(pi_rpRaster != 0);
    bool Result = false;

    // If the raster is scheduled to be cloned in the database, it is not a linked raster.
    // If it is not in the list, further inquiries
    if (find(m_RasterToClone.begin(),
             m_RasterToClone.end(),
             pi_rpRaster) == m_RasterToClone.end())
        {
        Result = true;
        }

    return (Result);
    }

//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
BOOL CActiveImageDoc::SaveModified()
    {

    // code from CDocument::SaveModified()
    if (!IsModified())
        return true;        // ok to continue

                            // get name/title of document
    CString name;

    CString prompt;
    AfxFormatString1(prompt, AFX_IDP_ASK_TO_SAVE, GetPathName());
    switch (AfxMessageBox(prompt, MB_YESNOCANCEL, AFX_IDP_ASK_TO_SAVE))
        {
            case IDCANCEL:
                return false;       // don't continue

            case IDYES:
                // If so, either Save or Update, as appropriate
                if (!DoFileSave())
                    return false;       // don't continue
                break;

            case IDNO:
            {
            HTREEITEM  SelObject = GetSelectedObject();
            if (SelObject != NULL && SelObject != m_DocItem)
                {
                HFCPtr<HRARaster> pSelectedObject;

                pSelectedObject = GetRaster(SelObject);
                if (pSelectedObject->IsCompatibleWith(HRAStoredRaster::CLASS_ID))
                    {
                    HFCPtr<HRAStoredRaster> pStoredRaster((HFCPtr<HRAStoredRaster>&)pSelectedObject);

                    while (pStoredRaster->CanUndo())
                        pStoredRaster->Undo(false);
                    }
                }
            }
            // If not saving changes, revert the document
            break;

            default:
                ASSERT(false);
                break;
        }
    return true;    // keep going


    if (!CDocument::SaveModified())
        {
        HTREEITEM  SelObject = GetSelectedObject();
        if (SelObject != NULL && SelObject != m_DocItem)
            {
            HFCPtr<HRARaster> pSelectedObject;

            pSelectedObject = GetRaster(SelObject);
            if (pSelectedObject->IsCompatibleWith(HRAStoredRaster::CLASS_ID))
                {
                HFCPtr<HRAStoredRaster> pStoredRaster((HFCPtr<HRAStoredRaster>&)pSelectedObject);

                while (pStoredRaster->CanUndo())
                    pStoredRaster->Undo(false);
                }
            return false;
            }
        }

    return true;
    }

BOOL CActiveImageDoc::DoSave(LPCTSTR lpszPathName, BOOL bReplace)
// Save the document data to a file
// lpszPathName = path name where to save document file
// if lpszPathName is NULL then the user will be prompted (SaveAs)
// note: lpszPathName can be different than 'm_strPathName'
// if 'bReplace' is true will change file name if successful (SaveAs)
// if 'bReplace' is false will not change path name (SaveCopyAs)
    {
    CString newName = lpszPathName;
    if (newName.IsEmpty())
        {
        CDocTemplate* pTemplate = GetDocTemplate();
        ASSERT(pTemplate != NULL);

        newName = m_strPathName;
        if (bReplace && newName.IsEmpty())
            {
            newName = m_strTitle;
#ifndef _MAC
            // check for dubious filename
            int iBad = newName.FindOneOf(_T(" #%;/\\"));
#else
            int iBad = newName.FindOneOf(_T(":"));
#endif
            if (iBad != -1)
                newName.ReleaseBuffer(iBad);

#ifndef _MAC
            // append the default suffix if there is one
            CString strExt;
            if (pTemplate->GetDocString(strExt, CDocTemplate::filterExt) &&
                !strExt.IsEmpty())
                {
                ASSERT(strExt[0] == '.');
                newName += strExt;
                }
#endif
            }

        if (!AfxGetApp()->DoPromptFileName(newName,
                                           bReplace ? AFX_IDS_SAVEFILE : AFX_IDS_SAVEFILECOPY,
                                           OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, false, pTemplate))
            return false;       // don't even attempt to save
        }

    CWaitCursor wait;

    if (m_FileName != newName)
        {
        // Delete any current file
        if ((!::DeleteFile(newName)) &&                 // Delete unsuccessful
            (GetLastError() != ERROR_FILE_NOT_FOUND))   // not because it does already does not exists :)
            {
            AfxMessageBox(_TEXT("Could not overwrite file!"));
            return false;
            }
        }

    // Save under the old name.  A copy will be done later
    if (!OnSaveDocument(newName))
        {
        AfxMessageBox(_TEXT("Could not overwrite file!"));
        return false;
        }

    // reset the title and change the document name
    SetPathName(newName, false);
    m_FileName = newName;

    // Specify that the file has a new name
    SetModifiedFlag(false);

    return true;        // success
    }

//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageDoc::OnInsertFromthisdocument()
    {
    HPRECONDITION(GetSelectedObject() != NULL);

    CInsertObjectDialog dlg("Insert from this document", this);

    if (dlg.DoModal() == IDOK)
        {
        // Get the selected object and get its raster
        HTREEITEM SelObject = GetSelectedObject();

        // insert the raster in its parent (if its not the document)
        if (SelObject != m_DocItem)
            {
            // get the parent mosaic
            HFCPtr<HIMMosaic> pParent = static_cast<HIMMosaic*>(GetRaster(SelObject).GetPtr());
            HASSERT(pParent != 0);

            // verify for that we're not insert itself
            if (!VerifyLoopForward((HFCPtr<HRARaster>&)pParent, dlg.m_pSelectedRaster) ||
                !VerifyLoopBackward((HFCPtr<HRARaster>&)pParent, dlg.m_pSelectedRaster))
                {
                AfxMessageBox(_TEXT("Inserting this object will create a loop"));
                goto WRAPUP;
                }

            // insert in the mosaic
            pParent->Add(dlg.m_pSelectedRaster);
            }

        // insert the raster in the tree structure
        InsertRasterInTree(dlg.m_pSelectedRaster, SelObject);
        RedrawView(dlg.m_pSelectedRaster);
        }

WRAPUP:;
    }

//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageDoc::ReplaceRaster(HTREEITEM pi_hItem, HFCPtr<HRARaster>& pi_pNewRaster)
    {
    HPRECONDITION(pi_pNewRaster != 0);

    // remove the map entry for the current tree item and
    // associate the tree item handle with the reference
    RasterMap::iterator Itr = m_RasterMap.find(pi_hItem);
    HASSERT(Itr != m_RasterMap.end());
    m_RasterMap.erase(Itr);
    m_RasterMap.insert(RasterMap::value_type(pi_hItem, pi_pNewRaster));
    }

//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageDoc::ReplaceChildRaster(HTREEITEM          pi_hItem,
                                         HFCPtr<HRARaster>& pi_pRaster,
                                         HFCPtr<HRARaster>& pi_pNewRaster)
    {
    HPRECONDITION(pi_pRaster != 0);
    HPRECONDITION(pi_pNewRaster != 0);
    HPRECONDITION(pi_hItem != NULL);
    CTreeCtrl* pTree = GetTreeCtrl();

    //
    // Parse each item directly under the given tree item
    //
    HTREEITEM CurrentItem = pTree->GetChildItem(pi_hItem);
    while (CurrentItem != NULL)
        {
        // Get the raster associated with the item
        HFCPtr<HRARaster> pRaster = GetRaster(CurrentItem);

        //
        // if the current item is the raster, replace it in the mosaic
        //
        if (pRaster == pi_pRaster)
            {
            // Associated the current tree handle with the new raster
            m_RasterMap.erase(CurrentItem);
            m_RasterMap.insert(RasterMap::value_type(CurrentItem, pi_pNewRaster));

            // Change the icon in the tree list
            if (pi_pRaster->GetClassID() == HIMMosaic::CLASS_ID)
                pTree->SetItemImage(CurrentItem, TLI_REF_MOSAIC, TLI_REF_MOSAIC + 1);
            else
                pTree->SetItemImage(CurrentItem, TLI_REF_RASTER, TLI_REF_RASTER + 1);


            // if the parent is a mosaic
            if (pi_hItem != m_DocItem)
                {
                // get the mosaic
                HFCPtr<HIMMosaic> pParent = static_cast<HIMMosaic*>(GetRaster(pi_hItem).GetPtr());
                HASSERT(pParent != 0);
                HASSERT(pParent->GetClassID() == HIMMosaic::CLASS_ID);

                // remove the raster from the mosaic
                pParent->Remove(pi_pRaster);

                // get the previous item in the mosaic (from the tree list)
                HTREEITEM PreviousItem = pTree->GetPrevSiblingItem(CurrentItem);

                // if the item is NULL, it means the current item is the first
                // In that case, add on top
                if (PreviousItem == NULL)
                    pParent->Add(pi_pNewRaster);
                else
                    {
                    // get the previous raster
                    HFCPtr<HRARaster> pPrevRaster = GetRaster(PreviousItem);
                    HASSERT(pPrevRaster != 0);

                    // add after the previous raster in the mosaic
                    pParent->AddAfter(pi_pNewRaster, pPrevRaster);
                    }
                }
            }


        //
        // if the current raster is a mosaic, recurse in the mosaic
        //
        if (pRaster->GetClassID() == HIMMosaic::CLASS_ID)
            ReplaceChildRaster(CurrentItem, pi_pRaster, pi_pNewRaster);

        // get the next tree item
        CurrentItem = pTree->GetNextSiblingItem(CurrentItem);
        }
    }


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageDoc::RedrawView(HFCPtr<HRARaster>& pi_pRaster)
    {
    HFCPtr<HRARaster> pRaster;

    // Get the selected object and get its raster
    HTREEITEM SelObject = GetSelectedObject();

    // if the object is not the document item, get the raster
    if ((SelObject != NULL) && (SelObject != m_DocItem))
        pRaster = GetRaster(SelObject);

    // get the shape of the raster
    HVEShape Shape(*(pi_pRaster->GetEffectiveShape()));

    // redraw the view only if the current selected obejct is a mosaic,
    // otherwise we don't have to redraw
    if ((pRaster != 0) &&
        (pRaster->GetClassID() == HIMMosaic::CLASS_ID))
        {
        HFCPtr<HIMMosaic>& pMosaic = (HFCPtr<HIMMosaic>&)pRaster;
        HASSERT(pMosaic != 0);

        // count the number of sources in the mosaic.
        HIMMosaic::IteratorHandle Iterator = pMosaic->StartIteration();
        uint32_t NumItems = pMosaic->CountElements(Iterator);
        pMosaic->StopIteration(Iterator);

        POSITION pos = GetFirstViewPosition();
        while (pos != NULL)
            {
            CActiveImageView* pView = (CActiveImageView*) GetNextView(pos);
            ASSERT_VALID(pView);

            // If there is only 1 raster (the new one), update the whole
            // view. Otherwise, redraw only the shape of the raster
            if (NumItems == 1)
                UpdateAllViews(0, UPDATE_REDRAW, 0);
            else
                pView->RedrawImage(Shape, Shape);
            }
        }
    else
        {
        POSITION posV = GetFirstViewPosition();
        CActiveImageView* pView1 = (CActiveImageView*) GetNextView(posV);
        ASSERT_VALID(pView1);
        pView1->RedrawWindow();
        }
    }

//-----------------------------------------------------------------------------
// public
// 
//-----------------------------------------------------------------------------
BOOL CActiveImageDoc::OnOpenInternetDocument(LPCTSTR lpszPathName)
    {
    // Open the IIP File
    HFCPtr<HRFRasterFile> pFile;
    HFCPtr<HFCURL>		  pURL = HFCURL::Instanciate(Utf8String(lpszPathName));

    if (pURL == NULL)
        {
        AfxMessageBox(L"Invalid URL");
        return false;
        }

    // Open the file
    pFile = HRFRasterFileFactory::GetInstance()->OpenFile(pURL);

    HASSERT(pFile != 0);

    list<HFCPtr<HRARaster> > RasterList;
    LoadRasterFile(pFile, &RasterList);

    // Insert the raster in the document
    if (!RasterList.empty())
        {
        WString filenameW(pURL->GetURL().c_str(), BentleyCharEncoding::Utf8);

        if (RasterList.size() == 1)
            {
            // Insert the raster in the document structure
            InsertRaster(*RasterList.begin(), true, m_DocItem, filenameW.c_str());
            }
        else
            {
            // insert all pages in the document structure
            InsertMultiPageRaster(RasterList, true, m_DocItem, filenameW.c_str());
            }
        SetModifiedFlag(false);
        }

    return true;
    }

#ifdef STRESS_APPS

void DoBackground()
    {

    BOOL bIdle = true;
    LONG lIdleCount = 0;
    MSG msgCur;

#if 0
    // phase1: check to see if we can do idle work
    while (bIdle && !::PeekMessage(&msgCur, NULL, NULL, NULL, PM_NOREMOVE))
        {
        // call OnIdle while in bIdle state
        if (!AfxGetApp()->OnIdle(lIdleCount++))
            bIdle = false; // assume "no idle" state
        }
#endif
    // phase2: pump messages while available
    do
        {
        // pump message, but quit on WM_QUIT
        if (!AfxGetApp()->PumpMessage())
            PostQuitMessage(0);

        // reset "no idle" state after pumping "normal" message
        if (AfxGetApp()->IsIdleMessage(&msgCur))
            {
            bIdle = true;
            lIdleCount = 0;
            }

        } while (::PeekMessage(&msgCur, NULL, NULL, NULL, PM_NOREMOVE));
    }

void CActiveImageDoc::InternetTest()
    {
    CString FullName;
    CString name;

    CStreesDlg *dlg;

    dlg = new CStreesDlg();

    dlg->Create(CStreesDlg::IDD, AfxGetMainWnd());


#if 0
    BOOL isHmr;
    unsigned short port;
    ImageList::iterator iteratorHandle;

    while (!dlg->m_IsStop)
        {
        iteratorHandle = gImageList.begin();
        while (iteratorHandle != gImageList.end() && !dlg->m_IsStop)
            {
            name = *iteratorHandle;

            srand((unsigned) time(NULL));
            isHmr = (BOOL) (rand() % 2);

            if (isHmr)
                port = gServerPort + 1;
            else
                port = gServerPort;

            // Create the IIP file
            HFCPtr<HRFRasterFile> pFile = new HRFIIPFile(gServerIp, port, name, isHmr, HRFFileOpenOption(HFC_READ_ONLY));

            DoBackground();
            // Generate the full image name
            FullName = gServerIp;
            FullName += ":";
            CString Port;
            Port.Format("%hu", gServerPort);
            FullName += Port;
            FullName += "/";
            FullName += name;

            // Put the IIP File in a cache
            HFCPtr<HRFRasterFile> pCache = m_pCacheManager->OpenCachedFile(pFile, m_pLog);
            HASSERT(pCache != 0);

            // Put the HRF raster in a store so that we can
            // pull a HRA raster afterwards
            HFCPtr<HRSObjectStore> pStore = new HRSObjectStore(m_pLog, pCache, 0, GetWorldCluster()->GetCoordSysReference(pCache->GetWorldIdentificator()));
            HASSERT(pStore != 0);

            // Get the raster from the store
            HFCPtr<HRARaster> pRaster = pStore->LoadRaster();
            HASSERT(pRaster != NULL);

            // Insert the raster
            InsertRaster(pRaster, true, m_DocItem, FullName);

            RedrawView(pRaster);
            iteratorHandle++;
            DoBackground();
            }

        DeleteContents();
        }
#endif
    delete dlg;
    }

#endif


void CActiveImageDoc::OnIdle()
    {
    m_HMGThread.DispatchMessages();
    }

//
// Remove Transparencies in the palette
//
void CActiveImageDoc::OnUpdateViewRemovetransparency(CCmdUI* pCmdUI)
    {
    pCmdUI->SetCheck(m_RemoveTransparency);
    }
void CActiveImageDoc::OnViewRemovetransparency()
    {
    m_RemoveTransparency = !m_RemoveTransparency;
    }


HFCPtr<HRFRasterFile>  CActiveImageDoc::MyImprove(HFCPtr<HRFRasterFile>       pi_rpRasterFile,
                                                  const HRFCacheFileCreator*  pi_pCreator)
    {
    HFCPtr<HRFRasterFile> pRasterFile = pi_rpRasterFile;

    // Add the Decoration HGR or the TFW Page File
    if (HRFPageFileFactory::GetInstance()->HasFor(pRasterFile))
        pRasterFile = new HRFRasterFilePageDecorator(pRasterFile, HRFPageFileFactory::GetInstance()->FindCreatorFor(pRasterFile));
    {
    // Adapt only when the raster file has not the same storage type
    pRasterFile = HRFRasterFileBlockAdapter::CreateBestAdapterFor(pRasterFile);

    // Add the cache or res booster when necessary
    //        if (HRFRasterFileResBooster::NeedResBoosterFor(pRasterFile))
    pRasterFile = new HRFRasterFileResBooster(pRasterFile, pi_pCreator);
    //        else
    //        if (HRFRasterFileCache::NeedCacheFor(pRasterFile))
    //            pRasterFile = new HRFRasterFileCache(pRasterFile, pi_pCreator);

    }
    return pRasterFile;
    }



void CActiveImageDoc::OnFilePreferencesCachecompressions()
    {
    POSITION pos = GetFirstViewPosition();
    CActiveImageView* pView = (CActiveImageView*) GetNextView(pos);
    ASSERT_VALID(pView);

    CCachePreferencesDlg dlg(pView);

    if (dlg.DoModal() == IDOK)
        {
        }
    }


void CActiveImageDoc::OnUpdateFilePreferenceFitImageOnView(CCmdUI* pCmdUI)
    {
    pCmdUI->SetCheck(m_FitImageOnView);
    }
void CActiveImageDoc::OnFilePreferenceFitImageOnView()
    {
    m_FitImageOnView = !m_FitImageOnView;
    }







void CActiveImageDoc::OnUpdateFileInfoTime(CCmdUI* pCmdUI)
    {
    HTREEITEM  SelObject = GetSelectedObject();
    pCmdUI->Enable((SelObject != NULL) && (SelObject != GetDocumentObject()));
    }
void CActiveImageDoc::OnFileInfoTime()
    {
    // FileInfo Stat
    HTREEITEM  SelObject = GetSelectedObject();
    if ((SelObject != NULL) &&
        (SelObject != GetDocumentObject()))
        {
        CStringW FileName = GetTreeCtrl()->GetItemText(SelObject);

        HFCPtr<HFCURL>  SrcFileName(HFCURL::Instanciate(Utf8String(FileName.GetString())));
        if (SrcFileName == 0)
            {
            // Open the raster file as a file
            SrcFileName = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + Utf8String(FileName.GetString()));
            }

        HFCStat Stat(SrcFileName);

        CTime CreationTime((time_t) Stat.GetCreationTime());
        CTime LastAccessTime((time_t) Stat.GetLastAccessTime());
        CTime ModifTime((time_t) Stat.GetModificationTime());

        WChar aBuffer[1024];
        _stprintf(aBuffer, _TEXT("Creation Time     : %s\nModif Time         : %s\nLastAccess Time : %s\nFileSize : %llu Bytes"),
                  (LPCTSTR) CreationTime.Format(_TEXT("%c")),
                  (LPCTSTR) ModifTime.Format(_TEXT("%c")),
                  (LPCTSTR) LastAccessTime.Format(_TEXT("%c")),
                  Stat.GetSize());
        AfxMessageBox(aBuffer, MB_OK);

        }
    }

void CActiveImageDoc::OnUpdateView1bitForeground(CCmdUI *pCmdUI)
    {
    pCmdUI->SetCheck(pCmdUI->m_nID == m_1BitForegroundMode);
    }

void CActiveImageDoc::OnView1bitForeground(UINT nID)
    {
    if (m_1BitForegroundMode != nID)
        {
        m_1BitForegroundMode = nID;

        HTREEITEM SelObject = GetSelectedObject();

        // Verify that the current selection is the document item
        if (SelObject == NULL || SelObject == GetDocumentObject())
            return;

        // get the currently selected object from the document
        HFCPtr<HRARaster> pRaster = GetRaster(SelObject);
        if (pRaster != NULL && pRaster->GetPixelType()->Get1BitInterface() != NULL)
            {
            switch (m_1BitForegroundMode)
                {
                    case ID_VIEW_1BIT_FOREGROUND:
                        pRaster->GetPixelType()->Get1BitInterface()->SetForegroundState(true);
                        break;
                    case ID_VIEW_1BIT_BACKGROUND:
                        pRaster->GetPixelType()->Get1BitInterface()->SetForegroundState(false);
                        break;
                    case ID_VIEW_1BIT_EQUALWEIGHT:
                    default:
                        pRaster->GetPixelType()->Get1BitInterface()->UndefineForegroundState();
                        break;
                }
            pRaster->InvalidateRaster();
            }

        UpdateAllViews(NULL, UPDATE_REDRAW | UPDATE_KEEPZOOM);
        }
    }




//-----------------------------------------------------------------------------
// private section
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// private
// OpenRasterFile
//-----------------------------------------------------------------------------
void CActiveImageDoc::OpenRasterFile(Utf8CP pi_pFileName,
                                     list<HFCPtr<HRARaster> >*  po_pPageList)
    {
    HPRECONDITION(po_pPageList != 0);

    BOOL                  Result = true;
    HFCPtr<HRARaster>     pRaster;
    uint64_t             Offset = 0;
    HFCPtr<HFCURL>        pFileName;
    bool                 IsPSSFile = false;

    WString filenameW(pi_pFileName, BentleyCharEncoding::Utf8);

    if (0 == strcmp(&pi_pFileName[strlen(pi_pFileName) - 3], ".ve"))
        {
        if (0 == strcmp(&pi_pFileName[strlen(pi_pFileName) - 9], "aerial.ve"))
            pFileName = new HFCURLHTTP(HFCURLHTTP::s_SchemeName() + "://" + "VirtualEarth.net?Aerial");
        else if (0 == strcmp(&pi_pFileName[strlen(pi_pFileName) - 9], "hybrid.ve"))
            pFileName = new HFCURLHTTP(HFCURLHTTP::s_SchemeName() + "://" + "VirtualEarth.net?Hybrid");
        else if (0 == strcmp(&pi_pFileName[strlen(pi_pFileName) - 13], "hillshaded.ve"))
            pFileName = new HFCURLHTTP(HFCURLHTTP::s_SchemeName() + "://" + "VirtualEarth.net?HillShaded");
        else
            pFileName = new HFCURLHTTP(HFCURLHTTP::s_SchemeName() + "://" + "VirtualEarth.net?Road");
        }
    else
        {
        if (0 == strcmp(&pi_pFileName[strlen(pi_pFileName) - 4], ".raw"))
            {
            COpenRawFileDialog  Dlg(NULL, filenameW.c_str(), 0, 0);
            if (Dlg.DoModal() == IDCANCEL)
                Result = false;
            else
                Offset = HRFRawCreator::GetInstance()->GetOffset();
            }

        if (0 == strcmp(&pi_pFileName[strlen(pi_pFileName) - 4], ".pss"))
            {
            IsPSSFile = true;
            }

        pFileName = new HFCURLFile(HFCURLFile::s_SchemeName() + "://" + pi_pFileName);
        }
    if (Result)
        {
        if (IsPSSFile)
            {
            // Open the picture script store
            LoadPSS(pFileName, po_pPageList);
            }
        else
            {
            // Try to open the file			
            HFCPtr<HRFRasterFile> pTrueRasterFile(HRFRasterFileFactory::GetInstance()->OpenFile(pFileName, m_ReadOnlyMode, Offset));
            HFCPtr<HRFRasterFile> pRasterFile;

            if (!m_ReadOnlyMode && !pTrueRasterFile->GetAccessMode().m_HasWriteAccess)
                {
                
                CString ErrorMessage(_TEXT("Warning : File : ") + CString(filenameW.c_str()) + _TEXT(" is treated as read only"));
                AfxMessageBox(ErrorMessage);
                }

            m_FileAccessMode = pTrueRasterFile->GetAccessMode();

#ifdef __HMR_DEBUG

            if (pTrueRasterFile->GetPageDescriptor(0)->HasTag<HRFAttribute3DTransformationMatrix>())
                {
                //HRFAttribute3DTransformationMatrix const* pMatrix3D = pTrueRasterFile->GetPageDescriptor(0)->FindTagCP<HRFAttribute3DTransformationMatrix>();                

                if (false)
                    {
                    pTrueRasterFile->GetPageDescriptor(0)->RemoveTag<HRFAttribute3DTransformationMatrix>();
                    }
                }
#endif

            pRasterFile = ImproveRasterFile(pTrueRasterFile);

            if (pRasterFile == 0)
                {
                CString ErrorMessage;
                AfxFormatString1(ErrorMessage, IDS_MSGERR_FILENOTSUPPORTED, filenameW.c_str());
                AfxMessageBox(ErrorMessage);
                }
            else
                {
                LoadRasterFile(pRasterFile, po_pPageList);
                }

            HASSERT(!po_pPageList->empty());
            }
        }
    }

HFCPtr<HRFRasterFile> CActiveImageDoc::ImproveRasterFile(HFCPtr<HRFRasterFile>& pi_rpRasterFile) const
    {
    HFCPtr<HRFRasterFile> pRasterFile = pi_rpRasterFile;

    if (m_LoadInMemoryID != ID_OPTIONS_LOADINMEMORY_DISABLED)
        {
        bool CreateBestAdapter = true;

        // Adapt Scan Line Orientation (1 bit images)
        if ((pRasterFile->IsCompatibleWith(HRFIntergraphFile::CLASS_ID)) ||
            (pRasterFile->IsCompatibleWith(HRFCalsFile::CLASS_ID)))
            {
            if (HRFSLOStripAdapter::NeedSLOAdapterFor(pRasterFile))
                {
                // Adapt only when the raster file has not a standard scan line orientation
                // i.e. with an upper left origin, horizontal scan line.
                pRasterFile = HRFSLOStripAdapter::CreateBestAdapterFor(pRasterFile);
                CreateBestAdapter = false;
                }
            }

        // Add the Decoration HGR or the TFW Page File
        if (HRFPageFileFactory::GetInstance()->HasFor(pRasterFile))
            pRasterFile = new HRFRasterFilePageDecorator(pRasterFile, HRFPageFileFactory::GetInstance()->FindCreatorFor(pRasterFile));

        if (ID_OPTIONS_LOADINMEMORY_IMAGE == m_LoadInMemoryID &&
            HRFRasterFileBlockAdapter::CanAdapt(pRasterFile, HRFBlockType::IMAGE, HRF_EQUAL_TO_RESOLUTION_WIDTH, HRF_EQUAL_TO_RESOLUTION_HEIGHT))
            {
            pRasterFile = new HRFRasterFileBlockAdapter(pRasterFile,
                                                        HRFBlockType::IMAGE,
                                                        HRF_EQUAL_TO_RESOLUTION_WIDTH,
                                                        HRF_EQUAL_TO_RESOLUTION_HEIGHT);
            }
        else if (ID_OPTIONS_LOADINMEMORY_STRIP == m_LoadInMemoryID &&
                 HRFRasterFileBlockAdapter::CanAdapt(pRasterFile, HRFBlockType::STRIP, HRF_EQUAL_TO_RESOLUTION_WIDTH, 256))
            {
            pRasterFile = new HRFRasterFileBlockAdapter(pRasterFile,
                                                        HRFBlockType::STRIP,
                                                        HRF_EQUAL_TO_RESOLUTION_WIDTH,
                                                        256);
            }
        else
            {
            // Adapt only when the raster file has not the same storage type
            pRasterFile = HRFRasterFileBlockAdapter::CreateBestAdapterFor(pRasterFile);

            // Add the cache or res booster when necessary
            if (HRFRasterFileResBooster::NeedResBoosterFor(pRasterFile))
                pRasterFile = new HRFRasterFileResBooster(pRasterFile, HRFiTiffCacheFileCreator::GetInstance());
            else if (HRFRasterFileCache::NeedCacheFor(pRasterFile))
                pRasterFile = new HRFRasterFileCache(pRasterFile, HRFiTiffCacheFileCreator::GetInstance());
            }
        }
    else if (!m_DisableCache && m_NormalCache)
        {
        // Add the necessary booster to improve the raster file performance.
        pRasterFile = GenericImprove(pRasterFile, HRFiTiffCacheFileCreator::GetInstance(), m_PageFileOverwrite);
        }
    else
        {
        bool CreateBestAdapter = true;

        pRasterFile = pi_rpRasterFile;

        // Adapt Scan Line Orientation (1 bit images)
        if ((pRasterFile->IsCompatibleWith(HRFIntergraphFile::CLASS_ID)) ||
            (pRasterFile->IsCompatibleWith(HRFCalsFile::CLASS_ID)))
            {
            if (HRFSLOStripAdapter::NeedSLOAdapterFor(pRasterFile))
                {
                // Adapt only when the raster file has not a standard scan line orientation
                // i.e. with an upper left origin, horizontal scan line.
                pRasterFile = HRFSLOStripAdapter::CreateBestAdapterFor(pRasterFile);
                CreateBestAdapter = false;
                }
            }

        // Add the Decoration HGR or the TFW Page File
        if (HRFPageFileFactory::GetInstance()->HasFor(pRasterFile, m_PageFileOverwrite))
            pRasterFile = new HRFRasterFilePageDecorator(pRasterFile, HRFPageFileFactory::GetInstance()->FindCreatorFor(pRasterFile));

        // Adapt only when the raster file has not the same storage type
        if (CreateBestAdapter)
            pRasterFile = HRFRasterFileBlockAdapter::CreateBestAdapterFor(pRasterFile);

        if (!m_DisableCache)
            {
#if 0
            // Add the cache or res booster when necessary
            if (HRFRasterFileResBooster::NeedResBoosterFor(pRasterFile))
                {
                HASSERT(pRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWidth() <= HULONG_MAX);
                HASSERT(pRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetHeight() <= HULONG_MAX);

                HGFResolutionDescriptor BoosterDescriptor((uint32_t) pRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWidth(),
                                                          (uint32_t) pRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetHeight());

                if (m_Cache16384)
                    BoosterDescriptor.AddResolutionThatSupports(16384, 16384, 256, 256, 2);
                if (m_Cache8192)
                    BoosterDescriptor.AddResolutionThatSupports(8192, 8192, 256, 256, 2);
                if (m_Cache4096)
                    BoosterDescriptor.AddResolutionThatSupports(4096, 4096, 256, 256, 2);
                if (m_Cache2048)
                    BoosterDescriptor.AddResolutionThatSupports(2048, 2048, 256, 256, 2);
                if (m_Cache1024)
                    BoosterDescriptor.AddResolutionThatSupports(1024, 1024, 256, 256, 2);
                if (m_Cache512)
                    BoosterDescriptor.AddResolutionThatSupports(512, 512, 256, 256, 2);
                if (m_Cache256)
                    BoosterDescriptor.AddResolutionThatSupports(256, 256, 256, 256, 2);
                if (m_Cache128)
                    BoosterDescriptor.AddResolutionThatSupports(128, 128, 256, 256, 2);

                HRFRasterFileResBooster::WantedResolutionsMap BoosterDescMap;
                for (uint32_t Page = 0; Page < pRasterFile->CountPages(); Page++)
                    BoosterDescMap.insert(HRFRasterFileResBooster::WantedResolutionsMap::value_type(Page, BoosterDescriptor));

                pRasterFile = new HRFRasterFileResBooster(pRasterFile, HRFiTiffCacheFileCreator::GetInstance(), &BoosterDescMap);
                }
            else if (HRFRasterFileCache::NeedCacheFor(pRasterFile))
                pRasterFile = new HRFRasterFileCache(pRasterFile, HRFiTiffCacheFileCreator::GetInstance());
#endif
            }
        }

    return pRasterFile;
    }

//-----------------------------------------------------------------------------
// private
// SaveRasterFile
//-----------------------------------------------------------------------------
BOOL CActiveImageDoc::SaveRasterFile(Utf8CP pi_pFileName)
    {
    // Obtain the currently selected Object
    HTREEITEM  SelObject = GetSelectedObject();
    if ((SelObject == NULL) ||
        (SelObject == m_DocItem))
        {
        AfxMessageBox(_TEXT("Cannot save document item as a raster file."));
        return false;
        }

#if 0  //DMx
    const HRFHMRRasterFileRegistry* pRegistry = ((CActiveImageApp*) AfxGetApp())->GetRegistry();
    const HRFRasterFileFormat*      pFormat;

    // Get the right file format from the registry
    pFormat = pRegistry->IdentifyByExtension(pi_pFileName);
    if (pFormat == 0)
        {
        CString Str;
        AfxFormatString1(Str, IDS_MSGERR_FILENOTSUPPORTED, pi_pFileName);
        AfxMessageBox(Str);
        return false;
        }
#endif

    // Save the Raster ????
    HFCPtr<HRARaster> pRaster(GetRaster(SelObject));
    if (pRaster == 0 || pRaster->GetStore() == 0)
        return false;

    GetRaster(SelObject)->Save();

    return true;
    }

//-----------------------------------------------------------------------------
// private
// InsertRaster
//-----------------------------------------------------------------------------
void CActiveImageDoc::InsertRaster(HFCPtr<HRARaster>&   pi_rpRaster,
                                   bool                pi_InsertAsLink,
                                   HTREEITEM            pi_ParentItem,
                                   const WChar*        pi_pFileName)
    {
    HPRECONDITION(pi_rpRaster != 0);
    HTREEITEM         NewItem = NULL;
    CTreeCtrl*        pTree = GetTreeCtrl();

    //
    // If the raster is not to be inserted as a link, add it to the list
    // of raster to clone.
    //
    if (!pi_InsertAsLink)
        m_RasterToClone.push_back(pi_rpRaster);

    //
    // Set the document's modify flag to true, since there is a change
    //
    SetModifiedFlag(true);

    //
    // insert the raster in the tree structure and in the mosaic if the parent is a mosaic
    //
    NewItem = InsertRasterInTree(pi_rpRaster, pi_ParentItem, pi_pFileName);
    if (pi_ParentItem != m_DocItem)
        {
        HFCPtr<HIMMosaic> pMosaic = static_cast<HIMMosaic*>(GetRaster(pi_ParentItem).GetPtr());
        HASSERT(pMosaic != 0);
        HASSERT(pMosaic->GetClassID() == HIMMosaic::CLASS_ID);
        HASSERT(!pMosaic->Contains(pi_rpRaster, true));
        pMosaic->Add(pi_rpRaster);
        }

    //#ifdef STRESS_APPS
    pTree->Select(NewItem, TVGN_CARET);
    //#endif
    }

//-----------------------------------------------------------------------------
// private
// InsertRasterInTree - Adds the raster to the Tree List
//-----------------------------------------------------------------------------
HTREEITEM CActiveImageDoc::InsertRasterInTree(HFCPtr<HRARaster> pi_pRaster,
                                              HTREEITEM         pi_ParentItem,
                                              const WChar*     pi_pRasterName,
                                              HTREEITEM         pi_InsertAfter)
    {
    CTreeCtrl* pTree = GetTreeCtrl();
    WChar     TreeLabel[1024];
    HTREEITEM  NewItem;
    HTREEITEM  InsertAfter = TVI_FIRST;
    uint32_t    ImageID;
    HPRECONDITION(pi_pRaster != 0);
    HPRECONDITION(pi_ParentItem != NULL);

    // Generate the label for the tree item
    if (pi_pRasterName == 0)
        {
        // verify if mosaic or raster
        if (pi_pRaster->GetClassID() == HIMMosaic::CLASS_ID)
            _stprintf(TreeLabel, _TEXT("Mosaic %d"), m_NumMosaic++);
        else
            _stprintf(TreeLabel, _TEXT("Image %d"), m_NumImages++);
        }
    else
        _stprintf(TreeLabel, _TEXT("%s"), pi_pRasterName);


    // If we're adding the raster to the document item, 
    // add it after all its sibling.  Otherwise add it on top of its sibling
    if (pi_ParentItem == m_DocItem)
        InsertAfter = TVI_LAST;


    // Select the Image ID for the tree item icon
    if (IsRasterLinked(pi_pRaster))
        {
        if (pi_pRaster->GetClassID() == HIMMosaic::CLASS_ID)
            ImageID = TLI_LINK_MOSAIC;
        else
            ImageID = TLI_LINK_RASTER;
        }
    else
        {
        // verify if the raster is in fact a reference, if so, verify with the source
        if (pi_pRaster->GetClassID() == HRAReferenceToRaster::CLASS_ID)
            {
            HFCPtr<HRARaster> pSource = ((HFCPtr<HRAReferenceToRaster>&)pi_pRaster)->GetSource();
            HASSERT(pSource != 0);
            if (pSource->GetClassID() == HIMMosaic::CLASS_ID)
                ImageID = TLI_REF_MOSAIC;
            else
                ImageID = TLI_REF_RASTER;
            }
        else
            {
            if (pi_pRaster->GetClassID() == HIMMosaic::CLASS_ID)
                ImageID = TLI_MOSAIC;
            else
                ImageID = TLI_RASTER;
            }
        }

    // Create the tree item
    NewItem = pTree->InsertItem(TreeLabel, ImageID, ImageID + 1, pi_ParentItem, InsertAfter);
    HASSERT(NewItem != NULL);

    // Add the raster in the raster map
    m_RasterMap.insert(RasterMap::value_type(NewItem, pi_pRaster));

    // if the raster is a mosaic, insert also its source
    if (pi_pRaster->GetClassID() == HIMMosaic::CLASS_ID)
        {
        HFCPtr<HIMMosaic>& pMosaic = (HFCPtr<HIMMosaic>&) pi_pRaster;
        HIMMosaic::IteratorHandle Iterator = pMosaic->StartIteration();
        if (pMosaic->CountElements(Iterator) > 0)
            {
            HFCPtr<HRARaster> pRaster = pMosaic->GetElement(Iterator);
            while (pRaster != 0)
                //        for (UInt32 i = 0; i < pMosaic->CountElements(Iterator); i++)
                {
                // Inser the current source
                InsertRasterInTree(pRaster, NewItem);

                // get the next raster
                pRaster = pMosaic->Iterate(Iterator);
                }
            }
        pMosaic->StopIteration(Iterator);
        }

    // expand the parent
    pTree->Expand(pi_ParentItem, TVE_EXPAND);

    return (NewItem);
    }
//-----------------------------------------------------------------------------
// private
// InsertMultiPageRaster
//-----------------------------------------------------------------------------
void CActiveImageDoc::InsertMultiPageRaster(list<HFCPtr<HRARaster> >&    pi_rPageList,
                                            bool                        pi_InsertAsLink,
                                            HTREEITEM                    pi_ParentItem,
                                            const WChar*                pi_pFileName)
    {
    HPRECONDITION(pi_rPageList.size() > 1);
    HPRECONDITION(pi_pFileName != 0);

    CTreeCtrl* pTree = GetTreeCtrl();
    HTREEITEM  InsertAfter;
    HTREEITEM  NewItem;

    // If we're adding the raster to the document item, 
    // add it after all its sibling.  Otherwise add it on top of its sibling
    if (pi_ParentItem == m_DocItem)
        InsertAfter = TVI_LAST;
    else
        InsertAfter = TVI_FIRST;

    NewItem = pTree->InsertItem(pi_pFileName, TLI_MOSAIC, TLI_MOSAIC + 1, pi_ParentItem, InsertAfter);
    HASSERT(NewItem != NULL);

    list<HFCPtr<HRARaster> >::const_iterator Itr(pi_rPageList.begin());
    uint32_t PageIndex = 0;
    while (Itr != pi_rPageList.end())
        {
        wostringstream Label;
        Label << _TEXT("Page") << PageIndex;
        InsertRasterInTree(*Itr,
                           NewItem,
                           Label.str().c_str(),
                           TVI_LAST);
        PageIndex++;
        Itr++;
        }

    // expand the parent
    pTree->Expand(pi_ParentItem, TVE_EXPAND);
    }

//-----------------------------------------------------------------------------
// private
// VerifyLoopForward
//-----------------------------------------------------------------------------
bool CActiveImageDoc::VerifyLoopForward(HFCPtr<HRARaster>& pi_pRaster1,
                                        HFCPtr<HRARaster>& pi_pRaster2)
    {
    bool Result = true;
    HPRECONDITION(pi_pRaster1 != 0);
    HPRECONDITION(pi_pRaster2 != 0);

    // verify if we're trying to insert the mosaic in itself
    if (pi_pRaster1 == pi_pRaster2)
        {
        Result = false;
        }
    else
        {
        // verify that the first raster is a Mosaic
        if (pi_pRaster1->GetClassID() == HIMMosaic::CLASS_ID)
            {
            HFCPtr<HIMMosaic>& pMosaic1 = (HFCPtr<HIMMosaic>&)pi_pRaster1;
            HASSERT(pMosaic1 != 0);

            // verify if the raster is in the mosaic
            if (pMosaic1->Contains(pi_pRaster2, true))
                {
                // the raster is in the mosaic somewhere, stop the search here
                Result = false;
                }
            else
                {
                // the raster is not in the mosaic.  If it is itself a mosaic,
                // verify if its sources are in the mosaic.
                if (pi_pRaster2->GetClassID() == HIMMosaic::CLASS_ID)
                    {
                    HFCPtr<HIMMosaic> pMosaic2 = (HFCPtr<HIMMosaic>&) pi_pRaster2;
                    HIMMosaic::IteratorHandle Iterator = pMosaic2->StartIteration();

                    for (uint32_t i = 0; i < pMosaic2->CountElements(Iterator); i++)
                        {
                        // get the current element
                        HFCPtr<HRARaster> pRaster = pMosaic2->GetElement(Iterator);
                        HASSERT(pRaster != 0);

                        // Recurse on the current raster
                        if (!(Result = VerifyLoopForward(pi_pRaster1, pRaster)))
                            break;

                        // get the next raster
                        pMosaic2->Iterate(Iterator);
                        }

                    pMosaic2->StopIteration(Iterator);
                    }
                }
            }
        }

    return (Result);
    }

//-----------------------------------------------------------------------------
// private
// VerifyLoopBackward
//-----------------------------------------------------------------------------
bool CActiveImageDoc::VerifyLoopBackward(HFCPtr<HRARaster>& pi_pRaster1,
                                         HFCPtr<HRARaster>& pi_pRaster2)
    {
    // verify forward, but invert the parameters
    return VerifyLoopForward(pi_pRaster2, pi_pRaster1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HFCPtr<HRFRasterFile>   GetOriginalRasterFile(HFCPtr<HRFRasterFile>& pRasterFile)
    {
    if (pRasterFile->IsCompatibleWith(HRFRasterFileExtender::CLASS_ID))
        return GetOriginalRasterFile(((HRFRasterFileExtender*) pRasterFile.GetPtr())->GetOriginalFile());

    return pRasterFile;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename Out_T>
inline bool IsValidDEMValue_T(Out_T const& value)
    {
    // *** If you get an error here, you must implement the specialization for type 'Out_T'
    }

template<>
inline bool IsValidDEMValue_T(uint8_t const& value)
    {
    return true;
    }

template<>
inline bool IsValidDEMValue_T(uint16_t const& value)
    {
    return true;
    }

template<>
inline bool IsValidDEMValue_T(uint32_t const& value)
    {
    return true;
    }

template<>
inline bool IsValidDEMValue_T(int16_t const& value)
    {
    return value > -30000;
    }

template<>
inline bool IsValidDEMValue_T(float const& value)
    {
    return value > -30000.0F;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
template<class Data_T, uint32_t Count_T>
bool NativeScan(vector<double>& minValues, vector<double>& maxValues, Byte const* pData, uint32_t width, uint32_t height, double const* pNoDataValue)
    {
    size_t pitch = width*sizeof(Data_T)*Count_T;

    Data_T NoDataValueTyped = pNoDataValue ? (Data_T) *pNoDataValue : 0;
    Data_T minValueTyped[Count_T];
    Data_T maxValueTyped[Count_T];
    bool isValidMin[Count_T];
    bool isValidMax[Count_T];

    for (uint32_t channel = 0; channel < Count_T; ++channel)
        {
        isValidMin[channel] = false;
        isValidMax[channel] = false;
        minValueTyped[channel] = std::numeric_limits<Data_T>::max();
        maxValueTyped[channel] = std::numeric_limits<Data_T>::lowest(); // min doesn't give negative value for float!
        }

    for (uint32_t line = 0; line < height; ++line)
        {
        Data_T const* pDataLine = (Data_T const*) (pData + line*pitch);

        for (uint32_t column = 0; column < width; ++column)
            {
            for (uint32_t channel = 0; channel < Count_T; ++channel)
                {
                if (!IsValidDEMValue_T(pDataLine[(column*Count_T) + channel]) ||
                    (pNoDataValue != NULL && NoDataValueTyped == pDataLine[(column*Count_T) + channel]))
                    continue;

                if (!isValidMin[channel] || pDataLine[(column*Count_T) + channel] < minValueTyped[channel])
                    {
                    minValueTyped[channel] = pDataLine[(column*Count_T) + channel];
                    isValidMin[channel] = true;
                    }

                if (!isValidMax[channel] || pDataLine[(column*Count_T) + channel] > maxValueTyped[channel])
                    {
                    maxValueTyped[channel] = pDataLine[(column*Count_T) + channel];
                    isValidMax[channel] = true;
                    }
                }
            }
        }

    minValues.resize(Count_T);
    maxValues.resize(Count_T);

    bool allValid = true;
    for (uint32_t channel = 0; channel < Count_T; ++channel)
        {
        minValues[channel] = isValidMin[channel] ? minValueTyped[channel] : std::numeric_limits<Data_T>::lowest();
        maxValues[channel] = isValidMax[channel] ? maxValueTyped[channel] : std::numeric_limits<Data_T>::max();

        allValid = allValid && isValidMin[channel] && isValidMax[channel];
        }

    return allValid;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ComputeMinMaxValues(vector<double>& minValues, vector<double>& maxValues, HFCPtr<HRFRasterFile>& pRasterFile, uint32_t pageNumber)
    {
    HRFAttributeMinSampleValue const* pMinValueTag = pRasterFile->GetPageDescriptor(pageNumber)->FindTagCP<HRFAttributeMinSampleValue>();
    HRFAttributeMaxSampleValue const* pMaxValueTag = pRasterFile->GetPageDescriptor(pageNumber)->FindTagCP<HRFAttributeMaxSampleValue>();
    if (pMinValueTag != NULL && pMaxValueTag != NULL)
        {
        minValues = pMinValueTag->GetData();
        maxValues = pMaxValueTag->GetData();
        return true;
        }

    // Need to scan the file
    HFCPtr<HRFRasterFile> pTrueRasterFile = GetOriginalRasterFile(pRasterFile);
    uint32_t preferedWidth = std::min<uint32_t>((uint32_t) pTrueRasterFile->GetPageDescriptor(pageNumber)->GetResolutionDescriptor(0)->GetWidth(), 1024);
    uint32_t preferedHeight = std::min<uint32_t>((uint32_t) pTrueRasterFile->GetPageDescriptor(pageNumber)->GetResolutionDescriptor(0)->GetWidth(), 1024);
    HFCPtr<HRFThumbnail> pThumbnail = HRFThumbnailMaker(pTrueRasterFile, pageNumber, &preferedWidth, &preferedHeight, false);

    bool succeeded = true;

    switch (pThumbnail->GetPixelType()->GetClassID())
        {
            case HRPPixelTypeId_V8Gray8:
                succeeded = NativeScan<uint8_t, 1>(minValues, maxValues, pThumbnail->GetDataP(), pThumbnail->GetWidth(), pThumbnail->GetHeight(), pThumbnail->GetPixelType()->GetChannelOrg().GetChannelPtr(0)->GetNoDataValue());
                break;

            case HRPPixelTypeId_V16Int16:
                succeeded = NativeScan<int16_t, 1>(minValues, maxValues, pThumbnail->GetDataP(), pThumbnail->GetWidth(), pThumbnail->GetHeight(), pThumbnail->GetPixelType()->GetChannelOrg().GetChannelPtr(0)->GetNoDataValue());
                break;

            case HRPPixelTypeId_V16Gray16:
                succeeded = NativeScan<uint16_t, 1>(minValues, maxValues, pThumbnail->GetDataP(), pThumbnail->GetWidth(), pThumbnail->GetHeight(), pThumbnail->GetPixelType()->GetChannelOrg().GetChannelPtr(0)->GetNoDataValue());
                break;

            case HRPPixelTypeId_V32Float32:
                succeeded = NativeScan<float, 1>(minValues, maxValues, pThumbnail->GetDataP(), pThumbnail->GetWidth(), pThumbnail->GetHeight(), pThumbnail->GetPixelType()->GetChannelOrg().GetChannelPtr(0)->GetNoDataValue());
                break;

            case HRPPixelTypeId_V48R16G16B16:
                succeeded = NativeScan<uint16_t, 3>(minValues, maxValues, pThumbnail->GetDataP(), pThumbnail->GetWidth(), pThumbnail->GetHeight(), pThumbnail->GetPixelType()->GetChannelOrg().GetChannelPtr(0)->GetNoDataValue());
                break;

            case HRPPixelTypeId_V64R16G16B16A16:
            case HRPPixelTypeId_V64R16G16B16X16:
                succeeded = NativeScan<uint16_t, 4>(minValues, maxValues, pThumbnail->GetDataP(), pThumbnail->GetWidth(), pThumbnail->GetHeight(), pThumbnail->GetPixelType()->GetChannelOrg().GetChannelPtr(0)->GetNoDataValue());
                break;

            case HRPPixelTypeId_V96R32G32B32:
                succeeded = NativeScan<uint32_t, 3>(minValues, maxValues, pThumbnail->GetDataP(), pThumbnail->GetWidth(), pThumbnail->GetHeight(), pThumbnail->GetPixelType()->GetChannelOrg().GetChannelPtr(0)->GetNoDataValue());
                break;

                // not supported.
                //case HRPPixelTypeId_I1R8G8B8:
                //case HRPPixelTypeId_I1R8G8B8:
                //case HRPPixelTypeId_I1R8G8B8RLE:
                //case HRPPixelTypeId_I1R8G8B8A8:
                //case HRPPixelTypeId_I1R8G8B8A8RLE:
                //case HRPPixelTypeId_I2R8G8B8:
                //case HRPPixelTypeId_I4R8G8B8:
                //case HRPPixelTypeId_I4R8G8B8A8:
                //case HRPPixelTypeId_V1Gray1:
                //case HRPPixelTypeId_V1GrayWhite1:
                //case HRPPixelTypeId_V16B5G5R5:
                //case HRPPixelTypeId_V16R5G6B5:
                //case HRPPixelTypeId_I8R8G8B8Mask:
                //case HRPPixelTypeId_I8R8G8B8:
                //case HRPPixelTypeId_I8R8G8B8A8:
                //case HRPPixelTypeId_I8VA8R8G8B8:    
                //case HRPPixelTypeId_I8Gray8:
                //case HRPPixelTypeId_V8GrayWhite8:
                //case HRPPixelTypeId_V32PRPhotoYCCA8:
                //case HRPPixelTypeId_V32PR8PG8PB8A8:
                //case HRPPixelTypeId_V32A8R8G8B8:
                //case HRPPixelTypeId_V32CMYK:
                //case HRPPixelTypeId_V32R8G8B8A8:
                //case HRPPixelTypeId_V32B8G8R8X8:
                //case HRPPixelTypeId_V32R8G8B8X8:
                //case HRPPixelTypeId_V24R8G8B8:
                //case HRPPixelTypeId_V24B8G8R8:
                //case HRPPixelTypeId_V24PhotoYCC:
                //case HRPPixelTypeId_V16PRGray8A8:
            default:
                succeeded = false;
                break;
        }

    return succeeded;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HFCPtr<HRARaster> AddDemFilter(HFCPtr<HRARaster>& pRaster, double minValue, double maxValue)
    {
    BeAssert(pRaster->GetPixelType()->GetChannelOrg().CountChannels() == 1);

    if (!pRaster->IsCompatibleWith(HRAReferenceToStoredRaster::CLASS_ID) && !pRaster->IsCompatibleWith(HRAStoredRaster::CLASS_ID))
        return pRaster;

    HRPDEMFilter::HillShadingSettings hillShading;
    hillShading.SetHillShadingState(false);

    HFCPtr<HRPDEMFilter> pDemFilter = new HRPDEMFilter(hillShading, HRPDEMFilter::Style_Elevation);

    pDemFilter->SetClipToEndValues(true);

    double step = (maxValue - minValue) / 255;    // First step is minimum.
    HRPDEMFilter::UpperRangeValues upperRangeValues;
    for (uint32_t i = 0; i < 256; ++i)
        upperRangeValues.insert(HRPDEMFilter::UpperRangeValues::value_type(minValue + (i*step), HRPDEMFilter::RangeInfo((Byte) i, (Byte) i, (Byte) i, true)));
    pDemFilter->SetUpperRangeValues(upperRangeValues);

    if (pRaster->IsCompatibleWith(HRAReferenceToStoredRaster::CLASS_ID))
        return new HRADEMRaster(reinterpret_cast<HFCPtr<HRAReferenceToStoredRaster>&> (pRaster), 100.0, 100.0, new HGF2DIdentity(), pDemFilter);

    return new HRADEMRaster(reinterpret_cast<HFCPtr<HRAStoredRaster>&> (pRaster), 100.0, 100.0, new HGF2DIdentity(), pDemFilter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HFCPtr<HRARaster> AddBestContrast(HFCPtr<HRARaster>& pRaster, HFCPtr<HRFRasterFile>& pRasterFile, uint32_t pageNumber)
    {
    if (pRaster->GetPixelType()->GetChannelOrg().GetChannelPtr(0)->GetSize() < 8)
        return pRaster;

    vector<double> minValues, maxValues;

    if (!ComputeMinMaxValues(minValues, maxValues, pRasterFile, pageNumber))
        return pRaster;

    if (pRaster->GetPixelType()->GetChannelOrg().CountChannels() == 1)
        return AddDemFilter(pRaster, minValues[0], maxValues[0]);

    if (pRaster->GetPixelType()->GetChannelOrg().GetChannelPtr(0)->GetSize() == 16)
        {
        HFCPtr<HRPContrastStretchFilter16> pFilter = new HRPContrastStretchFilter16(pRaster->GetPixelType());
        for (unsigned short i = 0; i < pRaster->GetPixelType()->GetChannelOrg().CountChannels(); ++i)
            pFilter->SetInterval(i, (int32_t) minValues[i], (int32_t) maxValues[i]);

        return new HIMFilteredImage(pRaster, pFilter.GetPtr());
        }

    return pRaster;
    }

//-----------------------------------------------------------------------------
// private 
// LoadRasterFile
//-----------------------------------------------------------------------------
void CActiveImageDoc::LoadRasterFile(HFCPtr<HRFRasterFile>&     pi_rpRasterFile,
                                     list<HFCPtr<HRARaster> >*  po_pRasterList)
    {
    HPRECONDITION(pi_rpRasterFile != 0);
    HPRECONDITION(po_pRasterList != 0);

    HFCPtr<HGF2DCoordSys> pLogical;


    HFCPtr<HRARaster> pRaster;
    HFCPtr<HRSObjectStore> pStore;

    for (uint32_t i = 0; i < pi_rpRasterFile->CountPages(); i++)
        {
        pLogical = GetWorldCluster()->GetCoordSysReference(pi_rpRasterFile->GetPageWorldIdentificator(i));
        pStore = new HRSObjectStore(m_pLog,
                                    pi_rpRasterFile,
                                    i,
                                    pLogical);

        if (pStore->CanBeResizable())
            pStore->SetResizableRasterSize(32000, 32000);

        // Get the raster from the store
        pRaster = pStore->LoadRaster();
        HASSERT(pRaster != NULL);

        InitializeRaster(pRaster);

        // Add best contrast
        if (m_BestContrast)
            pRaster = AddBestContrast(pRaster, pi_rpRasterFile, i);

        // If 3 band, 16bit --> ContrastStretch
        if (m_3band16bittorgbID != ID_3BAND16BITTORGB_NONE &&
            pRaster->GetPixelType()->GetChannelOrg().CountChannels() == 3 &&
            pRaster->GetPixelType()->GetChannelOrg().GetChannelPtr(0)->GetSize() == 16)
            {
            double nodata = 0.0;    // 0,0,0 --> transparency
            double HistoCut;
            switch (m_3band16bittorgbID)
                {
                case ID_3BAND16BITTORGB_NATIVESCAN:
                    HistoCut = 0.0;
                    break;

                case ID_3BAND16BITTORGB_0:
                    HistoCut = 0.1;
                    break;

                case ID_3BAND16BITTORGB_1:
                    HistoCut = 0.01;
                    break;

                case ID_3BAND16BITTORGB_2:
                    HistoCut = 0.001;
                    break;
                }

            pRaster = HUTLandSat8ToRGBA (pRaster, pi_rpRasterFile, i, &nodata, HistoCut, m_3band16bitSameValue);
            }

        if (!m_ReadOnlyMode)
            {
            HFCPtr<HRATransactionRecorder> pEditionRecorder(HRATIFFFileTransactionRecorder::CreateFor(pi_rpRasterFile->GetURL(), true)); // create a new file
            ((HFCPtr<HRAStoredRaster>&)pRaster)->SetTransactionRecorder(pEditionRecorder);
            }

        po_pRasterList->push_back(pRaster);
        }
    }

//-----------------------------------------------------------------------------
// private 
// InitializeRaster
//-----------------------------------------------------------------------------
void CActiveImageDoc::InitializeRaster(HFCPtr<HRARaster>& pio_rpRaster) const
    {
    HPRECONDITION(pio_rpRaster != 0);

    if (pio_rpRaster->GetPixelType()->Get1BitInterface() != NULL)
        {
        switch (m_1BitForegroundMode)
            {
                case ID_VIEW_1BIT_FOREGROUND:
                    pio_rpRaster->GetPixelType()->Get1BitInterface()->SetForegroundState(true);
                    break;
                case ID_VIEW_1BIT_BACKGROUND:
                    pio_rpRaster->GetPixelType()->Get1BitInterface()->SetForegroundState(false);
                    break;
                case ID_VIEW_1BIT_EQUALWEIGHT:
                default:
                    pio_rpRaster->GetPixelType()->Get1BitInterface()->UndefineForegroundState();
                    break;
            }
        }

    if (m_RemoveTransparency)
        {
        HFCPtr<HRPPixelType> pPixelType(pio_rpRaster->GetPixelType());

        if (pPixelType->CountIndexBits() > 0)
            {
            // Has a palette
            HRPPixelPalette& rPalette = pPixelType->LockPalette();
            size_t AlphaPos = rPalette.GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0);

            if (AlphaPos != HRPChannelType::FREE)
                {
                // Has an alpha channel
                size_t NumberOfBytes = (rPalette.GetChannelOrg().CountPixelCompositeValueBits() + 7) / 8;
                Byte* pTmpComposite = new Byte[NumberOfBytes];
                uint32_t PaletteEntryCount = rPalette.CountUsedEntries();

                // Pass all used entries in the palette
                for (uint32_t i = 0; i < PaletteEntryCount; i++)
                    {
                    memcpy(pTmpComposite, rPalette.GetCompositeValue(i), NumberOfBytes);

                    // Set opaque. We're assuming that all channels (up to alpha)
                    // are 8 bits wide.
                    pTmpComposite[AlphaPos] = 0xff;

                    rPalette.SetCompositeValue(i, pTmpComposite);
                    }
                delete pTmpComposite;
                }
            pPixelType->UnlockPalette();
            }
        }
    }

//-----------------------------------------------------------------------------
// private 
// LoadPSS
//-----------------------------------------------------------------------------
void CActiveImageDoc::LoadPSS(const HFCPtr<HFCURL>&     pi_rpFileName,
                              list<HFCPtr<HRARaster> >* po_pRasterList)
    {
    HPRECONDITION(pi_rpFileName != 0);
    HPRECONDITION(po_pRasterList != 0);

    HFCPtr<HPSObjectStore> pStore = new HPSObjectStore(m_pLog, pi_rpFileName, GetWorldCluster());

    // Get the raster from the store
    HFCPtr<HRARaster> pRaster = pStore->LoadRaster(0);
    HASSERT(pRaster != NULL);

    InitializeRaster(pRaster);
    po_pRasterList->push_back(pRaster);

    }


#if 0
//-----------------------------------------------------------------------------
// private 
// Test_DumpGeoKeysString
//-----------------------------------------------------------------------------
void CActiveImageDoc::Test_DumpGeoKeysString(HFCPtr<HRFRasterFile>& pi_rpTrueRasterFile,
                                             const WChar*          pi_pFileName) const
    {
    HPRECONDITION(pi_rpTrueRasterFile != 0);
    HPRECONDITION(pi_rpTrueRasterFile->IsCompatibleWith(HRFGeoTiffFile::CLASS_ID) ||
                  pi_rpTrueRasterFile->IsCompatibleWith(HRFiTiffFile::CLASS_ID));

    WString ListGeoKey;
    if (((HFCPtr<HRFTiffFile>&)pi_rpTrueRasterFile)->ConvertGeoKeysToString(&ListGeoKey))
        {
        for (size_t i = 0; i < ListGeoKey.size(); i++)
            {
            if (ListGeoKey[i] == _TEXT(','))
                ListGeoKey[i] = _TEXT('\n');
            }

        CString ErrorMessage((CString) _TEXT("Info : GeoKeys : ") +
                             (CString) pi_pFileName +
                             (CString) _TEXT("\n") +
                             (CString) ListGeoKey.c_str());
        AfxMessageBox(ErrorMessage);
        }
    else
        AfxMessageBox(_TEXT("No Geokeys"));

    if (HRFIsGeoCoded(pi_rpTrueRasterFile))
        {
        WString OriGeoKeyStr;
        WString NewGeoKeyStr;

        HRFGetStringGeoKeys(pi_rpTrueRasterFile, OriGeoKeyStr);
        // AfxMessageBox(GeoKeyStr.c_str());

        uint32_t Test = HRFSetStringGeoKeys(pi_rpTrueRasterFile, OriGeoKeyStr);
        // HASSERT(Test > 0);

        HRFGetStringGeoKeys(pi_rpTrueRasterFile, NewGeoKeyStr);

        WString TestDefault;
        HRFGenerateDefaulPCSGeokeys(TestDefault, 26767);

        unsigned short MyValue;
        HRFGetGeoKey(OriGeoKeyStr, 1024, MyValue);

        if (OriGeoKeyStr != NewGeoKeyStr)
            AfxMessageBox(_TEXT("Geokeys not equal!!!"));
        }
    else
        AfxMessageBox(_TEXT("IT'S NOT GEOCODED!"));
    }

#endif

void CActiveImageDoc::OnOptionsLoadInMemory(UINT nID)
    {
    if (m_LoadInMemoryID != nID)
        {
        m_LoadInMemoryID = nID;

        if (m_LoadInMemoryID != ID_OPTIONS_LOADINMEMORY_DISABLED)
            m_pLog->ChangeLimit(0);
        else
            m_pLog->ChangeLimit(theApp.GetObjectStore_Mem());
        }
    }

void CActiveImageDoc::OnUpdateOptionsLoadInMemory(CCmdUI *pCmdUI)
    {
    pCmdUI->SetCheck(pCmdUI->m_nID == m_LoadInMemoryID);
    }

void CActiveImageDoc::OnOptionsBestContrast()
    {
    m_BestContrast = !m_BestContrast;
    }

void CActiveImageDoc::On3band16bittorgb(UINT nID)
{
    if (m_3band16bittorgbID != nID)
        m_3band16bittorgbID = nID;
}

void CActiveImageDoc::OnUpdate3band16bittorgb(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck(pCmdUI->m_nID == m_3band16bittorgbID);

}

void CActiveImageDoc::On3band16bittorgbUsedsamevalue()
{
    m_3band16bitSameValue = !m_3band16bitSameValue;
}


void CActiveImageDoc::OnUpdate3band16bittorgbUsedsamevalue(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck(m_3band16bitSameValue);
}

void CActiveImageDoc::OnOptionsDisableCache()
    {
    m_DisableCache = !m_DisableCache;
    }

void CActiveImageDoc::OnUpdateCachedResolutions16384(CCmdUI *pCmdUI)
    {
    pCmdUI->SetCheck(m_Cache16384);
    }

void CActiveImageDoc::OnUpdateCachedResolutions8192(CCmdUI *pCmdUI)
    {
    pCmdUI->SetCheck(m_Cache8192);
    }

void CActiveImageDoc::OnUpdateCachedResolutions4096(CCmdUI *pCmdUI)
    {
    pCmdUI->SetCheck(m_Cache4096);
    }

void CActiveImageDoc::OnUpdateCachedResolutions2048(CCmdUI *pCmdUI)
    {
    pCmdUI->SetCheck(m_Cache2048);
    }

void CActiveImageDoc::OnUpdateCachedResolutions1024(CCmdUI *pCmdUI)
    {
    pCmdUI->SetCheck(m_Cache1024);
    }

void CActiveImageDoc::OnUpdateCachedResolutions512(CCmdUI *pCmdUI)
    {
    pCmdUI->SetCheck(m_Cache512);
    }

void CActiveImageDoc::OnUpdateCachedResolutions256(CCmdUI *pCmdUI)
    {
    pCmdUI->SetCheck(m_Cache256);
    }

void CActiveImageDoc::OnUpdateCachedResolutions128(CCmdUI *pCmdUI)
    {
    pCmdUI->SetCheck(m_Cache128);
    }

void CActiveImageDoc::OnUpdateCachedResolutionsNormalCache(CCmdUI *pCmdUI)
    {
    pCmdUI->SetCheck(m_NormalCache);
    }

void CActiveImageDoc::OnUpdateOptionsBestContrast(CCmdUI *pCmdUI)
    {
    pCmdUI->SetCheck(m_BestContrast);
    }

void CActiveImageDoc::OnUpdateOptionsDisableCache(CCmdUI *pCmdUI)
    {
    pCmdUI->SetCheck(m_DisableCache);

    if (pCmdUI->m_pMenu != 0)
        pCmdUI->m_pMenu->EnableMenuItem(ID_OPTIONS_CACHEDRESOLUTIONS, (m_DisableCache ? MF_GRAYED : MF_ENABLED));

    }

void CActiveImageDoc::OnCachedResolutions16384()
    {
    m_Cache16384 = !m_Cache16384;
    if (m_Cache16384)
        m_NormalCache = false;
    }

void CActiveImageDoc::OnCachedResolutions8192()
    {
    m_Cache8192 = !m_Cache8192;
    if (m_Cache8192)
        m_NormalCache = false;
    }

void CActiveImageDoc::OnCachedResolutions4096()
    {
    m_Cache4096 = !m_Cache4096;
    if (m_Cache4096)
        m_NormalCache = false;
    }

void CActiveImageDoc::OnCachedResolutions2048()
    {
    m_Cache2048 = !m_Cache2048;
    if (m_Cache2048)
        m_NormalCache = false;
    }

void CActiveImageDoc::OnCachedResolutions1024()
    {
    m_Cache1024 = !m_Cache1024;
    if (m_Cache1024)
        m_NormalCache = false;
    }

void CActiveImageDoc::OnCachedResolutions512()
    {
    m_Cache512 = !m_Cache512;
    if (m_Cache512)
        m_NormalCache = false;
    }

void CActiveImageDoc::OnCachedResolutions256()
    {
    m_Cache256 = !m_Cache256;
    if (m_Cache256)
        m_NormalCache = false;
    }

void CActiveImageDoc::OnCachedResolutions128()
    {
    m_Cache128 = !m_Cache128;
    if (m_Cache128)
        m_NormalCache = false;
    }

void CActiveImageDoc::OnCachedResolutionsNormalCache()
    {
    m_NormalCache = !m_NormalCache;
    if (m_NormalCache)
        {
        m_Cache16384 = false;
        m_Cache8192 = false;
        m_Cache4096 = false;
        m_Cache2048 = false;
        m_Cache1024 = false;
        m_Cache512 = false;
        m_Cache256 = false;
        m_Cache128 = false;
        }
    }


//-----------------------------------------------------------------------------
// public
// ExportBlockAccessListener
//-----------------------------------------------------------------------------
ExportBlockAccessListener::ExportBlockAccessListener()
    : m_Status(H_SUCCESS)
    {
    HRSBlockAccessIndicator::GetInstance()->AddListener(this);
    };

//-----------------------------------------------------------------------------
// public
// ~ExportBlockAccessListener
//-----------------------------------------------------------------------------
ExportBlockAccessListener::~ExportBlockAccessListener()
    {
    HRSBlockAccessIndicator::GetInstance()->RemoveListener(this);
    }

//-----------------------------------------------------------------------------
// public
// OnReadBlockError
//-----------------------------------------------------------------------------
void ExportBlockAccessListener::OnReadBlockError(HSTATUS pi_Error, HFCPtr<HFCURL> const& pi_pURL)
    {
    assert(pi_Error != H_SUCCESS);

    WString urlW(pi_pURL->GetURL().c_str(), BentleyCharEncoding::Utf8);

    WChar Msg[2048];
    _stprintf(Msg, _TEXT("OnReadBlockError  File: %s \n"), urlW.c_str());
    AfxMessageBox(Msg);

    }

//-----------------------------------------------------------------------------
// public
// OnWriteBlockError
//-----------------------------------------------------------------------------
void ExportBlockAccessListener::OnWriteBlockError(HSTATUS pi_Error, HFCPtr<HFCURL> const& pi_pURL)
    {
    assert(pi_Error != H_SUCCESS);
    m_Status = pi_Error;

    HUTExportProgressIndicator::GetInstance()->StopIteration();

    WString urlW(pi_pURL->GetURL().c_str(), BentleyCharEncoding::Utf8);

    WChar Msg[2048];
    _stprintf(Msg, _TEXT("OnWriteBlockError  File: %s \n"), urlW.c_str());
    AfxMessageBox(Msg);
    }

//-----------------------------------------------------------------------------
// public
// OnWriteBlockError
//-----------------------------------------------------------------------------
HSTATUS ExportBlockAccessListener::GetExportStatus()
    {
    return m_Status;
    }
//-----------------------------------------------------------------------------
// public
// SelectedObjectChanged - Called by the frame when the tree selector selection
// has changed
//-----------------------------------------------------------------------------
void CActiveImageDoc::SelectedObjectChanged()
    {
    UpdateAllViews(NULL, UPDATE_REDRAW);
    }


//-----------------------------------------------------------------------------
// public
// GetSelectedObject - Returns the item selected in the tree controls
//-----------------------------------------------------------------------------
HTREEITEM CActiveImageDoc::GetSelectedObject() const
    {
    CTreeCtrl* pTree = GetTreeCtrl();

    // Verify if the tree control is valid
    if (pTree == 0)
        return NULL;

    return (pTree->GetSelectedItem());
    }


//-----------------------------------------------------------------------------
// public
// GetDocumentItem - Returns the document item in the tree control
//-----------------------------------------------------------------------------
HTREEITEM CActiveImageDoc::GetDocumentObject() const
    {
    return (m_DocItem);
    }


//-----------------------------------------------------------------------------
// public
// GetClusterWorld - 
//-----------------------------------------------------------------------------
const HFCPtr<HGF2DWorldCluster> CActiveImageDoc::GetWorldCluster() const
    {
    return (HFCPtr<HGF2DWorldCluster>&)m_pWorldCluster;
    }


//-----------------------------------------------------------------------------
// public
// GetPool - 
//-----------------------------------------------------------------------------
HPMPool* CActiveImageDoc::GetPool() const
    {
    return m_pLog.get();
    }


//-----------------------------------------------------------------------------
// public
// GetRaster - Returns the raster associated with a tree item
//-----------------------------------------------------------------------------
HFCPtr<HRARaster> CActiveImageDoc::GetRaster(HTREEITEM pi_TreeItem) const
    {
    RasterMap::const_iterator Iterator;
    HFCPtr<HRARaster>   pRaster;
    HPRECONDITION(pi_TreeItem != NULL);
    HPRECONDITION(pi_TreeItem != GetDocumentObject());

    // Find the item in the map
    Iterator = m_RasterMap.find(pi_TreeItem);

    // verify if the item was found
    if (Iterator != m_RasterMap.end())
        pRaster = (*Iterator).second;

    return (pRaster);
    }


//-----------------------------------------------------------------------------
// Private
// 
//-----------------------------------------------------------------------------
CTreeCtrl* CActiveImageDoc::GetTreeCtrl() const
    {
    CWnd* pMainWnd = AfxGetMainWnd();
    CActiveImageFrame* pFrame;
    CTreeCtrl*         pTree;

    // get the main frame.  If it is invalid, return NULL
    pFrame = dynamic_cast<CActiveImageFrame*>(pMainWnd);
    if (pFrame == 0)
        return 0;

    // get the tree control.  Same as above
    pTree = pFrame->GetSelector();
    if (pTree == 0)
        return 0;

    // verify that the control is created
    if (!::IsWindow(pTree->m_hWnd))
        return 0;

    return pTree;
    }


//-----------------------------------------------------------------------------
// Protected
// 
//-----------------------------------------------------------------------------
void CActiveImageDoc::OnUpdateInsertFromExternalDocument(CCmdUI* pCmdUI)
    {
    bool bResult = false;

    // if the current item is the document item
    HTREEITEM  SelObject = GetSelectedObject();
    if (SelObject != m_DocItem)
        {
        // find the raster item in the map
        RasterMap::iterator itr = m_RasterMap.find(SelObject);
        if (itr != m_RasterMap.end())
            {
            // verify if the raster is a mosaic or not
            if ((*itr).second->GetClassID() == HIMMosaic::CLASS_ID)
                bResult = true;
            }
        }
    else
        bResult = true;

    pCmdUI->Enable(bResult);
    }


//-----------------------------------------------------------------------------
// Protected
// 
//-----------------------------------------------------------------------------
void CActiveImageDoc::OnUpdateInsertNewObject(CCmdUI* pCmdUI)
    {
    OnUpdateInsertFromExternalDocument(pCmdUI);
    }


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
void CActiveImageDoc::OnUpdateInsertFromthisdocument(CCmdUI* pCmdUI)
    {
    OnUpdateInsertFromExternalDocument(pCmdUI);
    }


//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
bool CActiveImageDoc::IsReadOnly() const
    {
    //An CActiveImageDoc shouldn't be created during an export
    HASSERT(m_FileAccessMode.m_HasCreateAccess == false);

    return !m_FileAccessMode.m_HasWriteAccess;
    }

//-----------------------------------------------------------------------------
// Public
// 
//-----------------------------------------------------------------------------
bool CActiveImageDoc::FitImageOnView() const
    {
    return m_FitImageOnView;
    }


void CActiveImageDoc::OnInfoTextlog()
    {
    if (m_pTextLogDlg == nullptr)
        m_pTextLogDlg = new TextLog(AfxGetMainWnd());

    m_pTextLogDlg->ShowWindow(SW_SHOWNORMAL);
    }

void CActiveImageDoc::OnReproject()
    {
    if (m_pReprojectDlg == nullptr)
        m_pReprojectDlg = new ReprojectDialog(AfxGetMainWnd());
    
    m_pReprojectDlg->DoModal();
    UpdateWkt();
    m_pReprojectDlg->ShowWindow(SW_SHOWNORMAL);
    UpdateAllViews(0, UPDATE_FIRST, 0);
    }

void CActiveImageDoc::AddStringToLog(const CString& pi_String)
    {
    if (m_pTextLogDlg == nullptr)
        {
        // Text Log Dialog
        m_pTextLogDlg = new TextLog(AfxGetMainWnd());
        m_pTextLogDlg->ShowWindow(SW_HIDE);
        }

    m_pTextLogDlg->AddString(pi_String);
    }


void CActiveImageDoc::CloseAllFiles()
    {
    // Clear the raster map
    // IMPORTANT: Instead of calling clear, we while delete each entry 1by1.
    // The clear method somehow locks a static critical section.  When destroying
    // a raster that is IIP, we get to the point where another thread tries to
    // enter the same critical section, thus resulting in a deadlock.
    while (m_RasterMap.size() > 0)
        {
        HFCPtr<HRARaster> pRaster;

        // Remove the entry from the map.
        {
        // get the first element
        RasterMap::iterator Itr = m_RasterMap.begin();

        // extract the raster from this element
        pRaster = (*Itr).second;

        // remove the element from the map
        m_RasterMap.erase(Itr);
        }

        // Release the raster, whitout fear of a deadlock
        pRaster = 0;
        }
    while (m_RasterToClone.size() > 0)
        {
        // just to keep a reference while the map is being 
        // remove from the current element.
        HFCPtr<HRARaster> pRaster;

        // Remove the entry from the map.
        {
        // get the first element from the list
        RasterList::iterator Itr = m_RasterToClone.begin();

        // extract the raster from the element
        pRaster = *Itr;

        // remove the element from the list
        m_RasterToClone.erase(Itr);
        }

        // Release the raster, whitout fear of a deadlock
        pRaster = 0;
        }

    // continue if the frame window is valid
    CTreeCtrl* pTree = GetTreeCtrl();
    if (pTree != 0)
        {
        // Delete the content of the tree
        pTree->SelectItem(NULL);
        pTree->DeleteAllItems();
        m_DocItem = pTree->InsertItem(_TEXT("Document"), TLI_DOCUMENT, TLI_DOCUMENT + 1);
        HASSERT(m_DocItem != NULL);
        pTree->SelectItem(m_DocItem);
        }
    }

void CActiveImageDoc::OnCloseDocument()
    {
    CloseAllFiles();
    CDocument::OnCloseDocument();
    }

void CActiveImageDoc::OnInfoImageinfo()
    {
    if (m_pImageInfoDlg == NULL)
        m_pImageInfoDlg = new ImageInfo(AfxGetMainWnd());

    m_pImageInfoDlg->SetImageInfo(); //Updates info whenever it's displayed
    m_pImageInfoDlg->ShowWindow(SW_SHOWNORMAL);
    }

HFCPtr<HGF2DTransfoModel> CActiveImageDoc::GetReprojectionModel()
    {
    return m_ReprojectionModel;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                          Laurent Robert-Veillette 02/2016
// Construct a polynomial model.
//+---------------+---------------+---------------+---------------+---------------+------
HFCPtr<HGFPolynomialModelAdapter> CActiveImageDoc::ComputePolynomialModel(const HGF2DTransfoModel& pExactModel,
                                                                          const HGF2DLiteExtent& pRectangle, double DimensionMeanPixel)
    {
    HGF2DRectangle shapeFromExtent(pRectangle);
    HFCPtr<HGFPolynomialModelAdapter> polyModel = new HGFPolynomialModelAdapter(pExactModel, shapeFromExtent, pRectangle.GetWidth() / 15,
                                                                                pRectangle.GetHeight() / 15, true);
    StatusInt status;

    if (!polyModel->HasEnoughTiePoints())
        status = ERROR;

    //Compute error with polynomial model
    double MeanError, MaxError, minErrorInSourceUnits;
    HGF2DPosition maxErrorPosition, minErrorPosition;
    status = polyModel->GetMeanError(&MeanError, &MaxError, &maxErrorPosition,
                                     &minErrorInSourceUnits, &minErrorPosition);

    //Prompt the info for the user
    m_stringInfoReprojection.Format(L"Polynomial approximation :\r\nMean Error = %g pixel\r\nMax Error = %g pixel\r\nError Status = %d",
                                    MeanError / DimensionMeanPixel, MaxError / DimensionMeanPixel, status);
    return polyModel;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                          Laurent Robert-Veillette 02/2016
// Construct a projective model.
//+---------------+---------------+---------------+---------------+---------------+------
HFCPtr<HGF2DProjective> CActiveImageDoc::ComputeProjectiveModel(const HGF2DTransfoModel& pExactModel,
                                                                        const HGF2DLiteExtent& pRectangle, double DimensionMeanPixel)
    {
    double minDimension = pRectangle.GetWidth() < pRectangle.GetHeight() ?
        pRectangle.GetWidth() : pRectangle.GetHeight();
    HFCPtr<HGF2DLinearModelAdapter> linearModel = new HGF2DLinearModelAdapter(pExactModel, pRectangle, minDimension / 100);

    //Test the error generated by the affine/projective model.
    double MeanError, MaxError;
    StatusInt status = SUCCESS;

    //Test the error generated by the affine/projective model. Do not return StatusInt. We cannot know if study was successful
    linearModel->StudyPrecisionOver(pRectangle, minDimension / 200, &MeanError, &MaxError);

    //Prompt the info for the user
    m_stringInfoReprojection.Format(L"Projective approximation :\r\nMean Error = %g pixel\r\nMax Error = %g pixel\r\nError Status = %d",
                                    MeanError / DimensionMeanPixel, MaxError / DimensionMeanPixel, status);
    return new HGF2DProjective(linearModel->GetMatrix());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                          Laurent Robert-Veillette 02/2016
// Updates the reprojection model following a GCS given in well-known-text and a selectioned model.
// Return the transfoModel constructed.
//+---------------+---------------+---------------+---------------+---------------+------
HFCPtr<HGF2DTransfoModel> CActiveImageDoc::UpdateReprojectionModel()
    {
    HTREEITEM  SelObject = GetSelectedObject();
    if (SelObject == nullptr || SelObject == GetDocumentObject())
        return m_ReprojectionModel;

    HFCPtr<HRARaster> selectedRaster = GetRaster(SelObject);
    if (selectedRaster->GetStore() == NULL || !selectedRaster->GetStore()->IsCompatibleWith(HRSObjectStore::CLASS_ID))
        return m_ReprojectionModel;

    HFCPtr<HRFRasterFile > pRasterFile = static_cast<HRSObjectStore*>(selectedRaster->GetStore())->GetRasterFile();

    if (m_pInfoReprojectionDlg == NULL)
        m_pInfoReprojectionDlg = new DlgInfoReprojection(AfxGetMainWnd());

    UpdateWkt();

    if (m_GcsPtrFromWellKnownText != nullptr)
        {
        GeoCoordinates::BaseGCSCPtr pGcsSrc = pRasterFile->GetPageDescriptor(0)->GetGeocodingCP(); //current raster GCS

        if (pGcsSrc != NULL && pGcsSrc->IsValid() && m_GcsPtrFromWellKnownText->IsValid())
            {
            if (pGcsSrc->IsEquivalent(*m_GcsPtrFromWellKnownText))
                {
                m_ReprojectionModel = new HGF2DIdentity();
                m_stringInfoReprojection.Format(L"Same source and destination GCS.\r\nNo reprojection.");
                UpdateReprojectionInfoDlg();
                return m_ReprojectionModel;
                }

            HGF2DExtent pixelMinimumSize, pixelMaximumSize;
            HGF2DLiteExtent domainDest;
            double minX, minY, maxX, maxY;
            HGF2DExtent extent;
            HFCPtr<HGF2DTransfoModel> pExactModel = new HCPGCoordModel(*m_GcsPtrFromWellKnownText, *pGcsSrc);
            double DimensionMeanPixel;

            if (m_reprojectionType != ID_REPROJECTIONMODEL_EXACTREPROJECTION)
                {            
                extent = selectedRaster->GetExtent();
                extent.ChangeCoordSys(g_pAIRefCoordSys);

                //Put the extent in the direct GCS by passing through the exact model
                pExactModel->ConvertInverse(extent.GetXMin(), extent.GetYMin(), &minX, &minY);
                pExactModel->ConvertInverse(extent.GetXMax(), extent.GetYMax(), &maxX, &maxY);
                domainDest = HGF2DLiteExtent(minX, minY, maxX, maxY);

                //Get the pixel size                
                selectedRaster->GetPixelSizeRange(pixelMinimumSize, pixelMaximumSize);

                pixelMaximumSize.ChangeCoordSys(g_pAIRefCoordSys);
                pixelMinimumSize.ChangeCoordSys(g_pAIRefCoordSys);

                DimensionMeanPixel = (pixelMinimumSize.GetWidth() + pixelMinimumSize.GetHeight() +
                                      pixelMaximumSize.GetWidth() + pixelMaximumSize.GetHeight()) / 4;
                }

            switch (m_reprojectionType) //N.B. brackets in cases are mandatory because we declare variables within the cases.
                {
                case ID_REPROJECTIONMODEL_EXACTREPROJECTION:
                    {
                    m_stringInfoReprojection.Format(L"Exact transformation used.\r\nNo error on reprojection");
                    m_ReprojectionModel = pExactModel;
                    break;
                    }
                case ID_REPROJECTIONMODEL_AFFINE:
                    {
                    HFCPtr<HGF2DProjective> projectiveModel = ComputeProjectiveModel(*pExactModel, domainDest, DimensionMeanPixel);
                    m_ReprojectionModel = projectiveModel;
                    break;
                    }
                case ID_REPROJECTIONMODEL_POLYNOMIALREPROJECTION:
                    {
                    HFCPtr<HGFPolynomialModelAdapter> polyModel = ComputePolynomialModel(*pExactModel, domainDest, DimensionMeanPixel);
                    m_ReprojectionModel = polyModel;
                    break;
                    }
                case ID_REPROJECTIONMODEL_BESTMODEL:
                    {
                    double  step = MIN(domainDest.GetHeight(), domainDest.GetWidth()) / 100;
                    double MeanError, MaxError;
                    HGF2DLiteExtent domainSource(extent.GetXMin(), extent.GetYMin(), extent.GetXMax(), extent.GetYMax());
                    HFCPtr<HGF2DTransfoModel> pApproxModel = HCPGCoordUtility::CreateGCoordBestAdaptedModel(*pGcsSrc,
                                                                                                            *m_GcsPtrFromWellKnownText,
                                                                                                            domainSource,
                                                                                                            step,
                                                                                                            DimensionMeanPixel,
                                                                                                            DimensionMeanPixel,
                                                                                                            &MeanError,
                                                                                                            &MaxError);
                    //HFCPtr<HGF2DTransfoModel> pApproxModel = ChooseModelWithPrecision(*pExactModel, domainDest, DimensionMeanPixel);
                    if (pApproxModel)
                        {
                        m_stringInfoReprojection.Format(L"Best Model Approximation :\r\nMean Error = %g pixel\r\nMax Error = %g pixel",
                                                        MeanError / DimensionMeanPixel, MaxError / DimensionMeanPixel);
                        pApproxModel->Reverse(); // We must reverse because the model is constructed in the opposite of our desired direction in HCPGCoordUtility
                        m_ReprojectionModel = pApproxModel;
                        }
                    else
                        {
                        AfxMessageBox(_TEXT("The approximation models do not meet precision standard. The raster will be reprojected using the exact model."));
                        m_stringInfoReprojection.Format(L"Exact transformation used.\r\n No error on reprojection");
                        m_ReprojectionModel = pExactModel;
                        }
                    break;
                    }
                default:
                        BeAssert(!"What kind of reprojection type is that?");
                        break;
                }
            }
        else
            {
            m_ReprojectionModel = new HGF2DIdentity();
            m_stringInfoReprojection = "No reprojection.";
            UpdateReprojectionInfoDlg();

            if (pGcsSrc == NULL || !pGcsSrc->IsValid())
                AfxMessageBox(_TEXT("The source GCS is invalid. The raster will not be reprojected."));
            else
                AfxMessageBox(_TEXT("The destination GCS is invalid. The raster will not be reprojected."));
            }
        }
    else
        {
        m_ReprojectionModel = new HGF2DIdentity();

        m_stringInfoReprojection = "No reprojection.";
        UpdateReprojectionInfoDlg();
        }

    //Update the non-modal window displaying the reprojection info.
    UpdateReprojectionInfoDlg();

    return m_ReprojectionModel;
    }

void CActiveImageDoc::UpdateWkt()
    {
    if (m_pReprojectDlg != nullptr)
        {
        CString wkt = m_pReprojectDlg->GetWkt();
        AddStringToLog(wkt);
        if (!wkt.IsEmpty())
            {
            m_GcsPtrFromWellKnownText = GeoCoordinates::BaseGCS::CreateGCS(); //Well-known text information GCS
            m_GcsPtrFromWellKnownText->InitFromWellKnownText(nullptr, nullptr, GeoCoordinates::BaseGCS::wktFlavorESRI, wkt);
            }
        else
            m_GcsPtrFromWellKnownText = nullptr;
        }
    else
        m_GcsPtrFromWellKnownText = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                          Laurent Robert-Veillette 02/2016
// return the projective or polynomial model if the error is below 
// the minimum pixel size. Otherwise, we return 0.
// If the projective or polynomial model is returned, the string information in the projection
// information dialog window is updated.
//+---------------+---------------+---------------+---------------+---------------+------
HFCPtr<HGF2DTransfoModel> CActiveImageDoc::ChooseModelWithPrecision(const HGF2DTransfoModel& pExactModel,
                                                                    const HGF2DLiteExtent& pRectangle, double DimensionMeanPixel)
    {
    //Test the AFFINE AND PROJECTIVE models
    double minDimension = pRectangle.GetWidth() < pRectangle.GetHeight() ?
        pRectangle.GetWidth() : pRectangle.GetHeight();
    HFCPtr<HGF2DLinearModelAdapter> linearModel = new HGF2DLinearModelAdapter(pExactModel, pRectangle, minDimension / 100);

    //Test the error generated by the affine/projective model. Do not return StatusInt. We cannot know if study was successful
    double MeanError, MaxError;
    linearModel->StudyPrecisionOver(pRectangle, minDimension / 400, &MeanError, &MaxError);

    StatusInt status = SUCCESS;

    if (MaxError < DimensionMeanPixel)
        {
        //Prompt the info for the user
        m_stringInfoReprojection.Format(L"Projective approximation :\r\nMean Error = %g pixel\r\nMax Error = %g pixel\r\nError Status = %d",
                                        MeanError / DimensionMeanPixel, MaxError / DimensionMeanPixel, status);
        return new HGF2DProjective(linearModel->GetMatrix());
        }


    //If affine/projective model do not fit, test the POLYNOMIAL model
    HGF2DRectangle shapeFromExtent(pRectangle);
    HFCPtr<HGFPolynomialModelAdapter> polyModel = new HGFPolynomialModelAdapter(pExactModel, shapeFromExtent, pRectangle.GetWidth() / 15,
                                                                                pRectangle.GetHeight() / 15, true);

    if (!polyModel->HasEnoughTiePoints())
        return nullptr;

    // Study the error for the polynomial model
    double minErrorInSourceUnits;
    HGF2DPosition maxErrorPosition, minErrorPosition;
    status = polyModel->GetMeanError(&MeanError, &MaxError, &maxErrorPosition,
                                     &minErrorInSourceUnits, &minErrorPosition);

    if (status == SUCCESS && MaxError < DimensionMeanPixel)
        {
        //Prompt the info for the user
        m_stringInfoReprojection.Format(L"Polynomial approximation :\r\nMean Error = %g pixel\r\nMax Error = %g pixel\r\nError Status = %d",
                                        MeanError / DimensionMeanPixel, MaxError / DimensionMeanPixel, status);
        return polyModel->Clone();
        }
    else
        return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                          Laurent Robert-Veillette 02/2016
//+---------------+---------------+---------------+---------------+---------------+------
void CActiveImageDoc::UpdateReprojectionInfoDlg()
    {
    CWnd *label = m_pInfoReprojectionDlg->GetDlgItem(IDC_STATIC_TEXT_REPRO_INFO_DLG);
    label->SetWindowText(m_stringInfoReprojection);

    if (IsRasterSelected() && m_GcsPtrFromWellKnownText != nullptr)
        m_pInfoReprojectionDlg->ShowWindow(SW_SHOWNORMAL);
    else
        m_pInfoReprojectionDlg->EndDialog(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                          Laurent Robert-Veillette 02/2016
// Updates the reprojection model following a GCS given in well-known-text and a selectioned model.
//+---------------+---------------+---------------+---------------+---------------+------
void CActiveImageDoc::OnReprojectionModelUpdate(UINT nID)
    {
    if (m_reprojectionType != nID)
        m_reprojectionType = nID;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                          Laurent Robert-Veillette 02/2016
// Checks on unchecks reprojection options in the menu.
//+---------------+---------------+---------------+---------------+---------------+------
void CActiveImageDoc::OnReprojectionModelUpdateCheckBox(CCmdUI* pCmdUI)
    {
    pCmdUI->SetCheck(pCmdUI->m_nID == m_reprojectionType);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                          Laurent Robert-Veillette 02/2016
// Display the reprojection information dialog box when the option is clicked in the menu.
//+---------------+---------------+---------------+---------------+---------------+------
void CActiveImageDoc::OnInfoReprojection()
    {
    if (m_pInfoReprojectionDlg == NULL)
        m_pInfoReprojectionDlg = new DlgInfoReprojection(AfxGetMainWnd());

    m_pInfoReprojectionDlg->ShowWindow(SW_SHOWNORMAL);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                          Laurent Robert-Veillette 02/2016
// return True if the selectioned item is a raster and false if it is a mosaic or the document item.
//+---------------+---------------+---------------+---------------+---------------+------

bool CActiveImageDoc::IsRasterSelected() const
    {
    HTREEITEM  SelObject = GetSelectedObject();

    if (SelObject == nullptr || SelObject == GetDocumentObject())
        return false;

    HFCPtr<HRARaster> selectedRaster = GetRaster(SelObject);
    if (selectedRaster->GetStore() == NULL || !selectedRaster->GetStore()->IsCompatibleWith(HRSObjectStore::CLASS_ID))
        return false;
    else
        return true;
    }







