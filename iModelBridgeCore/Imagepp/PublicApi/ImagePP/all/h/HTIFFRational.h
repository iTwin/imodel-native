//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HTIFFRational.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HTIFFRational
//-----------------------------------------------------------------------------
// This class is a wrapper for the rational number format used in the TIFF standard


BEGIN_IMAGEPP_NAMESPACE
template<typename T>
class HTIFFRational
    {
public:
    HTIFFRational                      (const T         pi_Numerator,
                                        const T         pi_Denominator);

    // All rational numbers operations could be implemented if needed with operator overloading

    T                           GetNumerator                       () const;
    T                           GetDenominator                     () const;

    double                     ConvertToDouble                    () const;


private:
    T                           m_Numerator;
    T                           m_Denominator;
    };


typedef HTIFFRational<uint32_t> HTIFFURational32;
typedef HTIFFRational<int32_t> HTIFFSRational32;
END_IMAGEPP_NAMESPACE


#include "HTIFFRational.hpp"
