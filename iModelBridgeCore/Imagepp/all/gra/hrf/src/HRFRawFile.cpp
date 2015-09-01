//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFRawFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFRawFile
//-----------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRFRawFile.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCBinStream.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HRFRawLineEditor.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HRFException.h>

#include <Imagepp/all/h/HRPPixelTypeV1Gray1.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV16PRGray8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>

#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>

//-----------------------------------------------------------------------------
// HRFRawBlockCapabilities
//-----------------------------------------------------------------------------
class HRFRawBlockCapabilities : public HRFRasterFileCapabilities
    {
public :
    // Constructor
    HRFRawBlockCapabilities ()
        : HRFRasterFileCapabilities()
        {
        // Block capability
        Add (new HRFLineCapability (HFC_READ_WRITE_CREATE,
                                    LONG_MAX,
                                    HRFBlockAccess::RANDOM));
        }
    };

//-----------------------------------------------------------------------------
// HRFRawCodecCapabilities
//-----------------------------------------------------------------------------
class HRFRawCodecCapabilities : public HRFRasterFileCapabilities
    {
public :
    // Constructor
    HRFRawCodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec
        Add (new HRFCodecCapability (HFC_READ_WRITE_CREATE,
                                     HCDCodecIdentity::CLASS_ID,
                                     new HRFRawBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFRawCapabilities
//-----------------------------------------------------------------------------
HRFRawCapabilities::HRFRawCapabilities()
    : HRFRasterFileCapabilities()
    {
    // PixelTYpeV1Gray1
    Add (new HRFPixelTypeCapability (HFC_READ_WRITE_CREATE,
                                     HRPPixelTypeV1Gray1::CLASS_ID,
                                     new HRFRawCodecCapabilities()));

    // PixelTypeV16Gray8A8
    Add (new HRFPixelTypeCapability (HFC_READ_WRITE_CREATE,
                                     HRPPixelTypeV16PRGray8A8::CLASS_ID,
                                     new HRFRawCodecCapabilities()));

    // PixelTypeV8Gray8
    Add (new HRFPixelTypeCapability (HFC_READ_WRITE_CREATE,
                                     HRPPixelTypeV8Gray8::CLASS_ID,
                                     new HRFRawCodecCapabilities()));

    // PixelTypeV24R8G8B8
    Add (new HRFPixelTypeCapability (HFC_READ_WRITE_CREATE,
                                     HRPPixelTypeV24R8G8B8::CLASS_ID,
                                     new HRFRawCodecCapabilities()));

    // PixelTypeV32R8G8B8A8
    Add (new HRFPixelTypeCapability (HFC_READ_WRITE_CREATE,
                                     HRPPixelTypeV32R8G8B8A8::CLASS_ID,
                                     new HRFRawCodecCapabilities()));

    // Scanline Orientation Capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));

    // Single Resolution Capability
    Add(new HRFSingleResolutionCapability(HFC_READ_WRITE_CREATE));

    // Interleave Capability
    Add(new HRFInterleaveCapability(HFC_READ_WRITE_CREATE, HRFInterleaveType::PIXEL));

    // Still Image Capability
    Add(new HRFStillImageCapability(HFC_READ_WRITE_CREATE));
    }

HFC_IMPLEMENT_SINGLETON(HRFRawCreator)

//-----------------------------------------------------------------------------
// Creator
// This is the creator to instantiate Raw format
//-----------------------------------------------------------------------------
HRFRawCreator::HRFRawCreator()
    : HRFRasterFileCreator(HRFRawFile::CLASS_ID)
    {
    // JPEG capabilities instance member initialization
    m_pCapabilities = 0;
    m_Width         = 0;
    m_Height        = 0;
    m_pPixelType    = 0;
    m_FileSize      = 0;
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------
WString HRFRawCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_RAW());  // Raw Data
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------
WString HRFRawCreator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

//-----------------------------------------------------------------------------
// Identification information
//-----------------------------------------------------------------------------
WString HRFRawCreator::GetExtensions() const
    {
    return WString(L"*.raw");
    }

//-----------------------------------------------------------------------------
// allow to Open an image file
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFRawCreator::Create(
    const HFCPtr<HFCURL>& pi_rpURL,
    HFCAccessMode         pi_AccessMode,
    uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);
    //HPRECONDITION(m_Width  != 0);
    //HPRECONDITION(m_Height != 0);
    HPRECONDITION(m_Offset >= 0);
    //HPRECONDITION((m_Width * m_Height * m_pPixelType->CountPixelRawDataBits() / 8) + m_Offset <= m_FileSize);



    // TODO : Should I add the information about the size of the file to the input param?
    HFCPtr<HRFRasterFile> pFile;

    if ((m_Width > 0) && (m_Height > 0) && (m_pPixelType != 0))
        {
        pFile = new HRFRawFile(pi_rpURL, m_Width, m_Height, m_pPixelType, pi_AccessMode, pi_Offset);
        }
    else
        {
        if (pi_AccessMode.m_HasCreateAccess)
            pFile = new HRFRawFile(pi_rpURL, 1, 1, new HRPPixelTypeV24R8G8B8(), pi_AccessMode, pi_Offset);
        else
            throw HFCFileNotCreatedException(pi_rpURL->GetURL());
        }

    HASSERT(pFile != 0);

    return (pFile);
    }

//-----------------------------------------------------------------------------
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------
bool HRFRawCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                  uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    bool Result = false;

    if (pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID) &&
        BeStringUtilities::Wcsicmp(((HFCPtr<HFCURLFile>&)pi_rpURL)->GetExtension().c_str(), L"raw") == 0)
        Result = true;

    return Result;
    }

