//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPLigthnessDensitySlicingFilter16.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPLigthnessDensitySlicingFilter16
//-----------------------------------------------------------------------------
// A contrast stretch filter for 8bits multiple channels pixel types
//-----------------------------------------------------------------------------
#pragma once

#include "HRPLigthnessDensitySlicingFilter.h"

class HRPLigthnessDensitySlicingFilter16 : public HRPLigthnessDensitySlicingFilter
    {
    HDECLARE_CLASS_ID(1362, HRPLigthnessDensitySlicingFilter)
    

public:             // Primary methods
    HRPLigthnessDensitySlicingFilter16();
    HRPLigthnessDensitySlicingFilter16(const HFCPtr<HRPPixelType>&      pi_pFilterPixelType);
    HRPLigthnessDensitySlicingFilter16(const HRPLigthnessDensitySlicingFilter16& pi_rFilter);

    virtual            ~HRPLigthnessDensitySlicingFilter16();

    // Cloning
    HRPFilter* Clone() const override;

private:
    HRPLigthnessDensitySlicingFilter16&
    operator=(const HRPLigthnessDensitySlicingFilter16& pi_rFilter);
    void            DeepCopy(const HRPLigthnessDensitySlicingFilter16& pi_rSrc);
    };


