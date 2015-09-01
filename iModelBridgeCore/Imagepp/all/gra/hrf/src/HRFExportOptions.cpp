//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFExportOptions.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class: HRFExportOptions
// ----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRFExportOptions.h>
#include <Imagepp/all/h/HPMPersistentObject.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDCodecFactory.h>



/** -----------------------------------------------------------------------------
    HRFExportOptions default constructor defined of persitense only.

    @h3{Note:}
    This constructor should never be used.
    @end
    -----------------------------------------------------------------------------
*/
HRFExportOptions::HRFExportOptions()
    {
    // NOTE : HPMPersistentObject::CLASS_ID is used as default value for all HMPClassKey
    //        member. Since HPMPersistentObject can not be instanciate, no class will ever
    //        have this ClassKey.

    m_pFileFormat               = 0;

    m_PixelType                 = HPMPersistentObject::CLASS_ID;
    m_SubResPixelType           = HPMPersistentObject::CLASS_ID;

    m_DownSamplingMethod        = HRFDownSamplingMethod::NONE;
    m_SubResDownSamplingMethod  = HRFDownSamplingMethod::NONE;

    // Default compression identity (the codec sample must be valid)
    m_pCodec                    = new HCDCodecIdentity();
    HASSERT(m_pCodec != 0);

    m_CompressionQuality        = 0;
    m_CompressionRatio          = 0;

    // Default compression identity (the codec sample must be valid)
    m_pSubResCodec                = new HCDCodecIdentity();
    HASSERT(m_pSubResCodec != 0);
    m_SubResCompressionQuality  = 0;
    m_SubResCompressionRatio  = 0;

    m_BlockType                 = HRFBlockType::AUTO_DETECT;
    m_BlockWidth                = 0;
    m_BlockHeight               = 0;

    m_SubResBlockType           = HRFBlockType::AUTO_DETECT;
    m_SubResBlockWidth          = 0;
    m_SubResBlockHeight         = 0;

    m_Resample                  = 0;
    m_ImageWidth                = 0;
    m_ImageHeight               = 0;
    m_ScaleFactorX              = 0.0;
    m_ScaleFactorY              = 0.0;

    m_Encoding                  = HRFEncodingType::STANDARD;
    m_GeoreferenceFormat        = HRFGeoreferenceFormat::GEOREFERENCE_IN_IMAGE;
    m_pGeocoding                = RasterFileGeocoding::Create();
    }

