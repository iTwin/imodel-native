//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFBmpFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFBmpFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HRFException.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HRFBmpFile.h>
#include <Imagepp/all/h/HRFBmpLineEditor.h>
#include <Imagepp/all/h/HRFBmpCompressImageEditor.h>
#include <Imagepp/all/h/HRPChannelOrgRGB.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI4R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV16R5G6B5.h>
#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>
#include <Imagepp/all/h/HRPPixelTypeV32B8G8R8X8.h>

#include <Imagepp/all/h/HCDCodecBMPRLE8.h>
#include <Imagepp/all/h/HCDCodecBMPRLE4.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HRPPixelTypeFactory.h>
#include <Imagepp/all/h/HRFUtility.h>

#include <Imagepp/all/h/HRFRasterFileCapabilities.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>

// Bit masks used for pixel type V16R5G6B5
static const BmpBitMasksHeader s_M_V16R5G6B5_MASKS(0x0000F800,0x000007E0,0x0000001F);

//-----------------------------------------------------------------------------
// HRFBMPBlockCapabilities
//-----------------------------------------------------------------------------
class HRFBMPBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFBMPBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Block Capability
        Add(new HRFLineCapability(HFC_READ_WRITE_CREATE,
                                  LONG_MAX,
                                  HRFBlockAccess::RANDOM));
        }
    };

//-----------------------------------------------------------------------------
// HRFBMPBlockCompressCapabilities
//-----------------------------------------------------------------------------
class HRFBMPBlockCompressCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFBMPBlockCompressCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Block Capability
        Add(new HRFImageCapability(HFC_READ_WRITE_CREATE,  // AccessMode
                                   LONG_MAX,               // MaxSizeInBytes
                                   0,                      // MinWidth
                                   LONG_MAX,               // MaxWidth
                                   0,                      // MinHeight
                                   LONG_MAX));             // MaxHeight
        }
    };


//-----------------------------------------------------------------------------
// HRFBMPCodecIdentityCapabilities
//-----------------------------------------------------------------------------
class HRFBMPCodecIdentityCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFBMPCodecIdentityCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFBMPBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFBMPCodec4BitsCapabilities
//-----------------------------------------------------------------------------
class HRFBMPCodec4BitsCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFBMPCodec4BitsCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFBMPBlockCapabilities()));

        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecBMPRLE4::CLASS_ID,
                                   new HRFBMPBlockCompressCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFBMPCodec8BitsCapabilities
//-----------------------------------------------------------------------------
class HRFBMPCodec8BitsCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFBMPCodec8BitsCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFBMPBlockCapabilities()));

        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecBMPRLE8::CLASS_ID,
                                   new HRFBMPBlockCompressCapabilities()));
        }
    };



//-----------------------------------------------------------------------------
// HRFBMPCapabilities
//-----------------------------------------------------------------------------
HRFBMPCapabilities::HRFBMPCapabilities()
    : HRFRasterFileCapabilities()
    {
    // PixelTypeI1R8G8B8
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeI1R8G8B8::CLASS_ID,
                                   new HRFBMPCodecIdentityCapabilities()));
    // PixelTypeI4R8G8B8
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeI4R8G8B8::CLASS_ID,
                                   new HRFBMPCodec4BitsCapabilities()));
    // PixelTypeI8R8G8B8
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeI8R8G8B8::CLASS_ID,
                                   new HRFBMPCodec8BitsCapabilities()));

    // PixelTypeV16R5G6B5
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE,
                                   HRPPixelTypeV16R5G6B5::CLASS_ID,
                                   new HRFBMPCodecIdentityCapabilities()));

    // PixelTypeV24B8G8R8
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV24B8G8R8::CLASS_ID,
                                   new HRFBMPCodecIdentityCapabilities()));

    // PixelTypeV32B8G8R8X8
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV32B8G8R8X8::CLASS_ID,
                                   new HRFBMPCodecIdentityCapabilities()));

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_WRITE_CREATE, HRFInterleaveType::PIXEL));

    // Single Resolution Capability
    Add(new HRFSingleResolutionCapability(HFC_READ_WRITE_CREATE));

    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_WRITE_CREATE));

    // Tag capability
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeXResolution(0.0)));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeYResolution(0.0)));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeResolutionUnit(0)));
    }

HFC_IMPLEMENT_SINGLETON(HRFBmpCreator)

//-----------------------------------------------------------------------------
// Constructor
// Public (HRFBmpCreator)
// This is the creator to instantiate bmp format
//-----------------------------------------------------------------------------
HRFBmpCreator::HRFBmpCreator()
    : HRFRasterFileCreator(HRFBmpFile::CLASS_ID)
    {
    // Bmp capabilities instance member initialization
    m_pCapabilities = 0;
    }


#ifdef __HMR_DEBUG

//-----------------------------------------------------------------------------
// CreateRawFileFromImageData
// Public (HRFBmpCreator)
// This function can be used to generate a bmp file from a image data buffer.
//-----------------------------------------------------------------------------
void HRFBmpCreator::CreateBmpFileFromImageData(HFCPtr<HFCURL>&       pi_rpFileName,
                                               uint32_t              pi_ImageDataWidth,
                                               uint32_t              pi_ImageDataHeight,
                                               HFCPtr<HRPPixelType>& pi_rpImageDataPixelType,
                                               Byte*               pi_ImageData)
    {
    /*
        HRFBmpCreator::GetInstance()->SetImageData(pi_ImageDataWidth, pi_ImageDataHeight, 0);
        HRFBmpCreator::GetInstance()->SetImagePixelType(pi_rpImageDataPixelType);
    */
    HFCPtr<HRFRasterFile> pRasterFile = HRFBmpCreator::GetInstance()->Create(pi_rpFileName,
                                                                             HFC_READ_WRITE_CREATE,
                                                                             0);

    HFCPtr<HRFResolutionDescriptor>         pResolution;
    HFCPtr<HRFPageDescriptor>               pPage;

    pResolution =  new HRFResolutionDescriptor(
        HFC_READ_WRITE_CREATE,                          // AccessMode,
        pRasterFile->GetCapabilities(),                 // Capabilities,
        1.0,                                            // XResolutionRatio,
        1.0,                                            // YResolutionRatio,
        pi_rpImageDataPixelType,                        // PixelType,
        new HCDCodecIdentity(),                         // Codec,
        HRFBlockAccess::RANDOM,                         // RBlockAccess,
        HRFBlockAccess::RANDOM,                         // WBlockAccess,
        HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL,  // ScanLineOrientation,
        HRFInterleaveType::PIXEL,                       // InterleaveType
        false,                                          // IsInterlace,
        pi_ImageDataWidth,                              // Width image ,
        pi_ImageDataHeight,                             // Height image,
        pi_ImageDataWidth,                              // BlockWidth,
        1,                                              // BlockHeight,
        0,                                              // BlocksDataFlag
        HRFBlockType::LINE);


    pPage = new HRFPageDescriptor (HFC_READ_WRITE_CREATE,
                                   pRasterFile->GetCapabilities(),                       // Capabilities,
                                   pResolution,                             // ResolutionDescriptor,
                                   0,                                       // RepresentativePalette,
                                   0,                                       // Histogram,
                                   0,                                       // Thumbnail,
                                   0,                                       // ClipShape,
                                   0,                                       // TransfoModel,
                                   0,                                       // Filters
                                   0);                                      // Attribute set


    pRasterFile->AddPage(pPage);

    HAutoPtr<HRFResolutionEditor> pResolutionEditor(pRasterFile->CreateResolutionEditor(0, (unsigned short)0, HFC_READ_WRITE_CREATE));
    Byte*                       pImageLine = pi_ImageData;
    unsigned short               BytesPerPixel = (unsigned short)(pi_rpImageDataPixelType->CountPixelRawDataBits() / 8);

    for (uint32_t pi_LineInd = 0; pi_LineInd < pi_ImageDataHeight; pi_LineInd++)
        {
        pResolutionEditor->WriteBlock(0, pi_LineInd, pImageLine);

        pImageLine += (pi_ImageDataWidth * BytesPerPixel);
        }
    }

#endif

//-----------------------------------------------------------------------------
// GetLabel
// Public (HRFBmpCreator)
// Identification information
//-----------------------------------------------------------------------------
WString HRFBmpCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_BMP()); // BMP File Format
    }

//-----------------------------------------------------------------------------
// GetSchemes
// Public (HRFBmpCreator)
// Identification information
//-----------------------------------------------------------------------------
WString HRFBmpCreator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

