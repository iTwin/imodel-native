//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFPcxFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCMacros.h"
#include "HFCAccessMode.h"
#include "HRFMacros.h"
#include "HRFRasterFileCapabilities.h"
#include "HRFRasterFile.h"
#include "HRFPcxLineEditor.h"
#include "HFCBinStream.h"

/** ---------------------------------------------------------------------------
    This is a classe to declare the generic capabilities of the PCX file format.

    @see HRFRasterFileCapabilities
---------------------------------------------------------------------------  */

BEGIN_IMAGEPP_NAMESPACE
class HRFPcxCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFPcxCapabilities();

    };

/** ---------------------------------------------------------------------------
    This is a kind of object required to create new files and reopen raster files
    that are specific to PCX raster file. It may be required by applications that
    deal with raster files, especially for creating and opening raster files. Raster
    file creators may be obtained from the factory. The instance is accessible from
    the static member GetInstance().

    @see HRFRasterFileCreator
    @see HRFRasterFileFactory
---------------------------------------------------------------------------  */

struct HRFPcxCreator : public HRFRasterFileCreator
    {
public:
    //:> Opens the file and verifies if it is the right type
    virtual bool                   IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                                 uint64_t              pi_Offset) const;

    //:> Identification information
    virtual WString                 GetLabel     () const;
    virtual WString                 GetSchemes     () const;
    virtual WString                 GetExtensions() const;

    //:> capabilities or Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>& GetCapabilities();

    //:> allow to Open an image file READ/WRITE and CREATE
    virtual HFCPtr<HRFRasterFile>   Create (const HFCPtr<HFCURL>& pi_rpURL,
                                            HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                            uint64_t             pi_Offset = 0) const;

private:

    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFPcxCreator)

    //:> Disabled methodes
    HRFPcxCreator();

    };

/** ---------------------------------------------------------------------------
    This is the class that is used to create, open, and close a PCX file in
    the HRF architecture. It will ensure that every informations in the file
    are valid.

    @see HRFRasterFile
    @see HRFPcxLineEditor
---------------------------------------------------------------------------  */

class HRFPcxFile : public HRFRasterFile
    {
    friend HRFPcxCreator;
    friend HRFPcxLineEditor;

public:
    HDECLARE_CLASS_ID(HRFFileId_Pcx, HRFRasterFile)

    //:> allow to Open or to create an empty file
    HRFPcxFile            (const HFCPtr<HFCURL>& pi_rpURL,
                           HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                           uint64_t             pi_Offset = 0);

    virtual                               ~HRFPcxFile           ();

    //:> File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities       () const;

    //:> File information
    virtual const HGF2DWorldIdentificator GetWorldIdentificator () const;

    //:> File manipulation
    virtual bool                         AddPage               (HFCPtr<HRFPageDescriptor> pi_pPage);

    virtual HRFResolutionEditor*          CreateResolutionEditor(uint32_t                  pi_Page,
                                                                 unsigned short           pi_Resolution,
                                                                 HFCAccessMode             pi_AccessMode);
    virtual void                          Save();

protected:
    //:> Methods
    //:> Constructor use only to create a child
    HRFPcxFile            (const HFCPtr<HFCURL>&  pi_rpURL,
                           HFCAccessMode          pi_AccessMode,
                           uint64_t              pi_Offset,
                           bool                  pi_DontOpenFile);

    virtual bool                         Open                  ();
    virtual void                          SavePcxFile           ();
    virtual void                          CreateDescriptors     ();
    HFCBinStream*                 GetFilePtr            () const;

private:
    //:> Method
    bool                                 Create                            ();
    void                                  GetHeaderFromPageDesc             ();
    void                                  ExtractEgaPaletteFromPageDesc     ();
    void                                  GetFileHeaderFromFile             ();
    HFCPtr<HRPPixelType>                  CreatePixelTypeFromFile           ();
    void                                  SetHeaderToFile                   ();

    //:> Definition of header

    //:Ignore
    typedef struct _PcxFileHeader
        {
        Byte Identifier;                  // PCX Id Number (Always 0x0A)
        Byte Version;                     // Version Number
        Byte Encoding;                    // Encoding Format
        Byte BitsPerPixel;                // Bits per Pixel
        unsigned short XStart;                      // Left of Image
        unsigned short YStart;                      // Top of Image
        unsigned short XEnd;                        // Right of Image
        unsigned short YEnd;                        // Bottom Of Image
        unsigned short HorzRes;                     // Horizontal Resolution
        unsigned short VertRes;                     // Vertical Resolution
        Byte Palette[48];                 // 16-Color EGA Palette
        Byte Reserved1;                   // Reserved (Always 0)
        Byte NumBitPlanes;                // Number of Bit Planes
        unsigned short BytesPerLine;                // Bytes per Scanline
        unsigned short PaletteType;                 // Palette Type
        unsigned short HorzScreenSize;              // Horizontal Screen Size
        unsigned short VertScreenSize;              // Vertical Screen Size
        Byte Reserved2[54];               // Reserved (Always 0)
        } PcxFileHeader;

    //:End Ignore

    //:> Attributes
    HAutoPtr<HFCBinStream>  m_pPcxFile;
    HAutoPtr<PcxFileHeader> m_pPcxHdr;      //:> A pointer on the header info of the PCX
    uint32_t                m_VGAPaletteOffset; //:> The offset of the
    uint32_t                m_OldVGAPaletteOffset; //:> It is use to verifie that the file size has change after an edition.



    //:> Methods Disabled
    HRFPcxFile(const HRFPcxFile& pi_rObj);
    HRFPcxFile& operator=(const HRFPcxFile& pi_rObj);
    HRFPcxFile();
    };
END_IMAGEPP_NAMESPACE


