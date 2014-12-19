//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSSurfaceCapability.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGSSurfaceCapability
//-----------------------------------------------------------------------------
#pragma once

#include "HFCCapability.h"
#include "HGSSurfaceAttribute.h"

class HGSSurfaceCapability : public HFCCapability
    {
    HDECLARE_CLASS_ID(1739, HFCCapability)

public:

    _HDLLg                 HGSSurfaceCapability(const HGSSurfaceAttribute& pi_rAttribute);
    _HDLLg virtual         ~HGSSurfaceCapability();

    virtual HFCCapability*
    Clone() const;

    virtual bool   IsSameAs(const HFCCapability& pi_rCapability) const;
    virtual bool   Supports(const HFCCapability& pi_rCapability) const;

    const HGSSurfaceAttribute&
    GetAttribute() const;

private:

    // members
    HAutoPtr<HGSSurfaceAttribute>   m_pAttribute;

    // disabled methods
    HGSSurfaceCapability(const HGSSurfaceCapability&);
    HGSSurfaceCapability&
    operator=(const HGSSurfaceCapability& pi_rObj);
    bool             operator==(const HGSSurfaceCapability& pi_rObj) const;
    bool             operator!=(const HGSSurfaceCapability& pi_rObj) const;
    };

