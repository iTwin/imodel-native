//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFFliFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFSpotFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRFFliFile.h>

#include <Imagepp/all/h/HRFException.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCMemoryBinStream.h>
#include <Imagepp/all/h/HFCBuffer.h>

#include <Imagepp/all/h/HRPChannelOrgGray.h>
#include <Imagepp/all/h/HRPChannelOrgRGB.h>
#include <Imagepp/all/h/HRPHistogram.h>
#include <Imagepp/all/h/HRFFliCompressLineEditor.h>
#include <Imagepp/all/h/HRFFliLineEditor.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeFactory.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDCodecFLIRLE8.h>
#include <Imagepp/all/h/HTIFFUtils.h>
#include <Imagepp/all/h/HRFUtility.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>



#include <Imagepp/all/h/HRFRasterFileCapabilities.h>


//////////////////////////////////////////////////////////////////////////
#define FLIC_CEL_DATA       3       /*registration and transparency*/
#define FLIC_COLOR_256      4       /*256-level colour palette*/
#define FLIC_DELTA_FLC      7       /*(FLI_SS2)  delta image, word oriented RLE*/
#define FLIC_COLOR_64       11      /*64-level colour palette*/
#define FLIC_DELTA_FLI      12      /*(FLI_LC)   delta image, Byte oriented RLE*/
#define FLIC_BLACK          13      /*full black frame (rare)*/
#define FLIC_BRUN           15      /*BYTE_RUN ()  full image, Byte oriented RLE*/
#define FLIC_COPY           16      /*uncompressed image (rare)*/
#define FLIC_PSTAMP         18      /*postage stamp (icon of the first frame)*/
#define FLIC_DTA_BRUN       25      /*full image, pixel oriented RLE*/
#define FLIC_DTA_COPY       26      /*uncompressed image*/
#define FLIC_DTA_LC         27      /*delta image, pixel oriented RLE*/

//-----------------------------------------------------------------------------
// HRFFliBlockCompressCapabilities
//-----------------------------------------------------------------------------
class HRFFliBlockCompressCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFFliBlockCompressCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Line Capability
        Add(new HRFLineCapability(HFC_READ_ONLY,        // AccessMode
                                  LONG_MAX,                                           // MaxWidth
                                  HRFBlockAccess::SEQUENTIAL));                       // BlockAccess

        // Image Capability
        Add(new HRFImageCapability(HFC_READ_ONLY,  // AccessMode
                                   LONG_MAX,        // MaxSizeInBytes
                                   0,               // MinWidth
                                   LONG_MAX,        // MaxWidth
                                   0,               // MinHeight
                                   LONG_MAX));      // MaxHeight
        }
    };

class HRFFliBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFFliBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Line Capability
        Add(new HRFLineCapability(HFC_READ_ONLY,        // AccessMode
                                  LONG_MAX,                                           // MaxWidth
                                  HRFBlockAccess::SEQUENTIAL));                       // BlockAccess

        // Image Capability
        Add(new HRFImageCapability(HFC_READ_ONLY,  // AccessMode
                                   LONG_MAX,        // MaxSizeInBytes
                                   0,               // MinWidth
                                   LONG_MAX,        // MaxWidth
                                   0,               // MinHeight
                                   LONG_MAX));      // MaxHeight
        }
    };


//-----------------------------------------------------------------------------
// HRFFliCodecCapabilities
//-----------------------------------------------------------------------------
class HRFFliCodec8BitsCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFFliCodec8BitsCapabilities()
        : HRFRasterFileCapabilities()
        {
        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecFLIRLE8::CLASS_ID,
                                   new HRFFliBlockCompressCapabilities()));

        Add(new HRFCodecCapability(HFC_READ_ONLY,
                                   HCDCodecIdentity::CLASS_ID,
                                   new HRFFliBlockCapabilities()));
        }
    };

