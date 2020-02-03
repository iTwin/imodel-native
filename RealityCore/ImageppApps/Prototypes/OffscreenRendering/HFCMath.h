//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Prototypes/OffscreenRendering/HFCMath.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCMath
//----------------------------------------------------------------------------
#pragma once

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

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
    public:    
    static HFCMath& GetInstance()
        {
        static HFCMath s_math;
        return s_math;
        }

private:
    HFCMath()
        {        
        m_QuotientsShort = new Byte[USHRT_MAX+1];
        for (long i=0; i<USHRT_MAX+1; ++i)
            m_QuotientsShort[i] = (Byte)(i / 255);

        m_MultiBy0X01010101 = new uint32_t[256];
        for (long i=0; i<256; ++i)
            m_MultiBy0X01010101[i] = i * 0x01010101;
        }
    ~HFCMath()
        {
        delete [] m_MultiBy0X01010101;
        delete [] m_QuotientsShort;
        }

    // Disabled
    HFCMath(HFCMath const& object);

    // Members
    Byte* m_QuotientsShort;
    uint32_t* m_MultiBy0X01010101;

public:
    // Interface
    inline int32_t DivideBy255(int32_t pi_Numerator) const;
    // Only positive value here.
    inline Byte UnsignedDivideBy255(unsigned short pi_Numerator) const;

    inline uint32_t MultiplyBy0X01010101(Byte pi_Value) const;
    };

#include "HFCMath.hpp"
//----------------------------------------------------------------------------