//-----------------------------------------------------------------------------
// GetExtensions
// Public (HRFBmpCreator)
// Identification information
//-----------------------------------------------------------------------------
WString HRFBmpCreator::GetExtensions() const
    {
    return WString(L"*.bmp;*.dib");
    }

//-----------------------------------------------------------------------------
// Create
// Public (HRFBmpCreator)
// allow to Open an image file
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFBmpCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                            HFCAccessMode   pi_AccessMode,
                                            uint64_t       pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // Open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFBmpFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }


//-----------------------------------------------------------------------------
// IsKindOfFile
// Public (HRFBmpCreator)
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------
bool HRFBmpCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                  uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    bool                   bResult = false;
    BmpFileHeader           FileHeader;
    BmpInfoHeader           InfoHeader;
    BitMapArrayHeader       ArrayHeader;
    BmpBitMasksHeader       BitMasks;
    HAutoPtr<HFCBinStream>  pFile;

    (const_cast<HRFBmpCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock(GetLockManager());

    // Open the BMP File & place file pointer at the start of the file
    pFile = HFCBinStream::Instanciate(pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

    if (pFile == 0 || pFile->GetLastException() != 0)
        goto WRAPUP;

    if (pFile->Read(&FileHeader.m_Type, sizeof FileHeader.m_Type) != sizeof FileHeader.m_Type)
        goto WRAPUP;

    // OS/2 Bitmap arrays processing. Choose the best resolution image.
    if (0x4142 == FileHeader.m_Type)
        {
        // Validate that we really got a bitmap array
        if (pFile->Read(&ArrayHeader.m_HeaderSize,    sizeof ArrayHeader.m_HeaderSize) != sizeof ArrayHeader.m_HeaderSize)
            goto WRAPUP; // Error

        if (40 != ArrayHeader.m_HeaderSize)
            goto WRAPUP; // Error

        // Skip to WRAPUP; we're ok with this checkup.
        bResult = true;
        goto WRAPUP;
        }

    // Verify that the type is BMP image
    if (FileHeader.m_Type != 0x4D42)
        goto WRAPUP;

    if (pFile->Read(&FileHeader.m_FileSize,      sizeof FileHeader.m_FileSize)      != sizeof FileHeader.m_FileSize      ||
        pFile->Read(&FileHeader.m_Reserved1,     sizeof FileHeader.m_Reserved1)     != sizeof FileHeader.m_Reserved1     ||
        pFile->Read(&FileHeader.m_Reserved2,     sizeof FileHeader.m_Reserved2)     != sizeof FileHeader.m_Reserved2     ||
        pFile->Read(&FileHeader.m_OffBitsToData, sizeof FileHeader.m_OffBitsToData) != sizeof FileHeader.m_OffBitsToData ||
        pFile->Read(&InfoHeader.m_StructSize,           sizeof InfoHeader.m_StructSize)    != sizeof InfoHeader.m_StructSize)
        goto WRAPUP;

    // Windows 3.x BMP
    if (InfoHeader.m_StructSize == sizeof(BmpInfoHeader))
        {
        if (pFile->Read(&InfoHeader.m_Width,       sizeof InfoHeader.m_Width)       != sizeof InfoHeader.m_Width     ||
            pFile->Read(&InfoHeader.m_Height,      sizeof InfoHeader.m_Height)      != sizeof InfoHeader.m_Height    ||
            pFile->Read(&InfoHeader.m_Planes,      sizeof InfoHeader.m_Planes)      != sizeof InfoHeader.m_Planes    ||
            pFile->Read(&InfoHeader.m_BitCount,    sizeof InfoHeader.m_BitCount)    != sizeof InfoHeader.m_BitCount  ||
            pFile->Read(&InfoHeader.m_Compression, sizeof InfoHeader.m_Compression) != sizeof InfoHeader.m_Compression ||
            !(InfoHeader.m_BitCount == 1 || InfoHeader.m_BitCount == 4 || InfoHeader.m_BitCount == 8 ||
              InfoHeader.m_BitCount == 16 || InfoHeader.m_BitCount == 24 || InfoHeader.m_BitCount == 32) )
            goto WRAPUP;
        }
    // PM DIBs
    else if (InfoHeader.m_StructSize  == sizeof(BitMapCoreHeader))
        {
        BitMapCoreHeader CoreHeader;

        if (pFile->Read(&CoreHeader.m_Width,        sizeof CoreHeader.m_Width)        != sizeof CoreHeader.m_Width        ||
            pFile->Read(&CoreHeader.m_Height,       sizeof CoreHeader.m_Height)       != sizeof CoreHeader.m_Height       ||
            pFile->Read(&CoreHeader.m_ColorPlanes,  sizeof CoreHeader.m_ColorPlanes)  != sizeof CoreHeader.m_ColorPlanes  ||
            pFile->Read(&CoreHeader.m_BitsPerPixel, sizeof CoreHeader.m_BitsPerPixel) != sizeof CoreHeader.m_BitsPerPixel ||
            !(CoreHeader.m_BitsPerPixel == 1 || CoreHeader.m_BitsPerPixel == 4 || CoreHeader.m_BitsPerPixel == 8 ||
              CoreHeader.m_BitsPerPixel == 16 || CoreHeader.m_BitsPerPixel == 24 || CoreHeader.m_BitsPerPixel == 32) )
            goto WRAPUP;

        InfoHeader.m_Width         = CoreHeader.m_Width;
        InfoHeader.m_Height        = CoreHeader.m_Height;
        InfoHeader.m_Planes        = CoreHeader.m_ColorPlanes;
        InfoHeader.m_BitCount      = CoreHeader.m_BitsPerPixel;
        InfoHeader.m_Compression   = 0;  // COMPRESS_RGB
        }
    else
        goto WRAPUP;

    // Verify if there are bit masks
    if ( InfoHeader.m_Compression == 3 )
        {
        // We need to check if the pixel type corresponding to those bit masks is supported

        if (pFile->Read(&InfoHeader.m_SizeImage,     sizeof InfoHeader.m_SizeImage)     != sizeof InfoHeader.m_SizeImage     ||
            pFile->Read(&InfoHeader.m_XPelsPerMeter, sizeof InfoHeader.m_XPelsPerMeter) != sizeof InfoHeader.m_XPelsPerMeter ||
            pFile->Read(&InfoHeader.m_YPelsPerMeter, sizeof InfoHeader.m_YPelsPerMeter) != sizeof InfoHeader.m_YPelsPerMeter ||
            pFile->Read(&InfoHeader.m_ClrUsed,       sizeof InfoHeader.m_ClrUsed)       != sizeof InfoHeader.m_ClrUsed       ||
            pFile->Read(&InfoHeader.m_ClrImportant,  sizeof InfoHeader.m_ClrImportant)  != sizeof InfoHeader.m_ClrImportant  ||
            pFile->Read(&BitMasks.m_RedMask,         sizeof BitMasks.m_RedMask)         != sizeof BitMasks.m_RedMask         ||
            pFile->Read(&BitMasks.m_GreenMask,       sizeof BitMasks.m_GreenMask)       != sizeof BitMasks.m_GreenMask       ||
            pFile->Read(&BitMasks.m_BlueMask,        sizeof BitMasks.m_BlueMask)        != sizeof BitMasks.m_BlueMask)
            goto WRAPUP;


        // If the bitmask is not that of V16R5G6B5 the Pixel Type is not yet supported
        if ( BitMasks != s_M_V16R5G6B5_MASKS )
            goto WRAPUP;
        }

    bResult = true;

WRAPUP:
    SisterFileLock.ReleaseKey();

    HASSERT(!(const_cast<HRFBmpCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFBmpCreator*>(this))->m_pSharingControl = 0;

    return bResult;
    }

//-----------------------------------------------------------------------------
// GetCapabilities
// Public (HRFBmpCreator)
// Create or get the singleton capabilities of BMP file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFBmpCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFBMPCapabilities();

    return m_pCapabilities;
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFBmpFile::HRFBmpFile(const HFCPtr<HFCURL>& pi_rURL,
                       HFCAccessMode         pi_AccessMode,
                       uint64_t             pi_Offset)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor store the access mode
    m_IsOpen                = false;

    m_IsBitmapArrayFile     = false;
    m_BmpFileHeaderOffset   = 0;

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
HRFBmpFile::HRFBmpFile(const HFCPtr<HFCURL>& pi_rURL,
                              HFCAccessMode         pi_AccessMode,
                              uint64_t             pi_Offset,
                              bool                 pi_DontOpenFile)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor store the access mode
    m_IsOpen     = false;

    m_IsBitmapArrayFile     = false;
    m_BmpFileHeaderOffset   = 0;
    }

//-----------------------------------------------------------------------------
// Destructor
// Public
// Destroy Bmp file object
//-----------------------------------------------------------------------------
HRFBmpFile::~HRFBmpFile()
    {
    try
        {
        // Close the BMP file and initialize the Compression Structure
        SaveBmpFile(true);
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
// GetWorldIdentificator
// File information
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFBmpFile::GetWorldIdentificator () const
    {
    return HGF2DWorld_UNKNOWNWORLD;
    }

//-----------------------------------------------------------------------------
// Protected
// GetFilePtr
// Get the Bmp file pointer.
//-----------------------------------------------------------------------------
HFCBinStream* HRFBmpFile::GetFilePtr  ()
    {
    return (m_pBmpFile);
    }

//-----------------------------------------------------------------------------
// CreateResolutionEditor
// Public
// File manipulation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFBmpFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                        unsigned short pi_Resolution,
                                                        HFCAccessMode  pi_AccessMode)
    {
    // Verify that the page number is 0, because we have one image per file
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    HRFResolutionEditor* pEditor = 0;

    if (!(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution)->GetCodec()->IsCompatibleWith(HCDCodecBMPRLE8::CLASS_ID) ||
          GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution)->GetCodec()->IsCompatibleWith(HCDCodecBMPRLE4::CLASS_ID)))
        pEditor = new HRFBmpLineEditor(this, pi_Page, pi_Resolution, pi_AccessMode);
    else
        // We have a compress BMP
        pEditor = new HRFBmpCompressImageEditor(this, pi_Page, pi_Resolution, pi_AccessMode);

    return pEditor;
    }

