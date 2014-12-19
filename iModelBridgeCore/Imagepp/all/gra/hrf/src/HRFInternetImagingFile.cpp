//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFInternetImagingFile.cpp $
//:>
//:> Copyright (c) 2003;  Bentley Systems, Inc., 685 Stockton Drive,
//:>                      Exton PA, 19341-0678, USA.  All Rights Reserved.
//:>
//:> This program is confidential, proprietary and unpublished property of Bentley Systems
//:> Inc. It may NOT be copied in part or in whole on any medium, either electronic or
//:> printed, without the express written consent of Bentley Systems, Inc.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFInternetImagingFile
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

// Precompiled Header
#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

// HRFInternet classes
#include <Imagepp/all/h/HRFInternetImagingFile.h>
#include <Imagepp/all/h/HRFInternetImagingTileEditor.h>
#include <Imagepp/all/h/HRFInternetImagingThread.h>
#include <Imagepp/all/h/HRFInternetImaging.h>
#include <Imagepp/all/h/HRFHTTPConnection.h>
#include <Imagepp/all/h/HRFSocketConnection.h>

// Common Handlers
#include <Imagepp/all/h/HRFInternetTileHandler.h>
#include <Imagepp/all/h/HRFInternetErrorHandler.h>
#include <Imagepp/all/h/HRFInternetROIHandler.h>
#include <Imagepp/all/h/HRFInternetResolutionHandler.h>
#include <Imagepp/all/h/HRFInternetCompGroupHandler.h>
#include <Imagepp/all/h/HRFInternetTransformHandler.h>

// IIP Handlers
#include <Imagepp/all/h/HRFInternetColorspaceHandler.h>
#include <Imagepp/all/h/HRFInternetMaxSizeHandler.h>
#include <Imagepp/all/h/HRFInternetIIPVersionHandler.h>

// HIP Handlers
#include <Imagepp/all/h/HRFInternetHIPVersionHandler.h>
#include <Imagepp/all/h/HRFInternetHIPTileSizeHandler.h>
#include <Imagepp/all/h/HRFInternetHIPPixelTypeHandler.h>
#include <Imagepp/all/h/HRFInternetHIPPaletteHandler.h>
#include <Imagepp/all/h/HRFInternetHIPImageSizeHandler.h>
#include <Imagepp/all/h/HRFInternetHIPFileInfoHandler.h>
#include <Imagepp/all/h/HRFInternetHIPWorldIDHandler.h>
#include <Imagepp/all/h/HRFInternetHIPLogicalShapeHandler.h>
#include <Imagepp/all/h/HRFInternetHIPFileHandler.h>
#include <Imagepp/all/h/HRFInternetHIPRedirectHandler.h>
#include <Imagepp/all/h/HRFInternetHIPFileCompressionHandler.h>
#include <Imagepp/all/h/HRFInternetHIPHistogramHandler.h>
#include <Imagepp/all/h/HRFInternetHIPBackgroundHandler.h>
#include <Imagepp/all/h/HRFInternetHIPFileTypeHandler.h>
#include <Imagepp/all/h/HRFInternetAttributesHandler.h>
#include <Imagepp/all/h/HRFInternetAttributeNamesHandler.h>
#include <Imagepp/all/h/HRFInternetHIPTileHandler.h>
#include <Imagepp/all/h/HRFInternetHIPPageNumberHandler.h>

// Codec Objects
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDCodecFlashpix.h>
#include <Imagepp/all/h/HCDCodecFPXSingleColor.h>
#include <Imagepp/all/h/HCDCodecZlib.h>
#include <Imagepp/all/h/HCDCodecHMRRLE1.h>
#include <Imagepp/all/h/HCDCodecHMRCCITT.h>
#include <Imagepp/all/h/HCDCodecHMRPackBits.h>
#include <Imagepp/all/h/HCDCodecJPEGAlpha.h>

// Others

#include <Imagepp/all/h/HFCStat.h>
#include <Imagepp/all/h/HRFMessages.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HRFURLInternetImagingHTTP.h>
#include <Imagepp/all/h/HFCURLInternetImagingSocket.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HRFInternetImagingException.h>
#include <Imagepp/all/h/HRPListPixelTypePtrs.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DProjective.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HFCEncodeDecodeASCII.h>
#include <Imagepp/all/h/HVETileIDIterator.h>
#include <Imagepp/all/h/HFCURLHTTPS.h>

#include <Imagepp/all/h/HFCResourceLoader.h>
#include <Imagepp/all/h/ImagePPMessages.xliff.h>

USING_NAMESPACE_IMAGEPP

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

// Creators
HFC_IMPLEMENT_SINGLETON(HRFInternetImagingFileCreator)
#if 0
/*
   #TR 250337, Raymond Gauthier, 23/07/2008:
   Merged the 3 Creators in in single one (HRFInternetImagingFileCreator) so that
   there is only one Creator associated with a class ID.
*/
HFC_IMPLEMENT_SINGLETON(HRFInternetFileSocketCreator)
HFC_IMPLEMENT_SINGLETON(HRFInternetFileHTTPCreator)
HFC_IMPLEMENT_SINGLETON(HRFInternetFileHTTPSCreator)
#endif


//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

// Flag fot notification status
static const uint32_t s_NotifyFlag = 0x00000001;

// Standard size for string
static const size_t s_StringSize = 255;

// for the look ahead buffer
const size_t        g_LookAheadGrowSize   = 512;
const size_t        g_LookAheadMaxSize    = 2048;

// Request fro compression group
static const string s_CompGroupRequest("obj=comp-group,2,*");

#define IPP_PROTOCOL        0
#define HIP_PROTOCOL        1
#define HIP_UTF8_PROTOCOL   2

// Protocols
// Normally we would place the version flags as the fourth parameter, but we cannot
// assume that the static creation order will be valid so PLACE THE SAME NUMBER as
// the static InternetImagingFlags!
static const HFCVersion s_IIP0100     (WString(L""), WString(L""), 3, 1, 0, IPP_PROTOCOL);
static const HFCVersion s_HIP0100     (WString(L""), WString(L""), 3, 1, 0, HIP_PROTOCOL);
static const HFCVersion s_HIP0110     (WString(L""), WString(L""), 3, 1, 1, HIP_PROTOCOL);
static const HFCVersion s_HIP0120     (WString(L""), WString(L""), 3, 1, 2, HIP_PROTOCOL);
static const HFCVersion s_HIP0130     (WString(L""), WString(L""), 3, 1, 3, HIP_PROTOCOL);
static const HFCVersion s_HIP0130_UTF8(WString(L""), WString(L""), 3, 1, 3, HIP_UTF8_PROTOCOL);
static const HFCVersion s_HIP0140     (WString(L""), WString(L""), 3, 1, 4, HIP_PROTOCOL);
static const HFCVersion s_HIP0140_UTF8(WString(L""), WString(L""), 3, 1, 4, HIP_UTF8_PROTOCOL);
static const HFCVersion s_HIP0150     (WString(L""), WString(L""), 3, 1, 5, HIP_PROTOCOL);
static const HFCVersion s_HIP0150_UTF8(WString(L""), WString(L""), 3, 1, 5, HIP_UTF8_PROTOCOL);
static const HFCVersion s_HIP0160     (WString(L""), WString(L""), 3, 1, 6, HIP_PROTOCOL);
static const HFCVersion s_HIP0160_UTF8(WString(L""), WString(L""), 3, 1, 6, HIP_UTF8_PROTOCOL);



//-----------------------------------------------------------------------------
// Static Member Initialization
//-----------------------------------------------------------------------------

// Protocol version
const uint32_t  HRFInternetImagingFile::s_IIP_Protocol         = IPP_PROTOCOL;
const uint32_t  HRFInternetImagingFile::s_HIP_Protocol         = HIP_PROTOCOL;
const uint32_t  HRFInternetImagingFile::s_HIP_UTF8_Protocol    = HIP_UTF8_PROTOCOL;

// Progress Information
HFCExclusiveKey             HRFInternetImagingFile::s_ProgressKey;
uint32_t                    HRFInternetImagingFile::s_ProgressTotalTiles        = 0;
uint32_t                    HRFInternetImagingFile::s_ProgressCurrentTiles      = 0;

// Disconnector thread
static HFCExclusiveKey      s_DisconnectorThreadKey;
HFCPtr<HRFInternetDisconnectionThread>
HRFInternetImagingFile::s_pDisconnectorThread;

//-----------------------------------------------------------------------------
// class HRFHandlers, HRFIIPHandlers & HRFHIPHandlers
//-----------------------------------------------------------------------------
class HRFHandlers
    {
public:
    // Returns the list of handlers
    const HandlerList& GetHandlerList()
        {
        return (m_Handlers);
        }

protected:
    // Inserts the common handlers
    HRFHandlers::HRFHandlers()
        {
        // common handlers
        m_Handlers.push_back(new HRFInternetTileHandler);
        m_Handlers.push_back(new HRFInternetHIPTileHandler);
        m_Handlers.push_back(new HRFInternetErrorHandler);
        m_Handlers.push_back(new HRFInternetROIHandler);
        m_Handlers.push_back(new HRFInternetResolutionHandler);
        m_Handlers.push_back(new HRFInternetCompGroupHandler);
        m_Handlers.push_back(new HRFInternetTransformHandler);
        }

    HandlerList m_Handlers;
    };

class HRFIIPHandlers : public HRFHandlers
    {
public:
    // Insert IIP specific handlers
    HRFIIPHandlers::HRFIIPHandlers()
        : HRFHandlers()
        {
        m_Handlers.push_back(new HRFInternetIIPVersionHandler);
        m_Handlers.push_back(new HRFInternetColorspaceHandler);
        m_Handlers.push_back(new HRFInternetMaxSizeHandler);
        }
    } s_IIPHandlers;

class HRFHIPHandlers : public HRFHandlers
    {
public:
    // Insert IIP specific handlers
    HRFHIPHandlers::HRFHIPHandlers()
        : HRFHandlers()
        {
        // HIP Specific Handlers
        m_Handlers.push_back(new HRFInternetHIPVersionHandler);
        m_Handlers.push_back(new HRFInternetHIPTileSizeHandler);
        m_Handlers.push_back(new HRFInternetHIPPixelTypeHandler);
        m_Handlers.push_back(new HRFInternetHIPPaletteHandler);
        m_Handlers.push_back(new HRFInternetHIPImageSizeHandler);
        m_Handlers.push_back(new HRFInternetHIPFileInfoHandler);
        m_Handlers.push_back(new HRFInternetHIPWorldIDHandler);
        m_Handlers.push_back(new HRFInternetHIPLogicalShapeHandler);
        m_Handlers.push_back(new HRFInternetHIPFileHandler);
        m_Handlers.push_back(new HRFInternetHIPRedirectHandler);
        m_Handlers.push_back(new HRFInternetHIPFileCompressionHandler);
        m_Handlers.push_back(new HRFInternetHIPHistogramHandler);
        m_Handlers.push_back(new HRFInternetHIPBackgroundHandler);
        m_Handlers.push_back(new HRFInternetHIPFileTypeHandler);
        m_Handlers.push_back(new HRFInternetAttributesHandler);
        m_Handlers.push_back(new HRFInternetAttributeNamesHandler);
        m_Handlers.push_back(new HRFInternetHIPPageNumberHandler);
        }
    } s_HIPHandlers;


//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
HRFInternetImagingFile::HRFInternetImagingFile(const HFCPtr<HFCURL>& pi_rpURL,
                                               HFCAccessMode         pi_AccessMode,
                                               uint64_t             pi_Offset,
                                               const WString&        pi_rOpeningPrefix)
    : HRFRasterFile(pi_rpURL, pi_AccessMode, pi_Offset),
      m_State                   (DISCONNECTED),
      m_IsConstructing          (true),
      m_Redirected              (false),
      m_Cancelled               (false),
      m_RequestStopped          (false),
      m_StateEvent              (true, false),
      m_ThreadExceptionEvent    (true, false),
      m_HistogramEvent          (true, false),
      m_LocalCacheFileEvent     (true, false),
      m_RedirectEvent           (true, false),
      m_AttributesEvent         (true, false),
      m_PageDescriptorAvailable (true, false),
      m_MultiPageSupported      (false),
      m_PageCount               (0),
      m_PageID                  (0),
      m_WaitTileOnRead          (true)
    {
    HPRECONDITION(!pi_AccessMode.m_HasWriteAccess);
    HPRECONDITION(!pi_AccessMode.m_HasCreateAccess);
    HPRECONDITION(pi_Offset == 0);
    HPRECONDITION(HRFInternetImagingFileCreator::GetInstance()->IsKindOfFile(pi_rpURL));


#if 0
    /*
       #TR 250337, Raymond Gauthier, 23/07/2008:
       Merged the 3 Creators in in single one (HRFInternetImagingFileCreator) so that
       there is only one Creator associated with a class ID.
    */
    HPRECONDITION(HRFInternetFileHTTPCreator::GetInstance()->IsKindOfFile(pi_rpURL) ||
                  HRFInternetFileHTTPSCreator::GetInstance()->IsKindOfFile(pi_rpURL) ||
                  HRFInternetFileSocketCreator::GetInstance()->IsKindOfFile(pi_rpURL) );
#endif

    Constructor();

    }

//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HRFInternetImagingFile::~HRFInternetImagingFile()
    {
    // Stop the thread
    m_pThread = 0;

    // Clear thread exception
    ClearThreadException();

    // Disconnect the connection
    HFCMonitor Monitor(m_ConnectionKey);
    s_pDisconnectorThread->AddConnection(m_pConnection.release());

    // Stop the disconnector thread
        {
        HFCMonitor DisconnectorThreadKeyMonitor(s_DisconnectorThreadKey);
        m_pDisconnectorThread = 0;
        if (s_pDisconnectorThread->GetRefCount() == 1)
            s_pDisconnectorThread = 0;
        }

    // Clear the pending tiles
    HFCMonitor ProgressMonitor(s_ProgressKey);
    s_ProgressTotalTiles   -= m_ProgressCurrentTiles;
    m_ProgressCurrentTiles = 0;
    }

//-----------------------------------------------------------------------------
// Public
// Returns the world to which the image is attached
//-----------------------------------------------------------------------------
bool HRFInternetImagingFile::GetLocalCachedFile()
    {
    string FileToOpen;
    bool  Result = false;
    const uint32_t HIPFileSizeUnknown(0);

    // Only available with HIP.
    if (IsHMRImagingProtocol(m_Version))
        {
        // verify if the file size is lower that the threshold
        if ((m_FileSize != HIPFileSizeUnknown) && 
            (m_FileSize <= ImageppLib::GetHost().GetImageppLibAdmin()._GetInternetImagingHIPSmallFileThreshold()))
            {
            // Place a URL around the HIP file name
            HFCPtr<HFCURL> pURL = HFCURL::Instanciate(WString(HFCURLFile::s_SchemeName() + L"://") + m_LocalCachedFileName);
            HASSERT(pURL != 0);

            // If the file is not available or if the time stamp is differente,
            // ask the server for the file
            struct _stat LocalFileInfo;
            if ((_wstat(m_LocalCachedFileName.c_str(), &LocalFileInfo) != 0) ||
                (LocalFileInfo.st_mtime != m_ModifyTime))
                {
                // Reset the state event to unsignaled
                m_LocalCacheFileEvent.Reset();

                // request the file from the server
                try
                    {
                    string Request("obj=hip-file");
                    HFCMonitor ConnectionMonitor(m_ConnectionKey);
                    m_pConnection->Send((const Byte*)Request.data(), Request.size());
                    }
                catch(HFCInternetConnectionException&)
                    {
                    }

                // Wait for the file to arrive
                WaitForEvent(m_LocalCacheFileEvent);

                // Stop the thread since it is not needed any more
                m_pThread = 0;

                // Close the connection
                HFCMonitor ConnectionMonitor(m_ConnectionKey);
                m_pDisconnectorThread->AddConnection(m_pConnection.release());
                }

            // open the file
            m_pLocalCachedFile = HRFRasterFileFactory::GetInstance()->OpenFile(pURL);
            HASSERT(m_pLocalCachedFile != 0);
            }
        }

    // If the file was opened, copy its descriptors in this object
    if (Result = (m_pLocalCachedFile != 0))
        {
        // set our page descriptor to the cached file's
        for (uint32_t Page = 0; Page < m_pLocalCachedFile->CountPages(); Page++)
            m_ListOfPageDescriptor.push_back(m_pLocalCachedFile->GetPageDescriptor(Page));
        }

    return (Result);
    }



