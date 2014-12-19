//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSGraphicToolCapability.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// General class for Graphic tool capability.
//-----------------------------------------------------------------------------
#pragma once

#include "HFCCapability.h"
#include "HGSGraphicToolAttribute.h"

//-----------------------------------------------------------------------------
// Class : HGSGraphicToolCapability
//-----------------------------------------------------------------------------
class HGSGraphicToolCapability : public HFCCapability
    {
    HDECLARE_CLASS_ID(1716, HFCCapability)

public:

    _HDLLg                 HGSGraphicToolCapability(const HGSGraphicToolAttribute& pi_rAttribute);
    _HDLLg virtual         ~HGSGraphicToolCapability();

    virtual HFCCapability*
    Clone() const;

    virtual bool   IsSameAs(const HFCCapability& pi_rCapability) const;
    virtual bool   Supports(const HFCCapability& pi_rCapability) const;

    const HGSGraphicToolAttribute&
    GetAttribute() const;

private:

    // members
    HAutoPtr<HGSGraphicToolAttribute>   m_pAttribute;

    // disabled methods
    HGSGraphicToolCapability();
    HGSGraphicToolCapability(const HGSGraphicToolCapability&);
    HGSGraphicToolCapability&
    operator=(const HGSGraphicToolCapability& pi_rObj);
    bool             operator==(const HGSGraphicToolCapability& pi_rObj) const;
    bool             operator!=(const HGSGraphicToolCapability& pi_rObj);
    };

