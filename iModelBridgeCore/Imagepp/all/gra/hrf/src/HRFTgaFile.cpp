//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFTgaFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFTgaFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HFCMath.h>
#include <Imagepp/all/h/HRFTgaFile.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HRFException.h>

#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>

#include <Imagepp/all/h/HCDCodecIdentity.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>

#ifndef DIVROUNDUP
#define DIVROUNDUP(x,y) ((x+y-1)/y)
#else
#error
#endif

#define COLOR_CORECTION_TABLE_ENTRY_SIZE 0x400

#define  BLOCKSIZE  0x400000

/**-----------------------------------------------------------------------------
 This class declare the Block Capabilitie of the none-compressed TGA file format.
 In this case, the only block type supported is the Line one. The lines can be
 accessed randomaly.
------------------------------------------------------------------------------*/
class HRFTgaBlockCapabilities : public HRFRasterFileCapabilities
    {
public :
    // Constructor
    HRFTgaBlockCapabilities ()
        : HRFRasterFileCapabilities()
        {
        // Block capability
        Add (new HRFLineCapability (HFC_READ_WRITE_CREATE,
                                    LONG_MAX,
                                    HRFBlockAccess::RANDOM));
        }
    };

/**-----------------------------------------------------------------------------
 This class declare the Block Capabilitie of the compressed TGA file format.
 In this case, the only block type supported is the Line one. The lines can only
 be accessed sequentialy.
------------------------------------------------------------------------------*/
class HRFTgaCompressBlockCapabilities : public HRFRasterFileCapabilities
    {
public :
    // Constructor
    HRFTgaCompressBlockCapabilities ()
        : HRFRasterFileCapabilities()
        {
        // Block capability
        Add (new HRFLineCapability (HFC_READ_WRITE_CREATE,
                                    LONG_MAX,
                                    HRFBlockAccess::SEQUENTIAL));

        Add(new HRFImageCapability(HFC_READ_ONLY,          // AccessMode
                                   LONG_MAX,               // MaxSizeInBytes
                                   0,                      // MinWidth
                                   LONG_MAX,               // MaxWidth
                                   0,                      // MinHeight
                                   LONG_MAX));             // MaxHeight
        }
    };

//-----------------------------------------------------------------------------
// HRFTgaCodecCapabilities
//-----------------------------------------------------------------------------
class HRFTgaCodecCapabilities : public  HRFRasterFileCapabilities
{
public:
    // Constructor
    HRFTgaCodecCapabilities()
        : HRFRasterFileCapabilities()
    {
        // Codec Identity
        Add (new HRFCodecCapability (HFC_READ_WRITE_CREATE,
            HCDCodecIdentity::CLASS_ID,
            new HRFTgaBlockCapabilities()));
        // Codec RLE
        Add (new HRFCodecCapability (HFC_READ_CREATE,
            HCDCodecTGARLE::CLASS_ID,
            new HRFTgaCompressBlockCapabilities()));
    }
};

/**-----------------------------------------------------------------------------
 This class declare all the hierarchy of the capabilities supported by the
 TGA file format. This include the pixel type supported, the scanline orientation
 and many others.
------------------------------------------------------------------------------*/
HRFTgaCapabilities::HRFTgaCapabilities()
    : HRFRasterFileCapabilities()
    {
    // PixelTypeI8R8G8B8
    Add (new HRFPixelTypeCapability (HFC_READ_WRITE_CREATE,
        HRPPixelTypeI8R8G8B8::CLASS_ID,
        new HRFTgaCodecCapabilities()));

    // PixelTypeI8R8G8B8A8
    Add (new HRFPixelTypeCapability (HFC_READ_WRITE_CREATE,
        HRPPixelTypeI8R8G8B8A8::CLASS_ID,
        new HRFTgaCodecCapabilities()));

    // PixelTypeV8Gray8
    Add (new HRFPixelTypeCapability (HFC_READ_WRITE_CREATE,
        HRPPixelTypeV8Gray8::CLASS_ID,
        new HRFTgaCodecCapabilities()));

    // PixelTypeV24R8G8B8
    Add (new HRFPixelTypeCapability (HFC_READ_WRITE_CREATE,
        HRPPixelTypeV24R8G8B8::CLASS_ID,
        new HRFTgaCodecCapabilities()));

    // PixelTypeV32R8G8B8A8
    Add (new HRFPixelTypeCapability (HFC_READ_WRITE_CREATE,
        HRPPixelTypeV32R8G8B8A8::CLASS_ID,
        new HRFTgaCodecCapabilities()));

    // Block Access Capability
    //Add(new HRFLineCapability(HFC_READ_WRITE_CREATE, LONG_MAX, HRFBlockAccess::RANDOM));

    // Thumbnail capability
    Add(new HRFThumbnailCapability(HFC_READ_WRITE_CREATE,
                                   0,       // pi_MinWidth
                                   64,      // pi_MaxWidth
                                   0,       // pi_WidthIncrement
                                   0,       // pi_MinHeight
                                   64,      // pi_MaxHeight
                                   0,       // pi_HeightIncrement
                                   16384,   // pi_MaxSizeInBytes
                                   false)); // pi_IsComposed

    // Scanline Orientation Capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));
    //Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::LOWER_LEFT_HORIZONTAL));
    //Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::UPPER_RIGHT_HORIZONTAL));
    //Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::LOWER_RIGHT_HORIZONTAL));

    // Single Resolution Capability
    Add(new HRFSingleResolutionCapability(HFC_READ_WRITE_CREATE));

    // Interleave Capability
    Add(new HRFInterleaveCapability(HFC_READ_WRITE_CREATE, HRFInterleaveType::PIXEL));

    // Still Image Capability
    Add(new HRFStillImageCapability(HFC_READ_WRITE_CREATE));

    // Tag capability
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeArtist));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeNotes));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeDateTime));

    //Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HPMAttribute<string>(HRF_TGA_UNIFORM_TAGLABEL_JOBNAMEID, "")));
    //Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HPMAttribute<string>(HRF_TGA_UNIFORM_TAGLABEL_JOBTIME, "")));

    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeSoftware));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeVersion));
    Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeBackground(0)));

    //Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HPMAttribute<UShort>(HRF_TGA_UNIFORM_TAGLABEL_ASPECT_RATIO_NUM, "")));
    //Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HPMAttribute<UShort>(HRF_TGA_UNIFORM_TAGLABEL_ASPECT_RATIO_DEN, "")));
    //Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HPMAttribute<UShort>(HRF_TGA_UNIFORM_TAGLABEL_GAMMA_NUM, "")));  <--- avoir
    //Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HPMAttribute<UShort>(HRF_TGA_UNIFORM_TAGLABEL_GAMMA_DEN, "")));  <--- avoir
    }

//:Ignore
HFC_IMPLEMENT_SINGLETON(HRFTgaCreator)
//:End Ignore

/**-----------------------------------------------------------------------------
 Constructor for this class. It only initialize the capabilities pointer
 to NULL.
------------------------------------------------------------------------------*/
HRFTgaCreator::HRFTgaCreator()
    : HRFRasterFileCreator(HRFTgaFile::CLASS_ID)
    {
    // TGA capabilities instance member initialization
    m_pCapabilities = 0;
    }

/**-----------------------------------------------------------------------------
 Return a string that describe the file format type.

 @return string TGA file format label.
------------------------------------------------------------------------------*/
WString HRFTgaCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_Targa()); // TGA Targa TrueVision
    }

/**-----------------------------------------------------------------------------
 Return the scheme of the URL.

 @return string The scheme of the URL.
------------------------------------------------------------------------------*/
WString HRFTgaCreator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

/**-----------------------------------------------------------------------------
 Return the extention accepted for the TGA file format.

 @return string The TGA extention.
------------------------------------------------------------------------------*/
WString HRFTgaCreator::GetExtensions() const
    {
    return WString(L"*.tga");
    }

/**-----------------------------------------------------------------------------
 Opens a raster file with the specified location and access mode.

 @param pi_rpURL The URL of the file.
 @param pi_AccessMode The access and sharing mode of the file.
 @param pi_Offset The starting offset into the file.

 @return HFCPtr<HRFRasterFile> A pointer on the HRFRasterfile instance
------------------------------------------------------------------------------*/
HFCPtr<HRFRasterFile> HRFTgaCreator::Create(
    const HFCPtr<HFCURL>& pi_rpURL,
    HFCAccessMode         pi_AccessMode,
    uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    HFCPtr<HRFRasterFile> pFile = new HRFTgaFile(pi_rpURL, pi_AccessMode);
    HASSERT(pFile != 0);
    return (pFile);
    }

