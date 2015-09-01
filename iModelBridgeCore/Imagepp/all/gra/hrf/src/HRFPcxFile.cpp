//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFPcxFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFPcxFile
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>
    //:> must be first for PreCompiledHeader Option

#include <Imagepp/all/h/HRFPcxFile.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HRFException.h>
#include <Imagepp/all/h/HRFPcxLineEditor.h>

#include <Imagepp/all/h/HRPPixelTypeV1Gray1.h>
#include <Imagepp/all/h/HRPPixelTypeI4R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>

#include <Imagepp/all/h/HCDCodecIdentity.h>

#include <Imagepp/all/h/ImagePPMessages.xliff.h>

#define EGAPALETTESIZE 48
#define RESERVEDSIZE   54
#define VGAPALETTESIZE 768

// The default palette when the version is without palette
static const Byte s_PCXDefaultPalette16[] = {   0,    0,    0,
                                            0xBF,    0,    0,
                                            0, 0xBF,    0,
                                            0xBF, 0xBF,    0,
                                            0,    0, 0xBF,
                                            0xBF,    0, 0xBF,
                                            0, 0xBF, 0xBF,
                                            0xC0, 0xC0, 0xC0,
                                            0x80, 0x80, 0x80,
                                            0xFF,    0,    0,
                                            0, 0xFF,    0,
                                            0xFF, 0xFF,    0,
                                            0,    0, 0xFF,
                                            0xFF,    0, 0xFF,
                                            0, 0xFF, 0xFF,
                                            0xFF, 0xFF, 0xFF
                                        };


/**-----------------------------------------------------------------------------
 This class declare the Block Capabilitie of the PCX file format. In this
 case, the only block type supported is the Line one. It imply that the raster
 data will be readed or writed sequentially.
------------------------------------------------------------------------------*/
class HRFPcxBlockCapabilities : public HRFRasterFileCapabilities
    {
public:
    //:> Constructor
    HRFPcxBlockCapabilities ()
        : HRFRasterFileCapabilities()
        {
        //:> Block capability
        Add(new HRFLineCapability (HFC_READ_WRITE_CREATE,
                                   65535L,
                                   HRFBlockAccess::SEQUENTIAL));
        }
    };

/**-----------------------------------------------------------------------------
 This class declare the Codec Capabilitie of the PCX file format. In this
 case, the only codec type supported is the identity one. In reality, the
 raster data are encoded with the compression algorithm RLE but we did not
 implement another RLE codec for this form of it.
------------------------------------------------------------------------------*/

class HRFPcxCodecCapabilities : public HRFRasterFileCapabilities
    {
public:
    //:> Constructor
    HRFPcxCodecCapabilities()
        : HRFRasterFileCapabilities()
        {
        //:> Codec
        Add (new HRFCodecCapability (HFC_READ_WRITE_CREATE,
                                     HCDCodecIdentity::CLASS_ID,
                                     new HRFPcxBlockCapabilities()));
        }
    };

/**-----------------------------------------------------------------------------
 This class declare all the hierarchy of the capabilities supported by the
 PCX file format. This include the pixel type supported, the scanline orientation
 and many others.
------------------------------------------------------------------------------*/
HRFPcxCapabilities::HRFPcxCapabilities()
    : HRFRasterFileCapabilities()
    {
    //:> PixelType I1R8G8B8
    Add (new HRFPixelTypeCapability (HFC_READ_WRITE_CREATE,
                                     HRPPixelTypeV1Gray1::CLASS_ID,
                                     new HRFPcxCodecCapabilities()));

    //:> PixelType I4R8G8B8
    Add (new HRFPixelTypeCapability (HFC_READ_WRITE_CREATE,
                                     HRPPixelTypeI4R8G8B8::CLASS_ID,
                                     new HRFPcxCodecCapabilities()));

    //:> PixelType I8R8G8B8
    Add (new HRFPixelTypeCapability (HFC_READ_WRITE_CREATE,
                                     HRPPixelTypeI8R8G8B8::CLASS_ID,
                                     new HRFPcxCodecCapabilities()));

    //:> PixelType V8Gray8
    Add (new HRFPixelTypeCapability (HFC_READ_WRITE_CREATE,
                                     HRPPixelTypeV8Gray8::CLASS_ID,
                                     new HRFPcxCodecCapabilities()));

    //:> PixelType V24R8G8B8
    Add (new HRFPixelTypeCapability (HFC_READ_WRITE_CREATE,
                                     HRPPixelTypeV24R8G8B8::CLASS_ID,
                                     new HRFPcxCodecCapabilities()));

    //:> Scanline Orientation Capability
    Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));

    //:> Single Resolution Capability
    Add (new HRFSingleResolutionCapability(HFC_READ_WRITE_CREATE));

    //:> Interleave Capability
    Add (new HRFInterleaveCapability(HFC_READ_WRITE_CREATE, HRFInterleaveType::PIXEL));

    //:> still Image Capability
    Add (new HRFStillImageCapability(HFC_READ_WRITE_CREATE));

    }

