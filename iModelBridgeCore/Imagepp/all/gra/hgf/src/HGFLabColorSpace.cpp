//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGFLabColorSpace.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HGFLabColorSpace
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGFLabColorSpace.h>
#include <Imagepp/all/h/HFCMaths.h>

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

HGFLabColorSpace::HGFLabColorSpace(unsigned short pi_BitsPerPixel)
    :HGFBasicColorSpace(pi_BitsPerPixel)
    {
    // Nothing to do here at this time.
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

HGFLabColorSpace::~HGFLabColorSpace()
    {
    // Nothing to do here at this time.
    }

//----------------------------------------------------------------------------
// Convert RGB triplet into L*a*b* color space
//----------------------------------------------------------------------------

void HGFLabColorSpace::ConvertFromRGB(Byte pi_Red, Byte pi_Green, Byte pi_Blue,
                                      double*       po_pL,  double*       po_pA,    double*       po_pB)
    {
    double X = m_pRGBToXRed[pi_Red] + m_pRGBToXGreen[pi_Green] + m_pRGBToXBlue[pi_Blue];
    double Y = m_pRGBToYRed[pi_Red] + m_pRGBToYGreen[pi_Green] + m_pRGBToYBlue[pi_Blue];
    double Z = m_pRGBToZRed[pi_Red] + m_pRGBToZGreen[pi_Green] + m_pRGBToZBlue[pi_Blue];

    // Pre-compute division wich will be used more than once.
    double XDivByX0 = X / m_ReferenceWhiteXYZ [0];
    double YDivByY0 = Y / m_ReferenceWhiteXYZ [1];
    double ZDivByZ0 = Z / m_ReferenceWhiteXYZ [2];

    // Pre-compute division wich will be used more than once.
    double ComputedConst = (16.0 / 116.0);

    double ComputedX;
    double ComputedY;
    double ComputedZ;

    // Preprocess ratio using tri-stimulus value: X/X0
    if (XDivByX0 > 0.008856)
        ComputedX = LimitedFastCubicRoot(XDivByX0);
    else
        ComputedX = (7.7787 * XDivByX0) + ComputedConst;

    // Preprocess ratio using tri-stimulus value: Y/Y0
    if (YDivByY0 > 0.008856)
        ComputedY = LimitedFastCubicRoot(YDivByY0);
    else
        ComputedY = (7.7787 * YDivByY0) + ComputedConst;

    // Preprocess ratio using tri-stimulus value: Z/Z0
    if (ZDivByZ0 > 0.008856)
        ComputedZ = LimitedFastCubicRoot(ZDivByZ0);
    else
        ComputedZ = (7.7787 * ZDivByZ0) + ComputedConst;

    // Two ways for ligthness computation when YDivByY0 is less than 0.008856...
    // Be sure they have comparable result..
    //if (YDivByY0 < 0.008856)
    //    HASSERT(fabs((903.29999 * YDivByY0) - ((116.0 * ComputedY) - 16.0)) < 0.009999);

    // Now, we are ready to convert from RGB to L*a*b*
    *po_pL = fabs((116.0 *  ComputedY) - 16.0);
    *po_pA =  500.0 * (ComputedX - ComputedY);
    *po_pB =  200.7 * (ComputedY - ComputedZ);
    }

//----------------------------------------------------------------------------
// Convert L*a*b* triplet into RGB color space
//----------------------------------------------------------------------------

void HGFLabColorSpace::ConvertToRGB (double         pi_L,    double         pi_A,      double         pi_B,
                                     Byte* po_pRed, Byte* po_pGreen, Byte* po_pBlue)
    {
    double ComputedY = (pi_L + 16.0) / 116.0;
    double ComputedX = (pi_A / 500.0) + ComputedY;
    double ComputedZ = -1.0 * (pi_B / 200.7 - ComputedY);

    double ComputedConst = (16.0 / 116.0);

    double XDivByX0;
    double YDivByY0;
    double ZDivByZ0;

    if (ComputedX > 0.206893)
        XDivByX0 = ComputedX * ComputedX * ComputedX;
    else
        XDivByX0 = (ComputedX - ComputedConst) / 7.7787;

    if (ComputedY > 0.206893)
        YDivByY0 = ComputedY * ComputedY * ComputedY;
    else
        YDivByY0 = (ComputedY - ComputedConst) / 7.7787;

    if (ComputedZ > 0.206893)
        ZDivByZ0 = ComputedZ * ComputedZ * ComputedZ;
    else
        ZDivByZ0 = (ComputedZ - ComputedConst) / 7.7787;

    double X = XDivByX0 * m_ReferenceWhiteXYZ [0];
    double Y = YDivByY0 * m_ReferenceWhiteXYZ [1];
    double Z = ZDivByZ0 * m_ReferenceWhiteXYZ [2];

    ConvertFromXYZ(X, Y, Z, po_pRed, po_pGreen, po_pBlue);
    }

//----------------------------------------------------------------------------
// Compute Chroma and  Hue from LAB value.
//----------------------------------------------------------------------------
// Un-optimize version.
//

inline void HGFLabColorSpace::GetHueChroma( double  pi_L,      double  pi_A, double pi_B,
                                            double* po_Chroma, double* po_Hue)
    {
    double EQUALITY_EPSILON = 9.99999E-8;

    // Extract the chromaticity value from the L*a*b* triplet
    *po_Chroma = sqrt((pi_A * pi_A) + (pi_B * pi_B));

    // Get the hue angle has describe...
    // *po_Hue = atan2f(pi_B, pi_A);        // arctan (B / A)

    // Before processing the arc tan(pi_b / pi_a) be sure we wont have
    // any zero dvision...
    if ((pi_A < EQUALITY_EPSILON) && (pi_A > -EQUALITY_EPSILON)) // If (A == 0)
        {
        if ((pi_B < EQUALITY_EPSILON) && (pi_B > -EQUALITY_EPSILON))
            *po_Hue = 0.0;
        else if (pi_B > EQUALITY_EPSILON)          // if (B > 0)
            *po_Hue = 90.0;
        else                                       // if (B < 0)
            *po_Hue = 270.0;
        }
    else if ((pi_B < EQUALITY_EPSILON) && (pi_B > -EQUALITY_EPSILON)) // If (B == 0)
        {
        if (pi_A < EQUALITY_EPSILON)               // if (A < 0)
            *po_Hue = 180.0;
        else
            *po_Hue = 0.0;                          // A > 0
        }
    else                                           // (A != 0) && (B != 0)
        {   // * 180 / 3.14159265359
        *po_Hue = atan2(pi_B, pi_A) * 57.2957795131;       // arctan (B / A)
        }
    }

//----------------------------------------------------------------------------
// Compute thye color difference using CMC equation. This equation allows
// calculation of tolerance ellipsoids.  The design of this formula allows
// two user define coefficient l and c (lightness and chroma) as normally
// specified CMC(l:c).
//----------------------------------------------------------------------------
//
// The CMC(2:1) is a british standard (BS:6923) for the assessment of small
// colour differences and is currently being considered as an ISO standard.
//
//   l=1, c=1 perceptibility
//   l=2, c=1 acceptability
//
// where l = pi_LightnessRange and c = pi_ChromaRange
//----------------------------------------------------------------------------
// Un-optimize version.
//

double HGFLabColorSpace::CMCDifference(double pi_L1, double pi_A1, double pi_B1,
                                       double pi_L2, double pi_A2, double pi_B2,
                                       double pi_LightnessRange,
                                       double pi_ChromaRange)
    {

    HPRECONDITION(false);
    /*
    double SL;
    double SC;
    double SH;

    if (
        SL = 0.040975
    else
        SL =
    */

    return 0.0;
    }

//----------------------------------------------------------------------------
// Compute the color difference using CIE94 equation. This equation is a
// simplification of the CMC equation. The design of this formula allows
// two user define coefficient l and c (lightness and chroma) as normally
// specified CIE94(l:c).
//----------------------------------------------------------------------------
//   l=1, c=1 perceptibility
//   l=2, c=1 acceptability
//
// where l = pi_LightnessRange and c = pi_ChromaRange
//----------------------------------------------------------------------------
// Un-optimize version.
//

double HGFLabColorSpace::CIE94Difference(double pi_L1, double pi_A1, double pi_B1,
                                         double pi_L2, double pi_A2, double pi_B2,
                                         double pi_LightnessRange,
                                         double pi_ChromaRange)
    {
    // pi_LightnessRange and pi_ChromaRange cant be null because a zero
    // division may occur.
    HPRECONDITION(pi_LightnessRange > 0.0);
    HPRECONDITION(pi_ChromaRange    > 0.0);

    double C1;
    double H1;
    double C2;
    double H2;

    // Extract the hue and the chroma from the first L*a*b* triplet
    GetHueChroma(pi_L1, pi_A1, pi_B1, &C1, &H1);

    // Extract the hue and the chroma from the second L*a*b* triplet
    GetHueChroma(pi_L2, pi_A2, pi_B2, &C2, &H2);

    // Value of the standard chroma is given by the goemetric mean.
    double C = sqrt(C1 * C2);

    double SL = 1.0;
    double SC = 1.0 + (0.045 * C);
    double SH = 1.0 + (0.015 * C);

    // Ligthness difference
    double dL = fabs(pi_L1 - pi_L2) / (pi_LightnessRange * SL);

    // Chroma difference
    double dC = fabs(C1 - C2)       / (pi_ChromaRange    * SC);

    // Hue difference
    double dH = fabs(H1 - H2)       / (pi_LightnessRange * SH);

    // 3 dimension cartesian distance equation.
    // Distance = ((dL)^2 + (dC)^2 + (dH)^2)^1/2
    return sqrt((dL * dL) + (dC * dC) + (dH * dH));
    }

#if 0
//----------------------------------------------------------------------------
// For information only, the purppose of this method is only to evaluate
// LUV domain from some RGB conversion.
//----------------------------------------------------------------------------
// Theorical limit:
//
//         L       A        B
// Min :   0.0 , -128.0 , -128.00
// Max : 100.0 ,  128.0 ,  128.0
//
//  Real limit from RGB color space
//
// Min :  0.000000 , -86.068455 , -108.097589
// Max : 99.848167 ,  98.106571 ,   94.682355

void HGFLabColorSpace::RetainMinMax(double pi_L,  double pi_A, double pi_B,
                                    double* MinL, double* MaxL, double* MinA,
                                    double* MaxA, double* MinB, double* MaxB)
    {
    if (pi_L < *MinL)
        *MinL = pi_L;

    if (pi_L > *MaxL)
        *MaxL = pi_L;

    if (pi_A < *MinA)
        *MinA = pi_A;

    if (pi_A > *MaxA)
        *MaxA = pi_A;

    if (pi_B < *MinB)
        *MinB = pi_B;

    if (pi_B > *MaxB)
        *MaxB = pi_B;
    }
#endif

