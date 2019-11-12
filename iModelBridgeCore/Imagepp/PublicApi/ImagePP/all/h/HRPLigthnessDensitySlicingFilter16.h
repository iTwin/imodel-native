//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPLigthnessDensitySlicingFilter16
//-----------------------------------------------------------------------------
// A contrast stretch filter for 8bits multiple channels pixel types
//-----------------------------------------------------------------------------
#pragma once

#include "HRPLigthnessDensitySlicingFilter.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPLigthnessDensitySlicingFilter16 : public HRPLigthnessDensitySlicingFilter
    {
    HDECLARE_CLASS_ID(HRPFilterId_LigthnessDensitySlicing16, HRPLigthnessDensitySlicingFilter)
    

public:             // Primary methods
    IMAGEPP_EXPORT HRPLigthnessDensitySlicingFilter16();
    IMAGEPP_EXPORT HRPLigthnessDensitySlicingFilter16(const HFCPtr<HRPPixelType>&      pi_pFilterPixelType);
    HRPLigthnessDensitySlicingFilter16(const HRPLigthnessDensitySlicingFilter16& pi_rFilter);

    virtual            ~HRPLigthnessDensitySlicingFilter16();

    // Cloning
    HRPFilter* Clone() const override;

private:
    HRPLigthnessDensitySlicingFilter16&
    operator=(const HRPLigthnessDensitySlicingFilter16& pi_rFilter);
    void            DeepCopy(const HRPLigthnessDensitySlicingFilter16& pi_rSrc);
    };
END_IMAGEPP_NAMESPACE


