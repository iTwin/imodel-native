//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFPageDescriptor.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Inline methods for class HRFPageDescriptor
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Public
// GetCapabilities
// Page information
//-----------------------------------------------------------------------------
inline const HFCPtr<HRFRasterFileCapabilities> HRFPageDescriptor::GetCapabilities() const
    {
    return m_pPageCapabilities;
    }

//-----------------------------------------------------------------------------
// Public
// IsUnlimitedResolution
// Unlimited resolution information
//-----------------------------------------------------------------------------
inline bool HRFPageDescriptor::IsUnlimitedResolution() const
    {
    HPRECONDITION(!m_EmptyPage);
    return m_UnlimitedResolution;
    }

//-----------------------------------------------------------------------------
// Public
// IsUnlimitedResolution
// Unlimited resolution information
//-----------------------------------------------------------------------------
inline uint64_t HRFPageDescriptor::GetMinWidth() const
    {
    HPRECONDITION(IsUnlimitedResolution());
    return m_MinWidth;
    }

//-----------------------------------------------------------------------------
// Public
// IsUnlimitedResolution
// Unlimited resolution information
//-----------------------------------------------------------------------------
inline uint64_t HRFPageDescriptor::GetMinHeight() const
    {
    HPRECONDITION(IsUnlimitedResolution());
    return m_MinHeight;
    }

//-----------------------------------------------------------------------------
// Public
// IsUnlimitedResolution
// Unlimited resolution information
//-----------------------------------------------------------------------------
inline uint64_t HRFPageDescriptor::GetMaxWidth() const
    {
    HPRECONDITION(IsUnlimitedResolution());
    return m_MaxWidth;
    }

//-----------------------------------------------------------------------------
// Public
// IsUnlimitedResolution
// Unlimited resolution information
//-----------------------------------------------------------------------------
inline uint64_t HRFPageDescriptor::GetMaxHeight() const
    {
    HPRECONDITION(IsUnlimitedResolution());
    return m_MaxHeight;
    }

//-----------------------------------------------------------------------------
// Public
// IsEmpty
// Page information
//-----------------------------------------------------------------------------
inline bool HRFPageDescriptor::IsEmpty() const
    {
    return m_EmptyPage;
    }

//-----------------------------------------------------------------------------
// Public
// IsResizable
// Page information
//-----------------------------------------------------------------------------
inline bool HRFPageDescriptor::IsResizable() const
    {
    return m_Resizable;
    }

//-----------------------------------------------------------------------------
// Public
// CountResolution
// Page information
//-----------------------------------------------------------------------------
inline unsigned short HRFPageDescriptor::CountResolutions() const
    {
    return (unsigned short)m_ListOfResolutionDescriptor.size();
    }

//-----------------------------------------------------------------------------
// Public
// GetResolutionDescriptor
// Page information
//-----------------------------------------------------------------------------
inline const HFCPtr<HRFResolutionDescriptor>&  HRFPageDescriptor::GetResolutionDescriptor(unsigned short pi_Resolution) const
    {
    HPRECONDITION(!m_EmptyPage);
    HPRECONDITION(pi_Resolution <= (CountResolutions() -1));

    return m_ListOfResolutionDescriptor[pi_Resolution];
    }

//-----------------------------------------------------------------------------
// Public
// GetDuration
// Get the frame duration in milliseconds
//-----------------------------------------------------------------------------
inline uint32_t HRFPageDescriptor::GetDuration() const
    {
    HPRECONDITION(!m_EmptyPage);

    // Look if we have a Shape
    return m_Duration;
    }


/** -----------------------------------------------------------------------------
    Get this page access mode.

    @return This page access mode.
    -----------------------------------------------------------------------------
*/
inline HFCAccessMode HRFPageDescriptor::GetAccessMode() const
    {
    HPRECONDITION(!m_EmptyPage);

    // Look if we have a Shape
    return m_AccessMode;
    }

//-----------------------------------------------------------------------------
// Public
// HasClipShape
// Page Logical Shape
//-----------------------------------------------------------------------------
inline bool HRFPageDescriptor::HasClipShape() const
    {
    HPRECONDITION(!m_EmptyPage);

    // Look if we have a Shape
    return (m_pClipShape != 0);
    }

//-----------------------------------------------------------------------------
// Public
// GetClipShape
// Page Logical Shape
//-----------------------------------------------------------------------------
inline const HFCPtr<HRFClipShape>& HRFPageDescriptor::GetClipShape() const
    {
    HPRECONDITION(!m_EmptyPage);
    HPRECONDITION(HasClipShape());
    return m_pClipShape;
    }

//-----------------------------------------------------------------------------
// Public
// HasTransfoModel
// Page Transformation Model
//-----------------------------------------------------------------------------
inline bool HRFPageDescriptor::HasTransfoModel() const
    {
    HPRECONDITION(!m_EmptyPage);

    // Look if we have a Transformation model
    return (m_pTransfoModel != 0);
    }

