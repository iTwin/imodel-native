//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFGSGAsciiGridFile.cpp $
//:>
//:>  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFGSGAsciiGridFile
//-----------------------------------------------------------------------------
// This class describes a File Raster image.

#include <ImageppInternal.h>

#ifdef IPP_HAVE_GDAL_SUPPORT
#include <ImagePP/all/h/HRFGSGAsciiGridFile.h>
#include <ImagePP/all/h/HFCURLFile.h>
#include <ImagePP/all/h/HFCBinStream.h>
#include <ImagePP/all/h/HRFUtility.h>
#include <ImagePP/all/h/HFCException.h>
#include <ImagePP/all/h/HTIFFUtils.h>

#include <ImagePP/all/h/HRPPixelTypeV32Float32.h>

#include <ImagePP/all/h/HCDCodecIdentity.h>
#include <ImagePP/all/h/HRFRasterFileCapabilities.h>

#include <ImagePP/all/h/HGF2DIdentity.h>
#include <ImagePP/all/h/HGF2DStretch.h>
#include <ImagePP/all/h/HGF2DTranslation.h>

#include <ImagePP/all/h/ImagePPMessages.xliff.h>

//GDAL
#include <ImagePP-GdalLib/gdal_priv.h>
#include <ImagePP-GdalLib/cpl_string.h>

USING_NAMESPACE_IMAGEPP

//-----------------------------------------------------------------------------
// HRFGSGAsciiGridBlockCapabilities
//-----------------------------------------------------------------------------
class HRFGSGAsciiGridBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFGSGAsciiGridBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Strip Capability
        Add(new HRFStripCapability( HFC_READ_ONLY,          // AccessMode
                                    LONG_MAX,               // MaxSizeInBytes
                                    0,                      // MinHeight
                                    LONG_MAX,               // MaxHeight
                                    1));                    // HeightIncrement

        // Tile Capability
        Add(new HRFTileCapability(  HFC_READ_ONLY,            // AccessMode
                                    LONG_MAX,                 // MaxSizeInBytes
                                    0,                        // MinWidth
                                    LONG_MAX,                 // MaxWidth
                                    1,                        // WidthIncrement
                                    0,                        // MinHeight
                                    LONG_MAX,                 // MaxHeight
                                    1,                        // HeightIncrement
                                    false));                  // Not Square

        // Image Capability
        Add(new HRFImageCapability( HFC_READ_ONLY,           // AccessMode
                                    LONG_MAX,                // MaxSizeInBytes
                                    0,                       // MinWidth
                                    LONG_MAX,                // MaxWidth
                                    0,                       // MinHeight
                                    LONG_MAX));              // MaxHeight

        }
    };


//-----------------------------------------------------------------------------
// HRFGSGAsciiGridCodecCapabilities
//-----------------------------------------------------------------------------
class HRFGSGAsciiGridCodecCapabilities : public HRFRasterFileCapabilities
    {
public :
    // Constructor
    HRFGSGAsciiGridCodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFGSGAsciiGridBlockCapabilities()));
        }
    };


//-----------------------------------------------------------------------------
// HRFGSGAsciiGridCapabilities
//-----------------------------------------------------------------------------
HRFGSGAsciiGridCapabilities::HRFGSGAsciiGridCapabilities()
    : HRFRasterFileCapabilities()
    {
    Add(new HRFPixelTypeCapability( HFC_READ_ONLY,
                                    HRPPixelTypeV32Float32::CLASS_ID,
                                    new HRFGSGAsciiGridCodecCapabilities()));

    // Transfo Model
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DStretch::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DTranslation::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DIdentity::CLASS_ID));

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));

    // Single Resolution Capability
    Add(new HRFSingleResolutionCapability(HFC_READ_ONLY));

    // Still Image Capability
    Add(new HRFStillImageCapability(HFC_READ_ONLY));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_ONLY, HRFInterleaveType::PIXEL));

    // NOTE: Determine what geocoding is really supported
    // Geocoding capability
    HFCPtr<HRFGeocodingCapability> pGeocodingCapability(new HRFGeocodingCapability(HFC_READ_ONLY));

    pGeocodingCapability->AddSupportedKey(GTModelType);
    pGeocodingCapability->AddSupportedKey(GTRasterType);
    pGeocodingCapability->AddSupportedKey(GeographicType);

    Add((HFCPtr<HRFCapability>&)pGeocodingCapability);
    }


HFC_IMPLEMENT_SINGLETON(HRFGSGAsciiGridCreator)



//-----------------------------------------------------------------------------
// HRFGSGAsciiGridCreator
// This is the creator to instantiate HMR format
//-----------------------------------------------------------------------------
HRFGSGAsciiGridCreator::HRFGSGAsciiGridCreator()
    : HRFRasterFileCreator(HRFGSGAsciiGridFile::CLASS_ID)
    {
    // Capabilities instance member initialization
    m_pCapabilities = 0;
    }


// Identification information
Utf8String HRFGSGAsciiGridCreator::GetLabel() const
    {
    return ImagePPMessages::GetString(ImagePPMessages::FILEFORMAT_GSGASCII());
    }


// Identification information
Utf8String HRFGSGAsciiGridCreator::GetSchemes() const
    {
    return Utf8String(HFCURLFile::s_SchemeName());
    }


