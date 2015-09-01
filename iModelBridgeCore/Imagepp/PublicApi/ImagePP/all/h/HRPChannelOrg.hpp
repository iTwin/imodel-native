//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPChannelOrg.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Inline methods for class HRPChannelOrg
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Returns the number of channels defined in this pixel type.
//-----------------------------------------------------------------------------
inline uint32_t HRPChannelOrg::CountChannels() const
    {
    return (uint32_t)m_ChannelsByPosition.size();
    }

//-----------------------------------------------------------------------------
// GetChannelIndex
//-----------------------------------------------------------------------------
inline uint32_t HRPChannelOrg::GetChannelIndex(   HRPChannelType::ChannelRole pi_Role,
                                                unsigned short             pi_Id) const
    {
    // We use two returns because this is faster

    // Search by ID if user channel
    if (pi_Role == HRPChannelType::USER)
        return(m_ChannelsById[pi_Id]);
    else
        return(m_ChannelsByRole[pi_Role]);
    }

//-----------------------------------------------------------------------------
// Returns a pointer to a channel object specified by its index.
//-----------------------------------------------------------------------------
inline const HRPChannelType* HRPChannelOrg::GetChannelPtr(uint32_t pi_Index) const
    {
    if (pi_Index < m_ChannelsByPosition.size())
        return m_ChannelsByPosition[pi_Index];
    else
        return 0;
    }

//-----------------------------------------------------------------------------
// Returns a pointer to a channel object specified by its index.
//-----------------------------------------------------------------------------
inline const HRPChannelType* HRPChannelOrg::operator[](uint32_t pi_Index) const
    {
    if (pi_Index < m_ChannelsByPosition.size())
        return m_ChannelsByPosition[pi_Index];
    else
        return 0;
    }

//-----------------------------------------------------------------------------
// Returns to total size required to store all channels of this pixel.
//-----------------------------------------------------------------------------
inline uint32_t HRPChannelOrg::CountPixelCompositeValueBits() const
    {
    return m_PixelCompositeValueBits;
    }

END_IMAGEPP_NAMESPACE
