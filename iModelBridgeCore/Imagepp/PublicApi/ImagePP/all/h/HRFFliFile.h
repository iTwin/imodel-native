//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFFliFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

// This class describes a File Raster image.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCURL.h"
#include "HFCMacros.h"
#include "HFCBinStream.h"
#include "HFCAccessMode.h"

#include "HRFRasterFile.h"
#include "HRFRasterFileCapabilities.h"

//////////////////////////////////////////////////////////////////////////

BEGIN_IMAGEPP_NAMESPACE
#define FLIC_HEADER_LENGTH 128
#define FLIC_FRAME_HEADER_LENGTH 16
#define FLIC_CHUNK_HEADER_LENGTH 6



typedef struct FliFileHeader
    {
    uint32_t size;          /* Size of FLIC including this header */
    unsigned short type;          /* File type 0xAF11, 0xAF12, 0xAF30, 0xAF44, ... */
    unsigned short frames;        /* Number of frames in first segment */
    unsigned short width;         /* FLIC width in pixels */
    unsigned short height;        /* FLIC height in pixels */
    unsigned short depth;         /* Bits per pixel (usually 8) */
    unsigned short flags;         /* Set to zero or to three */
    uint32_t speed;         /* Delay between frames */
    unsigned short reserved1;     /* Set to zero */
    uint32_t created;       /* Date of FLIC creation (FLC only) */
    uint32_t creator;       /* Serial number or compiler id (FLC only) */
    uint32_t updated;       /* Date of FLIC update (FLC only) */
    uint32_t updater;       /* Serial number (FLC only), see creator */
    unsigned short aspect_dx;     /* Width of square rectangle (FLC only) */
    unsigned short aspect_dy;     /* Height of square rectangle (FLC only) */
    unsigned short ext_flags;     /* EGI: flags for specific EGI extensions */
    unsigned short keyframes;     /* EGI: key-image frequency */
    unsigned short totalframes;   /* EGI: total number of frames (segments) */
    uint32_t req_memory;    /* EGI: maximum chunk size (uncompressed) */
    unsigned short max_regions;   /* EGI: max. number of regions in a CHK_REGION chunk */
    unsigned short transp_num;    /* EGI: number of transparent levels */
    Byte  reserved2[24]; /* Set to zero */
    uint32_t oframe1;       /* Offset to frame 1 (FLC only) */
    uint32_t oframe2;       /* Offset to frame 2 (FLC only) */
    Byte  reserved3[40]; /* Set to zero */
    } FliFileHeader;

typedef struct FliFilePrefixHeader
    {
    uint32_t size;           /* Size of the chunk, including subchunks */
    unsigned short type;           /* Chunk type: 0xF1FA */
    unsigned short chunks;         /* Number of subchunks */
    Byte  reserved[8];    /* Reserved, set to 0 */
    } FliFilePrefixHeader;

typedef struct FliChunkHeader
    {
    uint32_t chunkSize;      /*Bytes in this chunk.*/
    unsigned short chunkType;      /*Type of chunk.*/
    } FliChunkHeader;

typedef struct FliRGBColor
    {
    Byte    m_rgbBlue;
    Byte    m_rgbGreen;
    Byte    m_rgbRed;
    } FliRGBColor;



class HRFFliCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFFliCapabilities();

    };




class HRFFliFile : public HRFRasterFile
    {
public:
    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_Fli, HRFRasterFile)


    friend class HRFFliCompressLineEditor;
    friend class HRFFliLineEditor;

    // allow to Open an image file
    HRFFliFile (const  HFCPtr<HFCURL>&  pi_rpURL,
                HFCAccessMode    pi_AccessMode = HFC_READ_ONLY,
                uint64_t        pi_Offset = 0);

    virtual     ~HRFFliFile();

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>& GetCapabilities () const;

    // File information
    virtual const HGF2DWorldIdentificator GetWorldIdentificator () const;

    // File manipulation
    virtual bool                         AddPage               (HFCPtr<HRFPageDescriptor> pi_pPage);

    virtual HRFResolutionEditor*          CreateResolutionEditor(uint32_t                  pi_Page,
                                                                 unsigned short           pi_Resolution,
                                                                 HFCAccessMode             pi_AccessMode);

    virtual const HFCMemoryBinStream*     GetMemoryFilePtr() const;

    virtual void                          Save();

    virtual uint64_t                        GetFileCurrentSize() const;

protected:

    // Members.



    // Methods
    // Constructor use only to create a child
    //
    HRFFliFile          (const HFCPtr<HFCURL>&  pi_rpURL,
                         HFCAccessMode          pi_AccessMode,
                         uint64_t              pi_Offset,
                         bool                  pi_DontOpenFile);

    virtual bool               Open();
    virtual void                CreateDescriptors();
    HFCBinStream*               GetFilePtr();
    int                         GetChunkSize  (int chunkNb);
    int                         GetChunkHeaderSize();

private:

    // Members
    bool                       m_InterlaceFileReaded;
    HAutoPtr<HFCBinStream>      m_pFliFile;
    FliFileHeader               m_FliFileHeader;
    FliFilePrefixHeader         m_FliPrefixHeader;
    FliChunkHeader              m_FliChunkHeader[2];
    HAutoPtr<FliRGBColor>       m_RgbColors;
    uint32_t                     m_OffsetToData;


    // Histogram
    HAutoPtr<HRPHistogram>      m_pHistogram;

    // Create the file
    void                    SaveFliFile(bool pi_CloseFile);
    bool                   Create();
    void                    GetFileHeaderFromFile();
    void                    GetFirstFrameHeaderFromFile();
    void                    GetPaletteFromFile();
    void                    GetFirstChunkHeaderFromFile();
    void                    GetNextChunkHeaderFromFile();
    bool                   GetColorChunk();

    HFCPtr<HRPPixelType>    CreatePixelTypeFromFile () const;
    void                    GetTransfoModel         ();


    // Methods Disabled
    HRFFliFile(const HRFFliFile& pi_rObj);
    HRFFliFile&             operator= (const HRFFliFile& pi_rObj);
    };


// FLI/FLIC Creator.
struct HRFFliCreator : public HRFRasterFileCreator
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
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFFliCreator)

    // Disabled methods
    HRFFliCreator();
    };

END_IMAGEPP_NAMESPACE

