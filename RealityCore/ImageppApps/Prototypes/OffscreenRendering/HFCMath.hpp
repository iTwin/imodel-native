//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Prototypes/OffscreenRendering/HFCMath.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Class : HFCMath
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

#include <limits.h>

/** -----------------------------------------------------------------------------
    DivideBy255
    -----------------------------------------------------------------------------
*/
inline int32_t HFCMath::DivideBy255(int32_t pi_Numerator) const
    {
    assert (pi_Numerator <= USHRT_MAX && pi_Numerator >= -USHRT_MAX);

    if (pi_Numerator >= 0)
        return m_QuotientsShort[pi_Numerator];
    else
        return -m_QuotientsShort[-pi_Numerator];
    }


/** -----------------------------------------------------------------------------
    DivideBy255
    -----------------------------------------------------------------------------
*/
inline Byte HFCMath::UnsignedDivideBy255 (unsigned short pi_Numerator) const
    {
    return m_QuotientsShort[pi_Numerator];
    }

/** -----------------------------------------------------------------------------
    MultiplyBy0X01010101
    -----------------------------------------------------------------------------
*/
inline uint32_t HFCMath::MultiplyBy0X01010101 (Byte pi_Value) const
    {
    return m_MultiBy0X01010101[pi_Value];
    }