/**-----------------------------------------------------------------------------
 This function checks whether the specified URL is a TGA file.

 @param pi_rpURL Thr URL of the file.
 @param pi_Offset The starting offset into the file.

 @return bool True if the file is a TGA file.
------------------------------------------------------------------------------*/
bool HRFTgaCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                  uint64_t             pi_Offset) const
    {
    bool                       Result = false;
    bool                       IsCompressed = false;
    HAutoPtr<HFCBinStream>      pFile;
    HRFTgaFile::TgaFileHeader   TgaHdr;
    HRFTgaFile::TgaFileFooter   TgaFtr;

    (const_cast<HRFTgaCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock(GetLockManager());

    // Open the TGA File & place file pointer at the start of the file
    pFile = HFCBinStream::Instanciate(pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

    if (pFile == 0 || pFile->GetLastException() != 0)
        goto WRAPUP;

    // read header
    if (sizeof(HRFTgaFile::TgaFileHeader) != pFile->Read (&TgaHdr, sizeof(HRFTgaFile::TgaFileHeader)))
        goto WRAPUP;

    // seek for the footer
    pFile->SeekToPos(pFile->GetSize() - (long)(sizeof(HRFTgaFile::TgaFileFooter)));

    // read footer
    if (sizeof(HRFTgaFile::TgaFileFooter) != pFile->Read (&TgaFtr, sizeof(HRFTgaFile::TgaFileFooter)))
        goto WRAPUP;

    if (TgaHdr.m_ImageType == 2 || TgaHdr.m_ImageType == 3 ||
        TgaHdr.m_ImageType == 10 || TgaHdr.m_ImageType == 11)
        {
        if (TgaHdr.m_ColorMapType != 0 && TgaHdr.m_ColorMapType != 1)
            goto WRAPUP;

        if (TgaHdr.m_ImageType >= 9)
            IsCompressed = true;
        }
    else if (TgaHdr.m_ImageType == 1 || TgaHdr.m_ImageType == 9)
        {
        if (TgaHdr.m_ColorMapType != 1)
            goto WRAPUP;

        if (TgaHdr.m_ImageType == 9)
            IsCompressed = true;
        }
    else
        goto WRAPUP;

    if (TgaHdr.m_ColorMapLength > 256)
        goto WRAPUP;

    if (TgaHdr.m_ColorMapEntrySize != 0 &&
        TgaHdr.m_ColorMapEntrySize != 8 &&
        TgaHdr.m_ColorMapEntrySize != 15 &&
        TgaHdr.m_ColorMapEntrySize != 16 &&
        TgaHdr.m_ColorMapEntrySize != 24 &&
        TgaHdr.m_ColorMapEntrySize != 32)
        goto WRAPUP;

    if (TgaHdr.m_PixelDepth != 8 &&
        TgaHdr.m_PixelDepth != 15 &&
        TgaHdr.m_PixelDepth != 16 &&
        TgaHdr.m_PixelDepth != 24 &&
        TgaHdr.m_PixelDepth != 32)
        goto WRAPUP;

    if (TgaHdr.m_ImageDescriptor >= 0x40)
        goto WRAPUP;

    if (0 != strcmp (TgaFtr.m_Signature, (const char*)TGA_SIGNATURE))
        {
        // Old format... must validate the file size

        // HeaderSize
        uint32_t FileSize = sizeof (HRFTgaFile::TgaFileHeader);

        // ID size
        FileSize += TgaHdr.m_IdLength;

        // Colormap size
        FileSize += TgaHdr.m_ColorMapLength * DIVROUNDUP(TgaHdr.m_ColorMapEntrySize, 8);
        if (!IsCompressed)
            {
            // Raw data size
            FileSize += TgaHdr.m_ImageHeight * TgaHdr.m_ImageWidth * DIVROUNDUP(TgaHdr.m_PixelDepth, 8);

            //TR 216541 - Some TGA files seem to have metadata, padding lines or else at the end.
            if (FileSize > pFile->GetSize())
                goto WRAPUP;
            }
        else
            {
            // Raw data theorique maximum size
            FileSize += (uint32_t)(TgaHdr.m_ImageHeight * TgaHdr.m_ImageWidth * DIVROUNDUP(TgaHdr.m_PixelDepth, 8) * 1.5);
            if (pFile->GetSize() > FileSize)
                goto WRAPUP;
            }
        }

    Result = true;

WRAPUP:
    SisterFileLock.ReleaseKey();
    HASSERT(!(const_cast<HRFTgaCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFTgaCreator*>(this))->m_pSharingControl = 0;

    return Result;
    }

/**-----------------------------------------------------------------------------
 Return the capabilities of TGA file format.

 @return HFCPtr<HRFRasterFileCapabilities> A pointer on the singleton
         capabilities of TGA file.
------------------------------------------------------------------------------*/
const HFCPtr<HRFRasterFileCapabilities>& HRFTgaCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFTgaCapabilities();

    return m_pCapabilities;
    }

/**-----------------------------------------------------------------------------
 This method open the physical file on the disk and keep a pointer to it.

 @return true if the file exist and is opened.
------------------------------------------------------------------------------*/
bool HRFTgaFile::Open()
    {
    if (!m_IsOpen)
        {
        m_pTgaFile = HFCBinStream::Instanciate(GetURL(), m_Offset, GetAccessMode(), 0, true);

        // This creates the sister file for file sharing control if necessary.
        SharingControlCreate();

        // Lock the sister file
        HFCLockMonitor SisterFileLock (GetLockManager());

        // Initialisation of file struct.
        GetFileHeaderFromFile();
        GetMapInfoFromFile();
        GetFileFooterFromFile();
        GetExtensionAreaFromFile();

        // Unlock the sister file
        SisterFileLock.ReleaseKey();


        // If it's a compress raster file.
        if ((m_pTgaFileHeader->m_ImageType >= 9) && (m_pTgaFileHeader->m_ImageType <= 11) &&
            ( GetAccessMode().m_HasWriteAccess || GetAccessMode().m_HasCreateAccess ))
            {
            throw HRFAccessModeForCodeNotSupportedException(GetURL()->GetURL());
            }

        m_IsOpen = true;
        }

    return m_IsOpen;
    }

/**-----------------------------------------------------------------------------
  This method return the current size of the file
-----------------------------------------------------------------------------*/
uint64_t HRFTgaFile::GetFileCurrentSize() const
    {
    return HRFRasterFile::GetFileCurrentSize(m_pTgaFile);
    }

/**-----------------------------------------------------------------------------
 This method creates the resolution descriptor and the page descriptor for
 this file.
------------------------------------------------------------------------------*/
void HRFTgaFile::CreateDescriptors ()
    {
    HPRECONDITION (m_pTgaFile != NULL);

    // Create Page and resolution Description/Capabilities for this file.
    HFCPtr<HRFResolutionDescriptor>         pResolution;
    HFCPtr<HRFPageDescriptor>               pPage;
    HFCPtr<HRFThumbnail>                    pThumbnail;
    HFCPtr<HRPPixelType>                    pPixelType;
    HArrayAutoPtr<Byte>                   pData;

    // Scanline Orientation
    if (0 != (m_pTgaFileHeader->m_ImageDescriptor & 0x10))
        throw HRFSloNotSupportedException(GetURL()->GetURL());

    // Validate the alpha channel bits
    Byte AlphaChannelBits = m_pTgaFileHeader->m_ImageDescriptor & 0x0F;
    if (AlphaChannelBits != 0 && (AlphaChannelBits != 8 && !(AlphaChannelBits == 1 && m_pTgaFileHeader->m_PixelDepth == 16)))
        throw HFCCorruptedFileException(GetURL()->GetURL());

    // Pixel type
    pPixelType = CreatePixelTypeFromFile();

    // Thumbnail
    if ((m_pTgaExtTableArea != 0) && (m_pTgaExtentionArea->m_PostageStampOffset != 0))
        {
        uint32_t NbPixel        = m_pTgaExtTableArea->m_StampWidth * m_pTgaExtTableArea->m_StampHeight;
        unsigned short NbBytePerPixel = DIVROUNDUP (m_pTgaFileHeader->m_PixelDepth, 8);
        if (NbBytePerPixel == 2)
            NbBytePerPixel++;

        pData = new Byte[NbPixel * NbBytePerPixel];

        if ((m_pTgaFileHeader->m_ImageDescriptor & 0x20) == 0)
            {
            uint32_t NbBytesPerLine = m_pTgaExtTableArea->m_StampWidth * NbBytePerPixel;

            // Data is stored LOWER_LEFT_HORIZONTAL : Reverse lines
            uint32_t SrcLine = m_pTgaExtTableArea->m_StampHeight - 1;
            for (uint32_t DestLine = 0 ;
                 DestLine < m_pTgaExtTableArea->m_StampHeight ;
                 ++DestLine, --SrcLine)
                {
                memcpy (pData + (DestLine * NbBytesPerLine),
                        m_pTgaExtTableArea->m_pStampData + (SrcLine * NbBytesPerLine),
                        NbBytesPerLine);
                }
            }
        else
            memcpy (pData, m_pTgaExtTableArea->m_pStampData, NbPixel*NbBytePerPixel);

        pThumbnail  =  new HRFThumbnail(
            m_pTgaExtTableArea->m_StampWidth,               // Thumbnail width
            m_pTgaExtTableArea->m_StampHeight,              // Thumbnail height
            pPixelType,                                     // PixeType
            pData,                                          // Data
            HFC_READ_WRITE);                                // AccessMode

        }

    HFCPtr<HCDCodec>        pCodec = CreateCodecFromFile();
    HRFBlockAccess          BlockAccess;

    if (pCodec->IsCompatibleWith(HCDCodecIdentity::CLASS_ID))
        BlockAccess = HRFBlockAccess::RANDOM;
    else
        BlockAccess = HRFBlockAccess::SEQUENTIAL;

    // Image access for special RLE case, otherwise line access
    HRFBlockType BlockType;
    uint32_t BlockHeight;

    if (!pCodec->IsCompatibleWith(HCDCodecIdentity::CLASS_ID)    &&
        !GetAccessMode().m_HasWriteAccess                    &&
        !GetAccessMode().m_HasCreateAccess                   &&
        RunLengthsSpanScanlines())
        {
        BlockType = HRFBlockType::IMAGE;
        BlockHeight = m_pTgaFileHeader->m_ImageHeight;
        }
    else
        {
        BlockType = HRFBlockType::LINE;
        BlockHeight = 1;
        }

    pResolution =  new HRFResolutionDescriptor(
        GetAccessMode(),                                // AccessMode,
        GetCapabilities(),                              // Capabilities,
        1.0,                                            // XResolutionRatio,
        1.0,                                            // YResolutionRatio,
        pPixelType,                                     // PixelType,
        pCodec,                                         // Codec,
        HRFBlockAccess::RANDOM,                         // RBlockAccess,
        BlockAccess,                                    // WBlockAccess,
        HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL,  // ScanLineOrientation,
        HRFInterleaveType::PIXEL,                       // InterleaveType
        true,                                           // IsInterlace,
        m_pTgaFileHeader->m_ImageWidth,                 // Width image ,
        m_pTgaFileHeader->m_ImageHeight,                // Height image,
        m_pTgaFileHeader->m_ImageWidth,                 // BlockWidth,
        BlockHeight,                                    // BlockHeight,
        0,                                              // BlocksDataFlag
        BlockType);                                     // Block type

    // Tag information
    HPMAttributeSet             TagList;
    HFCPtr<HPMGenericAttribute> pTag;
    char                       StrInput[41];
    uint32_t                     Result;

    if (m_pTgaExtentionArea != 0)
        {
        // AuthorName Tag
        if (strlen((const char*)m_pTgaExtentionArea->m_AuthorName) != 0)
            {
            pTag = new HRFAttributeArtist(WString((char*)m_pTgaExtentionArea->m_AuthorName,false));
            TagList.Set(pTag);
            }

        // AuthorComment Tag
        if (strlen((const char*)m_pTgaExtentionArea->m_AuthorComments) != 0)
            {
            pTag = new HRFAttributeNotes(WString((char*)m_pTgaExtentionArea->m_AuthorComments,false));
            TagList.Set(pTag);
            }

        // Date/Time Tag
        Result = sprintf (StrInput,
                          "%04u/%02u/%02u  %02u:%02u:%02u",
                          m_pTgaExtentionArea->m_Year,
                          m_pTgaExtentionArea->m_Month,
                          m_pTgaExtentionArea->m_Day,
                          m_pTgaExtentionArea->m_Hour,
                          m_pTgaExtentionArea->m_Minute,
                          m_pTgaExtentionArea->m_Second);

        pTag = new HRFAttributeDateTime(WString(StrInput,false));
        TagList.Set(pTag);


        // JobNameId Tag
//        if (strlen((const char*)m_pTgaExtentionArea->m_JobNameId) != 0)
//        {
//            pTag = new HPMAttribute<string>(HRF_TGA_UNIFORM_TAGLABEL_JOBNAMEID, (char*)m_pTgaExtentionArea->m_JobNameId);
//            TagList.Set(pTag);
//        }


        // JobTime Tag
//        Result = sprintf (StrInput,
//                          "%u:%u:%u",
//                          m_pTgaExtentionArea->m_JobHours,
//                          m_pTgaExtentionArea->m_JobMinutes,
//                          m_pTgaExtentionArea->m_JobSeconds);
//        pTag = new HPMAttribute<string>(HRF_TGA_UNIFORM_TAGLABEL_JOBTIME, (string)StrInput);
//        TagList.Set(pTag);

        // SoftwareId Tag
        if (strlen((const char*)m_pTgaExtentionArea->m_SoftwareId) != 0)
            {
            pTag = new HRFAttributeSoftware(WString((char*)m_pTgaExtentionArea->m_SoftwareId,false));
            TagList.Set(pTag);
            }

        // SoftwareVersion Tag
        Result = sprintf (StrInput,
                          "%3.2f%c",
                          ((float)m_pTgaExtentionArea->m_SoftwareVersion/100.0),
                          m_pTgaExtentionArea->m_SoftwareVersionLetter);
        pTag = new HRFAttributeVersion(WString(StrInput,false));
        TagList.Set(pTag);

        // BackGround Color
        pTag = new HRFAttributeBackground(m_pTgaExtentionArea->m_BackgroundColor);
        TagList.Set(pTag);

        // AspectRationNum
//        pTag = new HPMAttribute<UShort>(HRF_TGA_UNIFORM_TAGLABEL_ASPECT_RATIO_NUM, m_pTgaExtentionArea->m_PixelRatioNum);
//        TagList.Set(pTag);

        // AspectRationDen
//        pTag = new HPMAttribute<UShort>(HRF_TGA_UNIFORM_TAGLABEL_ASPECT_RATIO_DEN, m_pTgaExtentionArea->m_PixelRatioDen);
//        TagList.Set(pTag);

        // GammaNum
//        pTag = new HPMAttribute<UShort>(HRF_TGA_UNIFORM_TAGLABEL_GAMMA_NUM, m_pTgaExtentionArea->m_GammaNum);
//        TagList.Set(pTag);

        // GammaDen
//        pTag = new HPMAttribute<UShort>(HRF_TGA_UNIFORM_TAGLABEL_GAMMA_DEN, m_pTgaExtentionArea->m_GammaDen);
//        TagList.Set(pTag);
        }

    pPage = new HRFPageDescriptor (GetAccessMode(),
                                   GetCapabilities(),                              // Capabilities,
                                   pResolution,                                    // ResolutionDescriptor,
                                   0,                                              // RepresentativePalette,
                                   0,                                              // Histogram,
                                   pThumbnail,                                     // Thumbnail,
                                   0,                                              // ClipShape,
                                   0,                                              // TransfoModel,
                                   0,                                              // Filters
                                   &TagList);                                      // Attribute set


    m_ListOfPageDescriptor.push_back(pPage);
    }

/**-----------------------------------------------------------------------------
 Public destructor of the class. This function does what it takes to close the
 raster file.
------------------------------------------------------------------------------*/
HRFTgaFile::~HRFTgaFile()
    {
    try
        {
        SaveTgaFile(true);

        }
    catch(...)
        {
        // Simply stop exceptions in the destructor
        // We want to known if a exception is throw.
        HASSERT(0);
        }
    }

/**-----------------------------------------------------------------------------
 Function close of the class. Here we must write to the file informations
 that has been modified or displace since the creation or the opening of the file.
 This includes footer, tag, palettes, etc..
------------------------------------------------------------------------------*/
void HRFTgaFile::SaveTgaFile(bool pi_CloseFile)
    {
    bool IsCreate  = false;
    bool CanWrite  = true;
    bool HasFtr    = true;
    bool HasExt    = false;

    if (m_IsOpen)
        // Initialize the bool variables
        if (!(GetAccessMode().m_HasCreateAccess))
            if (GetAccessMode().m_HasWriteAccess)
                if (m_pTgaFileFooter == 0)
                    HasFtr = false;
                else
                    {
                    if (m_pTgaExtentionArea != 0)
                        HasExt = true;
                    }
            else
                CanWrite = false;
        else
            IsCreate = true;

    // Be sure that the file is already open and that at least one page
    // has been add. Because if the destroyer is call afer a exception
    // is thrown, we want to be sure that the object is valid before we
    // execute the destroyer.
    if (m_IsOpen && (CountPages() > 0) && CanWrite)
        {

        // Lock the sister file.
        HFCLockMonitor SisterFileLock (GetLockManager());

        HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(0);

        // Set the color-map into file
        HASSERT (pPageDescriptor->CountResolutions() > 0);

        if ((pPageDescriptor->GetResolutionDescriptor(0)->GetPixelType()->CountIndexBits() > 0) &&
            (pPageDescriptor->GetResolutionDescriptor(0)->PaletteHasChanged()))
            SetPaletteToFile();

        // Set the extension area field that has to be update
        // Update each tag to the file when has changed.
        HPMAttributeSet::HPMASiterator TagIterator;

        for (TagIterator  = pPageDescriptor->GetTags().begin(); 
             TagIterator != pPageDescriptor->GetTags().end();
             TagIterator++)
            {
            HFCPtr<HPMGenericAttribute> pTag = (*TagIterator);

            if (pPageDescriptor->TagHasChanged(*pTag) || GetAccessMode().m_HasCreateAccess)
                {
                // AuthorName Tag
                if (pTag->GetID() == HRFAttributeArtist::ATTRIBUTE_ID) 
                    {
                    AString tempStrA;
                    BeStringUtilities::WCharToCurrentLocaleChar(tempStrA, ((HFCPtr<HRFAttributeArtist>&)pTag)->GetData().c_str());
                    BeStringUtilities::Strncpy((char*)m_pTgaExtentionArea->m_AuthorName, sizeof(m_pTgaExtentionArea->m_AuthorName), tempStrA.c_str());
                    HasExt = true;
                    }


                // AuthorComment Tag
                if (pTag->GetID() == HRFAttributeNotes::ATTRIBUTE_ID)
                    {
                    AString tempStrA;
                    BeStringUtilities::WCharToCurrentLocaleChar(tempStrA, ((HFCPtr<HRFAttributeNotes>&)pTag)->GetData().c_str());
                    BeStringUtilities::Strncpy((char*)m_pTgaExtentionArea->m_AuthorComments, sizeof(m_pTgaExtentionArea->m_AuthorComments), tempStrA.c_str());

                    HasExt = true;
                    }

                // Date/Time Tag
                if (pTag->GetID() == HRFAttributeDateTime::ATTRIBUTE_ID)
                    {
                    BE_STRING_UTILITIES_SWSCANF (((HFCPtr<HRFAttributeDateTime>&)pTag)->GetData().c_str(),
                                                L"%hu/%hu/%hu %hu:%hu:%hu",
                                                &m_pTgaExtentionArea->m_Year,
                                                &m_pTgaExtentionArea->m_Month,
                                                &m_pTgaExtentionArea->m_Day,
                                                &m_pTgaExtentionArea->m_Hour,
                                                &m_pTgaExtentionArea->m_Minute,
                                                &m_pTgaExtentionArea->m_Second);
                    m_pTgaExtentionArea->m_Year = (unsigned short)BeStringUtilities::Wtoi (((HFCPtr<HRFAttributeDateTime>&)pTag)->GetData().c_str());
                    HasExt = true;
                    }

                // JobNameId Tag
                //                if (pTag->GetLabel() == HRF_TGA_UNIFORM_TAGLABEL_JOBNAMEID
                //                {
                //                    strcpy((char*)m_pTgaExtentionArea->m_JobNameId, ((HFCPtr<HPMAttribute<string> >&)pTag)->GetData().c_str());
                //                    HasExt = true;
                //                }

                // JobTime Tag
                //                if (pTag->GetLabel() == HRF_TGA_UNIFORM_TAGLABEL_JOBTIME
                //                {
                //                    sscanf (((HFCPtr<HPMAttribute<string> >&)pTag)->GetData().c_str(),
                //                            "%u:%u:%u",
                //                            m_pTgaExtentionArea->m_JobHours,
                //                            m_pTgaExtentionArea->m_JobMinutes,
                //                            m_pTgaExtentionArea->m_JobSeconds);
                //                    m_pTgaExtentionArea->m_JobHours = atoi (((HFCPtr<HPMAttribute<string> >&)pTag)->GetData().c_str());
                //                    HasExt = true;
                //                }

                // SoftwareId Tag
                if (pTag->GetID() == HRFAttributeSoftware::ATTRIBUTE_ID)
                    {
                    AString softNameA;
                    BeStringUtilities::WCharToCurrentLocaleChar(softNameA, ((HFCPtr<HRFAttributeSoftware>&)pTag)->GetData().c_str());

                    BeStringUtilities::Strncpy((char*)m_pTgaExtentionArea->m_SoftwareId, sizeof(m_pTgaExtentionArea->m_SoftwareId), softNameA.c_str());
                    HasExt = true;
                    }

                // SoftwareVersion Tag
                if (pTag->GetID() == HRFAttributeVersion::ATTRIBUTE_ID)
                    {
                    float temp;
                    BE_STRING_UTILITIES_SWSCANF (((HFCPtr<HRFAttributeVersion>&)pTag)->GetData().c_str(),
                                                L"%f%hc",
                                                &temp,
                                                &m_pTgaExtentionArea->m_SoftwareVersionLetter);
                    //temp = (float)atof (((HFCPtr<HRFAttributeVersion>&)pTag)->GetData().c_str());
                    m_pTgaExtentionArea->m_SoftwareVersion = (unsigned short)(temp * 100);
                    HasExt = true;
                    }

                // BackGround Color
                if (pTag->GetID() == HRFAttributeBackground::ATTRIBUTE_ID)
                    {
                    m_pTgaExtentionArea->m_BackgroundColor = ((HFCPtr<HRFAttributeBackground>&)pTag)->GetData();
                    HasExt = true;
                    }

                // AspectRationNum
                //                if (pTag->GetLabel() == HRF_TGA_UNIFORM_TAGLABEL_ASPECT_RATIO_NUM
                //                {
                //                    m_pTgaExtentionArea->m_PixelRatioNum = ((HFCPtr<HPMAttribute<UShort> >&)pTag)->GetData();
                //                    HasExt = true;
                //                }

                // AspectRationDen
                //                if (pTag->GetLabel() == HRF_TGA_UNIFORM_TAGLABEL_ASPECT_RATIO_DEN
                //                {
                //                    m_pTgaExtentionArea->m_PixelRatioDen = ((HFCPtr<HPMAttribute<UShort> >&)pTag)->GetData();
                //                    HasExt = true;
                //                }

                // GammaNum
                //                if (pTag->GetLabel() == HRF_TGA_UNIFORM_TAGLABEL_GAMMA_NUM
                //                {
                //                    m_pTgaExtentionArea->m_GammaNum = ((HFCPtr<HPMAttribute<UShort> >&)pTag)->GetData();
                //                    HasExt = true;
                //                }

                // GammaDen
                //                if (pTag->GetLabel() == HRF_TGA_UNIFORM_TAGLABEL_GAMMA_DEN
                //                {
                //                    m_pTgaExtentionArea->m_GammaDen = ((HFCPtr<HPMAttribute<UShort> >&)pTag)->GetData();
                //                    HasExt = true;
                //                }
                }
            }

        // Update the thumbnail
        if (pPageDescriptor->ThumbnailHasChanged())
            if (SetThumbnailToFile())
                HasExt = true;

        //** Update the ColorCorrectionTable
        //** Not supported


        if ((m_pTgaExtTableArea != 0) && (m_pTgaExtTableArea->m_pScanLineTable != 0))
            HasExt = true;

        if (HasFtr)
            SetFileFooterToFile(HasExt);

        if (IsCreate && IsCompressed() && m_pTgaFile->IsCompatibleWith(HFCLocalBinStream::CLASS_ID))
            ((HFCPtr<HFCLocalBinStream>&)m_pTgaFile)->SetEOF();

        if(pi_CloseFile)
            {
            // Unlock the sister file.
            SisterFileLock.ReleaseKey();

            m_IsOpen = false;
            }
        else
            {
            m_pTgaFile->Flush();
            }


        }
    }

/**-----------------------------------------------------------------------------
 This method creates an editor for the specified page and resolution. The
 specified access mode will determine whether the editor will read, write or
 both.

 @param pi_Page The number of the page descriptor.
 @param pi_Resolution The number of the resolution descriptor.
 @param pi_AccessMode The access and sharing mode of the opened file.
------------------------------------------------------------------------------*/
HRFResolutionEditor* HRFTgaFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                        unsigned short pi_Resolution,
                                                        HFCAccessMode  pi_AccessMode)
    {
    HRFBlockType BlockType = GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution)->GetBlockType();
    if (BlockType == HRFBlockType::IMAGE)
        {
        return new HRFTgaCompressImageEditor(this, pi_Page, pi_Resolution, pi_AccessMode);
        }
    else
        {
        if (IsCompressed())
            return new HRFTgaCompressLineEditor(this, pi_Page, pi_Resolution, pi_AccessMode);
        else
            return new HRFTgaLineEditor(this, pi_Page, pi_Resolution, pi_AccessMode);
        }
    }

