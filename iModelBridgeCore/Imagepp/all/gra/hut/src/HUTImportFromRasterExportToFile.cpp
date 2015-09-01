//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hut/src/HUTImportFromRasterExportToFile.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class: HUTImportFromRasterExportToFile
// ----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>




#include <Imagepp/all/h/HUTImportFromRasterExportToFile.h>
#include <Imagepp/all/h/HRFRasterFileFactory.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HRFiTiffCacheFileCreator.h>
#include <Imagepp/all/h/HUTExportToFile.h>
#include <Imagepp/all/h/HRFRasterFileBlockAdapter.h>
#include <Imagepp/all/h/HRFRasterFileCache.h>
#include <Imagepp/all/h/HRAStoredRaster.h>
#include <Imagepp/all/h/HRAReferenceToRaster.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HIMStoredRasterEquivalentTransfo.h>
#include <Imagepp/all/h/HFCGrid.h>
#include <Imagepp/all/h/HRPPixelTypeFactory.h>
#include <Imagepp/all/h/HCPGeoTiffKeys.h>

//-----------------------------------------------------------------------------
// Constructor
// Creation and destruction interface
//-----------------------------------------------------------------------------
HUTImportFromRasterExportToFile::HUTImportFromRasterExportToFile (
    HFCPtr<HRARaster>                pi_pRaster,
    const HVEShape&                  pi_rClipShape,
    const HFCPtr<HGF2DWorldCluster>& pi_pWorldCluster)
    : HRFImportExport(pi_pWorldCluster),
      m_Resampling(HGSResampling::AVERAGE)
    {
    // The shape provided may not be empty
    HPRECONDITION(!pi_rClipShape.IsEmpty());

    // A raster must be provided
    HPRECONDITION(pi_pRaster != 0);

    // A cluster must be provided
    HPRECONDITION(pi_pWorldCluster != 0);

    m_pRaster               = pi_pRaster;
    m_ClipShape             = pi_rClipShape;
    m_pWorldCluster         = pi_pWorldCluster;
    m_NbColorsIfIndexed     = 0;    // MaxEntries by default
    m_BlendAlpha            = false;
    m_HasRepPalSamplingOptions = false;

    // We suppose that source has a model
    m_SourceHasTransfo      = true;

    // Compute output file size in pixel
    HVEShape ClipShape(m_ClipShape);

    HIMStoredRasterEquivalentTransfo SRETransfo(m_pRaster);
    if (SRETransfo.EquivalentTransfoCanBeComputed())
        {
        HVEShape ClipShapeOrig(m_ClipShape);
        SRETransfo.TransformLogicalShapeIntoPhysical(ClipShapeOrig);

        HFCGrid OriginalGrid(0.0,
                             0.0,
                             ClipShapeOrig.GetExtent().GetWidth(),
                             ClipShapeOrig.GetExtent().GetHeight());

        // Set Original size in HRFImportExport
        CHECK_HSINT64_TO_HDOUBLE_CONV(OriginalGrid.GetWidth())
        CHECK_HSINT64_TO_HDOUBLE_CONV(OriginalGrid.GetHeight())

        m_OriginalSize.SetX((double)OriginalGrid.GetWidth());
        m_OriginalSize.SetY((double)OriginalGrid.GetHeight());

        HFCPtr<HGF2DTransfoModel> pBaseToImageModel = SRETransfo.GetLogicalCoordSys()->GetTransfoModelTo(SRETransfo.GetPhysicalCoordSys());

        if (pBaseToImageModel != 0)
            {
            // Simplify the model
            HFCPtr<HGF2DTransfoModel> pSimplifiedModel = pBaseToImageModel->CreateSimplifiedModel();

            if (pSimplifiedModel != 0)
                m_SourceHasTransfo = (pSimplifiedModel->GetClassID() != HGF2DIdentity::CLASS_ID);
            else
                m_SourceHasTransfo = (pBaseToImageModel->GetClassID() != HGF2DIdentity::CLASS_ID);
            }
        else
            m_SourceHasTransfo = false;
        }
    else
        {
        // Here we have no other choice than to resample.
        m_OriginalSize.SetX(0.0);
        m_OriginalSize.SetY(0.0);
        SetResampleIsForce(true);

        // We have no infomation so we suppose that we have a model.
        m_SourceHasTransfo = true;
        }

    // Calculate the resample size and scale factor.
    ClipShape.ChangeCoordSys(m_pWorldCluster->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD));
    HGF2DExtent TmpExtentMin;
    HGF2DExtent TmpExtentMax;
    m_pRaster->GetPixelSizeRange(TmpExtentMin, TmpExtentMax);

    // The minimum pixel size must be defined and its width and height must be greater than 0.0
    HASSERT(TmpExtentMin.IsDefined());
    HASSERT(TmpExtentMin.GetWidth() != 0.0);
    HASSERT(TmpExtentMin.GetHeight() != 0.0);

    HGF2DExtent PixelSize(TmpExtentMin.CalculateApproxExtentIn(m_pWorldCluster->GetCoordSysReference(HGF2DWorld_UNKNOWNWORLD)));

    // PixelSize Area.
    m_DefaultResampleScaleFactorX = MIN(PixelSize.GetWidth(), PixelSize.GetHeight());

    // The pixel size may not be 0.0 (exact compare)
    HASSERT(m_DefaultResampleScaleFactorX != 0.0);

    // ScaleFactor
    m_DefaultResampleScaleFactorX =  1.0 / m_DefaultResampleScaleFactorX;
    m_DefaultResampleScaleFactorY =  m_DefaultResampleScaleFactorX;

    // Exceptionally, we specify a precision. We don't want to create
    // pixels that are not useful. 0.01 is quite arbitrary ;-)
    HFCGrid ResampleGrid(0.0,
                         0.0,
                         ClipShape.GetExtent().GetWidth()  * m_DefaultResampleScaleFactorX,
                         ClipShape.GetExtent().GetHeight() * m_DefaultResampleScaleFactorY,
                         0.01);

    // Set resample size in HRFImportExport.
    CHECK_HSINT64_TO_HDOUBLE_CONV(ResampleGrid.GetWidth())
    CHECK_HSINT64_TO_HDOUBLE_CONV(ResampleGrid.GetHeight())

    m_DefaultResampleSize.SetX((double)ResampleGrid.GetWidth());
    m_DefaultResampleSize.SetY((double)ResampleGrid.GetHeight());

    // Recompute the scale factors to reflect the real dimensions
    // as computed by the ResampleGrid.
    m_DefaultResampleScaleFactorX = ResampleGrid.GetWidth() / ClipShape.GetExtent().GetWidth();
    m_DefaultResampleScaleFactorY = ResampleGrid.GetHeight() / ClipShape.GetExtent().GetHeight();

    SetResample(true);
    SetResampleIsForce(false);

    HASSERT(CountExportFileFormat() > 0);
    SelectExportFileFormatByIndex(0);
    }