// Identification information
Utf8String HRFGSGAsciiGridCreator::GetExtensions() const
    {
    return "*.grd";
    }


//-----------------------------------------------------------------------------
// Allow to Open an image file READ/WRITE and CREATE
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFGSGAsciiGridCreator::Create   (const HFCPtr<HFCURL>& pi_rpURL,
                                                        HFCAccessMode         pi_AccessMode,
                                                        uint64_t              pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFGSGAsciiGridFile(pi_rpURL, pi_AccessMode, pi_Offset);

    HASSERT(pFile != 0);

    return pFile;
    }


//-----------------------------------------------------------------------------
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------
bool HRFGSGAsciiGridCreator::IsKindOfFile (const HFCPtr<HFCURL>&    pi_rpURL,
                                           uint64_t                  pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    if(!pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID))
        return false;

    //Will initialize GDal if not already initialize
    HRFGdalSupportedFile::Initialize();

    const size_t FILE_HEADER = 10;
    bool  Success            = TRUE;

    unsigned char       HeaderSampleBuffer[FILE_HEADER] = {0};

    HFCPtr<HFCURLFile> pFileURL(new HFCURLFile(pi_rpURL->GetURL()));

    HAutoPtr<HFCBinStream> pFile(HFCBinStream::Instanciate(pi_rpURL, HFC_READ_ONLY | HFC_SHARE_READ_WRITE));

    if (pFile != 0 && pFile->GetLastException() == NULL)
        {
        const size_t HeaderSampleLgt = pFile->Read(HeaderSampleBuffer, FILE_HEADER);

        if (HeaderSampleLgt < FILE_HEADER)
            Success = FALSE;

        /* Check for signature */
PUSH_DISABLE_DEPRECATION_WARNINGS
        else if (!EQUALN((const char *)HeaderSampleBuffer, "DSAA", 4)
POP_DISABLE_DEPRECATION_WARNINGS
            || (HeaderSampleBuffer[4] != '\x0D'
                && HeaderSampleBuffer[4] != '\x0A'))
            {
            Success = FALSE;
            }
        }
    else
        Success = FALSE;

    return Success;
    }


//-----------------------------------------------------------------------------
// Create or get the singleton capabilities of  File
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFGSGAsciiGridCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFGSGAsciiGridCapabilities();

    return m_pCapabilities;
    }


//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFGSGAsciiGridFile::HRFGSGAsciiGridFile   (const HFCPtr<HFCURL>&   pi_rpURL,
                                            HFCAccessMode           pi_AccessMode,
                                            uint64_t                pi_Offset)
    : HRFGdalSupportedFile("GSAG", pi_rpURL, pi_AccessMode, pi_Offset)
    {
    if (GetAccessMode().m_HasCreateAccess || GetAccessMode().m_HasWriteAccess)
        {
        //this is a read-only format
        throw HFCFileReadOnlyException(pi_rpURL->GetURL());
        }

    // The ancestor store the access mode
    m_IsOpen        = false;
    m_GTModelType   = TIFFGeo_Undefined;

    //Open the file. An exception should be thrown if the open failed.
    Open();

    HASSERT(m_IsOpen == true);

    // Create Page and Resolution Descriptors.
    CreateDescriptors();
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFGSGAsciiGridFile::~HRFGSGAsciiGridFile()
    {
    //All the work of closing the file and GDAL is done in the ancestor class.
    }

//-----------------------------------------------------------------------------
// Protected
// HandleNoDisplayBands
//-----------------------------------------------------------------------------
void HRFGSGAsciiGridFile::HandleNoDisplayBands()
    {
    //There should at least be one band in the file.
    HASSERT(m_NbBands >= 1);

    if (m_NbBands >= 1)
        {
        m_GrayBandInd = 1;
        }
    }

//-----------------------------------------------------------------------------
// Protected
// DetectPixelType
//-----------------------------------------------------------------------------
void HRFGSGAsciiGridFile::DetectPixelType()
    {
    m_doubleToFloatCnv = true;
    HRFGdalSupportedFile::DetectPixelType();
    }

//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors
//-----------------------------------------------------------------------------
void HRFGSGAsciiGridFile::CreateDescriptors()
    {
    HPRECONDITION(GetDataSet() != 0);

    // Create Page and resolution Description/Capabilities for this file.
    HPMAttributeSet                         TagList;

    HRFGdalSupportedFile::CreateDescriptorsWith(new HCDCodecIdentity(), TagList);
    }


//-----------------------------------------------------------------------------
// Public
// Save
// Saves the file
//-----------------------------------------------------------------------------
void HRFGSGAsciiGridFile::Save()
    {
    // Read Only File
    }


//-----------------------------------------------------------------------------
// Public
// AddPage
// File manipulation
//-----------------------------------------------------------------------------
bool HRFGSGAsciiGridFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    // No pages are supposed to be added (Read Only file)
    HASSERT(false);
    return false;
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Return the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFGSGAsciiGridFile::GetCapabilities() const
    {
    return (HRFGSGAsciiGridCreator::GetInstance()->GetCapabilities());
    }


//-----------------------------------------------------------------------------
// Protected
// GetScanLineOrientation
//-----------------------------------------------------------------------------
HRFScanlineOrientation HRFGSGAsciiGridFile::GetScanLineOrientation() const
    {
    return HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL;
    }


#endif  // IPP_HAVE_GDAL_SUPPORT