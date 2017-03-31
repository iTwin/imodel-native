//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFBsbFile.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFBsbFile
//-----------------------------------------------------------------------------
// This class describes a File Raster image.


#include <ImageppInternal.h>

#ifdef IPP_HAVE_GDAL_SUPPORT
#include <ImagePP/all/h/HRFBsbFile.h>
#include <ImagePP/all/h/HFCURLFile.h>
#include <ImagePP/all/h/HRPPixelTypeFactory.h>
#include <ImagePP/all/h/HFCBinStream.h>
#include <ImagePP/all/h/HRFUtility.h>
#include <ImagePP/all/h/HFCException.h>
#include <ImagePP/all/h/HRFException.h>

#include <ImagePP/all/h/HRPPixelTypeI8R8G8B8.h>

#include <ImagePP/all/h/HCDCodecIdentity.h>
#include <ImagePP/all/h/HRFRasterFileCapabilities.h>

#include <ImagePP/all/h/HGF2DAffine.h>
#include <ImagePP/all/h/HGF2DIdentity.h>
#include <ImagePP/all/h/HGF2DProjective.h>
#include <ImagePP/all/h/HGF2DStretch.h>
#include <ImagePP/all/h/HGF2DIdentity.h>
#include <ImagePP/all/h/HGF2DSimilitude.h>
#include <ImagePP/all/h/HGF2DTranslation.h>

#include <ImagePP/all/h/ImagePPMessages.xliff.h>




//GDAL
#include <ImagePP-GdalLib/gdal_priv.h>
#include <ImagePP-GdalLib/cpl_string.h>


//-----------------------------------------------------------------------------
// HRFBsbBlockCapabilities
//-----------------------------------------------------------------------------
class HRFBsbBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFBsbBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Strip Capability
        Add(new HRFStripCapability(HFC_READ_ONLY,          // AccessMode
                                   INT32_MAX,               // MaxSizeInBytes
                                   0,                      // MinHeight
                                   INT32_MAX,               // MaxHeight
                                   1));                    // HeightIncrement

        // Tile Capability
        Add(new HRFTileCapability(HFC_READ_ONLY,            // AccessMode
                                  INT32_MAX,                 // MaxSizeInBytes
                                  0,                        // MinWidth
                                  INT32_MAX,                 // MaxWidth
                                  1,                        // WidthIncrement
                                  0,                        // MinHeight
                                  INT32_MAX,                 // MaxHeight
                                  1,                        // HeightIncrement
                                  false));                  // Not Square

        // Image Capability
        Add(new HRFImageCapability(HFC_READ_ONLY,           // AccessMode
                                   INT32_MAX,                // MaxSizeInBytes
                                   0,                       // MinWidth
                                   INT32_MAX,                // MaxWidth
                                   0,                       // MinHeight
                                   INT32_MAX));              // MaxHeight

        }
    };


//-----------------------------------------------------------------------------
// HRFBsbCodecCapabilities
//-----------------------------------------------------------------------------
class HRFBsbCodecCapabilities : public HRFRasterFileCapabilities
    {
public :
    // Constructor
    HRFBsbCodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec
        Add (new HRFCodecCapability (   HFC_READ_ONLY,
                                        HCDCodecIdentity::CLASS_ID,
                                        new HRFBsbBlockCapabilities()));
        }
    };


//-----------------------------------------------------------------------------
// HRFBsbCapabilities
//-----------------------------------------------------------------------------
HRFBsbCapabilities::HRFBsbCapabilities()
    : HRFRasterFileCapabilities()
    {
    Add(new HRFPixelTypeCapability( HFC_READ_ONLY,
                                    HRPPixelTypeI8R8G8B8::CLASS_ID,
                                    new HRFBsbCodecCapabilities()));

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

    // NOTE: Determine what geocoding is really supported
    // Geocoding capability
    HFCPtr<HRFGeocodingCapability> pGeocodingCapability(new HRFGeocodingCapability(HFC_READ_ONLY));

    pGeocodingCapability->AddSupportedKey(GTModelType);
    pGeocodingCapability->AddSupportedKey(GTRasterType);
    pGeocodingCapability->AddSupportedKey(GeographicType);

    Add((HFCPtr<HRFCapability>&)pGeocodingCapability);
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeVerticalUnitRatioToMeter));
    }


HFC_IMPLEMENT_SINGLETON(HRFBsbCreator)


//-----------------------------------------------------------------------------
// HRFBsbCreator
// This is the creator to instantiate HMR format
//-----------------------------------------------------------------------------
HRFBsbCreator::HRFBsbCreator()
    : HRFRasterFileCreator(HRFBsbFile::CLASS_ID)
    {
    // HMR capabilities instance member initialization
    m_pCapabilities = 0;
    }


