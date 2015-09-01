//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFWebFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFWebFile
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------

// Precompiled Header
#include <ImagePPInternal/hstdcpp.h>


// HRFWebFile class
#include <ImagePP/all/h/HRFWebFile.h>

// Codec Objects
#include <ImagePP/all/h/HCDCodecIdentity.h>
#include <ImagePP/all/h/HCDCodecFlashpix.h>
#include <ImagePP/all/h/HCDCodecFPXSingleColor.h>
#include <ImagePP/all/h/HCDCodecZlib.h>
#include <ImagePP/all/h/HCDCodecHMRRLE1.h>
#include <ImagePP/all/h/HCDCodecHMRCCITT.h>
#include <ImagePP/all/h/HCDCodecHMRPackBits.h>
#include <ImagePP/all/h/HCDCodecJPEGAlpha.h>

// Others
#include <ImagePP/all/h/HFCStat.h>
#include <ImagePP/all/h/HRFRasterFileFactory.h>
#include <ImagePP/all/h/HRFUtility.h>
#include <ImagePP/all/h/HRPListPixelTypePtrs.h>
#include <ImagePP/all/h/HGF2DIdentity.h>
#include <ImagePP/all/h/HGF2DStretch.h>
#include <ImagePP/all/h/HGF2DAffine.h>
#include <ImagePP/all/h/HGF2DSimilitude.h>
#include <ImagePP/all/h/HGF2DProjective.h>
#include <ImagePP/all/h/HGF2DTranslation.h>
#include <ImagePP/all/h/HFCURLHTTP.h>
#include <ImagePP/all/h/HFCURLECWP.h>
#include <ImagePP/all/h/HFCURLECWPS.h>
#include <ImagePP/all/h/HRFURLInternetImagingHTTP.h>
#include <ImagePP/all/h/HRFVirtualEarthFile.h>
#include <ImagePP/all/h/HFCExclusiveKey.h>

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

// Creators
HFC_IMPLEMENT_SINGLETON(HRFWebFileCreator)


//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Static Member Initialization
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// HRFWebFileBlockCapabilities
//-----------------------------------------------------------------------------
class HRFWebFileBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFWebFileBlockCapabilities()
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
// HRFWebFileCodecCapabilities
//-----------------------------------------------------------------------------
class HRFWebFileCodecCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFWebFileCodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec IJG (Jpeg)
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecIJG::CLASS_ID,
                                   new HRFWebFileBlockCapabilities()));

        // Codec JPEG Alpha
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecJPEGAlpha::CLASS_ID,
                                   new HRFWebFileBlockCapabilities()));

        // Codec Zlib (Deflate)
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecZlib::CLASS_ID,
                                   new HRFWebFileBlockCapabilities()));
        // Codec Flashpix
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecFlashpix::CLASS_ID,
                                   new HRFWebFileBlockCapabilities()));
        // Codec FPX Single Color
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecFPXSingleColor::CLASS_ID,
                                   new HRFWebFileBlockCapabilities()));
        // Codec HMR RLE1
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecHMRRLE1::CLASS_ID,
                                   new HRFWebFileBlockCapabilities()));
        // Codec HMR CCITT
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecHMRCCITT::CLASS_ID,
                                   new HRFWebFileBlockCapabilities()));
        // Codec HMR PackBits
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecHMRPackBits::CLASS_ID,
                                   new HRFWebFileBlockCapabilities()));
        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFWebFileBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFWebFileCapabilities
//-----------------------------------------------------------------------------
HRFWebFileCapabilities::HRFWebFileCapabilities()
    : HRFRasterFileCapabilities()
    {
    // Pixel Types
    HRPListPixelTypePtrs PixelTypes;
    HRPListPixelTypePtrs::ListPixelTypePtrs::const_iterator Itr(PixelTypes.m_List.begin());
    while (Itr != PixelTypes.m_List.end())
        {
        HFCPtr<HRFPixelTypeCapability> pPixelType;
        pPixelType = new HRFPixelTypeCapability(HFC_READ_ONLY,
                                                ((HRPPixelType*)(*Itr))->GetClassID(),
                                                new HRFWebFileCodecCapabilities());
        Add((HFCPtr<HRFCapability>)pPixelType);

        Itr++;
        }

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
        false);         // ArbitaryYRatio);
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
// Public
// Constructor
//-----------------------------------------------------------------------------
HRFWebFile::HRFWebFile
(
    const HFCPtr<HFCURL>& pi_rpURL,
    HFCAccessMode         pi_AccessMode,
    uint64_t             pi_Offset
) :HRFRasterFile(pi_rpURL, pi_AccessMode, pi_Offset)
    {
    HPRECONDITION(!pi_AccessMode.m_HasWriteAccess);
    HPRECONDITION(!pi_AccessMode.m_HasCreateAccess);
    HPRECONDITION(pi_Offset == 0);
    HPRECONDITION(HRFWebFileCreator::GetInstance()->IsKindOfFile(pi_rpURL));

    WChar filename[MAX_PATH];

    // URLDownloadToCacheFile(...)
    // - always return a file path if SUCCESS
    // Flags :
    //  URLOSTRM_GETNEWESTVERSION       Download the resource from the Internet, if it is newer, and store it in the cache.
    //  URLOSTRM_USECACHEDCOPY          Download the resource from the cache if it is available; otherwise, download it from the Internet.
    //  URLOSTRM_USECACHEDCOPY_ONLY     Only download the resource from the cache.
#ifdef _WIN32
    if(0 != URLDownloadToCacheFileW(NULL, pi_rpURL->GetURL().c_str(), filename, URLOSTRM_GETNEWESTVERSION, 0, NULL))
#endif
        {
        throw HRFCannotDownloadToInternetCacheException(pi_rpURL->GetURL());
        }

    // compute local URL
    WString  localURL(L"file://");
    localURL.append(filename);
    m_pLocalURL = HFCURL::Instanciate(localURL);

    // Instanciate the local file
    m_pLocalRasterFile = HRFRasterFileFactory::GetInstance()->OpenFile(m_pLocalURL, pi_AccessMode, pi_Offset);
    HASSERT(m_pLocalRasterFile != NULL);

    // If the file was opened, copy its descriptors in this object
    if (m_pLocalRasterFile != 0)
        {
        m_IsOpen = true;
        SharingControlCreate();

        // set our page descriptor
        for (uint32_t Page = 0; Page < m_pLocalRasterFile->CountPages(); Page++)
            {
            m_ListOfPageDescriptor.push_back(m_pLocalRasterFile->GetPageDescriptor(Page));
            }
        }
    else
        {
        throw HFCFileNotSupportedException(pi_rpURL->GetURL());
        }
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
//-----------------------------------------------------------------------------
HRFWebFile::~HRFWebFile()
    {
    }

//-----------------------------------------------------------------------------
// Public
// Returns the world to which the image is attached
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFWebFile::GetCapabilities() const
    {
    HPRECONDITION(m_pLocalRasterFile != 0);

    if (m_pLocalRasterFile != 0)
        return m_pLocalRasterFile->GetCapabilities();

    return HRFWebFileCreator::GetInstance()->GetCapabilities();
    }


//-----------------------------------------------------------------------------
// Public
// Returns the world to which the image is attached
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFWebFile::GetWorldIdentificator () const
    {
    HPRECONDITION(m_pLocalRasterFile != 0);

    if (m_pLocalRasterFile != 0)
        return m_pLocalRasterFile->GetWorldIdentificator();

    return HGF2DWorld_UNKNOWNWORLD;
    }

//-----------------------------------------------------------------------------
// Public
// Impossible
//-----------------------------------------------------------------------------
bool HRFWebFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    return (false);
    }


