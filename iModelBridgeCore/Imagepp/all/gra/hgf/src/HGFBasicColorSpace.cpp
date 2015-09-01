//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGFBasicColorSpace.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HGFBasicColorSpace
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGFBasicColorSpace.h>

HGFBasicColorSpace::HGFBasicColorSpace(unsigned short pi_BitsPerPixel)
    {
    m_BitsPerPixel = pi_BitsPerPixel;
    m_ChannelSize  = 1 << m_BitsPerPixel;

    // Red component lookup table
    m_pRGBToXRed   = new double[m_ChannelSize];
    m_pRGBToYRed   = new double[m_ChannelSize];
    m_pRGBToZRed   = new double[m_ChannelSize];

    // Green component lookup table
    m_pRGBToXGreen = new double[m_ChannelSize];
    m_pRGBToYGreen = new double[m_ChannelSize];
    m_pRGBToZGreen = new double[m_ChannelSize];

    // Blue component lookup table
    m_pRGBToXBlue  = new double[m_ChannelSize];
    m_pRGBToYBlue  = new double[m_ChannelSize];
    m_pRGBToZBlue  = new double[m_ChannelSize];

    // Initialize object without using gamma correction.
    // See also UseGammaCorrection.
    UseGammaCorrection(1.0, false);
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

HGFBasicColorSpace::HGFBasicColorSpace(double pi_GammaCorrection, unsigned short pi_BitsPerPixel)
    {
    m_BitsPerPixel = pi_BitsPerPixel;
    m_ChannelSize  = 1 << m_BitsPerPixel;

    // Green component lookup table
    m_pRGBToXRed   = new double[m_ChannelSize];
    m_pRGBToYRed   = new double[m_ChannelSize];
    m_pRGBToZRed   = new double[m_ChannelSize];

    // Green component lookup table
    m_pRGBToXGreen = new double[m_ChannelSize];
    m_pRGBToYGreen = new double[m_ChannelSize];
    m_pRGBToZGreen = new double[m_ChannelSize];

    // Blue component lookup table
    m_pRGBToXBlue  = new double[m_ChannelSize];
    m_pRGBToYBlue  = new double[m_ChannelSize];
    m_pRGBToZBlue  = new double[m_ChannelSize];

    // Initialize object using gamma correction.
    // See also UseGammaCorrection.
    UseGammaCorrection(pi_GammaCorrection, true);
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

HGFBasicColorSpace::~HGFBasicColorSpace()
    {
    // Green component lookup table
    delete []m_pRGBToXRed;
    delete []m_pRGBToYRed;
    delete []m_pRGBToZRed;

#ifdef __HMR_DEBUG
    m_pRGBToXRed = 0;
    m_pRGBToYRed = 0;
    m_pRGBToZRed = 0;
#endif

    // Green component lookup table
    delete []m_pRGBToXGreen;
    delete []m_pRGBToYGreen;
    delete []m_pRGBToZGreen;

#ifdef __HMR_DEBUG
    m_pRGBToXGreen = 0;
    m_pRGBToYGreen = 0;
    m_pRGBToZGreen = 0;
#endif

    // Blue component lookup table
    delete []m_pRGBToXBlue;
    delete []m_pRGBToYBlue;
    delete []m_pRGBToZBlue;

#ifdef __HMR_DEBUG
    m_pRGBToXBlue = 0;
    m_pRGBToYBlue = 0;
    m_pRGBToZBlue = 0;
#endif

    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

void HGFBasicColorSpace::UseGammaCorrection(double pi_GammaCorrection, bool pi_UseGammaFactor)
    {
    m_UseGammaCorrection = pi_UseGammaFactor;

    // If not using gamma factor, be sure to have a "no effect" gamma factor.
    if (m_UseGammaCorrection)
        m_GammaCorrectionFactor = pi_GammaCorrection;
    else
        m_GammaCorrectionFactor = 1.0;

    // Rebuild lookup table according gamma factor to allow
    // bi-directional convertion between RGB and XYZ.
    BuildLookupTableFromXYZ();
    BuildLookupTableToXYZ();
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

void HGFBasicColorSpace::BuildLookupTableFromXYZ()
    {
    uint32_t TableIndex;
    double NormalizedValue;

    /* // Bruce Justin Lindbloom : Adobe RGB (1998) (D65 white point) to CIE XYZ
    double XYZToRBGMatrix [3][3] = { 0.576700, 0.297361 , 0.0270328,
                                      0.185556, 0.627355 , 0.0706879,
                                      0.188212, 0.0752847, 0.991248}; /* */

    // Poynton's : sRGB Rec 709 (D65 white point) to CIE XYZ
    double XYZToRBGMatrix [3][3] = { 0.412453, 0.357580, 0.180423,
                                      0.212671, 0.715160, 0.072169,
                                      0.019334, 0.119193, 0.950227
                                    }; /* */

    /*// International Standard (D65) adopted for HDTV
    double XYZToRBGMatrix [3][3] = { 0.640, 0.300, 0.150,
                                     0.330, 0.600, 0.060,
                                     0.030, 0.100, 0.790};*/

    /* // Alan Watt 3D Computer Graphics
    double XYZToRBGMatrix [3][3] = { 0.584, 0.188, 0.179,
                                     0.311, 0.614, 0.075,
                                     0.047, 0.103, 0.939};*/

    /*// Old NTSC Standard for 1953 phosphore into television.
    double XYZToRBGMatrix [3][3] = { 0.6070, 0.1740, 0.2000,
                                     0.2990, 0.5870, 0.1440,
                                     0.0000, 0.0660, 1.1120};*/

    for (TableIndex = 0; TableIndex < m_ChannelSize; TableIndex++)
        {
        // Divide TableIndex by 255.0 because valid RBG value sould be normalize
        // to fit within 0 to 1 range inclusively, instead of 0 to 255.
        NormalizedValue = (double)TableIndex / (double)(m_ChannelSize - 1);

        if (m_UseGammaCorrection)
            {
            // Compute normalized value with gamme correction factor.
            NormalizedValue = pow(NormalizedValue, m_GammaCorrectionFactor);
            }

        m_pRGBToXRed   [TableIndex] = NormalizedValue * XYZToRBGMatrix[0][0];
        m_pRGBToXGreen [TableIndex] = NormalizedValue * XYZToRBGMatrix[0][1];
        m_pRGBToXBlue  [TableIndex] = NormalizedValue * XYZToRBGMatrix[0][2];

        m_pRGBToYRed   [TableIndex] = NormalizedValue * XYZToRBGMatrix[1][0];
        m_pRGBToYGreen [TableIndex] = NormalizedValue * XYZToRBGMatrix[1][1];
        m_pRGBToYBlue  [TableIndex] = NormalizedValue * XYZToRBGMatrix[1][2];

        m_pRGBToZRed   [TableIndex] = NormalizedValue * XYZToRBGMatrix[2][0];
        m_pRGBToZGreen [TableIndex] = NormalizedValue * XYZToRBGMatrix[2][1];
        m_pRGBToZBlue  [TableIndex] = NormalizedValue * XYZToRBGMatrix[2][2];
        }

    // Compute reference white from RGB(255, 255, 255) to XYZ
    ConvertToXYZ((unsigned short)(m_ChannelSize - 1),
                 (unsigned short)(m_ChannelSize - 1),
                 (unsigned short)(m_ChannelSize - 1),
                 &m_ReferenceWhiteXYZ[0],
                 &m_ReferenceWhiteXYZ[1],
                 &m_ReferenceWhiteXYZ[2]);
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

void HGFBasicColorSpace::BuildLookupTableToXYZ()
    {
    // Poynton's : CIE XYZ to Rec 709 sRGB (D65 white point)
    m_RGBToXYZMatrix[0][0] =  3.240479;
    m_RGBToXYZMatrix[0][1] = -1.537150;
    m_RGBToXYZMatrix[0][2] = -0.498535;

    m_RGBToXYZMatrix[1][0] = -0.969256;
    m_RGBToXYZMatrix[1][1] =  1.875992;
    m_RGBToXYZMatrix[1][2] =  0.041556;

    m_RGBToXYZMatrix[2][0] =  0.055648;
    m_RGBToXYZMatrix[2][1] = -0.204043;
    m_RGBToXYZMatrix[2][2] =  1.057311;
    /* */

    /* // Bruce Justin Lindbloom : CIE XYZ to Adobe RGB (1998) (D65 white point)
    m_RGBToXYZMatrix[0][0] =  2.04148;
    m_RGBToXYZMatrix[0][1] = -0.969258;
    m_RGBToXYZMatrix[0][2] =  0.0134455;

    m_RGBToXYZMatrix[1][0] = -0.564977;
    m_RGBToXYZMatrix[1][1] =  1.87599;
    m_RGBToXYZMatrix[1][2] = -0.118373;

    m_RGBToXYZMatrix[2][0] = -0.344713;
    m_RGBToXYZMatrix[2][1] =  0.0415557;
    m_RGBToXYZMatrix[2][2] =  1.01527;
    /* */
    }

//----------------------------------------------------------------------------
// Get a the withness indicator
// Whitness  is a complex phenomenon that depends not only the luminance of
// a sample but also the chromanticity.  This method can be used to compair
// whitness between two evaluated sample.
//----------------------------------------------------------------------------

double HGFBasicColorSpace::Whitness(double pi_X, double pi_Y, double pi_Z)
    {
    double x, y;
    double xn, yn;

    // Get the pure color value of the reference white
    GetPureColor(m_ReferenceWhiteXYZ[0], m_ReferenceWhiteXYZ[1], m_ReferenceWhiteXYZ[2], &xn, &yn);

    // Convert color coordinate the extract the pure color value.
    GetPureColor(pi_X, pi_Y, pi_Z, &x, &y);

    return pi_Y + (800 * (xn - x)) + (1700 * (yn - y));
    }

//----------------------------------------------------------------------------
// Get a  Yellowness indicator
//
// A very simple ways to obtain the yellowness indicator, but the not best...
//----------------------------------------------------------------------------

double HGFBasicColorSpace::Yellowness(double pi_Y, double pi_Z)
    {
    return pi_Y - pi_Z;
    }

//----------------------------------------------------------------------------
// Use standard cartesian 3 dimensions distance. This method also return
// Delta for each axis.
// dE = ((dX)^2 + (dY)^2 + (dZ)^2)^1/2
//----------------------------------------------------------------------------
// Un-optimize version.
//

double HGFBasicColorSpace::EuclideanColorDifference(double  pi_X1, double  pi_Y1, double  pi_Z1,
                                                     double  pi_X2, double  pi_Y2, double  pi_Z2,
                                                     double* po_dX, double* po_dY, double* po_dZ)
    {
    *po_dX = fabs(pi_X1 - pi_X2);
    *po_dY = fabs(pi_Y1 - pi_Y2);
    *po_dZ = fabs(pi_Z1 - pi_Z2);

    return sqrt(((*po_dX) * (*po_dX)) + ((*po_dY) * (*po_dY)) + ((*po_dZ) * (*po_dZ)));
    }

//----------------------------------------------------------------------------
// Use standard cartesian 3 dimensions distance formula.
// dE = ((dX)^2 + (dY)^2 + (dZ)^2)^1/2
//----------------------------------------------------------------------------
// Un-optimize version.
//

double HGFBasicColorSpace::EuclideanColorDifference(double  pi_X1, double  pi_Y1, double  pi_Z1,
                                                     double  pi_X2, double  pi_Y2, double  pi_Z2)
    {
    return sqrt(((fabs(pi_X1 - pi_X2)) * (fabs(pi_X1 - pi_X2))) +
                ((fabs(pi_Y1 - pi_Y2)) * (fabs(pi_Y1 - pi_Y2))) +
                ((fabs(pi_Z1 - pi_Z2)) * (fabs(pi_Z1 - pi_Z2))));
    }