//-----------------------------------------------------------------------------
// Public
// Returns the world to which the image is attached
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFInternetImagingFile::GetCapabilities() const
    {
    if (HasLocalCachedFile())
        return m_pLocalCachedFile->GetCapabilities();
    else
        return HRFInternetImagingFileCreator::GetInstance()->GetCapabilities();
    }


//-----------------------------------------------------------------------------
// Public
// Returns the world to which the image is attached
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFInternetImagingFile::GetWorldIdentificator () const
    {
    HGF2DWorldIdentificator Result;

    if (HasLocalCachedFile())
        Result = m_pLocalCachedFile->GetWorldIdentificator();
    else
        {
        HFCMonitor Monitor(m_DataKey);
        Result = m_WorldID;
        }

    return (Result);
    }



//-----------------------------------------------------------------------------
// Public
// Impossible with IIP-HIP
//-----------------------------------------------------------------------------
bool HRFInternetImagingFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    return (false);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFInternetImagingFile::CreateResolutionEditor(uint32_t      pi_Page,
                                                                    unsigned short pi_ResIndex,
                                                                    HFCAccessMode pi_AccessMode)
    {
    HPRECONDITION(pi_Page < CountPages());
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(!GetPageDescriptor(pi_Page)->IsUnlimitedResolution());
    HPRECONDITION(pi_ResIndex < GetPageDescriptor(pi_Page)->CountResolutions());
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_ResIndex) != 0);
    HRFResolutionEditor* pResult = 0;

    if (HasLocalCachedFile())
        pResult = m_pLocalCachedFile->CreateResolutionEditor(pi_Page, pi_ResIndex, pi_AccessMode);
    else
        pResult = new HRFInternetImagingTileEditor(HFCPtr<HRFRasterFile>(this),
                                                   pi_Page,
                                                   pi_ResIndex,
                                                   pi_AccessMode,
                                                   m_WaitTileOnRead);

    return (pResult);
    }

//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFInternetImagingFile::CreateUnlimitedResolutionEditor(uint32_t      pi_Page,
                                                                             double       pi_Resolution,
                                                                             HFCAccessMode pi_AccessMode)
    {
    HPRECONDITION(pi_Page < CountPages());
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(GetPageDescriptor(pi_Page)->IsUnlimitedResolution());
    HPRECONDITION(!HasLocalCachedFile());

    HRFResolutionEditor* pResult = 0;

    pResult = new HRFInternetImagingTileEditor(HFCPtr<HRFRasterFile>(this),
                                               pi_Page,
                                               pi_Resolution,
                                               pi_AccessMode,
                                               m_WaitTileOnRead);

    return (pResult);
    }

//-----------------------------------------------------------------------------
// Public
// Saves the file
//-----------------------------------------------------------------------------
void HRFInternetImagingFile::Save()
    {
    HASSERT(!"HRFInternetImagingFile::Save():InternetImaging format is read only");
    }


//-----------------------------------------------------------------------------
// LookAhead methods
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public
// Indicates if the file supports LookAhead optimization
//-----------------------------------------------------------------------------
bool HRFInternetImagingFile::HasLookAheadByBlock(uint32_t pi_Page) const
    {
    // look a head is supported for multi page and unlimited resolution
    if (m_PageID != pi_Page)
        return static_cast<HRFRasterFile*>(GetPage(pi_Page).GetPtr())->HasLookAhead(pi_Page);
    else
        return !HasLocalCachedFile();
    }


//-----------------------------------------------------------------------------
// Public
// Only allow LookAhead if the connection state is CONNECTED
//-----------------------------------------------------------------------------
bool HRFInternetImagingFile::CanPerformLookAhead (uint32_t pi_Page) const
    {
    if (m_PageID != pi_Page)
        return static_cast<HRFRasterFile*>(GetPage(pi_Page).GetPtr())->CanPerformLookAhead(pi_Page);
    else
        return HRFRasterFile::CanPerformLookAhead(0);
    }



//-----------------------------------------------------------------------------
// Public
// StopLookAhead
//
// Stop stopPrefetch data in a given region
//-----------------------------------------------------------------------------
void HRFInternetImagingFile::StopLookAhead(uint32_t   pi_Page,
                                           uint32_t   pi_ConsumerID)
    {
    HPRECONDITION(HasLookAhead(pi_Page));

    if (m_PageID != pi_Page)
        static_cast<HRFRasterFile*>(GetPage(pi_Page).GetPtr())->StopLookAhead(pi_Page, pi_ConsumerID);
    else
        HRFRasterFile::StopLookAhead(0, pi_ConsumerID);
    }


//-----------------------------------------------------------------------------
// Public
// SetLookAhead
//
// Set the LookAhead for a specific resolution. For an unlimited resolution
// raster, pi_Resolution is the ResolutionIndex used by an
// HRFInternetImagingTileEditor.
//-----------------------------------------------------------------------------
void HRFInternetImagingFile::SetLookAhead(uint32_t          pi_Page,
                                          unsigned short   pi_Resolution,
                                          const HVEShape&   pi_rShape,
                                          uint32_t          pi_ConsumerID,
                                          bool             pi_Async)
    {
    HPRECONDITION(HasLookAhead(pi_Page));

    if (!m_IsUnlimitedResolution)
        HRFRasterFile::SetLookAhead(pi_Page,
                                    pi_Resolution,
                                    pi_rShape,
                                    pi_ConsumerID,
                                    pi_Async);
    else
        {
        // for an unlimited resolution, pi_Resolution is the resolution index used by an editor.
        // verify if the look ahead can be performed right now
        if (CanPerformLookAhead(pi_Page))
            {
            HGFTileIDList Blocks;
            if (!pi_rShape.IsEmpty())
                {
                // find the resolution editor into the ResolutionEditorRegistry
                ResolutionEditorRegistry::const_iterator Itr(m_ResolutionEditorRegistry.begin());
                const HRFResolutionEditor* pResEditor = 0;
                while (pResEditor == 0 && Itr != m_ResolutionEditorRegistry.end())
                    {
                    if ((*Itr)->GetResolutionIndex() == pi_Resolution)
                        pResEditor = *Itr;
                    else
                        Itr++;
                    }
                HASSERT(pResEditor != 0);

                // Extract the needed blocks from the region

                HFCPtr<HRFResolutionDescriptor> pResDesc = pResEditor->GetResolutionDescriptor();
                HGFTileIDDescriptor TileDesc(pResDesc->GetWidth(),
                                             pResDesc->GetHeight(),
                                             pResDesc->GetBlockWidth(),
                                             pResDesc->GetBlockHeight());

                // Make sure the input shape will not be destroyed
                HVEShape* pShape = const_cast<HVEShape*>(&pi_rShape);
                pShape->IncrementRef();
                    {
                    HVETileIDIterator TileIterator(&TileDesc, HFCPtr<HVEShape>(pShape));

                    uint64_t BlockIndex = TileIterator.GetFirstTileIndex();
                    while (BlockIndex != HGFTileIDDescriptor::INDEX_NOT_FOUND)
                        {
                        Blocks.push_back(s_TileDescriptor.ComputeIDFromIndex(BlockIndex, pi_Resolution));

                        BlockIndex = TileIterator.GetNextTileIndex();
                        }
                    }
                // Decrement after the block because the HVETileIDIterator keeps an
                // HFCPtr to the input shape, so we want the Iterator to be destroyed
                // before we decrement.
                pShape->DecrementRef();
                }
            HRFRasterFile::SetLookAhead(pi_Page, Blocks, pi_ConsumerID, pi_Async);
            }
        }
    }


//-----------------------------------------------------------------------------
// Public
// SetLookAhead
//
// Set the LookAhead for a specific page.
//-----------------------------------------------------------------------------
void HRFInternetImagingFile::SetLookAhead(uint32_t               pi_Page,
                                          const HGFTileIDList&   pi_rBlocks,
                                          uint32_t               pi_ConsumerID,
                                          bool                  pi_Async)
    {
    HPRECONDITION(HasLookAhead(pi_Page));

    if (m_PageID != pi_Page)
        static_cast<HRFRasterFile*>(GetPage(pi_Page).GetPtr())->SetLookAhead(pi_Page,
                                                                 pi_rBlocks,
                                                                 pi_ConsumerID,
                                                                 pi_Async);
    else
        HRFRasterFile::SetLookAhead(0,
                                    pi_rBlocks,
                                    pi_ConsumerID,
                                    pi_Async);
    }


//-----------------------------------------------------------------------------
// Public
// Indicates if the file is asynchronous or not
//-----------------------------------------------------------------------------
bool HRFInternetImagingFile::IsAsynchronous() const
    {
    return (HasLocalCachedFile() ? false : (m_TileWaitTime == 0));
    }


//-----------------------------------------------------------------------------
// Public
// Sets the asynchronicity of the image
//-----------------------------------------------------------------------------
void HRFInternetImagingFile::SetAsynchronous(bool pi_Async)
    {
    // Set the TileWaitTime depending on the async
    m_TileWaitTime = (pi_Async ? 0 : LONG_MAX);
    }

