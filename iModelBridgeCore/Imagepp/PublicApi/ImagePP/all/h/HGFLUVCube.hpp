//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFLUVCube.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Inline methods for class HGFLUVSet
//---------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline HGFLUVCube::HGFLUVCube()
    : m_Lmin(L_MIN), m_Lmax(L_MAX),
      m_Umin(U_MIN), m_Umax(U_MAX),
      m_Vmin(V_MIN), m_Vmax(V_MAX)
    {
    if (!s_Initialized)
        BuildLookupTable();
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline HGFLUVCube::HGFLUVCube(double pi_Lmin, double pi_Lmax,
                              double pi_Umin, double pi_Umax,
                              double pi_Vmin, double pi_Vmax)
    : m_Lmin(pi_Lmin), m_Lmax(pi_Lmax),
      m_Umin(pi_Umin), m_Umax(pi_Umax),
      m_Vmin(pi_Vmin), m_Vmax(pi_Vmax)
    {
    HPRECONDITION(pi_Lmin >= L_MIN && pi_Lmin <= L_MAX);
    HPRECONDITION(pi_Umin >= U_MIN && pi_Umin <= U_MAX);
    HPRECONDITION(pi_Vmin >= V_MIN && pi_Vmin <= V_MAX);

    HPRECONDITION(pi_Lmax >= L_MIN && pi_Lmax <= L_MAX);
    HPRECONDITION(pi_Umax >= U_MIN && pi_Umax <= U_MAX);
    HPRECONDITION(pi_Vmax >= V_MIN && pi_Vmax <= V_MAX);

    HPRECONDITION(pi_Lmin < pi_Lmax);
    HPRECONDITION(pi_Umin < pi_Umax);
    HPRECONDITION(pi_Vmin < pi_Vmax);


    if (!s_Initialized)
        BuildLookupTable();
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

inline HGFLUVCube::HGFLUVCube(const HGFLUVCube& pi_rSrc)
    : m_Lmin(pi_rSrc.m_Lmin), m_Lmax(pi_rSrc.m_Lmax),
      m_Umin(pi_rSrc.m_Umin), m_Umax(pi_rSrc.m_Umax),
      m_Vmin(pi_rSrc.m_Vmin), m_Vmax(pi_rSrc.m_Vmax)
    {

    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

inline HGFLUVCube::~HGFLUVCube()
    {
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

inline HGFLUVCube& HGFLUVCube::operator=(const HGFLUVCube& pi_rSrc)
    {
     if (this != &pi_rSrc)
        {
        m_Lmin = pi_rSrc.m_Lmin;
        m_Lmax = pi_rSrc.m_Lmax;
        m_Umin = pi_rSrc.m_Umin;
        m_Umax = pi_rSrc.m_Umax;
        m_Vmin = pi_rSrc.m_Vmin;
        m_Vmax = pi_rSrc.m_Vmax;
        }

    return *this;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline double HGFLUVCube::LimitedFastCubicRoot(double pi_Number) const
    {
    // For optimisation, we limit this function to accept only number
    // grater than 0 and less or equal than 1.
    HASSERT((pi_Number > 9.99999E-8) && (pi_Number <= 1));

    double y = 0.5;

    while (pi_Number < 1)
        {
        y *= 0.5;
        pi_Number *= 8;
        }

    int k;

    if (pi_Number < 2)
        {
        k = 0;
        }
    else if (pi_Number < 4)
        {
        k = 8;
        }
    else
        {
        k = 16;
        }

    // Be sure to not be out of bound.
    HASSERT(k <= 17);

    // s_C is a static member of this class.
    double u = ((((((s_C[k]      * pi_Number + s_C[k + 1]) * pi_Number +
                     s_C[k + 2]) * pi_Number + s_C[k + 3]) * pi_Number +
                   s_C[k + 4]) * pi_Number + s_C[k + 5]) * pi_Number +
                 s_C[k + 6]) * pi_Number + s_C[k + 7];

    y *= u + 3 * u * pi_Number / (pi_Number + 2 * u * u * u);

    return y;
    }


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline bool HGFLUVCube::IsIn(Byte pi_Red, Byte pi_Green, Byte pi_Blue) const
    {
    double ColorL;
    double ColorU;
    double ColorV;

    double Y = s_RGBToYRed[pi_Red] + s_RGBToYGreen[pi_Green] + s_RGBToYBlue[pi_Blue];
    double TempValue;
    double TempDivision;

    double X4 = s_XRed_x4[pi_Red] + s_XGreen_x4[pi_Green] + s_XBlue_x4[pi_Blue];
    double X9 = s_YRed_x9[pi_Red] + s_YGreen_x9[pi_Green] + s_YBlue_x9[pi_Blue];

    // Pre-compute division wich will be used two time to avoid one.
    TempValue = Y / s_ReferenceWhiteXYZ [1];

    double XAnd15YAnd3Z = s_Red_XAnd15YAnd3Z[pi_Red] + s_Green__XAnd15YAnd3Z[pi_Green] + s_Blue__XAnd15YAnd3Z[pi_Blue];

    // Compute L value (Ligthness)
    if (TempValue > 0.008856)
        ColorL = 116.0 * LimitedFastCubicRoot(TempValue) - 16.0;
    else
        ColorL = 903.29999 * TempValue; // ColorL = (116.0 * (7.787 * (Y / ReferenceWhiteXYZ [1])) + (16.0 / 116.0)) - 16.0;

    // Compute U and V value (Chroma)
    // Avoid zero division.
    if ( XAnd15YAnd3Z > 9.99999E-8 )
        {
        // Use a temp value to save a multiply.
        TempValue = 13.0 * ColorL;

        // A division a mush slower than a mul.
        // so, divide once, mul the inverse two time...
        TempDivision = 1 / XAnd15YAnd3Z;

        ColorU = TempValue * ((X4 * TempDivision) - s_UPrime);
        ColorV = TempValue * ((X9 * TempDivision) - s_VPrime);
        }
    else
        {
        ColorU = 0;
        ColorV = 0;
        }

    return    ((ColorL >= m_Lmin) && (ColorL <= m_Lmax))
              && ((ColorU >= m_Umin) && (ColorU <= m_Umax))
              && ((ColorV >= m_Vmin) && (ColorV <= m_Vmax));
    }
END_IMAGEPP_NAMESPACE