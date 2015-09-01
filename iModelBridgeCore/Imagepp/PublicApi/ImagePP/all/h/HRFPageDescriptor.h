//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFPageDescriptor.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include "HFCAccessMode.h"

#include "HGF2DTransfoModel.h"

#include "HMDMetaDataContainerList.h"

#include "HPMAttributeSet.h"
#include "HPMAttribute.h"

#include "HRPPixelPalette.h"
#include "HRPFilter.h"
#include "HRPHistogram.h"

#include "HRFRasterFileCapabilities.h"
#include "HRFResolutionDescriptor.h"
#include "HRFThumbnail.h"

#include "HTIFFTag.h"

#include <ImagePP/all/h/interface/IRasterGeoCoordinateServices.h>

IMAGEPP_TYPEDEFS(RasterFileGeocoding)
IMAGEPP_REF_COUNTED_PTR(RasterFileGeocoding)


BEGIN_IMAGEPP_NAMESPACE
class HRFRasterFile;
class HCPGeoTiffKeys;

/*=================================================================================**//**
* This class is intended to be use as a proxy class to access geocoding created from raster file.
* The class will keep a copy of the GeoTiff Keys used to create the geocoding object or extract them 
* only when needed. That way, the relation between the geokeys and geocoding will be keep as 
* one cannot always be recreate from the other.
* @bsiclass                                     		Marc.Bedard     06/2013
+===============+===============+===============+===============+===============+======*/
struct RasterFileGeocoding : public RefCountedBase
    {
private:
    IRasterBaseGcsPtr                   m_pGeocoding;
    mutable HFCPtr<HCPGeoTiffKeys>      m_pGeoTiffKeys;//Optimization: Will be query on first get if not provided at construction
    mutable bool                        m_isGeotiffKeysCreated;

    RasterFileGeocoding();
    RasterFileGeocoding(IRasterBaseGcsP pi_pGeocoding);
    RasterFileGeocoding(HCPGeoTiffKeys const* pi_pGeokeys);
    RasterFileGeocoding(const RasterFileGeocoding& object);

public:
    IMAGEPP_EXPORT static RasterFileGeocodingPtr Create();
    IMAGEPP_EXPORT static RasterFileGeocodingPtr Create(IRasterBaseGcsP pi_pGeocoding);
    IMAGEPP_EXPORT static RasterFileGeocodingPtr Create(HCPGeoTiffKeys const* pi_pGeokeys);
    IMAGEPP_EXPORT        RasterFileGeocodingPtr Clone() const;

    IMAGEPP_EXPORT HFCPtr<HGF2DTransfoModel> TranslateToMeter (const HFCPtr<HGF2DTransfoModel>& pi_pModel,
                                                              double                          pi_FactorModelToMeter=1.0,
                                                              bool                            pi_ProjectedCSTypeDefinedWithProjLinearUnitsInterpretation=false,
                                                              bool*                           po_DefaultUnitWasFound=0) const;
    IMAGEPP_EXPORT HFCPtr<HGF2DTransfoModel> TranslateFromMeter (const HFCPtr<HGF2DTransfoModel>& pi_pModel,
                                                                bool                            pi_ProjectedCSTypeDefinedWithProjLinearUnitsInterpretation=false,
                                                                bool*                           po_DefaultUnitWasFound=0) const;

    IRasterBaseGcsCP                       GetGeocodingCP() const;
    IMAGEPP_EXPORT HCPGeoTiffKeys const&    GetGeoTiffKeys() const;
    };