/**-----------------------------------------------------------------------------
Saves the file
------------------------------------------------------------------------------*/
void HRFTgaFile::Save()
    {
    //Keep last file position
    uint64_t CurrentPos = m_pTgaFile->GetCurrentPos();

    SaveTgaFile(false);

    //Set back position
    m_pTgaFile->SeekToPos(CurrentPos);
    }


/**-----------------------------------------------------------------------------
 Adds a new page to the raster file. This function can be called only if the
 file access mode has create access.

 @param pi_pPage A pointer on the page descriptor to be added.

 @return bool true if the page has been added.
------------------------------------------------------------------------------*/
bool HRFTgaFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    // Assert that no page has allready be entered
    HPRECONDITION (CountPages() == 0);
    HPRECONDITION (pi_pPage != 0);


    // Add the page descriptor to the list
    HRFRasterFile::AddPage(pi_pPage);

    m_pTgaFileHeader    = new TgaFileHeader;
    m_pTgaImageData     = new TgaImageData;
    m_pTgaFileFooter    = new TgaFileFooter;
    m_pTgaExtentionArea = new TgaExtensionArea;
    m_pTgaExtTableArea  = new TgaExtTableArea;

    // Lock the sister file.
    HFCLockMonitor SisterFileLock (GetLockManager());

    // make sure that the pixel depth is initialized;
    m_pTgaFileHeader->m_PixelDepth = 0;

    SetFileHeaderToFile();

    // Unlock the sister file.
    SisterFileLock.ReleaseKey();

    InitializeFileFooter();

    return true;
    }