//:Ignore
HFC_IMPLEMENT_SINGLETON(HRFPcxCreator)
//:End Ignore

/**-----------------------------------------------------------------------------
 Constructor for this class. It only initialize the capabilities pointer
 to NULL.
------------------------------------------------------------------------------*/
HRFPcxCreator::HRFPcxCreator()
    : HRFRasterFileCreator(HRFPcxFile::CLASS_ID)
    {
    //:> PCX capabilities instance member initialization
    m_pCapabilities = 0;
    }

/**-----------------------------------------------------------------------------
 Return a string that describe the file format type.

 @return string Pcx file format label.
------------------------------------------------------------------------------*/
WString HRFPcxCreator::GetLabel() const
    {
    return ImagePPMessages::GetStringW(ImagePPMessages::FILEFORMAT_PCX());  // Zsoft Paintbrush PCX
    }

/**-----------------------------------------------------------------------------
 Return the scheme of the URL.

 @return string The scheme of the URL.
------------------------------------------------------------------------------*/
WString HRFPcxCreator::GetSchemes() const
    {
    return WString(HFCURLFile::s_SchemeName());
    }

/**-----------------------------------------------------------------------------
 Return the extention accepted for the PCX file format.

 @return string The PCX extention.
------------------------------------------------------------------------------*/
WString HRFPcxCreator::GetExtensions() const
    {
    return WString(L"*.pcx");
    }

/**-----------------------------------------------------------------------------
 Opens a raster file with the specified location and access mode.

 @param pi_rpURL The URL of the file.
 @param pi_AccessMode The access and sharing mode of the file.
 @param pi_Offset The starting offset into the file.

 @return HFCPtr<HRFRasterFile> A pointer on the HRFRasterfile instance
------------------------------------------------------------------------------*/
HFCPtr<HRFRasterFile> HRFPcxCreator::Create(const HFCPtr<HFCURL>& pi_rpURL,
                                            HFCAccessMode         pi_AccessMode,
                                            uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);
    HPRECONDITION(pi_Offset == 0);

    HFCPtr<HRFRasterFile> pFile = new HRFPcxFile(pi_rpURL, pi_AccessMode);
    return (pFile);
    }

/**-----------------------------------------------------------------------------
 This function checks whether the specified URL is a PCX file.

 @param pi_rpURL Thr URL of the file.
 @param pi_Offset The starting offset into the file.

 @return bool True if the file is a PCX file.
------------------------------------------------------------------------------*/
bool HRFPcxCreator::IsKindOfFile(const HFCPtr<HFCURL>& pi_rpURL,
                                  uint64_t             pi_Offset) const
    {
    HPRECONDITION(pi_rpURL != 0);
    HPRECONDITION(pi_Offset == 0);

    bool                       Result = false;
    size_t                      Control = 0;
    HAutoPtr<HFCBinStream>      pFile;
    HRFPcxFile::PcxFileHeader   PcxHdr;

    (const_cast<HRFPcxCreator*>(this))->SharingControlCreate(pi_rpURL);
    HFCLockMonitor SisterFileLock(GetLockManager());

    //:> Open the PCX file & place file pointer at the start of the file
    pFile = HFCBinStream::Instanciate(pi_rpURL, pi_Offset, HFC_READ_ONLY | HFC_SHARE_READ_WRITE);

    if (pFile == 0 || pFile->GetLastException() != 0)
        goto WRAPUP;

    Byte buffer[54];
    memset(buffer, 0x00, 54);

    //:> Read header
    pFile->SeekToBegin();
    Control += pFile->Read(&PcxHdr.Identifier,     sizeof(Byte));
    Control += pFile->Read(&PcxHdr.Version,        sizeof(Byte));
    Control += pFile->Read(&PcxHdr.Encoding,       sizeof(Byte));
    Control += pFile->Read(&PcxHdr.BitsPerPixel,   sizeof(Byte));
    pFile->Seek(60);

    Control += pFile->Read(&PcxHdr.Reserved1,      sizeof(Byte));
    Control += pFile->Read(&PcxHdr.NumBitPlanes,   sizeof(Byte));
    pFile->Seek(2);
    Control += pFile->Read(&PcxHdr.PaletteType,    sizeof(unsigned short));
    pFile->Seek(4);
    Control += pFile->Read(PcxHdr.Reserved2, RESERVEDSIZE*sizeof(Byte));

    if (62 != Control)
        goto WRAPUP;

    // if (0x00 != PcxHdr.Reserved1)
    //     Result = false;
    // else
    if (0x0A != PcxHdr.Identifier)     //:> Identifier must be 0x0A
        goto WRAPUP;

    if ((0 != PcxHdr.Version) &&       //:>    Version 2.5 with fixed EGA palette information
        (2 != PcxHdr.Version) &&       //:>    Version 2.8 with modifiable EGA palette information
        (3 != PcxHdr.Version) &&       //:>    Version 2.8 without palette information
        (4 != PcxHdr.Version) &&       //:>    PC Paintbrush for Windows
        (5 != PcxHdr.Version))         //:>    Version 3.0 of PC Paintbrush and all 24-bit
        goto WRAPUP;

    if (1 != PcxHdr.Encoding)          //:>    Only RLE encoding is support
        goto WRAPUP;

    if (1 == PcxHdr.BitsPerPixel)
        {
        if ((1 != PcxHdr.NumBitPlanes) &&   //:>    Black & White
            (4 != PcxHdr.NumBitPlanes))     //:>    4-bit palette
            goto WRAPUP;
        }
    else if (8 == PcxHdr.BitsPerPixel)
        {
        if ((1 != PcxHdr.NumBitPlanes) &&   //:>    Gray scale and 8-bit palette
            (3 != PcxHdr.NumBitPlanes))     //:>    24-bit True Color
            goto WRAPUP;
        }
    else
        goto WRAPUP;

    Result = true;

WRAPUP:
    SisterFileLock.ReleaseKey();
    HASSERT(!(const_cast<HRFPcxCreator*>(this))->m_pSharingControl->IsLocked());
    (const_cast<HRFPcxCreator*>(this))->m_pSharingControl = 0;

    return Result;
    }

