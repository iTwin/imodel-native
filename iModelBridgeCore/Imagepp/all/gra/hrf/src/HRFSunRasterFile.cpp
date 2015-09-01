//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFSunRasterFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFSunRasterFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HRFException.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HRFSunRasterFile.h>
#include <Imagepp/all/h/HRFSunRasterEditor.h>
#include <Imagepp/all/h/HRPChannelOrgRGB.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV1Gray1.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HTIFFUtils.h>

#include <Imagepp/all/h/HRFRasterFileCapabilities.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>

// Constant initialisation
const uint32_t HRFSunRasterFile::RAS_MAGIC       = 0x59a66a95;
const uint32_t HRFSunRasterFile::RT_OLD          = 0;
const uint32_t HRFSunRasterFile::RT_STANDARD     = 1;
const uint32_t HRFSunRasterFile::RT_RLENCODED    = 2;
const uint32_t HRFSunRasterFile::RT_FOMRAT_RGB   = 3;
const uint32_t HRFSunRasterFile::RT_TIFF         = 4;
const uint32_t HRFSunRasterFile::RT_IFF          = 5;
const uint32_t HRFSunRasterFile::RT_EXPERIMENTAL = 0xffff;
const uint32_t HRFSunRasterFile::RMT_NOMAP       = 0;
const uint32_t HRFSunRasterFile::RMT_RGBMAP      = 1;
const uint32_t HRFSunRasterFile::RMT_RAWMAP      = 2;
const uint32_t HRFSunRasterFile::HEADER_OFFSET    = 0;
const uint32_t HRFSunRasterFile::COLORMAP_OFFSET  = 32;


//-----------------------------------------------------------------------------
// HRFSunRasterBlockCapabilities
//-----------------------------------------------------------------------------
class HRFSunRasterBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFSunRasterBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Block Capability
        Add(new HRFLineCapability(HFC_READ_WRITE_CREATE,
                                  LONG_MAX,
                                  HRFBlockAccess::RANDOM));

        Add(new HRFImageCapability(HFC_READ_WRITE,         // AccessMode
                                   LONG_MAX,               // MaxSizeInBytes
                                   0,                      // MinWidth
                                   LONG_MAX,               // MaxWidth
                                   0,                      // MinHeight
                                   LONG_MAX));             // MaxHeight
        }
    };

//-----------------------------------------------------------------------------
// HRFSunRasterCodecIdentityCapabilities
//-----------------------------------------------------------------------------
class HRFSunRasterCodecIdentityCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFSunRasterCodecIdentityCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFSunRasterBlockCapabilities()));
        }
    };


//-----------------------------------------------------------------------------
// HRFSunRasterCapabilities
//-----------------------------------------------------------------------------
HRFSunRasterCapabilities::HRFSunRasterCapabilities()
    : HRFRasterFileCapabilities()
    {
    // PixelTypeV11Gray1
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV1Gray1::CLASS_ID,
                                   new HRFSunRasterCodecIdentityCapabilities()));
    // PixelTypeI1R8G8B8
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeI1R8G8B8::CLASS_ID,
                                   new HRFSunRasterCodecIdentityCapabilities()));
    // PixelTypeI8R8G8B8
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeI8R8G8B8::CLASS_ID,
                                   new HRFSunRasterCodecIdentityCapabilities()));
    // HRPPixelTypeV8Gray8
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV8Gray8::CLASS_ID,
                                   new HRFSunRasterCodecIdentityCapabilities()));

    // HRPPixelTypeV24R8G8B8
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV24R8G8B8::CLASS_ID,
                                   new HRFSunRasterCodecIdentityCapabilities()));

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_WRITE_CREATE, HRFInterleaveType::PIXEL));

    // Single Resolution Capability
    Add(new HRFSingleResolutionCapability(HFC_READ_WRITE_CREATE));

    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_WRITE_CREATE));
    }

HFC_IMPLEMENT_SINGLETON(HRFSunRasterCreator)