//-----------------------------------------------------------------------------
// Public
// Returns a pointer on the LockManager object of the local file.
//-----------------------------------------------------------------------------
HFCBinStreamLockManager* HRFWebFile::GetLockManager()
    {
    HPRECONDITION (m_pLocalRasterFile != 0);
    return m_pLocalRasterFile->GetLockManager();
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFWebFile::CreateResolutionEditor(uint32_t      pi_Page,
                                                        unsigned short pi_Resolution,
                                                        HFCAccessMode pi_AccessMode)
    {
    HPRECONDITION(m_pLocalRasterFile != 0);
    HPRECONDITION(pi_Page < CountPages());
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(pi_Resolution < GetPageDescriptor(pi_Page)->CountResolutions());
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    return m_pLocalRasterFile->CreateResolutionEditor(pi_Page, pi_Resolution, pi_AccessMode);
    }

//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
void HRFWebFile::Save()
    {
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
HRFWebFileCreator::HRFWebFileCreator()
    : HRFRasterFileCreator(HRFWebFile::CLASS_ID),
      m_Label(L"Web File"),
      m_Schemes(L"http")
    {
    }


//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
WString HRFWebFileCreator::GetLabel() const
    {
    return m_Label;
    }


//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
WString HRFWebFileCreator::GetSchemes() const
    {
    return m_Schemes;
    }

//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
WString HRFWebFileCreator::GetExtensions() const
    {
    return m_Extensions;
    }


//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFWebFileCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFWebFileCapabilities();

    return m_pCapabilities;
    }

//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
bool HRFWebFileCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                      uint64_t             pi_Offset) const
    {
    return ((pi_rpURL->GetSchemeType() == HFCURLHTTP::s_SchemeName()) &&
            !(HRFURLInternetImagingHTTP::IsURLInternetImaging(pi_rpURL) ||
              HRFVirtualEarthFile::IsURLVirtualEarth(*pi_rpURL)));
    }


//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
bool HRFWebFileCreator::SupportsURL(const HFCPtr<HFCURL>& pi_rpURL) const
    {
    return IsKindOfFile(pi_rpURL, 0);
    }
//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFWebFileCreator::Create
(
    const HFCPtr<HFCURL>& pi_rpURL,
    HFCAccessMode         pi_AccessMode,
    uint64_t             pi_Offset
) const
    {
    HPRECONDITION(pi_rpURL != 0);
    HPRECONDITION(IsKindOfFile(pi_rpURL));
    HPRECONDITION(!pi_AccessMode.m_HasWriteAccess);
    HPRECONDITION(!pi_AccessMode.m_HasCreateAccess);
    HPRECONDITION(pi_Offset == 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFWebFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }


//-----------------------------------------------------------------------------
// Instantiate the file implementation
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public
// GetKey
// Returns the exclusive key assigned to this object
//-----------------------------------------------------------------------------
HFCExclusiveKey& HRFWebFile::GetKey() const
    {
    return (HRFRasterFile::GetKey());
    }

//-----------------------------------------------------------------------------
// Public
// GetLocalRasterFile
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFile> & HRFWebFile::GetLocalRasterFile() const
    {
    return m_pLocalRasterFile;
    }

//-----------------------------------------------------------------------------
// Public
// GetURL
// File information
//-----------------------------------------------------------------------------
const HFCPtr<HFCURL>& HRFWebFile::GetLocalURL() const
    {
    return m_pLocalURL;
    }

//-----------------------------------------------------------------------------
// Public
// Creates an instance of the HRFSharingControl class.
//-----------------------------------------------------------------------------
void HRFWebFile::SharingControlCreate()
    {
    HPRECONDITION (GetLocalURL() != 0);

    if (m_pSharingControl == 0)
        m_pSharingControl = new HRFSisterFileSharing(GetLocalURL(), GetAccessMode());
    }