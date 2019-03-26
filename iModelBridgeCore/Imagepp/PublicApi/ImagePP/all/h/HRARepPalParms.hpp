//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRARepPalParms.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HRARepPalParms
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// GetPixelType - Return the pixel type attribute
//-----------------------------------------------------------------------------
inline const HFCPtr<HRPPixelType>& HRARepPalParms::GetPixelType() const
    {
    return(m_pPixelType);
    }

//-----------------------------------------------------------------------------
// SetMaxEntries - Set the maximum entries into the palette
//-----------------------------------------------------------------------------
inline void HRARepPalParms::SetMaxEntries(uint32_t pi_MaxEntries)
    {
    // For best result you should pass the max entries value to the constructor.
    // Why ? Because the constructor init the palette to the max entries specified
    // while this method does not resize the pixel palette to the max entries.
    HPRECONDITION(false);
    m_MaxEntries = pi_MaxEntries;
    }

//-----------------------------------------------------------------------------
// GetMaxEntries - Get the maximum entries into the palette
//-----------------------------------------------------------------------------
inline uint32_t HRARepPalParms::GetMaxEntries() const
    {
    return m_MaxEntries;
    }

//-----------------------------------------------------------------------------
// UseCache - Set the percentage of tiles to scan
//-----------------------------------------------------------------------------
inline bool HRARepPalParms::UseCache() const
    {
    return(m_UseCache);
    }

//-----------------------------------------------------------------------------
// GetHistogram
//-----------------------------------------------------------------------------
inline const HFCPtr<HRPHistogram>& HRARepPalParms::GetHistogram() const
    {
    return m_pHistogram;
    }

//-----------------------------------------------------------------------------
// SetHistogram
//-----------------------------------------------------------------------------
inline void HRARepPalParms::SetHistogram(const HFCPtr<HRPHistogram>& pio_rpHistogram)
    {
    m_pHistogram = pio_rpHistogram;
    }

//-----------------------------------------------------------------------------
// SetSamplingOptions
//-----------------------------------------------------------------------------
inline void HRARepPalParms::SetSamplingOptions(const HRASamplingOptions& pi_rOptions)
    {
    m_SamplingOptions = pi_rOptions;
    }

//-----------------------------------------------------------------------------
// GetSamplingOptions
//-----------------------------------------------------------------------------
inline const HRASamplingOptions& HRARepPalParms::GetSamplingOptions() const
    {
    return m_SamplingOptions;
    }

//-----------------------------------------------------------------------------
// public
// SetCacheUse
//-----------------------------------------------------------------------------
inline void HRARepPalParms::SetCacheUse(bool pi_State)
    {
    m_UseCache = pi_State;
    }
END_IMAGEPP_NAMESPACE
