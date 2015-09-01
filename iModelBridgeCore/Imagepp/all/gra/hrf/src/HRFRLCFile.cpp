//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFRLCFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFRLCFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRFRLCFile.h>
#include <Imagepp/all/h/HRFRLCLineEditor.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>
#include <Imagepp/all/h/HRFUtility.h>

#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCBinStream.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HFCAccessMode.h>

#include <Imagepp/all/h/HGF2DProjective.h>
#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DIdentity.h>

#include <Imagepp/all/h/HRPPixelType.h>
#include <Imagepp/all/h/HRPPixelTypeV1Gray1.h>

#include <Imagepp/all/h/HCDCodecIdentity.h>

#include <Imagepp/all/h/HTIFFUtils.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>

//-----------------------------------------------------------------------------
// HRFRLCBlockCapabilities
//-----------------------------------------------------------------------------
class HRFRLCBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFRLCBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Block Capability
        Add(new HRFLineCapability(HFC_READ_ONLY,                // AccessMode
                                  LONG_MAX,                     // MaxWidth
                                  HRFBlockAccess::SEQUENTIAL)); // BlockAccess
        }
    };

//-----------------------------------------------------------------------------
// HRFRLCCodecCapabilities
//-----------------------------------------------------------------------------
class HRFRLCCodecCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFRLCCodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFRLCBlockCapabilities()));
        }
    };


//-----------------------------------------------------------------------------
// HRFRLCCapabilities
//-----------------------------------------------------------------------------
HRFRLCCapabilities::HRFRLCCapabilities()
    : HRFRasterFileCapabilities()
    {
    // PixelTypeV1Gray1
    // Read capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV1Gray1::CLASS_ID,
                                   new HRFRLCCodecCapabilities()));

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));

    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_ONLY));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_ONLY, HRFInterleaveType::PIXEL));

    // Single resolution capability
    Add(new HRFSingleResolutionCapability(HFC_READ_ONLY));

    // Transfo Model
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DProjective::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DAffine::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DSimilitude::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DStretch::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DTranslation::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_ONLY, HGF2DIdentity::CLASS_ID));

    // Tag capability
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeDontSupportPersistentColor(true)));
    }

HFC_IMPLEMENT_SINGLETON(HRFRLCCreator)

//-----------------------------------------------------------------------------
// Creator
// This is the creator to instantiate RLCformat
//-----------------------------------------------------------------------------
HRFRLCCreator::HRFRLCCreator()
    : HRFRasterFileCreator(HRFRLCFile::CLASS_ID)
    {
    // RLC capabilities instance member initialization
    m_pCapabilities = 0;
    }

//-----------------------------------------------------------------------------
// public
// Identification information
//-----------------------------------------------------------------------------
WString HRFRLCCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_RLC());  //RLC File Format
    }

//-----------------------------------------------------------------------------
// public
// Schemes information
//-----------------------------------------------------------------------------
WString HRFRLCCreator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

//-----------------------------------------------------------------------------
// public
// Extensions information
//-----------------------------------------------------------------------------
WString HRFRLCCreator::GetExtensions() const
    {
    return WString(L"*.rlc");
    }

//-----------------------------------------------------------------------------
// public
// allow to Open an image file
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFRLCCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                            HFCAccessMode         pi_AccessMode,
                                            uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFRLCFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }

//-----------------------------------------------------------------------------
// public
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------
bool HRFRLCCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                  uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    bool Result = false;

    if (pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID) &&
        BeStringUtilities::Wcsicmp(((HFCPtr<HFCURLFile>&)pi_rpURL)->GetExtension().c_str(), L"rlc") == 0)
        Result = true;

    return Result;
    }

//-----------------------------------------------------------------------------
// public
// Create or get the singleton capabilities of RLC file.
//-----------------------------------------------------------------------------

