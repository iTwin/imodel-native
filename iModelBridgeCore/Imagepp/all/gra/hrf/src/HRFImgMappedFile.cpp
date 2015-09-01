//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFImgMappedFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>
    //:> must be first for PreCompiledHeader Option

#include <Imagepp/all/h/HRFImgMappedFile.h>

#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCBinStream.h>
#include <Imagepp/all/h/HRFException.h>
#include <Imagepp/all/h/HRFImgMappedLineEditor.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HRPPixelTypeFactory.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPChannelOrgRGB.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>


/** ---------------------------------------------------------------------------
    Block capabilities of the ImgMapped file format.
    ImgMapped file format only supports line capability with random access.
    ---------------------------------------------------------------------------
 */
class HRFImgMappedBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    //:> Constructor
    HRFImgMappedBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Line Capability
        Add(new HRFLineCapability(HFC_READ_WRITE_CREATE,        // AccessMode
                                  9999,                         // MaxWidth in bytes (dimensions cannot exceed 9999 by 9999)
                                  HRFBlockAccess::RANDOM));     // BlockAccess
        }
    };

/** ---------------------------------------------------------------------------
    Codec capabilities of the ImgMapped file format.
    ImgMapped file format does not support any compression.
    ---------------------------------------------------------------------------
 */
class HRFImgMappedCodecCapabilities : public  HRFRasterFileCapabilities
    {
public :
    //:> Constructor
    HRFImgMappedCodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec
        Add (new HRFCodecCapability (HFC_READ_WRITE_CREATE,
                                     HCDCodecIdentity::CLASS_ID,
                                     new HRFImgMappedBlockCapabilities()));
        }
    };

/** ---------------------------------------------------------------------------
    Constructor.
    ---------------------------------------------------------------------------
 */
HRFImgMappedCapabilities::HRFImgMappedCapabilities()
    : HRFRasterFileCapabilities()
    {
    // PixelTypeI8R8G8B8
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeI8R8G8B8::CLASS_ID,
                                   new HRFImgMappedCodecCapabilities()));
    // PixelTypeV8Gray8
    // Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV8Gray8::CLASS_ID,
                                   new HRFImgMappedCodecCapabilities()));

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));

    // Single resolution capability
    Add(new HRFSingleResolutionCapability(HFC_READ_WRITE_CREATE));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_WRITE_CREATE, HRFInterleaveType::PIXEL));

    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_WRITE_CREATE));
    }


HFC_IMPLEMENT_SINGLETON(HRFImgMappedCreator)


/** ---------------------------------------------------------------------------
    Constructor.
    Creator.
    ---------------------------------------------------------------------------
 */
HRFImgMappedCreator::HRFImgMappedCreator()
    : HRFRasterFileCreator(HRFImgMappedFile::CLASS_ID)
    {
    // Capabilities instance member initialization
    m_pCapabilities = 0;
    }

/** ---------------------------------------------------------------------------
    Return file format label.

    @return string ImgMapped file format label.
    ---------------------------------------------------------------------------
 */
WString HRFImgMappedCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_ImgMapped()); //ImgMapped File Format
    }

/** ---------------------------------------------------------------------------
    Return file format scheme.

    @return string scheme of URL.
    ---------------------------------------------------------------------------
 */
WString HRFImgMappedCreator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

/** ---------------------------------------------------------------------------
    Return file format extension.

    @return string ImgMapped extension.
    ---------------------------------------------------------------------------
 */
WString HRFImgMappedCreator::GetExtensions() const
    {
    return WString(L"*.p");
    }

/** ---------------------------------------------------------------------------
    Open/Create ImgMapped raster file.

    @param pi_rpURL      File's URL.
    @param pi_AccessMode Access and sharing mode.
    @param pi_Offset     Starting point in the file.

    @return HFCPtr<HRFRasterFile> Address of the created HRFRasterFile instance.
    ---------------------------------------------------------------------------
 */
HFCPtr<HRFRasterFile> HRFImgMappedCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                                  HFCAccessMode         pi_AccessMode,
                                                  uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    HFCPtr<HRFRasterFile> pFile = new HRFImgMappedFile(pi_rpURL, pi_AccessMode, pi_Offset);

    HASSERT(pFile != 0);

    return (pFile);
    }

