//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFCalsFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFCalsFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRFCalsFile.h>
#include <Imagepp/all/h/HRFCalsLineEditor.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>
#include <Imagepp/all/h/HRFSLOModelComposer.h>
#include <Imagepp/all/h/HRFUtility.h>

#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCBinStream.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HFCAccessMode.h>

#include <Imagepp/all/h/HRPPixelType.h>
#include <Imagepp/all/h/HRPPixelTypeV1Gray1.h>

#include <Imagepp/all/h/HCDCodecHMRCCITT.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HCDCodecCCITTFax4.h>


//-----------------------------------------------------------------------------
// HRFCalsBlockCapabilities
//-----------------------------------------------------------------------------
class HRFCalsBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFCalsBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Block Capability
        Add(new HRFLineCapability(HFC_READ_WRITE_CREATE,        // AccessMode
                                  LONG_MAX,                     // MaxWidth
                                  HRFBlockAccess::SEQUENTIAL)); // BlockAccess
        }
    };

//-----------------------------------------------------------------------------
// HRFCalsCodecCapabilities
//-----------------------------------------------------------------------------
class HRFCalsCodecCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFCalsCodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec HMRCCITT
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecHMRCCITT::CLASS_ID,
                                   new HRFCalsBlockCapabilities()));
        }
    };


//-----------------------------------------------------------------------------
// HRFCalsCapabilities
//-----------------------------------------------------------------------------
HRFCalsCapabilities::HRFCalsCapabilities()
    : HRFRasterFileCapabilities()
    {
    // PixelTypeV1Gray1
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV1Gray1::CLASS_ID,
                                   new HRFCalsCodecCapabilities()));

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::UPPER_LEFT_VERTICAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::UPPER_RIGHT_VERTICAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::LOWER_LEFT_VERTICAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL));
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::LOWER_RIGHT_VERTICAL));

    // Transformation Model Capability
    // The file format doesn't really support transformation model, but for compatibility with Intergraph format that
    // concatenate the slo with the Transformation matrix, we simulate that the format support transformation model in read-only.
    // The transformation model returned will only contain rotations and flips according to the scan line orientation of the file.
    // We don't support transformation model save, so transformation model is not saved to file
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE, HGF2DAffine::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE, HGF2DSimilitude::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE, HGF2DStretch::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE, HGF2DTranslation::CLASS_ID));
    Add(new HRFTransfoModelCapability(HFC_READ_WRITE, HGF2DIdentity::CLASS_ID));

    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_WRITE_CREATE));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_WRITE_CREATE, HRFInterleaveType::PIXEL));

    // Single resolution capability
    Add(new HRFSingleResolutionCapability(HFC_READ_WRITE_CREATE));

    // Tag capability
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeBackground(0)));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeDontSupportPersistentColor(true)));
    }

HFC_IMPLEMENT_SINGLETON(HRFCalsCreator)

//-----------------------------------------------------------------------------
// Creator
// This is the creator to instantiate Cals format
//-----------------------------------------------------------------------------

HRFCalsCreator::HRFCalsCreator()
    : HRFRasterFileCreator(HRFCalsFile::CLASS_ID)
    {
    // Cals capabilities instance member initialization
    m_pCapabilities = 0;
    }

//-----------------------------------------------------------------------------
// public
// Identification information
//-----------------------------------------------------------------------------

WString HRFCalsCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_CALS()); //Cals File Format
    }

//-----------------------------------------------------------------------------
// public
// Schemes information
//-----------------------------------------------------------------------------

WString HRFCalsCreator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

//-----------------------------------------------------------------------------
// public
// Extensions information
//-----------------------------------------------------------------------------

WString HRFCalsCreator::GetExtensions() const
    {
    return WString(L"*.cal;*.cals;*.ct1");
    }

//-----------------------------------------------------------------------------
// public
// allow to Open an image file
//-----------------------------------------------------------------------------

HFCPtr<HRFRasterFile> HRFCalsCreator::Create(
    const HFCPtr<HFCURL>& pi_rpURL,
    HFCAccessMode         pi_AccessMode,
    uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFCalsFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }

//-----------------------------------------------------------------------------
// public
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------