//-----------------------------------------------------------------------------
// Public
// Tells if more tiles are pending from network connections
//-----------------------------------------------------------------------------
bool HRFInternetImagingFile::HasTilePending()
    {
    HFCMonitor ProgressMonitor(s_ProgressKey);
    return (s_ProgressCurrentTiles != s_ProgressTotalTiles);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
bool HRFInternetImagingFile::Reconnect()
    {
    HPRECONDITION(!HasLocalCachedFile());
    bool Result = false;

    SetState(RECONNECTING);

    // Remove the previous connection
    HFCMonitor Monitor(m_ConnectionKey);
    m_pDisconnectorThread->AddConnection(m_pConnection.release());

    // verify the protocol
    try
        {
        // Create the connection to the host
        ConnectToHost(GetURL());

        // Initialize connection using the right protocol
        if (IsHMRImagingProtocol(m_Version))
            {
            Result = TryHIPProtocol(m_Version);

            //There is no need to check the HIP protocol if the
            //HRFInternetImagingFile has already been constructed.
            if ((Result == true) || (m_IsConstructing == false))
                {
                Result = RequestFileWithHIPProtocol(GetURL());
                }
            }
        else if (IsInternetImagingProtocol(m_Version))
            {
            Result = RequestFileWithIIPProtocol(GetURL());
            }
        else
            {
            HASSERT(0);
            }

        }
    catch(HRFInternetImagingConnectionException&)
        {
        }

    // if the reconnection was succesful, clear the thread exception
    if (Result)
        ClearThreadException();

    return (Result);
    }

//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HRFInternetImagingFile::StopRequests()
    {
    m_RequestStopped = true;
    SetThreadException(HRFInternetImagingException(HRFII_CANCELLED_EXCEPTION, GetURL()->GetURL()));
    }


//-----------------------------------------------------------------------------
// Public
// If the image does not have a histogram, this method request
// a generated histogram from the server.
//-----------------------------------------------------------------------------
bool HRFInternetImagingFile::GenerateHistogram()
    {
    HPRECONDITION(!HasLocalCachedFile());
    HPRECONDITION(IsHMRImagingProtocol(m_Version));
    HPRECONDITION(CountPages() > 0);
    HPRECONDITION(GetPageDescriptor(0) != 0);
    static const string s_HistoRequest("obj=hip-histogram,1");
    bool Result = false;

    // get the page descriptor
    HFCPtr<HRFPageDescriptor> pPage(GetPageDescriptor(0));
    HASSERT(pPage != 0);

    if (!pPage->HasHistogram())
        {
        // Reset the state event to unsignaled
        m_HistogramEvent.Reset();

        // send the request to the server
        try
            {
            // send the request
            m_pConnection->Send((const Byte*)s_HistoRequest.data(), s_HistoRequest.size());

            // wait for the histo to arrive or an exception to be thrown
            WaitForEvent(m_HistogramEvent);

            // verify if the histogram has really arrived, if so, place it in the page descriptor
            HFCMonitor Monitor(m_DataKey);
            Result = ((m_pHistogram != 0) && pPage->SetHistogram(*m_pHistogram));
            }

        // catch any exception and reset the thread exception, for another
        // part of the class to handle (ReadTile, etc).
        catch(HRFInternetImagingException& InternetImagingException)
            {
            SetThreadException(InternetImagingException);
            }
        catch(HFCException& Exception)
            {
            SetThreadException(Exception);
            }
        catch(...)
            {
            SetThreadException(HFCException());
            }
        }

    return (Result);
    }

//-----------------------------------------------------------------------------
// Public
// Prepares a request for ALL the attributes from the server
//-----------------------------------------------------------------------------
bool HRFInternetImagingFile::DownloadAttributes()
    {
    HPRECONDITION(!HasLocalCachedFile());
    HPRECONDITION(IsHMRImagingProtocol(m_Version));
    HPRECONDITION(CountPages() > 0);
    HPRECONDITION(GetPageDescriptor(0) != 0);

    // prepare the request
    static const string s_Request("obj=hip-attributes,0");

    // Send the request
    return DownloadAttributes(s_Request);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
bool HRFInternetImagingFile::DownloadAttributes(const list<string>& pi_rAttributeNames)
    {
    HPRECONDITION(!HasLocalCachedFile());
    HPRECONDITION(IsHMRImagingProtocol(m_Version));
    HPRECONDITION(CountPages() > 0);
    HPRECONDITION(GetPageDescriptor(0) != 0);
    HPRECONDITION(pi_rAttributeNames.size() > 0);

    // prepare the request
    static const string s_Base("obj=hip-attributes,0");

    // prepare the request
    ostringstream Request;
    Request << s_Base;
    for (list<string>::const_iterator Itr = pi_rAttributeNames.begin();
         Itr != pi_rAttributeNames.end();
         ++Itr)
        Request << "," << *Itr;

    // Send the request
    return DownloadAttributes(Request.str());
    }



//-----------------------------------------------------------------------------
// Public
// Returns the type of the original file on the server
//-----------------------------------------------------------------------------
const WString& HRFInternetImagingFile::RequestOriginalFileType(uint32_t pi_TimeOut)
    {
    static const string s_TypeRequest("obj=hip-file-type");

    if ((m_OriginalFileType.empty()) &&
        (IsHMRImagingProtocol(m_Version)) &&
        (m_Version >= s_HIP0120) )
        {
        // Reset the state event to unsignaled
        m_OriginalFileTypeEvent.Reset();

        // send the request to the server
        try
            {
            // send the request
            m_pConnection->Send((const Byte*)s_TypeRequest.data(), s_TypeRequest.size());

            // wait for the histo to arrive or an exception to be thrown
            WaitForEvent(m_OriginalFileTypeEvent, pi_TimeOut);
            }

        // catch any exception and reset the thread exception, for another
        // part of the class to handle (ReadTile, etc).
        catch(HRFInternetImagingException& InternetImagingException)
            {
            SetThreadException(InternetImagingException);
            }
        catch(HFCException& Exception)
            {
            SetThreadException(Exception);
            }
        catch(...)
            {
            SetThreadException(HFCException());
            }
        }

    HFCMonitor Monitor(m_DataKey);
    return (m_OriginalFileType);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
bool HRFInternetImagingFile::IsServerOnline(const HFCURL* pi_pURL,
                                             uint32_t      pi_TimeOut)
    {
    HPRECONDITION(pi_pURL != 0);
    HPRECONDITION(pi_pURL->GetSchemeType() == HFCURLHTTP::s_SchemeName() ||
                  pi_pURL->GetSchemeType() == HFCURLHTTPS::s_SchemeName() ||
                  pi_pURL->GetSchemeType() == HFCURLInternetImagingSocket::s_SchemeName());
    HPRECONDITION(pi_TimeOut > 0);
    bool Result = false;

    try
        {
        const HFCURLCommonInternet* pInternetUrl = (HFCURLCommonInternet*)pi_pURL;

        // rebuild a iip URL just to connect the socket to see if available
        HFCURLInternetImagingSocket URL(WString(), WString(),         // user/password
                                        pInternetUrl->GetHost(),    // Host
                                        pInternetUrl->GetPort(),    // Port
                                        WString(L"dummy"));

        // Connect to the server
        HRFSocketConnection Connection(URL);
        Result = Connection.Connect(WString(L""), WString(L""), pi_TimeOut);
        }
    catch(HFCInternetConnectionException&)
        {
        }

    return (Result);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
bool HRFInternetImagingFile::IsServerHIPCompatible(const HFCURL* pi_pURL,
                                                    uint32_t      pi_TimeOut)
    {
    HPRECONDITION(pi_pURL != 0);
    HPRECONDITION((HRFURLInternetImagingHTTP::IsURLInternetImaging(pi_pURL) == true) ||
                  (pi_pURL->GetSchemeType() == HFCURLInternetImagingSocket::s_SchemeName()));
    HPRECONDITION(pi_TimeOut > 0);

    static const string            Marker("\r\n");
    static const string            HIPRequest("obj=hip,1.0");
    static const string            HIPResponse("hip:");
    bool                          Result = false;
    HAutoPtr<HFCInternetConnection> pConnection;

    try
        {
        // Create the connection
        if ((pi_pURL->GetSchemeType() == HFCURLHTTP::s_SchemeName()) ||
            (pi_pURL->GetSchemeType() == HFCURLHTTPS::s_SchemeName()))
            {
            pConnection = new HRFHTTPConnection((const HRFURLInternetImagingHTTP&)*pi_pURL);
            }
        else
            pConnection = new HRFSocketConnection((const HFCURLInternetImagingSocket&)*pi_pURL);

        HASSERT(pConnection != 0);

        // Connect to the server
        if (pConnection->Connect(WString(L""), WString(L""), pi_TimeOut))
            {
            // send the command "obj=hip,1.0" to the server
            pConnection->Send((const Byte*)HIPRequest.data(), HIPRequest.size());

            // If it response with "hip:1.0", it means it can handle the HIP protocol.
            HFCBuffer Buffer(1, 1);
            while ((pConnection->IsConnected()) &&
                   (Buffer.SearchFor((const Byte*)Marker.data(), Marker.size()) == -1))
                {
                // Wait for data to arrive
                size_t DataAvailable = pConnection->WaitDataAvailable();

                // read it
                pConnection->Receive(Buffer.PrepareForNewData(DataAvailable), DataAvailable);
                Buffer.SetNewDataSize(DataAvailable);
                }

            // Verify if the content of the buffer is "hip:..."
            if (Buffer.GetDataSize() >= HIPResponse.size())
                {
                // convert the available data into a string and lower-case it
                string Response((const char*)Buffer.GetData(), HIPResponse.size());

                // if the content starts with an OBJ=HIP response, then the server is HIP compatible
                Result = CaseInsensitiveStringToolsA().AreEqual(Response, HIPResponse);
                }
            }
        }
    catch(HFCInternetConnectionException&)
        {
        }

    return (Result);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
bool HRFInternetImagingFile::GetSupportedFormats(const HFCURL*   pi_pURL,
                                                  list<WString>*  po_pList,
                                                  bool           pi_OnlyFileBased,
                                                  uint32_t        pi_TimeOut)
    {
    HPRECONDITION(po_pList != 0);
    HPRECONDITION(po_pList->empty());
    HPRECONDITION(pi_pURL != 0);
    HPRECONDITION((HRFURLInternetImagingHTTP::IsURLInternetImaging(pi_pURL) == true) ||
                  (pi_pURL->GetSchemeType() == HFCURLInternetImagingSocket::s_SchemeName()));
    HPRECONDITION(pi_TimeOut > 0);
    static const string Marker("\r\n");
    static const string ResponsePrefix("format,");
    bool               Result = false;

    // build the formats request
    string Request("obj=formats");
    if (pi_OnlyFileBased)
        Request +=",1";

    try
        {
        // Create the connection
        HAutoPtr<HFCInternetConnection> pConnection;
        if ((pi_pURL->GetSchemeType() == HFCURLHTTP::s_SchemeName()) ||
            (pi_pURL->GetSchemeType() == HFCURLHTTPS::s_SchemeName()))
            {
            pConnection = new HRFHTTPConnection((const HRFURLInternetImagingHTTP&)*pi_pURL);
            }
        else
            pConnection = new HRFSocketConnection((const HFCURLInternetImagingSocket&)*pi_pURL);
        HASSERT(pConnection != 0);

        // Connect to the server
        if (pConnection->Connect(WString(L""), WString(L""), pi_TimeOut))
            {
            // send the Request to the server
            pConnection->Send((const Byte*)Request.data(), Request.size());

            uint32_t codePage;
            BeStringUtilities::GetCurrentCodePage(codePage);

            // Read until the final marker is found
            bool FinalMarkerFound = false;
            HFCBuffer Buffer(1, 1);
            while ((!FinalMarkerFound) && (pConnection->IsConnected()) )
                {
                // Wait for data to arrive
                size_t DataAvailable = pConnection->WaitDataAvailable();

                // read it
                pConnection->Receive(Buffer.PrepareForNewData(DataAvailable), DataAvailable);
                Buffer.SetNewDataSize(DataAvailable);

                // analyze the data while "\r\n"can be found
                size_t MarkerPos;
                while ((MarkerPos = Buffer.SearchFor((const Byte*)Marker.data(), Marker.size())) != -1)
                    {
                    // if the marker was found at position 0, it indicates the there is a lone marker
                    // in the response, which is the final marker.
                    FinalMarkerFound = (MarkerPos == 0);

                    // if it is not a final marker, we have a response string, analyze it
                    if (!FinalMarkerFound)
                        {
                        // Extract the response from the buffer
                        string Response((const char*)Buffer.GetData(), MarkerPos);
                        CaseInsensitiveStringToolsA().ToLower(Response);

                        // if the response does not start with the format response prefix,
                        // the server can not handle it (IIP).
                        if (Response.find(ResponsePrefix) != 0)
                            throw HFCException();

                        // Add the current response to the output list
                        WString tempoStr;
                        BeStringUtilities::LocaleCharToWChar(tempoStr,(const char*)(Buffer.GetData() + ResponsePrefix.size()), codePage, MarkerPos - ResponsePrefix.size());

                        po_pList->push_back(tempoStr);
                        }

                    // mark the data in the buffer as read
                    Buffer.MarkReadData(MarkerPos + Marker.size());
                    }
                }

            Result = true;
            }
        }
    catch(...)
        {
        po_pList->clear();
        Result = false;
        }

    return (Result);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
bool HRFInternetImagingFile::GetSecurityToken(const HFCURL* pi_pURL,
                                               string*       po_pToken,
                                               uint32_t      pi_TimeOut)
    {
    HPRECONDITION(po_pToken != 0);
    HPRECONDITION(po_pToken->empty());
    HPRECONDITION(pi_pURL != 0);
    HPRECONDITION((HRFURLInternetImagingHTTP::IsURLInternetImaging(pi_pURL) == true) ||
                  (pi_pURL->GetSchemeType() == HFCURLInternetImagingSocket::s_SchemeName()));
    HPRECONDITION(pi_TimeOut > 0);
    static const string Marker("\r\n");
    static const string Request("obj=hip,1.0&hip-security-token");
    static const string Prefix("hip-security-token");
    bool               Result = false;

    try
        {
        // Create the connection
        HAutoPtr<HFCInternetConnection> pConnection;
        if ((pi_pURL->GetSchemeType() == HFCURLHTTP::s_SchemeName()) ||
            (pi_pURL->GetSchemeType() == HFCURLHTTPS::s_SchemeName()))
            {
            pConnection = new HRFHTTPConnection((const HRFURLInternetImagingHTTP&)*pi_pURL);
            }
        else
            pConnection = new HRFSocketConnection((const HFCURLInternetImagingSocket&)*pi_pURL);
        HASSERT(pConnection != 0);

        // Connect to the server
        if (pConnection->Connect(WString(L""), WString(L""), pi_TimeOut))
            {
            // send the Request to the server
            pConnection->Send((const Byte*)Request.data(), Request.size());

            // Read until the final marker is found
            bool FinalMarkerFound = false;
            HFCBuffer Buffer(1024, 2048);
            HRFInternetErrorHandler ErrorHandler;
            while ((!FinalMarkerFound) && (pConnection->IsConnected()) )
                {
                // read 1 byte
                pConnection->Receive(Buffer.PrepareForNewData(1), 1);
                Buffer.SetNewDataSize(1);

                // verify for errors
                if (ErrorHandler.CanHandle(Buffer))
                    ErrorHandler.Handle(*((HRFInternetImagingFile*)0), // ceci est une crosse
                                        Buffer,
                                        *pConnection);

                // analyze the data while "\r\n"can be found
                size_t MarkerPos;
                while ((MarkerPos = Buffer.SearchFor((const Byte*)Marker.data(), Marker.size())) != -1)
                    {
                    // if the marker was found at position 0, it indicates the there is a lone marker
                    // in the response, which is the final marker.
                    FinalMarkerFound = (MarkerPos == 0);

                    // if it is not a final marker, we have a response string, analyze it
                    if (!FinalMarkerFound)
                        {
                        // Extract the response from the buffer
                        string Response((const char*)Buffer.GetData(), MarkerPos);

                        // verify if the response starts with the prefix
                        if (Response.size() < Prefix.size())
                            throw HRFInternetImagingException(HRFII_INVALID_RESPONSE_EXCEPTION, pi_pURL->GetURL());

                        // verify that the response starts with the prefix, regardless of case
                        // throw on the first different character because the server could not
                        // return the security token
                        for (string::size_type Pos = 0; Pos < Prefix.size(); Pos++)
                            if (tolower(Prefix[Pos]) != tolower(Response[Pos]))
                                throw HRFInternetImagingException(HRFII_INVALID_RESPONSE_EXCEPTION, pi_pURL->GetURL());

                        // the token is everything before the end marker
                        *po_pToken = string((const char*)Buffer.GetData() + Prefix.size() + 1, // after the prefix and ','
                                            MarkerPos - Prefix.size() - 1);
                        Result = true;
                        }

                    // mark the data in the buffer as read
                    Buffer.MarkReadData(MarkerPos + Marker.size());
                    }
                }
            }
        }
    catch(HRFInternetImagingException&)
        {
        po_pToken->resize(0);
        Result = false;
        throw;
        }
    catch(...)
        {
        po_pToken->resize(0);
        Result = false;
        }

    return (Result);
    }

//---------------------------------------------------------------------------------
// Public
// TEMPORARY - (TR 255711) Filter the setting of the default ratio to meter. The best solution
//             would have been to modify rasterlib, but because of time constraint,
//             it is done temporary here instead.
//---------------------------------------------------------------------------------
void HRFInternetImagingFile::SetDefaultRatioToMeter(double pi_RatioToMeter,
                                                    uint32_t pi_Page,
                                                    bool   pi_CheckSpecificUnitSpec,
                                                    bool   pi_GeoModelDefaultUnit,
                                                    bool   pi_InterpretUnitINTGR)
    {
    //TEMP - The code to find the extension has been copied from HFCURLFile::GetExtension()
    //       because of time constraint. It might be a good idea to have a static get
    //       extension function coded in HFCURL.
    WString            Extension;
    WString::size_type LastDotPos = m_ImageName.find_last_of(L".");

    if ((LastDotPos != WString::npos) && (LastDotPos < (m_ImageName.length() - 1)))
        Extension = m_ImageName.substr(LastDotPos + 1, m_ImageName.length() - LastDotPos - 1);

    if (BeStringUtilities::Wcsicmp(Extension.c_str(), L"pdf") != 0)
        {
        HRFRasterFile::SetDefaultRatioToMeter(pi_RatioToMeter,
                                              pi_Page,
                                              pi_CheckSpecificUnitSpec,
                                              pi_GeoModelDefaultUnit,
                                              pi_InterpretUnitINTGR);
        }
    }

//-----------------------------------------------------------------------------
// Protected section
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// protected
// This method is used in SetLookAhead to indicate to a derived class that
// the current LookAhead has been cancelled.
//-----------------------------------------------------------------------------
void HRFInternetImagingFile::CancelLookAhead(uint32_t pi_Page)
    {
    HPRECONDITION(pi_Page == m_PageID);

    HFCMonitor ConnectionMonitor(m_ConnectionKey);
    HFCMonitor PoolMonitor(m_TilePool);
    HFCMonitor ProgressMonitor(s_ProgressKey);

    // Clear the thread buffer
    m_pThread->ClearBuffer();

    // Invalidate the waiting tiles
    m_TilePool.Invalidate();
    m_TilePool.Clear();

#ifdef TRACE_LOOKAHEAD
    HDEBUGTEXT(L"CancelLookAhead\n");
#endif

    // update the progression
    s_ProgressTotalTiles   -= m_ProgressCurrentTiles;
    m_ProgressCurrentTiles = 0;

    // if the previous connect indicated that a redirect as occured,
    // reset the event
    if (m_Redirected)
        m_RedirectEvent.Reset();

    // Reconnect to the host
    Reconnect();

    // Specify that a cancellatino occured
    m_Cancelled = true;
    }


//-----------------------------------------------------------------------------
// protected
// This method is used in SetLookAhead to give the list of needed tiles
// to a derived class, since it knows how to obtain the tiles.
//-----------------------------------------------------------------------------
void HRFInternetImagingFile::RequestLookAhead(uint32_t              pi_Page,
                                              const HGFTileIDList&  pi_rBlocks,
                                              bool                 pi_Async)
    {
    HPRECONDITION(pi_Page == m_PageID);

    HGFTileIDList::const_iterator   Itr;
    uint64_t                        TileID;
    unsigned short                  TileRes;
    uint64_t                        TileIndex;
    uint64_t                        TilePosX;
    uint64_t                        TilePosY;
    uint32_t                       IIPResolution=0;
    HFCPtr<HRFTile>                 pTile;
    //char                          Request[s_StringSize] = {'\0'};
    uint32_t                       NewTileCount = 0;
    HGFTileIDList                   AvailableBlockList;
    uint32_t                       CurrentResIndex = -1;
    string                          CurrentResFactor;
    HAutoPtr<HGFTileIDDescriptor>   pTileIDDesc;
    uint32_t                       CurrentResEditorIndex = -1;



    // check if the asynchronicity is changing from async to sync.
    bool ToSync = (IsAsynchronous() && !pi_Async);

    // set this object to the specified asynchronicity
    SetAsynchronous(pi_Async);

    // Initialize the buffer & add prefix
    ostringstream LookAheadRequest;

    // for each tile in the list, create a til=id in a string and create
    // the empty tile object in the waiting list.
    for (Itr = pi_rBlocks.begin(); Itr != pi_rBlocks.end(); Itr++)
        {
#ifdef TRACE_LOOKAHEAD
        WChar Msg[256];
        BeStringUtilities::Snwprintf(Msg, L"RequestLookAhead : %d\n", *Itr);
        HDEBUGTEXT(Msg);
#endif


        // Compute the position of the tile, tile ID and IIP Resolution
        TileID    = (*Itr);
        TileRes   = (unsigned short)s_TileDescriptor.GetLevel(TileID); // the maximun level is 255
        TileIndex = s_TileDescriptor.GetIndex(TileID);

        if (m_IsUnlimitedResolution)
            {
            // for an unlimited resolution, TileRes is the resolution index into the UnlimitedResolutionMap
            // find the resolution editor into the ResolutionEditorRegistry
            if (CurrentResEditorIndex != TileRes)
                {
                ResolutionEditorRegistry::const_iterator Itr(m_ResolutionEditorRegistry.begin());
                const HRFResolutionEditor* pResEditor = 0;
                while (pResEditor == 0 && Itr != m_ResolutionEditorRegistry.end())
                    {
                    if ((*Itr)->GetResolutionIndex() == TileRes)
                        pResEditor = *Itr;
                    else
                        Itr++;
                    }
                HASSERT(pResEditor != 0);

                HFCPtr<HRFResolutionDescriptor> pResDesc = pResEditor->GetResolutionDescriptor();
                pTileIDDesc = new HGFTileIDDescriptor(pResDesc->GetWidth(),
                                                      pResDesc->GetHeight(),
                                                      pResDesc->GetBlockWidth(),
                                                      pResDesc->GetBlockHeight());

                CurrentResEditorIndex = TileRes;
                }

            pTileIDDesc->GetPositionFromIndex(TileIndex, &TilePosX, &TilePosY);
            }
        else
            {
            IIPResolution = m_ResolutionCount - TileRes - 1;
            m_pTileDescriptors[IIPResolution].GetPositionFromIndex(TileIndex, &TilePosX, &TilePosY);
            }

        // get the tile from the pool.
        pTile = m_TilePool.GetTile(TileID);

        // The tile does not already exists, so create it.
        if (pTile == 0 || m_Cancelled)
            {
            m_TilePool.CreateTile(TileID, TileIndex, TilePosX, TilePosY, TileRes);
            NewTileCount++;

            if (m_IsUnlimitedResolution)    // for unlimited resolution, use HIP-TIL command
                {
                if (CurrentResIndex != TileRes)
                    {
                    // find in the map the resolution factor
                    UnlimitedResolutionMap::const_iterator Itr(m_UnlimitedResolutionMap.begin());

                    bool ResFound = false;
                    while (!ResFound && Itr != m_UnlimitedResolutionMap.end())
                        {
                        if (Itr->second == TileRes)
                            ResFound = true;
                        else
                            Itr++;
                        }
                    HASSERT(ResFound);
                    CurrentResIndex = TileRes;
                    CurrentResFactor = Itr->first;
                    }

                if (LookAheadRequest.str().empty())
                    LookAheadRequest << "hip-til=";
                else
                    LookAheadRequest << "&hip-til=";

                LookAheadRequest << CurrentResFactor;
                }
            else
                {
                if (LookAheadRequest.str().empty())
                    LookAheadRequest << "til=";
                else
                    LookAheadRequest << "&til=";

                LookAheadRequest << IIPResolution;
                }

            LookAheadRequest << "," << TileIndex << "," << m_PageID;
            }

        // the tile already exists.  If the asynchronicity is getting from
        // asynchronous to synchronous, propagate the tile so that any HRA
        // object that owns this HRF update its internal state and re-read
        // the tile if needed.
        else
            {
            // Advise the superclass that we are already waiting for it
            if (pTile->WaitUntilSignaled(0))
                {
                AvailableBlockList.push_back(TileID);

                if (ToSync)
                    Propagate(HRFProgressImageChangedMsg(pi_Page, TileRes, TilePosX, TilePosY, false));
                }
            }
        }

    // These monitor must be created before the "if ()" scope in order
    // for them to be destroyed after the m_Cancelled = false.
    HFCMonitor ConnectionMonitor;
    HFCMonitor ProgressMonitor;

    // Send the request

    // send the request if valid and connection is on
    if (!LookAheadRequest.str().empty())
        {
        // wait of the redirect event (if needed)
        if (m_Cancelled && m_Redirected)
            m_RedirectEvent.WaitUntilSignaled();

        // Assign the keys to the monitors
        ConnectionMonitor.Assign(m_ConnectionKey);
        ProgressMonitor.Assign(s_ProgressKey);

        // Send the request
        try
            {
            if (m_pConnection)
                {
                m_pConnection->Send((const Byte*)LookAheadRequest.str().c_str(),
                                    LookAheadRequest.str().size() + 1);

                // Update Progress Information
                s_ProgressTotalTiles   += NewTileCount;
                m_ProgressCurrentTiles += NewTileCount;
                }
            else
                {
                // Clear the thread buffer
                m_pThread->ClearBuffer();

                // Invalidate the waiting tiles
                m_TilePool.Invalidate();
                m_TilePool.Clear();
                }
            }
        catch(HFCException& e)
            {
            // Clear the thread buffer
            m_pThread->ClearBuffer();

            // Invalidate the waiting tiles
            m_TilePool.Invalidate();
            m_TilePool.Clear();

            // Set the thread exception so that the ReadBlock may be notified.
            SetThreadException(e);
            }
        catch(...)
            {
            // Clear the thread buffer
            m_pThread->ClearBuffer();

            // Invalidate the waiting tiles
            m_TilePool.Invalidate();
            m_TilePool.Clear();

            // Set the thread exception so that the ReadBlock may be notified.
            SetThreadException(HFCException());
            }
        }

    // Remove the cancel flag for future LookAhead
    m_Cancelled = false;

    // send notifications for the available blocks
    ConnectionMonitor.ReleaseKey();
    ProgressMonitor.ReleaseKey();
    for (Itr = AvailableBlockList.begin(); Itr != AvailableBlockList.end(); Itr++)
        NotifyBlockReady(pi_Page, *Itr);
    }

//-----------------------------------------------------------------------------
// protected
// ResetLookAhead
//
// This method is called by a derived-class when the link with the
// source of data blocks has been re-established
//-----------------------------------------------------------------------------
void HRFInternetImagingFile::ResetLookAhead(uint32_t   pi_Page,
                                            bool      pi_Async)
    {
    HPRECONDITION(HasLookAhead(pi_Page));
    HPRECONDITION(pi_Page == m_PageID);

    HRFRasterFile::ResetLookAhead(0, pi_Async);
    }

//-----------------------------------------------------------------------------
// Protected
//
//-----------------------------------------------------------------------------
void HRFInternetImagingFile::TileHasArrived(HFCPtr<HRFTile>& pi_rpTile)
    {
    HFCMonitor TileMonitor(*pi_rpTile);
    HPRECONDITION(pi_rpTile != 0);
    bool ProgressEnd = false;

    // Update progress information
        {
        HFCMonitor ProgressMonitor(s_ProgressKey);
        s_ProgressCurrentTiles++;
        m_ProgressCurrentTiles--;
        if (s_ProgressCurrentTiles == s_ProgressTotalTiles)
            {
            ProgressEnd = true;
            s_ProgressTotalTiles   = 0;
            s_ProgressCurrentTiles = 0;
            }
        }

    try
        {
        // At this point the connection key is claim by a HFCMonitor class
        // elsewhere.  Release it by hand and make sure that it is reclaimed
        // when leaving this method so that the monitor object may do its
        // job properly
        m_ConnectionKey.ReleaseKey();

        // Notify that the tile is arrived
        NotifyBlockReady(m_PageID, pi_rpTile->GetID());

        // If the file is asynchronous, make it send a message that this tile has finally arrived.
        // Also, if the tile is invalid (see its compression type), do not propagate the message
        uint32_t CompressionType = *((uint32_t*)pi_rpTile->GetData());
        if ((m_TileWaitTime == 0) && (CompressionType != IIP_COMPRESSION_INVALID))
            {
            // Extract info from the tile
            unsigned short Res  = pi_rpTile->GetResolution();
            uint64_t PosX  = pi_rpTile->GetPosX();
            uint64_t PosY  = pi_rpTile->GetPosY();
            TileMonitor.ReleaseKey();

            // Propagate the message
            Propagate(HRFProgressImageChangedMsg(m_PageID, Res, PosX, PosY, ProgressEnd));
            }
        m_ConnectionKey.ClaimKey();
        }
    catch(...)
        {
        m_ConnectionKey.ClaimKey();
        throw;
        }
    }

//-----------------------------------------------------------------------------
// Protected
//
//-----------------------------------------------------------------------------
void HRFInternetImagingFile::SetThreadException(const HFCException& pi_rException)
    {
    HFCMonitor Monitor(m_ThreadExceptionKey);

    // Assign only if there was no prior exception
    if (m_pThreadException.get() == 0)
        {
        // Copy the exception
        m_pThreadException           = pi_rException.Clone();
        m_IsInternetImagingException = false;

        // Signal that an exception is ready
        m_ThreadExceptionEvent.Signal();
        }
    }


//-----------------------------------------------------------------------------
// Protected
//
//-----------------------------------------------------------------------------
void HRFInternetImagingFile::SetThreadException(const HRFInternetImagingException& pi_rException)
    {
    HFCMonitor Monitor(m_ThreadExceptionKey);

    // Assign only if there was no prior exception
    if (m_pThreadException.get() == 0)
        {
        // Copy the exception
        m_pThreadException           = pi_rException.Clone();
        m_IsInternetImagingException = true;

        // Signal that an exception is ready
        m_ThreadExceptionEvent.Signal();
        }
    }

//-----------------------------------------------------------------------------
// Protected
//
//-----------------------------------------------------------------------------
void HRFInternetImagingFile::HandleThreadException()
    {
    HFCMonitor Monitor(m_ThreadExceptionKey);

    // If an exception was assigned, throw it
    if (m_pThreadException.get() != 0)
        {
        // Extract the exception in another autoptr
        HAutoPtr<HFCException> pException(m_pThreadException.release());

        // Clear the last thread exception
        ClearThreadException();

        // Tell the exception to throw a copy (in order to keep
        // to type information).
        throw HFCException(*(pException.get()));
        }
    }

//-----------------------------------------------------------------------------
// Private section
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// constructor
//
// this constructor is used to open a specific page
//-----------------------------------------------------------------------------
HRFInternetImagingFile::HRFInternetImagingFile(const HRFInternetImagingFile*    pi_pFile,
                                               uint32_t                         pi_Page)
    : HRFRasterFile(pi_pFile->GetURL(),
                    pi_pFile->GetAccessMode(),
                    pi_pFile->GetOffset()),
    m_State                   (DISCONNECTED),
    m_IsConstructing          (true),
    m_Redirected              (false),
    m_Cancelled               (false),
    m_RequestStopped          (false),
    m_StateEvent              (true, false),
    m_ThreadExceptionEvent    (true, false),
    m_HistogramEvent          (true, false),
    m_LocalCacheFileEvent     (true, false),
    m_RedirectEvent           (true, false),
    m_AttributesEvent         (true, false),
    m_PageDescriptorAvailable (true, false),
    m_MultiPageSupported      (pi_pFile->m_MultiPageSupported),
    m_PageID                  (pi_Page),
    m_PageCount               (pi_pFile->m_PageCount),
    m_WaitTileOnRead          (true)
    {
    Constructor();
    }

//-----------------------------------------------------------------------------
// Construct the encoded and escaped version of the image name and the
// cookie according to the encoding of the protocol used.
//-----------------------------------------------------------------------------
void HRFInternetImagingFile::ConstructEncodedInfoForRequest(const WString& pi_rPWCookie,
                                                            const string&  pi_rPWCookieUTF8Escaped)
    {
    if ((GetURL()->IsCompatibleWith(HRFURLInternetImagingHTTP::CLASS_ID) == true) &&
        (((HFCPtr<HRFURLInternetImagingHTTP>&)GetURL())->GetUTF8EscapedImage(&m_EncodedImageNameForRequest) == true))
        {
        //If the user has passed the image name already in UTF8, the protocol to use must be UTF8.
        HASSERT(m_CurrentProtocol.GetNumber(2) == HRFInternetImagingFile::s_HIP_UTF8_Protocol);
        HASSERT(GetURL()->IsUTF8URL() == true);
        HASSERT(GetURL()->IsEncodedURL() == true);

        WString::size_type Pos;

        // Change all "\\" to "/"
        while ((Pos = m_EncodedImageNameForRequest.find('\\', 0)) != WString::npos)
            m_EncodedImageNameForRequest[Pos] = '/';

        m_EncodedPWCookieForRequest = pi_rPWCookieUTF8Escaped;
        }
    else
        {
        HASSERT(GetURL()->IsUTF8URL() == false);

        if (m_CurrentProtocol.GetNumber(2) == HRFInternetImagingFile::s_HIP_UTF8_Protocol)
            {
            //If a conversion is required the URL is expected to be not encoded.
            HASSERT(GetURL()->IsEncodedURL() == false);
            
            m_EncodedImageNameForRequest = Utf8String(m_ImageName).c_str();

            m_EncodedPWCookieForRequest  = Utf8String(pi_rPWCookie).c_str();
            }
        else
            {
            //If a conversion is required the URL is expected to be not encoded.
            HASSERT(GetURL()->IsEncodedURL() == false);

            size_t  destinationBuffSize = m_ImageName.GetMaxLocaleCharBytes();
            char*  imageNameMBS = (char*)_alloca (destinationBuffSize);

            m_EncodedImageNameForRequest = m_ImageName.ConvertToLocaleChars(imageNameMBS, destinationBuffSize);

            destinationBuffSize = pi_rPWCookie.GetMaxLocaleCharBytes();
            char*  PWCookieMBS= (char*)_alloca (destinationBuffSize);
            BeStringUtilities::WCharToCurrentLocaleChar(PWCookieMBS,pi_rPWCookie.c_str(),destinationBuffSize);

            m_EncodedPWCookieForRequest  = pi_rPWCookie.ConvertToLocaleChars(PWCookieMBS, destinationBuffSize);
            }
        }

    if (GetURL()->IsEncodedURL() == false)
        {
        m_EncodedImageNameForRequest = HFCURLCommonInternet::
                                       EscapeURLParamValue(m_EncodedImageNameForRequest);
        m_EncodedPWCookieForRequest  = HFCURLCommonInternet::
                                       EscapeURLParamValue(m_EncodedPWCookieForRequest);
        }
    }

//-----------------------------------------------------------------------------
// Constructor
//
// initialize the class
//-----------------------------------------------------------------------------
void HRFInternetImagingFile::Constructor()
    {
    WString::size_type Pos;

    // Create the attribute set
    m_pAttributes               = new HPMAttributeSet;
    m_pListOfMetaDataContainers = new HMDMetaDataContainerList;

    // No tiles in progression
    m_ProgressCurrentTiles = 0;

    // Set asynchronicity.  Will be set by look ahead mostly.
    m_TileWaitTime = LONG_MAX;

    // ProjectWise support
    WString PWCookie;
    string  PWCookieUTF8Escaped;

    ExtractPWCookie(&PWCookie, &PWCookieUTF8Escaped);

    string UTF8EscapedURLPath(((HFCPtr<HFCURLCommonInternet>&)GetURL())->GetUTF8EscapedURLPath());

    HASSERT((UTF8EscapedURLPath.empty() == true) ||
            ((GetURL()->IsUTF8URL() == true) && (GetURL()->IsEncodedURL() == true)));

    if (m_pURL->IsCompatibleWith(HFCURLHTTPBase::CLASS_ID) == true)
        {
        bool  UTF8URL = GetURL()->IsUTF8URL();
        bool  EncodedURL = GetURL()->IsEncodedURL();

        // If the URL is a plain HTTP, make it a HRFURLInternetImagingHTTP
        if (GetURL()->GetSchemeType() == HFCURLHTTP::s_SchemeName())
            {
            m_pURL = new HRFURLInternetImagingHTTP(GetURL()->GetURL(), true);
            }
        else if (GetURL()->GetSchemeType() == HFCURLHTTPS::s_SchemeName())
            {
            m_pURL = new HRFURLInternetImagingHTTP(GetURL()->GetURL(), false);
            }

        ((HFCPtr<HFCURLCommonInternet>&)m_pURL)->SetUTF8EscapedURLPath(&UTF8EscapedURLPath);
        m_pURL->SetUTF8URL(UTF8URL);
        m_pURL->SetEncodedURL(EncodedURL);
        }

    //
    // Generate the image name (logical path and file name only without any
    // Internet decoration)
    //
    if (GetURL()->GetSchemeType() == HFCURLHTTP::s_SchemeName() ||
        GetURL()->GetSchemeType() == HFCURLHTTPS::s_SchemeName())
        {
        m_ImageName = ((HFCPtr<HRFURLInternetImagingHTTP>&)GetURL())->GetImage();
        }
    else if (GetURL()->GetSchemeType() == HFCURLInternetImagingSocket::s_SchemeName())
        {
        //Disabled : m_ImageName = ((HFCPtr<HFCURLInternetImagingSocket>&)GetURL())->GetImage();
        throw HRFInternetImagingConnectionException(GetURL()->GetURL(),
                                                    HRFInternetImagingConnectionException::CANNOT_CONNECT);
        }

    HASSERT(!m_ImageName.empty());

    // Change all "\\" to "/"
    while ((Pos = m_ImageName.find(L'\\', 0)) != WString::npos)
        m_ImageName[Pos] = L'/';

    //
    // Generate a name for a HIP small file
    //
    m_LocalCachedFileName = m_ImageName;

    // Extract the file name only from the path
    if ((Pos = m_LocalCachedFileName.find_last_of(L'/')) != WString::npos)
        m_LocalCachedFileName.erase(0, Pos+1);

    // Get the location of the local cache
    BeFileName LocalCacheLocation;
    ImagePP::ImageppLib::GetHost().GetImageppLibAdmin()._GetInternetImagingHIPSmallFileLocation(LocalCacheLocation);
    if (LocalCacheLocation.IsEmpty())
        ImagePP::ImageppLib::GetHost().GetImageppLibAdmin()._GetDefaultTempDirectory(LocalCacheLocation);

    // Build the full local cached file name
    m_LocalCachedFileName.insert(0, L"/");
    m_LocalCachedFileName.insert(0, WString(LocalCacheLocation.GetName()));

    if (m_PageID != 0)
        {
        WChar Buffer[32];
        BeStringUtilities::Snwprintf(Buffer, L"%ld", m_PageID);
        m_LocalCachedFileName += L"_page_";
        m_LocalCachedFileName += Buffer;
        }

    //
    // Start the receiving thread
    //
    m_pThread = new HRFInternetImagingThread(*this);
    HASSERT(m_pThread.get() != 0);
    m_pThread->StartThread();

    //
    // Start the disconnector thread
    //
        {
        HFCMonitor DisconnectorThreadKeyMonitor(s_DisconnectorThreadKey);
        if (s_pDisconnectorThread == 0)
            s_pDisconnectorThread = new HRFInternetDisconnectionThread;
        m_pDisconnectorThread = s_pDisconnectorThread;
        }

    try
        {
        // Establish the connection to the host
            {
            HFCMonitor Monitor(m_ConnectionKey); // caller must have claimed it
            ConnectToHost(GetURL());
            }

        //If the server doesn't support any protocol supported by the client, throw a connection exception.
        if (FindBestSupportingProtocol(UTF8EscapedURLPath.empty() == true) == false)
            {
            throw HRFInternetImagingConnectionException(GetURL()->GetURL(),
                                                        HRFInternetImagingConnectionException::CANNOT_CONNECT);
            }

        //Construct the encoded (multibyte or UTF8) and escaped image name and cookie information.
        ConstructEncodedInfoForRequest(PWCookie,
                                       PWCookieUTF8Escaped);

        bool IsFileRequestSucceed;

        //Desactivated : The IIP protocol has been desactivated. To reactivate it,
        //               call RequestFileWithIIPProtocol instead of RequestFileWithHIPProtocol
        //               when IsHMRImagingProtocol returns false.
        HASSERT(IsHMRImagingProtocol(m_CurrentProtocol) == true);
        //IsFileRequestSucceed = RequestFileWithIIPProtocol(GetURL());

        IsFileRequestSucceed = RequestFileWithHIPProtocol(GetURL());

        if (IsFileRequestSucceed == false)
            {
            throw HRFInternetImagingConnectionException(GetURL()->GetURL(),
                                                        HRFInternetImagingConnectionException::CANNOT_CONNECT);
            }

        // wait for the state to change
        WaitForEvent(m_StateEvent);
        if (m_State != CONNECTED)
            throw HRFInternetImagingConnectionException(GetURL()->GetURL(), HRFInternetImagingConnectionException::CANNOT_CONNECT);

        // Ask for the whole file from the server.  If unsuccessful, create descriptors for
        // the Internet Imaging File.
        if (m_IsUnlimitedResolution || !GetLocalCachedFile())
            BuildDescriptors();

        // Tell that the contruction is over so that future exception be stamped
        // with a reference to this object for handlers to manage with.
        m_IsConstructing = false;
        }
    catch(HFCInternetConnectionException& ConnectionException)
        {
        // stop the thread before rethrowing the exception
        m_pThread = 0;

        // Stop the disconnector thread
            {
            HFCMonitor DisconnectorThreadKeyMonitor(s_DisconnectorThreadKey);
            m_pDisconnectorThread = 0;
            if (s_pDisconnectorThread->GetRefCount() == 1)
                s_pDisconnectorThread = 0;
            }

        // repackage as an Internet Imaging Connection Exception
        HRFInternetImagingConnectionException::ErrorType ErrorType;
        switch(ConnectionException.GetErrorType())
            {
            case HFCInternetConnectionException::CANNOT_CONNECT:
                ErrorType = HRFInternetImagingConnectionException::CANNOT_CONNECT;
                break;

            case HFCInternetConnectionException::CONNECTION_LOST:
                ErrorType = HRFInternetImagingConnectionException::CONNECTION_LOST;
                break;

            default:
            case HFCInternetConnectionException::UNKNOWN:
                ErrorType = HRFInternetImagingConnectionException::UNKNOWN;
                break;
            }

        // set the exception
        throw HRFInternetImagingConnectionException(GetURL()->GetURL(), ErrorType);
        }
    catch(...)
        {
        // stop the thread before rethrowing the exception
        m_pThread = 0;

        // Stop the disconnector thread
            {
            HFCMonitor DisconnectorThreadKeyMonitor(s_DisconnectorThreadKey);
            m_pDisconnectorThread = 0;
            if (s_pDisconnectorThread->GetRefCount() == 1)
                s_pDisconnectorThread = 0;
            }

        // re-throw the same exception
        throw;
        }
    }

//-----------------------------------------------------------------------------
// Protected
//
//-----------------------------------------------------------------------------
void HRFInternetImagingFile::EndOfCurrentRequest()
    {
    //
    // The current state is CONNECTING_XXX
    //
    if (GetState() == ESTABLISH_PROTOCOL)
        {
        if (VerifyProtocolVersion())
            SetState(CONNECTED);
        else
            SetState(DISCONNECTED);
        }
    else if ((GetState() == CONNECTING)     ||
             (GetState() == CONNECTING_IIP) ||
             (GetState() == CONNECTING_HIP) )
        {
        // The basic information is correct
        if (VerifyHIPBasicInfo() ||
            VerifyIIPBasicInfo())
            {
            // Signal the state to connected
            SetState(CONNECTED);
            }

        // something missing in the basic info
        else
            {
            // Signal the state to disconnected
            SetState(DISCONNECTED);
            }
        }
    }

//-----------------------------------------------------------------------------
// Private
//
//-----------------------------------------------------------------------------
void HRFInternetImagingFile::WaitForEvent(HFCEvent& pi_rEvent)
    {
    // Add the exception event and the given event
    HFCSynchroContainer SynchroList;
    SynchroList.AddSynchro(&m_ThreadExceptionEvent);
    SynchroList.AddSynchro(&pi_rEvent);

    // wait until one of the event is signaled, if the wanted event is signaled
    // return otherwise call the exception handler.
    switch (HFCSynchro::WaitForMultipleObjects(SynchroList, false))
        {
        case 0: // exception caused the wait to end
            HandleThreadException();
            break;

        case 1: // the given event caused the wait to end
        case HFC_SYNCHRO_ALL:   // both event caused the wait to end.  give us a chance
            break;

#ifdef __HMR_DEBUG
            // should not happen
        case HFC_SYNCHRO_TIMEOUT:
            HASSERT(0);
#endif
        }
    } 

//-----------------------------------------------------------------------------
// Private
//
//-----------------------------------------------------------------------------
bool HRFInternetImagingFile::WaitForEvent(HFCEvent& pi_rEvent, int32_t pi_TimeOut)
    {
    HPRECONDITION(pi_TimeOut >= 0);

    // Add the exception event and the given event
    HFCSynchroContainer SynchroList;
    SynchroList.AddSynchro(&m_ThreadExceptionEvent);
    SynchroList.AddSynchro(&pi_rEvent);

    // wait until one of the event is signaled, if the wanted event is signaled
    // return otherwise call the exception handler.
    switch (HFCSynchro::WaitForMultipleObjects(SynchroList, pi_TimeOut, false))
        {
        case 0: // exception caused the wait to end
            HandleThreadException();
            return (false); // to make compiler shut-up

        case 1: // the given event caused the wait to end
        case HFC_SYNCHRO_ALL:   // both event caused the wait to end.  give us a chance
            return (true);

        case HFC_SYNCHRO_TIMEOUT:
            return (false);
        }

    return (false);
    }
//-----------------------------------------------------------------------------
// ExtractPWCookie
//
// If any, extract the ProjectWise cookie from the URL.
//-----------------------------------------------------------------------------
void HRFInternetImagingFile::ExtractPWCookie(WString* po_pPWCookie,
                                             string*  po_pPWCookieUTF8Escaped)
    {
    HPRECONDITION((po_pPWCookie != 0) && (po_pPWCookie->empty() == true) &&
                  (po_pPWCookieUTF8Escaped != 0) && (po_pPWCookieUTF8Escaped->empty() == true));

    string             UTF8EscapedURLPath;
    WString::size_type Pos = 0;
    WString            URL(GetURL()->GetURL());

    while (po_pPWCookie->empty() && (Pos = URL.find(L'&', Pos)) != WString::npos)
        {
        if (BeStringUtilities::Wcsnicmp(URL.substr(Pos).c_str(), L"&PW=", 4) == 0)
            {
            // the PW cookies is found
            WString::size_type EndPos;
            //WString::size_type CookieSize = WString::npos;
            if ((EndPos = URL.find(L"&", Pos + 1)) != WString::npos)
                {
                Pos++;
                *po_pPWCookie = URL.substr(Pos, EndPos - Pos);

                // remove '&'
                URL.erase(Pos - 1, EndPos - Pos + 1);
                }
            else
                {
                *po_pPWCookie = URL.substr(Pos + 1);
                URL.erase(Pos);
                }

            bool  UTF8URL    = m_pURL->IsUTF8URL();
            bool  EncodedURL = m_pURL->IsEncodedURL();

            if (((HFCPtr<HFCURLCommonInternet>&)GetURL())->GetUTF8EscapedURLPath().empty() == false)
                {
                UTF8EscapedURLPath = ((HFCPtr<HFCURLCommonInternet>&)GetURL())->
                                     GetUTF8EscapedURLPath();
                }

            m_pURL = HFCURL::Instanciate(URL);

            HASSERT(m_pURL != 0);

            m_pURL->SetUTF8URL(UTF8URL);
            m_pURL->SetEncodedURL(EncodedURL);
            }
        else
            Pos++;
        }

    //Remove the cookie from the UT8 escaped URL if any.
    if (UTF8EscapedURLPath.empty() == false)
        {
        HAutoPtr<string> pUTF8EscapedURLPath(new string(((HFCPtr<HFCURLCommonInternet>&)GetURL())->
                                                        GetUTF8EscapedURLPath()));

        Pos = 0;
        while (po_pPWCookieUTF8Escaped->empty() && (Pos = pUTF8EscapedURLPath->find('&', Pos)) != string::npos)
            {
            if (BeStringUtilities::Strnicmp(pUTF8EscapedURLPath->substr(Pos).c_str(), "&PW=", 4) == 0)
                {
                // the PW cookies is found
                string::size_type EndPos;
                //string::size_type CookieSize = string::npos;
                if ((EndPos = UTF8EscapedURLPath.find("&", Pos + 1)) != string::npos)
                    {
                    Pos++;
                    *po_pPWCookieUTF8Escaped = pUTF8EscapedURLPath->substr(Pos, EndPos - Pos);

                    // remove '&'
                    UTF8EscapedURLPath.erase(Pos - 1, EndPos - Pos + 1);
                    }
                else
                    {
                    *po_pPWCookieUTF8Escaped = UTF8EscapedURLPath.substr(Pos + 1);
                    UTF8EscapedURLPath.erase(Pos);
                    }
                }
            else
                Pos++;
            }

        ((HFCPtr<HFCURLCommonInternet>&)m_pURL)->SetUTF8EscapedURLPath(&UTF8EscapedURLPath);
        }
    }

//-----------------------------------------------------------------------------
// Private
//
//-----------------------------------------------------------------------------
bool HRFInternetImagingFile::FindBestSupportingProtocol(bool pi_TryMultibyte)
    {
    bool IsProtocolFound = true;

    // Connect to the server, trying these protocols
    //
    //  HIP 1.6 UTF8
    //  HIP 1.6
    //  HIP 1.5 UTF8
    //  HIP 1.5
    //  HIP 1.4 UTF8
    //  HIP 1.4
    //  HIP 1.3 UTF8
    //  HIP 1.3
    //  HIP 1.2
    //  HIP 1.1
    //  HIP 1.0
    if (!TryHIPProtocol(s_HIP0160_UTF8)                            &&
        ((pi_TryMultibyte == false) || !TryHIPProtocol(s_HIP0160)) &&
        !TryHIPProtocol(s_HIP0150_UTF8) &&
        ((pi_TryMultibyte == false) || !TryHIPProtocol(s_HIP0150)) &&
        !TryHIPProtocol(s_HIP0140_UTF8) &&
        ((pi_TryMultibyte == false) || !TryHIPProtocol(s_HIP0140)) &&
        !TryHIPProtocol(s_HIP0130_UTF8) &&
        ((pi_TryMultibyte == false) || !TryHIPProtocol(s_HIP0130)) &&
        ((pi_TryMultibyte == false) || !TryHIPProtocol(s_HIP0120)) &&
        ((pi_TryMultibyte == false) || !TryHIPProtocol(s_HIP0110)) &&
        ((pi_TryMultibyte == false) || !TryHIPProtocol(s_HIP0100)))
        {
        //Default to IIP protocol if the server doesn't support the HIP protocol
        /*Desactivated
        if (pi_TryMultibyte == true)
        {
            m_CurrentProtocol = s_IIP0100;
        }
        */

        IsProtocolFound = false;
        }

    return IsProtocolFound;
    }

//-----------------------------------------------------------------------------
// Private
//
//-----------------------------------------------------------------------------
void HRFInternetImagingFile::BuildDescriptors()
    {
    // Build the array of tileID descriptors
    m_pTileDescriptors = new HGFTileIDDescriptor[m_ResolutionCount];

    // Validation of codecList
    // Must have a minimun of one entry
    if (m_pCodec == NULL)
        m_pCodec = new HCDCodecIdentity();

    // Instantiation of Resolution descriptors
    HRFPageDescriptor::ListOfResolutionDescriptor  ListOfResolutionDescriptor;
    for (unsigned short Res = 0; Res < m_ResolutionCount; Res++)
        {
        // COnvert the res to IIP res
        unsigned short IIPRes = m_ResolutionCount - Res - 1;

        // Build the descriptor for the current resolution
        HFCPtr<HRFResolutionDescriptor> pResolution =
            new HRFResolutionDescriptor(HFC_READ_ONLY,                                 // AccessMode,
                                        GetCapabilities(),                             // Capabilities,
                                        (1.0 / pow(2.0, (double)Res)),                // XResolutionRatio,
                                        (1.0 / pow(2.0, (double)Res)),                // YResolutionRatio,
                                        m_pPixelType[IIPRes],                          // PixelType,
                                        m_pCodec,                                      // Codecs,
                                        HRFBlockAccess::RANDOM,                        // RStorageAccess,
                                        HRFBlockAccess::RANDOM,                        // WStorageAccess,
                                        HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL, // ScanLineOrientation,
                                        HRFInterleaveType::PIXEL,                      // InterleaveType
                                        false,                                         // IsInterlace,
                                        m_ImageWidth[IIPRes],                          // Width,
                                        m_ImageHeight[IIPRes],                         // Height,
                                        m_TileWidth[IIPRes],                           // BlockWidth,
                                        m_TileHeight[IIPRes],                          // BlockHeight,
                                        0,                                             // BlocksDataFlag
                                        HRFBlockType::TILE);                                         // Storage Type

        ListOfResolutionDescriptor.push_back(pResolution);

        // Build the tileID descriptor for this resolution
        m_pTileDescriptors[IIPRes] = HGFTileIDDescriptor(m_ImageWidth [IIPRes],
                                                         m_ImageHeight[IIPRes],
                                                         m_TileWidth  [IIPRes],
                                                         m_TileHeight [IIPRes]);
        }

    // Make the ClipShape if present
    HFCPtr<HRFClipShape> pClipShape;
    if (m_pClipShape != 0)
        pClipShape = ImportShapeFromArrayOfDouble(m_pClipShape, m_ClipShapeSize);


    // Background Tag
    if (m_HasBackground)
        {
        // Create the background attribute
        HFCPtr<HPMGenericAttribute>
        pTag(new HRFAttributeBackground(m_Background));

        // add it to the set
        m_pAttributes->Set(pTag);
        }

    // Build the descriptor for the page
    HFCPtr<HRFPageDescriptor> pPage;
    pPage = new HRFPageDescriptor (HFC_READ_ONLY,
                                   GetCapabilities(),           // Capabilities,
                                   ListOfResolutionDescriptor,  // ResolutionDescriptor,
                                   0,                           // RepresentativePalette,
                                   m_pHistogram,                // Histogram,
                                   0,                           // Thumbnail,
                                   pClipShape,                  // ClipShape,
                                   m_pTransfoModel,             // TransfoModel,
                                   0,                           // Filters
                                   m_pAttributes,               // Tag
                                   0,                           // Duration
                                   false,                       // resizable
                                   m_IsUnlimitedResolution);    // UnlimitedResolution

    if (m_pListOfMetaDataContainers->GetNbContainers() > 0)
        {
        pPage->SetListOfMetaDataContainer(m_pListOfMetaDataContainers);
        }

    m_ListOfPageDescriptor.push_back(pPage);

    // now, the page descriptor can be use
    m_PageDescriptorAvailable.Signal();

    // Clear the attributes now.  If needed, the ones in the page will be user
    m_pAttributes->Clear();
    }

//-----------------------------------------------------------------------------
// Private
// Sends the prepares request and wait for the result
//-----------------------------------------------------------------------------
bool HRFInternetImagingFile::DownloadAttributes(const string& pi_rRequest)
    {
    HPRECONDITION(!HasLocalCachedFile());
    HPRECONDITION(IsHMRImagingProtocol(m_Version));
    HPRECONDITION(CountPages() > 0);
    HPRECONDITION(GetPageDescriptor(0) != 0);
    HPRECONDITION(pi_rRequest.size() > 0);
    bool Result = false;

    // reset the waiting event.
    m_AttributesEvent.Reset();

    try
        {
        // send the request
        m_pConnection->Send((const Byte*)pi_rRequest.data(), pi_rRequest.size());

        // wait for the histo to arrive or an exception to be thrown
        WaitForEvent(m_AttributesEvent);

        Result = true;
        }
    catch(...)
        {
        Result = false;
        }

    return (Result);
    }

//-----------------------------------------------------------------------------
// Private
//
//-----------------------------------------------------------------------------
void HRFInternetImagingFile::ConnectToHost(const HFCPtr<HFCURL>& pi_rpURL)
    {
    HPRECONDITION(!HasLocalCachedFile());
    HPRECONDITION(pi_rpURL != 0);
    //HFCMonitor Monitor(m_ConnectionKey); // caller must have claimed it

    // if not already connected, connect
    if ( (m_pConnection == 0) || (!m_pConnection->IsConnected()) )
        {
        // Build the connection (depending on URL scheme)
        if ((pi_rpURL->GetSchemeType() == HFCURLHTTP::s_SchemeName()) ||
            (pi_rpURL->GetSchemeType() == HFCURLHTTPS::s_SchemeName()))
            {
            m_pConnection = new HRFHTTPConnection((const HRFURLInternetImagingHTTP&)*pi_rpURL);
            m_HTTPConnection = true;
            }
        else if (pi_rpURL->GetSchemeType() == HFCURLInternetImagingSocket::s_SchemeName())
            {
            m_pConnection = new HRFSocketConnection((const HFCURLInternetImagingSocket&)*pi_rpURL);
            m_HTTPConnection = false;
            }
        HASSERT(m_pConnection.get() != 0);
        m_pConnection->SetTimeOut(INT_MAX);

        // Connect to the server
        uint32_t connectTimeOut = ImagePP::ImageppLib::GetHost().GetImageppLibAdmin()._GetInternetImagingConnectionTimeOut();
        if (!m_pConnection->Connect(WString(L""), WString(L""), connectTimeOut))
            throw HRFInternetImagingConnectionException(pi_rpURL->GetURL(), HRFInternetImagingConnectionException::CANNOT_CONNECT);
        }
    }

//-----------------------------------------------------------------------------
// Private
//
//-----------------------------------------------------------------------------
bool HRFInternetImagingFile::TryHIPProtocol(const HFCVersion& pi_rVersion)
    {
    HPRECONDITION(!HasLocalCachedFile());

    bool IsProtocolSupported = false;
    bool UseUTF8Protocol     = false;

    // first, check if the server support the specific protocol
    if (m_IsConstructing)
        {
        // Setup for HIP
        SetupForHIP(UseUTF8Protocol);

        m_CurrentProtocol = pi_rVersion;
        ostringstream TheRequest;

        TheRequest << "obj=hip,";
        TheRequest << pi_rVersion.GetNumber(0);
        TheRequest << ".";
        TheRequest << pi_rVersion.GetNumber(1);
        if (pi_rVersion.GetNumber(2) == HRFInternetImagingFile::s_HIP_UTF8_Protocol)
            {
            TheRequest << ",utf8";
            UseUTF8Protocol = true;
            }

        // Set the state to CONNECTING_HIP
        InitializeState(ESTABLISH_PROTOCOL);

        // Send the connection string
            {
            HFCMonitor Monitor(m_ConnectionKey);
            m_pConnection->Send((const Byte*)(TheRequest.str().data()), TheRequest.str().size());
            }

        // Wait until the thread tell us if it's OK or not with this protocol
        WaitForEvent(m_StateEvent);

        if ((m_State == CONNECTED) || (m_State == RECONNECTING))
            {
            IsProtocolSupported = true;
            }
        }

    return IsProtocolSupported;
    }

//-----------------------------------------------------------------------------
// Private
//
//-----------------------------------------------------------------------------
bool HRFInternetImagingFile::RequestFileWithHIPProtocol(const HFCPtr<HFCURL>& pi_rpURL)
    {
    HPRECONDITION((m_State == CONNECTED) || (m_State == RECONNECTING));

    bool Result = false;

    // Build the connection request
    ostringstream TheRequest;

    TheRequest << "fif=";
    TheRequest << m_EncodedImageNameForRequest;
    //TheRequest << "&obj=hip-redirect"; The server`s handler for this command does
    //nothing, so it is useless to send it.
    TheRequest << "&obj=hip,";
    TheRequest << m_CurrentProtocol.GetNumber(0);
    TheRequest << ".";
    TheRequest << m_CurrentProtocol.GetNumber(1);
    if (m_CurrentProtocol.GetNumber(2) == HRFInternetImagingFile::s_HIP_UTF8_Protocol)
        {
        TheRequest << ",utf8";
        }

    // multi page is supported with HIP 1.5
    if ((m_PageID != 0) &&
        (m_CurrentProtocol.GetNumber(0) >= 1) &&
        (m_CurrentProtocol.GetNumber(1) >= 5))
        {
        TheRequest << "&hip-page=" << m_PageID;
        }

    if (!m_EncodedPWCookieForRequest.empty())
        TheRequest << "&" << m_EncodedPWCookieForRequest;

    try
        {
        // Connecting for the first time
        if (m_IsConstructing)
            {
            // Set the state to CONNECTING_HIP
            InitializeState(CONNECTING);

            // Send the connection string
                {
                HFCMonitor Monitor(m_ConnectionKey);
                m_pConnection->Send((const Byte*)TheRequest.str().data(),
                                    TheRequest.str().size());
                }

            // Wait until the thread tell us if it's OK or not with this protocol
            WaitForEvent(m_StateEvent);
            if (Result = ((m_State == CONNECTED) || (m_State == CONNECTING_HIP)))
                {
                HFCMonitor Monitor(m_ConnectionKey);

                // if the connection is HTTP, set the file name to use for
                // further requests
                if (m_pConnection->GetClassID() == HRFHTTPConnection::CLASS_ID)
                    {
                    static_cast<HRFHTTPConnection*>(m_pConnection.get())->SetSearchBase(TheRequest.str());
                    }

                // Request the compression group info for JPEG.
                m_pConnection->Send((const Byte*)s_CompGroupRequest.data(), s_CompGroupRequest.size());
                }
            }
        // Reconnection
        else
            {
            HFCMonitor Monitor(m_ConnectionKey);

            // For a HTTP connection, set the file name to use for further requests.
            if (m_pConnection->GetClassID() == HRFHTTPConnection::CLASS_ID)
                {
                static_cast<HRFHTTPConnection*>(m_pConnection.get())->SetSearchBase(TheRequest.str());
                }
            else
                {
                // Send a dummy request just to create the reconnection and
                // be redirected if the host is a dispatcher
                TheRequest << "&obj=aspect-ratio";

                m_pConnection->Send((const Byte*)(TheRequest.str().data()), TheRequest.str().size());
                }

            // Set the state to CONNECTED
            SetState(CONNECTED);
            Result = true;
            }
        }
    catch(HRFInternetImagingServerException& IISException)
        {
        // If we're constructing and there is a file error, it means that
        // the file either is corrupted, does not exist or is unsuported, so
        // rethrow to get out of the connection loop in the constructor.
        if ((m_IsConstructing) && (IISException.GetClass() == IIP_ERROR_CLASS_FILE))
            throw;

        // If we're constructing and there is a server dinied error, it means that
        // we do not have access to the server, so do not push it
        if ((m_IsConstructing) &&
            (IISException.GetClass() == IIP_ERROR_CLASS_SERVER) &&
            (IISException.GetNumber() == IIP_SERVER_ERROR_DENIED))
            throw;

        Result = false;
        }
    catch(HFCInternetConnectionException&)
        {
        // Close the connection
        HFCMonitor Monitor(m_ConnectionKey);
        m_pDisconnectorThread->AddConnection(m_pConnection.release());
        throw;
        }

    return Result;
    }

//-----------------------------------------------------------------------------
// Private
//
//-----------------------------------------------------------------------------
bool HRFInternetImagingFile::RequestFileWithIIPProtocol(const HFCPtr<HFCURL>& pi_rpURL)
    {
    HPRECONDITION(!HasLocalCachedFile());
    HPRECONDITION(pi_rpURL != 0);
    bool      Result = false;

    // Build the connection request
    string Request("fif=");
    Request += m_EncodedImageNameForRequest;
    Request += "&obj=iip,1.0";

    if (!m_EncodedPWCookieForRequest.empty())
        Request += "&" + m_EncodedPWCookieForRequest;

    try
        {
        // Connecting for the first time
        if (m_IsConstructing)
            {
            // Setup for IIP
            SetupForIIP();

            // Set the state to CONNECTING_IIP
            SetState(CONNECTING);
            m_StateEvent.Reset();

            // Send the connection string
                {
                HFCMonitor Monitor(m_ConnectionKey);
                // iip doesn't support utf8
                m_pConnection->Send((const Byte*)Request.data(), Request.size());
                }

            // Wait until the thread tell us if it's OK or not with this protocol
            WaitForEvent(m_StateEvent);
            if (Result = ((m_State == CONNECTED)) || (m_State == CONNECTING_IIP))
                {
                HFCMonitor Monitor(m_ConnectionKey);

                // if the conneciton is HTTP, set the file name to use for
                // further requests
                if (m_pConnection->GetClassID() == HRFHTTPConnection::CLASS_ID)
                    {
                    static_cast<HRFHTTPConnection*>(m_pConnection.get())->SetSearchBase(Request);
                    }

                // Request the compression group info for JPEG.
                m_pConnection->Send((const Byte*)s_CompGroupRequest.data(), s_CompGroupRequest.size());
                }
            }
        // Reconnection
        else
            {
            HFCMonitor Monitor(m_ConnectionKey);

            // For a HTTP connection, set the file name to use for further requests.
            if (m_pConnection->GetClassID() == HRFHTTPConnection::CLASS_ID)
                {
                static_cast<HRFHTTPConnection*>(m_pConnection.get())->SetSearchBase(Request);
                }
            else
                {
                // Send a dummy request just to create the reconnection and
                // be redirected if the host is a dispatcher
                Request += "&obj=aspect-ratio";

                // iip doesn't support utf8
                m_pConnection->Send((const Byte*)Request.data(), Request.size());
                }

            // Set the state to CONNECTED
            SetState(CONNECTED);
            Result = true;
            }
        }
    catch(HRFInternetImagingServerException& IISException)
        {
        // If we're constructing and there is a file error, it means that
        // the file either is corrupted, does not exist or is unsuported, so
        // rethrow to get out of the connection loop in the constructor.
        if ((m_IsConstructing) && (IISException.GetClass() == IIP_ERROR_CLASS_FILE))
            throw;

        // If we're constructing and there is a server dinied error, it means that
        // we do not have access to the server, so do not push it
        if ((m_IsConstructing) &&
            (IISException.GetClass() == IIP_ERROR_CLASS_SERVER) &&
            (IISException.GetNumber() == IIP_SERVER_ERROR_DENIED))
            throw;

        Result = false;
        }
    catch(HFCInternetConnectionException&)
        {
        // Close the connection
        HFCMonitor Monitor(m_ConnectionKey);
        m_pDisconnectorThread->AddConnection(m_pConnection.release());
        throw;
        }

    return (Result);
    }

//-----------------------------------------------------------------------------
// Private
//
//-----------------------------------------------------------------------------
void HRFInternetImagingFile::SetupForHIP(bool pi_UseUTF8Protocol)
    {
    // Set the thread handlers to HIP
    m_pThread->ClearHandlers();
    m_pThread->AddHandlers(s_HIPHandlers.GetHandlerList());

    // Reset all events
    ResetValues();
    }


//-----------------------------------------------------------------------------
// Private
//
//-----------------------------------------------------------------------------
void HRFInternetImagingFile::SetupForIIP()
    {
    HFCMonitor Monitor(m_DataKey);

    // set the handlers for IIP
    m_pThread->ClearHandlers();
    m_pThread->AddHandlers(s_IIPHandlers.GetHandlerList());

    // Reset all events
    ResetValues();
    }



//-----------------------------------------------------------------------------
// Private
//
//-----------------------------------------------------------------------------
void HRFInternetImagingFile::ResetValues()
    {
    HFCMonitor Monitor(m_DataKey);

    // Reset the res count
    m_ResolutionCount = 0;

    // Reset resolution dependant values
    for (unsigned short Res = 0; Res < HRF_INTERNET_MAXIMUM_RESOLUTION; Res++)
        {
        m_TileWidth[Res]    = ULONG_MAX;
        m_TileHeight[Res]   = ULONG_MAX;
        m_ImageWidth[Res]   = ULONG_MAX;
        m_ImageHeight[Res]  = ULONG_MAX;
        m_pPixelType[Res]   = 0;
        }

    // reset page
    m_MultiPageSupported = false;
    m_PageCount = 0;

    // Reset histogram
    m_pHistogram = 0;

    // Reset background
    m_HasBackground = false;
    m_Background    = 0;

    // Reset the tranfo model
    m_pTransfoModel = new HGF2DIdentity();

    // Reset the list of codes
    m_pCodec = NULL;

    // Reset JPEG Tables
    m_JpegTables.clear();

    // Clear time stamp
    m_TimeStamp = "";

    // Reset World ID
    m_WorldID = HGF2DWorld_UNKNOWNWORLD;

    // Clear the version
    m_Version = HFCVersion(WString(L""), WString(L""), 0);

    // Clear the ROI
    m_ROI = HGF2DLiteExtent();

    // Clear Shape
    m_pClipShape = 0;

    // Clear Creation Time
    m_CreationTime = 0;

    // Clear Modify Time
    m_ModifyTime = 0;

    // Clear Access Time
    m_AccessTime = 0;

    // Clear File Size
    m_FileSize = 0;

    // Clear local cached file
    m_pLocalCachedFile = 0;

    // Clear the original file type
    m_OriginalFileType = L"";
    m_OriginalFileTypeEvent.Reset();
    }


//-----------------------------------------------------------------------------
// Private
//
//-----------------------------------------------------------------------------
bool HRFInternetImagingFile::VerifyProtocolVersion() const
    {
    bool Result = false;

    if (m_CurrentProtocol.GetNumberCount() == m_Version.GetNumberCount())
        {
        Result = true;
        for (uint32_t i = 0; i < m_Version.GetNumberCount() && Result; i++)
            Result = (m_CurrentProtocol.GetNumber(i) == m_Version.GetNumber(i));
        }

    return Result;
    }

//-----------------------------------------------------------------------------
// Private
//
//-----------------------------------------------------------------------------
bool HRFInternetImagingFile::VerifyHIPBasicInfo()
    {
    HFCMonitor Monitor(m_DataKey);
    HPRECONDITION(!HasLocalCachedFile());
    bool Result = false;

    // Check if we at least have the resolution
    if (m_ResolutionCount > 0)
        {
        // Suppose to be ok
        Result = true;

        // Check to see if we have all the image & tile sizes plus the pixel types
        for (unsigned short Res = 0; (Result) && (Res < m_ResolutionCount); Res++)
            {
            Result = ((m_TileWidth[Res]   != ULONG_MAX) &&
                      (m_TileHeight[Res]  != ULONG_MAX) &&
                      (m_ImageWidth[Res]  != ULONG_MAX) &&
                      (m_ImageHeight[Res] != ULONG_MAX) &&
                      (m_pPixelType[Res]  != 0) );
            }

        // If the resolution data is ok, then continue verifying
        Result = Result && (m_Version.GetNumberCount() == 3 &&
                            (m_Version.GetNumber(2) == HIP_PROTOCOL || m_Version.GetNumber(2) == HIP_UTF8_PROTOCOL));
        }

    return (Result);
    }


//-----------------------------------------------------------------------------
// Private
//
//-----------------------------------------------------------------------------
bool HRFInternetImagingFile::VerifyIIPBasicInfo()
    {
    HFCMonitor Monitor(m_DataKey);
    HPRECONDITION(!HasLocalCachedFile());
    bool Result = false;

    // Check if we at least have the resolution
    if (m_ResolutionCount > 0)
        {
        // Suppose to be ok
        Result = true;

        // Check to see if we have all the image & tile sizes plus the pixel types
        for (unsigned short Res = 0; (Result) && (Res < m_ResolutionCount); Res++)
            {
            Result = ((m_TileWidth[Res]   != 0) &&
                      (m_TileHeight[Res]  != 0) &&
                      (m_ImageWidth[Res]  != 0) &&
                      (m_ImageHeight[Res] != 0) &&
                      (m_pPixelType[Res]  != 0) );
            }

        // If the resolution data is ok, then continue verifying
        Result = Result && IsInternetImagingProtocol(m_Version);
        }

    return (Result);
    }


//-----------------------------------------------------------------------------
// Private
//
//-----------------------------------------------------------------------------
HFCPtr<HRFInternetImagingFile> HRFInternetImagingFile::GetPage(uint32_t pi_Page) const
    {
    HPRECONDITION(m_PageID == 0 || pi_Page == m_PageID);

    HFCPtr<HRFInternetImagingFile> pResult;


    if (pi_Page == m_PageID)
        pResult = const_cast<HRFInternetImagingFile*>(this);
    else
        {
        PageList::iterator Itr(m_Pages.find(pi_Page));
        if (Itr == m_Pages.end())
            {
            pResult = new HRFInternetImagingFile(this, pi_Page);
            HPRECONDITION(pResult != 0);

            m_Pages.insert(PageList::value_type(pi_Page, pResult));
            }
        else
            pResult = Itr->second;
        }

    HPOSTCONDITION(pResult != 0);
    return pResult;
    }


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Creators
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
HRFInternetImagingFileCreator::HRFInternetImagingFileCreator()
    : HRFRasterFileCreator(HRFInternetImagingFile::CLASS_ID),
      m_Label(L"Internet Imaging File (IIP/HIP)"),
      m_Schemes(HFCURLHTTP::s_SchemeName()  + static_cast<WString>(L";") +
                HFCURLHTTPS::s_SchemeName() + static_cast<WString>(L";") +
                HFCURLInternetImagingSocket::s_SchemeName())
    {
    }


//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
WString HRFInternetImagingFileCreator::GetLabel() const
    {
    return m_Label;
    }


//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
WString HRFInternetImagingFileCreator::GetSchemes() const
    {
    return m_Schemes;
    }

//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
WString HRFInternetImagingFileCreator::GetExtensions() const
    {
    return m_Extensions;
    }


//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFInternetImagingFileCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFInternetImagingFileCapabilities();

    return m_pCapabilities;
    }


//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFInternetImagingFileCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                                            HFCAccessMode         pi_AccessMode,
                                                            uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);
    HPRECONDITION(IsKindOfFile(pi_rpURL));
    HPRECONDITION(!pi_AccessMode.m_HasWriteAccess);
    HPRECONDITION(!pi_AccessMode.m_HasCreateAccess);
    HPRECONDITION(pi_Offset == 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile>
    pFile = new HRFInternetImagingFile(pi_rpURL,
                                       pi_AccessMode,
                                       pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }

//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
bool HRFInternetImagingFileCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                                  uint64_t             pi_Offset) const
    {

    return  (HRFURLInternetImagingHTTP::IsURLInternetImaging(pi_rpURL) == true) ||
            (pi_rpURL->GetSchemeType() == HFCURLInternetImagingSocket::s_SchemeName());
    }

//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
bool HRFInternetImagingFileCreator::SupportsURL(const HFCPtr<HFCURL>& pi_rpURL) const
    {
    return IsKindOfFile(pi_rpURL, 0);
    }


//-----------------------------------------------------------------------------
// HRFInternetImagingFileBlockCapabilities
//-----------------------------------------------------------------------------
class HRFInternetImagingFileBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFInternetImagingFileBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Tile Capability
        Add(new HRFTileCapability(HFC_READ_ONLY,  // AccessMode
                                  LONG_MAX,       // MaxSizeInBytes
                                  64,             // MinWidth
                                  256,            // MaxWidth
                                  8,              // WidthIncrement
                                  64,             // MinHeight
                                  256,            // MaxHeight
                                  8));            // HeightIncrement
        }
    };

//-----------------------------------------------------------------------------
// HRFInternetImagingFileCodecCapabilities
//-----------------------------------------------------------------------------
class HRFInternetImagingFileCodecCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFInternetImagingFileCodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec IJG (Jpeg)
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecIJG::CLASS_ID,
                                   new HRFInternetImagingFileBlockCapabilities()));

        // Codec JPEG Alpha
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecJPEGAlpha::CLASS_ID,
                                   new HRFInternetImagingFileBlockCapabilities()));

        // Codec Zlib (Deflate)
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFInternetImagingFileBlockCapabilities()));
        // Codec Flashpix
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecFlashpix::CLASS_ID,
                                   new HRFInternetImagingFileBlockCapabilities()));
        // Codec FPX Single Color
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecFPXSingleColor::CLASS_ID,
                                   new HRFInternetImagingFileBlockCapabilities()));
        // Codec HMR RLE1
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecHMRRLE1::CLASS_ID,
                                   new HRFInternetImagingFileBlockCapabilities()));
        // Codec HMR CCITT
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecHMRCCITT::CLASS_ID,
                                   new HRFInternetImagingFileBlockCapabilities()));
        // Codec HMR PackBits
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecHMRPackBits::CLASS_ID,
                                   new HRFInternetImagingFileBlockCapabilities()));
        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFInternetImagingFileBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFInternetImagingFileCapabilities
//-----------------------------------------------------------------------------
HRFInternetImagingFileCapabilities::HRFInternetImagingFileCapabilities()
    : HRFRasterFileCapabilities()
    {
    // Pixel Type

    // Pixel Types
    HRPListPixelTypePtrs PixelTypes;
    HRPListPixelTypePtrs::ListPixelTypePtrs::const_iterator Itr(PixelTypes.m_List.begin());
    while (Itr != PixelTypes.m_List.end())
        {
        HFCPtr<HRFPixelTypeCapability> pPixelType;
        pPixelType = new HRFPixelTypeCapability(HFC_READ_ONLY,
                                                ((HRPPixelType*)(*Itr))->GetClassID(),
                                                new HRFInternetImagingFileCodecCapabilities());
        Add((HFCPtr<HRFCapability>)pPixelType);

        Itr++;
        }

    // Geocoding capability
    HFCPtr<HRFGeocodingCapability> pGeocodingCapability;

    pGeocodingCapability = new HRFGeocodingCapability(HFC_READ_ONLY);

    pGeocodingCapability->AddSupportedKey(GTModelType);
    pGeocodingCapability->AddSupportedKey(GTRasterType);
    pGeocodingCapability->AddSupportedKey(PCSCitation);
    pGeocodingCapability->AddSupportedKey(ProjectedCSType);
    pGeocodingCapability->AddSupportedKey(GTCitation);
    pGeocodingCapability->AddSupportedKey(Projection);
    pGeocodingCapability->AddSupportedKey(ProjCoordTrans);
    pGeocodingCapability->AddSupportedKey(ProjLinearUnits);
    pGeocodingCapability->AddSupportedKey(ProjLinearUnitSize);
    pGeocodingCapability->AddSupportedKey(GeographicType);
    pGeocodingCapability->AddSupportedKey(GeogCitation);
    pGeocodingCapability->AddSupportedKey(GeogGeodeticDatum);
    pGeocodingCapability->AddSupportedKey(GeogPrimeMeridian);
    pGeocodingCapability->AddSupportedKey(GeogLinearUnits);
    pGeocodingCapability->AddSupportedKey(GeogLinearUnitSize);
    pGeocodingCapability->AddSupportedKey(GeogAngularUnits);
    pGeocodingCapability->AddSupportedKey(GeogAngularUnitSize);
    pGeocodingCapability->AddSupportedKey(GeogEllipsoid);
    pGeocodingCapability->AddSupportedKey(GeogSemiMajorAxis);
    pGeocodingCapability->AddSupportedKey(GeogSemiMinorAxis);
    pGeocodingCapability->AddSupportedKey(GeogInvFlattening);
    pGeocodingCapability->AddSupportedKey(GeogAzimuthUnits);
    pGeocodingCapability->AddSupportedKey(GeogPrimeMeridianLong);
    pGeocodingCapability->AddSupportedKey(ProjStdParallel1);
    pGeocodingCapability->AddSupportedKey(ProjStdParallel2);
    pGeocodingCapability->AddSupportedKey(ProjNatOriginLong);
    pGeocodingCapability->AddSupportedKey(ProjNatOriginLat);
    pGeocodingCapability->AddSupportedKey(ProjFalseEasting);
    pGeocodingCapability->AddSupportedKey(ProjFalseNorthing);
    pGeocodingCapability->AddSupportedKey(ProjFalseOriginLong);
    pGeocodingCapability->AddSupportedKey(ProjFalseOriginLat);
    pGeocodingCapability->AddSupportedKey(ProjFalseOriginEasting);
    pGeocodingCapability->AddSupportedKey(ProjFalseOriginNorthing);
    pGeocodingCapability->AddSupportedKey(ProjCenterLong);
    pGeocodingCapability->AddSupportedKey(ProjCenterLat);
    pGeocodingCapability->AddSupportedKey(ProjCenterEasting);
    pGeocodingCapability->AddSupportedKey(ProjCenterNorthing);
    pGeocodingCapability->AddSupportedKey(ProjScaleAtNatOrigin);
    pGeocodingCapability->AddSupportedKey(ProjScaleAtCenter);
    pGeocodingCapability->AddSupportedKey(ProjAzimuthAngle);
    pGeocodingCapability->AddSupportedKey(ProjRectifiedGridAngle);
    pGeocodingCapability->AddSupportedKey(ProjStraightVertPoleLong);
    pGeocodingCapability->AddSupportedKey(VerticalCSType);
    pGeocodingCapability->AddSupportedKey(VerticalCitation);
    pGeocodingCapability->AddSupportedKey(VerticalDatum);
    pGeocodingCapability->AddSupportedKey(VerticalUnits);

    Add((HFCPtr<HRFCapability>&)pGeocodingCapability);

    // Transfo Model
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DIdentity::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DStretch::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DAffine::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DSimilitude::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DProjective::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DTranslation::CLASS_ID));

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::UPPER_LEFT_VERTICAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::UPPER_RIGHT_VERTICAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::LOWER_LEFT_VERTICAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::LOWER_RIGHT_VERTICAL));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_ONLY, HRFInterleaveType::PIXEL));

    // Single resolution capability
    Add(new HRFSingleResolutionCapability(HFC_READ_ONLY));

    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_ONLY));

    // MultiResolution Capability
    HFCPtr<HRFCapability> pMultiResolutionCapability = new HRFMultiResolutionCapability(
        HFC_READ_ONLY,  // AccessMode,
        true,           // SinglePixelType,
        true,           // SingleStorageType,
        false,          // ArbitaryXRatio,
        false,          // ArbitaryYRatio
        true,           // XYRatioLocked (default value)
        256,            // SmallestResWidth (default value)
        256,            // SmallestResHeight (default value)
        ULONG_MAX,     // BiggestResWidth (default value)
        ULONG_MAX,     // BiggestResHeight (default value)
        true);          // UnlimitedResolution

    Add(pMultiResolutionCapability);

    // Tag capability. READ_WRITE because the attribute handler will set tags in the page
    // descriptor as they arrive.
    Add(new HRFUniversalTagCapability(HFC_READ_WRITE));

    // Shape capability
    Add(new HRFClipShapeCapability(HFC_READ_ONLY, HRFCoordinateType::PHYSICAL));

    // Histogram capability
    Add(new HRFHistogramCapability(HFC_READ_ONLY));
    }