//-----------------------------------------------------------------------------
// Constructor
// Public (HRFSunRasterCreator)
// This is the creator to instantiate SunRaster format
//-----------------------------------------------------------------------------
HRFSunRasterCreator::HRFSunRasterCreator()
    : HRFRasterFileCreator(HRFSunRasterFile::CLASS_ID)
    {
    // SunRaster capabilities instance member initialization
    m_pCapabilities = 0;
    }

//-----------------------------------------------------------------------------
// GetLabel
// Public (HRFSunRasterCreator)
// Identification information
//-----------------------------------------------------------------------------
WString HRFSunRasterCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_SunRaster()); // SunRaster File Format
    }

//-----------------------------------------------------------------------------
// GetSchemes
// Public (HRFSunRasterCreator)
// Identification information
//-----------------------------------------------------------------------------
WString HRFSunRasterCreator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

//-----------------------------------------------------------------------------
// GetExtensions
// Public (HRFSunRasterCreator)
// Identification information
//-----------------------------------------------------------------------------
WString HRFSunRasterCreator::GetExtensions() const
    {
    return WString(L"*.rs;*.ras");
    }

//-----------------------------------------------------------------------------
// Create
// Public (HRFSunRasterCreator)
// allow to Open an image file
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFSunRasterCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                                  HFCAccessMode   pi_AccessMode,
                                                  uint64_t       pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // Open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFSunRasterFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }


//-----------------------------------------------------------------------------
// IsKindOfFile
// Public (HRFSunRasterCreator)
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------
bool HRFSunRasterCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                        uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    bool                   bResult = false;
    HAutoPtr<HFCBinStream>  pFile;

    (const_cast<HRFSunRasterCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock(GetLockManager());

    // Open the SunRaster File & place file pointer at the start of the file
    pFile = HFCBinStream::Instanciate(pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

    if (pFile == 0 || pFile->GetLastException() != 0)
        goto WRAPUP;

    uint32_t MagicNumber;
    uint32_t Depth;
    uint32_t SkipValue;
    uint32_t Type;

    if (pFile->Read(&MagicNumber, sizeof MagicNumber) != sizeof MagicNumber  ||
        pFile->Read(&SkipValue,   sizeof SkipValue)   != sizeof SkipValue    ||
        pFile->Read(&SkipValue,   sizeof SkipValue)   != sizeof SkipValue    ||
        pFile->Read(&Depth,       sizeof Depth)       != sizeof Depth        ||
        pFile->Read(&SkipValue,   sizeof SkipValue)   != sizeof SkipValue    ||
        pFile->Read(&Type,        sizeof Type)        != sizeof Type         ||
        pFile->Read(&SkipValue,   sizeof SkipValue)   != sizeof SkipValue    ||
        pFile->Read(&SkipValue,   sizeof SkipValue)   != sizeof SkipValue)
        goto WRAPUP;

    // Utility from HTIFFUtils
    if (!SystemIsBigEndian())
        {
        SwabArrayOfLong(&MagicNumber, 1);
        SwabArrayOfLong(&Depth, 1);
        SwabArrayOfLong(&Type, 1);
        }

    // Verify that the type is SunRaster image
    if (MagicNumber != HRFSunRasterFile::RAS_MAGIC)
        goto WRAPUP;

    if(!((Type == HRFSunRasterFile::RT_OLD) ||
         (Type == HRFSunRasterFile::RT_STANDARD) ||
         (Type == HRFSunRasterFile::RT_FOMRAT_RGB) ||
         (Type == HRFSunRasterFile::RT_RLENCODED) ||
         (Type == HRFSunRasterFile::RT_TIFF) ||
         (Type == HRFSunRasterFile::RT_IFF)) )
        goto WRAPUP;

    if ((Type == HRFSunRasterFile::RT_RLENCODED) && (Depth != 8))
        goto WRAPUP;

    if (!((Depth == 1) || (Depth == 8) || (Depth == 24)) )
        goto WRAPUP;

    bResult = true;

WRAPUP:
    SisterFileLock.ReleaseKey();
    HASSERT(!(const_cast<HRFSunRasterCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFSunRasterCreator*>(this))->m_pSharingControl = 0;

    return bResult;
    }

//-----------------------------------------------------------------------------
// GetCapabilities
// Public (HRFSunRasterCreator)
// Create or get the singleton capabilities of SunRaster file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFSunRasterCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFSunRasterCapabilities();

    return m_pCapabilities;
    }


//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------

HRFSunRasterFile::HRFSunRasterFile(const HFCPtr<HFCURL>& pi_rURL,
                                   HFCAccessMode         pi_AccessMode,
                                   uint64_t             pi_Offset)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor store the access mode
    m_IsOpen            = false;
    m_HeaderChanged     = false;
    m_LinePadBits       = 16;

    if (GetAccessMode().m_HasCreateAccess)
        {
        Create();
        }
    else
        {
        // Create Page and Res Descriptors.
        Open();
        CreateDescriptors();
        }
    }

