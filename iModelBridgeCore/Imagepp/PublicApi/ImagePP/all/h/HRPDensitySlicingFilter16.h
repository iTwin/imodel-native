//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPDensitySlicingFilter16.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPDensitySlicingFilter16
//-----------------------------------------------------------------------------
// A contrast stretch filter for 8bits multiple channels pixel types
//-----------------------------------------------------------------------------
#pragma once

#include "HRPDensitySlicingFilter.h"

class HRPDensitySlicingFilter16 : public HRPDensitySlicingFilter
    {
    HDECLARE_CLASS_ID(1358, HRPDensitySlicingFilter)
    

public:             // Primary methods
    HRPDensitySlicingFilter16();
    HRPDensitySlicingFilter16(const HFCPtr<HRPPixelType>&      pi_pFilterPixelType);
    HRPDensitySlicingFilter16(const HRPDensitySlicingFilter16& pi_rFilter);

    virtual            ~HRPDensitySlicingFilter16();

    // Cloning
    HRPFilter* Clone() const override;

private:
    HRPDensitySlicingFilter16&
    operator=(const HRPDensitySlicingFilter16& pi_rFilter);
    void            DeepCopy(const HRPDensitySlicingFilter16& pi_rSrc);
    };