//-----------------------------------------------------------------------------
// HRFFilCapabilities
//-----------------------------------------------------------------------------
HRFFliCapabilities::HRFFliCapabilities()
    : HRFRasterFileCapabilities()
    {

    // PixelTypeI8R8G8B8
    // Read/Write/Create capabilities
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeI8R8G8B8::CLASS_ID,
                                   new HRFFliCodec8BitsCapabilities()));

    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_ONLY, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_ONLY, HRFInterleaveType::PIXEL));

    // Interlace capability
    Add(new HRFInterlaceCapability(HFC_READ_ONLY));

    // Single resolution capability
    Add(new HRFSingleResolutionCapability(HFC_READ_ONLY));

    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_ONLY));

    // Histogram capability
    Add(new HRFHistogramCapability(HFC_READ_ONLY));

    // Tag capability

    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeBackground(0)));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeTitle));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeArtist));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeImageDescription));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeCopyright));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeDateTime));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeSoftware));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeLegalDisclaimer));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeContentWarning));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeHostComputer));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeNotes));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeXResolution(0)));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeYResolution(0)));
    Add(new HRFTagCapability(HFC_READ_ONLY, new HRFAttributeResolutionUnit(0)));
    }

HFC_IMPLEMENT_SINGLETON(HRFFliCreator)

//-----------------------------------------------------------------------------
// Constructor
// Public (HRFFliCreator)
// This is the creator to instantiate FLI/FLIC format
//-----------------------------------------------------------------------------
HRFFliCreator::HRFFliCreator()
    : HRFRasterFileCreator(HRFFliFile::CLASS_ID)
    {
    // FLI/FLIC capabilities instance member initialization
    m_pCapabilities = 0;
    }

//-----------------------------------------------------------------------------
// GetLabel
// Public (HRFFliCreator)
// Identification information
//-----------------------------------------------------------------------------
WString HRFFliCreator::GetLabel() const
    {
    // FLI/FLIC File Format
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_FLI()); // FLI/FLIC File Format
    }

//-----------------------------------------------------------------------------
// GetSchemes
// Public (HRFFliCreator)
// Identification information
//-----------------------------------------------------------------------------
WString HRFFliCreator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

//-----------------------------------------------------------------------------
// GetExtensions
// Public (HRFFliCreator)
// Identification information
//-----------------------------------------------------------------------------
WString HRFFliCreator::GetExtensions() const
    {
    return WString(L"*.fli;*.flc");
    }

//-----------------------------------------------------------------------------
// Create
// Public (HRFFliCreator)
// Allow to Open an image file
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFFliCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                            HFCAccessMode   pi_AccessMode,
                                            uint64_t       pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFFliFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }


//-----------------------------------------------------------------------------
// IsKindOfFile
// Public (HRFFliCreator)
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------
bool HRFFliCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                  uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    HAutoPtr<HFCBinStream>  pFile;
    FliFileHeader           FileHeader;
    FliFilePrefixHeader     PrefixHeader;
    bool                   Result = false;



    (const_cast<HRFFliCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock(GetLockManager());

    // Open the FLI/FLIC File & place file pointer at the start of the file

    pFile = HFCBinStream::Instanciate(pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

    if (pFile == 0 || pFile->GetLastException() != 0)
        goto WRAPUP;

    if (pFile->Read(&FileHeader.size, sizeof FileHeader.size) != sizeof FileHeader.size)
        goto WRAPUP;

    if (FileHeader.size != pFile->GetSize())
        goto WRAPUP;

    if (pFile->Read(&FileHeader.type, sizeof FileHeader.type) != sizeof FileHeader.type)
        goto WRAPUP;

    //only FLI and FLC with 8-bit depth are supported as of now
    if (FileHeader.type != 0xAF11 && FileHeader.type != 0xAF12)
        goto WRAPUP;

    if (pFile->Read(&FileHeader.frames, sizeof FileHeader.frames) != sizeof FileHeader.frames ||
        pFile->Read(&FileHeader.width,  sizeof FileHeader.width)  != sizeof FileHeader.width ||
        pFile->Read(&FileHeader.height, sizeof FileHeader.height) != sizeof FileHeader.height ||
        pFile->Read(&FileHeader.depth,  sizeof FileHeader.depth)  != sizeof FileHeader.depth)
        goto WRAPUP;

    if(FileHeader.depth != 8)
        goto WRAPUP;

    if (pFile->Read(&FileHeader.flags,     sizeof FileHeader.flags)     != sizeof FileHeader.flags ||
        pFile->Read(&FileHeader.speed,     sizeof FileHeader.speed)     != sizeof FileHeader.speed ||
        pFile->Read(&FileHeader.reserved1, sizeof FileHeader.reserved1) != sizeof FileHeader.reserved1)
        goto WRAPUP;

    if(FileHeader.reserved1 != 0x0000)
        goto WRAPUP;

    pFile->SeekToBegin();
    pFile->Seek(FLIC_HEADER_LENGTH);
    if (pFile->Read(&PrefixHeader.size,          sizeof PrefixHeader.size) != sizeof PrefixHeader.size ||
        pFile->Read(&PrefixHeader.type,          sizeof PrefixHeader.type) != sizeof PrefixHeader.type)
        goto WRAPUP;

    if (PrefixHeader.type != 0xF1FA)
        goto WRAPUP;

    Result = true;

WRAPUP:
    SisterFileLock.ReleaseKey();
    HASSERT(!(const_cast<HRFFliCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFFliCreator*>(this))->m_pSharingControl = 0;

    return Result;
    }

//-----------------------------------------------------------------------------
// GetCapabilities
// Public (HRFFliCreator)
// Create or get the singleton capabilities of FIL/FLIC file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFFliCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFFliCapabilities();

    return m_pCapabilities;
    }


//-----------------------------------------------------------------------------
// Destructor
// Public
// Destructor of FIL/FLIC file
//-----------------------------------------------------------------------------
HRFFliFile::~HRFFliFile()
    {
    // Close the FLI/FLIC file
    SaveFliFile(true);
    }

//-----------------------------------------------------------------------------
// CreateResolutionEditor
// Public
// File manipulation
//-----------------------------------------------------------------------------
HRFResolutionEditor* HRFFliFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                        unsigned short pi_Resolution,
                                                        HFCAccessMode  pi_AccessMode)
    {
    // Verify that the page number is 0, because we have one image per file
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    HRFResolutionEditor* pEditor = 0;

    if (m_FliChunkHeader[1].chunkType == FLIC_BRUN)
        {
        // We have a compress FLI
        pEditor = new HRFFliCompressLineEditor(this, pi_Page, pi_Resolution, pi_AccessMode);
        }
    else if (m_FliChunkHeader[1].chunkType == FLIC_COPY)
        {
        pEditor = new HRFFliLineEditor(this, pi_Page, pi_Resolution, pi_AccessMode);
        }
    else
        {
        throw HRFCodecNotSupportedException(m_pURL->GetURL());
        }



    return pEditor;
    }

//-----------------------------------------------------------------------------
// AddPage
// Public
// File manipulation
//-----------------------------------------------------------------------------
bool HRFFliFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    // Validation if it's possible to add a page
    HPRECONDITION(pi_pPage != 0);
    HPRECONDITION(CountPages() == 0);

    // Add the page descriptor to the list
    return HRFRasterFile::AddPage(pi_pPage);
    }



//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Returns the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFFliFile::GetCapabilities () const
    {
    return (HRFFliCreator::GetInstance()->GetCapabilities());
    }

//-----------------------------------------------------------------------------
// Public
// Save
// This method saves the file.
//-----------------------------------------------------------------------------
void HRFFliFile::Save()
    {
    HASSERT(!"HRFFliFile::Save():Fli format is read only");
    }

//-----------------------------------------------------------------------------
// Public
// GetFileCurrentSize
// Get the file's current size.
//-----------------------------------------------------------------------------
uint64_t HRFFliFile::GetFileCurrentSize() const
    {
    return HRFRasterFile::GetFileCurrentSize(m_pFliFile);
    }

//-----------------------------------------------------------------------------
// Protected
// Open
// This method open the file.
//-----------------------------------------------------------------------------
bool HRFFliFile::Open()
    {
    bool Result = true;

    // Open the file
    if (!m_IsOpen)
        {

        m_pFliFile = HFCBinStream::Instanciate(GetURL(), m_Offset, GetAccessMode(), 0, true);

        // This creates the sister file for file sharing control if necessary.
        SharingControlCreate();

        // Initialisation of file struct.
        GetFileHeaderFromFile();

        GetFirstFrameHeaderFromFile();

        GetFirstChunkHeaderFromFile();

        switch(m_FliChunkHeader[0].chunkType)
            {
            case FLIC_COLOR_256:
            case FLIC_COLOR_64:
                {
                if(!GetColorChunk())
                    {
                    Result = false;
                    }
                else
                    {
                    GetNextChunkHeaderFromFile();
                    }
                }
            break;
            default:
                Result = false;
                break;
            }

        m_IsOpen = true;
        }
    return Result;
    }

