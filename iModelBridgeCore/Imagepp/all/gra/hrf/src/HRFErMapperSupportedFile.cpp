//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFErMapperSupportedFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFErMapperFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFErMapperSupportedFile.h>
#if defined(IPP_HAVE_ERMAPPER_SUPPORT) 

#include <ImagePP/all/h/ImageppLib.h>

#include <Imagepp/all/h/HCDCodecECW.h>
#include <Imagepp/all/h/HCDCodecJPEG2000.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCURLECWP.h>
#include <Imagepp/all/h/HFCURLECWPS.h>
#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DProjective.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HRFErMapperSupportedFileEditor.h>
#include <Imagepp/all/h/HRFException.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HRPChannelOrgRGB.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HTIFFTag.h>
#include <Imagepp/all/h/HVETileIDIterator.h>
#include <Imagepp/all/h/HFCStat.h>
#include <Imagepp/all/h/HRFURLInternetImagingHTTP.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>


// Includes from the ERMapper SDK
#include <ErdasEcwJpeg2000/NCSECWClient.h>
#include <ErdasEcwJpeg2000/NCSTypes.h>
#include <ErdasEcwJpeg2000/NCSECWCompressClient.h>



#define round(a) ((long)((a)<0.0?(a)-0.5:(a)+0.5))


//Ensure that the .objs related to these object are included in the final
//executable, otherwise, the global variables whose constructors
//register these schemes are never created.
HFCURLECWP  dummy;
HFCURLECWPS dummy2;

//-----------------------------------------------------------------------------
// HRFEcwBlockCapabilities
//-----------------------------------------------------------------------------
class HRFEcwBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFEcwBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Block Capability
        // Tile Capability
        Add(new HRFTileCapability(HFC_READ_CREATE,         // AccessMode
                                  LONG_MAX,            // MaxSizeInBytes
                                  1,                   // MinWidth
                                  LONG_MAX,            // MaxWidth
                                  0,                   // WidthIncrement
                                  1,                   // MinHeight
                                  LONG_MAX,            // MaxHeight
                                  0,                   // HeightIncrement
                                  true));              // IsSquare
        }
    };

//-----------------------------------------------------------------------------
// HRFEcwCodecIdentityCapabilities
//-----------------------------------------------------------------------------
class HRFEcwCodecCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFEcwCodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec
        Add(new HRFCodecCapability(HFC_READ_CREATE,
                                   HCDCodecECW::CLASS_ID,
                                   new HRFEcwBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFEcwCapabilities
//-----------------------------------------------------------------------------
HRFEcwCapabilities::HRFEcwCapabilities()
    : HRFRasterFileCapabilities()
    {
    Add(new HRFPixelTypeCapability(HFC_READ_CREATE,
                                   HRPPixelTypeV8Gray8::CLASS_ID,
                                   new HRFEcwCodecCapabilities()));

    Add(new HRFPixelTypeCapability(HFC_READ_CREATE,
                                   HRPPixelTypeV24R8G8B8::CLASS_ID,
                                   new HRFEcwCodecCapabilities()));

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_CREATE, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_CREATE, HRFInterleaveType::PIXEL));

    // MultiResolution Capability
    Add(new HRFMultiResolutionCapability(HFC_READ_CREATE));

    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_CREATE));

    // Maximum file size capability
    Add(new HRFMaxFileSizeCapability(HFC_READ_CREATE, (uint64_t)UINT64_MAX));

    // Transfo Model
    Add(new HRFTransfoModelCapability(HFC_READ_CREATE, HGF2DIdentity::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_CREATE, HGF2DTranslation::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_CREATE, HGF2DStretch::CLASS_ID));
#if 0 //DMx ECW SDK 5.0 support Geotiff Tag
    Add(new HRFTransfoModelCapability(HFC_READ_CREATE, HGF2DSimilitude::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_CREATE, HGF2DAffine::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_CREATE, HGF2DProjective::CLASS_ID));
#endif
    
    // Geocoding capability
    HFCPtr<HRFGeocodingCapability> pGeocodingCapability(new HRFGeocodingCapability(HFC_READ_CREATE));

    pGeocodingCapability->AddSupportedKey(GTModelType);
    pGeocodingCapability->AddSupportedKey(ProjectedCSType);
    pGeocodingCapability->AddSupportedKey(ProjectedCSTypeLong);
    pGeocodingCapability->AddSupportedKey(ProjLinearUnits);

    Add((HFCPtr<HRFCapability>&)pGeocodingCapability);
    }

///-----------------------------------------------------------------------------
// HRFEcw Stat Implementation Declaration
//-----------------------------------------------------------------------------

class HRFEcwStatImpl : public HFCStatImpl
    {
public:
    //--------------------------------------
    // Construction / Destruction
    //--------------------------------------

    HRFEcwStatImpl();
    virtual             ~HRFEcwStatImpl();


    //--------------------------------------
    // Methods
    //--------------------------------------

    // Indicates if an impl can handle an URL
    virtual bool       CanHandle(const HFCURL& pi_rURL) const override;

    // Resource time
    virtual time_t      GetCreationTime     (const HFCURL& pi_rURL) const override;
    virtual time_t      GetLastAccessTime   (const HFCURL& pi_rURL) const override;
    virtual time_t      GetModificationTime (const HFCURL& pi_rURL) const override;
    virtual void        SetModificationTime (const HFCURL& pi_rURL, time_t pi_NewTime) const override;

    // Resource size
    virtual uint64_t   GetSize(const HFCURL& pi_rURL) const override;

    virtual HFCStat::AccessStatus
    DetectAccess(const HFCURL& pi_rURL) const override;

    // Resource access mode
    virtual HFCAccessMode
    GetAccessMode(const HFCURL& pi_rURL) const override;
    };

//-----------------------------------------------------------------------------
// Instantiate the file implementation
//-----------------------------------------------------------------------------
//------------------------------------------------------------------------------
// HRFEcwStat 
//------------------------------------------------------------------------------
class HRFEcwStat
    {
    public:
        static void Register();
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void HRFEcwStat::Register()
    {
    static HRFEcwStatImpl s_impl;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HRFEcwStatImpl::HRFEcwStatImpl()
    {
    RegisterImpl(this);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HRFEcwStatImpl::~HRFEcwStatImpl()
    {
    UnregisterImpl(this);
    }


//-----------------------------------------------------------------------------
// Public
// Indicates if an impl can handle an URL
//-----------------------------------------------------------------------------
bool HRFEcwStatImpl::CanHandle(const HFCURL& pi_rURL) const
    {
    return (((pi_rURL.GetSchemeType() == HFCURLECWP::s_SchemeName()) ||
             (pi_rURL.GetSchemeType() == HFCURLECWPS::s_SchemeName())) &&
            (!HRFURLInternetImagingHTTP::IsURLInternetImaging(&pi_rURL))) ;
    }


//-----------------------------------------------------------------------------
// Public
// Resource modification time
//-----------------------------------------------------------------------------
time_t HRFEcwStatImpl::GetCreationTime(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));
    time_t Result = 0;

    /* NEEDS_WORK
        It looks like there is no easy/safe way to retrieve file information without downloading the file.

    try
    {
        // Copy the URL
        HFCPtr<HFCURL> pURL(HFCURL::Instanciate(pi_rURL.GetURL()));
        HASSERT(pURL != 0);

        // Create an web file but use the cache copy only. i.e. do not download the file
        Result = HFCStat(HRFWebFile(pURL, HFC_READ_ONLY|HFC_SHARE_READ_WRITE).GetLocalURL()).GetCreationTime();
    }
    catch(...)
    {
        HASSERT(false);
    }
    */

    return Result;
    }

//-----------------------------------------------------------------------------
// Public
// Resource modification time
//-----------------------------------------------------------------------------
time_t HRFEcwStatImpl::GetLastAccessTime(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));
    time_t Result = 0;

    /* NEEDS_WORK
        It seems like there is no easy/safe way to retrieve file information without downloading the file.

    try
    {
        // Copy the URL
        HFCPtr<HFCURL> pURL(HFCURL::Instanciate(pi_rURL.GetURL()));
        HASSERT(pURL != 0);

        // Create an web file but use the cache copy only. i.e. do not download the file
        Result = HFCStat(HRFWebFile(pURL, HFC_READ_ONLY|HFC_SHARE_READ_WRITE).GetLocalURL()).GetLastAccessTime();
    }
    catch(...)
    {
        HASSERT(false);
    }
    */

    return Result;
    }