//-----------------------------------------------------------------------------
// AddPage
// Public
// File manipulation
//-----------------------------------------------------------------------------
bool HRFBmpFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    // Validation if it's possible to add a page
    HPRECONDITION(CountPages() == 0);
    HPRECONDITION(pi_pPage != 0);

    // Add the page descriptor to the list
    HRFRasterFile::AddPage(pi_pPage);

    HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(0);
    HFCPtr<HRFResolutionDescriptor> pResolutionDescriptor = pPageDescriptor->GetResolutionDescriptor(0);

    // Display each tag.
    HPMAttributeSet::HPMASiterator TagIterator;

    unsigned short Unit = 1; // Unkown unit
    double XResolution = 0;
    double YResolution = 0;

    for (TagIterator  = pPageDescriptor->GetTags().begin();
         TagIterator != pPageDescriptor->GetTags().end(); TagIterator++)
        {
        HFCPtr<HPMGenericAttribute> pTag = (*TagIterator);

        // X Resolution Tag
        if (pTag->GetID() == HRFAttributeXResolution::ATTRIBUTE_ID)
            XResolution = ((HFCPtr<HRFAttributeXResolution>&)pTag)->GetData();
        // Y Resolution Tag
        else if (pTag->GetID() == HRFAttributeYResolution::ATTRIBUTE_ID)
            YResolution = ((HFCPtr<HRFAttributeYResolution>&)pTag)->GetData();
        // Resolution Unit
        else if (pTag->GetID() == HRFAttributeResolutionUnit::ATTRIBUTE_ID)
            Unit = ((HFCPtr<HRFAttributeResolutionUnit>&)pTag)->GetData();

        }

    // Set the resolution (in meter)
    if(XResolution !=0 || YResolution !=0)
        {
        // Unit is centimeter
        if (Unit == 3)
            {
            m_BmpInfo.m_BmpInfoHeader.m_XPelsPerMeter = (uint32_t)(XResolution * 100);
            m_BmpInfo.m_BmpInfoHeader.m_YPelsPerMeter = (uint32_t)(YResolution * 100);
            }
        // Unit is inch
        else if (Unit == 2)
            {
            m_BmpInfo.m_BmpInfoHeader.m_XPelsPerMeter = (uint32_t)((XResolution / 2.54) * 100);
            m_BmpInfo.m_BmpInfoHeader.m_YPelsPerMeter = (uint32_t)((YResolution / 2.54) * 100);
            }
        else
            {
            m_BmpInfo.m_BmpInfoHeader.m_XPelsPerMeter = (uint32_t) XResolution;
            m_BmpInfo.m_BmpInfoHeader.m_YPelsPerMeter = (uint32_t) YResolution;
            }
        }
    else
        {
        m_BmpInfo.m_BmpInfoHeader.m_XPelsPerMeter = (uint32_t) XResolution;
        m_BmpInfo.m_BmpInfoHeader.m_YPelsPerMeter = (uint32_t) YResolution;
        }



    // Set the image file information.
    m_BmpFileHeader.m_Type                  = 0x4d42;
    m_BmpFileHeader.m_Reserved1             = 0;
    m_BmpFileHeader.m_Reserved2             = 0;

    m_BmpInfo.m_BmpInfoHeader.m_StructSize  = 40;       // Allways set to 40
    m_BmpInfo.m_BmpInfoHeader.m_Planes      = 1;        // Allways set to 1
    m_BmpInfo.m_BmpInfoHeader.m_Height      = (uint32_t)pResolutionDescriptor->GetHeight();
    m_BmpInfo.m_BmpInfoHeader.m_Width       = (uint32_t)pResolutionDescriptor->GetWidth();

    // Finding padding bits per row.
    uint32_t LinePadBits                  = 32;
    uint32_t UsedBitsPerRow               = pResolutionDescriptor->GetPixelType()->CountPixelRawDataBits() *
                                         m_BmpInfo.m_BmpInfoHeader.m_Width;
    m_PaddingBitsPerRow                = (unsigned short)(LinePadBits - (UsedBitsPerRow % LinePadBits)) % LinePadBits;

    // Caculate image size including padding.
    m_BmpInfo.m_BmpInfoHeader.m_SizeImage   =  m_BmpInfo.m_BmpInfoHeader.m_Height *
                                               ((UsedBitsPerRow + m_PaddingBitsPerRow)/8);

    // Set the image color space.
    SetPixelTypeToPage(pResolutionDescriptor);

    if(pResolutionDescriptor->GetCodec()->IsCompatibleWith(HCDCodecBMPRLE8::CLASS_ID))
        {
        m_BmpInfo.m_BmpInfoHeader.m_Compression = 1;
        m_pLinesOffsetBuffer = new uint32_t[m_BmpInfo.m_BmpInfoHeader.m_Height+1];
        }
    else if(pResolutionDescriptor->GetCodec()->IsCompatibleWith(HCDCodecBMPRLE4::CLASS_ID))
        {
        m_BmpInfo.m_BmpInfoHeader.m_Compression = 2;
        }
    else if(m_BmpBitMasksHeader.IsInitialized())
        {
        // Compression field is set to 3 indicating the presence of bitmasks
        m_BmpInfo.m_BmpInfoHeader.m_Compression = 3;
        }

    else
        {
        m_BmpInfo.m_BmpInfoHeader.m_Compression = 0;
        }



    m_BmpFileHeader.m_OffBitsToData = sizeof (m_BmpFileHeader.m_Type) +
                                      sizeof (m_BmpFileHeader.m_FileSize) +
                                      sizeof (m_BmpFileHeader.m_Reserved1) +
                                      sizeof (m_BmpFileHeader.m_Reserved2) +
                                      sizeof (m_BmpFileHeader.m_OffBitsToData) +
                                      sizeof (m_BmpInfo.m_BmpInfoHeader) +
                                      (4 * m_BmpInfo.m_BmpInfoHeader.m_ClrUsed);

    if ( m_BmpInfo.m_BmpInfoHeader.m_Compression == 3 )
        {
        // The masks will be written after the headers so their size must be added
        m_BmpFileHeader.m_OffBitsToData += sizeof (BmpBitMasksHeader);
        }


    m_BmpFileHeader.m_FileSize      = m_BmpFileHeader.m_OffBitsToData +
                                      m_BmpInfo.m_BmpInfoHeader.m_SizeImage;

    // Write the file header information.
    SetFileHeaderToFile();
    SetBmpInfoHeaderToFile();

    return true;
    }

//-----------------------------------------------------------------------------
// Public
// GetFileCurrentSize
// Get the file's current size.
//-----------------------------------------------------------------------------
uint64_t HRFBmpFile::GetFileCurrentSize() const
    {
    return HRFRasterFile::GetFileCurrentSize(m_pBmpFile);
    }

//-----------------------------------------------------------------------------
// GetCapabilities
// Public
// Returnt the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFBmpFile::GetCapabilities () const
    {
    return (HRFBmpCreator::GetInstance()->GetCapabilities());
    }