//-----------------------------------------------------------------------------
// GetFileHeader
// Private
// Read file header from file
//-----------------------------------------------------------------------------
void HRFFliFile::GetFileHeaderFromFile()
    {
    HPRECONDITION(m_pFliFile != 0);

    // Lock the sister file for the getFileHeaderFromFile method
    HFCLockMonitor SisterFileLock(GetLockManager());

    m_pFliFile->Read(&m_FliFileHeader.size,              sizeof m_FliFileHeader.size);
    m_pFliFile->Read(&m_FliFileHeader.type,              sizeof m_FliFileHeader.type);
    m_pFliFile->Read(&m_FliFileHeader.frames,            sizeof m_FliFileHeader.frames);
    m_pFliFile->Read(&m_FliFileHeader.width,             sizeof m_FliFileHeader.width);
    m_pFliFile->Read(&m_FliFileHeader.height,            sizeof m_FliFileHeader.height);
    m_pFliFile->Read(&m_FliFileHeader.depth,             sizeof m_FliFileHeader.depth);



    // Unlock the sister file
    SisterFileLock.ReleaseKey();
    }

//-----------------------------------------------------------------------------
// GetFirstFrameHeaderFromFile
// Private
// Read the first frame header from file
//-----------------------------------------------------------------------------
void HRFFliFile::GetFirstFrameHeaderFromFile()
    {
    HPRECONDITION(m_pFliFile != 0);

    // Lock the sister file for the getFileHeaderFromFile method
    HFCLockMonitor SisterFileLock(GetLockManager());

    m_pFliFile->SeekToBegin();
    m_pFliFile->Seek(FLIC_HEADER_LENGTH);

    m_pFliFile->Read(&m_FliPrefixHeader.size,              sizeof m_FliPrefixHeader.size);
    m_pFliFile->Read(&m_FliPrefixHeader.type,              sizeof m_FliPrefixHeader.type);
    m_pFliFile->Read(&m_FliPrefixHeader.chunks,            sizeof m_FliPrefixHeader.chunks);
    m_pFliFile->Read(&m_FliPrefixHeader.reserved,          sizeof m_FliPrefixHeader.reserved);

    // Unlock the sister file
    SisterFileLock.ReleaseKey();
    }

//-----------------------------------------------------------------------------
// GetFirstChunkHeaderFromFile
// Private
// Read the first frame header from file
//-----------------------------------------------------------------------------
void HRFFliFile::GetFirstChunkHeaderFromFile()
    {
    HPRECONDITION(m_pFliFile != 0);

    // Lock the sister file for the getFileHeaderFromFile method
    HFCLockMonitor SisterFileLock(GetLockManager());

    m_pFliFile->SeekToBegin();
    m_pFliFile->Seek(FLIC_HEADER_LENGTH + FLIC_FRAME_HEADER_LENGTH);

    m_pFliFile->Read(&m_FliChunkHeader[0].chunkSize,              sizeof m_FliChunkHeader[0].chunkSize);
    m_pFliFile->Read(&m_FliChunkHeader[0].chunkType,              sizeof m_FliChunkHeader[0].chunkType);

    //as the oframe1 field of the header is only used by FLC files,
    //we prefer to set the offset manually
    // file header              //1st frame header      //first chunk (palette      //header of second chunk
    m_OffsetToData = FLIC_HEADER_LENGTH + FLIC_FRAME_HEADER_LENGTH + m_FliChunkHeader[0].chunkSize + FLIC_CHUNK_HEADER_LENGTH;

    // Unlock the sister file
    SisterFileLock.ReleaseKey();
    }

//-----------------------------------------------------------------------------
// GetFirstChunkHeaderFromFile
// Private
// Read the first frame header from file
//-----------------------------------------------------------------------------
void HRFFliFile::GetNextChunkHeaderFromFile()
    {
    HPRECONDITION(m_pFliFile != 0);

    // Lock the sister file for the getFileHeaderFromFile method
    HFCLockMonitor SisterFileLock(GetLockManager());

    m_pFliFile->Read(&m_FliChunkHeader[1].chunkSize,              sizeof m_FliChunkHeader[1].chunkSize);
    m_pFliFile->Read(&m_FliChunkHeader[1].chunkType,              sizeof m_FliChunkHeader[1].chunkType);

    // Unlock the sister file
    SisterFileLock.ReleaseKey();
    }

