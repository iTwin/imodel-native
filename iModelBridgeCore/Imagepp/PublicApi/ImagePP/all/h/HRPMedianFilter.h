//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPMedianFilter.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPMedianFilter
//-----------------------------------------------------------------------------
#pragma once

#include "HRPConvFilter.h"

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
class HRPMedianFilter : public HRPConvFilter
    {
    HDECLARE_CLASS_ID(HRPFilterId_Median, HRPConvFilter)

public:

    // Primary methods

    virtual         ~HRPMedianFilter();

    bool           IsAConvolutionFilter() const;


    virtual void    SetNeighbourhood(   uint32_t pi_Width,
                                        uint32_t pi_Height,
                                        uint32_t pi_XOrigin,
                                        uint32_t pi_YOrigin);
protected:
    HRPMedianFilter (const HFCPtr<HRPPixelType>& pi_pFilterPixelType);
    HRPMedianFilter (const HFCPtr<HRPPixelType>& pi_pFilterPixelType, const HRPPixelNeighbourhood& pi_rNeighbourhood);
    HRPMedianFilter(const HRPMedianFilter& pi_rFilter);


private:
    HRPMedianFilter& operator = (const HRPMedianFilter& pi_rFilter);

    };
END_IMAGEPP_NAMESPACE

