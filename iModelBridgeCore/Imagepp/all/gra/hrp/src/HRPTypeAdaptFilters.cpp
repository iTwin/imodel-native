//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPTypeAdaptFilters.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRPTypeAdaptFilters
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPTypeAdaptFilters.h>

#include <Imagepp/all/h/HRPConvFiltersV24R8G8B8.h>
#include <Imagepp/all/h/HRPConvFiltersV24PhotoYCC.h>

//-----------------------------------------------------------------------------
// Blur filter
//-----------------------------------------------------------------------------
HRPBlurAdaptFilter::HRPBlurAdaptFilter()
    : HRPTypeAdaptFilter()
    {
    m_Intensity = 0;
    }

HRPBlurAdaptFilter::HRPBlurAdaptFilter(Byte pi_Intensity)
    : HRPTypeAdaptFilter(HRPPixelNeighbourhood(5, 5, 2, 2))
    {
    m_Intensity = pi_Intensity;

    HRPBlurFilter FilterValue(pi_Intensity);
    Insert(&FilterValue);
    HRPBlurFilterV24PhotoYCC FilterYCCValue(pi_Intensity);
    Insert(&FilterYCCValue);
    }

HRPBlurAdaptFilter::~HRPBlurAdaptFilter()
    {
    }

Byte HRPBlurAdaptFilter::GetIntensity() const
    {
    return m_Intensity;
    }

//-----------------------------------------------------------------------------
// Sharpen filter
//-----------------------------------------------------------------------------
HRPSharpenAdaptFilter::HRPSharpenAdaptFilter()
    : HRPTypeAdaptFilter()
    {
    m_Intensity = 0;
    }

HRPSharpenAdaptFilter::HRPSharpenAdaptFilter(Byte pi_Intensity)
    : HRPTypeAdaptFilter(HRPPixelNeighbourhood(3, 3, 1, 1))
    {
    m_Intensity = pi_Intensity;

    HRPSharpenFilter FilterValue(pi_Intensity);
    Insert(&FilterValue);
    HRPSharpenFilterV24PhotoYCC FilterYCCValue(pi_Intensity);
    Insert(&FilterYCCValue);
    }

HRPSharpenAdaptFilter::~HRPSharpenAdaptFilter()
    {
    }

Byte HRPSharpenAdaptFilter::GetIntensity() const
    {
    return m_Intensity;
    }