/** -----------------------------------------------------------------------------
    HRFExportOptions constructor.
    @end

    @param pi_pFileFormat              Selected file format creator for the current export.
    @param pi_PixelType                Selected pixel type class key for the current export.
    @param pi_SubResPixelType          Selected sub-resolution pixel type for the current export.
    @param pi_DownSamplingMethod       Selected method for sub-resolution calculation.
    @param pi_SubResDownSamplingMethod Selected method for sub-resolution calculation.
    @param pi_TagList                  Selected tag list for the current export.
    @param pi_Codec                    Selected codec class key for the current export.
    @param pi_CompressionQuality       Selected compression quality for the current export.
    @param pi_SubResCodec              Selected codec class key for sub-resolutions for the current export.
    @param pi_SubResCompressionQuality Selected compression quality for sub-resolutions for the current export.
    @param pi_Resample                 Booleen value that indicates if resampling must be applied
                                       for this export. If true, resampling will be applied.
    @param pi_ImageWidth               Selected image width for the current export.
    @param pi_ImageHeight              Selected image height for the current export.
    @param pi_ScaleFactorX             Selected scale factor in X for the current export.
    @param pi_ScaleFactorY             Selected scale factor in Y for the current export.
    @param pi_BlockType                Selected block type for the current export.
    @param pi_BlockWidth               Selected block width for the current export.
    @param pi_BlockHeight              Selected block height for the current export.
    @param pi_SubResBlockType          Selected block type for sub-resolution for the current export.
    @param pi_SubResBlockWidth         Selected block width for sub-resolution for the current export.
    @param pi_SubResBlockHeight        Selected block height for sub-resolution for the current export.
    @param pi_Encoding                 Selected encoding type for sub-resolution for the current export.
                                       Encoding can be : Multiresolution, Standard(No sub-resolution
                                       is written to file) or Progressive.
    @param pi_GeoreferenceFormat       Selected georeference format for the current export.
    @param pi_CompressionRatio         Selected compression ratio for the current export.
    @param pi_SubResCompressionRatio   Selected compression ratio for sub-resolutions for the current export.
    @param pi_pGeocoding               The geocoding of the exported file. This can be NULL

    @end

    @see HRFImportExport
    @see HUTExportToFile
    @end
    -----------------------------------------------------------------------------
*/
HRFExportOptions::HRFExportOptions(const HRFRasterFileCreator*  pi_pFileFormat,
                                   HCLASS_ID                    pi_PixelType,
                                   HCLASS_ID                    pi_SubResPixelType,
                                   HRFDownSamplingMethod        pi_DownSamplingMethod,
                                   HRFDownSamplingMethod        pi_SubResDownSamplingMethod,
                                   HPMAttributeSet              pi_TagList,
                                   HCLASS_ID                    pi_Codec,
                                   uint32_t                    pi_CompressionQuality,
                                   HCLASS_ID                    pi_SubResCodec,
                                   uint32_t                    pi_SubResCompressionQuality,
                                   bool                         pi_Resample,
                                   uint32_t                    pi_ImageWidth,
                                   uint32_t                    pi_ImageHeight,
                                   double                       pi_ScaleFactorX,
                                   double                       pi_ScaleFactorY,
                                   HRFBlockType                 pi_BlockType,
                                   uint32_t                    pi_BlockWidth,
                                   uint32_t                    pi_BlockHeight,
                                   HRFBlockType                 pi_SubResBlockType,
                                   uint32_t                    pi_SubResBlockWidth,
                                   uint32_t                    pi_SubResBlockHeight,
                                   HRFEncodingType              pi_Encoding,
                                   HRFGeoreferenceFormat        pi_GeoreferenceFormat,
                                   uint32_t                    pi_CompressionRatio,
                                   uint32_t                    pi_SubResCompressionRatio,
                                   IRasterBaseGcsP              pi_pGeocoding)
    {
    m_pFileFormat               = const_cast<HRFRasterFileCreator*> (pi_pFileFormat);

    m_PixelType                 = pi_PixelType;
    m_SubResPixelType           = pi_SubResPixelType;

    m_DownSamplingMethod        = pi_DownSamplingMethod;
    m_SubResDownSamplingMethod  = pi_SubResDownSamplingMethod;

    // Codec to keep table and codec configuration
    m_pCodec                    = HCDCodecFactory::GetInstance().Create(pi_Codec);
    HASSERT(m_pCodec != 0);
    m_CompressionQuality        = pi_CompressionQuality;
    m_CompressionRatio          = pi_CompressionRatio;

    if(pi_SubResCodec != 0)
        {
        // Codec to keep table and codec configuration
        m_pSubResCodec = HCDCodecFactory::GetInstance().Create(pi_SubResCodec);
        m_SubResCompressionQuality  = pi_SubResCompressionQuality;
        m_SubResCompressionRatio    = pi_SubResCompressionRatio;
        }

    m_BlockType                 = pi_BlockType;
    m_BlockWidth                = pi_BlockWidth;
    m_BlockHeight               = pi_BlockHeight;

    m_SubResBlockType           = pi_SubResBlockType;
    m_SubResBlockWidth          = pi_SubResBlockWidth;
    m_SubResBlockHeight         = pi_SubResBlockHeight;

    m_Resample                  = pi_Resample;
    m_ImageWidth                = pi_ImageWidth;
    m_ImageHeight               = pi_ImageHeight;
    m_ScaleFactorX              = pi_ScaleFactorX;
    m_ScaleFactorY              = pi_ScaleFactorY;
    m_Encoding                  = pi_Encoding;
    m_GeoreferenceFormat        = pi_GeoreferenceFormat;
    m_pGeocoding                = RasterFileGeocoding::Create(pi_pGeocoding);

    // Set all the tags.
    HPMAttributeSet::HPMASiterator TagIterator;

    for (TagIterator = pi_TagList.begin();
         TagIterator != pi_TagList.end(); ++TagIterator)
        {
        SetTag(*TagIterator);
        }

    }

