//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSSurfaceCapabilities.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGSSurfaceCapabilities
//-----------------------------------------------------------------------------
#pragma once

#include "HFCCapabilities.h"
#include "HGSSurfaceCapability.h"

class HGSSurfaceCapabilities : public HFCCapabilities
    {
    HDECLARE_CLASS_ID(1701, HFCCapabilities)

public:

    // Primary methods
    _HDLLg                 HGSSurfaceCapabilities();
    _HDLLg                 HGSSurfaceCapabilities(const HGSSurfaceCapabilities& pi_rObj);
    _HDLLg                 HGSSurfaceCapabilities(const HGSSurfaceCapability& pi_rCapability);
    _HDLLg virtual         ~HGSSurfaceCapabilities();


protected:

private:

    // disabled methods
    HGSSurfaceCapabilities&
    operator=(const HGSSurfaceCapabilities& pi_rObj);
    bool             operator==(const HGSSurfaceCapabilities& pi_rObj) const;
    bool             operator!=(const HGSSurfaceCapabilities& pi_rObj) const;
    };

