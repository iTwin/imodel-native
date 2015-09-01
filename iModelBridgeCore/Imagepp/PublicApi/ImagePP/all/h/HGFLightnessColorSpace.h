//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFLightnessColorSpace.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// L*u*v* ColorSpace converter class declaration.
//-----------------------------------------------------------------------------

#pragma once

#include "HGFBasicColorSpace.h"

BEGIN_IMAGEPP_NAMESPACE

class HGFLightnessColorSpace : public HGFBasicColorSpace
    {
public:
    HGFLightnessColorSpace(double pi_GammaCorrection, unsigned short pi_BitsPerPixel);

    virtual ~HGFLightnessColorSpace();

    // Conversion from RGB to CIE L ( L*u*v* ColorSpace )
    double ConvertFromRGB (unsigned short pi_Red, unsigned short pi_Green, unsigned short pi_Blue) const;

    // Conversion between an array of RGB 8 bits/channel and and array of CIE L ( L*u*v* ColorSpace )
    void ConvertArrayFromRGB (double* po_pL,
                              Byte*  pi_pRed, Byte*  pi_pGreen, Byte*  pi_pBlue,
                              uint32_t pi_SampleCount) const;

    // Conversion between an array of RGB 16 bits/channel and and array of CIE L ( L*u*v* ColorSpace )
    void ConvertArrayFromRGB (double* po_pL,
                              unsigned short* pi_pRed, unsigned short*  pi_pGreen, unsigned short*  pi_pBlue,
                              uint32_t pi_SampleCount) const;

#if 0
    // Used for test experimentation and debugging.
    void RetainMinMax(double  pi_L,  double pi_U,  double pi_V,
                      double* MinL, double* MaxL, double* MinU,
                      double* MaxU, double* MinV, double* MaxV);
#endif

protected:
    // Nothing here at this time.

private:
    // U0 and V0 constant.
    double m_UPrime;
    double m_VPrime;

    // Initialize lookup table to speed up the convertion process
    void BuildLookupTableLUVToRGB();

    // Disable unused method to avoid unappropriate compiler initiative
    HGFLightnessColorSpace(const HGFLightnessColorSpace& pi_rSource);
    HGFLightnessColorSpace& operator=(const HGFLightnessColorSpace& pi_rSource);
    };

END_IMAGEPP_NAMESPACE