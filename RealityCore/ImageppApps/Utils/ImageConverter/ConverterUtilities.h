/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageConverter/ConverterUtilities.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// $Header: /imagepp-root/Apps/Utils/ImageConverter/ConverterUtilities.h,v 1.6 2011/07/18 21:12:34 Donald.Morissette Exp $
//-----------------------------------------------------------------------------
// Class ConverterUtilities
//-----------------------------------------------------------------------------

#ifndef __ConverterUtilities__H__
#define __ConverterUtilities__H__

#include <Imagepp/all/h/HFCBuffer.h>
#include <Imagepp/all/h/HFCURLFile.h>
#include <Imagepp/all/h/HUTImportFromFileExportToFile.h>
#include <ImagePP/all/h/HCDCodec.h>
#include <ImagePP/all/h/HCDCodecIJG.h>

//-----------------------------------------------------------------------------
// ExportProgressIndicator
//-----------------------------------------------------------------------------
#include <Imagepp/all/h/HFCProgressIndicator.h>

#define PSS_MIN_DIMENSION_FOR_CACHE        8192
#define PSS_MIN_CACHE_DIMENSION_IN_PERCENT 10

class ExportProgressListener : public HFCProgressDurationListener
{
  public:
	ExportProgressListener();
    virtual void Progression(HFCProgressIndicator* pi_pProgressIndicator, 
                             uint32_t                pi_Processed,		 
                             uint32_t                pi_CountProgression);  

  protected:
    uint32_t m_EyePos;
    bool  m_LeftDir;
    Utf8String m_TimeToBeProcessed;

};

//-----------------------------------------------------------------------------
//  Function prototype declaration
//-----------------------------------------------------------------------------

class ExportPropertiesContainer;

#ifdef THUMBNAIL_GENERATOR
bool ConvertPixelType(HFCPtr<HRFThumbnail> pi_pThumbnail,
                       HPMClassKey pi_DstPixelTypeClasskey,
                       HFCBuffer& po_rBuffer);

bool ConvertToJPEG(const Byte*   pi_pData,
                    size_t         pi_DataSize,
                    HFCBuffer&     po_rBuffer,
                    size_t         pi_Width,
                    size_t         pi_Height,
                    Byte         pi_Quality,
                    Byte         pi_Bits);

void CheckJpegDefault(ExportPropertiesContainer* po_pExportProperties);

void CheckJpegError(ExportPropertiesContainer* pi_pExportProperties, 
                    int32_t * po_pExitCode);

bool CreateThumbnail(HFCPtr<HFCURLFile>& pi_rpINURLFile,
                      HFCPtr<HFCURLFile>& pi_rpOUTURLFile,
                      ExportPropertiesContainer* pi_pProperties);

#endif

void PrintArgumentError(const WChar* InvalidArgument);
void ExportRasterFile(ExportPropertiesContainer* pi_pExportProperties,
                      HFCPtr<HFCURLFile>&        pi_pSrcFilename,
                      HFCPtr<HFCURLFile>&        pi_pDstFilename,
                      HPMPool*                   pio_pPool);

uint32_t ArgumentParser(ExportPropertiesContainer* po_pExportProperties, 
                      int argc, 
                      WChar* argv[],
                      int32_t * po_pExitCode);

bool BuildPathTree                 (HFCPtr<HFCURLFile>& pi_pCompletePath);
bool IsDirectory                   (HFCPtr<HFCURLFile>& pi_pSrcFilename);
bool IsFileNameExist               (HFCPtr<HFCURLFile>& pi_pSrcFilename);
bool IsWildcardFound               (HFCPtr<HFCURLFile>& pi_pSrcFilename);
bool IsRelativePath                (HFCPtr<HFCURLFile>& pi_pSrcFilename);
bool ValidateIfCopyPyramidIsChoose (HFCPtr<HFCURLFile>& pi_pSrcFilename,
                                     const ExportPropertiesContainer& pi_rExportProperties);
bool ExtractValueFromStringAtIndex (Utf8String pi_StringFilterValue, uint32_t* pio_pIndex, double* po_Value);
bool GetFilterValueFromString      (Utf8String pi_StringFilterValue, 
                                     double* po_pFirstValue,
                                     double* po_pSecondValue,
                                     double* po_pThirdValue);


Utf8String GetLeaf                (Utf8String CompletePath, int LeafLevel);
Utf8String PathInverseSubstractor (HFCPtr<HFCURLFile>& pi_pCompletePath, 
                               HFCPtr<HFCURLFile>& pi_pPathPartToRemove);

unsigned int GetLeafCount(Utf8String CompletePath);

HFCPtr<HFCURLFile> ConvertName      (HFCPtr<HFCURLFile>& pi_pSrcFilename, const Utf8String& pi_rNewExt);
HFCPtr<HFCURLFile> UnRelativePath   (HFCPtr<HFCURLFile>& pi_pSrcFilename);
HFCPtr<HFCURLFile> ConvertExtension (HFCPtr<HFCURLFile>& pi_pSrcFilename, const Utf8String& pi_rNewExt);
HFCPtr<HFCURLFile> ComposeFileNameWithNewPath(HFCPtr<HFCURLFile>& pi_pSrcFilename, 
                                              HFCPtr<HFCURLFile>& pi_pDestPath);

void GetResampleImageSize(const HFCPtr<HRFRasterFile>&  pi_rpRasterFile, 
                          const HFCPtr<HGF2DWorldCluster>&
                                                        pi_rpWorldCluster,
                          uint32_t*                       po_pWidth, 
                          uint32_t*                       po_pHeight);

bool AddBlobInFile                 (const ExportPropertiesContainer* pi_pExportProperties, 
                                     HFCPtr<HFCURLFile>&              pi_pDstFilename);