//-----------------------------------------------------------------------------
// Open
// Protected
// This method open the file.
//-----------------------------------------------------------------------------
bool HRFBmpFile::Open()
    {
    // Open the file
    if (!m_IsOpen)
        {
        // Open the actual bmp file.
        m_pBmpFile = HFCBinStream::Instanciate(GetURL(), m_Offset, GetPhysicalAccessMode(), 0, true);

        // This creates the sister file for file sharing control if necessary.
        SharingControlCreate();

        // Initialisation of file struct.
        if (!GetFileHeaderFromFile() ||
            !GetBmpInfoHeaderFromFile())
            return false; // Error


        m_IsOpen = true;
        }

    return true;
    }


//-----------------------------------------------------------------------------
// CreateDescriptors
// Protected
// Create Bmp File Descriptors
//-----------------------------------------------------------------------------
void HRFBmpFile::CreateDescriptors ()
    {
    // Obtain the width and height of the resolution.
    uint32_t Width  = m_BmpInfo.m_BmpInfoHeader.m_Width;
    uint32_t Height = m_BmpInfo.m_BmpInfoHeader.m_Height;

    // Create Page and Resolution Description/Capabilities for this file.
    HFCPtr<HRFResolutionDescriptor>     pResolution;
    HFCPtr<HRFPageDescriptor>           pPage;

    HFCPtr<HRPPixelType>  pPixelType = CreatePixelTypeFromFile();

    if (pPixelType == 0)
        {
        throw HRFPixelTypeNotSupportedException(GetURL()->GetURL());
        }

    // Find Padding Bits Per Row
    uint32_t LinePadBits     = 32;
    uint32_t UsedBitsPerRow  = pPixelType->CountPixelRawDataBits() *
                            m_BmpInfo.m_BmpInfoHeader.m_Width;
    m_PaddingBitsPerRow   = (unsigned short)(LinePadBits - (UsedBitsPerRow % LinePadBits)) % LinePadBits;

    // Create codec for compression.
    HFCPtr<HCDCodec> pCodec;

    HRFBlockAccess  BlockAccess = HRFBlockAccess::RANDOM;
    HRFBlockType    BlockType   = HRFBlockType::LINE;
    uint32_t        BlockHeight = 1;

    // Compression is RLE8
    if(m_BmpInfo.m_BmpInfoHeader.m_Compression == 1)
        {
        pCodec = new HCDCodecBMPRLE8();

        BlockType   = HRFBlockType::IMAGE;
        BlockHeight = Height;

        m_pLinesOffsetBuffer = new uint32_t[Height+1];
        }
    // Compression is RLE4
    else if(m_BmpInfo.m_BmpInfoHeader.m_Compression == 2)
        {
        pCodec = new HCDCodecBMPRLE4();

        BlockType   = HRFBlockType::IMAGE;
        BlockHeight = Height;

        // RLE4 is supported in read only.
        if (GetAccessMode().m_HasWriteAccess || GetAccessMode().m_HasCreateAccess)
            throw HRFAccessModeForCodeNotSupportedException(GetURL()->GetURL());
        }
    // Compression is NONE
    else
        {
        pCodec = new HCDCodecIdentity();
        }

    // Create Resolution Descriptor
    pResolution =  new HRFResolutionDescriptor(GetAccessMode(),               // AccessMode,
                                               GetCapabilities(),             // Capabilities,
                                               1.0,                           // XResolutionRatio,
                                               1.0,                           // YResolutionRatio,
                                               pPixelType,                    // PixelType,
                                               pCodec,                        // Codecs,
                                               BlockAccess,                   // RBlockAccess,
                                               BlockAccess,                   // WBlockAccess,
                                               HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL,  // ScanLineOrientation,
                                               HRFInterleaveType::PIXEL,      // InterleaveType
                                               0,                             // IsInterlace,
                                               Width,                         // Width,
                                               Height,                        // Height,
                                               Width,                         // BlockWidth,
                                               BlockHeight,                   // BlockHeight,
                                               0,                             // BlocksDataFlag
                                               BlockType);                    // BlockType
    // Tag information
    HPMAttributeSet TagList;

    HFCPtr<HPMGenericAttribute> pTag;

    // XRESOLUTION Tag
    pTag = new HRFAttributeXResolution(m_BmpInfo.m_BmpInfoHeader.m_XPelsPerMeter * 0.01);
    TagList.Set(pTag);

    // YRESOLUTION Tag
    pTag = new HRFAttributeYResolution(m_BmpInfo.m_BmpInfoHeader.m_YPelsPerMeter * 0.01);
    TagList.Set(pTag);

    // Resolution Unit
    pTag = new HRFAttributeResolutionUnit(3);
    TagList.Set(pTag);


    pPage = new HRFPageDescriptor (GetAccessMode(),         // AccessMode
                                   GetCapabilities(),       // Capabilities,
                                   pResolution,             // ResolutionDescriptor,
                                   0,                       // RepresentativePalette,
                                   0,                       // Histogram,
                                   0,                       // Thumbnail,
                                   0,                       // ClipShape,
                                   0,                       // TransfoModel,
                                   0,                       // Filters
                                   &TagList);               // Defined Tag

    m_ListOfPageDescriptor.push_back(pPage);
    }


//-----------------------------------------------------------------------------
// SaveBmpFile
// Private
// This method saves the file and close it if needed
//-----------------------------------------------------------------------------
void HRFBmpFile::SaveBmpFile(bool pi_CloseFile)
    {
    // Be sure that the file is already open and that at least one page
    // has been add. Because if the destroyer is call afer a exception
    // was thrown, we want to be sure that the object is valid before we
    // execute the destroyer.
    if (m_IsOpen && m_ListOfPageDescriptor.size() > 0)
        {

        if ( (GetAccessMode().m_HasWriteAccess) || (GetAccessMode().m_HasCreateAccess) )
            {
            bool   SaveHeader = false;
            unsigned short Unit = 1; // Unknown Unit
            double XResolution = 0;
            double YResolution = 0;

            HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(0);

            // Display each tag.
            bool ResolutionChanged = false;

            HPMAttributeSet::HPMASiterator TagIterator; 
            for (TagIterator  = pPageDescriptor->GetTags().begin();
                 TagIterator != pPageDescriptor->GetTags().end(); TagIterator++)
                {
                HFCPtr<HPMGenericAttribute> pTag = (*TagIterator);

                // X Resolution Tag
                if (pTag->GetID() == HRFAttributeXResolution::ATTRIBUTE_ID)
                    {
                    XResolution = ((HFCPtr<HRFAttributeXResolution>&)pTag)->GetData();
                    if (pPageDescriptor->TagHasChanged(*pTag))
                        ResolutionChanged = true;
                    }
                // Y Resolution Tag
                else if (pTag->GetID() == HRFAttributeYResolution::ATTRIBUTE_ID)
                    {
                    YResolution = ((HFCPtr<HRFAttributeYResolution>&)pTag)->GetData();
                    if (pPageDescriptor->TagHasChanged(*pTag))
                        ResolutionChanged = true;
                    }
                // Resolution Unit
                else if (pTag->GetID() == HRFAttributeResolutionUnit::ATTRIBUTE_ID)
                    {
                    Unit = ((HFCPtr<HRFAttributeResolutionUnit>&)pTag)->GetData();
                    if (pPageDescriptor->TagHasChanged(*pTag))
                        ResolutionChanged = true;
                    }
                }
            // Set the resolution (in meter)
            if(ResolutionChanged)
                {
                // Unit is centimeter
                if (Unit == 3)
                    {
                    m_BmpInfo.m_BmpInfoHeader.m_XPelsPerMeter = (uint32_t)(XResolution * 100);
                    m_BmpInfo.m_BmpInfoHeader.m_YPelsPerMeter = (uint32_t)(YResolution * 100);
                    }
                // Unit is inch
                else if (Unit == 2)
                    {
                    m_BmpInfo.m_BmpInfoHeader.m_XPelsPerMeter = (uint32_t)((XResolution / 2.54) * 100);
                    m_BmpInfo.m_BmpInfoHeader.m_YPelsPerMeter = (uint32_t)((YResolution / 2.54) * 100);
                    }
                else
                    {
                    m_BmpInfo.m_BmpInfoHeader.m_XPelsPerMeter = (uint32_t) XResolution;
                    m_BmpInfo.m_BmpInfoHeader.m_YPelsPerMeter = (uint32_t) YResolution;
                    }

                SaveHeader = true;
                }

            // Lock the sister file for the SetPalette operation
            HFCLockMonitor SisterFileLock(GetLockManager());

            if(pPageDescriptor->GetResolutionDescriptor(0)->PaletteHasChanged())
                {
                m_pBmpFile->SeekToPos(54 + m_BmpFileHeaderOffset);
                // Set the image color space.
                SetPixelTypeToPage(pPageDescriptor->GetResolutionDescriptor(0));
                SaveHeader = true;
                }

            // Write the file header information.
            if (SaveHeader)
                {
                m_pBmpFile->SeekToPos(m_BmpFileHeaderOffset);
                SetFileHeaderToFile();
                SetBmpInfoHeaderToFile();

                pPageDescriptor->Saved();
                pPageDescriptor->GetResolutionDescriptor(0)->Saved();
                }

            m_pBmpFile->Flush();

            // Unlock the sister file.
            SisterFileLock.ReleaseKey();
            }

        if(pi_CloseFile)
            {
            if(m_BmpInfo.m_BmpInfoHeader.m_Compression == 1)
                delete [] m_pLinesOffsetBuffer;

            m_IsOpen = false;
            m_pBmpFile = 0;
            }
        }
    }

