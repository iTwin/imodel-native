//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFPictFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFPictFile
//-----------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRFPictFile.h>
#include <Imagepp/all/h/HRFPictLineEditor.h>
#include <Imagepp/all/h/HRFRasterFileCapabilities.h>
#include <Imagepp/all/h/HRFUtility.h>

#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HFCBinStream.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HFCAccessMode.h>

#include <Imagepp/all/h/HRPPixelType.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDCodecHMRPackBits.h>

#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI4R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>

#include <Imagepp/all/h/HRPChannelOrgRGB.h>
#include <Imagepp/all/h/HTIFFUtils.h>

#include <Imagepp/all/h/HRFRasterFileCapabilities.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
static const uint64_t          s_OffBytesToFileHeader  = 512;


//-----------------------------------------------------------------------------
// Initialisation of default file headers
//-----------------------------------------------------------------------------
static const PictRect           s_DefaultRect           = {0x0000, 0x0000, 0x0000, 0x0000};
static const OpHeaderV2         s_DefaultOpHeaderV2     = {0xFFFF, 0xFFFF, 0x00000000, 0x00000000, s_DefaultRect, 0x00000000};
static const PictFileHeader     s_DefaultFileHeader     = {0x0000, s_DefaultRect, 0x0011, 0x02FF, 0x0C00, s_DefaultOpHeaderV2,
                                                           0x001E, 0x0001, 0x000A, s_DefaultRect, 0x0000
                                                          };
static const PictPackBitsHeader s_DefaultPackBitsHeader = {0x0000, s_DefaultRect, 0x0000, 0x0004, 0x0000, 0x00480000, 0x00480000,
                                                           0x0000, 0x0000, 0x0000, 0x0000, 0x00000000, 0x00000000, 0x00000000
                                                          };
static const PictPixelMapHeader s_DefaultDataHeader     = {0x000000FF, s_DefaultPackBitsHeader, s_DefaultRect, s_DefaultRect, 0x0000};
static const PictColorTable     s_DefaultColorTable     = {0xFFFF0000, 0x8000, 0x0000};



//-----------------------------------------------------------------------------
// HRFPictBlockCapabilities
//-----------------------------------------------------------------------------
class HRFPictBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFPictBlockCapabilities()
        : HRFRasterFileCapabilities()
        {
        HASSERT((numeric_limits<int16_t>::max()) == 32767);

        // Block Capability
        Add(new HRFLineCapability(  HFC_READ_WRITE_CREATE,          // AccessMode
                                    (numeric_limits<int16_t>::max()),                       // MaxLineWidthInBytes
                                    HRFBlockAccess::SEQUENTIAL));   // BlockAccess
        }
    };


class HRFPictCodecPaletteCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFPictCodecPaletteCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Packbits
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecHMRPackBits::CLASS_ID,
                                   new HRFPictBlockCapabilities()));
        }
    };


//-----------------------------------------------------------------------------
// HRFPictCodecIdentityCapabilities
//-----------------------------------------------------------------------------
class HRFPictCodecV24RGBCapabilities : public  HRFRasterFileCapabilities
    {
public:
    // Constructor
    HRFPictCodecV24RGBCapabilities()
        : HRFRasterFileCapabilities()
        {
        // Codec Packbits
        Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
                                   HCDCodecHMRPackBits::CLASS_ID,
                                   new HRFPictBlockCapabilities()));
        }
    };



//-----------------------------------------------------------------------------
// HRFPictCapabilities
//-----------------------------------------------------------------------------
HRFPictCapabilities::HRFPictCapabilities()
    : HRFRasterFileCapabilities()
    {

    // 256 colors palette
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeI8R8G8B8::CLASS_ID,
                                   new HRFPictCodecPaletteCapabilities()));

    // Grayscale 256 colors palette
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV8Gray8::CLASS_ID,
                                   new HRFPictCodecPaletteCapabilities()));

    // NOTE(TR#249202): This capability has been removed in write/create because we don't have a pixel type for
    // grayscale 16 colors palette and it seem that it is not correct that "16 colors" is shown as color mode
    // when saving. When the mentionned pixel type will be available, the write/create functionnalty could be reimplemented.

    // Grayscale 16 colors palette
    Add(new HRFPixelTypeCapability(HFC_READ_ONLY,
                                   HRPPixelTypeI4R8G8B8::CLASS_ID,
                                   new HRFPictCodecPaletteCapabilities()));


    // PixelTypeV24B8G8R8
    Add(new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
                                   HRPPixelTypeV24R8G8B8::CLASS_ID,
                                   new HRFPictCodecV24RGBCapabilities()));


    // Scanline orientation capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));

    // Interleave capability
    Add(new HRFInterleaveCapability(HFC_READ_WRITE_CREATE, HRFInterleaveType::PIXEL));

    // Single Resolution Capability
    Add(new HRFSingleResolutionCapability(HFC_READ_WRITE_CREATE));

    // Media type capability
    Add(new HRFStillImageCapability(HFC_READ_WRITE_CREATE));

    // NOTE: Add tag capability here.
    }


HFC_IMPLEMENT_SINGLETON(HRFPictCreator)

//-----------------------------------------------------------------------------
// Constructor
// Public (HRFPictCreator)
// This is the creator to instantiate bmp format
//-----------------------------------------------------------------------------
HRFPictCreator::HRFPictCreator()
    : HRFRasterFileCreator(HRFPictFile::CLASS_ID)
    {
    // Pict capabilities instance member initialization
    m_pCapabilities = 0;
    }

//-----------------------------------------------------------------------------
// GetLabel
// Public (HRFPictCreator)
// Identification information
//-----------------------------------------------------------------------------
WString HRFPictCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_PICT()); // Pict File Format
    }

//-----------------------------------------------------------------------------
// GetSchemes
// Public (HRFBmpCreator)
// Identification information
//-----------------------------------------------------------------------------
WString HRFPictCreator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

//-----------------------------------------------------------------------------
// GetExtensions
// Public (HRFPictCreator)
// Identification information
//-----------------------------------------------------------------------------
WString HRFPictCreator::GetExtensions() const
    {
    return WString(L"*.pct;*.pict");
    }

//-----------------------------------------------------------------------------
// Create
// Public (HRFPictCreator)
// allow to Open an image file
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HRFPictCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                             HFCAccessMode   pi_AccessMode,
                                             uint64_t       pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    // Open the new file with the given options
    HFCPtr<HRFRasterFile> pFile = new HRFPictFile(pi_rpURL, pi_AccessMode, pi_Offset);
    HASSERT(pFile != 0);

    return (pFile);
    }


