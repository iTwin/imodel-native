//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFErdasImgFile.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFErdasImgFile
//-----------------------------------------------------------------------------
// This class describes a File Raster image.
#pragma once

#include "HFCMacros.h"
#include "HFCAccessMode.h"
#include "HFCURL.h"
#include "HFCBinStream.h"
#include "HRFGdalSupportedFile.h"
#include "HRFRasterFileCapabilities.h"
#include "HRFErdasImgEditor.h"
#include "HRFMacros.h"
#include "HTIFFTag.h"

#define HRF_IMG_MAX_NB_CHANNEL_SPECIFIED 4

//--------------------------------------------------
// class HRFDoqCapabilities
//--------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
class HRFErdasImgCapabilities : public HRFRasterFileCapabilities
    {
public:
    HRFErdasImgCapabilities();
    };


struct HRFErdasImgCreator : public HRFRasterFileCreator
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



    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HRFErdasImgCreator)
private:



    // Disabled methods
    HRFErdasImgCreator();
    };



class HRFErdasImgFile : public HRFGdalSupportedFile
    {
public:
    friend HRFErdasImgCreator;
    friend HRFErdasImgEditor;

    typedef map<string, unsigned short> UnitNameToEPSGCodeMap;

    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_ErdasImg, HRFGdalSupportedFile)

    // allow to Open an image file
    HRFErdasImgFile       (const HFCPtr<HFCURL>&          pi_rpURL,
                           HFCAccessMode                  pi_AccessMode = HFC_READ_ONLY,
                           uint64_t                      pi_Offset = 0);

    virtual                               ~HRFErdasImgFile           ();

    // File capabilities
    virtual const HFCPtr<HRFRasterFileCapabilities>&
    GetCapabilities       () const;

    // File manipulation
    virtual HRFResolutionEditor*          CreateResolutionEditor(uint32_t                  pi_Page,
                                                                 unsigned short           pi_Resolution,
                                                                 HFCAccessMode             pi_AccessMode);

protected:

    virtual void                          DetectPixelType();

    virtual bool                          AreDataNeedToBeScaled();

    // Protected Methods
    virtual void                          CreateDescriptors() override;
    virtual HFCPtr<HGF2DTransfoModel>     BuildTransfoModel() override;
    virtual RasterFileGeocodingPtr        ExtractGeocodingInformation() override;

    virtual void                          HandleNoDisplayBands () override;

    virtual void                          SetCreationOptions(HFCPtr<HRFPageDescriptor>& pi_rpPageDesc,
                                                             char** &                    pio_rppCreationOptions) const override;

private:

    bool Create();

    void CreateUnitNameToEPSGCodeMap();

    void GetHistogramFromImgHeader(HFCPtr<HRPHistogram>& po_rHistogram);

    //TR 244928, CR 247184
    //- Temporary patch to allow the user to specify the mapping between
    //  bands found in an image file and the color channels of the pixel type used
    //  to display the image.
    bool                               m_IsBandSpecValid;

    static HAutoPtr<UnitNameToEPSGCodeMap> m_pUnitToNameToEPSGCodeMap;

    // Methods Disabled
    HRFErdasImgFile(const HRFErdasImgFile& pi_rObj);
    HRFErdasImgFile& operator=(const HRFErdasImgFile& pi_rObj);
    };
END_IMAGEPP_NAMESPACE
