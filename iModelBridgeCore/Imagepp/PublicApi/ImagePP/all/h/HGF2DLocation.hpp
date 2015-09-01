//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DLocation.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HGF2DLocation
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE

/** -----------------------------------------------------------------------------
    Default Constructor

    The location is created as an empty location (all coordinates at zero) without
    coordinate system specified so SetCoordSys() will
    have to be used eventually.

    -----------------------------------------------------------------------------
*/
inline HGF2DLocation::HGF2DLocation()
    : m_pCoordSys(new HGF2DCoordSys())
    {
    }

/** -----------------------------------------------------------------------------
    Constructor with specification of interpretation coordinate system

    The location is created as an empty location (all coordinates at zero).

    @param pi_rpCoordSys IN Constant reference to a smart pointer to a new
                            coordinate system object expressing the coordinate
                            system that will be used by this location.

    -----------------------------------------------------------------------------
*/
inline HGF2DLocation::HGF2DLocation(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : m_XValue(0.0),
      m_YValue(0.0),
      m_pCoordSys(pi_rpCoordSys)
    {
    }

//-----------------------------------------------------------------------------
// The copy constructor.  It duplicates another location object.
//-----------------------------------------------------------------------------
inline HGF2DLocation::HGF2DLocation(const HGF2DLocation& pi_rObj)
    : m_XValue(pi_rObj.m_XValue),
      m_YValue(pi_rObj.m_YValue),
      m_pCoordSys(pi_rObj.m_pCoordSys)
    {
    }

/** -----------------------------------------------------------------------------
    Alternate copy constructor

    It creates a copy of the given location, but while converting value interpretation
    to the alternate coordinate system.
    The constructor taking another location and a coordinate system is provided for
    performance reason only. It is the fastest way to obtain values of a constant
    location interpreted in another coordinate system, without first making a
    duplicate then converting this copy. All those steps are made efficiently in
    the constructor.

    @param pi_rObj IN Constant reference to the location object to duplicate.

    @param pi_rpCoordSys IN Smart pointer to new interpretation coordinate system.

    -----------------------------------------------------------------------------
*/
inline HGF2DLocation::HGF2DLocation(const HGF2DLocation& pi_rObj,
                                    const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : m_XValue(pi_rObj.m_XValue),
      m_YValue(pi_rObj.m_YValue),
      m_pCoordSys(pi_rpCoordSys)
    {
    // Convert values
    pi_rpCoordSys->ConvertFrom(pi_rObj.m_pCoordSys, &m_XValue, &m_YValue);
    }

/** -----------------------------------------------------------------------------
    Creates a new location by specification of coordinate values and coordinate
    system.

    @param pi_X IN X-axis raw coordinate of the location, interpreted in the
                X dimension units of the provided coordinate system.

    @param pi_Y IN Y-axis raw coordinate of the location, interpreted in the
                Y dimension units of the provided coordinate system.

    @param pi_rpCoordSys IN Constant reference to a smart pointer to a new
                         coordinate system object expressing the coordinate
                         system that will be used by this location.

    -----------------------------------------------------------------------------
*/
inline HGF2DLocation::HGF2DLocation(double pi_X,
                                    double pi_Y,
                                    const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : m_pCoordSys(pi_rpCoordSys),
      m_XValue(pi_X),
      m_YValue(pi_Y)
    {
    }


/** -----------------------------------------------------------------------------
    Constructor from position.

    Creates a location using values of the position and the given interpretation
    coordinate system.

    @param pi_rPoint IN Constant reference to an HGF2Dposition object from which
                        are obtained the raw coordinates of the new location.

    @param pi_rpCoordSys IN Constant reference to a smart pointer to a new
                        coordinate system object expressing the coordinate system
                        that will be used by this location.

    -----------------------------------------------------------------------------
*/
inline HGF2DLocation::HGF2DLocation(const HGF2DPosition& pi_rPoint,
                                    const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : m_pCoordSys(pi_rpCoordSys),
      m_XValue(pi_rPoint.GetX()),
      m_YValue(pi_rPoint.GetY())
    {
    }


//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HGF2DLocation::~HGF2DLocation()
    {
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another location object.  Current system
// is also changed to the one found in copied object.
//-----------------------------------------------------------------------------
inline HGF2DLocation& HGF2DLocation::operator=(const HGF2DLocation& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        m_XValue = pi_rObj.m_XValue;
        m_YValue = pi_rObj.m_YValue;
        m_pCoordSys = pi_rObj.m_pCoordSys;
        }

    return (*this);
    }


//-----------------------------------------------------------------------------
// operator!=
// See operator== : returns opposite value.
//-----------------------------------------------------------------------------
inline bool HGF2DLocation::operator!=(const HGF2DLocation& pi_rObj) const
    {
    return (!operator==(pi_rObj));
    }


/** -----------------------------------------------------------------------------
    This method changes the coordinate system object used by this location,
    without changing its position in the previous coordinate system used.
    To maintain the position, actual X and Y values of this location are converted.
    Reference to the coordinate system object is kept as a smart pointer to it,
    allowing utilization of a single coordinate system object among many rectangles,
    points and other objects using coordinates, without having to track which
    coordinate system is used or not.

    @param pi_rpCoordSys IN Constant reference to a smart pointer to a new
                        coordinate system object expressing the coordinate system
                        that will be used by this location.

    @see GetCoordSys()
    @see SetCoordSys()
    -----------------------------------------------------------------------------
*/
inline void  HGF2DLocation::ChangeCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    {
    // Transform to new coord system
    pi_rpCoordSys->ConvertFrom(m_pCoordSys, &m_XValue, &m_YValue);

    // Set new coord system
    m_pCoordSys = pi_rpCoordSys;
    }

/** -----------------------------------------------------------------------------
    Returns a position with X and Y values.

    @return The position

    -----------------------------------------------------------------------------
*/
inline HGF2DPosition HGF2DLocation::GetPosition() const
    {
    return(HGF2DPosition(m_XValue, m_YValue));
    }


/** -----------------------------------------------------------------------------
    Returns the value of the X dimension part coordinate of this location.

    @return The X dimension coordinate for this location, expressed in the
            current coordinate system units of the X dimension of the location
            object.

    @see SetX()
    -----------------------------------------------------------------------------
*/
inline double HGF2DLocation::GetX() const
    {
    // Compose and return distance with value and coordinate system unit
    return m_XValue;
    }

/** -----------------------------------------------------------------------------
    Returns the value of the Y dimension part coordinate of this location.

    @return The Y dimension coordinate for this location, expressed in the
            current coordinate system units of the X dimension of the location
            object.

    @see SetY()
    -----------------------------------------------------------------------------
*/
inline double HGF2DLocation::GetY() const
    {
    // Compose and return distance with value and coordinate system unit
    return m_YValue;
    }



/** -----------------------------------------------------------------------------
    Sets a new value for the X dimension coordinate of this location.

    @param pi_rX IN New X dimension numeric coordinate, expressed in the current
                    coordinate system of this location object.


    @see GetX()
    @see SetY()
    -----------------------------------------------------------------------------
*/
inline void HGF2DLocation::SetX(double pi_X)
    {
    // Set value after converting to current coordinate system unit
    m_XValue = pi_X;
    }

/** -----------------------------------------------------------------------------
    Sets a new value for the Y dimension coordinate of this location.

    @param pi_rY IN New Y dimension numeric coordinate, expressed in the current
                    coordinate system of this location object.


    @see GetY()
    @see SetX()
    -----------------------------------------------------------------------------
*/
inline void HGF2DLocation::SetY(double pi_Y)
    {
    // Set value after converting to current coordinate system unit
    m_YValue = pi_Y;
    }


/** -----------------------------------------------------------------------------
    Returns a constant reference to smart pointer to the coordinate system object
    used by this location to express all its values.

    @return Constant reference to smart pointer to the coordinate system object
            used by the location object.

    @see SetCoordSys()
    @see ChangeCoordSys()
    -----------------------------------------------------------------------------
*/
inline const HFCPtr<HGF2DCoordSys>& HGF2DLocation::GetCoordSys() const
    {
    return (m_pCoordSys);
    }

/** -----------------------------------------------------------------------------
    This method changes the coordinate system object used by this location.
    Actual X and Y values of this location remain the same, but they will be
    interpreted in the new coordinate system.

    Reference to the coordinate system object is kept as a smart pointer to
    it, allowing utilization of a single coordinate system object among many
    extents, points and other objects using coordinates, without having to track
    which coordinate system is used or not.


    @param pi_rpSystem IN Constant reference to smart pointer to a new coordinate
                         system object that will be used by this location object.

    @see GetCoordSys()
    @see ChangeCoordSys()
    -----------------------------------------------------------------------------
*/
inline void HGF2DLocation::SetCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpSystem)
    {
    m_pCoordSys = pi_rpSystem;
    }


/** -----------------------------------------------------------------------------
    Returns a duplicate of self expressed in another specified coordinate system.

    @param pi_rpSystem IN Constant reference to a smart pointer to a coordinate
                          system in which must be expressed the returned location.

    @see SetCoordSys()
    @see ChangeCoordSys()
    -----------------------------------------------------------------------------
*/
inline HGF2DLocation HGF2DLocation::ExpressedIn(const HFCPtr<HGF2DCoordSys>& pi_rpSystem) const
    {
    return(HGF2DLocation(*this, pi_rpSystem));
    }


/** -----------------------------------------------------------------------------
    Sets a new value for the location based on the location given. The coordinate
    system is not modified. This method operates as the operator=() method, but
    does not modify the coordinate system used in the interpretation of the self
    location. The values extracted from the given location are first converted to
    the self coordinate system.

    @param pi_rLocation IN Constant reference from which is taken the description
                           of the location.

    @see SetX()
    @see SetY()
    -----------------------------------------------------------------------------
*/
inline void HGF2DLocation::Set(const HGF2DLocation& pi_rLocation)
    {
    // Copy values
    m_XValue = pi_rLocation.m_XValue;
    m_YValue = pi_rLocation.m_YValue;

    if (m_pCoordSys != pi_rLocation.m_pCoordSys)
        {
        // Convert values
        m_pCoordSys->ConvertFrom(pi_rLocation.m_pCoordSys, &m_XValue, &m_YValue);
        }
    }




/** -----------------------------------------------------------------------------
    This method performs equality compare operations upon two locations with
    application of a tolerance (epsilon).
    It applies the global fixed epsilon
    The value of the epsilon is interpreted in the units of the dimensions of each
    X and Y dimension taken individually.
    The given location must have its interpretation coordinate system identical
    to the one used by self.

    @param pi_rObj Constant reference to the HGF2DLocation to compare with.

    @return true if the locations are equal and false otherwise.

    @see IsEqualTo()
    -----------------------------------------------------------------------------
*/
inline bool HGF2DLocation::IsEqualToSCS(const HGF2DLocation& pi_rObj) const
    {
    HPRECONDITION(GetCoordSys() == pi_rObj.GetCoordSys());

    // Compare coordinates
    return(HDOUBLE_EQUAL_EPSILON(pi_rObj.m_XValue, m_XValue) &&
           HDOUBLE_EQUAL_EPSILON(pi_rObj.m_YValue, m_YValue));
    }


/** -----------------------------------------------------------------------------
    This method performs equality compare operations upon two locations with
    application of a tolerance (epsilon).
    It applies the epsilon given as a parameter.
    The value of the epsilon is interpreted in the units of the dimensions of each
    X and Y dimension taken individually.
    The given location must have its interpretation coordinate system identical
    to the one used by self.

    @param pi_rObj Constant reference to the HGF2DLocation to compare with.

    @param pi_Epsilon IN Tolerance to apply to compare operation. This number
                         must be greater or equal to 0.0

    @return true if the locations are equal and false otherwise.

    @see IsEqualTo()
    -----------------------------------------------------------------------------
*/
inline bool HGF2DLocation::IsEqualToSCS(const HGF2DLocation& pi_rObj, double pi_Epsilon) const
    {
    HPRECONDITION(GetCoordSys() == pi_rObj.GetCoordSys());

    // Compare coordinates
    return(HDOUBLE_EQUAL(pi_rObj.m_XValue, m_XValue, pi_Epsilon) &&
           HDOUBLE_EQUAL(pi_rObj.m_YValue, m_YValue, pi_Epsilon));
    }

/** -----------------------------------------------------------------------------
    This method performs equality compare operations upon two locations with
    application of a tolerance (epsilon).
    It determines automatically  the most appropriate epsilon based on the current value of self.
    In this method, the epsilon applied to the X and Y coordinates is different, and based
    on the individual values of each dimensions.
    The value of the epsilon is interpreted in the units of the dimensions of each
    X and Y dimension taken individually.
    The given location must have its interpretation coordinate system identical
    to the one used by self.

    @param pi_rObj Constant reference to the HGF2DLocation to compare with.

    @return true if the locations are equal and false otherwise.

    @see IsEqualTo()
    -----------------------------------------------------------------------------
*/
inline bool HGF2DLocation::IsEqualToAutoEpsilonSCS(const HGF2DLocation& pi_rObj) const
    {
    HPRECONDITION(GetCoordSys() == pi_rObj.GetCoordSys());

    // Compare coordinates
    return(HDOUBLE_EQUAL(pi_rObj.m_XValue, m_XValue, fabs(HGLOBAL_EPSILON * m_XValue)) &&
           HDOUBLE_EQUAL(pi_rObj.m_YValue, m_YValue, fabs(HGLOBAL_EPSILON * m_YValue)));
    }




/** -----------------------------------------------------------------------------
    This method performs equality compare operations upon two locations with
    application of a tolerance (epsilon).
    It applies the global fixed epsilon
    The value of the epsilon is interpreted in the units of the dimensions of each
    X and Y dimension taken individually.

    @param pi_rObj Constant reference to the HGF2DLocation to compare with.

    @return true if the locations are equal and false otherwise.

    @see IsEqualTo()
    -----------------------------------------------------------------------------
*/
inline bool HGF2DLocation::IsEqualTo(const HGF2DLocation& pi_rObj) const
    {
    // Declared doubles to receive results
    double     NewX;
    double     NewY;

    // Obtain new coordinates (expressed in the coordinate sys of self
    m_pCoordSys->ConvertFrom (pi_rObj.m_pCoordSys, pi_rObj.m_XValue, pi_rObj.m_YValue,
                              &NewX, &NewY);

    // Compare coordinates
    return(HDOUBLE_EQUAL_EPSILON(NewX, m_XValue) && HDOUBLE_EQUAL_EPSILON(NewY, m_YValue));
    }


/** -----------------------------------------------------------------------------
    This method performs equality compare operations upon two locations with
    application of a tolerance (epsilon).
    It applies the epsilon given as a parameter.
    The value of the epsilon is interpreted in the units of the dimensions of each
    X and Y dimension taken individually.
    The given location must have its interpretation coordinate system identical
    to the one used by self.

    @param pi_rObj Constant reference to the HGF2DLocation to compare with.

    @param pi_Epsilon IN Tolerance to apply to compare operation. This number
                         must be greater or equal to 0.0

    @return true if the locations are equal and false otherwise.

    @see IsEqualTo()
    -----------------------------------------------------------------------------
*/
inline bool HGF2DLocation::IsEqualTo(const HGF2DLocation& pi_rObj, double pi_Epsilon) const
    {
    // Declared doubles to receive results
    double     NewX;
    double     NewY;

    // Obtain new coordinates (expressed in the coordinate sys of self
    m_pCoordSys->ConvertFrom (pi_rObj.m_pCoordSys, pi_rObj.m_XValue, pi_rObj.m_YValue,
                              &NewX, &NewY);

    // Compare coordinates
    return(HDOUBLE_EQUAL(NewX, m_XValue, pi_Epsilon) && HDOUBLE_EQUAL(NewY, m_YValue, pi_Epsilon));
    }

/** -----------------------------------------------------------------------------
    This method performs equality compare operations upon two locations with
    application of a tolerance (epsilon).
    It determines automatically  the most appropriate epsilon based on the current value of self.
    In this method, the epsilon applied to the X and Y coordinates is different, and based
    on the individual values of each dimensions.
    The value of the epsilon is interpreted in the units of the dimensions of each
    X and Y dimension taken individually.
    The given location must have its interpretation coordinate system identical
    to the one used by self.

    @param pi_rObj Constant reference to the HGF2DLocation to compare with.

    @return true if the locations are equal and false otherwise.

    @see IsEqualTo()
    -----------------------------------------------------------------------------
*/
inline bool HGF2DLocation::IsEqualToAutoEpsilon(const HGF2DLocation& pi_rObj) const
    {
    // Declared doubles to receive results
    double     NewX;
    double     NewY;

    // Obtain new coordinates (expressed in the coordinate sys of self
    m_pCoordSys->ConvertFrom (pi_rObj.m_pCoordSys, pi_rObj.m_XValue, pi_rObj.m_YValue,
                              &NewX, &NewY);

    // Compare coordinates
    return(HDOUBLE_EQUAL(NewX, m_XValue, fabs(HGLOBAL_EPSILON * MAX(m_XValue, NewX))) &&
           HDOUBLE_EQUAL(NewY, m_YValue, fabs(HGLOBAL_EPSILON * MAX(m_YValue, NewY))));
    }


END_IMAGEPP_NAMESPACE