//-----------------------------------------------------------------------------
// IsKindOfFile
// Public (HRFPictCreator)
// Opens the file and verifies if it is the right type
//-----------------------------------------------------------------------------
bool HRFPictCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                   uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);

    bool                               Result = false;
    HRFPictFile::DataHeaderType         DataHeaderType;
    PictFileHeader                      FileHeader;
    PictPixelMapHeader                  DataHeader;
    HAutoPtr<HFCBinStream>              pFile;

    (const_cast<HRFPictCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock(GetLockManager());

    // Open the BMP File & place file pointer at the start of the file
    pFile = HFCBinStream::Instanciate(pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

    if (pFile == 0 || pFile->GetLastException() != 0)
        goto WRAPUP;


    // Position just after the 512 bytes mac header used by the OS to keep track of the file
    pFile->SeekToPos(s_OffBytesToFileHeader);

    // Read the pict header and get a glimpse at the following opcode to determine the data representation header used
    if (!HRFPictFile::ReadPictFileHeader(&FileHeader, &DataHeaderType, pFile))
        goto WRAPUP;

    // Validate the file header and its version. We only support PICT header version 2.0 for the moment.
    if (0x0011 != FileHeader.m_VersionOpcode    ||
        0x0C00 != FileHeader.m_HeaderOpcode     ||
        0x02ff != FileHeader.m_Version)
        goto WRAPUP;


    // Pixelmap data header
    if (HRFPictFile::HEADER_PIXELMAP == DataHeaderType)
        {
        if (!HRFPictFile::ReadPixelMapHeader(&DataHeader, pFile))
            goto WRAPUP;

        }
    // Packbits data header
    else if (HRFPictFile::HEADER_PACKBITS == DataHeaderType)
        {
        if (!HRFPictFile::ReadPackBitsHeader(&DataHeader.m_PackBitHeader, pFile))
            goto WRAPUP;
        }
    // Unsupported header
    else
        {
        goto WRAPUP;
        }

    // Check for supported pixel types
    if (DataHeader.m_PackBitHeader.m_PixelType == 16)
        {
        if (!(DataHeader.m_PackBitHeader.m_PixelSize == 32 &&
              DataHeader.m_PackBitHeader.m_CmpCount  == 3  &&
              DataHeader.m_PackBitHeader.m_CmpSize   == 8))
            goto WRAPUP; // Unsupported pixel type
        }
    else if(DataHeader.m_PackBitHeader.m_PixelType == 0)
        {
        if (!((DataHeader.m_PackBitHeader.m_PixelSize == 8 &&
               DataHeader.m_PackBitHeader.m_CmpCount  == 1 &&
               DataHeader.m_PackBitHeader.m_CmpSize   == 8) ||
              (DataHeader.m_PackBitHeader.m_PixelSize == 0 &&
               DataHeader.m_PackBitHeader.m_CmpCount  == 0 &&
               DataHeader.m_PackBitHeader.m_CmpSize   == 0)))
            goto WRAPUP; // Unsupported pixel type
        }
    else
        {
        goto WRAPUP; // Unsupported pixel type
        }


    Result = true;

WRAPUP:
    SisterFileLock.ReleaseKey();

    HASSERT(!(const_cast<HRFPictCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFPictCreator*>(this))->m_pSharingControl = 0;

    return Result;
    }

//-----------------------------------------------------------------------------
// GetCapabilities
// Public (HRFBmpCreator)
// Create or get the singleton capabilities of BMP file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFPictCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFPictCapabilities();

    return m_pCapabilities;
    }



//-----------------------------------------------------------------------------
// Swab helpers
// Protected static
// Those helpers swap the bytes of respective data structures between their
// little endian representation and  their big endian representation.
// TDORAY: Would not be required if Swab was done as part of WriteField/ReadField
// instead
//-----------------------------------------------------------------------------
void HRFPictFile::SwabPictureFrame         (PictRect*               pio_pFrame)
    {
    HPRECONDITION(pio_pFrame != 0);

    SwabArrayOfShort(&pio_pFrame->m_Top, 4);
    }

void HRFPictFile::SwabOpCodeHeader         (OpHeaderV2*             pio_pOpHeader)
    {
    HPRECONDITION(pio_pOpHeader != 0);

    SwabArrayOfShort(&pio_pOpHeader->m_Version, 2);
    SwabArrayOfLong(&pio_pOpHeader->m_HRes, 2);
    SwabPictureFrame(&pio_pOpHeader->m_SrcRect);
    SwabArrayOfLong(&pio_pOpHeader->m_Reserved2, 1);
    }

void HRFPictFile::SwabPackBitsHeader       (PictPackBitsHeader*     pio_pPackBitsHeader)
    {
    HPRECONDITION(pio_pPackBitsHeader != 0);

    SwabArrayOfShort(&pio_pPackBitsHeader->m_RowBytes, 1);
    SwabPictureFrame(&pio_pPackBitsHeader->m_Bounds);
    SwabArrayOfShort(&pio_pPackBitsHeader->m_Version, 2);
    SwabArrayOfLong(&pio_pPackBitsHeader->m_PackSize, 3);
    SwabArrayOfShort(&pio_pPackBitsHeader->m_PixelType, 4);
    SwabArrayOfLong(&pio_pPackBitsHeader->m_PlaneBytes, 3);
    }

void HRFPictFile::SwabPixelMapHeader       (PictPixelMapHeader*     pio_pPixelMapHeader)
    {
    HPRECONDITION(pio_pPixelMapHeader != 0);

    SwabArrayOfLong(&pio_pPixelMapHeader->m_BaseAddr, 1);
    SwabPackBitsHeader(&pio_pPixelMapHeader->m_PackBitHeader);
    SwabPictureFrame(&pio_pPixelMapHeader->m_SrcRect);
    SwabPictureFrame(&pio_pPixelMapHeader->m_DstRect);
    SwabArrayOfShort(&pio_pPixelMapHeader->m_CopyMode, 1);
    }

void HRFPictFile::SwabPictFileHeader       (PictFileHeader*         pio_pFileHeader)
    {
    HPRECONDITION(pio_pFileHeader != 0);

    SwabArrayOfShort(&pio_pFileHeader->m_FileSize, 1);
    SwabPictureFrame(&pio_pFileHeader->m_Rect);
    SwabArrayOfShort(&pio_pFileHeader->m_VersionOpcode, 3);
    SwabOpCodeHeader(&pio_pFileHeader->m_OpHeader);
    SwabArrayOfShort(&pio_pFileHeader->m_DefHilite, 3);
    SwabPictureFrame(&pio_pFileHeader->m_ClipRect);
    SwabArrayOfShort(&pio_pFileHeader->m_OpCode, 1);
    }


namespace { // BEGIN UNNAMED NAMESPACE

//-----------------------------------------------------------------------------
// Template helper to facilitate reading a field from a binary stream
//-----------------------------------------------------------------------------
template <typename T>
inline bool ReadField  (HFCBinStream&   stream,
                        T&              field)
    {
    return sizeof field == stream.Read(&field, sizeof field);
    }

//-----------------------------------------------------------------------------
// Template helper to facilitate writing a field from a binary stream
//-----------------------------------------------------------------------------
template <typename T>
inline bool WriteField (HFCBinStream&   stream,
                        const T&        field)
    {
    return sizeof field == stream.Write(&field, sizeof field);
    }

//-----------------------------------------------------------------------------
// Reusable part for reading a PICT rect field
//-----------------------------------------------------------------------------
bool ReadPictRectField (HFCBinStream&   stream,
                        PictRect&       field)
    {
    return ReadField (stream, field.m_Top) &&
           ReadField (stream, field.m_Left) &&
           ReadField (stream, field.m_Bottom) &&
           ReadField (stream, field.m_Right);
    }

//-----------------------------------------------------------------------------
// Reusable part for writing a PICT rect field
//-----------------------------------------------------------------------------
bool WritePictRectField    (HFCBinStream&   stream,
                            const PictRect& field)
    {
    return WriteField (stream, field.m_Top) &&
           WriteField (stream, field.m_Left) &&
           WriteField (stream, field.m_Bottom) &&
           WriteField (stream, field.m_Right);
    }


//-----------------------------------------------------------------------------
// Reusable part for reading a PICT rgb color field
//-----------------------------------------------------------------------------
bool ReadPictRGBColorField (HFCBinStream&   stream,
                            PictRGBColor&   field)
    {
    return ReadField (stream, field.m_rgbReserved) &&
           ReadField (stream, field.m_rgbRed) &&
           ReadField (stream, field.m_rgbGreen) &&
           ReadField (stream, field.m_rgbBlue);
    }

//-----------------------------------------------------------------------------
// Reusable part for writing a PICT rgb color field
//-----------------------------------------------------------------------------
bool WritePictRGBColorField    (HFCBinStream&       stream,
                                const PictRGBColor& field)
    {
    return WriteField (stream, field.m_rgbReserved) &&
           WriteField (stream, field.m_rgbRed) &&
           WriteField (stream, field.m_rgbGreen) &&
           WriteField (stream, field.m_rgbBlue);
    }


//-----------------------------------------------------------------------------
// Reusable part for reading pack bits header
//-----------------------------------------------------------------------------
bool ReadPackBitsHeaderField   (HFCBinStream&           stream,
                                PictPackBitsHeader&     field)
    {
    return ReadField (stream, field.m_RowBytes) &&
           ReadPictRectField (stream, field.m_Bounds) &&
           ReadField (stream, field.m_Version) &&
           ReadField (stream, field.m_PackType) && 
           ReadField (stream, field.m_PackSize) &&
           ReadField (stream, field.m_HRes) &&
           ReadField (stream, field.m_VRes) &&
           ReadField (stream, field.m_PixelType) &&
           ReadField (stream, field.m_PixelSize) &&
           ReadField (stream, field.m_CmpCount) &&
           ReadField (stream, field.m_CmpSize) &&
           ReadField (stream, field.m_PlaneBytes) &&
           ReadField (stream, field.m_PmTable) &&
           ReadField (stream, field.m_PmReserved);
    }


//-----------------------------------------------------------------------------
// Reusable part for writing pack bits header
//-----------------------------------------------------------------------------
bool WritePackBitsHeaderField  (HFCBinStream&               stream,
                                const PictPackBitsHeader&   field)
    {
    return WriteField (stream, field.m_RowBytes) &&
           WritePictRectField (stream, field.m_Bounds) &&
           WriteField (stream, field.m_Version) &&
           WriteField (stream, field.m_PackType) && 
           WriteField (stream, field.m_PackSize) &&
           WriteField (stream, field.m_HRes) &&
           WriteField (stream, field.m_VRes) &&
           WriteField (stream, field.m_PixelType) &&
           WriteField (stream, field.m_PixelSize) &&
           WriteField (stream, field.m_CmpCount) &&
           WriteField (stream, field.m_CmpSize) &&
           WriteField (stream, field.m_PlaneBytes) &&
           WriteField (stream, field.m_PmTable) &&
           WriteField (stream, field.m_PmReserved);
    }


//-----------------------------------------------------------------------------
// Reusable part for reading op header V2
//-----------------------------------------------------------------------------
bool ReadOpHeaderField (HFCBinStream&           stream,
                        OpHeaderV2&             field)
    {
    return ReadField (stream, field.m_Version) &&
           ReadField (stream, field.m_Reserved1) &&
           ReadField (stream, field.m_HRes) &&
           ReadField (stream, field.m_VRes) &&
           ReadPictRectField (stream, field.m_SrcRect) &&
           ReadField (stream, field.m_Reserved2);
    }


//-----------------------------------------------------------------------------
// Reusable part for writing op header V2
//-----------------------------------------------------------------------------
bool WriteOpHeaderField    (HFCBinStream&           stream,
                            const OpHeaderV2&       field)
    {
    return WriteField (stream, field.m_Version) &&
           WriteField (stream, field.m_Reserved1) &&
           WriteField (stream, field.m_HRes) &&
           WriteField (stream, field.m_VRes) &&
           WritePictRectField (stream, field.m_SrcRect) &&
           WriteField (stream, field.m_Reserved2);
    }


} // END UNNAMED NAMESPACE


//-----------------------------------------------------------------------------
// ReadPictFileHeader
// Public static
// Read the content of an Pict File Header from a file
// Returns false if any error and true otherwise.
//-----------------------------------------------------------------------------
bool HRFPictFile::ReadPictFileHeader      (PictFileHeader*             po_pFileHeader,
                                            DataHeaderType*             po_pDataHeaderType,
                                            HFCBinStream*               pio_pFile)
    {
    HPRECONDITION(po_pFileHeader != 0);
    HPRECONDITION(po_pDataHeaderType != 0);
    HPRECONDITION(pio_pFile != 0);

    if (!ReadField (*pio_pFile, po_pFileHeader->m_FileSize) ||
        !ReadPictRectField (*pio_pFile, po_pFileHeader->m_Rect) ||
        !ReadField (*pio_pFile, po_pFileHeader->m_VersionOpcode) ||
        !ReadField (*pio_pFile, po_pFileHeader->m_Version) ||
        !ReadField (*pio_pFile, po_pFileHeader->m_HeaderOpcode) ||
        !ReadOpHeaderField(*pio_pFile, po_pFileHeader->m_OpHeader) ||
        !ReadField (*pio_pFile, po_pFileHeader->m_DefHilite) ||
        !ReadField (*pio_pFile, po_pFileHeader->m_ClipOpcode) ||
        !ReadField (*pio_pFile, po_pFileHeader->m_ClipSize) ||
        !ReadPictRectField (*pio_pFile, po_pFileHeader->m_ClipRect) ||
        !ReadField (*pio_pFile, po_pFileHeader->m_OpCode))
        return false; // Error


    SwabPictFileHeader(po_pFileHeader);


    // Pixelmap header
    if (0x009A == po_pFileHeader->m_OpCode)
        *po_pDataHeaderType = HEADER_PIXELMAP;
    // Packbits header
    else if (0x0098 == po_pFileHeader->m_OpCode)
        *po_pDataHeaderType = HEADER_PACKBITS;
    // Unsupported header
    else
        *po_pDataHeaderType = HEADER_UNSUPPORTED;


    return true; // No Error
    }


//-----------------------------------------------------------------------------
// ReadPixelMapHeader
// Public static
// Read the content of an PICT PixelMap Header from a file
// Returns false if any error and true otherwise.
//-----------------------------------------------------------------------------
bool HRFPictFile::ReadPixelMapHeader      (PictPixelMapHeader*         po_pPixelMapHeader,
                                            HFCBinStream*               pio_pFile)
    {
    HPRECONDITION(po_pPixelMapHeader != 0);
    HPRECONDITION(pio_pFile != 0);

    if (!ReadField (*pio_pFile, po_pPixelMapHeader->m_BaseAddr) ||
        !ReadPackBitsHeaderField (*pio_pFile, po_pPixelMapHeader->m_PackBitHeader) ||
        !ReadField (*pio_pFile, po_pPixelMapHeader->m_SrcRect) ||
        !ReadField (*pio_pFile, po_pPixelMapHeader->m_DstRect) ||
        !ReadField (*pio_pFile, po_pPixelMapHeader->m_CopyMode))
        return false; // Error

    SwabPixelMapHeader(po_pPixelMapHeader);

    return true; // No Error
    }


//-----------------------------------------------------------------------------
// ReadPackBitsHeader
// Public static
// Read the content of a PICT PackBit Header from a file
// Returns false if any error and true otherwise.
//-----------------------------------------------------------------------------
bool HRFPictFile::ReadPackBitsHeader      (PictPackBitsHeader*         po_pPackBitsHeader,
                                            HFCBinStream*               pio_pFile)
    {
    HPRECONDITION(po_pPackBitsHeader != 0);
    HPRECONDITION(pio_pFile != 0);

    if (!ReadPackBitsHeaderField (*pio_pFile, *po_pPackBitsHeader))
        return false; // Error

    SwabPackBitsHeader(po_pPackBitsHeader);

    return true; // No Error

    }


//-----------------------------------------------------------------------------
// ReadColorTable
// Protected static
// Read the color table of a PICT file
// Returns false if any error and true otherwise.
//
// NOTE: This method initialize po_rColorTableItems. This is why we pass an
//       autoptr as a reference.
//-----------------------------------------------------------------------------
bool HRFPictFile::ReadColorTable          (PictColorTable*                 po_pColorTable,
                                            HArrayAutoPtr<PictRGBColor>&    po_rColorTableItems,
                                            HFCBinStream*                   pio_pFile)
    {
    HPRECONDITION(po_pColorTable != 0);
    HPRECONDITION(pio_pFile != 0);

    if (!ReadField (*pio_pFile, po_pColorTable->m_CtSeed) ||
        !ReadField (*pio_pFile, po_pColorTable->m_CtFlags) ||
        !ReadField (*pio_pFile, po_pColorTable->m_CtSize))
        return false; // Error


    // Pict files are written using big endian data representation. We're on a little endian system. Swap the bytes.
    SwabArrayOfLong(&po_pColorTable->m_CtSeed, 1);
    SwabArrayOfShort(&po_pColorTable->m_CtFlags, 2);
    // TDORAY: Find why no Swabs for m_CtSize

    if (po_pColorTable->m_CtSize == 0)
        return true; // Ok, no color table.
    else if(po_pColorTable->m_CtSize > 255)
        return false; // Error, we do not support color table with more than 256 entries. This is probably a corrupted file.

    const size_t ItemsQty = po_pColorTable->m_CtSize + 1;

    po_rColorTableItems = new PictRGBColor[ItemsQty];

    for (size_t itemIdx = 0; itemIdx < ItemsQty; ++itemIdx)
        {
        // TDORAY: Find why no Swabs for color fields
        if (!ReadPictRGBColorField(*pio_pFile, po_rColorTableItems[itemIdx]))
            return false;
        }

    return true; // No Error
    }


//-----------------------------------------------------------------------------
// WritePictFileHeader
// Public static
// Write the content of a PICT file Header to a file
// Returns false if any error and true otherwise.
//
// NOTE: Because of the static variable, this method is not reentrant
//-----------------------------------------------------------------------------
bool HRFPictFile::WritePictFileHeader     (const PictFileHeader&       pi_rFileHeader,
                                            const DataHeaderType&       pi_rDataHeaderType,
                                            HFCBinStream*               pio_pFile)
    {
    HPRECONDITION(pio_pFile != 0);
    HPRECONDITION(HEADER_PIXELMAP == pi_rDataHeaderType || HEADER_PACKBITS == pi_rDataHeaderType);

    static PictFileHeader WrittenFileHeader;

    memcpy(&WrittenFileHeader, &pi_rFileHeader, sizeof WrittenFileHeader);
    SwabPictFileHeader(&WrittenFileHeader);

    if (!WriteField (*pio_pFile, WrittenFileHeader.m_FileSize) ||
        !WritePictRectField (*pio_pFile, WrittenFileHeader.m_Rect) ||
        !WriteField (*pio_pFile, WrittenFileHeader.m_VersionOpcode) ||
        !WriteField (*pio_pFile, WrittenFileHeader.m_Version) ||
        !WriteField (*pio_pFile, WrittenFileHeader.m_HeaderOpcode) ||
        !WriteOpHeaderField(*pio_pFile, WrittenFileHeader.m_OpHeader) ||
        !WriteField (*pio_pFile, WrittenFileHeader.m_DefHilite) ||
        !WriteField (*pio_pFile, WrittenFileHeader.m_ClipOpcode) ||
        !WriteField (*pio_pFile, WrittenFileHeader.m_ClipSize) ||
        !WritePictRectField (*pio_pFile, WrittenFileHeader.m_ClipRect) ||
        !WriteField (*pio_pFile, WrittenFileHeader.m_OpCode))
        return false; // Error

    return true; // No Error
    }


//-----------------------------------------------------------------------------
// WritePixelMapHeader
// Public static
// Write the content of a PICT PixelMap Header to a file
// Returns false if any error and true otherwise.
//
// NOTE: Because of the static variable, this method is not reentrant
//-----------------------------------------------------------------------------
bool HRFPictFile::WritePixelMapHeader     (const PictPixelMapHeader&   pi_rPixelMapHeader,
                                            HFCBinStream*               pio_pFile)
    {
    HPRECONDITION(pio_pFile != 0);

    static PictPixelMapHeader WrittenPixelMapHeader;

    memcpy(&WrittenPixelMapHeader, &pi_rPixelMapHeader, sizeof WrittenPixelMapHeader);
    SwabPixelMapHeader(&WrittenPixelMapHeader);

    if (!WriteField (*pio_pFile, WrittenPixelMapHeader.m_BaseAddr) ||
        !WritePackBitsHeaderField (*pio_pFile, WrittenPixelMapHeader.m_PackBitHeader) ||
        !WriteField (*pio_pFile, WrittenPixelMapHeader.m_SrcRect) ||
        !WriteField (*pio_pFile, WrittenPixelMapHeader.m_DstRect) ||
        !WriteField (*pio_pFile, WrittenPixelMapHeader.m_CopyMode))
        return false; // Error

    return true; // No Error
    }

//-----------------------------------------------------------------------------
// WritePackBitsHeader
// Public static
// Write the content of a PICT PackBits Header to a file
// Returns false if any error and true otherwise.
//
// NOTE: Because of the static variable, this method is not reentrant
//-----------------------------------------------------------------------------
bool HRFPictFile::WritePackBitsHeader     (const PictPackBitsHeader&   pi_rPackBitsHeader,
                                            HFCBinStream*               pio_pFile)
    {
    HPRECONDITION(pio_pFile != 0);

    static PictPackBitsHeader WrittenPackBitsHeader;

    memcpy(&WrittenPackBitsHeader, &pi_rPackBitsHeader, sizeof WrittenPackBitsHeader);
    SwabPackBitsHeader(&WrittenPackBitsHeader);


    if (!WritePackBitsHeaderField (*pio_pFile, WrittenPackBitsHeader))
        return false; // Error

    return true; // No Error
    }


//-----------------------------------------------------------------------------
// WriteColorTable
// Protected static
// Write the color table of a PICT file
// Returns false if any error and true otherwise.
//
// NOTE: Because of the static variable, this method is not reentrant
//-----------------------------------------------------------------------------
bool HRFPictFile::WriteColorTable         (const PictColorTable&               pi_rColorTable,
                                            const PictRGBColor*                 pi_pColorTableItems,
                                            HFCBinStream*                       pio_pFile)
    {
    HPRECONDITION(pio_pFile != 0);

    static PictColorTable WrittenColorTable;

    memcpy(&WrittenColorTable, &pi_rColorTable, sizeof WrittenColorTable);

    SwabArrayOfLong(&WrittenColorTable.m_CtSeed, 1);
    SwabArrayOfShort(&WrittenColorTable.m_CtFlags, 2);
    // TDORAY: Find why no Swabs for m_CtSize

    if (!WriteField (*pio_pFile, WrittenColorTable.m_CtSeed) ||
        !WriteField (*pio_pFile, WrittenColorTable.m_CtFlags) ||
        !WriteField (*pio_pFile, WrittenColorTable.m_CtSize))
        return false; // Error

    if(pi_rColorTable.m_CtSize != 0)
        {
        uint32_t ItemsQty = pi_rColorTable.m_CtSize + 1;

        for (uint32_t itemIdx = 0; itemIdx < ItemsQty; ++itemIdx)
            {
            // TDORAY: Find why no Swabs for color fields
            if (!WritePictRGBColorField(*pio_pFile, pi_pColorTableItems[itemIdx]))
                return false; // Error
            }
        }

    return true; // Not Error
    }


//-----------------------------------------------------------------------------
// ReadHeaders
// Private
// Read the content of a PICT file's headers
// Returns false if any error and true otherwise.
//-----------------------------------------------------------------------------
bool HRFPictFile::ReadHeaders()
    {
    // Position just after the 512 bytes mac header used by the OS to keep track of the file
    m_pPictFile->SeekToPos(s_OffBytesToFileHeader);

    // Read the pict header
    if(!ReadPictFileHeader(&m_FileHeader, &m_DataHeaderType, m_pPictFile))
        return false; // Error

    // Read data representation headers
    if (HEADER_PIXELMAP == m_DataHeaderType)
        {
        if(!ReadPixelMapHeader(&m_DataHeader, m_pPictFile))
            return false; // Error
        }
    else if (HEADER_PACKBITS == m_DataHeaderType)
        {
        if(!ReadPackBitsHeader(&m_DataHeader.m_PackBitHeader, m_pPictFile))
            return false; // Error

        if(!ReadColorTable(&m_ColorTable, m_pColorTableItemsList, m_pPictFile))
            return false; // Error


        if (!ReadPictRectField(*m_pPictFile, m_DataHeader.m_SrcRect) ||
            !ReadPictRectField(*m_pPictFile, m_DataHeader.m_DstRect) ||
            !ReadField(*m_pPictFile, m_DataHeader.m_CopyMode))
            return false; // Error

        SwabPictureFrame(&m_DataHeader.m_SrcRect);
        SwabPictureFrame(&m_DataHeader.m_DstRect);
        SwabArrayOfShort(&m_DataHeader.m_CopyMode, 1);
        }

    // Validate our position in the file
    HPOSTCONDITION(GetOffBytesToData() == m_pPictFile->GetCurrentPos());

    return true; // No Error
    }


//-----------------------------------------------------------------------------
// WriteHeaders
// Private
// Write the content of this PICT file's headers
// Returns false if any error and true otherwise.
//-----------------------------------------------------------------------------
bool HRFPictFile::WriteHeaders()
    {
    // NOTE: It may be preferable to write 0s to the apple reserved 512 bytes area
    // Position just after the 512 bytes mac header used by the OS to keep track of the file
    m_pPictFile->SeekToPos(s_OffBytesToFileHeader);

    // Read the pict header
    if(!WritePictFileHeader(m_FileHeader, m_DataHeaderType, m_pPictFile))
        return false; // Error

    // Read data representation headers
    if (HEADER_PIXELMAP == m_DataHeaderType)
        {
        if(!WritePixelMapHeader(m_DataHeader, m_pPictFile))
            return false; // Error
        }
    else if (HEADER_PACKBITS == m_DataHeaderType)
        {
        if(!WritePackBitsHeader(m_DataHeader.m_PackBitHeader, m_pPictFile))
            return false; // Error

        if(!WriteColorTable(m_ColorTable, m_pColorTableItemsList, m_pPictFile))
            return false; // Error

        PictPixelMapHeader WrittenDataHeader(m_DataHeader);

        SwabPictureFrame(&WrittenDataHeader.m_SrcRect);
        SwabPictureFrame(&WrittenDataHeader.m_DstRect);
        SwabArrayOfShort(&WrittenDataHeader.m_CopyMode, 1);

        if (!WritePictRectField(*m_pPictFile, WrittenDataHeader.m_SrcRect) ||
            !WritePictRectField(*m_pPictFile, WrittenDataHeader.m_DstRect) ||
            !WriteField(*m_pPictFile, WrittenDataHeader.m_CopyMode))
            return false; // Error
        }

    HPOSTCONDITION(GetOffBytesToData() == m_pPictFile->GetCurrentPos());

    return true; // No Error
    }

//-----------------------------------------------------------------------------
// Public
// Constructor
// allow to Open an image file
//-----------------------------------------------------------------------------
HRFPictFile::HRFPictFile               (const HFCPtr<HFCURL>&       pi_rURL,
                                        HFCAccessMode               pi_AccessMode,
                                        uint64_t                   pi_Offset)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset),
      m_DataHeaderType(HEADER_UNSUPPORTED)
    {
    m_IsOpen = false;

    // Initialize headers with 0 values
    memset(&m_FileHeader, 0, sizeof m_FileHeader);
    memset(&m_DataHeader, 0, sizeof m_DataHeader);
    memset(&m_ColorTable, 0, sizeof m_ColorTable);

    if (GetAccessMode().m_HasCreateAccess)
        {
        Create();
        }
    else
        {
        // Create Page and Res Descriptors.
        if(!Open())
            throw HFCCorruptedFileException(pi_rURL->GetURL());
        else
            CreateDescriptors();
        }

    }


