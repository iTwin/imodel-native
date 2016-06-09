//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFArcInfoAsciiGridFile.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFArcInfoAsciiGridFile
//-----------------------------------------------------------------------------
// This class describes a File Raster image.

#include <ImageppInternal.h>

#ifdef IPP_HAVE_GDAL_SUPPORT

#include <Imagepp/all/h/HRFArcInfoAsciiGridFile.h>
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


namespace {

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                   Raymond.Gauthier   12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct Property
    {
    const char*                         str;
    size_t                              strLen;

    template <size_t NAME_SIZE>
    explicit                            Property                                   (const char (&n)[NAME_SIZE])
        :   str(n),
            strLen(NAME_SIZE - 1) {}
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                   Raymond.Gauthier   12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const struct PropertyLess : std::binary_function<Property, Property, bool>
    {
    bool operator() (const Property& lhs, const Property& rhs) const    { return 0 > BeStringUtilities::Stricmp(lhs.str, rhs.str); }
    bool operator() (const char* lhs, const Property& rhs) const        { return 0 > BeStringUtilities::Strnicmp(lhs, rhs.str, rhs.strLen); }
    bool operator() (const Property& lhs, const char* rhs) const        { return 0 > BeStringUtilities::Strnicmp(lhs.str, rhs, lhs.strLen); }
    } PROPERTY_LESS;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                   Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const struct PropertyEqual : std::binary_function<Property, Property, bool>
    {
    bool operator() (const Property& lhs, const Property& rhs) const    { return 0 == BeStringUtilities::Stricmp(lhs.str, rhs.str); }
    bool operator() (const char* lhs, const Property& rhs) const        { return 0 == BeStringUtilities::Strnicmp(lhs, rhs.str, rhs.strLen); }
    bool operator() (const Property& lhs, const char* rhs) const        { return 0 == BeStringUtilities::Strnicmp(lhs.str, rhs, lhs.strLen); }
    } PROPERTY_EQUAL;


const struct IsPropertySeparator : public unary_function<char, bool>
    {
    bool operator() (char c) const { return (' ' == c || '\t' == c); }
    } IS_PROPERTY_SEPARATOR;
}


//-----------------------------------------------------------------------------
// HRFArcInfoAsciiGridBlockCapabilities
//-----------------------------------------------------------------------------
class HRFArcInfoAsciiGridBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFArcInfoAsciiGridBlockCapabilities()
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
// HRFArcInfoAsciiGridCodecCapabilities
//-----------------------------------------------------------------------------
class HRFArcInfoAsciiGridCodecCapabilities : public HRFRasterFileCapabilities
    {
public :
    // Constructor
    HRFArcInfoAsciiGridCodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFArcInfoAsciiGridBlockCapabilities()));
        }
    };