/** ---------------------------------------------------------------------------
    Verify file validity.

    @param pi_rpURL      File's URL.
    @param pi_Offset     Starting point in the file.

    @return true if the file is a valid ImgMapped file, false otherwise.
    ---------------------------------------------------------------------------
 */
bool HRFImgMappedCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                        uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    HAutoPtr<HFCBinStream>  pFile;

    char                   aBuffer[10]; // File read access buffer
    char*                  pBuffer;
    uint32_t                 VersionNo;
    uint32_t                HeaderSize;
    uint32_t                 ColorCount;
    uint32_t                 Width;
    uint32_t                 Height;
    bool                   Ret = false;

    (const_cast<HRFImgMappedCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock (GetLockManager());

    pFile = HFCBinStream::Instanciate(pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

    // Note: Extensive use of return here to keep code light
    if (pFile == 0 || pFile->GetLastException() != 0)
        goto WRAPUP;

    // File identification
    if ((pFile->Read(aBuffer, 8) != 8))
        goto WRAPUP;

    if ((aBuffer[0] != 'S') || (aBuffer[1] != 'C') || (aBuffer[2] != 'M') || (aBuffer[3] != 'I'))
        goto WRAPUP;

    // Version
    pBuffer = &aBuffer[4];

    if (sscanf(pBuffer, "%u", &VersionNo) != 1)
        goto WRAPUP;

    // We only support version number 1
    if (VersionNo != 1)
        goto WRAPUP;

    // File header
    if ((pFile->Read(aBuffer, 10) != 10))
        goto WRAPUP;

    if ((aBuffer[0] != 'A') || (aBuffer[1] != 'T'))
        goto WRAPUP;

    // ... header size
    pBuffer = &aBuffer[2];

    if (sscanf(pBuffer, "%lu", &HeaderSize) != 1)
        goto WRAPUP;

    // Dimensions and color space
    // Width
    if ((pFile->Read(aBuffer, 4) != 4))
        goto WRAPUP;

    if (sscanf(aBuffer, "%u", &Width) != 1)
        goto WRAPUP;

    // Height
    if ((pFile->Read(aBuffer, 4) != 4))
        goto WRAPUP;

    if (sscanf(aBuffer, "%u", &Height) != 1)
        goto WRAPUP;

    // Color space
    if ((pFile->Read(aBuffer, 4) != 4))
        goto WRAPUP;

    if ( (sscanf(aBuffer, "%u", &ColorCount) != 1) || (ColorCount>256) )
        goto WRAPUP;

    Ret = true;

WRAPUP:
    SisterFileLock.ReleaseKey();
    HASSERT(!(const_cast<HRFImgMappedCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFImgMappedCreator*>(this))->m_pSharingControl = 0;

    return Ret;
    }


/** ---------------------------------------------------------------------------
    Get capabilities of ImgMapped file format.

    @return ImgMapped format capabilities.
    ---------------------------------------------------------------------------
 */
const HFCPtr<HRFRasterFileCapabilities>& HRFImgMappedCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFImgMappedCapabilities();

    return m_pCapabilities;
    }


/** ---------------------------------------------------------------------------
    Constructor.
    Open/Create ImgMapped raster file.

    @param pi_rURL       File's URL.
    @param pi_AccessMode Access and sharing mode.
    @param pi_Offset     Starting point in the file.
    ---------------------------------------------------------------------------
 */
HRFImgMappedFile::HRFImgMappedFile(const HFCPtr<HFCURL>& pi_rpURL,
                                   HFCAccessMode         pi_AccessMode,
                                   uint64_t             pi_Offset)
    : HRFRasterFile(pi_rpURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor stores the access mode
    m_IsOpen = false;

    if (GetAccessMode().m_HasCreateAccess)
        Create();
    else
        {
        // Create Page and Res Descriptors.
        Open();
        CreateDescriptors();
        }
    }

/** ---------------------------------------------------------------------------
    Constructor.
    Create ImgMapped raster file without open.

      @param pi_rURL         File's URL.
    @param pi_AccessMode   Access and sharing mode.
    @param pi_Offset       Starting point in the file.
    @param pi_DontOpenFile Specify to not open file.
    ---------------------------------------------------------------------------
 */
HRFImgMappedFile::HRFImgMappedFile(const HFCPtr<HFCURL>& pi_rpURL,
                                   HFCAccessMode         pi_AccessMode,
                                   uint64_t             pi_Offset,
                                   bool                 pi_DontOpenFile)
    : HRFRasterFile(pi_rpURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor store the access mode
    m_IsOpen = false;
    }

/** ---------------------------------------------------------------------------
    Destructor.
    Destroy ImgMapped file object.
    ---------------------------------------------------------------------------
 */
HRFImgMappedFile::~HRFImgMappedFile()
    {
    try
        {
        // Close ImgMapped file
        SaveImgMappedFile(true);

        // Delete file if access is "Create" but no page has been added
        if ((GetAccessMode().m_HasCreateAccess) && (CountPages() < 1))
            {
            m_pImgMappedFile = 0;

            // Delete file
            HASSERT(GetURL()->IsCompatibleWith(HFCURLFile::CLASS_ID));
            
            BeFileName::BeDeleteFile(static_cast<HFCURLFile*>(GetURL().GetPtr())->GetAbsoluteFileName().c_str());
            }
        }
    catch(...)
        {
        // Simply stop exceptions in the destructor
        // We want to known if a exception is throw
        HASSERT(0);
        }
    }

/** ---------------------------------------------------------------------------
    Create ImgMapped line editor for data manipulation.

       @param pi_Page         Page to create an editor for (zero only).
    @param pi_Resolution   Resolution (zero).
    @param pi_AccessMode   Access and sharing mode.
    ---------------------------------------------------------------------------
 */
HRFResolutionEditor* HRFImgMappedFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                              unsigned short pi_Resolution,
                                                              HFCAccessMode  pi_AccessMode)
    {
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    return new HRFImgMappedLineEditor(this, pi_Page, pi_Resolution, pi_AccessMode);
    }