//-----------------------------------------------------------------------------
// Create or get the singleton capabilities of Raw file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFRawCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFRawCapabilities();

    return m_pCapabilities;
    }


//-----------------------------------------------------------------------------
// Set the dimension of the image.
//-----------------------------------------------------------------------------
void HRFRawCreator::SetImageData (uint32_t pi_Width, uint32_t pi_Height, uint64_t pi_Offset)
    {
    m_Width  = pi_Width;
    m_Height = pi_Height;
    m_Offset = pi_Offset;
    }

//-----------------------------------------------------------------------------
// Get the offset of the image.
//-----------------------------------------------------------------------------
uint64_t HRFRawCreator::GetOffset () const
    {
    return m_Offset;
    }

//-----------------------------------------------------------------------------
// Set the pixeltype of the image.
//-----------------------------------------------------------------------------
void  HRFRawCreator::SetImagePixelType (HFCPtr<HRPPixelType> pi_pPixelType)
    {
    m_pPixelType = pi_pPixelType;
    }

//-----------------------------------------------------------------------------
// Get the pixeltype of the image.
//-----------------------------------------------------------------------------
HFCPtr<HRPPixelType> HRFRawCreator::GetImagePixelType () const
    {
    return m_pPixelType;
    }

//-----------------------------------------------------------------------------
// Get the size in bytes of the image.
//-----------------------------------------------------------------------------
uint64_t HRFRawCreator::GetFileSize () const
    {
    return m_FileSize;
    }

//-----------------------------------------------------------------------------
// Return the image width that best fit the image size with the given height.
//-----------------------------------------------------------------------------
uint32_t HRFRawCreator::GetBestFitWidth(uint32_t pi_Height, uint64_t pi_Offset, size_t pi_Footer) const
    {
    uint64_t FileSize = ((m_FileSize - pi_Offset - pi_Footer) * 8) /
                       (m_pPixelType->CountPixelRawDataBits());

    return (uint32_t)(FileSize / pi_Height);
    }

//-----------------------------------------------------------------------------
// Return the image height that best fit the image size with the given width.
//-----------------------------------------------------------------------------
uint32_t HRFRawCreator::GetBestFitHeight(uint32_t pi_Width, uint64_t pi_Offset, size_t pi_Footer) const
    {
    uint64_t FileSize = ((m_FileSize - pi_Offset - pi_Footer) * 8) /
                       (m_pPixelType->CountPixelRawDataBits());

    return (uint32_t)(FileSize / pi_Width);
    }