//-----------------------------------------------------------------------------
// Protected
// Constructor
// allow to Create an image file object without open.
//-----------------------------------------------------------------------------
HRFPictFile::HRFPictFile               (const HFCPtr<HFCURL>&       pi_rURL,
                                        HFCAccessMode               pi_AccessMode,
                                        uint64_t                   pi_Offset,
                                        bool                 pi_DontOpenFile)
    : HRFRasterFile(pi_rURL, pi_AccessMode, pi_Offset),
      m_DataHeaderType(HEADER_UNSUPPORTED)
    {
    m_IsOpen = false;

    // Initialize headers with 0 values
    memset(&m_FileHeader, 0, sizeof m_FileHeader);
    memset(&m_DataHeader, 0, sizeof m_DataHeader);
    memset(&m_ColorTable, 0, sizeof m_ColorTable);
    }


//-----------------------------------------------------------------------------
// Destructor
// Public
// Destroy Bmp file object
//-----------------------------------------------------------------------------
HRFPictFile::~HRFPictFile()
    {
    try
        {
        // Close the BMP file and initialize the Compression Structure
        SavePictFile(true);
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
HRFResolutionEditor* HRFPictFile::CreateResolutionEditor   (uint32_t            pi_Page,
                                                            unsigned short     pi_Resolution,
                                                            HFCAccessMode       pi_AccessMode)
    {
    HPRECONDITION(pi_Page < CountPages());
    HPRECONDITION(GetPageDescriptor(pi_Page) != 0);
    HPRECONDITION(pi_Resolution < GetPageDescriptor(pi_Page)->CountResolutions());
    HPRECONDITION(GetPageDescriptor(pi_Page)->GetResolutionDescriptor(pi_Resolution) != 0);

    return new HRFPictLineEditor(this, pi_Page, pi_Resolution, pi_AccessMode);
    }

//-----------------------------------------------------------------------------
// AddPage
// Public
// File manipulation
//-----------------------------------------------------------------------------
bool HRFPictFile::AddPage                     (HFCPtr<HRFPageDescriptor>   pi_pPage)
    {
    // Validation if it's possible to add a page
    HPRECONDITION(CountPages() == 0);
    HPRECONDITION(pi_pPage != 0);

    // Add the page descriptor to the list
    HRFRasterFile::AddPage(pi_pPage);

    const HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(0);
    const HFCPtr<HRFResolutionDescriptor> pResolutionDescriptor = pPageDescriptor->GetResolutionDescriptor(0);
    const HFCPtr<HRPPixelType> pPixelType =  pResolutionDescriptor->GetPixelType();


    // Initialize headers with default values
    memcpy(&m_FileHeader, &s_DefaultFileHeader, sizeof m_FileHeader);
    memcpy(&m_DataHeader, &s_DefaultDataHeader, sizeof m_DataHeader);
    memcpy(&m_ColorTable, &s_DefaultColorTable, sizeof m_ColorTable);


    HASSERT((numeric_limits<int16_t>::max()) >= pResolutionDescriptor->GetWidth());
    HASSERT(0 < pResolutionDescriptor->GetWidth());
    HASSERT((numeric_limits<int16_t>::max()) >= pResolutionDescriptor->GetHeight());
    HASSERT(0 < pResolutionDescriptor->GetHeight());

    unsigned short Width    = static_cast<unsigned short>(pResolutionDescriptor->GetWidth());
    unsigned short Heigth   = static_cast<unsigned short>(pResolutionDescriptor->GetHeight());


    // Initialize image dimensions
    m_FileHeader.m_ClipRect.m_Right     = Width;
    m_FileHeader.m_ClipRect.m_Bottom    = Heigth;
    m_FileHeader.m_Rect.m_Right         = Width;
    m_FileHeader.m_Rect.m_Bottom        = Heigth;

    m_DataHeader.m_SrcRect.m_Right      = Width;
    m_DataHeader.m_SrcRect.m_Bottom     = Heigth;
    m_DataHeader.m_DstRect.m_Right      = Width;
    m_DataHeader.m_DstRect.m_Bottom     = Heigth;

    if (pPixelType->GetClassID() == HRPPixelTypeV24R8G8B8::CLASS_ID)
        {
        m_DataHeaderType = HEADER_PIXELMAP;
        m_FileHeader.m_OpCode = 0x009A;
        m_DataHeader.m_PackBitHeader.m_RowBytes = (Width * 4) | 0x8000;
        }
    else if (pPixelType->GetClassID() == HRPPixelTypeI8R8G8B8::CLASS_ID ||
             pPixelType->GetClassID() == HRPPixelTypeV8Gray8::CLASS_ID  ||
             pPixelType->GetClassID() == HRPPixelTypeI4R8G8B8::CLASS_ID)
        {
        m_DataHeaderType = HEADER_PACKBITS;
        m_FileHeader.m_OpCode = 0x0098;
        m_DataHeader.m_PackBitHeader.m_RowBytes = ((Width * (unsigned short)pPixelType->CountPixelRawDataBits() + 7)/8)  | 0x8000;
        }
    else
        {
        HASSERT(0); //Not supposed to be here!!
        }

    // NOTE: Add tag init here

    // Set the image color space
    SetPixelTypeToPage(pResolutionDescriptor->GetPixelType());


    // Write headers informations to the file
    if(!WriteHeaders())
        return false; // Error


    return true; // No Error
    }

//-----------------------------------------------------------------------------
// Public
// GetFileCurrentSize
// Get the file's current size.
//-----------------------------------------------------------------------------
uint64_t HRFPictFile::GetFileCurrentSize() const
    {
    return HRFRasterFile::GetFileCurrentSize(m_pPictFile);
    }


//-----------------------------------------------------------------------------
// GetCapabilities
// Public
// Returnt the capabilities of the file.
//-----------------------------------------------------------------------------
const HFCPtr<HRFRasterFileCapabilities>& HRFPictFile::GetCapabilities () const
    {
    return (HRFPictCreator::GetInstance()->GetCapabilities());
    }

//-----------------------------------------------------------------------------
// Open
// Protected
// This method open the file.
// Returns false when an error occurs and true otherwise.
//-----------------------------------------------------------------------------
bool HRFPictFile::Open()
    {
    // Open the file
    if (!m_IsOpen)
        {
        // Open the actual PICT file.
        m_pPictFile = HFCBinStream::Instanciate(GetURL(), m_Offset, GetPhysicalAccessMode(), 0, true);

        // This creates the sister file for file sharing control if necessary.
        SharingControlCreate();

        // Initialisation of files headers
        if(!ReadHeaders())
            return false; // Error

        m_IsOpen = true;
        }

    return true; // No Error
    }


//-----------------------------------------------------------------------------
// CreateDescriptors
// Protected
// Create Pict File Descriptors
//-----------------------------------------------------------------------------
void HRFPictFile::CreateDescriptors ()
    {
    // Obtain the width and height of the resolution.
    uint32_t Width  = m_FileHeader.m_Rect.m_Right - m_FileHeader.m_Rect.m_Left;
    uint32_t Height = m_FileHeader.m_Rect.m_Bottom - m_FileHeader.m_Rect.m_Top;

    // Create Page and Resolution Description/Capabilities for this file.
    HFCPtr<HRFResolutionDescriptor>     pResolution;
    HFCPtr<HRFPageDescriptor>           pPage;

    HFCPtr<HRPPixelType>  pPixelType(CreatePixelTypeFromFile());


    // Check for read-only pixel types
    if(GetAccessMode().m_HasCreateAccess || GetAccessMode().m_HasWriteAccess)
        {
        // NOTE(TR#249202): This pixel type is now Read-Only. See other note in capability section for further
        // Explanation. This will have to be removed if write/create access mode reimplemented.
        if(pPixelType->IsCompatibleWith(HRPPixelTypeI4R8G8B8::CLASS_ID))
            throw HFCFileReadOnlyException(GetURL()->GetURL());
        }



    // Create codec for compression.
    HRFBlockAccess  BlockAccess = HRFBlockAccess::SEQUENTIAL;
    HRFBlockType    BlockType   = HRFBlockType::LINE;
    uint32_t        BlockWidth  = Width;
    uint32_t        BlockHeight = 1;

    // Compression is PackBits
    HASSERT(HEADER_PACKBITS == m_DataHeaderType || HEADER_PIXELMAP == m_DataHeaderType);
        
    // Create Resolution Descriptor
    pResolution =  new HRFResolutionDescriptor(GetAccessMode(),               // AccessMode,
                                               GetCapabilities(),             // Capabilities,
                                               1.0,                           // XResolutionRatio,
                                               1.0,                           // YResolutionRatio,
                                               pPixelType,                    // PixelType,
                                               new HCDCodecHMRPackBits(),     // Codec,
                                               BlockAccess,                   // RBlockAccess,
                                               BlockAccess,                   // WBlockAccess,
                                               HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL,  // ScanLineOrientation,
                                               HRFInterleaveType::PIXEL,      // InterleaveType
                                               0,                             // IsInterlace,
                                               Width,                         // Width,
                                               Height,                        // Height,
                                               BlockWidth,                    // BlockWidth,
                                               BlockHeight,                   // BlockHeight,
                                               0,                             // BlocksDataFlag
                                               BlockType);                    // BlockType

    // Tag information
    HPMAttributeSet TagList;


    // NOTE: Add tag list creation here


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
// Protected
// GetOffBytesToData
// Get the position of the data in the file
//-----------------------------------------------------------------------------
uint64_t HRFPictFile::GetOffBytesToData() const
    {
    // Read data representation headers
    if(HEADER_PIXELMAP == m_DataHeaderType)
        {
        return s_OffBytesToFileHeader + (PictFileHeader_Size/*sizeof m_FileHeader*/) + (PictPixelMapHeader_Size/*sizeof m_DataHeader*/);
        }
    else if(HEADER_PACKBITS == m_DataHeaderType)
        {
        uint64_t ColorsSize = (m_ColorTable.m_CtSize == 0) ? (0) : ((m_ColorTable.m_CtSize + 1)*(PictRGBColor_Size/*sizeof PictRGBColor*/));    
        return GetOffBytesToColorTable()
             + PictColorTable_Size //(sizeof m_ColorTable)
             + ColorsSize
             + PictRect_Size //(sizeof m_DataHeader.m_SrcRect) 
             + PictRect_Size //(sizeof m_DataHeader.m_DstRect)
             + 2; //(sizeof m_DataHeader.m_CopyMode)
        }
    else
        {
        HASSERT(0);
        return 0; // Not supposed to be called
        }

    }


//-----------------------------------------------------------------------------
// Protected
// GetOffBytesToColorTable
// Get the position of the color table in the file
//-----------------------------------------------------------------------------
uint64_t HRFPictFile::GetOffBytesToColorTable() const
    {
    if(HEADER_PACKBITS == m_DataHeaderType)
        {
        return s_OffBytesToFileHeader 
               + PictFileHeader_Size //(sizeof m_FileHeader)
               + PictPackBitsHeader_Size; //(sizeof m_DataHeader.m_PackBitHeader)
        }
    else
        {
        HASSERT(0);
        return 0; // Not supposed to be called
        }
    }


//-----------------------------------------------------------------------------
// SavePictFile
// Private
// This method saves the file and close it if needed
//-----------------------------------------------------------------------------
void HRFPictFile::SavePictFile             (bool       pi_CloseFile)
    {
    // Be sure that the file is already open and that at least one page
    // has been add. Because if the destroyer is call afer a exception
    // was thrown, we want to be sure that the object is valid before we
    // execute the destroyer.
    if (m_IsOpen && m_ListOfPageDescriptor.size() > 0)
        {

        if ((GetAccessMode().m_HasWriteAccess) || (GetAccessMode().m_HasCreateAccess))
            {
            bool   SaveHeader = false;

            HFCPtr<HRFPageDescriptor> pPageDescriptor = GetPageDescriptor(0);

            // NOTE: Add tag processing here

            // Lock the sister file for the SetPalette operation
            HFCLockMonitor SisterFileLock(GetLockManager());

            if (pPageDescriptor->GetResolutionDescriptor(0)->PaletteHasChanged())
                {
                // Set the image color space.
                SetPixelTypeToPage(pPageDescriptor->GetResolutionDescriptor(0)->GetPixelType());
                SaveHeader = true;
                }

            // Write the file header information.
            if (SaveHeader)
                {
                if(!WriteHeaders())
                    {
                    HASSERT(0); // Error while writting!!
                    }

                pPageDescriptor->Saved();
                pPageDescriptor->GetResolutionDescriptor(0)->Saved();
                }

            m_pPictFile->Flush();

            // Unlock the sister file.
            SisterFileLock.ReleaseKey();
            }

        if (pi_CloseFile)
            {
            m_IsOpen = false;
            m_pPictFile = 0;
            }
        }
    }



//-----------------------------------------------------------------------------
// Save
// Private
// This method saves the file.
//-----------------------------------------------------------------------------
void HRFPictFile::Save()
    {
    //Keep last file position
    uint64_t CurrentPos = m_pPictFile->GetCurrentPos();

    SavePictFile(false);

    //Set back position
    m_pPictFile->SeekToPos(CurrentPos);
    }

//-----------------------------------------------------------------------------
// Create
// Private
// This method create the file.
//-----------------------------------------------------------------------------
bool HRFPictFile::Create()
    {
    // Open the file.
    m_pPictFile = HFCBinStream::Instanciate(GetURL(), GetAccessMode(), 0, true);

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
HFCPtr<HRPPixelType> HRFPictFile::CreatePixelTypeFromFile() const
    {
    HFCPtr<HRPPixelType> pPixelType;

    // 24bits RGB pixel
    if (m_DataHeader.m_PackBitHeader.m_PixelType == 16 &&
        m_DataHeader.m_PackBitHeader.m_PixelSize == 32 &&
        m_DataHeader.m_PackBitHeader.m_CmpCount  == 3  &&
        m_DataHeader.m_PackBitHeader.m_CmpSize   == 8)
        {
        pPixelType = new HRPPixelTypeV24R8G8B8();
        }
    // Palettes
    else if (m_DataHeader.m_PackBitHeader.m_PixelType == 0)
        {
        uint32_t ColorQty = m_ColorTable.m_CtSize + 1;
        HASSERT(ColorQty <= 256);


        // 256 colors palette or Greyscale palette
        if (m_DataHeader.m_PackBitHeader.m_PixelSize == 8 &&
            m_DataHeader.m_PackBitHeader.m_CmpCount  == 1 &&
            m_DataHeader.m_PackBitHeader.m_CmpSize   == 8)
            {
            Byte ColorValue[3];

            // It seems that m_CtSize = 0 is reserved for 'no colors' indication
            HASSERT(m_ColorTable.m_CtSize > 0);

            pPixelType = new HRPPixelTypeI8R8G8B8();

            // Set the palette
            HRPPixelPalette& rPalette = pPixelType->LockPalette();

            bool IsGrayScale = (ColorQty == 256) ? true : false;

            for (uint32_t i = 0; i < ColorQty; i++)
                {
                // Set the colors to the palette
                ColorValue[0]  = (Byte)m_pColorTableItemsList[i].m_rgbRed;
                ColorValue[1]  = (Byte)m_pColorTableItemsList[i].m_rgbGreen;
                ColorValue[2]  = (Byte)m_pColorTableItemsList[i].m_rgbBlue;
                rPalette.SetCompositeValue(i, ColorValue);

                // Test for grayscale palette
                if(!IsGrayScale || ColorValue[0] != i || ColorValue[1] != i || ColorValue[2] != i)
                    IsGrayScale = false;
                }

            pPixelType->UnlockPalette();

            // This was a grayscale palette. Use the corresponding class for pixel type.
            if (IsGrayScale)
                pPixelType = new HRPPixelTypeV8Gray8();

            }
        // 16 colors palette
        else if (m_DataHeader.m_PackBitHeader.m_PixelSize == 0 &&
                 m_DataHeader.m_PackBitHeader.m_CmpCount  == 0 &&
                 m_DataHeader.m_PackBitHeader.m_CmpSize   == 0)
            {
            // It is assumed that no colors have been stored to the file for 16 colors palette.
            HASSERT(m_ColorTable.m_CtSize == 0);

            // Create a default 16 color palette pixel type (Greyscale)
            pPixelType = new HRPPixelTypeI4R8G8B8();
            }
        }
    else
        {
        HASSERT(false); // Not supposed to be here
        }


    return pPixelType;
    }

void HRFPictFile::SetPixelTypeToPage       (const HFCPtr<HRPPixelType>          pi_pPixelType)
    {
    HPRECONDITION(pi_pPixelType != 0);

    // 24bits RGB pixel
    if (pi_pPixelType->GetClassID() == HRPPixelTypeV24R8G8B8::CLASS_ID)
        {
        // Set the pixel type header part for this kind of pixel
        m_DataHeader.m_PackBitHeader.m_PixelType = 16;
        m_DataHeader.m_PackBitHeader.m_PixelSize = 32;
        m_DataHeader.m_PackBitHeader.m_CmpCount  = 3;
        m_DataHeader.m_PackBitHeader.m_CmpSize   = 8;
        }
    // Palettes
    else
        {
        // Common indicator for all palettes
        m_DataHeader.m_PackBitHeader.m_PixelType = 0;

        if (pi_pPixelType->GetClassID() == HRPPixelTypeI8R8G8B8::CLASS_ID ||
            pi_pPixelType->GetClassID() == HRPPixelTypeV8Gray8::CLASS_ID)
            {
            // Set the pixel type header part for this kind of pixel
            m_DataHeader.m_PackBitHeader.m_PixelSize = 8;
            m_DataHeader.m_PackBitHeader.m_CmpCount  = 1;
            m_DataHeader.m_PackBitHeader.m_CmpSize   = 8;


            const HRPPixelPalette& rPalette = pi_pPixelType->GetPalette();
            uint32_t ColorQty = rPalette.CountUsedEntries();
            HASSERT(ColorQty <= 256);

            if (ColorQty == 0)
                {
                // Write the full 256 colors to the file for compatibility with the file format
                HASSERT(ColorQty == 0 && pi_pPixelType->GetClassID() == HRPPixelTypeV8Gray8::CLASS_ID);
                ColorQty = 256;

                m_pColorTableItemsList = new PictRGBColor[ColorQty];
                for (unsigned short i = 0; i < ColorQty; i++)
                    {
                    m_pColorTableItemsList[i].m_rgbRed      = i;
                    m_pColorTableItemsList[i].m_rgbGreen    = i;
                    m_pColorTableItemsList[i].m_rgbBlue     = i;
                    m_pColorTableItemsList[i].m_rgbReserved = 255;  //PLF - TR#306214 [ATP] Pict file exported using Image++ never compare (floating values)
                    }
                }
            else
                {
                // Set the file palette values
                m_ColorTable.m_CtSize = static_cast<unsigned short>(ColorQty) - 1;
                m_pColorTableItemsList = new PictRGBColor[ColorQty];

                Byte* pPaletteValue;
                for (uint32_t i = 0; i < ColorQty; i++)
                    {
                    pPaletteValue = (Byte*)rPalette.GetCompositeValue(i);
                    m_pColorTableItemsList[i].m_rgbRed      = pPaletteValue[0];
                    m_pColorTableItemsList[i].m_rgbGreen    = pPaletteValue[1];
                    m_pColorTableItemsList[i].m_rgbBlue     = pPaletteValue[2];
                    m_pColorTableItemsList[i].m_rgbReserved = 255;  //PLF - TR#306214 [ATP] Pict file exported using Image++ never compare (floating values)
                    }
                }

            // Set the palette size field
            m_ColorTable.m_CtSize = static_cast<unsigned short>(ColorQty) - 1;

            }
        else if (pi_pPixelType->GetClassID() == HRPPixelTypeI4R8G8B8::CLASS_ID)
            {
            // Set the pixel type header part for this kind of pixel
            m_DataHeader.m_PackBitHeader.m_PixelSize = 0;
            m_DataHeader.m_PackBitHeader.m_CmpCount  = 0;
            m_DataHeader.m_PackBitHeader.m_CmpSize   = 0;

            // It seems that the colors are never stored to the file for 16 colors palette.
            m_ColorTable.m_CtSize = 0;
            }
        else
            {
            HASSERT(0); // Not supposed to be here!!
            }
        }
    }

//-----------------------------------------------------------------------------
// Public
// GetWorldIdentificator
// File information
//-----------------------------------------------------------------------------
const HGF2DWorldIdentificator HRFPictFile::GetWorldIdentificator () const
    {
    return HGF2DWorld_UNKNOWNWORLD;
    }


//-----------------------------------------------------------------------------
// Protected
// GetFilePtr
// Get the Bmp file pointer.
//-----------------------------------------------------------------------------
HFCBinStream* HRFPictFile::GetFilePtr  ()
    {
    return (m_pPictFile);
    }
