//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFImgRGBFile.h $
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
    General capabilities of the ImgRGB file format
    (24 bits RGB, SLO upper left horizontal).
    ---------------------------------------------------------------------------
 */
BEGIN_IMAGEPP_NAMESPACE
class HRFImgRGBCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFImgRGBCapabilities();
    };


struct ImgRGBFileHeader
    {
    uint32_t m_Width;        //:> width of image in pixels
    uint32_t m_Height;       //:> height of image in pixels
    };


/** ---------------------------------------------------------------------------
    This class handle ImgRGB raster file operations.

    @see HRFRasterFile
    @see HRFImgRGBLineEditor
    ---------------------------------------------------------------------------
 */
class HRFImgRGBFile : public HRFRasterFile
    {
public:
    //:> Class ID for this class
    HDECLARE_CLASS_ID(HRFFileId_ImgRGB, HRFRasterFile)

    friend class HRFImgRGBLineEditor;

    //:> Allow to open an image file
    HRFImgRGBFile          (const HFCPtr<HFCURL>& pi_rpURL,
                            HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                            uint64_t             pi_Offset = 0);

    virtual                 ~HRFImgRGBFile         ();

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

    virtual void            Save();

    virtual uint64_t          GetFileCurrentSize() const;

protected:

    //:> Methods
    //:> Constructor use only to create a child
    HRFImgRGBFile          (const HFCPtr<HFCURL>&  pi_rpURL,
                            HFCAccessMode          pi_AccessMode,
                            uint64_t              pi_Offset,
                            bool                  pi_DontOpenFile);
    virtual bool           Open                   ();
    virtual void            CreateDescriptors      ();

private:

    void                    OpenFiles             ();
    void                    SaveImgRGBFile        (bool pi_CloseFile);
    bool                   Create                ();
    void                    GetFileHeaderFromFile ();
    void                    SetFileHeaderToFile   ();

    // Methods disabled
    HRFImgRGBFile(const HRFImgRGBFile& pi_rObj);
    HRFImgRGBFile& operator=(const HRFImgRGBFile& pi_rObj);

    HFCPtr<HFCURL>            m_pRedFileURL;
    HFCPtr<HFCURL>            m_pGreenFileURL;
    HFCPtr<HFCURL>            m_pBlueFileURL;

    HAutoPtr<HFCBinStream>  m_pImgRGBFile;
    HAutoPtr<HFCBinStream>  m_pRedFile;
    HAutoPtr<HFCBinStream>  m_pGreenFile;
    HAutoPtr<HFCBinStream>  m_pBlueFile;

    ImgRGBFileHeader        m_FileHeader;
    };


/** ---------------------------------------------------------------------------
    This struct handle ImgRGB raster file creations.
    ---------------------------------------------------------------------------
 */
struct HRFImgRGBCreator : public HRFRasterFileCreator
    {
    //:> Opens the file and verifies if it is the right type
    virtual bool                 IsKindOfFile(const HFCPtr<HFCURL>&    pi_rpURL,
                                               uint64_t                pi_Offset = 0) const;

    //:> Identification information
    virtual WString               GetLabel() const;
    virtual WString               GetSchemes() const;
    virtual WString               GetExtensions() const;

    //:> File format is multi-file
    virtual bool                 GetRelatedURLs(const HFCPtr<HFCURL>& pi_rpURL,
                                                 ListOfRelatedURLs&    pio_rRelatedURLs) const;
    //:> capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities();

    //:> allow to Open an image file READ/WRITE and CREATE
    virtual HFCPtr<HRFRasterFile> Create(const HFCPtr<HFCURL>& pi_rpURL,
                                         HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                         uint64_t             pi_Offset = 0) const;
private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFImgRGBCreator)

    //:> Disabled methods
    HRFImgRGBCreator();
    };
END_IMAGEPP_NAMESPACE


