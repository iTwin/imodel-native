//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCMaths.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline functions for maths utility.
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//----------------------------------------------------------------------------
// This fast cubic root method has been optimized. The valid domain
// range is [-MAX_DOUBLE, MAX_DOUBLE].  If the domain is restricted fom [0, 1]. We
// suggest to use LimitedFastCubicRoot, wich will be faster.
//----------------------------------------------------------------------------

inline double FastCubicRoot(double x)
    {
    int k;
    double w, y, u;

    static double c[24] = { 0.0015319394088521, -0.018843445653409,
                            0.10170534986,      -0.31702448761286,
                            0.63520892642253,   -0.88106985991189,
                            1.051750376454,      0.4267412323558,
                            1.507908365919e-5,  -3.7095709111375e-4,
                            0.0040043972242353, -0.024964114079723,
                            0.10003913718511,   -0.27751961573273,
                            0.66256121926465,    0.53766026150315,
                            1.4842542902609e-7, -7.3027601203435e-6,
                            1.5766326109233e-4, -0.0019658008013138,
                            0.015755176844105,  -0.0874132014051,
                            0.41738741349777,    0.6774094811598
                          };

    if (x == 0)
        {
        y = 0;
        return y;
        }

    if (x > 0)
        {
        w = x;
        y = 0.5;
        }
    else
        {
        w = -x;
        y = -0.5;
        }
    if (w > 8)
        {
        while (w > 281474976710656.0)
            {   // 2^48
            w *= 3.552713678800500929355621337890625e-15; // 2^(-48)
            y *= 65536.0; //  2^16
            }

        while (w > 8)
            {
            w *= 0.125;
            y *= 2;
            }
        }
    else if (w < 1)
        {
        while (w < 3.552713678800500929355621337890625e-15)
            {
            w *= 281474976710656.0;
            y *= 1.52587890625e-5; // 2^(-16)
            }
        while (w < 1)
            {
            w *= 8;
            y *= 0.5;
            }
        }

    if (w < 2)
        {
        k = 0;
        }
    else if (w < 4)
        {
        k = 8;
        }
    else
        {
        k = 16;
        }

    u = ((((((c[k] * w + c[k + 1]) * w +
             c[k + 2]) * w + c[k + 3]) * w +
           c[k + 4]) * w + c[k + 5]) * w +
         c[k + 6]) * w + c[k + 7];
    y *= u + 3 * u * w / (w + 2 * u * u * u);

    return y;
    }

//----------------------------------------------------------------------------
// This fast cubic root method has been highly optimized. The valid domain
// range is [0,1] "(inclusive).  For a much larger domain, use FastCubicRoot
// (declared below)
//----------------------------------------------------------------------------

inline double LimitedFastCubicRoot(double pi_Number)
    {
    // For optimisation, we limit this function to accept only number
    // grater than 0 and less or equal than 1.
    HPRECONDITION((pi_Number > 9.99999E-8) && (pi_Number <= 1));

    static double c[24] = { 0.0015319394088521, -0.018843445653409,
                            0.10170534986,      -0.31702448761286,
                            0.63520892642253,   -0.88106985991189,
                            1.051750376454,      0.4267412323558,
                            1.507908365919e-5,  -3.7095709111375e-4,
                            0.0040043972242353, -0.024964114079723,
                            0.10003913718511,   -0.27751961573273,
                            0.66256121926465,    0.53766026150315,
                            1.4842542902609e-7, -7.3027601203435e-6,
                            1.5766326109233e-4, -0.0019658008013138,
                            0.015755176844105,  -0.0874132014051,
                            0.41738741349777,    0.6774094811598
                          };

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

    double u = ((((((c[k]      * pi_Number + c[k + 1]) * pi_Number +
                    c[k + 2]) * pi_Number + c[k + 3]) * pi_Number +
                  c[k + 4]) * pi_Number + c[k + 5]) * pi_Number +
                c[k + 6]) * pi_Number + c[k + 7];

    y *= u + 3 * u * pi_Number / (pi_Number + 2 * u * u * u);

    return y;
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

inline double GeometricMean(double* pi_pArray, size_t pi_ArraySize)
    {
    HPRECONDITION (pi_pArray != 0);
    HPRECONDITION (pi_ArraySize > 0);

    double Mean = 1.0;

    for (size_t Index = 0; Index <= pi_ArraySize; Index++)
        Mean *= pi_pArray[Index];

    return pow(Mean, 1.0 / pi_ArraySize);
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

inline double GeometricMean(double pi_FirstValue, double pi_SecondValue)
    {
    HPRECONDITION (pi_FirstValue> 0.0 && pi_SecondValue > 0.0);

    return sqrt(pi_FirstValue * pi_SecondValue);
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

inline double ArithmeticMean(double* pi_pArray, size_t pi_ArraySize)
    {
    HPRECONDITION (pi_pArray != 0);
    HPRECONDITION (pi_ArraySize > 0);

    double Mean = 0.0;

    for (size_t Index = 0; Index <= pi_ArraySize; Index++)
        Mean += pi_pArray[Index];

    return Mean / pi_ArraySize;
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

inline double ArithmeticMean(double pi_FirstValue, double pi_SecondValue)
    {
    return (pi_FirstValue * pi_SecondValue) / 2.0;
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

inline double HarmonicMean(double* pi_pArray, size_t pi_ArraySize)
    {
    HPRECONDITION (pi_pArray != 0);
    HPRECONDITION (pi_ArraySize > 0);

    double Mean = 0.0;

    for (size_t Index = 0; Index <= pi_ArraySize; Index++)
        Mean += 1.0 / pi_pArray[Index];

    return pi_ArraySize / Mean;
    }

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

inline double HarmonicMean(double pi_FirstValue, double pi_SecondValue)
    {
    HPRECONDITION (pi_FirstValue> 0.0 && pi_SecondValue > 0.0);

    return 2.0 / ((1.0 / pi_FirstValue) + (1.0 / pi_SecondValue));
    }

END_IMAGEPP_NAMESPACE