//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCMath.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Class : HFCMath
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE

/** -----------------------------------------------------------------------------
    DivideBy255
    -----------------------------------------------------------------------------
*/
inline int32_t HFCMath::DivideBy255(int32_t pi_Numerator) const
    {
    HASSERT (pi_Numerator <= USHRT_MAX && pi_Numerator >= -USHRT_MAX);

    if (pi_Numerator >= 0)
        return m_QuotientsShort[pi_Numerator];
    else
        return -m_QuotientsShort[-pi_Numerator];
    }

inline Byte HFCMath::DivideBy255ToByte(int32_t pi_Numerator) const
    {
    // Do not mask becauss we want to detect "Run-Time Check Failure #1 - A cast to a smaller data type has caused a loss of data."
    // If we have overflows we should evalute the performance impact of doing something like max(0,min(255, result)
    //return CONVERT_TO_BYTE(DivideBy255(pi_Numerator));
    return (Byte)DivideBy255(pi_Numerator);
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

END_IMAGEPP_NAMESPACE