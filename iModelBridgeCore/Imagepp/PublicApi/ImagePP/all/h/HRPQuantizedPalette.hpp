//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPQuantizedPalette.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HRPQuantizedPalette
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Returns the maximum number of entries in the quantized palette
//-----------------------------------------------------------------------------
inline unsigned short HRPQuantizedPalette::GetMaxEntries() const
    {
    return(m_MaxEntries);
    }

END_IMAGEPP_NAMESPACE