//-----------------------------------------------------------------------------
// Public
// GetTransfoModel
// Page Transformation Model
//-----------------------------------------------------------------------------
inline const HFCPtr<HGF2DTransfoModel>& HRFPageDescriptor::GetTransfoModel() const
    {
    HPRECONDITION(!m_EmptyPage);
    HPRECONDITION(HasTransfoModel());
    return m_pTransfoModel;
    }

//-----------------------------------------------------------------------------
// Public
// GetTransfoModelSLO
// Page Transformation Model
//-----------------------------------------------------------------------------
inline HRFScanlineOrientation HRFPageDescriptor::GetTransfoModelOrientation() const
    {
    HPRECONDITION(!m_EmptyPage);
    HPRECONDITION(HasTransfoModel());
    return m_TransfoModelOrientation;
    }

//-----------------------------------------------------------------------------
// Public
// HasFilter
// Page Filtering
//-----------------------------------------------------------------------------
inline bool HRFPageDescriptor::HasFilter() const
    {
    HPRECONDITION(!m_EmptyPage);

    // Look if we have a filter
    return (m_pFilters.get() != 0);
    }

//-----------------------------------------------------------------------------
// Public
// GetFilter
// Page Filtering
//-----------------------------------------------------------------------------
inline const HRPFilter& HRFPageDescriptor::GetFilter() const
    {
    HPRECONDITION(!m_EmptyPage);
    HPRECONDITION(HasFilter());
    return *m_pFilters;
    }

//-----------------------------------------------------------------------------
// Public
// RemoveTag
//-----------------------------------------------------------------------------
template <typename AttributeT> inline void HRFPageDescriptor::RemoveTag ()
    {
    HPRECONDITION(!m_EmptyPage);

    m_pListOfTag->Remove<AttributeT>();
    m_ListOfModifiedTag.Remove<AttributeT>();
    }

//-----------------------------------------------------------------------------
// Public
// HasTag
// Page Tag
//-----------------------------------------------------------------------------
inline bool HRFPageDescriptor::HasTag (HPMGenericAttribute const& pi_Tag) const 
    {
    HPRECONDITION(!m_EmptyPage);
    return m_pListOfTag->HasAttribute(pi_Tag.GetID());
    }

//-----------------------------------------------------------------------------
// Public
// HasTag
// Page Tag
//-----------------------------------------------------------------------------
template <typename AttributeT> inline bool HRFPageDescriptor::HasTag () const 
    {
    HPRECONDITION(!m_EmptyPage);
    return m_pListOfTag->HasAttribute(static_cast<const HPMAttributesID>(AttributeT::ATTRIBUTE_ID));
    }

//-----------------------------------------------------------------------------
// Public
// FindTagCP
//-----------------------------------------------------------------------------
template <typename AttributeT> inline const AttributeT* HRFPageDescriptor::FindTagCP() const 
    {
    HPRECONDITION(!m_EmptyPage);
    return m_pListOfTag->FindAttributeCP<AttributeT>(); 
    }

//-----------------------------------------------------------------------------
// Public
// FindTagP
//-----------------------------------------------------------------------------
template <typename AttributeT> inline AttributeT* HRFPageDescriptor::FindTagP() 
    {
    HPRECONDITION(!m_EmptyPage);
    return m_pListOfTag->FindAttributeP<AttributeT>(); 
    }

//-----------------------------------------------------------------------------
// Public
// GetTag
// Page Tag
//-----------------------------------------------------------------------------
inline const HPMAttributeSet& HRFPageDescriptor::GetTags() const
    {
    HPRECONDITION(!m_EmptyPage);

    return *m_pListOfTag;
    }

//-----------------------------------------------------------------------------
// Public
// GetTagPtr
// Page Tag
//-----------------------------------------------------------------------------
inline HFCPtr<HPMAttributeSet>& HRFPageDescriptor::GetTagsPtr() const
    {
    HPRECONDITION(!m_EmptyPage);

    return (HFCPtr<HPMAttributeSet>&)m_pListOfTag;
    }

//-----------------------------------------------------------------------------
// Public
// HasRepresentativePalette
// Page Representative Palette
//-----------------------------------------------------------------------------
inline bool HRFPageDescriptor::HasRepresentativePalette() const
    {
    HPRECONDITION(!m_EmptyPage);

    // Look if we have a Representative Palette
    return (m_pRepresentativePalette.get() != 0);
    }

//-----------------------------------------------------------------------------
// Public
// GetRepresentativePalette
// Page Representative Palette
//-----------------------------------------------------------------------------
inline const HRPPixelPalette& HRFPageDescriptor::GetRepresentativePalette() const
    {
    HPRECONDITION(!m_EmptyPage);
    HPRECONDITION(HasRepresentativePalette());
    return *m_pRepresentativePalette;
    }

//-----------------------------------------------------------------------------
// Public
// SetRepresentativePalette
// Page Histogram
//-----------------------------------------------------------------------------
inline bool HRFPageDescriptor::HasHistogram() const
    {
    HPRECONDITION(!m_EmptyPage);

    // Look if we have an histogram
    return (m_pHistogram != 0);
    }

