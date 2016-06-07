//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFArcInfoGridFile.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFArcInfoGridFile
//-----------------------------------------------------------------------------
// This class describes a File Raster image.

#include <ImageppInternal.h>

#ifdef IPP_HAVE_GDAL_SUPPORT

#include <Imagepp/all/h/HRFArcInfoGridFile.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCBinStream.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HTIFFUtils.h>

#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV16Int16.h>
#include <Imagepp/all/h/HRPPixelTypeV32Float32.h>

#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>

#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DProjective.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DTranslation.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>

//GDAL
#include <ImagePP-GdalLib/gdal_priv.h>
#include <ImagePP-GdalLib/cpl_string.h>




//-----------------------------------------------------------------------------
// HRFArcInfoGridBlockCapabilities
//-----------------------------------------------------------------------------
class HRFArcInfoGridBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFArcInfoGridBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Strip Capability
        Add(new HRFStripCapability( HFC_READ_ONLY,          // AccessMode
                                   INT32_MAX,               // MaxSizeInBytes
                                    0,                      // MinHeight
                                   INT32_MAX,               // MaxHeight
                                    1));                    // HeightIncrement

        // Tile Capability
        Add(new HRFTileCapability(  HFC_READ_ONLY,            // AccessMode
                                  INT32_MAX,                 // MaxSizeInBytes
                                    0,                        // MinWidth
                                  INT32_MAX,                 // MaxWidth
                                    1,                        // WidthIncrement
                                    0,                        // MinHeight
                                  INT32_MAX,                 // MaxHeight
                                    1,                        // HeightIncrement
                                    false));                  // Not Square

        // Image Capability
        Add(new HRFImageCapability( HFC_READ_ONLY,           // AccessMode
                                   INT32_MAX,                // MaxSizeInBytes
                                    0,                       // MinWidth
                                   INT32_MAX,                // MaxWidth
                                    0,                       // MinHeight
                                   INT32_MAX));              // MaxHeight

        }
    };


//-----------------------------------------------------------------------------
// HRFArcInfoGridCodecCapabilities
//-----------------------------------------------------------------------------
class HRFArcInfoGridCodecCapabilities : public HRFRasterFileCapabilities
    {
public :
    // Constructor
    HRFArcInfoGridCodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFArcInfoGridBlockCapabilities()));
        }
    };


//-----------------------------------------------------------------------------
// HRFArcInfoGridCapabilities
//-----------------------------------------------------------------------------
HRFArcInfoGridCapabilities::HRFArcInfoGridCapabilities()
    : HRFRasterFileCapabilities()
    {
    // NOTE: Actually, pixel type V8 Gray 8 does not support No Data Values. However, it is
    // theoretically possible for a grid to store 8 bits data AND specify a No Data Value. We
    // will probably need to support the case in later version, but for now, we found nothing
    // like this case in practice.
    Add(new HRFPixelTypeCapability( HFC_READ_ONLY,
                                    HRPPixelTypeV8Gray8::CLASS_ID,
                                    new HRFArcInfoGridCodecCapabilities()));

    Add(new HRFPixelTypeCapability( HFC_READ_ONLY,
                                    HRPPixelTypeV16Int16::CLASS_ID,
                                    new HRFArcInfoGridCodecCapabilities()));

    Add(new HRFPixelTypeCapability( HFC_READ_ONLY,
                                    HRPPixelTypeV32Float32::CLASS_ID,
                                    new HRFArcInfoGridCodecCapabilities()));

    // Transfo Model
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DAffine::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DSimilitude::CLASS_ID));
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

    // Geocoding capability
    HFCPtr<HRFGeocodingCapability> pGeocodingCapability(new HRFGeocodingCapability(HFC_READ_ONLY));

    pGeocodingCapability->AddSupportedKey(GTModelType);
    pGeocodingCapability->AddSupportedKey(GTRasterType);
    pGeocodingCapability->AddSupportedKey(GeographicType);

    Add((HFCPtr<HRFCapability>&)pGeocodingCapability);
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeVerticalUnitRatioToMeter));
    }


HFC_IMPLEMENT_SINGLETON(HRFArcInfoGridCreator)


//-----------------------------------------------------------------------------
// HRFArcInfoGridCreator
// This is the creator to instantiate HMR format
//-----------------------------------------------------------------------------
HRFArcInfoGridCreator::HRFArcInfoGridCreator()
    : HRFRasterFileCreator(HRFArcInfoGridFile::CLASS_ID)
    {
    // HMR capabilities instance member initialization
    m_pCapabilities = 0;
    }


// Identification information
Utf8String HRFArcInfoGridCreator::GetLabel() const
    {
    return ImagePPMessages::GetString(ImagePPMessages::FILEFORMAT_ArcInfoBinary());
    }


