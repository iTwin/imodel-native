//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPLigthnessContrastStretch8.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

BEGIN_IMAGEPP_NAMESPACE
class HRPLigthnessContrastStretch8 : public HRPLigthnessContrastStretch
    {
    HDECLARE_CLASS_ID(HRPFilterId_LigthnessContrastStretch8, HRPLigthnessContrastStretch)
    

public:

    // Primary methods
    IMAGEPP_EXPORT                HRPLigthnessContrastStretch8();
    IMAGEPP_EXPORT                HRPLigthnessContrastStretch8(const HFCPtr<HRPPixelType>& pi_pFilterPixelType);
    IMAGEPP_EXPORT                HRPLigthnessContrastStretch8(const HRPLigthnessContrastStretch8& pi_rSrcFilter);

    IMAGEPP_EXPORT virtual        ~HRPLigthnessContrastStretch8();

    // Cloning
    virtual HRPFilter* Clone() const override;

private:
    // Disabled method.
    HRPLigthnessContrastStretch8& operator = (const HRPLigthnessContrastStretch8& pi_rFilter);

    // members
    };
END_IMAGEPP_NAMESPACE






