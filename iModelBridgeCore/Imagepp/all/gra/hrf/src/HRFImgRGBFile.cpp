//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFImgRGBFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>
    //:> must be first for PreCompiledHeader Option

#include <Imagepp/all/h/HRFImgRGBFile.h>

#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCBinStream.h>
#include <Imagepp/all/h/HFCStat.h>

#include <Imagepp/all/h/HRFException.h>
#include <Imagepp/all/h/HRFImgRGBLineEditor.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>

/** ---------------------------------------------------------------------------
    Block capabilities of the ImgRGB file format.
    ImgRGB file format only supports line capability with random access.
    ---------------------------------------------------------------------------
 */
class HRFImgRGBBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    //:> Constructor
    HRFImgRGBBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Line Capability
        Add(new HRFLineCapability(HFC_READ_WRITE_CREATE,        //:> AccessMode
                                  9999*3,                       //:> MaxWidth in bytes (dimensions cannot exceed 9999 by 9999)
                                  HRFBlockAccess::RANDOM));     //:> BlockAccess
        }
    };

/** ---------------------------------------------------------------------------
    Codec capabilities of the ImgRGB file format.
    ImgRGB file format does not support any compression.
    ---------------------------------------------------------------------------
 */
class HRFImgRGBCodecCapabilities : public  HRFRasterFileCapabilities
    {
public :
    //:> Constructor
    HRFImgRGBCodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec
        Add (new HRFCodecCapability (HFC_READ_WRITE_CREATE,
                                     HCDCodecIdentity::CLASS_ID,
                                     new HRFImgRGBBlockCapabilities()));
        }
    };

/** ---------------------------------------------------------------------------
    Constructor.
    ---------------------------------------------------------------------------
 */
HRFImgRGBCapabilities::HRFImgRGBCapabilities()
    : HRFRasterFileCapabilities()
    {
    // PixelTypeV24R8G8B8
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV24R8G8B8::CLASS_ID,
                                   new HRFImgRGBCodecCapabilities()));

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_WRITE_CREATE, HRFInterleaveType::PIXEL));

    // Single resolution capability
    Add(new HRFSingleResolutionCapability(HFC_READ_WRITE_CREATE));

    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_WRITE_CREATE));
    }


HFC_IMPLEMENT_SINGLETON(HRFImgRGBCreator)


/** ---------------------------------------------------------------------------
    Constructor.
    Creator.
    ---------------------------------------------------------------------------
 */
HRFImgRGBCreator::HRFImgRGBCreator()
    : HRFRasterFileCreator(HRFImgRGBFile::CLASS_ID)
    {
    // Capabilities instance member initialization
    m_pCapabilities = 0;
    }

/** ---------------------------------------------------------------------------
    Return file format label.

    @return string ImgRGB file format label.
    ---------------------------------------------------------------------------
 */
WString HRFImgRGBCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_ImgRGB()); //ImgRGB File Format
    }

/** ---------------------------------------------------------------------------
    Return file format scheme.

    @return string scheme of URL.
    ---------------------------------------------------------------------------
 */
WString HRFImgRGBCreator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

/** ---------------------------------------------------------------------------
    Return file format extension.

    @return string ImgRGB extension.
    ---------------------------------------------------------------------------
 */
WString HRFImgRGBCreator::GetExtensions() const
    {
    return WString(L"*.a");
    }

/** ---------------------------------------------------------------------------
    Open/Create ImgRGB raster file.

    @param pi_rpURL      File's URL.
    @param pi_AccessMode Access and sharing mode.
    @param pi_Offset     Starting point in the file.

    @return HFCPtr<HRFRasterFile> Address of the created HRFRasterFile instance.
    ---------------------------------------------------------------------------
 */