//-----------------------------------------------------------------------------
// Protected
// Constructor
// allow to Create an image file object without open.
//-----------------------------------------------------------------------------
HRFSunRasterFile::HRFSunRasterFile(const HFCPtr<HFCURL>& pi_rURL,
                                   HFCAccessMode         pi_AccessMode,
                                   uint64_t             pi_Offset,
                                   bool                 pi_DontOpenFile)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor store the access mode
    m_IsOpen        = false;
    m_HeaderChanged = false;
    m_LinePadBits   = 16;
    }

//-----------------------------------------------------------------------------
// Destructor
// Public
// Destroy SunRaster file object
//-----------------------------------------------------------------------------
HRFSunRasterFile::~HRFSunRasterFile()
    {
    try
        {
        // Close the SunRaster file and initialize the Compression Structure
        SaveSunRasterFile(true);
        }
    catch(...)
        {
        // Simply stop exceptions in the destructor
        // We want to known if a exception is throw.
        HASSERT(0);
        }
    }

//-----------------------------------------------------------------------------
// CreateResolutionEditor
// Public
// File manipulation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFSunRasterFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                              unsigned short pi_Resolution,
                                                              HFCAccessMode  pi_AccessMode)
    {
    // Verify that the page number is 0, because we have one image per file
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    HRFResolutionEditor* pEditor = 0;

    // Mode image if compressed
    if (m_FileHeader.m_Type == RT_RLENCODED)
        pEditor = new HRFSunRasterImageEditor(this, pi_Page, pi_Resolution, pi_AccessMode);
    else
        pEditor = new HRFSunRasterLineEditor(this, pi_Page, pi_Resolution, pi_AccessMode);

    return pEditor;
    }

//-----------------------------------------------------------------------------
// Save
// Public
// Saves file
//-----------------------------------------------------------------------------
void HRFSunRasterFile::Save()
    {

    //Keep last file position
    uint64_t CurrentPos = m_pSunRasterFile->GetCurrentPos();

    SaveSunRasterFile(false);

    //Set back position
    m_pSunRasterFile->SeekToPos(CurrentPos);
    }

