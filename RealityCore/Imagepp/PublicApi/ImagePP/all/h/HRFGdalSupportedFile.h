//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFGdalSupportedRasterFile.h
//-----------------------------------------------------------------------------
// This class describes a raster file which is accessed by using GDAL.
#pragma once

#ifdef IPP_HAVE_GDAL_SUPPORT

#include "HFCMacros.h"
#include "HFCAccessMode.h"
#include "HFCURL.h"
#include "HFCBinStream.h"
#include "HRFRasterFile.h"
#include "HRFMacros.h"
#include "HTIFFTag.h"

// From GdalLib
class GDALDataset;
class GDALDriver;
class GDALRasterBand;

BEGIN_IMAGEPP_NAMESPACE
enum DISPLAY_REP
    {
    UNDEFINED = 0,
    MONO,
    RGB,
    YCC,
    PALETTE
    };


#define CHECK_ERR(pi_FcnCall) \
    CPLErrorReset(); \
    pi_FcnCall \
    if ((CPLGetLastErrorNo() != CPLE_None) && \
        ((CPLGetLastErrorType() == CE_Failure) || \
         (CPLGetLastErrorType() == CE_Fatal))) \
    { \
        ThrowExBasedOnGDALErrorCode(CPLGetLastErrorNo()); \
    }

/*
void GDALErrorHandler(UShort     pi_ErrClass,
                      int32_t         pi_ErrNo,
                      const char* pi_Msg);
*/

class HRFGdalSupportedFileEditor;

