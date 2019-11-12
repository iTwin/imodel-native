//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCDCodecDeflate
//-----------------------------------------------------------------------------
// Deflate codec lass.
//-----------------------------------------------------------------------------

#pragma once

#include "HCDCodecVector.h"

BEGIN_IMAGEPP_NAMESPACE

class HCDCodecDeflate : public HCDCodecVector
    {
    HDECLARE_CLASS_ID(HCDCodecId_Deflate, HCDCodecVector)

public:

    ~HCDCodecDeflate();

protected:
    
    HCDCodecDeflate();

    HCDCodecDeflate(size_t pi_DataSize);

    HCDCodecDeflate(const HCDCodecDeflate& pi_rObj);
    };

END_IMAGEPP_NAMESPACE
