//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPLigthnessContrastStretch16.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPFunctionFilters
//-----------------------------------------------------------------------------
// Some common function filters.
//-----------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRPLigthnessContrastStretch16.h>
#include <Imagepp/all/h/HRPPixelTypeV48R16G16B16.h>
#include <Imagepp/all/h/HGFLuvColorSpace.h>

//-----------------------------------------------------------------------------
//
// Domain value for
//
//     m_pMaxValue,  m_pMinValue                : [  0, 65535]
//     m_pMinContrastValue, m_pMaxContrastValue : [  0, 65535]
//     m_pGammaFactor                           : [0.1,   9.9]
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Default constructor provided For HPM_REGISTER_CLASS( ONLY !
//-----------------------------------------------------------------------------

HRPLigthnessContrastStretch16::HRPLigthnessContrastStretch16()
    :HRPLigthnessContrastStretch(new HRPPixelTypeV48R16G16B16())
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPLigthnessContrastStretch16::HRPLigthnessContrastStretch16(const HFCPtr<HRPPixelType>& pi_pFilterPixelType)
    :HRPLigthnessContrastStretch(pi_pFilterPixelType)
    {
    HPRECONDITION(pi_pFilterPixelType->CountIndexBits() == 0);
    HPRECONDITION((pi_pFilterPixelType->CountPixelRawDataBits() % 16) == 0);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPLigthnessContrastStretch16::HRPLigthnessContrastStretch16(const HRPLigthnessContrastStretch16& pi_rFilter)
    :HRPLigthnessContrastStretch(pi_rFilter)
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPLigthnessContrastStretch16::~HRPLigthnessContrastStretch16()
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPFilter* HRPLigthnessContrastStretch16::Clone() const
    {
    return new HRPLigthnessContrastStretch16(*this);
    }
