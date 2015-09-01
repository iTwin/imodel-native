//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGFBearing.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HGFBearing
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


// Most methods are declared inline and can be found in gfbearng.hpp
#include <Imagepp/all/h/HGFBearing.h>




/** -----------------------------------------------------------------------------
    This method permits to extract the trigonometric angle value expressed in
    radian. This is an implementation method that enables use of standard
    trigonometric functions defined in the standard library. The returned
    value can directly be fed into the standard trigonometric functions.

    @return A double containing the trigonometric value of the bearing as defined
            in the trigonometric space. This angle is always located
            between 0.0 inclusive and 2*PI exclusive.

    -----------------------------------------------------------------------------
*/
double HGFBearing::CalculateTrigoAngle() const
    {
    // The value must be finite
    HPRECONDITION(BeNumerical::BeFinite(m_Angle));

    // Create a radian angle CCW
    double MyAngle = 0.0;

    // Increment by offset from base
    MyAngle += m_Angle;

    // The value must be finite
    HASSERT(BeNumerical::BeFinite(MyAngle));

    // Limit to 0 to 2*PI domain
    while (MyAngle < 0.0)
        {
        MyAngle = MyAngle + (2*PI);
        }

    while (MyAngle >= (2*PI))
        {
        MyAngle = MyAngle - (2*PI);
        }

    return(MyAngle);
    }


/** -----------------------------------------------------------------------------
    Indicates if the given bearing is within the given sweep from self.
    Note that the sweep is always interpreted counter-clockwise.

    @param pi_Sweep IN The sweep. This value must be positive to represent
                        a counter-clockwise sweep and positive for a clockwise sweep.

    @param pi_rBearing IN The bearing to check if it is within the sweep of the
                       self bearing

    @return true if the given angle is within the sweep and false otherwise.
    -----------------------------------------------------------------------------
*/
bool HGFBearing::IsBearingWithinSweep(double             pi_Sweep,
                                       const HGFBearing& pi_rBearing) const
    {
    bool Answer;

    // Obtain trigonometric values for all angles and sweep
    double SelfTrigoAngle = CalculateTrigoAngle();
    double GivenTrigoAngle = pi_rBearing.CalculateTrigoAngle();
    double TrigoSweep;

    TrigoSweep = pi_Sweep;

    // Check is sweep is positive
    if (TrigoSweep > 0.0)
        {
        // The sweep is positive ...
        Answer = (((GivenTrigoAngle >= SelfTrigoAngle) && (GivenTrigoAngle <= SelfTrigoAngle + TrigoSweep)) ||
                  ((GivenTrigoAngle < SelfTrigoAngle) && (GivenTrigoAngle + (2 * PI) <= SelfTrigoAngle + TrigoSweep)));
        }
    else
        {
        // The sweep is negative ...
        Answer = (((GivenTrigoAngle <= SelfTrigoAngle) && (GivenTrigoAngle >= SelfTrigoAngle + TrigoSweep)) ||
                  ((GivenTrigoAngle > SelfTrigoAngle) && (GivenTrigoAngle - (2 * PI) >= SelfTrigoAngle + TrigoSweep)));
        }

    return(Answer);
    }



/** -----------------------------------------------------------------------------
    Subtraction operation. This operation subtracts from the bearing the given
    bearing. If the bearing is expressed in different units, then internal
    conversion will occur.
    The result angle is the result containing the difference in angle between
    the two bearing from first operand to second turning in the counter-clockwise
    direction. The angle returned is expressed in the units of the left operand.

    @param pi_rObj IN The bearing to subtract

    @return angle resulting from subtraction
    -----------------------------------------------------------------------------
*/
double HGFBearing::operator-(const HGFBearing& pi_rObj) const
    {
    // Evaluate difference in trigonometric space
    double TrigoAngleDiff = CalculateTrigoAngle() - pi_rObj.CalculateTrigoAngle();

    // Compensate for origin overflow
    if (TrigoAngleDiff < 0.0)
        TrigoAngleDiff += (2 * PI);

    // return recipient
    return TrigoAngleDiff;
    }


