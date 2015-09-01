//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPHistogram.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Inline methods for class HRPHistogram
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// GetEntryCount
//-----------------------------------------------------------------------------
inline uint32_t HRPHistogram::GetEntryCount(uint32_t pi_EntryIndex, uint32_t pi_Channel) const
    {
    HPRECONDITION(pi_EntryIndex < m_EntryFrequenciesSize);
    HPRECONDITION(pi_Channel    < m_ChannelCount);

    return m_pEntryFrequencies[pi_Channel][pi_EntryIndex];
    }

//-----------------------------------------------------------------------------
// SetEntryCount
//-----------------------------------------------------------------------------
inline void HRPHistogram::SetEntryCount(uint32_t pi_EntryIndex, uint32_t Count, uint32_t pi_Channel)
    {
    HPRECONDITION(pi_EntryIndex < m_EntryFrequenciesSize);
    HPRECONDITION(pi_Channel    < m_ChannelCount);

    m_pEntryFrequencies[pi_Channel][pi_EntryIndex] = Count;
    }

//-----------------------------------------------------------------------------
// IncrementEntryCount
//-----------------------------------------------------------------------------
inline void HRPHistogram::IncrementEntryCount(uint32_t pi_EntryIndex, uint32_t pi_Count, uint32_t pi_Channel)
    {
    HPRECONDITION(pi_EntryIndex < m_EntryFrequenciesSize);
    HPRECONDITION(pi_Channel    < m_ChannelCount);

    // increment the count if it is not the maximum for a UInt32
    if(m_pEntryFrequencies[pi_Channel][pi_EntryIndex] <= (ULONG_MAX - pi_Count))
        (m_pEntryFrequencies[pi_Channel][pi_EntryIndex]) += pi_Count;
    }

//-----------------------------------------------------------------------------
// GetEntryFrequenciesSize
//-----------------------------------------------------------------------------
inline uint32_t HRPHistogram::GetEntryFrequenciesSize(uint32_t pi_Channel) const
    {
    HPRECONDITION(pi_Channel    < m_ChannelCount);

    return m_EntryFrequenciesSize;
    }

//-----------------------------------------------------------------------------
// GetEntryFrequencies
//-----------------------------------------------------------------------------
inline void HRPHistogram::GetEntryFrequencies(uint32_t* pio_pEntryFrequencies,
                                              uint32_t pi_Channel) const
    {
    HPRECONDITION(pio_pEntryFrequencies != 0);
    HPRECONDITION(pi_Channel < m_ChannelCount);

    memcpy(pio_pEntryFrequencies, m_pEntryFrequencies[pi_Channel], m_EntryFrequenciesSize * sizeof(uint32_t));
    }

//-----------------------------------------------------------------------------
// GetEntryFrequencies
//-----------------------------------------------------------------------------
inline uint32_t HRPHistogram::GetChannelCount() const
    {
    return m_ChannelCount;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

inline const HRPHistogram::COLOR_SPACE HRPHistogram::GetSamplingColorSpace() const
    {
    return m_ColorSpace;
    }
END_IMAGEPP_NAMESPACE
