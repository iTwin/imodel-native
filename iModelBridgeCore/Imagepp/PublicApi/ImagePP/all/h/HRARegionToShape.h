//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRARegionToShape.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRARegionToShape
//-----------------------------------------------------------------------------
// This class describes any graphical object of raster type (graphics that
// are composed of pixels).  Abstract class.
//-----------------------------------------------------------------------------

#pragma once

#include "HVEShape.h"
#include "HGSRegion.h"

class HRARegionToShape
    {
    HDECLARE_BASECLASS_ID(1737)

public:

    // Primary methods

    HRARegionToShape();
    virtual         ~HRARegionToShape();

    HVEShape*       CreateShape(const HGSRegion* pi_pRegion,
                                const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;
    };