/** -----------------------------------------------------------------------------
    HRFExportOptions constructor. This constructor should be used to keep codecs
    configurations. It take a codec's pointer instead of a codec's class key.
    @end

    @param pi_pFileFormat              Selected file format creator for the current export.
    @param pi_PixelType                Selected pixel type class key for the current export.
    @param pi_SubResPixelType          Selected sub-resolution pixel type for the current export.
    @param pi_DownSamplingMethod       Selected method for sub-resolution caculation.
    @param pi_SubResDownSamplingMethod Selected method for sub-resolution caculation.
    @param pi_TagList                  Selected tag list for the current export.
    @param pi_rCodec                   Selected codec's pointer for the current export. This pointer
                                       can contain extra infomation concerning codecs configuration.
    @param pi_CompressionQuality       Selected compression quality for the current export.
    @param pi_rSubResCodec             Selected codec pointer for sub-resolution for the current export.
                                       This pointer can contain extra infomation concerning codecs configuration.
    @param pi_SubResCompressionQuality Selected compression quality for sub-resolution for the current export.
    @param pi_Resample                 Booleen value that indicates if resampling must be applied
                                       for this export. If true, resampling will be applied.
    @param pi_ImageWidth               Selected image width for the current export.
    @param pi_ImageHeight              Selected image height for the current export.
    @param pi_ScaleFactorX             Selected scale factor in X for the current export.
    @param pi_ScaleFactorY             Selected scale factor in Y for the current export.
    @param pi_BlockType                Selected block type for the current export.
    @param pi_BlockWidth               Selected block width for the current export.
    @param pi_BlockHeight              Selected block height for the current export.
    @param pi_SubResBlockType          Selected block type for sub-resolution for the current export.
    @param pi_SubResBlockWidth         Selected block width for sub-resolution for the current export.
    @param pi_SubResBlockHeight        Selected block heigth for sub-resolution for the current export.
    @param pi_Encoding                 Selected encoding type for sub-resolution for the current export.
                                       Encoding can be : Multiresolution, Standard(No sub-resolution is written to the file)
                                       or Progressive.
    @param pi_GeoreferenceFormat       Selected georeference format for the current export.
    @param pi_CompressionRatio         Selected compression ratio for the current export.
    @param pi_SubResCompressionRatio   Selected compression ratio for sub-resolution for the current export.
    @param pi_pGeocoding               The geocoding of the exported file. This can be NULL
    @end

    @see HRFImportExport
    @see HUTExportToFile
    @end
    -----------------------------------------------------------------------------
*/
HRFExportOptions::HRFExportOptions(const HRFRasterFileCreator*  pi_pFileFormat,
                                   HCLASS_ID                    pi_PixelType,
                                   HCLASS_ID                    pi_SubResPixelType,
                                   HRFDownSamplingMethod        pi_DownSamplingMethod,
                                   HRFDownSamplingMethod        pi_SubResDownSamplingMethod,
                                   HPMAttributeSet              pi_TagList,
                                   HFCPtr<HCDCodec>&            pi_rpCodec,
                                   uint32_t                    pi_CompressionQuality,
                                   HFCPtr<HCDCodec>&            pi_rpSubResCodec,
                                   uint32_t                    pi_SubResCompressionQuality,
                                   bool                         pi_Resample,
                                   uint32_t                    pi_ImageWidth,
                                   uint32_t                    pi_ImageHeight,
                                   double                       pi_ScaleFactorX,
                                   double                       pi_ScaleFactorY,
                                   HRFBlockType                 pi_BlockType,
                                   uint32_t                    pi_BlockWidth,
                                   uint32_t                    pi_BlockHeight,
                                   HRFBlockType                 pi_SubResBlockType,
                                   uint32_t                    pi_SubResBlockWidth,
                                   uint32_t                    pi_SubResBlockHeight,
                                   HRFEncodingType              pi_Encoding,
                                   HRFGeoreferenceFormat        pi_GeoreferenceFormat,
                                   uint32_t                    pi_CompressionRatio,
                                   uint32_t                    pi_SubResCompressionRatio,
                                   IRasterBaseGcsP              pi_pGeocoding)
    {
    HPRECONDITION(pi_rpCodec != 0);
    HPRECONDITION(pi_rpSubResCodec != 0);

    m_pFileFormat               = const_cast<HRFRasterFileCreator*> (pi_pFileFormat);

    m_PixelType                 = pi_PixelType;
    m_SubResPixelType           = pi_SubResPixelType;

    m_DownSamplingMethod        = pi_DownSamplingMethod;
    m_SubResDownSamplingMethod  = pi_SubResDownSamplingMethod;

    // Codec to keep table and codec configuration
    m_pCodec                    = pi_rpCodec;
    m_CompressionQuality        = pi_CompressionQuality;
    m_CompressionRatio          = pi_CompressionRatio;

    // Codec to keep table and codec configuration
    m_pSubResCodec              = pi_rpSubResCodec;
    m_SubResCompressionQuality  = pi_SubResCompressionQuality;
    m_SubResCompressionRatio    = pi_SubResCompressionRatio;

    m_BlockType                 = pi_BlockType;
    m_BlockWidth                = pi_BlockWidth;
    m_BlockHeight               = pi_BlockHeight;

    m_SubResBlockType           = pi_SubResBlockType;
    m_SubResBlockWidth          = pi_SubResBlockWidth;
    m_SubResBlockHeight         = pi_SubResBlockHeight;

    m_Resample                  = pi_Resample;
    m_ImageWidth                = pi_ImageWidth;
    m_ImageHeight               = pi_ImageHeight;
    m_ScaleFactorX              = pi_ScaleFactorX;
    m_ScaleFactorY              = pi_ScaleFactorY;

    m_Encoding                  = pi_Encoding;
    m_GeoreferenceFormat        = pi_GeoreferenceFormat;
    m_pGeocoding                = RasterFileGeocoding::Create(pi_pGeocoding);

    // Set all the tags.
    HPMAttributeSet::HPMASiterator TagIterator;

    for (TagIterator = pi_TagList.begin();
         TagIterator != pi_TagList.end(); TagIterator++)
        {
        SetTag(*TagIterator);
        }
    }

/** -----------------------------------------------------------------------------
    Destructor.
    Nothing to do.
    -----------------------------------------------------------------------------
*/
HRFExportOptions::~HRFExportOptions()
    {
    }

/** -----------------------------------------------------------------------------
    Copy constructor.
    @param pi_rExportOptions
    -----------------------------------------------------------------------------
*/
HRFExportOptions::HRFExportOptions(const HRFExportOptions& pi_rExportOptions)
    {
    if (&pi_rExportOptions != this)
        DeepCopy(pi_rExportOptions);
    }

/** -----------------------------------------------------------------------------
    Operator equal.
    @param pi_rExportOptions
    -----------------------------------------------------------------------------
*/
HRFExportOptions& HRFExportOptions::operator=(const HRFExportOptions& pi_rExportOptions)
    {
    if (&pi_rExportOptions != this)
        DeepCopy(pi_rExportOptions);

    return *this;
    }

