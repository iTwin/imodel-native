//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRANearestSampler.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>-----------------------------------------------------------------------------
//:> Class : HRANearestSampler
//:>-----------------------------------------------------------------------------

#pragma once

#include "HRAGenericSampler.h"
#include "HGF2DRectangle.h"

class HGSGraphicToolAttributes;
class HGSSurfaceImplementation;

class HNOVTABLEINIT HRANearestSampler : public HRAGenericSampler
    {
    HDECLARE_CLASS_ID(1752, HRAGenericSampler)

public:

    // Primary methods
    HRANearestSampler(HGSMemoryBaseSurfaceDescriptor const& pi_rMemorySurface,
                      const HGF2DRectangle&                 pi_rSampleDimension,
                      double                                pi_DeltaX,
                      double                                pi_DeltaY);
    virtual        ~HRANearestSampler();


protected:

private:

    // disabled methods
    HRANearestSampler();
    HRANearestSampler(const HRANearestSampler& pi_rObj);
    HRANearestSampler&      operator=(const HRANearestSampler& pi_rObj);
    };