bool HRFCalsCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                   uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // This method read and verrify the presence of some ASCII character.
    // As for any valid CALS compliant raster file, we doesn't support unicode
    // or multi-byte char.  We MUST use here some function NOT unicode compliant.

    bool                   bResult = false;
    HAutoPtr<HFCBinStream>  pFile;
    char                   Marker[20];

    (const_cast<HRFCalsCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock(GetLockManager());

    // Open the Cals File & place file pointer at the start of the file
    pFile = HFCBinStream::Instanciate(pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);
    if (pFile == 0 || pFile->GetLastException() != 0)
        goto WRAPUP;

    if (pFile->Read(Marker, 8) != 8)
        goto WRAPUP;

    // Verify if we have a basic Type 1
    if (BeStringUtilities::Strnicmp(Marker, "srcdocid", 8) == 0)
        {
        pFile->SeekToPos(6 * HRF_CALS_TYPE1_RECORD_SIZE);

        // Read 6th record (of 128 byte each...)
        if (pFile->Read(Marker, 13) != 13)
            goto WRAPUP;

        // On read success...
        if (strncmp(Marker, "rtype:", 6) != 0)
            goto WRAPUP;

        char StringBuffer[5];

        strncpy(StringBuffer, (char*)&Marker[6], 4);
        StringBuffer[4] = 0;
        uint32_t Version = atoi(StringBuffer);

        // We only support some Cals version 1 for now.
        if (Version == 1)
            bResult = true;
        }
    // Verify if we have a MIL-R-28002A or a MIL-PRF-28002B
    else if (BeStringUtilities::Strnicmp(Marker, "specvers", 8) == 0)
        {
        pFile->SeekToPos(4 * HRF_CALS_TYPE1_RECORD_SIZE);

        // Read 4th record (of 128 byte each...)
        if (pFile->Read(Marker, 18) != 18)
            goto WRAPUP;

        // On read success...
        if (strncmp(Marker, "dtype:", 6) == 0)
            {
            char StringBuffer[7];

            strncpy(StringBuffer, &Marker[6], 6);
            StringBuffer[6] = 0;
            uint32_t Version = atoi(StringBuffer);

            // We only support some Cals version 1 for now.
            if (Version != 1)
                goto WRAPUP;

            // Be sure to exclude anything like : "1, B" where there is more than a
            // digit to describe the document type.
            for (int CarIndex = 0; (CarIndex < 6) && bResult; CarIndex++)
                {
                if (StringBuffer[CarIndex] && (!isdigit(StringBuffer[CarIndex]) && !isspace(StringBuffer[CarIndex])))
                    goto WRAPUP;
                }
            bResult = true;
            }
        else
            {
            pFile->SeekToPos(3 * HRF_CALS_TYPE1_RECORD_SIZE);

            // Read 3th record (of 128 byte each...)
            if (pFile->Read(Marker, 18) != 18)
                goto WRAPUP;

            // On read success...
            if (strncmp(Marker, "dtype:", 6) != 0)
                goto WRAPUP;

            char StringBuffer[7];

            strncpy(StringBuffer, (char*)&Marker[6], 6);
            StringBuffer[6] = 0;
            uint32_t Version = atoi(StringBuffer);

            // We only support some Cals version 1 for now.
            if (Version != 1)
                goto WRAPUP;

            // Be sure to exclude anything like : "1, B" where there is more than a
            // digit to describe the document type.
            for (int CarIndex = 0; (CarIndex < 6) && bResult; CarIndex++)
                {
                if (StringBuffer[CarIndex] && (!isdigit(StringBuffer[CarIndex]) && !isspace(StringBuffer[CarIndex])))
                    goto WRAPUP;
                }
            bResult = true;
            }
        }


WRAPUP:
    SisterFileLock.ReleaseKey();

    HASSERT(!(const_cast<HRFCalsCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFCalsCreator*>(this))->m_pSharingControl = 0;

    return bResult;
    }

//-----------------------------------------------------------------------------
// public
// Create or get the singleton capabilities of Cals file.
//-----------------------------------------------------------------------------

const HFCPtr<HRFRasterFileCapabilities>& HRFCalsCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFCalsCapabilities();

    return m_pCapabilities;
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFCalsFile::HRFCalsFile(const HFCPtr<HFCURL>& pi_rURL,
                         HFCAccessMode         pi_AccessMode,
                         uint64_t             pi_Offset)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor store the access mode

    // Initialise internal members...
    m_IsOpen              = false;
    m_HasHeaderFilled     = false;
    m_BitPerPixel         = 0;
    m_pCalsFile           = 0;
    m_pCalsHeader         = 0;
    m_CalsType            = UNKNOWN;
    m_ScanlineOrientation = HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL;

    if (GetAccessMode().m_HasCreateAccess)
        Create();
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
HRFCalsFile::HRFCalsFile(const HFCPtr<HFCURL>& pi_rURL,
                         HFCAccessMode         pi_AccessMode,
                         uint64_t             pi_Offset,
                         bool                 pi_DontOpenFile)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    // Initialise internal members...
    m_IsOpen              = false;
    m_HasHeaderFilled     = false;
    m_BitPerPixel         = 0;
    m_pCalsFile           = 0;
    m_pCalsHeader         = 0;
    m_CalsType            = UNKNOWN;
    m_ScanlineOrientation = HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL;
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------

HRFCalsFile::~HRFCalsFile()
    {
    // Close the CALS file and free the header structure

    SaveCalFile(true);
    }

//-----------------------------------------------------------------------------
// Public
// CreateResolutionEditor
// File manipulation
//-----------------------------------------------------------------------------

HRFResolutionEditor* HRFCalsFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                         unsigned short pi_Resolution,
                                                         HFCAccessMode  pi_AccessMode)
    {
    // verify that the page number is 0, because we have one image per file
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    return new HRFCalsLineEditor(this, pi_Page, pi_Resolution, pi_AccessMode);
    }