const HFCPtr<HRFRasterFileCapabilities>& HRFRLCCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFRLCCapabilities();

    return m_pCapabilities;
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFRLCFile::HRFRLCFile(const HFCPtr<HFCURL>& pi_rURL,
                              HFCAccessMode         pi_AccessMode,
                              uint64_t             pi_Offset)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {

    // The ancestor store the access mode

    // Initialise internal members...
    m_IsOpen    = false;

    if (GetAccessMode().m_HasCreateAccess || GetAccessMode().m_HasWriteAccess)
        {
        //this is a read-only format
        throw HFCFileReadOnlyException(pi_rURL->GetURL());
        }
    else
        {
        if (Open())                 // if Open success and it is not a new file
            CreateDescriptors();    // Create Page and Res Descriptors.
        }
    }

//-----------------------------------------------------------------------------
// Protected
// Constructor
// allow to Create an image file object without open.
//-----------------------------------------------------------------------------
HRFRLCFile::HRFRLCFile(const HFCPtr<HFCURL>& pi_rURL,
                              HFCAccessMode         pi_AccessMode,
                              uint64_t             pi_Offset,
                              bool                 pi_DontOpenFile)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    if (GetAccessMode().m_HasCreateAccess || GetAccessMode().m_HasWriteAccess)
        {
        //this is a read-only format
        throw HFCFileReadOnlyException(pi_rURL->GetURL());
        }

    // Initialise internal members...
    m_IsOpen    = false;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFRLCFile::~HRFRLCFile()
    {
    }

//-----------------------------------------------------------------------------
// Public
// CreateResolutionEditor
// File manipulation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFRLCFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                        unsigned short pi_Resolution,
                                                        HFCAccessMode  pi_AccessMode)
    {
    // verify that the page number is 0, because we have one image per file
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    return new HRFRLCLineEditor(this, pi_Page, pi_Resolution, pi_AccessMode);
    }

//-----------------------------------------------------------------------------
// Public
// Save
// Saves the file
//-----------------------------------------------------------------------------
void HRFRLCFile::Save()
    {
    //Nothing do to here
    }

//-----------------------------------------------------------------------------
// Public
// AddPage
// File manipulation
//-----------------------------------------------------------------------------
bool HRFRLCFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    // Add the page descriptor to the list
    return HRFRasterFile::AddPage(pi_pPage);
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Returnt the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFRLCFile::GetCapabilities () const
    {
    return (HRFRLCCreator::GetInstance()->GetCapabilities());
    }

//-----------------------------------------------------------------------------
// Protected section
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Protected
// This method open the file.
//-----------------------------------------------------------------------------
bool HRFRLCFile::Open()
    {
    HPRECONDITION(!m_IsOpen);
    HPRECONDITION(!m_pRLCFile);

    // Be sure the RLC raster file is NOT already open.
    m_pRLCFile = HFCBinStream::Instanciate(GetURL(), m_Offset, GetAccessMode(), 0, true);

    m_IsOpen = true;

    // This creates the sister file for file sharing control if necessary.
    SharingControlCreate();

    return m_IsOpen;
    }

//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors
//-----------------------------------------------------------------------------
void HRFRLCFile::CreateDescriptors ()
    {
    // read the image size
    unsigned short aBuffer[2];
    m_pRLCFile->Read(aBuffer, 2 * sizeof(unsigned short));

    // The file was write in little endian, we must swap bytes for big endian
    // this method come from HTIFFUtils
    SwabArrayOfShort(aBuffer, 2);

    m_Height = aBuffer[0];
    m_Width  = aBuffer[1];

    HFCPtr<HGF2DTransfoModel> pTransfoModel = new HGF2DIdentity();

    FindImageParams();

    // Obtain Resolution Information
    // resolution dimension
    HFCPtr<HRFResolutionDescriptor> pResolution =  new HRFResolutionDescriptor(
        GetAccessMode(),                                // AccessMode,
        GetCapabilities(),                              // Capabilities,
        1.0,                                            // XResolutionRatio,
        1.0,                                            // YResolutionRatio,
        new HRPPixelTypeV1Gray1(),                      // PixelType,
        new HCDCodecIdentity(),                         // Codec
        HRFBlockAccess::SEQUENTIAL,                     // RStorageAccess,
        HRFBlockAccess::SEQUENTIAL,                     // WStorageAccess,
        HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL,  // ScanLineOrientation,
        HRFInterleaveType::PIXEL,                       // InterleaveType
        false,                                          // IsInterlace
        m_Width,                                        // Width,
        m_Height,                                       // Height,
        m_Width,                                        // BlockWidth,
        1,                                              // BlockHeight,
        0,                                              // BlocksDataFlag
        HRFBlockType::LINE);                            // BlockType

    // BACKGROUND Tag
    HPMAttributeSet TagList;
    HFCPtr<HPMGenericAttribute> pTag;

    // Persistent color Tag
    pTag = new HRFAttributeDontSupportPersistentColor(true);
    TagList.Set(pTag);

    HFCPtr<HRFPageDescriptor> pPage;
    pPage = new HRFPageDescriptor (GetAccessMode(),
                                   GetCapabilities(),           // Capabilities
                                   pResolution,                 // ResolutionDescriptor
                                   0,                           // RepresentativePalette
                                   0,                           // Histogram
                                   0,                           // Thumbnail
                                   0,                           // ClipShape
                                   pTransfoModel,               // TransfoModel
                                   0,                           // Filters
                                   &TagList);                   // Defined Tag

    m_ListOfPageDescriptor.push_back(pPage);
    }