/** -----------------------------------------------------------------------------
    Copy a HRFExportOption into a other. Used by copy constructor and equality
    operator.
    @param pi_rExportOptions
    -----------------------------------------------------------------------------
*/
void HRFExportOptions::DeepCopy(const HRFExportOptions& pi_rExportOptions)
    {
    m_pFileFormat               = pi_rExportOptions.m_pFileFormat;

    m_PixelType                 = pi_rExportOptions.m_PixelType;
    m_SubResPixelType           = pi_rExportOptions.m_SubResPixelType;

    m_DownSamplingMethod        = pi_rExportOptions.m_DownSamplingMethod;
    m_SubResDownSamplingMethod  = pi_rExportOptions.m_SubResDownSamplingMethod;

    m_pCodec                    = pi_rExportOptions.m_pCodec;
    m_CompressionQuality        = pi_rExportOptions.m_CompressionQuality;
    m_CompressionRatio          = pi_rExportOptions.m_CompressionRatio;

    m_pSubResCodec              = pi_rExportOptions.m_pSubResCodec;
    m_SubResCompressionQuality  = pi_rExportOptions.m_SubResCompressionQuality;
    m_SubResCompressionRatio    = pi_rExportOptions.m_SubResCompressionRatio;

    m_BlockType                 = pi_rExportOptions.m_BlockType;
    m_BlockWidth                = pi_rExportOptions.m_BlockWidth;
    m_BlockHeight               = pi_rExportOptions.m_BlockHeight;

    m_SubResBlockType           = pi_rExportOptions.m_SubResBlockType;
    m_SubResBlockWidth          = pi_rExportOptions.m_SubResBlockWidth;
    m_SubResBlockHeight         = pi_rExportOptions.m_SubResBlockHeight;

    m_Resample                  = pi_rExportOptions.m_Resample;
    m_ImageWidth                = pi_rExportOptions.m_ImageWidth;
    m_ImageHeight               = pi_rExportOptions.m_ImageHeight;
    m_ScaleFactorX              = pi_rExportOptions.m_ScaleFactorX;
    m_ScaleFactorY              = pi_rExportOptions.m_ScaleFactorY;

    m_Encoding                  = pi_rExportOptions.m_Encoding;
    m_GeoreferenceFormat        = pi_rExportOptions.m_GeoreferenceFormat;
    m_ListOfMetaDataContainer   = pi_rExportOptions.m_ListOfMetaDataContainer;
    m_pGeocoding                = pi_rExportOptions.m_pGeocoding->Clone();

    // Set all the tags.
    HPMAttributeSet::HPMASiterator TagIterator;

    for (TagIterator = pi_rExportOptions.m_TagList.begin();
         TagIterator != pi_rExportOptions.m_TagList.end(); ++TagIterator)
        {
        SetTag(*TagIterator);
        }
    }

/** -----------------------------------------------------------------------------
    Operator ==.
    @param pi_rExportOptions
    -----------------------------------------------------------------------------
*/
bool HRFExportOptions::operator==(HRFExportOptions& pi_rExportOptions) const
    {
    bool IsEqual = false;

    if ((m_pFileFormat                 == pi_rExportOptions.m_pFileFormat                ) &&
        (m_PixelType                   == pi_rExportOptions.m_PixelType                  ) &&
        (m_SubResPixelType             == pi_rExportOptions.m_SubResPixelType            ) &&
        (m_DownSamplingMethod          == pi_rExportOptions.m_DownSamplingMethod         ) &&
        (m_SubResDownSamplingMethod    == pi_rExportOptions.m_SubResDownSamplingMethod   ) &&
        (m_pCodec->GetClassID()       == pi_rExportOptions.m_pCodec->GetClassID()      ) &&
        (m_CompressionQuality          == pi_rExportOptions.m_CompressionQuality         ) &&
        (m_pSubResCodec->GetClassID() == pi_rExportOptions.m_pSubResCodec->GetClassID()) &&
        (m_SubResCompressionQuality    == pi_rExportOptions.m_SubResCompressionQuality   ) &&
        (m_BlockType                   == pi_rExportOptions.m_BlockType                  ) &&
        (m_BlockWidth                  == pi_rExportOptions.m_BlockWidth                 ) &&
        (m_BlockHeight                 == pi_rExportOptions.m_BlockHeight                ) &&
        (m_SubResBlockType             == pi_rExportOptions.m_SubResBlockType            ) &&
        (m_SubResBlockWidth            == pi_rExportOptions.m_SubResBlockWidth           ) &&
        (m_SubResBlockHeight           == pi_rExportOptions.m_SubResBlockHeight          ) &&
        (m_Resample                    == pi_rExportOptions.m_Resample                   ) &&
        (m_ImageWidth                  == pi_rExportOptions.m_ImageWidth                 ) &&
        (m_ImageHeight                 == pi_rExportOptions.m_ImageHeight                ) &&
        (m_ScaleFactorX                == pi_rExportOptions.m_ScaleFactorX               ) &&
        (m_ScaleFactorY                == pi_rExportOptions.m_ScaleFactorY               ) &&
        (m_Encoding                    == pi_rExportOptions.m_Encoding                   ) &&
        (m_GeoreferenceFormat          == pi_rExportOptions.m_GeoreferenceFormat         ) &&
        (m_CompressionRatio            == pi_rExportOptions.m_CompressionRatio           ) &&
        (m_SubResCompressionRatio      == pi_rExportOptions.m_SubResCompressionRatio     ) &&
        ((m_pGeocoding                 == pi_rExportOptions.m_pGeocoding) ||
         ((m_pGeocoding->GetGeocodingCP() != NULL) && (pi_rExportOptions.m_pGeocoding->GetGeocodingCP() != NULL) && 
          (m_pGeocoding->GetGeocodingCP()->IsValid()) && (pi_rExportOptions.m_pGeocoding->GetGeocodingCP()->IsValid()) &&  
          (m_pGeocoding->GetGeocodingCP()->IsEquivalent(*(pi_rExportOptions.m_pGeocoding->GetGeocodingCP()))))               ))
        {
        // !!!! HChckSebG !!!!
        // Check also for taglist equality ???? if m_TagList;
        //
        IsEqual = true;
        }
    return IsEqual;
    }

