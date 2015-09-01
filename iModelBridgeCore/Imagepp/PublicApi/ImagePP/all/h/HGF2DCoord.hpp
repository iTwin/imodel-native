//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DCoord.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HGF2DCoord
//-----------------------------------------------------------------------------

#include "HGF2DDisplacement.h"

BEGIN_IMAGEPP_NAMESPACE

//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
template<class DataType> inline HGF2DCoord<DataType>::HGF2DCoord()
    : m_XValue(0), m_YValue(0)
    {
    }

//-----------------------------------------------------------------------------
// Full Constructor
//-----------------------------------------------------------------------------
template<class DataType> inline HGF2DCoord<DataType>::HGF2DCoord(DataType pi_X,
                                                                 DataType pi_Y)
    : m_XValue(pi_X), m_YValue(pi_Y)
    {
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
template<class DataType> inline HGF2DCoord<DataType>::~HGF2DCoord()
    {
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another position object.
//-----------------------------------------------------------------------------
template<class DataType> inline HGF2DCoord<DataType>& HGF2DCoord<DataType>::operator=(const HGF2DCoord<DataType>& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        m_XValue = pi_rObj.m_XValue;
        m_YValue = pi_rObj.m_YValue;
        }
    return (*this);
    }


/** -----------------------------------------------------------------------------
    Returns the value of the X dimension part coordinate of this coordinate.

    @return The X dimension coordinate for this coordinate.

    Example:
    @code
    HGF2DCoord<double>      MyPoint (10.0, 10.0);

    double X = MyPoint.GetX();
    @end

    @see SetX()
    @see GetY()
    @see operator[]()
    -----------------------------------------------------------------------------
*/
template<class DataType> inline DataType HGF2DCoord<DataType>::GetX() const
    {
    // Return x value
    return (m_XValue);
    }

/** -----------------------------------------------------------------------------
    Returns the value of the Y dimension part coordinate of this coordinate.

    @return The Y dimension coordinate for this coordinate.

    Example:
    @code
    HGF2DCoord<double>      MyPoint (10.0, 10.0);

    double Y = MyPoint.GetY();
    @end

    @see SetY()
    @see GetX()
    @see operator[]()
    -----------------------------------------------------------------------------
*/
template<class DataType> inline DataType HGF2DCoord<DataType>::GetY() const
    {
    // Return Y value
    return (m_YValue);
    }

/** -----------------------------------------------------------------------------
    Sets a new value for the X dimension of this coordinate.

    @param pi_X New X dimension numeric coordinate.

    Example:
    @code
    HGF2DCoord<double>      MyPoint;

    MyPoint.SetX (12.0);
    @end

    @see SetY()
    @see GetX()
    @see operator[]()
    -----------------------------------------------------------------------------
*/
template<class DataType> inline void HGF2DCoord<DataType>::SetX(DataType pi_X)
    {
    // Set value
    m_XValue = pi_X;
    }

/** -----------------------------------------------------------------------------
    Sets a new value for the Y dimension of this coordinate.

    @param pi_Y New Y dimension numeric coordinate.

    Example:
    @code
    HGF2DCoord<double>      MyPoint;

    MyPoint.SetY (12.0);
    @end

    @see GetY()
    @see SetX()
    @see operator[]()
    -----------------------------------------------------------------------------
*/
template<class DataType> inline void HGF2DCoord<DataType>::SetY(DataType pi_Y)
    {
    // Set value
    m_YValue = pi_Y;
    }

/** -----------------------------------------------------------------------------
    Equality operator.  Two coordinate objects are considered "equal" if they
    have the same coordinates.

    @param pi_rObj Constant reference to coordinate object to compare to.

    @return A boolean value that reflects the equality or non-equality
            between operands.

    Example:
    @code
    HGF2DCoord<double> MyFirstPoint (0, 0);
    HGF2DCoord<double> OtherPoint (10.0, 10.0)
    if (ImageOrigin == OtherPoint)
    {
      ...
    }
    @end

    @see operator!=()
    @see operator<()
    @see IsEqualTo()
    -----------------------------------------------------------------------------
*/
template<class DataType> inline bool HGF2DCoord<DataType>::operator==(const HGF2DCoord<DataType>& pi_rObj) const
    {
    return ((pi_rObj.m_XValue == m_XValue) && (pi_rObj.m_YValue == m_YValue));
    }

/** -----------------------------------------------------------------------------
    Inequality operator.  Two coordinate objects are considered "equal" if they
    have the same coordinates.

    @param pi_rObj Constant reference to coordinate object to compare to.

    @return A boolean value that reflects the equality or non-equality
            between operands.

    Example:
    @code
    HGF2DCoord<double> MyFirstPoint (0, 0);
    HGF2DCoord<double> OtherPoint (10.0, 10.0)
    if (ImageOrigin != OtherPoint)
    {
      ...
    }
    @end

    @see operator==()
    @see operator<()
    @see IsEqualTo()
    -----------------------------------------------------------------------------
*/
template<class DataType> inline bool HGF2DCoord<DataType>::operator!=(const HGF2DCoord<DataType>& pi_rObj) const
    {
    return ((pi_rObj.m_XValue != m_XValue) || (pi_rObj.m_YValue != m_YValue));
    }

//-----------------------------------------------------------------------------
// operator<
// smaller than operator.
// This operation is only provided because required on MS Compiler 5.0
// on ALPHA for implementation of an HGF2DCoordCollection (STL)
//-----------------------------------------------------------------------------
template<class DataType> inline bool HGF2DCoord<DataType>::operator<(const HGF2DCoord<DataType>& pi_rObj) const
    {
    return ((pi_rObj.m_XValue < m_XValue) || (pi_rObj.m_YValue < m_YValue));
    }


/** -----------------------------------------------------------------------------
    This method performs equality compare operations upon two coordinate
    with application of a tolerance (epsilon). It applies the global fixed epsilon,

    @param pi_rObj Constant reference to coordinate object to compare to.

    @return A boolean value that reflects the equality or non-equality
            between operands.

    Example:
    @code
    HGF2DCoord<double> MyFirstPoint (0, 0);
    HGF2DCoord<double> OtherPoint (10.0, 10.0)
    if (ImageOrigin.IsEqualTo(OtherPoint))
    {
      ...
    }
    @end

    @see operator==()
    @see operator<()
    -----------------------------------------------------------------------------
*/
template<class DataType> inline bool HGF2DCoord<DataType>::IsEqualTo(const HGF2DCoord<DataType>& pi_rObj) const
    {
    return(HNumeric<DataType>::EQUAL_EPSILON(pi_rObj.m_XValue, m_XValue) &&
           HNumeric<DataType>::EQUAL_EPSILON(pi_rObj.m_YValue, m_YValue));
    }

/** -----------------------------------------------------------------------------
    This method performs equality compare operations upon two coordinate
    with application of a tolerance (epsilon). The method requires the epsilon
    to be given as a parameter.

    @param pi_rObj Constant reference to coordinate object to compare to.

    @param pi_Epsilon Tolerance to apply to compare operation. This number must
                      be greater or equal to 0.0.

    @return A boolean value that reflects the equality or non-equality
            between operands.

    Example:
    @code
    HGF2DCoord<double> MyFirstPoint (0, 0);
    HGF2DCoord<double> OtherPoint (10.0, 10.0)
    if (ImageOrigin.IsEqualTo(OtherPoint, 0.000001))
    {
      ...
    }
    @end

    @see operator==()
    @see operator<()
    -----------------------------------------------------------------------------
*/
template<class DataType> inline bool HGF2DCoord<DataType>::IsEqualTo(const HGF2DCoord<DataType>& pi_rObj,
                                                                      DataType pi_Epsilon) const
    {
    return(HNumeric<DataType>::EQUAL(pi_rObj.m_XValue, m_XValue, pi_Epsilon) &&
           HNumeric<DataType>::EQUAL(pi_rObj.m_YValue, m_YValue, pi_Epsilon));
    }

/** -----------------------------------------------------------------------------
    This method performs equality compare operations upon two coordinate
    with application of a tolerance (epsilon).
    The method determines automatically the most appropriate epsilon based
    on the current value of self. In this last method, the epsilon
    applied to the X and Y coordinates is different, and based on the
    individual values of each dimensions.

    @param pi_rObj Constant reference to coordinate object to compare to.

    @return A boolean value that reflects the equality or non-equality
            between operands.

    Example:
    @code
    HGF2DCoord<double> MyFirstPoint (0, 0);
    HGF2DCoord<double> OtherPoint (10.0, 10.0)
    if (ImageOrigin.IsEqualToAutoEpsilon(OtherPoint))
    {
      ...
    }
    @end

    @see operator==()
    @see operator<()
    @see IsEqualTo()
    -----------------------------------------------------------------------------
*/
template<class DataType> inline bool HGF2DCoord<DataType>::IsEqualToAutoEpsilon(const HGF2DCoord<DataType>& pi_rObj) const
    {
    return(HNumeric<DataType>::EQUAL(pi_rObj.m_XValue, m_XValue, HNumeric<DataType>::AUTO_EPSILON(m_XValue)) &&
           HNumeric<DataType>::EQUAL(pi_rObj.m_YValue, m_YValue, HNumeric<DataType>::AUTO_EPSILON(m_YValue)));
    }


/** -----------------------------------------------------------------------------
    Dereference operator. It returns a reference to the specified coordinate.
    This value can be changed or consulted.

    @param pi_CoordinateRef An identificator that specifies the coordinate
                            X or Y that is to be returned. This
                            represents an integer value that can be 0 to make
                            reference to the X coordinate, and 1 to make reference
                            to the Y coordinate.

    @return A reference to the internal coordinate that can be modified.

    Example:
    @code
    HGF2DCoord<double>      ImageOrigin (10, 10);
    double NewCoord = ImageOrigin[0] + 12;
    ImageOrigin[1]+=23.1;
    @end

    @see GetX()
    @see GetY()
    @see SetX()
    @see SetY()
    -----------------------------------------------------------------------------
*/
template<class DataType> inline DataType& HGF2DCoord<DataType>::operator[](int pi_CoordinateRef)
    {
    HPRECONDITION((pi_CoordinateRef == 0) || (pi_CoordinateRef == 1));

    return ((pi_CoordinateRef == 0) ? m_XValue : m_YValue);
    }



/** -----------------------------------------------------------------------------
    Dereference operator. It returns a reference to the specified coordinate.
    This value cannot be modified since the object is const.

    @param pi_CoordinateRef An identificator that specifies the coordinate X
                            or Y that is to be returned. This represents
                            an integer value that can be equal to
                            0 to make reference to the X coordinate,
                            and 1 to make reference to the Y coordinate.

    @return A reference to the internal coordinate that cannot be modified.

    Example:
    @code
    HGF2DCoord<double>      ImageOrigin (10, 10);
    double NewCoord = ImageOrigin[0] + 12;
    @end

    @see GetX()
    @see GetY()
    -----------------------------------------------------------------------------
*/
template<class DataType> inline const DataType& HGF2DCoord<DataType>::operator[](int pi_CoordinateRef) const
    {
    HPRECONDITION((pi_CoordinateRef == 0) || (pi_CoordinateRef == 1));

    return ((pi_CoordinateRef == 0) ? m_XValue : m_YValue);
    }



//-----------------------------------------------------------------------------
// Calculates length to other point
//-----------------------------------------------------------------------------
template<class DataType> DataType HGF2DCoord<DataType>::CalculateLengthTo(const HGF2DCoord<DataType>& pi_rPoint) const
    {
    // Compute in meters squared the intermediate value
    HGF2DDisplacement newDisp(m_XValue - pi_rPoint.m_XValue, m_YValue - pi_rPoint.m_YValue);
    return newDisp.CalculateLength();
    }

//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
template<class DataType> void HGF2DCoord<DataType>::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    char    DumString[256];
    sprintf(DumString, "Object is a HGF2DCoord<?> undefined print state for this data type");
    HDUMP0(DumString);
    HDUMP0("\n");
    po_rOutput << DumString << endl;
#endif
    }

