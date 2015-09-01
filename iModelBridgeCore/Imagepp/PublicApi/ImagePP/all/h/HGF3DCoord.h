//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF3DCoord.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF3DCoord
//-----------------------------------------------------------------------------
// Position in two-dimension
//-----------------------------------------------------------------------------

#pragma once

#include "HGF2DCoord.h"
#include "HFCMatrix.h"
#include "HGF3DTransfoModel.h"

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 

    This class implements the nature and behavior for raw coordinates. Such coordinates
    are expressed using 3 numbers indicating values for three dimensions arbitrarily
    named X, Y and Z. The three numbers can be of any valid numeric data type (DataType).
    Conversion between coordinates using one data type to coordinates using another
    is possible as long as a conversion exists in the form of a copy contructor (DataType(OtherDataType)).
    This conversion method is implemented in the form of a templated constructor. Implicit
    cast from one type of coordinate to another is therefore possible.

    This class is used to describe position in three-dimensional coordinate
    systems.  It plays the role of an atomic "concrete data type" having no
    parent and not specifically designed to have child versions.
    Coordinate values labeled arbitrarily X, Y and Z describe the
    position.
    All three X, Y and Z values are kept in the form of a numeric value of the template
    arguments data type.
    The HGF3DCoord interface provides methods to access and change
    directly the values of the X, Y and Z dimensions.

    Note the existence of the following definition in addition to the HGF3DCoord<T> class:
    typedef vector<HGF3DCoord<T>, allocator<HGF3DCoord<T>> > HGF3DCoordCollection<T>;

    This defines a collection of coordinates, and answers in all
    points to the vector<> Standard Template Library interface.

    -----------------------------------------------------------------------------