//-----------------------------------------------------------------------------
// AddPage
// Public
// File manipulation
//-----------------------------------------------------------------------------
bool HRFSunRasterFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    // Validation if it's possible to add a page
    HPRECONDITION(CountPages() == 0);
    HPRECONDITION(pi_pPage != 0);

    // Add the page descriptor to the list
    HRFRasterFile::AddPage(pi_pPage);

    HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(0);
    HFCPtr<HRFResolutionDescriptor> pResolutionDescriptor = pPageDescriptor->GetResolutionDescriptor(0);

    // Set the image file information.
    m_FileHeader.m_Width        = (uint32_t)pResolutionDescriptor->GetWidth();
    m_FileHeader.m_Height       = (uint32_t)pResolutionDescriptor->GetHeight();

    if (pResolutionDescriptor->GetBlockType() == HRFBlockType::IMAGE)
        // Image --> compress RLE
        m_FileHeader.m_Type = RT_RLENCODED;
    else
        m_FileHeader.m_Type = RT_STANDARD;

    // Finding padding bits per row.
    uint32_t UsedBitsPerRow               = pResolutionDescriptor->GetPixelType()->CountPixelRawDataBits() *
                                         m_FileHeader.m_Width;
    m_PaddingBitsPerRow                = (m_LinePadBits - (UsedBitsPerRow % m_LinePadBits)) %
                                         m_LinePadBits;

    // Caculate image size including padding.
    m_FileHeader.m_Length = m_FileHeader.m_Height *
                            ((UsedBitsPerRow + m_PaddingBitsPerRow)/8);

    // Lock the sister file.
    HFCLockMonitor SisterFileLock (GetLockManager());

    // Set the image color space.
    SetPixelTypeToPage(pResolutionDescriptor);

    // Write the file header information.
    SetFileHeaderToFile();

    // Increment the counters
    SharingControlIncrementCount();

    // Unlock the sister file.
    SisterFileLock.ReleaseKey();

    return true;
    }

//-----------------------------------------------------------------------------
// GetCapabilities
// Public
// Returnt the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFSunRasterFile::GetCapabilities () const
    {
    return (HRFSunRasterCreator::GetInstance()->GetCapabilities());
    }

//-----------------------------------------------------------------------------
// Open
// Protected
// This method open the file.
//-----------------------------------------------------------------------------
bool HRFSunRasterFile::Open()
    {
    // Open the file
    if (!m_IsOpen)
        {
        m_pSunRasterFile = HFCBinStream::Instanciate(GetURL(), m_Offset, GetAccessMode(), 0, true);

        // This creates the sister file for file sharing control if necessary.
        SharingControlCreate();

        // Lock the sister file.
        HFCLockMonitor SisterFileLock (GetLockManager());

        // Initialisation of file struct.
        GetFileHeaderFromFile();
        GetPaletteFromFile();

        // Unlock the sister file.
        SisterFileLock.ReleaseKey();

        m_IsOpen = true;
        }

    return true;
    }


//-----------------------------------------------------------------------------
// CreateDescriptors
// Protected
// Create SunRaster File Descriptors
//-----------------------------------------------------------------------------
void HRFSunRasterFile::CreateDescriptors ()
    {
    // Create Page and Resolution Description/Capabilities for this file.
    HFCPtr<HRFResolutionDescriptor>     pResolution;
    HFCPtr<HRFPageDescriptor>           pPage;

    HFCPtr<HRPPixelType>  pPixelType = CreatePixelTypeFromFile();

    // Find Padding Bits Per Row
    uint32_t UsedBitsPerRow  = pPixelType->CountPixelRawDataBits() *
                            m_FileHeader.m_Width;
    m_PaddingBitsPerRow   = (m_LinePadBits - (UsedBitsPerRow % m_LinePadBits)) %
                            m_LinePadBits;

    HRFBlockAccess  BlockAccess;
    HRFBlockType    BlockType;
    uint32_t        BlockHeight(0);
    if (m_FileHeader.m_Type == RT_RLENCODED)
        {
        // If compressed, load in image mode.
        BlockAccess = HRFBlockAccess::SEQUENTIAL;
        BlockType   = HRFBlockType::IMAGE;
        BlockHeight = m_FileHeader.m_Height;
        }
    else
        {
        BlockAccess = HRFBlockAccess::RANDOM;
        BlockType   = HRFBlockType::LINE;
        BlockHeight = 1;
        }

    // Create Resolution Descriptor
    pResolution =  new HRFResolutionDescriptor(
        GetAccessMode(),               // AccessMode,
        GetCapabilities(),             // Capabilities,
        1.0,                           // XResolutionRatio,
        1.0,                           // YResolutionRatio,
        pPixelType,                    // PixelType,
        new HCDCodecIdentity(),        // Codec,
        BlockAccess,                   // RBlockAccess,
        BlockAccess,                   // WBlockAccess,
        HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL,  // ScanLineOrientation,
        HRFInterleaveType::PIXEL,      // InterleaveType
        0,                             // IsInterlace,
        m_FileHeader.m_Width,          // Width,
        m_FileHeader.m_Height,         // Height,
        m_FileHeader.m_Width,          // BlockWidth,
        BlockHeight,                   // BlockHeight,
        0,                             // BlocksDataFlag
        BlockType);                    // BlockType


    pPage = new HRFPageDescriptor (GetAccessMode(),
                                   GetCapabilities(),   // Capabilities,
                                   pResolution,         // ResolutionDescriptor,
                                   0,                   // RepresentativePalette,
                                   0,                   // Histogram,
                                   0,                   // Thumbnail,
                                   0,                   // ClipShape,
                                   0,                   // TransfoModel,
                                   0,                   // Filters
                                   0);                  // Defined Tag

    m_ListOfPageDescriptor.push_back(pPage);
    }


