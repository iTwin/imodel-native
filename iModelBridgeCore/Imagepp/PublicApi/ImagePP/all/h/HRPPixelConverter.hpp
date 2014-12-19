//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPixelConverter.hpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HRPPixelConverter
//-----------------------------------------------------------------------------


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