//-----------------------------------------------------------------------------
// Public
// Resource modification time
//-----------------------------------------------------------------------------
time_t HRFEcwStatImpl::GetModificationTime(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));
    time_t Result = 0;

    /* NEEDS_WORK
        It looks like there is no easy/safe way to retrieve file information without downloading the file.

    try
    {
        // Copy the URL
        HFCPtr<HFCURL> pURL(HFCURL::Instanciate(pi_rURL.GetURL()));
        HASSERT(pURL != 0);

        // Create an web file but use the cache copy only. i.e. do not download the file
        Result = HFCStat(HRFWebFile(pURL, HFC_READ_ONLY|HFC_SHARE_READ_WRITE).GetLocalURL()).GetModificationTime();
    }
    catch(...)
    {
        HASSERT(false);
    }
    */

    return Result;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HRFEcwStatImpl::SetModificationTime(const HFCURL& pi_rURL, time_t pi_NewTime) const
    {
    HPRECONDITION(CanHandle(pi_rURL));

    // Cannot so do nothing
    }

//-----------------------------------------------------------------------------
// Public
// Resource size
//-----------------------------------------------------------------------------
uint64_t HRFEcwStatImpl::GetSize(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));
    uint64_t Result = 0;

    /* NEEDS_WORK
        It looks like there is no easy/safe way to retrieve file information without downloading the file.

    try
    {
        // Copy the URL
        HFCPtr<HFCURL> pURL(HFCURL::Instanciate(pi_rURL.GetURL()));
        HASSERT(pURL != 0);

        // Create an web file but use the cache copy only. i.e. do not download the file
        Result = HFCStat(HRFWebFile(pURL, HFC_READ_ONLY|HFC_SHARE_READ_WRITE).GetLocalURL()).GetSize();
    }
    catch(...)
    {
        HASSERT(false);
    }
    */

    return Result;
    }

//-----------------------------------------------------------------------------
// Public
// Detect existence
//-----------------------------------------------------------------------------
HFCStat::AccessStatus HRFEcwStatImpl::DetectAccess(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));

    return HFCStat::AccessGranted;
    }

//-----------------------------------------------------------------------------
// Public
// Resource access mode
//-----------------------------------------------------------------------------
HFCAccessMode HRFEcwStatImpl::GetAccessMode(const HFCURL& pi_rURL) const
    {
    HPRECONDITION(CanHandle(pi_rURL));

    return (HFC_READ_ONLY);
    }



HFC_IMPLEMENT_SINGLETON(HRFEcwCreator)

//-----------------------------------------------------------------------------
// Constructor
// Public (HRFEcwCreator)
// This is the creator to instantiate Ecw format
//-----------------------------------------------------------------------------
HRFEcwCreator::HRFEcwCreator()
    : HRFRasterFileCreator(HRFEcwFile::CLASS_ID)
    {
    //Ecw capabilities instance member initialization
    m_pCapabilities = 0;

    m_pLabel = 0;
    m_pExtensions = 0;
    m_pSupportedPixelTypeList = 0;
    HRFEcwStat::Register();
    }

//-----------------------------------------------------------------------------
// GetLabel
// Public (HRFEcwCreator)
// Identification information
//-----------------------------------------------------------------------------
WString HRFEcwCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_ECW());
    }

//-----------------------------------------------------------------------------
// GetSchemes
// Public (HRFEcwCreator)
// Identification information
//-----------------------------------------------------------------------------
WString HRFEcwCreator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName() + L";" +
                   HFCURLECWP::s_SchemeName() + L";" +
                   HFCURLECWPS::s_SchemeName());
    }

//-----------------------------------------------------------------------------
// GetExtensions
// Public (HRFEcwCreator)
// Identification information
//-----------------------------------------------------------------------------
WString HRFEcwCreator::GetExtensions() const
    {
    return WString(L"*.ecw");
    }

//-----------------------------------------------------------------------------
// Public
// SupportsURL
// This function is overwritten because in the case of an internet URL
// (ECWP, ECWPS) the extension should not be checked.
//-----------------------------------------------------------------------------
bool HRFEcwCreator::SupportsURL(const HFCPtr<HFCURL>& pi_rpURL) const
    {
    bool IsURLSupported;

    //If it is an internet URL, check only the scheme.
    if (pi_rpURL->IsCompatibleWith(HFCURLCommonInternet::CLASS_ID) == true)
        {
        IsURLSupported = SupportsScheme(pi_rpURL->GetSchemeType());
        }
    else
        {
        IsURLSupported = HRFRasterFileCreator::SupportsURL(pi_rpURL);
        }

    return IsURLSupported;
    }


//-----------------------------------------------------------------------------
// Create
// Public (HRFEcwCreator)
// allow to Open an image file
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFEcwCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                            HFCAccessMode   pi_AccessMode,
                                            uint64_t       pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // Open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFEcwFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }


//-----------------------------------------------------------------------------
// IsKindOfFile
// Public (HRFEcwCreator)
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------
bool HRFEcwCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                  uint64_t             pi_Offset) const
    {
    bool IsKindOf = false;

    //No other creators should try to open streaming ecw files.
    if (pi_rpURL->IsCompatibleWith(HFCURLECWP::CLASS_ID) ||
        pi_rpURL->IsCompatibleWith(HFCURLECWPS::CLASS_ID))
        {
        IsKindOf = true;
        }
    else //Local ECW
        {
        // Try to create file view
        NCSFileView* pView = 0;

        HRFErMapperSupportedFile::InitErMapperLibrary();

        uint32_t UrlOffset = 0;
        HFCLockMonitor* pSisterFileLock = 0;

        if (pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID))
            {
            UrlOffset = 7; //Ignore the file:// placed at the front of the url
            (const_cast<HRFEcwCreator*>(this))->SharingControlCreate(pi_rpURL);
            pSisterFileLock = new HFCLockMonitor(GetLockManager());
            } 

        NCSError error = NCScbmOpenFileViewW(pi_rpURL->GetURL().substr(UrlOffset).c_str(), &pView, 0);

        if (error == NCS_SUCCESS)
            {
            //Check if it is really an ECW since ErMapper can also open Jpeg2000 files.
            if (NCScbmGetFileType(pView) == NCS_FILE_ECW)
                {
                IsKindOf = true;
                }
            }
        NCScbmCloseFileViewEx(pView, true);

        if (pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID))
            {
            pSisterFileLock->ReleaseKey();
            delete pSisterFileLock;
            HASSERT(!(const_cast<HRFEcwCreator*>(this))->m_pSharingControl->IsLocked());
            (const_cast<HRFEcwCreator*>(this))->m_pSharingControl = 0;
            }
        }

    return IsKindOf;
    }

