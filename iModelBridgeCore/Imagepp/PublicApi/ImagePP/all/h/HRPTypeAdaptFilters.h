//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPTypeAdaptFilters.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPTypeAdaptFilters
//-----------------------------------------------------------------------------
// This class describes TypeAdapt filters.
//-----------------------------------------------------------------------------
#pragma once

#include "HRPTypeAdaptFilter.h"

//-----------------------------------------------------------------------------
// Blur filter
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
class HRPBlurAdaptFilter : public HRPTypeAdaptFilter
    {
    HDECLARE_CLASS_ID(HRPFilterId_BlurAdapt, HRPTypeAdaptFilter)

public:

    IMAGEPP_EXPORT             HRPBlurAdaptFilter();
    IMAGEPP_EXPORT             HRPBlurAdaptFilter(Byte pi_Intensity);
    IMAGEPP_EXPORT             ~HRPBlurAdaptFilter();

    IMAGEPP_EXPORT Byte      GetIntensity() const;

private:

    Byte      m_Intensity;
    };

//-----------------------------------------------------------------------------
// Sharpen filter
//-----------------------------------------------------------------------------

class HRPSharpenAdaptFilter : public HRPTypeAdaptFilter
    {
    HDECLARE_CLASS_ID(HRPFilterId_SharpenAdapt, HRPTypeAdaptFilter)

public:

    IMAGEPP_EXPORT             HRPSharpenAdaptFilter();
    IMAGEPP_EXPORT             HRPSharpenAdaptFilter(Byte pi_Intensity);
    IMAGEPP_EXPORT             ~HRPSharpenAdaptFilter();

    IMAGEPP_EXPORT Byte      GetIntensity() const;

private:

    Byte      m_Intensity;
    };
END_IMAGEPP_NAMESPACE