//-----------------------------------------------------------------------------
// SaveSunRasterFile
// Private
// This method saves and close the file if needed.
//-----------------------------------------------------------------------------
void HRFSunRasterFile::SaveSunRasterFile(bool pi_CloseFile)
    {
    // Be sure that the file is already open and that at least one page
    // has been add. Because if the destroyer is call afer a exception
    // was thrown, we want to be sure that the object is valid before we
    // execute the destroyer.
    if (m_IsOpen && m_ListOfPageDescriptor.size() > 0)
        {

        HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(0);

        // Lock the sister file.
        HFCLockMonitor SisterFileLock (GetLockManager());

        // if Create mode or the header is changed
        if (GetAccessMode().m_HasCreateAccess || m_HeaderChanged)
            {
            SetFileHeaderToFile();

            // Increment the counters
            SharingControlIncrementCount();
            }

        if(pPageDescriptor->GetResolutionDescriptor(0)->PaletteHasChanged())
            {
            // Set the image color space.
            SetPixelTypeToPage(pPageDescriptor->GetResolutionDescriptor(0));
            SetPaletteToFile();

            // Increment the counters
            SharingControlIncrementCount();

            pPageDescriptor->Saved();
            pPageDescriptor->GetResolutionDescriptor(0)->Saved();
            }



        // Unlock the sister file.
        SisterFileLock.ReleaseKey();

        if(pi_CloseFile)
            m_IsOpen = false;


        }
    }

//-----------------------------------------------------------------------------
// Create
// Private
// This method create the file.
//-----------------------------------------------------------------------------
bool HRFSunRasterFile::Create()
    {
    // Open the file.
    m_pSunRasterFile = HFCBinStream::Instanciate(GetURL(), GetAccessMode(), 0, true);

    // Instanciate the Sharing Control Object.
    SharingControlCreate();

    memset(&m_FileHeader, 0, sizeof(m_FileHeader));
    m_FileHeader.m_MagicNumber = RAS_MAGIC;

    m_IsOpen = true;

    return true;
    }

