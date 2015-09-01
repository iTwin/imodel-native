//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPDensitySlicingFilter16.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRPDensitySlicingFilter16
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPDensitySlicingFilter16.h>
#include <Imagepp/all/h/HRPPixelTypeV48R16G16B16.h>

//-----------------------------------------------------------------------------
//  Custom Map8  Filter
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// Constructor.
//-----------------------------------------------------------------------------

HRPDensitySlicingFilter16::HRPDensitySlicingFilter16()
    :HRPDensitySlicingFilter(new HRPPixelTypeV48R16G16B16())
    {
    m_ChannelWidth       = 16;
    m_MaxSampleValue     = 65535;
    m_Channels           = 3;
    m_DesaturationFactor = 0.0;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPDensitySlicingFilter16::HRPDensitySlicingFilter16(const HRPDensitySlicingFilter16& pi_rFilter)
    :HRPDensitySlicingFilter(pi_rFilter)
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPDensitySlicingFilter16::HRPDensitySlicingFilter16(const HFCPtr<HRPPixelType>& pi_pFilterPixelType)
    :HRPDensitySlicingFilter(pi_pFilterPixelType)
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------

HRPDensitySlicingFilter16::~HRPDensitySlicingFilter16()
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPFilter* HRPDensitySlicingFilter16::Clone() const
    {
    return new HRPDensitySlicingFilter16(*this);
    }

