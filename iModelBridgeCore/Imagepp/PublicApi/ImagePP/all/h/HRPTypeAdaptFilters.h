//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPTypeAdaptFilters.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
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

class HRPBlurAdaptFilter : public HRPTypeAdaptFilter
    {
    HDECLARE_CLASS_ID(1168, HRPTypeAdaptFilter)

public:

    _HDLLg             HRPBlurAdaptFilter();
    _HDLLg             HRPBlurAdaptFilter(Byte pi_Intensity);
    _HDLLg             ~HRPBlurAdaptFilter();

    _HDLLg Byte      GetIntensity() const;

private:

    Byte      m_Intensity;
    };

//-----------------------------------------------------------------------------
// Sharpen filter
//-----------------------------------------------------------------------------

class HRPSharpenAdaptFilter : public HRPTypeAdaptFilter
    {
    HDECLARE_CLASS_ID(1168, HRPTypeAdaptFilter)

public:

    _HDLLg             HRPSharpenAdaptFilter();
    _HDLLg             HRPSharpenAdaptFilter(Byte pi_Intensity);
    _HDLLg             ~HRPSharpenAdaptFilter();

    _HDLLg Byte      GetIntensity() const;

private:

    Byte      m_Intensity;
    };

