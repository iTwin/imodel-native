//:>--------------------------------------------------------------------------------------+
//:>
//:>    $Source: PublicApi/ImagePP/all/h/HTIFFRational.hpp $
//:>
//:>    $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HTIFFRational
//-----------------------------------------------------------------------------


BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Public
// Constructor
//-----------------------------------------------------------------------------
template<typename T>
inline HTIFFRational<T>::HTIFFRational (const T pi_Numerator,
                                        const T pi_Denominator)
    :   m_Numerator     (pi_Numerator),
        m_Denominator   ((pi_Denominator != 0) ? pi_Denominator : 1)

    {
    HPRECONDITION(pi_Denominator != 0); // Invalid rational number. We will use 1 as denominator...
    }

//-----------------------------------------------------------------------------
// Public
// Accessor for the numerator
//-----------------------------------------------------------------------------
template<typename T>
inline T HTIFFRational<T>::GetNumerator() const
    {
    return m_Numerator;
    }


//-----------------------------------------------------------------------------
// Public
// Accessor for the denominator
//-----------------------------------------------------------------------------
template<typename T>
inline T HTIFFRational<T>::GetDenominator() const
    {
    return m_Denominator;
    }


//-----------------------------------------------------------------------------
// Public
// Conversion to double
//-----------------------------------------------------------------------------
template<typename T>
inline double HTIFFRational<T>::ConvertToDouble() const
    {
    if (m_Numerator == 0) // Denominator is not checked against zero because it is never allowed to take 0 as a value
        return 0.0;
    else
        return m_Numerator / static_cast<double>(m_Denominator);
    }
END_IMAGEPP_NAMESPACE
