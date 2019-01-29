//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAHistogramOptions.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    Get the palette.
    -----------------------------------------------------------------------------
*/
inline const HRPPixelType* HRAHistogramOptions::GetPixelType() const
    {
    return m_pPixelType.GetPtr();
    }

/** -----------------------------------------------------------------------------
    Set the SamplingOptions object.
    -----------------------------------------------------------------------------
*/
inline void HRAHistogramOptions::SetSamplingOptions(const HRASamplingOptions& pi_rOptions)
    {
    m_SamplingOptions = pi_rOptions;
    }

/** -----------------------------------------------------------------------------
    Get the SamplingOptions object.
    -----------------------------------------------------------------------------
*/
inline HRASamplingOptions& HRAHistogramOptions::GetSamplingOptions() const
    {
    return (HRASamplingOptions&) m_SamplingOptions;
    }

/** -----------------------------------------------------------------------------
    Set the result histogram object.
    -----------------------------------------------------------------------------
*/
inline void HRAHistogramOptions::SetHistogram(const HFCPtr<HRPHistogram>& pi_rpHistogram)
    {
    HPRECONDITION(pi_rpHistogram != 0);

    m_pHistogram = pi_rpHistogram;
    }

/** -----------------------------------------------------------------------------
    Get the result histogram object.
    -----------------------------------------------------------------------------
*/
inline const HFCPtr<HRPHistogram>& HRAHistogramOptions::GetHistogram() const
    {
    return m_pHistogram;
    }

/** -----------------------------------------------------------------------------
    Get the unused channels
    -----------------------------------------------------------------------------
*/
inline void HRAHistogramOptions::SetSamplingColorSpace(HRPHistogram::COLOR_SPACE pi_ColorSpace)
    {
    HASSERT (pi_ColorSpace != HRPHistogram::NATIVE);

    if (pi_ColorSpace != HRPHistogram::NATIVE)
        {
        m_ColorSpace = pi_ColorSpace;
        if (m_pHistogram)
            {
            m_pHistogram->SetSamplingColorSpace(pi_ColorSpace);
            }
        }
    }

/** -----------------------------------------------------------------------------
    Get the unused channels
    -----------------------------------------------------------------------------
*/
inline const HRPHistogram::COLOR_SPACE HRAHistogramOptions::GetSamplingColorSpace() const
    {
    // Be sure the color space is consistant..
    HPRECONDITION(m_pHistogram != 0 || m_pHistogram->GetSamplingColorSpace() == m_ColorSpace);

    return m_ColorSpace;
    }

/** -----------------------------------------------------------------------------
    Get the unused channels
    -----------------------------------------------------------------------------
*/

inline void HRAHistogramOptions::ClearHistogram()
    {
    if (m_pHistogram)
        m_pHistogram->Clear();
    }
END_IMAGEPP_NAMESPACE