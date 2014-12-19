//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRAGenBlitter.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRAGenBlitter
//-----------------------------------------------------------------------------
// General class for GenBlitters.
//-----------------------------------------------------------------------------

#pragma once

#include "HGSSurfaceImplementation.h"

class HNOVTABLEINIT HRAGenBlitter
    {
    HDECLARE_BASECLASS_ID(1745)

public:

    // Primary methods
    HRAGenBlitter();

    virtual         ~HRAGenBlitter();


    virtual const void*
    StretchRun(    double pi_StartPosX,
                   double pi_StartPosY,
                   void* po_pDstBuffer,
                   uint32_t pi_DstPixelCount) = 0;


protected:

private:

    // disabled methods
    HRAGenBlitter(const HRAGenBlitter& pi_rObj);
    HRAGenBlitter&  operator=(const HRAGenBlitter& pi_rObj);
    bool             operator==(const HRAGenBlitter& pi_rObj) const;
    bool             operator!=(const HRAGenBlitter& pi_rObj);
    };
