//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HRPPixelConverter
//-----------------------------------------------------------------------------


BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// GetDestinationPixelType
//-----------------------------------------------------------------------------
inline const HRPPixelType* HRPPixelConverter::GetDestinationPixelType () const
    {
    return (m_pDestPixelType);
    }

//-----------------------------------------------------------------------------
// GetSourcePixelType
//-----------------------------------------------------------------------------
inline const HRPPixelType* HRPPixelConverter::GetSourcePixelType      () const
    {
    return (m_pSourcePixelType);
    }

//-----------------------------------------------------------------------------
// GetLostChannels
//-----------------------------------------------------------------------------
inline const int16_t* HRPPixelConverter::GetLostChannels() const
    {
    return 0;
    }
END_IMAGEPP_NAMESPACE
