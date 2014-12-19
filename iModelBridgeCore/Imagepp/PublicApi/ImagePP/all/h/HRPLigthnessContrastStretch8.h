//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPLigthnessContrastStretch8.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPFunctionFilters
//-----------------------------------------------------------------------------
// Some common function filters.
//-----------------------------------------------------------------------------
#pragma once

#include "HRPFunctionFilter.h"
#include "HRPLigthnessContrastStretch.h"

//-----------------------------------------------------------------------------
// HRPColortwistFilter
//-----------------------------------------------------------------------------

class HRPLigthnessContrastStretch8 : public HRPLigthnessContrastStretch
    {
    HDECLARE_CLASS_ID(1352, HRPLigthnessContrastStretch)
    

public:

    // Primary methods
    _HDLLg                HRPLigthnessContrastStretch8();
    _HDLLg                HRPLigthnessContrastStretch8(const HFCPtr<HRPPixelType>& pi_pFilterPixelType);
    _HDLLg                HRPLigthnessContrastStretch8(const HRPLigthnessContrastStretch8& pi_rSrcFilter);

    _HDLLg virtual        ~HRPLigthnessContrastStretch8();

    // Cloning
    virtual HRPFilter* Clone() const override;

private:
    // Disabled method.
    HRPLigthnessContrastStretch8& operator = (const HRPLigthnessContrastStretch8& pi_rFilter);

    // members
    };






