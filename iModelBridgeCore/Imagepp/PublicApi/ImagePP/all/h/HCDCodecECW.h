//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecECW.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HCDCodecECW
//-----------------------------------------------------------------------------
// ECW codec lass.
//-----------------------------------------------------------------------------
#pragma once

#include "HCDCodecErMapperSupported.h"

BEGIN_IMAGEPP_NAMESPACE

class HCDCodecECW : public HCDCodecErMapperSupported
    {
    HDECLARE_CLASS_ID(HCDCodecId_ECW, HCDCodecErMapperSupported)

public:

    // primary methods
    IMAGEPP_EXPORT                 HCDCodecECW();
    IMAGEPP_EXPORT                 HCDCodecECW(const HCDCodecECW& pi_rObj);
    IMAGEPP_EXPORT virtual         ~HCDCodecECW();
    };

END_IMAGEPP_NAMESPACE