/** -----------------------------------------------------------------------------
    @version 1.0c
    @end

    This concrete class contains all attributes relate to an image or page of a
    raster file. Attributes such as meta information (Title, artist name, copyright,
    ink name), transformation model (scale, rotation, warp), image shaping (rectangle,
    polygon, complex shape), Color correction (contrast, brightness, sharpen, blur),
    number of resolutions and so on can be obtained or set.

    Many of these attributes are interpreted and applied by the application at display
    time. These tools of edition are non-destructive for the image data.

    <img src="..\..\all\gra\hrf\images\HRFPageDescriptor.gif" alt="Page descriptor" align=center>
    @end

    @h3{Note:}
    The first resolution represents an image at "1:1" pixel precision and the other resolutions
    represent the same image at lower sizes.

    To obtain a page descriptor you call GetPageDescriptor() from the rasterfile objec
    For more details refer to the HRF User's Guide
    @end

    @h3{Inheritance notes:}
    This class is not meant to be overridden. Raster files are in charge of extracting
    pages from an exiting raster file and these raster file are provided with standard
    page descriptor when new pages are added to a new or existing file. Overloading is
    not specifically forbidden however, but implications within the HRF architecture have
    not yet been studied and unexpected results may occur.
    @end

    @see HRFRasteFileCapabilities
    @see HRFRasteFile#GetPageDescriptor()
    @end

    @h3{Word Related Documentation:}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\HRFPageDescriptor.doc">Page Descriptor documentation</a>}
    @list{<a href = "..\..\Image++\all\gra\hrf\doc\HRF.doc">HRF User's Guide </a>}
    @end
    -----------------------------------------------------------------------------
*/
class HRFPageDescriptor : public HFCShareableObject<HRFPageDescriptor>
    {
    HDECLARE_SEALEDCLASS_ID(HRFPageDescriptorId_Base)

public:
    typedef vector<HFCPtr<HRFResolutionDescriptor>, allocator<HFCPtr<HRFResolutionDescriptor> > >
    ListOfResolutionDescriptor;

    // Page Creation and destruction
    HRFPageDescriptor (bool                                     pi_EmptyPage = false);
    HRFPageDescriptor (HFCAccessMode                            pi_AccessMode,
                       const HFCPtr<HRFRasterFileCapabilities>& pi_rpPageCapabilities,
                       const HFCPtr<HRFResolutionDescriptor>&   pi_rpResolutionDescriptor,
                       const HRPPixelPalette*                   pi_pRepresentativePalette,
                       const HRPHistogram*                      pi_pHistogram,
                       const HRFThumbnail*                      pi_pThumbnail,
                       const HRFClipShape*                      pi_pClipShape,
                       const HGF2DTransfoModel*                 pi_pTransfoModel,
                       const HRPFilter*                         pi_pFilters,
                       const HPMAttributeSet*                   pi_pTags = 0,
                       uint32_t                                pi_Duration = 0,
                       bool                                     pi_Resizable = false,
                       bool                                     pi_UnlimitedResolution = false,
                       uint64_t                                 pi_MinWidth = 1,             // use only when pi_UnlimitedResolution is true
                       uint64_t                                 pi_MinHeight = 1,            // use only when pi_UnlimitedResolution is true
                       uint64_t                                 pi_MaxWidth = UINT64_MAX,   // use only when pi_UnlimitedResolution is true
                       uint64_t                                 pi_MaxHeight = UINT64_MAX); // use only when pi_UnlimitedResolution is true

    IMAGEPP_EXPORT HRFPageDescriptor (HFCAccessMode                            pi_AccessMode,
                              const HFCPtr<HRFRasterFileCapabilities>& pi_rpPageCapabilities,
                              const ListOfResolutionDescriptor&        pi_rResolutionDescriptors,
                              const HRPPixelPalette*                   pi_pRepresentativePalette,
                              const HRPHistogram*                      pi_pHistogram,
                              const HRFThumbnail*                      pi_pThumbnail,
                              const HRFClipShape*                      pi_pClipShape,
                              const HGF2DTransfoModel*                 pi_pTransfoModel,
                              const HRPFilter*                         pi_pFilters,
                              const HPMAttributeSet*                   pi_pTags = 0,
                              uint32_t                                pi_Duration = 0,
                              bool                                     pi_Resizable = false,
                              bool                                     pi_UnlimitedResolution = false,
                              uint64_t                                 pi_MinWidth = 1,             // use only when pi_UnlimitedResolution is true
                              uint64_t                                 pi_MinHeight = 1,            // use only when pi_UnlimitedResolution is true
                              uint64_t                                 pi_MaxWidth = UINT64_MAX,   // use only when pi_UnlimitedResolution is true
                              uint64_t                                 pi_MaxHeight = UINT64_MAX); // use only when pi_UnlimitedResolution is true

    // Constructor for PageFile without resolution descriptor
    HRFPageDescriptor (HFCAccessMode                            pi_AccessMode,
                       const HFCPtr<HRFRasterFileCapabilities>& pi_rpPageCapabilities,
                       const HRPPixelPalette*                   pi_pRepresentativePalette,
                       const HRPHistogram*                      pi_pHistogram,
                       const HRFThumbnail*                      pi_pThumbnail,
                       const HRFClipShape*                      pi_pClipShape,
                       const HGF2DTransfoModel*                 pi_pTransfoModel,
                       const HRFScanlineOrientation*            pi_pTransfoModelOrientation,
                       const HRPFilter*                         pi_pFilters,
                       const HPMAttributeSet*                   pi_pTags = 0,
                       uint32_t                                pi_Duration = 0,
                       bool                                     pi_Resizable = false,
                       bool                                     pi_UnlimitedResolution = false,
                       uint64_t                                 pi_MinWidth = 1,            // use only when pi_UnlimitedResolution is true
                       uint64_t                                 pi_MinHeight = 1,           // use only when pi_UnlimitedResolution is true
                       uint64_t                                 pi_MaxWidth = ULONG_MAX,   // use only when pi_UnlimitedResolution is true
                       uint64_t                                 pi_MaxHeight = ULONG_MAX); // use only when pi_UnlimitedResolution is true

    // Combined two page descriptor
    HRFPageDescriptor (HFCAccessMode                            pi_AccessMode,
                       const HFCPtr<HRFRasterFileCapabilities>& pi_rpPageCapabilities,
                       const HFCPtr<HRFPageDescriptor>&         pi_rpPriorityPage,
                       const HFCPtr<HRFPageDescriptor>&         pi_rpSecondPage,
                       const ListOfResolutionDescriptor&        pi_rResolutionDescriptors,
                       bool                                     pi_UnlimitedResolution = false);

    IMAGEPP_EXPORT ~HRFPageDescriptor();

    HRFPageDescriptor   (const HRFPageDescriptor& pi_rObj);
    HRFPageDescriptor&  operator=(const HRFPageDescriptor& pi_rObj);

    IMAGEPP_EXPORT const HFCPtr<HMDMetaDataContainerList>&   GetListOfMetaDataContainer();
    IMAGEPP_EXPORT void                                      SetListOfMetaDataContainer(HFCPtr<HMDMetaDataContainerList>& pi_prMDContainers,
                                                                         bool                             pi_IsAModification = false);
    IMAGEPP_EXPORT const HFCPtr<HMDMetaDataContainer> GetMetaDataContainer(HMDMetaDataContainer::Type     pi_ContainerType) const;
    IMAGEPP_EXPORT void                               SetMetaDataContainer(HFCPtr<HMDMetaDataContainer>&  pi_rpMDContainer);

    bool CanCreateWith(HFCAccessMode                            pi_AccessMode,
                        const HFCPtr<HRFRasterFileCapabilities>& pi_rpPageCapabilities,
                        const ListOfResolutionDescriptor&        pi_rResolutionDescriptors) const;

    // Page information
    unsigned short                         CountResolutions        () const;
    uint64_t                               CountBlocksForAllRes    () const;
    const HFCPtr<HRFResolutionDescriptor>&  GetResolutionDescriptor (unsigned short pi_Resolution) const;
    bool                                   AddResolutionDescriptor (const HFCPtr<HRFResolutionDescriptor>& pi_rpResolutionDescriptor);
    HFCAccessMode                           GetAccessMode           () const;
    bool                                   IsEmpty                 () const;
    const HFCPtr<HRFRasterFileCapabilities> GetCapabilities         () const;

    // unlimited resolution information
    bool                                   IsUnlimitedResolution   () const;
    uint64_t                               GetMinWidth             () const;
    uint64_t                               GetMinHeight            () const;
    uint64_t                               GetMaxWidth             () const;
    uint64_t                               GetMaxHeight            () const;

    // Get the frame duration in milliseconds - if Media Type is Still Image = 0; if Media Type is Animation = n ms;
    uint32_t                              GetDuration             () const;
    bool                                   SetDuration             (uint32_t pi_Duration);

    // Page Logical Shape
    bool                                   HasClipShape            () const;
    const HFCPtr<HRFClipShape>&            GetClipShape            () const;
    bool                                   SetClipShape            (const HRFClipShape& pi_rShape);

    // Page Transformation Model
    bool                                   HasTransfoModel         () const;
    const HFCPtr<HGF2DTransfoModel>&       GetTransfoModel         () const;
    IMAGEPP_EXPORT bool                            SetTransfoModel         (const HGF2DTransfoModel& pi_rTransfoModel, bool pi_IgnoreCapabilties=false);
    HRFScanlineOrientation                 GetTransfoModelOrientation () const;

    // Page Filtering
    bool                                   HasFilter               () const;
    const HRPFilter&                       GetFilter               () const;
    bool                                   SetFilter               (const HRPFilter& pi_rFilters);

    // Page Tag
    bool                                    HasTag             (HPMGenericAttribute const& pi_Tag) const;
    template <typename AttributeT> bool     HasTag             () const; 

    HFCPtr<HPMAttributeSet>&                    GetTagsPtr         () const;
    const HPMAttributeSet&                      GetTags            () const;

    template <typename AttributeT> AttributeT const*	FindTagCP	() const; 
    template <typename AttributeT> AttributeT*          FindTagP	();

    IMAGEPP_EXPORT bool                                  SetTag 		(const HFCPtr<HPMGenericAttribute>& pi_rpTag);

    // Deprecated. Use non-ptr overload instead.
    IMAGEPP_EXPORT void                           		RemoveTag	(const HFCPtr<HPMGenericAttribute>& pi_rpTag);
    IMAGEPP_EXPORT void                           		RemoveTag	(const HPMGenericAttribute& pi_rTag);
    template <typename AttributeT> void		    RemoveTag	 ();

    IMAGEPP_EXPORT void                      InitFromRasterFileGeocoding(RasterFileGeocodingR pi_geocoding,bool flagGeocodingAsModified=false);
    IMAGEPP_EXPORT RasterFileGeocodingCR     GetRasterFileGeocoding() const;

    IMAGEPP_EXPORT IRasterBaseGcsCP        GetGeocodingCP() const;
    IMAGEPP_EXPORT void                                  SetGeocoding(IRasterBaseGcsP pi_pGeocoding);

    // Page Representative Palette
    bool                                   HasRepresentativePalette () const;
    const HRPPixelPalette&                  GetRepresentativePalette () const;
    bool                                   SetRepresentativePalette (const HRPPixelPalette& pi_rPalette);

    // Page Histogram
    bool                                   HasHistogram             () const;
    const HFCPtr<HRPHistogram>&            GetHistogram             () const;
    IMAGEPP_EXPORT bool                     SetHistogram             (const HRPHistogram& pi_rHistogram);

    // Page Thumbnail
    bool                                   HasThumbnail             () const;
    const HFCPtr<HRFThumbnail>&             GetThumbnail             () const;
    bool                                   SetThumbnail             (const HRFThumbnail& pi_rThumbnail);

    // Flag to know if the specified data has changed
    bool                                   ClipShapeHasChanged    () const;
    bool                                   TransfoModelHasChanged () const;
    bool                                   GeocodingHasChanged    () const;

    // Method normally use for internal purpose only...
    void                                   SetTransfoModelUnchanged ();
    bool                                   TagHasChanged            (HPMGenericAttribute const& pi_Tag) const; 
    bool                                   FiltersHasChanged        () const;
    bool                                   RepresentativePaletteHasChanged
    () const;
    bool                                   HistogramHasChanged      () const;
    bool                                   ThumbnailHasChanged      () const;
    bool                                   DurationHasChanged       () const;

    // re-size
    bool                                   PageSizeHasChanged      () const;
    bool                                   IsResizable             () const;

    IMAGEPP_EXPORT void                            Saved();


protected:

private:

    friend class HRFRasterFile;

    const HPMGenericAttribute*       FindTagImpl             (const HPMGenericAttribute& pi_rTag) const;
    HPMGenericAttribute*             FindTagImpl             (const HPMGenericAttribute& pi_rTag);

    bool ValidateTagCapabilities() const;

    // Capabilities descriptor
    HFCPtr<HRFRasterFileCapabilities>   m_pPageCapabilities;

    // Page information
    ListOfResolutionDescriptor          m_ListOfResolutionDescriptor;
    HFCPtr<HMDMetaDataContainerList>    m_pListOfMetaDataContainer;
    RasterFileGeocodingPtr              m_pGeocoding;
    uint32_t                            m_Duration;
    HFCAccessMode                       m_AccessMode;
    bool                                m_EmptyPage;
    bool                                m_Resizable;

    // Unlimited resolution information
    bool                                m_UnlimitedResolution;
    uint64_t                            m_MinWidth;
    uint64_t                            m_MinHeight;
    uint64_t                            m_MaxWidth;
    uint64_t                            m_MaxHeight;

    // Page Physical information
    HAutoPtr<HRPPixelPalette>           m_pRepresentativePalette;
    HFCPtr<HRPHistogram>                m_pHistogram;
    HFCPtr<HRFThumbnail>                m_pThumbnail;

    // Page Logical information
    HFCPtr<HRFClipShape>                m_pClipShape;
    HFCPtr<HGF2DTransfoModel>           m_pTransfoModel;
    HRFScanlineOrientation              m_TransfoModelOrientation;
    HAutoPtr<HRPFilter>                 m_pFilters;
    HFCPtr<HPMAttributeSet>             m_pListOfTag;
    HPMAttributeSet                     m_ListOfModifiedTag;

    // Flag to know if the specified data has changed
    bool                               m_ClipShapeHasChanged;
    bool                               m_TransfoModelHasChanged;
    bool                               m_FiltersHasChanged;
    bool                               m_RepresentativePaletteHasChanged;
    bool                               m_HistogramHasChanged;
    bool                               m_ThumbnailHasChanged;
    bool                               m_DurationHasChanged;
    bool                               m_GeocodingHasChanged;
    bool                               m_PageSizeHasChanged;
    };
END_IMAGEPP_NAMESPACE

#include "HRFPageDescriptor.hpp"