//-----------------------------------------------------------------------------
// GetCapabilities
// Public (HRFEcwCreator)
// Create or get the singleton capabilities of Ecw file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFEcwCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFEcwCapabilities();

    return m_pCapabilities;
    }

//-----------------------------------------------------------------------------
// Destructor
// Public
// Destroy an HRFEcwFile object
//-----------------------------------------------------------------------------
HRFEcwFile::~HRFEcwFile()
    {
    }

//-----------------------------------------------------------------------------
// GetCapabilities
// Public
// Return the capabilities of an ECW file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFEcwFile::GetCapabilities () const
    {
    const HFCPtr<HRFRasterFileCapabilities>* ppCapabilities;

    ppCapabilities = &(HRFEcwCreator::GetInstance()->GetCapabilities());

    return *ppCapabilities;
    }


//-----------------------------------------------------------------------------
// HRFJpeg2000BlockCapabilities
//-----------------------------------------------------------------------------
class HRFJpeg2000BlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFJpeg2000BlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Block Capability
        // Tile Capability
        Add(new HRFTileCapability(HFC_READ_CREATE,         // AccessMode
                                  LONG_MAX,            // MaxSizeInBytes
                                  1,                   // MinWidth
                                  LONG_MAX,            // MaxWidth
                                  0,                   // WidthIncrement
                                  1,                   // MinHeight
                                  LONG_MAX,            // MaxHeight
                                  0,                   // HeightIncrement
                                  true));              // IsSquare
        }
    };

//-----------------------------------------------------------------------------
// HRFEcwCodecIdentityCapabilities
//-----------------------------------------------------------------------------
class HRFJpeg2000CodecIdentityCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFJpeg2000CodecIdentityCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec
        Add(new HRFCodecCapability(HFC_READ_CREATE,
                                   HCDCodecJPEG2000::CLASS_ID,
                                   new HRFJpeg2000BlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFEcwCapabilities
//-----------------------------------------------------------------------------
HRFJpeg2000Capabilities::HRFJpeg2000Capabilities()
    : HRFRasterFileCapabilities()
    {
    Add(new HRFPixelTypeCapability(HFC_READ_CREATE,
                                   HRPPixelTypeV8Gray8::CLASS_ID,
                                   new HRFJpeg2000CodecIdentityCapabilities()));

    Add(new HRFPixelTypeCapability(HFC_READ_CREATE,
                                   HRPPixelTypeV24R8G8B8::CLASS_ID,
                                   new HRFJpeg2000CodecIdentityCapabilities()));

    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV32R8G8B8A8::CLASS_ID,
                                   new HRFJpeg2000CodecIdentityCapabilities()));

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_CREATE, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_CREATE, HRFInterleaveType::PIXEL));

    // MultiResolution Capability
    Add(new HRFMultiResolutionCapability(HFC_READ_CREATE));

    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_CREATE));

    // Maximum file size capability
    Add(new HRFMaxFileSizeCapability(HFC_READ_CREATE, (uint64_t)UINT64_MAX));

    // Transfo Model
    Add(new HRFTransfoModelCapability(HFC_READ_CREATE, HGF2DIdentity::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_CREATE, HGF2DTranslation::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_CREATE, HGF2DStretch::CLASS_ID));

    // Geocoding capability
    HFCPtr<HRFGeocodingCapability> pGeocodingCapability(new HRFGeocodingCapability(HFC_READ_CREATE));

    pGeocodingCapability->AddSupportedKey(GTModelType);
    pGeocodingCapability->AddSupportedKey(ProjectedCSType);
    pGeocodingCapability->AddSupportedKey(ProjectedCSTypeLong);
    pGeocodingCapability->AddSupportedKey(ProjLinearUnits);

    Add((HFCPtr<HRFCapability>&)pGeocodingCapability);
    }

HFC_IMPLEMENT_SINGLETON(HRFJpeg2000Creator)

//-----------------------------------------------------------------------------
// Constructor
// Public (HRFEcwCreator)
// This is the creator to instantiate Ecw format
//-----------------------------------------------------------------------------
HRFJpeg2000Creator::HRFJpeg2000Creator()
    : HRFRasterFileCreator(HRFJpeg2000File::CLASS_ID)
    {
    //Ecw capabilities instance member initialization
    m_pCapabilities = 0;

    m_pLabel = 0;
    m_pExtensions = 0;
    m_pSupportedPixelTypeList = 0;
    }

//-----------------------------------------------------------------------------
// GetLabel
// Public (HRFEcwCreator)
// Identification information
//-----------------------------------------------------------------------------
WString HRFJpeg2000Creator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_Jpeg2000());
    }

//-----------------------------------------------------------------------------
// GetSchemes
// Public (HRFEcwCreator)
// Identification information
//-----------------------------------------------------------------------------
WString HRFJpeg2000Creator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

//-----------------------------------------------------------------------------
// GetExtensions
// Public (HRFEcwCreator)
// Identification information
//-----------------------------------------------------------------------------
WString HRFJpeg2000Creator::GetExtensions() const
    {
    return WString(L"*.jp2;*.j2k;*.jpm;*.j2c;*.jpc;*.jpx;*.jpf");
    }

//-----------------------------------------------------------------------------
// Create
// Public (HRFEcwCreator)
// allow to Open an image file
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFJpeg2000Creator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                                 HFCAccessMode         pi_AccessMode,
                                                 uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // Open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFJpeg2000File(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }


//-----------------------------------------------------------------------------
// IsKindOfFile
// Public (HRFJpeg2000Creator)
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------
bool HRFJpeg2000Creator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                       uint64_t             pi_Offset) const
    {
    if(!pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID))
        return false;

    bool IsKindOf = false;

    // Try to create file view
    NCSFileView* pView = 0;

    (const_cast<HRFJpeg2000Creator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock (GetLockManager());

    HRFErMapperSupportedFile::InitErMapperLibrary();

    NCSError error = NCScbmOpenFileViewW(static_cast<HFCURLFile*>(pi_rpURL.GetPtr())->GetAbsoluteFileName().c_str(), &pView, 0);
    if (error == NCS_SUCCESS)
        {
        //Check if it is really an ECW since ErMapper can also open Jpeg2000 files.
        if (NCScbmGetFileType(pView) == NCS_FILE_JP2)
            {
            IsKindOf = true;
            }
        }
    NCScbmCloseFileViewEx((NCSFileView*)pView, true);

    SisterFileLock.ReleaseKey();
    HASSERT(!(const_cast<HRFJpeg2000Creator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFJpeg2000Creator*>(this))->m_pSharingControl = 0;

    return IsKindOf;
    }

//-----------------------------------------------------------------------------
// GetCapabilities
// Public (HRFEcwCreator)
// Create or get the singleton capabilities of Ecw file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFJpeg2000Creator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFJpeg2000Capabilities();

    return m_pCapabilities;
    }

//-----------------------------------------------------------------------------
// Destructor
// Public
// Destroy an HRFJpeg2000File object
//-----------------------------------------------------------------------------
HRFJpeg2000File::~HRFJpeg2000File()
    {
    }

