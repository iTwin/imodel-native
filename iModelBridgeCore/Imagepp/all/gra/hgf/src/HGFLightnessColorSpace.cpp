//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGFLightnessColorSpace.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HGFLightnessColorSpace
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HGFLightnessColorSpace.h>
#include <Imagepp/all/h/HFCMaths.h>

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

HGFLightnessColorSpace::HGFLightnessColorSpace(double pi_GammaCorrection, unsigned short pi_BitsPerPixel)
    :HGFBasicColorSpace(pi_GammaCorrection, pi_BitsPerPixel)
    {
    BuildLookupTableLUVToRGB();
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

HGFLightnessColorSpace::~HGFLightnessColorSpace()
    {
    // Nothing to do here at this time.
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

void HGFLightnessColorSpace::BuildLookupTableLUVToRGB()
    {
    m_UPrime = (4.0 * m_ReferenceWhiteXYZ [0]) / (m_ReferenceWhiteXYZ [0] + ( 15.0 * m_ReferenceWhiteXYZ [1] ) + (3.0 * m_ReferenceWhiteXYZ [2]));
    m_VPrime = (9.0 * m_ReferenceWhiteXYZ [1])/  (m_ReferenceWhiteXYZ [0] + ( 15.0 * m_ReferenceWhiteXYZ [1] ) + (3.0 * m_ReferenceWhiteXYZ [2]));
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

double HGFLightnessColorSpace::ConvertFromRGB (unsigned short pi_Red, unsigned short pi_Green, unsigned short pi_Blue) const
    {
    double Y = m_pRGBToYRed[pi_Red] + m_pRGBToYGreen[pi_Green] + m_pRGBToYBlue[pi_Blue];

    // supposed to be : Y = Y / m_ReferenceWhiteXYZ [1];
    // but because m_ReferenceWhiteXYZ [1] (Lighness) should always be == 1 so...

    // Compute L value (Lightness)
    if (Y > 0.008856)
        return (116.0 * LimitedFastCubicRoot(Y) - 16.0);
    else
        return (903.29999 * Y);
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

void HGFLightnessColorSpace::ConvertArrayFromRGB (double* po_pL,
                                                  Byte*  pi_pRed, Byte*  pi_pGreen, Byte*  pi_pBlue,
                                                  uint32_t pi_SampleCount) const
    {
    double Y;

    for (uint32_t ArrayIndex = 0; ArrayIndex < pi_SampleCount; ArrayIndex++)
        {
        Y = m_pRGBToYRed[*pi_pRed] + m_pRGBToYGreen[*pi_pGreen] + m_pRGBToYBlue[*pi_pBlue];

        // Pre-compute division wich will be used two time to avoid one.
        // m_ReferenceWhiteXYZ [1] (Lighness) is == 1 so...
        // supposed to be : TempValue = Y / m_ReferenceWhiteXYZ [1];

        // Compute L value (Lightness)
        if (Y > 0.008856)
            *po_pL = 116.0 * LimitedFastCubicRoot(Y) - 16.0;
        else
            *po_pL = 903.29999 * Y;

        // Go to the next pixel.
        ++pi_pRed;
        ++pi_pGreen;
        ++pi_pBlue;
        ++po_pL;
        }
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

void HGFLightnessColorSpace::ConvertArrayFromRGB (double* po_pL,
                                                  unsigned short* pi_pRed, unsigned short*  pi_pGreen, unsigned short*  pi_pBlue,
                                                  uint32_t pi_SampleCount) const
    {
    double Y;

    for (uint32_t ArrayIndex = 0; ArrayIndex < pi_SampleCount; ArrayIndex++)
        {
        Y = m_pRGBToYRed[*pi_pRed] + m_pRGBToYGreen[*pi_pGreen] + m_pRGBToYBlue[*pi_pBlue];

        // Pre-compute division wich will be used two time to avoid one.
        // m_ReferenceWhiteXYZ [1] (Lighness) is == 1 so...
        // supposed to be : TempValue = Y / m_ReferenceWhiteXYZ [1];

        // Compute L value (Lightness)
        if (Y > 0.008856)
            *po_pL = 116.0 * LimitedFastCubicRoot(Y) - 16.0;
        else
            *po_pL = 903.29999 * Y;

        // Go to the next pixel.
        ++pi_pRed;
        ++pi_pGreen;
        ++pi_pBlue;
        ++po_pL;
        }
    }

#if 0
//----------------------------------------------------------------------------
// For information only, the purppose of this method is only to evaluate
// LUV domain from some RGB conversion.
//----------------------------------------------------------------------------

void HGFLightnessColorSpace::RetainMinMax(double pi_L, double pi_U, double pi_V,
                                          double* MinL, double* MaxL, double* MinU,
                                          double* MaxU, double* MinV, double* MaxV)
    {
    if (pi_L < *MinL)
        *MinL = pi_L;

    if (pi_L > *MaxL)
        *MaxL = pi_L;

    if (pi_U < *MinU)
        *MinU = pi_U;

    if (pi_U > *MaxU)
        *MaxU = pi_U;

    if (pi_V < *MinV)
        *MinV = pi_V;

    if (pi_V > *MaxV)
        *MaxV = pi_V;
    }
#endif

