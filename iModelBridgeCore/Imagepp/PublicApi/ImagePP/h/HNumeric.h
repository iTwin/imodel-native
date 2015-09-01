//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/h/HNumeric.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include <limits>

BEGIN_IMAGEPP_NAMESPACE
/**

    The HFCNumericLimits is a template class used to encapsulate limitations
    to different numerical data types.

    The template class is instanciated for the different known datatypes
    and returns values such as tolerance and tolerance factors.

    The class is derived from the numeric_limits stl template class and
    thus all numeric_limits members are automatically defined

*/
#define HNUMERIC_local_min(a, b)  ((a)<(b)?(a):(b))
#define HNUMERIC_local_max(a, b)  ((a)>(b)?(a):(b))

template<class DataType> class HNumeric : std::numeric_limits<DataType>
    {
public:
    /**----------------------------------------------------------------------------
     STATIC METHOD
     EQUAL
     This static method allows comparing the equality of two values within a given
     tolerance.

     @param v1 First value to compare.

     @param v2 Second value to compare.

     @param precision Tolerance to apply in compare operation

     @see HNumeric<T>::EQUAL_EPSILON()
     @see HNumeric<T>::EQUAL_AUTO_EPSILON()
    -----------------------------------------------------------------------------*/
    static bool EQUAL(DataType v1, DataType v2, DataType precision) {
        return ((v1 <= (v2+precision)) && (v1 >= (v2-precision)));
        }

    /**----------------------------------------------------------------------------
     STATIC METHOD
     EQUAL_EPSILON
     This static method allows comparing the equality of two values within a global
     tolerance of HNumeric<T>::GLOBAL_EPSILON().

     @param v1 First value to compare.

     @param v2 Second value to compare.

     @see HNumeric<T>::EQUAL()
     @see HNumeric<T>::EQUAL_AUTO_EPSILON()
    -----------------------------------------------------------------------------*/
    static bool EQUAL_EPSILON(DataType v1, DataType v2) {
        return ((v1 <= (v2+GLOBAL_EPSILON())) && (v1 >= (v2-GLOBAL_EPSILON())));
        }

    /**----------------------------------------------------------------------------
     STATIC METHOD
     EQUAL_AUTO_EPSILON
     This static method allows comparing the equality of two values within a computed
     tolerance as calculated by HNumeric<T>::AUTO_EPSILON().

     Note that since the tolerance varies with the values provided, comparing with 0.0
     will result in a tolerance small enough that 0.0 will not be included in the interval.
     It follows that comparing with 0.0 for either of the two values will return false
     unless both values are exactly 0.0

     @param v1 First value to compare.

     @param v2 Second value to compare.

     @see HNumeric<T>::EQUAL()
     @see HNumeric<T>::EQUAL_EPSILON()
    -----------------------------------------------------------------------------*/
    static bool EQUAL_AUTO_EPSILON(DataType v1, DataType v2) {
        return ((v1 <= (v2+AUTO_EPSILON(v1, v2))) && (v1 >= (v2-AUTO_EPSILON(v1, v2))));
        }

    /**----------------------------------------------------------------------------
     STATIC METHOD
     GREATER_OR_EQUAL
     This static method allows comparing the inequality of two values within a given
     tolerance.

     @param v1 First value to compare.

     @param v2 Second value to compare.

     @param precision Tolerance to apply in compare operation

     @see HNumeric<T>::GREATER_OR_EQUAL_EPSILON()
     @see HNumeric<T>::SMALLER_OR_EQUAL()
    -----------------------------------------------------------------------------*/
    static bool GREATER_OR_EQUAL(DataType v1, DataType v2, DataType precision) {
        return (v1 >= (v2-precision));
        }

    /**----------------------------------------------------------------------------
     STATIC METHOD
     GREATER_OR_EQUAL_EPSILON
     This static method allows comparing the inequality of two values within a global
     tolerance of HNumeric<T>::GLOBAL_EPSILON().

     @param v1 First value to compare.

     @param v2 Second value to compare.

     @see HNumeric<T>::GREATER_OR_EQUAL()
     @see HNumeric<T>::SMALLER_OR_EQUAL_EPSILON()
    -----------------------------------------------------------------------------*/
    static bool GREATER_OR_EQUAL_EPSILON(DataType v1, DataType v2) {
        return (v1 >= (v2-GLOBAL_EPSILON()));
        }

    /**----------------------------------------------------------------------------
     STATIC METHOD
     SMALLER_OR_EQUAL
     This static method allows comparing the inequality of two values within a given
     tolerance.

     @param v1 First value to compare.

     @param v2 Second value to compare.

     @param precision Tolerance to apply in compare operation

     @see HNumeric<T>::SMALLER_OR_EQUAL_EPSILON()
     @see HNumeric<T>::GREATER_OR_EQUAL()
    -----------------------------------------------------------------------------*/
    static bool SMALLER_OR_EQUAL(DataType v1, DataType v2, DataType precision) {
        return (v1 <= (v2+precision));
        }


    /**----------------------------------------------------------------------------
     STATIC METHOD
     SMALLER_OR_EQUAL_EPSILON
     This static method allows comparing the inequality of two values within a global
     tolerance of HNumeric<T>::GLOBAL_EPSILON().

     @param v1 First value to compare.

     @param v2 Second value to compare.

     @see HNumeric<T>::GREATER_OR_EQUAL()
     @see HNumeric<T>::SMALLER_OR_EQUAL_EPSILON()
    -----------------------------------------------------------------------------*/
    static bool SMALLER_OR_EQUAL_EPSILON(DataType v1, DataType v2) {
        return (v1 <= (v2+GLOBAL_EPSILON()));
        }


    /**----------------------------------------------------------------------------
     STATIC METHOD
     GREATER
     This static method allows comparing the inequality of two values within a given
     tolerance.

     @param v1 First value to compare.

     @param v2 Second value to compare.

     @param precision Tolerance to apply in compare operation

     @see HNumeric<T>::GREATER_OR_EQUAL()
     @see HNumeric<T>::SMALLER()
    -----------------------------------------------------------------------------*/
    static bool GREATER(DataType v1, DataType v2, DataType precision) {
        return (v1 > (v2+precision));
        }

    /**----------------------------------------------------------------------------
     STATIC METHOD
     GREATER_EPSILON
     This static method allows comparing the inequality of two values within a global
     tolerance of HNumeric<T>::GLOBAL_EPSILON().

     @param v1 First value to compare.

     @param v2 Second value to compare.

     @see HNumeric<T>::GREATER_OR_EQUAL_EPSILON()
     @see HNumeric<T>::SMALLER_EPSILON()
    -----------------------------------------------------------------------------*/
    static bool GREATER_EPSILON(DataType v1, DataType v2) {
        return (v1 > (v2+GLOBAL_EPSILON()));
        }


    /**----------------------------------------------------------------------------
     STATIC METHOD
     SMALLER
     This static method allows comparing the inequality of two values within a given
     tolerance.

     @param v1 First value to compare.

     @param v2 Second value to compare.

     @param precision Tolerance to apply in compare operation

     @see HNumeric<T>::SMALLER_OR_EQUAL()
     @see HNumeric<T>::GREATER()
    -----------------------------------------------------------------------------*/
    static bool SMALLER(DataType v1, DataType v2, DataType precision) {
        return (v1 < (v2-precision));
        }


    /**----------------------------------------------------------------------------
     STATIC METHOD
     SMALLER_EPSILON
     This static method allows comparing the inequality of two values within a global
     tolerance of HNumeric<T>::GLOBAL_EPSILON().

     @param v1 First value to compare.

     @param v2 Second value to compare.

     @see HNumeric<T>::GREATER_EPSILON()
     @see HNumeric<T>::SMALLER_OR_EQUAL_EPSILON()
    -----------------------------------------------------------------------------*/
    static bool SMALLER_EPSILON(DataType v1, DataType v2) {
        return (v1 <= (v2-GLOBAL_EPSILON()));
        }


    /**----------------------------------------------------------------------------
     STATIC METHOD
     AUTO_EPSILON
     This static method computes the automatic epsilon applicable to the comparison of
     the two provided values. The automatic epsilon is the absolute value of the
     highest absolute value of the two numbers to compare multiplied by GLOBAL_EPSILON().

     Note that since the tolerance varies with the values provided, comparing with 0.0
     will result in a tolerance small enough that 0.0 will not be included in the interval.
     It follows that comparing with 0.0 for either of the two values will return false
     unless both values are exactly 0.0

     @param v1 First value to compare.

     @param v2 Second value to compare.

     @see HNumeric<T>::GLOBAL_EPSILON()
    -----------------------------------------------------------------------------*/
    static DataType AUTO_EPSILON(DataType v1, DataType v2) {
        return (v1 < 0 ? GLOBAL_EPSILON() * -(HNUMERIC_local_min(v1, v2)) : GLOBAL_EPSILON() * HNUMERIC_local_max(v1, v2));
        }


    /**----------------------------------------------------------------------------
     STATIC METHOD
     GLOBAL_EPSILON
     This static method returns the global epsilon applicable to the comparison of
     the two values for this DataType.

     For any integral type, this epsilon should be 0. For floating point values,
     the method should be template overlaoded.
    -----------------------------------------------------------------------------*/
    static DataType GLOBAL_EPSILON() {
        return std::numeric_limits<DataType>::epsilon();
        }

    /**----------------------------------------------------------------------------
     STATIC METHOD
     EPSILON_MULTIPLICATOR
     This static method returns the epsilon multiplicator applicable to obtain the
     epsilon from values. Typically, the value is 1 / (NumberOfDigits-1) for the
     datatype.

     For any integral type, this epsilon should be 0. For floating point values,
     the method should be template overlaoded.
    -----------------------------------------------------------------------------*/
    static DataType EPSILON_MULTIPLICATOR() {
        return numeric_limits<DataType>::epsilon();
        }

    /**----------------------------------------------------------------------------
     STATIC METHOD
     MAX_EPSILON
     This static method returns the maximum epsilon that will remain applicable to
     the datatype. This value should be smaller than the maximum value of the
     datatype multiplied by EPSILON_MULTIPLICATOR() for this same datatype.
    -----------------------------------------------------------------------------*/
    static DataType MAX_EPSILON() {
        return (numeric_limits<DataType>::epsilon());
        }

#ifdef __HMR_DEBUG
    /**----------------------------------------------------------------------------
     STATIC METHOD
     DEBUG METHOD
     EQUAL_FACTOR
     This static method allows comparing the equality of two values within a
     tolerance based on the number of digits significant for the greatest of the
     two values provided. The number of digits is provided as parameters. If
     the number of digits is 0 then 1 digit is nevertheless used. If the number of
     digits is too high for the DataType in use, an overflow may result.

     The method is provided for debugging purposes only since precision computations
     require a significant amount of time.

     @param v1 First value to compare.

     @param v2 Second value to compare.

     @param Digits A positive integer specifying the number of digits of precision
                   desired.

     @see HNumeric<T>::EQUAL()
     @see HNumeric<T>::EQUAL_EPSILON()
    -----------------------------------------------------------------------------*/
    static bool EQUAL_FACTOR(DataType v1, DataType v2, uint32_t Digits) {
        return ((v1 <= (v2+AUTO_EPSILON(v1, v2, Digits))) && (v1 >= (v2-AUTO_EPSILON(v1, v2, Digits))));
        }

    /**----------------------------------------------------------------------------
     STATIC METHOD
     DEBUG METHOD
     AUTO_EPSILON
     This static method computes the automatic epsilon applicable to the comparison of
     the two provided values. The automatic epsilon is the absolute value of the
     highest absolute value of the two numbers to compare multiplied by EPSILON_FACTOR().

     Note that since the tolerance varies with the values provided, comparing with 0.0
     will result in a tolerance small enough that 0.0 will not be included in the interval.
     It follows that comparing with 0.0 for either of the two values will return false
     unless both values are exactly 0.0

     If the number of digits is 0 then 1 digit is nevertheless used. If the number of
     digits is too high for the DataType in use, an overflow may result.

     The method is provided for debugging purposes only since precision computations
     require a significant amount of time.


     @param v1 First value to compare.

     @param v2 Second value to compare.

     @param Digits A positive integer specifying the number of digits of precision
                   desired.

     @see HNumeric<T>::GLOBAL_EPSILON()
    -----------------------------------------------------------------------------*/
    static DataType AUTO_EPSILON(DataType v1, DataType v2, uint32_t Digits) {
        return (v1 < 0 ? EPSILON_FACTOR(Digits) * -(HNUMERIC_local_min(v1, v2)) : EPSILON_FACTOR(Digits) * HNUMERIC_local_max(v1, v2));
        }

    /**----------------------------------------------------------------------------
     STATIC METHOD
     DEBUG METHOD
     EPSILON_FACTOR
     This static method computes the epsilon factor applicable to the specified number
     of digits.

     If the number of digits is 0 then 1 digit is nevertheless used. If the number of
     digits is too high for the DataType in use, an overflow may result.

     The method is provided for debugging purposes only since precision computations
     require a significant amount of time.

     @param Digits A positive integer specifying the number of digits of precision
                   desired.
    -----------------------------------------------------------------------------*/
    static DataType EPSILON_FACTOR(uint32_t Digits)
        {
        DataType Factor = 10;
        for (uint32_t i = 0 ; i < Digits ; ++ i)
            Factor *= 10;

        return(1.0 / Factor);
        }
#endif
    };

/**----------------------------------------------------------------------------
 double TEMPLATE OVERLOAD FOR HNumeric
-----------------------------------------------------------------------------*/
template<>
inline double HNumeric<double>::GLOBAL_EPSILON()
    {
    return(HGLOBAL_EPSILON);
    }

template<>
inline double HNumeric<double>::EPSILON_MULTIPLICATOR()
    {
    return(HEPSILON_MULTIPLICATOR);
    }

template<>
inline double HNumeric<double>::MAX_EPSILON()
    {
    return(HMAX_EPSILON);
    }


END_IMAGEPP_NAMESPACE