class HRFGdalSupportedFile : public HRFRasterFile
    {
public:

    friend class HRFGdalSupportedFileEditor;
    /*
        friend void  GDALErrorHandler(UShort     pi_ErrClass,
                                      int32_t         pi_ErrNo,
                                      const char* pi_Msg);
      */
    // Class ID for this class.
    HDECLARE_CLASS_ID(HRFFileId_GdalSupported, HRFRasterFile)

    virtual HRFResolutionEditor*          CreateResolutionEditor(uint32_t                  pi_Page,
                                                                 uint16_t           pi_Resolution,
                                                                 HFCAccessMode             pi_AccessMode);

    virtual void                          Save();

    virtual bool                         AddPage(HFCPtr<HRFPageDescriptor> pi_pPage);

    const HGF2DWorldIdentificator         GetWorldIdentificator () const;

    static void                         Initialize();

protected:

    // allow to Open an image file
    HRFGdalSupportedFile(const char* pi_pDriverName,
                         const HFCPtr<HFCURL>& pi_rpURL,
                         HFCAccessMode         pi_AccessMode = HFC_READ_ONLY,
                         uint64_t              pi_Offset = 0);

    virtual              ~HRFGdalSupportedFile();


    // Methods

    // Constructor use only to create a child
    //
    virtual void            CreateDescriptors() = 0;

    virtual void            HandleNoDisplayBands() = 0;

    virtual bool            Open();

    void                    CreateDescriptorsWith(const HFCPtr<HCDCodec>& pi_rpCodec,
                                                  HPMAttributeSet&       pi_rTagList,
                                                  HRPHistogram*          pi_pHistogram = 0);

    virtual void            SetCreationOptions(HFCPtr<HRFPageDescriptor>& pi_rpPage,
                                               char** &                    pio_rppCreationOptions) const;

    void                    ThrowExBasedOnGDALErrorCode(uint32_t pi_GDALErrorCode);

    bool                   GetGeoRefMatrix();
    bool                   SetGeoRefMatrix();

    GDALRasterBand*         GetRasterBand(int32_t i)const;
    GDALDataset*            GetDataSet() const;
    int32_t                 GetNbImgBands() const;
    signed char             GetBandInd(Byte pi_ColorType) const;

    //access
    uint32_t                GetBitsPerPixelPerBand()const;
    bool                   IsUnsignedPixelTypeForSignedData() const;
    bool                   IsIntegerPixelTypeForRealData() const;
    bool                   IsReadPixelReal() const;
    bool                   IsReadPixelSigned() const;
    bool                   IsGrayScale()const;
    bool                   HasPalette()const;
    int32_t                GetNbBands()const;
    int32_t                GetNbDisplayableBands()const;
    int32_t                GetImageWidth()const;
    int32_t                GetImageHeight()const;
    int32_t                GetBlockWidth()const;
    int32_t                GetBlockHeight()const;
    uint32_t               GetTotalRowBytes() const;
    void                   SetHasPalette(bool pi_hasPalette);
    void                   SetColorAttributes();
    void                   SetNoDataValue(HFCPtr<HRPPixelType>& pixelType);

    bool                               IsValidGeoRefInfo() const;
    virtual HFCPtr<HGF2DTransfoModel>   BuildTransfoModel();
    virtual HRFScanlineOrientation      GetScanLineOrientation()const;

    virtual GeoCoordinates::BaseGCSPtr   ExtractGeocodingInformation(double* po_pVerticalUnitRatioToMeter);
    bool                                 SetGeocodingInformation();
    //void                                 AddVerticalUnitToGeocoding(GeoCoordinates::BaseGCSR pio_pBaseGCS) const;

    typedef list<int32_t> BandIndList;

    void                                AddMinMaxSampleValueTags(HPMAttributeSet& pi_rTagList);

    void                                AddSampleValueLimitTag(HPMAttributeSet& pi_rTagList,
                                                               BandIndList&     pi_rBandIndList,
                                                               bool            pi_AddMinimumSampleValue);

    virtual HRPChannelType::ChannelRole GetBandRole(int32_t pi_RasterBand) const;

    uint32_t                 GetCorrespondingGDALDataType(size_t pi_NbBits,
                                                         size_t pi_NbChannel,
                                                         bool  pi_HasPalette,
                                                         bool  pi_IsSigned,
                                                         bool  pi_IsFloat);

    DISPLAY_REP             GetDispRep() const;

    //TR 244928, CR 247184
    //- Temporary patch
    virtual void  DetectPixelType();

    virtual void DetectOptimalBlockAccess();

    virtual bool AreDataNeedToBeScaled();

    static double GetMinimumPossibleValue(uint16_t pi_DataType);
    static double GetMaximumPossibleValue(uint16_t pi_DataType);


    typedef struct GeoRefInfo
        {
        double  m_A00;
        double  m_A01;
        double  m_A10;
        double  m_A11;
        double  m_Tx;
        double  m_Ty;
        } GeoRefInfo;

    bool                                m_IsGeoReference;
    GeoRefInfo                          m_GeoRefInfo;
    uint32_t                           m_GTModelType;
    uint16_t                      m_GTRasterType;

    HFCPtr<HRPPixelType>                m_pPixelType;
    int32_t                                 m_NbBands;
    int32_t                                 m_BlockWidth;
    int32_t                                 m_BlockHeight;
    Byte                                m_ShiftSize;
    signed char                         m_GrayBandInd;
    signed char                         m_RedBandInd;
    signed char                         m_GreenBandInd;
    signed char                         m_BlueBandInd;
    signed char                         m_AlphaBandInd;
    signed char                         m_PaletteBandInd;
    signed char                         m_ExtendedBandInd;
    signed char                         m_YbandInd;
    signed char                         m_CbBandInd;
    signed char                         m_CrBandInd;
    GDALDataset*                        m_poDataset;

    //TR 244928, CR 247184
    //- Temporary patch - These variables should be private
    bool                                m_Signed;
    uint16_t                      m_BitsPerPixelPerBand;
    DISPLAY_REP                         m_DisplayRep;
    bool                                m_doubleToFloatCnv;

private:

    // Attributs
    HRFScanlineOrientation              m_SLO;

    GDALDriver*                         m_pGdalDriver;

    bool                                m_IsUPixelTypeForSignedData;
    bool                                m_IsIntegerPixelTypeForRealData;
    bool                                m_IsReadPixelReal;
    bool                                m_IsGrayScale;
    bool                                m_HasPalette;

    void DetectPixelTypeMono();
    void DetectPixelTypeRgb();
    void DetectPixelTypePalette();
    void DetectPixelTypeYCbCr();

    void WriteColorTable(uint32_t              pi_Page = 0,
                         uint16_t       pi_Resolution = 0);

    void AddSupportedGeoTag(HFCPtr<HRFRasterFileCapabilities>& pi_rpSupportedGeoTags,
                            HFCPtr<HPMGenericAttribute>&       pi_rpGdalObtainedTag,
                            HPMAttributeSet&                   pio_rTagList);

    // Methods Disabled
    HRFGdalSupportedFile(const HRFGdalSupportedFile& pi_rObj);
    HRFGdalSupportedFile& operator=(const HRFGdalSupportedFile& pi_rObj);
    };
END_IMAGEPP_NAMESPACE

#endif