//-----------------------------------------------------------------------------
// GetFirstChunkHeaderFromFile
// Private
// Read the first frame header from file
//-----------------------------------------------------------------------------
bool HRFFliFile::GetColorChunk()
    {
    HPRECONDITION(m_pFliFile != 0);

    unsigned short nbPackets;
    Byte    skipCount;
    Byte    copyCount;

    bool   Result = true;

    // Lock the sister file for the getFileHeaderFromFile method
    HFCLockMonitor SisterFileLock(GetLockManager());

    m_pFliFile->SeekToBegin();
    m_pFliFile->Seek(FLIC_HEADER_LENGTH + FLIC_FRAME_HEADER_LENGTH + FLIC_CHUNK_HEADER_LENGTH);

    m_pFliFile->Read(&nbPackets,              sizeof nbPackets);
    m_pFliFile->Read(&skipCount,              sizeof skipCount);
    m_pFliFile->Read(&copyCount,              sizeof copyCount);

    if(nbPackets == 1)
        {
        if(skipCount == 0)
            {
            if(copyCount == 0 || copyCount == 2)
                {
                GetPaletteFromFile();
                }
            else
                Result = false;
            }
        else
            Result = false;
        }
    else
        Result = false;



    // Unlock the sister file
    SisterFileLock.ReleaseKey();

    return Result;
    }

//-----------------------------------------------------------------------------
// Protected
// CreateDescriptors
//-----------------------------------------------------------------------------
void HRFFliFile::CreateDescriptors ()
    {
    // Obtain the width and height of the resolution.
    uint32_t width  = m_FliFileHeader.width;
    uint32_t height = m_FliFileHeader.height;


    // Create Page and Resolution Description/Capabilities for this file.
    HFCPtr<HRFResolutionDescriptor>     pResolution;
    HFCPtr<HRFPageDescriptor>           pPage;
    
    // Create codec for compression.
    HFCPtr<HCDCodec> pCodec;

    HRFBlockType    BlockType   = HRFBlockType::LINE;
    uint32_t        BlockHeight = 1;

    // Compression is RLE8
    if(m_FliChunkHeader[1].chunkType == FLIC_BRUN)
        {
        pCodec = new HCDCodecFLIRLE8();

        //its a line by line compression but nothing indicates the end of the line
        //so the codec takes care of it
        BlockType   = HRFBlockType::IMAGE;
        BlockHeight = height;
        }
    else if (m_FliChunkHeader[1].chunkType == FLIC_COPY)
        {
        pCodec = new HCDCodecIdentity();

        //its a line by line compression but nothing indicates the end of the line
        //so the codec takes care of it
        BlockType   = HRFBlockType::IMAGE;
        BlockHeight = height;
        }
    else
        {
        throw HRFCodecNotSupportedException(m_pURL->GetURL());
        }

    // Tag information
    HPMAttributeSet TagList;
    HFCPtr<HPMGenericAttribute> pTag;

    // Create Resolution Descriptor
    pResolution =  new HRFResolutionDescriptor(
        GetAccessMode(),                                // AccessMode,
        GetCapabilities(),                              // Capabilities,
        1.0,                                            // XResolutionRatio,
        1.0,                                            // YResolutionRatio,
        CreatePixelTypeFromFile(),                      // PixelType,
        pCodec,                                         // Codec,  
        HRFBlockAccess::SEQUENTIAL,                     // RStorageAccess,
        HRFBlockAccess::SEQUENTIAL,                     // WStorageAccess,
        HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL,  // ScanLineOrientation,
        HRFInterleaveType::PIXEL,                       // InterleaveType
        false,                                          // IsInterlace,
        width,                                          // Width,
        height,                                         // Height,
        width,                                          // BlockWidth,
        BlockHeight,                                              // BlockHeight,
        0,                                              // BlocksDataFlag
        BlockType);

    pPage = new HRFPageDescriptor (GetAccessMode(),
                                   GetCapabilities(),   // Capabilities,
                                   pResolution,         // ResolutionDescriptor,
                                   0,                   // RepresentativePalette,
                                   0,                   // Histogram,
                                   0,                   // Thumbnail,
                                   0,                   // ClipShape,
                                   0,                   // TransfoModel,
                                   0,                   // Filters
                                   &TagList);           // Defined Tag

    m_ListOfPageDescriptor.push_back(pPage);
    }


