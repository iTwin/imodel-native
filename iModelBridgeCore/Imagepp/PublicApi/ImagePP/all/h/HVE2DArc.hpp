//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DArc.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#include "HVE2DCircle.h"

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
inline HVE2DArc::HVE2DArc()
    : HVE2DBasicLinear(),
      m_RotationDirection(HGFAngle::CCW)
    {
    }



/** -----------------------------------------------------------------------------
    Creates an arc of circle based on a center point, radius, start bearing
    end bearing and a rotation direction. The start and end points are computed
    from these settings.
    The radius may not be null nor negative.
    The interpretation coordinate system is obtained from the center point.

    @param pi_rCenter The location of the center point of the circle to
                      which the constructed arc belongs to.

    @param pi_rStartBearing The bearing from center point to start point.

    @param pi_rEndBearing  The bearing from center point to end point.

    @param pi_rRadius  The radius of the circle the arc belongs to.

    @param pi_Direction The rotation direction upon arc from start point
                        to end point. The value must be either
                        CW(ClockWise) or CCW(CounterClockWise)

    @see HGF2DLocation
    @see HGFBearing
    -----------------------------------------------------------------------------
*/
inline HVE2DArc::HVE2DArc(const HGF2DLocation& pi_rCenter,
                          const HGFBearing&    pi_rStartBearing,
                          const HGFBearing&    pi_rEndBearing,
                          double               pi_Radius,
                          HGFAngle::AngleDirection pi_Direction)
    : HVE2DBasicLinear(pi_rCenter.GetCoordSys()),
      m_Center(pi_rCenter),
      m_RotationDirection(pi_Direction)
    {
    // Radius may not be 0 or negative
    HPRECONDITION(pi_Radius > 0.0);

    // Calculate start point
    m_StartPoint = m_Center + HGF2DDisplacement(pi_rStartBearing, pi_Radius);

    // Calculate end point
    m_EndPoint = m_Center + HGF2DDisplacement(pi_rEndBearing, pi_Radius);
    }


/** -----------------------------------------------------------------------------
    Creates an arc of circle based on a circle a start bearing and the
    sweep of the angle. The sweep may be negative to specify a clockwise
    arc rotation or position for a counter clockwise rotation.
    The interpretation coordinate system is obtained from the circle.

    @param pi_rCircle  The circle the arc belongs to.

    @param pi_rStartBearing The bearing from center point or given
                            circle to start point.

    @param pi_Sweep The sweep of the arc. The sweep is an angle indicating
                     the portion of a full circle captured by the arc.
                     This sweep angle also specifies the direction from start
                     to end point.
                     The absolute value of the angle must not exceed 2PI radians.

    @see HGFBearing
    @see HVE2DCircle
    @see HGFAngle
    -----------------------------------------------------------------------------
*/
inline HVE2DArc::HVE2DArc(const HVE2DCircle& pi_rCircle,
                          const HGFBearing&  pi_rStartBearing,
                          double             pi_Sweep)
    : HVE2DBasicLinear(pi_rCircle.GetCoordSys()),
      m_Center(pi_rCircle.GetCenter())
    {
    // Sweep must be smaller than 360 degrees
    HPRECONDITION(fabs(pi_Sweep) < 2*PI);

    // Calculate start point
    m_StartPoint = m_Center + HGF2DDisplacement(pi_rStartBearing, pi_rCircle.GetRadius());

    // Calculate end point
    m_EndPoint = m_Center + HGF2DDisplacement(pi_rStartBearing + pi_Sweep,
                                              pi_rCircle.GetRadius());

    if (pi_Sweep > 0.0)
        m_RotationDirection = HGFAngle::CCW;
    else
        m_RotationDirection = HGFAngle::CW;

    }