/**-----------------------------------------------------------------------------
 Private
 Find and create pixel type from file.

 @return HFCPtr<HRPPixelType> The pixel type detected from the file.
------------------------------------------------------------------------------*/
HFCPtr<HRPPixelType> HRFTgaFile::CreatePixelTypeFromFile() const
    {
    HPRECONDITION (m_pTgaFile != 0);
    HPRECONDITION (m_pTgaFileHeader != 0);

    uint32_t                 i;
    uint32_t                 j;
    Byte                  ColorMapEntrySizeInByte;
    Byte*                 pValue;
    HFCPtr<HRPPixelType>    pPixelType = 0;
    HRPPixelPalette*        pPalette;

    // If there is no color-map
    // Parse the image type
    switch (m_pTgaFileHeader->m_ImageType)
        {
        case 2 :
        case 10 :
            // True color 16, 24 or 32
            if ((m_pTgaFileHeader->m_PixelDepth == 16) || (m_pTgaFileHeader->m_PixelDepth == 24))
                pPixelType = new HRPPixelTypeV24R8G8B8();
            else if (m_pTgaFileHeader->m_PixelDepth == 32)
                pPixelType = new HRPPixelTypeV32R8G8B8A8();
            break;
        case 3 :
        case 11 :
            // Grayscale 8
            if (m_pTgaFileHeader->m_PixelDepth == 8)
                pPixelType = new HRPPixelTypeV8Gray8();
            break;
        case 1:
        case 9:
            // ColorMap 16, 24 or 32
            if (m_pTgaFileHeader->m_ColorMapType == 1)
                {
                ColorMapEntrySizeInByte = DIVROUNDUP (m_pTgaFileHeader->m_ColorMapEntrySize, 8);

                // If the pixel type is I8B5G5R5, we must consider it as a 3 bytes value.
                if (ColorMapEntrySizeInByte == 2)
                    ColorMapEntrySizeInByte++;

                // Creation of the pixeltype for the palette
                if (3 == ColorMapEntrySizeInByte)
                    pPixelType = new HRPPixelTypeI8R8G8B8();
                else
                    pPixelType = new HRPPixelTypeI8R8G8B8A8();

                // Get a reference of the palette of the pixel type
                pPalette = (HRPPixelPalette*)&(pPixelType->GetPalette());

                pValue = new Byte[ColorMapEntrySizeInByte];

                // Copy the TGA palette into the pixel palette
                for (i = 0, j = 0; i < m_pTgaFileHeader->m_ColorMapLength; i++, j += ColorMapEntrySizeInByte)
                    {
                    memcpy (pValue, &(m_pTgaImageData->m_pColorMap[j]), ColorMapEntrySizeInByte);

                    // Add the entry to the pixel palette
                    pPalette->SetCompositeValue (i, pValue);
                    }

                delete[] pValue;
                }
            break;
        }

    if (pPixelType == 0)
        throw HRFPixelTypeNotSupportedException(GetURL()->GetURL());

    return pPixelType;
    }

/**-----------------------------------------------------------------------------
 Private
 Find and create codec type from file.

 @return HFCPtr<HCDCodec> The codec detected from the file.
------------------------------------------------------------------------------*/
HFCPtr<HCDCodec> HRFTgaFile::CreateCodecFromFile() const
    {
    HFCPtr<HCDCodec>  pCodec;

    switch (m_pTgaFileHeader->m_ImageType)
        {
        case 1 :
        case 2 :
        case 3 :
            // No compression in data
            pCodec = new HCDCodecIdentity();
            break;
        case 9 :
        case 10 :
        case 11 :
            // Data are RLE encoded
            pCodec = new HCDCodecTGARLE();
            break;
        default :
            //All unsupported image types should be filtered out by the IsKindOf function
            HASSERT(0);
        }

    return pCodec;
    }