//-----------------------------------------------------------------------------
// Public
// Save
// Saves the file
//-----------------------------------------------------------------------------

void HRFCalsFile::Save()
    {
    try
        {
        //Keep last file position
        uint64_t CurrentPos = m_pCalsFile->GetCurrentPos();

        SaveCalFile(false);

        //Set back position
        m_pCalsFile->SeekToPos(CurrentPos);

        }
    catch(...)
        {
        // Simply stop exceptions in the destructor
        // We want to known if a exception is throw.
        HASSERT(0);
        }
    }

//-----------------------------------------------------------------------------
// Public
// Get the file's current size.
//-----------------------------------------------------------------------------
uint64_t HRFCalsFile::GetFileCurrentSize() const
    {
    return HRFRasterFile::GetFileCurrentSize(m_pCalsFile);
    }

//-----------------------------------------------------------------------------
// Public
// AddPage
// File manipulation
//-----------------------------------------------------------------------------

bool HRFCalsFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    // Validation if it's possible to add a page
    // HPRECONDITION(CountPages() == 0);

    // Add the page descriptor to the list
    HRFRasterFile::AddPage(pi_pPage);
    m_HasHeaderFilled = CreateFileHeader(pi_pPage);

    return m_HasHeaderFilled;
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Returnt the capabilities of the file.
//-----------------------------------------------------------------------------

const HFCPtr<HRFRasterFileCapabilities>& HRFCalsFile::GetCapabilities () const
    {
    return (HRFCalsCreator::GetInstance()->GetCapabilities());
    }

//-----------------------------------------------------------------------------
// Protected
// This method open the file.
//-----------------------------------------------------------------------------

bool HRFCalsFile::Open()
    {
    HPRECONDITION(!m_IsOpen);
    HPRECONDITION(!m_pCalsFile);

    // Be sure the Intergraph raster file is NOT already open.
    m_pCalsFile = HFCBinStream::Instanciate(GetURL(), m_Offset, GetAccessMode(), 0, true);

    m_IsOpen = true;

    // This creates the sister file for file sharing control if necessary.
    SharingControlCreate();

    return m_IsOpen;
    }


//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors
//-----------------------------------------------------------------------------
void HRFCalsFile::CreateDescriptors ()
    {
    HPRECONDITION (m_IsOpen);

    // Initialise some members and read file header Read
    InitOpenedFile();

    // Obtain Resolution Information
    // resolution dimension
    HFCPtr<HRFResolutionDescriptor> pResolution =  new HRFResolutionDescriptor(
        GetAccessMode(),                        // AccessMode,
        GetCapabilities(),                      // Capabilities,
        1.0,                                    // XResolutionRatio,
        1.0,                                    // YResolutionRatio,
        GetPixelType(),                         // PixelType,
        new HCDCodecHMRCCITT(),                 // Codecs
        HRFBlockAccess::SEQUENTIAL,             // RStorageAccess,
        HRFBlockAccess::SEQUENTIAL,             // WStorageAccess,
        GetScanlineOrientation(),               // ScanLineOrientation,
        HRFInterleaveType::PIXEL,               // InterleaveType
        false,                                  // IsInterlace
        m_Width,                                // Width,
        m_Height,                               // Height,
        m_Width,                                // BlockWidth,
        1,                                      // BlockHeight,
        0,                                      // BlocksDataFlag
        HRFBlockType::LINE);                    // BlockType

    // BACKGROUND Tag
    HPMAttributeSet TagList;
    HFCPtr<HPMGenericAttribute> pTag;
    pTag = new HRFAttributeBackground(0);
    TagList.Set(pTag);

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
                                   GetTransfoModel(),           // TransfoModel
                                   0,                           // Filters
                                   &TagList);                   // Defined Tag

    m_ListOfPageDescriptor.push_back(pPage);
    }