//-----------------------------------------------------------------------------
// Destructor
// Creation and destruction interface
//-----------------------------------------------------------------------------
HUTImportFromRasterExportToFile::~HUTImportFromRasterExportToFile()
    {
    }

//-----------------------------------------------------------------------------
// StartExport
// Export interface
//-----------------------------------------------------------------------------
HFCPtr<HRFRasterFile> HUTImportFromRasterExportToFile::StartExport()
    {
    const uint32_t         StripHeight = 256;
    HFCPtr<HRFRasterFile> pDstRasterFile;

    try
        {
        // Create the export file.
        pDstRasterFile = CreateFileFromSelectedValues();

        bool IsCompressedImage = false;

        ValidateUncompressedExportSize(pDstRasterFile, &IsCompressedImage);

        // Get the block acces of the raster file, since if it is random we don't
        // need to add an adapter.
        HRFBlockAccess WriterBlockAccess;
        WriterBlockAccess = pDstRasterFile->GetPageDescriptor(0)->GetResolutionDescriptor(0)->GetWriterBlockAccess();

        // Adapt the destination raster file to srip.
        // The strip adapter is the only one that guarantee that all data can be write
        // correctly without the use of a cache.
        HASSERT(pDstRasterFile->CountPages() == 1);
        HRFRasterFileBlockAdapter::BlockDescriptorMap BlockDescMap;
        HRFRasterFileBlockAdapter::BlockDescriptor    BlockDesc;
        BlockDesc.m_BlockType   = HRFBlockType(HRFBlockType::STRIP);
        BlockDesc.m_BlockWidth  = GetImageWidth();
        BlockDesc.m_BlockHeight = StripHeight;
        BlockDescMap.insert(HRFRasterFileBlockAdapter::BlockDescriptorMap::value_type(0, BlockDesc));
        if (HRFRasterFileBlockAdapter::CanAdapt(pDstRasterFile, BlockDescMap))
            pDstRasterFile = new HRFRasterFileBlockAdapter(pDstRasterFile, BlockDescMap);

        if (WriterBlockAccess == HRFBlockAccess::SEQUENTIAL && pDstRasterFile->GetPageDescriptor(0)->CountResolutions() > 1)
            pDstRasterFile  = new HRFRasterFileCache(pDstRasterFile , HRFiTiffCacheFileCreator::GetInstance(), true);

        // Export the raster.
        // Set the scale for export.
        HASSERT((!GetResample() && GetScaleFactorX() == 1.0 && GetScaleFactorX() == 1.0) ||
                GetResample());

        HUTExportToFile Export(m_pRaster,
                               pDstRasterFile,
                               m_pWorldCluster,
                               GetScaleFactorX(),
                               GetScaleFactorY(),
                               GetResample());

        Export.SetClipShape(m_ClipShape);
        Export.SetBlendAlpha(m_BlendAlpha);
        Export.SetResamplingMode(m_Resampling);

        if (m_NbColorsIfIndexed != 0)
            Export.SetNumberOfColorDestination(m_NbColorsIfIndexed);

        if (m_UseDestinationPaletteIfIndexed)
            Export.SetUseDestinationPaletteIfIndexed(true);

        if(m_HasRepPalSamplingOptions)
            Export.SetRepresentativePaletteSamplingOptions(m_RepPalSamplingOptions);

        Export.SetExportSizeEstimation(IsCompressedImage);

        Export.Export();
        }
    catch (...)
        {
        //Ensure that never written blocks during the export process
        //won't be considered as empty and written during the save
        //operation occurring at the destruction of the destination raster file.
        if (pDstRasterFile != 0)
            {
            pDstRasterFile->CancelCreate();
            }

        throw;
        }

    return pDstRasterFile;
    }