//-----------------------------------------------------------------------------
// Internet Imaging HFCStat Implementation Declaration
//-----------------------------------------------------------------------------

class HRFInternetImagingStatImpl : public HFCStatImpl
    {
public:
    //--------------------------------------
    // Construction / Destruction
    //--------------------------------------

    HRFInternetImagingStatImpl();
    virtual             ~HRFInternetImagingStatImpl();


    //--------------------------------------
    // Methods
    //--------------------------------------

    // Indicates if an impl can handle an URL
    virtual bool       CanHandle(const HFCURL& pi_rURL) const override;

    // Resource time
    virtual time_t      GetCreationTime     (const HFCURL& pi_rURL) const override;
    virtual time_t      GetLastAccessTime   (const HFCURL& pi_rURL) const override;
    virtual time_t      GetModificationTime (const HFCURL& pi_rURL) const override;
    virtual void        SetModificationTime (const HFCURL& pi_rURL,
                                             time_t        pi_NewTime) const override;

    // Resource size
    virtual uint64_t   GetSize(const HFCURL& pi_rURL) const override;

    HFCStat::AccessStatus DetectAccess(const HFCURL& pi_rURL) const override;

    // Resource access mode
    virtual HFCAccessMode
    GetAccessMode(const HFCURL& pi_rURL) const override;
    };

