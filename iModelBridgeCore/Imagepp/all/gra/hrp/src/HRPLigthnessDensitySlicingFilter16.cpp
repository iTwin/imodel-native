//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPLigthnessDensitySlicingFilter16.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRPLigthnessDensitySlicingFilter16
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPLigthnessDensitySlicingFilter16.h>
#include <Imagepp/all/h/HRPPixelTypeV48R16G16B16.h>

//-----------------------------------------------------------------------------
//  Custom Map8  Filter
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// Constructor.
//-----------------------------------------------------------------------------

HRPLigthnessDensitySlicingFilter16::HRPLigthnessDensitySlicingFilter16()
    :HRPLigthnessDensitySlicingFilter(new HRPPixelTypeV48R16G16B16())
    {
    m_ChannelWidth       = 16;
    m_MaxSampleValue     = 65535;
    m_Channels           = 3;
    m_DesaturationFactor = 0.0;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPLigthnessDensitySlicingFilter16::HRPLigthnessDensitySlicingFilter16(const HRPLigthnessDensitySlicingFilter16& pi_rFilter)
    :HRPLigthnessDensitySlicingFilter(pi_rFilter)
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPLigthnessDensitySlicingFilter16::HRPLigthnessDensitySlicingFilter16(const HFCPtr<HRPPixelType>& pi_pFilterPixelType)
    :HRPLigthnessDensitySlicingFilter(pi_pFilterPixelType)
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------

HRPLigthnessDensitySlicingFilter16::~HRPLigthnessDensitySlicingFilter16()
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPFilter* HRPLigthnessDensitySlicingFilter16::Clone() const
    {
    return new HRPLigthnessDensitySlicingFilter16(*this);
    }

