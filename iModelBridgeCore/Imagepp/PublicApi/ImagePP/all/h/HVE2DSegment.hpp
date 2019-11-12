//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------


BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
inline HVE2DSegment::HVE2DSegment()
    : HVE2DBasicLinear()
    {
    m_VolatilePeer = true;
    }

//-----------------------------------------------------------------------------
// Constructor (by two points)
//-----------------------------------------------------------------------------
inline HVE2DSegment::HVE2DSegment(const HGF2DLocation& pi_rStartPoint,
                                  const HGF2DLocation& pi_rEndPoint)
    : HVE2DBasicLinear(pi_rStartPoint, pi_rEndPoint)
    {
    m_VolatilePeer = true;

    // Reset tolerance
    ResetTolerance();
    }

//-----------------------------------------------------------------------------
// Constructor (by two positions)
//-----------------------------------------------------------------------------
inline HVE2DSegment::HVE2DSegment(const HGF2DPosition& pi_rStartPoint,
                                  const HGF2DPosition& pi_rEndPoint,
                                  const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HVE2DBasicLinear(pi_rStartPoint, pi_rEndPoint, pi_rpCoordSys)
    {
    m_VolatilePeer = true;

    // Reset tolerance
    ResetTolerance();
    }


//-----------------------------------------------------------------------------
// Constructor (by start point plus displacement)
//-----------------------------------------------------------------------------
inline HVE2DSegment::HVE2DSegment(const HGF2DLocation& pi_rStartPoint,
                                  const HGF2DDisplacement& pi_rDisplacement)
    : HVE2DBasicLinear(pi_rStartPoint, pi_rStartPoint + pi_rDisplacement)
    {

    m_VolatilePeer = true;

    // Reset tolerance
    ResetTolerance();
    }

//-----------------------------------------------------------------------------
// Constructor (setting of coordinate system only)
//-----------------------------------------------------------------------------
inline HVE2DSegment::HVE2DSegment(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HVE2DBasicLinear(pi_rpCoordSys)
    {
    m_VolatilePeer = true;

    // Default tolerance is used
    }

//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HVE2DSegment object.
//-----------------------------------------------------------------------------
inline HVE2DSegment::HVE2DSegment(const HVE2DSegment& pi_rObj)
    : HVE2DBasicLinear(pi_rObj)
    {
    m_VolatilePeer = true;
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HVE2DSegment::~HVE2DSegment()
    {
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another line object.
//-----------------------------------------------------------------------------
inline HVE2DSegment& HVE2DSegment::operator=(const HVE2DSegment& pi_rObj)
    {
    // Invoque the ancester operator
    HVE2DBasicLinear::operator=(pi_rObj);

    // clear peer
    ClearPeer();
    
    // Return reference to self
    return (*this);
    }


//-----------------------------------------------------------------------------
// GetBasicLinearType
// Return the basic linear type
//-----------------------------------------------------------------------------
inline HVE2DBasicLinearTypeId HVE2DSegment::GetBasicLinearType() const
    {
    return (HVE2DSegment::CLASS_ID);
    }

/** -----------------------------------------------------------------------------
    This method sets the start point of the segment. If the
    coordinates are expressed in a different coordinate system than
    the one used by the segment, conversion will occur.

    @param pi_rNewStartPoint Constant reference to the HGF2DLocation object
                             containing the definition of the new first point
                             of segment..

    Example
    @code
    HFCPtr<HGF2DCoordSys>    pMyWorld(new HGF2DCoordSys());
    HVE2DSegment            MySeg1(pMyWorld);
    HGF2DLocation            MyFirstPoint(10, 10,  pMyWorld);
    MySeg1.SetStartPoint(MyFirstPoint);
    @end

    @see GetStartPoint()
    @see SetEndPoint()
    @see SetRawStartPoint()
    @see HGF2DLocation
    @see HGF2DCoordSys
    -----------------------------------------------------------------------------
*/
inline void HVE2DSegment::SetStartPoint(const HGF2DLocation& pi_rNewStartPoint)
    {
    // Set the start point to given expressed in the segment coordinate
    // system
    SetLinearStartPoint(pi_rNewStartPoint);

    // Reset tolerance
    ResetTolerance();
    
    // clear peer
    ClearPeer();
    }

/** -----------------------------------------------------------------------------
    This method sets the end point of the segment. If the
    coordinates are expressed in a different coordinate system than
    the one used by the segment, conversion will occur.

    @param pi_rNewEndPoint Constant reference to the HGF2DLocation object
                           containing the definition of the new end point
                           of segment.

    Example
    @code
    HFCPtr<HGF2DCoordSys>    pMyWorld(new HGF2DCoordSys());
    HVE2DSegment             MySeg1(pMyWorld);
    HGF2DLocation            MyEndPoint(10, 10, pMyWorld);
    ...
    MySeg1.SetEndPoint(MyEndPoint);
    @end

    @see GetEndPoint()
    @see SetStartPoint()
    @see SetRawEndPoint()
    @see HGF2DLocation
    @see HGF2DCoordSys
    -----------------------------------------------------------------------------
*/
inline void HVE2DSegment::SetEndPoint(const HGF2DLocation& pi_rNewEndPoint)
    {
    // Set the end point to given expressed in the segment coordinate
    // system
    SetLinearEndPoint(pi_rNewEndPoint);

    // Reset tolerance
    ResetTolerance();
    
    // clear peer
    ClearPeer();
    }

/** -----------------------------------------------------------------------------
    This method sets the start point of the segment by specification
    of raw coordinate values. These values are interpreted in the
    coordinate system of the segment, in the coordinate units of
    the X and Y dimension of this coordinate system.

    @param pi_X Value of the X dimension of the newly specified
                start point.

    @param pi_Y Value of the Y dimension of the newly specified
                start point.

    Example
    @code
    HFCPtr<HGF2DCoordSys>    pMyWorld(new HGF2DCoordSys());
    HGF2DSegment             MySeg1(pMyWorld);
    ...
    MySeg1.SetRawStartPoint(10.0, 10.0);
    @end

    @see SetStartPoint()
    @see GetStartPoint()
    @see SetRawEndPoint()
    @see HGF2DCoordSys
    -----------------------------------------------------------------------------
*/
inline void HVE2DSegment::SetRawStartPoint(double pi_X, double pi_Y)
    {
    SetStartPoint(HGF2DLocation(pi_X, pi_Y, GetCoordSys()));

    // Reset tolerance
    ResetTolerance();
    
    // clear peer
    ClearPeer();
    }

/** -----------------------------------------------------------------------------
    This method sets the end point of the segment by specification
    of raw coordinate values. These values are interpreted in the
    coordinate system of the segment, in the coordinate units of
    the X and Y dimension of this coordinate system.

    @param pi_X Value of the X dimension of the newly specified
                end point.

    @param pi_Y Value of the Y dimension of the newly specified
                end point.

    Example
    @code
    HFCPtr<HGF2DCoordSys>    pMyWorld(new HGF2DCoordSys());
    HGF2DSegment             MySeg1(pMyWorld);
    ...
    MySeg1.SetRawEndPoint(10.0, 10.0);
    @end

    @see SetEndPoint()
    @see GetEndPoint()
    @see SetRawStartPoint()
    @see HGF2DCoordSys
    -----------------------------------------------------------------------------
*/
inline void HVE2DSegment::SetRawEndPoint(double pi_X, double pi_Y)
    {
    SetEndPoint(HGF2DLocation(pi_X, pi_Y, GetCoordSys()));

    // Reset tolerance
    ResetTolerance();
    
    // clear peer
    ClearPeer();
    }


/** -----------------------------------------------------------------------------
    This method calculates and returns the line, the segment is a part
    of. The interpretation coordinate system of the line is the same
    as the one of the segment.

    @return The line the segment is a part of.

    Example
    @code
    @end

    @see HGF2DLine
    -----------------------------------------------------------------------------
*/
inline HGF2DLine HVE2DSegment::CalculateLine() const
    {
    return (HGF2DLine(GetStartPoint(), GetEndPoint()));
    }

/** -----------------------------------------------------------------------------
    This method returns true if the segment and the line are parallel one to
    the other.

    @param pi_rLine    Constant reference to the line to check if it
                    is parallel to self.

    @return true if segment is parallel to line; false otherwise.

    Example
    @code
    @end

    @see HGF2DLine
    -----------------------------------------------------------------------------
*/
inline bool HVE2DSegment::IsParallelTo(const HGF2DLine& pi_rLine) const
    {
    // Check if given line and the line the segment belongs are parallel
    return (pi_rLine.IsParallelTo(HGF2DLine(GetStartPoint(), GetEndPoint())));
    }

/** -----------------------------------------------------------------------------
    This method returns true if the segments are parallel one to
    the other.

    @param pi_rSegment Constant reference to the segment to check if
                       it is parallel to self

    @return true if segments are parallel; false otherwise.

    Example
    @code
    HFCPtr<HGF2DCoordSys>    pMyWorld(new HGF2DCoordSys());
    HGF2DLocation            MyFirstPoint(10, 10, pMyWorld);
    HGF2DLocation            MySecondPoint(15, 16, pMyWorld);
    HVE2DSegment             MySeg1(MyFirstPoint, MySecondPoint);
    ...
    HGF2DLocation            MyThirdPoint(10, 10, pMyWorld);
    HGF2DLocation            MyFourthPoint(15, 16, pMyWorld);
    HVE2DSegment             MySeg2(MyThrirdPoint, MyFourthPoint);

    bool AreTheyParallel = MySeg1.IsParallelTo(MySeg2);
    @end

    @see HGF2DLine
    -----------------------------------------------------------------------------
*/
inline bool HVE2DSegment::IsParallelTo(const HVE2DSegment& pi_rSegment) const
    {
    // Check if the lines the segments belong to are parallel
    return (HGF2DLine(GetStartPoint(),
                      GetEndPoint()).IsParallelTo(HGF2DLine(pi_rSegment.GetStartPoint(),
                                                         pi_rSegment.GetEndPoint())));
    }

//-----------------------------------------------------------------------------
// CalculateBearing
// Calculates and returns the bearing of segment at specified point
//-----------------------------------------------------------------------------
inline HGFBearing HVE2DSegment::CalculateBearing(const HGF2DLocation& pi_rPoint,
                                                 HVE2DVector::ArbitraryDirection pi_Direction) const
    {
    // The segment must not be NULL
    HPRECONDITION(GetStartPoint() != GetEndPoint());

    // The point must be located on segment
    HPRECONDITION(IsPointOn(pi_rPoint));

    // The bearing on a segment is constant whatever the given point
    return (pi_Direction == HVE2DVector::BETA ?
            (GetEndPoint() - GetStartPoint()).CalculateBearing() :
            (GetStartPoint() - GetEndPoint()).CalculateBearing());
    }

//-----------------------------------------------------------------------------
// CalculateAngularAcceleration
// Calculates and returns the angular acceleration of segment at specified point
//-----------------------------------------------------------------------------
inline double HVE2DSegment::CalculateAngularAcceleration(const HGF2DLocation& pi_rPoint,
                                                                         HVE2DVector::ArbitraryDirection pi_Direction) const
    {
    // The point must be located on segment
    HPRECONDITION(IsPointOn(pi_rPoint));

    // The acceleration on a segment is always 0.0
    return 0.0;
    }


//-----------------------------------------------------------------------------
// CalculateRayArea
// Calculates and returns the area of the ray extended from given point to
// the given linear
//-----------------------------------------------------------------------------
inline double HVE2DSegment::CalculateRayArea(const HGF2DLocation& pi_rPoint) const
    {
    return ((GetStartPoint().GetX() * ((GetEndPoint() - pi_rPoint).GetDeltaY())) / 2);
    }

//-----------------------------------------------------------------------------
// CalculateRelativePoint
// Calculates and returns the location based on the given relative position.
//-----------------------------------------------------------------------------
inline HGF2DLocation HVE2DSegment::CalculateRelativePoint(double pi_RelativePos) const
    {
    // The relative position must be between 0.0 and 1.0
    HPRECONDITION((pi_RelativePos >= 0.0) && (pi_RelativePos <= 1.0));

    return (GetStartPoint() + (pi_RelativePos * (GetEndPoint() - GetStartPoint())));
    }

//-----------------------------------------------------------------------------
// CalculateRelativePosition
// Calculates and returns the reltive position of given location on segment.
//-----------------------------------------------------------------------------
inline double HVE2DSegment::CalculateRelativePosition(const HGF2DLocation& pi_rPointOnLinear) const
    {
    // The given point must be located on segment
    HPRECONDITION(IsPointOn(pi_rPointOnLinear));

    // The segment must not be null
    HPRECONDITION(GetStartPoint() != GetEndPoint());

    return GetSegmentPeer().CalculateRelativePosition(pi_rPointOnLinear.ExpressedIn(GetCoordSys()).GetPosition());
    }

//-----------------------------------------------------------------------------
// Shorten
// Shortens the segment definition by specification of relative positions to self.
//-----------------------------------------------------------------------------
inline void HVE2DSegment::Shorten(double pi_StartRelativePos, double pi_EndRelativePos)
    {
    // The relative positions given must be between 0.0 and 1.0
    HPRECONDITION((pi_StartRelativePos >= 0.0) && (pi_StartRelativePos <= 1.0));
    HPRECONDITION((pi_EndRelativePos >= 0.0) && (pi_EndRelativePos <= 1.0));

    // The given end relative position must be greater than the start
    // relative position
    HPRECONDITION(pi_StartRelativePos <= pi_EndRelativePos);

    double     DeltaX(GetEndPoint().GetX() - GetStartPoint().GetX());
    double     DeltaY(GetEndPoint().GetY() - GetStartPoint().GetY());

    SetEndPoint(HGF2DLocation(GetStartPoint().GetX() + (DeltaX * pi_EndRelativePos),  GetStartPoint().GetY() + (DeltaY * pi_EndRelativePos), GetCoordSys()));
    SetStartPoint(HGF2DLocation(GetStartPoint().GetX() + (DeltaX * pi_StartRelativePos), GetStartPoint().GetY() + (DeltaY * pi_StartRelativePos), GetCoordSys()));

    // Reset tolerance
    ResetTolerance();
    
    // clear peer
    ClearPeer();
    }


//-----------------------------------------------------------------------------
// Shorten
// Shortens the segment definition by specification of new start and end points
//-----------------------------------------------------------------------------
inline void HVE2DSegment::Shorten(const HGF2DLocation& pi_rNewStartPoint,
                                  const HGF2DLocation& pi_rNewEndPoint)
    {
    // The shorten points must be located on segment
    HPRECONDITION(IsPointOn(pi_rNewStartPoint) && IsPointOn(pi_rNewEndPoint));

    // The relative position of given start point must be smaller than
    // the relative position of given end point
    HPRECONDITION((HDOUBLE_EQUAL(CalculateLength(), 0.0, GetTolerance())) ||
                  (HDOUBLE_SMALLER_OR_EQUAL_EPSILON(CalculateRelativePosition(pi_rNewStartPoint), CalculateRelativePosition(pi_rNewEndPoint))));

    // Set extremity points while keeping expression in the segment coordinate system
    SetEndPoint(pi_rNewEndPoint);
    SetStartPoint(pi_rNewStartPoint);

    // Reset tolerance
    ResetTolerance();
    
    // clear peer
    ClearPeer();
    }


//-----------------------------------------------------------------------------
// ShortenTo
// Shortens the segment definition by specification of a new end point
// In the case of a segment this is identical to setting a new end point with
// SetEndPoint(), except that it is required that new point be located ON
// segment
//-----------------------------------------------------------------------------
inline void HVE2DSegment::ShortenTo(const HGF2DLocation& pi_rNewEndPoint)
    {
    // The given point must be located on segment
    HPRECONDITION(IsPointOn(pi_rNewEndPoint));

    // The end point is set while keeping interpretation coordinate system
    // to the one of the segment
    SetEndPoint(pi_rNewEndPoint);

    // Reset tolerance
    ResetTolerance();
    
    // clear peer
    ClearPeer();
    }

//-----------------------------------------------------------------------------
// ShortenTo
// Shortens the segment definition by specification of a new end relative point
//-----------------------------------------------------------------------------
inline void HVE2DSegment::ShortenTo(double pi_EndRelativePos)
    {
    // The given relative position must be greater than 0.0 and smaller (or equal) than 1.0
    HPRECONDITION((pi_EndRelativePos >= 0.0) && (pi_EndRelativePos <= 1.0));

    // Change the segment which will go from current start point to
    // described end point
    SetEndPoint(HGF2DLocation(GetStartPoint().GetX() +
                               (pi_EndRelativePos * (GetEndPoint().GetX() - GetStartPoint().GetX())),
                              GetStartPoint().GetY() +
                               (pi_EndRelativePos * (GetEndPoint().GetY() - GetStartPoint().GetY())),
                            GetCoordSys()));

    // Reset tolerance
    ResetTolerance();
    
    // clear peer
    ClearPeer();
    }


//-----------------------------------------------------------------------------
// ShortenFrom
// Shortens the segment definition by specification of a new start point
// In the case of a segment this is identical to setting a new start point with
// SetStartPoint(), except that it is required that new point be located ON
// segment
//-----------------------------------------------------------------------------
inline void HVE2DSegment::ShortenFrom(const HGF2DLocation& pi_rNewStartPoint)
    {
    // The given point must be located on segment
    HPRECONDITION(IsPointOn(pi_rNewStartPoint));

    // The start point is set while keeping interpretation coordinate system
    // to the one of the segment
    SetStartPoint(pi_rNewStartPoint);

    // Reset tolerance
    ResetTolerance();
    
    // clear peer
    ClearPeer();
    }


//-----------------------------------------------------------------------------
// ShortenFrom
// Shortens the segment definition by specification of a new start relative point
//-----------------------------------------------------------------------------
inline void HVE2DSegment::ShortenFrom(double pi_StartRelativePos)
    {
    // The given relative position must be greater (or equal) than 0.0 and smaller than 1.0
    HPRECONDITION((pi_StartRelativePos >= 0.0) && (pi_StartRelativePos <= 1.0));

    // Change the segment which will go from described start point
    SetStartPoint(HGF2DLocation(GetStartPoint().GetX() + (pi_StartRelativePos * (GetEndPoint().GetX() - GetStartPoint().GetX())), 
                                GetStartPoint().GetY() + (pi_StartRelativePos * (GetEndPoint().GetY() - GetStartPoint().GetY())),
                                GetCoordSys()));

    // Reset tolerance
    ResetTolerance();
    
    // clear peer
    ClearPeer();
    }


//-----------------------------------------------------------------------------
// AutoCrosses
// Indicates if the linear crosses itself
// This is of course impossible for a segment
//-----------------------------------------------------------------------------
inline bool HVE2DSegment::AutoCrosses() const
    {
    return(false);
    }


//-----------------------------------------------------------------------------
// GetExtent
// Returns the extent of the segment
//-----------------------------------------------------------------------------
inline HGF2DExtent HVE2DSegment::GetExtent() const
    {
    return(HGF2DExtent(MIN(GetStartPoint().GetX(), GetEndPoint().GetX()),
                       MIN(GetStartPoint().GetY(), GetEndPoint().GetY()),
                       MAX(GetStartPoint().GetX(), GetEndPoint().GetX()),
                       MAX(GetStartPoint().GetY(), GetEndPoint().GetY()),
                       GetCoordSys()));
    }


//-----------------------------------------------------------------------------
// Clone
// Returns a dynamically allocated copy of the segment
//-----------------------------------------------------------------------------
inline HPMPersistentObject* HVE2DSegment::Clone() const
    {
    return(new HVE2DSegment(*this));
    }

//-----------------------------------------------------------------------------
// AreSegmentsTouching
// Indicates if the two segments are touching
//-----------------------------------------------------------------------------
inline bool HVE2DSegment::AreSegmentsTouching(const HVE2DSegment& pi_rSegment) const
    {
    // Check if they share the same coordinate system or they are related
    // through a linearity preserving model
    HPRECONDITION((GetCoordSys() == pi_rSegment.GetCoordSys()) ||
                  (GetCoordSys()->HasLinearityPreservingRelationTo(pi_rSegment.GetCoordSys())));

    // For two segments to be touching, they must first have extents which overlap
    // then one must be connected to the other, or they must cross each other
    return (GetExtent().OutterOverlaps(pi_rSegment.GetExtent(), MIN(GetTolerance(), pi_rSegment.GetTolerance())) &&
            (AreSegmentsCrossing(pi_rSegment) ||
             ConnectsTo(pi_rSegment) ||
             pi_rSegment.ConnectsTo(*this) ||
             AreAdjacent(pi_rSegment)));
    }

//-----------------------------------------------------------------------------
// AreSegmentsCrossing
// Indicates if the two segments are touching
//-----------------------------------------------------------------------------
inline bool HVE2DSegment::AreSegmentsCrossing(const HVE2DSegment& pi_rSegment) const
    {
    // Check if they share the same coordinate system or they are related
    // through a linearity preserving model
    HPRECONDITION((GetCoordSys() == pi_rSegment.GetCoordSys()) ||
                  (GetCoordSys()->HasLinearityPreservingRelationTo(pi_rSegment.GetCoordSys())));

    HGF2DLocation DummyPoint(GetCoordSys());

    // For two segments to be crossing, they must first have extents which overlap
    // then they must intersect
    return (GetExtent().InnerOverlaps(pi_rSegment.GetExtent(), MIN(GetTolerance(), pi_rSegment.GetTolerance())) &&
            (IntersectSegment(pi_rSegment, &DummyPoint) == HVE2DSegment::CROSS_FOUND));
    }


//-----------------------------------------------------------------------------
// AreSegmentsCrossingSCS
// Indicates if the two segments are touching
//-----------------------------------------------------------------------------
inline bool HVE2DSegment::AreSegmentsCrossingSCS(const HVE2DSegment& pi_rSegment) const
    {
    HPRECONDITION(GetCoordSys() == pi_rSegment.GetCoordSys());

    HGF2DLocation DummyPoint(GetCoordSys());

    // For two segments to be crossing, they must first have extents which overlap
    // then they must intersect
    return (GetExtent().InnerOverlaps(pi_rSegment.GetExtent(), MIN(GetTolerance(), pi_rSegment.GetTolerance())) &&
            (IntersectSegmentSCS(pi_rSegment, &DummyPoint) == HVE2DSegment::CROSS_FOUND));
    }



//-----------------------------------------------------------------------------
// AreSegmentsAdjacent
// Indicates if the two segments are adjacent.
//-----------------------------------------------------------------------------
inline bool HVE2DSegment::AreSegmentsAdjacent(const HVE2DSegment& pi_rSegment) const
    {
    // Check if they share the same coordinate system or they are related
    // through a linearity preserving model
    HPRECONDITION((GetCoordSys() == pi_rSegment.GetCoordSys()) ||
                  (GetCoordSys()->HasLinearityPreservingRelationTo(pi_rSegment.GetCoordSys())));

    // To be adjacent, two segments must first have extents which overlap and
    // share the same line
    // and be connected on each other at any point
    return (GetExtent().OutterOverlaps(pi_rSegment.GetExtent(), MIN(GetTolerance(), pi_rSegment.GetTolerance())) &&
            (HGF2DLine(GetStartPoint(), GetEndPoint()) == HGF2DLine(pi_rSegment.GetStartPoint(), pi_rSegment.GetEndPoint())) &&
            (ConnectsTo(pi_rSegment) || pi_rSegment.ConnectsTo(*this)));
    }

//-----------------------------------------------------------------------------
// AreSegmentsAdjacentSCS
// Indicates if the two segments are adjacent.
//-----------------------------------------------------------------------------
inline bool HVE2DSegment::AreSegmentsAdjacentSCS(const HVE2DSegment& pi_rSegment) const
    {
    HPRECONDITION(GetCoordSys() == pi_rSegment.GetCoordSys());

    // To be adjacent, two segments must first have extents which overlap and
    // share the same line
    // and be connected on each other at any point
    return (GetExtent().OutterOverlaps(pi_rSegment.GetExtent(), MIN(GetTolerance(), pi_rSegment.GetTolerance())) &&
            (HGF2DLine(GetStartPoint(), GetEndPoint()) == HGF2DLine(pi_rSegment.GetStartPoint(), pi_rSegment.GetEndPoint())) &&
            (ConnectsTo(pi_rSegment) || pi_rSegment.ConnectsTo(*this)));
    }


//-----------------------------------------------------------------------------
// AreSegmentsFlirting
// Indicates if the two segments are flirting.
//-----------------------------------------------------------------------------
inline bool HVE2DSegment::AreSegmentsFlirting(const HVE2DSegment& pi_rSegment) const
    {
    // Check if they share the same coordinate system or they are related
    // through a linearity preserving model
    HPRECONDITION((GetCoordSys() == pi_rSegment.GetCoordSys()) ||
                  (GetCoordSys()->HasLinearityPreservingRelationTo(pi_rSegment.GetCoordSys())));

    // For two segments to be flirting, they must first have extents which overlap
    // then one must be connected to the other,
    // and at the same time not be contiguous
    return ((ConnectsTo(pi_rSegment) || pi_rSegment.ConnectsTo(*this)) &&
            (!AreSegmentsContiguous(pi_rSegment)));
    }


//-----------------------------------------------------------------------------
// IsNull
// Indicates if the segment is null (no length)
//-----------------------------------------------------------------------------
inline bool HVE2DSegment::IsNull() const
    {
    return(GetStartPoint().IsEqualToSCS(GetEndPoint(), GetTolerance()));
    }

//-----------------------------------------------------------------------------
// AdjustStartPointTo
// Adjust the start point to an more exact point located less than
// an epsilon from previous point
//-----------------------------------------------------------------------------
inline void HVE2DSegment::AdjustStartPointTo(const HGF2DLocation& pi_rPoint)
    {
    HPRECONDITION(GetStartPoint().IsEqualTo(pi_rPoint, GetTolerance()));

    SetStartPoint(pi_rPoint);

    // clear peer
    ClearPeer();
    
    // Tolerance is not adjusted since change is only minor
    }

//-----------------------------------------------------------------------------
// AdjustEndPointTo
// Adjust the end point to an more exact point located less than
// an epsilon from previous point
//-----------------------------------------------------------------------------
inline void HVE2DSegment::AdjustEndPointTo(const HGF2DLocation& pi_rPoint)
    {
    HPRECONDITION(GetEndPoint().IsEqualTo(pi_rPoint, GetTolerance()));

    SetEndPoint(pi_rPoint);
    
    // clear peer
    ClearPeer();

    // Tolerance is not adjusted since change is only minor
    }


//-----------------------------------------------------------------------------
// Drop
// Returns the description of segments in the form of raw location
// segments
//-----------------------------------------------------------------------------
inline void HVE2DSegment::Drop(HGF2DLocationCollection* po_pPoints,
                               double                   pi_rTolerance,
                               EndPointProcessing       pi_EndPointProcessing) const
    {
    HPRECONDITION(po_pPoints != 0);

    // NOTE : Tolerance is unused since drop is exact

    // Check if end point must be added
    if (pi_EndPointProcessing == HVE2DLinear::INCLUDE_END_POINT)
        {
        // Both start and end point must be added ... allocate 2 spaces in list
        po_pPoints->reserve(2);

        // Add both points
        po_pPoints->push_back(GetStartPoint());
        po_pPoints->push_back(GetEndPoint());
        }
    else
        {
        // Add only the start point
        po_pPoints->push_back(GetStartPoint());
        }

    }

//-----------------------------------------------------------------------------
// ResetTolerance
// PRIVATE method
// Recalculates the tolerance if needed and permitted
//-----------------------------------------------------------------------------
inline void HVE2DSegment::ResetTolerance()
    {
    // Check if auto-tolerance is active
    if (IsAutoToleranceActive())
        {
        // Compute appropriate tolerance (default is global epsilon)
        double Tolerance = HGLOBAL_EPSILON;

        // If coordinates are greater than global tolerance divided by
        // float precision (~= 1E-8 / 1E-15 -> 1E7 ... 1E6 for all cases)
        // Then we use global epsilon * coordinate
        Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(GetStartPoint().GetX()));
        Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(GetStartPoint().GetY()));
        Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(GetEndPoint().GetX()));
        Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(GetEndPoint().GetY()));

        SetTolerance(MIN(HMAX_EPSILON, Tolerance));
        }
    }


END_IMAGEPP_NAMESPACE