//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
template<> inline void HGF2DCoord<char>::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    char    DumString[256];
    sprintf(DumString, "Object is a HGF2DCoord<char>: X = %c , Y = %c", m_XValue, m_YValue);
    HDUMP0(DumString);
    HDUMP0("\n");
    po_rOutput << DumString << endl;
#endif
    }

//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
template<> inline void HGF2DCoord<signed char>::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    char    DumString[256];
    sprintf(DumString, "Object is a HGF2DCoord<signed char>: X = %c , Y = %c", m_XValue, m_YValue);
    HDUMP0(DumString);
    HDUMP0("\n");
    po_rOutput << DumString << endl;
#endif
    }


//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
template<> inline void HGF2DCoord<unsigned char>::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    char    DumString[256];
    sprintf(DumString, "Object is a HGF2DCoord<unsigned char>: X = %c , Y = %c", m_XValue, m_YValue);
    HDUMP0(DumString);
    HDUMP0("\n");
    po_rOutput << DumString << endl;
#endif
    }





//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
template<> inline void HGF2DCoord<short>::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    char    DumString[256];
    sprintf(DumString, "Object is a HGF2DCoord<short> X = %5hd , Y = %5hd", m_XValue, m_YValue);
    HDUMP0(DumString);
    HDUMP0("\n");
    po_rOutput << DumString << endl;
