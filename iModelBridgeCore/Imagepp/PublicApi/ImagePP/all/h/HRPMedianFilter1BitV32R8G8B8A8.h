//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPMedianFilter1BitV32R8G8B8A8.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPMedianFilter1BitV32R8G8B8A8
//-----------------------------------------------------------------------------
#pragma once

#include "HRPMedianFilter.h"

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
class HRPMedianFilter1BitV32R8G8B8A8 : public HRPMedianFilter
    {
    HDECLARE_CLASS_ID(HRPFilterId_Median1BitV32R8G8B8A8, HRPMedianFilter)
    

public:

    // Primary methods
    IMAGEPP_EXPORT HRPMedianFilter1BitV32R8G8B8A8();
    HRPMedianFilter1BitV32R8G8B8A8(const HRPPixelNeighbourhood& pi_rNeighbourhood);

    virtual         ~HRPMedianFilter1BitV32R8G8B8A8();

    // Cloning
    virtual HRPFilter* Clone() const override;

private:
    HRPMedianFilter1BitV32R8G8B8A8(const HRPMedianFilter1BitV32R8G8B8A8& pi_rFilter);
    HRPMedianFilter1BitV32R8G8B8A8& operator = (const HRPMedianFilter1BitV32R8G8B8A8& pi_rFilter);

    // Convolution method
    void            Convoluate(const void*      pi_pSrcRawData[],
                               void*            pi_pDestRawData,
                               uint32_t         pi_Width,
                               const double*   pi_pPositionsX,
                               const double*   pi_pPositionsY) const;


    };
END_IMAGEPP_NAMESPACE


