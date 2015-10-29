//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFPageDescriptor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class HRFPageDescriptor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRFThumbnail.h>
#include <Imagepp/all/h/HRFResolutionDescriptor.h>
#include <Imagepp/all/h/HRFPageDescriptor.h>
#include <Imagepp/all/h/HGFResolutionDescriptor.h>
#include <Imagepp/all/h/HRFCapability.h>
#include <Imagepp/all/h/HCPGeotiffKeys.h>
#include <Imagepp/all/h/HGF2DStretch.h>



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
RasterFileGeocodingPtr RasterFileGeocoding::Create()
    {
    return new RasterFileGeocoding();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
RasterFileGeocodingPtr RasterFileGeocoding::Create(IRasterBaseGcsP pi_pGeocoding)
    {
    return new RasterFileGeocoding(pi_pGeocoding);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
RasterFileGeocodingPtr RasterFileGeocoding::Create(HCPGeoTiffKeys const* pi_pGeokeys)
    {
    return new RasterFileGeocoding(pi_pGeokeys);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
RasterFileGeocoding::RasterFileGeocoding(IRasterBaseGcsP pi_pGeocoding)
:m_isGeotiffKeysCreated(false), 
m_pGeocoding(pi_pGeocoding)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
RasterFileGeocoding::RasterFileGeocoding()
:m_isGeotiffKeysCreated(false)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
RasterFileGeocoding::RasterFileGeocoding(HCPGeoTiffKeys const* pi_pGeokeys):m_pGeoTiffKeys(pi_pGeokeys->Clone()),m_isGeotiffKeysCreated(true)
    {
    m_pGeocoding = HRFGeoCoordinateProvider::CreateRasterGcsFromGeoTiffKeys(NULL, NULL, *m_pGeoTiffKeys);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
RasterFileGeocoding::RasterFileGeocoding(const RasterFileGeocoding& object):m_isGeotiffKeysCreated(object.m_isGeotiffKeysCreated)
    {
    if (object.m_pGeoTiffKeys!=NULL)
        m_pGeoTiffKeys = object.m_pGeoTiffKeys->Clone();
    if (object.m_pGeocoding !=NULL)
        m_pGeocoding = object.m_pGeocoding->Clone();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
RasterFileGeocodingPtr RasterFileGeocoding::Clone() const
    {
    return new RasterFileGeocoding(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
IRasterBaseGcsCP RasterFileGeocoding::GetGeocodingCP() const
    {
    return m_pGeocoding.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HCPGeoTiffKeys const& RasterFileGeocoding::GetGeoTiffKeys() const
    {
    //Optimization: We extract the geotiff keys only when requested
    if (!m_isGeotiffKeysCreated)
        {
        m_pGeoTiffKeys  = new HCPGeoTiffKeys();
        if (m_pGeocoding!=NULL)
            {
            m_isGeotiffKeysCreated=true;
            m_pGeocoding->GetGeoTiffKeys(m_pGeoTiffKeys);
#if 0
            //Sanity check - can we recreate the same geocoding from geotiff keys extracted?
            RasterFileGeocodingPtr ptemp(Create(m_pGeoTiffKeys));
            BeAssert(m_pGeocoding->IsEquivalent(*(ptemp->GetGeocodingCP())));
#endif
            }
        }

    //Cannot be NULL
    return *m_pGeoTiffKeys;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HFCPtr<HGF2DTransfoModel> RasterFileGeocoding::TranslateToMeter
(
const HFCPtr<HGF2DTransfoModel>& pi_pModel,
double                           pi_FactorModelToMeter,
bool                             pi_ProjectedCSTypeDefinedWithProjLinearUnitsInterpretation,
bool*                            po_pDefaultUnitWasFound
) const
    {
    HPRECONDITION(pi_pModel != 0);
    HFCPtr<HGF2DTransfoModel> pTransfo = pi_pModel;

    double effectiveFactorModelToMeter = pi_FactorModelToMeter;
    double unitsfromMeters;
    bool   isUnitWasFound(HRFGeoCoordinateProvider::GetUnitsFromMeters(unitsfromMeters, GetGeoTiffKeys(), pi_ProjectedCSTypeDefinedWithProjLinearUnitsInterpretation));
    if (isUnitWasFound)
        effectiveFactorModelToMeter = 1.0 / unitsfromMeters;

    if (po_pDefaultUnitWasFound != NULL)
        {
        *po_pDefaultUnitWasFound = isUnitWasFound;
        }

    // Apply to Matrix
    if (effectiveFactorModelToMeter != 1.0)
        {
        HFCPtr<HGF2DStretch> pScaleModel = new HGF2DStretch();

        pScaleModel->SetXScaling(effectiveFactorModelToMeter);
        pScaleModel->SetYScaling(effectiveFactorModelToMeter);

        pTransfo = pTransfo->ComposeInverseWithDirectOf(*pScaleModel);
        }

    return pTransfo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
HFCPtr<HGF2DTransfoModel> RasterFileGeocoding::TranslateFromMeter
(
const HFCPtr<HGF2DTransfoModel>& pi_pModel,
bool                            pi_ProjectedCSTypeDefinedWithProjLinearUnitsInterpretation,
bool*                           po_pDefaultUnitWasFound
) const
    {
    HPRECONDITION(pi_pModel != 0);

    double FactorModelToMeter = 1.0;
    double unitsfromMeters;
    bool   isUnitWasFound(HRFGeoCoordinateProvider::GetUnitsFromMeters(unitsfromMeters, GetGeoTiffKeys(), pi_ProjectedCSTypeDefinedWithProjLinearUnitsInterpretation));
    if (isUnitWasFound)
        FactorModelToMeter = 1.0 / unitsfromMeters;


    if (po_pDefaultUnitWasFound != NULL)
        {
        *po_pDefaultUnitWasFound = isUnitWasFound;
        }


    HFCPtr<HGF2DTransfoModel> pTransfo = pi_pModel;


    // Apply inverse factor to Matrix
    if (FactorModelToMeter != 1.0)
        {
        HASSERT(FactorModelToMeter != 0.0);
        HFCPtr<HGF2DStretch> pScaleModel = new HGF2DStretch();
        pScaleModel->SetXScaling(1.0 / FactorModelToMeter);
        pScaleModel->SetYScaling(1.0 / FactorModelToMeter);

        pTransfo = pTransfo->ComposeInverseWithDirectOf(*pScaleModel);
        }

    return pTransfo;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                                   Eric.Paquet         10/2015
//-----------------------------------------------------------------------------------------
bool RasterFileGeocoding::IsValid() const
    {
    if (m_pGeocoding != nullptr || m_pGeoTiffKeys != nullptr)
        return true;

    return false;
    }

/** -----------------------------------------------------------------------------
    This constructor should @b{not be used}.
    -----------------------------------------------------------------------------
*/
HRFPageDescriptor::HRFPageDescriptor(bool pi_EmptyPage)
    : m_EmptyPage(pi_EmptyPage)
    {
    }



/** -----------------------------------------------------------------------------
    This constructor create a new page descriptor with only one resolution and other
    specific option.

    @param pi_AccessMode
           The access mode of the current page. This access mode will be used to valide
           selected option within the capabilities.
    @param pi_rpPageCapabilities
           This argument defines the structure of page. Within the capabilities are
           contained information such has the compression type, pixel type, organization
           and so on. These capabilities are used to test the integrity of new page descriptor.
    @param pi_rResolutionDescriptors
           The list of resolution descriptors. If more than one resolution is provided then the
           file format must indicate support for multiple resolution and the resolutions descriptor
           information must agree with HRFMultiResolutionCapability parameters.
    @param pi_pRepresentativePalette
           The representative palette to assign to the new page. If 0 is given,
           then no representative palette is assigned. This palette is useful because we
           have a palette per resolution but one representative for the image.
    @param pi_pHistogram
           The histogram to assign to the new page. If 0 is given then no histogram
           is assigned. Use to keep the Histogram of HMR format.
    @param pi_pThumbnail
           The thumbnail of the page. If 0 then no thumbnail is assigned. If a
           thumbnail is given then the capabilities must indicate support for thumbnails.
    @param pi_pClipShape
           The clip shape assigned to the page. If a shape is given then the capabilities
           must indicate support for shapes. If 0 is given, then no shape is assigned to the page.
    @param pi_pTransfoModel
           The transformation model assigned to the page. The type of the transformation model
           must be supported as indicated in the capabilities. If 0 is given then no transformation
           model will be assigned, resulting in a non-positioned image. If a transformation model
           orientation is provided, then the model applies to this orientation. If the orientation
           indicated is not supported by the format, then the transformation model may be changed
           to concur with capabilities.
    @param pi_pFilters
           A list of filter to keep color corection. Filters must be supported as indicated in
           the capabilities and the type of the filter must be known. If 0 is given then no
           filter is assigned to the page.
    @param pi_pTags
           The list of meta-data information. All tags must be known and supported as indicated
           in the capabilities. If 0 is given, then no tag is assigned to the image. If mandatory
           tags exist for the format, then the raster file will provided a judiciously chosen
           default value upon storage
    @param pi_Duration
           The duration of the image in milliseconds. Durations are only valid for video type
           of rasters. If the format only supports still images, then 0 must be provided.Any
           other value implies that the raster file format supports multiple pages and animations.
    @param pi_Resizable
           true if the page can be change size, false otherwise
    @param pi_UnlimitedResolution
           The HRFRasterFile support an unlimited number of resolution.
    @param pi_MinWidth
           The minimum width for an unlimited resolution raster.
    @param pi_MinHeight
           The minimum height for an unlimited resolution raster.
    @param pi_MaxWidth
           The maximum width for an unlimited resolution raster.
    @param pi_MaxHeight
           The maximum height for an unlimited resolution raster.
    @end


    @h3{Word Related Documentation:}
    @list{<a href = "..\..\Image++\all\gra\hrf\HRFPageDescriptor.doc">Page Descriptor documentation</a>}
    @list{<a href = "..\..\Image++\all\gra\hrf\HRF.doc">HRF Tutorial </a>}
    @end
    -----------------------------------------------------------------------------
*/
HRFPageDescriptor::HRFPageDescriptor(HFCAccessMode                            pi_AccessMode,
                                     const HFCPtr<HRFRasterFileCapabilities>& pi_rpPageCapabilities,
                                     const HFCPtr<HRFResolutionDescriptor>&   pi_rpResolutionDescriptor,
                                     const HRPPixelPalette*                   pi_pRepresentativePalette,
                                     const HRPHistogram*                      pi_pHistogram,
                                     const HRFThumbnail*                      pi_pThumbnail,
                                     const HRFClipShape*                      pi_pClipShape,
                                     const HGF2DTransfoModel*                 pi_pTransfoModel,
                                     const HRPFilter*                         pi_pFilters,
                                     const HPMAttributeSet*                   pi_pTags,
                                     uint32_t                                 pi_Duration,
                                     bool                                    pi_Resizable,
                                     bool                                    pi_UnlimitedResolution,
                                     uint64_t                                pi_MinWidth,
                                     uint64_t                                pi_MinHeight,
                                     uint64_t                                pi_MaxWidth,
                                     uint64_t                                pi_MaxHeight)
    : m_EmptyPage(false)
    {
    // Validate if the client pass the capabilities
    HPRECONDITION(pi_rpPageCapabilities != 0);
    m_pPageCapabilities = pi_rpPageCapabilities;

    // Page information
    HPRECONDITION(pi_rpResolutionDescriptor != 0);
    m_ListOfResolutionDescriptor.push_back(pi_rpResolutionDescriptor);
    m_pListOfMetaDataContainer = 0;

    m_AccessMode = pi_AccessMode;
    if (pi_UnlimitedResolution)
        {
        // Validation with the capabilities if it's possible to be an unlimited resolution
        HPRECONDITION(m_pPageCapabilities->GetCapabilityOfType(HRFMultiResolutionCapability::CLASS_ID, pi_AccessMode) != 0 &&
                     static_cast<HRFMultiResolutionCapability*>(m_pPageCapabilities->GetCapabilityOfType(HRFMultiResolutionCapability::CLASS_ID,
                              pi_AccessMode).GetPtr())->IsUnlimitedResolution());
        m_UnlimitedResolution = pi_UnlimitedResolution;
        }
    else
        m_UnlimitedResolution = false;

    m_MinWidth  = pi_MinWidth;
    m_MinHeight = pi_MinHeight;
    m_MaxWidth  = pi_MaxWidth;
    m_MaxHeight = pi_MaxHeight;

    // Page Physical information
    if (pi_pRepresentativePalette)
        {
        HPRECONDITION(m_pPageCapabilities->GetCapabilityOfType(HRFRepresentativePaletteCapability::CLASS_ID, m_AccessMode) != 0);
        m_pRepresentativePalette = new HRPPixelPalette(*pi_pRepresentativePalette);
        HASSERT(m_pRepresentativePalette.get() != 0);
        }

    if (pi_pHistogram)
        {
        HPRECONDITION(m_pPageCapabilities->GetCapabilityOfType(HRFHistogramCapability::CLASS_ID, m_AccessMode) != 0);
        m_pHistogram = new HRPHistogram(*pi_pHistogram);
        HASSERT(m_pHistogram != 0);
        }

    if (pi_pThumbnail)
        {
        HPRECONDITION(m_pPageCapabilities->GetCapabilityOfType(HRFThumbnailCapability::CLASS_ID, m_AccessMode) != 0);
        m_pThumbnail = new HRFThumbnail(*pi_pThumbnail);
        HASSERT(m_pThumbnail != 0);
        }

    // Page Logical information
    if (pi_pClipShape)
        {
        HFCPtr<HRFCapability> pCapability = new HRFClipShapeCapability(m_AccessMode, pi_pClipShape->GetCoordinateType());
        HPRECONDITION(m_pPageCapabilities->Supports(pCapability));
        m_pClipShape = new HRFClipShape(*pi_pClipShape);
        HASSERT(m_pClipShape != 0);
        }

    if (pi_pTransfoModel)
        {
        HFCPtr<HRFCapability> pCapability = new HRFTransfoModelCapability(m_AccessMode, pi_pTransfoModel->GetClassID());
        HPRECONDITION(m_pPageCapabilities->Supports(pCapability));
        m_pTransfoModel = pi_pTransfoModel->Clone();
        HASSERT(m_pTransfoModel != 0);

        HPRECONDITION(CountResolutions() > 0);
        m_TransfoModelOrientation = GetResolutionDescriptor(0)->GetScanlineOrientation();
        }

    if (pi_pFilters)
        {
        HFCPtr<HRFCapability> pCapability = new HRFFilterCapability(m_AccessMode, pi_pFilters->GetClassID());
        HPRECONDITION(m_pPageCapabilities->Supports(pCapability));
        m_pFilters = pi_pFilters->Clone();
        HASSERT(m_pFilters != 0);
        }

    // Media type information
    if (pi_Duration == 0)
        HPRECONDITION(m_pPageCapabilities->GetCapabilityOfType(HRFStillImageCapability::CLASS_ID, m_AccessMode) != 0);
    else
        HPRECONDITION(m_pPageCapabilities->GetCapabilityOfType(HRFAnimationCapability::CLASS_ID, m_AccessMode) != 0);
    m_Duration = pi_Duration;

    m_pListOfTag = new HPMAttributeSet();
    if (pi_pTags)
        {
        *m_pListOfTag = *pi_pTags;
        HPRECONDITION(ValidateTagCapabilities());
        }

    HPRECONDITION(!pi_Resizable || m_pPageCapabilities->GetCapabilityOfType(HRFResizableCapability::CLASS_ID, m_AccessMode) != 0);
    m_Resizable = pi_Resizable;

    m_pGeocoding = RasterFileGeocoding::Create();//No geocoding by default, HRFFile MUST set it if supported

    // Flag to know if the specified data has changed
    m_ClipShapeHasChanged               = false;
    m_TransfoModelHasChanged            = false;
    m_FiltersHasChanged                 = false;
    m_RepresentativePaletteHasChanged   = false;
    m_HistogramHasChanged               = false;
    m_ThumbnailHasChanged               = false;
    m_DurationHasChanged                = false;
    m_GeocodingHasChanged               = false;
    m_PageSizeHasChanged                = false;
    }

/** -----------------------------------------------------------------------------
    This constructor create a new page descriptor using a list of resolution and others
    specifics options.

    @param pi_AccessMode
           The access mode of the current page. This access mode will be used to valide
           selected option within the capabilities.
    @param pi_rpPageCapabilities
           This argument defines the structure of page. Within the capabilities are
           contained information such has the compression type, pixel type, organization
           and so on. These capabilities are used to test the integrity of new page descriptor.
    @param pi_rResolutionDescriptors
           The list of resolution descriptors. If more than one resolution is provided then the
           file format must indicate support for multiple resolution and the resolutions descriptor
           information must agree with HRFMultiResolutionCapability parameters.
    @param pi_pRepresentativePalette
           The representative palette to assign to the new page. If 0 is given,
           then no representative palette is assigned. This palette is useful because we
           have a palette per resolution but one representative for the image.
    @param pi_pHistogram
           The histogram to assign to the new page. If 0 is given then no histogram
           is assigned. Use to keep the Histogram of HMR format.
    @param pi_pThumbnail
           The thumbnail of the page. If 0 then no thumbnail is assigned. If a
           thumbnail is given then the capabilities must indicate support for thumbnails.
    @param pi_pClipShape
           The clip shape assigned to the page. If a shape is given then the capabilities
           must indicate support for shapes. If 0 is given, then no shape is assigned to the page.
    @param pi_pTransfoModel
           The transformation model assigned to the page. The type of the transformation model
           must be supported as indicated in the capabilities. If 0 is given then no transformation
           model will be assigned, resulting in a non-positioned image. If a transformation model
           orientation is provided, then the model applies to this orientation. If the orientation
           indicated is not supported by the format, then the transformation model may be changed
           to concur with capabilities.
    @param pi_pFilters
           A list of filter to keep color corection. Filters must be supported as indicated in
           the capabilities and the type of the filter must be known. If 0 is given then no
           filter is assigned to the page.
    @param pi_pTags
           The list of meta-data information. All tags must be known and supported as indicated
           in the capabilities. If 0 is given, then no tag is assigned to the image. If mandatory
           tags exist for the format, then the raster file will provided a judiciously chosen
           default value upon storage
    @param pi_Duration
           The duration of the image in milliseconds. Durations are only valid for video type
           of rasters. If the format only supports still images, then 0 must be provided.Any
           other value implies that the raster file format supports multiple pages and animations.
    @param pi_Resizable
           true if the page can be change size, false otherwise
    @param pi_UnlimitedResolution
           The HRFRasterFile support an unlimited number of resolution.
    @param pi_MinWidth
           The minimum width for an unlimited resolution raster.
    @param pi_MinHeight
           The minimum height for an unlimited resolution raster.
    @param pi_MaxWidth
           The maximum width for an unlimited resolution raster.
    @param pi_MaxHeight
           The maximum height for an unlimited resolution raster.



    @h3{Word Related Documentation:}
    @list{<a href = "..\..\all\gra\hrf\HRFPageDescriptor.doc">Page Descriptor documentation</a>}
    @list{<a href = "..\..\all\gra\hrf\HRF.doc">HRF Tutorial </a>}
    @end
    -----------------------------------------------------------------------------
*/
HRFPageDescriptor::HRFPageDescriptor(HFCAccessMode                            pi_AccessMode,
                                     const HFCPtr<HRFRasterFileCapabilities>& pi_rpPageCapabilities,
                                     const ListOfResolutionDescriptor&        pi_rResolutionDescriptors,
                                     const HRPPixelPalette*                   pi_pRepresentativePalette,
                                     const HRPHistogram*                      pi_pHistogram,
                                     const HRFThumbnail*                      pi_pThumbnail,
                                     const HRFClipShape*                      pi_pClipShape,
                                     const HGF2DTransfoModel*                 pi_pTransfoModel,
                                     const HRPFilter*                         pi_pFilters,
                                     const HPMAttributeSet*                   pi_pTags,
                                     uint32_t                                 pi_Duration,
                                     bool                                    pi_Resizable,
                                     bool                                    pi_UnlimitedResolution,
                                     uint64_t                                pi_MinWidth,
                                     uint64_t                                pi_MinHeight,
                                     uint64_t                                pi_MaxWidth,
                                     uint64_t                                pi_MaxHeight)
    : m_EmptyPage(false)
    {
    // Validate if the client pass the capabilities
    HPRECONDITION(pi_rpPageCapabilities != 0);
    m_pPageCapabilities = pi_rpPageCapabilities;

    // Validation with the capabilities if it's possible to add a resolution
    HPRECONDITION((pi_rResolutionDescriptors.size() == 1 &&
                   m_pPageCapabilities->GetCapabilityOfType(HRFSingleResolutionCapability::CLASS_ID, pi_AccessMode) != 0) ||
                  m_pPageCapabilities->GetCapabilityOfType(HRFMultiResolutionCapability::CLASS_ID, pi_AccessMode) != 0);

    // Page information

    // Resolution information
    m_ListOfResolutionDescriptor = pi_rResolutionDescriptors;
    m_pListOfMetaDataContainer   = 0;
    m_AccessMode                 = pi_AccessMode;

    if (pi_UnlimitedResolution)
        {
        // Validation with the capabilities if it's possible to be an unlimited resolution
        HPRECONDITION(m_pPageCapabilities->GetCapabilityOfType(HRFMultiResolutionCapability::CLASS_ID, pi_AccessMode) != 0 &&
                      static_cast<HRFMultiResolutionCapability*>(m_pPageCapabilities->GetCapabilityOfType(HRFMultiResolutionCapability::CLASS_ID,
                              pi_AccessMode).GetPtr())->IsUnlimitedResolution());
        m_UnlimitedResolution = pi_UnlimitedResolution;
        m_MinWidth  = pi_MinWidth;
        m_MinHeight = pi_MinHeight;
        m_MaxWidth  = pi_MaxWidth;
        m_MaxHeight = pi_MaxHeight;
        }
    else
        m_UnlimitedResolution = false;

    // Page Physical information
    if (pi_pRepresentativePalette)
        {
        HPRECONDITION(m_pPageCapabilities->GetCapabilityOfType(HRFRepresentativePaletteCapability::CLASS_ID, m_AccessMode) != 0);
        m_pRepresentativePalette = new HRPPixelPalette(*pi_pRepresentativePalette);
        HASSERT(m_pRepresentativePalette.get() != 0);
        }

    if (pi_pHistogram)
        {
        HPRECONDITION(m_pPageCapabilities->GetCapabilityOfType(HRFHistogramCapability::CLASS_ID, m_AccessMode) != 0);
        m_pHistogram = new HRPHistogram(*pi_pHistogram);
        HASSERT(m_pHistogram != 0);
        }

    if (pi_pThumbnail)
        {
        HPRECONDITION(m_pPageCapabilities->GetCapabilityOfType(HRFThumbnailCapability::CLASS_ID, m_AccessMode) != 0);
        m_pThumbnail = new HRFThumbnail(*pi_pThumbnail);
        HASSERT(m_pThumbnail != 0);
        }

    // Page Logical information
    if (pi_pClipShape)
        {
        HFCPtr<HRFCapability> pCapability = new HRFClipShapeCapability(m_AccessMode, pi_pClipShape->GetCoordinateType());
        HPRECONDITION(m_pPageCapabilities->Supports(pCapability));
        m_pClipShape = new HRFClipShape(*pi_pClipShape);
        HASSERT(m_pClipShape != 0);
        }

    if (pi_pTransfoModel)
        {
        HFCPtr<HRFCapability> pCapability = new HRFTransfoModelCapability(m_AccessMode, pi_pTransfoModel->GetClassID());
        HPRECONDITION(m_pPageCapabilities->Supports(pCapability));
        m_pTransfoModel = pi_pTransfoModel->Clone();
        HASSERT(m_pTransfoModel != 0);

        HPRECONDITION(CountResolutions() > 0);
        m_TransfoModelOrientation = GetResolutionDescriptor(0)->GetScanlineOrientation();
        }

    if (pi_pFilters)
        {
        HFCPtr<HRFCapability> pCapability = new HRFFilterCapability(m_AccessMode, pi_pFilters->GetClassID());
        HPRECONDITION(m_pPageCapabilities->Supports(pCapability));
        m_pFilters = pi_pFilters->Clone();
        HASSERT(m_pFilters != 0);
        }

    m_pListOfTag = new HPMAttributeSet();
    if (pi_pTags)
        {
        *m_pListOfTag = *pi_pTags;
        HPRECONDITION(ValidateTagCapabilities());
        }

    // Media type information
    if (pi_Duration == 0)
        HPRECONDITION(m_pPageCapabilities->GetCapabilityOfType(HRFStillImageCapability::CLASS_ID, m_AccessMode) != 0);
    else
        HPRECONDITION(m_pPageCapabilities->GetCapabilityOfType(HRFAnimationCapability::CLASS_ID, m_AccessMode) != 0);
    m_Duration = pi_Duration;

    HPRECONDITION(!pi_Resizable || m_pPageCapabilities->GetCapabilityOfType(HRFResizableCapability::CLASS_ID, m_AccessMode) != 0);
    m_Resizable = pi_Resizable;

    m_pGeocoding = RasterFileGeocoding::Create();//No geocoding by default, HRFFile MUST set it if supported


    // Flag to know if the specified data has changed
    m_ClipShapeHasChanged               = false;
    m_TransfoModelHasChanged            = false;
    m_FiltersHasChanged                 = false;
    m_RepresentativePaletteHasChanged   = false;
    m_HistogramHasChanged               = false;
    m_ThumbnailHasChanged               = false;
    m_DurationHasChanged                = false;
    m_GeocodingHasChanged               = false;
    }

/** -----------------------------------------------------------------------------
    This constructor create a new page descriptor without any resolution descriptor.

    @param pi_AccessMode
           The access mode of the current page. This access mode will be used to valide
           selected option within the capabilities.
    @param pi_rpPageCapabilities
           This argument defines the structure of page. Within the capabilities are
           contained information such has the compression type, pixel type, organization
           and so on. These capabilities are used to test the integrity of new page descriptor.
    @param pi_rResolutionDescriptors
           The list of resolution descriptors. If more than one resolution is provided then the
           file format must indicate support for multiple resolution and the resolutions descriptor
           information must agree with HRFMultiResolutionCapability parameters.
    @param pi_pRepresentativePalette
           The representative palette to assign to the new page. If 0 is given,
           then no representative palette is assigned. This palette is useful because we
           have a palette per resolution but one representative for the image.
    @param pi_pHistogram
           The histogram to assign to the new page. If 0 is given then no histogram
           is assigned. Use to keep the Histogram of HMR format.
    @param pi_pThumbnail
           The thumbnail of the page. If 0 then no thumbnail is assigned. If a
           thumbnail is given then the capabilities must indicate support for thumbnails.
    @param pi_pClipShape
           The clip shape assigned to the page. If a shape is given then the capabilities
           must indicate support for shapes. If 0 is given, then no shape is assigned to the page.
    @param pi_pTransfoModel
           The transformation model assigned to the page. The type of the transformation model
           must be supported as indicated in the capabilities. If 0 is given then no transformation
           model will be assigned, resulting in a non-positioned image. If a transformation model
           orientation is provided, then the model applies to this orientation. If the orientation
           indicated is not supported by the format, then the transformation model may be changed
           to concur with capabilities.
    @param pi_pFilters
           A list of filter to keep color corection. Filters must be supported as indicated in
           the capabilities and the type of the filter must be known. If 0 is given then no
           filter is assigned to the page.
    @param pi_pTags
           The list of meta-data information. All tags must be known and supported as indicated
           in the capabilities. If 0 is given, then no tag is assigned to the image. If mandatory
           tags exist for the format, then the raster file will provided a judiciously chosen
           default value upon storage
    @param pi_Duration
           The duration of the image in milliseconds. Durations are only valid for video type
           of rasters. If the format only supports still images, then 0 must be provided.Any
           other value implies that the raster file format supports multiple pages and animations.
    @param pi_Resizable
           true if the page can be change size, false otherwise
    @param pi_UnlimitedResolution
           The HRFRasterFile support an unlimited number of resolution.
    @param pi_MinWidth
           The minimum width for an unlimited resolution raster.
    @param pi_MinHeight
           The minimum height for an unlimited resolution raster.
    @param pi_MaxWidth
           The maximum width for an unlimited resolution raster.
    @param pi_MaxHeight
           The maximum height for an unlimited resolution raster.



    @h3{Word Related Documentation:}
    @list{<a href = "..\..\Image++\all\gra\hrf\HRFPageDescriptor.doc">Page Descriptor documentation</a>}
    @list{<a href = "..\..\Image++\all\gra\hrf\HRF.doc">HRF Tutorial </a>}
    @end
    -----------------------------------------------------------------------------
*/
HRFPageDescriptor::HRFPageDescriptor (HFCAccessMode                            pi_AccessMode,
                                      const HFCPtr<HRFRasterFileCapabilities>& pi_rpPageCapabilities,
                                      const HRPPixelPalette*                   pi_pRepresentativePalette,
                                      const HRPHistogram*                      pi_pHistogram,
                                      const HRFThumbnail*                      pi_pThumbnail,
                                      const HRFClipShape*                      pi_pClipShape,
                                      const HGF2DTransfoModel*                 pi_pTransfoModel,
                                      const HRFScanlineOrientation*            pi_pTransfoModelOrientation,
                                      const HRPFilter*                         pi_pFilters,
                                      const HPMAttributeSet*                   pi_pTags,
                                      uint32_t                                 pi_Duration,
                                      bool                                    pi_Resizable,
                                      bool                                    pi_UnlimitedResolution,
                                      uint64_t                                pi_MinWidth,
                                      uint64_t                                pi_MinHeight,
                                      uint64_t                                pi_MaxWidth,
                                      uint64_t                                pi_MaxHeight)
    : m_EmptyPage(false)
    {
    // Validate if the client pass the capabilities
    HPRECONDITION(pi_rpPageCapabilities != 0);
    m_pPageCapabilities = pi_rpPageCapabilities;
    m_AccessMode        = pi_AccessMode;

    if (pi_UnlimitedResolution)
        {
        // Validation with the capabilities if it's possible to be an unlimited resolution
        HPRECONDITION(m_pPageCapabilities->GetCapabilityOfType(HRFMultiResolutionCapability::CLASS_ID, pi_AccessMode) != 0 &&
                      static_cast<HRFMultiResolutionCapability*>(m_pPageCapabilities->GetCapabilityOfType(HRFMultiResolutionCapability::CLASS_ID,
                              pi_AccessMode).GetPtr())->IsUnlimitedResolution());
        m_UnlimitedResolution = pi_UnlimitedResolution;
        }
    else
        m_UnlimitedResolution = false;

    m_MinWidth  = pi_MinWidth;
    m_MinHeight = pi_MinHeight;
    m_MaxWidth  = pi_MaxWidth;
    m_MaxHeight = pi_MaxHeight;

    // Page Physical information
    if (pi_pRepresentativePalette)
        {
        HPRECONDITION(m_pPageCapabilities->GetCapabilityOfType(HRFRepresentativePaletteCapability::CLASS_ID, m_AccessMode) != 0);
        m_pRepresentativePalette = new HRPPixelPalette(*pi_pRepresentativePalette);
        HASSERT(m_pRepresentativePalette.get() != 0);
        }

    if (pi_pHistogram)
        {
        HPRECONDITION(m_pPageCapabilities->GetCapabilityOfType(HRFHistogramCapability::CLASS_ID, m_AccessMode) != 0);
        m_pHistogram = new HRPHistogram(*pi_pHistogram);
        HASSERT(m_pHistogram != 0);
        }

    if (pi_pThumbnail)
        {
        HPRECONDITION(m_pPageCapabilities->GetCapabilityOfType(HRFThumbnailCapability::CLASS_ID, m_AccessMode) != 0);
        m_pThumbnail = new HRFThumbnail(*pi_pThumbnail);
        HASSERT(m_pThumbnail != 0);
        }

    // Page Logical information
    if (pi_pClipShape)
        {
        HFCPtr<HRFCapability> pCapability = new HRFClipShapeCapability(m_AccessMode, pi_pClipShape->GetCoordinateType());
        HPRECONDITION(m_pPageCapabilities->Supports(pCapability));
        m_pClipShape = new HRFClipShape(*pi_pClipShape);
        HASSERT(m_pClipShape != 0);
        }

    if (pi_pTransfoModel)
        {
        HPRECONDITION(pi_pTransfoModelOrientation != 0);

        HFCPtr<HRFCapability> pCapability = new HRFTransfoModelCapability(m_AccessMode, pi_pTransfoModel->GetClassID());
        HPRECONDITION(m_pPageCapabilities->Supports(pCapability));
        m_pTransfoModel = pi_pTransfoModel->Clone();
        HASSERT(m_pTransfoModel != 0);
        m_TransfoModelOrientation = *pi_pTransfoModelOrientation;
        }

    if (pi_pFilters)
        {
        HFCPtr<HRFCapability> pCapability = new HRFFilterCapability(m_AccessMode, pi_pFilters->GetClassID());
        HPRECONDITION(m_pPageCapabilities->Supports(pCapability));
        m_pFilters = pi_pFilters->Clone();
        HASSERT(m_pFilters != 0);
        }

    m_pListOfTag = new HPMAttributeSet();
    if (pi_pTags)
        {
        // loop through the vector and test each tag in the capabilities.
        HPMAttributeSet::HPMASiterator TagIterator;

        // create the universal tag capability
        HFCPtr<HRFCapability> pTagUniverse(new HRFUniversalTagCapability(m_AccessMode));

        for (TagIterator  = pi_pTags->begin();
             TagIterator != pi_pTags->end(); TagIterator++)
            {
            HFCPtr<HRFCapability> pCapability   = new HRFTagCapability(m_AccessMode,  (*TagIterator));
            HFCPtr<HRFCapability> pCapabilityRO = new HRFTagCapability(HFC_READ_ONLY, (*TagIterator));
            if (!(m_pPageCapabilities->Supports(pCapability) || m_pPageCapabilities->Supports(pTagUniverse)))
                {
                // If we do not support the capability in the given access mode, check if
                // we support it in read only access.
                if (m_pPageCapabilities->Supports(pCapabilityRO))
                    {
                    // If we do, just output a message to say so.

                    // HRFMessage("Tag :" + (*TagIterator)->GetLabel() + " is supported in read only);

                    // Do nothing for now.
                    }
                else
                    {
                    // We do not support this capababilitie.
                    HASSERT(m_pPageCapabilities->Supports(pCapability) || m_pPageCapabilities->Supports(pTagUniverse));
                    }
                }
            }
        *m_pListOfTag = *pi_pTags;
        }

    // Media type information
    if (pi_Duration == 0)
        HPRECONDITION(m_pPageCapabilities->GetCapabilityOfType(HRFStillImageCapability::CLASS_ID, m_AccessMode) != 0);
    else
        HPRECONDITION(m_pPageCapabilities->GetCapabilityOfType(HRFAnimationCapability::CLASS_ID, m_AccessMode) != 0);
    m_Duration = pi_Duration;

    // re-sizable
    HPRECONDITION(!pi_Resizable || m_pPageCapabilities->GetCapabilityOfType(HRFResizableCapability::CLASS_ID, m_AccessMode) != 0);
    m_Resizable = pi_Resizable;

    m_pGeocoding = RasterFileGeocoding::Create();//No geocoding by default, HRFFile MUST set it if supported

    // Flag to know if the specified data has changed
    m_ClipShapeHasChanged               = false;
    m_TransfoModelHasChanged            = false;
    m_FiltersHasChanged                 = false;
    m_RepresentativePaletteHasChanged   = false;
    m_HistogramHasChanged               = false;
    m_ThumbnailHasChanged               = false;
    m_DurationHasChanged                = false;
    m_GeocodingHasChanged               = false;
    m_PageSizeHasChanged                = false;
    }


//-----------------------------------------------------------------------------
// Public
// Destructor
// Page Creation and destruction
//-----------------------------------------------------------------------------
HRFPageDescriptor::~HRFPageDescriptor()
    {
    }

//-----------------------------------------------------------------------------
// public
// Copy Constructor
// Page Creation
//-----------------------------------------------------------------------------
HRFPageDescriptor::HRFPageDescriptor(const HRFPageDescriptor& pi_rObj)
    {
    m_EmptyPage = pi_rObj.m_EmptyPage;

    if (!m_EmptyPage)
        {
        // capabilities
        m_pPageCapabilities = pi_rObj.m_pPageCapabilities;

        // Page information
        m_ListOfResolutionDescriptor = pi_rObj.m_ListOfResolutionDescriptor;

        if (m_pListOfMetaDataContainer != 0)
            {
            m_pListOfMetaDataContainer = new HMDMetaDataContainerList(*pi_rObj.m_pListOfMetaDataContainer);
            }

        // Page Physical information
        if (pi_rObj.m_pRepresentativePalette != 0)
            {
            m_pRepresentativePalette = new HRPPixelPalette(((const HRPPixelPalette&)pi_rObj.m_pRepresentativePalette));
            HASSERT(m_pRepresentativePalette.get() != 0);
            }
        m_pHistogram                = pi_rObj.m_pHistogram;
        m_pThumbnail                = pi_rObj.m_pThumbnail;

        // Page Logical information
        m_pClipShape                = pi_rObj.m_pClipShape;
        m_pTransfoModel             = pi_rObj.m_pTransfoModel;
        m_TransfoModelOrientation   = pi_rObj.m_TransfoModelOrientation;
        if (pi_rObj.m_pFilters != 0)
            {
            m_pFilters = pi_rObj.m_pFilters->Clone();
            HASSERT(m_pFilters != 0);
            }

        m_pListOfTag                = pi_rObj.m_pListOfTag;
        m_ListOfModifiedTag         = pi_rObj.m_ListOfModifiedTag;
        m_pGeocoding                = pi_rObj.m_pGeocoding;

        // Flag to know if the specified data has changed
        m_ClipShapeHasChanged               = pi_rObj.m_ClipShapeHasChanged;
        m_TransfoModelHasChanged            = pi_rObj.m_TransfoModelHasChanged;
        m_FiltersHasChanged                 = pi_rObj.m_FiltersHasChanged;
        m_RepresentativePaletteHasChanged   = pi_rObj.m_RepresentativePaletteHasChanged;
        m_HistogramHasChanged               = pi_rObj.m_HistogramHasChanged;
        m_ThumbnailHasChanged               = pi_rObj.m_ThumbnailHasChanged;
        m_PageSizeHasChanged                = pi_rObj.m_PageSizeHasChanged;
        m_Resizable                         = pi_rObj.m_Resizable;
        m_Duration                          = pi_rObj.m_Duration;
        m_DurationHasChanged                = pi_rObj.m_DurationHasChanged;
        m_GeocodingHasChanged               = pi_rObj.m_GeocodingHasChanged;
        m_AccessMode                        = pi_rObj.m_AccessMode;
        m_UnlimitedResolution               = pi_rObj.m_UnlimitedResolution;
        m_MinWidth                          = pi_rObj.m_MinWidth;
        m_MinHeight                         = pi_rObj.m_MinHeight;
        m_MaxWidth                          = pi_rObj.m_MaxWidth;
        m_MaxHeight                         = pi_rObj.m_MaxHeight;
        }
    }

//-----------------------------------------------------------------------------
// public
// Copy operator equal
// Page Creation
//-----------------------------------------------------------------------------
HRFPageDescriptor& HRFPageDescriptor::operator=(const HRFPageDescriptor& pi_rObj)
    {
    m_EmptyPage = pi_rObj.m_EmptyPage;
    if (!m_EmptyPage)
        {
        // capabilities
        m_pPageCapabilities = pi_rObj.m_pPageCapabilities;

        // Page information
        m_ListOfResolutionDescriptor = pi_rObj.m_ListOfResolutionDescriptor;
        m_pListOfMetaDataContainer   = new HMDMetaDataContainerList(*pi_rObj.m_pListOfMetaDataContainer);

        // Page Physical information
        m_pRepresentativePalette    = new HRPPixelPalette(((const HRPPixelPalette&)pi_rObj.m_pRepresentativePalette));
        HASSERT(m_pRepresentativePalette.get() != 0);
        m_pHistogram                = pi_rObj.m_pHistogram;
        m_pThumbnail                = pi_rObj.m_pThumbnail;

        // Page Logical information
        m_pClipShape                = pi_rObj.m_pClipShape;
        m_pTransfoModel             = pi_rObj.m_pTransfoModel;
        m_TransfoModelOrientation   = pi_rObj.m_TransfoModelOrientation;
        m_pFilters                  = pi_rObj.m_pFilters->Clone();
        HASSERT(m_pFilters != 0);

        m_pListOfTag                = pi_rObj.m_pListOfTag;
        m_ListOfModifiedTag         = pi_rObj.m_ListOfModifiedTag;
        m_pGeocoding                = pi_rObj.m_pGeocoding;

        // Flag to know if the specified data has changed
        m_ClipShapeHasChanged               = pi_rObj.m_ClipShapeHasChanged;
        m_TransfoModelHasChanged            = pi_rObj.m_TransfoModelHasChanged;
        m_FiltersHasChanged                 = pi_rObj.m_FiltersHasChanged;
        m_RepresentativePaletteHasChanged   = pi_rObj.m_RepresentativePaletteHasChanged;
        m_HistogramHasChanged               = pi_rObj.m_HistogramHasChanged;
        m_ThumbnailHasChanged               = pi_rObj.m_ThumbnailHasChanged;
        m_PageSizeHasChanged                = pi_rObj.m_PageSizeHasChanged;

        m_Duration                          = pi_rObj.m_Duration;
        m_DurationHasChanged                = pi_rObj.m_DurationHasChanged;

        m_GeocodingHasChanged               = pi_rObj.m_GeocodingHasChanged;

        m_AccessMode                        = pi_rObj.m_AccessMode;
        m_Resizable                         = pi_rObj.m_Resizable;
        m_UnlimitedResolution               = pi_rObj.m_UnlimitedResolution;
        m_MinWidth                          = pi_rObj.m_MinWidth;
        m_MinHeight                         = pi_rObj.m_MinHeight;
        m_MaxWidth                          = pi_rObj.m_MaxWidth;
        m_MaxHeight                         = pi_rObj.m_MaxHeight;
        }
    return *this;
    }

/** -----------------------------------------------------------------------------
    This constructor combine two page descriptor into one.

    @param pi_rpPageCapabilities
           This argument defines the structure of page. Within the capabilities are
           contained information such has the compression type, pixel type, organization
           and so on. These capabilities are used to test the integrity of new page descriptor.
    @param pi_rpPriorityPage
           Priority page used to create the new page descriptor.
    @param pi_rpSecondPage
           Second page used to create the new page descriptor.
    @param pi_rResolutionDescriptors
           The list of resolution descriptors. If more than one resolution is provided then the
           file format must indicate support for multiple resolution and the resolutions descriptor
           information must agree with HRFMultiResolutionCapability parameters.


    @see HRFResolutionDescritpor
    @end

    @h3{Word Related Documentation:}
    @list{<a href = "..\..\Image++\all\gra\hrf\HRFPageDescriptor.doc">Capability documentation</a>}
    @list{<a href = "..\..\Image++\all\gra\hrf\HRF.doc">HRF Tutorial </a>}
    @end
    -----------------------------------------------------------------------------
*/
HRFPageDescriptor::HRFPageDescriptor(HFCAccessMode                            pi_AccessMode,
                                     const HFCPtr<HRFRasterFileCapabilities>& pi_rpPageCapabilities,
                                     const HFCPtr<HRFPageDescriptor>&         pi_rpPriorityPage,
                                     const HFCPtr<HRFPageDescriptor>&         pi_rpSecondPage,
                                     const ListOfResolutionDescriptor&        pi_rResolutionDescriptors,
                                     bool                                    pi_UnlimitedResolution)
    {
    // Validate if the client pass the capabilities
    HPRECONDITION(pi_rpPageCapabilities != 0);
    HPRECONDITION(pi_rpPriorityPage != 0);
    HPRECONDITION(!pi_rpPriorityPage->IsEmpty());
    HPRECONDITION(pi_rpSecondPage != 0);
    HPRECONDITION(!pi_rpSecondPage->IsEmpty());

    m_MinHeight = 1;
    m_MinWidth  = 1;
    m_MaxHeight = UINT64_MAX;
    m_MaxWidth  = UINT64_MAX;

    if (pi_rpPriorityPage->IsUnlimitedResolution())
        {
        m_MinHeight = MAX(m_MinHeight, pi_rpPriorityPage->GetMinHeight());
        m_MinWidth  = MAX(m_MinWidth,  pi_rpPriorityPage->GetMinWidth());
        m_MaxHeight = MIN(m_MaxHeight, pi_rpPriorityPage->GetMaxHeight());
        m_MaxWidth  = MIN(m_MaxWidth,  pi_rpPriorityPage->GetMaxWidth());
        }

    if (pi_rpSecondPage->IsUnlimitedResolution())
        {
        m_MinHeight = MAX(m_MinHeight, pi_rpSecondPage->GetMinHeight());
        m_MinWidth  = MAX(m_MinWidth,  pi_rpSecondPage->GetMinWidth());
        m_MaxHeight = MIN(m_MaxHeight, pi_rpSecondPage->GetMaxHeight());
        m_MaxWidth  = MIN(m_MaxWidth,  pi_rpSecondPage->GetMaxWidth());
        }

    m_UnlimitedResolution = pi_rpPriorityPage->IsUnlimitedResolution() || pi_rpSecondPage->IsUnlimitedResolution();

    m_EmptyPage = false;
    m_pPageCapabilities = pi_rpPageCapabilities;

    m_AccessMode = pi_AccessMode;
    if (pi_UnlimitedResolution)
        {
        // Validation with the capabilities if it's possible to be an unlimited resolution
        HPRECONDITION(m_pPageCapabilities->GetCapabilityOfType(HRFMultiResolutionCapability::CLASS_ID, pi_AccessMode) != 0 &&
                      static_cast<HRFMultiResolutionCapability*>(m_pPageCapabilities->GetCapabilityOfType(HRFMultiResolutionCapability::CLASS_ID,
                              pi_AccessMode).GetPtr())->IsUnlimitedResolution());
        m_UnlimitedResolution = pi_UnlimitedResolution;
        }
    else
        m_UnlimitedResolution = false;

    // TranfoModel
    
    RasterFileGeocodingPtr pGeoCoding;
	
	// We first set the geocoding according to availability and priority
    if (pi_rpPriorityPage->m_pGeocoding.IsValid() && pi_rpPriorityPage->m_pGeocoding->IsValid())
        {
        pGeoCoding = pi_rpPriorityPage->m_pGeocoding;
		}
    else if (pi_rpSecondPage->m_pGeocoding.IsValid() && pi_rpSecondPage->m_pGeocoding->IsValid())
        {
        pGeoCoding = pi_rpSecondPage->m_pGeocoding;
        }

    // We select transfo model according to availability and priority
    // We force geocoding to the page imposing its transfo model only if present, this way
    // the page that has a transfo model AND geocoding has higher priority to set geocoding
    // but if page has no geocoding then the other imposes its geocoding.
    // This may result in transfo model coming from one page but geocoding from the other
    // this is done by intent due to the possible presence of a page file to specify geocoding only		
    HFCPtr<HGF2DTransfoModel> pTransfoModel = 0;
    HRFScanlineOrientation TransfoModelOrientation;
    if (pi_rpPriorityPage->HasTransfoModel())
        {
    	if (pi_rpPriorityPage->m_pGeocoding.IsValid() && pi_rpPriorityPage->m_pGeocoding->IsValid())
	        pGeoCoding = pi_rpPriorityPage->m_pGeocoding;
        pTransfoModel = pi_rpPriorityPage->GetTransfoModel();
        TransfoModelOrientation = pi_rpPriorityPage->GetTransfoModelOrientation();
        }
    else if (pi_rpSecondPage->HasTransfoModel())
        {
    	if (pi_rpSecondPage->m_pGeocoding.IsValid() && pi_rpSecondPage->m_pGeocoding->IsValid())		
        	pGeoCoding = pi_rpSecondPage->m_pGeocoding;
        pTransfoModel = pi_rpSecondPage->GetTransfoModel();
        TransfoModelOrientation = pi_rpSecondPage->GetTransfoModelOrientation();
        }

    if (pTransfoModel)
        {
        HFCPtr<HRFCapability> pCapability = new HRFTransfoModelCapability(m_AccessMode, pTransfoModel->GetClassID());
        HPRECONDITION(m_pPageCapabilities->Supports(pCapability));
        m_pTransfoModel = pTransfoModel->Clone();
        HASSERT(m_pTransfoModel != 0);
        m_TransfoModelOrientation = TransfoModelOrientation;
        }

    if(pGeoCoding.IsValid() && pGeoCoding->IsValid())
        {
        HFCPtr<HRFCapability> pCapability = new HRFGeocodingCapability(m_AccessMode);
        HPRECONDITION(m_pPageCapabilities->Supports(pCapability));
        m_pGeocoding = pGeoCoding->Clone();
        HASSERT(m_pGeocoding != 0);        
        }
    else
        {
        m_pGeocoding = RasterFileGeocoding::Create();  // We always need to allocate one
        }

    // ClipShape
    HFCPtr<HRFClipShape> pClipShape = 0;
    if (pi_rpPriorityPage->HasClipShape())
        pClipShape = pi_rpPriorityPage->GetClipShape();
    else if (pi_rpSecondPage->HasClipShape())
        pClipShape = pi_rpSecondPage->GetClipShape();

    if (pClipShape)
        {
        HFCPtr<HRFCapability> pCapability = new HRFClipShapeCapability(m_AccessMode, pClipShape->GetCoordinateType());
        HPRECONDITION(m_pPageCapabilities->Supports(pCapability));
        m_pClipShape = new HRFClipShape(*pClipShape);
        HASSERT(m_pClipShape != 0);
        }

    // Histogram
    HFCPtr<HRPHistogram> pHistogram = 0;
    if (pi_rpPriorityPage->HasHistogram())
        pHistogram = pi_rpPriorityPage->GetHistogram();
    else if (pi_rpSecondPage->HasHistogram())
        pHistogram = pi_rpSecondPage->GetHistogram();
    if (pHistogram)
        {
        HPRECONDITION(m_pPageCapabilities->GetCapabilityOfType(HRFHistogramCapability::CLASS_ID, m_AccessMode) != 0);
        m_pHistogram = new HRPHistogram(*pHistogram);
        HASSERT(m_pHistogram != 0);
        }

    // Media type information
    m_Duration = 0;
    if (m_pPageCapabilities->GetCapabilityOfType(HRFAnimationCapability::CLASS_ID, m_AccessMode) != 0)
        {
        if (pi_rpPriorityPage->GetDuration() > 0)
            pHistogram = pi_rpPriorityPage->GetHistogram();
        else if (pi_rpSecondPage->GetDuration() > 0)
            pHistogram = pi_rpSecondPage->GetHistogram();
        }

    // pThumbnail
    HFCPtr<HRFThumbnail> pThumbnail = 0;
    if (pi_rpPriorityPage->HasThumbnail())
        pThumbnail = pi_rpPriorityPage->GetThumbnail();
    else if (pi_rpSecondPage->HasThumbnail())
        pThumbnail = pi_rpSecondPage->GetThumbnail();

    if (pThumbnail)
        {
        HPRECONDITION(m_pPageCapabilities->GetCapabilityOfType(HRFThumbnailCapability::CLASS_ID, m_AccessMode) != 0);
        m_pThumbnail = new HRFThumbnail(*pThumbnail);
        HASSERT(m_pThumbnail != 0);
        }

    // Filter
    HRPFilter const* pFilter = 0;
    if (pi_rpPriorityPage->HasFilter())
        pFilter = &(pi_rpPriorityPage->GetFilter());
    else if (pi_rpSecondPage->HasFilter())
        pFilter = &(pi_rpSecondPage->GetFilter());

    if (pFilter)
        {
        HFCPtr<HRFCapability> pCapability = new HRFFilterCapability(m_AccessMode, pFilter->GetClassID());
        HPRECONDITION(m_pPageCapabilities->Supports(pCapability));
        m_pFilters = pFilter->Clone();
        HASSERT(m_pFilters != 0);
        }

    // RepresentativePalette
    HRPPixelPalette* pPixelPalette = 0;
    if (pi_rpPriorityPage->HasRepresentativePalette())
        pPixelPalette = (HRPPixelPalette*)&(pi_rpPriorityPage->GetRepresentativePalette());
    else if (pi_rpSecondPage->HasRepresentativePalette())
        pPixelPalette = (HRPPixelPalette*)&(pi_rpSecondPage->GetRepresentativePalette());

    if (pPixelPalette)
        {
        HPRECONDITION(m_pPageCapabilities->GetCapabilityOfType(HRFRepresentativePaletteCapability::CLASS_ID, m_AccessMode) != 0);
        m_pRepresentativePalette = new HRPPixelPalette(*pPixelPalette);
        HASSERT(m_pRepresentativePalette.get() != 0);
        }

    // Metadata
    HFCPtr<HMDMetaDataContainer>     pMDContainer;
    HFCPtr<HMDMetaDataContainerList> pSecondPageMDs(pi_rpSecondPage->GetListOfMetaDataContainer());

    if (pSecondPageMDs != 0)
        {
        m_pListOfMetaDataContainer = new HMDMetaDataContainerList();

        for (unsigned short ContainerInd = 0; ContainerInd < pSecondPageMDs->GetNbContainers(); ContainerInd++)
            {
            pSecondPageMDs->GetMetaDataContainer(ContainerInd, pMDContainer);
            m_pListOfMetaDataContainer->SetMetaDataContainer(pMDContainer);
            }
        }

    HFCPtr<HMDMetaDataContainerList> pPriorityPageMDs(pi_rpPriorityPage->GetListOfMetaDataContainer());

    if (pPriorityPageMDs != 0)
        {
        if (m_pListOfMetaDataContainer == 0)
            {
            m_pListOfMetaDataContainer = new HMDMetaDataContainerList();
            }

        for (unsigned short ContainerInd = 0; ContainerInd < pPriorityPageMDs->GetNbContainers(); ContainerInd++)
            {

            //MST MD HPRECONDITION(m_pPageCapabilities->GetCapabilityOfType(HRFRepresentativePaletteCapability::CLASS_ID, m_AccessMode) != 0);
            pPriorityPageMDs->GetMetaDataContainer(ContainerInd, pMDContainer);
            m_pListOfMetaDataContainer->SetMetaDataContainer(pMDContainer);
            }
        }

    // Tags
    m_pListOfTag = new HPMAttributeSet;
    HPMAttributeSet::HPMASiterator TagIterator;
    HFCPtr<HRFCapability> pReadTagUniverse(new HRFUniversalTagCapability(HFC_READ_ONLY));
    HFCPtr<HRFCapability> pWriteTagUniverse(new HRFUniversalTagCapability(HFC_WRITE_ONLY));
    HFCPtr<HRFCapability> pCreateTagUniverse(new HRFUniversalTagCapability(HFC_CREATE_ONLY));

    for (TagIterator  = pi_rpSecondPage->GetTags().begin();
         TagIterator != pi_rpSecondPage->GetTags().end(); TagIterator++)
        {
        HFCPtr<HRFCapability> pReadCapability  = new HRFTagCapability(HFC_READ_ONLY, (*TagIterator));
        HFCPtr<HRFCapability> pWriteCapability = new HRFTagCapability(HFC_WRITE_ONLY, (*TagIterator));
        HFCPtr<HRFCapability> pCreateCapability = new HRFTagCapability(HFC_CREATE_ONLY, (*TagIterator));

        if (m_pPageCapabilities->Supports(pReadCapability)   ||
            m_pPageCapabilities->Supports(pWriteCapability)  ||
            m_pPageCapabilities->Supports(pCreateCapability) ||
            m_pPageCapabilities->Supports(pReadTagUniverse)  ||
            m_pPageCapabilities->Supports(pWriteTagUniverse) ||
            m_pPageCapabilities->Supports(pWriteTagUniverse))
            {
            m_pListOfTag->Set((*TagIterator));
            }
        }

    for (TagIterator  = pi_rpPriorityPage->GetTags().begin();
         TagIterator != pi_rpPriorityPage->GetTags().end(); TagIterator++)
        {
        HFCPtr<HRFCapability> pReadCapability  = new HRFTagCapability(HFC_READ_ONLY, (*TagIterator));
        HFCPtr<HRFCapability> pWriteCapability = new HRFTagCapability(HFC_WRITE_ONLY, (*TagIterator));
        HFCPtr<HRFCapability> pCreateCapability = new HRFTagCapability(HFC_CREATE_ONLY, (*TagIterator));

        if (m_pPageCapabilities->Supports(pReadCapability)   ||
            m_pPageCapabilities->Supports(pWriteCapability)  ||
            m_pPageCapabilities->Supports(pCreateCapability) ||
            m_pPageCapabilities->Supports(pReadTagUniverse)  ||
            m_pPageCapabilities->Supports(pWriteTagUniverse) ||
            m_pPageCapabilities->Supports(pCreateTagUniverse))
            {
            m_pListOfTag->Set((*TagIterator));
            }
        }

    // Resolution information
    m_ListOfResolutionDescriptor = pi_rResolutionDescriptors;

    // need work
    m_Resizable = false;

    // Flag to know if the specified data has changed
    m_ClipShapeHasChanged               = false;
    m_TransfoModelHasChanged            = false;
    m_FiltersHasChanged                 = false;
    m_RepresentativePaletteHasChanged   = false;
    m_HistogramHasChanged               = false;
    m_ThumbnailHasChanged               = false;
    m_DurationHasChanged                = false;
    m_GeocodingHasChanged               = false;
    m_PageSizeHasChanged                = false;

    }

//-----------------------------------------------------------------------------
// Public
// CountBlocksForAllRes
// Return the total number of blocks for all resolutions of the page
//-----------------------------------------------------------------------------
uint64_t HRFPageDescriptor::CountBlocksForAllRes() const
    {
    uint64_t                       NbBlocks = 0;
    uint32_t                        NbRes = (uint32_t)m_ListOfResolutionDescriptor.size();
    HFCPtr<HRFResolutionDescriptor> pResolutionDesc;

    for (unsigned short ResInd = 0; ResInd < NbRes; ResInd++)
        {
        pResolutionDesc = GetResolutionDescriptor(ResInd);
        NbBlocks += pResolutionDesc->GetBlocksPerHeight() *
                    pResolutionDesc->GetBlocksPerWidth();
        }

    return NbBlocks;
    }

//-----------------------------------------------------------------------------
// Public
// SetDuration
// Set the frame duration in milliseconds
//-----------------------------------------------------------------------------
bool HRFPageDescriptor::SetDuration(uint32_t pi_Duration)
    {
    HPRECONDITION(!m_EmptyPage);

    // Media type information
    HPRECONDITION(pi_Duration == 0 ? (m_pPageCapabilities->GetCapabilityOfType(HRFStillImageCapability::CLASS_ID, HFC_WRITE_ONLY) != 0) :
                                     (m_pPageCapabilities->GetCapabilityOfType(HRFAnimationCapability::CLASS_ID, HFC_WRITE_ONLY) != 0));

    if (pi_Duration != m_Duration)
        {
        m_Duration = pi_Duration;
        m_DurationHasChanged = true;
        }
    return true;
    }

//-----------------------------------------------------------------------------
// Public
// SetClipShape
// Page Logical Shape
//-----------------------------------------------------------------------------
bool HRFPageDescriptor::SetClipShape(const HRFClipShape& pi_rShape)
    {
    HPRECONDITION(!m_EmptyPage);

    // Validation with the capabilities if it's possible to set a logical shape
    HFCPtr<HRFCapability> pShapeCapability = new HRFClipShapeCapability(HFC_WRITE_ONLY, pi_rShape.GetCoordinateType());
    HPRECONDITION(m_pPageCapabilities->Supports(pShapeCapability));

    // Update the flag if the user replace the shape
    m_ClipShapeHasChanged = true;

    // Replace the shape by the specified
    m_pClipShape = new HRFClipShape(pi_rShape);
    HASSERT(m_pClipShape != 0);

    return true;
    }

//-----------------------------------------------------------------------------
// Public
// SetTransfoModel
// Page Transformation Model
//-----------------------------------------------------------------------------
bool HRFPageDescriptor::SetTransfoModel(const HGF2DTransfoModel& pi_rTransfoModel, bool pi_IgnoreCapabilties)
    {
    HPRECONDITION(!m_EmptyPage);

    // Should be true only for internal use only.
    if (!pi_IgnoreCapabilties)
        {
        // Validation with the capabilities if it's possible to set a Transfo Model
        HFCPtr<HRFCapability> pWriteCapability = new HRFTransfoModelCapability(HFC_WRITE_ONLY,  pi_rTransfoModel.GetClassID());
        HFCPtr<HRFCapability> pCreateCapability = new HRFTransfoModelCapability(HFC_CREATE_ONLY, pi_rTransfoModel.GetClassID());

        HPRECONDITION(m_pPageCapabilities->Supports(pWriteCapability) || m_pPageCapabilities->Supports(pCreateCapability));
        }

    // Update the flag if the user replace the model
    m_TransfoModelHasChanged = true;

    // Replace the current model by the specified and flush the old physical
    // CoordSys.
    m_pTransfoModel  = pi_rTransfoModel.Clone();
    HASSERT(m_pTransfoModel != 0);

    return true;
    }


//-----------------------------------------------------------------------------
// Public
// SetFilter
// Page Filtering
//-----------------------------------------------------------------------------
bool HRFPageDescriptor::SetFilter(const HRPFilter& pi_rFilters)
    {
    HPRECONDITION(!m_EmptyPage);

    // Validation with the capabilities if it's possible to set a filter
    HFCPtr<HRFCapability> pCapability = new HRFFilterCapability(HFC_WRITE_ONLY, pi_rFilters.GetClassID());
    HPRECONDITION(m_pPageCapabilities->Supports(pCapability));

    // Update the flag if the user replace the filter
    m_FiltersHasChanged = true;

    // Replace the current filter by the specified
    m_pFilters = pi_rFilters.Clone();
    HASSERT(m_pFilters.get() != 0);

    return true;
    }



//-----------------------------------------------------------------------------
// Public
// GeocodingHasChanged
// Flag to know if the specified data has changed
//-----------------------------------------------------------------------------
bool HRFPageDescriptor::GeocodingHasChanged() const
    {
    return m_GeocodingHasChanged;
    }

//-----------------------------------------------------------------------------
// Public
// GetGeocoding
// Get the page descriptor geocoding
//-----------------------------------------------------------------------------
IRasterBaseGcsCP  HRFPageDescriptor::GetGeocodingCP() const
    {
    BeAssert(m_pGeocoding!=NULL);
    return m_pGeocoding->GetGeocodingCP();
    }

//-----------------------------------------------------------------------------
// Public
// SetGeocoding
// Set the page descriptor geocoding
// Geo coding given can be null or invalid indicating that there is no geo coding
//-----------------------------------------------------------------------------
void HRFPageDescriptor::SetGeocoding(IRasterBaseGcsP pi_pGeocoding)
    {
    m_pGeocoding = RasterFileGeocoding::Create(pi_pGeocoding);
    m_GeocodingHasChanged=true;
    }

//-----------------------------------------------------------------------------
// Public
// SetGeocoding
// Set the page descriptor geocoding
// Geo coding given can be null or invalid indicating that there is no geo coding
//-----------------------------------------------------------------------------
void HRFPageDescriptor::InitFromRasterFileGeocoding(RasterFileGeocoding& pi_geocoding,bool flagGeocodingAsModified)
    {
    m_pGeocoding = &pi_geocoding;
    m_GeocodingHasChanged=flagGeocodingAsModified;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
RasterFileGeocoding const& HRFPageDescriptor::GetRasterFileGeocoding() const
    {
    BeAssert(m_pGeocoding!=NULL);
    return *m_pGeocoding;
    }

//-----------------------------------------------------------------------------
// Public
// Saved
// Resets flags when PageDescriptor have been saved
//-----------------------------------------------------------------------------
void HRFPageDescriptor::Saved()
    {
    SetTransfoModelUnchanged();

    if (m_pListOfMetaDataContainer != 0)
        {
        m_pListOfMetaDataContainer->SetModificationStatus(false);
        }

    m_ClipShapeHasChanged               = false;
    m_FiltersHasChanged                 = false;
    m_RepresentativePaletteHasChanged   = false;
    m_HistogramHasChanged               = false;
    m_ThumbnailHasChanged               = false;
    m_DurationHasChanged                = false;
    m_GeocodingHasChanged               = false;

    m_ListOfModifiedTag.Clear();
    }

//-----------------------------------------------------------------------------
// Public
// SetListOfMetaDataContainer
// Set the list of metadata container
//-----------------------------------------------------------------------------
void HRFPageDescriptor::SetListOfMetaDataContainer(HFCPtr<HMDMetaDataContainerList>& pi_prMDContainers,
                                                          bool                             pi_IsAModification)
    {
    m_pListOfMetaDataContainer = pi_prMDContainers;

    //Should be true during the page descriptor construction.
    //It ensures that all metadata containers aren't seen as modified
    //after the page descriptor is constructed.
    if (pi_IsAModification == false)
        {
        m_pListOfMetaDataContainer->SetModificationStatus(false);
        }
    }

//-----------------------------------------------------------------------------
// Public
// SetTag
//-----------------------------------------------------------------------------
bool HRFPageDescriptor::SetTag(const HFCPtr<HPMGenericAttribute>& pi_rpTag)
    {
    HPRECONDITION(!m_EmptyPage);

    // Validation with the capabilities if it's possible to set the specified TAG
    HDEBUGCODE(HFCPtr<HRFCapability> pCapability(new HRFTagCapability(HFC_WRITE_ONLY, pi_rpTag)));
    HDEBUGCODE(HFCPtr<HRFCapability> pTagUniverse(new HRFUniversalTagCapability(HFC_WRITE_ONLY)));
    HPRECONDITION(m_pPageCapabilities->Supports(pCapability) || m_pPageCapabilities->Supports(pTagUniverse));

    m_pListOfTag->Set(pi_rpTag);
    m_ListOfModifiedTag.Set(pi_rpTag);

    return true;
    }

//-----------------------------------------------------------------------------
// Public
// RemoveTag
//-----------------------------------------------------------------------------
void HRFPageDescriptor::RemoveTag(const HFCPtr<HPMGenericAttribute>& pi_rpTag)
    {
    HPRECONDITION(!m_EmptyPage);

    // Validation with the capabilities if it's possible to set the specified TAG
    HDEBUGCODE(HFCPtr<HRFCapability> pCapability(new HRFTagCapability(HFC_WRITE_ONLY, pi_rpTag)));
    HDEBUGCODE(HFCPtr<HRFCapability> pTagUniverse(new HRFUniversalTagCapability(HFC_WRITE_ONLY)));
    HPRECONDITION(m_pPageCapabilities->Supports(pCapability) || m_pPageCapabilities->Supports(pTagUniverse));

    m_pListOfTag->Remove(*pi_rpTag);
    m_ListOfModifiedTag.Remove(*pi_rpTag);
    }

//-----------------------------------------------------------------------------
// Public
// RemoveTag
//-----------------------------------------------------------------------------
void HRFPageDescriptor::RemoveTag (const HPMGenericAttribute& pi_rTag)
    {
    HPRECONDITION(!m_EmptyPage);

    m_pListOfTag->Remove(pi_rTag);
    m_ListOfModifiedTag.Remove(pi_rTag);
    }

//-----------------------------------------------------------------------------
// Public
// AddMetaDataContainer
// Add the metadata container to the page descriptor
//-----------------------------------------------------------------------------
void HRFPageDescriptor::SetMetaDataContainer(HFCPtr<HMDMetaDataContainer>& pi_rpMDContainer)
    {
    if (m_pListOfMetaDataContainer == 0)
        {
        m_pListOfMetaDataContainer = new HMDMetaDataContainerList();
        }

    m_pListOfMetaDataContainer->SetMetaDataContainer(pi_rpMDContainer);
    }

//-----------------------------------------------------------------------------
// Public
// GetMetadataContainer
// Get the metadata container of the specified type
//-----------------------------------------------------------------------------
const HFCPtr<HMDMetaDataContainer> HRFPageDescriptor::GetMetaDataContainer(HMDMetaDataContainer::Type pi_ContainerType) const
    {
    HFCPtr<HMDMetaDataContainer> pContainerFound;

    if (m_pListOfMetaDataContainer != 0)
        {
        pContainerFound = m_pListOfMetaDataContainer->GetMetaDataContainer(pi_ContainerType);
        }

    return pContainerFound;
    }

//-----------------------------------------------------------------------------
// Public
// CanCreateWith
//-----------------------------------------------------------------------------
bool HRFPageDescriptor::CanCreateWith(HFCAccessMode                            pi_AccessMode,
                                       const HFCPtr<HRFRasterFileCapabilities>& pi_rpPageCapabilities,
                                       const ListOfResolutionDescriptor&        pi_rResolutionDescriptors) const
    {
    // Validate if the client pass the capabilities
    HPRECONDITION(pi_rpPageCapabilities != 0);

    // Validation with the capabilities if it's possible to add a resolution
    if ((pi_rResolutionDescriptors.size() > 1 && pi_rpPageCapabilities->GetCapabilityOfType(HRFMultiResolutionCapability::CLASS_ID, pi_AccessMode) == 0) ||
        (pi_rpPageCapabilities->GetCapabilityOfType(HRFMultiResolutionCapability::CLASS_ID, pi_AccessMode) == 0 &&
         pi_rpPageCapabilities->GetCapabilityOfType(HRFSingleResolutionCapability::CLASS_ID, pi_AccessMode) == 0))
        return false;

    // Page information

    // Resolution information
    // Validation with the capabilities if it's possible to be an unlimited resolution
    if (m_UnlimitedResolution)
        {
        if (pi_rpPageCapabilities->GetCapabilityOfType(HRFMultiResolutionCapability::CLASS_ID, pi_AccessMode) == 0 ||
            !static_cast<HRFMultiResolutionCapability*>(pi_rpPageCapabilities->GetCapabilityOfType(HRFMultiResolutionCapability::CLASS_ID,
                    pi_AccessMode).GetPtr())->IsUnlimitedResolution())
            return false;
        }

    // Page Physical information
    if (m_pRepresentativePalette)
        {
        if (pi_rpPageCapabilities->GetCapabilityOfType(HRFRepresentativePaletteCapability::CLASS_ID, pi_AccessMode) == 0)
            return false;
        }

    if (m_pHistogram)
        {
        if (pi_rpPageCapabilities->GetCapabilityOfType(HRFHistogramCapability::CLASS_ID, pi_AccessMode) == 0)
            return false;
        }

    if (m_pThumbnail)
        {
        if (pi_rpPageCapabilities->GetCapabilityOfType(HRFThumbnailCapability::CLASS_ID, pi_AccessMode) == 0)
            return false;
        }

    // Page Logical information
    if (m_pClipShape)
        {
        HFCPtr<HRFCapability> pCapability = new HRFClipShapeCapability(pi_AccessMode, m_pClipShape->GetCoordinateType());
        if (!pi_rpPageCapabilities->Supports(pCapability))
            return false;
        }

    if (m_pTransfoModel)
        {
        HFCPtr<HRFCapability> pCapability = new HRFTransfoModelCapability(pi_AccessMode, m_pTransfoModel->GetClassID());
        if (!pi_rpPageCapabilities->Supports(pCapability))
            return false;
        }

    if (m_pFilters)
        {
        HFCPtr<HRFCapability> pCapability = new HRFFilterCapability(pi_AccessMode, m_pFilters->GetClassID());
        if (!pi_rpPageCapabilities->Supports(pCapability))
            return false;
        }

    if (m_pListOfTag)
        {
        // loop through the vector and test each tag in the capabilities.
        HPMAttributeSet::HPMASiterator TagIterator;

        // create the universal tag capability
        HFCPtr<HRFCapability> pTagUniverse(new HRFUniversalTagCapability(pi_AccessMode));

        for (TagIterator  = m_pListOfTag->begin();
             TagIterator != m_pListOfTag->end(); TagIterator++)
            {
            HFCPtr<HRFCapability> pCapability   = new HRFTagCapability(pi_AccessMode,  (*TagIterator));
            if (!pi_rpPageCapabilities->Supports(pCapability))
                return false;
            }
        }

    // Media type information
    if (m_Duration == 0)
        {
        if (pi_rpPageCapabilities->GetCapabilityOfType(HRFStillImageCapability::CLASS_ID, pi_AccessMode) == 0)
            return false;
        }
    else
        {
        if (pi_rpPageCapabilities->GetCapabilityOfType(HRFAnimationCapability::CLASS_ID, pi_AccessMode) == 0)
            return false;
        }

    return true;
    }

//-----------------------------------------------------------------------------
// Public
// AddResolutionDescriptor
// Page information
//-----------------------------------------------------------------------------
bool HRFPageDescriptor::AddResolutionDescriptor(const HFCPtr<HRFResolutionDescriptor>& pi_rpResolutionDescriptor)
    {
    HPRECONDITION(!m_EmptyPage);
    // Validation with the capabilities if it's possible to add a resolution
    HPRECONDITION(m_pPageCapabilities->GetCapabilityOfType(HRFMultiResolutionCapability::CLASS_ID, HFC_WRITE_ONLY) != 0);
    m_ListOfResolutionDescriptor.push_back(pi_rpResolutionDescriptor);

    return true;
    }

//-----------------------------------------------------------------------------
// Public
// SetRepresentativePalette
// Page Representative Palette
//-----------------------------------------------------------------------------
bool HRFPageDescriptor::SetRepresentativePalette(const HRPPixelPalette& pi_rPalette)
    {
    HPRECONDITION(!m_EmptyPage);

    // Validation with the capabilities if it's possible to set a Representative Palette
    HPRECONDITION(m_pPageCapabilities->GetCapabilityOfType(HRFRepresentativePaletteCapability::CLASS_ID, HFC_WRITE_ONLY) != 0);

    // Update the flag if the user replace the Representative Palette
    m_RepresentativePaletteHasChanged = true;

    // Replace the Representative Palette by the specified
    m_pRepresentativePalette = new HRPPixelPalette(pi_rPalette);
    HASSERT(m_pRepresentativePalette.get() != 0);

    return true;
    }


//-----------------------------------------------------------------------------
// Public
// SetHistogram
// Page Histogram
//-----------------------------------------------------------------------------
bool HRFPageDescriptor::SetHistogram(const HRPHistogram& pi_rHistogram)
    {
    HPRECONDITION(!m_EmptyPage);

    // For now, we only support native histogram because this is the one that will be written to disk.
    // Histogram update is handle by XYZ::NotifyContentChanged() methods.
    HPRECONDITION(pi_rHistogram.GetSamplingColorSpace() == HRPHistogram::NATIVE);

    // Validation with the capabilities if it's possible to set an histogram
    HPRECONDITION(m_pPageCapabilities->GetCapabilityOfType(HRFHistogramCapability::CLASS_ID, HFC_WRITE_ONLY) != 0);

    // Update the flag if the user replace the histogram
    m_HistogramHasChanged = true;

    // Replace the histogram by the specified
    m_pHistogram = new HRPHistogram(pi_rHistogram);
    HASSERT(m_pHistogram != 0);

    return true;
    }


//-----------------------------------------------------------------------------
// Public
// SetThumbnail
// Page Thumbnail
//-----------------------------------------------------------------------------
bool HRFPageDescriptor::SetThumbnail(const HRFThumbnail& pi_rThumbnail)
    {
    HPRECONDITION(!m_EmptyPage);

    // Validation with the capabilities if it's possible to set a Thumbnail
    HPRECONDITION(m_pPageCapabilities->GetCapabilityOfType(HRFThumbnailCapability::CLASS_ID, HFC_WRITE_ONLY) != 0);

    // Update the flag if the user replace the thumbnail
    m_ThumbnailHasChanged = true;

    // Replace the Thumbnail by the specified
    m_pThumbnail = new HRFThumbnail(pi_rThumbnail);
    HASSERT(m_pThumbnail != 0);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
bool HRFPageDescriptor::ValidateTagCapabilities() const
    {
#ifdef __HMR_DEBUG
    // loop through the vector and test each tag in the capabilities.

    // create the universal tag capability
    HFCPtr<HRFCapability> pTagUniverse(new HRFUniversalTagCapability(m_AccessMode));

    for (HPMAttributeSet::HPMASiterator TagIterator = m_pListOfTag->begin(); TagIterator != m_pListOfTag->end(); TagIterator++)
        {
        HFCPtr<HRFCapability> pCapability   = new HRFTagCapability(m_AccessMode,  (*TagIterator));
        HFCPtr<HRFCapability> pCapabilityRO = new HRFTagCapability(HFC_READ_ONLY, (*TagIterator));
        if (!(m_pPageCapabilities->Supports(pCapability) || m_pPageCapabilities->Supports(pTagUniverse)))
            {
            // If we do not support the capability in the given access mode, check if
            // we support it in read only access.
            if (m_pPageCapabilities->Supports(pCapabilityRO))
                {
                // If we do, just output a message to say so.

                // HRFMessage("Tag :" + (*TagIterator)->GetLabel() + " is supported in read only);

                // Do nothing for now.
                }
             else if(!(m_pPageCapabilities->Supports(pCapability) || m_pPageCapabilities->Supports(pTagUniverse)))
                {
                // We do not support this capability.
                return false;
                }
            }
        }
   
#endif
    return true;
    }