//-----------------------------------------------------------------------------
// Private
// This method saves the file and close it if needed
//-----------------------------------------------------------------------------

void HRFCalsFile::SaveCalFile(bool pi_CloseFile)
    {
    // Be sure that the file is already open and that at least one page
    // has been add. Because if the destroyer is call afer a exception
    // was thrown, we want to be sure that the object is valid before we
    // execute the destroyer.
    if (m_IsOpen && m_ListOfPageDescriptor.size() > 0)
        {

        // Should do something here ???
        // At least re-write the tag !!!

        if(pi_CloseFile)
            {
            delete m_pCalsHeader;
            m_pCalsHeader = 0;

            // Set the open flag to false
            m_IsOpen = false;
            }
        else
            {
            m_pCalsFile->Flush();
            }
        }
    }

//-----------------------------------------------------------------------------
// Private
// This method create the file.
//-----------------------------------------------------------------------------

bool HRFCalsFile::Create()
    {
    // Be sure the Intergraph raster file is NOT already open.
    HPRECONDITION(!m_IsOpen);
    HPRECONDITION(!m_pCalsFile);

    m_pCalsFile = HFCBinStream::Instanciate(GetURL(), m_Offset, GetAccessMode(), 0, true);

    m_IsOpen = true;

    // This creates the sister file for file sharing control if necessary.
    SharingControlCreate();

    return m_IsOpen;
    }

//-----------------------------------------------------------------------------
// public
// GetPixelType
//-----------------------------------------------------------------------------

bool HRFCalsFile::ConstructSlo()
    {
    HPRECONDITION(m_HasHeaderFilled);
    HPRECONDITION(m_pCalsHeader != 0);
    HPRECONDITION(m_CalsType != UNKNOWN);

    // This method read and verrify the presence of some ASCII character.
    // As for any valid CALS compliant raster file, we doesn't support unicode
    // or multi-byte char.  We MUST use here some function NOT unicode compliant.

    bool Status = false;
    char StringBuffer[4];

    // Initialise to an SLO HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL.
    unsigned short PelPath  = 0;
    unsigned short LineProg = 270;

    char* Orientation = 0;

    if (MIL_28002C == m_CalsType)
        Orientation = ((CalsHeaderBlock_MIL_28002C*)m_pCalsHeader)->Orientation;
    else if (MIL_R_28002A == m_CalsType)
        Orientation = ((CalsHeaderBlock_MIL_R_28002A*)m_pCalsHeader)->Orientation;
    else if (MIL_PRF_28002B == m_CalsType)
        Orientation = ((CalsHeaderBlock_MIL_PRF_28002B*)m_pCalsHeader)->Orientation;
    HDEBUGCODE(else HASSERT(false););

    // Be sure to have the right record entry
    if (strncmp(Orientation, "rorient", 7) == 0)
        {
        // Get the raster Width from the header record entry
        strncpy(StringBuffer, &Orientation[9], 3);
        StringBuffer[3] = 0;
        PelPath = (unsigned short)atoi(StringBuffer);

        // Get the raster Width from the header record entry
        strncpy(StringBuffer, &Orientation[13], 3);
        StringBuffer[3] = 0;
        LineProg = (unsigned short)atoi(StringBuffer);

        Status = true;
        }

    switch(PelPath)
        {
        case 0  :
            if (LineProg == 90)
                {
                m_ScanlineOrientation = HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL;
                }
            else
                m_ScanlineOrientation = HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL;
            break;
        case 90 :
            if (LineProg == 90)
                {
                m_ScanlineOrientation = HRFScanlineOrientation::LOWER_RIGHT_VERTICAL;
                }
            else
                {
                m_ScanlineOrientation = HRFScanlineOrientation::LOWER_LEFT_VERTICAL;
                }
            break;
        case 180:
            if (LineProg == 90)
                {
                m_ScanlineOrientation = HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL;
                }
            else
                {
                m_ScanlineOrientation = HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL;
                }
            break;
        case 270:
            if (LineProg == 90)
                {
                m_ScanlineOrientation = HRFScanlineOrientation::UPPER_LEFT_VERTICAL;
                }
            else
                {
                m_ScanlineOrientation = HRFScanlineOrientation::UPPER_RIGHT_VERTICAL;
                }
            break;
        default :
            m_ScanlineOrientation = HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL;
            break;
        }

    return Status;
    }

