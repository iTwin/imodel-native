//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFImgMappedFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HFCURL.h"
#include "HFCMacros.h"
#include "HFCAccessMode.h"

#include "HFCBinStream.h"
#include "HRFRasterFile.h"
#include "HRFRasterFileCapabilities.h"


/** ---------------------------------------------------------------------------
    General capabilities of the ImgMapped file format
    (24 bits RGB, SLO upper left horizontal).
    ---------------------------------------------------------------------------
 */
BEGIN_IMAGEPP_NAMESPACE
class HRFImgMappedCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFImgMappedCapabilities();
    };


typedef struct _ImgMappedFileInfo
    {
    uint32_t m_Width;            //:> width of image in pixels
    uint32_t m_Height;           //:> height of image in pixels
    uint32_t m_ColorCount;       //:> number of colors
    Byte m_aColorMap[256*3]; //:> 256 colors palette
    } ImgMappedFileInfo;


/** ---------------------------------------------------------------------------
    This class handles ImgMapped raster file operations.

    @see HRFRasterFile
    @see HRFImgMappedLineEditor
    ---------------------------------------------------------------------------
 */
class HRFImgMappedFile : public HRFRasterFile
    {
public:
    //:> Class ID for this class
    HDECLARE_CLASS_ID(HRFFileId_ImgMapped, HRFRasterFile)

    friend class HRFImgMappedLineEditor;

    //:> Allow to open an image file
    HRFImgMappedFile       (const HFCPtr<HFCURL>& pi_rpURL,
                            HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                            uint64_t             pi_Offset = 0);

    virtual                 ~HRFImgMappedFile      ();

    //:> File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities        () const;

    //:> File information
    virtual const HGF2DWorldIdentificator
    GetWorldIdentificator  () const;

    //:> File manipulation
    virtual bool           AddPage                (HFCPtr<HRFPageDescriptor> pi_pPage);

    virtual HRFResolutionEditor*
    CreateResolutionEditor (uint32_t      pi_Page,
                            unsigned short pi_Resolution,
                            HFCAccessMode pi_AccessMode);

    virtual uint64_t          GetFileCurrentSize() const;

protected:

    //:> Methods
    //:> Constructor use only to create a child
    HRFImgMappedFile       (const HFCPtr<HFCURL>&  pi_rpURL,
                            HFCAccessMode          pi_AccessMode,
                            uint64_t              pi_Offset,
                            bool                  pi_DontOpenFile);
    virtual bool           Open                   ();
    virtual void            CreateDescriptors      ();
    virtual void            Save                   ();

private:

    void                    OpenFiles               ();
    void                    SaveImgMappedFile       (bool pi_CloseFile);
    bool                   Create                  ();
    HFCPtr<HRPPixelType>    CreatePixelTypeFromFile () const;
    void                    InitFileInfoFromDescriptors();
    void                    ReadFileHeader          ();
    void                    WriteFileHeader         ();

    HAutoPtr<HFCBinStream>  m_pImgMappedFile;

    ImgMappedFileInfo       m_FileInfo;
    uint32_t                m_OffsetToPixelData;
    };


/** ---------------------------------------------------------------------------
    This struct handles ImgMapped raster file creations.
    ---------------------------------------------------------------------------
 */
struct HRFImgMappedCreator : public HRFRasterFileCreator
    {
    //:> Opens the file and verifies if it is the right type
    virtual bool                 IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                               uint64_t                pi_Offset = 0) const;

    //:> Identification information
    virtual WString               GetLabel() const;
    virtual WString               GetSchemes() const;
    virtual WString               GetExtensions() const;

    //:> capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities();

    //:> allow to Open an image file READ/WRITE and CREATE
    virtual HFCPtr<HRFRasterFile> Create(const HFCPtr<HFCURL>& pi_rpURL,
                                         HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                         uint64_t             pi_Offset = 0) const;
private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFImgMappedCreator)

    //:> Disabled methodes
    HRFImgMappedCreator();
    };
END_IMAGEPP_NAMESPACE


