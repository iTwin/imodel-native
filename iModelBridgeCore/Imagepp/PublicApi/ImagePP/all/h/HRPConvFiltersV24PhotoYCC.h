//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPConvFiltersV24PhotoYCC.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRPConvFiltersV24PhotoYCC
//-----------------------------------------------------------------------------

#pragma once

#include "HRPConvFilterV24PhotoYCC.h"

//-----------------------------------------------------------------------------
// HRPBlurFilterV24PhotoYCC
// Blur filter definition
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
class HRPBlurFilterV24PhotoYCC : public HRPConvFilterV24PhotoYCC
    {
    HDECLARE_CLASS_ID(HRPFilterId_BlurV24PhotoYCC, HRPConvFilterV24PhotoYCC)
    

public:

    HRPBlurFilterV24PhotoYCC();
    HRPBlurFilterV24PhotoYCC(Byte pi_Intensity);
    virtual            ~HRPBlurFilterV24PhotoYCC();

    // Cloning
    virtual HRPFilter* Clone() const override;

    Byte  GetIntensity() const;

protected:
    HRPBlurFilterV24PhotoYCC(const HRPBlurFilterV24PhotoYCC& pi_rFilter);

private:
    // Disabled methods
    HRPBlurFilterV24PhotoYCC& operator =(const HRPBlurFilterV24PhotoYCC& pi_rObj);

    Byte  m_Intensity;
    };

//-----------------------------------------------------------------------------
// HRPSharpenFilterV24PhotoYCC
// Sharpen filter definition
//-----------------------------------------------------------------------------

class HRPSharpenFilterV24PhotoYCC : public HRPConvFilterV24PhotoYCC
    {
    HDECLARE_CLASS_ID(HRPFilterId_SharpenV24PhotoYCC, HRPConvFilterV24PhotoYCC)
    

public:
    HRPSharpenFilterV24PhotoYCC();
    HRPSharpenFilterV24PhotoYCC(Byte pi_Intensity);
    virtual             ~HRPSharpenFilterV24PhotoYCC();

    // Cloning
    virtual HRPFilter* Clone() const override;

    Byte  GetIntensity() const;

protected:
    HRPSharpenFilterV24PhotoYCC(const HRPSharpenFilterV24PhotoYCC& pi_rFilter);

private:
    // Disabled methods
    HRPSharpenFilterV24PhotoYCC& operator =(const HRPSharpenFilterV24PhotoYCC& pi_rObj);

    Byte  m_Intensity;
    };
END_IMAGEPP_NAMESPACE