//-----------------------------------------------------------------------------
// GetCapabilities
// Public
// Return the capabilities of a Jpeg2000 file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFJpeg2000File::GetCapabilities () const
    {
    const HFCPtr<HRFRasterFileCapabilities>* ppCapabilities;

    ppCapabilities = &(HRFJpeg2000Creator::GetInstance()->GetCapabilities());

    return *ppCapabilities;
    }


//-----------------------------------------------------------------------------
// Destructor
// Public
// Destroy an HRFErMapperSupportedFile object
//-----------------------------------------------------------------------------
HRFErMapperSupportedFile::~HRFErMapperSupportedFile()
    {
    Close();
    }

//-----------------------------------------------------------------------------
// Close
// Public
// Close the Ecw file
//-----------------------------------------------------------------------------
void HRFErMapperSupportedFile::Close()
    {
    if (m_IsOpen)
        {
        if (m_pNCSFileView)
            {
            NCScbmCloseFileViewEx((NCSFileView*)m_pNCSFileView, true);
            m_pNCSFileView = 0;
            }

        m_IsOpen = false;
        }

    if (m_pBandList != 0)
        {
        delete m_pBandList;
        }

    if (m_pRatio != 0)
        {
        delete m_pRatio;
        }
    }


//-----------------------------------------------------------------------------
// CreateResolutionEditor
// Public
// File manipulation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFErMapperSupportedFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                                      unsigned short pi_Resolution,
                                                                      HFCAccessMode  pi_AccessMode)
    {
    // Verify that the page number is 0, because we have one image per file
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    HRFResolutionEditor* pEditor = 0;

    pEditor = new HRFErMapperSupportedFileEditor(this, pi_Page, pi_Resolution, pi_AccessMode);

    return pEditor;
    }

//-----------------------------------------------------------------------------
// AddPage
// Public
// File manipulation
//-----------------------------------------------------------------------------
bool HRFErMapperSupportedFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    return HRFRasterFile::AddPage(pi_pPage);
    }

//-----------------------------------------------------------------------------
//  This method must be called before all other calls since result is then unpredictable.
//  Application must call this method to set :
//    - A default unit. (By default, default unit is meter.)
//
//    - To impose an interpretation of ProjectedCSType and ProjLinearUnits.
//      Refer to the section concerning interpretation of geo-reference for details.
//      Our implementation exclude ProjLinearUnits if the ProjectedCSType != UserDefined.
//
//    - To impose an interpretation on the unit.
//      Refer to the section concerning interpretation of geo-reference for details.
//      Our implementation will exclude ProjLinearUnits and use INTERGRAPH coordsys only if unit
//      cannot be resolve from other settings and this flag is true.(see TR 97326)
//
//      This method doesn't set the status flag Changed on the TransfoModel in the PageDescriptor.
//
//  @param pi_RatioToMeter       Ratio used to translate the model to meter.
//  @param pi_InterpretUnit      true : The files will be opened using ProjLinearUnits, can
//                                      redefine the unit of ProjectedCSType.
//                               false: Our standard.(default)
//  @param pi_InterpretUnitINTGR true : We consider the file to be in the INTERGRAPH coord sys
//                                      is unit cannot be resolved (see TR 97326).
//                               false: Our standard.(default)
//-----------------------------------------------------------------------------
void HRFErMapperSupportedFile::SetDefaultRatioToMeter(double pi_RatioToMeter,
                                                      uint32_t pi_Page,
                                                      bool   pi_CheckSpecificUnitSpec,
                                                      bool   pi_InterpretUnitINTGR)
    {
    HPRECONDITION(CountPages() == 1);
    HPRECONDITION(GetPageDescriptor(0) != 0);

    // Update the model in each page
    bool                         defaultUnitWasFound = false;
    HFCPtr<HRFPageDescriptor>     pPageDescriptor     = GetPageDescriptor(0);

    IRasterBaseGcsCP pBaseGCS = pPageDescriptor->GetGeocodingCP();

    if ((pBaseGCS != NULL) && (pBaseGCS->IsValid()) && (pBaseGCS->GetBaseGCS() != NULL))
        {
        HFCPtr<HGF2DTransfoModel> pTransfoModel;
        BuildTransfoModelMatrix (pTransfoModel);

        pTransfoModel = pPageDescriptor->GetRasterFileGeocoding().TranslateToMeter(pTransfoModel,
                                                                                   pi_RatioToMeter, 
                                                                                   pi_CheckSpecificUnitSpec,
                                                                                   &defaultUnitWasFound);


        pPageDescriptor->SetTransfoModel(*pTransfoModel, true);
        pPageDescriptor->SetTransfoModelUnchanged();
        }

    SetUnitFoundInFile(defaultUnitWasFound);
    }

//-----------------------------------------------------------------------------
// GetCapabilities
// Public
// Return the file's capabilities.
//-----------------------------------------------------------------------------
/*
const HFCPtr<HRFRasterFileCapabilities>& HRFErMapperSupportedFile::GetCapabilities () const
{
HASSERT(0); //Must be defined by its derived classes.
return 0;
}
*/
//-----------------------------------------------------------------------------
// Open
// Protected
// This method open the file.
//-----------------------------------------------------------------------------
bool HRFErMapperSupportedFile::Open()
    {
    bool HasFileBeenOpened = false;

    // Don't try this if a file is already opened!
    if (m_IsOpen == false)
        {
        // initialize ErMapper library if not alread done
        HRFErMapperSupportedFile::InitErMapperLibrary();

        uint32_t UrlOffset = 0;

        if (m_pURL->IsCompatibleWith(HFCURLFile::CLASS_ID))
            {
            // This method creates the sharing control sister file
            SharingControlCreate();
            UrlOffset = 7; //Ignore the file:// placed at the front of the url
            }

        NCSError error = NCScbmOpenFileViewW(m_pURL->GetURL().substr(UrlOffset).c_str(), (NCSFileView**)&m_pNCSFileView, 0);

        if (error != 0)
            {
            m_pNCSFileView = 0;
            }
        else
            {
            error = NCScbmGetViewFileInfoEx((NCSFileView*)m_pNCSFileView, (NCSFileViewFileInfoEx**)&m_pNCSFileViewFileInfoEx);
            }

        if (error == 0)
            {
            // Prepare bands (optimization)
            m_pBandList = new uint32_t[((NCSFileViewFileInfoEx*)m_pNCSFileViewFileInfoEx)->nBands];
            for (unsigned short Band = 0; Band < ((NCSFileViewFileInfoEx*)m_pNCSFileViewFileInfoEx)->nBands; Band++)
                m_pBandList[Band] = Band;

            HasFileBeenOpened = true;
            m_IsOpen = true;
            }
        }

    return HasFileBeenOpened;
    }