//-----------------------------------------------------------------------------
// public
// GetPixelType
//-----------------------------------------------------------------------------

bool HRFCalsFile::ComputeRasterDimension()
    {
    // This method read and verrify the presence of some ASCII character.
    // As for any valid CALS compliant raster file, we doesn't support unicode
    // or multi-byte char.  We MUST use here some function NOT unicode compliant.

    HPRECONDITION(m_HasHeaderFilled);
    HPRECONDITION(m_CalsType != UNKNOWN);

    bool Status = false;
    char* PelCount = 0;

    if (MIL_28002C == m_CalsType)
        PelCount = ((CalsHeaderBlock_MIL_28002C*)m_pCalsHeader)->PelCount;
    else if (MIL_R_28002A == m_CalsType)
        PelCount = ((CalsHeaderBlock_MIL_R_28002A*)m_pCalsHeader)->PelCount;
    else if (MIL_PRF_28002B == m_CalsType)
        PelCount = ((CalsHeaderBlock_MIL_PRF_28002B*)m_pCalsHeader)->PelCount;
    HDEBUGCODE(else HASSERT(false););

    // a valid value is rpelcnt: 999999, 999999
    if (strncmp(PelCount, "rpelcnt: ", 9) == 0)
        {
        char Dummy = PelCount[23];// take a copy of the char
        PelCount[23] = 0;          // put a end of string

        int32_t Width;
        int32_t Height;

        if (sscanf(&PelCount[9], "%ld,%ld", &Width, &Height) == 2)
            {
            if (Width > 0 && Height > 0)
                {
                m_Width  = (uint32_t)Width;
                m_Height = (uint32_t)Height;
                Status = true;
                }
            }
        PelCount[23] = Dummy;
        }
    return Status;
    }

//-----------------------------------------------------------------------------
// public
// GetPixelType
//-----------------------------------------------------------------------------

HFCPtr<HRPPixelType> HRFCalsFile::GetPixelType() const
    {
    HPRECONDITION(m_HasHeaderFilled);
    return m_pPixelType;
    }

//-----------------------------------------------------------------------------
// Publics
// Create a new header and write it to a file stream.
// On error return false.
//-----------------------------------------------------------------------------

bool HRFCalsFile::CreateFileHeader(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    HPRECONDITION(m_IsOpen);
    HPRECONDITION(m_pCalsFile != 0);

    HFCPtr<HRFResolutionDescriptor> pResolutionDescriptor = pi_pPage->GetResolutionDescriptor(0);

    HFCPtr<HRPPixelType> pPixelType = pResolutionDescriptor->GetPixelType();

    // Create an header block
    if (CreateHeaderBlock(pResolutionDescriptor))
        {
        // Lock the sister file
        HFCLockMonitor SisterFileLock(GetLockManager());

        // Write freshly created header physically into the file...
        m_pCalsFile->Write(m_pCalsHeader, HRF_CALS_TYPE1_BLOCK_SIZE);

        // Unlock the sister file
        SisterFileLock.ReleaseKey();

        // Initialise some members and read file header Read
        m_HasHeaderFilled = true;
        InitOpenedFile();
        }
    return m_HasHeaderFilled;
    }

//-----------------------------------------------------------------------------
// Publics
// Create a new header and write it to a file stream.
// On error return false.
//-----------------------------------------------------------------------------

