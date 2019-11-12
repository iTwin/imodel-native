//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPDensitySlicingFilter8
//-----------------------------------------------------------------------------
// A contrast stretch filter for 8bits multiple channels pixel types
//-----------------------------------------------------------------------------
#pragma once

#include "HRPDensitySlicingFilter.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPDensitySlicingFilter8 : public HRPDensitySlicingFilter
    {
    HDECLARE_CLASS_ID(HRPFilterId_DensitySlicing8, HRPDensitySlicingFilter)
    

public:             // Primary methods
    IMAGEPP_EXPORT HRPDensitySlicingFilter8();
    IMAGEPP_EXPORT HRPDensitySlicingFilter8(const HFCPtr<HRPPixelType>&     pi_pFilterPixelType);
    HRPDensitySlicingFilter8(const HRPDensitySlicingFilter8& pi_rFilter);

    virtual            ~HRPDensitySlicingFilter8();

    // Cloning
    HRPFilter* Clone() const override;

private:
    HRPDensitySlicingFilter8&
    operator=(const HRPDensitySlicingFilter8& pi_rFilter);
    void            DeepCopy(const HRPDensitySlicingFilter8& pi_rSrc);
    };
END_IMAGEPP_NAMESPACE