HFCPtr<HRFRasterFile> HRFImgRGBCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                               HFCAccessMode         pi_AccessMode,
                                               uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    HFCPtr<HRFRasterFile> pFile = new HRFImgRGBFile(pi_rpURL, pi_AccessMode, pi_Offset);

    HASSERT(pFile != 0);

    return (pFile);
    }

/** ---------------------------------------------------------------------------
    Verify file validity.

    @param pi_rpURL      File's URL.
    @param pi_Offset     Starting point in the file.

    @return true if the file is a valid ImgRGB file, false otherwise.
    ---------------------------------------------------------------------------
 */
bool HRFImgRGBCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                     uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    if(!pi_rpURL->IsCompatibleWith(HFCURLFile::CLASS_ID))
        return false;

    HAutoPtr<HFCBinStream> pFile;

    bool                  Result  = false;
    char                  aBuffer[4];
    uint32_t                Width =0 ;
    uint32_t                Height = 0;
    HFCPtr<HFCURL>         pRedFileURL;
    HFCPtr<HFCURL>         pGreenFileURL;
    HFCPtr<HFCURL>         pBlueFileURL;

    (const_cast<HRFImgRGBCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock (GetLockManager());

    // Check if the file is ImgRGB or not.
    // Read 4 bytes 2 times and read 4 bytes again (blank). Header must be 12 bytes long.
    pFile = HFCBinStream::Instanciate(pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

    if (pFile == 0 || pFile->GetLastException() != 0)
        goto WRAPUP;

    // This test reduces the number of false RGB file, like some worldfiles.
    if (pFile->GetSize() != 12)
        goto WRAPUP;

    // Read .a file
    if (pFile->Read(aBuffer,  4) != 4 || sscanf(aBuffer, "%u", &Width) != 1  || Width == 0) // validate if it is in 1..9999 range
        goto WRAPUP;

    if (pFile->Read(aBuffer,  4) != 4 || sscanf(aBuffer, "%u", &Height) != 1 || Height == 0)// validate if it is in 1..9999 range
        goto WRAPUP;

    if (pFile->Read(aBuffer,  4) != 4)
        goto WRAPUP;

        {
        WString Path = ((HFCPtr<HFCURLFile>&)pi_rpURL)->GetPath();
        WString Extension;

        // Find the file extension
        WString::size_type DotPos = Path.rfind(L'.');

        if (DotPos != WString::npos)
            {
            Extension = Path.substr(DotPos+1, Extension.length() - DotPos - 1);
            Path      = Path.substr(0, DotPos);
            }

        CaseInsensitiveStringTools().ToLower(Extension);

        // For this format, extension MUST be ".a"
        if (SupportsExtension(Extension))
            {
            // Compose url for .r, .g and .b files
            WString FileName = WString(HFCURLFile::s_SchemeName() + L"://")
                               + ((HFCPtr<HFCURLFile>&)pi_rpURL)->GetHost() + WString(L"\\")
                               + Path;

            pRedFileURL   = new HFCURLFile(FileName + WString(L".r"));
            pGreenFileURL = new HFCURLFile(FileName + WString(L".g"));
            pBlueFileURL  = new HFCURLFile(FileName + WString(L".b"));

            // Check if .r, .g and .b files exist
            if (!HFCStat(pRedFileURL).IsExistent() ||
                !HFCStat(pGreenFileURL).IsExistent() ||
                !HFCStat(pBlueFileURL).IsExistent())
                goto WRAPUP;
            }
        }

    Result = true;

WRAPUP:
    SisterFileLock.ReleaseKey();
    HASSERT(!(const_cast<HRFImgRGBCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFImgRGBCreator*>(this))->m_pSharingControl = 0;

    return Result;
    }


/** ---------------------------------------------------------------------------
    Get list of related files from a given URL.

    @param pi_rpURL          Main file's URL.
    @param pio_rRelatedURLs  List of related URLs.

    @return true (current file format is multi-file).
    ---------------------------------------------------------------------------
 */
bool HRFImgRGBCreator::GetRelatedURLs(const HFCPtr<HFCURL>& pi_rpURL,
                                       ListOfRelatedURLs&    pio_rRelatedURLs) const
    {
    HASSERT (pio_rRelatedURLs.size() == 0);

    // Find the file extension
    WString Path = ((HFCPtr<HFCURLFile>&)pi_rpURL)->GetPath();

    WString::size_type DotPos = Path.rfind(L'.');

    if (DotPos != WString::npos)
        Path = Path.substr(0, DotPos);

    // Compose url for .r, .g and .b files
    WString FileName = WString(HFCURLFile::s_SchemeName() + L"://")
                       + ((HFCPtr<HFCURLFile>&)pi_rpURL)->GetHost() + WString(L"\\")
                       + Path;

    // Create related files
    HFCPtr<HFCURL> pRedFileURL   = new HFCURLFile(FileName + WString(L".r"));
    HFCPtr<HFCURL> pGreenFileURL = new HFCURLFile(FileName + WString(L".g"));
    HFCPtr<HFCURL> pBlueFileURL  = new HFCURLFile(FileName + WString(L".b"));

    pio_rRelatedURLs.push_back(pRedFileURL);
    pio_rRelatedURLs.push_back(pGreenFileURL);
    pio_rRelatedURLs.push_back(pBlueFileURL);

    return true;
    }


/** ---------------------------------------------------------------------------
    Get capabilities of ImgRGB file format.

    @return ImgRGB format capabilities.
    ---------------------------------------------------------------------------
 */
const HFCPtr<HRFRasterFileCapabilities>& HRFImgRGBCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFImgRGBCapabilities();

    return m_pCapabilities;
    }


/** ---------------------------------------------------------------------------
    Constructor.
    Open/Create ImgRGB raster file.

    @param pi_rURL       File's URL.
    @param pi_AccessMode Access and sharing mode.
    @param pi_Offset     Starting point in the file.
    ---------------------------------------------------------------------------
 */
HRFImgRGBFile::HRFImgRGBFile(const HFCPtr<HFCURL>& pi_rpURL,
                             HFCAccessMode         pi_AccessMode,
                             uint64_t             pi_Offset)
    : HRFRasterFile(pi_rpURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor stores the access mode
    m_IsOpen = false;

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

/** ---------------------------------------------------------------------------
    Constructor.
    Create ImgRGB raster file without opening it.

      @param pi_rURL         File's URL.
    @param pi_AccessMode   Access and sharing mode.
    @param pi_Offset       Starting point in the file.
    @param pi_DontOpenFile Specify to not open file.
    ---------------------------------------------------------------------------
 */
HRFImgRGBFile::HRFImgRGBFile(const HFCPtr<HFCURL>& pi_rpURL,
                             HFCAccessMode         pi_AccessMode,
                             uint64_t             pi_Offset,
                             bool                 pi_DontOpenFile)
    : HRFRasterFile(pi_rpURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor stores the access mode
    m_IsOpen = false;
    }

/** ---------------------------------------------------------------------------
    Destructor.
    Destroy ImgRGB file object.
    ---------------------------------------------------------------------------
 */
HRFImgRGBFile::~HRFImgRGBFile()
    {
    try
        {
        // Close the ImgRGB file
        SaveImgRGBFile(true);
        }
    catch(...)
        {
        // Simply stop exceptions in the destructor
        // We want to known if a exception is throw
        HASSERT(0);
        }

    // Delete files if access is "Create" but no page has been added
    if ((GetAccessMode().m_HasCreateAccess) && (CountPages() < 1))
        {
        m_pImgRGBFile = 0;
        m_pRedFile    = 0;
        m_pGreenFile  = 0;
        m_pBlueFile   = 0;

        // Delete all files
        BeFileName::BeDeleteFile(static_cast<HFCURLFile*>(GetURL().GetPtr())->GetAbsoluteFileName().c_str());
        BeFileName::BeDeleteFile(static_cast<HFCURLFile*>(m_pRedFileURL.GetPtr())->GetAbsoluteFileName().c_str());
        BeFileName::BeDeleteFile(static_cast<HFCURLFile*>(m_pGreenFileURL.GetPtr())->GetAbsoluteFileName().c_str());
        BeFileName::BeDeleteFile(static_cast<HFCURLFile*>(m_pBlueFileURL.GetPtr())->GetAbsoluteFileName().c_str());
        }
    }

/** ---------------------------------------------------------------------------
    Create ImgRGB line editor for data manipulation.

       @param pi_Page         Page to create an editor for (zero only supported).
    @param pi_Resolution   Resolution (zero).
    @param pi_AccessMode   Access and sharing mode.
    ---------------------------------------------------------------------------
 */
HRFResolutionEditor* HRFImgRGBFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                           unsigned short pi_Resolution,
                                                           HFCAccessMode  pi_AccessMode)
    {
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    return new HRFImgRGBLineEditor(this, pi_Page, pi_Resolution, pi_AccessMode);
    }

/** ---------------------------------------------------------------------------
    Saves the file
---------------------------------------------------------------------------
*/
void HRFImgRGBFile::Save()
    {

    //Keep last file position
    uint64_t CurrentPos = m_pImgRGBFile->GetCurrentPos();

    SaveImgRGBFile(false);

    //Set back position
    m_pImgRGBFile->SeekToPos(CurrentPos);
    }

/** ---------------------------------------------------------------------------
Get the file's current size.
---------------------------------------------------------------------------
*/
uint64_t HRFImgRGBFile::GetFileCurrentSize() const
    {
    return HRFRasterFile::GetFileCurrentSize(m_pImgRGBFile);
    }

/** ---------------------------------------------------------------------------
    Add a new page for the file (creation mode).
    (ImgRGB format only have one page)

    @param pi_pPage  Page descriptor to add.

    @return bool true if the page has been added.
    ---------------------------------------------------------------------------
 */
bool HRFImgRGBFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    // Validation if it's possible to add a page
    HPRECONDITION(CountPages() == 0);
    HPRECONDITION(pi_pPage != 0);

    bool Result  = true;

    HFCPtr<HRFResolutionDescriptor> pResolutionDescriptor = pi_pPage->GetResolutionDescriptor(0);

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
            // Set the image file information
            m_FileHeader.m_Width  = (uint32_t)pResolutionDescriptor->GetWidth();
            m_FileHeader.m_Height = (uint32_t)pResolutionDescriptor->GetHeight();

            // Lock the sister file.
            HFCLockMonitor SisterFileLock (GetLockManager());

            SetFileHeaderToFile();

            // Unlock the sister file.
            SisterFileLock.ReleaseKey();
            }
        else
            Result = false;
        }

    return Result;
    }

