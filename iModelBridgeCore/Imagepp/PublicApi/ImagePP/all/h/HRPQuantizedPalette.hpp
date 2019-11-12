//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HRPQuantizedPalette
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Returns the maximum number of entries in the quantized palette
//-----------------------------------------------------------------------------
inline uint16_t HRPQuantizedPalette::GetMaxEntries() const
    {
    return(m_MaxEntries);
    }

END_IMAGEPP_NAMESPACE
