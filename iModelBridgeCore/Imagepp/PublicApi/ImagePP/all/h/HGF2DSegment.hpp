//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DSegment.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "HGF2DLiteLine.h"

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
inline HGF2DSegment::HGF2DSegment()
    : HGF2DBasicLinear()
    {
    }

//-----------------------------------------------------------------------------
// Constructor (by two points)
//-----------------------------------------------------------------------------
inline HGF2DSegment::HGF2DSegment(const HGF2DPosition& pi_rStartPoint,
                                  const HGF2DPosition& pi_rEndPoint)
    : HGF2DBasicLinear(pi_rStartPoint, pi_rEndPoint)
    {
    // Reset tolerance
    ResetTolerance();
    }



//-----------------------------------------------------------------------------
// Constructor (by start point plus displacement)
//-----------------------------------------------------------------------------
inline HGF2DSegment::HGF2DSegment(const HGF2DPosition& pi_rStartPoint,
                              const HGF2DDisplacement& pi_rDisplacement)
    : HGF2DBasicLinear(pi_rStartPoint, pi_rStartPoint + pi_rDisplacement)
    {
    // Reset tolerance
    ResetTolerance();
    }

//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HGF2DSegment object.
//-----------------------------------------------------------------------------
inline HGF2DSegment::HGF2DSegment(const HGF2DSegment& pi_rObj)
    : HGF2DBasicLinear(pi_rObj)
    {
    if (this != &pi_rObj)
        {
        ResetTolerance();
        }
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HGF2DSegment::~HGF2DSegment()
    {
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another line object.
//-----------------------------------------------------------------------------
inline HGF2DSegment& HGF2DSegment::operator=(const HGF2DSegment& pi_rObj)
    {
    // Invoque the ancester operator
    HGF2DBasicLinear::operator=(pi_rObj);

    // Return reference to self
    return (*this);
    }


//-----------------------------------------------------------------------------
// GetBasicLinearType
// Return the basic linear type
//-----------------------------------------------------------------------------
inline HGF2DBasicLinearTypeId HGF2DSegment::GetBasicLinearType() const
    {
    return (HGF2DSegment::CLASS_ID);
    }

/** -----------------------------------------------------------------------------
    This method sets the start point of the segment. If the
    coordinates are expressed in a different coordinate system than
    the one used by the segment, conversion will occur.

    @param pi_rNewStartPoint Constant reference to the HGF2DPosition object
                             containing the definition of the new first point
                             of segment..

    Example
    @code
    HGF2DSegment            MySeg1();
    HGF2DPosition            MyFirstPoint(10, 10);
    MySeg1.SetStartPoint(MyFirstPoint);
    @end

    @see GetStartPoint()
    @see SetEndPoint()
    @see SetStartPoint()
    @see HGF2DPosition
    -----------------------------------------------------------------------------
*/
inline void HGF2DSegment::SetStartPoint(const HGF2DPosition& pi_rNewStartPoint)
    {
    // Set the start point to given expressed in the segment coordinate
    // system
    m_StartPoint = pi_rNewStartPoint;

    // Reset tolerance
    ResetTolerance();
    }

/** -----------------------------------------------------------------------------
    This method sets the end point of the segment. If the
    coordinates are expressed in a different coordinate system than
    the one used by the segment, conversion will occur.

    @param pi_rNewEndPoint Constant reference to the HGF2DPosition object
                           containing the definition of the new end point
                           of segment.

    Example
    @code
    HGF2DSegment            MySeg1;
    HGF2DPosition            MyEndPoint(10.0, 10.0);
    ...
    MySeg1.SetEndPoint(MyEndPoint);
    @end

    @see GetEndPoint()
    @see SetStartPoint()
    @see SetEndPoint()
    @see HGF2DPosition
    -----------------------------------------------------------------------------
*/
inline void HGF2DSegment::SetEndPoint(const HGF2DPosition& pi_rNewEndPoint)
    {
    // Set the end point to given expressed in the segment coordinate
    // system
    m_EndPoint = pi_rNewEndPoint;

    // Reset tolerance
    ResetTolerance();
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
inline HGF2DLiteLine HGF2DSegment::CalculateLine() const
    {
    return (HGF2DLiteLine(m_StartPoint, m_EndPoint));
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
inline bool HGF2DSegment::IsParallelTo(const HGF2DLiteLine& pi_rLine) const
    {
    // Check if given line and the line the segment belongs are parallel
    return (pi_rLine.IsParallelTo(HGF2DLiteLine(m_StartPoint, m_EndPoint)));
    }

/** -----------------------------------------------------------------------------
    This method returns true if the segments are parallel one to
    the other.

    @param pi_rSegment Constant reference to the segment to check if
                       it is parallel to self

    @return true if segments are parallel; false otherwise.

    Example
    @code
    HGF2DPosition        MyFirstPoint(10.0, 10.0);
    HGF2DPosition        MySecondPoint(15.0, 16.0);
    HGF2DSegment        MySeg1(MyFirstPoint, MySecondPoint);
    ...
    HGF2DPosition      MyThirdPoint(12.0, 13.0);
    HGF2Dlocation        MyFourthPoint(12.0,-3.0);
    HGF2DSegment        MySeg2(MyThridrPoint, MyFourthPoint);

    bool AreTheyParallel = MySeg1.IsParallelTo(MySeg2);
    @end

    @see HGF2DLiteLine
    -----------------------------------------------------------------------------
*/
inline bool HGF2DSegment::IsParallelTo(const HGF2DSegment& pi_rSegment) const
    {
    // Check if the lines the segments belong to are parallel
    return (HGF2DLiteLine(m_StartPoint,
                      m_EndPoint).IsParallelTo(HGF2DLiteLine(pi_rSegment.m_StartPoint,
                                                         pi_rSegment.m_EndPoint)));
    }

//-----------------------------------------------------------------------------
// CalculateBearing
// Calculates and returns the bearing of segment at specified point
//-----------------------------------------------------------------------------
inline HGFBearing HGF2DSegment::CalculateBearing(const HGF2DPosition& pi_rPoint,
                                                 HGF2DVector::ArbitraryDirection pi_Direction) const
    {
    // The segment must not be NULL
    HPRECONDITION(m_StartPoint != m_EndPoint);

    // The point must be located on segment
    HPRECONDITION(IsPointOn(pi_rPoint));

    // The bearing on a segment is constant whatever the given point
    return (pi_Direction == HGF2DVector::BETA ?
            (m_EndPoint - m_StartPoint).CalculateBearing() :
            (m_StartPoint - m_EndPoint).CalculateBearing());
    }

//-----------------------------------------------------------------------------
// CalculateAngularAcceleration
// Calculates and returns the angular acceleration of segment at specified point
//-----------------------------------------------------------------------------
inline double HGF2DSegment::CalculateAngularAcceleration(const HGF2DPosition& pi_rPoint,
                                                                         HGF2DVector::ArbitraryDirection pi_Direction) const
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
inline double HGF2DSegment::CalculateRayArea(const HGF2DPosition& pi_rPoint) const
    {
    return ((m_StartPoint.GetX() * ((m_EndPoint - pi_rPoint).GetDeltaY())) / 2);
    }

//-----------------------------------------------------------------------------
// CalculateRelativePoint
// Calculates and returns the location based on the given relative position.
//-----------------------------------------------------------------------------
inline HGF2DPosition HGF2DSegment::CalculateRelativePoint(double pi_RelativePos) const
    {
    // The relative position must be between 0.0 and 1.0
    HPRECONDITION((pi_RelativePos >= 0.0) && (pi_RelativePos <= 1.0));

    return (m_StartPoint + (pi_RelativePos * (m_EndPoint - m_StartPoint)));
    }

//-----------------------------------------------------------------------------
// CalculateRelativePosition
// Calculates and returns the reltive position of given location on segment.
//-----------------------------------------------------------------------------
inline double HGF2DSegment::CalculateRelativePosition(const HGF2DPosition& pi_rPointOnLinear) const
    {
    // The given point must be located on segment
    HPRECONDITION(IsPointOn(pi_rPointOnLinear));

    // The segment must not be null
    HPRECONDITION(m_StartPoint != m_EndPoint);

    // The relative position is distance ratio between distance to given and to end point
    // from start point

    // Check if segment is vertical or close but not perfectly horizontal
    return((HDOUBLE_EQUAL(m_StartPoint.GetX(), m_EndPoint.GetX(), GetTolerance()) &&
            (m_StartPoint.GetY() != m_EndPoint.GetY())) ?
           ((pi_rPointOnLinear.GetY() - m_StartPoint.GetY()) / (m_EndPoint.GetY() - m_StartPoint.GetY())) :
           ((pi_rPointOnLinear.GetX() - m_StartPoint.GetX()) / (m_EndPoint.GetX() - m_StartPoint.GetX())));
    }

//-----------------------------------------------------------------------------
// Shorten
// Shortens the segment definition by specification of relative positions to self.
//-----------------------------------------------------------------------------
inline void HGF2DSegment::Shorten(double pi_StartRelativePos, double pi_EndRelativePos)
    {
    // The relative positions given must be between 0.0 and 1.0
    HPRECONDITION((pi_StartRelativePos >= 0.0) && (pi_StartRelativePos <= 1.0));
    HPRECONDITION((pi_EndRelativePos >= 0.0) && (pi_EndRelativePos <= 1.0));

    // The given end relative position must be greater than the start
    // relative position
    HPRECONDITION(pi_StartRelativePos <= pi_EndRelativePos);

    double     DeltaX(m_EndPoint.GetX() - m_StartPoint.GetX());
    double     DeltaY(m_EndPoint.GetY() - m_StartPoint.GetY());

    m_EndPoint.SetX(m_StartPoint.GetX() + (DeltaX * pi_EndRelativePos));
    m_EndPoint.SetY(m_StartPoint.GetY() + (DeltaY * pi_EndRelativePos));

    m_StartPoint.SetX(m_StartPoint.GetX() + (DeltaX * pi_StartRelativePos));
    m_StartPoint.SetY(m_StartPoint.GetY() + (DeltaY * pi_StartRelativePos));

    // Reset tolerance
    ResetTolerance();
    }


//-----------------------------------------------------------------------------
// Shorten
// Shortens the segment definition by specification of new start and end points
//-----------------------------------------------------------------------------                           z
inline void HGF2DSegment::Shorten(const HGF2DPosition& pi_rNewStartPoint,
                                  const HGF2DPosition& pi_rNewEndPoint)
    {
    // The shorten points must be located on segment
    HPRECONDITION(IsPointOn(pi_rNewStartPoint) && IsPointOn(pi_rNewEndPoint));

    // The relative position of given start point must be smaller than
    // the relative position of given end point
    HPRECONDITION((HDOUBLE_EQUAL(CalculateLength(), 0.0, GetTolerance())) ||
                  (HDOUBLE_SMALLER_OR_EQUAL_EPSILON(CalculateRelativePosition(pi_rNewStartPoint), CalculateRelativePosition(pi_rNewEndPoint))));

    // Set extremity points while keeping expression in the segment coordinate system
    m_EndPoint = pi_rNewEndPoint;
    m_StartPoint = pi_rNewStartPoint;

    // Reset tolerance
    ResetTolerance();
    }


//-----------------------------------------------------------------------------
// ShortenTo
// Shortens the segment definition by specification of a new end point
// In the case of a segment this is identical to setting a new end point with
// SetEndPoint(), except that it is required that new point be located ON
// segment
//-----------------------------------------------------------------------------
inline void HGF2DSegment::ShortenTo(const HGF2DPosition& pi_rNewEndPoint)
    {
    // The given point must be located on segment
    HPRECONDITION(IsPointOn(pi_rNewEndPoint));

    // The end point is set while keeping interpretation coordinate system
    // to the one of the segment
    m_EndPoint = pi_rNewEndPoint;

    // Reset tolerance
    ResetTolerance();
    }

//-----------------------------------------------------------------------------
// ShortenTo
// Shortens the segment definition by specification of a new end relative point
//-----------------------------------------------------------------------------
inline void HGF2DSegment::ShortenTo(double pi_EndRelativePos)
    {
    // The given relative position must be greater than 0.0 and smaller (or equal) than 1.0
    HPRECONDITION((pi_EndRelativePos >= 0.0) && (pi_EndRelativePos <= 1.0));

    // Change the segment which will go from current start point to
    // described end point
    m_EndPoint.SetX(m_StartPoint.GetX() +
                       (pi_EndRelativePos * (m_EndPoint.GetX() - m_StartPoint.GetX())));
    m_EndPoint.SetY(m_StartPoint.GetY() +
                       (pi_EndRelativePos * (m_EndPoint.GetY() - m_StartPoint.GetY())));

    // Reset tolerance
    ResetTolerance();
    }


//-----------------------------------------------------------------------------
// ShortenFrom
// Shortens the segment definition by specification of a new start point
// In the case of a segment this is identical to setting a new start point with
// SetStartPoint(), except that it is required that new point be located ON
// segment
//-----------------------------------------------------------------------------
inline void HGF2DSegment::ShortenFrom(const HGF2DPosition& pi_rNewStartPoint)
    {
    // The given point must be located on segment
    HPRECONDITION(IsPointOn(pi_rNewStartPoint));

    // The start point is set while keeping interpretation coordinate system
    // to the one of the segment
    m_StartPoint = pi_rNewStartPoint;

    // Reset tolerance
    ResetTolerance();
    }


//-----------------------------------------------------------------------------
// ShortenFrom
// Shortens the segment definition by specification of a new start relative point
//-----------------------------------------------------------------------------
inline void HGF2DSegment::ShortenFrom(double pi_StartRelativePos)
    {
    // The given relative position must be greater (or equal) than 0.0 and smaller than 1.0
    HPRECONDITION((pi_StartRelativePos >= 0.0) && (pi_StartRelativePos <= 1.0));

    // Change the segment which will go from described start point
    m_StartPoint.SetX(m_StartPoint.GetX() + (pi_StartRelativePos * (m_EndPoint.GetX() - m_StartPoint.GetX())));
    m_StartPoint.SetY(m_StartPoint.GetY() + (pi_StartRelativePos * (m_EndPoint.GetY() - m_StartPoint.GetY())));

    // Reset tolerance
    ResetTolerance();
    }


//-----------------------------------------------------------------------------
// AutoCrosses
// Indicates if the linear crosses itself
// This is of course impossible for a segment
//-----------------------------------------------------------------------------
inline bool HGF2DSegment::AutoCrosses() const
    {
    return(false);
    }

#if (0)
//-----------------------------------------------------------------------------
// AutoIntersect
// Finds and returns all auto intersection points of linear
//-----------------------------------------------------------------------------
inline size_t HGF2DSegment::AutoIntersect(HGF2DPositionCollection* po_pPoints) const
    {
    HPRECONDITION(po_pPoints);

    // A segment cannot auto intersect
    return(0);
    }
#endif


//-----------------------------------------------------------------------------
// GetExtent
// Returns the extent of the segment
//-----------------------------------------------------------------------------
inline HGF2DLiteExtent HGF2DSegment::GetExtent() const
    {
    return(HGF2DLiteExtent(MIN(m_StartPoint.GetX(), m_EndPoint.GetX()),
                           MIN(m_StartPoint.GetY(), m_EndPoint.GetY()),
                           MAX(m_StartPoint.GetX(), m_EndPoint.GetX()),
                           MAX(m_StartPoint.GetY(), m_EndPoint.GetY())));
    }


//-----------------------------------------------------------------------------
// Clone
// Returns a dynamically allocated copy of the segment
//-----------------------------------------------------------------------------
inline HGF2DVector* HGF2DSegment::Clone() const
    {
    return(new HGF2DSegment(*this));
    }

//-----------------------------------------------------------------------------
// AreSegmentsTouching
// Indicates if the two segments are touching
//-----------------------------------------------------------------------------
inline bool HGF2DSegment::AreSegmentsTouching(const HGF2DSegment& pi_rSegment) const
    {
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
inline bool HGF2DSegment::AreSegmentsCrossing(const HGF2DSegment& pi_rSegment) const
    {

    HGF2DPosition DummyPoint;

    // For two segments to be crossing, they must first have extents which overlap
    // then they must intersect
    return (GetExtent().InnerOverlaps(pi_rSegment.GetExtent(), MIN(GetTolerance(), pi_rSegment.GetTolerance())) &&
            (IntersectSegment(pi_rSegment, &DummyPoint) == HGF2DSegment::CROSS_FOUND));
    }




//-----------------------------------------------------------------------------
// AreSegmentsAdjacent
// Indicates if the two segments are adjacent.
//-----------------------------------------------------------------------------
inline bool HGF2DSegment::AreSegmentsAdjacent(const HGF2DSegment& pi_rSegment) const
    {
    // To be adjacent, two segments must first have extents which overlap and
    // share the same line
    // and be connected on each other at any point
    return (GetExtent().OutterOverlaps(pi_rSegment.GetExtent(), MIN(GetTolerance(), pi_rSegment.GetTolerance())) &&
            (HGF2DLiteLine(m_StartPoint, m_EndPoint) == HGF2DLiteLine(pi_rSegment.m_StartPoint, pi_rSegment.m_EndPoint)) &&
            (ConnectsTo(pi_rSegment) || pi_rSegment.ConnectsTo(*this)));
    }


//-----------------------------------------------------------------------------
// AreSegmentsFlirting
// Indicates if the two segments are flirting.
//-----------------------------------------------------------------------------
inline bool HGF2DSegment::AreSegmentsFlirting(const HGF2DSegment& pi_rSegment) const
    {
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
inline bool HGF2DSegment::IsNull() const
    {
    return(m_StartPoint.IsEqualTo(m_EndPoint, GetTolerance()));
    }

//-----------------------------------------------------------------------------
// AdjustStartPointTo
// Adjust the start point to an more exact point located less than
// an epsilon from previous point
//-----------------------------------------------------------------------------
inline void HGF2DSegment::AdjustStartPointTo(const HGF2DPosition& pi_rPoint)
    {
    HPRECONDITION(m_StartPoint.IsEqualTo(pi_rPoint, GetTolerance()));

    m_StartPoint = pi_rPoint;

    // Tolerance is not adjusted since change is only minor
    }

//-----------------------------------------------------------------------------
// AdjustEndPointTo
// Adjust the end point to an more exact point located less than
// an epsilon from previous point
//-----------------------------------------------------------------------------
inline void HGF2DSegment::AdjustEndPointTo(const HGF2DPosition& pi_rPoint)
    {
    HPRECONDITION(m_EndPoint.IsEqualTo(pi_rPoint, GetTolerance()));

    m_EndPoint = pi_rPoint;

    // Tolerance is not adjusted since change is only minor
    }


//-----------------------------------------------------------------------------
// Drop
// Returns the description of segments in the form of raw location
// segments
//-----------------------------------------------------------------------------
inline void HGF2DSegment::Drop(HGF2DPositionCollection* po_pPoints,
                               double                 pi_rTolerance,
                               EndPointProcessing pi_EndPointProcessing) const
    {
    HPRECONDITION(po_pPoints != 0);

    // NOTE : Tolerance is unused since drop is exact

    // Check if end point must be added
    if (pi_EndPointProcessing == HGF2DLinear::INCLUDE_END_POINT)
        {
        // Both start and end point must be added ... allocate 2 spaces in list
        po_pPoints->reserve(2);

        // Add both points
        po_pPoints->push_back(m_StartPoint);
        po_pPoints->push_back(m_EndPoint);
        }
    else
        {
        // Add only the start point
        po_pPoints->push_back(m_StartPoint);
        }

    }

//-----------------------------------------------------------------------------
// ResetTolerance
// PRIVATE method
// Recalculates the tolerance if needed and permitted
//-----------------------------------------------------------------------------
inline void HGF2DSegment::ResetTolerance()
    {
    // Check if auto-tolerance is active
    if (IsAutoToleranceActive())
        {
        // Compute appropriate tolerance (default is global epsilon)
        double Tolerance = HGLOBAL_EPSILON;

        // If coordinates are greater than global tolerance divided by
        // float precision (~= 1E-8 / 1E-15 -> 1E7 ... 1E6 for all cases)
        // Then we use global epsilon * coordinate
        Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(m_StartPoint.GetX()));
        Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(m_StartPoint.GetY()));
        Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(m_EndPoint.GetX()));
        Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(m_EndPoint.GetY()));

        SetTolerance(MIN(HMAX_EPSILON, Tolerance));
        }
    }



END_IMAGEPP_NAMESPACE