/**-----------------------------------------------------------------------------
 Private
 Read the file Header from the file
------------------------------------------------------------------------------*/
void HRFTgaFile::GetFileHeaderFromFile()
    {
    HPRECONDITION (m_pTgaFile != 0);
    HPRECONDITION (SharingControlIsLocked());

    m_pTgaFile->SeekToPos(0);
    m_pTgaFileHeader = new HRFTgaFile::TgaFileHeader;
    if (sizeof(HRFTgaFile::TgaFileHeader) != m_pTgaFile->Read(m_pTgaFileHeader, sizeof(HRFTgaFile::TgaFileHeader)))
        throw HFCCorruptedFileException(m_pURL->GetURL());

    m_RasterDataOffset = sizeof(HRFTgaFile::TgaFileHeader);
    m_RasterDataEndOffset = (uint32_t)m_pTgaFile->GetSize();
    }

/**-----------------------------------------------------------------------------
 Private
 Read the file footer from the file
------------------------------------------------------------------------------*/
void HRFTgaFile::GetFileFooterFromFile()
    {
    HPRECONDITION (m_pTgaFile != 0);
    HPRECONDITION (SharingControlIsLocked());

    // Seek for 26 caracters from the end of the file.
    m_pTgaFile->SeekToPos(m_pTgaFile->GetSize() - (long)(sizeof(HRFTgaFile::TgaFileFooter)));

    m_pTgaFileFooter = new TgaFileFooter;

    if (sizeof(TgaFileFooter) != m_pTgaFile->Read(m_pTgaFileFooter, sizeof(TgaFileFooter)))
        throw HFCCorruptedFileException(m_pURL->GetURL());

    // If the file footer area is not present, we will not find the standard signature
    if (0 != (strcmp ((const char*)m_pTgaFileFooter->m_Signature, TGA_SIGNATURE)))
        m_pTgaFileFooter = 0;
    else
        {
        if (m_pTgaFileFooter->m_DeveloperDirectoryOffset != 0)
            m_RasterDataEndOffset = m_pTgaFileFooter->m_DeveloperDirectoryOffset;
        else
            m_RasterDataEndOffset -= sizeof (TgaFileFooter);
        }
    }

/**----------------------------------------------------------------------------
 Private
 Read the file color map info section from the file.
-----------------------------------------------------------------------------*/
void HRFTgaFile::GetMapInfoFromFile()
    {
    HPRECONDITION (m_pTgaFile != 0);
    HPRECONDITION (SharingControlIsLocked());

    // Create the ImageData Structure
    m_pTgaImageData = new TgaImageData;

    // Set the position of the file pointer
    m_pTgaFile->SeekToPos (sizeof (TgaFileHeader));

    // Read the Image ID
    if (0 != m_pTgaFileHeader->m_IdLength)
        {
        m_pTgaImageData->m_pImageId = new char[m_pTgaFileHeader->m_IdLength+1];
        m_pTgaImageData->m_pImageId[m_pTgaFileHeader->m_IdLength] = 0x00;

        if (0 == m_pTgaFile->Read (m_pTgaImageData->m_pImageId, m_pTgaFileHeader->m_IdLength))
            throw HFCCorruptedFileException(m_pURL->GetURL());

        m_RasterDataOffset += m_pTgaFileHeader->m_IdLength;
        }
    else
        m_pTgaImageData->m_pImageId = 0;


    // Read the Color-Map Table
    if (0 != m_pTgaFileHeader->m_ColorMapLength)
        {
        uint32_t                 i;
        uint32_t                 j;
        //UInt32                   ColorMapEntrySizeInByte = DIVROUNDUP(m_pTgaFileHeader->m_ColorMapEntrySize, 8);
        uint32_t                 ColorMapSizeInPixel     = m_pTgaFileHeader->m_ColorMapLength;
        Byte                  PixelSizeInByte;
        Byte                  Swap;
        HArrayAutoPtr<unsigned short>   pBuffer;

        switch (m_pTgaFileHeader->m_ColorMapEntrySize)
            {
            case 15 :
            case 16 :
                {
                PixelSizeInByte = 3;
                pBuffer = new unsigned short[ColorMapSizeInPixel];
                m_pTgaImageData->m_pColorMap = new Byte[ColorMapSizeInPixel * PixelSizeInByte];

                if (sizeof(unsigned short)*ColorMapSizeInPixel != m_pTgaFile->Read (pBuffer, ColorMapSizeInPixel * sizeof(unsigned short)))
                    throw HFCCorruptedFileException(m_pURL->GetURL());

                // Conversion
                for (i = 0, j = 0; i < ColorMapSizeInPixel; i++)
                    {
                    m_pTgaImageData->m_pColorMap[j++] = CONVERT_TO_BYTE(((pBuffer[i] & 0x7C00) >> 10) * 0xFF / 0x1F);
                    m_pTgaImageData->m_pColorMap[j++] = CONVERT_TO_BYTE(((pBuffer[i] & 0x3E0) >> 5)   * 0xFF / 0x1F);
                    m_pTgaImageData->m_pColorMap[j++] = CONVERT_TO_BYTE((pBuffer[i] & 0x1F)          * 0xFF / 0x1F);
                    }
                break;
                }
            case 24 :
            case 32 :
                PixelSizeInByte = m_pTgaFileHeader->m_ColorMapEntrySize / 8;
                m_pTgaImageData->m_pColorMap = new Byte[ColorMapSizeInPixel * PixelSizeInByte];

                if ((ColorMapSizeInPixel * PixelSizeInByte) != m_pTgaFile->Read (m_pTgaImageData->m_pColorMap, ColorMapSizeInPixel * PixelSizeInByte))
                    throw HFCCorruptedFileException(m_pURL->GetURL());

                // Conversion
                for (i = 0, j = 0; i < ColorMapSizeInPixel; i++, j += PixelSizeInByte)
                    {
                    Swap = m_pTgaImageData->m_pColorMap[j];
                    m_pTgaImageData->m_pColorMap[j] = m_pTgaImageData->m_pColorMap[j+2];
                    m_pTgaImageData->m_pColorMap[j+2] = Swap;
                    }
                break;
            default :
                throw HFCCorruptedFileException(m_pURL->GetURL());
            }

        m_RasterDataOffset += ColorMapSizeInPixel * DIVROUNDUP (m_pTgaFileHeader->m_ColorMapEntrySize, 8);
        }
    else
        m_pTgaImageData->m_pColorMap = 0;

    }