//-----------------------------------------------------------------------------
// Instantiate the file implementation
//-----------------------------------------------------------------------------

static HRFInternetImagingStatImpl      s_FileImpl;


//-----------------------------------------------------------------------------
// Internet Imaging HFCStat Implementation Definition
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HRFInternetImagingStatImpl::HRFInternetImagingStatImpl()
    {
    RegisterImpl(this);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HRFInternetImagingStatImpl::~HRFInternetImagingStatImpl()
    {
    }


//-----------------------------------------------------------------------------
// Public
// Indicates if an impl can handle an URL
//-----------------------------------------------------------------------------
bool HRFInternetImagingStatImpl::CanHandle(const HFCURL& pi_rURL) const
    {
    return (HRFURLInternetImagingHTTP::IsURLInternetImaging(&pi_rURL) == true) ||
           (pi_rURL.GetSchemeType() == HFCURLInternetImagingSocket::s_SchemeName());
    }


//-----------------------------------------------------------------------------
// Public
// Resource modification time
//-----------------------------------------------------------------------------
time_t HRFInternetImagingStatImpl::GetCreationTime(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));
    time_t Result = 0;

    try
        {
        // Copy the URL
        HFCPtr<HFCURL> pURL(HFCURL::Instanciate(pi_rURL.GetURL()));
        HASSERT(pURL != 0);

        // Create an internet imaging file to get its modify time
        Result = HRFInternetImagingFile(pURL).GetCreationTime();
        }
    catch(HRFInternetImagingException&)
        {
        throw;
        }
    catch(...)
        {
        }


    return (Result);
    }

