//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPLigthnessDensitySlicingFilter8.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HRPLigthnessDensitySlicingFilter8
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HRPLigthnessDensitySlicingFilter8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>

//-----------------------------------------------------------------------------
//  Custom Map8  Filter
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// public
// Constructor.
//-----------------------------------------------------------------------------

HRPLigthnessDensitySlicingFilter8::HRPLigthnessDensitySlicingFilter8()
    :HRPLigthnessDensitySlicingFilter(new HRPPixelTypeV24R8G8B8())
    {
    m_ChannelWidth       = 8;
    m_MaxSampleValue     = 255;
    m_Channels           = 3;
    m_DesaturationFactor = 0.0;
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPLigthnessDensitySlicingFilter8::HRPLigthnessDensitySlicingFilter8(const HRPLigthnessDensitySlicingFilter8& pi_rFilter)
    :HRPLigthnessDensitySlicingFilter(pi_rFilter)
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPLigthnessDensitySlicingFilter8::HRPLigthnessDensitySlicingFilter8(const HFCPtr<HRPPixelType>& pi_pFilterPixelType)
    :HRPLigthnessDensitySlicingFilter(pi_pFilterPixelType)
    {
    }

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------

HRPLigthnessDensitySlicingFilter8::~HRPLigthnessDensitySlicingFilter8()
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPFilter* HRPLigthnessDensitySlicingFilter8::Clone() const
    {
    return new HRPLigthnessDensitySlicingFilter8(*this);
    }