/**----------------------------------------------------------------------------
 Private
 Read the extention area data from the file.
-----------------------------------------------------------------------------*/
void HRFTgaFile::GetExtensionAreaFromFile()
    {
    HPRECONDITION (m_pTgaFile != 0);
    HPRECONDITION (SharingControlIsLocked());

    if ((m_pTgaFileFooter != 0) && (0 != m_pTgaFileFooter->m_ExtensionAreaOffset))
        {
        if (m_pTgaFileFooter->m_ExtensionAreaOffset < m_RasterDataEndOffset)
            m_RasterDataEndOffset = m_pTgaFileFooter->m_ExtensionAreaOffset;

        m_pTgaExtentionArea = new TgaExtensionArea;
        m_pTgaFile->SeekToPos(m_pTgaFileFooter->m_ExtensionAreaOffset);
        if (sizeof (TgaExtensionArea) != m_pTgaFile->Read(m_pTgaExtentionArea, sizeof (TgaExtensionArea)))
            throw HFCCorruptedFileException(GetURL()->GetURL());

        m_pTgaExtTableArea = new TgaExtTableArea;

        // Read the Scan Line Table
        if (0 != m_pTgaExtentionArea->m_ScanLineOffset)
            {
            m_pTgaExtTableArea->m_pScanLineTable = new uint32_t[m_pTgaFileHeader->m_ImageHeight];
            m_pTgaFile->SeekToPos(m_pTgaExtentionArea->m_ScanLineOffset);
            if ((m_pTgaFileHeader->m_ImageHeight * 4) != m_pTgaFile->Read(m_pTgaExtTableArea->m_pScanLineTable, m_pTgaFileHeader->m_ImageHeight * 4))
                throw HFCCorruptedFileException(GetURL()->GetURL());

            if (m_pTgaExtentionArea->m_ScanLineOffset < m_RasterDataEndOffset)
                m_RasterDataEndOffset = m_pTgaExtentionArea->m_ScanLineOffset;
            }
        else
            m_pTgaExtTableArea->m_pScanLineTable = 0;

        // Read the thumbnail Raster Data
        if (0 != m_pTgaExtentionArea->m_PostageStampOffset)
            {
            uint32_t                 i;
            uint32_t                 j;
            uint32_t                 StampSizeInPixel;
            Byte                  PixelSizeInByte;
            Byte                  Swap;
            HArrayAutoPtr<unsigned short>   pBuffer;

            m_pTgaFile->SeekToPos (m_pTgaExtentionArea->m_PostageStampOffset);
            m_pTgaFile->Read (&m_pTgaExtTableArea->m_StampWidth, sizeof(unsigned char));
            m_pTgaFile->Read (&m_pTgaExtTableArea->m_StampHeight, sizeof(unsigned char));

            StampSizeInPixel = m_pTgaExtTableArea->m_StampWidth * m_pTgaExtTableArea->m_StampHeight;

            switch (m_pTgaFileHeader->m_PixelDepth)
                {
                case 8 :
                    m_pTgaExtTableArea->m_pStampData = new Byte[StampSizeInPixel];
                    if (StampSizeInPixel != m_pTgaFile->Read (m_pTgaExtTableArea->m_pStampData, StampSizeInPixel))
                        throw HFCCorruptedFileException(GetURL()->GetURL());

                    // No Conversion
                    break;
                case 15 :
                case 16 :
                    pBuffer = new unsigned short[StampSizeInPixel];
                    m_pTgaExtTableArea->m_pStampData = new Byte[StampSizeInPixel * 3];

                    if ((StampSizeInPixel * sizeof(unsigned short)) != m_pTgaFile->Read (pBuffer, StampSizeInPixel * sizeof(unsigned short)))
                        throw HFCCorruptedFileException(GetURL()->GetURL());

                    // Conversion
                    for (i = 0, j = 0; i < StampSizeInPixel; i++)
                        {
                        m_pTgaExtTableArea->m_pStampData[j++] = CONVERT_TO_BYTE(((pBuffer[i] & 0x7C00) >> 10) * 0xFF / 0x1F);
                        m_pTgaExtTableArea->m_pStampData[j++] = CONVERT_TO_BYTE(((pBuffer[i] & 0x3E0) >> 5)   * 0xFF / 0x1F);
                        m_pTgaExtTableArea->m_pStampData[j++] = CONVERT_TO_BYTE((pBuffer[i] & 0x1F)          * 0xFF / 0x1F);
                        }
                    break;
                case 24 :
                case 32 :
                    PixelSizeInByte = m_pTgaFileHeader->m_PixelDepth / 8;
                    m_pTgaExtTableArea->m_pStampData = new Byte[StampSizeInPixel * PixelSizeInByte];

                    if ((StampSizeInPixel * PixelSizeInByte) != m_pTgaFile->Read (m_pTgaExtTableArea->m_pStampData, StampSizeInPixel * PixelSizeInByte))
                        throw HFCCorruptedFileException(GetURL()->GetURL());

                    // Conversion
                    for (i = 0, j = 0; i < StampSizeInPixel; i++, j += PixelSizeInByte)
                        {
                        Swap = m_pTgaExtTableArea->m_pStampData[j];
                        m_pTgaExtTableArea->m_pStampData[j] = m_pTgaExtTableArea->m_pStampData[j+2];
                        m_pTgaExtTableArea->m_pStampData[j+2] = Swap;
                        }
                    break;
                default :
                    throw HFCCorruptedFileException(GetURL()->GetURL());

                }

            if (m_pTgaExtentionArea->m_PostageStampOffset < m_RasterDataEndOffset)
                m_RasterDataEndOffset = m_pTgaExtentionArea->m_PostageStampOffset;
            }

        // Read the color corection table
        if (0 != m_pTgaExtentionArea->m_ColorCorrectionOffset)
            {
            m_pTgaFile->SeekToPos(m_pTgaExtentionArea->m_ColorCorrectionOffset);
            m_pTgaExtTableArea->m_pColorCorrectionTable = new unsigned short[COLOR_CORECTION_TABLE_ENTRY_SIZE];
            if ((COLOR_CORECTION_TABLE_ENTRY_SIZE*2) != m_pTgaFile->Read(m_pTgaExtTableArea->m_pColorCorrectionTable,
                                                                         COLOR_CORECTION_TABLE_ENTRY_SIZE * 2))
                throw HFCCorruptedFileException(GetURL()->GetURL());

            if (m_pTgaExtentionArea->m_ColorCorrectionOffset < m_RasterDataEndOffset)
                m_RasterDataEndOffset = m_pTgaExtentionArea->m_ColorCorrectionOffset;
            }
        }
    else
        {
        m_pTgaExtentionArea = 0;
        m_pTgaExtTableArea  = 0;
        }
    }

/**-----------------------------------------------------------------------------
 This method create a new file on the disk and keep a pointer to it.

 @return true If the file is instanciate correctly.
------------------------------------------------------------------------------*/
bool HRFTgaFile::Create()
    {
    // Open the file
    m_pTgaFile = HFCBinStream::Instanciate(GetURL(), GetAccessMode(), 0, true);

    // Instanciate the SharingControl object for file sharing
    SharingControlCreate();

    m_IsOpen = true;

    return m_IsOpen;
    }

/**-----------------------------------------------------------------------------
 This method return true if the file has a palette.

 @return true If the file has a color palette.
------------------------------------------------------------------------------*/
bool HRFTgaFile::HasPalette() const
    {
    HPRECONDITION (CountPages() > 0);
    HPRECONDITION (GetPageDescriptor(0)->CountResolutions() > 0);

    if ((GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetPixelType()->IsCompatibleWith(HRPPixelTypeI8R8G8B8::CLASS_ID)) ||
        (GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetPixelType()->IsCompatibleWith(HRPPixelTypeI8R8G8B8A8::CLASS_ID)))
        return true;
    else
        return false;
    }

/**-----------------------------------------------------------------------------
 This method return true if the raster data are compressed.

 @return true If the file is compressed.
------------------------------------------------------------------------------*/
bool HRFTgaFile::IsCompressed() const
    {
    HPRECONDITION (CountPages() == 1);
    HPRECONDITION (GetPageDescriptor(0)->CountResolutions() > 0);

    if (GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetCodec()->IsCompatibleWith(HCDCodecIdentity::CLASS_ID))
        return false;
    else
        return true;
    }

/**-----------------------------------------------------------------------------
 This method write the header part to the file
------------------------------------------------------------------------------*/
void HRFTgaFile::SetFileHeaderToFile()
    {
    HPRECONDITION (m_pTgaFile != 0);
    HPRECONDITION (SharingControlIsLocked());
    HPRECONDITION (GetAccessMode().m_HasWriteAccess || GetAccessMode().m_HasCreateAccess);
    HPRECONDITION (CountPages() > 0);

    HFCPtr<HRFPageDescriptor>       pPageDescriptor = GetPageDescriptor(0);

    HASSERT (pPageDescriptor->CountResolutions() > 0);

    HFCPtr<HRFResolutionDescriptor> pResolutionDescriptor = pPageDescriptor->GetResolutionDescriptor(0);
    HRPPixelPalette                 Palette;

    if (GetAccessMode().m_HasCreateAccess)
        {
        m_pTgaFileHeader->m_IdLength    = 0;               // Could be improve
        m_pTgaFileHeader->m_XOrigin     = 0;               // Could be improve
        m_pTgaFileHeader->m_YOrigin     = 0;               // Could be improve
        m_pTgaFileHeader->m_ImageWidth  = (unsigned short)pResolutionDescriptor->GetWidth();
        m_pTgaFileHeader->m_ImageHeight = (unsigned short)pResolutionDescriptor->GetHeight();

        m_pTgaFileHeader->m_PixelDepth  = (Byte)pResolutionDescriptor->GetBitsPerPixel();
        m_pTgaFileHeader->m_ImageDescriptor = 0x20;        // Origin is top-left

        // Set the palette informations
        HFCPtr<HRPPixelType> pPixelType = pResolutionDescriptor->GetPixelType();
        if (pPixelType->IsCompatibleWith(HRPPixelTypeI8R8G8B8::CLASS_ID))
            {
            Palette                   = pPixelType->GetPalette();
            m_pTgaFileHeader->m_ColorMapType = 1;
            m_pTgaFileHeader->m_ColorMapFirstEntryIndex = 0;
            m_pTgaFileHeader->m_ColorMapLength          = (unsigned short)Palette.CountUsedEntries();
            m_pTgaFileHeader->m_ColorMapEntrySize       = 24;
            }
        else
            {
            if (pPixelType->IsCompatibleWith(HRPPixelTypeI8R8G8B8A8::CLASS_ID))
                {
                Palette                              = pPixelType->GetPalette();
                m_pTgaFileHeader->m_ColorMapType            = 1;
                m_pTgaFileHeader->m_ColorMapFirstEntryIndex = 0;
                m_pTgaFileHeader->m_ColorMapLength          = (unsigned short)Palette.CountUsedEntries();
                m_pTgaFileHeader->m_ColorMapEntrySize       = 32;
                }
            else
                {
                m_pTgaFileHeader->m_ColorMapType            = 0;
                m_pTgaFileHeader->m_ColorMapFirstEntryIndex = 0;
                m_pTgaFileHeader->m_ColorMapLength          = 0;
                m_pTgaFileHeader->m_ColorMapEntrySize       = 0;

                // tha alpha channel bit was enable only with V32R8G8B8
                if (pPixelType->IsCompatibleWith(HRPPixelTypeV32R8G8B8A8::CLASS_ID))
                    m_pTgaFileHeader->m_ImageDescriptor |= 0x08;   // set the alpha channel bit to 8
                }
            }

        // Set the image type
        if (HasPalette())
            {
            if (IsCompressed())                     // Compressed palette
                m_pTgaFileHeader->m_ImageType = 9;
            else                                    // Uncompressed palette
                m_pTgaFileHeader->m_ImageType = 1;
            }
        else
            {
            if (!IsCompressed())
                {
                if (8 == m_pTgaFileHeader->m_PixelDepth)   // Uncompressed black and white
                    m_pTgaFileHeader->m_ImageType = 3;
                else
                    m_pTgaFileHeader->m_ImageType = 2;     // Uncompressed true-color
                }
            else
                {
                if (8 == m_pTgaFileHeader->m_PixelDepth)   // Compressed black and white
                    m_pTgaFileHeader->m_ImageType = 11;
                else
                    m_pTgaFileHeader->m_ImageType = 10;    // Compressed true-color
                }
            }
        }
    else if (GetAccessMode().m_HasWriteAccess)
        {
        if (IsCompressed())
            {
            m_pTgaFileHeader->m_ImageDescriptor = 0x20;

            // tha alpha channel bit was enable only with V32R8G8B8A8
            if (pResolutionDescriptor->GetPixelType()->GetClassID() == HRPPixelTypeV32R8G8B8A8::CLASS_ID)
                m_pTgaFileHeader->m_ImageDescriptor |= 0x08;   // set the alpha channel bit to 8
            }
        }

    // Write header
    m_pTgaFile->SeekToPos(0);
    m_pTgaFile->Write (m_pTgaFileHeader, sizeof(TgaFileHeader));
    m_RasterDataOffset = sizeof(TgaFileHeader);

    // Write the image ID
    // Only in case of rewriting... New created files do not support this section.
    if (0 != m_pTgaFileHeader->m_IdLength)
        {
        m_pTgaFile->Write (m_pTgaImageData->m_pImageId, m_pTgaFileHeader->m_IdLength);
        m_RasterDataOffset += m_pTgaFileHeader->m_IdLength;
        }

    // Reserve space for the image palette
    if (HasPalette())
        {
        m_RasterDataOffset += sizeof(unsigned char) *
                              m_pTgaFileHeader->m_ColorMapLength *
                              DIVROUNDUP (m_pTgaFileHeader->m_ColorMapEntrySize, 8);

        // Write the current palette
        SetPaletteToFile();
        }

    // Ready to write the raster data...
    }

