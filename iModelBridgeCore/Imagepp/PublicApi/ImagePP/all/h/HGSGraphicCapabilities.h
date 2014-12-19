//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSGraphicCapabilities.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGSGraphicCapabilities
//-----------------------------------------------------------------------------
// General class for Graphics.
//-----------------------------------------------------------------------------

#pragma once

#include "HGSGraphicCapability.h"

class HNOVTABLEINIT HGSGraphicCapabilities
    {
    HDECLARE_SEALEDCLASS_ID(1767)

public:

    // Primary methods
    HGSGraphicCapabilities();
    HGSGraphicCapabilities(const HGSGraphicCapabilities& pi_rObj);
    HGSGraphicCapabilities(const HGSGraphicCapability* pi_pCapability);
    ~HGSGraphicCapabilities();

    // Capabilities editing
    void            Add(const HGSGraphicCapability* pi_pCapability);

    // Tests the Capabilities
    bool           Supports(const HGSGraphicCapability* pi_pCapability) const;
    bool           Supports(const HGSGraphicCapabilities& pi_rCapabilities) const;

protected:

private:

    // list of Capabilities
    typedef list<HFCPtr<HGSGraphicCapability>, allocator<HFCPtr<HGSGraphicCapability> > >
    Capabilities;

    // private members
    Capabilities      m_Capabilities;

    // disabled methods
    HGSGraphicCapabilities&
    operator=(const HGSGraphicCapabilities& pi_rObj);
    bool             operator==(const HGSGraphicCapabilities& pi_rObj) const;
    bool             operator!=(const HGSGraphicCapabilities& pi_rObj) const;
    };

#include "HGSGraphicCapabilities.hpp"