//-----------------------------------------------------------------------------
// HRFArcInfoAsciiGridCapabilities
//-----------------------------------------------------------------------------
HRFArcInfoAsciiGridCapabilities::HRFArcInfoAsciiGridCapabilities()
    : HRFRasterFileCapabilities()
    {
    // NOTE: Actually, pixel type V8 Gray 8 does not support No Data Values. However, it is
    // theoretically possible for a grid to store 8 bits data AND specify a No Data Value. We
    // will probably need to support the case in later version, but for now, we found nothing
    // like this case in practice.
    Add(new HRFPixelTypeCapability( HFC_READ_ONLY,
                                    HRPPixelTypeV8Gray8::CLASS_ID,
                                    new HRFArcInfoAsciiGridCodecCapabilities()));

    Add(new HRFPixelTypeCapability( HFC_READ_ONLY,
                                    HRPPixelTypeV16Int16::CLASS_ID,
                                    new HRFArcInfoAsciiGridCodecCapabilities()));

    Add(new HRFPixelTypeCapability( HFC_READ_ONLY,
                                    HRPPixelTypeV32Float32::CLASS_ID,
                                    new HRFArcInfoAsciiGridCodecCapabilities()));

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


HFC_IMPLEMENT_SINGLETON(HRFArcInfoAsciiGridCreator)



//-----------------------------------------------------------------------------
// HRFArcInfoAsciiGridCreator
// This is the creator to instantiate HMR format
//-----------------------------------------------------------------------------
HRFArcInfoAsciiGridCreator::HRFArcInfoAsciiGridCreator()
    : HRFRasterFileCreator(HRFArcInfoAsciiGridFile::CLASS_ID)
    {
    // HMR capabilities instance member initialization
    m_pCapabilities = 0;
    
    }


// Identification information
Utf8String HRFArcInfoAsciiGridCreator::GetLabel() const
    {
    return ImagePPMessages::GetString(ImagePPMessages::FILEFORMAT_ArcInfoASCII());
    }


// Identification information
Utf8String HRFArcInfoAsciiGridCreator::GetSchemes() const
    {
    return HFCURLFile::s_SchemeName();
    }


// Identification information
Utf8String HRFArcInfoAsciiGridCreator::GetExtensions() const
    {
    return "*.asc;*.grd";
    }


//-----------------------------------------------------------------------------
// Allow to Open an image file READ/WRITE and CREATE
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFArcInfoAsciiGridCreator::Create   (const HFCPtr<HFCURL>& pi_rpURL,
                                                            HFCAccessMode         pi_AccessMode,
                                                            uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFArcInfoAsciiGridFile(pi_rpURL, pi_AccessMode, pi_Offset);

    HASSERT(pFile != 0);

    return pFile;
    }


//-----------------------------------------------------------------------------
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------
bool HRFArcInfoAsciiGridCreator::IsKindOfFile (const HFCPtr<HFCURL>&    pi_rpURL,
                                                uint64_t                pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    if(!pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID))
        return false;

    //Will initialize GDal if not already initialize
    HRFGdalSupportedFile::Initialize();

    /*
     * Entries in this set are assumed to be manually ordered to the same result as
     * if it was sorted using std::sort(PROPERTY_SET, PROPERTY_SET_END, PropertyLess()).
     */
    static const Property PROPERTY_CELLSIZE("CELLSIZE");
    static const Property PROPERTY_NCOLS("NCOLS");
    static const Property PROPERTY_NODATA_VALUE ("NODATA_VALUE");
    static const Property PROPERTY_NROWS("NROWS");
    static const Property PROPERTY_XLLCENTER("XLLCENTER");
    static const Property PROPERTY_XLLCORNER("XLLCORNER");
    static const Property PROPERTY_YLLCENTER("YLLCENTER");
    static const Property PROPERTY_YLLCORNER("YLLCORNER");
    static const Property          PROPERTY_SET[] = 
        {
        PROPERTY_CELLSIZE,
        PROPERTY_NCOLS,
        PROPERTY_NODATA_VALUE,
        PROPERTY_NROWS,
        PROPERTY_XLLCENTER,
        PROPERTY_XLLCORNER,
        PROPERTY_YLLCENTER,
        PROPERTY_YLLCORNER,
        };

    static const size_t            PROPERTY_SET_SIZE = sizeof(PROPERTY_SET) / sizeof(PROPERTY_SET[0]);
    static const Property* const   PROPERTY_SET_END = PROPERTY_SET + PROPERTY_SET_SIZE;

#ifndef NDEBUG
    // TDORAY: Use std::is_sorted when available.
    static const bool SET_SORTED 
        = (PROPERTY_SET_END == std::adjacent_find(PROPERTY_SET, PROPERTY_SET_END, not2(PROPERTY_LESS)));
    HPOSTCONDITION(SET_SORTED);
#endif //!NDEBUG

    static const size_t FILE_HEADER_SAMPLE_LGT      = 100;
    bool                Success                     = FALSE;

    unsigned char       HeaderSampleBuffer[FILE_HEADER_SAMPLE_LGT];

    HFCPtr<HFCURLFile> pFileURL(new HFCURLFile(pi_rpURL->GetURL()));

    HAutoPtr<HFCBinStream> pFile(HFCBinStream::Instanciate(pi_rpURL, HFC_READ_ONLY | HFC_SHARE_READ_WRITE));

    if (pFile != 0 && pFile->GetLastException() == NULL)
        {
        const size_t HeaderSampleLgt = pFile->Read(HeaderSampleBuffer, FILE_HEADER_SAMPLE_LGT);

        const char* const HeaderSampleBegin = reinterpret_cast<char*>(HeaderSampleBuffer);
        const char* const HeaderSampleBufferEnd = HeaderSampleBegin + HeaderSampleLgt;

        static const size_t  MAX_SAMPLED_PROPERTY_COUNT = 5;
        static const size_t  MIN_SAMPLED_PROPERTY_COUNT = 2;

        const char* NewLineBegin = HeaderSampleBegin;
        bool  FoundInvalidProperty = false;
        size_t  FoundPropertyCount = 0;

        for (; FoundPropertyCount < MAX_SAMPLED_PROPERTY_COUNT; ++FoundPropertyCount)
            {
            const char* PropertyBegin = find_if(NewLineBegin, HeaderSampleBufferEnd, not1(IS_PROPERTY_SEPARATOR));

            NewLineBegin = find(PropertyBegin, HeaderSampleBufferEnd, '\n');
            if (HeaderSampleBufferEnd == NewLineBegin)
                break; // Reached end of sample

            ++NewLineBegin; // Skip newLine char

            const Property* FoundIt 
                = lower_bound(PROPERTY_SET, PROPERTY_SET_END, PropertyBegin, PROPERTY_LESS);

            if (FoundIt == PROPERTY_SET_END || !PROPERTY_EQUAL(*FoundIt, PropertyBegin))
                {
                FoundInvalidProperty = true;
                break; // Found invalid property
                }
            }

        // In order for the file to be recognized, no invalid 
        Success = !FoundInvalidProperty && 
                  (MIN_SAMPLED_PROPERTY_COUNT <= FoundPropertyCount);
        }

    return Success;
    }


