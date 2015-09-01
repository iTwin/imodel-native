//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFLabColorSpace.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// L*a*b*  ColorSpace converter class declaration.
//-----------------------------------------------------------------------------

#pragma once

#include "HGFBasicColorSpace.h"

BEGIN_IMAGEPP_NAMESPACE
class HGFLabColorSpace : public HGFBasicColorSpace
    {
public:

    HGFLabColorSpace(unsigned short pi_BitsPerPixel = 8);
    virtual ~HGFLabColorSpace();

    // Conversion both side between RGB and CIE LAB (Not done yet)
    void ConvertFromRGB (Byte pi_Red, Byte pi_Green, Byte pi_Blue,
                         double*       po_pL,  double*       po_pA,    double*       po_pB);

    void ConvertToRGB (double         pi_L,    double         pi_A,      double         pi_B,
                       Byte* po_pRed, Byte* po_pGreen, Byte* po_pBlue);

    void GetHueChroma ( double pi_L, double pi_A, double pi_B,
                        double* po_Chroma, double* po_Hue);

    double CIE94Difference(double pi_L1, double pi_A1, double pi_B1,
                           double pi_L2, double pi_A2, double pi_B2,
                           double pi_LightnessRange,
                           double pi_ChromaRange);

    double CMCDifference(double pi_L1, double pi_A1, double pi_B1,
                         double pi_L2, double pi_A2, double pi_B2,
                         double pi_LightnessRange,
                         double pi_ChromaRange);


#if 0
    // To be killed.
    // Used for test experimentation and debugging.
    void RetainMinMax(double pi_L,  double pi_A,  double pi_B,
                      double* MinL, double* MaxL, double* MinA,
                      double* MaxA, double* MinB, double* MaxB);
#endif


protected:
    // Nothing here at this time.

private:

    // Disable unused method to avoid unappropriate compiler initiative
    HGFLabColorSpace(const HGFLabColorSpace& pi_rSource);
    HGFLabColorSpace& operator=(const HGFLabColorSpace& pi_rSource);

    };

END_IMAGEPP_NAMESPACE