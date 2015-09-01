//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPLigthnessContrastStretch8.cpp $
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

#include <Imagepp/all/h/HRPLigthnessContrastStretch8.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPLigthnessContrastStretch8::HRPLigthnessContrastStretch8()
    :HRPLigthnessContrastStretch(new HRPPixelTypeV24R8G8B8())
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPLigthnessContrastStretch8::HRPLigthnessContrastStretch8(const HFCPtr<HRPPixelType>& pi_pFilterPixelType)
    :HRPLigthnessContrastStretch(pi_pFilterPixelType)
    {
    HPRECONDITION(pi_pFilterPixelType->CountIndexBits() == 0);
    HPRECONDITION((pi_pFilterPixelType->CountPixelRawDataBits() % 8) == 0);
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPLigthnessContrastStretch8::HRPLigthnessContrastStretch8(const HRPLigthnessContrastStretch8& pi_rFilter)
    :HRPLigthnessContrastStretch(pi_rFilter)
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPLigthnessContrastStretch8::~HRPLigthnessContrastStretch8()
    {
    }

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

HRPFilter* HRPLigthnessContrastStretch8::Clone() const
    {
    return new HRPLigthnessContrastStretch8(*this);
    }
