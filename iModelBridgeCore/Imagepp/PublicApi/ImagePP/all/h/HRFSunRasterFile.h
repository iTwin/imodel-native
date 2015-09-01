//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFSunRasterFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFSunRasterFile
//-----------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCURL.h"
#include "HFCMacros.h"
#include "HFCBinStream.h"
#include "HFCAccessMode.h"

#include "HRFMacros.h"
#include "HRFRasterFile.h"
#include "HRFRasterFileCapabilities.h"


BEGIN_IMAGEPP_NAMESPACE
class HRFSunRasterCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFSunRasterCapabilities();
    };

// Information:
//  Line data is rounded up to the nearesr 16 bits.
//

typedef struct _SunRasterFileHeader
    {
    uint32_t   m_MagicNumber;  // magic number
    uint32_t   m_Width;        // width of image in pixels
    uint32_t   m_Height;       // height of image in pixels
    uint32_t   m_Depth;        // depth of each pixel
    uint32_t   m_Length;       // length of the image in bytes (if compressed)
    // minus length of header and colormap
    uint32_t   m_Type;         // format of file
    uint32_t   m_Maptype;      // type of colormap
    uint32_t   m_Maplen;       // length of colormap in bytes
    } SunRasterFileHeader;


class HRFSunRasterFile : public HRFRasterFile
    {
public:
    // Constants
    // File identify code.
    //
    static const uint32_t RAS_MAGIC;

    // These are the possible file formats
    //
    static const uint32_t RT_OLD;                // old format (same as Standard with m_Length = 0)
    static const uint32_t RT_STANDARD;           // standard format
    static const uint32_t RT_RLENCODED;          // run length encoding to compress the image
    static const uint32_t RT_FOMRAT_RGB;         // RGB instead of BGR
    static const uint32_t RT_TIFF;               // Same as Standard, origin of the image is a TIFF file
    static const uint32_t RT_IFF;                // Same as Standard, origin of the image is a IFF file
    static const uint32_t RT_EXPERIMENTAL;       // Good luck :-)

    // These are the possible colormap types.  if it's in RGB format,
    // the map is made up of three byte arrays (red, green, then blue)
    // that are each 1/3 of the colormap length.
    //
    static const uint32_t RMT_NOMAP;             // no colormap follows the header (m_Maplen = 0)
    static const uint32_t RMT_RGBMAP;            // rgb colormap (m_Maplen/3 by channel)
    static const uint32_t RMT_RAWMAP;            // raw colormap; good luck :-)

    static const uint32_t HEADER_OFFSET;          // Header begin at 0
    static const uint32_t COLORMAP_OFFSET;        // Colormap begin at 32


    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_SunRaster, HRFRasterFile)

    friend class HRFSunRasterLineEditor;
    friend class HRFSunRasterImageEditor;
    friend struct HRFSunRasterCreator;

    // allow to Open an image file
    HRFSunRasterFile (const  HFCPtr<HFCURL>&  pi_rpURL,
                      HFCAccessMode    pi_AccessMode = HFC_READ_ONLY,
                      uint64_t        pi_Offset = 0);

    virtual     ~HRFSunRasterFile();

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>& GetCapabilities () const;

    // File information
    virtual const HGF2DWorldIdentificator GetWorldIdentificator () const;

    // File manipulation
    virtual bool                         AddPage               (HFCPtr<HRFPageDescriptor> pi_pPage);

    virtual HRFResolutionEditor*          CreateResolutionEditor(uint32_t                  pi_Page,
                                                                 unsigned short           pi_Resolution,
                                                                 HFCAccessMode             pi_AccessMode);

    virtual void                          Save();

protected:

    // Methods
    // Constructor use only to create a child
    //
    HRFSunRasterFile          (const HFCPtr<HFCURL>&  pi_rpURL,
                               HFCAccessMode          pi_AccessMode,
                               uint64_t              pi_Offset,
                               bool                  pi_DontOpenFile);
    virtual bool                       Open                ();
    virtual void                        CreateDescriptors   ();
    HFCBinStream*               GetFilePtr          ();

private:

    // Members
    HAutoPtr<HFCBinStream>  m_pSunRasterFile;

    SunRasterFileHeader     m_FileHeader;
    bool                   m_HeaderChanged;
    Byte                  m_LinePadBits;              // set to 16 bits
    Byte                  m_PaddingBitsPerRow;        // Line padding 16bits
    bool                   m_IsBGR;
    HArrayAutoPtr<Byte>   m_pColorMapB;
    HArrayAutoPtr<Byte>   m_pColorMapG;
    HArrayAutoPtr<Byte>   m_pColorMapR;

    // Create the file
    void                    SaveSunRasterFile(bool pi_CloseFile);
    bool                   Create();

    HFCPtr<HRPPixelType>    CreatePixelTypeFromFile() const;
    void                    SetPixelTypeToPage(HFCPtr<HRFResolutionDescriptor> pi_pResolutionDescriptor);
    void                    GetFileHeaderFromFile();
    void                    SetFileHeaderToFile();
    void                    GetPaletteFromFile();
    void                    SetPaletteToFile();

    // Methods Disabled
    HRFSunRasterFile(const HRFSunRasterFile& pi_rObj);
    HRFSunRasterFile&             operator= (const HRFSunRasterFile& pi_rObj);
    };


// SunRaster Creator.
struct HRFSunRasterCreator : public HRFRasterFileCreator
    {
    // Opens the file and verifies if it is the right type
    virtual bool                     IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                                   uint64_t                pi_Offset = 0) const;

    // Identification information
    virtual WString                   GetLabel() const;
    virtual WString                   GetSchemes() const;
    virtual WString                   GetExtensions() const;

    // capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities();

    // allow to Open an image file READ/WRITE and CREATE
    virtual HFCPtr<HRFRasterFile>     Create(const HFCPtr<HFCURL>& pi_rpURL,
                                             HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                             uint64_t             pi_Offset = 0) const;
private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFSunRasterCreator)

    // Disabled methodes
    HRFSunRasterCreator();
    };
END_IMAGEPP_NAMESPACE

