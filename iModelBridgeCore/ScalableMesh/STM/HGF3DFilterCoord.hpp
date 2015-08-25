//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/HGF3DFilterCoord.hpp $
//:>    $RCSfile: HGF3DFilterCoord.hpp,v $
//:>   $Revision: 1.2 $
//:>       $Date: 2010/05/18 17:25:48 $
//:>     $Author: Mathieu.St-Pierre $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Inline methods for class HGF3DCoord
//-----------------------------------------------------------------------------

#include "HNumeric.h"

//-----------------------------------------------------------------------------
// Default Constructor 
//-----------------------------------------------------------------------------
template<class DataType> inline HGF3DCoord<DataType>::HGF3DCoord()
: m_XValue(0), m_YValue(0), m_ZValue(0)
{
}

//-----------------------------------------------------------------------------
// Full Constructor 
//-----------------------------------------------------------------------------
template<class DataType> inline HGF3DCoord<DataType>::HGF3DCoord(DataType pi_X,
                                                                 DataType pi_Y,
                                                                 DataType pi_Z)
: m_XValue(pi_X), m_YValue(pi_Y), m_ZValue(pi_Z)
{
}

#if (0)
//-----------------------------------------------------------------------------
// Full Constructor from HGF2DCoord
//-----------------------------------------------------------------------------
template<class DataType> inline HGF3DCoord<DataType>::HGF3DCoord(const HGF2DCoord<DataType> pi_2DPoint,
                                                                 DataType pi_Elevation)
: m_XValue(pi_2DPoint.GetX()), m_YValue(pi_2DPoint.GetY()), m_ZValue(pi_Elevation)
{
}

#endif



//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
template<class DataType> inline HGF3DCoord<DataType>::~HGF3DCoord()
{
}

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another position object.
//-----------------------------------------------------------------------------
template<class DataType> inline HGF3DCoord<DataType>& HGF3DCoord<DataType>::operator=(const HGF3DCoord<DataType>& pi_rObj)
{
    if (this != &pi_rObj) 
    {
        m_XValue = pi_rObj.m_XValue;
        m_YValue = pi_rObj.m_YValue;
        m_ZValue = pi_rObj.m_ZValue;
    }
    return (*this);
}


/** -----------------------------------------------------------------------------
    Returns the value of the X dimension part coordinate of this coordinate.
    
    @return The X dimension coordinate for this coordinate.

    Example:
    @code
    HGF3DCoord<double>      MyPoint (10.0, 10.0);

    double X = MyPoint.GetX();
    @end
                                   
    @see SetX()
    @see GetY()
    @see GetZ()
    @see operator[]()
    -----------------------------------------------------------------------------
*/
template<class DataType> inline DataType HGF3DCoord<DataType>::GetX() const
{
    // Return x value
    return (m_XValue);
}

/** -----------------------------------------------------------------------------
    Returns the value of the Y dimension part coordinate of this coordinate.
    
    @return The Y dimension coordinate for this coordinate.

    Example:
    @code
    HGF3DCoord<double>      MyPoint (10.0, 10.0);

    double Y = MyPoint.GetY();
    @end
                                   
    @see SetY()
    @see GetX()
    @see GetZ()
    @see operator[]()
    -----------------------------------------------------------------------------
*/
template<class DataType> inline DataType HGF3DCoord<DataType>::GetY() const
{
    // Return Y value
    return (m_YValue);
}

/** -----------------------------------------------------------------------------
    Returns the value of the Z dimension part coordinate of this coordinate.
    
    @return The Z dimension coordinate for this coordinate.

    Example:
    @code
    HGF3DCoord<double>      MyPoint (10.0, 10.0, 23.0);

    double Z = MyPoint.GetZ();
    @end
                                   
    @see SetZ()
    @see GetX()
    @see GetY()
    @see operator[]()
    -----------------------------------------------------------------------------
*/
template<class DataType> inline DataType HGF3DCoord<DataType>::GetZ() const
{
    // Return Z value
    return (m_ZValue);
}


/** -----------------------------------------------------------------------------
    Sets a new value for the X dimension of this coordinate.    
    
    @param pi_X New X dimension numeric coordinate.

    Example:
    @code
    HGF3DCoord<double>      MyPoint;

    MyPoint.SetX (12.0);
    @end
                                   
    @see SetY()
    @see SetZ()
    @see GetX()
    @see operator[]()
    -----------------------------------------------------------------------------
*/
template<class DataType> inline void HGF3DCoord<DataType>::SetX(DataType pi_X)
{
    // Set value
    m_XValue = pi_X;
}