//-----------------------------------------------------------------------------
// public
// SetLookAhead
//
// Sets the LookAhead for a shape
//-----------------------------------------------------------------------------
void HRFErMapperSupportedFile::SetLookAhead(uint32_t        pi_Page,
                                            unsigned short pi_Resolution,
                                            const HVEShape& pi_rShape,
                                            uint32_t        pi_ConsumerID,
                                            bool           pi_Async)
    {
    HPRECONDITION(pi_Page == 0);
    HPRECONDITION(pi_Resolution < GetPageDescriptor(0)->CountResolutions());
    //The lookahead is only supported for ECWP for now.
    HPRECONDITION(m_IsECWP == true);

    HFCMonitor Monitor(m_TilePoolKey);
    TilePool::iterator Itr(m_TilePool.begin());
    while (Itr != m_TilePool.end())
        {
        delete[] Itr->second;
        Itr++;
        }
    m_TilePool.clear();

    if (!pi_rShape.IsEmpty())
        {
        // Extract the needed blocks from the region

        HFCPtr<HRFResolutionDescriptor> pResDesc = GetPageDescriptor(0)->GetResolutionDescriptor(pi_Resolution);

        uint32_t BlockWidth  = pResDesc->GetBlockWidth();
        uint32_t BlockHeight = pResDesc->GetBlockHeight();

        HGFTileIDDescriptor TileDesc(pResDesc->GetWidth(),
                                     pResDesc->GetHeight(),
                                     BlockWidth,
                                     BlockHeight);

        uint32_t MinX = ULONG_MAX;
        uint32_t MinY = ULONG_MAX;
        uint32_t MaxX = 0;
        uint32_t MaxY = 0;

        // Make sure the input shape will not be destroyed
        HVEShape* pShape = const_cast<HVEShape*>(&pi_rShape);
        pShape->IncrementRef();
            {
            HVETileIDIterator TileIterator(&TileDesc, HFCPtr<HVEShape>(pShape));

            uint64_t PosX;
            uint64_t PosY;
            uint64_t BlockIndex = TileIterator.GetFirstTileIndex();
            while (BlockIndex != HGFTileIDDescriptor::INDEX_NOT_FOUND)
                {
                TileDesc.GetPositionFromIndex(BlockIndex, &PosX, &PosY);
                HASSERT(PosX <= ULONG_MAX);
                HASSERT(PosY <= ULONG_MAX);

                MinX = MIN(MinX, (uint32_t)PosX);
                MinY = MIN(MinY, (uint32_t)PosY);
                MaxX = MAX(MaxX, (uint32_t)PosX + BlockWidth);
                MaxY = MAX(MaxY, (uint32_t)PosY + BlockHeight);

                BlockIndex = TileIterator.GetNextTileIndex();
                }
            }
        // Decrement after the block because the HVETileIDIterator keeps an
        // HFCPtr to the input shape, so we want the Iterator to be destroyed
        // before we decrement.
        pShape->DecrementRef();

        uint32_t Width = MaxX - MinX;
        uint32_t Height = MaxY - MinY;
        Byte NbBytePerPixel;

        HArrayAutoPtr<Byte> pData;
        if (((NCSFileViewFileInfoEx*)m_pNCSFileViewFileInfoEx)->nBands == 1)
            {
            NbBytePerPixel = 1;
            }
        else if (((NCSFileViewFileInfoEx*)m_pNCSFileViewFileInfoEx)->nBands == 3)
            {
            NbBytePerPixel = 3;
            }
        else
            {
            if (GetSpecifiedBands().empty() || GetSpecifiedBands().size() == 3)
                {
                NbBytePerPixel = 3;
                }
            else
                {
                NbBytePerPixel = 4;
                }
            }

        pData = new Byte[Width * Height * NbBytePerPixel];   // 24 bits

        if (pData != 0)
            {
            try
                {
                HFCLockMonitor SisterFileLock;
                // Lock the file.
                if (m_IsECWP == false)
                    {
                    SisterFileLock.Assign(GetLockManager());
                    }

                const HRFResolutionEditor* pResEditor(GetResolutionEditor(pi_Resolution));

                HASSERT(pResEditor != 0);

                HSTATUS Status;

                Status = ((HRFErMapperSupportedFileEditor*)pResEditor)->
                         ReadBlock(MinX, MinY,
                                   Width, Height,
                                   pData, &SisterFileLock);

                if (Status == H_SUCCESS)
                    {
                    uint32_t NbXTile = Width / BlockWidth;
                    uint32_t NbYTile = Height / BlockHeight;
                    uint32_t BytesPerBlockWidth = pResDesc->GetBytesPerBlockWidth();
                    uint32_t SrcWidth = Width * NbBytePerPixel;
                    Byte* pSrc;
                    Byte* pDst;
                    uint32_t BlockX;
                    uint32_t PosX;
                    uint32_t BlockY;
                    uint32_t PosY;
                    HArrayAutoPtr<Byte> pBlockData;
                    for (BlockX = 0, PosX = MinX; BlockX < NbXTile; ++BlockX, PosX += BlockWidth)
                        {
                        for (BlockY = 0, PosY = MinY; BlockY < NbYTile; ++BlockY, PosY += BlockHeight)
                            {
                            pBlockData = new Byte[pResDesc->GetBlockSizeInBytes()];
                            pDst = pBlockData.get();
                            pSrc = pData.get() + BlockY * BlockHeight * SrcWidth;
                            pSrc += BlockX * BytesPerBlockWidth;

                            for (uint32_t Line = 0; Line < BlockHeight; ++Line)
                                {
                                memcpy(pDst, pSrc, BytesPerBlockWidth);
                                pDst += BytesPerBlockWidth;
                                pSrc += SrcWidth;
                                }
                            pair<TilePool::iterator, bool> InsertPair(m_TilePool.insert(TilePool::value_type(TileDesc.ComputeID(PosX, PosY, pi_Resolution),
                                                                                        pBlockData.release())));

                            HASSERT(InsertPair.second);
                            }
                        }
                    }
                }
            catch (...)
                {
                m_TilePool.clear();
                _ASSERT(false);
                }
            }
        }
    }

//-----------------------------------------------------------------------------
// public
// setLookAhead
//
// Sets the LookAhead for a list of blocks
//-----------------------------------------------------------------------------
void HRFErMapperSupportedFile::SetLookAhead(uint32_t               pi_Page,
                                            const HGFTileIDList&   pi_rBlocks,
                                            uint32_t               pi_ConsumerID,
                                            bool                  pi_Async)
    {
    HASSERT(0);
    }

//-----------------------------------------------------------------------------
// Private
// This method create the file.
//-----------------------------------------------------------------------------
bool HRFErMapperSupportedFile::Create()
    {
    // This creates the sister file for file sharing control if necessary.
    SharingControlCreate();

    return true;
    }

