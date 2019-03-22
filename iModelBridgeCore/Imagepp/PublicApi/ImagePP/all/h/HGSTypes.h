//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGSTypes.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGSTypes
//-----------------------------------------------------------------------------
// This is the type used by the HGS
//-----------------------------------------------------------------------------

#pragma once

BEGIN_IMAGEPP_NAMESPACE


class HGSResampling
    {
    HDECLARE_SEALEDCLASS_ID(HGSResamplingId_Base)

public:

    // IMPORTANT: The neighborhood size must be added in the
    // following GetNeighborhoodSize method.
    enum ResamplingMethod
        {
        NEAREST_NEIGHBOUR=0,
        AVERAGE=1,
        VECTOR_AWARENESS=2,
        CUBIC_CONVOLUTION=3,
        HMR_CUBIC_CONVOLUTION=4,
        DESCARTES_CUBIC_CONVOLUTION=5,
        UNDEFINED=6,
        ORING4=7,
        NONE=8,
        BILINEAR=9
        };

    HGSResampling(ResamplingMethod pi_ResamplingMethod);
    HGSResampling(const HGSResampling& pi_rObj);
    ~HGSResampling();

    HGSResampling&  operator=(const HGSResampling& pi_rObj);
    bool           operator==(const HGSResampling& pi_rObj) const;
    bool           operator!=(const HGSResampling& pi_rObj) const;

    bool           Supports(const HGSResampling& pi_rObj) const;

    HGSResampling::ResamplingMethod
    GetResamplingMethod() const;

    /** ---------------------------------------------------------------------------
        Retrieve the needed pixels on each side of a sample for the
        current resampling mode. Coded here to remind that the size must
        be added to the array when a new method is added to the enum.
        ---------------------------------------------------------------------------
    */
    uint32_t        GetNeighborhoodSize() const
        {
        static uint32_t aSizes[] = {0,  // NEAREST_NEIGHBOUR
                                  0,  // AVERAGE
                                  0,  // VECTOR_AWARENESS
                                  2,  // CUBIC_CONVOLUTION
                                  2,  // HMR_CUBIC_CONVOLUTION
                                  2,  // DESCARTES_CUBIC_CONVOLUTION
                                  0,  // UNDEFINED
                                  0,  // ORING4
                                  0,  // NONE
                                  1
                                 }; // BILINEAR

        return aSizes[m_ResamplingMethod];
        };

private:

    // members
    ResamplingMethod    m_ResamplingMethod;

    // disabled method
    HGSResampling();
    };

END_IMAGEPP_NAMESPACE

#include "HGSTypes.hpp"

