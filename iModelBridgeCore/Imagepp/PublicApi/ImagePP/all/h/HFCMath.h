//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCMath.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCMath
//----------------------------------------------------------------------------
#pragma once

//----------------------------------------------------------------------------

#include <Imagepp/all/h/HFCMacros.h>

BEGIN_IMAGEPP_NAMESPACE
//----------------------------------------------------------------------------

#define CONVERT_TO_BYTE(Value)  (0xFF & (Value))

//#define CONVERT_8BIT_TO_16BITx(ubyteValue)  (((UShort)(ubyteValue))  << 8)
//#define CONVERT_8BIT_TO_16BITxx(ubyteValue)  ((((UShort)0|ubyteValue)<<8) |ubyteValue)
#define CONVERT_8BIT_TO_16BIT(ubyteValue)  (((unsigned short)ubyteValue) * 0x0101)
#define CONVERT_16BIT_TO_8BIT(uShortValue) ((Byte)(uShortValue >> 8))

//#define CONVERT_8BIT_TO_32BITx(ubyteValue)   (((UInt32)(ubyteValue)) << 24)
//#define CONVERT_8BIT_TO_32BITxx(ubyteValue)   ((((((((UInt32)0|ubyteValue)<<8) |ubyteValue)<< 8) |ubyteValue)<<8) |ubyteValue)
#define CONVERT_8BIT_TO_32BIT(ubyteValue)   (((uint32_t)ubyteValue) * 0x01010101)
#define CONVERT_32BIT_TO_8BIT(uIntValue)    ((Byte)(uIntValue >> 24))

//#define CONVERT_16BIT_TO_32BITx(uShortValue)  (((UInt32)(uShortValue)) << 16)
//#define CONVERT_16BIT_TO_32BITxx(uShortValue)    ((((UInt32)0|uShortValue)<<16) |uShortValue)
#define CONVERT_16BIT_TO_32BIT(uShortValue)  (((uint32_t)uShortValue) * 0x00010001)
#define CONVERT_32BIT_TO_16BIT(uIntValue)    ((unsigned short)(uIntValue >> 16))

class HFCMath
    {
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HFCMath)

private:
    HFCMath();
    ~HFCMath();

    // Disabled
    HFCMath(HFCMath const& object);

    // Members
    HArrayAutoPtr<Byte> m_QuotientsShort;
    HArrayAutoPtr<uint32_t> m_MultiBy0X01010101;

public:
    // Interface
    int32_t DivideBy255(int32_t pi_Numerator) const;
    Byte DivideBy255ToByte(int32_t pi_Numerator) const;
    // Only positive value here.
    Byte UnsignedDivideBy255(unsigned short pi_Numerator) const;

    uint32_t MultiplyBy0X01010101(Byte pi_Value) const;
    };

END_IMAGEPP_NAMESPACE

#include <Imagepp/all/h/HFCMath.hpp>
//----------------------------------------------------------------------------