//-----------------------------------------------------------------------------
// Save
// Private
// This method saves the file.
//-----------------------------------------------------------------------------
void HRFBmpFile::Save()
    {
    //Keep last file position
    uint64_t CurrentPos = m_pBmpFile->GetCurrentPos();

    SaveBmpFile(false);

    //Set back position
    m_pBmpFile->SeekToPos(CurrentPos);
    }

//-----------------------------------------------------------------------------
// Create
// Private
// This method create the file.
//-----------------------------------------------------------------------------
bool HRFBmpFile::Create()
    {
    // Open the file.
    m_pBmpFile = HFCBinStream::Instanciate(GetURL(), GetAccessMode(), 0, true);

    // This creates the sister file for file sharing control if necessary.
    SharingControlCreate();

    m_IsOpen = true;

    return true;
    }

//-----------------------------------------------------------------------------
// CreatePixelTypeFromFile
// Private
// Find and create pixel type from file
//-----------------------------------------------------------------------------
HFCPtr<HRPPixelType> HRFBmpFile::CreatePixelTypeFromFile() const
    {
    HFCPtr<HRPPixelType> pPixelType;
    uint32_t              Index;
    HRPPixelPalette*     pPalette;
    Byte                Value[3];
    uint32_t              maxColor;

    if (m_BmpInfo.m_BmpInfoHeader.m_ClrUsed == 0)
        maxColor = (uint32_t)pow(2.0, m_BmpInfo.m_BmpInfoHeader.m_BitCount);
    else
        maxColor = m_BmpInfo.m_BmpInfoHeader.m_ClrUsed;

    switch (m_BmpInfo.m_BmpInfoHeader.m_BitCount)
        {
        case 1:
        case 4:
        case 8:
            pPixelType = HRPPixelTypeFactory::GetInstance()->Create(HRPChannelOrgRGB(8,
                                                                                     8,
                                                                                     8,
                                                                                     0,
                                                                                     HRPChannelType::UNUSED,
                                                                                     HRPChannelType::VOID_CH,
                                                                                     0),
                                                                    m_BmpInfo.m_BmpInfoHeader.m_BitCount);

            // Get the palette from the pixel type.
            pPalette          = (HRPPixelPalette*)&(pPixelType->GetPalette());

            // Copy the BMP palette into the pixel palette.
            for (Index = 0; Index < maxColor ; ++Index)
                {
                Value[0] = m_BmpInfo.m_RgbColors[Index].m_rgbRed;
                Value[1] = m_BmpInfo.m_RgbColors[Index].m_rgbGreen;
                Value[2] = m_BmpInfo.m_RgbColors[Index].m_rgbBlue;

                // Add the entry to the pixel palette.
                pPalette->SetCompositeValue(Index, Value);
                }
            break;

            // We have a color image
            // Note : Only a BMP with V16R5G6B5 is supported for the moment
        case 16:

            if ( m_BmpBitMasksHeader == s_M_V16R5G6B5_MASKS )
                {
                pPixelType = new HRPPixelTypeV16R5G6B5();
                }

            break;

            // We have a color image (RGB).
        case 24:
            HASSERT(!(m_BmpInfo.m_BmpInfoHeader.m_Compression == 1 || m_BmpInfo.m_BmpInfoHeader.m_Compression == 2));
            pPixelType = new HRPPixelTypeV24B8G8R8();
            break;

            // We have a color image (RGBX).
        case 32:
            HASSERT(!(m_BmpInfo.m_BmpInfoHeader.m_Compression == 1 || m_BmpInfo.m_BmpInfoHeader.m_Compression == 2));
            pPixelType = new HRPPixelTypeV32B8G8R8X8();
            break;

        }
    return (pPixelType);
    }

//-----------------------------------------------------------------------------
// SetPixelTypeToPage
// Private
// Find and set pixel type to page
//-----------------------------------------------------------------------------
void HRFBmpFile::SetPixelTypeToPage(HFCPtr<HRFResolutionDescriptor> pi_pResolutionDescriptor)
    {
    HPRECONDITION(pi_pResolutionDescriptor != 0);

    uint32_t maxColor;

    if (pi_pResolutionDescriptor->GetPixelType()->GetClassID() == HRPPixelTypeV32B8G8R8X8::CLASS_ID)
        {
        m_BmpInfo.m_BmpInfoHeader.m_BitCount     = 32;
        m_BmpInfo.m_BmpInfoHeader.m_ClrUsed      = 0;
        m_BmpInfo.m_BmpInfoHeader.m_ClrImportant = 0;
        }
    else if (pi_pResolutionDescriptor->GetPixelType()->GetClassID() == HRPPixelTypeV24B8G8R8::CLASS_ID)
        {
        m_BmpInfo.m_BmpInfoHeader.m_BitCount     = 24;
        m_BmpInfo.m_BmpInfoHeader.m_ClrUsed      = 0;
        m_BmpInfo.m_BmpInfoHeader.m_ClrImportant = 0;
        }
    else if (pi_pResolutionDescriptor->GetPixelType()->GetClassID() == HRPPixelTypeV16R5G6B5::CLASS_ID)
        {
        m_BmpInfo.m_BmpInfoHeader.m_BitCount     = 16;
        m_BmpInfo.m_BmpInfoHeader.m_ClrUsed      = 0;
        m_BmpInfo.m_BmpInfoHeader.m_ClrImportant = 0;

        // The bit masks are set to this PixelType
        m_BmpBitMasksHeader = s_M_V16R5G6B5_MASKS;
        }

    else if (pi_pResolutionDescriptor->GetPixelType()->GetClassID() == HRPPixelTypeI1R8G8B8::CLASS_ID)
        {
        Byte*                 pPaletteValue;
        const HRPPixelPalette& rPalette = pi_pResolutionDescriptor->GetPixelType()->GetPalette();

        m_BmpInfo.m_BmpInfoHeader.m_BitCount     = 1;
        m_BmpInfo.m_BmpInfoHeader.m_ClrUsed      = rPalette.CountUsedEntries();
        m_BmpInfo.m_BmpInfoHeader.m_ClrImportant = m_BmpInfo.m_BmpInfoHeader.m_ClrUsed;

        if (m_BmpInfo.m_BmpInfoHeader.m_ClrUsed == 0)
            maxColor = (uint32_t)pow(2.0, m_BmpInfo.m_BmpInfoHeader.m_BitCount);
        else
            maxColor = m_BmpInfo.m_BmpInfoHeader.m_ClrUsed;

        m_BmpInfo.m_RgbColors = new RGBColor[maxColor];

        for (uint32_t Index=0 ; Index<m_BmpInfo.m_BmpInfoHeader.m_ClrUsed ; Index++)
            {
            pPaletteValue = (Byte*)rPalette.GetCompositeValue(Index);
            m_BmpInfo.m_RgbColors[Index].m_rgbRed      = pPaletteValue[0];
            m_BmpInfo.m_RgbColors[Index].m_rgbGreen    = pPaletteValue[1];
            m_BmpInfo.m_RgbColors[Index].m_rgbBlue     = pPaletteValue[2];
            m_BmpInfo.m_RgbColors[Index].m_rgbReserved = 0;
            }
        }
    else if (pi_pResolutionDescriptor->GetPixelType()->GetClassID() ==
             HRPPixelTypeI4R8G8B8::CLASS_ID)
        {
        Byte*          pPaletteValue;
        HRPPixelPalette Palette = pi_pResolutionDescriptor->GetPixelType()->GetPalette();

        m_BmpInfo.m_BmpInfoHeader.m_BitCount     = 4;
        m_BmpInfo.m_BmpInfoHeader.m_ClrUsed      = Palette.CountUsedEntries();
        m_BmpInfo.m_BmpInfoHeader.m_ClrImportant = m_BmpInfo.m_BmpInfoHeader.m_ClrUsed;

        if (m_BmpInfo.m_BmpInfoHeader.m_ClrUsed == 0)
            maxColor = (uint32_t)pow(2.0, m_BmpInfo.m_BmpInfoHeader.m_BitCount);
        else
            maxColor = m_BmpInfo.m_BmpInfoHeader.m_ClrUsed;

        m_BmpInfo.m_RgbColors = new RGBColor[maxColor];

        for (uint32_t Index=0 ; Index<m_BmpInfo.m_BmpInfoHeader.m_ClrUsed ; Index++)
            {
            pPaletteValue = (Byte*)Palette.GetCompositeValue(Index);
            m_BmpInfo.m_RgbColors[Index].m_rgbRed      = pPaletteValue[0];
            m_BmpInfo.m_RgbColors[Index].m_rgbGreen    = pPaletteValue[1];
            m_BmpInfo.m_RgbColors[Index].m_rgbBlue     = pPaletteValue[2];
            m_BmpInfo.m_RgbColors[Index].m_rgbReserved = 0;
            }
        }
    else if(pi_pResolutionDescriptor->GetPixelType()->GetClassID() ==
            HRPPixelTypeI8R8G8B8::CLASS_ID)
        {
        Byte*                 pPaletteValue;
        const HRPPixelPalette& rPalette = pi_pResolutionDescriptor->GetPixelType()->GetPalette();

        m_BmpInfo.m_BmpInfoHeader.m_BitCount     = 8;
        m_BmpInfo.m_BmpInfoHeader.m_ClrUsed      = rPalette.CountUsedEntries();
        m_BmpInfo.m_BmpInfoHeader.m_ClrImportant = m_BmpInfo.m_BmpInfoHeader.m_ClrUsed;

        if (m_BmpInfo.m_BmpInfoHeader.m_ClrUsed == 0)
            maxColor = (uint32_t)pow(2.0, m_BmpInfo.m_BmpInfoHeader.m_BitCount);
        else
            maxColor = m_BmpInfo.m_BmpInfoHeader.m_ClrUsed;

        m_BmpInfo.m_RgbColors = new RGBColor[maxColor];

        for (uint32_t Index=0 ; Index<m_BmpInfo.m_BmpInfoHeader.m_ClrUsed ; Index++)
            {
            pPaletteValue = (Byte*)rPalette.GetCompositeValue(Index);
            m_BmpInfo.m_RgbColors[Index].m_rgbRed      = pPaletteValue[0];
            m_BmpInfo.m_RgbColors[Index].m_rgbGreen    = pPaletteValue[1];
            m_BmpInfo.m_RgbColors[Index].m_rgbBlue     = pPaletteValue[2];
            m_BmpInfo.m_RgbColors[Index].m_rgbReserved = 0;
            }
        }
    }


