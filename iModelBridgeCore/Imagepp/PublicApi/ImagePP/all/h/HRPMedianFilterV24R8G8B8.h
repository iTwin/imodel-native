//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPMedianFilterV24R8G8B8.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPMedianFilterV24R8G8B8
//-----------------------------------------------------------------------------
#pragma once

#include "HRPMedianFilter.h"

BEGIN_IMAGEPP_NAMESPACE
class HGFLuvColorSpace;
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

class HRPMedianFilterV24R8G8B8 : public HRPMedianFilter
    {
    HDECLARE_CLASS_ID(HRPFilterId_MedianV24R8G8B8, HRPMedianFilter)
    

public:

    // Primary methods
    HRPMedianFilterV24R8G8B8();
    HRPMedianFilterV24R8G8B8(const HRPPixelNeighbourhood& pi_rNeighbourhood);

    virtual         ~HRPMedianFilterV24R8G8B8();

    // Cloning
    virtual HRPFilter* Clone() const override;

private:
    HRPMedianFilterV24R8G8B8(const HRPMedianFilterV24R8G8B8& pi_rFilter);
    HRPMedianFilterV24R8G8B8& operator = (const HRPMedianFilterV24R8G8B8& pi_rFilter);

    // Convolution method
    void            Convoluate(const void*      pi_pSrcRawData[],
                               void*            pi_pDestRawData,
                               uint32_t         pi_Width,
                               const double*   pi_pPositionsX,
                               const double*   pi_pPositionsY) const;

    HGFLuvColorSpace* m_pColorSpaceConverter;


    };
END_IMAGEPP_NAMESPACE


