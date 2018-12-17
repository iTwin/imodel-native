//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPCustomConvFilter.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPCustomConvFilter
//-----------------------------------------------------------------------------
// This class describes a user defined convolution filter.
//-----------------------------------------------------------------------------
#pragma once

#include "HRPConvFilterV24R8G8B8.h"

BEGIN_IMAGEPP_NAMESPACE
class HRPCustomConvFilter : public HRPConvFilterV24R8G8B8
    {
    HDECLARE_CLASS_ID(HRPFilterId_CustomConv, HRPConvFilterV24R8G8B8)
    
public:
    IMAGEPP_EXPORT             HRPCustomConvFilter();

    IMAGEPP_EXPORT             HRPCustomConvFilter(uint32_t pi_MatrixWidth,
                                           uint32_t pi_MatrixHeigth,
                                           uint32_t pi_XOrigin,
                                           uint32_t pi_YOrigin,
                                           int32_t* pi_pWeightMatrix);
    IMAGEPP_EXPORT virtual     ~HRPCustomConvFilter();

    // Cloning
    virtual HRPFilter* Clone() const override;

private:
    HRPCustomConvFilter(const HRPCustomConvFilter& pi_rSrcFilter);


    int32_t    ComputeDivisionFactor(int32_t* pi_pWeightMatrix,
                                      uint32_t pi_MatrixSize);

    uint32_t    m_MatrixWidth;
    uint32_t    m_MatrixHeight;

    int32_t*     m_WeightMatrix;

    // Disable operator=
    HRPCustomConvFilter&
    operator=(const HRPCustomConvFilter& pi_rSrcFilter);
    };
END_IMAGEPP_NAMESPACE