/** -----------------------------------------------------------------------------
    Sets a new value for the Y dimension of this coordinate.    
    
    @param pi_Y New Y dimension numeric coordinate.

    Example:
    @code
    HGF3DCoord<double>      MyPoint;

    MyPoint.SetY (12.0);
    @end
                                   
    @see GetY()
    @see SetX()
    @see SetZ()
    @see operator[]()
    -----------------------------------------------------------------------------
*/
template<class DataType> inline void HGF3DCoord<DataType>::SetY(DataType pi_Y)
{
    // Set value 
    m_YValue = pi_Y;
}

/** -----------------------------------------------------------------------------
    Sets a new value for the Z dimension of this coordinate.    
    
    @param pi_Z New Z dimension numeric coordinate.

    Example:
    @code
    HGF3DCoord<double>      MyPoint;

    MyPoint.SetZ (12.0);
    @end
                                   
    @see GetZ()
    @see SetX()
    @see SetY()
    @see operator[]()
    -----------------------------------------------------------------------------
*/
template<class DataType> inline void HGF3DCoord<DataType>::SetZ(DataType pi_Z)
{
    // Set value 
    m_ZValue = pi_Z;
}



/** -----------------------------------------------------------------------------
    Equality operator.  Two coordinate objects are considered "equal" if they 
    have the same coordinates. 
    
    @param pi_rObj Constant reference to coordinate object to compare to.
    
    @return A boolean value that reflects the equality or non-equality 
            between operands.

    Example:
    @code
    HGF3DCoord<double> MyFirstPoint (0, 0);
    HGF3DCoord<double> OtherPoint (10.0, 10.0)
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
template<class DataType> inline int HGF3DCoord<DataType>::operator==(const HGF3DCoord<DataType>& pi_rObj) const
{
    return ((pi_rObj.m_XValue == m_XValue) && (pi_rObj.m_YValue == m_YValue) && (pi_rObj.m_ZValue == m_ZValue));
}

/** -----------------------------------------------------------------------------
    Inequality operator.  Two coordinate objects are considered "equal" if they 
    have the same coordinates. 
    
    @param pi_rObj Constant reference to coordinate object to compare to.
    
    @return A boolean value that reflects the equality or non-equality 
            between operands.

    Example:
    @code
    HGF3DCoord<double> MyFirstPoint (0, 0);
    HGF3DCoord<double> OtherPoint (10.0, 10.0)
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
template<class DataType> inline int HGF3DCoord<DataType>::operator!=(const HGF3DCoord<DataType>& pi_rObj) const
{
    return ((pi_rObj.m_XValue != m_XValue) || (pi_rObj.m_YValue != m_YValue) || (pi_rObj.m_ZValue != m_ZValue));
}

//-----------------------------------------------------------------------------
// operator<
// smaller than operator. 
// This operation is only provided because required on MS Compiler 5.0
// on ALPHA for implementation of an HGF3DCoordCollection (STL)
//-----------------------------------------------------------------------------
template<class DataType> inline int HGF3DCoord<DataType>::operator<(const HGF3DCoord<DataType>& pi_rObj) const
{
    return ((pi_rObj.m_XValue < m_XValue) || (pi_rObj.m_YValue < m_YValue) || (pi_rObj.m_ZValue < m_ZValue));
}


/** -----------------------------------------------------------------------------
    This method performs equality compare operations upon two coordinate 
    with application of a tolerance (epsilon). It applies the global fixed epsilon, 
    
    @param pi_rObj Constant reference to coordinate object to compare to.
    
    @return A boolean value that reflects the equality or non-equality 
            between operands.

    Example:
    @code
    HGF3DCoord<double> MyFirstPoint (0, 0);
    HGF3DCoord<double> OtherPoint (10.0, 10.0)
    if (ImageOrigin.IsEqualTo(OtherPoint))
    {
      ...
    }
    @end
                                   
    @see operator==()
    @see operator<()
    -----------------------------------------------------------------------------
*/
template<class DataType> inline bool HGF3DCoord<DataType>::IsEqualTo(const HGF3DCoord<DataType>& pi_rObj) const
{
    return(HNumeric<DataType>::EQUAL_EPSILON(pi_rObj.m_XValue, m_XValue) && 
           HNumeric<DataType>::EQUAL_EPSILON(pi_rObj.m_YValue, m_YValue) &&
           HNumeric<DataType>::EQUAL_EPSILON(pi_rObj.m_ZValue, m_ZValue));
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
    HGF3DCoord<double> MyFirstPoint (0, 0);
    HGF3DCoord<double> OtherPoint (10.0, 10.0)
    if (ImageOrigin.IsEqualTo(OtherPoint, 0.000001))
    {
      ...
    }
    @end
                                   
    @see operator==()
    @see operator<()
    -----------------------------------------------------------------------------
*/
template<class DataType> inline bool HGF3DCoord<DataType>::IsEqualTo(const HGF3DCoord<DataType>& pi_rObj,
                                    DataType pi_Epsilon) const
{
    return(HNumeric<DataType>::EQUAL(pi_rObj.m_XValue, m_XValue, pi_Epsilon) && 
           HNumeric<DataType>::EQUAL(pi_rObj.m_YValue, m_YValue, pi_Epsilon) &&
           HNumeric<DataType>::EQUAL(pi_rObj.m_ZValue, m_ZValue, pi_Epsilon));
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
    HGF3DCoord<double> MyFirstPoint (0, 0);
    HGF3DCoord<double> OtherPoint (10.0, 10.0)
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
template<class DataType> inline bool HGF3DCoord<DataType>::IsEqualToAutoEpsilon(const HGF3DCoord<DataType>& pi_rObj) const
{
    return(HNumeric<DataType>::EQUAL(pi_rObj.m_XValue, m_XValue, HNumeric<DataType>::AUTO_EPSILON(m_XValue)) && 
           HNumeric<DataType>::EQUAL(pi_rObj.m_YValue, m_YValue, HNumeric<DataType>::AUTO_EPSILON(m_YValue)) &&
           HNumeric<DataType>::EQUAL(pi_rObj.m_ZValue, m_ZValue, HNumeric<DataType>::AUTO_EPSILON(m_ZValue)));
}


/** -----------------------------------------------------------------------------
    This method performs equality compare operations upon two coordinate in 2D
    with application of a tolerance (epsilon). It applies the global fixed epsilon, 
    Only the X and Y components are compared
    
    @param pi_rObj Constant reference to coordinate object to compare to.
    
    @return A boolean value that reflects the equality or non-equality 
            between operands.

    Example:
    @code
    HGF3DCoord<double> MyFirstPoint (0, 0);
    HGF3DCoord<double> OtherPoint (10.0, 10.0)
    if (ImageOrigin.IsEqualTo2D(OtherPoint))
    {
      ...
    }
    @end
                                   
    @see operator==()
    @see operator<()
    -----------------------------------------------------------------------------
*/
template<class DataType> inline bool HGF3DCoord<DataType>::IsEqualTo2D(const HGF3DCoord<DataType>& pi_rObj) const
{
    return(HNumeric<DataType>::EQUAL_EPSILON(pi_rObj.m_XValue, m_XValue) && 
           HNumeric<DataType>::EQUAL_EPSILON(pi_rObj.m_YValue, m_YValue));
}

/** -----------------------------------------------------------------------------
    This method performs equality compare operations upon two coordinate in 2D
    with application of a tolerance (epsilon). The method requires the epsilon 
    to be given as a parameter. 
    Only the X and Y components are compared
    
    @param pi_rObj Constant reference to coordinate object to compare to.
    
    @param pi_Epsilon Tolerance to apply to compare operation. This number must 
                      be greater or equal to 0.0.
    
    @return A boolean value that reflects the equality or non-equality 
            between operands.

    Example:
    @code
    HGF3DCoord<double> MyFirstPoint (0, 0);
    HGF3DCoord<double> OtherPoint (10.0, 10.0)
    if (ImageOrigin.IsEqualTo2D(OtherPoint, 0.000001))
    {
      ...
    }
    @end
                                   
    @see operator==()
    @see operator<()
    -----------------------------------------------------------------------------
*/
template<class DataType> inline bool HGF3DCoord<DataType>::IsEqualTo2D(const HGF3DCoord<DataType>& pi_rObj,
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
    Only the X and Y components are compared
    
    @param pi_rObj Constant reference to coordinate object to compare to.
    
    @return A boolean value that reflects the equality or non-equality 
            between operands.

    Example:
    @code
    HGF3DCoord<double> MyFirstPoint (0, 0);
    HGF3DCoord<double> OtherPoint (10.0, 10.0)
    if (ImageOrigin.IsEqualTo2DAutoEpsilon(OtherPoint))
    {
      ...
    }
    @end
                                   
    @see operator==()
    @see operator<()
    @see IsEqualTo()
    -----------------------------------------------------------------------------
*/
template<class DataType> inline bool HGF3DCoord<DataType>::IsEqualTo2DAutoEpsilon(const HGF3DCoord<DataType>& pi_rObj) const
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
    HGF3DCoord<double>      ImageOrigin (10, 10);
    double NewCoord = ImageOrigin[0] + 12;
    ImageOrigin[1]+=23.1;
    @end
                                   
    @see GetX()
    @see GetY()
    @see GetZ()
    @see SetX()
    @see SetY()
    @see SetZ()
    -----------------------------------------------------------------------------
*/
template<class DataType> inline DataType& HGF3DCoord<DataType>::operator[](int pi_CoordinateRef)
{
    HPRECONDITION((pi_CoordinateRef == 0) || (pi_CoordinateRef == 1) || (pi_CoordinateRef == 2));

    return ((pi_CoordinateRef == 0) ? m_XValue : ((pi_CoordinateRef == 1) ? m_YValue : m_ZValue));
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
    HGF3DCoord<double>      ImageOrigin (10, 10);
    double NewCoord = ImageOrigin[0] + 12;
    @end
                                   
    @see GetX()
    @see GetY()
    @see GetZ()
    -----------------------------------------------------------------------------
*/
template<class DataType> inline const DataType& HGF3DCoord<DataType>::operator[](int pi_CoordinateRef) const
{
    HPRECONDITION((pi_CoordinateRef == 0) || (pi_CoordinateRef == 1) || (pi_CoordinateRef == 2));

    return ((pi_CoordinateRef == 0) ? m_XValue : ((pi_CoordinateRef == 1) ? m_YValue : m_ZValue));
}





/** -----------------------------------------------------------------------------
    Transform
    Transforms the point through the provided matrix
    -----------------------------------------------------------------------------
*/
template<class DataType> inline void HGF3DCoord<DataType>::Transform(const HFCMatrix<4, 4>& pi_rTransfoMatrix)
{

    DataType NewX = m_XValue * pi_rTransfoMatrix[0][0] + 
                    m_YValue * pi_rTransfoMatrix[0][1] + 
                    m_ZValue * pi_rTransfoMatrix[0][2] + 
                    pi_rTransfoMatrix[0][3];
    DataType NewY = m_XValue * pi_rTransfoMatrix[1][0] + 
                    m_YValue * pi_rTransfoMatrix[1][1] + 
                    m_ZValue * pi_rTransfoMatrix[1][2] + 
                    pi_rTransfoMatrix[1][3];

    DataType NewZ = m_XValue * pi_rTransfoMatrix[2][0] + 
                    m_YValue * pi_rTransfoMatrix[2][1] + 
                    m_ZValue * pi_rTransfoMatrix[2][2] + 
                    pi_rTransfoMatrix[2][3];

    DataType Norm = m_XValue * pi_rTransfoMatrix[3][0] + 
                    m_YValue * pi_rTransfoMatrix[3][1] + 
                    m_ZValue * pi_rTransfoMatrix[3][2] + 
                    pi_rTransfoMatrix[3][3];

    HASSERT(Norm != 0);

    m_XValue = NewX / Norm;
    m_YValue = NewY / Norm;
    m_ZValue = NewZ / Norm;

}



/** -----------------------------------------------------------------------------
    Calculate2DLength
    Returns the 2D length of the vector formed from origin to location
    -----------------------------------------------------------------------------
*/
template<class DataType> inline DataType HGF3DCoord<DataType>::Calculate2DLength() const
{
    return(sqrt((m_XValue * m_XValue) + (m_YValue * m_YValue)));
}

/** -----------------------------------------------------------------------------
    Calculate3DLength
    Returns the 3D length of the vector formed from origin to location
    -----------------------------------------------------------------------------
*/
template<class DataType> inline DataType HGF3DCoord<DataType>::Calculate3DLength() const
{
    return(sqrt((m_XValue * m_XValue) + (m_YValue * m_YValue) + (m_ZValue * m_ZValue)));
}

/** -----------------------------------------------------------------------------
    operator-
    Returns a coordinate representing the displacement vector from self point
    to given point
    -----------------------------------------------------------------------------
*/
template<class DataType> inline HGF3DCoord<DataType> HGF3DCoord<DataType>::operator-(const HGF3DCoord<DataType>& i_rPoint) const
{
    return(HGF3DCoord<DataType>(m_XValue - i_rPoint.m_XValue,
                                m_YValue - i_rPoint.m_YValue,
                                m_ZValue - i_rPoint.m_ZValue));
}





/** -----------------------------------------------------------------------------
    Transform
    Transforms the point through the provided model
    -----------------------------------------------------------------------------
*/
template<class DataType> inline void HGF3DCoord<DataType>::Transform(const HGF3DTransfoModel& pi_rTransfoModel)
{

    pi_rTransfoModel.ConvertDirect(&m_XValue, &m_YValue, &m_ZValue);
}


//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
template<class DataType> void HGF3DCoord<DataType>::PrintState(ostream& po_rOutput) const
{
#ifdef __HMR_PRINTSTATE
    char    DumString[256];
    sprintf(DumString, "Object is a HGF3DCoord<?> undefined print state for this data type");
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
inline void HGF3DCoord<char>::PrintState(ostream& po_rOutput) const
{
#ifdef __HMR_PRINTSTATE
    char    DumString[256];
    sprintf(DumString, "Object is a HGF3DCoord<char>: X = %c , Y = %c , Z = %c", m_XValue, m_YValue, m_ZValue);
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
inline void HGF3DCoord<signed char>::PrintState(ostream& po_rOutput) const
{
#ifdef __HMR_PRINTSTATE
    char    DumString[256];
    sprintf(DumString, "Object is a HGF3DCoord<signed char>: X = %c , Y = %c , Z = %c", m_XValue, m_YValue, m_ZValue);
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
inline void HGF3DCoord<unsigned char>::PrintState(ostream& po_rOutput) const
{
#ifdef __HMR_PRINTSTATE
    char    DumString[256];
    sprintf(DumString, "Object is a HGF3DCoord<unsigned char>: X = %c , Y = %c , Z = %c", m_XValue, m_YValue, m_ZValue);
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
inline void HGF3DCoord<Short>::PrintState(ostream& po_rOutput) const
{
#ifdef __HMR_PRINTSTATE
    char    DumString[256];
    sprintf(DumString, "Object is a HGF3DCoord<Short> X = %5hd , Y = %5hd , Z = %5hd", m_XValue, m_YValue, m_ZValue);
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
inline void HGF3DCoord<UShort>::PrintState(ostream& po_rOutput) const
{
#ifdef __HMR_PRINTSTATE
    char    DumString[256];
    sprintf(DumString, "Object is a HGF3DCoord<UShort> X = %5hu , Y = %5hu , Z = %5hu", m_XValue, m_YValue, m_ZValue);
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
inline void HGF3DCoord<double>::PrintState(ostream& po_rOutput) const
{
#ifdef __HMR_PRINTSTATE
    char    DumString[256];
    sprintf(DumString, "Object is a HGF3DCoord<double> X = %5.15lf , Y = %5.15lf , Z = %5.15lf", m_XValue, m_YValue, m_ZValue);
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
inline void HGF3DCoord<Long32>::PrintState(ostream& po_rOutput) const
{
#ifdef __HMR_PRINTSTATE
    char    DumString[256];
    sprintf(DumString, "Object is a HGF3DCoord<Long32> X = %15ld , Y = %15ld , Z = %15ld", m_XValue, m_YValue, m_ZValue);
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
inline void HGF3DCoord<ULong32>::PrintState(ostream& po_rOutput) const
{
#ifdef __HMR_PRINTSTATE
    char    DumString[256];
    sprintf(DumString, "Object is a HGF3DCoord<ULong32> X = %15lu , Y = %15lu , Z = %15lu", m_XValue, m_YValue, m_ZValue);
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
inline void HGF3DCoord<float>::PrintState(ostream& po_rOutput) const
{
#ifdef __HMR_PRINTSTATE
    char    DumString[256];
    sprintf(DumString, "Object is a HGF3DCoord<float> X = %5.15f , Y = %5.15f , Z = %5.15f", m_XValue, m_YValue, m_ZValue);
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
inline void HGF3DCoord<int32_t>::PrintState(ostream& po_rOutput) const
{
#ifdef __HMR_PRINTSTATE
    char    DumString[256];
    sprintf(DumString, "Object is a HGF3DCoord<int32_t> X = %15d , Y = %15d , Z = %15d", m_XValue, m_YValue, m_ZValue);
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
inline void HGF3DCoord<uint32_t>::PrintState(ostream& po_rOutput) const
{
#ifdef __HMR_PRINTSTATE
    char    DumString[256];
    sprintf(DumString, "Object is a HGF3DCoord<uint32_t> X = %15u , Y = %15u , Z = %15u", m_XValue, m_YValue, m_ZValue);
    HDUMP0(DumString);
    HDUMP0("\n");
    po_rOutput << DumString << endl;
#endif
}






