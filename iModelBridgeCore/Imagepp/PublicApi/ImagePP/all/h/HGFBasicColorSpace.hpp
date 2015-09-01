//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFBasicColorSpace.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Basic XYZ ColorSpace converter base class inline method
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

inline bool HGFBasicColorSpace::IsGammaCorrected()
    {
    return m_UseGammaCorrection;
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

inline double HGFBasicColorSpace::GetGammaCorrectionFactor()
    {
    // Do not ask for something we cannot use...
    HASSERT(!m_UseGammaCorrection);

    return m_GammaCorrectionFactor;
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
/*
inline void HGFBasicColorSpace::ConvertToXYZ(Byte    pi_Red,Byte   pi_Green, Byte   pi_Blue,
                                             double  *po_pX, double *po_pY,    double *po_pZ)
{
    // Domain value according sRGB Rec 709 (D65 white point) to CIE XYZ
    // is : X[0.0 , 95.02] Y[0.0 , 100] Z[0.0 , 108.74]

    *po_pX = m_pRGBToXRed[pi_Red] + m_pRGBToXGreen[pi_Green] + m_pRGBToXBlue[pi_Blue];
    *po_pY = m_pRGBToYRed[pi_Red] + m_pRGBToYGreen[pi_Green] + m_pRGBToYBlue[pi_Blue];
    *po_pZ = m_pRGBToZRed[pi_Red] + m_pRGBToZGreen[pi_Green] + m_pRGBToZBlue[pi_Blue];
} /* */

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

inline void HGFBasicColorSpace::ConvertToXYZ(unsigned short pi_Red,unsigned short pi_Green, unsigned short pi_Blue,
                                             double*  po_pX, double* po_pY,    double* po_pZ)
    {
    // Domain value according sRGB Rec 709 (D65 white point) to CIE XYZ
    // is : X[0.0 , 95.02] Y[0.0 , 100] Z[0.0 , 108.74]

    *po_pX = m_pRGBToXRed[pi_Red] + m_pRGBToXGreen[pi_Green] + m_pRGBToXBlue[pi_Blue];
    *po_pY = m_pRGBToYRed[pi_Red] + m_pRGBToYGreen[pi_Green] + m_pRGBToYBlue[pi_Blue];
    *po_pZ = m_pRGBToZRed[pi_Red] + m_pRGBToZGreen[pi_Green] + m_pRGBToZBlue[pi_Blue];
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

inline void HGFBasicColorSpace::ConvertFromXYZ(double  pi_X,   double  pi_Y,     double  pi_Z,
                                               Byte*  po_pRed,Byte*  po_pGreen, Byte* po_pBlue)
    {
    // Should used a HGFGrid, but at this time, an EPSILON should be correct.
    double PRECISION_EPSILON = 0.0001999;
    double NormalizedColor;
    double MaxChannelValue = m_ChannelSize - 1.0;

    if (m_UseGammaCorrection)
        {
        double GammaCorrectionInverse = 1 / m_GammaCorrectionFactor;

        //--- Red ------------------------------------------------------
        // Get the red component normalized (value within [0,1] range)
        NormalizedColor = (m_RGBToXYZMatrix[0][0] * pi_X) + (m_RGBToXYZMatrix[0][1] * pi_Y) + (m_RGBToXYZMatrix[0][2] * pi_Z);

        // Remove applied gamma correction
        NormalizedColor = pow(NormalizedColor, GammaCorrectionInverse);

        // Convert the processed value to the standard domain [0,255]
        *po_pRed = (Byte)((NormalizedColor * MaxChannelValue) + PRECISION_EPSILON);

        //--- Green ----------------------------------------------------
        // Get the green component normalized (value within [0,1] range)
        NormalizedColor = (m_RGBToXYZMatrix[1][0] * pi_X) + (m_RGBToXYZMatrix[1][1] * pi_Y) + (m_RGBToXYZMatrix[1][2] * pi_Z);

        // Remove applied gamma correction
        NormalizedColor = pow(NormalizedColor, GammaCorrectionInverse);

        // Convert the processed value to the standard domain [0,255]
        *po_pGreen = (Byte)((NormalizedColor * MaxChannelValue) + PRECISION_EPSILON);

        //--- Blue -----------------------------------------------------
        // Get the blue component normalized (value within [0,1] range)
        NormalizedColor  = (m_RGBToXYZMatrix[2][0] * pi_X) + (m_RGBToXYZMatrix[2][1] * pi_Y) + (m_RGBToXYZMatrix[2][2] * pi_Z);

        // Remove applied gamma correction
        NormalizedColor = pow(NormalizedColor, GammaCorrectionInverse);

        // Convert the processed value to the standard domain [0,255]
        *po_pBlue = (Byte)((NormalizedColor * MaxChannelValue) + PRECISION_EPSILON);
        }
    else
        {
        // Get the red component normalized (value within [0,1] range)
        NormalizedColor = (m_RGBToXYZMatrix[0][0] * pi_X) + (m_RGBToXYZMatrix[0][1] * pi_Y) + (m_RGBToXYZMatrix[0][2] * pi_Z);

        // Convert the processed value to the standard domain [0,255]
        *po_pRed = (Byte)((NormalizedColor * MaxChannelValue) + PRECISION_EPSILON);

        // Get the green component normalized (value within [0,1] range)
        NormalizedColor = (m_RGBToXYZMatrix[1][0] * pi_X) + (m_RGBToXYZMatrix[1][1] * pi_Y) + (m_RGBToXYZMatrix[1][2] * pi_Z);

        // Convert the processed value to the standard domain [0,255]
        *po_pGreen = (Byte)((NormalizedColor * MaxChannelValue) + PRECISION_EPSILON);

        // Get the blue component normalized (value within [0,1] range)
        NormalizedColor  = (m_RGBToXYZMatrix[2][0] * pi_X) + (m_RGBToXYZMatrix[2][1] * pi_Y) + (m_RGBToXYZMatrix[2][2] * pi_Z);

        // Convert the processed value to the standard domain [0,255]
        *po_pBlue = (Byte)((NormalizedColor * MaxChannelValue) + PRECISION_EPSILON);
        }
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

inline void HGFBasicColorSpace::ConvertFromXYZ(double  pi_X,   double  pi_Y,      double  pi_Z,
                                               unsigned short* po_pRed,unsigned short* po_pGreen, unsigned short* po_pBlue)
    {
    // Should used a HGFGrid, but at this time, an EPSILON should be correct.
    double PRECISION_EPSILON = 0.0001999;
    double NormalizedColor;
    double MaxChannelValue = m_ChannelSize - 1.0;

    if (m_UseGammaCorrection)
        {
        double GammaCorrectionInverse = 1 / m_GammaCorrectionFactor;

        //--- Red ------------------------------------------------------
        // Get the red component normalized (value within [0,1] range)
        NormalizedColor = (m_RGBToXYZMatrix[0][0] * pi_X) + (m_RGBToXYZMatrix[0][1] * pi_Y) + (m_RGBToXYZMatrix[0][2] * pi_Z);

        // Remove applied gamma correction
        NormalizedColor = pow(NormalizedColor, GammaCorrectionInverse);

        // Convert the processed value to the standard domain [0,255]
        *po_pRed = (unsigned short)((NormalizedColor * MaxChannelValue) + PRECISION_EPSILON);

        //--- Green ----------------------------------------------------
        // Get the green component normalized (value within [0,1] range)
        NormalizedColor = (m_RGBToXYZMatrix[1][0] * pi_X) + (m_RGBToXYZMatrix[1][1] * pi_Y) + (m_RGBToXYZMatrix[1][2] * pi_Z);

        // Remove applied gamma correction
        NormalizedColor = pow(NormalizedColor, GammaCorrectionInverse);

        // Convert the processed value to the standard domain [0,255]
        *po_pGreen = (unsigned short)((NormalizedColor * MaxChannelValue) + PRECISION_EPSILON);

        //--- Blue -----------------------------------------------------
        // Get the blue component normalized (value within [0,1] range)
        NormalizedColor  = (m_RGBToXYZMatrix[2][0] * pi_X) + (m_RGBToXYZMatrix[2][1] * pi_Y) + (m_RGBToXYZMatrix[2][2] * pi_Z);

        // Remove applied gamma correction
        NormalizedColor = pow(NormalizedColor, GammaCorrectionInverse);

        // Convert the processed value to the standard domain [0,255]
        *po_pBlue = (unsigned short)((NormalizedColor * MaxChannelValue) + PRECISION_EPSILON);
        }
    else
        {
        // Get the red component normalized (value within [0,1] range)
        NormalizedColor = (m_RGBToXYZMatrix[0][0] * pi_X) + (m_RGBToXYZMatrix[0][1] * pi_Y) + (m_RGBToXYZMatrix[0][2] * pi_Z);

        // Convert the processed value to the standard domain [0,255]
        *po_pRed = (unsigned short)((NormalizedColor * MaxChannelValue) + PRECISION_EPSILON);

        // Get the green component normalized (value within [0,1] range)
        NormalizedColor = (m_RGBToXYZMatrix[1][0] * pi_X) + (m_RGBToXYZMatrix[1][1] * pi_Y) + (m_RGBToXYZMatrix[1][2] * pi_Z);

        // Convert the processed value to the standard domain [0,255]
        *po_pGreen = (unsigned short)((NormalizedColor * MaxChannelValue) + PRECISION_EPSILON);

        // Get the blue component normalized (value within [0,1] range)
        NormalizedColor  = (m_RGBToXYZMatrix[2][0] * pi_X) + (m_RGBToXYZMatrix[2][1] * pi_Y) + (m_RGBToXYZMatrix[2][2] * pi_Z);

        // Convert the processed value to the standard domain [0,255]
        *po_pBlue = (unsigned short)((NormalizedColor * MaxChannelValue) + PRECISION_EPSILON);
        }
    }

//----------------------------------------------------------------------------
// Pure color is define as color with absence of brigthness.
// By convention, CIE use the notation of "little" x and "little" y.
//----------------------------------------------------------------------------

inline void HGFBasicColorSpace::GetPureColor(double  pi_X, double  pi_Y, double pi_Z,
                                             double* po_x, double* po_y)
    {
    // A divider is pre-compute to avoid one time consuming division;
    // two multiplications is much faster than one division.
    double Divider = 1.0 / pi_X + pi_Y + pi_Z;

    // Little x calculation.
    *po_x = pi_X * Divider;   // x = X / (X + Y + Z)

    // Little y calculation.
    *po_y = pi_Y * Divider;   // y = Y / (X + Y + Z)
    }
END_IMAGEPP_NAMESPACE
