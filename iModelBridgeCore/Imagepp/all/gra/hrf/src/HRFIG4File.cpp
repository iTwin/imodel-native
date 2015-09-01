//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFIG4File.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFIG4File
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRFIG4File.h>
#include <Imagepp/all/h/HRFIG4StripEditor.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>
#include <Imagepp/all/h/HRFUtility.h>

#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCBinStream.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HFCAccessMode.h>

#include <Imagepp/all/h/HRPPixelType.h>

#include <Imagepp/all/h/HRPPixelTypeV1Gray1.h>

#include <Imagepp/all/h/HCDCodecIdentity.h>

#include <Imagepp/all/h/HTIFFUtils.h>


#include <Imagepp/all/h/HCDCodecHMRCCITT.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>

//-----------------------------------------------------------------------------
// HRFIG4BlockCapabilities
//-----------------------------------------------------------------------------
class HRFIG4BlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFIG4BlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Block Capability

        // Strip Capability
        Add(new HRFStripCapability(HFC_READ_ONLY,          // AccessMode
                                   LONG_MAX,               // MaxSizeInBytes
                                   1,                      // MinHeight
                                   LONG_MAX,               // MaxHeight
                                   1,                      // HeightIncrement
                                   HRFBlockAccess::SEQUENTIAL));
        }
    };

//-----------------------------------------------------------------------------
// HRFIG4CodecCapabilities
//-----------------------------------------------------------------------------
class HRFIG4CodecCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFIG4CodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Identity
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFIG4BlockCapabilities()));
        }
    };


//-----------------------------------------------------------------------------
// HRFIG4Capabilities
//-----------------------------------------------------------------------------
HRFIG4Capabilities::HRFIG4Capabilities()
    : HRFRasterFileCapabilities()
    {
    // PixelTypeV1Gray1
    // Read capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeV1Gray1::CLASS_ID,
                                   new HRFIG4CodecCapabilities()));

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));

    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_ONLY));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_ONLY, HRFInterleaveType::PIXEL));

    // Single resolution capability
    Add(new HRFSingleResolutionCapability(HFC_READ_ONLY));
    }

HFC_IMPLEMENT_SINGLETON(HRFIG4Creator)

//-----------------------------------------------------------------------------
// Creator
// This is the creator to instantiate IG4format
//-----------------------------------------------------------------------------
HRFIG4Creator::HRFIG4Creator()
    : HRFRasterFileCreator(HRFIG4File::CLASS_ID)
    {
    // IG4 capabilities instance member initialization
    m_pCapabilities = 0;
    }

//-----------------------------------------------------------------------------
// public
// Identification information
//-----------------------------------------------------------------------------
WString HRFIG4Creator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::GRA_HRF_IG4_FILE_FORMAT());  //IG4 File Format
    }

//-----------------------------------------------------------------------------
// public
// Schemes information
//-----------------------------------------------------------------------------
WString HRFIG4Creator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

//-----------------------------------------------------------------------------
// public
// Extensions information
//-----------------------------------------------------------------------------
WString HRFIG4Creator::GetExtensions() const
    {
    return WString(L"*.ig4");
    }

//-----------------------------------------------------------------------------
// public
// allow to Open an image file
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFIG4Creator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                            HFCAccessMode         pi_AccessMode,
                                            uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFIG4File(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }

//-----------------------------------------------------------------------------
// public
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------
bool HRFIG4Creator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                  uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    bool Result = false;

    if (pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID) &&
        BeStringUtilities::Wcsicmp(((HFCPtr<HFCURLFile>&)pi_rpURL)->GetExtension().c_str(), L"ig4") == 0)
        Result = true;

    return Result;
    }

//-----------------------------------------------------------------------------
// public
// Create or get the singleton capabilities of IG4 file.
//-----------------------------------------------------------------------------