//-----------------------------------------------------------------------------
// BestMatchSelectedValues
// BestMatch interface
//-----------------------------------------------------------------------------
void HUTImportFromRasterExportToFile::BestMatchSelectedValues()
    {
    // If the source format has a tranfo model and georeference format is in image for the output file
    // and the selected format do not support transfo model, we have to force resample.
    if (m_SourceHasTransfo &&
        (GetSelectedGeoreferenceFormat() == HRFGeoreferenceFormat::GEOREFERENCE_IN_IMAGE) &&
        !m_pSelectedFileFormatCapabilities->HasCapabilityOfType(HRFTransfoModelCapability::CLASS_ID,
                                                                HFC_CREATE_ONLY))
        {
        SetResample(true);
        SetResampleIsForce(true);
        }
    else
        {
        SetResample(true);
        SetResampleIsForce(false);
        }

    SetImageSizeIsLock(false);
    SetScaleFactorIsLock(false);
    SetMaintainAspectRatio(false);

    SetImageWidth ((uint32_t)GetDefaultResampleSize().GetX());
    SetImageHeight((uint32_t)GetDefaultResampleSize().GetY());
    SetScaleFactorX(GetDefaultResampleScaleFactorX());
    SetScaleFactorY(GetDefaultResampleScaleFactorY());

    SelectBestPixelType(m_pRaster->GetPixelType());

    // Best match encoding methode.
    HASSERT(CountEncoding() > 0);
    SelectEncodingByIndex(0);

    // Tag best match
    HPMAttributeSet::HPMASiterator TagIterator;

    // Be sure to add new tag from an empty list.
    ClearTagList();

#ifdef IPP_HPM_ATTRIBUTES_ON_HRA
    // Lock down the attributes
    HPMAttributeSet& rAttributes = m_pRaster->LockAttributes();

    for (TagIterator  = rAttributes.begin();
         TagIterator != rAttributes.end(); TagIterator++)
        {
        HFCPtr<HPMGenericAttribute> pTag = (*TagIterator);

        if (HasTag(*pTag))
            SetTag(pTag);
        }

    // Let go the attributes
    m_pRaster->UnlockAttributes();
#endif
    }