/** ---------------------------------------------------------------------------

    Get capabilities of ImgRGB file format.

    @return ImgRGB format capabilities.
    ---------------------------------------------------------------------------
 */
const HFCPtr<HRFRasterFileCapabilities>& HRFImgRGBFile::GetCapabilities () const
    {
    return (HRFImgRGBCreator::GetInstance()->GetCapabilities());
    }

/** ---------------------------------------------------------------------------
    Open files, read header, keep pointers (.a, .r, .g, .b).

    @return true if files exist and have been opened.
    ---------------------------------------------------------------------------
 */
bool HRFImgRGBFile::Open()
    {
    // Open the file
    if (!m_IsOpen)
        {
        try
            {
            // Get files ptrs
            OpenFiles();
            }
        catch(...)
            {
            // If one file couldn't be opened, close all others to assure object consistency
            m_pImgRGBFile = 0;
            m_pRedFile    = 0;
            m_pGreenFile  = 0;
            m_pBlueFile   = 0;

            throw; // propagate exception to caller.
            }

        // This creates the sister file for file sharing control if necessary.
        SharingControlCreate();

        // Lock the sister file.
        HFCLockMonitor SisterFileLock (GetLockManager());

        // Get width and height
        GetFileHeaderFromFile();

        // Unlock the sister file.
        SisterFileLock.ReleaseKey();

        m_IsOpen = true;
        }

    return true;
    }