/** ---------------------------------------------------------------------------
GetFileCurrentSize
Get the file's current size.
---------------------------------------------------------------------------
*/
uint64_t HRFImgMappedFile::GetFileCurrentSize() const
    {
    return HRFRasterFile::GetFileCurrentSize(m_pImgMappedFile);
    }


/** ---------------------------------------------------------------------------
    Save
    Saves the file
    ---------------------------------------------------------------------------
*/
void HRFImgMappedFile::Save()
    {
    //Keep last file position
    uint64_t CurrentPos = m_pImgMappedFile->GetCurrentPos();

    SaveImgMappedFile(false);

    //Set back position
    m_pImgMappedFile->SeekToPos(CurrentPos);
    }
/** ---------------------------------------------------------------------------
    Add a new page for the file (creation mode).
    (ImgMapped format only has one page)

    @param pi_pPage  Page descriptor to add.

    @return bool true if the page has been added.
    ---------------------------------------------------------------------------
 */
bool HRFImgMappedFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    // Validation if it's possible to add a page
    HPRECONDITION(CountPages() == 0);
    HPRECONDITION(pi_pPage != 0);

    bool Result  = true;

    // Cannot create a file with dimensions above 9999 (width or height)
    if ((pi_pPage->GetResolutionDescriptor(0)->GetWidth()  > 9999) ||
        (pi_pPage->GetResolutionDescriptor(0)->GetHeight() > 9999))
        {
        throw HRFInvalidNewFileDimensionException(GetURL()->GetURL(),
                                                  9999,
                                                  9999);
        }
    else
        {
        // Add the page descriptor to the list
        if (HRFRasterFile::AddPage(pi_pPage))
            {
            // Lock the sister file
            HFCLockMonitor SisterFileLock(GetLockManager());

            // Write new header to file
            WriteFileHeader();

            // Unlock the sister file
            SisterFileLock.ReleaseKey();
            }
        else
            Result = false;
        }

    return Result;
    }

/** ---------------------------------------------------------------------------
    Initialize file info struct from resolution descriptor.
    ---------------------------------------------------------------------------
 */
