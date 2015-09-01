//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPLigthnessContrastStretch16.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRPFunctionFilters
//-----------------------------------------------------------------------------
// Some common function filters.
//-----------------------------------------------------------------------------
#pragma once

#include "HRPLigthnessContrastStretch.h"

BEGIN_IMAGEPP_NAMESPACE
// Forward declaration.
class HGFLuvColorSpace;

//-----------------------------------------------------------------------------
// HRPColortwistFilter
//-----------------------------------------------------------------------------

class HRPLigthnessContrastStretch16 : public HRPLigthnessContrastStretch
    {
    HDECLARE_CLASS_ID(HRPFilterId_LigthnessContrastStretch16, HRPLigthnessContrastStretch)
    

public:

    // Primary methods
    // For HPM_REGISTER_CLASS ONLY!
    IMAGEPP_EXPORT                HRPLigthnessContrastStretch16();
    IMAGEPP_EXPORT                HRPLigthnessContrastStretch16(const HFCPtr<HRPPixelType>& pi_pFilterPixelType);
    IMAGEPP_EXPORT                HRPLigthnessContrastStretch16(const HRPLigthnessContrastStretch16& pi_rSrcFilter);

    IMAGEPP_EXPORT virtual        ~HRPLigthnessContrastStretch16();

    // Cloning
    virtual HRPFilter* Clone() const override;

private:
    // Disabled method.
    HRPLigthnessContrastStretch16& operator = (const HRPLigthnessContrastStretch16& pi_rFilter);
    };
END_IMAGEPP_NAMESPACE