//-----------------------------------------------------------------------------
// ReadBestResolutionArrayHeader
// Public
// Seek in the file for the best resolution bmp. This applies only to bitmap
// array files. Return as a reference, the bitmap array header.Return an error
// if the file is not as expected for a OS/2 bitmap array.
//
// Returns false on errors, true otherwise
//-----------------------------------------------------------------------------
bool HRFBmpFile::ReadBestResolutionArrayHeader         (HFCBinStream*              pio_pFile,
                                                         BitMapArrayHeader*         po_pBmpArrayHeader)
    {
    HPRECONDITION(pio_pFile != NULL);
    // At this point, the file type field is supposed to be the only thing that has been read
    HPRECONDITION(pio_pFile->GetCurrentPos() == (sizeof po_pBmpArrayHeader->m_Type));

    // Unread the file type field.
    // NOTE: This had been done to keep the code clean of repetitions. One could as well unroll 1 iteration of the while loop
    //       and read only the fields after the type field on this first unrolled iteration. It would probably be a trifle more performant.
    pio_pFile->Seek(-(int64_t)(sizeof po_pBmpArrayHeader->m_Type));


    po_pBmpArrayHeader->m_NextOffset = 0;

    // Skip to last embedded image since in samples OS/2 bitmaps these have highest resolution.
    // NOTE: It is assumed that there is at least one resolution image embedded in a OS/2 bitmap.
    do
        {
        pio_pFile->SeekToPos(po_pBmpArrayHeader->m_NextOffset);

        if (pio_pFile->Read(&po_pBmpArrayHeader->m_Type,          sizeof po_pBmpArrayHeader->m_Type)       != sizeof po_pBmpArrayHeader->m_Type         ||
            pio_pFile->Read(&po_pBmpArrayHeader->m_HeaderSize,    sizeof po_pBmpArrayHeader->m_HeaderSize) != sizeof po_pBmpArrayHeader->m_HeaderSize   ||
            pio_pFile->Read(&po_pBmpArrayHeader->m_NextOffset,    sizeof po_pBmpArrayHeader->m_NextOffset) != sizeof po_pBmpArrayHeader->m_NextOffset)
            return false; // Error

        if (0x4142 != po_pBmpArrayHeader->m_Type || 40 != po_pBmpArrayHeader->m_HeaderSize)
            return false; // Error
        }
    while(po_pBmpArrayHeader->m_NextOffset);


    if (pio_pFile->Read(&po_pBmpArrayHeader->m_XDisplay,  sizeof po_pBmpArrayHeader->m_XDisplay)   != sizeof po_pBmpArrayHeader->m_XDisplay ||
        pio_pFile->Read(&po_pBmpArrayHeader->m_YDisplay,  sizeof po_pBmpArrayHeader->m_YDisplay)   != sizeof po_pBmpArrayHeader->m_YDisplay)
        return false; // Error


    return true; // No Errors
    }

//-----------------------------------------------------------------------------
// GetFileHeader
// Private
// Read file header from file
//-----------------------------------------------------------------------------
bool HRFBmpFile::GetFileHeaderFromFile()
    {
    HPRECONDITION(m_pBmpFile != 0);

    // Lock the sister file for the getFileHeaderFromFile method
    HFCLockMonitor SisterFileLock(GetLockManager());

    if (m_pBmpFile->Read(&m_BmpFileHeader.m_Type, sizeof m_BmpFileHeader.m_Type) != sizeof m_BmpFileHeader.m_Type)
        return false; // Error

    // This is an OS/2 Bitmap array. Move to the best resolution bitmap in the array.
    if (0x4142 == m_BmpFileHeader.m_Type)
        {
        if(!ReadBestResolutionArrayHeader(m_pBmpFile, &m_BmpArrayHeader))
            return false; // Error

        m_IsBitmapArrayFile = true;
        m_BmpFileHeaderOffset = m_pBmpFile->GetCurrentPos();

        // Reread the type field in the File Header so that the behaviour stays as if it was a normal bmp (not a bitmap array)
        if (m_pBmpFile->Read(&m_BmpFileHeader.m_Type, sizeof m_BmpFileHeader.m_Type) != sizeof m_BmpFileHeader.m_Type)
            return false; // Error
        }

    if (m_pBmpFile->Read(&m_BmpFileHeader.m_FileSize,      sizeof m_BmpFileHeader.m_FileSize)        != sizeof m_BmpFileHeader.m_FileSize    ||
        m_pBmpFile->Read(&m_BmpFileHeader.m_Reserved1,     sizeof m_BmpFileHeader.m_Reserved1)       != sizeof m_BmpFileHeader.m_Reserved1   ||
        m_pBmpFile->Read(&m_BmpFileHeader.m_Reserved2,     sizeof m_BmpFileHeader.m_Reserved2)       != sizeof m_BmpFileHeader.m_Reserved2   ||
        m_pBmpFile->Read(&m_BmpFileHeader.m_OffBitsToData, sizeof m_BmpFileHeader.m_OffBitsToData)   != sizeof m_BmpFileHeader.m_OffBitsToData)
        return false; // Error

    // Unlock the sister file
    SisterFileLock.ReleaseKey();

    return true; // No Errors
    }