/** -----------------------------------------------------------------------------
    @return The selected raster file creator for this export set option. This is
            the destination file format creator.
    -----------------------------------------------------------------------------
*/
HRFRasterFileCreator* HRFExportOptions::GetFileFormat() const
    {
    return m_pFileFormat;
    }

/** -----------------------------------------------------------------------------
    @return The selected pixel type class key for this export set option.
    -----------------------------------------------------------------------------
*/
HCLASS_ID HRFExportOptions::GetPixelType() const
    {
    return m_PixelType;
    }

/** -----------------------------------------------------------------------------
    @return The selected pixel type class key for sub-resolutions for this export
            set option.
    -----------------------------------------------------------------------------
*/
HCLASS_ID HRFExportOptions::GetSubResPixelType() const
    {
    return m_SubResPixelType;
    }

/** -----------------------------------------------------------------------------
    @return The selected method used to calulate the sub-resolution for this
            export set option.
    -----------------------------------------------------------------------------
*/
HRFDownSamplingMethod HRFExportOptions::GetDownSamplingMethod() const
    {
    return m_DownSamplingMethod;
    }

/** -----------------------------------------------------------------------------
    @return The selected method used to calulate the sub-resolution for this
            export set option.
    -----------------------------------------------------------------------------
*/
HRFDownSamplingMethod HRFExportOptions::GetSubResDownSamplingMethod() const
    {
    return m_SubResDownSamplingMethod;
    }

/** -----------------------------------------------------------------------------
    @return A list of the selected tags for this export set option.
    -----------------------------------------------------------------------------
*/
const HPMAttributeSet& HRFExportOptions::GetTagList() const
    {
    return m_TagList;
    }

//-----------------------------------------------------------------------------
// public : GetMetaDataContainerList
// Get the metadata container of the specified type
//-----------------------------------------------------------------------------
const HMDMetaDataContainerList& HRFExportOptions::GetMetaDataContainerList() const
    {
    return m_ListOfMetaDataContainer;
    }

/** -----------------------------------------------------------------------------
    @return The selected codec's class key for for this export set option.
    -----------------------------------------------------------------------------
*/
HCLASS_ID HRFExportOptions::GetCodec() const
    {
    if (m_pCodec)
        return m_pCodec->GetClassID();
    else
        return HPMPersistentObject::CLASS_ID;
    }


/** -----------------------------------------------------------------------------
    @return A pointer on the codec set for this export. This pointer can
            contain extra infomation concerning codecs configurations.
    -----------------------------------------------------------------------------
*/
const HFCPtr<HCDCodec>& HRFExportOptions::GetCodecSample() const
    {
    return m_pCodec;
    }


/** -----------------------------------------------------------------------------
    @return The compression quality set for this export.
    -----------------------------------------------------------------------------
*/
uint32_t HRFExportOptions::GetCompressionQuality() const
    {
    return m_CompressionQuality;
    }

/** -----------------------------------------------------------------------------
@return The compression quality set for this export.
-----------------------------------------------------------------------------
*/
uint32_t HRFExportOptions::GetCompressionRatio() const
    {
    return m_CompressionRatio;
    }


/** -----------------------------------------------------------------------------
    @return The selected codec's class key for sub-resolutions for this export
            set option.
    -----------------------------------------------------------------------------
*/
HCLASS_ID HRFExportOptions::GetSubResCodec() const
    {
    if (m_pSubResCodec)
        return m_pSubResCodec->GetClassID();
    else
        return HPMPersistentObject::CLASS_ID;

    }


/** -----------------------------------------------------------------------------
    @return The a pointer on the codec set for this export. This pointer can
            contain extra information concerning codecs configurations.
    -----------------------------------------------------------------------------
*/
const HFCPtr<HCDCodec>& HRFExportOptions::GetSubResCodecSample() const
    {
    return m_pSubResCodec;
    }

/** -----------------------------------------------------------------------------
    @return The compression quality set for this export.
    -----------------------------------------------------------------------------
*/
uint32_t HRFExportOptions::GetSubResCompressionQuality() const
    {
    return m_SubResCompressionQuality;
    }

/** -----------------------------------------------------------------------------
@return The compression ratio set for this export.
-----------------------------------------------------------------------------
*/
uint32_t HRFExportOptions::GetSubResCompressionRatio() const
    {
    return m_SubResCompressionRatio;
    }

/** -----------------------------------------------------------------------------
    @return True if resampling must be apply on this export.
    -----------------------------------------------------------------------------
*/
bool HRFExportOptions::GetResample() const
    {
    return m_Resample;
    }

/** -----------------------------------------------------------------------------
    @return The image width for this export.
    -----------------------------------------------------------------------------
*/
uint32_t HRFExportOptions::GetImageWidth() const
    {
    return m_ImageWidth;
    }

/** -----------------------------------------------------------------------------
    @return The image height for this export.
    -----------------------------------------------------------------------------
*/
uint32_t HRFExportOptions::GetImageHeight() const
    {
    return m_ImageHeight;
    }