void HRFImgMappedFile::InitFileInfoFromDescriptors()
    {
    HFCPtr<HRFResolutionDescriptor> pResolutionDescriptor = GetPageDescriptor(0)->GetResolutionDescriptor(0);

    // Init dimensions info
    m_FileInfo.m_Width  = (uint32_t)pResolutionDescriptor->GetWidth();
    m_FileInfo.m_Height = (uint32_t)pResolutionDescriptor->GetHeight();

    // Init color count (always 256 colors in Create mode)
    //(HRPPixelTypeI8R8G8B8 or HRPPixelTypeV8Gray8)
    m_FileInfo.m_ColorCount = 256;

    // Init palette
    if (pResolutionDescriptor->GetPixelType()->IsCompatibleWith(HRPPixelTypeV8Gray8::CLASS_ID))
        {
        // Create a grayscale palette.
        for (uint32_t Index = 0; Index < 256 ; ++Index)
            memset(m_FileInfo.m_aColorMap + (3*Index), (Byte)Index, 3);
        }
    else // HRPPixelTypeI8R8G8B8
        {
        // Get palette from pixel type
        const HRPPixelPalette& rPalette = pResolutionDescriptor->GetPixelType()->GetPalette();

        // Fill color map
        for (uint32_t Index = 0, StepBy3 = 0; Index < rPalette.GetMaxEntries(); ++Index, StepBy3 += 3)
            {
            if (Index < rPalette.CountUsedEntries())
                memcpy (&(m_FileInfo.m_aColorMap[StepBy3]), (Byte*)rPalette.GetCompositeValue(Index), 3);
            else
                memset(&(m_FileInfo.m_aColorMap[StepBy3]), (Byte)Index, 3);
            }
        }
    }

/** ---------------------------------------------------------------------------
    Get capabilities of ImgMapped file format.

    @return ImgMapped format capabilities.
    ---------------------------------------------------------------------------
 */
const HFCPtr<HRFRasterFileCapabilities>& HRFImgMappedFile::GetCapabilities () const
    {
    return (HRFImgMappedCreator::GetInstance()->GetCapabilities());
    }

/** ---------------------------------------------------------------------------
    Open file, read header, keep pointer.

    @return true if file exist and has been opened.
    ---------------------------------------------------------------------------
 */
bool HRFImgMappedFile::Open()
    {
    // Open the file
    if (!m_IsOpen)
        {
        // Open main file
        m_pImgMappedFile = HFCBinStream::Instanciate(GetURL(), m_Offset, GetAccessMode(), 0, true);

        // Creating the sister file for sharing control
        SharingControlCreate();

        // Lock the sister file
        HFCLockMonitor SisterFileLock(GetLockManager());

        // Get file info
        ReadFileHeader();

        // Unlock the sister file.
        SisterFileLock.ReleaseKey();

        m_IsOpen = true;
        }

    return true;
    }

/** ---------------------------------------------------------------------------
    Create resolution and page descriptors based on format properties and from
    file header.
    ---------------------------------------------------------------------------
 */
void HRFImgMappedFile::CreateDescriptors ()
    {
    HFCPtr<HRFResolutionDescriptor> pResolution;
    HFCPtr<HRFPageDescriptor>       pPage;

    // Create resolution descriptor
    pResolution =  new HRFResolutionDescriptor(GetAccessMode(),                                // AccessMode,
                                               GetCapabilities(),                              // Capabilities,
                                               1.0,                                            // XResolutionRatio,
                                               1.0,                                            // YResolutionRatio,
                                               CreatePixelTypeFromFile(),                      // PixelType,
                                               new HCDCodecIdentity(),                         // Codec,
                                               HRFBlockAccess::RANDOM,                         // RBlockAccess,
                                               HRFBlockAccess::RANDOM,                         // WBlockAccess,
                                               HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL,  // ScanLineOrientation,
                                               HRFInterleaveType::PIXEL,                       // InterleaveType
                                               false,                                          // IsInterlace,
                                               m_FileInfo.m_Width,                             // Width,
                                               m_FileInfo.m_Height,                            // Height,
                                               m_FileInfo.m_Width,                             // BlockWidth,
                                               1,                                               // BlockHeight,
                                               0,                                              // BlocksDataFlag
                                               HRFBlockType::LINE);

    // Create page descriptor
    pPage = new HRFPageDescriptor (GetAccessMode(),     // Access Mode
                                   GetCapabilities(),   // Capabilities,
                                   pResolution,         // ResolutionDescriptor,
                                   0,                   // RepresentativePalette,
                                   0,                   // Histogram,
                                   0,                   // Thumbnail,
                                   0,                   // ClipShape,
                                   0,                   // TransfoModel,
                                   0);                  // Filters

    m_ListOfPageDescriptor.push_back(pPage);
    }