/**-----------------------------------------------------------------------------
 Return the capabilities of PCX file format.

 @return HFCPtr<HRFRasterFileCapabilities> A pointer on the singleton
         capabilities of PCX file.
------------------------------------------------------------------------------*/
const HFCPtr<HRFRasterFileCapabilities>& HRFPcxCreator::GetCapabilities()
    {
    if (m_pCapabilities == 0)
        m_pCapabilities  = new HRFPcxCapabilities();

    return m_pCapabilities;
    }

/**-----------------------------------------------------------------------------
 Public constructor for the HRFPcxFile. It does the gestion of the creation or
 the opening of the PCX file.

 @param pi_rURL The name of the file to create or open
 @param pi_AccessMode The access mode and the sharing mode of the file
 @param pi_Offset The begining offset in the file
------------------------------------------------------------------------------*/
HRFPcxFile::HRFPcxFile(const HFCPtr<HFCURL>& pi_rpURL,
                              HFCAccessMode         pi_AccessMode,
                              uint64_t             pi_Offset)
    : HRFRasterFile(pi_rpURL, pi_AccessMode, pi_Offset)
    {
    //:> The ancestor store the access mode
    m_IsOpen                = false;
    m_pPcxHdr               = 0;
    m_VGAPaletteOffset      = 0;
    m_OldVGAPaletteOffset   = 0;

    if (GetAccessMode().m_HasCreateAccess)
        {
        Create();
        }
    else
        {
        //:> Create Page and Res Descriptors.
        Open();
        CreateDescriptors();
        }

    }

/**-----------------------------------------------------------------------------
 Protected constructor for the HRFPcsFile. It allow to create an image file
 object without open.

  @param pi_rURL The name of the file to create or open
  @param pi_AccessMode The access mode and the sharing mode of the file
  @param pi_Offset The begining offset in the file
------------------------------------------------------------------------------*/
HRFPcxFile::HRFPcxFile(const HFCPtr<HFCURL>& pi_rpURL,
                              HFCAccessMode         pi_AccessMode,
                              uint64_t             pi_Offset,
                              bool                 pi_DontOpenFile)
    : HRFRasterFile(pi_rpURL, pi_AccessMode, pi_Offset)
    {
    //:> The ancestor store the access mode
    m_IsOpen    = false;
    }

/**-----------------------------------------------------------------------------
 Public destructor of the class. This function does what it takes to close the
 raster file.
------------------------------------------------------------------------------*/
HRFPcxFile::~HRFPcxFile()
    {
    try
        {
        SavePcxFile();

        }
    catch(...)
        {
        //:> Simply stop exceptions in the destructor
        //:> We want to known if a exception is throw.
        HASSERT(0);
        }
    }