/** -----------------------------------------------------------------------------
    @return The scale factor in x for this export.
    -----------------------------------------------------------------------------
*/
double HRFExportOptions::GetScaleFactorX() const
    {
    return m_ScaleFactorX;
    }

/** -----------------------------------------------------------------------------
    @return The scale factor in y for this export.
    -----------------------------------------------------------------------------
*/
double HRFExportOptions::GetScaleFactorY() const
    {
    return m_ScaleFactorY;
    }

/** -----------------------------------------------------------------------------
    @return The block type for this export.
    -----------------------------------------------------------------------------
*/
HRFBlockType HRFExportOptions::GetBlockType() const
    {
    return m_BlockType;
    }

/** -----------------------------------------------------------------------------
    @return The block width for this export.
    -----------------------------------------------------------------------------
*/
uint32_t HRFExportOptions::GetBlockWidth() const
    {
    return m_BlockWidth;
    }

/** -----------------------------------------------------------------------------
    @return The block height for this export.
    -----------------------------------------------------------------------------
*/
uint32_t HRFExportOptions::GetBlockHeight() const
    {
    return m_BlockHeight;
    }

/** -----------------------------------------------------------------------------
    @return The block type for sub-resolution for this export.
    -----------------------------------------------------------------------------
*/
HRFBlockType HRFExportOptions::GetSubResBlockType() const
    {
    return m_SubResBlockType;
    }

/** -----------------------------------------------------------------------------
    @return The block width for sub-resolution for this export.
    -----------------------------------------------------------------------------
*/
uint32_t HRFExportOptions::GetSubResBlockWidth() const
    {
    return m_SubResBlockWidth;
    }

/** -----------------------------------------------------------------------------
    @return The block heigth for sub-resolution for this export.
    -----------------------------------------------------------------------------
*/
uint32_t HRFExportOptions::GetSubResBlockHeight() const
    {
    return m_SubResBlockHeight;
    }


/** -----------------------------------------------------------------------------
    @return The encoding type for this export. Encoding can be : Multiresolution,
            Standard(No sub-resolution is writed to file) or Progressive.
    -----------------------------------------------------------------------------
*/
HRFEncodingType HRFExportOptions::GetEncoding() const
    {
    return m_Encoding;
    }

/** -----------------------------------------------------------------------------
    @return The georereference format for this export. This is the way that
            tranformation should be save in the file. (In HGR, in World File ...)
    -----------------------------------------------------------------------------
*/
HRFGeoreferenceFormat HRFExportOptions::GetGeoreferenceFormat() const
    {
    return m_GeoreferenceFormat;
    }

/** -----------------------------------------------------------------------------
    @return The geocoding pointer for this export. The pointer returned may be NULL 
             if no geocoding is specified.
    -----------------------------------------------------------------------------
*/
RasterFileGeocoding const& HRFExportOptions::GetRasterFileGeocoding() const
    {
    return *m_pGeocoding;
    }



/** -----------------------------------------------------------------------------
    In this method we assume that all the data set in this objet is valid. @b{So the
    caller must validate all paramater before setting them.}

    @param pi_pCreator The selected raster file creator for this export set option. This is
                       the destination file format creator.
    -----------------------------------------------------------------------------
*/
void HRFExportOptions::SetFileFormat(const HRFRasterFileCreator* pi_pCreator)
    {
    m_pFileFormat = const_cast<HRFRasterFileCreator*> (pi_pCreator);
    }

/** -----------------------------------------------------------------------------
    In this method we assume that all the data set in this objet is valid. @b{So the
    caller must validate all paramater before setting them.}

    @param pi_PixelType The pixel type class key for the current export.
    -----------------------------------------------------------------------------
*/
void HRFExportOptions::SetPixelType(HCLASS_ID pi_PixelType)
    {
    m_PixelType = pi_PixelType;
    }

/** -----------------------------------------------------------------------------
    In this method we assume that all the data set in this objet is valid. @b{So the
    caller must validate all paramater before setting them.}

    @param pi_SubResPixelType The sub-resolution pixel type class key for the current export
    -----------------------------------------------------------------------------
*/
void HRFExportOptions::SetSubResPixelType(HCLASS_ID pi_SubResPixelType)
    {
    m_SubResPixelType = pi_SubResPixelType;
    }

/** -----------------------------------------------------------------------------
    In this method we assume that all the data set in this objet is valid. @b{So the
    caller must validate all paramater before setting them.}

    @param pi_DownSamplingMethod The method for sub-resolution calculation.
    -----------------------------------------------------------------------------
*/
void HRFExportOptions::SetDownSamplingMethod(HRFDownSamplingMethod pi_DownSamplingMethod)
    {
    m_DownSamplingMethod = pi_DownSamplingMethod;
    }

/** -----------------------------------------------------------------------------
    In this method we assume that all the data set in this objet is valid. @b{So the
    caller must validate all paramater before setting them.}

    @param pi_SubResDownSamplingMethod The method for sub-resolution calculation.
    -----------------------------------------------------------------------------
*/
void HRFExportOptions::SetSubResDownSamplingMethod(HRFDownSamplingMethod pi_SubResDownSamplingMethod)
    {
    m_SubResDownSamplingMethod = pi_SubResDownSamplingMethod;
    }