#endif
    }

//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
template<> inline void HGF2DCoord<unsigned short>::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    char    DumString[256];
    sprintf(DumString, "Object is a HGF2DCoord<unsigned short> X = %5hu , Y = %5hu", m_XValue, m_YValue);
    HDUMP0(DumString);
    HDUMP0("\n");
    po_rOutput << DumString << endl;
#endif
    }




//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
template<> inline void HGF2DCoord<double>::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    char    DumString[256];
    sprintf(DumString, "Object is a HGF2DCoord<double> X = %5.15lf , Y = %5.15lf", m_XValue, m_YValue);
    HDUMP0(DumString);
    HDUMP0("\n");
    po_rOutput << DumString << endl;
#endif
    }

//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
template<> inline void HGF2DCoord<int32_t>::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    char    DumString[256];
    sprintf(DumString, "Object is a HGF2DCoord<int32_t> X = %15ld , Y = %15ld", m_XValue, m_YValue);
    HDUMP0(DumString);
    HDUMP0("\n");
    po_rOutput << DumString << endl;
#endif
    }

//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
template<> inline void HGF2DCoord<uint32_t>::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    char    DumString[256];
    sprintf(DumString, "Object is a HGF2DCoord<uint32_t> X = %15lu , Y = %15lu", m_XValue, m_YValue);
    HDUMP0(DumString);
    HDUMP0("\n");
    po_rOutput << DumString << endl;
#endif
    }

//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
template<> inline void HGF2DCoord<float>::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    char    DumString[256];
    sprintf(DumString, "Object is a HGF2DCoord<float> X = %5.15f , Y = %5.15f", m_XValue, m_YValue);
    HDUMP0(DumString);
    HDUMP0("\n");
    po_rOutput << DumString << endl;
#endif
    }


END_IMAGEPP_NAMESPACE




