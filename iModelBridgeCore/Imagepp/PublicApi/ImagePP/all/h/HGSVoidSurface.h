//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSVoidSurface.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:> Class : HGSVoidSurface
//:>-----------------------------------------------------------------------------
#pragma once

#include "HGSSurfaceImplementation.h"
#include "HGSSurfaceImplementationCreator.h"
#include "HGSSurfaceDescriptor.h"
#include "HGSMacros.h"


HGS_DECLARE_SURFACE_DLL(_HDLLg, HGSVoidSurface)

class HGSVoidSurface : public HGSSurfaceImplementation
    {
    HDECLARE_CLASS_ID(1725, HGSSurfaceImplementation)

    HGS_DECLARE_SURFACECAPABILITIES()

public:

    // Primary methods
    HGSVoidSurface(const HFCPtr<HGSSurfaceDescriptor>&  pi_rpDescriptor);
    virtual                 ~HGSVoidSurface();


protected:

    // Primary methos

private:



    // disabled methods
    HGSVoidSurface(const HGSVoidSurface& pi_rObj);
    HGSVoidSurface&     operator=(const HGSVoidSurface& pi_rObj);
    };

#include "HGSVoidSurface.hpp"