//-----------------------------------------------------------------------------
// Private
// SaveFliFile
// This method close the file.
//-----------------------------------------------------------------------------
void HRFFliFile::SaveFliFile(bool pi_CloseFile)
    {
    // Be sure that the file is already open and that at least one page
    // has been add. Because if the destroyer is call afer a exception
    // is thrown, we want to be sure that the object is valid before we
    // execute the destroyer.
    if (m_IsOpen && m_ListOfPageDescriptor.size() > 0)
        {
        try
            {
            if (GetAccessMode().m_HasCreateAccess)
                {
                HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(0);

                // Lock the sister file.
                HFCLockMonitor SisterFileLock (GetLockManager());

                // Free all memory allocated by the read or write process.
                // If the number of writed rows is not equal to the number of rows
                // we can not save the PngInfo structure.


                m_pFliFile->Flush();

                // Unlock the sister file.
                SisterFileLock.ReleaseKey();
                }

            if(pi_CloseFile)
                {
                // Close the file.
                // disable std io fclose(m_pPngFile);
                m_pFliFile = 0;
                m_IsOpen = false;
                }
            }
        catch(...)
            {
            // Simply stop exceptions in the destructor
            // We want to known if a exception is throw.
            HASSERT(0);
            }
        }
    }

//-----------------------------------------------------------------------------
// Private
// Create
// This method create the file.
//-----------------------------------------------------------------------------
bool HRFFliFile::Create()
    {
    // Open the file.
    m_pFliFile = HFCBinStream::Instanciate(GetURL(), m_Offset, GetAccessMode(), 0, true);

    // Create the sister file for file sharing control
    SharingControlCreate();

    return true;
    }

//-----------------------------------------------------------------------------
// GetPaletteToFile
// Private
// Read palette from file
//-----------------------------------------------------------------------------
void HRFFliFile::GetPaletteFromFile()
    {
    HPRECONDITION(m_pFliFile != 0);

    uint32_t maxColor;

    maxColor = (uint32_t)pow(2.0, (int)m_FliFileHeader.depth);

    m_RgbColors = new FliRGBColor[maxColor];

    // Lock the sister file for the GetPaletteFromFile method
    HFCLockMonitor SisterFileLock(GetLockManager());

    switch(m_FliChunkHeader[0].chunkType)
        {
        case FLIC_COLOR_256:

            for (uint32_t color=0; color < maxColor; color++)
                {
                m_pFliFile->Read(&m_RgbColors[color].m_rgbRed,  sizeof m_RgbColors[color].m_rgbRed);
                m_pFliFile->Read(&m_RgbColors[color].m_rgbGreen, sizeof m_RgbColors[color].m_rgbGreen);
                m_pFliFile->Read(&m_RgbColors[color].m_rgbBlue,   sizeof m_RgbColors[color].m_rgbBlue);
                }
            break;

        case FLIC_COLOR_64:
            for (uint32_t color=0; color < maxColor; color++)
                {
                m_pFliFile->Read(&m_RgbColors[color].m_rgbRed,  sizeof m_RgbColors[color].m_rgbRed);
                m_pFliFile->Read(&m_RgbColors[color].m_rgbGreen, sizeof m_RgbColors[color].m_rgbGreen);
                m_pFliFile->Read(&m_RgbColors[color].m_rgbBlue,   sizeof m_RgbColors[color].m_rgbBlue);
                }
            //we mult by 4 so the range goes from 0 to 256
            for (uint32_t color=0; color < maxColor; color++)
                {
                m_RgbColors[color].m_rgbBlue *= 4;
                m_RgbColors[color].m_rgbGreen *= 4;
                m_RgbColors[color].m_rgbRed *=4 ;
                }
            break;

        default :
            throw HFCCorruptedFileException(m_pURL->GetURL());
            break;

        }



    // Unlock the sister file
    SisterFileLock.ReleaseKey();

    }



