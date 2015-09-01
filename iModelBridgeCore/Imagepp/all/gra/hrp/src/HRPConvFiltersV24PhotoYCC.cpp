//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPConvFiltersV24PhotoYCC.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Convolution filters
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPConvFiltersV24PhotoYCC.h>

//-----------------------------------------------------------------------------
// Blur filter
//-----------------------------------------------------------------------------

HRPBlurFilterV24PhotoYCC::HRPBlurFilterV24PhotoYCC()
    {
    m_Intensity = 0;
    }

HRPBlurFilterV24PhotoYCC::HRPBlurFilterV24PhotoYCC(Byte pi_Intensity)
    : HRPConvFilterV24PhotoYCC(HRPPixelNeighbourhood(5, 5, 2, 2))
    {
    m_Intensity = pi_Intensity;

    int32_t WeightMatrix[5][5];

    for(unsigned short Y = 0; Y < 5; Y++)
        {
        for(unsigned short X = 0; X < 5; X++)
            {
            if(X == 0 || X == 4 || Y == 0 || Y == 4)
                WeightMatrix[Y][X] = 1;
            else
                WeightMatrix[Y][X] = 0;
            }
        }

    // we can see "blur" effect with a center weight of 31 to 0 (observation)
    WeightMatrix[2][2] = 31 - pi_Intensity * 31 / 255;

    SetWeightMatrix((int32_t*)WeightMatrix, 16 + WeightMatrix[2][2]);
    }

HRPBlurFilterV24PhotoYCC::HRPBlurFilterV24PhotoYCC(const HRPBlurFilterV24PhotoYCC& pi_rFilter)
    :HRPConvFilterV24PhotoYCC(pi_rFilter)
    {
    m_Intensity = pi_rFilter.m_Intensity;
    }

HRPBlurFilterV24PhotoYCC::~HRPBlurFilterV24PhotoYCC()
    {
    }

Byte HRPBlurFilterV24PhotoYCC::GetIntensity() const
    {
    return m_Intensity;
    }

HRPFilter* HRPBlurFilterV24PhotoYCC::Clone() const
    {
    return new HRPBlurFilterV24PhotoYCC(*this);
    }

//-----------------------------------------------------------------------------
// Sharpen filter
//-----------------------------------------------------------------------------

HRPSharpenFilterV24PhotoYCC::HRPSharpenFilterV24PhotoYCC()
    {
    m_Intensity = 0;
    }

HRPSharpenFilterV24PhotoYCC::HRPSharpenFilterV24PhotoYCC(Byte pi_Intensity)
    : HRPConvFilterV24PhotoYCC(HRPPixelNeighbourhood(3, 3, 1, 1))
    {
    m_Intensity = pi_Intensity;

    int32_t WeightMatrix[3][3];

    for(unsigned short Y = 0; Y < 3; Y++)
        for(unsigned short X = 0; X < 3; X++)
            WeightMatrix[Y][X] = -2;

    // we can see "shrapen" effect with a center weight of 48 to 17 (observation)
    WeightMatrix[1][1] = 31 - pi_Intensity * 31 / 255 + 17;

    SetWeightMatrix((int32_t*)WeightMatrix, WeightMatrix[1][1] - 16);
    }

HRPSharpenFilterV24PhotoYCC::HRPSharpenFilterV24PhotoYCC(const HRPSharpenFilterV24PhotoYCC& pi_rFilter)
    :HRPConvFilterV24PhotoYCC(pi_rFilter)
    {
    m_Intensity = pi_rFilter.m_Intensity;
    }

HRPSharpenFilterV24PhotoYCC::~HRPSharpenFilterV24PhotoYCC()
    {
    }

Byte HRPSharpenFilterV24PhotoYCC::GetIntensity() const
    {
    return m_Intensity;
    }

HRPFilter* HRPSharpenFilterV24PhotoYCC::Clone() const
    {
    return new HRPSharpenFilterV24PhotoYCC(*this);
    }
