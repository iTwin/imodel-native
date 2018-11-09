//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPLigthnessDensitySlicingFilter8.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPLigthnessDensitySlicingFilter8
//-----------------------------------------------------------------------------
// A contrast stretch filter for 8bits multiple channels pixel types
//-----------------------------------------------------------------------------
#pragma once

#include "HRPLigthnessDensitySlicingFilter.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPLigthnessDensitySlicingFilter8 : public HRPLigthnessDensitySlicingFilter
    {
    HDECLARE_CLASS_ID(HRPFilterId_LigthnessDensitySlicing8, HRPLigthnessDensitySlicingFilter)
    

public:             // Primary methods
    IMAGEPP_EXPORT          HRPLigthnessDensitySlicingFilter8();
    IMAGEPP_EXPORT          HRPLigthnessDensitySlicingFilter8(const HFCPtr<HRPPixelType>&     pi_pFilterPixelType);
    HRPLigthnessDensitySlicingFilter8(const HRPLigthnessDensitySlicingFilter8& pi_rFilter);

    virtual            ~HRPLigthnessDensitySlicingFilter8();

    // Cloning
    HRPFilter* Clone() const override;

private:
    HRPLigthnessDensitySlicingFilter8&
    operator=(const HRPLigthnessDensitySlicingFilter8& pi_rFilter);
    void            DeepCopy(const HRPLigthnessDensitySlicingFilter8& pi_rSrc);
    };
END_IMAGEPP_NAMESPACE