//-----------------------------------------------------------------------------
// CreatePixelTypeFromFile
// Private
// Find and create pixel type from file
//-----------------------------------------------------------------------------
HFCPtr<HRPPixelType> HRFSunRasterFile::CreatePixelTypeFromFile() const
    {
    HFCPtr<HRPPixelType> pPixelType;
    HRPPixelPalette*     pPalette;
    Byte                Value[3];
    uint32_t              MaxColor = 0;

    if ((m_FileHeader.m_Maptype != RMT_NOMAP) &&
        (m_FileHeader.m_Maplen != 0))
        {
        MaxColor = m_FileHeader.m_Maplen / 3;
        }

    switch (m_FileHeader.m_Depth)
        {
        case 1:
            if (MaxColor == 0)
                pPixelType = new HRPPixelTypeV1Gray1();
            else
                pPixelType = new HRPPixelTypeI1R8G8B8();
            break;

        case 8:
            if (MaxColor == 0)
                pPixelType = new HRPPixelTypeV8Gray8();
            else
                pPixelType = new HRPPixelTypeI8R8G8B8();
            break;

        case 24:
            pPixelType = new HRPPixelTypeV24R8G8B8();
            break;

        }

    // Pixeltype with palette ?
    if (MaxColor != 0)
        {
        // Get the palette from the pixel type.
        pPalette          = (HRPPixelPalette*)&(pPixelType->GetPalette());

        // Copy the SunRaster palette into the pixel palette.
        for (uint32_t Index = 0; Index < MaxColor ; ++Index)
            {
            Value[0] = m_pColorMapR[Index];
            Value[1] = m_pColorMapG[Index];
            Value[2] = m_pColorMapB[Index];

            // Add the entry to the pixel palette.
            pPalette->SetCompositeValue(Index, Value);
            }
        }

    return (pPixelType);
    }

//-----------------------------------------------------------------------------
// SetPixelTypeToPage
// Private
// Find and set pixel type to page
//-----------------------------------------------------------------------------
void HRFSunRasterFile::SetPixelTypeToPage(HFCPtr<HRFResolutionDescriptor> pi_pResolutionDescriptor)
    {
    HPRECONDITION(pi_pResolutionDescriptor != 0);

    m_IsBGR = false;

    if (pi_pResolutionDescriptor->GetPixelType()->GetClassID() ==
        HRPPixelTypeV24R8G8B8::CLASS_ID)
        {
        // Type == Normal
        m_IsBGR = true;

        m_FileHeader.m_Depth    = 24;
        m_FileHeader.m_Maptype  = RMT_NOMAP;
        m_FileHeader.m_Maplen   =  0;
        }

    else if (pi_pResolutionDescriptor->GetPixelType()->GetClassID() ==
             HRPPixelTypeV8Gray8::CLASS_ID)
        {
        m_FileHeader.m_Depth    = 8;
        m_FileHeader.m_Maptype  = RMT_NOMAP;
        m_FileHeader.m_Maplen   =  0;
        }

    else if (pi_pResolutionDescriptor->GetPixelType()->GetClassID() ==
             HRPPixelTypeV1Gray1::CLASS_ID)
        {
        m_FileHeader.m_Depth    = 1;
        m_FileHeader.m_Maptype  = RMT_NOMAP;
        m_FileHeader.m_Maplen   =  0;
        }

    else if (pi_pResolutionDescriptor->GetPixelType()->GetClassID() ==
             HRPPixelTypeI1R8G8B8::CLASS_ID)
        {
        Byte*                 pPaletteValue;
        const HRPPixelPalette& rPalette = pi_pResolutionDescriptor->GetPixelType()->GetPalette();

        m_FileHeader.m_Depth    = 1;
        m_FileHeader.m_Maptype  = RMT_RGBMAP;
        m_FileHeader.m_Maplen   = 6;

        m_pColorMapR = new Byte[6];
        memset(m_pColorMapR, 0, 6);
        m_pColorMapG = new Byte[6];
        memset(m_pColorMapG, 0, 6);
        m_pColorMapB = new Byte[6];
        memset(m_pColorMapB, 0, 6);

        uint32_t NBEntry = rPalette.CountUsedEntries();
        for (uint32_t Index=0 ; Index<NBEntry ; Index++)
            {
            pPaletteValue = (Byte*)rPalette.GetCompositeValue(Index);
            m_pColorMapR[Index] = pPaletteValue[0];
            m_pColorMapG[Index] = pPaletteValue[1];
            m_pColorMapB[Index] = pPaletteValue[2];
            }
        }

    else if (pi_pResolutionDescriptor->GetPixelType()->GetClassID() ==
             HRPPixelTypeI8R8G8B8::CLASS_ID)
        {
        Byte*                 pPaletteValue;
        const HRPPixelPalette& rPalette = pi_pResolutionDescriptor->GetPixelType()->GetPalette();

        m_FileHeader.m_Depth    = 8;
        m_FileHeader.m_Maptype  = RMT_RGBMAP;
        m_FileHeader.m_Maplen   = 768;

        m_pColorMapR = new Byte[256];
        memset(m_pColorMapR, 0, 256);
        m_pColorMapG = new Byte[256];
        memset(m_pColorMapG, 0, 256);
        m_pColorMapB = new Byte[256];
        memset(m_pColorMapB, 0, 256);

        uint32_t NBEntry = rPalette.CountUsedEntries();
        for (uint32_t Index=0 ; Index<NBEntry ; Index++)
            {
            pPaletteValue = (Byte*)rPalette.GetCompositeValue(Index);
            // The palette is RGB, need convertion to BGR
            m_pColorMapR[Index] = pPaletteValue[0];
            m_pColorMapG[Index] = pPaletteValue[1];
            m_pColorMapB[Index] = pPaletteValue[2];
            }
        }
    }