//-----------------------------------------------------------------------------
// Public
// SetRepresentativePalette
// Page Histogram
//-----------------------------------------------------------------------------
inline const HFCPtr<HRPHistogram>& HRFPageDescriptor::GetHistogram() const
    {
    HPRECONDITION(!m_EmptyPage);
    HPRECONDITION(HasHistogram());
    return m_pHistogram;
    }

//-----------------------------------------------------------------------------
// Public
// HasThumbnail
// Page Thumbnail
//-----------------------------------------------------------------------------
inline bool HRFPageDescriptor::HasThumbnail() const
    {
    HPRECONDITION(!m_EmptyPage);

    // Look if we have a Thumbnail
    return (m_pThumbnail != 0);
    }

//-----------------------------------------------------------------------------
// Public
// GetThumbnail
// Page Thumbnail
//-----------------------------------------------------------------------------
inline const HFCPtr<HRFThumbnail>& HRFPageDescriptor::GetThumbnail() const
    {
    HPRECONDITION(!m_EmptyPage);
    HPRECONDITION(HasThumbnail());
    return m_pThumbnail;
    }

//-----------------------------------------------------------------------------
// Public
// ClipShapeHasChanged
// Flag to know if the specified data has changed
//-----------------------------------------------------------------------------
inline bool HRFPageDescriptor::ClipShapeHasChanged() const
    {
    HPRECONDITION(!m_EmptyPage);

    return m_ClipShapeHasChanged;
    }

//-----------------------------------------------------------------------------
// Public
// TransfoModelHasChanged
// Flag to know if the specified data has changed
//-----------------------------------------------------------------------------
inline bool HRFPageDescriptor::TransfoModelHasChanged() const
    {
    HPRECONDITION(!m_EmptyPage);

    return m_TransfoModelHasChanged;
    }

//-----------------------------------------------------------------------------
// Public
// SetTransfoModelUnChanged
// Reset the flag to false, the model is not changed
//-----------------------------------------------------------------------------
inline void HRFPageDescriptor::SetTransfoModelUnchanged()
    {
    HPRECONDITION(!m_EmptyPage);

    m_TransfoModelHasChanged = false;
    }

//-----------------------------------------------------------------------------
// Public
// FiltersHasChanged
// Flag to know if the specified data has changed
//-----------------------------------------------------------------------------
inline bool HRFPageDescriptor::FiltersHasChanged() const
    {
    HPRECONDITION(!m_EmptyPage);

    return m_FiltersHasChanged;
    }

//-----------------------------------------------------------------------------
// Public
// RepresentativePaletteHasChanged
// Flag to know if the specified data has changed
//-----------------------------------------------------------------------------
inline bool HRFPageDescriptor::RepresentativePaletteHasChanged() const
    {
    HPRECONDITION(!m_EmptyPage);

    return m_RepresentativePaletteHasChanged;
    }

//-----------------------------------------------------------------------------
// Public
// HistogramHasChanged
// Flag to know if the specified data has changed
//-----------------------------------------------------------------------------
inline bool HRFPageDescriptor::HistogramHasChanged() const
    {
    HPRECONDITION(!m_EmptyPage);

    return m_HistogramHasChanged;
    }

//-----------------------------------------------------------------------------
// Public
// ThumbnailHasChanged
// Flag to know if the specified data has changed
//-----------------------------------------------------------------------------
inline bool HRFPageDescriptor::ThumbnailHasChanged() const
    {
    HPRECONDITION(!m_EmptyPage);

    return m_ThumbnailHasChanged;
    }

//-----------------------------------------------------------------------------
// Public
// TagHasChanged
// Flag to know if the specified tag has changed
//-----------------------------------------------------------------------------
inline bool HRFPageDescriptor::TagHasChanged(HPMGenericAttribute const& pi_Tag) const
    {
    HPRECONDITION(!m_EmptyPage);
    return m_ListOfModifiedTag.HasAttribute(pi_Tag.GetID());
    }

//-----------------------------------------------------------------------------
// Public
// DurationHasChanged
// Flag to know if the duration has changed
//-----------------------------------------------------------------------------
inline bool HRFPageDescriptor::DurationHasChanged() const
    {
    HPRECONDITION(!m_EmptyPage);

    return m_DurationHasChanged;
    }

//-----------------------------------------------------------------------------
// Public
// PageSizeHasChanged
// Flag to know if the page size has changed
//-----------------------------------------------------------------------------
inline bool HRFPageDescriptor::PageSizeHasChanged() const
    {
    HPRECONDITION(!m_EmptyPage);

    return m_PageSizeHasChanged;
    }

//-----------------------------------------------------------------------------
// Public
// GetListOfMetaDataContainer
// Get the list of metadata container
//-----------------------------------------------------------------------------
inline const HFCPtr<HMDMetaDataContainerList>& HRFPageDescriptor::GetListOfMetaDataContainer()
    {
    return m_pListOfMetaDataContainer;
    }

END_IMAGEPP_NAMESPACE
