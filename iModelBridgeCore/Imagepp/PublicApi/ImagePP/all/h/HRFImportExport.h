//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFImportExport.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFImportExport
//-----------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"
#include "HFCURL.h"
#include "HRFRasterFile.h"
#include "HRFRasterFileFactory.h"
#include "HRFExportOptions.h"
#include "HGF2DWorldCluster.h"

BEGIN_IMAGEPP_NAMESPACE
// This value is use in the file HUTImportExport and
// HUTImportFromFileExportToFile to synchronize the size of
// the StripAdapter.
#define HRFImportExport_ADAPT_HEIGHT    256

class HPMPool;
class HCDCodec;

/** -----------------------------------------------------------------
    HRFImportExport is the class used to do all export image++.
    ...

    @h3{Note:}
    Any notes or remark on my class used
    @end

    @h3{Inheritance notes:}
    Be careful if you overwrite the class, you have to use the get/set/select function
    to get or set the data member. Those data member are related one to the other, if
    you set it directly, you will lost interaction beteen data member.
    @end

    @see HRFExportOption
    @see HUTExportToFile
    @see HUTImportFromRasterExportToFile
    @see HUTImportFromFileExportToFile
    @end

    @h3{Word Related Documentation:}
    @list{<a href = "..\..\Image++\all\gra\hrf\HRF.doc"> HRF user guide. </a>}
    @end

    -----------------------------------------------------------------
*/
class HRFImportExport
    {
public:
    // Friend class
    friend class HRFExportOptions;

    // Creation and destruction interface
    IMAGEPP_EXPORT HRFImportExport (const HFCPtr<HGF2DWorldCluster>& pi_pWorldCluster);

    IMAGEPP_EXPORT virtual ~HRFImportExport();

    // Export options
    IMAGEPP_EXPORT virtual void                            SetExportOptions(const HFCPtr<HRFExportOptions>& pi_rpExportOptions);
    IMAGEPP_EXPORT virtual HRFExportOptions                GetSelectedExportOptions() const;

    // Export File Format interface
    IMAGEPP_EXPORT virtual uint32_t                        CountExportFileFormat() const;
    IMAGEPP_EXPORT virtual HRFRasterFileCreator*           GetExportFileFormat(uint32_t pi_index) const;

    IMAGEPP_EXPORT virtual void                            SelectExportFileFormat(const HRFRasterFileCreator* pi_pCreator);
    IMAGEPP_EXPORT virtual void                            SelectExportFileFormatByIndex(uint32_t pi_index);

    IMAGEPP_EXPORT virtual const HRFRasterFileCreator*     GetSelectedExportFileFormat() const;
    IMAGEPP_EXPORT virtual uint32_t                        GetSelectedExportFileFormatIndex() const;

    // Export File interface
    IMAGEPP_EXPORT virtual void                            SelectExportFilename(const HFCPtr<HFCURL>& pi_rpURLPath);
    IMAGEPP_EXPORT virtual const HFCPtr<HFCURL>&           GetSelectedExportFilename() const;

    // Pixel Type interface
    IMAGEPP_EXPORT virtual uint32_t                        CountPixelType() const;
    IMAGEPP_EXPORT virtual HCLASS_ID                     GetPixelType(uint32_t pi_index) const;

    IMAGEPP_EXPORT virtual void                            SelectPixelType(HCLASS_ID pi_PixelType);
    IMAGEPP_EXPORT virtual void                            SelectPixelTypeByIndex(uint32_t pi_Index);
    IMAGEPP_EXPORT virtual void                            SelectPixelType(const HFCPtr<HRPPixelType>& pi_rpPixelType);

    IMAGEPP_EXPORT virtual HCLASS_ID                     GetSelectedPixelType() const;
    IMAGEPP_EXPORT virtual uint32_t                        GetSelectedPixelTypeIndex() const;
    IMAGEPP_EXPORT virtual const HFCPtr<HRPPixelType>&     GetPixelType() const;

    // Sub resolution Pixel Type interface
    IMAGEPP_EXPORT virtual uint32_t                        CountSubResPixelType() const;
    IMAGEPP_EXPORT virtual HCLASS_ID                     GetSubResPixelType(uint32_t pi_index) const;

    IMAGEPP_EXPORT virtual void                            SelectSubResPixelType(HCLASS_ID pi_PixelType);
    IMAGEPP_EXPORT virtual void                            SelectSubResPixelTypeByIndex(uint32_t pi_Index);
    IMAGEPP_EXPORT virtual void                            SelectSubResPixelType(const HFCPtr<HRPPixelType>& pi_rpPixelType);

    IMAGEPP_EXPORT virtual HCLASS_ID                     GetSelectedSubResPixelType() const;
    IMAGEPP_EXPORT virtual uint32_t                        GetSelectedSubResPixelTypeIndex() const;
    IMAGEPP_EXPORT virtual const HFCPtr<HRPPixelType>&     GetSubResPixelType() const;

    // Down sampling method interface
    IMAGEPP_EXPORT virtual uint32_t                        CountDownSamplingMethod() const;
    IMAGEPP_EXPORT virtual HRFDownSamplingMethod           GetDownSamplingMethod(uint32_t pi_index) const;

    IMAGEPP_EXPORT virtual void                            SelectDownSamplingMethod(HRFDownSamplingMethod pi_DownSamplingMethod);
    IMAGEPP_EXPORT virtual void                            SelectDownSamplingMethodByIndex(uint32_t pi_Index);

    IMAGEPP_EXPORT virtual HRFDownSamplingMethod           GetSelectedDownSamplingMethod() const;
    IMAGEPP_EXPORT virtual uint32_t                        GetSelectedDownSamplingMethodIndex() const;

    // Sub Down sampling method interface
    IMAGEPP_EXPORT virtual uint32_t                        CountSubResDownSamplingMethod() const;
    IMAGEPP_EXPORT virtual HRFDownSamplingMethod           GetSubResDownSamplingMethod(uint32_t pi_index) const;

    IMAGEPP_EXPORT virtual void                            SelectSubResDownSamplingMethod(HRFDownSamplingMethod pi_DownSamplingMethod);
    IMAGEPP_EXPORT virtual void                            SelectSubResDownSamplingMethodByIndex(uint32_t pi_Index);

    IMAGEPP_EXPORT virtual HRFDownSamplingMethod           GetSelectedSubResDownSamplingMethod() const;
    IMAGEPP_EXPORT virtual uint32_t                        GetSelectedSubResDownSamplingMethodIndex() const;

    // Special override method, for DownSampling
    // These methods set a DownSamplingMethod, without any consideration of type file.
    // Normally used by special application only.
    IMAGEPP_EXPORT void                                    OverrideDownSamplingMethod(HRFDownSamplingMethod pi_DownSamplingMethod);
    IMAGEPP_EXPORT void                                    OverrideSubResDownSamplingMethod(HRFDownSamplingMethod pi_DownSamplingMethod);


    // Codec interface
    IMAGEPP_EXPORT virtual uint32_t                    CountCodecs() const;
    IMAGEPP_EXPORT virtual HCLASS_ID                    GetCodec(uint32_t pi_index) const;

    IMAGEPP_EXPORT virtual bool                         SelectCodec(HCLASS_ID pi_Codec);
    IMAGEPP_EXPORT virtual bool                         SelectCodecSample(const HFCPtr<HCDCodec>& pi_rpCodec);
    IMAGEPP_EXPORT virtual void                         SelectCodecByIndex(uint32_t pi_index);

    IMAGEPP_EXPORT virtual HCLASS_ID                    GetSelectedCodec() const;
    IMAGEPP_EXPORT virtual const HFCPtr<HCDCodec>&      GetSelectedCodecSample() const;
    IMAGEPP_EXPORT virtual uint32_t                    GetSelectedCodecIndex() const;

    IMAGEPP_EXPORT virtual uint32_t                    CountCompressionStep() const;
    IMAGEPP_EXPORT virtual void                         SelectCompressionQuality(uint32_t pi_Quality);
    IMAGEPP_EXPORT virtual uint32_t                    GetSelectedCompressionQuality() const;

    IMAGEPP_EXPORT virtual uint32_t                    CountCompressionRatioStep() const;
    IMAGEPP_EXPORT virtual void                         SelectCompressionRatio(uint32_t pi_Quality);
    IMAGEPP_EXPORT virtual uint32_t                    GetSelectedCompressionRatio() const;

    // Sub resolution Codec interface
    IMAGEPP_EXPORT virtual uint32_t                    CountSubResCodecs() const;
    IMAGEPP_EXPORT virtual HCLASS_ID                    GetSubResCodec(uint32_t pi_index) const;

    IMAGEPP_EXPORT virtual bool                         SelectSubResCodec(HCLASS_ID pi_Codec);
    IMAGEPP_EXPORT virtual bool                         SelectSubResCodecSample(const HFCPtr<HCDCodec>& pi_rpCodec);
    IMAGEPP_EXPORT virtual void                         SelectSubResCodecByIndex(uint32_t pi_index);

    IMAGEPP_EXPORT virtual HCLASS_ID                    GetSelectedSubResCodec() const;
    IMAGEPP_EXPORT virtual const HFCPtr<HCDCodec>&      GetSelectedSubResCodecSample() const;
    IMAGEPP_EXPORT virtual uint32_t                    GetSelectedSubResCodecIndex() const;

    IMAGEPP_EXPORT virtual uint32_t                    CountSubResCompressionStep() const;
    IMAGEPP_EXPORT virtual void                         SelectSubResCompressionQuality(uint32_t pi_Quality);
    IMAGEPP_EXPORT virtual uint32_t                    GetSelectedSubResCompressionQuality() const;

    IMAGEPP_EXPORT virtual uint32_t                    CountSubResCompressionRatioStep() const;
    IMAGEPP_EXPORT virtual void                         SelectSubResCompressionRatio(uint32_t pi_Quality);
    IMAGEPP_EXPORT virtual uint32_t                    GetSelectedSubResCompressionRatio() const;

    // Block Type  interface
    IMAGEPP_EXPORT virtual uint32_t                    CountBlockType() const;
    IMAGEPP_EXPORT virtual HRFBlockType                 GetBlockType(uint32_t pi_index) const;

    IMAGEPP_EXPORT virtual void                         SelectBlockType(HRFBlockType pi_BlockType);
    IMAGEPP_EXPORT virtual void                         SelectBlockTypeByIndex(uint32_t pi_Index);

    IMAGEPP_EXPORT virtual HRFBlockType                 GetSelectedBlockType() const;
    IMAGEPP_EXPORT virtual uint32_t                    GetSelectedBlockTypeIndex() const;

    IMAGEPP_EXPORT virtual void                         SetBlockWidth(uint32_t pi_Width);
    IMAGEPP_EXPORT virtual void                         SetBlockHeight(uint32_t pi_Height);

    IMAGEPP_EXPORT virtual uint32_t                    GetBlockWidth() const;
    IMAGEPP_EXPORT virtual uint32_t                    GetBlockHeight() const;

    IMAGEPP_EXPORT virtual uint32_t                    GetMinimumBlockWidth() const;
    IMAGEPP_EXPORT virtual uint32_t                    GetMinimumBlockHeight() const;

    IMAGEPP_EXPORT virtual uint32_t                    CountBlockWidthIncrementStep() const;
    IMAGEPP_EXPORT virtual uint32_t                    CountBlockHeightIncrementStep() const;

    IMAGEPP_EXPORT virtual uint32_t                    GetBlockWidthIncrementStep() const;
    IMAGEPP_EXPORT virtual uint32_t                    GetBlockHeightIncrementStep() const;

    IMAGEPP_EXPORT virtual void                         SelectBlockWidthIncrementStep(uint32_t pi_Index);
    IMAGEPP_EXPORT virtual void                         SelectBlockHeightIncrementStep(uint32_t pi_Index);

    IMAGEPP_EXPORT virtual uint32_t                    GetSelectedBlockWidthIncrementStep() const;
    IMAGEPP_EXPORT virtual uint32_t                    GetSelectedBlockHeightIncrementStep() const;

    // Sub resolution Block Interface
    IMAGEPP_EXPORT virtual uint32_t                    CountSubResBlockType() const;
    IMAGEPP_EXPORT virtual HRFBlockType                 GetSubResBlockType(uint32_t pi_index) const;

    IMAGEPP_EXPORT virtual void                         SelectSubResBlockType(HRFBlockType pi_BlockType);
    IMAGEPP_EXPORT virtual void                         SelectSubResBlockTypeByIndex(uint32_t pi_Index);

    IMAGEPP_EXPORT virtual HRFBlockType                 GetSelectedSubResBlockType() const;
    IMAGEPP_EXPORT virtual uint32_t                    GetSelectedSubResBlockTypeIndex() const;

    // Tag interface
    IMAGEPP_EXPORT virtual uint32_t                         CountTag() const;

    IMAGEPP_EXPORT virtual const HFCPtr<HPMGenericAttribute> GetTag(uint32_t pi_Index) const;
    IMAGEPP_EXPORT virtual void                              SetTag(const HFCPtr<HPMGenericAttribute>&  pi_rpTag);

    template <typename AttributeT> AttributeT const* FindTagCP() const;  
    template <typename AttributeT> AttributeT*       FindTagP(); 
    
    template <typename AttributeT> bool              HasTag() const; 
    IMAGEPP_EXPORT virtual bool                              HasTag(HPMGenericAttribute const& pi_Tag) const; 
    
    //MetaData interface
    const HMDMetaDataContainerList&         GetMetaDataContainerList() const;
    const HFCPtr<HMDMetaDataContainer>      GetMetaDataContainer(HMDMetaDataContainer::Type     pi_ContainerType) const;
    IMAGEPP_EXPORT void                                    SetMetaDataContainer(HFCPtr<HMDMetaDataContainer>&  pi_rpMDContainer);

    // Geocoding interface
    IMAGEPP_EXPORT RasterFileGeocodingCR        GetRasterFileGeocoding() const;
    IMAGEPP_EXPORT void                              SetRasterFileGeocoding(RasterFileGeocodingR pi_pGeocoding);
    IMAGEPP_EXPORT IRasterBaseGcsCP             GetGeocodingCP() const;
    IMAGEPP_EXPORT void                              SetGeocoding(IRasterBaseGcsP pi_pGeocoding);

    // Resample interface
    IMAGEPP_EXPORT virtual void                            SetResample(bool pi_Resample);
    IMAGEPP_EXPORT virtual bool                           GetResample() const;

    IMAGEPP_EXPORT virtual void                            SetResampleIsForce(bool pi_Resampling);
    IMAGEPP_EXPORT virtual bool                           GetResampleIsForce() const;

    IMAGEPP_EXPORT virtual void                            SetResamplingMethod(const HFCPtr<HRPFilter>& pi_rFilter);
    IMAGEPP_EXPORT virtual const HFCPtr<HRPFilter>&        GetResamplingMethod() const;


    // Image size interface
    IMAGEPP_EXPORT virtual void                            SetImageWidth(uint32_t pi_Width);
    IMAGEPP_EXPORT virtual void                            SetImageHeight(uint32_t pi_Height);

    IMAGEPP_EXPORT virtual uint32_t                        GetImageWidth() const;
    IMAGEPP_EXPORT virtual uint32_t                        GetImageHeight() const;

    // Scale  interface
    IMAGEPP_EXPORT virtual void                            SetScaleFactorX(double pi_ScaleFactorX);
    IMAGEPP_EXPORT virtual void                            SetScaleFactorY(double pi_ScaleFactorY);

    IMAGEPP_EXPORT virtual double                         GetScaleFactorX() const;
    IMAGEPP_EXPORT virtual double                         GetScaleFactorY() const;

    // Encoding type interface
    IMAGEPP_EXPORT virtual uint32_t                        CountEncoding() const;
    IMAGEPP_EXPORT virtual HRFEncodingType                 GetEncoding(uint32_t pi_Index) const;

    IMAGEPP_EXPORT virtual void                            SelectEncodingByIndex(uint32_t pi_Index);
    IMAGEPP_EXPORT virtual void                            SelectEncoding(HRFEncodingType pi_EncodingType);

    IMAGEPP_EXPORT virtual HRFEncodingType                 GetSelectedEncoding() const;
    IMAGEPP_EXPORT virtual uint32_t                        GetSelectedEncodingIndex() const;

    // Georeference format interface
    IMAGEPP_EXPORT virtual uint32_t                        CountGeoreferenceFormats() const;
    IMAGEPP_EXPORT virtual HRFGeoreferenceFormat           GetGeoreferenceFormat(uint32_t pi_index) const;

    IMAGEPP_EXPORT virtual void                            SelectGeoreferenceFormat(HRFGeoreferenceFormat pi_Format);
    IMAGEPP_EXPORT virtual void                            SelectGeoreferenceFormatByIndex(uint32_t pi_Index);

    IMAGEPP_EXPORT virtual HRFGeoreferenceFormat           GetSelectedGeoreferenceFormat() const;
    IMAGEPP_EXPORT virtual uint32_t                        GetSelectedGeoreferenceFormatIndex() const;

    // Export interface
    IMAGEPP_EXPORT virtual HFCPtr<HRFRasterFile>           StartExport() = 0;

    // BestMatch interface
    IMAGEPP_EXPORT virtual void                            BestMatchSelectedValues() = 0;

    IMAGEPP_EXPORT virtual WString                         ComposeFilenameWithOptions() const;
    IMAGEPP_EXPORT virtual uint32_t                        ExportToAllOptions(const HFCPtr<HFCURL>& pi_rpURLPath);

    // Default Values
    IMAGEPP_EXPORT virtual HGF2DPosition                   GetOriginalSize() const;
    IMAGEPP_EXPORT virtual HGF2DPosition                   GetDefaultResampleSize() const;
    IMAGEPP_EXPORT virtual double                         GetDefaultResampleScaleFactorX() const;
    IMAGEPP_EXPORT virtual double                         GetDefaultResampleScaleFactorY() const;

    // Status Information
    IMAGEPP_EXPORT virtual bool                           ImageSizeIsLock();
    IMAGEPP_EXPORT virtual bool                           ScaleFactorIsLock();
    IMAGEPP_EXPORT virtual bool                           MaintainAspectRatioIsCheck();

    IMAGEPP_EXPORT virtual void                            SetImageSizeIsLock(bool pi_Check);
    IMAGEPP_EXPORT virtual void                            SetScaleFactorIsLock(bool pi_Check);
    IMAGEPP_EXPORT virtual void                            SetMaintainAspectRatio(bool pi_Check);

    // Default color
    IMAGEPP_EXPORT virtual void const*                     GetRGBDefaultColor() const;
    IMAGEPP_EXPORT virtual void                            SetRGBDefaultColor(const void* pi_pValue);
    IMAGEPP_EXPORT virtual void                            SetRGBADefaultColor(const void* pi_pValue);

    IMAGEPP_EXPORT virtual bool                           UseDestinationPaletteIfIndexed() const;
    IMAGEPP_EXPORT virtual void                            SetUseDestinationPaletteIfIndexed(bool pi_UseDestinationPalette);

protected:
    // EncodingType list
    typedef vector<HRFBlockType>
    ListOfBlockType;

    // EncodingType list
    typedef vector<HRFEncodingType>
    ListOfEncodingType;

    // GeoreferenceFormat list
    typedef vector<HRFGeoreferenceFormat>
    ListOfGeoreferenceFormats;

    // Tag list
    typedef vector<HFCPtr<HPMGenericAttribute> >
    ListOfTag; 

    HRFExportOptions  m_ExportOptions;

    HFCPtr<HFCURL>                      m_pSelectedExportFilename;
    HRFRasterFileFactory::Creators      m_ListOfFileFormat;
    HFCPtr<HRFRasterFileCapabilities>   m_pSelectedFileFormatCapabilities;

    HFCPtr<HRFPixelTypeCapability>      m_pSelectedPixelTypeCapability;       // acceleration
    HFCPtr<HRFPixelTypeCapability>      m_pSelectedSubResPixelTypeCapability; // acceleration

    HFCPtr<HRFCodecCapability>          m_pSelectedCodecCapability;           // acceleration
    HFCPtr<HRFCodecCapability>          m_pSelectedSubResCodecCapability;     // acceleration

    HFCPtr<HRFTileCapability>           m_pTileCapability;                    // acceleration
    HFCPtr<HRFStripCapability>          m_pStripCapability;                   // acceleration
    HFCPtr<HRFLineCapability>           m_pLineCapability;                    // acceleration
    HFCPtr<HRFImageCapability>          m_pImageCapability;                   // acceleration
    ListOfBlockType                     m_ListOfValidBlockType;

    HFCPtr<HRFTileCapability>           m_pSubResTileCapability;              // acceleration
    HFCPtr<HRFStripCapability>          m_pSubResStripCapability;             // acceleration
    HFCPtr<HRFLineCapability>           m_pSubResLineCapability;              // acceleration
    HFCPtr<HRFImageCapability>          m_pSubResImageCapability;             // acceleration
    ListOfBlockType                     m_ListOfValidSubResBlockType;


    // In those member we keep the valid option for the current selection
    HFCPtr<HRFRasterFileCapabilities>   m_pListOfValidPixelType;              // acceleration
    HFCPtr<HRFRasterFileCapabilities>   m_pListOfValidCodec;                  // acceleration
    HFCPtr<HRFRasterFileCapabilities>   m_pListOfValidSubResCodec;            // acceleration
    ListOfEncodingType                  m_ListOfValidEncodingType;
    ListOfGeoreferenceFormats           m_ListOfValidGeoreferenceFormats;
    ListOfTag                           m_ListOfValidTag;

    // Block attributs used for validation.
    uint32_t                            m_BlockWidthIncrementStep;
    uint32_t                            m_BlockHeightIncrementStep;
    uint32_t                            m_CountBlockWidthIncrementStep;
    uint32_t                            m_CountBlockHeightIncrementStep;
    uint32_t                            m_MinBlockWidth;
    uint32_t                            m_MinBlockHeight;

    // Codec attributs used for validation.
    uint32_t                            m_CountCompressionStep;
    uint32_t                            m_SubResCountCompressionStep;

    uint32_t                            m_CountCompressionRatioStep;
    uint32_t                            m_SubResCountCompressionRatioStep;

    IMAGEPP_EXPORT virtual void                        PrepareExportFileFormatOptions();
    IMAGEPP_EXPORT virtual HFCPtr<HRFPageDescriptor>   CreatePageFromSelectedValues();
    IMAGEPP_EXPORT virtual HFCPtr<HRFRasterFile>       CreateFileFromSelectedValues();
    IMAGEPP_EXPORT virtual void                        UpdateBlockValues();
    IMAGEPP_EXPORT         void                        ClearTagList();

    IMAGEPP_EXPORT         void                        ValidateUncompressedExportSize(HFCPtr<HRFRasterFile>& pi_prDstRasterFile,
                                                                              bool*                 po_pIsCompressedImg = 0) const;

    HFCPtr<HRPFilter>                   m_pResamplingFilter;

    // Export option.
    bool m_ResampleIsForce;
    bool m_SourceHasTransfo;
    bool m_ImageSizeIsLock;
    bool m_ScaleFactorIsLock;
    bool m_MaintainAspectRatio;
    bool m_UseDestinationPaletteIfIndexed;



    // Default Values
    HGF2DPosition   m_OriginalSize;
    HGF2DPosition   m_DefaultResampleSize;
    double         m_DefaultResampleScaleFactorX;
    double         m_DefaultResampleScaleFactorY;

    // The export shape
    HVEShape        m_ClipShape;

    HFCPtr<HGF2DWorldCluster> m_pWorldCluster;

    // Default color
    Byte            m_aRGBDefaultColor[3];
    Byte            m_aRGBADefaultColor[4];
    bool           m_isRGBADefaultColorSet;

    mutable HFCPtr<HRPPixelType>
    m_pPixelType;
    mutable bool   m_BuildedPixelTypeSetted;
    mutable HFCPtr<HRPPixelType>
    m_pSubResPixelType;
    mutable bool   m_BuildedSubResPixelTypeSetted;

private:

    HRFDownSamplingMethod       m_OverrideDownSamplingMethod;
    HRFDownSamplingMethod       m_OverrideSubResDownSamplingMethod;


    // Disabled methods
    HRFImportExport(const HRFImportExport&);
    HRFImportExport& operator=(const HRFImportExport&);
    };
END_IMAGEPP_NAMESPACE

#include "HRFImportExport.hpp"