void DumpBlobInFile                 (HFCPtr<HFCURLFile>&              pi_pSrcFilename,
                                     HFCPtr<HFCURLFile>&              pi_pDstFilename);

//-----------------------------------------------------------------------------
// class ExportPropertiesContainer
//-----------------------------------------------------------------------------

class ExportPropertiesContainer
{
    public:

        ExportPropertiesContainer();
        ~ExportPropertiesContainer();

        uint32_t                  PoolSizeInKB;

        HCLASS_ID               PixelType;
        bool                   PixelTypeSpecified;

        HCLASS_ID               SubPixelType;
        bool                   SubPixelTypeSpecified;

        bool                   CodecSpecified;
        HCLASS_ID               CodecType;
        HFCPtr<HCDCodec>        pCodec;

        bool                   SubCodecSpecified;
        HCLASS_ID               SubCodecType;
        HFCPtr<HCDCodec>        pSubCodec;

        uint32_t                  Quality;
        bool                   QualitySpecified;

        uint32_t                  SubQuality;
        bool                   SubQualitySpecified;

        HRFEncodingType         EncodingType;    
        bool                   EncodingTypeSpecified;

        HRFBlockType            BlockType;
        bool                   BlockTypeSpecified;

        bool                   BlockWidthSpecified;
        bool                   BlockHeightSpecified;

        HRFDownSamplingMethod   ResamplingMethod;
        bool                   ResamplingSpecified;
        bool                   ResamplingCopyPyramid;

        bool                   CubicConvolutionSpecified;
        HCLASS_ID               CubicConvolutionFilter;

        bool                   ReplaceExtensionSpecified;
        bool                   dnOptionSpecified;
        bool                   OutputOverwriteSpecified;
        bool                   ScanAllSubDirectorySpecified;

        bool                   WrongArgumentFound;
        
        uint32_t                  BlockWidth;
        uint32_t                  BlockHeight;


        bool                   FileFormatSpecified;
        Utf8String                  FileFormat;

        bool                   ForceResampling;
        bool                   FeedbackOn;             // Display progress on screen

#ifdef THUMBNAIL_GENERATOR

        bool                   ThumbnailSpecified;
        bool                   ThumbnailSizeSpecified;
        uint32_t                  ThumbnailWidth;
        uint32_t                  ThumbnailHeight;
        bool                   BackgroundSpecified;
        Byte                  BackgroundRed;
        Byte                  BackgroundGreen;
        Byte                  BackgroundBlue;
        Byte                  ThumbnailNbBits;
#endif
#ifdef _HMR_ALTAPHOTO
        Utf8String                 AltaPhotoBlobName; 
        bool                   ConvolutionFilterSpecified;
        bool                   AtlaPhotoDumpBlob;      // debug use only   
#endif
        
        // Members for reprojection
        bool                   Reproject;
        uint32_t                  SourceProjection;
        uint32_t                  DestinationProjection;
        double                 MaxReprojectionError;
        Utf8String                 ReprojectDatabase;

        bool                   CreateOnDemandMosaicPSS;
        bool                   CreateOnDemandMosaicPSSCache;
        uint32_t                  PSSCacheDimension;
        bool                   IsPSSCacheDimensionAPercentage;
        uint32_t                  PSSMinimumDimensionForCache;
};

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

inline ExportPropertiesContainer::ExportPropertiesContainer()
{
    WrongArgumentFound              = false;
    
    PixelTypeSpecified              = false;
    SubPixelTypeSpecified           = false;
    CodecSpecified                  = false;
    SubCodecSpecified               = false;
    Quality = 95;
    QualitySpecified                = false;
    SubQuality = 95;
    SubQualitySpecified             = false;

    EncodingTypeSpecified           = false;
    BlockType                       = HRFBlockType::LINE;
    BlockTypeSpecified              = false;
    
    ResamplingSpecified             = false;
    ResamplingCopyPyramid           = false;

    CubicConvolutionSpecified       = false;

    ReplaceExtensionSpecified       = true;
    dnOptionSpecified               = false;
    OutputOverwriteSpecified        = false;
    ScanAllSubDirectorySpecified    = false;
    
    PoolSizeInKB                    = 16192;

    BlockWidth                      = 256;
    BlockHeight                     = 256;
    FileFormatSpecified             = false;
    FileFormat                      = "itiff";

    ForceResampling                 = false;
    FeedbackOn                      = false;

#ifdef THUMBNAIL_GENERATOR
    BackgroundSpecified             = false;
    BackgroundRed                   = 0;
    BackgroundGreen                 = 0;
    BackgroundBlue                  = 0;

    ThumbnailSpecified              = false;
    ThumbnailSizeSpecified          = false;
    ThumbnailWidth                  = 96;
    ThumbnailHeight                 = 96;
    ThumbnailNbBits                 = 24;
#endif
#ifdef _HMR_ALTAPHOTO
    AtlaPhotoDumpBlob               = false;
    ConvolutionFilterSpecified      = false;
#endif
    Reproject                       = false;
    SourceProjection                = 0;
    DestinationProjection           = 0;
    MaxReprojectionError            = 0.0;

    CreateOnDemandMosaicPSS         = false;
    CreateOnDemandMosaicPSSCache    = false;

    PSSCacheDimension               = PSS_MIN_CACHE_DIMENSION_IN_PERCENT;   //Default cache dimension is 10% of the PSS dimension
    IsPSSCacheDimensionAPercentage  = true;
    PSSMinimumDimensionForCache     = PSS_MIN_DIMENSION_FOR_CACHE; //Default mimimum PSS dimension is 4096
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------

inline ExportPropertiesContainer::~ExportPropertiesContainer()
{
    // There is nothing to do here at this time.
}

#endif // __ConverterUtilities__H__