/**-----------------------------------------------------------------------------
 Private
 This method write the footer part to the file

 @param pi_HasExt Tells the method if it shall write the extention area or not.
------------------------------------------------------------------------------*/
void HRFTgaFile::SetFileFooterToFile(bool pi_HasExt)
    {
    HPRECONDITION (m_pTgaFile != 0);
    HPRECONDITION (SharingControlIsLocked());
    HPRECONDITION (CountPages() > 0);
    HPRECONDITION (GetPageDescriptor(0)->CountResolutions() > 0);

    HFCPtr<HRFResolutionDescriptor> pResolutionDescriptor     = GetPageDescriptor(0)->GetResolutionDescriptor(0);
    uint32_t                        Offset;

    if (IsCompressed())
        Offset = m_RasterDataEndOffset;
    else
        Offset = m_RasterDataOffset + (DIVROUNDUP(m_pTgaFileHeader->m_PixelDepth, 8) * (uint32_t)pResolutionDescriptor->GetWidth() * (uint32_t)pResolutionDescriptor->GetHeight());

    // Place the file pointer at the end of the data
    m_pTgaFile->SeekToPos (Offset);

    // Set the Extension offsets and write it to the file
    if (pi_HasExt)
        {
        if (m_pTgaExtTableArea->m_pStampData != 0)
            {
            m_pTgaExtentionArea->m_PostageStampOffset = Offset;
            Offset += m_pTgaExtTableArea->m_StampWidth * m_pTgaExtTableArea->m_StampHeight * DIVROUNDUP(m_pTgaFileHeader->m_PixelDepth,8);
            Offset += 2;
            }

        m_pTgaFileFooter->m_ExtensionAreaOffset = Offset;
        Offset += m_pTgaExtentionArea->m_ExtensionSize;

        if (m_pTgaExtTableArea->m_pScanLineTable != 0)
            {
            m_pTgaExtentionArea->m_ScanLineOffset = Offset;
            Offset += m_pTgaFileHeader->m_ImageHeight * sizeof(uint32_t);
            }

        if (m_pTgaExtTableArea->m_pColorCorrectionTable != 0)
            m_pTgaExtentionArea->m_ColorCorrectionOffset = Offset;
        }


    if (m_pTgaExtentionArea && m_pTgaExtentionArea->m_PostageStampOffset != 0)
        {
        m_pTgaFile->Write (&m_pTgaExtTableArea->m_StampWidth, sizeof (Byte));
        m_pTgaFile->Write (&m_pTgaExtTableArea->m_StampHeight, sizeof (Byte));
        m_pTgaFile->Write (m_pTgaExtTableArea->m_pStampData, m_pTgaExtTableArea->m_StampWidth *
                           m_pTgaExtTableArea->m_StampHeight *
                           DIVROUNDUP (m_pTgaFileHeader->m_PixelDepth, 8));
        }

    if (m_pTgaFileFooter->m_ExtensionAreaOffset != 0)
        m_pTgaFile->Write (m_pTgaExtentionArea, m_pTgaExtentionArea->m_ExtensionSize);

    if (m_pTgaExtentionArea && m_pTgaExtentionArea->m_ScanLineOffset != 0)
        m_pTgaFile->Write (m_pTgaExtTableArea->m_pScanLineTable, m_pTgaFileHeader->m_ImageHeight * 4);

    if (m_pTgaExtentionArea && m_pTgaExtentionArea->m_ColorCorrectionOffset != 0)
        m_pTgaFile->Write (m_pTgaExtTableArea->m_pColorCorrectionTable, 2048);

    m_pTgaFile->Write (m_pTgaFileFooter, sizeof(TgaFileFooter));
    }

/**-----------------------------------------------------------------------------
 Private
 This method write the palette to the file.
------------------------------------------------------------------------------*/
void HRFTgaFile::SetPaletteToFile()
    {
    HPRECONDITION (m_pTgaFile != 0);
    HPRECONDITION (SharingControlIsLocked());
    HPRECONDITION (CountPages() > 0);
    HPRECONDITION (GetPageDescriptor(0)->CountResolutions() > 0);

    uint32_t                Offset;
    Byte                  Swap;
    Byte                  BytesPerPixel;
    const HRPPixelPalette&  rPalette        = GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetPixelType()->GetPalette();

    if ((m_pTgaFileHeader->m_ColorMapEntrySize == 24) || (m_pTgaFileHeader->m_ColorMapEntrySize == 32))
        {
        uint32_t i;
        uint32_t j;

        BytesPerPixel = DIVROUNDUP (m_pTgaFileHeader->m_ColorMapEntrySize, 8);
        m_pTgaImageData->m_pColorMap = new Byte [BytesPerPixel * m_pTgaFileHeader->m_ColorMapLength];

        // Convert from I8R8G8B8 to I8B8G8R8 or from I8R8G8B8A8 TO I8B8G8R8A8
        for (i = 0, j = 0; i < m_pTgaFileHeader->m_ColorMapLength; i++, j += BytesPerPixel)
            {
            if (i < rPalette.CountUsedEntries())
                {
                memcpy (m_pTgaImageData->m_pColorMap+j, (Byte*)rPalette.GetCompositeValue(i), BytesPerPixel);
                Swap = m_pTgaImageData->m_pColorMap[j];
                m_pTgaImageData->m_pColorMap[j] = m_pTgaImageData->m_pColorMap[j+2];
                m_pTgaImageData->m_pColorMap[j+2] = Swap;
                }
            else
                {
                memset (m_pTgaImageData->m_pColorMap+j, i, BytesPerPixel);
                }
            }
        }
    else
        throw HFCCorruptedFileException(m_pURL->GetURL());

    Offset = sizeof(TgaFileHeader) + m_pTgaFileHeader->m_IdLength;
    m_pTgaFile->SeekToPos (Offset);
    m_pTgaFile->Write (m_pTgaImageData->m_pColorMap, sizeof(Byte) *
                       m_pTgaFileHeader->m_ColorMapLength *
                       DIVROUNDUP (m_pTgaFileHeader->m_ColorMapEntrySize,8));
    }

/**-----------------------------------------------------------------------------
 Private
 This method set the thumbnail from the page descriptor to the file.
------------------------------------------------------------------------------*/
bool HRFTgaFile::SetThumbnailToFile()
    {
    HPRECONDITION (SharingControlIsLocked());
    HPRECONDITION (CountPages() > 0);

    HFCPtr<HRFPageDescriptor>   pPageDescriptor = GetPageDescriptor(0);

    HASSERT (pPageDescriptor->CountResolutions() > 0);

    const HFCPtr<HRPPixelType>&    pPixelType      = pPageDescriptor->GetResolutionDescriptor(0)->GetPixelType();
    const HFCPtr<HRFThumbnail>& pThumbnail      = pPageDescriptor->GetThumbnail();
    uint32_t                    Width           = pThumbnail->GetWidth();
    uint32_t                    Height          = pThumbnail->GetHeight();
    uint32_t                    NbPixel         = Width * Height;
    unsigned short              BytesPerPixel;
    Byte                      Swap;
    bool                       Result          = true;

    // Validate that the pixeltype of the thumbnail is the same as the pixeltype of the image
    if (pThumbnail->GetPixelType() != pPixelType)
        Result = false;
    else
        {
        if (m_pTgaExtTableArea == 0)
            {
            m_pTgaExtentionArea = new TgaExtensionArea;
            m_pTgaExtTableArea = new TgaExtTableArea;
            InitializeFileFooter();
            m_pTgaExtTableArea->m_pStampData = new Byte [pThumbnail->GetBytesPerWidth() * Height];
            }

        m_pTgaExtTableArea->m_StampWidth  = (Byte)Width;
        m_pTgaExtTableArea->m_StampHeight = (Byte)Height;

        switch (m_pTgaFileHeader->m_PixelDepth)
            {
            case 8 :
                m_pTgaExtTableArea->m_pStampData = new Byte [NbPixel];
                pThumbnail->Read(m_pTgaExtTableArea->m_pStampData);
                // No conversion
                break;
            case 24 :
            case 32 :
                {
                uint32_t i;
                uint32_t j;
                BytesPerPixel = m_pTgaFileHeader->m_PixelDepth / 8;
                m_pTgaExtTableArea->m_pStampData = new Byte[NbPixel*BytesPerPixel];
                pThumbnail->Read(m_pTgaExtTableArea->m_pStampData);

                // Conversion from V24R8G8B8 to V24B8G8R8 or
                // Conversion from V32R8G8B8A8 to V32B8G8R8A8
                for (i = 0, j = 0; i < NbPixel; i++, j += BytesPerPixel)
                    {
                    Swap = m_pTgaExtTableArea->m_pStampData[j];
                    m_pTgaExtTableArea->m_pStampData[j] = m_pTgaExtTableArea->m_pStampData[j+2];
                    m_pTgaExtTableArea->m_pStampData[j+2] = Swap;
                    }
                break;
                }
            default :
                throw HFCCorruptedFileException(GetURL()->GetURL());

            }
        }

    return Result;
    }

