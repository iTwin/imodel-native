//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecDeflate.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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