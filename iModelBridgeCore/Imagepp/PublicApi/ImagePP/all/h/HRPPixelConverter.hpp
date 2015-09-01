//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelConverter.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
inline const short* HRPPixelConverter::GetLostChannels() const
    {
    return 0;
    }
END_IMAGEPP_NAMESPACE