/**-----------------------------------------------------------------------------
 This function returns the capabilities for the TGA file format

 @return HFCPtr<HRFRasterFileCapabilities> A pointer on the capabilities
         description for TGA file.
------------------------------------------------------------------------------*/
const HFCPtr<HRFRasterFileCapabilities>& HRFTgaFile::GetCapabilities () const
    {
    return (HRFTgaCreator::GetInstance()->GetCapabilities());
    }

/**-----------------------------------------------------------------------------
 Private
 Reset and rewrite the file header.
------------------------------------------------------------------------------*/
void HRFTgaFile::ResetHeader()
    {
    HPRECONDITION (SharingControlIsLocked());

    if ((0x00 == m_pTgaFileHeader->m_ImageDescriptor) || (0x10 == m_pTgaFileHeader->m_ColorMapEntrySize))
        {
        SetFileHeaderToFile();
        }
    }

/**-----------------------------------------------------------------------------
 Initialize the file footer to default values
------------------------------------------------------------------------------*/
void HRFTgaFile::InitializeFileFooter()
    {
    HPRECONDITION (m_pTgaFileFooter != 0);
    HPRECONDITION (m_pTgaExtentionArea != 0);
    HPRECONDITION (m_pTgaExtTableArea != 0);

    time_t    Timer;
    struct tm Gm;

    // Initialise default field of the extension et footer of the file
    m_pTgaFileFooter->m_ExtensionAreaOffset        = 0;
    m_pTgaFileFooter->m_DeveloperDirectoryOffset   = 0;
    strcpy (m_pTgaFileFooter->m_Signature, TGA_SIGNATURE);

    // Extention area table initialization
    m_pTgaExtTableArea->m_pScanLineTable          = 0;
    m_pTgaExtTableArea->m_StampWidth              = 0;
    m_pTgaExtTableArea->m_StampHeight             = 0;
    m_pTgaExtTableArea->m_pStampData              = 0;
    m_pTgaExtTableArea->m_pColorCorrectionTable   = 0;

    // Extension area field initialization
    m_pTgaExtentionArea->m_ExtensionSize = 495;
    memset (m_pTgaExtentionArea->m_AuthorName, 0x00, 41);
    memset (m_pTgaExtentionArea->m_AuthorComments, 0x00, 324);

    // Get the time and hour of the file creation
    time (&Timer);
    Gm = *localtime (&Timer);
    m_pTgaExtentionArea->m_Year                       = (unsigned short)(1900 + Gm.tm_year);
    m_pTgaExtentionArea->m_Month                      = (unsigned short)(1 + Gm.tm_mon);
    m_pTgaExtentionArea->m_Day                        = (unsigned short)Gm.tm_mday;
    m_pTgaExtentionArea->m_Hour                       = (unsigned short)Gm.tm_hour;
    m_pTgaExtentionArea->m_Minute                     = (unsigned short)Gm.tm_min;
    m_pTgaExtentionArea->m_Second                     = (unsigned short)Gm.tm_sec;

    memset (m_pTgaExtentionArea->m_JobNameId, 0x00, 41);

    m_pTgaExtentionArea->m_JobHours                   = 0;
    m_pTgaExtentionArea->m_JobMinutes                 = 0;
    m_pTgaExtentionArea->m_JobSeconds                 = 0;

    memset (m_pTgaExtentionArea->m_SoftwareId, 0x00, 41);
    m_pTgaExtentionArea->m_SoftwareVersion            = 0;
    m_pTgaExtentionArea->m_SoftwareVersionLetter      = 32;
    m_pTgaExtentionArea->m_BackgroundColor            = 0x0000;
    m_pTgaExtentionArea->m_PixelRatioNum              = 0;
    m_pTgaExtentionArea->m_PixelRatioDen              = 0;
    m_pTgaExtentionArea->m_GammaNum                   = 0;
    m_pTgaExtentionArea->m_GammaDen                   = 0;

    m_pTgaExtentionArea->m_ColorCorrectionOffset      = 0;
    m_pTgaExtentionArea->m_PostageStampOffset         = 0;
    m_pTgaExtentionArea->m_ScanLineOffset             = 0;
    if (GetAccessMode().m_HasCreateAccess && (m_pTgaFileHeader->m_PixelDepth == 32))
        m_pTgaExtentionArea->m_AttributesType         = 3;
    else
        m_pTgaExtentionArea->m_AttributesType         = 0;
    }


/**-----------------------------------------------------------------------------
 Check if there are RLE blocks that span across scanlines. If there are,
 the line editor is not available.
------------------------------------------------------------------------------*/
bool HRFTgaFile::RunLengthsSpanScanlines()
    {
    bool Result = false;

    // Lock the sister file
    HFCLockMonitor SisterFileLock(GetLockManager());

    if (SharingControlNeedSynchronization())
        SharingControlSynchronize();

    uint32_t Pos;
    uint32_t NbPixels;
    uint32_t NbLines = 0;
    uint32_t SizeToProcess;
    uint32_t PosFromLastBlock = GetRasterDataOffset();
    uint32_t SizeOfData       = GetRasterDataEndOffset() - GetRasterDataOffset();
    Byte  BytesPerPixel    = (m_pTgaFileHeader->m_PixelDepth + 7) / 8;
    Byte* pBlockOfData;
    uint32_t LastValidPos = PosFromLastBlock;

    if (SizeOfData > BLOCKSIZE)
        pBlockOfData = new Byte [BLOCKSIZE];
    else
        pBlockOfData = new Byte [SizeOfData];

    // Process all the data;
    while (!Result && PosFromLastBlock < GetRasterDataEndOffset() && NbLines < m_pTgaFileHeader->m_ImageHeight)
        {
        SizeToProcess = GetRasterDataEndOffset() - PosFromLastBlock;
        SizeToProcess = SizeToProcess < BLOCKSIZE ? SizeToProcess : BLOCKSIZE;
        m_pTgaFile->SeekToPos (PosFromLastBlock);

        m_pTgaFile->Read (pBlockOfData, SizeToProcess);

        LastValidPos = 0;
        Pos = 0;

        // Process a block
        while (Pos < SizeToProcess && NbLines < m_pTgaFileHeader->m_ImageHeight)
            {
            // Reset Pixels counter.
            NbPixels = 0;

            // Process a line
            while ((NbPixels < m_pTgaFileHeader->m_ImageWidth) && (Pos < SizeToProcess) )
                {
                NbPixels += ((pBlockOfData[Pos] & 0x7f) + 1);
                if (pBlockOfData[Pos] & 0x80)
                    Pos += BytesPerPixel + 1;
                else
                    Pos += (BytesPerPixel * ((pBlockOfData[Pos] & 0x7f) + 1)) + 1;
                }
            //TR # 161740 : We now count the number of lines to know when ends the data
            //HASSERT(PosFromLastBlock + Pos <= GetRasterDataEndOffset());
            if (NbPixels == m_pTgaFileHeader->m_ImageWidth)
                {
                LastValidPos = Pos;
                NbLines++;
                }
            else if (NbPixels > m_pTgaFileHeader->m_ImageWidth)
                {
                Result = true;
                break;  // Out of block loop
                }
            }
        PosFromLastBlock += LastValidPos;
        }

    delete[] pBlockOfData;
    return Result;
    }
/** ---------------------------------------------------------------------------
  Public
  Constructor
  allow to Open an image file

  @param pi_rURL The name of the file to create or open
  @param pi_AccessMode The access mode and the sharing mode of the file
  @param pi_Offset The begining offset in the file
---------------------------------------------------------------------------  */
HRFTgaFile::HRFTgaFile(const HFCPtr<HFCURL>& pi_rURL,
                              HFCAccessMode         pi_AccessMode,
                              uint64_t             pi_Offset)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor store the access mode
    m_IsOpen                = false;

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
  Public
  GetWorldIdentificator
  File information

  @return HGF2DWorld_UNKNOWNWORLD because this feature is not supported by the
  TGA file format.
---------------------------------------------------------------------------  */
const HGF2DWorldIdentificator HRFTgaFile::GetWorldIdentificator () const
    {
    return HGF2DWorld_UNKNOWNWORLD;
    }


/**-----------------------------------------------------------------------------
  Protected
  Constructor
  allow to Create an image file object without open.

  @param pi_rURL The name of the file to create or open
  @param pi_AccessMode The access mode and the sharing mode of the file
  @param pi_Offset The begining offset in the file
------------------------------------------------------------------------------*/
HRFTgaFile::HRFTgaFile(const HFCPtr<HFCURL>& pi_rURL,
                              HFCAccessMode         pi_AccessMode,
                              uint64_t             pi_Offset,
                              bool                 pi_DontOpenFile)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset)
    {
    // The ancestor store the access mode
    m_IsOpen     = false;
    }


/**-----------------------------------------------------------------------------
  Protected method that return a pointer on the physical instance of the TGA file

  @return A pointer to the class that hold the physical instance of the TGA file.

  @see HFCBinStream
  @see HFCLocalBinStream
------------------------------------------------------------------------------*/
HFCBinStream* HRFTgaFile::GetFilePtr  () const
    {
    return (m_pTgaFile);
    }

/**-----------------------------------------------------------------------------
  This protected method returns the offset in the file of the raster data.

  @return UInt32 The offset in the file of the begining of the raster data.
------------------------------------------------------------------------------*/
uint32_t HRFTgaFile::GetRasterDataOffset() const
    {
    return m_RasterDataOffset;
    }

/**-----------------------------------------------------------------------------
  This protected method returns the offset in the file of the end of raster
  data.

  @return UInt32 The offset in the file of the ending of the raster data.
------------------------------------------------------------------------------*/
uint32_t HRFTgaFile::GetRasterDataEndOffset() const
    {
    return m_RasterDataEndOffset;
    }

/**-----------------------------------------------------------------------------
  This protected method returns the number of bits per pixel.

  @return UShort The number of bits per pixel given by the file header.
------------------------------------------------------------------------------*/
unsigned short HRFTgaFile::GetBitsPerPixel() const
    {
    return m_pTgaFileHeader->m_PixelDepth;
    }