// Identification information
Utf8String HRFArcInfoGridCreator::GetSchemes() const
    {
    return HFCURLFile::s_SchemeName();
    }


// Identification information
Utf8String HRFArcInfoGridCreator::GetExtensions() const
    {
    return "*.adf";
    }


//-----------------------------------------------------------------------------
// Allow to Open an image file READ/WRITE and CREATE
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFArcInfoGridCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                                    HFCAccessMode         pi_AccessMode,
                                                    uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFArcInfoGridFile(pi_rpURL, pi_AccessMode, pi_Offset);

    HASSERT(pFile != 0);

    return pFile;
    }


//-----------------------------------------------------------------------------
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------
bool HRFArcInfoGridCreator::IsKindOfFile  (const HFCPtr<HFCURL>&    pi_rpURL,
                                            uint64_t                pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);
    
    if(!pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID))
        return false;

    //Will initialize GDal if not already initialize
    HRFGdalSupportedFile::Initialize();

    static const uint32_t        HEADER_FIELD_MAX_LGT    = 8;
    static const uint32_t        LONG_FIELD              = HEADER_FIELD_MAX_LGT;
    static const uint32_t        SHORT_FIELD             = HEADER_FIELD_MAX_LGT/2;

    bool                       Success                 = false;
    Byte                       HdrFieldBuffer[HEADER_FIELD_MAX_LGT];


    // Extension need to be *.adf in order to do any further processing on the file
    Utf8String FileExtension = (static_cast<HFCURLFile*>(pi_rpURL.GetPtr()))->GetExtension();
    if (0 != FileExtension.compare("adf"))
        return false;

    // The header of this file is external. Any *.adf file in the grid directory could be selected.
    HFCPtr<HFCURL> pHeaderURL(new HFCURLFile(pi_rpURL->GetURL()));
    (static_cast<HFCURLFile*>(pHeaderURL.GetPtr()))->SetFileName(Utf8String("hdr.adf"));

    HAutoPtr<HFCBinStream> pHeaderFile(HFCBinStream::Instanciate(pHeaderURL, HFC_READ_ONLY | HFC_SHARE_READ_WRITE));

    if (pHeaderFile != 0 && pHeaderFile->GetLastException() == NULL)
        {
        pHeaderFile->Read(HdrFieldBuffer, LONG_FIELD);
        string MagicString(HdrFieldBuffer, HdrFieldBuffer + (LONG_FIELD - 1));

        if (0 == MagicString.compare("GRID1.2"))
            {
            pHeaderFile->SeekToPos(16);
            pHeaderFile->Read(HdrFieldBuffer, SHORT_FIELD);
            uint32_t* pCellType = reinterpret_cast<uint32_t*>(HdrFieldBuffer);
            SwabArrayOfLong(pCellType, 1);

            if (1 == *pCellType || 2 == *pCellType)
                {
                Success = true;
                }
            }
        }

    return Success;
    }


//-----------------------------------------------------------------------------
// Create or get the singleton capabilities of  File
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFArcInfoGridCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFArcInfoGridCapabilities();

    return m_pCapabilities;
    }


//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFArcInfoGridFile::HRFArcInfoGridFile (const HFCPtr<HFCURL>&           pi_rpURL,
                                        HFCAccessMode                   pi_AccessMode,
                                        uint64_t                       pi_Offset)
    : HRFGdalSupportedFile("AIG", pi_rpURL, pi_AccessMode, pi_Offset)
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
HRFArcInfoGridFile::~HRFArcInfoGridFile()
    {
    //All the work of closing the file and GDAL is done in the ancestor class.
    }

//-----------------------------------------------------------------------------
// Protected
// HandleNoDisplayBands
//-----------------------------------------------------------------------------
void HRFArcInfoGridFile::HandleNoDisplayBands()
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
// CreateDescriptors
//-----------------------------------------------------------------------------
void HRFArcInfoGridFile::CreateDescriptors()
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
void HRFArcInfoGridFile::Save()
    {
    // Read Only File
    }


//-----------------------------------------------------------------------------
// Public
// AddPage
// File manipulation
//-----------------------------------------------------------------------------
bool HRFArcInfoGridFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
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
const HFCPtr<HRFRasterFileCapabilities>& HRFArcInfoGridFile::GetCapabilities() const
    {
    return (HRFArcInfoGridCreator::GetInstance()->GetCapabilities());
    }


//-----------------------------------------------------------------------------
// Protected
// GetScanLineOrientation
//-----------------------------------------------------------------------------
HRFScanlineOrientation HRFArcInfoGridFile::GetScanLineOrientation() const
    {
    return HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL;
    }


#endif