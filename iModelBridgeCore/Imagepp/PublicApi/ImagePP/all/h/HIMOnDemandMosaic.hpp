//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIMOnDemandMosaic.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE

/** ---------------------------------------------------------------------------
    Start to iterate on the mosaic's source images
    ---------------------------------------------------------------------------
*/
inline HIMOnDemandMosaic::IteratorHandle HIMOnDemandMosaic::StartIteration(void) const
    {
    return m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria());
    }

/** ---------------------------------------------------------------------------
    Advance to the next image in the mosaic
    ---------------------------------------------------------------------------
*/
inline const HFCPtr<HRARaster> HIMOnDemandMosaic::Iterate(IteratorHandle pi_Handle) const
    {
    HFCPtr<HRARaster> pRaster;

    ((IndexType::IndexableList*)pi_Handle)->pop_front();

    if (((IndexType::IndexableList*)pi_Handle)->size() > 0)
        {
        pRaster = GetRaster(((IndexType::IndexableList*)pi_Handle)->front()->GetObject());
        }

    return pRaster;
    }


/** ---------------------------------------------------------------------------
    Get the current image
    ---------------------------------------------------------------------------
*/
inline const HFCPtr<HRARaster> HIMOnDemandMosaic::GetElement(IteratorHandle pi_Handle) const
    {
    return GetRaster(((IndexType::IndexableList*)pi_Handle)->front()->GetObject());
    }


/** ---------------------------------------------------------------------------
    Get the current image's visible portion
    ---------------------------------------------------------------------------
*/
inline const HFCPtr<HVEShape> HIMOnDemandMosaic::GetVisiblePart(IteratorHandle pi_Handle) const
    {
    return m_pIndex->GetIndex1()->GetVisibleSurfaceOf(((IndexType::IndexableList*)pi_Handle)->front());
    }


/** ---------------------------------------------------------------------------
    Destroy the iterator.
    ---------------------------------------------------------------------------
*/
inline void HIMOnDemandMosaic::StopIteration(IteratorHandle pi_Handle) const
    {
    delete ((IndexType::IndexableList*)pi_Handle);
    }


/** ---------------------------------------------------------------------------
    Get the image count
    ---------------------------------------------------------------------------
*/
inline uint32_t HIMOnDemandMosaic::CountElements(IteratorHandle pi_Handle) const
    {
    return (uint32_t)((IndexType::IndexableList*)pi_Handle)->size();
    }

/** ---------------------------------------------------------------------------
    Set representative PSS for worlds
    ---------------------------------------------------------------------------
*/
inline void HIMOnDemandMosaic::SetRepresentativePSSForWorlds(WString& pi_rPSSDescriptiveNode)
    {
    m_WorldDescriptivePSS = pi_rPSSDescriptiveNode;
    }

/** ---------------------------------------------------------------------------
    Return the downsampling method used for the cache file. 
    ---------------------------------------------------------------------------
*/
inline HRFDownSamplingMethod::DownSamplingMethod HIMOnDemandMosaic::GetCacheFileDownSamplingMethod() const
    {
    return m_CacheFileDownSamplingMethod;
    }

/** ---------------------------------------------------------------------------
    Return true if the on-demand mosaic has a cache file.
    ---------------------------------------------------------------------------
*/
inline bool HIMOnDemandMosaic::HasCache()
    {
    return (m_pSubRes != 0);
    }

/** ---------------------------------------------------------------------------
    Remove the cache file from the mosaic.
    ---------------------------------------------------------------------------
*/
inline void HIMOnDemandMosaic::RemoveCache()
    {
    m_pSubRes = 0;
    m_CacheFileDownSamplingMethod = HRFDownSamplingMethod::NONE;
    }

END_IMAGEPP_NAMESPACE