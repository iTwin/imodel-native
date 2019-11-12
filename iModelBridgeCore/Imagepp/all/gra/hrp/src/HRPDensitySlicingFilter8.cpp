//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRPDensitySlicingFilter8
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <ImagePP/all/h/HRPDensitySlicingFilter8.h>
#include <ImagePP/all/h/HRPPixelTypeV24R8G8B8.h>

//-----------------------------------------------------------------------------
//  Custom Map8  Filter
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// Constructor.
//-----------------------------------------------------------------------------

HRPDensitySlicingFilter8::HRPDensitySlicingFilter8()
    :HRPDensitySlicingFilter(new HRPPixelTypeV24R8G8B8())
    {
    m_ChannelWidth       = 8;
    m_MaxSampleValue     = 255;
    m_Channels           = 3;
    m_DesaturationFactor = 0.0;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPDensitySlicingFilter8::HRPDensitySlicingFilter8(const HRPDensitySlicingFilter8& pi_rFilter)
    :HRPDensitySlicingFilter(pi_rFilter)
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPDensitySlicingFilter8::HRPDensitySlicingFilter8(const HFCPtr<HRPPixelType>& pi_pFilterPixelType)
    :HRPDensitySlicingFilter(pi_pFilterPixelType)
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------

HRPDensitySlicingFilter8::~HRPDensitySlicingFilter8()
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPFilter* HRPDensitySlicingFilter8::Clone() const
    {
    return new HRPDensitySlicingFilter8(*this);
    }