//-----------------------------------------------------------------------------
// SelectBestPixelType
// BestMatch from input pixelType
//-----------------------------------------------------------------------------
void HUTImportFromRasterExportToFile::SelectBestPixelType(const HFCPtr<HRPPixelType>& pImportPixelType)
    {
    bool                SamePixelTypeFound    = false;
    HFCPtr<HRPPixelType> pPixelTypeSameNumberOfBits;
    bool                 SameNumberOfBitsFound = false;

    // check if we have color or grayscale pixel types
    bool ImportPixelTypeIsGray = false;
    if ((pImportPixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::GRAY, 0) != HRPChannelType::FREE) ||
        (pImportPixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::PREMULTIPLIED_GRAY, 0) != HRPChannelType::FREE))
        ImportPixelTypeIsGray = true;

    // Try to find the same PixelType
    for (uint32_t Index = 0; (Index < CountPixelType()) && (!SamePixelTypeFound); Index++)
        {
        HFCPtr<HRFPixelTypeCapability> pPixelTypeCapability;
        pPixelTypeCapability  = ((HFCPtr<HRFPixelTypeCapability>&)m_pListOfValidPixelType->GetCapability(Index));

        if (pPixelTypeCapability->GetPixelTypeClassID() == pImportPixelType->GetClassID())
            {
            SelectPixelType(pImportPixelType->GetClassID());

            if(CountSubResPixelType())
                SelectSubResPixelType(pImportPixelType->GetClassID());

            SamePixelTypeFound = true;
            }
        }

    if (!SamePixelTypeFound)
        {
        HFCPtr<HRPPixelType> pSameChannelOrgPixelType;

        // Try to find the PixelType with the same number and type of channels
        for (uint32_t Index = 0; (Index < CountPixelType()) && (!SamePixelTypeFound); Index++)
            {
            HFCPtr<HRFPixelTypeCapability> pPixelTypeCapability;
            pPixelTypeCapability  = ((HFCPtr<HRFPixelTypeCapability>&)m_pListOfValidPixelType->GetCapability(Index));
            // Create the selected pixel type
            HFCPtr<HRPPixelType> pPixelType = HRPPixelTypeFactory::GetInstance()->Create(pPixelTypeCapability->GetPixelTypeClassID());
            bool                HasPalette       = (pPixelType->CountIndexBits() != 0);
            bool                ImportHasPalette = (pImportPixelType->CountIndexBits() != 0);

            if ((pPixelType->GetChannelOrg().CountChannels() ==
                 pImportPixelType->GetChannelOrg().CountChannels()) &&
                (HasPalette == ImportHasPalette))
                {
                const HRPChannelOrg&  rChannelOrg       = pPixelType->GetChannelOrg();
                const HRPChannelOrg&  rImportChannelOrg = pImportPixelType->GetChannelOrg();
                uint32_t              ChannelInd        = 0;

                for (; ChannelInd < rChannelOrg.CountChannels(); ChannelInd++)
                    {
                    const HRPChannelType* pChannelType(rChannelOrg.
                                                       GetChannelPtr(ChannelInd));
                    const HRPChannelType* pImportChannelType(rImportChannelOrg.
                                                             GetChannelPtr(ChannelInd));

                    if ((pChannelType->GetRole() != pImportChannelType->GetRole()) ||
                        (pChannelType->GetDataType() != pImportChannelType->GetDataType()))
                        {
                        break;
                        }
                    }

                if (ChannelInd == rChannelOrg.CountChannels())
                    {
                    if (pSameChannelOrgPixelType != 0)
                        {
                        // this assert pop in ATP but method can actually handle the case correctly - why assert them?
                        // HASSERT(pPixelType->CountPixelRawDataBits() != pSameChannelOrgPixelType->CountPixelRawDataBits());

                        if (pPixelType->CountPixelRawDataBits() >
                            pSameChannelOrgPixelType->CountPixelRawDataBits())
                            {
                            pSameChannelOrgPixelType = pPixelType;
                            }
                        }
                    else
                        {
                        pSameChannelOrgPixelType = pPixelType;
                        }
                    }
                }
            }

        if (pSameChannelOrgPixelType != 0)
            {
            SelectPixelType(pSameChannelOrgPixelType->GetClassID());

            if(CountSubResPixelType())
                SelectSubResPixelType(pSameChannelOrgPixelType->GetClassID());

            SamePixelTypeFound = true;
            }
        }
    if (!SamePixelTypeFound)
        {
        // Try to find the PixelType with the same number of bits
        for (uint32_t Index = 0; (Index < CountPixelType()) && (!SamePixelTypeFound); Index++)
            {
            HFCPtr<HRFPixelTypeCapability> pPixelTypeCapability;
            pPixelTypeCapability  = ((HFCPtr<HRFPixelTypeCapability>&)m_pListOfValidPixelType->GetCapability(Index));
            // Create the selected pixel type
            HFCPtr<HRPPixelType> pPixelType = HRPPixelTypeFactory::GetInstance()->Create(pPixelTypeCapability->GetPixelTypeClassID());

            if (pPixelType->CountPixelRawDataBits() == pImportPixelType->CountPixelRawDataBits())
                {
                // check if we have color or grayscale pixel types
                // When we have color in source we prefer to select
                // an upper pixeltype in bits to a gray scale pixeltype
                bool PixelTypeIsGray = false;
                if ((pPixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::GRAY, 0) != HRPChannelType::FREE) ||
                    (pPixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::PREMULTIPLIED_GRAY, 0) != HRPChannelType::FREE))
                    PixelTypeIsGray = true;

                if (ImportPixelTypeIsGray == PixelTypeIsGray)
                    {
                    SelectPixelType(pPixelType->GetClassID());
                    if(CountSubResPixelType())
                        SelectSubResPixelType(pPixelType->GetClassID());
                    SamePixelTypeFound = true;
                    }
                else
                    {
                    pPixelTypeSameNumberOfBits = pPixelType;
                    SameNumberOfBitsFound = true;
                    }
                }
            }
        }
    if (!SamePixelTypeFound)
        {
        // Try to find the PixelType over and under in bits
        HFCPtr<HRPPixelType> pPixelTypeOver;
        HFCPtr<HRPPixelType> pPixelTypeUnder;

        // Try to find the near PixelType in bits
        for (uint32_t Index = 0; (Index < CountPixelType()) && (!SamePixelTypeFound); Index++)
            {
            HFCPtr<HRFPixelTypeCapability> pPixelTypeCapability;
            pPixelTypeCapability  = ((HFCPtr<HRFPixelTypeCapability>&)m_pListOfValidPixelType->GetCapability(Index));
            // Create the selected pixel type
            HFCPtr<HRPPixelType> pPixelType = HRPPixelTypeFactory::GetInstance()->Create(pPixelTypeCapability->GetPixelTypeClassID());

            if (pPixelType->CountPixelRawDataBits() > pImportPixelType->CountPixelRawDataBits())
                {
                // Find the pixel type over in bits
                if (pPixelTypeOver == 0)
                    pPixelTypeOver = pPixelType;
                else if (pPixelType->CountPixelRawDataBits() < pPixelTypeOver->CountPixelRawDataBits())
                    pPixelTypeOver = pPixelType;
                }
            else
                {
                // Find the pixel type over in bits
                if (pPixelTypeUnder == 0)
                    pPixelTypeUnder = pPixelType;
                else if (pPixelType->CountPixelRawDataBits() > pPixelTypeUnder->CountPixelRawDataBits())
                    pPixelTypeUnder = pPixelType;
                }
            }
        if (pPixelTypeOver == 0)
            {
            if (SameNumberOfBitsFound)
                {
                // Set the best Pixel Type
                SelectPixelType(pPixelTypeSameNumberOfBits->GetClassID());
                if(CountSubResPixelType())
                    SelectSubResPixelType(pPixelTypeSameNumberOfBits->GetClassID());
                SamePixelTypeFound = true;
                }
            else
                {
                // Set the best Pixel Type
                SelectPixelType(pPixelTypeUnder->GetClassID());
                if(CountSubResPixelType())
                    SelectSubResPixelType(pPixelTypeUnder->GetClassID());
                SamePixelTypeFound = true;
                }
            }
        else
            {
            // check if we have color or grayscale pixel types
            // When we have color in source we prefer to select
            // an upper pixeltype in bits to a gray scale pixeltype
            bool PixelTypeOverIsGray = false;
            if ((pPixelTypeOver->GetChannelOrg().GetChannelIndex(HRPChannelType::GRAY, 0) != HRPChannelType::FREE) ||
                (pPixelTypeOver->GetChannelOrg().GetChannelIndex(HRPChannelType::PREMULTIPLIED_GRAY, 0) != HRPChannelType::FREE))
                PixelTypeOverIsGray = true;

            if ((PixelTypeOverIsGray == ImportPixelTypeIsGray) || (!SameNumberOfBitsFound))
                {
                // Set the best Pixel Type
                SelectPixelType(pPixelTypeOver->GetClassID());
                if(CountSubResPixelType())
                    SelectSubResPixelType(pPixelTypeOver->GetClassID());
                SamePixelTypeFound = true;
                }
            else
                {
                // Set the best Pixel Type
                SelectPixelType(pPixelTypeSameNumberOfBits->GetClassID());
                if(CountSubResPixelType())
                    SelectSubResPixelType(pPixelTypeSameNumberOfBits->GetClassID());
                SamePixelTypeFound = true;
                }
            }
        }
    }

