//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFImportExport.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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
    _HDLLg HRFImportExport (const HFCPtr<HGF2DWorldCluster>& pi_pWorldCluster);

    _HDLLg virtual ~HRFImportExport();

    // Export options
    _HDLLg virtual void                            SetExportOptions(const HFCPtr<HRFExportOptions>& pi_rpExportOptions);
    _HDLLg virtual HRFExportOptions                GetSelectedExportOptions() const;

    // Export File Format interface
    _HDLLg virtual uint32_t                        CountExportFileFormat() const;
    _HDLLg virtual HRFRasterFileCreator*           GetExportFileFormat(uint32_t pi_index) const;

    _HDLLg virtual void                            SelectExportFileFormat(const HRFRasterFileCreator* pi_pCreator);
    _HDLLg virtual void                            SelectExportFileFormatByIndex(uint32_t pi_index);

    _HDLLg virtual const HRFRasterFileCreator*     GetSelectedExportFileFormat() const;
    _HDLLg virtual uint32_t                        GetSelectedExportFileFormatIndex() const;

    // Export File interface
    _HDLLg virtual void                            SelectExportFilename(const HFCPtr<HFCURL>& pi_rpURLPath);
    _HDLLg virtual const HFCPtr<HFCURL>&           GetSelectedExportFilename() const;

    // Pixel Type interface
    _HDLLg virtual uint32_t                        CountPixelType() const;
    _HDLLg virtual HCLASS_ID                     GetPixelType(uint32_t pi_index) const;

    _HDLLg virtual void                            SelectPixelType(HCLASS_ID pi_PixelType);
    _HDLLg virtual void                            SelectPixelTypeByIndex(uint32_t pi_Index);
    _HDLLg virtual void                            SelectPixelType(const HFCPtr<HRPPixelType>& pi_rpPixelType);

    _HDLLg virtual HCLASS_ID                     GetSelectedPixelType() const;
    _HDLLg virtual uint32_t                        GetSelectedPixelTypeIndex() const;
    _HDLLg virtual const HFCPtr<HRPPixelType>&     GetPixelType() const;

    // Sub resolution Pixel Type interface
    _HDLLg virtual uint32_t                        CountSubResPixelType() const;
    _HDLLg virtual HCLASS_ID                     GetSubResPixelType(uint32_t pi_index) const;

    _HDLLg virtual void                            SelectSubResPixelType(HCLASS_ID pi_PixelType);
    _HDLLg virtual void                            SelectSubResPixelTypeByIndex(uint32_t pi_Index);
    _HDLLg virtual void                            SelectSubResPixelType(const HFCPtr<HRPPixelType>& pi_rpPixelType);

    _HDLLg virtual HCLASS_ID                     GetSelectedSubResPixelType() const;
    _HDLLg virtual uint32_t                        GetSelectedSubResPixelTypeIndex() const;
    _HDLLg virtual const HFCPtr<HRPPixelType>&     GetSubResPixelType() const;

    // Down sampling method interface
    _HDLLg virtual uint32_t                        CountDownSamplingMethod() const;
    _HDLLg virtual HRFDownSamplingMethod           GetDownSamplingMethod(uint32_t pi_index) const;

    _HDLLg virtual void                            SelectDownSamplingMethod(HRFDownSamplingMethod pi_DownSamplingMethod);
    _HDLLg virtual void                            SelectDownSamplingMethodByIndex(uint32_t pi_Index);

    _HDLLg virtual HRFDownSamplingMethod           GetSelectedDownSamplingMethod() const;
    _HDLLg virtual uint32_t                        GetSelectedDownSamplingMethodIndex() const;

    // Sub Down sampling method interface
    _HDLLg virtual uint32_t                        CountSubResDownSamplingMethod() const;
    _HDLLg virtual HRFDownSamplingMethod           GetSubResDownSamplingMethod(uint32_t pi_index) const;

    _HDLLg virtual void                            SelectSubResDownSamplingMethod(HRFDownSamplingMethod pi_DownSamplingMethod);
    _HDLLg virtual void                            SelectSubResDownSamplingMethodByIndex(uint32_t pi_Index);

    _HDLLg virtual HRFDownSamplingMethod           GetSelectedSubResDownSamplingMethod() const;
    _HDLLg virtual uint32_t                        GetSelectedSubResDownSamplingMethodIndex() const;

    // Special override method, for DownSampling
    // These methods set a DownSamplingMethod, without any consideration of type file.
    // Normally used by special application only.
    _HDLLg void                                    OverrideDownSamplingMethod(HRFDownSamplingMethod pi_DownSamplingMethod);
    _HDLLg void                                    OverrideSubResDownSamplingMethod(HRFDownSamplingMethod pi_DownSamplingMethod);


    // Codec interface
    _HDLLg virtual uint32_t                    CountCodecs() const;
    _HDLLg virtual HCLASS_ID                    GetCodec(uint32_t pi_index) const;

    _HDLLg virtual bool                         SelectCodec(HCLASS_ID pi_Codec);
    _HDLLg virtual bool                         SelectCodecSample(const HFCPtr<HCDCodec>& pi_rpCodec);
    _HDLLg virtual void                         SelectCodecByIndex(uint32_t pi_index);

    _HDLLg virtual HCLASS_ID                    GetSelectedCodec() const;
    _HDLLg virtual const HFCPtr<HCDCodec>&      GetSelectedCodecSample() const;
    _HDLLg virtual uint32_t                    GetSelectedCodecIndex() const;

    _HDLLg virtual uint32_t                    CountCompressionStep() const;
    _HDLLg virtual void                         SelectCompressionQuality(uint32_t pi_Quality);
    _HDLLg virtual uint32_t                    GetSelectedCompressionQuality() const;

    _HDLLg virtual uint32_t                    CountCompressionRatioStep() const;
    _HDLLg virtual void                         SelectCompressionRatio(uint32_t pi_Quality);
    _HDLLg virtual uint32_t                    GetSelectedCompressionRatio() const;

    // Sub resolution Codec interface
    _HDLLg virtual uint32_t                    CountSubResCodecs() const;
    _HDLLg virtual HCLASS_ID                    GetSubResCodec(uint32_t pi_index) const;

    _HDLLg virtual bool                         SelectSubResCodec(HCLASS_ID pi_Codec);
    _HDLLg virtual bool                         SelectSubResCodecSample(const HFCPtr<HCDCodec>& pi_rpCodec);
    _HDLLg virtual void                         SelectSubResCodecByIndex(uint32_t pi_index);

    _HDLLg virtual HCLASS_ID                    GetSelectedSubResCodec() const;
    _HDLLg virtual const HFCPtr<HCDCodec>&      GetSelectedSubResCodecSample() const;
    _HDLLg virtual uint32_t                    GetSelectedSubResCodecIndex() const;

    _HDLLg virtual uint32_t                    CountSubResCompressionStep() const;
    _HDLLg virtual void                         SelectSubResCompressionQuality(uint32_t pi_Quality);
    _HDLLg virtual uint32_t                    GetSelectedSubResCompressionQuality() const;

    _HDLLg virtual uint32_t                    CountSubResCompressionRatioStep() const;
    _HDLLg virtual void                         SelectSubResCompressionRatio(uint32_t pi_Quality);
    _HDLLg virtual uint32_t                    GetSelectedSubResCompressionRatio() const;

    // Block Type  interface
    _HDLLg virtual uint32_t                    CountBlockType() const;
    _HDLLg virtual HRFBlockType                 GetBlockType(uint32_t pi_index) const;

    _HDLLg virtual void                         SelectBlockType(HRFBlockType pi_BlockType);
    _HDLLg virtual void                         SelectBlockTypeByIndex(uint32_t pi_Index);

    _HDLLg virtual HRFBlockType                 GetSelectedBlockType() const;
    _HDLLg virtual uint32_t                    GetSelectedBlockTypeIndex() const;

    _HDLLg virtual void                         SetBlockWidth(uint32_t pi_Width);
    _HDLLg virtual void                         SetBlockHeight(uint32_t pi_Height);

    _HDLLg virtual uint32_t                    GetBlockWidth() const;
    _HDLLg virtual uint32_t                    GetBlockHeight() const;

    _HDLLg virtual uint32_t                    GetMinimumBlockWidth() const;
    _HDLLg virtual uint32_t                    GetMinimumBlockHeight() const;

    _HDLLg virtual uint32_t                    CountBlockWidthIncrementStep() const;
    _HDLLg virtual uint32_t                    CountBlockHeightIncrementStep() const;

    _HDLLg virtual uint32_t                    GetBlockWidthIncrementStep() const;
    _HDLLg virtual uint32_t                    GetBlockHeightIncrementStep() const;

    _HDLLg virtual void                         SelectBlockWidthIncrementStep(uint32_t pi_Index);
    _HDLLg virtual void                         SelectBlockHeightIncrementStep(uint32_t pi_Index);

    _HDLLg virtual uint32_t                    GetSelectedBlockWidthIncrementStep() const;
    _HDLLg virtual uint32_t                    GetSelectedBlockHeightIncrementStep() const;

    // Sub resolution Block Interface
    _HDLLg virtual uint32_t                    CountSubResBlockType() const;
    _HDLLg virtual HRFBlockType                 GetSubResBlockType(uint32_t pi_index) const;

    _HDLLg virtual void                         SelectSubResBlockType(HRFBlockType pi_BlockType);
    _HDLLg virtual void                         SelectSubResBlockTypeByIndex(uint32_t pi_Index);

    _HDLLg virtual HRFBlockType                 GetSelectedSubResBlockType() const;
    _HDLLg virtual uint32_t                    GetSelectedSubResBlockTypeIndex() const;

    // Tag interface
    _HDLLg virtual uint32_t                         CountTag() const;

    _HDLLg virtual const HFCPtr<HPMGenericAttribute> GetTag(uint32_t pi_Index) const;
    _HDLLg virtual void                              SetTag(const HFCPtr<HPMGenericAttribute>&  pi_rpTag);

    template <typename AttributeT> AttributeT const* FindTagCP() const;  
    template <typename AttributeT> AttributeT*       FindTagP(); 
    
    template <typename AttributeT> bool              HasTag() const; 
    _HDLLg virtual bool                              HasTag(HPMGenericAttribute const& pi_Tag) const; 
    
    //MetaData interface
    const HMDMetaDataContainerList&         GetMetaDataContainerList() const;
    const HFCPtr<HMDMetaDataContainer>      GetMetaDataContainer(HMDMetaDataContainer::Type     pi_ContainerType) const;
    _HDLLg void                                    SetMetaDataContainer(HFCPtr<HMDMetaDataContainer>&  pi_rpMDContainer);

    // Geocoding interface
    _HDLLg IRasterBaseGcsPtr                GetGeocoding() const;
    _HDLLg void                             SetGeocoding(IRasterBaseGcsPtr pi_pGeocoding);

    // Resample interface
    _HDLLg virtual void                            SetResample(bool pi_Resample);
    _HDLLg virtual bool                           GetResample() const;

    _HDLLg virtual void                            SetResampleIsForce(bool pi_Resampling);
    _HDLLg virtual bool                           GetResampleIsForce() const;

    _HDLLg virtual void                            SetResamplingMethod(const HFCPtr<HRPFilter>& pi_rFilter);
    _HDLLg virtual const HFCPtr<HRPFilter>&        GetResamplingMethod() const;


    // Image size interface
    _HDLLg virtual void                            SetImageWidth(uint32_t pi_Width);
    _HDLLg virtual void                            SetImageHeight(uint32_t pi_Height);

    _HDLLg virtual uint32_t                        GetImageWidth() const;
    _HDLLg virtual uint32_t                        GetImageHeight() const;

    // Scale  interface
    _HDLLg virtual void                            SetScaleFactorX(double pi_ScaleFactorX);
    _HDLLg virtual void                            SetScaleFactorY(double pi_ScaleFactorY);

    _HDLLg virtual double                         GetScaleFactorX() const;
    _HDLLg virtual double                         GetScaleFactorY() const;

    // Encoding type interface
    _HDLLg virtual uint32_t                        CountEncoding() const;
    _HDLLg virtual HRFEncodingType                 GetEncoding(uint32_t pi_Index) const;

    _HDLLg virtual void                            SelectEncodingByIndex(uint32_t pi_Index);
    _HDLLg virtual void                            SelectEncoding(HRFEncodingType pi_EncodingType);

    _HDLLg virtual HRFEncodingType                 GetSelectedEncoding() const;
    _HDLLg virtual uint32_t                        GetSelectedEncodingIndex() const;

    // Georeference format interface
    _HDLLg virtual uint32_t                        CountGeoreferenceFormats() const;
    _HDLLg virtual HRFGeoreferenceFormat           GetGeoreferenceFormat(uint32_t pi_index) const;

    _HDLLg virtual void                            SelectGeoreferenceFormat(HRFGeoreferenceFormat pi_Format);
    _HDLLg virtual void                            SelectGeoreferenceFormatByIndex(uint32_t pi_Index);

    _HDLLg virtual HRFGeoreferenceFormat           GetSelectedGeoreferenceFormat() const;
    _HDLLg virtual uint32_t                        GetSelectedGeoreferenceFormatIndex() const;

    // Export interface
    _HDLLg virtual HFCPtr<HRFRasterFile>           StartExport() = 0;

    // BestMatch interface
    _HDLLg virtual void                            BestMatchSelectedValues() = 0;

    _HDLLg virtual WString                         ComposeFilenameWithOptions() const;
    _HDLLg virtual uint32_t                        ExportToAllOptions(const HFCPtr<HFCURL>& pi_rpURLPath);

    // Default Values
    _HDLLg virtual HGF2DPosition                   GetOriginalSize() const;
    _HDLLg virtual HGF2DPosition                   GetDefaultResampleSize() const;
    _HDLLg virtual double                         GetDefaultResampleScaleFactorX() const;
    _HDLLg virtual double                         GetDefaultResampleScaleFactorY() const;

    // Status Information
    _HDLLg virtual bool                           ImageSizeIsLock();
    _HDLLg virtual bool                           ScaleFactorIsLock();
    _HDLLg virtual bool                           MaintainAspectRatioIsCheck();

    _HDLLg virtual void                            SetImageSizeIsLock(bool pi_Check);
    _HDLLg virtual void                            SetScaleFactorIsLock(bool pi_Check);
    _HDLLg virtual void                            SetMaintainAspectRatio(bool pi_Check);

    // Default color
    _HDLLg virtual void*                           GetRGBDefaultColor() const;
    _HDLLg virtual void                            SetRGBDefaultColor(const void* pi_pValue);
    _HDLLg virtual void                            SetRGBADefaultColor(const void* pi_pValue);

    _HDLLg virtual bool                           UseDestinationPaletteIfIndexed() const;
    _HDLLg virtual void                            SetUseDestinationPaletteIfIndexed(bool pi_UseDestinationPalette);

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

    _HDLLg virtual void                        PrepareExportFileFormatOptions();
    _HDLLg virtual HFCPtr<HRFPageDescriptor>   CreatePageFromSelectedValues();
    _HDLLg virtual HFCPtr<HRFRasterFile>       CreateFileFromSelectedValues();
    _HDLLg virtual void                        UpdateBlockValues();
    _HDLLg         void                        ClearTagList();

    _HDLLg         void                        ValidateUncompressedExportSize(HFCPtr<HRFRasterFile>& pi_prDstRasterFile,
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

#include "HRFImportExport.hpp"