//-----------------------------------------------------------------------------
// Public
// Resource modification time
//-----------------------------------------------------------------------------
time_t HRFInternetImagingStatImpl::GetLastAccessTime(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));
    time_t Result = 0;

    try
        {
        // Copy the URL
        HFCPtr<HFCURL> pURL(HFCURL::Instanciate(pi_rURL.GetURL()));
        HASSERT(pURL != 0);

        // Create an internet imaging file to get its modify time
        Result = HRFInternetImagingFile(pURL).GetLastAccessTime();
        }
    catch(HRFInternetImagingException&)
        {
        throw;
        }
    catch(...)
        {
        }


    return (Result);
    }

//-----------------------------------------------------------------------------
// Public
// Resource modification time
//-----------------------------------------------------------------------------
time_t HRFInternetImagingStatImpl::GetModificationTime(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));
    time_t Result = 0;

    try
        {
        // Copy the URL
        HFCPtr<HFCURL> pURL(HFCURL::Instanciate(pi_rURL.GetURL()));
        HASSERT(pURL != 0);

        // Create an internet imaging file to get its modify time
        Result = HRFInternetImagingFile(pURL).GetModificationTime();
        }
    catch(HRFInternetImagingException&)
        {
        throw;
        }
    catch(...)
        {
        }
    return (Result);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HRFInternetImagingStatImpl::SetModificationTime(const HFCURL& pi_rURL,
                                                     time_t         pi_NewTime) const
    {
    HPRECONDITION(CanHandle(pi_rURL));

    // Cannot so do nothing
    }

