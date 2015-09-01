//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPConvFiltersV24R8G8B8.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Convolution filters
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPConvFiltersV24R8G8B8.h>

int32_t HRPSmoothFilter::WeightMatrix[3][3] = { {1, 1, 1},
        {1, 5, 1},
        {1, 1, 1}
    };

int32_t HRPDetailFilter::WeightMatrix[3][3] =  {    { 0, -1,  0},
        {-1, 10, -1},
        { 0, -1,  0}
    };

int32_t HRPEdgeEnhanceFilter::WeightMatrix[3][3] = {{-1, -1, -1},
        {-1, 10, -1},
        {-1, -1, -1}
    };

int32_t HRPFindEdgesFilter::WeightMatrix[3][3] =   {{-1, -1, -1},
        {-1,  8, -1},
        {-1, -1, -1}
    };

int32_t HRPAverageFilter::WeightMatrix[2][2] = { {1, 1},
        {1, 1}
    };

//-----------------------------------------------------------------------------
// Average Filter
//-----------------------------------------------------------------------------

HRPAverageFilter::HRPAverageFilter()
    :HRPConvFilterV24R8G8B8(HRPPixelNeighbourhood(2, 2, 0, 0), (int32_t*)WeightMatrix, 4)
    {
    }

HRPAverageFilter::HRPAverageFilter(const HRPAverageFilter& pi_rFilter)
    :HRPConvFilterV24R8G8B8(pi_rFilter)
    {
    }

HRPAverageFilter::~HRPAverageFilter()
    {
    }

HRPFilter* HRPAverageFilter::Clone() const
    {
    return new HRPAverageFilter(*this);
    }

//-----------------------------------------------------------------------------
// Smooth Filter
//-----------------------------------------------------------------------------

HRPSmoothFilter::HRPSmoothFilter()
    :HRPConvFilterV24R8G8B8(HRPPixelNeighbourhood(3, 3, 1, 1), (int32_t*)WeightMatrix,13)
    {
    }

HRPSmoothFilter::HRPSmoothFilter(const HRPSmoothFilter& pi_rFilter)
    :HRPConvFilterV24R8G8B8(pi_rFilter)
    {
    }

HRPSmoothFilter::~HRPSmoothFilter()
    {
    }

HRPFilter* HRPSmoothFilter::Clone() const
    {
    return new HRPSmoothFilter(*this);
    }

//-----------------------------------------------------------------------------
// Blur Filter
//-----------------------------------------------------------------------------

HRPBlurFilter::HRPBlurFilter()
    : HRPConvFilterV24R8G8B8(HRPPixelNeighbourhood(5, 5, 2, 2))
    {
    m_Intensity = 0;
    }

HRPBlurFilter::HRPBlurFilter(Byte pi_Intensity)
    : HRPConvFilterV24R8G8B8(HRPPixelNeighbourhood(5, 5, 2, 2))
    {
    SetIntensity (pi_Intensity);
    }

HRPBlurFilter::HRPBlurFilter(const HRPBlurFilter& pi_rFilter)
    :HRPConvFilterV24R8G8B8(pi_rFilter)
    {
    m_Intensity = pi_rFilter.m_Intensity;
    }

HRPBlurFilter::~HRPBlurFilter()
    {
    }

Byte HRPBlurFilter::GetIntensity() const
    {
    return m_Intensity;
    }

void HRPBlurFilter::SetIntensity(Byte pi_Intensity)
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

HRPFilter* HRPBlurFilter::Clone() const
    {
    return new HRPBlurFilter(*this);
    }

//-----------------------------------------------------------------------------
// Sharpen Filter
//-----------------------------------------------------------------------------

HRPSharpenFilter::HRPSharpenFilter()
    : HRPConvFilterV24R8G8B8(HRPPixelNeighbourhood(3, 3, 1, 1))
    {
    m_Intensity = 0;
    }

HRPSharpenFilter::HRPSharpenFilter(Byte pi_Intensity)
    : HRPConvFilterV24R8G8B8(HRPPixelNeighbourhood(3, 3, 1, 1))
    {
    SetIntensity (pi_Intensity);
    }

HRPSharpenFilter::HRPSharpenFilter(const HRPSharpenFilter& pi_rFilter)
    :HRPConvFilterV24R8G8B8(pi_rFilter)
    {
    m_Intensity = pi_rFilter.m_Intensity;
    }

HRPSharpenFilter::~HRPSharpenFilter()
    {
    }

Byte HRPSharpenFilter::GetIntensity() const
    {
    return m_Intensity;
    }

void HRPSharpenFilter::SetIntensity(Byte pi_Intensity)
    {
    m_Intensity = pi_Intensity;

    int32_t WeightMatrix[3][3];

    for(unsigned short Y = 0; Y < 3; Y++)
        for(unsigned short X = 0; X < 3; X++)
            WeightMatrix[Y][X] = -2;

    // we can see "sharpen" effect with a center weight of 48 to 17 (observation)
    WeightMatrix[1][1] = 31 - pi_Intensity * 31 / 255 + 17;

    SetWeightMatrix((int32_t*)WeightMatrix, WeightMatrix[1][1] - 16);
    }

HRPFilter* HRPSharpenFilter::Clone() const
    {
    return new HRPSharpenFilter(*this);
    }

//-----------------------------------------------------------------------------
// Detail Filter
//-----------------------------------------------------------------------------

HRPDetailFilter::HRPDetailFilter()
    : HRPConvFilterV24R8G8B8(HRPPixelNeighbourhood(3, 3, 1, 1), (int32_t*)WeightMatrix,6)
    {
    }

HRPDetailFilter::HRPDetailFilter(const HRPDetailFilter& pi_rFilter)
    :HRPConvFilterV24R8G8B8(pi_rFilter)
    {
    }

HRPDetailFilter::~HRPDetailFilter()
    {
    }

HRPFilter* HRPDetailFilter::Clone() const
    {
    return new HRPDetailFilter(*this);
    }

//-----------------------------------------------------------------------------
// Edge Enhance Filter
//-----------------------------------------------------------------------------

HRPEdgeEnhanceFilter::HRPEdgeEnhanceFilter()
    : HRPConvFilterV24R8G8B8(HRPPixelNeighbourhood(3, 3, 1, 1), (int32_t*)WeightMatrix, 2)
    {
    }

HRPEdgeEnhanceFilter::HRPEdgeEnhanceFilter(const HRPEdgeEnhanceFilter& pi_rFilter)
    :HRPConvFilterV24R8G8B8(pi_rFilter)
    {
    }

HRPEdgeEnhanceFilter::~HRPEdgeEnhanceFilter()
    {
    }

HRPFilter* HRPEdgeEnhanceFilter::Clone() const
    {
    return new HRPEdgeEnhanceFilter(*this);
    }

//-----------------------------------------------------------------------------
// Find Edges Filter
//-----------------------------------------------------------------------------

HRPFindEdgesFilter::HRPFindEdgesFilter()
    : HRPConvFilterV24R8G8B8(HRPPixelNeighbourhood(3, 3, 1, 1),(int32_t*)WeightMatrix,1)
    {
    }

HRPFindEdgesFilter::HRPFindEdgesFilter(const HRPFindEdgesFilter& pi_rFilter)
    :HRPConvFilterV24R8G8B8(pi_rFilter)
    {
    }

HRPFindEdgesFilter::~HRPFindEdgesFilter()
    {
    }

HRPFilter* HRPFindEdgesFilter::Clone() const
    {
    return new HRPFindEdgesFilter(*this);
    }
