//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRPTypedFilter
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HRPMedianFilter.h>

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// HRPMedianFilter
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPMedianFilter::HRPMedianFilter(const HFCPtr<HRPPixelType>& pi_pFilterPixelType)
    : HRPConvFilter(pi_pFilterPixelType)
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPMedianFilter::HRPMedianFilter(const HFCPtr<HRPPixelType>& pi_pFilterPixelType, const HRPPixelNeighbourhood& pi_rNeighbourhood)
    : HRPConvFilter(pi_pFilterPixelType, pi_rNeighbourhood)
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPMedianFilter::HRPMedianFilter(const HRPMedianFilter& pi_rFilter)
    : HRPConvFilter(pi_rFilter)
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPMedianFilter::~HRPMedianFilter()
    {
    }

//-----------------------------------------------------------------------------
// public
// SetNeighbourhood
//-----------------------------------------------------------------------------
void HRPMedianFilter::SetNeighbourhood(uint32_t pi_Width,
                                       uint32_t pi_Height,
                                       uint32_t pi_XOrigin,
                                       uint32_t pi_YOrigin)
    {
    HRPConvFilter::SetNeighbourhood (HRPPixelNeighbourhood(pi_Width, pi_Height, pi_XOrigin, pi_YOrigin));
    }

//-----------------------------------------------------------------------------
// public
// IsAConvolutionFilter
//-----------------------------------------------------------------------------
bool HRPMedianFilter::IsAConvolutionFilter() const
    {
    return false;
    }