//-----------------------------------------------------------------------------
// CreateDescriptors
// Protected
// Create Bmp File Descriptors
//-----------------------------------------------------------------------------
void HRFErMapperSupportedFile::CreateDescriptors ()
    {
    // Get image dimensions.
    unsigned long Width  = ((NCSFileViewFileInfoEx*)m_pNCSFileViewFileInfoEx)->nSizeX;
    unsigned long Height = ((NCSFileViewFileInfoEx*)m_pNCSFileViewFileInfoEx)->nSizeY;

    // Calculate resolution count
    unsigned int  ResCount = 1;
    unsigned long ResWidth = Width;
    unsigned long ResHeight = Height;

    while ((ResWidth > BLOCK_WIDTH_ERMAPPER) && (ResHeight > BLOCK_HEIGHT_ERMAPPER))
        {
        ResWidth = (unsigned long)ceil(ResWidth / 2.0);
        ResHeight= (unsigned long)ceil(ResHeight / 2.0);
        ResCount++;
        }
    ResCount = MIN(ResCount, 254);  // don't use 255, 255 means Unlimited resolution.

    HRFPageDescriptor::ListOfResolutionDescriptor  ListOfResolutionDescriptor;
    HFCPtr<HRFResolutionDescriptor>                   pResolutionDesc;

    HFCPtr<HRPPixelType>                           pPixelType = 0;
    HFCPtr<HCDCodec>                               pCodec;

    // Pixel type
    if (((NCSFileViewFileInfoEx*)m_pNCSFileViewFileInfoEx)->nBands == 1)
        {
        pPixelType = new HRPPixelTypeV8Gray8();
        }
    else if (((NCSFileViewFileInfoEx*)m_pNCSFileViewFileInfoEx)->nBands == 3)
        {
        pPixelType = new HRPPixelTypeV24R8G8B8();
        }
    else
        {
        if (GetSpecifiedBands().empty() || GetSpecifiedBands().size() == 3)
            {
            pPixelType = new HRPPixelTypeV24R8G8B8();
            }
        else
            {
            pPixelType = new HRPPixelTypeV32R8G8B8A8();
            }
        }

    // Codec
    if (m_IsJpeg2000 == true)
        {
        pCodec = new HCDCodecJPEG2000();
        }
    else
        {
        pCodec = new HCDCodecECW();
        }

    // Allocate memory for ReadBlock() utilities (optimization)
    m_pRatio         = new double[ResCount];

    // Fill resolution descriptors
    ResWidth = Width;
    ResHeight = Height;

    for (unsigned int count = 0; count < ResCount; count++)
        {
        // At the same time init StdViewWidth and StdViewHeight (optimization)
        // This avoid to compute these values each time ReadBlock() is called
        m_pRatio[count] = RoundRatio(Width, ResWidth);

        // Create Resolution Descriptor
        pResolutionDesc =  new HRFResolutionDescriptor(GetAccessMode(),                 // AccessMode,
                                                       m_pCapabilities,             // Capabilities,
                                                       m_pRatio[count],                // XResolutionRatio,
                                                       m_pRatio[count],                // YResolutionRatio,
                                                       pPixelType,                    // PixelType,
                                                       pCodec,                        // Codecs,
                                                       HRFBlockAccess::RANDOM,        // RBlockAccess,
                                                       HRFBlockAccess::RANDOM,        // WBlockAccess,
                                                       HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL,  // ScanLineOrientation,
                                                       HRFInterleaveType::PIXEL,      // InterleaveType
                                                       0,                             // IsInterlace,
                                                       ResWidth,                         // Width,
                                                       ResHeight,                        // Height,
                                                       BLOCK_WIDTH_ERMAPPER,             // BlockWidth,
                                                       BLOCK_HEIGHT_ERMAPPER                  // BlockHeight,
                                                      );

        ResWidth = (unsigned long)ceil(ResWidth / 2.0);
        ResHeight= (unsigned long)ceil(ResHeight / 2.0);

        ListOfResolutionDescriptor.push_back(pResolutionDesc);
        }


    double factorModelToMeter(1.0);
    RasterFileGeocodingPtr pGeocoding = ExtractGeocodingInformation(factorModelToMeter);

    HFCPtr<HGF2DTransfoModel> pTransfoModel;
    bool                      defaultUnitWasFound = false;

    BuildTransfoModelMatrix(pTransfoModel);
    pTransfoModel = pGeocoding->TranslateToMeter(pTransfoModel,
                                                 factorModelToMeter,
                                                 false,
                                                 &defaultUnitWasFound);

    SetUnitFoundInFile(defaultUnitWasFound);

    HFCPtr<HRFPageDescriptor> pPage;
    pPage = new HRFPageDescriptor (GetAccessMode(),
                                   m_pCapabilities,             // Capabilities,
                                   ListOfResolutionDescriptor,  // ResolutionDescriptor,
                                   0,                           // RepresentativePalette,
                                   0,                           // Histogram,
                                   0,                           // Thumbnail,
                                   0,                           // ClipShape,
                                   pTransfoModel,               // TransfoModel,
                                   0,                           // Filters
                                   0,                           // Tag
                                   0);                          // Duration

    // Set geocoding
    pPage->InitFromRasterFileGeocoding(*pGeocoding);

    m_ListOfPageDescriptor.push_back(pPage);
    }

