//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DLinear.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "HGF2DDisplacement.h"

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    Default constructor for linear.
    The interpretation coordinate system is dynamically allocated.

    -----------------------------------------------------------------------------
*/
inline HGF2DLinear::HGF2DLinear()
    : HGF2DVector()
    {
    }

/** -----------------------------------------------------------------------------
    Constructor for a linear. The start and end points of the linear are provided.
    The interpretation coordinate system is obtained from the start point.

    @param pi_rStartPoint Constant reference to a HGF2DPosition object from
                          which is taken the start point position and
                          coordinate system.

    @param pi_rEndPoint Constant reference to a HGF2DPosition object from
                        which is taken the end point position.

    Example
    @code
    @end

    -----------------------------------------------------------------------------
*/
inline HGF2DLinear::HGF2DLinear (const HGF2DPosition& pi_rStartPoint,
                              const HGF2DPosition& pi_rEndPoint)
    : HGF2DVector(),
      m_StartPoint(pi_rStartPoint),
      m_EndPoint(pi_rEndPoint)
    {
    }



//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HGF2DLinear object.
//-----------------------------------------------------------------------------
inline HGF2DLinear::HGF2DLinear(const HGF2DLinear& pi_rObj)
    : HGF2DVector(pi_rObj),
      m_StartPoint(pi_rObj.m_StartPoint),
      m_EndPoint(pi_rObj.m_EndPoint)
    {
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HGF2DLinear::~HGF2DLinear()
    {
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another linear object.
//-----------------------------------------------------------------------------
inline HGF2DLinear& HGF2DLinear::operator=(const HGF2DLinear& pi_rObj)
    {
    // Check that object to copy is not self
    if (this != &pi_rObj)
        {
        // Invoque ancester copy
        HGF2DVector::operator=(pi_rObj);

        // Copy attributes
        m_StartPoint=pi_rObj.m_StartPoint;
        m_EndPoint=pi_rObj.m_EndPoint;
        }

    // Return reference to self
    return (*this);
    }

/** -----------------------------------------------------------------------------
    This method checks if the two linear objects connect at their
    extremity point or not. To be linked, two linear must share one
    extremity point. The method does not check if the linear
    segment crosses at other points.

    @param pi_rLinear    Constant reference to the HGF2DLinear to determine link
                        with.

    @return true if the linears link and false otherwise.

    Example
    @code
    HGF2DPosition        MyFirstPoint(10, 10);
    HGF2DPosition        MySecondPoint(15, 16);
    HVE2DSegment         MySeg1(MyFirstPoint, MySecondPoint);
    ...
    HGF2DPosition        MyThirdPoint(12, 13);
    HGF2Dlocation        MyFourthPoint(12, -3);
    HVE2DSegment         MySeg2(MyThirdPoint, MyFourthPoint);

    if (MySeg1.LinksTo(MySeg2))
    {
        ...
    }
    @end

    @see GetStartPoint()
    @see GetEndPoint()
    @see ConnectsTo()
    -----------------------------------------------------------------------------
*/
inline bool   HGF2DLinear::LinksTo(const HGF2DLinear& pi_rLinear) const
    {
    // For two linear to connect,
    // they must have one extremity point equal to either extremity
    // point of the other
    return ((m_EndPoint.IsEqualTo(pi_rLinear.m_StartPoint, GetTolerance())) ||
            (m_EndPoint.IsEqualTo(pi_rLinear.m_EndPoint, GetTolerance())) ||
            (m_StartPoint.IsEqualTo(pi_rLinear.m_StartPoint, GetTolerance())) ||
            (m_StartPoint.IsEqualTo(pi_rLinear.m_EndPoint, GetTolerance())));
    }


/** -----------------------------------------------------------------------------
    This method returns the start point. This returned HGF2DPosition
    is always expressed in the same coordinate system as the linear.

    @return The start point of the linear expressed in the coordinate
            system of the linear.

    Example
    @code
    HGF2DPosition        MyFirstPoint(10, 10);
    HGF2DPosition        MySecondPoint(15, 16);
    // A segment is a kind of linear
    HGF2DSegment        MySeg1(MyFirstPoint, MySecondPoint);
    ...
    HGF2DPosition TheStartPoint = MySeg1.GetStartPoint();
    @end

    @see GetEndPoint()
    @see HGF2DPosition
    -----------------------------------------------------------------------------
*/
inline const HGF2DPosition& HGF2DLinear::GetStartPoint() const
    {
    return (m_StartPoint);
    }

/** -----------------------------------------------------------------------------
    This method returns the end point. This returned HGF2DPosition is always
    expressed in the same coordinate system as the linear.

    @return The end point of the linear expressed in the coordinate
            system of the linear.

    Example
    @code
    HGF2DPosition        MyFirstPoint(10, 10);
    HGF2DPosition        MySecondPoint(15, 16);
    // A segment is a kind of linear
    HGF2DSegment        MySeg1(MyFirstPoint, MySecondPoint);
    ...
    HGF2DPosition TheStartPoint = MySeg1.GetEndPoint();
    @end

    @see GetStartPoint()
    @see HGF2DPosition
    -----------------------------------------------------------------------------
*/
inline const HGF2DPosition& HGF2DLinear::GetEndPoint() const
    {
    return (m_EndPoint);
    }

//-----------------------------------------------------------------------------
// Move
// Moves the linear by specified displacement
//-----------------------------------------------------------------------------
inline void    HGF2DLinear::Move(const HGF2DDisplacement& pi_rDisplacement)
    {
    m_StartPoint += pi_rDisplacement;
    m_EndPoint += pi_rDisplacement;
    }



//-----------------------------------------------------------------------------
// Scale
// Scales the linear by specified scale around specified location.
//-----------------------------------------------------------------------------
inline void HGF2DLinear::Scale(double pi_ScaleFactor,
                             const HGF2DPosition& pi_rScaleOrigin)
    {
    // The given scale factor may not be equal to 0.0
    HPRECONDITION(pi_ScaleFactor != 0.0);

    m_StartPoint = pi_rScaleOrigin + (pi_ScaleFactor * (m_StartPoint - pi_rScaleOrigin));
    m_EndPoint = pi_rScaleOrigin + (pi_ScaleFactor * (m_EndPoint - pi_rScaleOrigin));
    }


/** -----------------------------------------------------------------------------
    This method checks if the self linear connects on the given vector
    by one of its extremity. The method does not check if the
    linear crosses at other points.

    @param pi_rVector    Constant reference to the HGF2DVector to
                        determine connection to.

    @return true if the linear connects and false otherwise.

    Example
    @code
    HGF2DPosition        MyFirstPoint(10, 10);
    HGF2DPosition        MySecondPoint(15, 16);
    // A segment is a kind of linear
    HGF2DSegment        MySeg1(MyFirstPoint, MySecondPoint);

    HGF2DPosition       MyThirdPoint(12, 13);
    HGF2Dlocation       MyFourthPoint(12, -3);
    HVE2DSegment        MySeg2(MyThirdPoint, MyFourthPoint);

    if (MySeg1.ConnectsTo(MySeg2))
    {
        ...
    }
    @end

    @see GetStartPoint()
    @see GetEndPoint()
    @see LinksTo()
    -----------------------------------------------------------------------------
*/
inline bool HGF2DLinear::ConnectsTo(const HGF2DVector& pi_rVector) const
    {
    bool   Answer;

    // Obtain tolerance
    double Tolerance = MIN(GetTolerance(), pi_rVector.GetTolerance());

    Answer = (pi_rVector.IsPointOn(m_StartPoint, HGF2DVector::INCLUDE_EXTREMITIES, Tolerance) ||
              pi_rVector.IsPointOn(m_EndPoint, HGF2DVector::INCLUDE_EXTREMITIES, Tolerance));

    return(Answer);
    }

/** -----------------------------------------------------------------------------
    IsAtAnExtremity
    -----------------------------------------------------------------------------
*/
inline bool HGF2DLinear::IsAtAnExtremity(const HGF2DPosition& pi_rLocation,
                                       double pi_Tolerance) const
    {
    // Obtain tolerance
    double Tolerance = pi_Tolerance;
    if (Tolerance < 0.0)
        Tolerance = GetTolerance();

    return (pi_rLocation.IsEqualTo(m_StartPoint, Tolerance) ||
            pi_rLocation.IsEqualTo(m_EndPoint, Tolerance));
    }

/** -----------------------------------------------------------------------------
    Reverses the direction followed by the linear. The start point becomes the
    end point, and the end point becomes the start point. The path followed in
    space by the line is not changed.

    This virtual method must be overriden but the present method called
    to perform reversal of start and end point.

    Example
    @code
    HGF2DPosition        MyFirstPoint(10, 10);
    HGF2DPosition        MySecondPoint(15, 16);
    // A segment is a kind of linear
    HGF2DSegment        MySeg1(MyFirstPoint, MySecondPoint);
    ...
    MySeg1.Reverse();
    @end

    @see GetStartPoint()
    @see GetEndPoint()
    -----------------------------------------------------------------------------
*/
inline void HGF2DLinear::Reverse()
    {
    // Swap start and end point
    HGF2DPosition   DummyPoint(m_StartPoint);
    m_StartPoint = m_EndPoint;
    m_EndPoint = DummyPoint;
    }

//-----------------------------------------------------------------------------
// GetMainVectorType
// Returns the vector type
//-----------------------------------------------------------------------------
inline HGF2DVectorTypeId HGF2DLinear::GetMainVectorType() const
    {
    return (HGF2DLinear::CLASS_ID);
    }

END_IMAGEPP_NAMESPACE