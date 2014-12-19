//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCDCodecDeflate.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HCDCodecDeflate
//-----------------------------------------------------------------------------
// Deflate codec lass.
//-----------------------------------------------------------------------------

#pragma once

#include "HCDCodecVector.h"

class HCDCodecDeflate : public HCDCodecVector
    {
    HDECLARE_CLASS_ID(1174, HCDCodecVector)

public:

    ~HCDCodecDeflate();

protected:
    
    HCDCodecDeflate();

    HCDCodecDeflate(size_t pi_DataSize);

    HCDCodecDeflate(const HCDCodecDeflate& pi_rObj);
    };