/** -----------------------------------------------------------------------------
    In this method we assume that all the data set in this objet is valid. @b{So the
    caller must validate all paramater before setting them.}

    @param pi_ListOfTag The tag's list for the current export.
    -----------------------------------------------------------------------------
*/
void HRFExportOptions::SetTagList (HPMAttributeSet pi_ListOfTag)
    {
    // Set all the tags.
    HPMAttributeSet::HPMASiterator TagIterator;

    for (TagIterator = pi_ListOfTag.begin();
         TagIterator != pi_ListOfTag.end(); ++TagIterator)
        {
        SetTag(*TagIterator);
        }
    }

/** -----------------------------------------------------------------------------
    In this method we assume that all the data set in this objet is valid. @b{So the
    caller must validate all parameters before setting them.}

    Add one tag to the tag list for this export.

    @param pi_rpTag The tag to be added in the list.
    -----------------------------------------------------------------------------
*/
void HRFExportOptions::SetTag(const HFCPtr<HPMGenericAttribute>& pi_rpTag)
    {
    bool TagFound = false;
    HPMAttributeSet::HPMASiterator  TagIterator;

    for (TagIterator = m_TagList.begin(); TagIterator != m_TagList.end(); ++TagIterator)
        {
        if (pi_rpTag->SameAttributeAs(**TagIterator))
            TagFound = true;
        }

    if (!TagFound)
        m_TagList.Set(pi_rpTag);
    }

/** -----------------------------------------------------------------------------
Add the metadata container to the page descriptor

Add one metadata container for this export. If the export already contains a
container of the same type as the container to be added, it will be replaced.

@param pi_rpMDContainer The metadata container to be added in the list.
-----------------------------------------------------------------------------
*/
void HRFExportOptions::SetMetaDataContainer(const HFCPtr<HMDMetaDataContainer>& pi_rpMDContainer)
    {
    m_ListOfMetaDataContainer.SetMetaDataContainer(pi_rpMDContainer);
    }

/** -----------------------------------------------------------------------------
    In this method we assume that all the data set in this objet is valid. @b{So the
    caller must validate all paramater before setting them.}

    @param pi_rpCodec The codec's pointer for the current export. This pointer
                      can contain extra information concerning codecs configuration
    -----------------------------------------------------------------------------
*/
void HRFExportOptions::SetCodecSample(const HFCPtr<HCDCodec>& pi_rpCodec)
    {
    HPRECONDITION(pi_rpCodec != 0);
    m_pCodec = pi_rpCodec;
    }

/** -----------------------------------------------------------------------------
    In this method we assume that all the data set in this objet is valid. @b{So the
    caller must validate all paramater before setting them.}

    @param pi_CompressionQuality The compression quality for the current export.
    -----------------------------------------------------------------------------
*/
void HRFExportOptions::SetCompressionQuality (uint32_t pi_CompressionQuality)
    {
    m_CompressionQuality = pi_CompressionQuality;
    }

/** -----------------------------------------------------------------------------
In this method we assume that all the data set in this objet is valid. @b{So the
caller must validate all paramater before setting them.}

@param pi_CompressionRatio The compression ratio for the current export.
-----------------------------------------------------------------------------
*/
void HRFExportOptions::SetCompressionRatio (uint32_t pi_CompressionRatio)
    {
    m_CompressionRatio = pi_CompressionRatio;
    }

/** -----------------------------------------------------------------------------
    In this method we assume that all the data set in this objet is valid. @b{So the
    caller must validate all paramater before setting them.}

    @param pi_rpSubResCodec The codec class key for sub-resolution for the current export.
    -----------------------------------------------------------------------------
*/
void HRFExportOptions::SetSubResCodecSample(const HFCPtr<HCDCodec>& pi_rpSubResCodec)
    {
    HPRECONDITION(pi_rpSubResCodec != 0);
    m_pSubResCodec = pi_rpSubResCodec;
    }

/** -----------------------------------------------------------------------------
    In this method we assume that all the data set in this objet is valid. @b{So the
    caller must validate all paramater before setting them.}

    @param pi_SubResCompressionQuality The compression quality for the sub-resolution
    -----------------------------------------------------------------------------
*/
void HRFExportOptions::SetSubResCompressionQuality (uint32_t pi_SubResCompressionQuality)
    {
    m_SubResCompressionQuality = pi_SubResCompressionQuality;
    }

/** -----------------------------------------------------------------------------
In this method we assume that all the data set in this objet is valid. @b{So the
caller must validate all paramater before setting them.}

@param pi_SubResCompressionRatio The compression ratio for the sub-resolution
-----------------------------------------------------------------------------
*/
void HRFExportOptions::SetSubResCompressionRatio (uint32_t pi_SubResCompressionRatio)
    {
    m_SubResCompressionRatio = pi_SubResCompressionRatio;
    }


/** -----------------------------------------------------------------------------
    In this method we assume that all the data set in this objet is valid. @b{So the
    caller must validate all paramater before setting them.}

    @param pi_Resample Booleen value that indicates if resampling must be applied
                       for this export. If true, resampling will be applied.
    -----------------------------------------------------------------------------
*/
void HRFExportOptions::SetResample(bool pi_Resample)
    {
    m_Resample = pi_Resample;
    }

/** -----------------------------------------------------------------------------
    In this method we assume that all the data set in this objet is valid. @b{So the
    caller must validate all paramater before setting them.}

    @param pi_ImageWidth The image width for the current export.
    -----------------------------------------------------------------------------
*/
void HRFExportOptions::SetImageWidth (uint32_t pi_ImageWidth)
    {
    m_ImageWidth = pi_ImageWidth;
    }