// Identification information
Utf8String HRFBsbCreator::GetLabel() const
    {
    return ImagePPMessages::GetString(ImagePPMessages::FILEFORMAT_BSB()); // BSB File Format
    }


// Identification information
Utf8String HRFBsbCreator::GetSchemes() const
    {
    return HFCURLFile::s_SchemeName();
    }

// Identification information
Utf8String HRFBsbCreator::GetExtensions() const
    {
    return "*.kap";
    }

// allow to Open an image file
HFCPtr<HRFRasterFile> HRFBsbCreator::Create(const HFCPtr<HFCURL>&       pi_rpURL,
                                            HFCAccessMode               pi_AccessMode,
                                            uint64_t                   pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFBsbFile(pi_rpURL, pi_AccessMode, pi_Offset);

    HASSERT(pFile != 0);

    return (pFile);
    }


//-----------------------------------------------------------------------------
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------
bool HRFBsbCreator::IsKindOfFile  (const HFCPtr<HFCURL>& pi_rpURL,
                                    uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);
    
    if(!pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID))
        return false;

    //Will initialize GDal if not already initialize
    HRFGdalSupportedFile::Initialize();

    bool                       Result = false;
    HAutoPtr<HFCBinStream>      pFile;

    // Open the IMG File & place file pointer at the start of the file
    pFile = HFCBinStream::Instanciate(pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

    if (pFile != 0 && pFile->GetLastException() == 0)
        {
        // Check the header validity by looking for a correctly named first record in the first 1000 bytes
        char TestBuffer[1000];
        if(pFile->Read(TestBuffer, sizeof(TestBuffer)) == sizeof(TestBuffer))   // Read successful
            {
            uint32_t i;
            for (i = 0; i < sizeof(TestBuffer) - 4; i++)
                {
                /* Test for "BSB/" */
                if (TestBuffer[i+0] == 'B' &&
                    TestBuffer[i+1] == 'S' &&
                    TestBuffer[i+2] == 'B' &&
                    TestBuffer[i+3] == '/')
                    break;

                /* Test for "NOS/" */
                if (TestBuffer[i+0] == 'N' &&
                    TestBuffer[i+1] == 'O' &&
                    TestBuffer[i+2] == 'S' &&
                    TestBuffer[i+3] == '/')
                    break;

                /* Test for "NOS/" offset by 9 in ASCII for NO1 files */
                if (TestBuffer[i+0] == 'W' &&
                    TestBuffer[i+1] == 'X' &&
                    TestBuffer[i+2] == '\\' &&
                    TestBuffer[i+3] == '8')
                    break;
                }

            if (i < sizeof(TestBuffer) - 4) // Correct first record found
                Result = true;

            }
        }

    return Result;
    }


//-----------------------------------------------------------------------------
// Create or get the singleton capabilities of  File
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFBsbCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFBsbCapabilities();

    return m_pCapabilities;
    }


//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFBsbFile::HRFBsbFile                         (const HFCPtr<HFCURL>&       pi_rURL,
                                                HFCAccessMode               pi_AccessMode,
                                                uint64_t                   pi_Offset)

    : HRFGdalSupportedFile("BSB", pi_rURL, pi_AccessMode, pi_Offset)
    {
    if (GetAccessMode().m_HasCreateAccess || GetAccessMode().m_HasWriteAccess)
        {
        //this is a read-only format
        throw HFCFileReadOnlyException(pi_rURL->GetURL());
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
HRFBsbFile::~HRFBsbFile()
    {
    //All the work of closing the file and GDAL is done in the ancestor class.
    }


//-----------------------------------------------------------------------------
// Protected
// HandleNoDisplayBands
//
//-----------------------------------------------------------------------------
void HRFBsbFile::HandleNoDisplayBands()
    {
    //There should at least be one band in the file.
    HASSERT(m_NbBands >= 1);

    if (m_NbBands >= 1)
        {
        m_PaletteBandInd = 1;
        }
    }

//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors
//-----------------------------------------------------------------------------
void HRFBsbFile::CreateDescriptors()
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
void HRFBsbFile::Save()
    {
    }


//-----------------------------------------------------------------------------
// Public
// AddPage
// File manipulation
//-----------------------------------------------------------------------------
bool HRFBsbFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
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
const HFCPtr<HRFRasterFileCapabilities>& HRFBsbFile::GetCapabilities () const
    {
    return (HRFBsbCreator::GetInstance()->GetCapabilities());
    }


//-----------------------------------------------------------------------------
// Protected
// GetScanLineOrientation
//-----------------------------------------------------------------------------
HRFScanlineOrientation HRFBsbFile::GetScanLineOrientation()const
    {
    return HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL;
    }
#endif
