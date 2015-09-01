//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPTypedFilter.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HRPTypedFilter
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// protected
// AreThereLostChannels
//-----------------------------------------------------------------------------
inline bool HRPTypedFilter::AreThereLostChannels() const
    {
    return(m_LostChannels);
    }

//-----------------------------------------------------------------------------
// protected
// GetFilterPixelType
//-----------------------------------------------------------------------------
inline const HFCPtr<HRPPixelType>& HRPTypedFilter::GetFilterPixelType() const
    {
    return(m_pFilterPixelType);
    }

//-----------------------------------------------------------------------------
// protected
// GetInputConverter
//-----------------------------------------------------------------------------
inline const HFCPtr<HRPPixelConverter>& HRPTypedFilter::GetInputConverter() const
    {
    return(m_pInputConverter);
    }

//-----------------------------------------------------------------------------
// protected
// GetInOutConverter
//-----------------------------------------------------------------------------
inline const HFCPtr<HRPPixelConverter>& HRPTypedFilter::GetInOutConverter() const
    {
    return(m_pInOutConverter);
    }

//-----------------------------------------------------------------------------
// protected
// GetLostChannelsMask
//-----------------------------------------------------------------------------
inline const bool* HRPTypedFilter::GetLostChannelsMask() const
    {
    return(m_CopyLostChannelsMask);
    }

//-----------------------------------------------------------------------------
// protected
// GetOutputConverter
//-----------------------------------------------------------------------------
inline const HFCPtr<HRPPixelConverter>& HRPTypedFilter::GetOutputConverter() const
    {
    return(m_pOutputConverter);
    }
END_IMAGEPP_NAMESPACE