//-----------------------------------------------------------------------------
// Public
// Resource size
//-----------------------------------------------------------------------------
uint64_t HRFInternetImagingStatImpl::GetSize(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));
    uint64_t Result = 0;

    try
        {
        // Copy the URL
        HFCPtr<HFCURL> pURL(HFCURL::Instanciate(pi_rURL.GetURL()));
        HASSERT(pURL != 0);

        // Create an internet imaging file to get its file size
        Result = HRFInternetImagingFile(pURL).GetFileSize();
        }
    catch(HRFInternetImagingException&)
        {
        throw;
        }
    catch(...)
        {
        }
    return (Result);
    }

//-----------------------------------------------------------------------------
// Public
// Resource existence
//-----------------------------------------------------------------------------
HFCStat::AccessStatus HRFInternetImagingStatImpl::DetectAccess(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));

    return HFCStat::AccessGranted;
    }

//-----------------------------------------------------------------------------
// Public
// Resource access mode
//-----------------------------------------------------------------------------
HFCAccessMode HRFInternetImagingStatImpl::GetAccessMode(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));

    return (HFC_READ_ONLY);
    }

//-----------------------------------------------------------------------------
// Public
// GetKey
// Returns the exclusive key assigned to this object
//-----------------------------------------------------------------------------
HFCExclusiveKey& HRFInternetImagingFile::GetKey() const
    {
    if (m_pLocalCachedFile != 0)
        return (m_pLocalCachedFile->GetKey());
    else
        return (HRFRasterFile::GetKey());
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
bool HRFInternetImagingFile::PagesAreRasterFile() const
    {
    return true;
    }

//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFInternetImagingFile::GetPageFile(uint32_t pi_Page) const
    {
    HPRECONDITION(pi_Page < CountPages());

    return static_cast<HRFRasterFile*>(GetPage(pi_Page).GetPtr());
    }

//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
uint32_t HRFInternetImagingFile::CountPages() const
    {
    HFCMonitor Monitor(m_DataKey);

    uint32_t PageCount;
    if (m_MultiPageSupported)
        {
        HPRECONDITION(m_PageCount > 0); // we are in read-only, a valid HRF must have at least one page
        PageCount = m_PageCount;
        }
    else
        PageCount = HRFRasterFile::CountPages();

    return PageCount;
    }

//-----------------------------------------------------------------------------
// Public
//
// Each page was implemented with HRFInternetImagingFile.
//
//-----------------------------------------------------------------------------
HFCPtr<HRFPageDescriptor> HRFInternetImagingFile::GetPageDescriptor(uint32_t pi_Page) const
    {
    HFCMonitor Monitor(m_DataKey);

    HFCPtr<HRFPageDescriptor> pPage;

    if (m_MultiPageSupported)
        {
        HPRECONDITION(pi_Page == m_PageID || (m_PageID == 0 && pi_Page < m_PageCount));

        if (pi_Page == m_PageID)
            pPage = HRFRasterFile::GetPageDescriptor(0);
        else
            pPage = GetPage(pi_Page)->GetPageDescriptor(pi_Page);
        }
    else
        pPage = HRFRasterFile::GetPageDescriptor(pi_Page);

    HPOSTCONDITION(pPage != 0);
    return pPage;
    }



//-----------------------------------------------------------------------------
// Public
// Indicates if the Internet Imaging File is caching a local file
//-----------------------------------------------------------------------------
bool HRFInternetImagingFile::HasLocalCachedFile() const
    {
    return (m_pLocalCachedFile != 0);
    }



//-----------------------------------------------------------------------------
// Public
// Returns the time stamp obtain from the server
//-----------------------------------------------------------------------------
time_t HRFInternetImagingFile::GetCreationTime() const
    {
    HFCMonitor Monitor(const_cast<HRFInternetImagingFile*>(this)->m_DataKey);

    return (m_CreationTime);
    }

//-----------------------------------------------------------------------------
// Public
// Returns the time stamp obtain from the server
//-----------------------------------------------------------------------------
time_t HRFInternetImagingFile::GetLastAccessTime() const
    {
    HFCMonitor Monitor(const_cast<HRFInternetImagingFile*>(this)->m_DataKey);

    return (m_AccessTime);
    }

//-----------------------------------------------------------------------------
// Public
// Returns the time stamp obtain from the server
//-----------------------------------------------------------------------------
time_t HRFInternetImagingFile::GetModificationTime() const
    {
    HFCMonitor Monitor(const_cast<HRFInternetImagingFile*>(this)->m_DataKey);

    return (m_ModifyTime);
    }


//-----------------------------------------------------------------------------
// Public
// Returns the protocol in use with the current Internet image
//-----------------------------------------------------------------------------
const HFCVersion& HRFInternetImagingFile::GetProtocolVersion() const
    {
    HFCMonitor Monitor(const_cast<HRFInternetImagingFile*>(this)->m_DataKey);

    return (m_Version);
    }


//-----------------------------------------------------------------------------
// Public
// Returns the size fo the file on the server size
//-----------------------------------------------------------------------------
uint64_t HRFInternetImagingFile::GetFileSize() const
    {
    HFCMonitor Monitor(const_cast<HRFInternetImagingFile*>(this)->m_DataKey);

    return (m_FileSize);
    }


//-----------------------------------------------------------------------------
// Public
// SetWaitBlockOnRead
//
// Special method for VPR...
// if pi_WaitBlockOnRead is true, the editor will wait until the server send
// something, if false, ReadBlock will return H_NOT_FOUND if the tile data is
// not already arrived. By default, editors wait until the server send something.
//
// Note : This method must be called before creating editors.
//-----------------------------------------------------------------------------
void HRFInternetImagingFile::SetWaitBlockOnRead(bool pi_WaitBlockOnRead)
    {
    m_WaitTileOnRead = pi_WaitBlockOnRead;
    }


//-----------------------------------------------------------------------------
// Protected
// Set the state of the internet image and reset the event
//-----------------------------------------------------------------------------
void HRFInternetImagingFile::InitializeState(HRFInternetImagingState pi_State)
    {
    m_State = pi_State;
    m_StateEvent.Reset();
    }

//-----------------------------------------------------------------------------
// Protected
// Changes the state of the internet image and signals the change of state
//-----------------------------------------------------------------------------
void HRFInternetImagingFile::SetState(HRFInternetImagingState pi_State)
    {
    m_State = pi_State;
    m_StateEvent.Signal();
    }


//-----------------------------------------------------------------------------
// Protected
// Returns the current state of the internet image
//-----------------------------------------------------------------------------
HRFInternetImagingFile::HRFInternetImagingState
HRFInternetImagingFile::GetState() const
    {
    return (m_State);
    }

//-----------------------------------------------------------------------------
// private
// IsHMRImagingProtocol
//
// protocol version are defined with these values (see HRFInternetImagingFile.cpp)
// #define IPP_PROTOCOL        0
// #define HIP_PROTOCOL        1
// #define HIP_UTF8_PROTOCOL   2
//-----------------------------------------------------------------------------
bool HRFInternetImagingFile::IsHMRImagingProtocol(const HFCVersion& pi_rVersion) const
    {
    HPRECONDITION(pi_rVersion.GetNumberCount() == 3);

    return (pi_rVersion.GetNumber(2) == 1 || pi_rVersion.GetNumber(2) == 2);
    }

//-----------------------------------------------------------------------------
// private
// IsInternetImagingProtocol
//
// protocol version are defined with these values (see HRFInternetImagingFile.cpp)
// #define IPP_PROTOCOL        0
// #define HIP_PROTOCOL        1
// #define HIP_UTF8_PROTOCOL   2
//-----------------------------------------------------------------------------
bool HRFInternetImagingFile::IsInternetImagingProtocol(const HFCVersion& pi_rVersion) const
    {
    HPRECONDITION(pi_rVersion.GetNumberCount() == 3);

    return (pi_rVersion.GetNumber(2) == 0);
    }


//-----------------------------------------------------------------------------
// Protected
//
//-----------------------------------------------------------------------------
void HRFInternetImagingFile::ClearThreadException()
    {
    HFCMonitor Monitor(m_ThreadExceptionKey);

    m_pThreadException = 0;
    m_ThreadExceptionEvent.Reset();
    }

//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HRFInternetJpegTable::HRFInternetJpegTable()
    : m_Buffer(1)
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HRFInternetJpegTable::~HRFInternetJpegTable()
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HRFInternetJpegTable::SetData(const Byte* pi_pData, uint32_t pi_DataSize)
    {
    HPRECONDITION(pi_pData != 0);
    HPRECONDITION(pi_DataSize > 0);

    m_Buffer.Clear();
    m_Buffer.AddData(pi_pData, pi_DataSize);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
uint32_t HRFInternetJpegTable::GetDataSize() const
    {
    return (uint32_t)(m_Buffer.GetDataSize());
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
const Byte* HRFInternetJpegTable::GetData() const
    {
    HPRECONDITION(GetDataSize() > 0);

    return (m_Buffer.GetData());
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HRFInternetJpegTable::Clear()
    {
    m_Buffer.Clear();
    }

#if 0
/*
   #TR 250337, Raymond Gauthier, 23/07/2008:
   Merged the 3 Creators in in single one (HRFInternetImagingFileCreator) so that
   there is only one Creator associated with a class ID.
*/
//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
HRFInternetFileSocketCreator::HRFInternetFileSocketCreator()
    : HRFInternetFileCreator()
    {
    m_Schemes = HFCURLInternetImagingSocket::s_SchemeName();
    }


//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
bool HRFInternetFileSocketCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                                 uint64_t             pi_Offset) const
    {
    // if it as come to here and the URL scheme is IIP, let it through
    return (pi_rpURL->GetSchemeType() == HFCURLInternetImagingSocket::s_SchemeName());
    }

//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
bool HRFInternetFileSocketCreator::SupportsURL(const HFCPtr<HFCURL>& pi_rpURL) const
    {
    return IsKindOfFile(pi_rpURL, 0);
    }


//-----------------------------------------------------------------------------
// HRFInternetFileHTTPCreator class
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
HRFInternetFileHTTPCreator::HRFInternetFileHTTPCreator()
    : HRFInternetFileCreator()
    {
    m_Schemes = HFCURLHTTP::s_SchemeName();
    }


//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
bool HRFInternetFileHTTPCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                               uint64_t             pi_Offset) const
    {
    return HRFURLInternetImagingHTTP::IsURLInternetImaging(pi_rpURL);
    }

//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
bool HRFInternetFileHTTPCreator::SupportsURL(const HFCPtr<HFCURL>& pi_rpURL) const
    {
    return IsKindOfFile(pi_rpURL, 0);
    }


//-----------------------------------------------------------------------------
// HRFInternetFileHTTPSCreator class
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
HRFInternetFileHTTPSCreator::HRFInternetFileHTTPSCreator()
    : HRFInternetFileCreator()
    {
    m_Schemes = HFCURLHTTPS::s_SchemeName();
    }


//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
bool HRFInternetFileHTTPSCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                                uint64_t             pi_Offset) const
    {
    return HRFURLInternetImagingHTTPS::IsURLInternetImaging(pi_rpURL);
    }

//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
bool HRFInternetFileHTTPSCreator::SupportsURL(const HFCPtr<HFCURL>& pi_rpURL) const
    {
    return IsKindOfFile(pi_rpURL, 0);
    }

#endif