//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSGraphicToolCapabilities.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGSGraphicToolCapabilities
//-----------------------------------------------------------------------------
// General class for Graphics.
//-----------------------------------------------------------------------------
#pragma once

#include "HGSGraphicToolCapability.h"
#include "HFCCapabilities.h"

class HGSGraphicToolCapabilities : public HFCCapabilities
    {
    HDECLARE_CLASS_ID(1703, HFCCapabilities)

public:

    // Primary methods
    _HDLLg                 HGSGraphicToolCapabilities();
    _HDLLg                 HGSGraphicToolCapabilities(const HGSGraphicToolCapabilities& pi_rObj);
    _HDLLg                 HGSGraphicToolCapabilities(const HGSGraphicToolCapability* pi_pCapability);
    _HDLLg virtual         ~HGSGraphicToolCapabilities();


private:

    // disabled methods
    HGSGraphicToolCapabilities&
    operator=(const HGSGraphicToolCapabilities& pi_rObj);
    bool             operator==(const HGSGraphicToolCapabilities& pi_rObj) const;
    bool             operator!=(const HGSGraphicToolCapabilities& pi_rObj) const;
    };