bool HRFCalsFile::CreateHeaderBlock(HRFResolutionDescriptor* pi_pResolutionDescriptor)
    {
    // This method write and modify some ASCII character.
    // As for any valid CALS raster file we doesn't support unicode
    // or multi-byte char.  We MUST use here some function NOT unicode compliant.

    HPRECONDITION(m_IsOpen);
    HPRECONDITION(m_pCalsFile != 0);
    HPRECONDITION(m_pCalsHeader == 0);

    bool Result = true;
    uint32_t BufferSize;

    char WidthString [7];
    char HeightString[7];
    char StringBuffer[20];

    // Use the most well recognized format.
    m_CalsType    = MIL_28002C;
    m_pCalsHeader = new CalsHeaderBlock_MIL_28002C;

    memset(StringBuffer, 0, 19);

    // Construct a standard header block information...
    memset(m_pCalsHeader, 0, HRF_CALS_TYPE1_BLOCK_SIZE);

    strncpy(((CalsHeaderBlock_MIL_28002C*)m_pCalsHeader)->SourceDocID, "srcdocid: NONE",  14);
    strncpy(((CalsHeaderBlock_MIL_28002C*)m_pCalsHeader)->DestDocID,   "dstdocid: NONE",  14);
    strncpy(((CalsHeaderBlock_MIL_28002C*)m_pCalsHeader)->TextFileID,  "txtfilid: NONE",  14);
    strncpy(((CalsHeaderBlock_MIL_28002C*)m_pCalsHeader)->FigureID,    "figid: NONE",     11);
    strncpy(((CalsHeaderBlock_MIL_28002C*)m_pCalsHeader)->SourceGraph, "srcgph: NONE",    12);
    strncpy(((CalsHeaderBlock_MIL_28002C*)m_pCalsHeader)->DocClass,    "doccls: NONE",    12);
    strncpy(((CalsHeaderBlock_MIL_28002C*)m_pCalsHeader)->RasterType,  "rtype: 1",         8);
    strncpy(((CalsHeaderBlock_MIL_28002C*)m_pCalsHeader)->Orientation, "rorient: 000,270",16);
    strncpy(((CalsHeaderBlock_MIL_28002C*)m_pCalsHeader)->PelCount,    "rpelcnt: 000,000",16);
    strncpy(((CalsHeaderBlock_MIL_28002C*)m_pCalsHeader)->Density,     "rdensty: 0300",   13);
    strncpy(((CalsHeaderBlock_MIL_28002C*)m_pCalsHeader)->Notes,       "notes: NONE",     11);

    // rpelcnt: Image size in pixel.  Width, Height
    BeStringUtilities::Snprintf (WidthString, 7, "%I64u", pi_pResolutionDescriptor->GetWidth());
    BeStringUtilities::Snprintf (HeightString, 7, "%I64u", pi_pResolutionDescriptor->GetHeight());

    BufferSize = (uint32_t)strlen(WidthString);
    if (BufferSize < 6)
        strncpy(StringBuffer, "      ", 6 - BufferSize);
    else
        BufferSize = 6;
    strncpy(StringBuffer + (6 - BufferSize), WidthString, BufferSize);
    strncpy(StringBuffer + 6 , ", ", 2);

    BufferSize = (uint32_t)strlen(HeightString);
    if (BufferSize < 6)
        strncpy(StringBuffer + 8, "      ", 6 - BufferSize);
    else
        BufferSize = 6;
    strncpy(StringBuffer + 8 + (6 - BufferSize), HeightString, BufferSize);
    strncpy(((CalsHeaderBlock_MIL_28002C*)m_pCalsHeader)->PelCount + 9, StringBuffer, strlen(StringBuffer));

    return Result;
    }


//-----------------------------------------------------------------------------
// Private
// CreatePixelTypeFromFile
//-----------------------------------------------------------------------------

void HRFCalsFile::InitOpenedFile()
    {
    HPRECONDITION(m_IsOpen);
    HPRECONDITION(m_pCalsFile != 0);

    if (!GetAccessMode().m_HasCreateAccess)
        {
        // Lock the sister file
        HFCLockMonitor SisterFileLock(GetLockManager());

        m_CalsType = GetCalsType();
        HASSERT(m_CalsType != UNKNOWN);

        if (MIL_28002C == m_CalsType)
            m_pCalsHeader = new CalsHeaderBlock_MIL_28002C;
        else if (MIL_R_28002A == m_CalsType)
            m_pCalsHeader = new CalsHeaderBlock_MIL_R_28002A;
        else if (MIL_PRF_28002B == m_CalsType)
            m_pCalsHeader = new CalsHeaderBlock_MIL_PRF_28002B;
        HDEBUGCODE(else HASSERT(false););

        // Read and fill the Type1BlockHeader
        m_pCalsFile->Read(m_pCalsHeader, HRF_CALS_TYPICAL_READ_BUFFER_SIZE);
        m_HasHeaderFilled = true;

        // Unlock the sister file.
        SisterFileLock.ReleaseKey();
        }

    if (m_HasHeaderFilled)
        {
        // Get some basic information contained in the header
        ComputeRasterDimension();
        ConstructSlo();

        // Set a pixelType...
        m_pPixelType = new HRPPixelTypeV1Gray1();

        // Create and initialise a new codec
        m_pCodec = new HCDCodecCCITTFax4(m_Width, m_Height);
        m_pCodec->SetPhotometric((unsigned short)ImageppLib::GetHost().GetImageppLibAdmin()._GetDefaultPhotometricInterpretation());
        m_pCodec->SetSubset(m_Width,1);

        m_BitPerPixel     = 1;
        m_HasHeaderFilled = true;
        }
    }

