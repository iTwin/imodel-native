//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPAlphaRange
//-----------------------------------------------------------------------------
// Inline methods for HRPAlphaRange.
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// public
// IsIn
//-----------------------------------------------------------------------------
inline bool HRPAlphaRange::IsIn(Byte pi_Red, Byte pi_Green, Byte pi_Blue) const
    {
    return m_pSet->IsIn(pi_Red, pi_Green, pi_Blue);
    }

//-----------------------------------------------------------------------------
// public
// GetAlphaValue
//-----------------------------------------------------------------------------
inline Byte HRPAlphaRange::GetAlphaValue() const
    {
    return m_AlphaValue;
    }
END_IMAGEPP_NAMESPACE
