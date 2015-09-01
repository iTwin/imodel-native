//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGFLuvColorSpace.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HGFLuvColorSpace
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HGFLuvColorSpace.h>
#include <Imagepp/all/h/HFCMaths.h>

#define CIE_E     (216.0 / 24389.0)  // 0.00885645168
#define CIE_K     (24389.0 / 27.0 )  // 903.296296296

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

HGFLuvColorSpace::HGFLuvColorSpace(unsigned short pi_BitsPerPixel)
    :HGFBasicColorSpace(pi_BitsPerPixel)
    {
    m_pXRed_x4   = new double[m_ChannelSize];
    m_pXGreen_x4 = new double[m_ChannelSize];
    m_pXBlue_x4  = new double[m_ChannelSize];

    m_pYRed_x9   = new double[m_ChannelSize];
    m_pYGreen_x9 = new double[m_ChannelSize];
    m_pYBlue_x9  = new double[m_ChannelSize];

    m_pRed_XAnd15YAnd3Z    = new double[m_ChannelSize];
    m_pGreen__XAnd15YAnd3Z = new double[m_ChannelSize];
    m_pBlue__XAnd15YAnd3Z  = new double[m_ChannelSize];

    BuildLookupTableLUVToRGB();
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

HGFLuvColorSpace::HGFLuvColorSpace(double pi_GammaCorrection, unsigned short pi_BitsPerPixel)
    :HGFBasicColorSpace(pi_GammaCorrection, pi_BitsPerPixel)
    {
    m_pXRed_x4   = new double[m_ChannelSize];
    m_pXGreen_x4 = new double[m_ChannelSize];
    m_pXBlue_x4  = new double[m_ChannelSize];

    m_pYRed_x9   = new double[m_ChannelSize];
    m_pYGreen_x9 = new double[m_ChannelSize];
    m_pYBlue_x9  = new double[m_ChannelSize];

    m_pRed_XAnd15YAnd3Z    = new double[m_ChannelSize];
    m_pGreen__XAnd15YAnd3Z = new double[m_ChannelSize];
    m_pBlue__XAnd15YAnd3Z  = new double[m_ChannelSize];

    BuildLookupTableLUVToRGB();
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

HGFLuvColorSpace::~HGFLuvColorSpace()
    {
    // Nothing to do here at this time.
    delete []m_pXRed_x4;
    delete []m_pXGreen_x4;
    delete []m_pXBlue_x4;

#ifdef __HMR_DEBUG
    m_pXRed_x4   = 0;
    m_pXGreen_x4 = 0;
    m_pXBlue_x4  = 0;
#endif

    delete []m_pYRed_x9;
    delete []m_pYGreen_x9;
    delete []m_pYBlue_x9;

#ifdef __HMR_DEBUG
    m_pYRed_x9   = 0;
    m_pYGreen_x9 = 0;
    m_pYBlue_x9  = 0;
#endif

    delete []m_pRed_XAnd15YAnd3Z;
    delete []m_pGreen__XAnd15YAnd3Z;
    delete []m_pBlue__XAnd15YAnd3Z;

#ifdef __HMR_DEBUG
    m_pRed_XAnd15YAnd3Z    = 0;
    m_pGreen__XAnd15YAnd3Z = 0;
    m_pBlue__XAnd15YAnd3Z  = 0;
#endif
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

void HGFLuvColorSpace::BuildLookupTableLUVToRGB()
    {
    for (uint32_t TableIndex = 0; TableIndex < m_ChannelSize; TableIndex++)
        {
        // To improve LUV conversion processing, build some more lookup table
        // built with pre-calculated constant value.

        // X Table Value multiplicated by 4
        m_pXRed_x4  [TableIndex] = m_pRGBToXRed  [TableIndex] * 4;
        m_pXGreen_x4[TableIndex] = m_pRGBToXGreen[TableIndex] * 4;
        m_pXBlue_x4 [TableIndex] = m_pRGBToXBlue [TableIndex] * 4;

        // Y Table Value multiplicated by 9
        m_pYRed_x9  [TableIndex] = m_pRGBToYRed  [TableIndex] * 9;
        m_pYGreen_x9[TableIndex] = m_pRGBToYGreen[TableIndex] * 9;
        m_pYBlue_x9 [TableIndex] = m_pRGBToYBlue [TableIndex] * 9;

        // X Table value added to
        // Y Table Value multiplicated by 15 added to
        // Z Table Value multiplicated by 3
        m_pRed_XAnd15YAnd3Z   [TableIndex] = m_pRGBToXRed  [TableIndex] + (15.0 * m_pRGBToYRed  [TableIndex]) + (3.0 * m_pRGBToZRed[TableIndex]);
        m_pGreen__XAnd15YAnd3Z[TableIndex] = m_pRGBToXGreen[TableIndex] + (15.0 * m_pRGBToYGreen[TableIndex]) + (3.0 * m_pRGBToZGreen[TableIndex]);
        m_pBlue__XAnd15YAnd3Z [TableIndex] = m_pRGBToXBlue [TableIndex] + (15.0 * m_pRGBToYBlue [TableIndex]) + (3.0 * m_pRGBToZBlue[TableIndex]);
        }
    m_UPrime = (4.0 * m_ReferenceWhiteXYZ [0]) / (m_ReferenceWhiteXYZ [0] + ( 15.0 * m_ReferenceWhiteXYZ [1] ) + (3.0 * m_ReferenceWhiteXYZ [2]));
    m_VPrime = (9.0 * m_ReferenceWhiteXYZ [1])/  (m_ReferenceWhiteXYZ [0] + ( 15.0 * m_ReferenceWhiteXYZ [1] ) + (3.0 * m_ReferenceWhiteXYZ [2]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
unsigned short HGFLuvColorSpace::GrayFromLuminance(double pi_L) const
    {
    double X;
    double Y;
    double Z;

    double NormalizedColor;
    double PRECISION_EPSILON = 0.0001999;

    double ChannelMaxValue = m_ChannelSize - 1.0;

    if (pi_L > CIE_E)
        {
        // Use a Temp value to avoid unnecessary division..
        double TempValue = (pi_L + 16.0) / 116.0;

        // Instead of using pow(TempValue, 3)...
        Y = TempValue * TempValue * TempValue;
        }
    else
        Y = pi_L / CIE_K;

    //--------------------------------------------------------------
    // Convert from L*u*v* to CIE XYZ color space
    X = (-9.0 * Y * m_UPrime) / (((m_UPrime - 4.0) * m_VPrime) - (m_UPrime * m_VPrime));
    Z = ((9.0 * Y) - (15.0 * m_VPrime * Y) - m_VPrime * X) / (3.0 * m_VPrime);

    //--- Red ------------------------------------------------------
    // Get the red component normalized (value within [0,1] range)
    NormalizedColor = (m_RGBToXYZMatrix[0][0] * X) + (m_RGBToXYZMatrix[0][1] * Y) + (m_RGBToXYZMatrix[0][2] * Z);

    NormalizedColor = MIN(NormalizedColor, 1.0);
    NormalizedColor = MAX(NormalizedColor, 0.0);

    if (m_UseGammaCorrection)
        return (unsigned short)(ChannelMaxValue * pow( NormalizedColor, 1.0/m_GammaCorrectionFactor));
        
    // Convert the processed value to the standard domain
    return (unsigned short)((NormalizedColor * ChannelMaxValue) + PRECISION_EPSILON);
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

void HGFLuvColorSpace::ConvertToRGB (double   pi_L,    double   pi_U,      double   pi_V,
                                     Byte*   po_pRed, Byte*   po_pGreen, Byte*   po_pBlue) const
    {
    double UPrime;
    double VPrime;

    double X;
    double Y;
    double Z;

    double NormalizedColor;
    double PRECISION_EPSILON = 0.0001999;

    double ChannelMaxValue = m_ChannelSize - 1.0;

    // Do not allow zero division.
    double TempValue = 1.0 / (13.0 * MAX(pi_L, 0.0001));

    UPrime = (pi_U * TempValue) + m_UPrime;
    VPrime = (pi_V * TempValue) + m_VPrime;

    if (pi_L > CIE_E)
        {
        // Use a Temp value to avoid unnecessary division..
        TempValue = (pi_L + 16.0) / 116.0;

        // Instead of using pow(TempValue, 3)...
        Y = TempValue * TempValue * TempValue;
        }
    else
        Y = pi_L / CIE_K;

    //--------------------------------------------------------------
    // Convert from L*u*v* to CIE XYZ color space
    X = (-9.0 * Y * UPrime) / (((UPrime - 4.0) * VPrime) - (UPrime * VPrime));
    Z = ((9.0 * Y) - (15.0 * VPrime * Y) - VPrime * X) / (3.0 * VPrime);

    //--- Red ------------------------------------------------------
    // Get the red component normalized (value within [0,1] range)
    NormalizedColor = (m_RGBToXYZMatrix[0][0] * X) + (m_RGBToXYZMatrix[0][1] * Y) + (m_RGBToXYZMatrix[0][2] * Z);

    NormalizedColor = MIN(NormalizedColor, 1.0);
    NormalizedColor = MAX(NormalizedColor, 0.0);

    if (m_UseGammaCorrection)
        {
        // *po_pRed = (Byte)((NormalizedColor * ChannelMaxValue) + PRECISION_EPSILON);
        *po_pRed = (Byte)(ChannelMaxValue * pow( NormalizedColor, 1.0/m_GammaCorrectionFactor));
        }
    else
        {
        // Convert the processed value to the standard domain [0,255]
        *po_pRed = (Byte)((NormalizedColor * ChannelMaxValue) + PRECISION_EPSILON);
        }

    //--- Green ----------------------------------------------------
    // Get the green component normalized (value within [0,1] range)
    NormalizedColor = (m_RGBToXYZMatrix[1][0] * X) + (m_RGBToXYZMatrix[1][1] * Y) + (m_RGBToXYZMatrix[1][2] * Z);

    NormalizedColor = MIN(NormalizedColor, 1.0);
    NormalizedColor = MAX(NormalizedColor, 0.0);

    if (m_UseGammaCorrection)
        {
        *po_pGreen = (Byte)(ChannelMaxValue * pow( NormalizedColor, 1.0/m_GammaCorrectionFactor));
        }
    else
        {
        // Convert the processed value to the standard domain [0,255]
        *po_pGreen = (Byte)((NormalizedColor * ChannelMaxValue) + PRECISION_EPSILON);
        }

    //--- Blue -----------------------------------------------------
    // Get the blue component normalized (value within [0,1] range)
    NormalizedColor  = (m_RGBToXYZMatrix[2][0] * X) + (m_RGBToXYZMatrix[2][1] * Y) + (m_RGBToXYZMatrix[2][2] * Z);

    NormalizedColor = MIN(NormalizedColor, 1.0);
    NormalizedColor = MAX(NormalizedColor, 0.0);

    if (m_UseGammaCorrection)
        {
        *po_pBlue = (Byte)(ChannelMaxValue * pow( NormalizedColor, 1.0/m_GammaCorrectionFactor));
        }
    else
        {
        // Convert the processed value to the standard domain [0,255]
        *po_pBlue = (Byte)((NormalizedColor * ChannelMaxValue) + PRECISION_EPSILON);
        }
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

void HGFLuvColorSpace::ConvertToRGB (double   pi_L,    double   pi_U,      double   pi_V,
                                     unsigned short*  po_pRed, unsigned short*  po_pGreen, unsigned short*  po_pBlue) const
    {
    double UPrime;
    double VPrime;

    double X;
    double Y;
    double Z;

    double NormalizedColor;
    double PRECISION_EPSILON = 0.0001999;

    double ChannelMaxValue = m_ChannelSize - 1.0;

    // Do not allow zero division.
    double TempValue = 1.0 / (13.0 * MAX(pi_L, 0.0001));

    UPrime = (pi_U * TempValue) + m_UPrime;
    VPrime = (pi_V * TempValue) + m_VPrime;

    if (pi_L > CIE_E)
        {
        // Use a Temp value to avoid unnecessary division..
        TempValue = (pi_L + 16.0) / 116.0;

        // Instead of using pow(TempValue, 3)...
        Y = TempValue * TempValue * TempValue;
        }
    else
        Y = pi_L / CIE_K;

    //--------------------------------------------------------------
    // Convert from L*u*v* to CIE XYZ color space
    X = (-9.0 * Y * UPrime) / (((UPrime - 4.0) * VPrime) - (UPrime * VPrime));
    Z = ((9.0 * Y) - (15.0 * VPrime * Y) - VPrime * X) / (3.0 * VPrime);

    //--- Red ------------------------------------------------------
    // Get the red component normalized (value within [0,1] range)
    NormalizedColor = (m_RGBToXYZMatrix[0][0] * X) + (m_RGBToXYZMatrix[0][1] * Y) + (m_RGBToXYZMatrix[0][2] * Z);

    NormalizedColor = MIN(NormalizedColor, 1.0);
    NormalizedColor = MAX(NormalizedColor, 0.0);

    if (m_UseGammaCorrection)
        {
        *po_pRed = (unsigned short)(ChannelMaxValue * pow( NormalizedColor, 1.0/m_GammaCorrectionFactor));
        }
    else
        {
        // Convert the processed value to the standard domain [0,255]
        *po_pRed = (unsigned short)((NormalizedColor * ChannelMaxValue) + PRECISION_EPSILON);
        }

    //--- Green ----------------------------------------------------
    // Get the green component normalized (value within [0,1] range)
    NormalizedColor = (m_RGBToXYZMatrix[1][0] * X) + (m_RGBToXYZMatrix[1][1] * Y) + (m_RGBToXYZMatrix[1][2] * Z);

    NormalizedColor = MIN(NormalizedColor, 1.0);
    NormalizedColor = MAX(NormalizedColor, 0.0);

    if (m_UseGammaCorrection)
        {
        *po_pGreen = (unsigned short)(ChannelMaxValue * pow( NormalizedColor, 1.0/m_GammaCorrectionFactor));
        }
    else
        {
        // Convert the processed value to the standard domain [0,255]
        *po_pGreen = (unsigned short)((NormalizedColor * ChannelMaxValue) + PRECISION_EPSILON);
        }

    //--- Blue -----------------------------------------------------
    // Get the blue component normalized (value within [0,1] range)
    NormalizedColor  = (m_RGBToXYZMatrix[2][0] * X) + (m_RGBToXYZMatrix[2][1] * Y) + (m_RGBToXYZMatrix[2][2] * Z);

    NormalizedColor = MIN(NormalizedColor, 1.0);
    NormalizedColor = MAX(NormalizedColor, 0.0);

    if (m_UseGammaCorrection)
        {
        *po_pBlue = (unsigned short)(ChannelMaxValue * pow( NormalizedColor, 1.0/m_GammaCorrectionFactor));
        }
    else
        {
        // Convert the processed value to the standard domain [0,255]
        *po_pBlue = (unsigned short)((NormalizedColor * ChannelMaxValue) + PRECISION_EPSILON);
        }
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

void HGFLuvColorSpace::ConvertArrayToRGB (double*  pi_L,   double*  pi_U,     double* pi_V,
                                          Byte*  po_pRed, Byte*  po_pGreen, Byte*  po_pBlue,
                                          uint32_t pi_ArraySize) const
    {

    double UPrime;
    double VPrime;
    double TempValue;

    double X;
    double Y;
    double Z;

    double ChannelMaxValue = m_ChannelSize - 1.0;

    for (uint32_t ArrayIndex = 0; ArrayIndex < pi_ArraySize; ArrayIndex++)
        {
        // Do not allow zero division.
        TempValue = 1.0 / (13.0 * MAX(*pi_L, 0.0001));

        UPrime = (*pi_U * TempValue) + m_UPrime;
        VPrime = (*pi_V * TempValue) + m_VPrime;

        if (*pi_L > CIE_E)
            {
            // Use a Temp value to avoid unnecessary division..
            TempValue = (*pi_L + 16.0) / 116.0;

            // Instead of using pow(TempValue, 3)...
            Y = TempValue * TempValue * TempValue;
            }
        else
            Y = *pi_L / CIE_K;

        // Convert from L*u*v* to XYZ
        X = (-9 * Y * UPrime) / (((UPrime - 4) * VPrime) - (UPrime * VPrime));
        Z = ((9 * Y) - (15 * VPrime * Y) - VPrime * X) / (3 * VPrime);

        // Convert from XYZ to RGB
        //--- Red ------------------------------------------------------
        *po_pRed   = (Byte)(((m_RGBToXYZMatrix[0][0] * X) + (m_RGBToXYZMatrix[0][1] * Y) + (m_RGBToXYZMatrix[0][2] * Z) * ChannelMaxValue) + 0.0001999);

        //--- Green ----------------------------------------------------
        *po_pGreen = (Byte)(((m_RGBToXYZMatrix[1][0] * X) + (m_RGBToXYZMatrix[1][1] * Y) + (m_RGBToXYZMatrix[1][2] * Z) * ChannelMaxValue) + 0.0001999);

        //--- Blue -----------------------------------------------------
        *po_pBlue  = (Byte)(((m_RGBToXYZMatrix[2][0] * X) + (m_RGBToXYZMatrix[2][1] * Y) + (m_RGBToXYZMatrix[2][2] * Z) * ChannelMaxValue) + 0.0001999);

        // Go to the next pixel.
        ++pi_L;
        ++pi_U;
        ++pi_V;
        ++po_pRed;
        ++po_pGreen;
        ++po_pBlue;
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
double HGFLuvColorSpace::LuminanceFromGray (unsigned short pi_gray) const
    {
    HPRECONDITION(pi_gray < m_ChannelSize); 

    // Pre-compute division wich will be used two time to avoid one.
    double TempValue = (m_pRGBToYRed[pi_gray] + m_pRGBToYGreen[pi_gray] + m_pRGBToYBlue[pi_gray]) / m_ReferenceWhiteXYZ [1];

    // Compute L value (Ligthness)
    if (TempValue > CIE_E)
        return 116.0 * LimitedFastCubicRoot(TempValue) - 16.0;
    
    return CIE_K * TempValue; // *pi_pL = (116.0 * (7.787 * (Y / ReferenceWhiteXYZ [1])) + (16.0 / 116.0)) - 16.0;
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

void HGFLuvColorSpace::ConvertFromRGB (Byte   pi_Red, Byte   pi_Green, Byte   pi_Blue,
                                       double* pi_pL,  double* pi_pU,    double* pi_pV) const
    {
    double Y = m_pRGBToYRed[pi_Red] + m_pRGBToYGreen[pi_Green] + m_pRGBToYBlue[pi_Blue];
    double TempValue;
    double TempDivision;

    double X4 = m_pXRed_x4[pi_Red] + m_pXGreen_x4[pi_Green] + m_pXBlue_x4[pi_Blue];
    double X9 = m_pYRed_x9[pi_Red] + m_pYGreen_x9[pi_Green] + m_pYBlue_x9[pi_Blue];

    // Pre-compute division wich will be used two time to avoid one.
    TempValue = Y / m_ReferenceWhiteXYZ [1];

    double XAnd15YAnd3Z = m_pRed_XAnd15YAnd3Z[pi_Red] + m_pGreen__XAnd15YAnd3Z[pi_Green] + m_pBlue__XAnd15YAnd3Z[pi_Blue];

    // Compute L value (Ligthness)
    if (TempValue > CIE_E)
        *pi_pL = 116.0 * LimitedFastCubicRoot(TempValue) - 16.0;
    else
        *pi_pL = CIE_K * TempValue; // *pi_pL = (116.0 * (7.787 * (Y / ReferenceWhiteXYZ [1])) + (16.0 / 116.0)) - 16.0;

    // Compute U and V value (Chroma)
    // Avoid zero division.
    if ( XAnd15YAnd3Z > 9.99999E-8 )
        {
        // Use a temp value to save a multiply.
        TempValue = 13.0 * (*pi_pL);

        // A division a mush slower than a mul.
        // so, divide once, mul the inverse two time...
        TempDivision = 1 / XAnd15YAnd3Z;

        *pi_pU = TempValue * ((X4 * TempDivision) - m_UPrime);
        *pi_pV = TempValue * ((X9 * TempDivision) - m_VPrime);
        }
    else
        {
        *pi_pU = 0;
        *pi_pV = 0;
        }
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

void HGFLuvColorSpace::ConvertFromRGB (unsigned short pi_Red, unsigned short pi_Green, unsigned short pi_Blue,
                                       double* pi_pL,  double* pi_pU,    double* pi_pV) const
    {
    double Y = m_pRGBToYRed[pi_Red] + m_pRGBToYGreen[pi_Green] + m_pRGBToYBlue[pi_Blue];
    double TempValue;
    double TempDivision;

    double X4 = m_pXRed_x4[pi_Red] + m_pXGreen_x4[pi_Green] + m_pXBlue_x4[pi_Blue];
    double X9 = m_pYRed_x9[pi_Red] + m_pYGreen_x9[pi_Green] + m_pYBlue_x9[pi_Blue];

    // Pre-compute division wich will be used two time to avoid one.
    TempValue = Y / m_ReferenceWhiteXYZ [1];

    double XAnd15YAnd3Z = m_pRed_XAnd15YAnd3Z[pi_Red] + m_pGreen__XAnd15YAnd3Z[pi_Green] + m_pBlue__XAnd15YAnd3Z[pi_Blue];

    // Compute L value (Ligthness)
    if (TempValue > CIE_E)
        *pi_pL = 116.0 * LimitedFastCubicRoot(TempValue) - 16.0;
    else
        *pi_pL = CIE_K * TempValue; // *pi_pL = (116.0 * (7.787 * (Y / ReferenceWhiteXYZ [1])) + (16.0 / 116.0)) - 16.0;

    // Compute U and V value (Chroma)
    // Avoid zero division.
    if ( XAnd15YAnd3Z > 9.99999E-8 )
        {
        // Use a temp value to save a multiply.
        TempValue = 13.0 * (*pi_pL);

        // A division a mush slower than a mul.
        // so, divide once, mul the inverse two time...
        TempDivision = 1 / XAnd15YAnd3Z;

        *pi_pU = TempValue * ((X4 * TempDivision) - m_UPrime);
        *pi_pV = TempValue * ((X9 * TempDivision) - m_VPrime);
        }
    else
        {
        *pi_pU = 0;
        *pi_pV = 0;
        }
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

void HGFLuvColorSpace::ConvertArrayFromRGB (Byte*  pi_pRed, Byte*  pi_pGreen, Byte*  pi_pBlue,
                                            double* po_pL,   double* po_pU,     double* po_pV,
                                            uint32_t pi_ArraySize) const
    {
    double Y;
    double X4 = m_pXRed_x4[*pi_pRed] + m_pXGreen_x4[*pi_pGreen] + m_pXBlue_x4[*pi_pBlue];
    double X9 = m_pYRed_x9[*pi_pRed] + m_pYGreen_x9[*pi_pGreen] + m_pYBlue_x9[*pi_pBlue];
    double XAnd15YAnd3Z;
    double TempValue;
    double TempDivision;

    for (uint32_t ArrayIndex = 0; ArrayIndex < pi_ArraySize; ArrayIndex++)
        {
        Y = m_pRGBToYRed[*pi_pRed] + m_pRGBToYGreen[*pi_pGreen] + m_pRGBToYBlue[*pi_pBlue];

        // Pre-compute division wich will be used two time to avoid one.
        // m_ReferenceWhiteXYZ [1] (Lighness) is == 1 so...
        // supposed to be : TempValue = Y / m_ReferenceWhiteXYZ [1];
        TempValue = Y;

        XAnd15YAnd3Z = m_pRed_XAnd15YAnd3Z[*pi_pRed] + m_pGreen__XAnd15YAnd3Z[*pi_pGreen] + m_pBlue__XAnd15YAnd3Z[*pi_pBlue];

        // Compute L value (Ligthness)
        if (TempValue > CIE_E)
            *po_pL = 116.0 * LimitedFastCubicRoot(TempValue) - 16.0;
        else
            *po_pL = CIE_K * TempValue;

        // Compute U and V value (Chroma)
        // Avoid zero division.
        if ( XAnd15YAnd3Z > 9.99999E-8 )
            {
            // A division a mush slower than a mul.
            // so, divide once, mul the inverse two time...
            TempDivision = 1 / XAnd15YAnd3Z;

            // Use a temp value to save a multiply.
            TempValue = 13.0 * (*po_pL);

            *po_pU = TempValue * ((X4 * TempDivision) - m_UPrime);
            *po_pV = TempValue * ((X9 * TempDivision) - m_VPrime);
            }
        else
            {
            *po_pU = 0;
            *po_pV = 0;
            }

        // Go to the next pixel.
        ++pi_pRed;
        ++pi_pGreen;
        ++pi_pBlue;
        ++po_pL;
        ++po_pU;
        ++po_pV;
        }
    }

//----------------------------------------------------------------------------
// Compute Chroma, Hue and Psychometric saturation from LUV value.
//----------------------------------------------------------------------------
// Un-optimize version.
//

void HGFLuvColorSpace::GetChromaHueSaturation( double pi_L, double pi_U, double pi_V,
                                               double* po_Chroma, double* po_Hue, double* po_Saturation)  const
    {
    *po_Chroma     = sqrt((pi_U * pi_U)  + (pi_V * pi_V));
    *po_Hue        = atan2 (pi_V, pi_U);   // arctan (V / U)
    *po_Saturation = *po_Chroma / pi_L;
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

bool HGFLuvColorSpace::SafeConvertToRGB (double  pi_L,      double  pi_U,        double   pi_V,
                                          Byte*  po_pRed,   Byte*  po_pGreen,   Byte*   po_pBlue) const
    {
    double UPrime;
    double VPrime;

    double X;
    double Y;
    double Z;

    double NormalizedColor;
    double PRECISION_EPSILON = 0.0001999;

    double ChannelMaxValue = m_ChannelSize - 1.0;

    bool  ConversionDone = true;

    // Do not allow zero division.
    double TempValue = 1.0 / (13.0 * MAX(pi_L, 0.0001));

    UPrime = (pi_U * TempValue) + m_UPrime;
    VPrime = (pi_V * TempValue) + m_VPrime;

    if (pi_L > CIE_E)
        {
        // Use a Temp value to avoid unnecessary division..
        TempValue = (pi_L + 16.0) / 116.0;

        // Instead of using pow(TempValue, 3)...
        Y = TempValue * TempValue * TempValue;
        }
    else
        Y = pi_L / CIE_K;

    //--------------------------------------------------------------
    // Convert from L*u*v* to CIE XYZ color space
    // Y = TempValue * TempValue * TempValue;      // Instead of using pow(TempValue, 3)...
    X = (-9.0 * Y * UPrime) / (((UPrime - 4.0) * VPrime) - (UPrime * VPrime));
    Z = ((9.0 * Y) - (15.0 * VPrime * Y) - VPrime * X) / (3.0 * VPrime);

    //--- Red ------------------------------------------------------
    // Get the red component normalized (value within [0,1] range)
    NormalizedColor = (m_RGBToXYZMatrix[0][0] * X) + (m_RGBToXYZMatrix[0][1] * Y) + (m_RGBToXYZMatrix[0][2] * Z);

    if (NormalizedColor < 0.0 || NormalizedColor > 1.0)
        ConversionDone = false;

    NormalizedColor = MIN(NormalizedColor, 1.0);
    NormalizedColor = MAX(NormalizedColor, 0.0);

    if (m_UseGammaCorrection)
        {
        *po_pRed = (Byte)(ChannelMaxValue * pow( NormalizedColor, 1.0/m_GammaCorrectionFactor));
        }
    else
        {
        *po_pRed = (Byte)((NormalizedColor * ChannelMaxValue) + PRECISION_EPSILON);
        }

    //--- Green ----------------------------------------------------
    // Get the green component normalized (value within [0,1] range)
    NormalizedColor = (m_RGBToXYZMatrix[1][0] * X) + (m_RGBToXYZMatrix[1][1] * Y) + (m_RGBToXYZMatrix[1][2] * Z);

    if (NormalizedColor < 0.0 || NormalizedColor > 1.0)
        ConversionDone = false;

    NormalizedColor = MIN(NormalizedColor, 1.0);
    NormalizedColor = MAX(NormalizedColor, 0.0);

    if (m_UseGammaCorrection)
        {
        *po_pGreen = (Byte)(ChannelMaxValue * pow( NormalizedColor, 1.0/m_GammaCorrectionFactor));
        }
    else
        {
        *po_pGreen = (Byte)((NormalizedColor * ChannelMaxValue) + PRECISION_EPSILON);
        }

    //--- Blue -----------------------------------------------------
    // Get the blue component normalized (value within [0,1] range)
    NormalizedColor  = (m_RGBToXYZMatrix[2][0] * X) + (m_RGBToXYZMatrix[2][1] * Y) + (m_RGBToXYZMatrix[2][2] * Z);

    if (NormalizedColor < 0.0 || NormalizedColor > 1.0)
        ConversionDone = false;

    NormalizedColor = MIN(NormalizedColor, 1.0);
    NormalizedColor = MAX(NormalizedColor, 0.0);

    if (m_UseGammaCorrection)
        {
        *po_pBlue = (Byte)(ChannelMaxValue * pow( NormalizedColor, 1.0/m_GammaCorrectionFactor));
        }
    else
        {
        *po_pBlue = (Byte)((NormalizedColor * ChannelMaxValue) + PRECISION_EPSILON);
        }

    return ConversionDone;
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

bool HGFLuvColorSpace::SafeConvertToRGB (double   pi_L,    double   pi_U,      double   pi_V,
                                          unsigned short*  po_pRed, unsigned short*  po_pGreen, unsigned short*  po_pBlue) const
    {
    double UPrime;
    double VPrime;

    double X;
    double Y;
    double Z;

    double NormalizedColor;
    double PRECISION_EPSILON = 0.0001999;

    bool  ConversionDone = true;

    double ChannelMaxValue = m_ChannelSize - 1.0;

    // Do not allow zero division.
    double TempValue = 1.0 / (13.0 * MAX(pi_L, 0.001));

    UPrime = (pi_U * TempValue) + m_UPrime;
    VPrime = (pi_V * TempValue) + m_VPrime;

    if (pi_L > CIE_E)
        {
        // Use a Temp value to avoid unnecessary division..
        TempValue = (pi_L + 16.0) / 116.0;

        // Instead of using pow(TempValue, 3)...
        Y = TempValue * TempValue * TempValue;
        }
    else
        Y = pi_L / CIE_K;

    //--------------------------------------------------------------
    // Convert from L*u*v* to CIE XYZ color space
    X = (-9.0 * Y * UPrime) / (((UPrime - 4.0) * VPrime) - (UPrime * VPrime));
    Z = ((9.0 * Y) - (15.0 * VPrime * Y) - VPrime * X) / (3.0 * VPrime);

    //--- Red ------------------------------------------------------
    // Get the red component normalized (value within [0,1] range)
    NormalizedColor = (m_RGBToXYZMatrix[0][0] * X) + (m_RGBToXYZMatrix[0][1] * Y) + (m_RGBToXYZMatrix[0][2] * Z);

    if (NormalizedColor < 0.0 || NormalizedColor > 1.0)
        {
        ConversionDone = false;
        }

    NormalizedColor = MIN(NormalizedColor, 1.0);
    NormalizedColor = MAX(NormalizedColor, 0.0);

    if (m_UseGammaCorrection)
        {
        *po_pRed = (unsigned short)(ChannelMaxValue * pow( NormalizedColor, 1.0/m_GammaCorrectionFactor));
        }
    else
        {
        // Convert the processed value to the standard domain [0,255]
        *po_pRed = (unsigned short)((NormalizedColor * ChannelMaxValue) + PRECISION_EPSILON);
        }

    //--- Green ----------------------------------------------------
    // Get the green component normalized (value within [0,1] range)
    NormalizedColor = (m_RGBToXYZMatrix[1][0] * X) + (m_RGBToXYZMatrix[1][1] * Y) + (m_RGBToXYZMatrix[1][2] * Z);

    if (NormalizedColor < 0.0 || NormalizedColor > 1.0)
        ConversionDone = false;

    NormalizedColor = MIN(NormalizedColor, 1.0);
    NormalizedColor = MAX(NormalizedColor, 0.0);

    if (m_UseGammaCorrection)
        {
        *po_pGreen = (unsigned short)(ChannelMaxValue * pow( NormalizedColor, 1.0/m_GammaCorrectionFactor));
        }
    else
        {
        // Convert the processed value to the standard domain [0,255]
        *po_pGreen = (unsigned short)((NormalizedColor * ChannelMaxValue) + PRECISION_EPSILON);
        }

    //--- Blue -----------------------------------------------------
    // Get the blue component normalized (value within [0,1] range)
    NormalizedColor  = (m_RGBToXYZMatrix[2][0] * X) + (m_RGBToXYZMatrix[2][1] * Y) + (m_RGBToXYZMatrix[2][2] * Z);

    if (NormalizedColor < 0.0 || NormalizedColor > 1.0)
        ConversionDone = false;

    NormalizedColor = MIN(NormalizedColor, 1.0);
    NormalizedColor = MAX(NormalizedColor, 0.0);

    if (m_UseGammaCorrection)
        {
        *po_pBlue = (unsigned short)(ChannelMaxValue * pow( NormalizedColor, 1.0/m_GammaCorrectionFactor));
        }
    else
        {
        // Convert the processed value to the standard domain [0,255]
        *po_pBlue = (unsigned short)((NormalizedColor * ChannelMaxValue) + PRECISION_EPSILON);
        }
    return ConversionDone;
    }

#if 0
//----------------------------------------------------------------------------
// For information only, the purppose of this method is only to evaluate
// LUV domain from some RGB conversion.
//----------------------------------------------------------------------------

void HGFLuvColorSpace::RetainMinMax(double pi_L, double pi_U, double pi_V,
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