//-----------------------------------------------------------------------------
// Private
// CreatePixelTypeFromFile
// Create pixel type for the current image
//-----------------------------------------------------------------------------
HFCPtr<HRPPixelType> HRFFliFile::CreatePixelTypeFromFile() const
    {
    HFCPtr<HRPPixelType> pPixelType;
    uint32_t              Index;
    HRPPixelPalette*     pPalette;
    Byte                Value[3];
    uint32_t              maxColor;


    maxColor = (uint32_t)pow(2.0, (int)m_FliFileHeader.depth);


    switch (m_FliFileHeader.depth)
        {
        case 8:
            pPixelType = HRPPixelTypeFactory::GetInstance()->Create(HRPChannelOrgRGB(8,
                                                                                     8,
                                                                                     8,
                                                                                     0,
                                                                                     HRPChannelType::UNUSED,
                                                                                     HRPChannelType::VOID_CH,
                                                                                     0),
                                                                    m_FliFileHeader.depth);

            // Get the palette from the pixel type.
            pPalette          = (HRPPixelPalette*)&(pPixelType->GetPalette());

            // Copy the palette into the pixel palette.
            for (Index = 0; Index < maxColor ; ++Index)
                {
                Value[0] = m_RgbColors[Index].m_rgbRed;
                Value[1] = m_RgbColors[Index].m_rgbGreen;
                Value[2] = m_RgbColors[Index].m_rgbBlue;

                // Add the entry to the pixel palette.
                pPalette->SetCompositeValue(Index, Value);
                }
            break;


            // We have a color image

        case 16:

            //TODO : not supported yet

            break;


            // We have a color image (RGB).
        case 24:

            //TODO : not supported yet

            break;

        }


    return pPixelType;
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFFliFile::HRFFliFile(const HFCPtr<HFCURL>& pi_rURL,
                              HFCAccessMode         pi_AccessMode,
                              uint64_t             pi_Offset)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor store the access mode
    m_IsOpen     = false;

    if (GetAccessMode().m_HasCreateAccess || GetAccessMode().m_HasWriteAccess)
        {
        //this is a read-only format
        throw HFCFileReadOnlyException(pi_rURL->GetURL());
        }
    else
        {
        // if Open success and it is not a new file
        if(!Open())
            throw HFCCorruptedFileException(m_pURL->GetURL());
        // Create Page and Res Descriptors.
        CreateDescriptors();
        }
    }


//-----------------------------------------------------------------------------
// Protected
// Constructor
// allow to Create an image file object without open.
//-----------------------------------------------------------------------------
HRFFliFile::HRFFliFile(const HFCPtr<HFCURL>& pi_rURL,
                              HFCAccessMode         pi_AccessMode,
                              uint64_t             pi_Offset,
                              bool                 pi_DontOpenFile)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor store the access mode
    m_IsOpen     = false;

    if (GetAccessMode().m_HasCreateAccess || GetAccessMode().m_HasWriteAccess)
        {
        //this is a read-only format
        throw HFCFileReadOnlyException(pi_rURL->GetURL());
        }

    }

//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
// File information
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFFliFile::GetWorldIdentificator () const
    {
    return HGF2DWorld_UNKNOWNWORLD;
    }

//-----------------------------------------------------------------------------
// Public
// GetMemoryFilePtr
// Return the file pointer if the FLI/FLIC was in memory, 0 otherwise
//-----------------------------------------------------------------------------
const HFCMemoryBinStream* HRFFliFile::GetMemoryFilePtr() const
    {
    HPOSTCONDITION(m_pFliFile != 0);

    if (m_pFliFile->IsCompatibleWith(HFCMemoryBinStream::CLASS_ID))
        return (HFCMemoryBinStream*)m_pFliFile.get();
    else
        return 0;
    }

//-----------------------------------------------------------------------------
// protected
// GetFilePtr   - Get the FLI/FLIC file pointer.
//-----------------------------------------------------------------------------
HFCBinStream* HRFFliFile::GetFilePtr  ()
    {
    return (m_pFliFile);
    }

//-----------------------------------------------------------------------------
// protected
// GetChunkSize( int n)   - Get the n Chunk size
//-----------------------------------------------------------------------------
int HRFFliFile::GetChunkSize  (int chunkNb)
    {
    return m_FliChunkHeader[chunkNb].chunkSize;
    }

//-----------------------------------------------------------------------------
// protected
// GetChunkHeaderSize()   - Get the n Chunk size
//-----------------------------------------------------------------------------
int HRFFliFile::GetChunkHeaderSize()
    {
    return FLIC_CHUNK_HEADER_LENGTH;
    }
