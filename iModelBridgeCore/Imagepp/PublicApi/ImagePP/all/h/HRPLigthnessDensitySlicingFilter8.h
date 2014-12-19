//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPLigthnessDensitySlicingFilter8.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPLigthnessDensitySlicingFilter8
//-----------------------------------------------------------------------------
// A contrast stretch filter for 8bits multiple channels pixel types
//-----------------------------------------------------------------------------
#pragma once

#include "HRPLigthnessDensitySlicingFilter.h"

class HRPLigthnessDensitySlicingFilter8 : public HRPLigthnessDensitySlicingFilter
    {
    HDECLARE_CLASS_ID(1361, HRPLigthnessDensitySlicingFilter)
    

public:             // Primary methods
    HRPLigthnessDensitySlicingFilter8();
    _HDLLg          HRPLigthnessDensitySlicingFilter8(const HFCPtr<HRPPixelType>&     pi_pFilterPixelType);
    HRPLigthnessDensitySlicingFilter8(const HRPLigthnessDensitySlicingFilter8& pi_rFilter);

    virtual            ~HRPLigthnessDensitySlicingFilter8();

    // Cloning
    HRPFilter* Clone() const override;

private:
    HRPLigthnessDensitySlicingFilter8&
    operator=(const HRPLigthnessDensitySlicingFilter8& pi_rFilter);
    void            DeepCopy(const HRPLigthnessDensitySlicingFilter8& pi_rSrc);
    };