/**-----------------------------------------------------------------------------
 Function close of the class. Here we must write to the file informations
 that has been modified or displace since the creation or the opening of the file.
 This includes footer, tag, palettes, etc..
------------------------------------------------------------------------------*/
void HRFPcxFile::SavePcxFile()
    {
    uint32_t                 NumberOfColor;
    bool                   IsCreate        = GetAccessMode().m_HasCreateAccess;
    bool                   CanWrite        = GetAccessMode().m_HasWriteAccess;
    const HRPPixelPalette&  rPalette        = GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetPixelType()->GetPalette();
    HArrayAutoPtr<Byte>    pVgaPalette     (new Byte[VGAPALETTESIZE+1]);

    // Be sure that the file is already open and that at least one page
    // has been add. Because if the destroyer is call after an exception
    // is thrown, we want to be sure that the object is valid before we
    // execute the destroyer.
    if (m_IsOpen && m_ListOfPageDescriptor.size() > 0 && (CanWrite || IsCreate))
        {

        //:> Set the VGA color-map at the end of the file only if we are in 256 colors mode (I8R8G8B8)
        NumberOfColor = rPalette.GetMaxEntries();
        if (256 == NumberOfColor)
            {
            if (GetPageDescriptor(0)->GetResolutionDescriptor(0)->PaletteHasChanged() ||
                (m_VGAPaletteOffset != m_OldVGAPaletteOffset))
                {
                //:> Initialise the buffer for the VGA palette
                pVgaPalette[0] = 0x0C;
                memset (pVgaPalette+1, 0x00, VGAPALETTESIZE);

                //:> Fill out the buffer
                Byte* pPtr = pVgaPalette+1;
                for (uint32_t i = 0; i < rPalette.CountUsedEntries(); i++, pPtr += 3)
                    memcpy (pPtr, (Byte*)rPalette.GetCompositeValue(i), 3);

                //:> Lock the sister file for the GetField operation
                HFCLockMonitor SisterFileLock (GetLockManager());

                //:> Write the palette at the end of the raster data.
                HASSERT (m_VGAPaletteOffset != 0);
                m_pPcxFile->SeekToPos(m_VGAPaletteOffset);

                m_pPcxFile->Write(pVgaPalette, VGAPALETTESIZE+1);

                //:> Set the end of file caracter at the current position.
                if (m_pPcxFile->IsCompatibleWith(HFCLocalBinStream::CLASS_ID))
                    ((HFCPtr<HFCLocalBinStream>&)m_pPcxFile)->SetEOF();

                //:> Unlock the sister file.
                SisterFileLock.ReleaseKey();
                }
            }
        //:> Reset the EGA palette to the header in the file
        else if (16 == NumberOfColor)
            {
            if (GetPageDescriptor(0)->GetResolutionDescriptor(0)->PaletteHasChanged())
                {
                //:> Update the EGA palette
                ExtractEgaPaletteFromPageDesc();

                //:> ReWrite the header to file to update the palette.
                SetHeaderToFile();
                }
            }
        //:> Set a Gray Scale color-map at the end of the file for this particular pixel type (V8Gray8)
        else if (GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetPixelType()->IsCompatibleWith(HRPPixelTypeV8Gray8::CLASS_ID))
            {
            if (IsCreate)
                {
                pVgaPalette[0] = 0x0C;
                Byte* pPtr = pVgaPalette+1;
                for (uint32_t i = 0; i < 256; i++, pPtr+=3)
                    memset (pPtr, (Byte)i, 3);

                //:> Lock the sister file for the GetField operation
                HFCLockMonitor SisterFileLock (GetLockManager());

                //:> Write the palette at the end of the raster data.
                HASSERT (m_VGAPaletteOffset != 0);
                m_pPcxFile->SeekToPos(m_VGAPaletteOffset);

                m_pPcxFile->Write(pVgaPalette, VGAPALETTESIZE+1);

                //:> Unlock the sister file.
                SisterFileLock.ReleaseKey();
                }
            }

        m_pPcxFile->Flush();

        GetPageDescriptor(0)->Saved();
        GetPageDescriptor(0)->GetResolutionDescriptor(0)->Saved();

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
HRFResolutionEditor* HRFPcxFile::CreateResolutionEditor(uint32_t       pi_Page,
                                                        unsigned short pi_Resolution,
                                                        HFCAccessMode  pi_AccessMode)
    {
    return new HRFPcxLineEditor(this, pi_Page, pi_Resolution, pi_AccessMode);
    }


/**-----------------------------------------------------------------------------
This method saves the file
------------------------------------------------------------------------------*/
void HRFPcxFile::Save()
    {
    //Keep last file position
    uint64_t CurrentPos = m_pPcxFile->GetCurrentPos();

    SavePcxFile();

    //Set back position
    m_pPcxFile->SeekToPos(CurrentPos);
    }

/**-----------------------------------------------------------------------------
 Adds a new page to the raster file. This function can be called only if the
 file access mode has create access.

 @param pi_pPage A pointer on the page descriptor to be added.

 @return bool true if the page has been added.
------------------------------------------------------------------------------*/
bool HRFPcxFile::AddPage(HFCPtr<HRFPageDescriptor> pi_pPage)
    {
    //:> Assert that no page has allready be enterd
    HPRECONDITION (CountPages() == 0);
    HPRECONDITION (pi_pPage != 0);

    //:> Add the page descriptor to the list
    HRFRasterFile::AddPage(pi_pPage);

    m_pPcxHdr   = new PcxFileHeader;

    GetHeaderFromPageDesc();

    return true;
    }

/**-----------------------------------------------------------------------------
 This function returns the capabilities for the PCX file format

 @return HFCPtr<HRFRasterFileCapabilities> A pointer on the capabilities
         description for PCX file.
------------------------------------------------------------------------------*/
const HFCPtr<HRFRasterFileCapabilities>& HRFPcxFile::GetCapabilities () const
    {
    return (HRFPcxCreator::GetInstance()->GetCapabilities());
    }

/**----------------------------------------------------------------------------
 This method create the  file header from the page descriptor and write it to
 the file.
------------------------------------------------------------------------------*/
void HRFPcxFile::GetHeaderFromPageDesc()
    {
    HPRECONDITION (m_pPcxFile != 0);

    HFCPtr<HRFPageDescriptor>       pPageDesc   = GetPageDescriptor(0);
    HFCPtr<HRFResolutionDescriptor> pResDesc    = pPageDesc->GetResolutionDescriptor(0);
    HRPPixelPalette                 Palette;

    m_pPcxHdr->Identifier       = 0x0A;
    m_pPcxHdr->Encoding         = 1;
    m_pPcxHdr->XStart           = 0;                                //:> Could be improve
    m_pPcxHdr->YStart           = 0;                                //:> Could be improve
    m_pPcxHdr->XEnd             = (unsigned short)pResDesc->GetWidth() -1;  //:> Could be improve
    m_pPcxHdr->YEnd             = (unsigned short)pResDesc->GetHeight() -1; //:> Could be improve
    m_pPcxHdr->HorzRes          = 0;                                //:> Could be improve
    m_pPcxHdr->VertRes          = 0;                                //:> Could be improve
    m_pPcxHdr->Reserved1        = 0x00;
    m_pPcxHdr->HorzScreenSize   = 0;                                //:> Could be improve
    m_pPcxHdr->VertScreenSize   = 0;                                //:> Could be improve
    memset (m_pPcxHdr->Reserved2, 0x00, RESERVEDSIZE);
    memset (m_pPcxHdr->Palette,   0x00, EGAPALETTESIZE);

    //:> Find the best version with pixeltype
    HFCPtr<HRPPixelType> pPixelType = pResDesc->GetPixelType();
    if (pPixelType->IsCompatibleWith(HRPPixelTypeV1Gray1::CLASS_ID))
        {
        m_pPcxHdr->Version      = 0;
        m_pPcxHdr->BitsPerPixel = 1;
        m_pPcxHdr->PaletteType  = 1;
        m_pPcxHdr->NumBitPlanes = 1;
        //:> Set the second entry of the EGA palette to 255 255 255 for white.
        //:> Everything else will remain set to 0 0 0 for black or empty
        memset (m_pPcxHdr->Palette+3, 0xFF, 3);
        }
    else if (pPixelType->IsCompatibleWith(HRPPixelTypeI4R8G8B8::CLASS_ID))
        {
        m_pPcxHdr->Version      = 2;
        m_pPcxHdr->BitsPerPixel = 1;
        m_pPcxHdr->PaletteType  = 1;
        m_pPcxHdr->NumBitPlanes = 4;
        ExtractEgaPaletteFromPageDesc();
        }
    else if (pPixelType->IsCompatibleWith(HRPPixelTypeI8R8G8B8::CLASS_ID))
        {
        m_pPcxHdr->Version      = 5;
        m_pPcxHdr->BitsPerPixel = 8;
        m_pPcxHdr->PaletteType  = 1;
        m_pPcxHdr->NumBitPlanes = 1;
        //:> The 48 bytes palette shall remain filled only with 0
        //:> because there is more than 16 colors. We will write the
        //:> palette at the end of the file.
        }
    else if (pPixelType->IsCompatibleWith(HRPPixelTypeV8Gray8::CLASS_ID))
        {
        m_pPcxHdr->Version      = 5;
        m_pPcxHdr->BitsPerPixel = 8;
        m_pPcxHdr->PaletteType  = 2;
        m_pPcxHdr->NumBitPlanes = 1;
        //:> The 48 bytes palette shall remain filled only with 0
        }
    else if (pPixelType->IsCompatibleWith(HRPPixelTypeV24R8G8B8::CLASS_ID))
        {
        m_pPcxHdr->Version      = 5;
        m_pPcxHdr->BitsPerPixel = 8;
        m_pPcxHdr->PaletteType  = 1;
        m_pPcxHdr->NumBitPlanes = 3;
        //:> The 48 bytes palette shall remain filled only with 0
        }
    else
        HASSERT (false);

    //:> Set the number of bytes per line. It only takes care of the bits per pixel
    //:> field for this width. The number of plane does not influence the calculation.
    //:> The BytesPerLine field represents the number of bytes per planes for one line
    //:> alligned on 16 bits.
    // m_pPcxHdr->BytesPerLine = (UShort)(((pResDesc->GetWidth() * m_pPcxHdr->BitsPerPixel) + 15) / 16);
    // m_pPcxHdr->BytesPerLine <<= 2;

    m_pPcxHdr->BytesPerLine = (unsigned short)(((pResDesc->GetWidth() * m_pPcxHdr->BitsPerPixel) + 7) / 8);
    m_pPcxHdr->BytesPerLine += m_pPcxHdr->BytesPerLine % 2;

    SetHeaderToFile();

    //:> Ready to write the raster data...
    }

/**-----------------------------------------------------------------------------
 This method put the EGA palette information into  the header.
------------------------------------------------------------------------------*/
void HRFPcxFile::ExtractEgaPaletteFromPageDesc()
    {
    const HRPPixelPalette& rPalette = GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetPixelType()->GetPalette();

    HASSERT (rPalette.CountUsedEntries() >= 16);

    memset (m_pPcxHdr->Palette, 0x00, EGAPALETTESIZE);
    Byte* pPtr = m_pPcxHdr->Palette;
    for (uint32_t i = 0; i < rPalette.CountUsedEntries(); i++, pPtr+=3)
        memcpy (pPtr, (Byte*)rPalette.GetCompositeValue(i), 3);
    }

/**-----------------------------------------------------------------------------
 This method create a new file on the disk and keep a pointer to it.

 @return true If the file is instantiate correctly.
------------------------------------------------------------------------------*/
bool HRFPcxFile::Create()
    {
    //:> This method creates the sharing control sister file
    SharingControlCreate();

    //:> Open the file
    m_pPcxFile = HFCBinStream::Instanciate(GetURL(), GetAccessMode(), 0, true);

    m_IsOpen = true;

    return true;
    }

/**-----------------------------------------------------------------------------
 This method open the physical file on the disk and keep a pointer to it.

 @return true if the file exist and is opened.
------------------------------------------------------------------------------*/
bool HRFPcxFile::Open()
    {
    if (!m_IsOpen)
        {
        m_pPcxFile = HFCBinStream::Instanciate(GetURL(), m_Offset, GetAccessMode(), 0, true);

        //:> This creates the sister file for file sharing control if necessary.
        SharingControlCreate();

        //:> Initialisation of file struct.
        GetFileHeaderFromFile();

        //:> If the file is a 8-bit palette, get the offset of the palette.
        if ((8 == m_pPcxHdr->BitsPerPixel) && (1 == m_pPcxHdr->NumBitPlanes) && (1 == m_pPcxHdr->PaletteType))
            {
            m_VGAPaletteOffset = (uint32_t)(m_pPcxFile->GetSize() - (VGAPALETTESIZE+1));
            m_OldVGAPaletteOffset = m_VGAPaletteOffset;
            }

        m_IsOpen = true;
        }

    return m_IsOpen;
    }

/**-----------------------------------------------------------------------------
 Get the file header from the physical file and keep it into the PcxFileHeader
 structure.
------------------------------------------------------------------------------*/
void HRFPcxFile::GetFileHeaderFromFile()
    {
    HPRECONDITION (m_pPcxFile != 0);

    m_pPcxHdr = new PcxFileHeader;

    //:> Lock the sister file for the GetField operation
    HFCLockMonitor SisterFileLock (GetLockManager());

    m_pPcxFile->SeekToBegin();
    m_pPcxFile->Read(&m_pPcxHdr->Identifier,     sizeof(Byte));
    m_pPcxFile->Read(&m_pPcxHdr->Version,        sizeof(Byte));
    m_pPcxFile->Read(&m_pPcxHdr->Encoding,       sizeof(Byte));
    m_pPcxFile->Read(&m_pPcxHdr->BitsPerPixel,   sizeof(Byte));
    m_pPcxFile->Read(&m_pPcxHdr->XStart,         sizeof(unsigned short));
    m_pPcxFile->Read(&m_pPcxHdr->YStart,         sizeof(unsigned short));
    m_pPcxFile->Read(&m_pPcxHdr->XEnd,           sizeof(unsigned short));
    m_pPcxFile->Read(&m_pPcxHdr->YEnd,           sizeof(unsigned short));
    m_pPcxFile->Read(&m_pPcxHdr->HorzRes,        sizeof(unsigned short));
    m_pPcxFile->Read(&m_pPcxHdr->VertRes,        sizeof(unsigned short));
    m_pPcxFile->Read( m_pPcxHdr->Palette,        sizeof(Byte)*EGAPALETTESIZE);
    m_pPcxFile->Read(&m_pPcxHdr->Reserved1,      sizeof(Byte));
    m_pPcxFile->Read(&m_pPcxHdr->NumBitPlanes,   sizeof(Byte));
    m_pPcxFile->Read(&m_pPcxHdr->BytesPerLine,   sizeof(unsigned short));
    m_pPcxFile->Read(&m_pPcxHdr->PaletteType,    sizeof(unsigned short));
    m_pPcxFile->Read(&m_pPcxHdr->HorzScreenSize, sizeof(unsigned short));
    m_pPcxFile->Read(&m_pPcxHdr->VertScreenSize, sizeof(unsigned short));
    m_pPcxFile->Read( m_pPcxHdr->Reserved2,       sizeof(Byte)*RESERVEDSIZE);

    //:> Unlock the sister file
    SisterFileLock.ReleaseKey();
    }

/**-----------------------------------------------------------------------------
 This method creates the resolution descriptor and the page descriptor for
 this file.
------------------------------------------------------------------------------*/
void HRFPcxFile::CreateDescriptors ()
    {
    HPRECONDITION (m_pPcxFile != 0);
    HPRECONDITION (m_pPcxHdr != 0);

    //:> Create Page and Resolution Descriptor/Capabilities for this file.
    HFCPtr<HRFResolutionDescriptor>     pResolution;
    HFCPtr<HRFPageDescriptor>           pPage;
    HFCPtr<HRPPixelType>                pPixelType;

    //:> Pixel type
    pPixelType = CreatePixelTypeFromFile();

    pResolution = new HRFResolutionDescriptor(
        GetAccessMode(),                                            //:> AccessMode
        GetCapabilities(),                                          //:> Capabilities
        1.0,                                                        //:> XResolutionRatio
        1.0,                                                        //:> YResolutionRatio
        pPixelType,                                                 //:> PixelType
        new HCDCodecIdentity(),                                     //:> Codec
        HRFBlockAccess::SEQUENTIAL,                                 //:> RBlockAccess
        HRFBlockAccess::SEQUENTIAL,                                 //:> WBlockAccess

        HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL,              //:> ScanLineOrientation
        HRFInterleaveType::PIXEL,                                   //:> Interleave Type
        true,                                                       //:> Interlace
        m_pPcxHdr->XEnd - m_pPcxHdr->XStart + 1,                    //:> Image width
        m_pPcxHdr->YEnd - m_pPcxHdr->YStart + 1,                    //:> Image height
        m_pPcxHdr->XEnd - m_pPcxHdr->XStart + 1,                    //:> Block width
        1,                                                          //:> Block height
        0,                                                          //:> Block data flag
        HRFBlockType::LINE);                                        //:> Block type

    pPage = new HRFPageDescriptor (GetAccessMode(),                                     //:> Access Mode
                                   GetCapabilities(),                                   //:> Capabilities
                                   pResolution,                                         //:> Resolution Descriptor
                                   0,                                                   //:> Representative Palette
                                   0,                                                   //:> Histogram
                                   0,                                                   //:> Thumbnail
                                   0,                                                   //:> Clip Shape
                                   0,                                                   //:> Transfo Model
                                   0,                                                   //:> Filters
                                   0);                                                  //:> Attribute set

    m_ListOfPageDescriptor.push_back(pPage);
    }

/**-----------------------------------------------------------------------------
 This fonction creates the proper pixel type for the file and returns it for
 the ${#CreateDescripor} method.

 @return HFCPtr<HRPPixelType> The pixel type of the file.
------------------------------------------------------------------------------*/
HFCPtr<HRPPixelType> HRFPcxFile::CreatePixelTypeFromFile()
    {
    HPRECONDITION (m_pPcxFile != NULL);
    HPRECONDITION (m_pPcxHdr  != NULL);

    bool                   Error = false;
    Byte                  ControlNumber;
    HAutoPtr<Byte>        pValue;
    HFCPtr<HRPPixelType>    pPixelType;
    HRPPixelPalette*        rPalette;
    uint32_t                 i;

    //:> Pixel type
    if (2 == m_pPcxHdr->PaletteType)        //:> 8 bits Grayscale
        pPixelType = new HRPPixelTypeV8Gray8();
    else if (1 == m_pPcxHdr->BitsPerPixel)
        switch (m_pPcxHdr->NumBitPlanes)
            {
            case 1 :        //:> Monochrome
                pPixelType = new HRPPixelTypeV1Gray1();
                break;
            case 4 :        //:> 16 colors palette
                {
                pPixelType = new HRPPixelTypeI4R8G8B8();

                //:> Get the color palette from the pixel type
                pValue = new Byte[3];

                rPalette = &pPixelType->LockPalette();

                //:> Set the color palette from the file

                Byte* pPtr;

                // Use default palette if necessary
                if (m_pPcxHdr->Version == 3)
                    pPtr = const_cast<Byte*>(s_PCXDefaultPalette16);
                else
                    pPtr = m_pPcxHdr->Palette;

                for (i = 0; i < 16; i++, pPtr+=3)
                    rPalette->SetCompositeValue(i, pPtr);

                pPixelType->UnlockPalette();
                break;
                }
            default:
                Error = true;
            }
    else if (8 == m_pPcxHdr->BitsPerPixel)
        switch (m_pPcxHdr->NumBitPlanes)
            {
            case 1 :        //:> 256 colors palette
                {
                pPixelType = new HRPPixelTypeI8R8G8B8();

                //:> Lock the sister file for the GetField operation
                HFCLockMonitor SisterFileLock (GetLockManager());

                pValue = new Byte[VGAPALETTESIZE];
                m_pPcxFile->SeekToEnd();
                m_pPcxFile->Seek(-(VGAPALETTESIZE+1));
                m_pPcxFile->Read(&ControlNumber, 1);
                if (0x0C != ControlNumber)
                    Error = true;
                else
                    {
                    m_pPcxFile->Read(pValue, VGAPALETTESIZE);

                    rPalette = &pPixelType->LockPalette();

                    //:> Add the entry to the pixel color palette
                    Byte* pPtr = pValue;
                    for (i = 0; i < 256; i++, pPtr+=3)
                        rPalette->SetCompositeValue(i, pPtr);

                    pPixelType->UnlockPalette();
                    }

                //:> Unlock the sister file.
                SisterFileLock.ReleaseKey();
                break;
                }
            case 3 :        //:> 24 bits True Colors
                pPixelType = new HRPPixelTypeV24R8G8B8();
                break;
            default:
                Error = true;
            }
    else
        Error = true;

    if (Error)
        throw HRFPixelTypeNotSupportedException(GetURL()->GetURL());

    return pPixelType;
    }

/**-----------------------------------------------------------------------------
 This method writes the PCX header into the physical file.
------------------------------------------------------------------------------*/
void HRFPcxFile::SetHeaderToFile()
    {

    //:> Lock the sister file for the GetField operation
    HFCLockMonitor SisterFileLock (GetLockManager());

    m_pPcxFile->SeekToBegin();
    m_pPcxFile->Write(&m_pPcxHdr->Identifier,     sizeof(Byte));
    m_pPcxFile->Write(&m_pPcxHdr->Version,        sizeof(Byte));
    m_pPcxFile->Write(&m_pPcxHdr->Encoding,       sizeof(Byte));
    m_pPcxFile->Write(&m_pPcxHdr->BitsPerPixel,   sizeof(Byte));
    m_pPcxFile->Write(&m_pPcxHdr->XStart,         sizeof(unsigned short));
    m_pPcxFile->Write(&m_pPcxHdr->YStart,         sizeof(unsigned short));
    m_pPcxFile->Write(&m_pPcxHdr->XEnd,           sizeof(unsigned short));
    m_pPcxFile->Write(&m_pPcxHdr->YEnd,           sizeof(unsigned short));
    m_pPcxFile->Write(&m_pPcxHdr->HorzRes,        sizeof(unsigned short));
    m_pPcxFile->Write(&m_pPcxHdr->VertRes,        sizeof(unsigned short));
    m_pPcxFile->Write( m_pPcxHdr->Palette,        sizeof(Byte)*EGAPALETTESIZE);
    m_pPcxFile->Write(&m_pPcxHdr->Reserved1,      sizeof(Byte));
    m_pPcxFile->Write(&m_pPcxHdr->NumBitPlanes,   sizeof(Byte));
    m_pPcxFile->Write(&m_pPcxHdr->BytesPerLine,   sizeof(unsigned short));
    m_pPcxFile->Write(&m_pPcxHdr->PaletteType,    sizeof(unsigned short));
    m_pPcxFile->Write(&m_pPcxHdr->HorzScreenSize, sizeof(unsigned short));
    m_pPcxFile->Write(&m_pPcxHdr->VertScreenSize, sizeof(unsigned short));
    m_pPcxFile->Write( m_pPcxHdr->Reserved2,      sizeof(Byte)*RESERVEDSIZE);

    //:> Unlock the sister file.
    SisterFileLock.ReleaseKey();
    }

/**-----------------------------------------------------------------------------
 This is a public method that returns the HGF2DWorldIdentificator of a raster
 file.

 @return HGF2DWorld_UNKNOWNWORLD because this feature is not supported by the
         PCX file format.
------------------------------------------------------------------------------*/
const HGF2DWorldIdentificator HRFPcxFile::GetWorldIdentificator () const
    {
    return HGF2DWorld_UNKNOWNWORLD;
    }

/**-----------------------------------------------------------------------------
 Protected method that return a pointer on the physical instance of the PCX file

 @return A pointer to the class that hold the physical instance of the PCX file.

 @see HFCBinStream
 @see HFCLocalBinStream
------------------------------------------------------------------------------*/
HFCBinStream* HRFPcxFile::GetFilePtr  () const
    {
    return m_pPcxFile;
    }
