//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIMMosaic.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
/** ---------------------------------------------------------------------------
    Start to iterate on the mosaic's source images
    ---------------------------------------------------------------------------
*/
inline HIMMosaic::IteratorHandle HIMMosaic::StartIteration(void) const
    {
    return m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria());
    }


/** ---------------------------------------------------------------------------
    Advance to the next image in the mosaic
    ---------------------------------------------------------------------------
*/
inline const HFCPtr<HRARaster> HIMMosaic::Iterate(IteratorHandle pi_Handle) const
    {
    ((IndexType::IndexableList*)pi_Handle)->pop_front();
    return ((IndexType::IndexableList*)pi_Handle)->size() > 0 ?
           ((IndexType::IndexableList*)pi_Handle)->front()->GetObject()
           :   0;
    }


/** ---------------------------------------------------------------------------
    Get the current image
    ---------------------------------------------------------------------------
*/
inline const HFCPtr<HRARaster> HIMMosaic::GetElement(IteratorHandle pi_Handle) const
    {
    return ((IndexType::IndexableList*)pi_Handle)->front()->GetObject();
    }


/** ---------------------------------------------------------------------------
    Get the current image's visible portion
    ---------------------------------------------------------------------------
*/
inline const HFCPtr<HVEShape> HIMMosaic::GetVisiblePart(IteratorHandle pi_Handle) const
    {
    return m_pIndex->GetIndex1()->GetVisibleSurfaceOf(((IndexType::IndexableList*)pi_Handle)->front());
    }


/** ---------------------------------------------------------------------------
    Destroy the iterator.
    ---------------------------------------------------------------------------
*/
inline void HIMMosaic::StopIteration(IteratorHandle pi_Handle) const
    {
    delete ((IndexType::IndexableList*)pi_Handle);
    }


/** ---------------------------------------------------------------------------
    Get the image count
    ---------------------------------------------------------------------------
*/
inline uint32_t HIMMosaic::CountElements(IteratorHandle pi_Handle) const
    {
    return (uint32_t)((IndexType::IndexableList*)pi_Handle)->size();
    }
END_IMAGEPP_NAMESPACE