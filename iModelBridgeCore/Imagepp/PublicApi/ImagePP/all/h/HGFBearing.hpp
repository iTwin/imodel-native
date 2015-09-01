//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFBearing.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    Default constructor

    The bearing created has a value of 0
    -----------------------------------------------------------------------------
*/
inline HGFBearing::HGFBearing()
    : m_Angle(0.0)
    {
    }

/** -----------------------------------------------------------------------------
    Default constructor

    The bearing created has a value of 0
    -----------------------------------------------------------------------------
*/
inline HGFBearing::HGFBearing(double            pi_Angle)
    : m_Angle(pi_Angle)
    {
    }


//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HGFBearing object.
//-----------------------------------------------------------------------------
inline HGFBearing::HGFBearing(const HGFBearing& pi_rObj)
    : m_Angle(pi_rObj.m_Angle)
    {
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HGFBearing::~HGFBearing()
    {
    }


//-----------------------------------------------------------------------------
// operator=
// Assignment operator.
//-----------------------------------------------------------------------------
inline HGFBearing& HGFBearing::operator=(const HGFBearing& pi_rObj)
    {
    // Check that object to copy is not self
    if (this != &pi_rObj)
        {
        m_Angle = pi_rObj.m_Angle;
        }

    // Return reference to self
    return(*this);
    }


/** -----------------------------------------------------------------------------
    This method extracts the angle value of the bearing.

    @return The angle of the bearing.

    @see GetBase()
    @see SetAngle()
    -----------------------------------------------------------------------------
*/
inline double HGFBearing::GetAngle () const
    {
    return(m_Angle);
    }


/** -----------------------------------------------------------------------------
    This method sets the angle of the bearing.

    @param pi_Angle IN The new angle to assign to bearing.

    @see GetAngle()
    -----------------------------------------------------------------------------
*/
inline void HGFBearing::SetAngle (double pi_Angle)
    {
    m_Angle = pi_Angle;
    }


/** -----------------------------------------------------------------------------
    Equality evaluation operator.  Two bearing objects are considered "equal" if
    they represent the same bearing, whatever the bearing unit used.
    If they are not expressed in the same bearing unit ,
    internal conversion of either one will occur.

    @param pi_rObj IN Constant reference to HGFBearing object to compare to.

    @return A Boolean value that reflects the equality or inequality between operands.

    @see IsEqualTo()
    -----------------------------------------------------------------------------
*/
inline bool HGFBearing::operator==(const HGFBearing& pi_rObj) const
    {
    return(CalculateTrigoAngle() == pi_rObj.CalculateTrigoAngle());
    }

/** -----------------------------------------------------------------------------
    This method performs equality compare operations upon two bearings
    with application of a tolerance (epsilon).
    It applies the global fixed epsilon.

    The value of the epsilon in all case is interpreted in the units of the self distance.

    @param pi_rObj IN Constant reference to HGFBearing object to compare to.

    @return A Boolean value that reflects the equality or inequality between operands.

    @see operator==()
    -----------------------------------------------------------------------------
*/
inline bool HGFBearing::IsEqualTo(const HGFBearing& pi_rObj) const
    {
    double FirstAngle = CalculateTrigoAngle();
    double SecondAngle = pi_rObj.CalculateTrigoAngle();

    return(HDOUBLE_EQUAL_EPSILON(FirstAngle, SecondAngle));
    }


/** -----------------------------------------------------------------------------
    This method performs equality compare operations upon two bearings
    with application of a tolerance (epsilon).
    It requires the epsilon to be given as a parameter.

    The value of the epsilon in all case is interpreted in the units of the self distance.

    @param pi_rObj IN Constant reference to HGFBearing object to compare to.

    @return A Boolean value that reflects the equality or inequality between operands.

    @see operator==()
    -----------------------------------------------------------------------------
*/
inline bool HGFBearing::IsEqualTo(const HGFBearing& pi_rObj, double pi_Epsilon) const
    {
    HPRECONDITION(pi_Epsilon >= 0.0);

    double FirstAngle = CalculateTrigoAngle();
    double SecondAngle = pi_rObj.CalculateTrigoAngle();

    return(HDOUBLE_EQUAL(FirstAngle, SecondAngle, pi_Epsilon));
    }

/** -----------------------------------------------------------------------------
    This method performs equality compare operations upon two bearings
    with application of a tolerance (epsilon).
    It determines automatically the most appropriate epsilon based on the current
    value of self.

    The value of the epsilon in all case is interpreted in the units of the self distance.

    @param pi_rObj IN Constant reference to HGFBearing object to compare to.

    @return A Boolean value that reflects the equality or inequality between operands.

    @see operator==()
    -----------------------------------------------------------------------------
*/
inline bool HGFBearing::IsEqualToAutoEpsilon(const HGFBearing& pi_rObj) const
    {
    double FirstAngle = CalculateTrigoAngle();
    double SecondAngle = pi_rObj.CalculateTrigoAngle();
    double Epsilon =  HGLOBAL_EPSILON * fabs(FirstAngle);

    return(HDOUBLE_EQUAL(FirstAngle, SecondAngle, Epsilon));
    }


//-----------------------------------------------------------------------------
// operator!=
// See operator== : returns opposite value.
//-----------------------------------------------------------------------------
inline bool HGFBearing::operator!=(const HGFBearing& pi_rObj) const
    {
    return(CalculateTrigoAngle() != pi_rObj.CalculateTrigoAngle());
    }

/** -----------------------------------------------------------------------------
    Addition operation. This operation adds the bearing and the specified angle.

    @param pi_Angle IN Angle to add.

    @return HGFBearing resulting from addition

    @see operator-()
    -----------------------------------------------------------------------------
*/
inline HGFBearing HGFBearing::operator+(double pi_Angle) const
    {
    return(HGFBearing(m_Angle + pi_Angle));
    }

/** -----------------------------------------------------------------------------
    Subtraction operation. This operation subtracts the angle from bearing.

    @param pi_Angle IN Angle to subtract.

    @return HGFBearing resulting from subtraction

    @see operator+()
    -----------------------------------------------------------------------------
*/
inline HGFBearing HGFBearing::operator-(double  pi_Angle) const
    {
    return(HGFBearing(m_Angle - pi_Angle));
    }

/** -----------------------------------------------------------------------------
    Increment operation. This operation increments the bearing by the angle
    represented by the second operand. If the bearing and angle are expressed
    in different units then internal conversion may occur.

    @param pi_Angle IN Angle to add.

    @return Reference to self to be used as an l-value.

    @see operator-=()
    @see operator+()
    -----------------------------------------------------------------------------
*/
inline HGFBearing& HGFBearing::operator+=(double pi_Angle)
    {
    // Set angle to sum
    m_Angle += pi_Angle;

    // return reference to self
    return(*this);
    }

/** -----------------------------------------------------------------------------
    Decrement operation. This operation decrements the bearing by the angle
    represented by the second operand. If the bearing and the angle are
    expressed in different units then internal conversion may occur.

    @param pi_Angle IN Angle to subtract.

    @return Reference to self to be used as an l-value.

    @see operator+=()
    @see operator-()
    -----------------------------------------------------------------------------
*/
inline HGFBearing& HGFBearing::operator-=(double pi_Angle)
    {
    // Set value to difference
    m_Angle -= pi_Angle;

    // return reference to self
    return(*this);
    }


END_IMAGEPP_NAMESPACE