//-----------------------------------------------------------------------------
// SetFileHeader
// Private
// Write file header to file
//-----------------------------------------------------------------------------
void HRFBmpFile::SetFileHeaderToFile()
    {
    HPRECONDITION(m_pBmpFile != 0);

    // Lock the sister file for the SetFileHeadertoFile method
    HFCLockMonitor SisterFileLock(GetLockManager());

    m_pBmpFile->Write(&m_BmpFileHeader.m_Type,          sizeof m_BmpFileHeader.m_Type);
    m_pBmpFile->Write(&m_BmpFileHeader.m_FileSize,      sizeof m_BmpFileHeader.m_FileSize);
    m_pBmpFile->Write(&m_BmpFileHeader.m_Reserved1,     sizeof m_BmpFileHeader.m_Reserved1);
    m_pBmpFile->Write(&m_BmpFileHeader.m_Reserved2,     sizeof m_BmpFileHeader.m_Reserved2);
    m_pBmpFile->Write(&m_BmpFileHeader.m_OffBitsToData, sizeof m_BmpFileHeader.m_OffBitsToData);

    // Unlock the sister file.
    SisterFileLock.ReleaseKey();
    }

//-----------------------------------------------------------------------------
// GetBmpInfoHeaderFromFile
// Private
// Read info header from file
//-----------------------------------------------------------------------------
bool HRFBmpFile::GetBmpInfoHeaderFromFile()
    {
    HPRECONDITION(m_pBmpFile != 0);

    // Lock the sister file for the GetBmpInfoHeaderFromFile method
    HFCLockMonitor SisterFileLock(GetLockManager());

    m_pBmpFile->Read(&m_BmpInfo.m_BmpInfoHeader.m_StructSize,    sizeof m_BmpInfo.m_BmpInfoHeader.m_StructSize);

    // Windows 3.x BMP
    if (m_BmpInfo.m_BmpInfoHeader.m_StructSize == sizeof(BmpInfoHeader))
        {
        m_IsRGBQuad = true;

        m_pBmpFile->Read(&m_BmpInfo.m_BmpInfoHeader.m_Width,         sizeof m_BmpInfo.m_BmpInfoHeader.m_Width);
        m_pBmpFile->Read(&m_BmpInfo.m_BmpInfoHeader.m_Height,        sizeof m_BmpInfo.m_BmpInfoHeader.m_Height);
        m_pBmpFile->Read(&m_BmpInfo.m_BmpInfoHeader.m_Planes,        sizeof m_BmpInfo.m_BmpInfoHeader.m_Planes);
        m_pBmpFile->Read(&m_BmpInfo.m_BmpInfoHeader.m_BitCount,      sizeof m_BmpInfo.m_BmpInfoHeader.m_BitCount);
        m_pBmpFile->Read(&m_BmpInfo.m_BmpInfoHeader.m_Compression,   sizeof m_BmpInfo.m_BmpInfoHeader.m_Compression);
        m_pBmpFile->Read(&m_BmpInfo.m_BmpInfoHeader.m_SizeImage,     sizeof m_BmpInfo.m_BmpInfoHeader.m_SizeImage);
        m_pBmpFile->Read(&m_BmpInfo.m_BmpInfoHeader.m_XPelsPerMeter, sizeof m_BmpInfo.m_BmpInfoHeader.m_XPelsPerMeter);
        m_pBmpFile->Read(&m_BmpInfo.m_BmpInfoHeader.m_YPelsPerMeter, sizeof m_BmpInfo.m_BmpInfoHeader.m_YPelsPerMeter);
        m_pBmpFile->Read(&m_BmpInfo.m_BmpInfoHeader.m_ClrUsed,       sizeof m_BmpInfo.m_BmpInfoHeader.m_ClrUsed);
        m_pBmpFile->Read(&m_BmpInfo.m_BmpInfoHeader.m_ClrImportant,  sizeof m_BmpInfo.m_BmpInfoHeader.m_ClrImportant);

        if ( m_BmpInfo.m_BmpInfoHeader.m_Compression == 3 )
            {
            //We need to read the bit masks
            m_pBmpFile->Read(&m_BmpBitMasksHeader.m_RedMask,            sizeof m_BmpBitMasksHeader.m_RedMask);
            m_pBmpFile->Read(&m_BmpBitMasksHeader.m_GreenMask,          sizeof m_BmpBitMasksHeader.m_GreenMask);
            m_pBmpFile->Read(&m_BmpBitMasksHeader.m_BlueMask,           sizeof m_BmpBitMasksHeader.m_BlueMask);
            }
        }
    // PM DIBs
    else if (m_BmpInfo.m_BmpInfoHeader.m_StructSize  == sizeof(BitMapCoreHeader))
        {
        BitMapCoreHeader CoreHeader;

        m_IsRGBQuad = false;

        m_pBmpFile->Read(&CoreHeader.m_Width,        sizeof CoreHeader.m_Width);
        m_pBmpFile->Read(&CoreHeader.m_Height,       sizeof CoreHeader.m_Height);
        m_pBmpFile->Read(&CoreHeader.m_ColorPlanes,  sizeof CoreHeader.m_ColorPlanes);
        m_pBmpFile->Read(&CoreHeader.m_BitsPerPixel, sizeof CoreHeader.m_BitsPerPixel);

        m_BmpInfo.m_BmpInfoHeader.m_StructSize  = sizeof (BmpInfoHeader);

        m_BmpInfo.m_BmpInfoHeader.m_Width         = CoreHeader.m_Width;
        m_BmpInfo.m_BmpInfoHeader.m_Height        = CoreHeader.m_Height;
        m_BmpInfo.m_BmpInfoHeader.m_Planes        = CoreHeader.m_ColorPlanes;
        m_BmpInfo.m_BmpInfoHeader.m_BitCount      = CoreHeader.m_BitsPerPixel;
        m_BmpInfo.m_BmpInfoHeader.m_Compression   = 0;  // COMPRESS_RGB
        m_BmpInfo.m_BmpInfoHeader.m_SizeImage     = 0;
        m_BmpInfo.m_BmpInfoHeader.m_XPelsPerMeter = 0;
        m_BmpInfo.m_BmpInfoHeader.m_YPelsPerMeter = 0;
        m_BmpInfo.m_BmpInfoHeader.m_ClrUsed       = 0;
        m_BmpInfo.m_BmpInfoHeader.m_ClrImportant  = 0;
        }
    else //V4 and V5 are not supported
        throw HRFUnsupportedBMPVersionException(GetURL()->GetURL());

    // The maximum size should be where each line is aligned on double word boundary.
    if (!m_BmpInfo.m_BmpInfoHeader.m_SizeImage)
        {
        uint32_t BitsPerWord;
        uint32_t BytesPerLine;
        uint32_t LongsPerLine;

        BitsPerWord  = 32 / m_BmpInfo.m_BmpInfoHeader.m_BitCount;
        LongsPerLine = m_BmpInfo.m_BmpInfoHeader.m_Width / BitsPerWord +
                       (m_BmpInfo.m_BmpInfoHeader.m_Width % BitsPerWord != 0);
        BytesPerLine = LongsPerLine * sizeof(long);
        m_BmpInfo.m_BmpInfoHeader.m_SizeImage = BytesPerLine * m_BmpInfo.m_BmpInfoHeader.m_Height;
        }

    GetPaletteFromFile();

    // Unlock the sister file;
    SisterFileLock.ReleaseKey();

    // NOTE: It was initially assumed that no read error could happen reading those fields
    return true; // No Errors
    }