//-----------------------------------------------------------------------------
// Create or get the singleton capabilities of  File
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFArcInfoAsciiGridCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFArcInfoAsciiGridCapabilities();

    return m_pCapabilities;
    }


//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFArcInfoAsciiGridFile::HRFArcInfoAsciiGridFile   (const HFCPtr<HFCURL>&           pi_rpURL,
                                                    HFCAccessMode                   pi_AccessMode,
                                                    uint64_t                       pi_Offset)
    : HRFGdalSupportedFile("AAIGrid", pi_rpURL, pi_AccessMode, pi_Offset)
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
HRFArcInfoAsciiGridFile::~HRFArcInfoAsciiGridFile()
    {
    //All the work of closing the file and GDAL is done in the ancestor class.
    }

//-----------------------------------------------------------------------------
// Protected
// HandleNoDisplayBands
//-----------------------------------------------------------------------------
void HRFArcInfoAsciiGridFile::HandleNoDisplayBands()
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
void HRFArcInfoAsciiGridFile::CreateDescriptors()
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
void HRFArcInfoAsciiGridFile::Save()
    {
    // Read Only File
    }


//-----------------------------------------------------------------------------
// Public
// AddPage
// File manipulation
//-----------------------------------------------------------------------------
bool HRFArcInfoAsciiGridFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
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
const HFCPtr<HRFRasterFileCapabilities>& HRFArcInfoAsciiGridFile::GetCapabilities() const
    {
    return (HRFArcInfoAsciiGridCreator::GetInstance()->GetCapabilities());
    }


//-----------------------------------------------------------------------------
// Protected
// GetScanLineOrientation
//-----------------------------------------------------------------------------
HRFScanlineOrientation HRFArcInfoAsciiGridFile::GetScanLineOrientation() const
    {
    return HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL;
    }


#endif