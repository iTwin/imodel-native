//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIMColorBalancedImage.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
/** ---------------------------------------------------------------------------
    Specify if the positional algorithm will be applied.
    ---------------------------------------------------------------------------
*/
inline void HIMColorBalancedImage::ApplyPositionalAlgorithm(bool pi_Apply)
    {
// HChk MR: TEMPORARY
// Accept this, but change both at the same time, for testing.
    // Don't change the algorithms once we started computing parameters
//    HASSERT(GetNumberOfNeighbors() == 0);

    m_ApplyPositional = pi_Apply;
    }


/** ---------------------------------------------------------------------------
    Ask if the positional algorithm will be applied.
    ---------------------------------------------------------------------------
*/
inline bool HIMColorBalancedImage::PositionalAlgorithmApplied() const
    {
    return m_ApplyPositional;
    }


/** ---------------------------------------------------------------------------
    Specify if the global algorithm will be applied.
    ---------------------------------------------------------------------------
*/
inline void HIMColorBalancedImage::ApplyGlobalAlgorithm(bool pi_Apply)
    {
// HChk MR: TEMPORARY
// Accept this, but change both at the same time, for testing.
    // Don't change the algorithms once we started computing parameters
//    HASSERT(GetNumberOfNeighbors() == 0);

    m_ApplyGlobal = pi_Apply;
    }


/** ---------------------------------------------------------------------------
    Ask if the global algorithm will be applied.
    ---------------------------------------------------------------------------
*/
inline bool HIMColorBalancedImage::GlobalAlgorithmApplied() const
    {
    return m_ApplyGlobal;
    }


/** ---------------------------------------------------------------------------
    Return the number of neighbors currently set.
    ---------------------------------------------------------------------------
*/
inline size_t HIMColorBalancedImage::GetNumberOfNeighbors() const
    {
    size_t Result = 0;
    if (m_HasLeftNeighbor)   ++Result;
    if (m_HasRightNeighbor)  ++Result;
    if (m_HasTopNeighbor)    ++Result;
    if (m_HasBottomNeighbor) ++Result;

    return Result;
    }


/** ---------------------------------------------------------------------------
    Check if the specified raster is used here as primary image or as
    one of the neighbors.
    ---------------------------------------------------------------------------
*/
inline bool HIMColorBalancedImage::Uses(const HFCPtr<HRARaster>& pi_rpRaster) const
    {
    if (pi_rpRaster == this)
        return true;

    if (m_HasLeftNeighbor && pi_rpRaster == (HFCPtr<HRARaster>&) m_pLeftNeighbor)
        return true;

    if (m_HasRightNeighbor && pi_rpRaster == (HFCPtr<HRARaster>&) m_pRightNeighbor)
        return true;

    if (m_HasTopNeighbor && pi_rpRaster == (HFCPtr<HRARaster>&) m_pTopNeighbor)
        return true;

    if (m_HasBottomNeighbor && pi_rpRaster == (HFCPtr<HRARaster>&) m_pBottomNeighbor)
        return true;

    return false;
    }


/** ---------------------------------------------------------------------------
    Retrieve the sampling quality for histogram computing
    ---------------------------------------------------------------------------
*/
inline HIMColorBalancedImage::SamplingQuality HIMColorBalancedImage::GetSamplingQuality() const
    {
    return m_SamplingQuality;
    }


/** ---------------------------------------------------------------------------
    Set the quality/speed for histograms computing.
    ---------------------------------------------------------------------------
*/
inline void HIMColorBalancedImage::SetSamplingQuality(HIMColorBalancedImage::SamplingQuality pi_Quality)
    {
    if (m_SamplingQuality != pi_Quality)
        {
        m_SamplingQuality = pi_Quality;

        RecomputeHistograms();
        }
    }


/** ---------------------------------------------------------------------------
    Retrieve the color mode
    ---------------------------------------------------------------------------
*/
inline HIMColorBalancedImage::ColorMode HIMColorBalancedImage::GetColorMode() const
    {
    return m_ColorMode;
    }


END_IMAGEPP_NAMESPACE