//-----------------------------------------------------------------------------
// Return an approximation of the width and heigth of the image.
//-----------------------------------------------------------------------------
void HRFRawCreator::AutoDetectFileSize (const HFCPtr<HFCURL>& pi_rpURL,
                                        uint64_t             pi_Offset,
                                        uint32_t              pi_Footer,
                                        uint32_t              pi_BitsPerPixel,
                                        uint32_t&               po_Width,
                                        uint32_t&               po_Height)
    {
    HASSERT (pi_rpURL != 0);
    HASSERT (pi_BitsPerPixel != 0);

    size_t                  FileSize;
    uint32_t                 Size;
    HAutoPtr<HFCBinStream>  pFile;

    (const_cast<HRFRawCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock(GetLockManager());

    // Open the Raw File & place file pointer at the start of the file
    pFile = HFCBinStream::Instanciate(pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

    if (pFile == 0 || pFile->GetLastException() != 0)
        po_Width = po_Height = 0;
    else
        {
        m_FileSize = (size_t)pFile->GetSize();
        HASSERT(m_FileSize);

        FileSize = (size_t)((pFile->GetSize() - pi_Offset - pi_Footer) * 8 / pi_BitsPerPixel);
        HASSERT(FileSize);

        Size = (uint32_t)sqrt((double)FileSize);
        HASSERT(Size);


        while ((Size > 0) && (0 != (FileSize % Size)))
            Size--;

        if (Size == 0)
            po_Height = po_Width = (uint32_t)sqrt((double)FileSize);
        else
            {
            po_Height = Size;

            HASSERT_X64((FileSize / Size) < ULONG_MAX);
            po_Width = (uint32_t)(FileSize / Size);
            }
        }

    SisterFileLock.ReleaseKey();
    HASSERT(!(const_cast<HRFRawCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFRawCreator*>(this))->m_pSharingControl = 0;

    }

//-----------------------------------------------------------------------------
// Return an approximation of the pixel type of the image based on it size.
//-----------------------------------------------------------------------------
HFCPtr<HRPPixelType> HRFRawCreator::AutoDetectPixelType (const HFCPtr<HFCURL>& pi_rpURL,
                                                         uint64_t             pi_Offset,
                                                         uint32_t              pi_Footer,
                                                         uint32_t              pi_Width,
                                                         uint32_t              pi_Height)
    {
    HASSERT (pi_Width != 0);
    HASSERT (pi_Height != 0);

    size_t                  FileSize;
    HAutoPtr<HFCBinStream>  pFile;
    HFCPtr<HRPPixelType>    pPixelType = 0;

    (const_cast<HRFRawCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock(GetLockManager());

    // Open the Raw File & place file pointer at the start of the file
    pFile = HFCBinStream::Instanciate(pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

    if (pFile != 0 && pFile->GetLastException() == 0)
        {
        FileSize = (size_t)(pFile->GetSize() - pi_Offset - pi_Footer);
        FileSize /= pi_Width * pi_Height;

        switch (FileSize)
            {
            case 0 :
                pPixelType = new HRPPixelTypeV1Gray1();
                break;
            case 1 :
                pPixelType = new HRPPixelTypeV8Gray8();
                break;
            case 2 :
                pPixelType = new HRPPixelTypeV16PRGray8A8();
                break;
            case 3 :
                pPixelType = new HRPPixelTypeV24R8G8B8();
                break;
            default:
                pPixelType = new HRPPixelTypeV32R8G8B8A8();
                break;
            }
        }

    SisterFileLock.ReleaseKey();
    HASSERT(!(const_cast<HRFRawCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFRawCreator*>(this))->m_pSharingControl = 0;

    m_pPixelType = pPixelType;

    return pPixelType;
    }

//-----------------------------------------------------------------------------
// Protected
// Open
// This method open the file
//-----------------------------------------------------------------------------
bool HRFRawFile::Open()
    {
    if (!m_IsOpen)
        {
        m_pRawFile = HFCBinStream::Instanciate(GetURL(), m_Offset, GetAccessMode(), 0, true);

        // This creates the sister file for file sharing control if necessary.
        SharingControlCreate();

        m_IsOpen = true;
        }

    return true;
    }

//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors ()
// Create Raw File Descriptors
//-----------------------------------------------------------------------------
void HRFRawFile::CreateDescriptors ()
    {
    HPRECONDITION (m_pRawFile != 0);
    HPRECONDITION (m_pPixelType != 0);
    HPRECONDITION (m_ImageWidth > 0);
    HPRECONDITION (m_ImageHeight > 0);

    // Create Page and resolution Description/Capabilities for this file.
    HFCPtr<HRFResolutionDescriptor>         pResolution;
    HFCPtr<HRFPageDescriptor>               pPage;

    pResolution =  new HRFResolutionDescriptor(
        GetAccessMode(),                                // AccessMode,
        GetCapabilities(),                              // Capabilities,
        1.0,                                            // XResolutionRatio,
        1.0,                                            // YResolutionRatio,
        m_pPixelType,                                   // PixelType,
        new HCDCodecIdentity(),                         // Codec,
        HRFBlockAccess::RANDOM,                         // RBlockAccess,
        HRFBlockAccess::RANDOM,                         // WBlockAccess,
        HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL,  // ScanLineOrientation,
        HRFInterleaveType::PIXEL,                       // InterleaveType
        false,                                          // IsInterlace,
        m_ImageWidth,                                   // Width image ,
        m_ImageHeight,                                  // Height image,
        m_ImageWidth,                                   // BlockWidth,
        1,                                              // BlockHeight,
        0,                                              // BlocksDataFlag
        HRFBlockType::LINE);


    pPage = new HRFPageDescriptor (GetAccessMode(),
                                   GetCapabilities(),                       // Capabilities,
                                   pResolution,                             // ResolutionDescriptor,
                                   0,                                       // RepresentativePalette,
                                   0,                                       // Histogram,
                                   0,                                       // Thumbnail,
                                   0,                                       // ClipShape,
                                   0,                                       // TransfoModel,
                                   0,                                       // Filters
                                   0);                                      // Attribute set


    m_ListOfPageDescriptor.push_back(pPage);
    }


//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFRawFile::HRFRawFile(const HFCPtr<HFCURL>& pi_rURL,
                              uint32_t              pi_Width,
                              uint32_t              pi_Height,
                              HFCPtr<HRPPixelType>  pi_pPixelType,
                              HFCAccessMode         pi_AccessMode,
                              uint64_t             pi_Offset)

    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor store the access mode
    m_IsOpen        = false;
    m_ImageWidth    = pi_Width;
    m_ImageHeight   = pi_Height;
    m_pPixelType    = pi_pPixelType;

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
HRFRawFile::HRFRawFile(const HFCPtr<HFCURL>& pi_rURL,
                              HFCAccessMode         pi_AccessMode,
                              uint64_t             pi_Offset,
                              bool                 pi_DontOpenFile)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor store the access mode
    m_IsOpen     = false;
    }


//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HRFRawFile::~HRFRawFile()
    {
    // Be sure that the file is already open and that at least one page
    // has been add. Because if the destroyer is call afer a exception
    // is thrown, we want to be sure that the object is valid before we
    // execute the destroyer.
    if (m_IsOpen && m_ListOfPageDescriptor.size() > 0)
        m_IsOpen = false;
    }

//-----------------------------------------------------------------------------
// Public
// CreateResolutionEditor
// File manipulation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFRawFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                        unsigned short pi_Resolution,
                                                        HFCAccessMode  pi_AccessMode)
    {
    HRFResolutionEditor* pEditor = 0;

    pEditor = new HRFRawLineEditor(this, pi_Page, pi_Resolution, pi_AccessMode);

    return pEditor;
    }

//-----------------------------------------------------------------------------
// Public
// Save
// Saves the file
//-----------------------------------------------------------------------------
void HRFRawFile::Save()
    {

    //Nothing to do here
    }

//-----------------------------------------------------------------------------
// Public
// AddPage
// File manipulation
//-----------------------------------------------------------------------------
bool HRFRawFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    // Assert that no page has allready be entered
    HPRECONDITION (CountPages() == 0);
    HPRECONDITION (pi_pPage != 0);


    // Add the page descriptor to the list
    HRFRasterFile::AddPage(pi_pPage);


    // Create a space at the beginning of the file to add an header
    if (m_Offset != 0)
        {
        Byte* pTempon = new Byte[(size_t)m_Offset];
        memset(pTempon,0x00,(size_t)m_Offset);

        // Lock the sister file.
        HFCLockMonitor SisterFileLock (GetLockManager());

        m_pRawFile->SeekToBegin();
        m_pRawFile->Write(pTempon, (size_t)m_Offset);

        // Unlock the sister file.
        SisterFileLock.ReleaseKey();

        delete[] pTempon;
        }
    return true;
    }

//-----------------------------------------------------------------------------
// Private
// Create
// This method create the file.
//-----------------------------------------------------------------------------
bool HRFRawFile::Create()
    {
    // Open the file
    m_pRawFile = HFCBinStream::Instanciate(GetURL(), GetAccessMode(), 0, true);

    // Create the sharing control object for file sharing
    SharingControlCreate();

    m_IsOpen = true;

    return true;
    }

//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Return the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFRawFile::GetCapabilities () const
    {
    return (HRFRawCreator::GetInstance()->GetCapabilities());
    }


//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
// File information
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFRawFile::GetWorldIdentificator () const
    {
    return HGF2DWorld_UNKNOWNWORLD;
    }

//-----------------------------------------------------------------------------
// Protected
// GetFilePtr
// Get the Raw file pointer.
//-----------------------------------------------------------------------------
HFCBinStream* HRFRawFile::GetFilePtr  () const
    {
    return (m_pRawFile);
    }