const HFCPtr<HRFRasterFileCapabilities>& HRFIG4Creator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFIG4Capabilities();

    return m_pCapabilities;
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFIG4File::HRFIG4File(const HFCPtr<HFCURL>& pi_rURL,
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
        throw HFCFileException(HFC_FILE_READ_ONLY_EXCEPTION, pi_rURL->GetURL());
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
HRFIG4File::HRFIG4File(const HFCPtr<HFCURL>& pi_rURL,
                       HFCAccessMode         pi_AccessMode,
                       uint64_t             pi_Offset,
                       bool                 pi_DontOpenFile)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    if (GetAccessMode().m_HasCreateAccess || GetAccessMode().m_HasWriteAccess)
        {
        //this is a read-only format
        throw HFCFileException(HFC_FILE_READ_ONLY_EXCEPTION, pi_rURL->GetURL());
        }

    // Initialise internal members...
    m_IsOpen    = false;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFIG4File::~HRFIG4File()
    {
    }

//-----------------------------------------------------------------------------
// Public
// CreateResolutionEditor
// File manipulation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFIG4File::CreateResolutionEditor(uint32_t       pi_Page,
                                                        unsigned short pi_Resolution,
                                                        HFCAccessMode  pi_AccessMode)
    {
    // verify that the page number is 0, because we have one image per file
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    return new HRFIG4StripEditor(this, pi_Page, pi_Resolution, pi_AccessMode);
    }

//-----------------------------------------------------------------------------
// Public
// Save
// Saves the file
//-----------------------------------------------------------------------------
void HRFIG4File::Save()
    {
    //Nothing do to here
    }

//-----------------------------------------------------------------------------
// Public
// AddPage
// File manipulation
//-----------------------------------------------------------------------------
bool HRFIG4File::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    // Add the page descriptor to the list
    return HRFRasterFile::AddPage(pi_pPage);
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Returnt the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFIG4File::GetCapabilities () const
    {
    return (HRFIG4Creator::GetInstance()->GetCapabilities());
    }

//-----------------------------------------------------------------------------
// Protected section
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Protected
// This method open the file.
//-----------------------------------------------------------------------------
bool HRFIG4File::Open()
    {
    HPRECONDITION(!m_IsOpen);
    HPRECONDITION(!m_pIG4File);

    // Be sure the IG4 raster file is NOT already open.
    m_pIG4File = HFCBinStream::Instanciate(GetURL(), m_Offset, GetAccessMode(), 0, true);

    m_IsOpen = true;

    // This creates the sister file for file sharing control if necessary.
    SharingControlCreate();


    // read the image size
    unsigned short aBuffer[2];
    m_pIG4File->Read(aBuffer, 2 * sizeof(unsigned short));

    // The file was write in little endian, we must swap bytes for big endian
    // this method come from HTIFFUtils
    SwabArrayOfShort(aBuffer, 2);

    m_Height = aBuffer[0];
    m_Width  = aBuffer[1];

    m_pIG4File->SeekToEnd();
    m_pIG4File->Seek(-HEADER_SIZE_AT_FILE_END_IN_BYTES);

    m_pIG4File->Read(&m_HeaderAtEOF, sizeof(HeaderAtEndOfFile));

    m_pIG4File->SeekToPos(m_HeaderAtEOF.m_ImageStripInfoOffset);

    m_pIG4File->Read(&m_StripInfo.m_NbStrips, sizeof(m_StripInfo.m_NbStrips));
    m_pIG4File->Seek(1);
    m_pIG4File->Read(&m_StripInfo.m_StripHeight, sizeof(m_StripInfo.m_StripHeight));

    m_StripInfo.m_StripHeight -= 1;

    //Skip unknown data
    m_pIG4File->Seek(4);

    m_StripInfo.m_pStripOffsets = new uint32_t[m_StripInfo.m_NbStrips];

    for (Byte StripInd = 0; StripInd < m_StripInfo.m_NbStrips; StripInd++)
        {
        m_pIG4File->Read(&(m_StripInfo.m_pStripOffsets[StripInd]),
                         sizeof(uint32_t));
        }

    // Set a pixelType...
    m_pPixelType = new HRPPixelTypeV1Gray1();

    static bool Test = true;

    //84003001.IG4 : Does strip have 512 lines of 511 lines? HMRCCITT works with both value
    //but CCITTFAX4 HASSERT with 512
    if (Test)
        {
        m_pCodec = new HCDCodecHMRCCITT(m_Width, m_Height);
        HFCPtr<HCDCodecHMRCCITT> pCodec = (HFCPtr<HCDCodecHMRCCITT>&)m_pCodec;
        pCodec->SetBitRevTable(false);
        pCodec->SetCCITT3(false);
        pCodec->SetPhotometric(ImagePP::CCITT_PHOTOMETRIC_MINISBLACK);
        pCodec->SetGroup3Options(CCITT_GROUP3OPT_2DENCODING | CCITT_GROUP3OPT_FILLBITS);
        pCodec->SetOptions (CCITT_FAX3_NOEOL | CCITT_FAX3_BYTEALIGN);
        }
    else
        {
        m_pCodec = new HCDCodecCCITTFax4(m_Width, m_StripInfo.m_StripHeight);
        m_pCodec->SetPhotometric(ImagePP::CCITT_PHOTOMETRIC_MINISBLACK);
        }


    HFCPtr<HCDCodecCCITT> pCCITTCodec ((HFCPtr<HCDCodecCCITT>&)m_pCodec);

    // Set padding line pixel
    if ((m_Width % 8) != 0)
        pCCITTCodec->SetLinePaddingBits(8-(m_Width % 8));

    pCCITTCodec->SetBitRevTable(true);

    m_pCodec->SetSubset(m_Width, m_StripInfo.m_StripHeight);

    return m_IsOpen;
    }


