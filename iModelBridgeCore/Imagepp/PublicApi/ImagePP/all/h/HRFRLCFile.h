//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFRLCFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFRLCFile
//-----------------------------------------------------------------------------
// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCURL.h"
#include "HFCMacros.h"

#include "HRFRasterFile.h"
#include "HRFRasterFileCapabilities.h"

BEGIN_IMAGEPP_NAMESPACE
class  HRPPixelType;
class  HFCBinStream;

//Info on .rlc format extracted from IrasB's rlchd.h file.
#define RLC_SEG_RD    1        /* raster data segment */
#define RLC_SEG_RH    2        /* raster header segment */
#define RLC_SEG_RI    3        /* raster index segment */
#define RLC_SEG_RP    12        /* raster 8 bit palette segment */
#define RLC_SEG_RG    13        /* raster 8 bit histogram segment */
#define RLC_SEG_R8    14        /* raster 8 bit header segment */


/* raster data segment formats */
#define RLC_RUN        2        /* run length format */
#define RLC_IG4        5        /* CCITT / Group 4 format */
#define RLC_IGS        262        /* 8 bit grayscale format */

/* raster mode bits */
#define RLC_MODE_ON    1        /* image visible */
#define RLC_MODE_REF1    2        /* reference segment bit */
#define RLC_MODE_REF2    4        /* reference segment bit */
#define RLC_MODE_XXX    8        /* unknown */
#define RLC_MODE_BORDER    16        /* IMG_BORDER */
#define RLC_MODE_ANNOTATE    32    /* IMG_ANNOTATE */

struct rlc_image_parms
    {
    double x_pos, y_pos;        /* lower left of image in world */
    double scale;           /* always 1.0 */
    double density;         /* pixel density image in world */
    double skew;            /* skew angle of image in world */
    short  skip;            /* line skipping factor */
    short  speckle;         /* 0 - 255 */
    short  rotate;          /* 0=0; 1=90; 2=180; 3=270 */
    short  mode;            /* bitwise image mode */
    short  color;           /* 1-255 */
    short  split;           /* always 0 */
    long   reserved;        /* always 0 */
    char   filename[144];       /* original name of file */
    };

typedef struct rlc_raster_header
    {
    unsigned short byteorder;   /* always 0x4d4d (mc68k) */
    unsigned short orientation; /* always 0 */
    unsigned short length;      /* pixel height of raster data */
    unsigned short width;       /* pixel width of raster data */
    struct rlc_image_parms iparms;  /* raster data information */
    } RLC_RASTER_HEADER;

struct rlc_seghdr
    {
    short seg_type;         /* type of this segment */
    short format;           /* segment format (2 = standard RLC) */
    short version;          /* always 2 */
    short data_count;       /* number of fixed-length data items */
    long  offset;           /* offset in file to segment data */
    long  size;         /* segment size in bytes */
    };

typedef struct rlc_file_description
    {
    char magic[16];         /* \000\000\000\000\000\000\000\000 */
    /* \032\032\0x55\0x64\0x73\0x82\0x43\0x4a */
    short byteorder;        /* always 0x4949 */
    short seg_type;         /* always -1 */
    short version;          /* always 2 */
    short seg_count;        /* number of segments in file */
    struct rlc_seghdr segment[20];  /* segment tables */
    } RLC_FILE_DESCRIPTION;

typedef struct rlc_raster_index
    {
    short strip_cnt;        /* always 256 */
    short strip_size;       /* image_length / 256 - 1 */
    short strip_last;       /* # lines in last strip */
    short reserved;         /* reserved */
    long  strip_offset[256];    /* strip offsets */
    } RLC_RASTER_INDEX;

struct rlc_image_parms_8
    {
    double x_pos, y_pos;        /* lower left of image in world */
    double scale;           /* always 1.0 */
    double density;         /* pixel density image in world */
    double skew;            /* skew angle of image in world */
    double contrast;        /* contrast enhancement for image */
    short  brightness;      /* brightness setting for image */
    short  skip;            /* line skipping factor */
    short  rotate;          /* 0=0; 1=90; 2=180; 3=270 */
    short  mode;            /* bitwise image mode */
    short  hrange;          /* histogram data range */
    short  border;          /* color of border */
    short  annotation;      /* color of annotations */
    char   reserved[50];        /* all 0 */
    char   filename[144];       /* original name of file */
    };

typedef struct rlc_raster_header_8
    {
    unsigned short byteorder;   /* always 0x4d4d (mc68k) */
    unsigned short orientation; /* always 0 */
    unsigned short length;      /* pixel height of raster data */
    unsigned short width;       /* pixel width of raster data */
    struct rlc_image_parms_8 iparms;/* raster data information */
    } RLC_RASTER_HEADER_8;

typedef struct rlc_raster_histogram_8
    {
    unsigned short colors;      /* number of colors in table */
    unsigned short reserved;    /* alignment variable */
    unsigned long  count[256];  /* histogram data */
    } RLC_RASTER_HISTOGRAM_8;

typedef struct rlc_raster_palette_8
    {
    unsigned short colors;      /* number of valid entries */
    Byte palette[256];    /* lookup table entries */
    } RLC_RASTER_PALETTE_8;

/* Notes:    Raster Data Segment (RD)
seg_type = 1;
format   = 2;
version  = 2;

Raster Header Segment (RH)
seg_type = 2;
format   = 0;
version  = 2;

/* End of info extracted from rlchd.h */

class HRFRLCCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFRLCCapabilities();

    };

class HRFRLCFile : public HRFRasterFile
    {
public:
    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_RLC, HRFRasterFile)

    friend class HRFRLCLineEditor;

    // allow to Open an image file
    HRFRLCFile (const HFCPtr<HFCURL>& pi_rpURL,
                HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                uint64_t             pi_Offset = 0);

    virtual        ~HRFRLCFile();

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities () const;

    // File information
    virtual const HGF2DWorldIdentificator GetWorldIdentificator () const;
    const   HRFScanlineOrientation
    GetScanlineOrientation () const;

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
    HRFRLCFile (const HFCPtr<HFCURL>&  pi_rpURL,
                HFCAccessMode          pi_AccessMode,
                uint64_t              pi_Offset,
                bool                  pi_DontOpenFile);
    virtual bool   Open ();
    virtual void    CreateDescriptors ();

    virtual const HFCBinStream* GetRLCFilePtr() const;

    void    FindImageParams();

private:

    uint32_t                    m_Width;
    uint32_t                    m_Height;

    HAutoPtr<HFCBinStream>      m_pRLCFile;

    HFCPtr<HRPPixelType>        m_pPixelType;

    HAutoPtr<rlc_image_parms>   m_pImageParams;


    // Method
    void            Close();


    // Methods Disabled
    HRFRLCFile (const HRFRLCFile& pi_rObj);
    HRFRLCFile& operator=(const HRFRLCFile& pi_rObj);
    };

// RLC Creator.
struct HRFRLCCreator : public HRFRasterFileCreator
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
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFRLCCreator)

    // Disabled methodes
    HRFRLCCreator();
    };
END_IMAGEPP_NAMESPACE