//-----------------------------------------------------------------------------
// Private
// GetCalsType
//-----------------------------------------------------------------------------

HRFCalsFile::CALS_TYPE HRFCalsFile::GetCalsType()
    {
    HPRECONDITION(m_pCalsFile != 0);

    // This method read and verrify the presence of some ASCII character.
    // As for any valid CALS compliant raster file, we doesn't support unicode
    // or multi-byte char.  We MUST use here some function NOT unicode compliant.

    char Marker[20];
    CALS_TYPE CurrentFileType = UNKNOWN;

    HFCLockMonitor SisterFileLock(GetLockManager());

    // Open the Cals File & place file pointer at the start of the file
    if (m_pCalsFile != 0)
        {
        // Backup our file cursor position.
        uint64_t CurrentCursorPos = m_pCalsFile->GetCurrentPos();

        if (m_pCalsFile->Read(Marker, 8) == 8)
            {
            if (BeStringUtilities::Strnicmp(Marker, "srcdocid", 8) == 0)
                {
                // Read 7th record (of 128 byte each...)
                m_pCalsFile->SeekToPos(6 * HRF_CALS_TYPE1_RECORD_SIZE);
                if (m_pCalsFile->Read(Marker, 13) == 13)
                    {
                    // On read success...
                    if (strncmp(Marker, "rtype:", 6) == 0)
                        {
                        char StringBuffer[5];

                        strncpy(StringBuffer, (char*)&Marker[6], 4);
                        StringBuffer[4] = 0;
                        uint32_t Version = atoi(StringBuffer);

                        // Assure to have a Cals version 1 other not support at this time.
                        if (Version == 1)
                            CurrentFileType = MIL_28002C;
                        }
                    }
                }
            // Verrify if we have a MIL-R-28002A or a MIL-PRF-28002B
            else if (BeStringUtilities::Strnicmp(Marker, "specvers", 8) == 0)
                {
                // Read 4th record (of 128 byte each...)
                m_pCalsFile->SeekToPos(4 * HRF_CALS_TYPE1_RECORD_SIZE);
                if (m_pCalsFile->Read(Marker, 13) == 13)
                    {
                    // On read success...
                    if (strncmp(Marker, "dtype:", 6) == 0)
                        {
                        char StringBuffer[5];

                        strncpy(StringBuffer, (char*)&Marker[6], 4);
                        StringBuffer[4] = 0;
                        uint32_t Version = atoi(StringBuffer);

                        // Assure to have a Cals version 1 other not support at this time.
                        if (Version == 1)
                            CurrentFileType = MIL_R_28002A;
                        }
                    else
                        {
                        // Read 3th record (of 128 byte each...)
                        m_pCalsFile->SeekToPos(3 * HRF_CALS_TYPE1_RECORD_SIZE);
                        if (m_pCalsFile->Read(Marker, 13) == 13)
                            {
                            // On read success...
                            if (strncmp(Marker, "dtype:", 6) == 0)
                                {
                                char StringBuffer[5];

                                strncpy(StringBuffer, (char*)&Marker[6], 4);
                                StringBuffer[4] = 0;
                                uint32_t Version = atoi(StringBuffer);

                                // Assure to have a Cals version 1 other not support at this time.
                                if (Version == 1)
                                    CurrentFileType = MIL_PRF_28002B;
                                }
                            }
                        }
                    }
                }
            }
        // Reset file cursor position.
        m_pCalsFile->SeekToPos(CurrentCursorPos);
        }
    SisterFileLock.ReleaseKey();

    return CurrentFileType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  9/2005
+---------------+---------------+---------------+---------------+---------------+------*/
HFCPtr<HGF2DTransfoModel> HRFCalsFile::GetTransfoModel() const
    {
    HFCPtr<HGF2DTransfoModel> pTransfoModel (CreateScanLineOrientationModel(GetScanlineOrientation(), m_Width, m_Height));

    return pTransfoModel;
    }

/*---------------------------------------------------------------------------------**//**
* Creates a transformation model that convert the given scan line orientation to
* Upper Left Horizontal (SLO 4).
@bsimethod                                                    StephanePoulin  9/2005
+---------------+---------------+---------------+---------------+---------------+------*/
HFCPtr<HGF2DTransfoModel> HRFCalsFile::CreateScanLineOrientationModel(HRFScanlineOrientation  scanlineOrientation,
                                                                      uint32_t                physicalWidth,
                                                                      uint32_t                physicalHeight)const
    {
    HFCPtr<HGF2DAffine> pSLOAffine = new HGF2DAffine();

    // SLO 0 (ULV)
    if (scanlineOrientation == HRFScanlineOrientation::UPPER_LEFT_VERTICAL)
        {
        pSLOAffine->SetByMatrixParameters(0.0, 0.0, 1.0, 0.0, 1.0, 0.0);
        }
    // SLO 1 (URV)
    else if (scanlineOrientation == HRFScanlineOrientation::UPPER_RIGHT_VERTICAL)
        {
        pSLOAffine->SetByMatrixParameters(physicalHeight, 0.0, -1.0, 0.0, 1.0, 0.0);
        }
    // SLO 2 (LLV)
    else if (scanlineOrientation == HRFScanlineOrientation::LOWER_LEFT_VERTICAL)
        {
        pSLOAffine->SetByMatrixParameters(0.0, 0.0, 1.0, physicalWidth, -1.0, 0.0);
        }
    // SLO 3 (LRV)
    else if (scanlineOrientation == HRFScanlineOrientation::LOWER_RIGHT_VERTICAL)
        {
        pSLOAffine->SetByMatrixParameters(physicalHeight, 0.0, -1.0, physicalWidth, -1.0, 0.0);
        }
    // SLO 4 (ULH)
    else if (scanlineOrientation == HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL)
        {
        pSLOAffine->SetByMatrixParameters(0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
        }
    // SLO 5 (URH)
    else if (scanlineOrientation == HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL)
        {
        pSLOAffine->SetByMatrixParameters(physicalWidth, -1.0, 0.0, 0.0, 0.0, 1.0);
        }
    // SLO 6 (LLH)
    else if (scanlineOrientation == HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL)
        {
        pSLOAffine->SetByMatrixParameters(0.0, 1.0, 0.0, physicalHeight, 0.0, -1.0);
        }
    // SLO 7 (LRH)
    else if (scanlineOrientation == HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL)
        {
        pSLOAffine->SetByMatrixParameters(physicalWidth, -1.0, 0.0, physicalHeight, 0.0, -1.0);
        }

    return((HFCPtr<HGF2DTransfoModel>&)pSLOAffine);
    }
//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
// File information
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFCalsFile::GetWorldIdentificator () const
    {
    return HGF2DWorld_UNKNOWNWORLD;
    }

//-----------------------------------------------------------------------------
// Public
// GetIntergraphFilePtr
// File information
//-----------------------------------------------------------------------------

const HFCBinStream* HRFCalsFile::GetCalsFilePtr() const
    {
    return m_pCalsFile;
    }

//-----------------------------------------------------------------------------
// Public
// GetIntergraphFilePtr
// File information
//-----------------------------------------------------------------------------
const HFCPtr<HCDCodecCCITTFax4>& HRFCalsFile::GetCalsCodecPtr() const
    {
    return m_pCodec;
    }


//-----------------------------------------------------------------------------
// public
// GetPixelType
//-----------------------------------------------------------------------------

const HRFScanlineOrientation HRFCalsFile::GetScanlineOrientation() const
    {
    HPRECONDITION(m_HasHeaderFilled);

    return m_ScanlineOrientation;
    }

//-----------------------------------------------------------------------------
// Public
// SetDefaultRatioToMeter
//-----------------------------------------------------------------------------
void HRFCalsFile::SetDefaultRatioToMeter(double pi_RatioToMeter,
                                                uint32_t pi_Page,
                                                bool   pi_CheckSpecificUnitSpec,
                                                bool   pi_InterpretUnitINTGR)
    {
    //The transformation matrix is used only for SLO compensation,
    //not for geo-localization.
    }