/** -----------------------------------------------------------------------------
    Creates a null arc of circle using simply the interpretation coordinate
    system. The center point is located at (0,0) and the radius is null. This
    creates an unuseable arc of circle for which properties should
    be set after creation.

    @param pi_rpCoordSys Smart pointer to interpretation coordinate system. This
                         pointer may not be null.

    @see HGF2DCoordSys
    -----------------------------------------------------------------------------
*/
inline HVE2DArc::HVE2DArc(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HVE2DBasicLinear(pi_rpCoordSys),
      m_Center(pi_rpCoordSys),
      m_RotationDirection(HGFAngle::CCW)
    {
    }

/** -----------------------------------------------------------------------------
    Copy Constructor. Creates an arc as the duplicate of given arc of circle.

    @param pi_rObj The arc to duplicate

    -----------------------------------------------------------------------------
*/
inline HVE2DArc::HVE2DArc(const HVE2DArc& pi_rObj)
    : HVE2DBasicLinear(pi_rObj),
      m_Center(pi_rObj.m_Center),
      m_RotationDirection(pi_rObj.m_RotationDirection)
    {
    }

/** -----------------------------------------------------------------------------
    Destroyer.

    -----------------------------------------------------------------------------
*/
inline HVE2DArc::~HVE2DArc()
    {
    }

/** -----------------------------------------------------------------------------
    Assignment operator.  It duplicates another arc object.
    -----------------------------------------------------------------------------
*/
inline HVE2DArc& HVE2DArc::operator=(const HVE2DArc& pi_rObj)
    {
    // Check that object to copy is not self
    if (this != &pi_rObj)
        {
        // Invoque the ancester operator
        HVE2DBasicLinear::operator=(pi_rObj);

        // Copy attributes
        m_Center = pi_rObj.m_Center;
        m_RotationDirection = pi_rObj.m_RotationDirection;
        }

    // Return reference to self
    return (*this);
    }


//-----------------------------------------------------------------------------
// AutoCrosses
// Indicates if the linear crosses itself
// This is of course impossible for an arc
//-----------------------------------------------------------------------------
inline bool HVE2DArc::AutoCrosses() const
    {
    return(false);
    }

//-----------------------------------------------------------------------------
// GetBasicLinearType
// Return the basic linear type
//-----------------------------------------------------------------------------
inline HVE2DBasicLinearTypeId HVE2DArc::GetBasicLinearType() const
    {
    return (HVE2DArc::CLASS_ID);
    }


/** -----------------------------------------------------------------------------
    Calculates and returns the circle to which the arc belongs to

    @return The circle the arc belongs to.

    @see HVE2DCircle
    -----------------------------------------------------------------------------
*/
inline HVE2DCircle HVE2DArc::CalculateCircle() const
    {
    // Check that the arc is not NULL
    HPRECONDITION(!IsNull());

    // Return center
    return(HVE2DCircle(m_Center, (m_StartPoint - m_Center).CalculateLength()));
    }

/** -----------------------------------------------------------------------------
    Calculates and returns the bearing of arc at specified point.
    The given point must be located upon the arc of circle.
    -----------------------------------------------------------------------------
*/
inline HGFBearing HVE2DArc::CalculateBearing(const HGF2DLocation& pi_rPoint,
                                             HVE2DVector::ArbitraryDirection pi_Direction) const
    {
    // The arc must not be NULL
    HPRECONDITION(!IsNull());

    // The point must be located on arc
    HPRECONDITION(IsPointOn(pi_rPoint));

    // Create right angle to use latter
    double    RightAngle = PI/2;

    // Evaluate right bearing to point
    HGFBearing  MainBearing = (m_Center - pi_rPoint).CalculateBearing();

    HGFBearing  TheBearing;

    // Check from rotation direction and direction desired if we must add or substract
    // right angle
    if (((m_RotationDirection == HGFAngle::CCW) && (pi_Direction == HVE2DVector::BETA)) ||
        ((m_RotationDirection == HGFAngle::CW) && (pi_Direction != HVE2DVector::BETA)))
        {
        TheBearing = MainBearing + RightAngle;
        }
    else
        {
        TheBearing = MainBearing - RightAngle;
        }

    return (TheBearing);
    }

//-----------------------------------------------------------------------------
// CalculateAngularAcceleration
// Calculates and returns the angular acceleration of arc at specified point
//-----------------------------------------------------------------------------
inline double HVE2DArc::CalculateAngularAcceleration(
    const HGF2DLocation& pi_rPoint,
    HVE2DVector::ArbitraryDirection pi_Direction) const
    {
    // The point must be located on arc
    HPRECONDITION(IsPointOn(pi_rPoint));

    // The acceleration on an arc is constant
    // Obtain bearing difference between start and end point
    double DiffAngle = (m_Center - m_EndPoint).CalculateBearing() -
                       (m_Center - m_StartPoint).CalculateBearing();

    // Obtain a 1 radian angle
    double  UnaryAngle = 1.0;

    // Declare recipient variable
    double TheAcceleration = 0.0;

    // Check the direction of arc and direction of acceleration wanted
    if (((m_RotationDirection == HGFAngle::CCW) && (pi_Direction == HVE2DVector::BETA)) ||
        ((m_RotationDirection == HGFAngle::CW) && (pi_Direction != HVE2DVector::BETA)))
        {
        TheAcceleration = UnaryAngle / (m_Center - m_StartPoint).CalculateLength();
        }
    else
        {
        TheAcceleration = - UnaryAngle / (m_Center - m_StartPoint).CalculateLength();
        }

    return(TheAcceleration);
    }


//-----------------------------------------------------------------------------
// CalculateRayArea
// Calculates and returns the area of the ray extended from given point to
// the linear
//-----------------------------------------------------------------------------
inline double HVE2DArc::CalculateRayArea(const HGF2DLocation& pi_rPoint) const
    {
    double Radius = CalculateRadius();

    return(((m_StartPoint.GetX() * ((m_EndPoint - pi_rPoint).GetDeltaY())) +
            (Radius * Radius * CalculateNormalizedTrigoValue(CalculateSweep()))) / 2);
    }

/** -----------------------------------------------------------------------------
    Calculates and returns the bearing from center to start point.

    @return The bearing from center to start point.

    @see HGFBearing
    -----------------------------------------------------------------------------
*/
inline HGFBearing HVE2DArc::CalculateStartBearing() const
    {
    return((m_StartPoint - m_Center).CalculateBearing());
    }


/** -----------------------------------------------------------------------------
    Calculates and returns the bearing from center to end point.

    @return The bearing from center to end point.

    @see HGFBearing
    -----------------------------------------------------------------------------
*/
inline HGFBearing HVE2DArc::CalculateEndBearing() const
    {
    return((m_EndPoint - m_Center).CalculateBearing());
    }

//-----------------------------------------------------------------------------
// CalculateRelativePoint
// Calculates and returns the location based on the given relative position.
//-----------------------------------------------------------------------------
inline HGF2DLocation HVE2DArc::CalculateRelativePoint(double pi_RelativePos) const
    {
    // The relative position must be between 0.0 and 1.0
    HPRECONDITION((pi_RelativePos >= 0.0) && (pi_RelativePos <= 1.0));

    // Obtain displacement from center to start point
    HGF2DDisplacement  FromCenterToStart(m_StartPoint - m_Center);

    // Compute full angle sweep
    double  Sweep(CalculateSweep());

    // Adjust Sweep
    Sweep *= pi_RelativePos;

    // Evaluate displacement from center to
    HGF2DDisplacement  NewDisplacement(FromCenterToStart.CalculateBearing() + Sweep,
                                       FromCenterToStart.CalculateLength());

    return (m_Center + NewDisplacement);
    }

//-----------------------------------------------------------------------------
// Shorten
// Shortens the arc definition by specification of relative positions to self.
//-----------------------------------------------------------------------------
inline void HVE2DArc::Shorten(double pi_StartRelativePos, double pi_EndRelativePos)
    {
    // The relative positions given must be between 0.0 and 1.0
    HPRECONDITION((pi_StartRelativePos >= 0.0) && (pi_StartRelativePos <= 1.0));
    HPRECONDITION((pi_EndRelativePos >= 0.0) && (pi_EndRelativePos <= 1.0));

    Shorten(CalculateRelativePoint(pi_StartRelativePos),
            CalculateRelativePoint(pi_EndRelativePos));
    }


//-----------------------------------------------------------------------------
// Shorten
// Shortens the arc definition by specification of new start and end points
//-----------------------------------------------------------------------------
inline void HVE2DArc::Shorten(const HGF2DLocation& pi_rNewStartPoint,
                              const HGF2DLocation& pi_rNewEndPoint)
    {
    // The shorten points must be located on arc
    HPRECONDITION(IsPointOn(pi_rNewStartPoint) && IsPointOn(pi_rNewEndPoint));

    // The relative position of given start point must be smaller than
    // the relative position of given end point
    HPRECONDITION((CalculateLength() == 0.0) ||
                  (CalculateRelativePosition(pi_rNewStartPoint) <=
                   CalculateRelativePosition(pi_rNewEndPoint)));

    // Set extremity points while keeping expression in the arc coordinate system
    m_EndPoint.Set(pi_rNewEndPoint);
    m_StartPoint.Set(pi_rNewStartPoint);
    }


//-----------------------------------------------------------------------------
// ShortenTo
// Shortens the arc definition by specification of a new end point
//-----------------------------------------------------------------------------
inline void HVE2DArc::ShortenTo(const HGF2DLocation& pi_rNewEndPoint)
    {
    // The given point must be located on arc
    HPRECONDITION(IsPointOn(pi_rNewEndPoint));

    // The end point is set while keeping interpretation coordinate system
    // to the one of the arc
    m_EndPoint.Set(pi_rNewEndPoint);
    }

//-----------------------------------------------------------------------------
// ShortenTo
// Shortens the arc definition by specification of a new end relative point
//-----------------------------------------------------------------------------
inline void HVE2DArc::ShortenTo(double pi_EndRelativePos)
    {
    // The given relative position must be greater than 0.0 and smaller (or equal) than 1.0
    HPRECONDITION((pi_EndRelativePos >= 0.0) && (pi_EndRelativePos <= 1.0));

    ShortenTo(CalculateRelativePoint(pi_EndRelativePos));
    }


//-----------------------------------------------------------------------------
// ShortenFrom
// Shortens the arc definition by specification of a new start point
//-----------------------------------------------------------------------------
inline void HVE2DArc::ShortenFrom(const HGF2DLocation& pi_rNewStartPoint)
    {
    // The given point must be located on arc
    HPRECONDITION(IsPointOn(pi_rNewStartPoint));

    // The start point is set while keeping interpretation coordinate system
    // to the one of the arc
    m_StartPoint.Set(pi_rNewStartPoint);
    }


//-----------------------------------------------------------------------------
// ShortenFrom
// Shortens the arc definition by specification of a new start relative point
//-----------------------------------------------------------------------------
inline void HVE2DArc::ShortenFrom(double pi_StartRelativePos)
    {
    // The given relative position must be greater (or equal) than 0.0 and smaller than 1.0
    HPRECONDITION((pi_StartRelativePos >= 0.0) && (pi_StartRelativePos <= 1.0));

    ShortenFrom(CalculateRelativePoint(pi_StartRelativePos));
    }




//-----------------------------------------------------------------------------
// Clone
// Returns a dynamically allocated copy of the arc
//-----------------------------------------------------------------------------
inline HPMPersistentObject* HVE2DArc::Clone() const
    {
    return(new HVE2DArc(*this));
    }

//-----------------------------------------------------------------------------
// Move
// Moves the arc by specified displacement
//-----------------------------------------------------------------------------
inline void HVE2DArc::Move(const HGF2DDisplacement& pi_rDisplacement)
    {
    // Move the end point
    HVE2DLinear::Move(pi_rDisplacement);

    // Move center
    m_Center += pi_rDisplacement;
    }

//-----------------------------------------------------------------------------
// Scale
// Scales the arc by specified scale fator around origin
//-----------------------------------------------------------------------------
inline void HVE2DArc::Scale(double pi_ScaleFactor,
                            const HGF2DLocation& pi_rScaleOrigin)
    {
    // The scale factor must be positive
    HPRECONDITION(pi_ScaleFactor > 0.0);

    // Scale the end points
    HVE2DLinear::Scale(pi_ScaleFactor, pi_rScaleOrigin);

    // Scale center
    m_Center += (m_Center - pi_rScaleOrigin) * pi_ScaleFactor;
    }


//-----------------------------------------------------------------------------
// CalculateLength
// Calculates and returns the length of arc
//-----------------------------------------------------------------------------
inline double HVE2DArc::CalculateLength() const
    {
    return (CalculateRadius() * CalculateNormalizedTrigoValue(CalculateSweep()));
    }

/** -----------------------------------------------------------------------------
    Calculates and returns the radius of the circle the arc belongs to. The
    unots used in the representation of the returned distance are the units
    of th X dimension of the coordinate system used in the interpretation
    of the linear.

    @return The radius of circle the arc belongs to.

    -----------------------------------------------------------------------------
*/
inline double HVE2DArc::CalculateRadius() const
    {
    return ((m_Center - m_StartPoint).CalculateLength());
    }


/** -----------------------------------------------------------------------------
    Calculates and returns the sweep of the arc. The sweep is always returned
    in radians and is negative for a clockwise sweep and positive otherwise.
    The sweep is the size of the arc from start to end point. The value
    is always between 0 and 2PI radians inclusive.

    @return The sweep of the arc

    -----------------------------------------------------------------------------
*/
inline double HVE2DArc::CalculateSweep() const
    {
    return (m_RotationDirection == HGFAngle::CCW ?
            ((m_EndPoint - m_Center).CalculateBearing() -
             (m_StartPoint - m_Center).CalculateBearing()) :
            -((m_StartPoint - m_Center).CalculateBearing() -
              (m_EndPoint - m_Center).CalculateBearing()));
    }


//-----------------------------------------------------------------------------
// IsNull
// Indicates if the arc is null (no length)
//-----------------------------------------------------------------------------
inline bool HVE2DArc::IsNull() const
    {
    return(m_StartPoint.IsEqualTo(m_EndPoint) && HDOUBLE_EQUAL(CalculateLength(), 0.0));
    }

//-----------------------------------------------------------------------------
// AdjustStartPointTo
// INCOMPLETE ?????
// Adjust the start point to an more exact point located less than
// an epsilon from previous point
//-----------------------------------------------------------------------------
inline void HVE2DArc::AdjustStartPointTo(const HGF2DLocation& pi_rPoint)
    {
    HPRECONDITION(m_StartPoint.IsEqualTo(pi_rPoint, GetTolerance()));

    m_StartPoint.Set(pi_rPoint);
    }

//-----------------------------------------------------------------------------
// AdjustEndPointTo
// INCOMPLETE ?????
// Adjust the end point to an more exact point located less than
// an epsilon from previous point
//-----------------------------------------------------------------------------
inline void HVE2DArc::AdjustEndPointTo(const HGF2DLocation& pi_rPoint)
    {
    HPRECONDITION(m_EndPoint.IsEqualTo(pi_rPoint, GetTolerance()));

    m_EndPoint.Set(pi_rPoint);
    }

END_IMAGEPP_NAMESPACE