//-----------------------------------------------------------------------------
// GetTagInfo
// Private
// Get a list of relevant tags found embedded in the file.
//-----------------------------------------------------------------------------
RasterFileGeocodingPtr HRFErMapperSupportedFile::ExtractGeocodingInformation(double & factorModelToMeter) const
    {
    WString osUnits;

    if (((NCSFileViewFileInfoEx*)m_pNCSFileViewFileInfoEx)->eCellSizeUnits == ECW_CELL_UNITS_FEET )
        {
        if (ImageppLib::GetHost().GetImageppLibAdmin()._IsErMapperUseFeetInsteadofSurveyFeet())
            {
            factorModelToMeter = 0.3048000000;//  meters by international feet.
            osUnits = L"IFEET";
            }
        else
            {
            factorModelToMeter = 12000.0 / 39370.0; //0.30480060960121919 meters by survey feet.
            osUnits = L"FEET";
            }
        }
    else if (((NCSFileViewFileInfoEx*) m_pNCSFileViewFileInfoEx)->eCellSizeUnits == ECW_CELL_UNITS_METERS)
        {
        factorModelToMeter=1.0;
        osUnits = L"METERS";
        }
    else if (((NCSFileViewFileInfoEx*) m_pNCSFileViewFileInfoEx)->eCellSizeUnits == ECW_CELL_UNITS_DEGREES)
        {
        factorModelToMeter = 1.0;
        osUnits = L"DEGREES";
        }

    WString projection;
    WString datum;

    BeStringUtilities::CurrentLocaleCharToWChar(projection,((NCSFileViewFileInfoEx*)m_pNCSFileViewFileInfoEx)->szProjection);
    BeStringUtilities::CurrentLocaleCharToWChar(datum,((NCSFileViewFileInfoEx*)m_pNCSFileViewFileInfoEx)->szDatum);

    uint32_t EPSGCodeFomrERLibrary = GetEPSGFromProjectionAndDatum(projection, datum);
    IRasterBaseGcsPtr  pBaseGCS = HRFGeoCoordinateProvider::CreateRasterGcsFromERSIDS(EPSGCodeFomrERLibrary, projection,datum,osUnits);

    return RasterFileGeocoding::Create(pBaseGCS.get());
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t HRFErMapperSupportedFile::GetEPSGFromProjectionAndDatum(WStringCR pi_rErmProjection, WStringCR pi_rErmDatum)
    {
    INT32 nEPSGCode = TIFFGeo_UserDefined;

    // We try to obtain the EPSG code
    size_t  destinationBuffSize = pi_rErmProjection.GetMaxLocaleCharBytes();
    char*  szProjectionMBS= (char*)_alloca (destinationBuffSize);
    BeStringUtilities::WCharToCurrentLocaleChar(szProjectionMBS,pi_rErmProjection.c_str(),destinationBuffSize);
    destinationBuffSize = pi_rErmDatum.GetMaxLocaleCharBytes();
    char*  szDatumMBS= (char*)_alloca (destinationBuffSize);
    BeStringUtilities::WCharToCurrentLocaleChar(szDatumMBS,pi_rErmDatum.c_str(),destinationBuffSize);

    if (NCS_SUCCESS != NCSGetEPSGCode(szProjectionMBS, szDatumMBS, &nEPSGCode))
        {
        if (NCS_SUCCESS != NCSGetEPSGCode(szDatumMBS, szProjectionMBS, &nEPSGCode))
            {
            nEPSGCode = TIFFGeo_UserDefined;
            }
        }
    return (uint32_t)nEPSGCode;
    }


//-----------------------------------------------------------------------------
// BuildTransfoModelMatrix
// Protected
// This method build the transformation matrix.
//-----------------------------------------------------------------------------
void HRFErMapperSupportedFile::BuildTransfoModelMatrix(HFCPtr<HGF2DTransfoModel>&     po_prTranfoModel)
    {
    HFCMatrix<3, 3> TransfoMatrix;

#if 0       //DMx ECW SDK 5.0 support Geotiff Tag
            // If we want to use Geotiff tag, we need to update the code below to support it.    
    double         *pFileMatrix=nullptr;
    int            NbPoints;
    if (NCSGetGeotiffTag((NCSFileView *)GetFileView(), GTIFF_TRANSMATRIX, &NbPoints, &pFileMatrix) == NCS_SUCCESS &&
        (NbPoints == 16))
        {
        TransfoMatrix[0][0] = pFileMatrix[0];
        TransfoMatrix[0][1] = pFileMatrix[1];
        TransfoMatrix[0][2] = pFileMatrix[3];
        TransfoMatrix[1][0] = pFileMatrix[4];
        TransfoMatrix[1][1] = pFileMatrix[5];
        TransfoMatrix[1][2] = pFileMatrix[7];
        TransfoMatrix[2][0] = pFileMatrix[12];
        TransfoMatrix[2][1] = pFileMatrix[13];
        if (!HDOUBLE_EQUAL_EPSILON(pFileMatrix[15], 0.0))
            TransfoMatrix[2][2] = pFileMatrix[15];
        else
            TransfoMatrix[2][2] = 1.0;
        }
    else
#endif
    {
    switch(((NCSFileViewFileInfoEx*)m_pNCSFileViewFileInfoEx)->eCellSizeUnits)
        {
        case ECW_CELL_UNITS_METERS:
        case ECW_CELL_UNITS_UNKNOWN:
        case ECW_CELL_UNITS_DEGREES:
        case ECW_CELL_UNITS_FEET:
            if (((NCSFileViewFileInfoEx*)m_pNCSFileViewFileInfoEx)->fCellIncrementX == 0 ||
                ((NCSFileViewFileInfoEx*)m_pNCSFileViewFileInfoEx)->fCellIncrementY == 0)
                throw HRFTransfoModelNotSupportedException(m_pURL->GetURL().c_str());

            // The unit will be treated by the tag ProjLinearUnits
            TransfoMatrix[0][0] = ((NCSFileViewFileInfoEx*)m_pNCSFileViewFileInfoEx)->fCellIncrementX;
            TransfoMatrix[0][1] = 0.0;
            TransfoMatrix[0][2] = ((NCSFileViewFileInfoEx*)m_pNCSFileViewFileInfoEx)->fOriginX;
            TransfoMatrix[1][0] = 0.0;
            TransfoMatrix[1][1] = ((NCSFileViewFileInfoEx*)m_pNCSFileViewFileInfoEx)->fCellIncrementY;
            TransfoMatrix[1][2] = ((NCSFileViewFileInfoEx*)m_pNCSFileViewFileInfoEx)->fOriginY;
            TransfoMatrix[2][0] = 0.0;
            TransfoMatrix[2][1] = 0.0;
            TransfoMatrix[2][2] = 1.0;
            break;

        default: // ECW_CELL_UNITS_INVALID

            // Identity
            TransfoMatrix[0][0] = 1.0;
            TransfoMatrix[0][1] = 0.0;
            TransfoMatrix[0][2] = 0.0;
            TransfoMatrix[1][0] = 0.0;
            TransfoMatrix[1][1] = 1.0;
            TransfoMatrix[1][2] = 0.0;
            TransfoMatrix[2][0] = 0.0;
            TransfoMatrix[2][1] = 0.0;
            TransfoMatrix[2][2] = 1.0;
        }

    // TR 78633
    // The physical and logical SLO of a ECW file are SLO4. If the file as the GeoTiff
    // tag, the HRFCOMInstance see the file like a GeoTiff, SLO4 as the physical and
    // SLO6 as logical. For this reason, we must flip the logical model if was in SLO4
    if (TransfoMatrix[1][1] > 0.0)
        {
        TransfoMatrix[1][1] =  -TransfoMatrix[1][1]; // y scale
        }
    }

    po_prTranfoModel = new HGF2DProjective(TransfoMatrix);




    // Get the simplest model possible.
    HFCPtr<HGF2DTransfoModel> pTempTransfoModel = po_prTranfoModel->CreateSimplifiedModel();

    //It should always be a simplified model since ECW has only the stretch capability.
    HASSERT(pTempTransfoModel != 0);

    if (pTempTransfoModel != 0)
        po_prTranfoModel = pTempTransfoModel;
    }

//-----------------------------------------------------------------------------
// Save
// Private
// This method saves the file.
//-----------------------------------------------------------------------------
void HRFErMapperSupportedFile::Save()
    {
    //Read only file
    }

//-----------------------------------------------------------------------------
// RoundRatio
// Protected
// Round ratio. Try to round to multiple of 2, if possible --> at 1 pixel.
//-----------------------------------------------------------------------------
double HRFErMapperSupportedFile::RoundRatio(unsigned long pi_MainImageSize, unsigned long pi_ResImageSize)
    {
    double ResCurrent = (double)pi_ResImageSize / (double)pi_MainImageSize;

    unsigned long Exposent = round(log(1.0 / ResCurrent) / log(2.0));
    double Val2Exposent = pow(2.0, (double)Exposent);
    double ResSize = pi_MainImageSize / Val2Exposent;

    if (abs((int)(ResSize - (double)pi_ResImageSize)) < 1.0)
        ResCurrent = 1.0 / Val2Exposent;

    return ResCurrent;
    }

//-----------------------------------------------------------------------------
// Protected
// GetFileView
// Get the file view.
//-----------------------------------------------------------------------------
void* HRFErMapperSupportedFile::GetFileView()
    {
    return m_pNCSFileView;
    }

//-----------------------------------------------------------------------------
// Protected
// GetFileViewFileInfo
// Get the file view's info.
//-----------------------------------------------------------------------------
const void* HRFErMapperSupportedFile::GetFileViewFileInfoEx()
    {
    return m_pNCSFileViewFileInfoEx;
    }


//-----------------------------------------------------------------------------
// Public
// InitErMapperLibrary
// Initialize the ErMapper library.
//-----------------------------------------------------------------------------
void HRFErMapperSupportedFile::InitErMapperLibrary()
    {
    static bool sIsLibraryInitialized = false;

    if (sIsLibraryInitialized == false)
        {
        WString pProperty;
        if (BSISUCCESS != ImageppLib::GetHost().GetImageppLibAdmin()._GetECWDataPath(pProperty))
            return;

        NCSecwInit();

        size_t  destinationBuffSize = pProperty.GetMaxLocaleCharBytes();
        char*   multiByteDestination= (char*)_alloca (destinationBuffSize);
        BeStringUtilities::WCharToCurrentLocaleChar(multiByteDestination,pProperty.c_str(),destinationBuffSize);

        NCSSetGDTPath(multiByteDestination);
//DMx        CNCSGDTLocation::SetGuessPath(true);

#if defined(_WIN32) || defined(WIN32)
        //By default, the doc says that it is going to use 25% of the RAM, that seems to be much more than that.
        // Since we already cache on top of ECW library, we decided to limit to 512 MB.
        NCSecwSetConfig(NCSCFG_CACHE_MAXMEM, 512*1024*1024);    // tr #243826, limit the cache to 8 meg in 2008
                                                                   // Now we need a minimum of 128Meg in 64Bit (2013)
#else
    #error Need to setup ErMapper MEM usage using available hardware RAM.
#endif

        sIsLibraryInitialized = true;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
const HRFErMapperSupportedFile::BandList& HRFErMapperSupportedFile::GetSpecifiedBands()
    {
    static BandList s_SpecifiedBands;
    static bool bInit=false;

    if (!bInit)
        {
        bInit=true;
        s_SpecifiedBands.clear();
        ChannelToBandIndexMapping specifiedBand;
        bool IsBandSpecDefined = ImageppLib::GetHost().GetImageppLibAdmin()._GetChannelToBandIndexMapping(specifiedBand);
        if (IsBandSpecDefined)
            {
            //Init s_SpecifiedBands
            //Note that ChannelToBandIndexMapping start at index=1 and s_SpecifiedBands at index=0
            s_SpecifiedBands.push_back(specifiedBand.GetIndex(ChannelToBandIndexMapping::RED) - 1);
            s_SpecifiedBands.push_back(specifiedBand.GetIndex(ChannelToBandIndexMapping::GREEN) - 1);
            s_SpecifiedBands.push_back(specifiedBand.GetIndex(ChannelToBandIndexMapping::BLUE) - 1);
            if (specifiedBand.IsAlphaChannelDefined())
                s_SpecifiedBands.push_back(specifiedBand.GetIndex(ChannelToBandIndexMapping::ALPHA) - 1);
            }
        }
    return s_SpecifiedBands;
    }


//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFEcwFile::HRFEcwFile(const HFCPtr<HFCURL>& pi_rURL,
                              HFCAccessMode         pi_AccessMode,
                              uint64_t             pi_Offset)
    : HRFErMapperSupportedFile(pi_rURL,
                               GetCapabilities(),
                               pi_AccessMode,
                               pi_Offset)
    {
    }

HRFJpeg2000File::HRFJpeg2000File(const HFCPtr<HFCURL>& pi_rURL,
                                        HFCAccessMode  pi_AccessMode,
                                        uint64_t       pi_Offset)
    : HRFErMapperSupportedFile(pi_rURL,
                               GetCapabilities(),
                               pi_AccessMode,
                               pi_Offset,
                               true)
    {
    }

HRFErMapperSupportedFile::HRFErMapperSupportedFile(const HFCPtr<HFCURL>&                    pi_rURL,
                                                   const HFCPtr<HRFRasterFileCapabilities>& pi_prCapabilities,
                                                   HFCAccessMode                            pi_AccessMode,
                                                   uint64_t                                pi_Offset,
                                                   bool                                    pi_IsJpeg2000)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    m_pCapabilities = pi_prCapabilities;
    m_IsJpeg2000    = pi_IsJpeg2000;
    m_IsECWP        = (pi_rURL->IsCompatibleWith(HFCURLECWP::CLASS_ID) ||
                       pi_rURL->IsCompatibleWith(HFCURLECWPS::CLASS_ID));
    m_ECWVersion    = 2;        // Only use at the creation for the moment - SDK previous of 5.0, like V8i

    // The ancestor store the access mode
    m_IsOpen                = false;

    m_pNCSFileView              = 0;
    m_pNCSFileViewFileInfoEx    = 0;

    m_pBandList         = 0;
    m_pRatio            = 0;

    if (GetAccessMode().m_HasWriteAccess)
        {
        throw HFCFileNotSupportedInWriteModeException(GetURL()->GetURL());
        }
    else if (GetAccessMode().m_HasReadAccess)
        {
        // Create Page and Res Descriptors.
        if (Open() == false)
            {
            throw HFCCannotOpenFileException(GetURL()->GetURL());
            }

        CreateDescriptors();
        }
    else if (GetAccessMode().m_HasCreateAccess)
        {
        Create();
        }


    }

//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
// File information
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFErMapperSupportedFile::GetWorldIdentificator () const
    {
    HPRECONDITION(CountPages() > 0);

    //TR 243852 - When an image with no georefence and no geocoding
    //            accompagnied by a sister file is saved to ECW, the world
    //            must be HMR according to how the generated ECW is open by
    //            other third party applications.
    HGF2DWorldIdentificator World = HGF2DWorld_GEOTIFFUNKNOWN;

    // Check geotiff tags
    HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(0);

    bool hasGTModelTypeKey = false;

    RasterFileGeocoding const& fileGeocoding = pPageDescriptor->GetRasterFileGeocoding();
    HCPGeoTiffKeys const& geoKeyContainer = fileGeocoding.GetGeoTiffKeys();

    hasGTModelTypeKey = geoKeyContainer.HasKey(GTModelType);

    if (hasGTModelTypeKey)
        {
        // Change world id if GTModelType is ModelTypeGeographic
        uint32_t GeoLongValue;
        geoKeyContainer.GetValue(GTModelType, &GeoLongValue);

        switch (GeoLongValue)
            {
            case TIFFGeo_ModelTypeProjected:
                World = HGF2DWorld_HMRWORLD;
                break;

            case TIFFGeo_ModelTypeGeographic:
                World = HGF2DWorld_GEOGRAPHIC;
                break;

            case TIFFGeo_ModelTypeSpecialMSJ:
            case TIFFGeo_ModelTypeSpecialMSSE:
                World = HGF2DWorld_INTERGRAPHWORLD;
                break;
                /*
                default :
                    World = HGF2DWorld_GEOTIFFUNKNOWN;
                    break;
                */
            }
        }

    return World;
    }

//-----------------------------------------------------------------------------
// Public
// HasLookAheadByExtent
// Return true if the format supports a look ahead by extent, otherwise false.
//-----------------------------------------------------------------------------
bool HRFErMapperSupportedFile::HasLookAheadByExtent(uint32_t pi_Page) const
    {
    HPRECONDITION(pi_Page == 0);

    return (m_IsECWP == true);
    }

//-----------------------------------------------------------------------------
// Public
// CanPerformLookAhead
// Return true if the format supports look ahead, otherwise false.
//-----------------------------------------------------------------------------
bool HRFErMapperSupportedFile::CanPerformLookAhead(uint32_t pi_Page) const
    {
    return (m_IsECWP == true);
    }

//-----------------------------------------------------------------------------
// Public
// StopLookAhead
// Stop the last look ahead request.
//-----------------------------------------------------------------------------
void HRFErMapperSupportedFile::StopLookAhead(uint32_t pi_Page,
                                                    uint32_t pi_ConsumerID)
    {
    // the LookAhead is not threaded, do nothing
    }

//-----------------------------------------------------------------------------
// Protected
// GetRatio
// Get the main resolution to requested sub-resolution ratio.
//-----------------------------------------------------------------------------
double HRFErMapperSupportedFile::GetRatio(unsigned short pi_ResolutionNb)
    {
    return m_pRatio[pi_ResolutionNb];
    }

//-----------------------------------------------------------------------------
// Protected
// GetBandList
// Get the band list.
//-----------------------------------------------------------------------------
uint32_t* HRFErMapperSupportedFile::GetBandList()
    {
    return m_pBandList;
    }

#endif // IPP_HAVE_ERMAPPER_SUPPORT