//-----------------------------------------------------------------------------
// SetBmpInfoHeaderFromFile
// Private
// Write info header to file
//-----------------------------------------------------------------------------
void HRFBmpFile::SetBmpInfoHeaderToFile()
    {
    HPRECONDITION(m_pBmpFile != 0);

    // Lock the sister file for the SetBmpInfoHeaderToFile method
    HFCLockMonitor SisterFileLock(GetLockManager());

    m_pBmpFile->Write(&m_BmpInfo.m_BmpInfoHeader.m_StructSize,    sizeof m_BmpInfo.m_BmpInfoHeader.m_StructSize);
    m_pBmpFile->Write(&m_BmpInfo.m_BmpInfoHeader.m_Width,         sizeof m_BmpInfo.m_BmpInfoHeader.m_Width);
    m_pBmpFile->Write(&m_BmpInfo.m_BmpInfoHeader.m_Height,        sizeof m_BmpInfo.m_BmpInfoHeader.m_Height);
    m_pBmpFile->Write(&m_BmpInfo.m_BmpInfoHeader.m_Planes,        sizeof m_BmpInfo.m_BmpInfoHeader.m_Planes);
    m_pBmpFile->Write(&m_BmpInfo.m_BmpInfoHeader.m_BitCount,      sizeof m_BmpInfo.m_BmpInfoHeader.m_BitCount);
    m_pBmpFile->Write(&m_BmpInfo.m_BmpInfoHeader.m_Compression,   sizeof m_BmpInfo.m_BmpInfoHeader.m_Compression);
    m_pBmpFile->Write(&m_BmpInfo.m_BmpInfoHeader.m_SizeImage,     sizeof m_BmpInfo.m_BmpInfoHeader.m_SizeImage);
    m_pBmpFile->Write(&m_BmpInfo.m_BmpInfoHeader.m_XPelsPerMeter, sizeof m_BmpInfo.m_BmpInfoHeader.m_XPelsPerMeter);
    m_pBmpFile->Write(&m_BmpInfo.m_BmpInfoHeader.m_YPelsPerMeter, sizeof m_BmpInfo.m_BmpInfoHeader.m_YPelsPerMeter);
    m_pBmpFile->Write(&m_BmpInfo.m_BmpInfoHeader.m_ClrUsed,       sizeof m_BmpInfo.m_BmpInfoHeader.m_ClrUsed);
    m_pBmpFile->Write(&m_BmpInfo.m_BmpInfoHeader.m_ClrImportant,  sizeof m_BmpInfo.m_BmpInfoHeader.m_ClrImportant);

    // If the compression is set to 3
    if ( m_BmpInfo.m_BmpInfoHeader.m_Compression == 3 )
        {
        //We need to write the bit masks
        m_pBmpFile->Write(&m_BmpBitMasksHeader.m_RedMask,            sizeof m_BmpBitMasksHeader.m_RedMask);
        m_pBmpFile->Write(&m_BmpBitMasksHeader.m_GreenMask,          sizeof m_BmpBitMasksHeader.m_GreenMask);
        m_pBmpFile->Write(&m_BmpBitMasksHeader.m_BlueMask,           sizeof m_BmpBitMasksHeader.m_BlueMask);
        }

    SetPaletteToFile();

    // Unlock the sister file.
    SisterFileLock.ReleaseKey();
    }

//-----------------------------------------------------------------------------
// GetPaletteToFile
// Private
// Read palette from file
//-----------------------------------------------------------------------------
void HRFBmpFile::GetPaletteFromFile()
    {
    HPRECONDITION(m_pBmpFile != 0);

    // We do not have palette in 24 bits bitmap
    if (m_BmpInfo.m_BmpInfoHeader.m_BitCount == 24)
        {
        m_BmpInfo.m_RgbColors = NULL;
        }
    // If we have a 16 bits image and color used is 0
    // Note : This doesn't take into consideration that a 16 bits image can have a palette, something better
    // must be found for such images.
    else if ( (m_BmpInfo.m_BmpInfoHeader.m_BitCount == 16) && (m_BmpInfo.m_BmpInfoHeader.m_ClrUsed == 0) )
        {
        m_BmpInfo.m_RgbColors = NULL;
        }
    else
        {
        uint32_t maxColor;

        if (m_BmpInfo.m_BmpInfoHeader.m_ClrUsed == 0)
            maxColor = (uint32_t)pow(2.0, m_BmpInfo.m_BmpInfoHeader.m_BitCount);
        else
            maxColor = m_BmpInfo.m_BmpInfoHeader.m_ClrUsed;

        m_BmpInfo.m_RgbColors = new RGBColor[maxColor];

        // Lock the sister file for the GetPaletteFromFile method
        HFCLockMonitor SisterFileLock(GetLockManager());

        for (uint32_t color=0; color < maxColor; color++)
            {
            m_pBmpFile->Read(&m_BmpInfo.m_RgbColors[color].m_rgbBlue,  sizeof m_BmpInfo.m_RgbColors[color].m_rgbBlue);
            m_pBmpFile->Read(&m_BmpInfo.m_RgbColors[color].m_rgbGreen, sizeof m_BmpInfo.m_RgbColors[color].m_rgbGreen);
            m_pBmpFile->Read(&m_BmpInfo.m_RgbColors[color].m_rgbRed,   sizeof m_BmpInfo.m_RgbColors[color].m_rgbRed);
            if(m_IsRGBQuad)
                m_pBmpFile->Read(&m_BmpInfo.m_RgbColors[color].m_rgbReserved, sizeof m_BmpInfo.m_RgbColors[color].m_rgbReserved);
            else
                m_BmpInfo.m_RgbColors[color].m_rgbReserved = 0;
            }

        // Unlock the sister file
        SisterFileLock.ReleaseKey();
        }

    }

//-----------------------------------------------------------------------------
// SetPaletteToFile
// Private
// Write palette to file
//-----------------------------------------------------------------------------
void HRFBmpFile::SetPaletteToFile()
    {
    HPRECONDITION(m_pBmpFile != 0);

    // We do not have palette in 24 bits bitmap
    // Note : This doesn't take into consideration that a 16 bits image can have a palette, something better
    // must be found for such images.
    if ((m_BmpInfo.m_BmpInfoHeader.m_BitCount != 16) &&
        (m_BmpInfo.m_BmpInfoHeader.m_BitCount != 24) &&
        (m_BmpInfo.m_BmpInfoHeader.m_BitCount != 32))
        {
        uint32_t maxColor;

        if (m_BmpInfo.m_BmpInfoHeader.m_ClrUsed == 0)
            maxColor = (uint32_t)pow(2.0, m_BmpInfo.m_BmpInfoHeader.m_BitCount);
        else
            maxColor = m_BmpInfo.m_BmpInfoHeader.m_ClrUsed;

        // Lock the sister file for the SetPaletteToFile method
        HFCLockMonitor SisterFileLock(GetLockManager());

        for (uint32_t color=0; color<maxColor; color++)
            {
            m_pBmpFile->Write(&m_BmpInfo.m_RgbColors[color].m_rgbBlue,     sizeof m_BmpInfo.m_RgbColors[color].m_rgbBlue);
            m_pBmpFile->Write(&m_BmpInfo.m_RgbColors[color].m_rgbGreen,    sizeof m_BmpInfo.m_RgbColors[color].m_rgbGreen);
            m_pBmpFile->Write(&m_BmpInfo.m_RgbColors[color].m_rgbRed,      sizeof m_BmpInfo.m_RgbColors[color].m_rgbRed);
            m_pBmpFile->Write(&m_BmpInfo.m_RgbColors[color].m_rgbReserved, sizeof m_BmpInfo.m_RgbColors[color].m_rgbReserved);
            }

        // Unlock the sister file.
        SisterFileLock.ReleaseKey();
        }
    }

//-----------------------------------------------------------------------------
// Constructor
// Public (BmpBitMasksHeader)
// Class constructor
//-----------------------------------------------------------------------------
BmpBitMasksHeader::BmpBitMasksHeader(uint32_t pi_RedMask, uint32_t pi_GreenMask, uint32_t pi_BlueMask )
    {
    m_RedMask = pi_RedMask;
    m_GreenMask = pi_GreenMask;
    m_BlueMask = pi_BlueMask;
    }

//-----------------------------------------------------------------------------
// ==
// Public (BmpBitMasksHeader)
// Is Equal operator
//-----------------------------------------------------------------------------
bool BmpBitMasksHeader::operator==(BmpBitMasksHeader pi_Mask) const
    {
    if ( ( pi_Mask.m_RedMask != m_RedMask) || ( pi_Mask.m_BlueMask != m_BlueMask) || ( pi_Mask.m_GreenMask != m_GreenMask) )
        {
        return false;
        }
    else
        {
        return true;
        }
    }

//-----------------------------------------------------------------------------
// !=
// Public (BmpBitMasksHeader)
// Not equal operator
//-----------------------------------------------------------------------------
bool BmpBitMasksHeader::operator!=(BmpBitMasksHeader pi_Mask) const
    {
    if ( (pi_Mask.m_RedMask == m_RedMask) && (pi_Mask.m_GreenMask == m_GreenMask) && (pi_Mask.m_BlueMask == m_BlueMask) )
        {
        return false;
        }
    else
        {
        return true;
        }
    }

//-----------------------------------------------------------------------------
// =
// Public (BmpBitMasksHeader)
// Assignation operator
//-----------------------------------------------------------------------------
BmpBitMasksHeader& BmpBitMasksHeader::operator=(BmpBitMasksHeader pi_Mask)
    {
    m_RedMask = pi_Mask.m_RedMask;
    m_GreenMask = pi_Mask.m_GreenMask;
    m_BlueMask = pi_Mask.m_BlueMask;

    return *this;
    }

//-----------------------------------------------------------------------------
// isInitialized
// Public (BmpBitMasksHeader)
// Checks wether filters have been set
//-----------------------------------------------------------------------------
bool BmpBitMasksHeader::IsInitialized() const
    {
    if ( (m_RedMask == 0) && (m_GreenMask == 0) && (m_BlueMask == 0) )
        {
        return false;
        }
    else
        {
        return true;
        }
    }
