//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSGraphicCapability.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGSGraphicCapability
//-----------------------------------------------------------------------------
// General class for Graphics.
//-----------------------------------------------------------------------------

#pragma once

#include "HFCPtr.h"

class HGSGraphicCapability : public HFCShareableObject<HGSGraphicCapability>
    {
    HDECLARE_BASECLASS_ID(1766)

public:

    virtual         ~HGSGraphicCapability();

    virtual HGSGraphicCapability*
    Clone() const = 0;

    virtual bool   IsSameAs(const HGSGraphicCapability* pi_pCapability) const = 0;

protected:

    HGSGraphicCapability();
    HGSGraphicCapability(const HGSGraphicCapability& pi_rObj);

    HGSGraphicCapability&
    operator=(const HGSGraphicCapability& pi_rObj);
    bool           operator==(const HGSGraphicCapability& pi_rObj) const;

private:

    // disabled methods
    bool           operator!=(const HGSGraphicCapability& pi_rObj);
    };

