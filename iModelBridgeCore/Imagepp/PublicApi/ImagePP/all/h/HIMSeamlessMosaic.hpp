//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIMSeamlessMosaic.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "HRPPixelTypeV24R8G8B8.h"
#include "HRPPixelTypeGray.h"

BEGIN_IMAGEPP_NAMESPACE
/** ---------------------------------------------------------------------------
    Start to iterate on the images. The iterator must be destroyed by calling
    StopIteration when done.
    @see HIMSeamlessMosaic::StopIteration
    ---------------------------------------------------------------------------
*/
inline HIMSeamlessMosaic::IteratorHandle HIMSeamlessMosaic::StartIteration(void) const
    {
    return m_pIndex->GetIndex1()->QueryIndexables(HIDXSearchCriteria());
    }


/** ---------------------------------------------------------------------------
    Advance to next element in list
    @return A smart pointer to the next element in the list.
    ---------------------------------------------------------------------------
*/
inline const HFCPtr<HRARaster> HIMSeamlessMosaic::Iterate(IteratorHandle pi_Handle) const
    {
    ((IndexType::IndexableList*)pi_Handle)->pop_front();
    return ((IndexType::IndexableList*)pi_Handle)->size() > 0 ?
           ((IndexType::IndexableList*)pi_Handle)->front()->GetObject()
           :   0;
    }


/** ---------------------------------------------------------------------------
    Get the current element
    @return A smart pointer to the current element in the list.
    ---------------------------------------------------------------------------
*/
inline const HFCPtr<HRARaster> HIMSeamlessMosaic::GetElement(IteratorHandle pi_Handle) const
    {
    return ((IndexType::IndexableList*)pi_Handle)->front()->GetObject();
    }


/** ---------------------------------------------------------------------------
    Get the current element's visible shape
    @return A shape representing the visible part of the current image.
    ---------------------------------------------------------------------------
*/
inline const HFCPtr<HVEShape> HIMSeamlessMosaic::GetVisiblePart(IteratorHandle pi_Handle) const
    {
    return m_pIndex->GetIndex1()->GetVisibleSurfaceOf(((IndexType::IndexableList*)pi_Handle)->front());
    }


/** ---------------------------------------------------------------------------
    Stop the iteration
    ---------------------------------------------------------------------------
*/
inline void HIMSeamlessMosaic::StopIteration(IteratorHandle pi_Handle) const
    {
    delete ((IndexType::IndexableList*)pi_Handle);
    }


/** ---------------------------------------------------------------------------
    Get the element count
    ---------------------------------------------------------------------------
*/
inline uint32_t HIMSeamlessMosaic::CountElements(IteratorHandle pi_Handle) const
    {
    return (uint32_t)((IndexType::IndexableList*)pi_Handle)->size();
    }


/** ---------------------------------------------------------------------------
    Check if the global color balancing will be applied
    ---------------------------------------------------------------------------
*/
inline bool HIMSeamlessMosaic::GlobalAlgorithmApplied() const
    {
    return m_ApplyGlobalAlgorithm;
    }


/** ---------------------------------------------------------------------------
    Check if the positional color balancing will be applied
    ---------------------------------------------------------------------------
*/
inline bool HIMSeamlessMosaic::PositionalAlgorithmApplied() const
    {
    return m_ApplyPositionalAlgorithm;
    }


/** ---------------------------------------------------------------------------
    Retrieve the width of the blend corridors
    ---------------------------------------------------------------------------
*/
inline double HIMSeamlessMosaic::GetBlendWidth() const
    {
    return m_BlendWidth;
    }


/** ---------------------------------------------------------------------------
    Retrieve the sampling quality for histogram computing
    ---------------------------------------------------------------------------
*/
inline HIMSeamlessMosaic::SamplingQuality HIMSeamlessMosaic::GetSamplingQuality() const
    {
    return m_SamplingQuality;
    }


/** ---------------------------------------------------------------------------
    Check if the image can be used inside the mosaic. Constraints are:
    - The pixeltype must be V8Gray8 or V24R8G8B8
    ---------------------------------------------------------------------------
*/
inline bool HIMSeamlessMosaic::IsAValidSource(const HFCPtr<HRARaster>& pi_rpRaster) const
    {
    return (pi_rpRaster->GetPixelType()->IsCompatibleWith(HRPPixelTypeV24R8G8B8::CLASS_ID) ||
            pi_rpRaster->GetPixelType()->IsCompatibleWith(HRPPixelTypeGray::CLASS_ID));
    }

END_IMAGEPP_NAMESPACE