//-----------------------------------------------------------------------------
// GetFileHeader
// Private
// Read file header from file
//-----------------------------------------------------------------------------
void HRFSunRasterFile::GetFileHeaderFromFile()
    {
    HPRECONDITION(m_pSunRasterFile != 0);
    HPRECONDITION(SharingControlIsLocked());

    m_pSunRasterFile->SeekToPos(HEADER_OFFSET);

    m_pSunRasterFile->Read(&m_FileHeader.m_MagicNumber, sizeof m_FileHeader.m_MagicNumber);
    m_pSunRasterFile->Read(&m_FileHeader.m_Width,       sizeof m_FileHeader.m_Width);
    m_pSunRasterFile->Read(&m_FileHeader.m_Height,      sizeof m_FileHeader.m_Height);
    m_pSunRasterFile->Read(&m_FileHeader.m_Depth,       sizeof m_FileHeader.m_Depth);
    m_pSunRasterFile->Read(&m_FileHeader.m_Length,      sizeof m_FileHeader.m_Length);
    m_pSunRasterFile->Read(&m_FileHeader.m_Type,        sizeof m_FileHeader.m_Type);
    m_pSunRasterFile->Read(&m_FileHeader.m_Maptype,     sizeof m_FileHeader.m_Maptype);
    m_pSunRasterFile->Read(&m_FileHeader.m_Maplen,      sizeof m_FileHeader.m_Maplen);

    // Utility from HTIFFUtils
    if (!SystemIsBigEndian())
        {
        SwabArrayOfLong(&m_FileHeader.m_MagicNumber, 1);
        SwabArrayOfLong(&m_FileHeader.m_Width, 1);
        SwabArrayOfLong(&m_FileHeader.m_Height, 1);
        SwabArrayOfLong(&m_FileHeader.m_Depth, 1);
        SwabArrayOfLong(&m_FileHeader.m_Length, 1);
        SwabArrayOfLong(&m_FileHeader.m_Type, 1);
        SwabArrayOfLong(&m_FileHeader.m_Maptype, 1);
        SwabArrayOfLong(&m_FileHeader.m_Maplen, 1);
        }

    if (m_FileHeader.m_Depth >= 24)
        {
        if (m_FileHeader.m_Type == RT_FOMRAT_RGB)
            m_IsBGR = false;
        else
            m_IsBGR = true;
        }
    else
        m_IsBGR = false;
    }

