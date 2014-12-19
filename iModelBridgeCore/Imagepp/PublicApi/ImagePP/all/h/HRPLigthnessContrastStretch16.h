//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPLigthnessContrastStretch16.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPFunctionFilters
//-----------------------------------------------------------------------------
// Some common function filters.
//-----------------------------------------------------------------------------
#pragma once

#include "HRPLigthnessContrastStretch.h"

// Forward declaration.
class HGFLuvColorSpace;

//-----------------------------------------------------------------------------
// HRPColortwistFilter
//-----------------------------------------------------------------------------

class HRPLigthnessContrastStretch16 : public HRPLigthnessContrastStretch
    {
    HDECLARE_CLASS_ID(1353, HRPLigthnessContrastStretch)
    

public:

    // Primary methods
    // For HPM_REGISTER_CLASS ONLY!
    _HDLLg                HRPLigthnessContrastStretch16();
    _HDLLg                HRPLigthnessContrastStretch16(const HFCPtr<HRPPixelType>& pi_pFilterPixelType);
    _HDLLg                HRPLigthnessContrastStretch16(const HRPLigthnessContrastStretch16& pi_rSrcFilter);

    _HDLLg virtual        ~HRPLigthnessContrastStretch16();

    // Cloning
    virtual HRPFilter* Clone() const override;

private:
    // Disabled method.
    HRPLigthnessContrastStretch16& operator = (const HRPLigthnessContrastStretch16& pi_rFilter);
    };






