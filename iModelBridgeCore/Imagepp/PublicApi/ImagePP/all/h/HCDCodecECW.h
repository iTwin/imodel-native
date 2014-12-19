//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecECW.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCDCodecECW
//-----------------------------------------------------------------------------
// ECW codec lass.
//-----------------------------------------------------------------------------
#pragma once

#include "HCDCodecErMapperSupported.h"

class HCDCodecECW : public HCDCodecErMapperSupported
    {
    HDECLARE_CLASS_ID(1307, HCDCodecErMapperSupported)

public:

    // primary methods
    _HDLLu                 HCDCodecECW();
    _HDLLu                 HCDCodecECW(const HCDCodecECW& pi_rObj);
    _HDLLu virtual         ~HCDCodecECW();
    };