*/
template<class DataType = double> class HGF3DCoord
    {
public:

    // This enum defined coordinate X, Y and Z ids
    // to be used to extract values
    enum CoordRef
        {
        X = 0,
        Y = 1,
        Z = 2
        };

    // Primary methods
    /** -----------------------------------------------------------------------------
        Constructors and copy constructor for this class.  They allow three
        ways to construct a coordinate object: as an empty position (all
        coordinates at zero), as a new coord with specified coordinates,
        or as a duplicate of another coordinate object.

        @param pi_X X-axis coordinate of the position.

        @param pi_Y Y-axis coordinate of the position.

        @param pi_Z Z-axis coordinate of the position.

        @param pi_rObject Constant reference to a HGF3DCoord to duplicate.

        Example:
        @code
        HGF3DCoord<double>         ImageOrigin (10.0, 10.0, 0.0);
            HGF3DCoord<double>             ImageWorld(ImageOrigin);
            HGF3DCoord<double>     APoint;
        @end

        -----------------------------------------------------------------------------
    */
    HGF3DCoord();
    HGF3DCoord(DataType pi_X,
               DataType pi_Y,
               DataType pi_Z = 0.0);
//                                    DataType pi_Z = HNumeric<DataType>::ZERO());

    // MICROSOFT BUG ... DEFINITION MUST BE INSIDE DECLARATION
    template<class OtherDataType>
    HGF3DCoord(const HGF3DCoord<OtherDataType>& pi_rObj)
        : m_XValue(pi_rObj.GetX()), m_YValue(pi_rObj.GetY()), m_ZValue(pi_rObj.GetZ())
        {
        }

    template<class OtherDataType>
    HGF3DCoord(const HGF2DCoord<OtherDataType>& pi_rObj,
               OtherDataType pi_Elevation = 0.0)
        : m_XValue(pi_rObj.GetX()), m_YValue(pi_rObj.GetY()), m_ZValue(pi_Elevation)
        {
        }




    ~HGF3DCoord();
    HGF3DCoord<DataType>&     operator=(const HGF3DCoord<DataType>& pi_rObj);

    // Compare operations
    bool                operator==(const HGF3DCoord<DataType>& pi_rObj) const;
    bool                operator!=(const HGF3DCoord<DataType>& pi_rObj) const;

    // MICROSOFT BUG ... REQUIRED FOR ALPHA VC5.0 to create STL lists of coordinates
    bool                operator<(const HGF3DCoord<DataType>& pi_rObj) const;


    // Compare operations with epsilon
    bool              IsEqualTo(const HGF3DCoord<DataType>& pi_rObj) const;
    bool              IsEqualTo(const HGF3DCoord<DataType>& pi_rObj, DataType pi_Epsilon) const;
    bool              IsEqualToAutoEpsilon(const HGF3DCoord<DataType>& pi_rObj) const;

    bool               IsEqualTo2D(const HGF3DCoord<DataType>& pi_rObj) const;
    bool               IsEqualTo2D(const HGF3DCoord<DataType>& pi_rObj, DataType pi_Epsilon) const;
    bool               IsEqualTo2DAutoEpsilon(const HGF3DCoord<DataType>& pi_rObj) const;


    // Location management
    DataType           GetX() const;
    DataType           GetY() const;
    DataType           GetZ() const;
    void               SetX(DataType pi_X);
    void               SetY(DataType pi_Y);
    void               SetZ(DataType pi_Y);
    DataType&          operator[](int pi_CoordinateRef);

    // MICROSOFT BUG ... DEFINITION MUST BE INSIDE DECLARATION
    /** -----------------------------------------------------------------------------
        Dereference operator. It returns a reference to the specified coordinate.
        This value can be changed or consulted.

        @param pi_CoordinateRef An identificator that specifies the coordinate X
                                Y or Z that is to be returned. This represents
                                an integer value that can be equal to
                                HGF3DCoord::X  to make reference to
                                the X coordinate, HGF3DCoord::Y
                                to make reference to the Y coordinate and
                                HGF3DCoord::Z to make reference to the Z coordinate

        @return A reference to the internal coordinate that can be modified.

        Example:
        @code
        HGF3DCoord<double>      ImageOrigin (10, 10);
            double NewCoord = ImageOrigin[HGF3DCoord<double>::X] + 12;
            ImageOrigin[HGF3DCoord<double>::Y]+=23.1;
        @end

        @see GetX()
        @see GetY()
        @see SetX()
        @see SetY()
        -----------------------------------------------------------------------------
    */
    DataType&          operator[](CoordRef pi_CoordinateRef)
        {
        return ((pi_CoordinateRef == HGF3DCoord<DataType>::X) ? m_XValue :
                (pi_CoordinateRef == HGF3DCoord<DataType>::Y) ? m_YValue : m_ZValue);
        }

    const DataType&    operator[](int pi_CoordinateRef) const;

    // MICROSOFT BUG ... DEFINITION MUST BE INSIDE DECLARATION
    /** -----------------------------------------------------------------------------
        Dereference operator. It returns a reference to the specified coordinate.
        This value cannot be modified since the object is const.

        @param pi_CoordinateRef An identificator that specifies the coordinate X
                                or Y that is to be returned. This represents
                                an integer value that can be equal to
                                HGF3DCoord::X  to make reference to
                                the X coordinate, HGF3DCoord::Y
                                to make reference to the Y coordinate and
                                HGF3DCoord::Z to make reference to the Z coordinate


        @return A reference to the internal coordinate that cannot be modified.

        Example:
        @code
        HGF3DCoord<double>      ImageOrigin (10, 10);
            double NewCoord = ImageOrigin[HGF3DCoord<double>::X] + 12;
        @end

        @see GetX()
        @see GetY()
        @see GetZ()
        -----------------------------------------------------------------------------
    */
    const DataType&    operator[](CoordRef pi_CoordinateRef) const
        {
        return ((pi_CoordinateRef == HGF3DCoord<DataType>::X) ? m_XValue :
                (pi_CoordinateRef == HGF3DCoord<DataType>::Y) ? m_YValue : m_ZValue);
        }


    void Transform(const HFCMatrix<4, 4>& pi_rTransfoMatrix);
    void Transform(const HGF3DTransfoModel& pi_rTransfo);

    DataType Calculate2DLength() const;
    DataType Calculate3DLength() const;

    HGF3DCoord<DataType> operator-(const HGF3DCoord<DataType>& i_rPoint) const;


    void       PrintState(ostream& po_rOutput) const;

protected:


private:
public :
    // Coordinates of this position
    DataType         m_XValue;
    DataType         m_YValue;
    DataType         m_ZValue;
    };



/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 

    This template class implements the nature and behavior for a list of raw coordinates.
    The list inherits from the vector STL class and thus provides all the vector class interface
    -----------------------------------------------------------------------------
*/
template<class DataType> class HGF3DCoordCollection : public vector<HGF3DCoord<DataType>, allocator<HGF3DCoord<DataType> > >
    {
public:
    HGF3DCoordCollection() {};
    virtual ~HGF3DCoordCollection() {};

#if (0)
    HGF3DCoordCollection<DataType>& operator=(const HGF3DCoordCollection<DataType>& pi_rObj)
        {
        if (this != &pi_rObj)
            {
            vector<HGF3DCoord<DataType>, allocator<HGF3DCoord<DataType> > >::operator=(pi_rObj);
            }
        return(*this);
        }
#endif
    };


template <class DataType> std::ostream& operator<<(std::ostream& io_OutputStream, const HGF3DCoord<DataType>& i_rPoint)
    {
    return(io_OutputStream << i_rPoint.GetX() << ", " << i_rPoint.GetY() << ", " << i_rPoint.GetZ());
    }

END_IMAGEPP_NAMESPACE


#include "HGF3DCoord.hpp"