void HRFRLCFile::FindImageParams()
    {
#if 0
    Byte TempBuffer[16];
    Byte Magic[16];

    Magic[0]  = 0;
    Magic[ 1] = 0;
    Magic[ 2] = 0;
    Magic[ 3] = 0;
    Magic[ 4] = 0;
    Magic[ 5] = 0;
    Magic[ 6] = 0;
    Magic[ 7] = 0;
    Magic[ 8] = 26;
    Magic[ 9] = 26;
    Magic[10] = 0x55;
    Magic[11] = 0x64;
    Magic[12] = 0x73;
    Magic[13] = (char) 0x82;
    Magic[14] = 0x43;
    Magic[15] = 0x4a;

    m_pRLCFile->SeekToEnd();

    uint64_t HasReadSucceeded;
    bool   IsFileDescFound = false;

    rlc_file_description

    do
        {
        m_pRLCFile->Seek(-16);

        //When the seek is invalid the file isn't read
        HasReadSucceeded = (m_pRLCFile->Read(TempBuffer, 16) == 16);

        if (HasReadSucceeded)
            IsFileDescFound = (memcmp(TempBuffer, Magic, 16) == 0);
        }
    while (HasReadSucceeded && !IsFileDescFound)

        if (IsFileDescFound == false)
            {
//        throw HRFException(HRF_MAGIC_NUMBERS_CANNOT_BE_FOUND_EXCEPTION, m_pRLCFile->GetURL());
            }

    m_pRLCFile->Seek(sizeof(rlc_raster_header));

    rlc_raster_header RasterHeader;

    if (m_pRLCFile->Read(RasterHeader, rlc_raster_header) == sizeof(rlc_raster_header))
        {
#if 0
        double x_pos, y_pos;        /* lower left of image in world */
        double scale;           /* always 1.0 */
        double density;         /* pixel density image in world */
        double skew;            /* skew angle of image in world */
        short  skip;            /* line skipping factor */
        short  speckle;         /* 0 - 255 */
        short  rotate;          /* 0=0; 1=90; 2=180; 3=270 */
        short  mode;            /* bitwise image mode */
        short  color;           /* 1-255 */
        short  split;           /* always 0 */
        long   reserved;        /* always 0 */
        char   filename[144];       /* original name of file */
#endif
        //The image paramaters are in little endian order, so swap them.
        //Swaps first five double parameters
        SwabArrayOfDouble(&RasterHeader.iparms, 5);

        //Swap next five short parameters
        SwabArrayOfShort(&(RasterHeader.iparms.skip), 5);

        //The split and reserved fields don't have to be swapped,
        //since they are always 0.

        m_pImageParams = new rlc_image_parms;
        m_pImageParams = RasterHeader.iparms;
        }
    else
        {
        //throw ()
        }
#endif
    }

//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
// File information
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFRLCFile::GetWorldIdentificator () const
    {
    return HGF2DWorld_UNKNOWNWORLD;
    }

//-----------------------------------------------------------------------------
// Public
// GetRLCFilePtr
// File information
//-----------------------------------------------------------------------------
const HFCBinStream* HRFRLCFile::GetRLCFilePtr() const
    {
    return m_pRLCFile;
    }
