//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecIJG12Bits.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCDCodecIJG12Bits
//-----------------------------------------------------------------------------
// IJG codec lass.
//-----------------------------------------------------------------------------
#pragma once

#define JPEGLIB_SUPPORT_12BITS
#ifdef IJG12BITS
#undef IJG12BITS
#endif
#define IJG12BITS(x) x##_12bits

#ifdef HCDCodecIJG8Bits_H
#undef HCDCodecIJG8Bits_H
#endif

#include "HCDCodecIJG8Bits.h"

#undef JPEGLIB_SUPPORT_12BITS