/** ---------------------------------------------------------------------------
    Open main file and channels files.
    ---------------------------------------------------------------------------
 */
void HRFImgRGBFile::OpenFiles()
    {
    HPRECONDITION(GetURL()->IsCompatibleWith(HFCURLFile::CLASS_ID));

    // Open the file
    if (!m_IsOpen)
        {
        // Open main file
        m_pImgRGBFile = HFCBinStream::Instanciate(GetURL(), m_Offset, GetAccessMode(), 0, true);

        // Open 3 channel files
        WString Path(((HFCPtr<HFCURLFile>&)GetURL())->GetPath());

        // Find the file extension
        WString::size_type DotPos = Path.rfind(L'.');

        if (DotPos != WString::npos)
            Path = Path.substr(0, DotPos);

        // Compose url for .r, .g and .b files
        WString FileName = WString(HFCURLFile::s_SchemeName() + L"://")
                           + ((HFCPtr<HFCURLFile>&)GetURL())->GetHost() + WString(L"\\")
                           + Path;

        m_pRedFileURL   = new HFCURLFile(FileName + WString(L".r"));
        m_pGreenFileURL = new HFCURLFile(FileName + WString(L".g"));
        m_pBlueFileURL  = new HFCURLFile(FileName + WString(L".b"));

        m_pRedFile   = HFCBinStream::Instanciate(m_pRedFileURL, m_Offset, GetAccessMode(), 0, true);

        m_pGreenFile = HFCBinStream::Instanciate(m_pGreenFileURL, m_Offset, GetAccessMode(), 0, true);

        m_pBlueFile  = HFCBinStream::Instanciate(m_pBlueFileURL, m_Offset, GetAccessMode(), 0, true);

        HASSERT (m_ListOfRelatedURLs.size() == 0);
        m_ListOfRelatedURLs.push_back(m_pRedFileURL);
        m_ListOfRelatedURLs.push_back(m_pGreenFileURL);
        m_ListOfRelatedURLs.push_back(m_pBlueFileURL);
        }
    }