/** -----------------------------------------------------------------------------
    In this method we assume that all the data set in this objet is valid. @b{So the
    caller must validate all paramater before setting them.}

    @param pi_ImageHeight The image heigth for the current export.
    -----------------------------------------------------------------------------
*/
void HRFExportOptions::SetImageHeight (uint32_t pi_ImageHeight)
    {
    m_ImageHeight = pi_ImageHeight;
    }

/** -----------------------------------------------------------------------------
    In this method we assume that all the data set in this objet is valid. @b{So the
    caller must validate all paramater before setting them.}

    @param pi_ScaleX The scale width for the current export.
    -----------------------------------------------------------------------------
*/
void HRFExportOptions::SetScaleFactorX (double pi_ScaleX)
    {
    m_ScaleFactorX = pi_ScaleX;
    }

/** -----------------------------------------------------------------------------
    In this method we assume that all the data set in this objet is valid. @b{So the
    caller must validate all paramater before setting them.}

    @param pi_ScaleY The scale heigth for the current export.
    -----------------------------------------------------------------------------
*/
void HRFExportOptions::SetScaleFactorY (double pi_ScaleY)
    {
    m_ScaleFactorY = pi_ScaleY;
    }

/** -----------------------------------------------------------------------------
    In this method we assume that all the data set in this objet is valid. @b{So the
    caller must validate all paramater before setting them.}

    @param pi_BlockType The block type for the current export.
    -----------------------------------------------------------------------------
*/
void HRFExportOptions::SetBlockType (HRFBlockType pi_BlockType)
    {
    m_BlockType = pi_BlockType;
    }

/** -----------------------------------------------------------------------------
    In this method we assume that all the data set in this objet is valid. @b{So the
    caller must validate all paramater before setting them.}

    @param pi_BlockWidth The block width for the current export.
    -----------------------------------------------------------------------------
*/
void HRFExportOptions::SetBlockWidth (uint32_t pi_BlockWidth)
    {
    m_BlockWidth = pi_BlockWidth;
    }

/** -----------------------------------------------------------------------------
    In this method we assume that all the data set in this objet is valid. @b{So the
    caller must validate all paramater before setting them.}

    @param pi_BlockHeight The block heigth for the current export.
    -----------------------------------------------------------------------------
*/
void HRFExportOptions::SetBlockHeight (uint32_t pi_BlockHeight)
    {
    m_BlockHeight = pi_BlockHeight;
    }


/** -----------------------------------------------------------------------------
    In this method we assume that all the data set in this objet is valid. @b{So the
    caller must validate all paramater before setting them.}

    @param pi_SubResBlockType The block type for sub-resolution for the current export.
    -----------------------------------------------------------------------------
*/
void HRFExportOptions::SetSubResBlockType (HRFBlockType pi_SubResBlockType)
    {
    m_SubResBlockType = pi_SubResBlockType;
    }

/** -----------------------------------------------------------------------------
    In this method we assume that all the data set in this objet is valid. @b{So the
    caller must validate all paramater before setting them.}

    @param pi_SubResBlockWidth The block width for sub-resolution for the current export.
    -----------------------------------------------------------------------------
*/
void HRFExportOptions::SetSubResBlockWidth (uint32_t pi_SubResBlockWidth)
    {
    m_SubResBlockWidth = pi_SubResBlockWidth;
    }

/** -----------------------------------------------------------------------------
    In this method we assume that all the data set in this objet is valid. @b{So the
    caller must validate all paramater before setting them.}

    @param pi_SubResBlockHeight The block heigth for sub-resolution for the current export.
    -----------------------------------------------------------------------------
*/
void HRFExportOptions::SetSubResBlockHeight (uint32_t pi_SubResBlockHeight)
    {
    m_SubResBlockHeight = pi_SubResBlockHeight;
    }

/** -----------------------------------------------------------------------------
    In this method we assume that all the data set in this objet is valid. @b{So the
    caller must validate all paramater before setting them.}

    @param pi_EncodingType The encoding type for this export. Encoding can be : Multiresolution,
                           Standard(No sub-resolution is writed to file) or Progressive.
    -----------------------------------------------------------------------------
*/
void HRFExportOptions::SetEncoding (HRFEncodingType pi_EncodingType)
    {
    m_Encoding = pi_EncodingType;
    }

/** -----------------------------------------------------------------------------
    In this method we assume that all the data set in this objet is valid. @b{So the
    caller must validate all paramater before setting them.}

    @param pi_GeoreferenceFormat The georereference format for this export. This is
                                 the way that tranformation should be save in the file.
                                 (In HGR, in World File ...)
    -----------------------------------------------------------------------------
*/
void HRFExportOptions::SetGeoreferenceFormat (HRFGeoreferenceFormat pi_GeoreferenceFormat)
    {
    m_GeoreferenceFormat = pi_GeoreferenceFormat;
    }

/** -----------------------------------------------------------------------------
    In this method we assume that all the data set in this objet is valid. @b{So the
    caller must validate all paramater before setting them.}

    @param pi_Geocoding The geocoding for this export. The geocoding can be NULL if
                        no geocoding is specified.
    -----------------------------------------------------------------------------
*/
void HRFExportOptions::SetRasterFileGeocoding (RasterFileGeocoding& pi_pGeocoding)
    {
    m_pGeocoding = &pi_pGeocoding;
    }

