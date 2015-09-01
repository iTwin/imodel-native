//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DCoord.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGF2DCoord
//-----------------------------------------------------------------------------
// Position in two-dimension
//-----------------------------------------------------------------------------

#pragma once

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert

    This class implements the nature and behavior for raw coordinates. Such coordinates
    are expressed using 2 numbers indicating values for two dimensions arbitrarily
    named X and Y. The two numbers can be of any valid numeric data type (DataType).
    Conversion between coordinates using one data type to coordinates using another
    is possible as long as a conversion exists in the form of a copy contructor (DataType(OtherDataType)).
    This conversion method is implemented in the form of a templated constructor. Implicit
    cast from one type of coordinate to another is therefore possible.

    This class is used to describe locations in two-dimensional coordinate
    systems.  It plays the role of an atomic "concrete data type" having no
    parent and not specifically designed to have child versions.
    Coordinate values labeled arbitrarily X and Y describe the
    position.
    Both X and Y values are kept in the form of a numeric value of the template
    arguments data type.
    The HGF2DCoord interface provides methods to access and change
    directly the values of the X and Y dimensions.
    The present class is to be used in any class that uses many coordinates,
    but requires only one coordinate system reference. It is more lightweight
    than the equivalent HGF2DLocation, but is to be used internally.

    Note the existence of the following definition in addition to the HGF2DCoord<T> class:
    typedef vector<HGF2DCoord<T>, allocator<HGF2DCoord<T>> > HGF2DCoordCollection<T>;

    This defines a collection of coordinates, and answers in all
    points to the vector<> Standard Template Library interface.

    -----------------------------------------------------------------------------
*/
template<class DataType = double> class HGF2DCoord
    {

public:

    typedef DataType NumberDataType;
    typedef DataType value_type; // Use STL convention also

    // This enum defined coordinate X and Y ids
    // to be used to extract values
    enum CoordRef
        {
        X = 0,
        Y = 1
        };

    // Primary methods
    /** -----------------------------------------------------------------------------
        Constructors and copy constructor for this class.  They allow three
        ways to construct a coordinate object: as an empty position (all
        coordinates at zero), as a new coord with specified coordinates,
        or as a duplicate of another coordinate object.

        @param pi_X X-axis coordinate of the position.

        @param pi_Y Y-axis coordinate of the position.

        @param pi_rObject Constant reference to a HGF2DCoord to duplicate.

        Example:
        @code
        HGF2DCoord<double>       ImageOrigin (10.0, 10.0);
        HGF2DCoord<double>        ImageWorld(ImageOrigin);
        HGF2DCoord<double>        APoint;
        @end

        -----------------------------------------------------------------------------
    */
    HGF2DCoord();
    HGF2DCoord(DataType pi_X,
               DataType pi_Y);

    // MICROSOFT BUG ... DEFINITION MUST BE INSIDE DECLARATION
    template<class OtherDataType>
    HGF2DCoord(const HGF2DCoord<OtherDataType>& pi_rObj)
        : m_XValue(pi_rObj.GetX()), m_YValue(pi_rObj.GetY())
        {
        }


    ~HGF2DCoord();
    HGF2DCoord<DataType>&     operator=(const HGF2DCoord<DataType>& pi_rObj);

    // Compare operations
    bool              operator==(const HGF2DCoord<DataType>& pi_rObj) const;
    bool              operator!=(const HGF2DCoord<DataType>& pi_rObj) const;

    // MICROSOFT BUG ... REQUIRED FOR ALPHA VC5.0 to create STL lists of coordinates
    bool              operator<(const HGF2DCoord<DataType>& pi_rObj) const;


    // Compare operations with epsilon
    bool              IsEqualTo(const HGF2DCoord<DataType>& pi_rObj) const;
    bool              IsEqualTo(const HGF2DCoord<DataType>& pi_rObj, DataType pi_Epsilon) const;
    bool              IsEqualToAutoEpsilon(const HGF2DCoord<DataType>& pi_rObj) const;

    // Location management
    DataType           GetX() const;
    DataType           GetY() const;
    void               SetX(DataType pi_X);
    void               SetY(DataType pi_Y);
    DataType&          operator[](int pi_CoordinateRef);

    // MICROSOFT BUG ... DEFINITION MUST BE INSIDE DECLARATION
    /** -----------------------------------------------------------------------------
        Dereference operator. It returns a reference to the specified coordinate.
        This value can be changed or consulted.

        @param pi_CoordinateRef An identificator that specifies the coordinate X
                                or Y that is to be returned. This represents
                                an integer value that can be equal to
                                HGF2DCoord::X  to make reference to
                                the X coordinate, and HGF2DCoord::Y
                                to make reference to the Y coordinate

        @return A reference to the internal coordinate that can be modified.

        Example:
        @code
        HGF2DCoord<double>      ImageOrigin (10, 10);
        double NewCoord = ImageOrigin[HGF2DCoord<double>::X] + 12;
        ImageOrigin[HGF2DCoord<double>::Y]+=23.1;
        @end

        @see GetX()
        @see GetY()
        @see SetX()
        @see SetY()
        -----------------------------------------------------------------------------
    */
    DataType&          operator[](CoordRef pi_CoordinateRef)
        {
        return ((pi_CoordinateRef == HGF2DCoord<DataType>::X) ? m_XValue : m_YValue);
        }

    const DataType&    operator[](int pi_CoordinateRef) const;

    // MICROSOFT BUG ... DEFINITION MUST BE INSIDE DECLARATION
    /** -----------------------------------------------------------------------------
        Dereference operator. It returns a reference to the specified coordinate.
        This value cannot be modified since the object is const.

        @param pi_CoordinateRef An identificator that specifies the coordinate X
                                or Y that is to be returned. This represents
                                an integer value that can be equal to
                                HGF2DCoord::X  to make reference to
                                the X coordinate, and HGF2DCoord::Y
                                to make reference to the Y coordinate.

        @return A reference to the internal coordinate that cannot be modified.

        Example:
        @code
        HGF2DCoord<double>      ImageOrigin (10, 10);
        double NewCoord = ImageOrigin[HGF2DCoord<double>::X] + 12;
        @end

        @see GetX()
        @see GetY()
        -----------------------------------------------------------------------------
    */
    const DataType&    operator[](CoordRef pi_CoordinateRef) const
        {
        return ((pi_CoordinateRef == HGF2DCoord<DataType>::X) ? m_XValue : m_YValue);
        }


    DataType CalculateLengthTo(const HGF2DCoord<DataType>& pi_rPoint) const;

    void       PrintState(ostream& po_rOutput) const;

protected:


private:
public :
    // Coordinates of this position
    DataType         m_XValue;
    DataType         m_YValue;
    };



/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert

    This template class implements the nature and behavior for a list of raw coordinates.
    The list inherits from the vector STL class and thus provides all the vector class interface
    -----------------------------------------------------------------------------
*/
template<class DataType> class HGF2DCoordCollection : public vector<HGF2DCoord<DataType>, allocator<HGF2DCoord<DataType> > >
    {
public:
    HGF2DCoordCollection() {};
    virtual ~HGF2DCoordCollection() {};

#if (0)
    HGF2DCoordCollection<DataType>& operator=(const HGF2DCoordCollection<DataType>& pi_rObj)
        {
        if (this != &pi_rObj)
            {
            vector<HGF2DCoord<DataType>, allocator<HGF2DCoord<DataType> > >::operator=(pi_rObj);
            }
        return(*this);
        }
#endif
    };

END_IMAGEPP_NAMESPACE

#include "HGF2DCoord.hpp"

