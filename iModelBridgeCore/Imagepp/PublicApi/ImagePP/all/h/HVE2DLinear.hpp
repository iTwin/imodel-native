//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DLinear.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    Default constructor for linear.
    The interpretation coordinate system is dynamically allocated.

    -----------------------------------------------------------------------------
*/
inline HVE2DLinear::HVE2DLinear()
    : HVE2DVector()
    {
    // Set extremity points coordinate system to the one used by the graphic object
    m_StartPoint.SetCoordSys(GetCoordSys());
    m_EndPoint.SetCoordSys(GetCoordSys());
    }

/** -----------------------------------------------------------------------------
    Constructor for a linear. The start and end points of the linear are provided.
    The interpretation coordinate system is obtained from the start point.

    @param pi_rStartPoint Constant reference to a HGF2DLocation object from
                          which is taken the start point position and
                          coordinate system.

    @param pi_rEndPoint Constant reference to a HGF2DLocation object from
                        which is taken the end point position.

    Example
    @code
    @end

    -----------------------------------------------------------------------------
*/
inline HVE2DLinear::HVE2DLinear (const HGF2DLocation& pi_rStartPoint,
                                 const HGF2DLocation& pi_rEndPoint)
    : HVE2DVector(pi_rStartPoint.GetCoordSys()),
      m_StartPoint(pi_rStartPoint),
      m_EndPoint(pi_rEndPoint, pi_rStartPoint.GetCoordSys())
    {
    }