/** ---------------------------------------------------------------------------
    Create the appropriate pixel type according to the file header.

    @return HFCPtr<HRPPixelType> Appropriate created pixel type.
    ---------------------------------------------------------------------------
 */
HFCPtr<HRPPixelType> HRFImgMappedFile::CreatePixelTypeFromFile() const
    {
    HFCPtr<HRPPixelType>    pPixelType;

    // Image always "seen" as a I8R8G8B8
    pPixelType = HRPPixelTypeFactory::GetInstance()->Create(HRPChannelOrgRGB(8,
                                                                             8,
                                                                             8,
                                                                             0,
                                                                             HRPChannelType::UNUSED,
                                                                             HRPChannelType::VOID_CH,
                                                                             0),
                                                            8);

    // Set palette to pixel type
    HRPPixelPalette& rPalette = pPixelType->LockPalette();

    // Set the color palette from the file
    for (uint32_t Index = 0, StepBy3 = 0; Index < m_FileInfo.m_ColorCount; Index++, StepBy3 += 3)
        rPalette.SetCompositeValue(Index, &(m_FileInfo.m_aColorMap[StepBy3]));

    pPixelType->UnlockPalette();

    return pPixelType;
    }

/** ---------------------------------------------------------------------------
    Close file.
    ---------------------------------------------------------------------------
 */
void HRFImgMappedFile::SaveImgMappedFile(bool pi_CloseFile)
    {
    // Be sure that the file is already open and that at least one page
    // has been add. Because if the destroyer is call afer a exception
    // was thrown, we want to be sure that the object is valid before we
    // execute the destroyer.
    if (m_IsOpen && m_ListOfPageDescriptor.size() > 0)
        {

        HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(0);

        // if Create mode or the header is changed
        if (GetAccessMode().m_HasCreateAccess)
            {
            // Lock the sister file
            HFCLockMonitor SisterFileLock(GetLockManager());

            WriteFileHeader();

            m_pImgMappedFile->Flush();

            // Unlock the sister file
            SisterFileLock.ReleaseKey();
            }

        pPageDescriptor->Saved();

        if(pi_CloseFile)
            m_IsOpen = false;

        }
    }

/** ---------------------------------------------------------------------------
    Create main file.

      @return true if file has been created.
    ---------------------------------------------------------------------------
 */
bool HRFImgMappedFile::Create()
    {
    HPRECONDITION(!m_IsOpen);
    HPRECONDITION(!m_pImgMappedFile);

    bool Result = false;

    if (!m_IsOpen)
        {
        // Open file
        m_pImgMappedFile = HFCBinStream::Instanciate(GetURL(), m_Offset, GetAccessMode(), 0, true);

        // Creation of the sister file
        SharingControlCreate();

        m_IsOpen = true;

        Result = true;
        }

    return Result;
    }

/** ---------------------------------------------------------------------------
    Read header from file and initialize file info structure.
    ---------------------------------------------------------------------------
 */
