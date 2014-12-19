//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRASurface.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRASurface
//-----------------------------------------------------------------------------
// General class for surfaces.
//-----------------------------------------------------------------------------
#pragma once

#include "HGSSurfaceImplementation.h"

#include "HGSMacros.h"
#include "HGSMemorySurfaceDescriptor.h"
#include "HGSRegion.h"
#include "HRPPixelType.h"

HGS_DECLARE_SURFACE_DLL(_HDLLg, HRASurface)

class HRASurface : public HGSSurfaceImplementation
    {
    HDECLARE_CLASS_ID(1731, HGSSurfaceImplementation)

    HGS_DECLARE_SURFACECAPABILITIES()

public:

    // Primary methods
    _HDLLg                 HRASurface(const HFCPtr<HGSSurfaceDescriptor>&  pi_rpDescriptor);
    _HDLLg virtual         ~HRASurface();

protected:

private:

    // private members

    // disabled methods
    HRASurface(const HRASurface& pi_rObj);
    HRASurface&
    operator=(const HRASurface& pi_rObj);
    };