/** ---------------------------------------------------------------------------
    Create resolution and page descriptors based on format properties and from
    file header.
    ---------------------------------------------------------------------------
 */
void HRFImgRGBFile::CreateDescriptors ()
    {
    // Create Page and Resolution Description/Capabilities for this file
    HFCPtr<HRFResolutionDescriptor>     pResolution;
    HFCPtr<HRFPageDescriptor>           pPage;

    HFCPtr<HRPPixelType>  pPixelType = new HRPPixelTypeV24R8G8B8();


    pResolution =  new HRFResolutionDescriptor(GetAccessMode(),                                // AccessMode,
                                               GetCapabilities(),                              // Capabilities,
                                               1.0,                                            // XResolutionRatio,
                                               1.0,                                            // YResolutionRatio,
                                               pPixelType,                                     // PixelType,
                                               new HCDCodecIdentity(),                         // Codec,
                                               HRFBlockAccess::RANDOM,                         // RBlockAccess,
                                               HRFBlockAccess::RANDOM,                         // WBlockAccess,
                                               HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL,  // ScanLineOrientation,
                                               HRFInterleaveType::PIXEL,                       // InterleaveType
                                               false,                                          // IsInterlace,
                                               m_FileHeader.m_Width,                           // Width,
                                               m_FileHeader.m_Height,                          // Height,
                                               m_FileHeader.m_Width,                           // BlockWidth,
                                               1,                                               // BlockHeight,
                                               0,                                              // BlocksDataFlag
                                               HRFBlockType::LINE);


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
    Save the file and close it if needed
    ---------------------------------------------------------------------------
 */
void HRFImgRGBFile::SaveImgRGBFile(bool pi_CloseFile)
    {
    // Be sure that the file is already open and that at least one page
    // has been add. Because if the destroyer is call afer a exception
    // was thrown, we want to be sure that the object is valid before we
    // execute the destroyer.
    if (m_IsOpen && m_ListOfPageDescriptor.size() > 0)
        {

        // if Create mode or the header is changed
        if (GetAccessMode().m_HasCreateAccess)
            {
            // Lock the sister file
            HFCLockMonitor SisterFileLock(GetLockManager());

            SetFileHeaderToFile();

            m_pImgRGBFile->Flush();
            m_pRedFile->Flush();
            m_pGreenFile->Flush();
            m_pBlueFile->Flush();

            // Unlock the sister file.
            SisterFileLock.ReleaseKey();
            }

        if(pi_CloseFile)
            m_IsOpen = false;

        }
    }

/** ---------------------------------------------------------------------------
    Create main file and channels files.

      @return true if files have been created.
    ---------------------------------------------------------------------------
 */
bool HRFImgRGBFile::Create()
    {
    HPRECONDITION(!m_IsOpen);
    HPRECONDITION(!m_pImgRGBFile);

    try
        {
        // Create files (open with create access)
        OpenFiles();
        }
    catch(...)
        {
        // If one file couldn't be created, close all others to assure object consistency
        m_pImgRGBFile = 0;
        m_pRedFile    = 0;
        m_pGreenFile  = 0;
        m_pBlueFile   = 0;

        throw; // propagate exception to caller.
        }

    // This creates the sister file for file sharing control if necessary.
    SharingControlCreate();

    m_IsOpen = true;

    return true;
    }

/** ---------------------------------------------------------------------------
    Read file header from file and initialize file header structure.
    ---------------------------------------------------------------------------
 */
void HRFImgRGBFile::GetFileHeaderFromFile()
    {
    HPRECONDITION(m_pImgRGBFile != 0);
    HPRECONDITION(SharingControlIsLocked());

    char aBuffer[5];
    aBuffer[4] = 0; // put a null terminate string

    m_pImgRGBFile->SeekToPos(0);

    // Read Width
    m_pImgRGBFile->Read(aBuffer, 4);
    sscanf(aBuffer, "%u", &(m_FileHeader.m_Width));

    // Read Height
    m_pImgRGBFile->Read(aBuffer, 4);
    sscanf(aBuffer, "%u", &(m_FileHeader.m_Height));

    // Read <blank>
    m_pImgRGBFile->Read(aBuffer, 4);
    }

/** ---------------------------------------------------------------------------
    Write file header to file.
    ---------------------------------------------------------------------------
 */
void HRFImgRGBFile::SetFileHeaderToFile()
    {
    HPRECONDITION(m_pImgRGBFile != 0);
    HPRECONDITION(SharingControlIsLocked());

    char aBuffer[12];
    memset(aBuffer, ' ', 12);

    m_pImgRGBFile->SeekToPos(0);

    // Prepare to write bytes (4-digits format)
    sprintf (aBuffer, "%4u%4u", m_FileHeader.m_Width, m_FileHeader.m_Height);

    // Write to main file
    m_pImgRGBFile->Write(aBuffer, 12);
    }

/** ---------------------------------------------------------------------------
    Get the world identificator of the ImgRGB file format.
    In this case, always upper-left origin, so it is an "unknown world".

    @return HGF2DWorld_UNKNOWNWORLD.
    ---------------------------------------------------------------------------
 */
const HGF2DWorldIdentificator HRFImgRGBFile::GetWorldIdentificator () const
    {
    return HGF2DWorld_UNKNOWNWORLD;
    }