//-----------------------------------------------------------------------------
// SetFileHeader
// Private
// Write file header to file
//-----------------------------------------------------------------------------
void HRFSunRasterFile::SetFileHeaderToFile()
    {
    HPRECONDITION(m_pSunRasterFile != 0);
    HPRECONDITION(SharingControlIsLocked());

    uint32_t MagicNumber = m_FileHeader.m_MagicNumber;
    uint32_t Width       = m_FileHeader.m_Width;
    uint32_t Height      = m_FileHeader.m_Height;
    uint32_t Depth       = m_FileHeader.m_Depth;
    uint32_t Length      = m_FileHeader.m_Length;
    uint32_t Type        = m_FileHeader.m_Type;
    uint32_t Maptype     = m_FileHeader.m_Maptype;
    uint32_t Maplen      = m_FileHeader.m_Maplen;

    // Determinate if the system is Big or little Endian
    // Utility from HTIFFUtils
    if (!SystemIsBigEndian())
        {
        SwabArrayOfLong(&MagicNumber, 1);
        SwabArrayOfLong(&Width, 1);
        SwabArrayOfLong(&Height, 1);
        SwabArrayOfLong(&Depth, 1);
        SwabArrayOfLong(&Length, 1);
        SwabArrayOfLong(&Type, 1);
        SwabArrayOfLong(&Maptype, 1);
        SwabArrayOfLong(&Maplen, 1);
        }

    m_pSunRasterFile->SeekToPos(HEADER_OFFSET);

    m_pSunRasterFile->Write(&MagicNumber, sizeof MagicNumber);
    m_pSunRasterFile->Write(&Width,    sizeof Width);
    m_pSunRasterFile->Write(&Height,   sizeof Height);
    m_pSunRasterFile->Write(&Depth,    sizeof Depth);
    m_pSunRasterFile->Write(&Length,   sizeof Length);
    m_pSunRasterFile->Write(&Type,     sizeof Type);
    m_pSunRasterFile->Write(&Maptype,  sizeof Maptype);
    m_pSunRasterFile->Write(&Maplen,   sizeof Maplen);

    SetPaletteToFile();

    m_HeaderChanged = false;
    }


//-----------------------------------------------------------------------------
// GetPaletteFromFile
// Private
// Read palette from file
//-----------------------------------------------------------------------------
void HRFSunRasterFile::GetPaletteFromFile()
    {
    HPRECONDITION(m_pSunRasterFile != 0);
    HPRECONDITION(SharingControlIsLocked());

    if ((m_FileHeader.m_Maptype != RMT_NOMAP) &&
        (m_FileHeader.m_Maplen != 0))
        {
        m_pSunRasterFile->SeekToPos(COLORMAP_OFFSET);

        uint32_t MapChannelLen = m_FileHeader.m_Maplen / 3;
        m_pColorMapR = new Byte[MapChannelLen];
        m_pColorMapG = new Byte[MapChannelLen];
        m_pColorMapB = new Byte[MapChannelLen];

        m_pSunRasterFile->Read(m_pColorMapR, MapChannelLen);
        m_pSunRasterFile->Read(m_pColorMapG, MapChannelLen);
        m_pSunRasterFile->Read(m_pColorMapB, MapChannelLen);
        }
    }

//-----------------------------------------------------------------------------
// SetPaletteToFile
// Private
// Write palette to file
//-----------------------------------------------------------------------------
void HRFSunRasterFile::SetPaletteToFile()
    {
    HPRECONDITION(m_pSunRasterFile != 0);
    HPRECONDITION(SharingControlIsLocked());

    if ((m_FileHeader.m_Maptype != RMT_NOMAP) && (m_pColorMapB != 0))
        {
        m_pSunRasterFile->SeekToPos(COLORMAP_OFFSET);

        uint32_t MapChannelLen = m_FileHeader.m_Maplen / 3;
        m_pSunRasterFile->Write(m_pColorMapR, MapChannelLen);
        m_pSunRasterFile->Write(m_pColorMapG, MapChannelLen);
        m_pSunRasterFile->Write(m_pColorMapB, MapChannelLen);
        }
    }

//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
// File information
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFSunRasterFile::GetWorldIdentificator () const
    {
    return HGF2DWorld_UNKNOWNWORLD;
    }


//-----------------------------------------------------------------------------
// Protected
// GetFilePtr
// Get the Bmp file pointer.
//-----------------------------------------------------------------------------
HFCBinStream* HRFSunRasterFile::GetFilePtr  ()
    {
    return (m_pSunRasterFile);
    }