//-----------------------------------------------------------------------------
// Public
// GetStripInfo
//-----------------------------------------------------------------------------
void HRFIG4File::GetStripInfo(uint32_t pi_PosY,
                              uint32_t& po_rOffset,
                              uint32_t& po_rSize)
    {
    HPRECONDITION(pi_PosY < m_Height);

    uint32_t StripInd = pi_PosY / m_StripInfo.m_StripHeight;

    po_rOffset = m_StripInfo.m_pStripOffsets[StripInd];

    if (StripInd < (uint32_t)(m_StripInfo.m_NbStrips - 1))
        {
        po_rSize = m_StripInfo.m_pStripOffsets[StripInd + 1] - po_rOffset;
        }
    else
        {
        //The last strip's data seem to end when the image information start
        po_rSize = m_HeaderAtEOF.m_ImageInfoOffset - po_rOffset;
        }
    }

//-----------------------------------------------------------------------------
// Public
// GetStripHeight
//-----------------------------------------------------------------------------
uint32_t HRFIG4File::GetStripHeight()
    {
    return m_StripInfo.m_StripHeight;
    }

//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors
//-----------------------------------------------------------------------------
void HRFIG4File::CreateDescriptors ()
    {
    // Obtain Resolution Information
    // resolution dimension
    HFCPtr<HRFResolutionDescriptor> pResolution =  new HRFResolutionDescriptor(
        GetAccessMode(),                                // AccessMode,
        GetCapabilities(),                              // Capabilities,
        1.0,                                            // XResolutionRatio,
        1.0,                                            // YResolutionRatio,
        new HRPPixelTypeV1Gray1(),                      // PixelType,
        new HCDCodecIdentity(),                         // CodecsList
        HRFBlockAccess::RANDOM,                         // RStorageAccess,
        HRFBlockAccess::RANDOM,                         // WStorageAccess,
        HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL,  // ScanLineOrientation,
        HRFInterleaveType::PIXEL,                       // InterleaveType
        false,                                          // IsInterlace
        m_Width,                                        // Width,
        m_Height,                                       // Height,
        m_Width,                                        // BlockWidth,
        m_StripInfo.m_StripHeight,                      // BlockHeight,
        0,                                              // BlocksDataFlag
        HRFBlockType::STRIP);                            // BlockType

    // BACKGROUND Tag
    HPMAttributeSet TagList;
    HFCPtr<HPMGenericAttribute> pTag;

    HFCPtr<HRFPageDescriptor> pPage;
    pPage = new HRFPageDescriptor (GetAccessMode(),
                                   GetCapabilities(),           // Capabilities
                                   pResolution,                 // ResolutionDescriptor
                                   0,                           // RepresentativePalette
                                   0,                           // Histogram
                                   0,                           // Thumbnail
                                   0,                           // ClipShape
                                   0,                           // TransfoModel
                                   0,                           // Filters
                                   &TagList);                   // Defined Tag

    m_ListOfPageDescriptor.push_back(pPage);
    }
//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
// File information
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFIG4File::GetWorldIdentificator () const
    {
    return HGF2DWorld_UNKNOWNWORLD;
    }

//-----------------------------------------------------------------------------
// Public
// GetIG4FilePtr
// File information
//-----------------------------------------------------------------------------
const HFCBinStream* HRFIG4File::GetIG4FilePtr() const
    {
    return m_pIG4File;
    }


//-----------------------------------------------------------------------------
// Public
// GetIntergraphFilePtr
// File information
//-----------------------------------------------------------------------------
const HFCPtr<HCDCodecCCITT>& HRFIG4File::GetIG4CodecPtr() const
    {
    return m_pCodec;
    }