/** -----------------------------------------------------------------------------
    Constructor for a linear. The start and end points of the linear are provided.
    The interpretation coordinate system is the one provided.

    @param pi_rStartPoint Constant reference to a HGF2DPosition object from
                          which is taken the start point position.

    @param pi_rEndPoint Constant reference to a HGF2DPosition object from
                        which is taken the end point position.

    @param pi_rpCoordSys Constant reference to a smart pointer to coordinate
                         system that will be used in the interpretation
                         of the linear.

    Example
    @code
    @end

    -----------------------------------------------------------------------------
*/
inline HVE2DLinear::HVE2DLinear (const HGF2DPosition& pi_rStartPoint,
                                 const HGF2DPosition& pi_rEndPoint,
                                 const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HVE2DVector(pi_rpCoordSys),
      m_StartPoint(pi_rStartPoint, pi_rpCoordSys),
      m_EndPoint(pi_rEndPoint, pi_rpCoordSys)
    {
    }

/** -----------------------------------------------------------------------------
    Constructor for a linear. The start and end points of the linear are
    automatically set to the origin of the coordinate system (0, 0).
    The interpretation coordinate system is the one provided.

    @param pi_rpCoordSys Constant reference to a smart pointer to coordinate
                         system that will be used in the interpretation
                         of the linear.

    Example
    @code
    @end

    -----------------------------------------------------------------------------
*/
inline HVE2DLinear::HVE2DLinear(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HVE2DVector(pi_rpCoordSys),
      m_StartPoint(pi_rpCoordSys),
      m_EndPoint(pi_rpCoordSys)
    {
    }

//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HVE2DLinear object.
//-----------------------------------------------------------------------------
inline HVE2DLinear::HVE2DLinear(const HVE2DLinear& pi_rObj)
    : HVE2DVector(pi_rObj),
      m_StartPoint(pi_rObj.m_StartPoint),
      m_EndPoint(pi_rObj.m_EndPoint)
    {
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HVE2DLinear::~HVE2DLinear()
    {
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another linear object.
//-----------------------------------------------------------------------------
inline HVE2DLinear& HVE2DLinear::operator=(const HVE2DLinear& pi_rObj)
    {
    // Check that object to copy is not self
    if (this != &pi_rObj)
        {
        // Invoque ancester copy
        HVE2DVector::operator=(pi_rObj);

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

    @param pi_rLinear    Constant reference to the HVE2DLinear to determine link
                        with.

    @return true if the linears link and false otherwise.

    Example
    @code
    HFCPtr<HGF2DCoordSys>    pMyWorld(new HGF2DCoordSys());
    HGF2DLocation            MyFirstPoint(10.0, 10.0, pMyWorld);
    HGF2DLocation            MySecondPoint(15, 16, pMyWorld);
    HVE2DSegment             MySeg1(MyFirstPoint, MySecondPoint);
    ...
    HGF2DLocation            MyThirdPoint(12, 13, pMyWorld);
    HGF2Dlocation            MyFourthPoint(12, -3, pMyWorld);
    HVE2DSegment             MySeg2(MyThirdPoint, MyFourthPoint);

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
inline bool   HVE2DLinear::LinksTo(const HVE2DLinear& pi_rLinear) const
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
    This method returns the start point. This returned HGF2DLocation
    is always expressed in the same coordinate system as the linear.

    @return The start point of the linear expressed in the coordinate
            system of the linear.

    Example
    @code
    HFCPtr<HGF2DCoordSys>    pMyWorld(new HGF2DCoordSys());
    HGF2DLocation            MyFirstPoint(10.0, 10.0, pMyWorld);
    HGF2DLocation            MySecondPoint(15, 16, pMyWorld);
    HVE2DSegment             MySeg1(MyFirstPoint, MySecondPoint);
    ...
    HGF2DLocation TheStartPoint = MySeg1.GetStartPoint();
    @end

    @see GetEndPoint()
    @see HGF2DLocation
    -----------------------------------------------------------------------------
*/
inline const HGF2DLocation& HVE2DLinear::GetStartPoint() const
    {
    return (m_StartPoint);
    }

/** -----------------------------------------------------------------------------
    This method returns the end point. This returned HGF2DLocation is always
    expressed in the same coordinate system as the linear.

    @return The end point of the linear expressed in the coordinate
            system of the linear.

    Example
    @code
    HFCPtr<HGF2DCoordSys>    pMyWorld(new HGF2DCoordSys());
    HGF2DLocation            MyFirstPoint(10.0, 10.0, pMyWorld);
    HGF2DLocation            MySecondPoint(15, 16, pMyWorld);
    HVE2DSegment             MySeg1(MyFirstPoint, MySecondPoint);
    ...                      
    HGF2DLocation TheStartPoint = MySeg1.GetEndPoint();
    @end

    @see GetStartPoint()
    @see HGF2DLocation
    -----------------------------------------------------------------------------
*/
inline const HGF2DLocation& HVE2DLinear::GetEndPoint() const
    {
    return (m_EndPoint);
    }

//-----------------------------------------------------------------------------
// Move
// Moves the linear by specified displacement
//-----------------------------------------------------------------------------
inline void    HVE2DLinear::Move(const HGF2DDisplacement& pi_rDisplacement)
    {
    m_StartPoint += pi_rDisplacement;
    m_EndPoint += pi_rDisplacement;
    }



//-----------------------------------------------------------------------------
// Scale
// Scales the linear by specified scale around specified location.
//-----------------------------------------------------------------------------
inline void HVE2DLinear::Scale(double pi_ScaleFactor,
                               const HGF2DLocation& pi_rScaleOrigin)
    {
    // The given scale factor may not be equal to 0.0
    HPRECONDITION(pi_ScaleFactor != 0.0);

    // Obtain the scale origin in the current coordinate system
    HGF2DLocation   MyScaleOrigin(pi_rScaleOrigin, GetCoordSys());

    m_StartPoint = MyScaleOrigin + (pi_ScaleFactor * (m_StartPoint - MyScaleOrigin));
    m_EndPoint = MyScaleOrigin + (pi_ScaleFactor * (m_EndPoint - MyScaleOrigin));
    }


/** -----------------------------------------------------------------------------
    This method checks if the self linear connects on the given vector
    by one of its extremity. The method does not check if the
    linear crosses at other points.

    @param pi_rVector    Constant reference to the HVE2DVector to
                        determine connection to.

    @return true if the linear connects and false otherwise.

    Example
    @code
    HFCPtr<HGF2DCoordSys>    pMyWorld(new HGF2DCoordSys());
    HGF2DLocation            MyFirstPoint(10.0, 10.0, pMyWorld);
    HGF2DLocation            MySecondPoint(15, 16, pMyWorld);
    HVE2DSegment             MySeg1(MyFirstPoint, MySecondPoint);
    ...
    HGF2DLocation            MyThirdPoint(12, 13, pMyWorld);
    HGF2Dlocation            MyFourthPoint(12, -3, pMyWorld);
    HVE2DSegment             MySeg2(MyThirdPoint, MyFourthPoint);

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
inline bool HVE2DLinear::ConnectsTo(const HVE2DVector& pi_rVector) const
    {
    bool   Answer;

    // Obtain tolerance
    double Tolerance = MIN(GetTolerance(), pi_rVector.GetTolerance());

    if (GetCoordSys() == pi_rVector.GetCoordSys())
        Answer = (pi_rVector.IsPointOnSCS(m_StartPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance) ||
                  pi_rVector.IsPointOnSCS(m_EndPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance));
    else
        Answer = (pi_rVector.IsPointOn(m_StartPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance) ||
                  pi_rVector.IsPointOn(m_EndPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance));

    return(Answer);
    }

/** -----------------------------------------------------------------------------
    IsAtAnExtremity
    -----------------------------------------------------------------------------
*/
inline bool HVE2DLinear::IsAtAnExtremity(const HGF2DLocation& pi_rLocation,
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
    HFCPtr<HGF2DCoordSys>    pMyWorld(new HGF2DCoordSys());
    HGF2DLocation            MyFirstPoint(10.0, 10.0, pMyWorld);
    HGF2DLocation            MySecondPoint(15, 16, pMyWorld);
    HVE2DSegment             MySeg1(MyFirstPoint, MySecondPoint);
    ...
    MySeg1.Reverse();
    @end

    @see GetStartPoint()
    @see GetEndPoint()
    -----------------------------------------------------------------------------
*/
inline void HVE2DLinear::Reverse()
    {
    // Swap start and end point
    HGF2DLocation   DummyPoint(m_StartPoint);
    m_StartPoint = m_EndPoint;
    m_EndPoint = DummyPoint;
    }

//-----------------------------------------------------------------------------
// GetMainVectorType
// Returns the vector type
//-----------------------------------------------------------------------------
inline HVE2DVectorTypeId HVE2DLinear::GetMainVectorType() const
    {
    return (HVE2DLinear::CLASS_ID);
    }


END_IMAGEPP_NAMESPACE