void HRFImgMappedFile::ReadFileHeader()
    {
    HPRECONDITION(m_pImgMappedFile != 0);
    HPRECONDITION(SharingControlIsLocked());

    char  aBuffer[11]; // File read access buffer
    char* pBuffer;

    uint32_t SectionSize;
    uint32_t BytesToStepOver;

    // Prepare to read file
    m_pImgMappedFile->SeekToPos(0);

    // 1-
    // File identification section (skip)
    m_pImgMappedFile->Read(aBuffer, 8);

    // 2-
    // Attribute section
    memset(aBuffer, 0, 11);
    m_pImgMappedFile->Read(aBuffer, 10);
    pBuffer = &aBuffer[2];

    // Read section size
    sscanf (pBuffer, "%8u", &SectionSize);

    // Read attibutes
    memset(aBuffer, 0, 11);
    m_pImgMappedFile->Read(aBuffer, 4);
    sscanf (aBuffer, "%4u", &m_FileInfo.m_Width);

    memset(aBuffer, 0, 11);
    m_pImgMappedFile->Read(aBuffer, 4);
    sscanf (aBuffer, "%4u", &m_FileInfo.m_Height);

    memset(aBuffer, 0, 11);
    m_pImgMappedFile->Read(aBuffer, 4);
    sscanf (aBuffer, "%4u", &m_FileInfo.m_ColorCount);

    // If there is still some unknown info to read... step over
    if ((BytesToStepOver = SectionSize - 12) > 0)
        {
        // Simply bypass unknown info
        m_pImgMappedFile->SeekToPos(m_pImgMappedFile->GetCurrentPos() + BytesToStepOver);
        }

    // 3-
    // Color map section
    // Init color map
    memset(m_FileInfo.m_aColorMap, 0, 256*3);

    // Read section header
    memset(aBuffer, 0, 11);
    m_pImgMappedFile->Read(aBuffer, 10);
    pBuffer = &aBuffer[2];

    // Read section size
    sscanf (pBuffer, "%8u", &SectionSize);

    // (Prevent memory destruction that could be caused by an error in file)
    if (SectionSize > 256*3)
        SectionSize = 256*3;

    // Fill color map
    m_pImgMappedFile->Read(m_FileInfo.m_aColorMap, SectionSize);

    // 4-
    // Pixel data section
    m_pImgMappedFile->Read(aBuffer, 10);

    // Set offset to pixel data value
    m_OffsetToPixelData = (uint32_t)m_pImgMappedFile->GetCurrentPos();
    }

/** ---------------------------------------------------------------------------
    Write header to file.
    ---------------------------------------------------------------------------
 */
void HRFImgMappedFile::WriteFileHeader()
    {
    HPRECONDITION(m_pImgMappedFile != 0);
    HPRECONDITION(SharingControlIsLocked());

    char  aBuffer[13]; // File write access buffer
    char* pBuffer;

    // Be consistant with page and resolution descriptor
    InitFileInfoFromDescriptors();

    // Prepare to write to file
    m_pImgMappedFile->SeekToPos(0);

    // 1-
    // File identification section
    memset(aBuffer, ' ', 8);

    // Version number
    sprintf (aBuffer, "SCMI%4u", 1 /*version 1*/);

    // Write to file
    m_pImgMappedFile->Write(aBuffer, 8);

    // 2-
    // Attribute section
    memset(aBuffer, ' ', 10);

    // Section size
    sprintf (aBuffer, "AT%8u", 12 /*section is 12 bytes long*/);

    // Write to file
    m_pImgMappedFile->Write(aBuffer, 10);

    memset(aBuffer, ' ', 12);

    // Section info
    sprintf (aBuffer, "%4u", m_FileInfo.m_Width /*image width*/);

    pBuffer = &aBuffer[4];
    sprintf (pBuffer, "%4u", m_FileInfo.m_Height /*image height*/);

    pBuffer = &aBuffer[8];
    sprintf (pBuffer, "%4u", 256 /*number of colors*/);

    // Write to file
    m_pImgMappedFile->Write(aBuffer, 12);

    // 3-
    // Color map section
    memset(aBuffer, ' ', 10);

    // Section size
    sprintf (aBuffer, "CM%8u", 3 * 256 /*space for palette*/);

    // Write to file
    m_pImgMappedFile->Write(aBuffer, 10);

    // Write color map to file
    m_pImgMappedFile->Write(m_FileInfo.m_aColorMap, 3*256);

    // 4-
    // Pixel Data section
    memset(aBuffer, ' ', 10);

    // Image size
    sprintf (aBuffer, "PD%8u", m_FileInfo.m_Width * m_FileInfo.m_Height /*space for image data*/);

    // Write to file
    m_pImgMappedFile->Write(aBuffer, 10);

    // Set offset to pixel data value
    m_OffsetToPixelData = (uint32_t)m_pImgMappedFile->GetCurrentPos();
    }

/** ---------------------------------------------------------------------------
    Get world identificator of ImgMapped file format.
    In this case, always upper-left origin, so it is an "unknown world".

    @return HGF2DWorld_UNKNOWNWORLD.
    ---------------------------------------------------------------------------
 */
const HGF2DWorldIdentificator HRFImgMappedFile::GetWorldIdentificator () const
    {
    return HGF2DWorld_UNKNOWNWORLD;
    }