//-----------------------------------------------------------------------------
// SetNumberOfColorDestination
//-----------------------------------------------------------------------------
void HUTImportFromRasterExportToFile::SetNumberOfColorDestination(uint32_t pi_NbColors)
    {
    m_NbColorsIfIndexed = pi_NbColors;
    }

//-----------------------------------------------------------------------------
// GetNumberOfColorDestination
//-----------------------------------------------------------------------------
uint32_t HUTImportFromRasterExportToFile::GetNumberOfColorDestination() const
    {
    return m_NbColorsIfIndexed;
    }


//-----------------------------------------------------------------------------
// SetBlendAlpha
//-----------------------------------------------------------------------------
void HUTImportFromRasterExportToFile::SetBlendAlpha(bool pi_BlendAlpha)
    {
    m_BlendAlpha = pi_BlendAlpha;
    }

//-----------------------------------------------------------------------------
// GetBlendAlpha
//-----------------------------------------------------------------------------
bool HUTImportFromRasterExportToFile::GetBlendAlpha() const
    {
    return m_BlendAlpha;
    }

//-----------------------------------------------------------------------------
// GetRepresentativePaletteSamplingOptions
//-----------------------------------------------------------------------------
HRASamplingOptions HUTImportFromRasterExportToFile::GetRepresentativePaletteSamplingOptions() const
    {
    return m_RepPalSamplingOptions;
    }

//-----------------------------------------------------------------------------
// SetRepresentativePaletteSamplingOptions
//-----------------------------------------------------------------------------
void HUTImportFromRasterExportToFile::SetRepresentativePaletteSamplingOptions(const HRASamplingOptions& pi_rRepPalSamplingOptions)
    {
    m_HasRepPalSamplingOptions = true;
    m_RepPalSamplingOptions = pi_rRepPalSamplingOptions;
    }

//-----------------------------------------------------------------------------
// public
// SetResamplingMode
//-----------------------------------------------------------------------------
void HUTImportFromRasterExportToFile::SetResamplingMode(const HGSResampling& pi_rResampling)
    {
    m_Resampling = pi_rResampling;
    }

//-----------------------------------------------------------------------------
// public
// GetResamplingMode
//-----------------------------------------------------------------------------
const HGSResampling& HUTImportFromRasterExportToFile::GetResamplingMode() const
    {
    return m_Resampling;
    }


