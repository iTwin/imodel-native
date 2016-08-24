//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFxChFile.h $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HFCURL.h"
#include "HFCMacros.h"
#include "HFCAccessMode.h"

#include "HRFRasterFile.h"
#include "HRFRasterFileCapabilities.h"


// RGB[A] example
//  <MultiChannelImageFileFormat>
//      <VERSION>1.0< / VERSION>
//      <CHANNELS>
//          < COUNT>4 < / COUNT >
//          <RED>LC80450292013225LGN00_B4.tif< / RED>
//          <GREEN>LC80450292013225LGN00_B3.tif< / GREEN>
//          <BLUE>LC80450292013225LGN00_B2.tif< / BLUE>
//          <ALPHA>LC80450292013225LGN00_B9.tif< / ALPHA>
//      < / CHANNELS>
//  < / MultiChannelImageFileFormat>
//

// Landsat8 Panchromatic (the Panchromatic file has a 2x better resolution)
//      ALPHA channel not supported for the moment.
//
//  <MultiChannelImageFileFormat>
//      <VERSION>1.0< / VERSION>
//      <CHANNELS>
//          < COUNT>4 < / COUNT >
//          <PANCHROMATIC>LC80450292013225LGN00_B8.tif< / PANCHROMATIC>
//          <RED>LC80450292013225LGN00_B4.tif< / RED>
//          <GREEN>LC80450292013225LGN00_B3.tif< / GREEN>
//          <BLUE>LC80450292013225LGN00_B2.tif< / BLUE>
//      < / CHANNELS>
//  < / MultiChannelImageFileFormat>

/** ---------------------------------------------------------------------------
    General capabilities of the xCh file format.
    ---------------------------------------------------------------------------
 */
BEGIN_IMAGEPP_NAMESPACE
class HRFxChCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFxChCapabilities();
    };


/** ---------------------------------------------------------------------------
    This class handle xCh raster file operations.

    @see HRFRasterFile
    @see HRFxChEditorRGBA
    ---------------------------------------------------------------------------
 */
class HRFxChFile : public HRFRasterFile
    {
public:
    //:> Class ID for this class
    HDECLARE_CLASS_ID(HRFFileId_xCh, HRFRasterFile)

    friend class HRFxChEditorRGBA;
    friend class HRFxChEditorPanchromatic;

    IMAGEPP_EXPORT                         HRFxChFile(const HFCPtr<HFCURL>& pi_rpURL,
                                                      HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                                      uint64_t              pi_Offset = 0);

    IMAGEPP_EXPORT                         HRFxChFile (const HFCPtr<HFCURL>& pi_rpRedFileURL,
                                                       const HFCPtr<HFCURL>& pi_rpGreenFileURL,
                                                       const HFCPtr<HFCURL>& pi_rpBlueFileURL,
                                                       const HFCPtr<HFCURL>& pi_rpAlphaFileURL = 0);

    IMAGEPP_EXPORT virtual                 ~HRFxChFile               ();

    //:> File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities        () const;

    //:> File information
    virtual const HGF2DWorldIdentificator
    GetWorldIdentificator  () const;

    //:> File manipulation
    virtual HRFResolutionEditor*
    CreateResolutionEditor (uint32_t      pi_Page,
                            unsigned short pi_Resolution,
                            HFCAccessMode pi_AccessMode);

    virtual void            Save();
protected:
    //:> Open main file (xml)
    HRFxChFile             (const HFCPtr<HFCURL>&  pi_rpURL,
                            HFCAccessMode          pi_AccessMode,
                            uint64_t              pi_Offset,
                            bool                  pi_DontOpenFile);

    //:> Open all channel raster files
    virtual bool           Open                   ();

    //:> Initialization
    virtual void            CreateDescriptors      ();

    virtual void            CreateChannelResolutionEditors    ();

    Byte*                   GetGrayscalePalette    (HFCPtr<HRPPixelType> pi_pPixelType);

    virtual void            ValidateChannelFilesRGB (const HFCPtr<HRFRasterFile>& pi_rpRedFile,
                                                     const HFCPtr<HRFRasterFile>& pi_rpGreenFile,
                                                     const HFCPtr<HRFRasterFile>& pi_rpBlueFile);

    virtual void            ValidateChannelFilesRGBA(const HFCPtr<HRFRasterFile>& pi_rpRedFile,
                                                    const HFCPtr<HRFRasterFile>& pi_rpGreenFile,
                                                    const HFCPtr<HRFRasterFile>& pi_rpBlueFile,
                                                    const HFCPtr<HRFRasterFile>& pi_rpAlphaFile);

    virtual void            ValidateChannelFilesPanchromatic(const HFCPtr<HRFRasterFile>& pi_rpPanchromaticFile,
                                                            const HFCPtr<HRFRasterFile>& pi_rpRedFile,
                                                            const HFCPtr<HRFRasterFile>& pi_rpGreenFile,
                                                            const HFCPtr<HRFRasterFile>& pi_rpBlueFile);

private:
    //:> Close all channel raster files
    void                    Close                  ();

    // Methods disabled
    HRFxChFile(const HRFxChFile& pi_rObj);
    HRFxChFile& operator=(const HRFxChFile& pi_rObj);

    uint32_t                 m_ChannelCount;
    HFCPtr<HRFRasterFile>   m_pRedFile;
    HFCPtr<HRFRasterFile>   m_pGreenFile;
    HFCPtr<HRFRasterFile>   m_pBlueFile;
    HFCPtr<HRFRasterFile>   m_pAlphaFile;
    HFCPtr<HRFRasterFile>   m_pPanchromaticFile;

    HArrayAutoPtr<Byte>    m_pRedMap;
    HArrayAutoPtr<Byte>    m_pGreenMap;
    HArrayAutoPtr<Byte>    m_pBlueMap;
    HArrayAutoPtr<Byte>    m_pAlphaMap;

    typedef vector<HRFResolutionEditor*, allocator<HRFResolutionEditor*> >
    ListOfResolutionEditor;

    ListOfResolutionEditor  m_RedFileResolutionEditor;
    ListOfResolutionEditor  m_GreenFileResolutionEditor;
    ListOfResolutionEditor  m_BlueFileResolutionEditor;
    ListOfResolutionEditor  m_AlphaFileResolutionEditor;
    ListOfResolutionEditor  m_PanchromaticFileResolutionEditor;
};


/** ---------------------------------------------------------------------------
    This struct handle xCh raster file creations.
    ---------------------------------------------------------------------------
 */
struct HRFxChCreator : public HRFRasterFileCreator
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
    //:> Capabilities of Raster file.
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities();

    //:> Allows to open an image file READONLY
    virtual HFCPtr<HRFRasterFile> Create(const HFCPtr<HFCURL>& pi_rpURL,
                                         HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                                         uint64_t             pi_Offset = 0) const;

private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFxChCreator)

    //:> Disabled methods
    HRFxChCreator();
    };
END_IMAGEPP_NAMESPACE

