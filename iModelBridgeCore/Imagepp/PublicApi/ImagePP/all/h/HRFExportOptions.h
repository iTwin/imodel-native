//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFExportOptions.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFExportOptions
//-----------------------------------------------------------------------------
// This class is used by application to hold export options.
//-----------------------------------------------------------------------------

#pragma once

#include "HFCPtr.h"
#include "HFCURL.h"

#include "HMDMetaDataContainerList.h"

#include "HRFRasterFile.h"


/** ---------------------------------------------------------------------------
    This class contain all infomation about a specific export. HRFExportOption
    work closely with HRFImportExport besause the data set into this
    object is validate in HRFImportExport. HRFExportOption should not be used in
    a other contexte. This is a container for export option. It can be used to
    reproduce a specific export or a set of export.

    @h3{Note:}
    All the set methode assume that all the data set in this objet is
    valid. @b{So the caller must validate all paramater before setting them.}
    @end

    @see HRFImportExport
    @end

   ----------------------------------------------------------------------------
*/
BEGIN_IMAGEPP_NAMESPACE
class HRFExportOptions : public HFCShareableObject<HRFExportOptions>
    {
public:

    // Friend class
    friend class HRFImportExport;

    // Constructor
    HRFExportOptions(const HRFRasterFileCreator*    pi_pFileFormat,
                     HCLASS_ID                      pi_PixelType,
                     HCLASS_ID                      pi_SubResPixelType,
                     HRFDownSamplingMethod          pi_DownSamplingMethod,
                     HRFDownSamplingMethod          pi_SubResDownSamplingMethod,
                     HPMAttributeSet const&         pi_TagList,
                     HCLASS_ID                      pi_Codec,
                     uint32_t                      pi_CompressionQuality,
                     HCLASS_ID                      pi_SubResCodec,
                     uint32_t                      pi_SubResCompressionQuality,
                     bool                           pi_Resample,
                     uint32_t                      pi_ImageWidth,
                     uint32_t                      pi_ImageHeight,
                     double                         pi_ScaleFactorX,
                     double                         pi_ScaleFactorY,
                     HRFBlockType                   pi_BlockType,
                     uint32_t                      pi_BlockWidth,
                     uint32_t                      pi_BlockHeight,
                     HRFBlockType                   pi_SubResBlockType,
                     uint32_t                      pi_SubResBlockWidth,
                     uint32_t                      pi_SubResBlockHeight,
                     HRFEncodingType                pi_Encoding,
                     HRFGeoreferenceFormat          pi_GeoreferenceFormat,
                     uint32_t                      pi_CompressionRatio,
                     uint32_t                      pi_SubResCompressionRatio,
                     GeoCoordinates::BaseGCSP      pi_Geocoding);

    HRFExportOptions(const HRFRasterFileCreator* pi_pFileFormat,
                     HCLASS_ID                      pi_PixelType,
                     HCLASS_ID                      pi_SubResPixelType,
                     HRFDownSamplingMethod          pi_DownSamplingMethod,
                     HRFDownSamplingMethod          pi_SubResDownSamplingMethod,
                     HPMAttributeSet const&         pi_TagList,
                     HFCPtr<HCDCodec>&              pi_rpCodec,
                     uint32_t                       pi_CompressionQuality,
                     HFCPtr<HCDCodec>&              pi_rpSubResCodec,
                     uint32_t                       pi_SubResCompressionQuality,
                     bool                           pi_Resample,
                     uint32_t                       pi_ImageWidth,
                     uint32_t                       pi_ImageHeight,
                     double                         pi_ScaleFactorX,
                     double                         pi_ScaleFactorY,
                     HRFBlockType                   pi_BlockType,
                     uint32_t                       pi_BlockWidth,
                     uint32_t                       pi_BlockHeight,
                     HRFBlockType                   pi_SubResBlockType,
                     uint32_t                       pi_SubResBlockWidth,
                     uint32_t                       pi_SubResBlockHeight,
                     HRFEncodingType                pi_Encoding,
                     HRFGeoreferenceFormat          pi_GeoreferenceFormat,
                     uint32_t                       pi_CompressionRatio,
                     uint32_t                       pi_SubResCompressionRatio,
                     GeoCoordinates::BaseGCSP       pi_Geocoding);

    // Basic operation.
    HRFExportOptions (const HRFExportOptions& pi_rExportOptions);
    HRFExportOptions& operator=(const HRFExportOptions& pi_rExportOptions);
    bool operator==(HRFExportOptions& pi_rExportOptions) const;

    // Destructor
    virtual ~HRFExportOptions();

    // Get
    HRFRasterFileCreator*           GetFileFormat() const;

    HCLASS_ID                       GetPixelType() const;
    HCLASS_ID                       GetSubResPixelType() const;

    HRFDownSamplingMethod           GetDownSamplingMethod() const;
    HRFDownSamplingMethod           GetSubResDownSamplingMethod() const;

    HCLASS_ID                       GetCodec() const;
    const HFCPtr<HCDCodec>&         GetCodecSample() const;
    uint32_t                       GetCompressionQuality() const;
    uint32_t                       GetCompressionRatio() const;


    HCLASS_ID                       GetSubResCodec() const;
    const HFCPtr<HCDCodec>&         GetSubResCodecSample() const;
    uint32_t                       GetSubResCompressionQuality() const;
    uint32_t                       GetSubResCompressionRatio() const;

    HRFBlockType                    GetBlockType() const;
    uint32_t                       GetBlockWidth() const;
    uint32_t                       GetBlockHeight() const;

    HRFBlockType                    GetSubResBlockType() const;
    uint32_t                       GetSubResBlockWidth() const;
    uint32_t                       GetSubResBlockHeight() const;

    bool                            GetResample() const;
    uint32_t                       GetImageWidth() const;
    uint32_t                       GetImageHeight() const;
    double                          GetScaleFactorX() const;
    double                          GetScaleFactorY() const;

    HRFEncodingType                 GetEncoding() const;
    HRFGeoreferenceFormat           GetGeoreferenceFormat() const;
    RasterFileGeocodingCR           GetRasterFileGeocoding() const;

    const HPMAttributeSet&          GetTagList() const;

    const HMDMetaDataContainerList& GetMetaDataContainerList() const;

private:

    // Constructors
    HRFExportOptions();

    // Method used by the copy constructor
    void DeepCopy(const HRFExportOptions& pi_rExportOptions);

    // Set function accesible only from friend class.
    void SetFileFormat (const HRFRasterFileCreator* pi_pCreator);

    void SetPixelType (HCLASS_ID pi_PixelType);
    void SetSubResPixelType (HCLASS_ID pi_SubPixelType);

    void SetDownSamplingMethod (HRFDownSamplingMethod pi_DownSamplingMethod);
    void SetSubResDownSamplingMethod (HRFDownSamplingMethod pi_SubDownSamplingMethod);

    void SetCodecSample(const HFCPtr<HCDCodec>& pi_rpCodec);
    void SetCompressionQuality (uint32_t pi_CompressionQuality);
    void SetCompressionRatio (uint32_t pi_CompressionRatio);

    void SetSubResCodecSample (const HFCPtr<HCDCodec>& pi_rpCodec);
    void SetSubResCompressionQuality (uint32_t pi_SubCompressionQuality);
    void SetSubResCompressionRatio (uint32_t pi_SubCompressionRatio);

    void SetBlockType (HRFBlockType pi_BlockType);
    void SetBlockWidth (uint32_t pi_BlockWidth);
    void SetBlockHeight (uint32_t pi_BlockHeight);

    void SetSubResBlockType (HRFBlockType pi_BlockType);
    void SetSubResBlockWidth (uint32_t pi_BlockWidth);
    void SetSubResBlockHeight (uint32_t pi_BlockHeight);

    void SetEncoding (HRFEncodingType pi_EncodingType);
    void SetGeoreferenceFormat (HRFGeoreferenceFormat pi_GeoreferenceFormat);
    void SetRasterFileGeocoding(RasterFileGeocodingR pi_pGeocoding);

    void SetResample (bool pi_Resample);
    void SetImageWidth (uint32_t pi_ImageWidth);
    void SetImageHeight (uint32_t pi_ImageHeight);
    void SetScaleFactorX (double pi_ImageWidth);
    void SetScaleFactorY (double pi_ImageHeight);

    void SetTagList (HPMAttributeSet pi_ListOfTag);
    void SetTag (const HFCPtr<HPMGenericAttribute>& pi_rpTag);

    void SetMetaDataContainer (const HFCPtr<HMDMetaDataContainer>& pi_rpMDContainer);

    // Data
    HRFRasterFileCreator*       m_pFileFormat;

    HCLASS_ID                   m_PixelType;
    HCLASS_ID                   m_SubResPixelType;

    HRFDownSamplingMethod       m_DownSamplingMethod;
    HRFDownSamplingMethod       m_SubResDownSamplingMethod;

    // Keep table and codec configuration
    HFCPtr<HCDCodec>            m_pCodec;
    uint32_t                   m_CompressionQuality;
    uint32_t                   m_CompressionRatio;

    // Keep table and codec configuration
    HFCPtr<HCDCodec>            m_pSubResCodec;
    uint32_t                   m_SubResCompressionQuality;
    uint32_t                   m_SubResCompressionRatio;

    HRFBlockType                m_BlockType;
    uint32_t                   m_BlockWidth;
    uint32_t                   m_BlockHeight;

    HRFBlockType                m_SubResBlockType;
    uint32_t                   m_SubResBlockWidth;
    uint32_t                   m_SubResBlockHeight;

    bool                        m_Resample;
    uint32_t                   m_ImageWidth;
    uint32_t                   m_ImageHeight;
    double                      m_ScaleFactorX;
    double                      m_ScaleFactorY;

    HRFEncodingType             m_Encoding;
    HRFGeoreferenceFormat       m_GeoreferenceFormat;
    RasterFileGeocodingPtr      m_pGeocoding;

    HPMAttributeSet             m_TagList;
    HMDMetaDataContainerList    m_ListOfMetaDataContainer;
    };
END_IMAGEPP_NAMESPACE

