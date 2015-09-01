//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hve/src/HVE2DPolygonOfSegments.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HVE2DPolygonOfSegments
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HVE2DPolygonOfSegments.h>
#include <Imagepp/all/h/HGF2DPolygonOfSegments.h>
#include <Imagepp/all/h/HVE2DHoledShape.h>
#include <Imagepp/all/h/HVE2DComplexShape.h>
#include <Imagepp/all/h/HGF2DLiteSegment.h>
#include <Imagepp/all/h/HVE2DRectangle.h>
#include <Imagepp/all/h/HVE2DPolygon.h>
#include <Imagepp/all/h/HVE2DSegment.h>
#include <Imagepp/all/h/HVE2DVoidShape.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGFScanLines.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HGF2DLiteExtent.h>

HPM_REGISTER_CLASS(HVE2DPolygonOfSegments, HVE2DSimpleShape)


enum PointUsage
    {
    UNKNOWN,
    ON_POINT,
    USED
    };





//----------------------------------------------------------------------------
// Class HVEDecompositionException
// This class is a local file internal exception used during decomposition.
// It cannot and should not be thrown outside the present code.
// IT DOES NOT INHERIT FROM HFCException
//----------------------------------------------------------------------------
class HVEDecompositionException
    {
public:
    // Primary methods.
    // Contructor and destructor.
    HVEDecompositionException()
    {
    }
    virtual         ~HVEDecompositionException()
    {
    }
    HVEDecompositionException&   operator=(const HVEDecompositionException& pi_rObj)
    {
    return *this;
    }
    HVEDecompositionException(const HVEDecompositionException& pi_rObj)
    {
    }
    };


/*---------------------------------------------------------------------------------**//**
    PUBLIC HELPER FUNCTION

    Do not make a method out of it since externally the HVEShape does not
    know of HVE2DPolySegment
+---------------+---------------+---------------+---------------+---------------+------*/
/*static*/ HVE2DShape* HVE2DPolygonOfSegments::CreateShapeFromAutoCrossingPolySegment(const HVE2DPolySegment& pi_rAutoCrossingPolySegment)
    {
    HVE2DShape* pResultShape = NULL;

    /*----------------------------------------------------------------------------
        The polysegment auto crosses meaning we must use special procedure to obtain
        a valid result shape. We must obtain a list of non crossing components
        then arrange these together depending upon their spatial relation.
    ----------------------------------------------------------------------------*/
    list<HFCPtr<HVE2DPolySegment> > ListOfPolySegments;

    if (!pi_rAutoCrossingPolySegment.SplitIntoNonAutoCrossing(&ListOfPolySegments, true))
        {
        // Tough luck ... return empty
        return NULL;
        }

    // Make that there are at least one component (otherwise, list of
    // points is considered invalid)
    HASSERT(ListOfPolySegments.size() > 0);

    /*----------------------------------------------------------------------------
        Now we are sure that all components do not cross with each other.
        We are also sure that each component is closed. We do not know however
        if these are distinct shapes or one shape with holes into this master shape.
        It can be either ones. We analyze the result set to know the type.
    ----------------------------------------------------------------------------*/
    // Search for any shape into any other

    list<HFCPtr<HVE2DPolySegment> >::iterator Itr = ListOfPolySegments.begin();
    list<HFCPtr<HVE2DPolySegment> >::iterator MasterItr;
    bool MasterFound = false;

    // For every polysegment ...
    for ( ; !MasterFound && Itr != ListOfPolySegments.end(); ++Itr)
        {
        // Create a polygon of segments
        HFCPtr<HVE2DPolygonOfSegments> pFirstPolygon = new HVE2DPolygonOfSegments(**Itr);

        // For every other polysegment
        list<HFCPtr<HVE2DPolySegment> >::iterator SecondItr = Itr;
        ++SecondItr;
        for ( ; !MasterFound && SecondItr != ListOfPolySegments.end(); ++SecondItr)
            {
            // Create a polygon of segments
            HFCPtr<HVE2DPolygonOfSegments> pSecondPolygon = new HVE2DPolygonOfSegments(**SecondItr);

            // Check spatial relation of second to first
            if (HVE2DShape::S_IN == pFirstPolygon->CalculateSpatialPositionOf(*pSecondPolygon))
                {
                MasterFound = true;
                MasterItr = Itr;
                }
            else if (HVE2DShape::S_IN == pSecondPolygon->CalculateSpatialPositionOf(*pFirstPolygon))
                {
                MasterFound = true;
                MasterItr = SecondItr;
                }
            }
        }

    // Check if a master shape was found ...
    if (MasterFound)
        {
        // Since a master shape was found, then all others are holes
        // Create base holed shape
        HAutoPtr<HVE2DHoledShape> pNewHoledShape(new HVE2DHoledShape(HVE2DPolygonOfSegments(**MasterItr)));

        // Add all other shapes as holes
        list<HFCPtr<HVE2DPolySegment> >::iterator Itr = ListOfPolySegments.begin();
        for ( ; Itr != ListOfPolySegments.end(); ++Itr)
            {
            // If shape is not master shape
            if (Itr != MasterItr)
                {
                pNewHoledShape->AddHole(HVE2DPolygonOfSegments(**Itr));
                }
            }

        pResultShape = pNewHoledShape.release();

        }
    else
        {
        // Since no master was found all shapes are disjoint (though flirting)
        // The result is a complex shape with all components at the same level.
        HAutoPtr<HVE2DComplexShape> pComplexShape(new HVE2DComplexShape(pi_rAutoCrossingPolySegment.GetCoordSys()));

        list<HFCPtr<HVE2DPolySegment> >::iterator Itr = ListOfPolySegments.begin();
        for ( ; Itr != ListOfPolySegments.end(); ++Itr)
            {
            pComplexShape->AddShape(HVE2DPolygonOfSegments(**Itr));
            }

        pResultShape = pComplexShape.release();
        }

    return(pResultShape);
    }

//-----------------------------------------------------------------------------
// private inline method
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// CalculateSpatialPositionOfPositionSegment2
// PRIVATE
// This method returns the spatial position relative to shape of given points
// This method check for all three points of a segment to make sure that
// The segment is really ON if ON is obtained for point.
// This method is equivalent to CalculateSpatialPositionOfPositionSegment
// however will not make errors and will process multi-flirting segments.
//-----------------------------------------------------------------------------
inline HVE2DShape::SpatialPosition HVE2DPolygonOfSegments::CalculateSpatialPositionOfPositionSegment2(const HGF2DLiteExtent& pi_rSelfLiteExtent,
        const HGF2DPosition&   pi_rStartPoint,
        const HGF2DPosition&   pi_rEndPoint,
        double                pi_Tolerance) const
    {
    // The tolerance must be greater than 0
    HPRECONDITION(pi_Tolerance > 0.0);

    // Declare return value
    HVE2DShape::SpatialPosition MyPartPosition;

    // Extract position values
    HGF2DPosition MidPoint;

    MidPoint.SetX((pi_rStartPoint.GetX() + pi_rEndPoint.GetX()) / 2.0);
    MidPoint.SetY((pi_rStartPoint.GetY() + pi_rEndPoint.GetY()) / 2.0);

    // Check if there is possible interaction
    double XMin = pi_rSelfLiteExtent.GetXMin();
    double XMax = pi_rSelfLiteExtent.GetXMax();
    double YMin = pi_rSelfLiteExtent.GetYMin();
    double YMax = pi_rSelfLiteExtent.GetYMax();

    // Extract position values
    double X(MidPoint.GetX());
    double Y(MidPoint.GetY());

    // Check if there is possible interaction
//    if (pi_rSelfLiteExtent.IsPointOutterIn(MidPoint, pi_Tolerance))
    if (((X > XMin) || HDOUBLE_EQUAL(X, XMin, pi_Tolerance)) &&
        ((X < XMax) || HDOUBLE_EQUAL(X, XMax, pi_Tolerance))&&
        ((Y > YMin) || HDOUBLE_EQUAL(Y, YMin, pi_Tolerance))&&
        ((Y < YMax) || HDOUBLE_EQUAL(Y, YMax, pi_Tolerance)))
        {
        MyPartPosition = CalculateSpatialPositionOfPosition(MidPoint, pi_Tolerance);
        }
    else
        MyPartPosition = HVE2DShape::S_OUT;


    // If the solution was ON and the delta X and delta Y are less than 2 EPSILON
    // between previous and current coordinates, then it may happen
    // that the segment is effectively OUT or IN
    if (MyPartPosition == HVE2DShape::S_ON)
        {

        // We create a full fledged segment
        HVE2DSegment MySegment(HGF2DLocation(pi_rStartPoint, GetCoordSys()), HGF2DLocation(pi_rEndPoint, GetCoordSys()));
        MySegment.SetAutoToleranceActive(false);
        MySegment.SetTolerance(pi_Tolerance);

        MyPartPosition = CalculateSpatialPositionOf(MySegment);
        }

    return(MyPartPosition);
    }
//-----------------------------------------------------------------------------
// end private inline method
//-----------------------------------------------------------------------------






/** -----------------------------------------------------------------------------
    This method allocates a parallel copy of the polygon of segments.
    The principle of parallel copying is extremely complex for special cases
    when notable the change of direction is very sharp and when the distance
    of the copy is great. The present implantation allocates an elegant
    copy, preventing the reversal of segments and the introduction of
    auto-cross points. The parallel copy copies segments parallel to themselves
    and joins them by elongation of subsequent segments. The copy of the segments
    is performed for each start and end point independantly in the direction of
    the median of the angle formed by the two segments the point belongs to.
    If the direction of start to end point changes (reverses) from the original
    segment, a segment reversal is detected and the segment is not added, nor are any
    other segments that would introduce an autocross point.

    There are two directions in which a parallel copy can be performed. Specification
    of the direction requires knowledge about the direction of rotation of the polygon
    of segments. If the polygon rotates counter-clockwise and ALPHA direction is
    provided then the copy will be external to the polygon of segments. If BETA is
    provided the the copy will be internal.
    If on the contrary, the polygon of segments turns clockwise, BETA specifies
    and external copy, while ALPHA specifies and internal copy.

    Note that it is possible for the copy to be empty if an internal copy is
    perfomed.

    For details refer to the vector handbook.

    @param pi_rOffset A positive distance indicating the distance to copy from/

    @param pi_DirectionToRight ALPHA or BETA to indicate direction. Refer
                               to text above for details.

    @return A dynamically allocated copy of the polygon of segments that must be
            freed when needed no more.

    Example
    @code
    @end

    @see Vector Handbook
    @see CalculateRotationDirection()
    -----------------------------------------------------------------------------
*/
HVE2DPolygonOfSegments*  HVE2DPolygonOfSegments::AllocateParallelCopy(double                          pi_Offset,
                                                                      HVE2DVector::ArbitraryDirection pi_DirectionToRight) const

    {
    // The offset distance is greater than 0.0
    HPRECONDITION(pi_Offset > 0.0);


    // Obtain bearing perpendicular to start point
    double Sweep;
    HGFBearing PerpendicularBearing = CalculatePerpendicularBearingAt(m_PolySegment.GetStartPoint(), pi_DirectionToRight, &Sweep);

    // Make a line along this bearing
    HGF2DLine MyStartLine(m_PolySegment.GetStartPoint(), PerpendicularBearing);

    HAutoPtr<HVE2DPolySegment> pPolyParallel(m_PolySegment.AllocateParallelCopy(pi_Offset, pi_DirectionToRight, &MyStartLine, &MyStartLine));

    // Make sure the polysegment closes
    pPolyParallel->AdjustEndPointTo(pPolyParallel->GetStartPoint());

    // Create new polygon of segments
    HAutoPtr<HVE2DPolygonOfSegments> pNewPolygon(new HVE2DPolygonOfSegments(*pPolyParallel));

    return(pNewPolygon.release());
    }



//-----------------------------------------------------------------------------
// Clip
// PRIVATE METHOD
// Intersects self with given rectangle
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DPolygonOfSegments::Clip(const HVE2DRectangle& pi_rRectangle) const
    {
    HINVARIANTS;

    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rRectangle.GetCoordSys());

    HVE2DShape*  pResultShape = NULL;


    if (IsEmpty() || pi_rRectangle.IsEmpty())
        {
        // Since at least one shape is empty, the result is empty
        pResultShape = new HVE2DVoidShape(GetCoordSys());
        }
    else
        {
        // Neither shapes are empty

        // We first compare if their extent overlap
        if (!GetExtent().InnerOverlaps(pi_rRectangle.GetExtent(), MIN(GetTolerance(), pi_rRectangle.GetTolerance())))
            {
            // The two shapes cannot possibly intersect ... therefore, the intersection is empty
            pResultShape = new HVE2DVoidShape(GetCoordSys());
            }
        else
            {
            // We now have rectangle of which extents overlap
            // expressed in the same coordinate system

            // The first task is to discard all segments which cannot interact with
            // the rectangle

            }
        }

    return(pResultShape);

    }

/** -----------------------------------------------------------------------------
    Checks if the polygon of segments can be simplified to a rectangle.
    Theorically a polygon can be simplified to a rectangle if it is equivalent
    to a rectangle within its current tolerance, but for performance reason
    the current implementation requires that there are only 5 segments
    in the polygon and that the direction of all segments be appropriate within
    a small tolerance to right angled direction.

    @return true if the polygon of segments can be represented as a rectangle.

    Example
    @code
    @end

    @see GenerateCorrespondingRectangle()
    -----------------------------------------------------------------------------
*/
bool HVE2DPolygonOfSegments::RepresentsARectangle() const
    {
    HINVARIANTS;

    bool   DoesRepresentARectangle = false;

    // A rectangle must be formed of exactly 5 points
    if (m_PolySegment.GetSize() == 5)
        {
        // The polygon has 5 points
        HGF2DPositionCollection::const_iterator Itr = m_PolySegment.m_Points.begin();
        HGF2DPositionCollection::const_iterator NextItr = Itr;
        NextItr++;

        // Check if all segments are alligned to the axes
        bool AllSegmentsAreAligned = true;

        for (; NextItr != m_PolySegment.m_Points.end() && AllSegmentsAreAligned ; ++NextItr)
            {
            // Check if points are colinear
            HGFBearing SegmentBearing = (HGF2DLocation(*NextItr, GetCoordSys()) -
                                         HGF2DLocation(*Itr, GetCoordSys())).CalculateBearing();

            // Obtain trigonometric value of bearing
            double TrigoBearing = SegmentBearing.CalculateTrigoAngle();

            AllSegmentsAreAligned =  (HDOUBLE_EQUAL_EPSILON(TrigoBearing, 0.0) ||
                                      HDOUBLE_EQUAL_EPSILON(TrigoBearing, PI/2) ||
                                      HDOUBLE_EQUAL_EPSILON(TrigoBearing, PI) ||
                                      HDOUBLE_EQUAL_EPSILON(TrigoBearing, 3 * PI/2) ||
                                      HDOUBLE_EQUAL_EPSILON(TrigoBearing, 2 * PI));


            // Advance iterator
            Itr = NextItr;
            }

        // If all segments were aligned, then it is a rectangle
        DoesRepresentARectangle = AllSegmentsAreAligned;
        }

    return(DoesRepresentARectangle);
    }

//-----------------------------------------------------------------------------
// Simplify()
// PRIVATE METHOD
// Simplifies the polygon
//-----------------------------------------------------------------------------
void HVE2DPolygonOfSegments::Simplify()
    {
    HINVARIANTS;

    double x1;
    double y1;
    double x2;
    double y2;

    // x0*(y1-y2) - y0*(x1-x2) + ((x1*y2) - (x2*y1))

    // The first and last points must be equal (WITHOUT EPSILON)
    HPRECONDITION(m_PolySegment.m_Points[0] == m_PolySegment.m_Points[m_PolySegment.GetSize() - 1]);

    // The polygon of segments is not empty
    // First stage is to eliminate all colinear consecutive segments
    // The polygon of segments is not empty ... it has at least three points
    // For every segment, check if two consecutive form a single segments
    HGF2DPositionCollection::iterator Itr = m_PolySegment.m_Points.begin();
    HGF2DPositionCollection::iterator NextItr = Itr;
    ++NextItr;
    HGF2DPositionCollection::iterator SecondNextItr = NextItr;
    ++SecondNextItr;
    for (; SecondNextItr != m_PolySegment.m_Points.end() ; ++SecondNextItr)
        {
        x1 = NextItr->GetX() - Itr->GetX();
        y1 = NextItr->GetY() - Itr->GetY();
        x2 = SecondNextItr->GetX() - Itr->GetX();
        y2 = SecondNextItr->GetY() - Itr->GetY();

        double Det = ((x1*y2) - (x2*y1)); // Determinant is 2 times the area of the triangle formed by three points.

        double Length1 = sqrt (x1*x1 + y1*y1);
        double Length2 = sqrt (x2*x2 + y2*y2);

        double MaxLength = MAX(Length1, Length2);

        double AreaTolerance = MaxLength * GetTolerance();

        if (HNumeric<double>::EQUAL (Det, 0.0, AreaTolerance))
            {
            // Points are colinear ...
            // Middle one will be removed
            NextItr = m_PolySegment.m_Points.erase(NextItr);

            SecondNextItr = NextItr;
            }
        else
            {
            // Advance iterators
            Itr = NextItr;
            NextItr = SecondNextItr;
            }
        }

    // There is still the first triplet to check
    // Get last point (different from start ... this is next to last point)
    HGF2DPositionCollection::reverse_iterator LastItr = m_PolySegment.m_Points.rbegin();
    ++LastItr;
    NextItr = m_PolySegment.m_Points.begin();
    SecondNextItr = NextItr;
    ++SecondNextItr;

    // Check if points are colinear (They should not be equal)
    if ((((HGF2DLocation(*SecondNextItr, GetCoordSys()) -
           HGF2DLocation(*NextItr, GetCoordSys())).CalculateBearing().IsEqualTo
          ((HGF2DLocation(*NextItr, GetCoordSys()) -
            HGF2DLocation(*LastItr, GetCoordSys())).CalculateBearing()))))
        {
        // Points are colinear ...
        // first one will be removed
        m_PolySegment.m_Points.erase(NextItr);

        // Set last point to new first point
        m_PolySegment.m_Points.back() = m_PolySegment.m_Points[0];

        m_PolySegment.m_StartPoint = HGF2DLocation(m_PolySegment.m_Points.front(), GetCoordSys());
        m_PolySegment.m_EndPoint = m_PolySegment.m_StartPoint;
        }

    HPOSTCONDITION(m_PolySegment.m_Points[0] == m_PolySegment.m_Points[m_PolySegment.GetSize() - 1]);


    }

/** -----------------------------------------------------------------------------
    Returns the rectangle corresponding to the polygon of segments shape. In
    order to call this function, the polygon of segment must represent a rectangle
    as defined by RepresentsARectangle().

    @return A dynamically allocated HVE2DRectangle containing the rectangle
            equivalent to self.

    Example
    @code
    @end

    @see RepresentsARectangle()
    -----------------------------------------------------------------------------
*/
HVE2DRectangle* HVE2DPolygonOfSegments::GenerateCorrespondingRectangle() const
    {
    HINVARIANTS;

    // The polygon of segment must represent a rectangle
    HPRECONDITION(RepresentsARectangle());

    // Obtain extent  and create rectangle with it
    HVE2DRectangle*  pNewRectangle = new HVE2DRectangle(GetExtent());

    // Set tolerance into rectangle
    pNewRectangle->SetAutoToleranceActive(IsAutoToleranceActive());
    pNewRectangle->SetTolerance(GetTolerance());

    return(pNewRectangle);
    }

//-----------------------------------------------------------------------------
// SetCoordSysImplementation
// Changes the interpretation coordinate system
//-----------------------------------------------------------------------------
void HVE2DPolygonOfSegments::SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    {
    // Call ancester set
    HVE2DShape::SetCoordSysImplementation(pi_rpCoordSys);

    // Indicate that pre-calculated rotation direction is no good
    m_RotationDirectionUpToDate = false;

    // Set coordinate system of component linear
    m_PolySegment.SetCoordSys(GetCoordSys());

    HINVARIANTS;
    }


//-----------------------------------------------------------------------------
// Constructor from rectangle
//-----------------------------------------------------------------------------
HVE2DPolygonOfSegments::HVE2DPolygonOfSegments(const HVE2DRectangle& pi_rRectangle)
    : HVE2DSimpleShape(pi_rRectangle.GetCoordSys()),
      m_PolySegment(pi_rRectangle.GetCoordSys()),
      m_RotationDirectionUpToDate(false)
    {

    double     XMin;
    double     XMax;
    double     YMin;
    double     YMax;

    // Extract rectangle definition
    pi_rRectangle.GetRectangle(&XMin, &YMin, &XMax, &YMax);

    // Set auto tolerance active setting
    SetAutoToleranceActive(pi_rRectangle.IsAutoToleranceActive());

    // Pre-allocate memory
    m_PolySegment.Reserve(5);

    // Append positions
    m_PolySegment.AppendPosition(HGF2DPosition(XMin, YMin));
    m_PolySegment.AppendPosition(HGF2DPosition(XMin, YMax));
    m_PolySegment.AppendPosition(HGF2DPosition(XMax, YMax));
    m_PolySegment.AppendPosition(HGF2DPosition(XMax, YMin));
    m_PolySegment.AppendPosition(HGF2DPosition(XMin, YMin));

    // Set polygon tolerance to settings of polysegment
    HVE2DVector::SetTolerance(m_PolySegment.GetTolerance());

    SetStrokeTolerance(pi_rRectangle.GetStrokeTolerance());

    HINVARIANTS;

    }


//-----------------------------------------------------------------------------
// Constructor with setting of complex linear
//-----------------------------------------------------------------------------
HVE2DPolygonOfSegments::HVE2DPolygonOfSegments(const HVE2DComplexLinear& pi_rComplex)
    : HVE2DSimpleShape(pi_rComplex.GetCoordSys()),
      m_PolySegment(pi_rComplex.GetCoordSys()),
      m_RotationDirectionUpToDate(false)
    {
    HINVARIANTS;

    // The shape forming a polygon may not cross its own path
//    HPRECONDITION(!pi_rComplex.AutoCrosses());

    // A linear forming a polygon must close on itself
    HPRECONDITION(pi_rComplex.GetStartPoint() == pi_rComplex.GetEndPoint());

    // Set auto tolerance determination
    SetAutoToleranceActive(pi_rComplex.IsAutoToleranceActive());

    // Insert the start point
    m_PolySegment.AppendPosition(HGF2DPosition(pi_rComplex.GetStartPoint().GetX(),
                                               pi_rComplex.GetStartPoint().GetY()));

    // We copy the end point of all segments of the given complex
    HVE2DComplexLinear::LinearList::const_iterator  Itr;
    for (Itr = pi_rComplex.GetLinearList().begin() ; Itr != pi_rComplex.GetLinearList().end() ; Itr++)
        {
        // The current component must be a segment or a polysegment
        // The linear must be a basic linear
        HASSERT((*Itr)->IsABasicLinear());

        // The linear must be a segment or polysegment
        HASSERT( ((static_cast<HVE2DBasicLinear*>(*Itr))->GetBasicLinearType() == HVE2DSegment::CLASS_ID) ||
                 ((static_cast<HVE2DBasicLinear*>(*Itr))->GetBasicLinearType() == HVE2DPolySegment::CLASS_ID) );

        // Check if it is a segment
        if ((static_cast<HVE2DBasicLinear*>(*Itr))->GetBasicLinearType() == HVE2DSegment::CLASS_ID)
            {
            // Extract and save end point
            m_PolySegment.AppendPosition(HGF2DPosition((*Itr)->GetEndPoint().GetX(), (*Itr)->GetEndPoint().GetY()));
            }
        else
            {
            // The linear is a polysegment
            HVE2DPolySegment* pPolySegment = static_cast<HVE2DPolySegment*>(*Itr);

            size_t Index;
            for (Index = 1; Index < pPolySegment->GetSize() ; Index++)
                {
                m_PolySegment.AppendPosition(pPolySegment->GetPosition(Index));
                }
            }
        }

    // Set the polygon to the same settings as complex
    HVE2DVector::SetTolerance(m_PolySegment.GetTolerance());

    // The area of new polygon may not be 0.0
    HPOSTCONDITION(!HDOUBLE_EQUAL(CalculateArea(), 0.0, MIN(HMAX_EPSILON, GetTolerance() * GetTolerance())));
    }

//-----------------------------------------------------------------------------
// Constructor with setting of polysegment
//-----------------------------------------------------------------------------
HVE2DPolygonOfSegments::HVE2DPolygonOfSegments(const HVE2DPolySegment& pi_rPolySegment)
    : HVE2DSimpleShape(pi_rPolySegment.GetCoordSys()),
      m_PolySegment(pi_rPolySegment),
      m_RotationDirectionUpToDate(false)
    {
    HASSERT(m_PolySegment.GetCoordSys() == GetCoordSys());

    // The shape forming a polygon may not cross its own path
    HASSERTSUPERDEBUG(!pi_rPolySegment.AutoCrosses());

    // A linear forming a polygon must close on itself
    HPRECONDITION(pi_rPolySegment.GetStartPoint() == pi_rPolySegment.GetEndPoint());

    // Set the polygon to the same settings as polysegment
    HVE2DVector::SetAutoToleranceActive(m_PolySegment.IsAutoToleranceActive());
    HVE2DVector::SetTolerance(m_PolySegment.GetTolerance());

    // The area of new polygon may not be 0.0
    // Note that this conditin may fail in some specific cases of parallel copy but the result should then be discarded
    // immediately after the copy.
    HASSERTSUPERDEBUG(!HDOUBLE_EQUAL(CalculateArea(), 0.0, MIN(HMAX_EPSILON, GetTolerance() * GetTolerance())));

    HINVARIANTS;

    }


//-----------------------------------------------------------------------------
// Constructor with a fence
//-----------------------------------------------------------------------------
HVE2DPolygonOfSegments::HVE2DPolygonOfSegments(const HGF2DPolygonOfSegments&  pi_rShape,
                                               const HFCPtr<HGF2DCoordSys>&   pi_rpCoordSys)
    : HVE2DSimpleShape(pi_rpCoordSys),
      m_RotationDirectionUpToDate(false),
      m_RotationDirection(CW),
      m_PolySegment(pi_rpCoordSys)
    {

    HASSERT (pi_rShape.GetListOfPoints().size());

    // Obtain list of points
    const HGF2DPositionCollection& rCoordCollection = pi_rShape.GetListOfPoints();

    // Create iterator
    HGF2DPositionCollection::const_iterator CoordItr = rCoordCollection.begin();

    // Pre-allocate points
    m_PolySegment.m_Points.reserve(rCoordCollection.size());

    // For every point
    for ( ; CoordItr != rCoordCollection.end() ; ++CoordItr)
        {
        // Copy point
        m_PolySegment.m_Points.push_back(HGF2DPosition(CoordItr->GetX(), CoordItr->GetY()));
        }

    // Adjust start and end points
    m_PolySegment.m_StartPoint = HGF2DLocation(m_PolySegment.m_Points[0], pi_rpCoordSys);
    m_PolySegment.m_EndPoint = HGF2DLocation (m_PolySegment.m_Points[m_PolySegment.m_Points.size() - 1], pi_rpCoordSys);

    // Adjust tolerance if needed
    ResetTolerance();

    HINVARIANTS;

    }



//-----------------------------------------------------------------------------
// Constructor with an array of values
//-----------------------------------------------------------------------------
HVE2DPolygonOfSegments::HVE2DPolygonOfSegments(size_t  pi_BufferLength,
                                               double pi_aBuffer[],
                                               const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HVE2DSimpleShape(pi_rpCoordSys),
      m_RotationDirectionUpToDate(false),
      m_RotationDirection(CW),
      m_PolySegment(pi_rpCoordSys)
    {

    // There must be at least 3 points provided
    HPRECONDITION(pi_BufferLength >= 6);

    // There must be an even number of values
    HPRECONDITION(pi_BufferLength % 2 == 0);

    // Preallocate points
    m_PolySegment.Reserve(pi_BufferLength / 2);

    // Preset tolerance activity
    m_PolySegment.SetAutoToleranceActive(IsAutoToleranceActive());

#if (0)
    // Check if auto tolerance is active
    if (IsAutoToleranceActive())
        {
        double Tolerance = HGLOBAL_EPSILON;

        // Copy points
        for (size_t Index = 0 ; Index < pi_BufferLength ; Index += 2)
            {
            m_PolySegment.AppendPoint(HGF2DPosition(pi_aBuffer[Index], pi_aBuffer[Index + 1]));

            // Adjust tolerance
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(pi_aBuffer[Index]));
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(pi_aBuffer[Index+1]));
            }

        SetTolerance(Tolerance);
        }
    else
        {
        // Copy points
        for (size_t Index = 0 ; Index < pi_BufferLength ; Index += 2)
            {
            m_PolySegment.m_Points.push_back(HGF2DPosition(pi_aBuffer[Index], pi_aBuffer[Index + 1]));
            }
        }

#else
    // Copy points
    for (size_t Index = 0 ; Index < pi_BufferLength ; Index += 2)
        {
        m_PolySegment.AppendPosition(HGF2DPosition(pi_aBuffer[Index], pi_aBuffer[Index + 1]));
        }

#endif

    // Check if given points represent an auto-closed shape
    if (pi_aBuffer[pi_BufferLength - 2] != pi_aBuffer[0] || pi_aBuffer[pi_BufferLength - 1] != pi_aBuffer[1])
        {
        // The given points do not close ... add initial point
        m_PolySegment.AppendPosition(HGF2DPosition(pi_aBuffer[0], pi_aBuffer[1]));
        }

    // Make sure tolerances are synchronized
    HVE2DVector::SetTolerance(m_PolySegment.GetTolerance());

    HINVARIANTS;
    }


//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another polygon object.
//-----------------------------------------------------------------------------
HVE2DPolygonOfSegments& HVE2DPolygonOfSegments::operator=(const HVE2DPolygonOfSegments& pi_rObj)
    {
    HINVARIANTS;

    if (&pi_rObj != this)
        {
        // Empty the polygon
        m_PolySegment.MakeEmpty();

        // Invoque the ancester operator
        HVE2DSimpleShape::operator=(pi_rObj);

        // Copy list of points
        m_PolySegment = pi_rObj.m_PolySegment;
        m_RotationDirection = pi_rObj.m_RotationDirection;
        m_RotationDirectionUpToDate = pi_rObj.m_RotationDirectionUpToDate;

        }

    // Return reference to self
    return (*this);
    }

//-----------------------------------------------------------------------------
// GetLinear
// This method returns the complex linear.of the shape
//-----------------------------------------------------------------------------
HVE2DComplexLinear HVE2DPolygonOfSegments::GetLinear() const
    {
    HINVARIANTS;

    // Create returned linear
    HVE2DComplexLinear      ReturnedComplex(GetCoordSys());

    // Check if there are any points in list
    if (m_PolySegment.GetSize() > 0)
        {
        HGF2DPosition   StartPoint(m_PolySegment.GetPosition(0));
        // For each and every point

        for (size_t Index = 1 ; Index < m_PolySegment.GetSize() ; Index++)
            {
            // Create segment and append to complex
            ReturnedComplex.AppendLinear(HVE2DSegment(StartPoint, m_PolySegment.GetPosition(Index), GetCoordSys()));

            // Save start point of next segment
            StartPoint = m_PolySegment.GetPosition(Index);
            }
        }

    // Set returned complex tolerance
    ReturnedComplex.SetAutoToleranceActive(IsAutoToleranceActive());
    ReturnedComplex.SetTolerance(GetTolerance());

    return (ReturnedComplex);
    }

/** -----------------------------------------------------------------------------
    This method sets the spatial definition of the polygon of segments.
    The given linear must be complex, not auto-intersect,
    must auto-close and only be constituted of HVE2DSegment objects.

    @param pi_rLinear The complex linear that contains the new polygon
                      path definition.

    Example
    @code
    @end

    @see GetLinear()
    -----------------------------------------------------------------------------
*/
void HVE2DPolygonOfSegments::SetLinear(const HVE2DLinear& pi_rLinear)
    {
    HINVARIANTS;

    // The shape forming a polygon may not cross its own path
//    HPRECONDITION(!pi_rLinear.AutoCrosses());

    // A linear forming a polygon must close on itself
    HPRECONDITION(pi_rLinear.GetStartPoint() == pi_rLinear.GetEndPoint());

    // The given must be a complex linear
    HPRECONDITION(pi_rLinear.IsComplex());

    // We empty current list of points
    m_PolySegment.MakeEmpty();

    // Reset tolerance to polysegment
    HVE2DVector::SetTolerance(m_PolySegment.GetTolerance());

    // Cast into a complex linear
    HVE2DComplexLinear*  pTheComplex = (HVE2DComplexLinear*)&pi_rLinear;

    // Insert first point
    m_PolySegment.AppendPosition(HGF2DPosition(pTheComplex->GetStartPoint().GetX(),
                                               pTheComplex->GetStartPoint().GetY()));


    // We copy the end point of all segments of the given complex
    HVE2DComplexLinear::LinearList::const_iterator  Itr;
    for (Itr = pTheComplex->GetLinearList().begin() ; Itr != pTheComplex->GetLinearList().end() ; Itr++)
        {
        // The current component must be a segment or a polysegment
        // The linear must be a basic linear
        HASSERT((*Itr)->IsABasicLinear());

        // The linear must be a segment or polysegment
        HASSERT( ((static_cast<HVE2DBasicLinear*>(*Itr))->GetBasicLinearType() == HVE2DSegment::CLASS_ID) ||
                 ((static_cast<HVE2DBasicLinear*>(*Itr))->GetBasicLinearType() == HVE2DSegment::CLASS_ID) );

        // Check if it is a segment
        if ((static_cast<HVE2DBasicLinear*>(*Itr))->GetBasicLinearType() == HVE2DSegment::CLASS_ID)
            {
            // Extract location expressed in current coordinate system
            HGF2DLocation   TheCurrentPoint((*Itr)->GetEndPoint(), GetCoordSys());

            // Extract and save end point
            m_PolySegment.AppendPosition(HGF2DPosition(TheCurrentPoint.GetX(), TheCurrentPoint.GetY()));

            }
        else
            {
            // The linear is a polysegment
            HVE2DPolySegment* pPolySegment = static_cast<HVE2DPolySegment*>(*Itr);

            size_t Index;
            for (Index = 1; Index < pPolySegment->GetSize() ; Index++)
                {
                // Extract location expressed in current coordinate system
                m_PolySegment.AppendPoint(pPolySegment->GetPoint(Index));
                }
            }
        }

    // Synchronize tolerance
    HVE2DVector::SetTolerance(m_PolySegment.GetTolerance());

    // Indicate rotation direction is dirty
    m_RotationDirectionUpToDate = false;
    }


//-----------------------------------------------------------------------------
// CalculateClosestPoint
// This method returns the closest point on polygon boundary to given point.
//-----------------------------------------------------------------------------
HGF2DLocation HVE2DPolygonOfSegments::CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const
    {
    HINVARIANTS;
    return(m_PolySegment.CalculateClosestPoint(pi_rPoint));
    }


//-----------------------------------------------------------------------------
// AreContiguousAt
// This method checks if the polygon is contiguous with given vector
// at specified point
//-----------------------------------------------------------------------------
bool HVE2DPolygonOfSegments::AreContiguousAt(const HVE2DVector& pi_rVector,
                                              const HGF2DLocation& pi_rPoint) const
    {
    HINVARIANTS;

    return(m_PolySegment.AreContiguousAt(pi_rVector, pi_rPoint));
    }

//-----------------------------------------------------------------------------
// GetExtent
// This method returns the extent of the polygon.
//-----------------------------------------------------------------------------
HGF2DExtent HVE2DPolygonOfSegments::GetExtent() const
    {
    HINVARIANTS;

    return(m_PolySegment.GetExtent());
    }

//-----------------------------------------------------------------------------
// Scale
// This method scales the polygon by the specified scaling factor
// oround the given location
//-----------------------------------------------------------------------------
void HVE2DPolygonOfSegments::Scale(double pi_ScaleFactor, const HGF2DLocation& pi_rScaleOrigin)
    {
    HINVARIANTS;

    // The given scale must be different from 0.0
    HPRECONDITION(pi_ScaleFactor != 0.0);

    // Scale polysegment
    m_PolySegment.Scale(pi_ScaleFactor, pi_rScaleOrigin);

    // Make sure that tolerances are synchronized
    HVE2DVector::SetTolerance(m_PolySegment.GetTolerance());

    // Indicate rotation direction is dirty
    m_RotationDirectionUpToDate = false;
    }

//-----------------------------------------------------------------------------
// Rotate
// This method rotates the polygon of segment by the specified angle
// around the given location
//-----------------------------------------------------------------------------
void HVE2DPolygonOfSegments::Rotate(double pi_Angle, const HGF2DLocation& pi_rRotationOrigin)
    {
    HINVARIANTS;

    // Rotate polysegment
    m_PolySegment.Rotate(pi_Angle, pi_rRotationOrigin);

    // Reset tolerance
    HVE2DVector::SetTolerance(m_PolySegment.GetTolerance());

    // Indicate pre-calculated rotation direction is dirty
    m_RotationDirectionUpToDate = false;
    }


//-----------------------------------------------------------------------------
// Scale
// This method scales the polygon by the specified scaling factor
// around the given location
//-----------------------------------------------------------------------------
void HVE2DPolygonOfSegments::Scale(double              pi_ScaleFactorX,
                                   double              pi_ScaleFactorY,
                                   const HGF2DLocation& pi_rScaleOrigin)
    {
    HINVARIANTS;

    // The given scale factors must be different from 0.0
    HPRECONDITION(pi_ScaleFactorX != 0.0);
    HPRECONDITION(pi_ScaleFactorY != 0.0);

    // Obtain scale origin in self coordinate system
    HGF2DLocation   ScaleOrigin(pi_rScaleOrigin, GetCoordSys());
    double ScaleOriginX = ScaleOrigin.GetX();
    double ScaleOriginY = ScaleOrigin.GetY();

    // For every part ... scale
    for(size_t Index = 0 ; Index < m_PolySegment.GetSize() ; Index++)
        {
        m_PolySegment.m_Points[Index][HGF2DPosition::X] += pi_ScaleFactorX * (ScaleOriginX - m_PolySegment.GetPosition(Index)[HGF2DPosition::X]);
        m_PolySegment.m_Points[Index][HGF2DPosition::Y] += pi_ScaleFactorY * (ScaleOriginY - m_PolySegment.GetPosition(Index)[HGF2DPosition::Y]);
        }

    // Reset tolerance
    ResetTolerance();

    // Indicate pre-calculated rotation direction is dirty
    m_RotationDirectionUpToDate = false;
    }


//-----------------------------------------------------------------------------
// CalculateRawArea
// This method calculates the raw area (signed) of the shape
//-----------------------------------------------------------------------------
double HVE2DPolygonOfSegments::CalculateRawArea() const
    {
    HINVARIANTS;

    double     TotalArea(0.0);

    // Check if there are at least 3 points
    if (m_PolySegment.GetSize() > 3)
        {
        // Calculate area of first and last triangles
        TotalArea += m_PolySegment.GetPosition(m_PolySegment.GetSize() - 1).GetX() * (m_PolySegment.GetPosition(0).GetY() - m_PolySegment.GetPosition(m_PolySegment.GetSize() - 2).GetY());

        TotalArea += m_PolySegment.GetPosition(0).GetX() * (m_PolySegment.GetPosition(1).GetY() - m_PolySegment.GetPosition(m_PolySegment.GetSize() - 1).GetY());

        // Add all other triangles
        for(size_t Index = 2 ; Index < m_PolySegment.GetSize() ; Index++)
            {
            TotalArea += m_PolySegment.GetPosition(Index - 1).GetX() * (m_PolySegment.GetPosition(Index).GetY() - m_PolySegment.GetPosition(Index - 2).GetY());
            }
        }

    return(TotalArea / 2.0);
    }


/** -----------------------------------------------------------------------------
    This method tells if the polygon is convex or not. A convex polygon is
    one for which the path does not drastically change direction back and
    forth. A convex polygon always turns either to the right or the the left
    although this amount of direction change may vary.

    This property is useful for simplification of some algorithms.

    @return true if the polygon is convex.

    Example
    @code
    @end

    @see CalculateRotationDirection()
    -----------------------------------------------------------------------------
*/
bool HVE2DPolygonOfSegments::IsConvex() const
    {
    HINVARIANTS;

    bool Result = true;

    // Check if there are at least 3 points
    if (m_PolySegment.GetSize() > 3)
        {
        HGF2DPositionCollection::const_iterator FirstItr(m_PolySegment.m_Points.begin());
        HGF2DPositionCollection::const_iterator SecondItr(FirstItr);
        ++SecondItr;
        HGF2DPositionCollection::const_iterator ThirdItr(SecondItr);
        ++ThirdItr;
        short Direction = 0;

        // Check for possible winding side change
        while (Result && ThirdItr != m_PolySegment.m_Points.end())
            {
            double TempResult = ((FirstItr->GetX() - SecondItr->GetX()) * (SecondItr->GetY() - ThirdItr->GetY())) -
                                 ((FirstItr->GetY() - SecondItr->GetY()) * (SecondItr->GetX() - ThirdItr->GetX()));
            if (HDOUBLE_GREATER(TempResult, 0.0, GetTolerance()))
                {
                if (Direction < 0)  // -1
                    Result = false;
                else if (Direction == 0)
                    Direction = 1;
                }
            else if (HDOUBLE_SMALLER(TempResult, 0.0, GetTolerance()))
                {
                if (Direction > 0)  // 1
                    Result = false;
                else if (Direction == 0)
                    Direction = -1;
                }

            ++FirstItr;
            ++SecondItr;
            ++ThirdItr;
            }

        if (Result)
            {
            // The previous loop didn't check the direction of the first segment
            // relative to the last segment. Check now...

            double TempResult = ((FirstItr->GetX() - SecondItr->GetX()) * (SecondItr->GetY() - m_PolySegment.m_Points[1].GetY())) -
                                 ((FirstItr->GetY() - SecondItr->GetY()) * (SecondItr->GetX() - m_PolySegment.m_Points[1].GetX()));

            if ((HDOUBLE_GREATER(TempResult, 0.0, GetTolerance()) && Direction < 0) ||
                (HDOUBLE_SMALLER(TempResult, 0.0, GetTolerance()) && Direction > 0))
                Result = false;
            }
        }

    return Result;
    }

//-----------------------------------------------------------------------------
// AllocateCopyInCoordSys
// This method allocates dynamically a copy of the polygon
//-----------------------------------------------------------------------------
HVE2DVector* HVE2DPolygonOfSegments::AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const
    {
    HINVARIANTS;

    HVE2DVector*    pResultVector;

    if (GetCoordSys() == pi_rpCoordSys)
        {
        pResultVector = new HVE2DPolygonOfSegments(*this);
        }
    else
        {
        // Check if this model between coordinate systems is parallelism preserving
        if (GetCoordSys()->HasParallelismPreservingRelationTo(pi_rpCoordSys))
            {
            pResultVector = AllocateCopyInParallismPreservingRelatedCoordSys(pi_rpCoordSys);
            }
        // Check if this model between coordinate systems is linearity preserving
        // Even though not parallelism preserving
        else if (GetCoordSys()->HasLinearityPreservingRelationTo(pi_rpCoordSys))
            {
            // The relation between coordinate system preserves linearity. This
            // implies that the polysegment converted can twist upon itself
            // In other words, the lines of the poly segment can result in autocrossing.
            // However with a linearity preserving relation, lines remain lines
            // therefore a polysegment will remain a polysegment. The typical relation
            // is a projective. This means that a single point results in autocrossing conditions.
            // This point can be checked
            pResultVector = AllocateCopyInLinearityPreservingRelatedCoordSys(pi_rpCoordSys);
            }
        else
            {
            // general case
            // In such case, anything is possible such as transformation of lines into
            // curves with autocrossing points etc.
            pResultVector = AllocateCopyInGeneralRelatedCoordSys(pi_rpCoordSys);
            }

        pResultVector->SetStrokeTolerance(m_pStrokeTolerance);
        }

    return(pResultVector);
    }

//-----------------------------------------------------------------------------
// CalculateBearing
// This method returns the bearing at specified position
//-----------------------------------------------------------------------------
HGFBearing HVE2DPolygonOfSegments::CalculateBearing(const HGF2DLocation& pi_rPositionPoint,
                                                    HVE2DVector::ArbitraryDirection pi_Direction) const
    {
    HINVARIANTS;

    // The point must be located on polygon
    HASSERTSUPERDEBUG(IsPointOn(pi_rPositionPoint));

    HGFBearing  ReturnValue;

    // Check if it is the end point and direction is ALPHA
    if ((pi_rPositionPoint.IsEqualTo(HGF2DLocation(m_PolySegment.GetPosition(m_PolySegment.GetSize() - 1), GetCoordSys()), GetTolerance())) &&
        (pi_Direction == HVE2DVector::ALPHA))
        {
        // The given point is the end point of polygon ... special processing

        // Obtain bearing from last linear in complex
        HVE2DSegment TempSegment(m_PolySegment.GetPosition(m_PolySegment.GetSize() - 2),
                                 m_PolySegment.GetPosition(m_PolySegment.GetSize() - 1),
                                 GetCoordSys());
        TempSegment.SetAutoToleranceActive(IsAutoToleranceActive());
        TempSegment.SetTolerance(GetTolerance());

        ReturnValue = TempSegment.CalculateBearing(pi_rPositionPoint, pi_Direction);
        }
    else
        {
        ReturnValue = m_PolySegment.CalculateBearing(pi_rPositionPoint, pi_Direction);
        }


    return(ReturnValue);
    }


//-----------------------------------------------------------------------------
// CalculateAngularAcceleration
// This method returns the angular acceleration at specified position
//-----------------------------------------------------------------------------
double HVE2DPolygonOfSegments::CalculateAngularAcceleration(const HGF2DLocation& pi_rPositionPoint,
                                                            HVE2DVector::ArbitraryDirection pi_Direction) const
    {
    HINVARIANTS;

    // The point must be located on polygon
    HASSERTSUPERDEBUG(IsPointOn(pi_rPositionPoint));

    // Being made of segments, the only acceleration possible is 0
    return 0.0;
    }


//-----------------------------------------------------------------------------
// IsPointOn
// This method checks if the point is located on the polygon boundary
//-----------------------------------------------------------------------------
bool HVE2DPolygonOfSegments::IsPointOn(const HGF2DLocation& pi_rPoint,
                                        HVE2DVector::ExtremityProcessing pi_ExtremitityProcessing,
                                        double pi_Tolerance) const
    {
    // Note that the pi_Extremity processing value is ignored since
    // a polygon has no extremities
    return(m_PolySegment.IsPointOn(pi_rPoint, INCLUDE_EXTREMITIES, pi_Tolerance));
    }

//-----------------------------------------------------------------------------
// IsPointOnSCS
// This method checks if the point is located on the polygon boundary
//-----------------------------------------------------------------------------
bool HVE2DPolygonOfSegments::IsPointOnSCS(const HGF2DLocation& pi_rPoint,
                                           HVE2DVector::ExtremityProcessing pi_ExtremitityProcessing,
                                           double pi_Tolerance) const
    {
    HINVARIANTS;

    HPRECONDITION(GetCoordSys() == pi_rPoint.GetCoordSys());

    // Note that the pi_Extremity processing value is ignored since
    // a polygon has no extremities

    return(m_PolySegment.IsPointOnSCS(pi_rPoint, INCLUDE_EXTREMITIES, pi_Tolerance));

    }


//-----------------------------------------------------------------------------
// IsPointIn
// This method checks if the point is located inside the polygon area
//-----------------------------------------------------------------------------
bool HVE2DPolygonOfSegments::IsPointIn(const HGF2DLocation& pi_rPoint, double pi_Tolerance) const
    {
    HINVARIANTS;

    // Set tolerance
    double Tolerance = pi_Tolerance;
    if (pi_Tolerance == HVE_USE_INTERNAL_EPSILON)
        Tolerance = GetTolerance();

    // Obtain copy of points in polygon of segments coordinate system
    HGF2DLocation   ThePoint(pi_rPoint, GetCoordSys());

    // Obtain a position copy of the point
    HGF2DPosition   TheRawPoint(ThePoint.GetX(), ThePoint.GetY());

    HVE2DShape::SpatialPosition ThePointSpatialPosition;

    ThePointSpatialPosition = CalculateSpatialPositionOfPositionSameUnits(TheRawPoint, Tolerance);

    return(ThePointSpatialPosition == HVE2DShape::S_IN);

    }





//-----------------------------------------------------------------------------
// CalculateSpatialPositionOfPositionSameUnits
// This method returns the spatial position relative to shape of given point
//-----------------------------------------------------------------------------
HVE2DShape::SpatialPosition HVE2DPolygonOfSegments::CalculateSpatialPositionOfPositionSameUnits(const HGF2DPosition& pi_rPoint,
        double pi_Tolerance) const
    {
    HINVARIANTS;

    bool   PointIsOn = false;

    size_t  NumberOfCrossPoints = 0;

    // Check and set tolerance
    double Tolerance = pi_Tolerance;
    if (pi_Tolerance == HVE_USE_INTERNAL_EPSILON)
        Tolerance = GetTolerance();

    // Force calculation of extent if required
    if (!m_PolySegment.m_ExtentUpToDate)
        GetExtent();

    double X = pi_rPoint.GetX();
    double Y = pi_rPoint.GetY();


    double PolyXMin = m_PolySegment.m_Extent.GetXMin();
    double PolyXMax = m_PolySegment.m_Extent.GetXMax();
    double PolyYMin = m_PolySegment.m_Extent.GetYMin();
    double PolyYMax = m_PolySegment.m_Extent.GetYMax();

    bool Result = (((PolyXMax > X) || HDOUBLE_EQUAL(PolyXMax, X, Tolerance)) &&
                    ((PolyXMin < X) || HDOUBLE_EQUAL(PolyXMin, X, Tolerance)) &&
                    ((PolyYMax > Y) || HDOUBLE_EQUAL(PolyYMax, Y, Tolerance)) &&
                    ((PolyYMin < Y) || HDOUBLE_EQUAL(PolyYMin, Y, Tolerance)));


    if (Result)
        {

        // Evaluate the external X coordinate
        double     TheX = (PolyXMin - fabs(PolyXMin) - 1.0);

        // This could be optimised with the use of a real horizontal segment
        HGF2DLiteSegment    MyHorizontalSegment(pi_rPoint, HGF2DPosition(TheX, pi_rPoint.GetY()), Tolerance);

        HGF2DPositionCollection::const_iterator Itr = m_PolySegment.m_Points.begin();
        HGF2DPositionCollection::const_iterator   PrevItr = Itr;

        // For performance reason (the present function is called OFTEN!!!) we minimize the constructions
        // This does help for almost a full percent of performance for large polygons!!!
        HGF2DLiteSegment   TheSegment;
        TheSegment.SetTolerance (Tolerance);

        for (Itr++ ; Itr != m_PolySegment.m_Points.end() && !PointIsOn ; Itr++)
            {
            // Check if the segment can possibly intersect
            double XMin = MIN(PrevItr->GetX(), Itr->GetX());
            double XMax = MAX(PrevItr->GetX(), Itr->GetX());
            double YMin = MIN(PrevItr->GetY(), Itr->GetY());
            double YMax = MAX(PrevItr->GetY(), Itr->GetY());

            // A segment which is all farther in any dimension cannot interact
            if (((XMax > X) || HDOUBLE_EQUAL(XMax, X, Tolerance)) &&
                ((XMin < X) || HDOUBLE_EQUAL(XMin, X, Tolerance)) &&
                ((YMax > Y) || HDOUBLE_EQUAL(YMax, Y, Tolerance)) &&
                ((YMin < Y) || HDOUBLE_EQUAL(YMin, Y, Tolerance)))
                {
                // Set segment
                TheSegment.SetStartPoint (*PrevItr);
                TheSegment.SetEndPoint (*Itr);

                // Check if point is on segment
                if (!(PointIsOn = TheSegment.IsPointOn(pi_rPoint)))
                    {
                    // Check if YMin is positioned exactly of Y
                    // but segment is not horizontal
                    // There is a subtle difference here!!! Sometimes, the YMin and YMax are
                    // equal but the point is not equal to both.
                    if (HDOUBLE_EQUAL(YMin, Y, Tolerance) && !HDOUBLE_EQUAL(Y, YMax, Tolerance))
                        {
                        // There will be (counted) crossing if the X of the point at YMin
                        // is smaller than X (By the same time, ON point is checked)
                        // Exact compare is volontary ... we want the point that resulted in YMin
                        if (PrevItr->GetY() == YMin)
                            {
                            if (HDOUBLE_SMALLER(PrevItr->GetX(), X, Tolerance))
                                NumberOfCrossPoints++;
                            }
                        else
                            {
                            // The other point is necessarily exactly equal to YMin
                            HASSERT(Itr->GetY() == YMin);

                            if (HDOUBLE_SMALLER(Itr->GetX(), X, Tolerance))
                                NumberOfCrossPoints++;
                            }
                        }
                    else
                        {
                        // Possible interaction ...
                        if (TheSegment.Crosses(MyHorizontalSegment))
                            {
                            NumberOfCrossPoints++;
                            }
                        }
                    }
                }
            // A segment which is all farther than x value of point cannot interact
            else if (HDOUBLE_SMALLER_OR_EQUAL(XMin, X, Tolerance))
                {
                // The segment is possibly corectly positioned

                // if the Y values are both greater or smaller than that of point
                // cannot intersect either
                if (HDOUBLE_SMALLER(YMin, Y, Tolerance) && HDOUBLE_GREATER(YMax, Y, Tolerance))
                    {
                    // The segments possibly crosses
                    // If the maximum X is smaller than X of point, there shurely is a crossing
                    NumberOfCrossPoints++;
                    }
                else if (HDOUBLE_GREATER(YMax, Y, Tolerance) && HDOUBLE_EQUAL(YMin, Y, Tolerance) &&
                         ((HDOUBLE_EQUAL(PrevItr->GetY(), Y, Tolerance) && (PrevItr->GetX() < X)) ||
                          (HDOUBLE_EQUAL(Itr->GetY(), Y, Tolerance) && (Itr->GetX() < X))))
                    {
                    // The segment has greater Y values that point, but connects by one point in lower X
                    NumberOfCrossPoints++;
                    }
                }


            PrevItr = Itr;
            }
        }

    return (PointIsOn ? HVE2DShape::S_ON : ((NumberOfCrossPoints % 2 == 1) ? HVE2DShape::S_IN : HVE2DShape::S_OUT));
    }



//-----------------------------------------------------------------------------
// ObtainContiguousnessPoints
// Finds contiguousness points with vector
// If start point is in a contiguousness region, then this point
// will be located in first contiguousness region
//-----------------------------------------------------------------------------
size_t HVE2DPolygonOfSegments::ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                          HGF2DLocationCollection* po_pContiguousnessPoints) const
    {
    HINVARIANTS;

    double Tolerance = MIN(GetTolerance(), pi_rVector.GetTolerance());

    // We save the initial number of points
    size_t  InitialNumberOfPoints = po_pContiguousnessPoints->size();

    // For every segment of the polygon
    for(size_t Index = 1 ; Index < m_PolySegment.GetSize() ; Index++)
        {
        // Create the segment
        HVE2DSegment    CurrentSegment(m_PolySegment.GetPosition(Index - 1), m_PolySegment.GetPosition(Index), GetCoordSys());
        CurrentSegment.SetTolerance(Tolerance);

        // Check if this part is contiguous
        if (CurrentSegment.AreContiguous(pi_rVector))
            {
            // They are contiguous ... obtain contiguousness points
            HGF2DLocationCollection TempPoints;

            if (CurrentSegment.ObtainContiguousnessPoints(pi_rVector, &TempPoints) != 0)
                {
                // There is at least one pair of points
                HGF2DLocationCollection::const_iterator MyPointIterator = TempPoints.begin();

                // We check if first returned point is equal to last point obtained previously
                if (po_pContiguousnessPoints->size() > InitialNumberOfPoints)
                    {
                    // We have previous points ... compare first of new one with last of previous
                    if ((*(po_pContiguousnessPoints->rbegin())).IsEqualTo(*MyPointIterator, GetTolerance()))
                        {
                        // These two are equal ... we remove previous last one
                        po_pContiguousnessPoints->pop_back();

                        // We advance to next new point
                        MyPointIterator++;
                        }
                    }

                // We copy new points to returned list
                while (MyPointIterator != TempPoints.end())
                    {
                    po_pContiguousnessPoints->push_back(*MyPointIterator);

                    MyPointIterator++;
                    }
                }
            }
        }

    // We check if new points were found
    if (po_pContiguousnessPoints->size() > InitialNumberOfPoints)
        {
        // There are some contiguousness points ... check first and last point found
        if ((*po_pContiguousnessPoints)[InitialNumberOfPoints].IsEqualTo((*po_pContiguousnessPoints)[po_pContiguousnessPoints->size() - 1], GetTolerance()))
            {
            // The first and last ones are equal ... we must remove those ...

            // We first check if there are some remaining points found ....
            if (po_pContiguousnessPoints->size() - InitialNumberOfPoints > 2)
                {
                // There are more than 2 new contiguousness points ... the first one become the
                // previous to last
                (*po_pContiguousnessPoints)[InitialNumberOfPoints] = (*po_pContiguousnessPoints)[po_pContiguousnessPoints->size() - 2];
                }

            // We remove the last 2 entries
            po_pContiguousnessPoints->pop_back();
            po_pContiguousnessPoints->pop_back();
            }
        }


    return (po_pContiguousnessPoints->size() - InitialNumberOfPoints);
    }


//-----------------------------------------------------------------------------
// ObtainContiguousnessPointsAt
// Finds contiguousness points with vector at specified point
//-----------------------------------------------------------------------------
void HVE2DPolygonOfSegments::ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
                                                          const HGF2DLocation& pi_rPoint,
                                                          HGF2DLocation* po_pFirstContiguousnessPoint,
                                                          HGF2DLocation* po_pSecondContiguousnessPoint) const
    {
    HINVARIANTS;

    // The two vectors must be contiguous at specified point
    HASSERTSUPERDEBUG(AreContiguousAt(pi_rVector, pi_rPoint));

    // Ask complex linear to obtain contiguousness points at
    m_PolySegment.ObtainContiguousnessPointsAt(pi_rVector,
                                               pi_rPoint,
                                               po_pFirstContiguousnessPoint,
                                               po_pSecondContiguousnessPoint);

    // Check if first contiguousness point is the start point
    if ((*po_pFirstContiguousnessPoint).IsEqualTo(HGF2DLocation(m_PolySegment.GetPosition(0), GetCoordSys()), GetTolerance()))
        {
        // The contiguousness region may possibly extent in last component linear ...

        // We obtain all contiguousness points
        HGF2DLocationCollection     AllContiguousnessPoints;
        ObtainContiguousnessPoints(pi_rVector, &AllContiguousnessPoints);


        // We know from nature of method that the start point is always located
        // in first returned contiguousness region
        if (AllContiguousnessPoints.size() > 1)
            {
            *po_pFirstContiguousnessPoint = AllContiguousnessPoints[0];
            *po_pSecondContiguousnessPoint = AllContiguousnessPoints[1];
            }
        else
            {
            // This should not happen but it occasionally does ...
            HDEBUGCODE(int a = 0; a++); //  located there for breakpoint purposes
            }
        }
    }



//-----------------------------------------------------------------------------
// Crosses
// This method checks if the polygon crosses with given vector.
//-----------------------------------------------------------------------------
bool HVE2DPolygonOfSegments::Crosses(const HVE2DVector& pi_rVector) const
    {
    HINVARIANTS;

    bool   Answer = false;

    // Check if the shape is not empty
    if (m_PolySegment.GetSize() > 0)
        {
        if ((pi_rVector.GetClassID() == HVE2DPolygonOfSegments::CLASS_ID) ||
            (pi_rVector.GetClassID() == HVE2DRectangle::CLASS_ID))
            {
            //TR 217810
            HAutoPtr<HVE2DPolygonOfSegments> pPolygonOfSegments;

            if (pi_rVector.GetClassID() == HVE2DRectangle::CLASS_ID)
                {
                pPolygonOfSegments = new HVE2DPolygonOfSegments((const HVE2DRectangle&) pi_rVector);
                }
            else
                {
                pPolygonOfSegments = new HVE2DPolygonOfSegments((const HVE2DPolygonOfSegments&) pi_rVector);
                }

            if (GetCoordSys() == pi_rVector.GetCoordSys())
                {
                Answer = CrossesPolygonOfSegmentsSCS(*pPolygonOfSegments);
                }
            else
                {
                HAutoPtr<HVE2DPolygonOfSegments> pGivenPoly((HVE2DPolygonOfSegments*)pPolygonOfSegments->AllocateCopyInCoordSys(GetCoordSys()));

                Answer = CrossesPolygonOfSegmentsSCS(*pGivenPoly);
                }
            }
        else
            {
            // Check if complex linear crosses
            Answer = m_PolySegment.Crosses(pi_rVector);

            // If the complex does not cross
            if (!Answer)
                {
                HGF2DLocation   TheEndPoint(m_PolySegment.GetPosition(0), GetCoordSys());

                // It does not cross, but there still may be a crossing at auto-closing point
                // We check if end point of complex is located on vector
                // We exclude from the test the extremities of vector, since then it would not cross
                // Unfortunately this method only works if the relationship between coordinate systems is direction preserving.
                // So we first check
                if ((GetCoordSys() == pi_rVector.GetCoordSys()) || GetCoordSys()->HasDirectionPreservingRelationTo(pi_rVector.GetCoordSys()))
                    Answer = IntersectsAtSplitPoint(pi_rVector, TheEndPoint, TheEndPoint, false);
                else
                    {
                    HFCPtr<HVE2DVector> pTempVector = pi_rVector.AllocateCopyInCoordSys(GetCoordSys());
                    if (pTempVector != NULL)
                        Answer = IntersectsAtSplitPoint(*pTempVector, TheEndPoint, TheEndPoint, false);

                    }
                }
            }
        }

    return (Answer);
    }


//-----------------------------------------------------------------------------
// CrossesPolygonOfSegmentsSCS
// Test if two polygons cross.
//-----------------------------------------------------------------------------
bool HVE2DPolygonOfSegments::CrossesPolygonOfSegmentsSCS(const HVE2DPolygonOfSegments& pi_rPolygon) const
    {
    bool Answer = false;

    double Tolerance = MIN(GetTolerance(), pi_rPolygon.GetTolerance());

    if (GetExtent().OutterOverlaps(pi_rPolygon.GetExtent(), GetTolerance()))
        {
        // The polygons probably interact. We know that they cross if one of the polygons
        // has at least one point inside and one point outside the other polygon. We
        // also use middle points of the segments.

        HVE2DShape::SpatialPosition LastSelfPosition = S_ON;
        HVE2DShape::SpatialPosition LastGivenPosition = S_ON;
        HVE2DShape::SpatialPosition TempPosition;

        HGF2DPosition TempPoint;

        HGF2DPositionCollection::const_iterator SelfItr(m_PolySegment.m_Points.begin());
        HGF2DPositionCollection::const_iterator GivenItr(pi_rPolygon.m_PolySegment.m_Points.begin());

        while (!Answer && ( (SelfItr != m_PolySegment.m_Points.end()) || (GivenItr != pi_rPolygon.m_PolySegment.m_Points.end()) ))
            {
            if (SelfItr != m_PolySegment.m_Points.end())
                {
                // Test the current self point
                TempPosition = pi_rPolygon.CalculateSpatialPositionOfPosition(*SelfItr, Tolerance);

                if ((LastSelfPosition == S_IN && TempPosition == S_OUT) ||
                    (LastSelfPosition == S_OUT && TempPosition == S_IN))
                    {
                    // We have a crossing, stop
                    Answer = true;
                    }
                else
                    {
                    // Note the current point's position
                    if (LastSelfPosition == S_ON)
                        LastSelfPosition = TempPosition;

                    TempPoint = *SelfItr;
                    if (++SelfItr != m_PolySegment.m_Points.end())
                        {
                        // Try the middle point of the current segment
                        TempPosition = pi_rPolygon.CalculateSpatialPositionOfPosition(
                                           HGF2DPosition((TempPoint.GetX() + (*SelfItr).GetX()) / 2.0,
                                                         (TempPoint.GetY() + (*SelfItr).GetY()) / 2.0));

                        if ((LastSelfPosition == S_IN && TempPosition == S_OUT) ||
                            (LastSelfPosition == S_OUT && TempPosition == S_IN))
                            {
                            // We have a crossing, stop
                            Answer = true;
                            }
                        else
                            {
                            // Note the current point's position
                            if (LastSelfPosition == S_ON)
                                LastSelfPosition = TempPosition;
                            }
                        }
                    }
                }

            if (GivenItr != pi_rPolygon.m_PolySegment.m_Points.end())
                {
                // Test current given point
                TempPosition = CalculateSpatialPositionOfPosition(*GivenItr, Tolerance);

                if ((LastGivenPosition == S_IN && TempPosition == S_OUT) ||
                    (LastGivenPosition == S_OUT && TempPosition == S_IN))
                    {
                    // Found a crossing
                    Answer = true;
                    }
                else
                    {
                    // Note current position
                    if (LastGivenPosition == S_ON)
                        LastGivenPosition = TempPosition;

                    TempPoint = *GivenItr;
                    if (++GivenItr != pi_rPolygon.m_PolySegment.m_Points.end())
                        {
                        // Try the middle point of the current segment
                        TempPosition = CalculateSpatialPositionOfPosition(
                                           HGF2DPosition((TempPoint.GetX() + (*GivenItr).GetX()) / 2.0,
                                                         (TempPoint.GetY() + (*GivenItr).GetY()) / 2.0),
                                           Tolerance);

                        if ((LastGivenPosition == S_IN && TempPosition == S_OUT) ||
                            (LastGivenPosition == S_OUT && TempPosition == S_IN))
                            {
                            // We have a crossing, stop
                            Answer = true;
                            }
                        else
                            {
                            // Note the current point's position
                            if (LastGivenPosition == S_ON)
                                LastGivenPosition = TempPosition;
                            }
                        }
                    }
                }
            }

        // Use the default method if the previous one failed. The conditions are:
        //
        // 1. We didn't find a crossing yet (obvious)
        // 2. All points are not ON (both polygons). If they are, there will be no crossing.
        // 3. All points of both polygons are OUT of the other. There may be crossing...
        // 4. One of the polygons is not convex.
        //
        // So the condition is 1 && 2 && (3 || 4)
        if (!Answer && !(LastSelfPosition == S_ON && LastGivenPosition == S_ON) &&
            ((LastSelfPosition == S_OUT && LastGivenPosition == S_OUT) ||
             !IsConvex() || !pi_rPolygon.IsConvex()))
            {
            // The previous method didn't find a crossing, but there may
            // still be one. Ask the PolySegments to do the test.

            Answer = m_PolySegment.CrossesPolySegmentSCS(pi_rPolygon.m_PolySegment);

            // If they don't cross
            if (!Answer)
                {
                HGF2DLocation TheEndPoint(m_PolySegment.GetPosition(0), GetCoordSys());

                // It does not cross, but there still may be a crossing at auto-closing point
                // We check if end point of complex is located on vector
                // We exclude from the test the extremities of vector, since then it would not cross
                Answer = IntersectsAtSplitPoint(pi_rPolygon, TheEndPoint, TheEndPoint, false);
                }

            // If they don't cross
            if (!Answer)
                {
                HGF2DLocation TheEndPoint(pi_rPolygon.m_PolySegment.GetPosition(0), GetCoordSys());

                // It does not cross, but there still may be a crossing at auto-closing point
                // We check if end point of complex is located on vector
                // We exclude from the test the extremities of vector, since then it would not cross
                Answer = pi_rPolygon.IntersectsAtSplitPoint(*this, TheEndPoint, TheEndPoint, false);
                }
            }
        }

    return Answer;
    }


//-----------------------------------------------------------------------------
// Intersect
// This method intersects with given vector and returns the cross points
//-----------------------------------------------------------------------------
size_t HVE2DPolygonOfSegments::Intersect(const HVE2DVector& pi_rVector, HGF2DLocationCollection* po_pCrossPoints) const
    {
    HINVARIANTS;

    size_t NumberOfNewCrossPoints = 0;

    // Check if polygon is not empty
    if (m_PolySegment.GetSize() > 0)
        {

        // Ask linear for intersect points
        NumberOfNewCrossPoints = m_PolySegment.Intersect(pi_rVector, po_pCrossPoints);

        // Now it is quite possible that a vector crosses at the junction point between
        // complex extremities. It is also possible that the vector is contiguous
        // over this junction and does cross somewhere

        HGF2DLocation   AutoClosingPoint(m_PolySegment.GetPosition(0), GetCoordSys());

        if (IntersectsAtSplitPoint(pi_rVector, AutoClosingPoint, AutoClosingPoint, false))
            {
            po_pCrossPoints->push_back(AutoClosingPoint);

            NumberOfNewCrossPoints++;
            }
        }

    return(NumberOfNewCrossPoints);
    }




//-----------------------------------------------------------------------------
// Decompose
// PRIVATE
// This method decomposes the different parts of two polygon of segments interaction
//-----------------------------------------------------------------------------
void HVE2DPolygonOfSegments::Decompose(const HVE2DPolygonOfSegments& pi_rPolygon,
                                       const HGF2DPositionCollection& pi_rPoly1,
                                       const HGF2DPositionCollection& pi_rPoly2,
                                       HVE2DPolygonOfSegments::DecomposeOperation pi_Operation,
                                       HoleList& pi_rListOfShapes) const
    {
    HINVARIANTS;

    HPRECONDITION(GetCoordSys() == pi_rPolygon.GetCoordSys());

    bool AllOnPoints = false;

    switch (pi_Operation)
        {
        case HVE2DPolygonOfSegments::DIFF :
            // Perform super scan in DIFF mode
            SuperScan(pi_rPolygon, pi_rPoly1, pi_rPoly2, false, true, true, false, pi_rListOfShapes, &AllOnPoints);
            break;

        case HVE2DPolygonOfSegments::DIFFFROM :
            // Perform super scan in DIFF mode
            SuperScan(pi_rPolygon, pi_rPoly1, pi_rPoly2, true, false, false, true, pi_rListOfShapes, &AllOnPoints);

            // If there are no shapes in result either both are equal or self is
            // completely outside of the other, If they are equal then void is the result
            // , otherwise given is the result
            // Since the shapes interact, it is odd that no result appear ... inverse operation
            if (pi_rListOfShapes.size() == 0)
                {
                // Perform super scan in DIFF mode
                pi_rPolygon.SuperScan(*this, pi_rPoly2, pi_rPoly1, false, true, true, false, pi_rListOfShapes, &AllOnPoints);

                // If the result is null, then effectively, they are equal ...
                // and this is the result.
                }
            break;

        case HVE2DPolygonOfSegments::INTERSECT:

            // Perform superscan in intersect mode
            SuperScan(pi_rPolygon, pi_rPoly1, pi_rPoly2, true, true, true, true, pi_rListOfShapes, &AllOnPoints);

            // If there are no shapes in result either both are equal or self is
            // completely outside of the other, If they are equal then either one is
            // the result, otherwise the including part is the result
            if (pi_rListOfShapes.size() == 0)
                {
                // SuperScan is now way much faster than calculate spatial positon
                // So we inverse operations
                pi_rPolygon.SuperScan(*this, pi_rPoly2, pi_rPoly1, true, true, true, true, pi_rListOfShapes, &AllOnPoints);


                if (pi_rListOfShapes.size() == 0)
                    {
                    // In some extremely RARE cases such as the following:
                    // Self = -1.4901161193848E-8, -1.4901161193848E-8
                    //        -1.4901161193848E-8,255.99999998509884010
                    //        255.99999998509884010 , 256.0
                    //        255.99999998509884010 , 0.0 + close
                    // Given = -1.4901274880685E-8, -2.2737367544323E-13
                    //         -1.4901274880685E-8, 256.0
                    //         255.99999998509884010, 256.0
                    //         255.99999998509884010, 0.0 + close
                    // The operation does not work ... it is because, the
                    // shapes are super close to be identical ...
                    // we take either one
                    // It may also occur that the shapes are dizsjoint but very closely flirting
                    // Resolution of the two cases is performed by checking if all points are on.
                    if (AllOnPoints)
                        pi_rListOfShapes.push_back((HVE2DPolygonOfSegments*)pi_rPolygon.Clone());
                    else
                        pi_rListOfShapes.push_back(new HVE2DVoidShape(GetCoordSys()));
                    }
                }
            break;

        case HVE2DPolygonOfSegments::UNION:
            // Perform superscan in union mode
            SuperScan(pi_rPolygon, pi_rPoly1, pi_rPoly2, false, true, false, true, pi_rListOfShapes, &AllOnPoints);
            // If there are no shapes in result either both are equal or self is
            // completely included in the other, If they are equal then either one is
            // the result, otherwise the excluding part is the result
            if (pi_rListOfShapes.size() == 0)
                {
                // Obtain spatial position of self in given
                HVE2DShape::SpatialPosition SelfSpatialPosition = pi_rPolygon.CalculateSpatialPositionOfPolygonOfSegmentsSCS(*this);

                // If the spatial position is IN
                if (SelfSpatialPosition == HVE2DShape::S_IN)
                    {
                    // Self is completely included in the other ... given is the result
                    pi_rListOfShapes.push_back((HVE2DPolygonOfSegments*)pi_rPolygon.Clone());
                    }
                else
                    {
                    // Either self is ON (Equal) or out ... In both case, self is the result
                    pi_rListOfShapes.push_back((HVE2DPolygonOfSegments*)Clone());
                    }
                }

            break;

        }
    }


//-----------------------------------------------------------------------------
// DifferentiateFromShapeSCS
// This method create a new shape as the difference between self and given.
// The coordinate system of both shapes must be the same
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DPolygonOfSegments::DifferentiateFromShapeSCS(const HVE2DShape& pi_rShape) const
    {
    HINVARIANTS;

    HPRECONDITION(GetCoordSys() == pi_rShape.GetCoordSys());

    HVE2DShape*     pMyResultShape;

    if (IsEmpty())
        {
        // Since self is empty, the result is given
        pMyResultShape = (HVE2DShape*)pi_rShape.Clone();
        }
    else if (pi_rShape.IsEmpty())
        {
        // Given shape is empty, the result is empty
        pMyResultShape = new HVE2DVoidShape(GetCoordSys());
        }
    // We separate the process depending on the complexity of given shape
    else if (!pi_rShape.IsSimple())
        {
        // The given is either a complex shape or a holed shape, thus
        // the process is transfered to them
        pMyResultShape = pi_rShape.DifferentiateShapeSCS(*this);
        }
    else
        {
        if(pi_rShape.GetShapeType() == HVE2DRectangle::CLASS_ID)
            pMyResultShape = DifferentiateFromPolygonSCS(HVE2DPolygonOfSegments(*(HVE2DRectangle*)(&pi_rShape)));
        else if (pi_rShape.GetShapeType() == HVE2DPolygonOfSegments::CLASS_ID)
            pMyResultShape = DifferentiateFromPolygonSCS((*(HVE2DPolygonOfSegments*)(&pi_rShape)));
        else
            pMyResultShape = pi_rShape.DifferentiateShapeSCS(*this);
        }

    return (pMyResultShape);
    }

//-----------------------------------------------------------------------------
// DifferentiateFromPolygonSCS
// PRIVATE METHOD
// This method create a new shape as the difference between self and given.
// The coordinate system of both shapes must be the same
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DPolygonOfSegments::DifferentiateFromPolygonSCS(const HVE2DPolygonOfSegments& pi_rPolygon) const
    {
    HINVARIANTS;

    HPRECONDITION(GetCoordSys() == pi_rPolygon.GetCoordSys());

    HPRECONDITION(!IsEmpty() && !pi_rPolygon.IsEmpty());

    HVE2DShape*     pMyResultShape;

    // We separate the process depending on the fact that they cross or not

    // We first compare if their extent overlap
    if (!GetExtent().InnerOverlaps(pi_rPolygon.GetExtent(), MIN(GetTolerance(), pi_rPolygon.GetTolerance())))
        {
        // The two simple shape cannot possibly intersect ... therefore, the difference is given
        if (pi_rPolygon.RepresentsARectangle())
            pMyResultShape = (HVE2DShape*)pi_rPolygon.GenerateCorrespondingRectangle();
        else
            pMyResultShape = (HVE2DShape*)pi_rPolygon.Clone();
        }
    else
        {
        HGF2DPositionCollection SelfPolyPoints;
        HGF2DPositionCollection GivenPolyPoints;

        if (InteractsWith(pi_rPolygon, &SelfPolyPoints, &GivenPolyPoints, HVE2DPolygonOfSegments::DIFFFROM))
            {
            // The shape interact with each other ... long process
            pMyResultShape = DifferentiateFromCrossingPolygonSCS(pi_rPolygon, SelfPolyPoints, GivenPolyPoints);
            }
        else
            {
            // Even though their extent is not disjoint, they may still be disjoint
            // We therefore evaluate their spatial position
            HVE2DShape::SpatialPosition     TheGivenPosition = CalculateSpatialPositionOfNonCrossingPolygonOfSegmentsSCS(pi_rPolygon);

            if ((TheGivenPosition == HVE2DShape::S_IN) || (TheGivenPosition == HVE2DShape::S_ON))
                {
                // The given is completely located inside self ... the result is therefore empty
                pMyResultShape = new HVE2DVoidShape(GetCoordSys());

                }
            else
                {
                // The given shape is OUT, but this does not imply that self is also OUT ... check
                HVE2DShape::SpatialPosition     TheSelfPosition = pi_rPolygon.CalculateSpatialPositionOfNonCrossingPolygonOfSegmentsSCS(*this);

                if (TheSelfPosition == HVE2DShape::S_OUT)
                    {
                    // The two simple shape are disjoint ... therefore, the difference is given
                    if (pi_rPolygon.RepresentsARectangle())
                        pMyResultShape = (HVE2DShape*)pi_rPolygon.GenerateCorrespondingRectangle();
                    else
                        pMyResultShape = (HVE2DShape*)pi_rPolygon.Clone();
                    }
                else
                    {
                    // Self cannot be PARTIALY_IN nor ON the given
                    // (since the given is not PARTIALY_IN nor ON self), then
                    // Self is located IN the given ...

                    // The result is therefore a holed polygon
                    // with given as outter shape and self as hole
                    HVE2DHoledShape* pMyResultHoledShape = new HVE2DHoledShape(pi_rPolygon);
                    pMyResultHoledShape->AddHole(*this);

                    pMyResultShape = pMyResultHoledShape;
                    }
                }
            }
        }

    return (pMyResultShape);

    }


//-----------------------------------------------------------------------------
// DifferentiateFromCrossingPolygonSCS
// PRIVATE METHOD
// This method create a new shape as the difference between self and given.
// The coordinate system of both shapes must be the same
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DPolygonOfSegments::DifferentiateFromCrossingPolygonSCS(const HVE2DPolygonOfSegments& pi_rPolygon,
                                                                        const HGF2DPositionCollection& pi_rPoly1,
                                                                        const HGF2DPositionCollection& pi_rPoly2) const
    {
    HINVARIANTS;

    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rPolygon.GetCoordSys());

    // The two shapes must cross each other or at least be contiguous.
    HASSERTSUPERDEBUG(Crosses(pi_rPolygon) || AreContiguous(pi_rPolygon));

    // neither shape must be empty
    HPRECONDITION(!IsEmpty() && !pi_rPolygon.IsEmpty());

    HVE2DShape*     pMyResultShape;

    // Create recipient list
    HVE2DShape::HoleList   MyListOfPolygons;

    // Perform decomposition process
    Decompose(pi_rPolygon, pi_rPoly1, pi_rPoly2, HVE2DPolygonOfSegments::DIFFFROM, MyListOfPolygons);

    // In the case of a differentiation, all the different shapes returned are disjoint
    if (MyListOfPolygons.size() == 0)
        {
        // The two shapes were either identical or ...
        pMyResultShape = new HVE2DVoidShape(GetCoordSys());
        }
    else if (MyListOfPolygons.size() > 1)
        {
        // Generate a complex shape
        pMyResultShape = new HVE2DComplexShape(MyListOfPolygons);
        }
    else
        {
        pMyResultShape = (HVE2DShape*)(*MyListOfPolygons.begin())->Clone();
        }

    // Destroy list
    HVE2DShape::HoleList::iterator    MyIterator = MyListOfPolygons.begin();

    while (MyIterator != MyListOfPolygons.end())
        {
        delete *MyIterator;

        MyIterator++;
        }

    return (pMyResultShape);

    }



//-----------------------------------------------------------------------------
// DifferentiateShapeSCS
// This method create a new shape as the difference between self and given.
// The coordinate system of both shapes must be the same
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DPolygonOfSegments::DifferentiateShapeSCS(const HVE2DShape& pi_rShape) const
    {
    HINVARIANTS;

    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rShape.GetCoordSys());

    HVE2DShape*     pMyResultShape;

    // We check if self is empty
    if (IsEmpty())
        {
        // Since self is empty ... result is empty
        pMyResultShape = new HVE2DVoidShape(GetCoordSys());
        }
    else if (pi_rShape.IsEmpty())
        {
        // Since given is empty, result is self
        pMyResultShape = (HVE2DShape*)Clone();
        }
    // We separate the process depending on the complexity of given shape
    else if (!pi_rShape.IsSimple())
        {
        // The given is either a complex shape or a holed shape, thus
        // the process is transfered to them
        pMyResultShape = pi_rShape.DifferentiateFromShapeSCS(*this);
        }
    else
        {
        if (pi_rShape.GetShapeType() == HVE2DRectangle::CLASS_ID)
            pMyResultShape = DifferentiatePolygonSCS(HVE2DPolygonOfSegments(*(HVE2DRectangle*)(&pi_rShape)));
        else if (pi_rShape.GetShapeType() == HVE2DPolygonOfSegments::CLASS_ID)
            pMyResultShape = DifferentiatePolygonSCS((*(HVE2DPolygonOfSegments*)(&pi_rShape)));
        else
            pMyResultShape = pi_rShape.DifferentiateFromShapeSCS(*this);
        }

    return (pMyResultShape);
    }


//-----------------------------------------------------------------------------
// DifferentiatePolygonSCS
// PRIVATE METHOD
// This method create a new shape as the difference between self and given.
// The coordinate system of both shapes must be the same
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DPolygonOfSegments::DifferentiatePolygonSCS(const HVE2DPolygonOfSegments& pi_rPolygon) const
    {
    HINVARIANTS;

    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rPolygon.GetCoordSys());

    // The shapes cannot be empty
    HPRECONDITION(!IsEmpty() && !pi_rPolygon.IsEmpty());

    HVE2DShape*     pMyResultShape;

    // We separate the process depending on the fact that they cross or not
    // We  compare if their extent overlap
    if (!GetExtent().InnerOverlaps(pi_rPolygon.GetExtent(), MIN(GetTolerance(), pi_rPolygon.GetTolerance())))
        {
        // The two simple shape cannot possibly intersect ... therefore, the difference is self
        pMyResultShape = (HVE2DShape*)Clone();
        }
    else
        {
        HGF2DPositionCollection SelfPolyPoints;
        HGF2DPositionCollection GivenPolyPoints;

        if (InteractsWith(pi_rPolygon, &SelfPolyPoints, &GivenPolyPoints, HVE2DPolygonOfSegments::DIFF))
            {
            // The shape intersect each other ... long process
            pMyResultShape = DifferentiateCrossingPolygonSCS(pi_rPolygon, SelfPolyPoints, GivenPolyPoints);
            }
        else
            {
            // Even though their extent is not disjoint, they may still be disjoint
            // We therefore evaluate their spatial position
            HVE2DShape::SpatialPosition     TheGivenPosition = CalculateSpatialPositionOfNonCrossingPolygonOfSegmentsSCS(pi_rPolygon);

            if (TheGivenPosition == HVE2DShape::S_ON)
                {
                // The two shapes are identical
                pMyResultShape = new HVE2DVoidShape(GetCoordSys());
                }
            else if (TheGivenPosition == HVE2DShape::S_IN)
                {
                // The given is completely located inside self ...
                //the result is therefore a holed polygon
                // with self as outter shape and given as hole
                HVE2DHoledShape* pMyResultHoledShape = new HVE2DHoledShape(*this);
                pMyResultHoledShape->AddHole(pi_rPolygon);

                pMyResultShape = pMyResultHoledShape;
                }
            else
                {
                // The given shape is OUT, but this does not imply that self is also OUT ... check
                HVE2DShape::SpatialPosition     TheSelfPosition = pi_rPolygon.CalculateSpatialPositionOfNonCrossingPolygonOfSegmentsSCS(*this);

                if (TheSelfPosition == HVE2DShape::S_OUT)
                    {
                    // The two simple shape are disjoint ... therefore, the difference is self
                    pMyResultShape = (HVE2DShape*)Clone();
                    }
                else
                    {
                    // Self cannot be PARTIALY_IN nor ON the given
                    // (since the given is not PARTIALY_IN nor ON self), then
                    // Self is located IN the given ... the result is therefore empty
                    pMyResultShape = new HVE2DVoidShape(GetCoordSys());
                    }
                }
            }
        }

    return (pMyResultShape);

    }

//-----------------------------------------------------------------------------
// DifferentiateCrossingPolygonSCS
// PRIVATE METHOD
// This method create a new shape as the difference between self and given.
// The coordinate system of both shapes must be the same
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DPolygonOfSegments::DifferentiateCrossingPolygonSCS(const HVE2DPolygonOfSegments& pi_rPolygon,
                                                                    const HGF2DPositionCollection& pi_rPoly1,
                                                                    const HGF2DPositionCollection& pi_rPoly2) const
    {
    HINVARIANTS;

    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rPolygon.GetCoordSys());

    // The shapes must cross each other or at least be contiguous.
    // We have a case where there is interaction but not contiguousness nor crossing (see test program line 7388)
    //    HPRECONDITION(Crosses(pi_rPolygon) || AreContiguous(pi_rPolygon));

    // Neither shapes must be empty
    HPRECONDITION(!IsEmpty() && !pi_rPolygon.IsEmpty());

    HVE2DShape*     pMyResultShape;


    // Create recipient list
    HVE2DShape::HoleList   MyListOfPolygons;

    bool SpecialProcessing  = false;
    try
        {
        // Perform decomposition process
        Decompose(pi_rPolygon, pi_rPoly1, pi_rPoly2, HVE2DPolygonOfSegments::DIFF, MyListOfPolygons);
        }
    catch (HVEDecompositionException&)
        {
        SpecialProcessing = true;
        }

    if (SpecialProcessing)
        {
        HFCPtr<HVE2DShape> pNewShape1 = static_cast<HVE2DShape*>(this->Clone());
        HFCPtr<HVE2DShape> pNewShape2 = static_cast<HVE2DShape*>(pi_rPolygon.Clone());

        pNewShape1->SetAutoToleranceActive(false);
        pNewShape1->SetTolerance(this->GetTolerance() * 2.0);
        pNewShape2->SetAutoToleranceActive(false);
        pNewShape2->SetTolerance(pi_rPolygon.GetTolerance() * 2.0);

        pMyResultShape = pNewShape1->DifferentiateShape(*pNewShape2);
        }
    else
        {

        // In the case of a differentiation, all the different shapes returned are disjoint
        if (MyListOfPolygons.size() == 0)
            {
            pMyResultShape = new HVE2DVoidShape(GetCoordSys());
            }
        else if (MyListOfPolygons.size() > 1)
            {
            // Generate a complex shape
            pMyResultShape = new HVE2DComplexShape(MyListOfPolygons);
            }
        else
            {
            pMyResultShape = (HVE2DShape*)(*MyListOfPolygons.begin())->Clone();
            }
        }

    // Destroy list
    HVE2DShape::HoleList::iterator    MyIterator = MyListOfPolygons.begin();

    while (MyIterator != MyListOfPolygons.end())
        {
        delete *MyIterator;

        MyIterator++;
        }

    return (pMyResultShape);


    }


//-----------------------------------------------------------------------------
// IntersectShapeSCS
// This method create a new shape as the intersection between self and given.
// The coordinate system of both shapes must be the same
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DPolygonOfSegments::IntersectShapeSCS(const HVE2DShape& pi_rShape) const
    {
    HINVARIANTS;

    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rShape.GetCoordSys());

    HVE2DShape*     pMyResultShape;

    if (IsEmpty() || pi_rShape.IsEmpty())
        {
        // Since at least one shape is empty, the result is empty
        pMyResultShape = new HVE2DVoidShape(GetCoordSys());
        }
    // We separate the process depending on the complexity of given shape
    else if (!pi_rShape.IsSimple())
        {
        // The given is either a complex shape or a holed shape, thus
        // the process is transfered to them
        pMyResultShape = pi_rShape.IntersectShapeSCS(*this);
        }
    else
        {
        if (pi_rShape.GetShapeType() == HVE2DRectangle::CLASS_ID)
            pMyResultShape = IntersectPolygonSCS(HVE2DPolygonOfSegments(*(HVE2DRectangle*)(&pi_rShape)));
        else if (pi_rShape.GetShapeType() == HVE2DPolygonOfSegments::CLASS_ID)
            pMyResultShape = IntersectPolygonSCS((*(HVE2DPolygonOfSegments*)(&pi_rShape)));
        else
            pMyResultShape = pi_rShape.IntersectShapeSCS(*this);
        }

    return (pMyResultShape);

    }


//-----------------------------------------------------------------------------
// IntersectPolygonSCS
// PRIVATE METHOD
// This method create a new shape as the intersection between self and given.
// The coordinate system of both shapes must be the same
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DPolygonOfSegments::IntersectPolygonSCS(const HVE2DPolygonOfSegments& pi_rPolygon) const
    {
    HINVARIANTS;

    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rPolygon.GetCoordSys());

    // Neither shape should be empty
    HPRECONDITION(!IsEmpty() && !pi_rPolygon.IsEmpty());

    HVE2DShape* pMyResultShape = 0;

    // We separate the process depending on the fact that they cross or not

    // We first compare if their extent overlap
    if (!GetExtent().InnerOverlaps(pi_rPolygon.GetExtent(), MIN(GetTolerance(), pi_rPolygon.GetTolerance())))
        {
        // The two simple shape cannot possibly intersect ... therefore, the intersection is empty
        pMyResultShape = new HVE2DVoidShape(GetCoordSys());
        }
    else
        {
        HGF2DPositionCollection SelfPolyPoints;
        HGF2DPositionCollection GivenPolyPoints;
        bool ContiguousInteraction;

        if (InteractsWith(pi_rPolygon, &SelfPolyPoints, &GivenPolyPoints, HVE2DPolygonOfSegments::INTERSECT, true, &ContiguousInteraction))
            {
            // The shape intersect each other ... long process
            // We append contiguousness points if any
            pMyResultShape = IntersectCrossingPolygonSCS(pi_rPolygon, SelfPolyPoints, GivenPolyPoints);
            }
        else
            {
            // Even though their extent is not disjoint, they may still be disjoint
            // We therefore evaluate their spatial position
            HVE2DShape::SpatialPosition     TheGivenPosition;

            // Even if there is no cross interaction, the fact that shapes are contigous
            // indicate that it is possible for the InteractsWith method to be erroneous
            // and thus the complete spatial calculation is used
            if (ContiguousInteraction)
                TheGivenPosition = CalculateSpatialPositionOf(pi_rPolygon);
            else
                TheGivenPosition = CalculateSpatialPositionOfNonCrossingPolygonOfSegmentsSCS(pi_rPolygon);

            if ((TheGivenPosition == HVE2DShape::S_IN) || (TheGivenPosition == HVE2DShape::S_ON))
                {
                // The given is completely located inside self ... the result is therefore given
                if (pi_rPolygon.RepresentsARectangle())
                    pMyResultShape = (HVE2DShape*)pi_rPolygon.GenerateCorrespondingRectangle();
                else
                    pMyResultShape = (HVE2DShape*)pi_rPolygon.Clone();
                }
            else if (TheGivenPosition == HVE2DShape::S_OUT)
                {
                // The given shape is OUT, but this does not imply that self is also OUT ... check
                HVE2DShape::SpatialPosition     TheSelfPosition;
                if (ContiguousInteraction)
                    TheSelfPosition = pi_rPolygon.CalculateSpatialPositionOf(*this);
                else
                    TheSelfPosition = pi_rPolygon.CalculateSpatialPositionOfNonCrossingPolygonOfSegmentsSCS(*this);

                if (TheSelfPosition == HVE2DShape::S_OUT)
                    {
                    // The two simple shape are disjoint ... therefore, the intersection is empty
                    pMyResultShape = new HVE2DVoidShape(GetCoordSys());
                    }
                else if (TheSelfPosition == HVE2DShape::S_IN)
                    {
                    // Self cannot be PARTIALY_IN nor ON the given
                    // (since the given is not PARTIALY_IN nor ON self), then
                    // Self is located IN the given ... the result is therefore self
                    pMyResultShape = (HVE2DShape*)Clone();
                    }
                else
                    {
                    // This situation is technically impossible ...
                    // We have a special case where the tolerance appears to be too small
                    // We will thus increase it and reprocess.
                    HFCPtr<HVE2DShape> pNewShape1 = static_cast<HVE2DShape*>(this->Clone());
                    HFCPtr<HVE2DShape> pNewShape2 = static_cast<HVE2DShape*>(pi_rPolygon.Clone());

                    pNewShape1->SetAutoToleranceActive(false);
                    pNewShape1->SetTolerance(this->GetTolerance() * 2.0);
                    pNewShape2->SetAutoToleranceActive(false);
                    pNewShape2->SetTolerance(pi_rPolygon.GetTolerance() * 2.0);

                    pMyResultShape = pNewShape1->IntersectShape(*pNewShape2);
                    }
                }
            else
                {
                //This should never occur in ordinary conditions
                // We have a special case where the tolerance appears to be too small
                // We will thus increase it and reprocess.
                HFCPtr<HVE2DShape> pNewShape1 = static_cast<HVE2DShape*>(this->Clone());
                HFCPtr<HVE2DShape> pNewShape2 = static_cast<HVE2DShape*>(pi_rPolygon.Clone());

                pNewShape1->SetAutoToleranceActive(false);
                pNewShape1->SetTolerance(this->GetTolerance() * 2.0);
                pNewShape2->SetAutoToleranceActive(false);
                pNewShape2->SetTolerance(pi_rPolygon.GetTolerance() * 2.0);

                pMyResultShape = pNewShape1->IntersectShape(*pNewShape2);

                }
            }
        }

    return (pMyResultShape);

    }


//-----------------------------------------------------------------------------
// IntersectCrossingPolygonSCS
// PRIVATE METHOD
// This method create a new shape as the intersection between self and given.
// The coordinate system of both shapes must be the same
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DPolygonOfSegments::IntersectCrossingPolygonSCS(const HVE2DPolygonOfSegments& pi_rPolygon,
                                                                const HGF2DPositionCollection& pi_rPoly1,
                                                                const HGF2DPositionCollection& pi_rPoly2) const
    {
    HINVARIANTS;

    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rPolygon.GetCoordSys());

    // The two shapes must cross or at least be contiguous
    // This precondition should be verified. However, there are cases where
    // the test fails, but the result is still good. Crosses or AreContiguous
    // must have a slight problem in very special cases. We will assume that the
    // caller has verified the conditions for us...
//    HASSERTDUMP2(Crosses(pi_rPolygon) || AreContiguous(pi_rPolygon), *this, pi_rPolygon);

    // Neither shape should be empty
    HPRECONDITION(!IsEmpty() && !pi_rPolygon.IsEmpty());



    HVE2DShape*     pMyResultShape;


    // Create recipient list
    HVE2DShape::HoleList   MyListOfPolygons;

    // Perform decomposition process
    // Patch from AlainR to solve TR 75690
    bool SpecialProcessing  = false;
    try
        {
        // Perform decomposition process
        Decompose(pi_rPolygon, pi_rPoly1, pi_rPoly2, HVE2DPolygonOfSegments::INTERSECT, MyListOfPolygons);
        }
    catch (HFCException&)
        {
        SpecialProcessing = true;
        }
    if (SpecialProcessing)
        {
        HFCPtr<HVE2DShape> pNewShape1 = static_cast<HVE2DShape*>(this->Clone());
        HFCPtr<HVE2DShape> pNewShape2 = static_cast<HVE2DShape*>(pi_rPolygon.Clone());

        pNewShape1->SetAutoToleranceActive(false);
        pNewShape1->SetTolerance(this->GetTolerance() * 2.0);
        pNewShape2->SetAutoToleranceActive(false);
        pNewShape2->SetTolerance(pi_rPolygon.GetTolerance() * 2.0);

        pMyResultShape = pNewShape1->IntersectShape(*pNewShape2);
        }
    else
        {
        // There should always be at least one shape in result
        HPOSTCONDITION(MyListOfPolygons.size() != 0);

        // In the case of a intersection, all the different shapes returned are disjoint
        if (MyListOfPolygons.size() > 1)
            {
            // Generate a complex shape
            pMyResultShape = new HVE2DComplexShape(MyListOfPolygons);
            }
        else
            {
            pMyResultShape = (HVE2DShape*)(*MyListOfPolygons.begin())->Clone();
            }

        // Destroy list
        HVE2DShape::HoleList::iterator    MyIterator = MyListOfPolygons.begin();

        while (MyIterator != MyListOfPolygons.end())
            {
            delete *MyIterator;

            MyIterator++;
            }
        }

    return (pMyResultShape);
    }




//-----------------------------------------------------------------------------
// UnifyShapeSCS
// This method create a new shape as the union between self and given.
// The coordinate system of both shapes must be the same
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DPolygonOfSegments::UnifyShapeSCS(const HVE2DShape& pi_rShape) const
    {
    HINVARIANTS;

    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rShape.GetCoordSys());

    HVE2DShape*     pMyResultShape;

    if (IsEmpty())
        {
        // Since self is empty ... result is given
        pMyResultShape = (HVE2DShape*)pi_rShape.Clone();
        }
    else if (pi_rShape.IsEmpty())
        {
        // Since given is empty, result is self
        pMyResultShape = (HVE2DShape*)Clone();
        }
    // We separate the process depending on the complexity of given shape
    else if (!pi_rShape.IsSimple())
        {
        // The given is either a complex shape or a holed shape, thus
        // the process is transfered to them
        pMyResultShape = pi_rShape.UnifyShapeSCS(*this);
        }
    else
        {
        if (pi_rShape.GetShapeType() == HVE2DRectangle::CLASS_ID)
            pMyResultShape = UnifyPolygonSCS(HVE2DPolygonOfSegments(*(HVE2DRectangle*)(&pi_rShape)));
        else if (pi_rShape.GetShapeType() == HVE2DPolygonOfSegments::CLASS_ID)
            pMyResultShape = UnifyPolygonSCS((*(HVE2DPolygonOfSegments*)(&pi_rShape)));
        else
            pMyResultShape = pi_rShape.UnifyShapeSCS(*this);
        }

    return (pMyResultShape);

    }

//-----------------------------------------------------------------------------
// UnifyPolygonSCS
// PRIVATE METHOD
// This method create a new shape as the union between self and given.
// The coordinate system of both shapes must be the same
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DPolygonOfSegments::UnifyPolygonSCS(const HVE2DPolygonOfSegments& pi_rPolygon) const
    {
    HINVARIANTS;

    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rPolygon.GetCoordSys());

    // Neither shape should be empty
    HPRECONDITION(!IsEmpty() && !pi_rPolygon.IsEmpty());


    HVE2DShape*     pMyResultShape;

    // We separate the process depending on the fact that they cross or not

    // We first compare if their extent overlap
    if (!GetExtent().OutterOverlaps(pi_rPolygon.GetExtent(), MIN(GetTolerance(), pi_rPolygon.GetTolerance())) && !AreContiguous(pi_rPolygon))
        {
        // The two simple shape cannot possibly intersect ... therefore, the union is a
        // complex shape containing both of them
        HVE2DComplexShape* pMyResultComplexShape = new HVE2DComplexShape(GetCoordSys());

        // Append to this complex shape the two simple shapes
        pMyResultComplexShape->AddShape(*this);
        pMyResultComplexShape->AddShape(pi_rPolygon);

        pMyResultShape = pMyResultComplexShape;
        }
    else
        {
        HGF2DPositionCollection SelfPolyPoints;
        HGF2DPositionCollection GivenPolyPoints;

        if (InteractsWith(pi_rPolygon, &SelfPolyPoints, &GivenPolyPoints, HVE2DPolygonOfSegments::UNION))
            {
            // The shape intersect each other ... long process
            // We append contiguousness points if any
            pMyResultShape = UnifyCrossingPolygonSCS(pi_rPolygon, SelfPolyPoints, GivenPolyPoints);
            }
        else
            {

            // Even though their extent is not disjoint, they may still be disjoint
            // We therefore evaluate their spatial position
            HVE2DShape::SpatialPosition     TheGivenPosition = CalculateSpatialPositionOfNonCrossingPolygonOfSegmentsSCS(pi_rPolygon);

            if (TheGivenPosition == HVE2DShape::S_IN || TheGivenPosition == HVE2DShape::S_ON)
                {
                // The given is completely located inside self ... the result is therefore self
                pMyResultShape = (HVE2DShape*) Clone();
                }
            else
                {
                // The given shape is OUT, but this does not imply that self is also OUT ... check
                HVE2DShape::SpatialPosition     TheSelfPosition = pi_rPolygon.CalculateSpatialPositionOfNonCrossingPolygonOfSegmentsSCS(*this);

                if (TheSelfPosition == HVE2DShape::S_OUT)
                    {
                    // The two shapes are out of each other
                    // The two simple shape are disjoint ... therefore, the union is a
                    // complex shape containing both of them
                    HVE2DComplexShape* pMyResultComplexShape = new HVE2DComplexShape(GetCoordSys());

                    // Append to this complex shape the two simple shapes
                    pMyResultComplexShape->AddShape(*this);
                    pMyResultComplexShape->AddShape(pi_rPolygon);

                    pMyResultShape = pMyResultComplexShape;
                    }
                else
                    {
                    // Self cannot be PARTIALY_IN nor ON the given
                    // (since the given is not PARTIALY_IN nor ON self), then
                    // Self is located IN the given ... the result is therefore the given
                    pMyResultShape = (HVE2DShape*) pi_rPolygon.Clone();
                    }
                }
            }
        }

    return (pMyResultShape);

    }


//-----------------------------------------------------------------------------
// UnifyCrossingPolygonSCS
// PRIVATE METHOD
// This method create a new shape as the union between self and given.
// The coordinate system of both shapes must be the same
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DPolygonOfSegments::UnifyCrossingPolygonSCS(const HVE2DPolygonOfSegments& pi_rPolygon,
                                                            const HGF2DPositionCollection& pi_rPoly1,
                                                            const HGF2DPositionCollection& pi_rPoly2) const
    {
    HINVARIANTS;

    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rPolygon.GetCoordSys());

    // The shapes should cross or be contiguous
// We have a case where there is interaction but not contiguousness nor crossing (see test program line 7388)
//    HPRECONDITION(Crosses(pi_rPolygon) || AreContiguous(pi_rPolygon));

    // Neither shape should be empty
    HPRECONDITION(!IsEmpty() && !pi_rPolygon.IsEmpty());

    HVE2DShape*     pMyResultShape;


    // Create recipient list
    HVE2DShape::HoleList   MyListOfPolygons;

    bool SpecialProcessing = FALSE;

    try
    	{
        // Perform decomposition process
        Decompose(pi_rPolygon, pi_rPoly1, pi_rPoly2, HVE2DPolygonOfSegments::UNION, MyListOfPolygons);
    	}
    catch (HFCException&)
    	{
        SpecialProcessing = TRUE;
    	}

    if (SpecialProcessing)
    	{
        HFCPtr<HVE2DShape> pNewShape1 = static_cast<HVE2DShape*>(this->Clone());
        HFCPtr<HVE2DShape> pNewShape2 = static_cast<HVE2DShape*>(pi_rPolygon.Clone());
    
        pNewShape1->SetAutoToleranceActive(FALSE);
        pNewShape1->SetTolerance(this->GetTolerance() * 2.0);
        pNewShape2->SetAutoToleranceActive(FALSE);
        pNewShape2->SetTolerance(pi_rPolygon.GetTolerance() * 2.0);
    
        pMyResultShape = pNewShape1->UnifyShape(*pNewShape2);
    	}
    else
    	{
        // There should always be at least one shape in result
        HPOSTCONDITION(MyListOfPolygons.size() != 0);

        // In the case of a union, if there are more than one simple shape returned, then only one is the outter shape
        // and the rest are holes
        if (MyListOfPolygons.size() > 1)
        	{
       	 	bool Found = false;

            // There are many shapes ... find the one containing the others
            // As sson as one contain any other, it has been found
            HVE2DShape::SimpleShapeList::iterator MyShapeIterator = MyListOfPolygons.begin();

            // The loop should never go past the end of the list
            while (!Found && MyShapeIterator != MyListOfPolygons.end())
            	{
                HVE2DShape::SimpleShapeList::iterator MyOtherShapeIterator = MyListOfPolygons.begin();

                while (!Found && MyOtherShapeIterator != MyListOfPolygons.end())
                	{
                    if (MyShapeIterator != MyOtherShapeIterator)
                        Found = ((*MyShapeIterator)->CalculateSpatialPositionOf(**MyOtherShapeIterator) == HVE2DShape::S_IN);

                    MyOtherShapeIterator++;
            		}   
                if (!Found)
                	MyShapeIterator++;   
            	}

            // Check if the outter shape was found ...
            if (Found)
            	{

                // Create holed shape
                HVE2DHoledShape* pMyResultHoledShape = new HVE2DHoledShape(**MyShapeIterator);

                // Add all holes and detroy shapes at the same time
                HVE2DShape::SimpleShapeList::iterator MyFinalShapeIterator = MyListOfPolygons.begin();

                while (MyFinalShapeIterator != MyListOfPolygons.end())
                	{
                    if (MyShapeIterator != MyFinalShapeIterator)
                        pMyResultHoledShape->AddHole(**MyFinalShapeIterator);


                    delete *MyFinalShapeIterator;

                    MyFinalShapeIterator++;
            		}   

                pMyResultShape = pMyResultHoledShape;
            	}
            else
            	{
                // We have  a problem in the sense we have no outter shape.
                // This may occur in rare occurences where the self shape creates a new hole and
                // is completely included in this new hole.
                // Since the unify operation is commutative, we ask for the given to operate
                // the unification
                pMyResultShape = pi_rPolygon.UnifyShape(*this);
            	}

        	}
        else
        	{
            // There is a single shape ... return it
            pMyResultShape = (*MyListOfPolygons.begin());
        	}

    	}

    return (pMyResultShape);
    }



//-----------------------------------------------------------------------------
// SuperScan
// This static method decomposes the different parts of two simple shape interaction
// The first two parameters are the two polygon of segments, the next two are
// lists of points representing these same polygons but with additional
// interaction points included.
// There is a potential bug here ....
// the points must include all interaction points between polygons INCLUDING
// flirting points
//-----------------------------------------------------------------------------
void HVE2DPolygonOfSegments::SuperScan(const HVE2DPolygonOfSegments&  pi_rGiven,
                                       const HGF2DPositionCollection& pi_rPoly1,
                                       const HGF2DPositionCollection& pi_rPoly2,
                                       bool                          pi_WantInPtsOfShape1,
                                       bool                          pi_ScanShape1CW,
                                       bool                          pi_WantInPtsOfShape2,
                                       bool                          pi_ScanShape2CW,
                                       HVE2DShape::SimpleShapeList&   pi_rListOfShapes,
                                       bool*                         po_pAllOn) const
    {
    HINVARIANTS;

    // The two polygon must be expressed in the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rGiven.GetCoordSys());

    // Recipient variable must be provided
    HPRECONDITION(po_pAllOn != 0);

    *po_pAllOn = false;


    // Debug variable ... maximum allowable number of points
    size_t    MaximumNumberOfPointsPerShape = pi_rPoly1.size() + pi_rPoly2.size();

    bool IrregularShapes = false;

    double Tolerance = MIN(GetTolerance(), pi_rGiven.GetTolerance());

    HGF2DPosition MyStartPoint;
    HGF2DPosition CurrentPoint;
    HGF2DPosition PreviousPoint;

    double X;
    double Y;

    // Allocate a list of flags the same size as new self linear
    size_t          NumberOfFlags = pi_rPoly1.size();
    PointUsage*     pMyFlags = new PointUsage[NumberOfFlags];

    // Initialize all flags to false
    for (size_t FlagIndex = 0 ; FlagIndex < NumberOfFlags ; FlagIndex++)
        pMyFlags[FlagIndex] = UNKNOWN;

    // Since last point is a duplicate ... we desactivate right away
    pMyFlags[NumberOfFlags - 1] = USED;


    // Set up linear flipper
    const HVE2DPolygonOfSegments* const     apPoly[2] = {this, &pi_rGiven};
    const HGF2DPositionCollection* const    apPoints[2] = {&pi_rPoly1, &pi_rPoly2};
    bool                                   PolyIn[2] = {pi_WantInPtsOfShape1, pi_WantInPtsOfShape2};

    // Do Until all parts of self have been processed ...
    size_t      Index = 1;
    bool       AllSet;

    HGF2DPosition   DumPoint;
    HGF2DPosition   MidPoint;

    // Extract extent of both polygons
    HGF2DExtent     MySelfExtent = GetExtent();
    HGF2DExtent     MyGivenExtent = pi_rGiven.GetExtent();

    // The extent is transformed into raw values into arrays
    double XMin[2];
    double XMax[2];
    double YMin[2];
    double YMax[2];

    // Obtain raw extent values for self
    XMin[0] = MySelfExtent.GetXMin();
    XMax[0] = MySelfExtent.GetXMax();
    YMin[0] = MySelfExtent.GetYMin();
    YMax[0] = MySelfExtent.GetYMax();

    // Obtain raw extent values for given
    XMin[1] = MyGivenExtent.GetXMin();
    XMax[1] = MyGivenExtent.GetXMax();
    YMin[1] = MyGivenExtent.GetYMin();
    YMax[1] = MyGivenExtent.GetYMax();


    do
        {

        size_t PreviousIndex;
        // Check if this point of self is properly positioned
        CurrentPoint = pi_rPoly1[Index];

        if (Index == 0)
            PreviousIndex = pi_rPoly1.size() - 2;
        else
            PreviousIndex = Index - 1;

        PreviousPoint = pi_rPoly1[PreviousIndex];

        MidPoint.SetX((CurrentPoint.GetX() + PreviousPoint.GetX()) / 2.0);
        MidPoint.SetY((CurrentPoint.GetY() + PreviousPoint.GetY()) / 2.0);

        X = MidPoint.GetX();
        Y = MidPoint.GetY();

        HVE2DShape::SpatialPosition MyPartPosition;

        // Check if there is possible interaction
        if (((X > XMin[1]) || HDOUBLE_EQUAL(X, XMin[1], Tolerance)) &&
            ((X < XMax[1]) || HDOUBLE_EQUAL(X, XMax[1], Tolerance))&&
            ((Y > YMin[1]) || HDOUBLE_EQUAL(Y, YMin[1], Tolerance))&&
            ((Y < YMax[1]) || HDOUBLE_EQUAL(Y, YMax[1], Tolerance)))
            {

            MyPartPosition = pi_rGiven.CalculateSpatialPositionOfPosition(MidPoint, Tolerance);
            }
        else
            MyPartPosition = HVE2DShape::S_OUT;


        if ((MyPartPosition == HVE2DShape::S_OUT && pi_WantInPtsOfShape1) ||
            (MyPartPosition == HVE2DShape::S_IN && !pi_WantInPtsOfShape1))
            {
            // This part is mis-positioned
            // Indicate this part has been processed (discarded)
            pMyFlags[Index] = USED;
            }
        else if(MyPartPosition == HVE2DShape::S_ON)
            {
            // The part overlays part of the given shape
            // We discard it, while remembering that even
            // if it cannot be used as a starting linear
            // it may be part of the solution
            pMyFlags[Index] = ON_POINT;
            }
        else
            {
            // This is a part we want for our result
            pMyFlags[Index] = USED;

            // We create a new complex linear
            HVE2DPolygonOfSegments* pMyNewPoly = new HVE2DPolygonOfSegments(GetCoordSys());

            // Pre-allocate some points
            pMyNewPoly->m_PolySegment.Reserve(m_PolySegment.GetSize());

            // Append the current part of self
            pMyNewPoly->m_PolySegment.m_Points.push_back(PreviousPoint);
            pMyNewPoly->m_PolySegment.m_StartPoint = HGF2DLocation(PreviousPoint, GetCoordSys());
            pMyNewPoly->m_PolySegment.m_Points.push_back(CurrentPoint);

            // Followed shape is self
            HSINTX  ShapeIndex = 0;
            HSINTX  TestShapeIndex = 1;

            // Obtain start point
            MyStartPoint = PreviousPoint;

            // Do until we have come back to the start point
            do
                {
                // Increment index
                Index++;
                PreviousPoint = CurrentPoint;

                // Adjust index for passage out of valid range
                if (Index >= apPoints[ShapeIndex]->size() - 1)
                    Index = 0;

                // Check validity of this part
                CurrentPoint = (*(apPoints[ShapeIndex]))[Index];

                // Check if this point is on self and has already been used (if it is not the start point)
                if ((apPoly[ShapeIndex] == this) && (pMyFlags[Index] == USED) && !CurrentPoint.IsEqualTo(MyStartPoint, Tolerance))
                    {
                    // The point cannot be valid ... no need to check for
                    // its spatial position
                    // Note that this case occurs in extremely rare cases where the self
                    // shape flirts on the given shape or if the point has already been determined as invalid

                    // Change followed shape
                    HSINTX  DummyLong = ShapeIndex;
                    ShapeIndex = TestShapeIndex;
                    TestShapeIndex = DummyLong;

                    // Find the linear which has current point as start point
                    for (Index = 0;
                         (Index < apPoints[ShapeIndex]->size() &&
                          !PreviousPoint.IsEqualTo((*apPoints[ShapeIndex])[Index], Tolerance));
                         Index++)
                        ;

                    // Make sure a valid point was found
                    IrregularShapes = (!(Index < apPoints[ShapeIndex]->size()));

                    // Adjust index for overflow (we want point next to one found)
                    if (Index >= apPoints[ShapeIndex]->size() - 2)
                        {
                        // Index overflow ... we take first point
                        CurrentPoint = (*(apPoints[ShapeIndex]))[0];
                        Index = 0;
                        }
                    else
                        {
                        // No overflow ... take next point
                        CurrentPoint = (*(apPoints[ShapeIndex]))[Index+1];
                        Index++;
                        }

                    // Indicate this part has been processed (discarded) if it is on self
                    if (apPoly[ShapeIndex] == this)
                        pMyFlags[Index] = USED;
                    }
                else
                    {
                    MidPoint.SetX((CurrentPoint.GetX() + PreviousPoint.GetX()) / 2.0);
                    MidPoint.SetY((CurrentPoint.GetY() + PreviousPoint.GetY()) / 2.0);

                    X = MidPoint.GetX();
                    Y = MidPoint.GetY();

                    // Check if there is possible interaction
                    if (((X > XMin[TestShapeIndex]) || HDOUBLE_EQUAL(X, XMin[TestShapeIndex], Tolerance)) &&
                        ((X < XMax[TestShapeIndex]) || HDOUBLE_EQUAL(X, XMax[TestShapeIndex], Tolerance))&&
                        ((Y > YMin[TestShapeIndex]) || HDOUBLE_EQUAL(Y, YMin[TestShapeIndex], Tolerance))&&
                        ((Y < YMax[TestShapeIndex]) || HDOUBLE_EQUAL(Y, YMax[TestShapeIndex], Tolerance)))
                        {
                        MyPartPosition = apPoly[TestShapeIndex]->CalculateSpatialPositionOfPosition(MidPoint, Tolerance);
                        }
                    else
                        MyPartPosition = HVE2DShape::S_OUT;


                    // If the solution was ON and the delta X and delta Y are less than 2 EPSILON
                    // between previous and current coordinates, then it may happen
                    // that the segment is effectively OUT or IN
                    if (MyPartPosition == HVE2DShape::S_ON)
                        {

                        // In such a case, it is possible for the mid point to report ON, and the segment
                        // effectively be out or in. To make sure, we check the other two points of the segment
                        // If any is either in or out, then the result is in or out
                        X = CurrentPoint.GetX();
                        Y = CurrentPoint.GetY();

                        // Check if there is possible interaction
                        if (((X > XMin[TestShapeIndex]) || HDOUBLE_EQUAL(X, XMin[TestShapeIndex], Tolerance)) &&
                            ((X < XMax[TestShapeIndex]) || HDOUBLE_EQUAL(X, XMax[TestShapeIndex], Tolerance))&&
                            ((Y > YMin[TestShapeIndex]) || HDOUBLE_EQUAL(Y, YMin[TestShapeIndex], Tolerance))&&
                            ((Y < YMax[TestShapeIndex]) || HDOUBLE_EQUAL(Y, YMax[TestShapeIndex], Tolerance)))
                            {
                            MyPartPosition = apPoly[TestShapeIndex]->CalculateSpatialPositionOfPosition(CurrentPoint, Tolerance);
                            }
                        else
                            MyPartPosition = HVE2DShape::S_OUT;

                        // If this result was ON, we must still check the previous point
                        if (MyPartPosition == HVE2DShape::S_ON)
                            {
                            X = PreviousPoint.GetX();
                            Y = PreviousPoint.GetY();

                            // Check if there is possible interaction
                            if (((X > XMin[TestShapeIndex]) || HDOUBLE_EQUAL(X, XMin[TestShapeIndex], Tolerance)) &&
                                ((X < XMax[TestShapeIndex]) || HDOUBLE_EQUAL(X, XMax[TestShapeIndex], Tolerance))&&
                                ((Y > YMin[TestShapeIndex]) || HDOUBLE_EQUAL(Y, YMin[TestShapeIndex], Tolerance))&&
                                ((Y < YMax[TestShapeIndex]) || HDOUBLE_EQUAL(Y, YMax[TestShapeIndex], Tolerance)))
                                {
                                MyPartPosition = apPoly[TestShapeIndex]->CalculateSpatialPositionOfPosition(PreviousPoint, Tolerance);
                                }
                            else
                                MyPartPosition = HVE2DShape::S_OUT;

                            // If still it is ON, then the result is probably ON ...
                            // There is a few limit case where the segment in NOT ON,
                            // but for performance reason, we let go ... this case is trapped elsewhere
                            }
                        }

                    // Check if new segment is ill positions
                    if ((MyPartPosition == HVE2DShape::S_OUT && PolyIn[ShapeIndex]) ||
                        (MyPartPosition == HVE2DShape::S_IN && !PolyIn[ShapeIndex]))
                        {
                        // Ill positioned point ...
                        // Indicate this part has been processed (discarded) if it is on self
                        if (apPoly[ShapeIndex] == this)
                            pMyFlags[Index] = USED;

                        // Change followed shape
                        HSINTX  DummyLong = ShapeIndex;
                        ShapeIndex = TestShapeIndex;
                        TestShapeIndex = DummyLong;

                        // Find the linear which has current point as start point
                        for (Index = 0;
                             (Index < apPoints[ShapeIndex]->size() &&
                              !PreviousPoint.IsEqualTo((*apPoints[ShapeIndex])[Index], Tolerance));
                             Index++)
                            ;

                        // Make sure a valid point was found
                        IrregularShapes = (!(Index < apPoints[ShapeIndex]->size()));

                        // Adjust index for overflow (we want point next to one found)
                        if (Index >= apPoints[ShapeIndex]->size() - 2)
                            {
                            // Index overflow ... we take first point
                            CurrentPoint = (*(apPoints[ShapeIndex]))[0];
                            Index = 0;
                            }
                        else
                            {
                            // No overflow ... take next point
                            CurrentPoint = (*(apPoints[ShapeIndex]))[Index+1];
                            Index++;
                            }

                        // Indicate this part has been processed (discarded) if it is on self
                        if (apPoly[ShapeIndex] == this)
                            pMyFlags[Index] = USED;


                        }
                    else if (MyPartPosition == HVE2DShape::S_ON)
                        {
                        // This part is located ON the test shape
                        // We continue on current only if the others part is
                        // not properly positioned

                        // Find the linear which has current point as start point
                        size_t  DumIndex;
                        for (DumIndex = 0;
                             ((DumIndex < apPoints[TestShapeIndex]->size() - 1) &&
                              !PreviousPoint.IsEqualTo((*(apPoints[TestShapeIndex]))[DumIndex], Tolerance)) ;
                             DumIndex++)
                            ;

                        // Now we have the point on self, however
                        // in extremely rare cases, there is twice the point
                        // in the same self shape ... we check if this one has never
                        // been used
                        if ((apPoly[TestShapeIndex] == this) && (pMyFlags[DumIndex] == USED))
                            {
                            // This is our extremely rare case
                            // We must fix things up before continuing

                            // Search for a second point that satisfies
                            size_t  DumIndex2;
                            for (DumIndex2 = 0;
                                 (((DumIndex2 < apPoints[TestShapeIndex]->size() - 1) &&
                                   !PreviousPoint.IsEqualTo((*(apPoints[TestShapeIndex]))[DumIndex2], Tolerance)) ||
                                  (DumIndex == DumIndex2)) ;
                                 DumIndex2++)
                                ;

                            if (DumIndex2 < apPoints[TestShapeIndex]->size() - 1)
                                {
                                // YES there is a second point that satisfies
                                DumIndex = DumIndex2;
                                }

                            }


                        IrregularShapes = (!(DumIndex < (apPoints[TestShapeIndex]->size() - 1)));

                        HGF2DPosition   DumPreviousPoint((*(apPoints[TestShapeIndex]))[DumIndex]);
                        HGF2DPosition   DumCurrentPoint;

                        if (DumIndex >= apPoints[TestShapeIndex]->size() - 2)
                            {
                            DumCurrentPoint = (*(apPoints[TestShapeIndex]))[0];
                            DumIndex = 0;
                            }
                        else
                            {
                            DumCurrentPoint = (*(apPoints[TestShapeIndex]))[DumIndex+1];
                            DumIndex++;
                            }


                        MidPoint.SetX((DumCurrentPoint.GetX() + DumPreviousPoint.GetX()) / 2.0);
                        MidPoint.SetY((DumCurrentPoint.GetY() + DumPreviousPoint.GetY()) / 2.0);

                        X = MidPoint.GetX();
                        Y = MidPoint.GetY();

                        // Obtain its position
                        if (((X > XMin[ShapeIndex]) || HDOUBLE_EQUAL(X, XMin[ShapeIndex], Tolerance)) &&
                            ((X < XMax[ShapeIndex]) || HDOUBLE_EQUAL(X, XMax[ShapeIndex], Tolerance))&&
                            ((Y > YMin[ShapeIndex]) || HDOUBLE_EQUAL(Y, YMin[ShapeIndex], Tolerance))&&
                            ((Y < YMax[ShapeIndex]) || HDOUBLE_EQUAL(Y, YMax[ShapeIndex], Tolerance)))
                            {
                            MyPartPosition = apPoly[ShapeIndex]->CalculateSpatialPositionOfPosition(MidPoint, Tolerance);
                            }
                        else
                            MyPartPosition = HVE2DShape::S_OUT;

                        // Determine if this part is properly positioned
                        if ((MyPartPosition == HVE2DShape::S_OUT && !PolyIn[TestShapeIndex]) ||
                            (MyPartPosition == HVE2DShape::S_IN && PolyIn[TestShapeIndex]))
                            {
                            // It is properly positionned ... so we change
                            // Indicate this part has been processed (discarded) if it is on self
                            if (apPoly[ShapeIndex] == this)
                                pMyFlags[Index] = USED;

                            // Change followed shape
                            HSINTX  DummyLong = ShapeIndex;
                            ShapeIndex = TestShapeIndex;
                            TestShapeIndex = DummyLong;

                            Index = DumIndex;

                            // Indicate that new part is taken (if on self)
                            if (apPoly[ShapeIndex] == this)
                                pMyFlags[Index] = USED;

                            CurrentPoint = DumCurrentPoint;
                            }
                        else
                            {
                            // We stay on current linear ... indicate this part
                            // Check that if we can know (current is self) the newly selected part has not been used
                            // yet !
                            HASSERTDUMP2(CurrentPoint.IsEqualTo(MyStartPoint, Tolerance) ||
                                         (apPoly[ShapeIndex] == this ? pMyFlags[Index] != USED : true), *this, pi_rGiven);

                            // has been taken (if on self)
                            if (apPoly[ShapeIndex] == this)
                                pMyFlags[Index] = USED;

                            // As a result of a TR319814 we observed it is far better to call superscan2 in such case ...
                            // We will therefore indicate the shapes as irregular.
                            IrregularShapes = true;

                            }
                        }
                    else
                        {
                        // Check that if we can know (current is self) the newly selected part has not been used
                        // yet !
                        HASSERTDUMP2(CurrentPoint.IsEqualTo(MyStartPoint, Tolerance) ||
                                     (apPoly[ShapeIndex] == this ? pMyFlags[Index] != USED : true), *this, pi_rGiven);

                        // Indicate this part has been processed (taken) if it is on self
                        if (apPoly[ShapeIndex] == this)
                            pMyFlags[Index] = USED;
                        }
                    }


                // Append this linear to our new linear if it follows some conditions
                // Check if there are already two points in result
                if (pMyNewPoly->m_PolySegment.m_Points.size() >= 2)
                    {
                    // There are more than 2 points ... verify
                    // that current point is different from next before last
                    if (CurrentPoint.IsEqualTo(
                            pMyNewPoly->m_PolySegment.m_Points[pMyNewPoly->m_PolySegment.m_Points.size() - 2],
                            Tolerance))
                        {
                        // We have a backtrack ... this occurs rarely for minuscule segments
                        // We remove it
                        pMyNewPoly->m_PolySegment.m_Points.pop_back();
                        pMyNewPoly->m_PolySegment.m_Points.pop_back();
                        }
                    }

                // In all case, even if we removed points we add this point
                pMyNewPoly->m_PolySegment.m_Points.push_back(CurrentPoint);

                // The number of points may not be greater than the maximum allowed

                // This used to be an assertion, but it does appear irregular
                // polgygon produce this kind of error, so we will go to
                // process irregular shapes.
                IrregularShapes = IrregularShapes ||
                                  (pMyNewPoly->m_PolySegment.m_Points.size() > MaximumNumberOfPointsPerShape);
//              HASSERTDUMP2(pMyNewPoly->m_PolySegment.m_Points.size() <= MaximumNumberOfPointsPerShape,
//                             *this,
//                             pi_rGiven);

                }
            while (!CurrentPoint.IsEqualTo(MyStartPoint, Tolerance) && !IrregularShapes);

            // Check if we did not encounter an irregular shape
            if (!IrregularShapes)
                {
                // Check if there are any points in result ... yes this may occur
                // If it is empty we do not add it ...
                if (pMyNewPoly->m_PolySegment.m_Points.size() > 2)
                    {
                    // Check if points are exactly equal ... DO NOT USE EPSILON

                    //!!!! MICROSOFT VISUAL C++ ON ALPHA : Workaround for a compiler bug
                    if (pMyNewPoly->m_PolySegment.m_Points[0] != CurrentPoint) // (pMyNewPoly->m_PolySegment.GetPosition(0) != pMyNewPoly->m_PolySegment.GetPosition(m_PolySegment.GetSize() - 1))
                        {
                        // The points are different by less than an epsilon ... adjust
                        pMyNewPoly->m_PolySegment.m_Points.back() = pMyNewPoly->m_PolySegment.m_Points.front();
                        }

                    pMyNewPoly->m_PolySegment.m_EndPoint = HGF2DLocation(pMyNewPoly->m_PolySegment.m_Points.back(), GetCoordSys());

                    // Set tolerance for new polygon
                    pMyNewPoly->SetAutoToleranceActive(IsAutoToleranceActive());
                    pMyNewPoly->SetTolerance(Tolerance);
                    pMyNewPoly->ResetTolerance();

                    // The area of new polygon may not be 0.0
                    HASSERTDUMP2(!HDOUBLE_EQUAL(pMyNewPoly->CalculateArea(), 0.0, Tolerance * Tolerance),
                                 *this,
                                 pi_rGiven);

                    // Try simplifying the polygon
                    pMyNewPoly->Simplify();

                    // Check if it represents a rectangle
                    if (pMyNewPoly->RepresentsARectangle())
                        {
                        // It is a rectangle ... we transform it into such
                        pi_rListOfShapes.push_back(pMyNewPoly->GenerateCorrespondingRectangle());

                        // Destroy polygon
                        delete pMyNewPoly;
                        }
                    else
                        {
                        // The polygon is not a rectangle ... we add it as it is
                        // Append this polygon to list of shapes
                        pi_rListOfShapes.push_back(pMyNewPoly);
                        }
                    }
                }  // !IrregularShapes
            }

        // Check if all flags are set and if not find first not set
        AllSet = true;
        for (Index = 0 ; ((Index < NumberOfFlags) && (AllSet = (pMyFlags[Index] != UNKNOWN))) ; Index++)
            ;
        }
    while (!AllSet && !IrregularShapes);



    // Check if we encountered irregular shapes (or if there was no result)
    if (IrregularShapes || pi_rListOfShapes.size()== 0)
        {
        // We encountered a major problem
        // We must use the special processing superscan (slower)
        // First removed all shapes from list
        HVE2DShape::SimpleShapeList::iterator Itr;

        for (Itr = pi_rListOfShapes.begin() ; Itr != pi_rListOfShapes.end() ; ++Itr)
            {
            delete (*Itr);
            }
        pi_rListOfShapes.clear();


        // It may happen that there is no result shape in the case, the shapes are identical
        // or closely flirting-contiguous disjoint.
        // The differentiation is performed by checking if all points of self are ON
        if (pi_rListOfShapes.size() == 0)
            {
            // No result in shape ... check if all points are on
            *po_pAllOn = true;
            for (Index = 0 ; ((Index < NumberOfFlags - 1) && (*po_pAllOn = (pMyFlags[Index] == ON_POINT))) ; Index++)
                ;
            }

        if (!*po_pAllOn)
            {


            SuperScan2(pi_rGiven,
                       pi_rPoly1,
                       pi_rPoly2,
                       pi_WantInPtsOfShape1,
                       pi_ScanShape1CW,
                       pi_WantInPtsOfShape2,
                       pi_ScanShape2CW,
                       pi_rListOfShapes,
                       po_pAllOn);
            }
        }

    delete[] pMyFlags;

    }


//-----------------------------------------------------------------------------
// SuperScan2
// This works only for IRREGULAR SHAPES
// This static method decomposes the different parts of two simple shape interaction
// The first two parameters are the two polygon of segments, the next two are
// lists of points representing these same polygons but with additional
// interaction points included.
// There is a potential bug here ....
// the points must include all interaction points between polygons INCLUDING
// flirting points
//-----------------------------------------------------------------------------
void HVE2DPolygonOfSegments::SuperScan2(const HVE2DPolygonOfSegments&  pi_rGiven,
                                        const HGF2DPositionCollection& pi_rPoly1,
                                        const HGF2DPositionCollection& pi_rPoly2,
                                        bool                          pi_WantInPtsOfShape1,
                                        bool                          pi_ScanShape1CW,
                                        bool                          pi_WantInPtsOfShape2,
                                        bool                          pi_ScanShape2CW,
                                        HVE2DShape::SimpleShapeList&   pi_rListOfShapes,
                                        bool*                         po_pAllOn) const
    {

    HINVARIANTS;

    // The two polygon must be expressed in the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rGiven.GetCoordSys());

    // Recipient variable must be provided
    HPRECONDITION(po_pAllOn != 0);

    *po_pAllOn = false;

    // Recondition list of points.
    // One of the possible problem of irregularity in polgons is that it is auto-flirting
    // and that there is not a point for both places of this auto-flirting ... we recondition
    // the list

    // First we duplicate the collections
    HGF2DPositionCollection ListPoly1 = pi_rPoly1;
    HGF2DPositionCollection ListPoly2 = pi_rPoly2;

    // Now we insert the auto-flirt points
    InsertAutoFlirtPoints(ListPoly1);
    pi_rGiven.InsertAutoFlirtPoints(ListPoly2);

    // Debug variable ... maximum allowable number of points
    HDEBUGCODE(size_t    MaximumNumberOfPointsPerShape = ListPoly1.size() + ListPoly2.size());

    // We define a maximum number of loops to operate quite arbitrarily. As we sometimes backtrack the number of loops is quite different from
    // the maximum number of points.
    size_t MaximumNumberOfOutterLoops = (ListPoly1.size() + ListPoly2.size()) * 50;
    size_t currentOutterLoop = 0;

    double Tolerance = MIN(GetTolerance(), pi_rGiven.GetTolerance());

    HGF2DPosition MyStartPoint;
    HGF2DPosition CurrentPoint;
    HGF2DPosition PreviousPoint;

    // Allocate a list of flags the same size as new self linear
    size_t          NumberOfFlagsSelf = ListPoly1.size();
    size_t          NumberOfFlagsGiven = ListPoly2.size();
    PointUsage*     pMyFlagsSelf = new PointUsage[NumberOfFlagsSelf];
    PointUsage*     pMyFlagsGiven = new PointUsage[NumberOfFlagsGiven];

    // Initialize all flags of self to false
    size_t FlagIndex;
    for (FlagIndex = 0 ; FlagIndex < NumberOfFlagsSelf ; FlagIndex++)
        pMyFlagsSelf[FlagIndex] = UNKNOWN;

    // Since last point is a duplicate ... we desactivate right away
    pMyFlagsSelf[NumberOfFlagsSelf - 1] = USED;

    // Initialize all flags of self to false
    for (FlagIndex = 0 ; FlagIndex < NumberOfFlagsGiven ; FlagIndex++)
        pMyFlagsGiven[FlagIndex] = UNKNOWN;

    // Since last point is a duplicate ... we desactivate right away
    pMyFlagsGiven[NumberOfFlagsGiven - 1] = USED;

    // Set up linear flipper
    const HVE2DPolygonOfSegments* const     apPoly[2] = {this, &pi_rGiven};
    const HGF2DPositionCollection*     apPoints[2] = {&ListPoly1, &ListPoly2};
    bool                                   PolyIn[2] = {pi_WantInPtsOfShape1, pi_WantInPtsOfShape2};
    PointUsage*                             Flags[2] = {pMyFlagsSelf, pMyFlagsGiven};

    // Do Until all parts of self have been processed ...
    size_t      Index = 1;
    size_t      PrevIndex;
    bool       AllSet;

    HGF2DPosition   DumPoint;

    // Extract extent of both polygons
    HGF2DExtent     MySelfExtent = GetExtent();
    HGF2DExtent     MyGivenExtent = pi_rGiven.GetExtent();

    // The extent is transformed into lite extent raw values into array
    HGF2DLiteExtent PolyExtents[2];

    // Obtain raw extent values for self
    PolyExtents[0].Set(MySelfExtent.GetXMin(),
                       MySelfExtent.GetYMin(),
                       MySelfExtent.GetXMax(),
                       MySelfExtent.GetYMax());

    // Obtain raw extent values for given
    PolyExtents[1].Set(MyGivenExtent.GetXMin(),
                       MyGivenExtent.GetYMin(),
                       MyGivenExtent.GetXMax(),
                       MyGivenExtent.GetYMax());


    do
        {
        // Check if this point of self is properly positioned
        CurrentPoint = ListPoly1[Index];

        if (Index == 0)
            {
            PrevIndex = ListPoly1.size() - 2;
            PreviousPoint = ListPoly1[PrevIndex];
            }
        else
            {
            PrevIndex = Index - 1;
            PreviousPoint = ListPoly1[PrevIndex];
            }

        HVE2DShape::SpatialPosition MyPartPosition;

        MyPartPosition = pi_rGiven.CalculateSpatialPositionOfPositionSegment2(PolyExtents[1], PreviousPoint, CurrentPoint, Tolerance);
        // Check if there is possible interaction

        if ((MyPartPosition == HVE2DShape::S_OUT && pi_WantInPtsOfShape1) ||
            (MyPartPosition == HVE2DShape::S_IN && !pi_WantInPtsOfShape1))
            {
            // This part is mis-positioned
            // Indicate this part has been processed (discarded)
            Flags[0][Index] = USED;
            }
        else if(MyPartPosition == HVE2DShape::S_ON)
            {
            // The part overlays part of the given shape
            // We discard it, while remembering that even
            // if it cannot be used as a starting linear
            // it may be part of the solution
            Flags[0][Index] = ON_POINT;
            }
        else
            {
            // This is a part we want for our result
            Flags[0][Index] = USED;


            bool StopLoop;

            // We create a new complex linear
            HVE2DPolygonOfSegments* pMyNewPoly = new HVE2DPolygonOfSegments(GetCoordSys());

            // Pre-allocate some points
            pMyNewPoly->m_PolySegment.Reserve(m_PolySegment.GetSize());

            // Append the current part of self
            pMyNewPoly->m_PolySegment.m_Points.push_back(PreviousPoint);
            pMyNewPoly->m_PolySegment.m_StartPoint = HGF2DLocation(PreviousPoint, GetCoordSys());
            pMyNewPoly->m_PolySegment.m_Points.push_back(CurrentPoint);

            // Followed shape is self
            size_t  ShapeIndex = 0;
            size_t  TestShapeIndex = 1;

            size_t StartIndex = PrevIndex;
            size_t StartShape = ShapeIndex;

            // Obtain start point
            MyStartPoint = PreviousPoint;


            // We define a maximum number of loops to operate quite arbitrarily. As we sometimes backtrack the number of loops is quite different from
            // the maximum number of points.
            size_t MaximumNumberOfInnerLoops = (ListPoly1.size() + ListPoly2.size()) * 5;
            size_t currentInnerLoop = 0;

            // Do until we have come back to the start point
            do
                {
                // Increment index
                PrevIndex = Index;
                Index++;
                PreviousPoint = CurrentPoint;

                // Adjust index for passage out of valid range
                if (Index >= apPoints[ShapeIndex]->size() - 1)
                    Index = 0;

                // Check validity of this part
                CurrentPoint = (*(apPoints[ShapeIndex]))[Index];

                    {
                    MyPartPosition = apPoly[TestShapeIndex]->CalculateSpatialPositionOfPositionSegment2(PolyExtents[TestShapeIndex], PreviousPoint, CurrentPoint, Tolerance);

                    // Check if new segment is ill positions
                    if (((Flags[ShapeIndex][Index] == USED) && !CurrentPoint.IsEqualTo(MyStartPoint, Tolerance))
                        || ((MyPartPosition == HVE2DShape::S_OUT && PolyIn[ShapeIndex]) ||
                            (MyPartPosition == HVE2DShape::S_IN && !PolyIn[ShapeIndex])))

                        {

                        // Ill positioned point ...

                        // Indicate this part has been processed (discarded) if it is on self
                        Flags[ShapeIndex][Index] = USED;


                        ChangeShape(apPoly,
                                    apPoints,
                                    PolyExtents,
                                    Flags,
                                    PolyIn,
                                    ShapeIndex,
                                    TestShapeIndex,
                                    Index,
                                    PrevIndex,
                                    PreviousPoint,
                                    CurrentPoint,
                                    Tolerance);

                        // Indicate this new part has been processed (discarded) if it is on self
                        Flags[ShapeIndex][Index] = USED;

                        }

                    else if (MyPartPosition == HVE2DShape::S_ON)
                        {
                        Flags[ShapeIndex][Index] = ON_POINT;

                        ChangeShape(apPoly,
                                    apPoints,
                                    PolyExtents,
                                    Flags,
                                    PolyIn,
                                    ShapeIndex,
                                    TestShapeIndex,
                                    Index,
                                    PrevIndex,
                                    PreviousPoint,
                                    CurrentPoint,
                                    Tolerance);

                        Flags[ShapeIndex][Index] = USED;
                        }
                    else
                        {
                        // Check that if we can know (current is self) the newly selected part has not been used
                        // yet !
                        HASSERTDUMP2(CurrentPoint.IsEqualTo(MyStartPoint, Tolerance) ||
                                     (apPoly[ShapeIndex] == this ? Flags[0][Index] != USED : true), *this, pi_rGiven);

                        // Indicate this part has been processed (taken) if it is on self
                        Flags[ShapeIndex][Index] = USED;
                        }
                    }


                // Append this linear to our new linear if it follows some conditions
                // Check if there are already two points in result
                if (pMyNewPoly->m_PolySegment.m_Points.size() >= 2)
                    {
                    // There are more than 2 points ... verify
                    // that current point is different from next before last
                    if (CurrentPoint.IsEqualTo(
                            pMyNewPoly->m_PolySegment.m_Points[pMyNewPoly->m_PolySegment.m_Points.size() - 2],
                            Tolerance))
                        {
                        // We have a backtrack ... this occurs rarely for minuscule segments
                        // We remove it
                        pMyNewPoly->m_PolySegment.m_Points.pop_back();
                        pMyNewPoly->m_PolySegment.m_Points.pop_back();
                        }
                    }

                // In all case, even if we removed points we add this point
                pMyNewPoly->m_PolySegment.m_Points.push_back(CurrentPoint);

                // The number of points may not be greater than the maximum allowed
                HASSERTDUMP2(pMyNewPoly->m_PolySegment.m_Points.size() <= MaximumNumberOfPointsPerShape,
                             *this,
                             pi_rGiven);

                // Check if loopstop conditions attained
                StopLoop = false;
                if (CurrentPoint.IsEqualTo(MyStartPoint, Tolerance))
                    {
                    // Strong potential to stop loop ... check is start index is current index
                    if (ShapeIndex == StartShape && StartIndex == Index)
                        StopLoop = true;
                    else
                        {
                        // HOHO!!!! We may have a problem
                        // Increment index
                        size_t Index3 = Index + 1;
                        HGF2DPosition PreviousPoint3 = CurrentPoint;

                        // Adjust index for passage out of valid range
                        if (Index3 >= apPoints[ShapeIndex]->size() - 1)
                            Index3 = 0;

                        // Check validity of this part
                        HGF2DPosition CurrentPoint3 = (*(apPoints[ShapeIndex]))[Index3];

                        MyPartPosition = apPoly[TestShapeIndex]->CalculateSpatialPositionOfPositionSegment2(PolyExtents[TestShapeIndex], PreviousPoint3, CurrentPoint3, Tolerance);

                        // Check if new segment is ill positions
                        if (((Flags[ShapeIndex][Index3] == USED) && !CurrentPoint3.IsEqualTo(MyStartPoint, Tolerance))
                            || ((MyPartPosition == HVE2DShape::S_OUT && PolyIn[ShapeIndex]) ||
                                (MyPartPosition == HVE2DShape::S_IN && !PolyIn[ShapeIndex])))
                            {
                            StopLoop = true;
                            }
                        else
                            // If ON it is considered ill positioned ... therefore we stop
                            StopLoop = (MyPartPosition == HVE2DShape::S_ON);

                        }
                    }

                currentInnerLoop++;
                if (currentInnerLoop >= MaximumNumberOfInnerLoops)
                    {
                        // Debug dump
#if defined(__HMR_SUPERDEBUG)

                    HASSERTDUMP2(false, *this, pi_rGiven);
#endif

                    throw HVEDecompositionException();
                    }
            }
            while (!StopLoop);

            Flags[StartShape][StartIndex] = USED;

            // Check if there are any points in result ... yes this may occur
            // If it is empty we do not add it ...
            if (pMyNewPoly->m_PolySegment.m_Points.size() > 2)
                {
                // Check if points are exactly equal ... DO NOT USE EPSILON
                //!!!! MICROSOFT VISUAL C++ ON ALPHA : Workaround for a compiler bug
                if (pMyNewPoly->m_PolySegment.m_Points[0] != CurrentPoint) // (pMyNewPoly->m_PolySegment.GetPosition(0) != pMyNewPoly->m_PolySegment.GetPosition(m_PolySegment.GetSize() - 1))
                    {
                    // The points are different by less than an epsilon ... adjust
                    pMyNewPoly->m_PolySegment.m_Points.back() = pMyNewPoly->m_PolySegment.m_Points.front();
                    }

                pMyNewPoly->m_PolySegment.m_EndPoint = HGF2DLocation(pMyNewPoly->m_PolySegment.m_Points.back(), GetCoordSys());

                // Remove needles (rare occurence)
                pMyNewPoly->m_PolySegment.RemoveAutoContiguousNeedles(true);

                // Set tolerance for new polygon
                pMyNewPoly->SetAutoToleranceActive(IsAutoToleranceActive());
                pMyNewPoly->SetTolerance(Tolerance);
                pMyNewPoly->ResetTolerance();

                // The area of new polygon may not be 0.0
                HASSERTDUMP2(!HDOUBLE_EQUAL(pMyNewPoly->CalculateArea(), 0.0, Tolerance * Tolerance),
                             *this,
                             pi_rGiven);

                // Try simplifying the polygon
                pMyNewPoly->Simplify();

                // Check if it represents a rectangle
                if (pMyNewPoly->RepresentsARectangle())
                    {
                    // It is a rectangle ... we transform it into such
                    pi_rListOfShapes.push_back(pMyNewPoly->GenerateCorrespondingRectangle());

                    // Destroy polygon
                    delete pMyNewPoly;
                    }
                else
                    {
                    // The polygon is not a rectangle ... we add it as it is
                    // Append this polygon to list of shapes
                    pi_rListOfShapes.push_back(pMyNewPoly);
                    }
                }
            }

        // Check if all flags are set and if not find first not set
        AllSet = true;
        for (Index = 0 ; ((Index < NumberOfFlagsSelf) && (AllSet = (Flags[0][Index] != UNKNOWN))) ; Index++)
            ;

        currentOutterLoop++;
        if (currentOutterLoop >= MaximumNumberOfOutterLoops)
            {
            // Debug dump
            HASSERTDUMP2(false, *this, pi_rGiven);

            throw HVEDecompositionException();
            }

        }
    while (!AllSet);

    // It may happen that there is no result shape in the case, the shapes are identical
    // or closely flirting-contiguous disjoint.
    // The differentiation is performed by checking if all points of self are ON
    if (pi_rListOfShapes.size() == 0)
        {
        // No result in shape ... check if all points are on
        *po_pAllOn = true;
        for (Index = 0 ; ((Index < NumberOfFlagsSelf) && (*po_pAllOn = (Flags[0][Index] == ON_POINT))) ; Index++)
            ;
        }

    delete pMyFlagsSelf;
    delete pMyFlagsGiven;

    }




//-----------------------------------------------------------------------------
// InteractsWithSameUnits
// PRIVATE METHOD
// This method analyses the two given polygons of segments, and in the case there
// is any interaction points, produces copies of the points of the polygons
// to which have been added all interaction points, in the direction
// appropriate for super scanning.
// It returns true if polygons are contiguous or intersect
//-----------------------------------------------------------------------------
bool HVE2DPolygonOfSegments::InteractsWithSameUnits(const HVE2DPolygonOfSegments& pi_rPolygon,
                                                     HGF2DPositionCollection* po_pSelfPolyPoints,
                                                     HGF2DPositionCollection* po_pGivenPolyPoints,
                                                     HVE2DPolygonOfSegments::DecomposeOperation pi_Operation,
                                                     bool pi_IgnoreSimpleContiguousness,
                                                     bool* po_pContiguousInteraction) const

    {
    HINVARIANTS;

    // None of the two shape may be empty
    HPRECONDITION(!IsEmpty() && !pi_rPolygon.IsEmpty());

    // The two polygons must be expressed in the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rPolygon.GetCoordSys());

    HPRECONDITION(po_pSelfPolyPoints != 0);
    HPRECONDITION(po_pGivenPolyPoints != 0);

    if (NULL != po_pContiguousInteraction)
        *po_pContiguousInteraction = false;

    // Define tolerance
    double Tolerance = MIN(GetTolerance(), pi_rPolygon.GetTolerance());

    // Pre-allocate some points in return lists
    po_pSelfPolyPoints->reserve(2 * m_PolySegment.GetSize() - 1);
    po_pGivenPolyPoints->reserve(2 * pi_rPolygon.m_PolySegment.GetSize() - 1);

    bool   ReturnValue = false;

    bool   PotentialReturnValue = false;

    bool   SelfPolyCW = (pi_Operation == DIFF || pi_Operation == UNION || pi_Operation == INTERSECT);
    bool   GivenPolyCW = (pi_Operation == DIFFFROM || pi_Operation == UNION || pi_Operation == INTERSECT);

    // We copy in appropriate order the self polygon points
    if ((SelfPolyCW && CalculateRotationDirection() == HVE2DSimpleShape::CW) ||
        (!SelfPolyCW && CalculateRotationDirection() == HVE2DSimpleShape::CCW))
        {
        // The desired direction for self is the correct one
        *po_pSelfPolyPoints = m_PolySegment.m_Points;
        }
    else
        {
        HGF2DPositionCollection::const_reverse_iterator SelfItr(m_PolySegment.m_Points.rbegin());

        while (SelfItr != m_PolySegment.m_Points.rend())
            {
            po_pSelfPolyPoints->push_back(*SelfItr++);
            }
        }


    // We copy in appropriate order the given polygon points
    if ((GivenPolyCW && pi_rPolygon.CalculateRotationDirection() == HVE2DSimpleShape::CW) ||
        (!GivenPolyCW && pi_rPolygon.CalculateRotationDirection() == HVE2DSimpleShape::CCW))
        {
        // The desired direction for given is the correct one
        *po_pGivenPolyPoints = pi_rPolygon.m_PolySegment.m_Points;
        }
    else
        {
        // The desired direction for self is not the correct one
        HGF2DPositionCollection::const_reverse_iterator GivenItr(pi_rPolygon.m_PolySegment.m_Points.rbegin());

        while (GivenItr != pi_rPolygon.m_PolySegment.m_Points.rend())
            {
            po_pGivenPolyPoints->push_back(*GivenItr++);
            }
        }


    // Debug variables to catch running conditions
    HDEBUGCODE(int64_t MaxLoop = m_PolySegment.GetSize() * pi_rPolygon.m_PolySegment.GetSize() * m_PolySegment.GetSize() * pi_rPolygon.m_PolySegment.GetSize());
    HDEBUGCODE(int64_t LoopNumber = 0);

    HGF2DPosition   CrossPoint;

    // Declare two iterators ... one for previous point and one for current point
    HGF2DPositionCollection::iterator   SelfItr(po_pSelfPolyPoints->begin());
    HGF2DPositionCollection::iterator   PrevSelfItr = SelfItr;
    for (++SelfItr ; SelfItr != po_pSelfPolyPoints->end() ; ++SelfItr)
        {
        // Create segment
        HGF2DLiteSegment    SelfSegment(*PrevSelfItr, *SelfItr, Tolerance);

        double SelfXMin = MIN(SelfSegment.GetStartPoint().GetX(), SelfSegment.GetEndPoint().GetX());
        double SelfXMax = MAX(SelfSegment.GetStartPoint().GetX(), SelfSegment.GetEndPoint().GetX());
        double SelfYMin = MIN(SelfSegment.GetStartPoint().GetY(), SelfSegment.GetEndPoint().GetY());
        double SelfYMax = MAX(SelfSegment.GetStartPoint().GetY(), SelfSegment.GetEndPoint().GetY());

        HGF2DPositionCollection::iterator   GivenItr(po_pGivenPolyPoints->begin());
        HGF2DPositionCollection::iterator   PrevGivenItr = GivenItr;

        for (++GivenItr ; GivenItr != po_pGivenPolyPoints->end() ; ++GivenItr)
            {
            // In debug count
            HDEBUGCODE(++LoopNumber);
            HASSERTDUMP2(LoopNumber < (MaxLoop), *this, pi_rPolygon);

            // Break in case of running conditions in debug mode
            // Check if the two segments can possibly interact
            double GivenXMin = MIN(PrevGivenItr->GetX(), GivenItr->GetX());
            double GivenXMax = MAX(PrevGivenItr->GetX(), GivenItr->GetX());
            double GivenYMin = MIN(PrevGivenItr->GetY(), GivenItr->GetY());
            double GivenYMax = MAX(PrevGivenItr->GetY(), GivenItr->GetY());


            bool Result = (((SelfXMax > GivenXMin) || HDOUBLE_EQUAL(SelfXMax, GivenXMin, Tolerance)) &&
                            ((SelfXMin < GivenXMax) || HDOUBLE_EQUAL(SelfXMin, GivenXMax, Tolerance)) &&
                            ((SelfYMax > GivenYMin) || HDOUBLE_EQUAL(SelfYMax, GivenYMin, Tolerance)) &&
                            ((SelfYMin < GivenYMax) || HDOUBLE_EQUAL(SelfYMin, GivenYMax, Tolerance))
                           );

            if (Result)
                {
                bool SegmentsCross = false;
                bool CrossesAtExtremity = false;

                // Create segment
                HGF2DLiteSegment    GivenSegment(*PrevGivenItr, *GivenItr, Tolerance);

                // Between two segments, there can be one interaction type only :
                // Either they cross, are contiguous or are connected one on the other (without linking)
                // Check if they cross
                SegmentsCross = (SelfSegment.IntersectSegmentExtremityIncluded(GivenSegment, &CrossPoint, &CrossesAtExtremity) == HGF2DLiteSegment::CROSS_FOUND);

                if (CrossesAtExtremity)
                    {
                    PotentialReturnValue = true;
                    SegmentsCross = false;
                    }

                if (SegmentsCross)
                    {
                    // They do cross ... we insert cross point before current point
                    SelfItr = po_pSelfPolyPoints->insert(SelfItr, CrossPoint);
                    PrevSelfItr = SelfItr;
                    --PrevSelfItr;
                    GivenItr = po_pGivenPolyPoints->insert(GivenItr, CrossPoint);
                    PrevGivenItr = GivenItr;
                    --PrevGivenItr;

                    // Set end point of segments to cross point
                    SelfSegment.SetEndPoint(CrossPoint);

                    // Update extent
                    SelfXMin = MIN(SelfSegment.GetStartPoint().GetX(), SelfSegment.GetEndPoint().GetX());
                    SelfXMax = MAX(SelfSegment.GetStartPoint().GetX(), SelfSegment.GetEndPoint().GetX());
                    SelfYMin = MIN(SelfSegment.GetStartPoint().GetY(), SelfSegment.GetEndPoint().GetY());
                    SelfYMax = MAX(SelfSegment.GetStartPoint().GetY(), SelfSegment.GetEndPoint().GetY());

                    ReturnValue = true;
                    }
                // Check if they are contiguous
                else if (SelfSegment.AreContiguous(GivenSegment))
                    {
                    InteractsWithSameUnitsContiguousProcessing (SelfSegment,
                                                                GivenSegment,
                                                                Tolerance,
                                                                pi_IgnoreSimpleContiguousness,
                                                                po_pContiguousInteraction,
                                                                &ReturnValue,
                                                                po_pSelfPolyPoints,
                                                                po_pGivenPolyPoints,
                                                                SelfItr,
                                                                PrevSelfItr,
                                                                GivenItr,
                                                                PrevGivenItr);

                    SelfXMin = MIN(SelfSegment.GetStartPoint().GetX(), SelfSegment.GetEndPoint().GetX());
                    SelfXMax = MAX(SelfSegment.GetStartPoint().GetX(), SelfSegment.GetEndPoint().GetX());
                    SelfYMin = MIN(SelfSegment.GetStartPoint().GetY(), SelfSegment.GetEndPoint().GetY());
                    SelfYMax = MAX(SelfSegment.GetStartPoint().GetY(), SelfSegment.GetEndPoint().GetY());
                    }
                else
                    {
                    // The segments do not cross nor are contiguous

                    // Check if given connects to self by end point
                    if (SelfSegment.IsPointOn(GivenSegment.GetEndPoint()))
                        {
                        if (!SelfSegment.LinksTo(GivenSegment))
                            {
                            // The end point connects upon
                            SelfItr = po_pSelfPolyPoints->insert(SelfItr, GivenSegment.GetEndPoint());
                            PrevSelfItr = SelfItr;
                            --PrevSelfItr;

                            // Set end point of segments to point
                            SelfSegment.SetEndPoint(GivenSegment.GetEndPoint());

                            // Update extent
                            SelfXMin = MIN(SelfSegment.GetStartPoint().GetX(), SelfSegment.GetEndPoint().GetX());
                            SelfXMax = MAX(SelfSegment.GetStartPoint().GetX(), SelfSegment.GetEndPoint().GetX());
                            SelfYMin = MIN(SelfSegment.GetStartPoint().GetY(), SelfSegment.GetEndPoint().GetY());
                            SelfYMax = MAX(SelfSegment.GetStartPoint().GetY(), SelfSegment.GetEndPoint().GetY());
                            }

                        // The fact that segments connect is not a sufficient reason to presume interaction
                        // We set a special flag
                        PotentialReturnValue = true;
                        }

                    // Check if self connects to given by end point
                    if (GivenSegment.IsPointOn(SelfSegment.GetEndPoint()))
                        {
                        if (!GivenSegment.LinksTo(SelfSegment))
                            {
                            // The end point connects upon
                            GivenItr = po_pGivenPolyPoints->insert(GivenItr, SelfSegment.GetEndPoint());
                            PrevGivenItr = GivenItr;
                            --PrevGivenItr;
                            }

                        // The fact that segments connect is not a sufficient reason to presume interaction
                        // We set a special flag
                        PotentialReturnValue = true;
                        }
                    }
                }

            // Save previous given iterator
            PrevGivenItr = GivenItr;
            }

        // Save previous self iterator
        PrevSelfItr = SelfItr;
        }

    // Check if they did not interact but segments connected
    if (!ReturnValue && PotentialReturnValue)
        {
        // Some segments did connect, but we do not know if it was an intersection point or a flirting
        // we resolve the hard way
        ReturnValue = Crosses(pi_rPolygon);
        }

    return(ReturnValue);
    }




//-----------------------------------------------------------------------------
// InteractsWithSameUnitsContiguousProcessing
// PRIVATE METHOD
// This method contains the contiguousness processing of the InteractsWithSameUnits
// method. It has been extraction even though the process is not properly
// speaking "well-defined" in order to ease maintenance and enable more precise
// performance analysis.
//-----------------------------------------------------------------------------
void HVE2DPolygonOfSegments::InteractsWithSameUnitsContiguousProcessing(
    HGF2DLiteSegment& SelfSegment,
    HGF2DLiteSegment& GivenSegment,
    double Tolerance,
    bool pi_IgnoreSimpleContiguousness,
    bool* po_pContiguousInteraction,
    bool* ReturnValue,
    HGF2DPositionCollection*             po_pSelfPolyPoints,
    HGF2DPositionCollection*             po_pGivenPolyPoints,
    HGF2DPositionCollection::iterator&   SelfItr,
    HGF2DPositionCollection::iterator&   PrevSelfItr,
    HGF2DPositionCollection::iterator&   GivenItr,
    HGF2DPositionCollection::iterator&   PrevGivenItr) const
    {
    // The two segments are contiguous ... obtain contiguousness points
    HGF2DPositionCollection ContiguousnessPoints;
    SelfSegment.ObtainContiguousnessPoints(GivenSegment, &ContiguousnessPoints);

    // There are exactly 2 contiguousness points for contiguous segments
    HASSERT(ContiguousnessPoints.size() == 2);

    // Make sure that the contiguousness points are located on segments
    // Prevents OLD bug from re-occuring
    HASSERT(SelfSegment.IsPointOn(ContiguousnessPoints[0]));
    HASSERT(SelfSegment.IsPointOn(ContiguousnessPoints[1]));
    HASSERT(GivenSegment.IsPointOn(ContiguousnessPoints[0]));
    HASSERT(GivenSegment.IsPointOn(ContiguousnessPoints[1]));

    // Check if points are identical... this may happen sometimes
    if (!(ContiguousnessPoints[0].IsEqualTo(ContiguousnessPoints[1], Tolerance)))
        {
        //Contiguousness points are different ... continue

        // Indicate interaction if contiguousness is sufficient
        if (!pi_IgnoreSimpleContiguousness)
            *ReturnValue = true;

        if (NULL != po_pContiguousInteraction)
            *po_pContiguousInteraction = true;

        // For the self segments, they are necessarily located in the proper order

        // For the given segment, the order of contiguousness points is unknown
        // it must be determined
        if ((HGF2DLocation(*PrevSelfItr, GetCoordSys()) - HGF2DLocation(ContiguousnessPoints[0], GetCoordSys())).CalculateLength() <
            (HGF2DLocation(*PrevSelfItr, GetCoordSys()) - HGF2DLocation(ContiguousnessPoints[1], GetCoordSys())).CalculateLength())
            {

            // We check if second contiguousness point is different from end point
            if (!ContiguousnessPoints[1].IsEqualTo(SelfSegment.GetEndPoint(), Tolerance))
                {
                if (!pi_IgnoreSimpleContiguousness)
                    *ReturnValue = true;

                // The second contiguousness point is different from end point ...
                // we insert it
                SelfItr = po_pSelfPolyPoints->insert(SelfItr, ContiguousnessPoints[1]);
                PrevSelfItr = SelfItr;
                --PrevSelfItr;

                // Set end point of segments to point
                SelfSegment.SetEndPoint(ContiguousnessPoints[1]);

                }

            // We check if first contiguousness point is different from start point
            if (!ContiguousnessPoints[0].IsEqualTo(SelfSegment.GetStartPoint(), Tolerance))
                {
                if (!pi_IgnoreSimpleContiguousness)
                    *ReturnValue = true;

                // The first contiguousness point is different from start point ...
                // we insert it
                SelfItr = po_pSelfPolyPoints->insert(SelfItr, ContiguousnessPoints[0]);
                PrevSelfItr = SelfItr;
                --PrevSelfItr;

                // Set end point of segments to point
                SelfSegment.SetEndPoint(ContiguousnessPoints[0]);

                }
            }
        else
            {
            // We check if second contiguousness point is different from end point
            if (!ContiguousnessPoints[0].IsEqualTo(SelfSegment.GetEndPoint(), Tolerance))
                {
                if (!pi_IgnoreSimpleContiguousness)
                    *ReturnValue = true;

                // The second contiguousness point is different from end point ...
                // we insert it
                SelfItr = po_pSelfPolyPoints->insert(SelfItr, ContiguousnessPoints[0]);
                PrevSelfItr = SelfItr;
                --PrevSelfItr;

                // Set end point of segments to point
                SelfSegment.SetEndPoint(ContiguousnessPoints[0]);

                }

            // We check if first contiguousness point is different from start point
            if (!ContiguousnessPoints[1].IsEqualTo(SelfSegment.GetStartPoint(), Tolerance))
                {
                if (!pi_IgnoreSimpleContiguousness)
                    *ReturnValue = true;

                // The first contiguousness point is different from start point ...
                // we insert it
                SelfItr = po_pSelfPolyPoints->insert(SelfItr, ContiguousnessPoints[1]);
                PrevSelfItr = SelfItr;
                --PrevSelfItr;

                // Set end point of segments to point
                SelfSegment.SetEndPoint(ContiguousnessPoints[1]);
                }
            }


        // For the given segment, the order of contiguousness points is unknown
        // it must be determined
        if ((HGF2DLocation(*PrevGivenItr, GetCoordSys()) - HGF2DLocation(ContiguousnessPoints[0], GetCoordSys())).CalculateLength() <
            (HGF2DLocation(*PrevGivenItr, GetCoordSys()) - HGF2DLocation(ContiguousnessPoints[1], GetCoordSys())).CalculateLength())
            {
            // The contiguousness points are in the proper order

            // We check if second contiguousness point is different from end point
            if (!ContiguousnessPoints[1].IsEqualTo(GivenSegment.GetEndPoint(), Tolerance))
                {
                if (!pi_IgnoreSimpleContiguousness)
                    *ReturnValue = true;

                // The second contiguousness point is different from end point ...
                // we insert it
                GivenItr = po_pGivenPolyPoints->insert(GivenItr, ContiguousnessPoints[1]);
                PrevGivenItr = GivenItr;
                --PrevGivenItr;
                }

            // We check if first contiguousness point is different from start point
            if (!ContiguousnessPoints[0].IsEqualTo(GivenSegment.GetStartPoint(), Tolerance))
                {
                if (!pi_IgnoreSimpleContiguousness)
                    *ReturnValue = true;

                // The first contiguousness point is different from start point ...
                // we insert it
                GivenItr = po_pGivenPolyPoints->insert(GivenItr, ContiguousnessPoints[0]);
                PrevGivenItr = GivenItr;
                --PrevGivenItr;
                }
            }
        else
            {
            // The contiguousness points are in reverse order

            // We check if first contiguousness point is different from end point
            if (!ContiguousnessPoints[0].IsEqualTo(GivenSegment.GetEndPoint(), Tolerance))
                {
                if (!pi_IgnoreSimpleContiguousness)
                    *ReturnValue = true;

                // The first contiguousness point is different from end point ...
                // we insert it
                GivenItr = po_pGivenPolyPoints->insert(GivenItr, ContiguousnessPoints[0]);
                PrevGivenItr = GivenItr;
                --PrevGivenItr;
                }

            // We check if second contiguousness point is different from start point
            if (!ContiguousnessPoints[1].IsEqualTo(GivenSegment.GetStartPoint(), Tolerance))
                {
                if (!pi_IgnoreSimpleContiguousness)
                    *ReturnValue = true;

                // The second contiguousness point is different from start point ...
                // we insert it
                GivenItr = po_pGivenPolyPoints->insert(GivenItr, ContiguousnessPoints[1]);
                PrevGivenItr = GivenItr;
                --PrevGivenItr;
                }
            }
        }
    }





//-----------------------------------------------------------------------------
// AreAdjacent
// This method checks if the polygon is adjacent with given vector.
//-----------------------------------------------------------------------------
bool HVE2DPolygonOfSegments::AreAdjacent(const HVE2DVector& pi_rVector) const
    {
    HINVARIANTS;

    return(m_PolySegment.AreAdjacent(pi_rVector));
    }

//-----------------------------------------------------------------------------
// AreContiguous
// This method checks if the polygon is contiguous with given vector.
//-----------------------------------------------------------------------------
bool HVE2DPolygonOfSegments::AreContiguous(const HVE2DVector& pi_rVector) const
    {
    return(m_PolySegment.AreContiguous(pi_rVector));
    }


//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
void HVE2DPolygonOfSegments::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE

    HVE2DSimpleShape::PrintState(po_rOutput);

    HDUMP0("BEGIN Dumping a HVE2DPolygonOfSegments object\n");
    po_rOutput << "BEGIN Dumping a HVE2DPolygonOfSegments object" << endl;

    // Dump the coordinate system

    // Dump the points
    char    DumString[256];
    HGF2DPositionCollection::const_iterator Itr = m_PolySegment.m_Points.begin();

    for (Itr = m_PolySegment.m_Points.begin() ; Itr != m_PolySegment.m_Points.end() ; Itr++)
        {
        sprintf(DumString, "%5.15lf , %5.15lf", (*Itr).GetX(), (*Itr).GetY());
        HDUMP0(DumString);
        HDUMP0("\n");
        po_rOutput << DumString << endl;
        }

    HDUMP0("END (HVE2DPolygonOfSegments)\n");
    po_rOutput << "END (HVE2DPolygonOfSegments)" << endl;
#endif
    }


//-----------------------------------------------------------------------------
// GetLightShape
// Allocates a light shape representing polygon 
//-----------------------------------------------------------------------------
HGF2DShape* HVE2DPolygonOfSegments::GetLightShape() const
{
    HINVARIANTS;

    // Allocate a new polygon fence
    HAutoPtr<HGF2DPolygonOfSegments > pNewFence;

    // Check if polygon of segments is not empty
    if (!IsEmpty())
        {
        // Get access to list of points in polygon fence
        HGF2DPositionCollection ListOfPoints;

        // Reserve space for all points
        ListOfPoints.reserve(m_PolySegment.GetSize());
        
        // Create iterator
        HGF2DPositionCollection::const_iterator Itr = m_PolySegment.m_Points.begin();

        // Add points
        for (Itr = m_PolySegment.m_Points.begin() ; Itr != m_PolySegment.m_Points.end() ; Itr++)
            {
            ListOfPoints.push_back(HGF2DPosition(Itr->GetX(), Itr->GetY()));
            }

        pNewFence = new HGF2DPolygonOfSegments(ListOfPoints);
        }
    else
        {
        pNewFence = new HGF2DPolygonOfSegments();
        }

    return(pNewFence.release());
}



//-----------------------------------------------------------------------------
// Drop
// Returns the description of shape in the form of raw location
// segments
//-----------------------------------------------------------------------------
void HVE2DPolygonOfSegments::Drop(HGF2DLocationCollection* po_pPoints,
                                  double                   pi_Tolerance) const
    {
    HINVARIANTS;

    HPRECONDITION(po_pPoints != 0);

    // Check if polygon is not empty
    if (!IsEmpty())
        {
        // Reserve space for all points
        po_pPoints->reserve(m_PolySegment.GetSize());

        // Create iterator
        HGF2DPositionCollection::const_iterator Itr = m_PolySegment.m_Points.begin();

        // Add points
        for ( ; Itr != m_PolySegment.m_Points.end() ; Itr++)
            {
            po_pPoints->push_back(HGF2DLocation(*Itr, GetCoordSys()));
            }
        }
    }

//-----------------------------------------------------------------------------
// ResetTolerance
// PRIVATE
// This method recalculates the tolerance of polygon if automatic tolerance is
// active
//-----------------------------------------------------------------------------
void HVE2DPolygonOfSegments::ResetTolerance()
    {
    HINVARIANTS;

    // Ask polysegment to reset tolerance
    m_PolySegment.ResetTolerance();

    // Set tolerance to polygon of segment
    HVE2DVector::SetTolerance(m_PolySegment.GetTolerance());

    }

//-----------------------------------------------------------------------------
// SetTolerance
// This method sets the tolerance for polysegment
//-----------------------------------------------------------------------------
void HVE2DPolygonOfSegments::SetTolerance(double pi_Tolerance)
    {
    HINVARIANTS;

    // Ask polysegment to reset tolerance
    m_PolySegment.SetTolerance(pi_Tolerance);

    // Set tolerance to polysegment
    HVE2DVector::SetTolerance(pi_Tolerance);
    }


//-----------------------------------------------------------------------------
// SetStrokeTolerance
// This method sets the tolerance for polysegment
//-----------------------------------------------------------------------------
void HVE2DPolygonOfSegments::SetStrokeTolerance(const HFCPtr<HGFTolerance> & pi_Tolerance)
    {
    HINVARIANTS;

    // Ask polysegment to reset tolerance
    m_PolySegment.SetStrokeTolerance(pi_Tolerance);

    // Set tolerance to polysegment
    HVE2DVector::SetStrokeTolerance(pi_Tolerance);
    }


//-----------------------------------------------------------------------------
// SetAutoToleranceActive
// This method sets the auto tolerance determination for polysegment
//-----------------------------------------------------------------------------
void HVE2DPolygonOfSegments::SetAutoToleranceActive(bool pi_ActiveAutoTolerance)
    {
    HINVARIANTS;

    // Ask polysegment to reset tolerance
    m_PolySegment.SetAutoToleranceActive(pi_ActiveAutoTolerance);

    // Set tolerance to polysegment
    HVE2DVector::SetAutoToleranceActive(pi_ActiveAutoTolerance);

    // Make sure polysegment and segment have the same tolerance
    HVE2DVector::SetTolerance(m_PolySegment.GetTolerance());
    }


//-----------------------------------------------------------------------------
// CalculateSpatialPositionOfPolygonOfSegmentsSCS
// PRIVATE
// This method calculates the spatial position of current polygon
// compared to given
//-----------------------------------------------------------------------------
HVE2DShape::SpatialPosition HVE2DPolygonOfSegments::CalculateSpatialPositionOfPolygonOfSegmentsSCS(const HVE2DPolygonOfSegments& pi_rPolygon) const
    {
    HINVARIANTS;

    // The two polygons must share the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rPolygon.GetCoordSys());

    HVE2DShape::SpatialPosition     ThePosition = HVE2DShape::S_OUT;

    // Check if their extents overlap
    if (GetExtent().OutterOverlaps(pi_rPolygon.GetExtent(), MIN(GetTolerance(), pi_rPolygon.GetTolerance())))
        {
        // The extents overlap ... check if they cross ?
        if (CrossesPolygonOfSegmentsSCS(pi_rPolygon))
            {
            // Since they do cross, the vector is PARTIALY IN (passes through the shape boundary)
            ThePosition = HVE2DShape::S_PARTIALY_IN;
            }
        else
            {
            // Calculate position for a non crossing polygon
            ThePosition = CalculateSpatialPositionOfNonCrossingPolygonOfSegmentsSCS(pi_rPolygon);
            }
        }

    return (ThePosition);

    }


//-----------------------------------------------------------------------------
// CalculateSpatialPositionOfNonCrossingPolygonOfSegmentsSCS
// PRIVATE
// This method calculates the spatial position of current polygon
// compared to given
//-----------------------------------------------------------------------------
HVE2DShape::SpatialPosition HVE2DPolygonOfSegments::CalculateSpatialPositionOfNonCrossingPolygonOfSegmentsSCS(const HVE2DPolygonOfSegments& pi_rPolygon) const
    {
    HINVARIANTS;

    // The two polygons must share the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rPolygon.GetCoordSys());

    // The polygons must not cross
    HPRECONDITION(!Crosses(pi_rPolygon));

    HVE2DShape::SpatialPosition     ThePosition = HVE2DShape::S_OUT;

    // Determine the tolerance
    double Tolerance = MIN(GetTolerance(), pi_rPolygon.GetTolerance());

    // Check if their extents overlap
    if (GetExtent().OutterOverlaps(pi_rPolygon.GetExtent(), Tolerance))
        {
        // Obtain position of points until something
        // else that ON is returned.

        HGF2DPositionCollection::const_iterator Itr = pi_rPolygon.m_PolySegment.m_Points.begin();

        // Obtain the position of each point until IN or OUT is obtained
        for (ThePosition = HVE2DShape::S_ON; Itr != pi_rPolygon.m_PolySegment.m_Points.end() && ThePosition == HVE2DShape::S_ON ; ++Itr)
            ThePosition = CalculateSpatialPositionOfPosition(*Itr, Tolerance);

        // Check that this point is not ON
        if (ThePosition == HVE2DShape::S_ON)
            {
            // Since all points are ON we cannot conclude ... we must analyse the position of self
            // relative to given polygon of segments
            // Obtain the position of each point until IN or OUT is obtained
            HGF2DPositionCollection::const_iterator Itr = m_PolySegment.m_Points.begin();
            for (; Itr != m_PolySegment.m_Points.end() && ThePosition == HVE2DShape::S_ON ; ++Itr)
                ThePosition = pi_rPolygon.CalculateSpatialPositionOfPosition(*Itr, Tolerance);

            // Check that this point is not ON
            if (ThePosition != HVE2DShape::S_ON)
                {
                // Self has points out or in given ... it follows since they do not cross that
                // given has the inverse relation to self
                ThePosition = (ThePosition == HVE2DShape::S_OUT ? HVE2DShape::S_IN : HVE2DShape::S_OUT);
                }
            else
                {
                // All points of self are on given ... we cannot conclude but the result is most probably ON

                // We obtain the area of both polygons
                double SelfArea = CalculateArea();
                double GivenArea = pi_rPolygon.CalculateArea();

                // The smaller area polygon is IN the other
                // If areas are equal then they are on each other
                if (HDOUBLE_EQUAL(SelfArea, GivenArea, GetTolerance() * GetTolerance()))
                    ThePosition = HVE2DShape::S_ON;
                else
                    ThePosition = (SelfArea > GivenArea ? HVE2DShape::S_IN : HVE2DShape::S_OUT);
                }
            }
        }

    return (ThePosition);
    }

//-----------------------------------------------------------------------------
// CalculateSpatialPositionOfPositionSegment
// PRIVATE
// This method returns the spatial position relative to shape of given points
// This method check for all three points of a segment to make sure that
// The segment is really ON if ON is obtained for point.
// This method may response an inadequate result in the case of multi-flirting
// segments. If multi-flirting segment is to be checked use
// CalculateSpatialPositionOfPositionSegment2
//-----------------------------------------------------------------------------
HVE2DShape::SpatialPosition HVE2DPolygonOfSegments::CalculateSpatialPositionOfPositionSegment(const HGF2DPosition& pi_rPoint,
        const HGF2DPosition& pi_rStartPoint,
        const HGF2DPosition& pi_rEndPoint,
        double              pi_Tolerance) const
    {
    HINVARIANTS;

    // The tolerance must be greater than 0
    HPRECONDITION(pi_Tolerance > 0.0);

    // Declare return value
    HVE2DShape::SpatialPosition MyPartPosition;

    // Obtain extent of polygon of segment
    HGF2DExtent SelfExtent(GetExtent());

    double XMin = SelfExtent.GetXMin();
    double XMax = SelfExtent.GetXMax();
    double YMin = SelfExtent.GetYMin();
    double YMax = SelfExtent.GetYMax();

    // Obtain tolerance
    double Tolerance = pi_Tolerance;

    // Extract position values
    double X(pi_rPoint.GetX());
    double Y(pi_rPoint.GetY());

    // Check if there is possible interaction
    if (((X > XMin) || HDOUBLE_EQUAL(X, XMin, Tolerance)) &&
        ((X < XMax) || HDOUBLE_EQUAL(X, XMax, Tolerance))&&
        ((Y > YMin) || HDOUBLE_EQUAL(Y, YMin, Tolerance))&&
        ((Y < YMax) || HDOUBLE_EQUAL(Y, YMax, Tolerance)))
        {
        MyPartPosition = CalculateSpatialPositionOfPosition(pi_rPoint, Tolerance);
        }
    else
        MyPartPosition = HVE2DShape::S_OUT;


    // If the solution was ON and the delta X and delta Y are less than 2 EPSILON
    // between previous and current coordinates, then it may happen
    // that the segment is effectively OUT or IN
    if (MyPartPosition == HVE2DShape::S_ON)
        {

        // In such a case, it is possible for the mid point to report ON, and the segment
        // effectively be out or in. To make sure, we check the other two points of the segment
        // If any is either in or out, then the result is in or out
        X = pi_rEndPoint.GetX();
        Y = pi_rEndPoint.GetY();

        // Check if there is possible interaction
        if (((X > XMin) || HDOUBLE_EQUAL(X, XMin, Tolerance)) &&
            ((X < XMax) || HDOUBLE_EQUAL(X, XMax, Tolerance))&&
            ((Y > YMin) || HDOUBLE_EQUAL(Y, YMin, Tolerance))&&
            ((Y < YMax) || HDOUBLE_EQUAL(Y, YMax, Tolerance)))
            {
            MyPartPosition = CalculateSpatialPositionOfPosition(pi_rEndPoint, Tolerance);
            }
        else
            MyPartPosition = HVE2DShape::S_OUT;

        // If this result was ON, we must still check the previous point
        if (MyPartPosition == HVE2DShape::S_ON)
            {
            X = pi_rStartPoint.GetX();
            Y = pi_rStartPoint.GetY();

            // Check if there is possible interaction
            if (((X > XMin) || HDOUBLE_EQUAL(X, XMin, Tolerance)) &&
                ((X < XMax) || HDOUBLE_EQUAL(X, XMax, Tolerance))&&
                ((Y > YMin) || HDOUBLE_EQUAL(Y, YMin, Tolerance))&&
                ((Y < YMax) || HDOUBLE_EQUAL(Y, YMax, Tolerance)))
                {
                MyPartPosition = CalculateSpatialPositionOfPosition(pi_rStartPoint, Tolerance);
                }
            else
                MyPartPosition = HVE2DShape::S_OUT;

            // If still it is ON, then the result is probably ON ...
            // There is a few limit case where the segment in NOT ON,
            // but for performance reason, we let go ... this case is trapped elsewhere
            }
        }
    return(MyPartPosition);
    }







//-----------------------------------------------------------------------------
// AllocateComplexShapeFromAutoContiguousPolySegment
// PRIVATE
// This method is for the exclusive use of the AllocateCopyInCoordSys method
// The method enables to create a complex shape from autocontiguous polysegment
// resulting from a change of coordinate system of the current polygon of
// segment.
// The provided polysegment MUST be autocontiguous and this autocontiguousness
// must not be the result of contiguousness needles
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DPolygonOfSegments::AllocateComplexShapeFromAutoContiguousPolySegment(const HVE2DPolySegment& pi_rPolySegment) const
    {
    // The provided polysegment MUST be autocontiguous and this autocontiguousness
    // must not be the result of contiguousness needles
    HDEBUGCODE(HVE2DPolySegment TempPolySegment(pi_rPolySegment));
    HDEBUGCODE(TempPolySegment.RemoveAutoContiguousNeedles(true));
    HPRECONDITION(TempPolySegment.IsAutoContiguous());


    // For debugging purposes ... see below for condition
    HDEBUGCODE(size_t MaxNumberOfPoints = pi_rPolySegment.m_Points.size() * 3);

    // First we search for a section of the polysegment which is not auto
    // contiguous to any other parts. The nature of the polysegment implies
    // that there is at least one segment which is not contiguous to any other
    // segment

    // Pre-calculate tolerance
    double Tolerance = pi_rPolySegment.GetTolerance();

    HAutoPtr<HVE2DShape> pResultShape;

    // Create result complex shape
    HAutoPtr<HVE2DComplexShape> pResultComplex(new HVE2DComplexShape(pi_rPolySegment.GetCoordSys()));
    pResultComplex->SetAutoToleranceActive(pi_rPolySegment.IsAutoToleranceActive());
    pResultComplex->SetTolerance(pi_rPolySegment.GetTolerance());


    // Create a work copy of polysegment
    HVE2DPolySegment WorkPolySegment(pi_rPolySegment);

    // Inserts all autocontiguousness points in polysegment
    HArrayAutoPtr<PointUsage> Flags(InsertAutoContiguousPoints(WorkPolySegment));

    // Create and initialize flags
    size_t NumberOfFlags = WorkPolySegment.m_Points.size();

    // Since last point is a duplicate ... we desactivate
    Flags[NumberOfFlags - 1] = USED;

    size_t Index;
    size_t PrevIndex;
    bool  AllSet;
    HGF2DPosition CurrentPoint;
    HGF2DPosition PreviousPoint;

    // Find a segment which is not autocontiguous
    for (Index = 0 ; ((Index < NumberOfFlags) && (Flags[Index] != UNKNOWN)) ; Index++)
        ;

    // Until all segments have been processed or are contiguous ...
    do
        {
        CurrentPoint = WorkPolySegment.m_Points[Index];

        if (Index == 0)
            PreviousPoint = WorkPolySegment.m_Points[WorkPolySegment.m_Points.size() - 2];
        else
            PreviousPoint = WorkPolySegment.m_Points[Index - 1];

        // Save start point
        HGF2DPosition StartPoint = PreviousPoint;

        // Create a new polysegment
        HVE2DPolySegment NewPolySegment(WorkPolySegment.GetCoordSys());

        // Append start segment
        NewPolySegment.m_Points.push_back(PreviousPoint);
        NewPolySegment.m_Points.push_back(CurrentPoint);
        Flags[Index] = USED;

        do
            {
            // Increment index
            PrevIndex = Index;
            Index++;
            PreviousPoint = CurrentPoint;

            // Adjust index for passage out of valid range
            if (Index >= WorkPolySegment.m_Points.size() - 1)
                Index = 0;

            CurrentPoint = WorkPolySegment.m_Points[Index];

            if (Flags[Index] == ON_POINT)
                {
                // We are entering an autocontiguousness region ...
                // we shift to other segment

                // Find the linear which has current point as start point
                for (Index = 0;
                     ((PrevIndex == Index) ||
                      (Index < WorkPolySegment.m_Points.size() &&
                       !PreviousPoint.IsEqualTo(WorkPolySegment.m_Points[Index], Tolerance)));
                     Index++)
                    ;

                // Make sure a valid point was found
                HASSERTDUMP2(Index < WorkPolySegment.m_Points.size(), *this, pi_rPolySegment);

                // Adjust index for overflow (we want point next to one found)
                if (Index >= WorkPolySegment.m_Points.size() - 2)
                    {
                    // Index overflow ... we take first point
                    CurrentPoint = WorkPolySegment.m_Points[0];
                    PrevIndex = WorkPolySegment.m_Points.size() - 2;
                    Index = 0;
                    }
                else
                    {
                    // No overflow ... take next point
                    CurrentPoint = WorkPolySegment.m_Points[Index+1];
                    PrevIndex = Index;
                    Index++;
                    }
                }

            // Add new point in polysegment
            NewPolySegment.m_Points.push_back(CurrentPoint);

            // Indicate this part has been processed (discarded)
            Flags[Index] = USED;

            // Check if we are not in an infinite loop
            HASSERTDUMP(MaxNumberOfPoints > NewPolySegment.m_Points.size(), pi_rPolySegment);

            }
        while (!CurrentPoint.IsEqualTo(StartPoint, Tolerance));


        // We have a new polysegment
        // Set start and end point.
        NewPolySegment.m_StartPoint = HGF2DLocation(NewPolySegment.m_Points[0], NewPolySegment.GetCoordSys());
        NewPolySegment.m_EndPoint = HGF2DLocation(NewPolySegment.m_Points[0], NewPolySegment.GetCoordSys());

        // The end point must be approximatively equal to start point (within tolerance)
        HASSERT(NewPolySegment.m_Points[0].IsEqualTo(NewPolySegment.m_Points[NewPolySegment.m_Points.size() - 1], Tolerance));

        NewPolySegment.m_Points[NewPolySegment.m_Points.size() - 1] = NewPolySegment.m_Points[0];

        // Reset tolerance
        NewPolySegment.SetAutoToleranceActive(pi_rPolySegment.IsAutoToleranceActive());
        NewPolySegment.SetTolerance(Tolerance);
        NewPolySegment.ResetTolerance();


        HASSERTDUMP(NewPolySegment.m_Points.size() > 3, pi_rPolySegment);

        // Add new polysegment to complex shape
        pResultComplex->AddShape(HVE2DPolygonOfSegments(NewPolySegment));

        // Check if all flags are set and if not find first not set
        AllSet = true;
        for (Index = 0 ; ((Index < NumberOfFlags) && (AllSet = (Flags[Index] != UNKNOWN))) ; Index++)
            ;
        }
    while (!AllSet);


    // There should be at least 2 polygons in result
    // but in rare cases, one of the shape folds upon itself and there is only one shape
    // there may even be 0 shapes in some cases

    // Check if there is a single shape
    if (pResultComplex->GetShapeList().size() == 1)
        {
        // Result is single component shape
        pResultShape.reset(static_cast<HVE2DShape*>((*pResultComplex->GetShapeList().begin())->Clone()));
        }
    // Check if there is no shape
    else if (pResultComplex->GetShapeList().size() == 0)
        {
        // Result is a void shape
        pResultShape.reset(new HVE2DVoidShape(pi_rPolySegment.GetCoordSys()));
        }
    else
        {
        // Result is the complex
        pResultShape.reset(pResultComplex.release());
        }


    return(pResultShape.release());
    }

//-----------------------------------------------------------------------------
// InsertAutoContiguousPoints
// PRIVATE
// This method is for the exclusive use of AllocateCopyInCoordSys()
// It is used to add all autocontiguousness points of a polysegment
// to the list of points in this polysegment
// In addition, the method allocates and returns an heap allocated
// array of point usage flags where are flags are set to UNKNOWN
// except for the last point of an autocontiguousness region which are
// set to ON_POINT.
// This array must be deallocated after it is needed no more.
//-----------------------------------------------------------------------------
HVE2DPolygonOfSegments::PointUsage* HVE2DPolygonOfSegments::InsertAutoContiguousPoints(HVE2DPolySegment& pio_rPolySegment) const
    {
    // Allocate iterators
    HGF2DPositionCollection::iterator Itr;
    HGF2DPositionCollection::iterator PrevItr;

    double SelfXMin;
    double SelfXMax;
    double SelfYMin;
    double SelfYMax;

    // Pre-calculate tolerance
    double Tolerance = pio_rPolySegment.GetTolerance();


    // Declare flags
    vector<PointUsage> MyPointsUsage;

    // Reserve some space
    MyPointsUsage.reserve(pio_rPolySegment.m_Points.size());
    vector<PointUsage>::iterator MyPointsUsageItr;
    vector<PointUsage>::iterator MyOtherPointsUsageItr;

    // Insert and initialize all points usage to UNKNOWN
    size_t MyPointsUsageIndex = 0;
    for (; MyPointsUsageIndex < pio_rPolySegment.m_Points.size(); ++MyPointsUsageIndex)
        {
        MyPointsUsage.push_back(UNKNOWN);
        }

    // For every segments ...
    MyPointsUsageItr = MyPointsUsage.begin();
    ++MyPointsUsageItr;
    Itr = pio_rPolySegment.m_Points.begin();
    PrevItr = Itr;
    ++Itr;
    for ( ; Itr != pio_rPolySegment.m_Points.end() ; ++PrevItr, ++Itr, ++MyPointsUsageItr)
        {
        // Obtain extent of this segment
        SelfXMin = MIN(Itr->GetX(), PrevItr->GetX());
        SelfXMax = MAX(Itr->GetX(), PrevItr->GetX());
        SelfYMin = MIN(Itr->GetY(), PrevItr->GetY());
        SelfYMax = MAX(Itr->GetY(), PrevItr->GetY());

        // Create a lite segment to represent current segment
        HGF2DLiteSegment TheLiteSegment(*PrevItr, *Itr, Tolerance);

        // For every segments ... (again)
        HGF2DPositionCollection::iterator OtherItr;
        HGF2DPositionCollection::iterator PrevOtherItr;

        OtherItr = pio_rPolySegment.m_Points.begin();
        PrevOtherItr = OtherItr;
        ++OtherItr;
        MyOtherPointsUsageItr = MyPointsUsage.begin();
        ++MyOtherPointsUsageItr;

        bool Result;
        for (; (OtherItr != pio_rPolySegment.m_Points.end()) ; ++OtherItr , ++PrevOtherItr, ++MyOtherPointsUsageItr)
            {
            // Check that it is not the same segment
            if (OtherItr != Itr)
                {

                // Check if current self segment and current other segment may interact
                Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, MIN(OtherItr->GetX(), PrevOtherItr->GetX()), Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, MAX(OtherItr->GetX(), PrevOtherItr->GetX()), Tolerance) &&
                          HDOUBLE_GREATER_OR_EQUAL(SelfYMax, MIN(OtherItr->GetY(), PrevOtherItr->GetY()), Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, MAX(OtherItr->GetY(), PrevOtherItr->GetY()), Tolerance)
                         );
                if (Result)
                    {
                    // The two segments may interact ... check if they are auto contiguous
                    HGF2DLiteSegment TheOtherLiteSegment(*PrevOtherItr, *OtherItr, Tolerance);

                    if (TheOtherLiteSegment.AreContiguous(TheLiteSegment))
                        {
                        // The segments are contiguous

                        // Find and insert contiguousness points
                        HGF2DPositionCollection ContiguousnessPoints;
                        TheLiteSegment.ObtainContiguousnessPoints(TheOtherLiteSegment,
                                                                  &ContiguousnessPoints);

                        // Check if points are identical... this may happen sometimes
                        if (!(ContiguousnessPoints[0].IsEqualTo(ContiguousnessPoints[1], Tolerance)))
                            {

                            // Make sure compare segment has proper usage
                            if (ContiguousnessPoints[0].IsEqualTo(*OtherItr, Tolerance) ||
                                ContiguousnessPoints[1].IsEqualTo(*OtherItr, Tolerance))
                                {
                                *MyOtherPointsUsageItr = ON_POINT;
                                }

                            // For the segment, the order of contiguousness points is unknown
                            // it must be determined
                            if ((HGF2DLocation(*PrevItr, pio_rPolySegment.GetCoordSys()) - HGF2DLocation(ContiguousnessPoints[0], pio_rPolySegment.GetCoordSys())).CalculateLength() <
                                (HGF2DLocation(*PrevItr, pio_rPolySegment.GetCoordSys()) - HGF2DLocation(ContiguousnessPoints[1], pio_rPolySegment.GetCoordSys())).CalculateLength())
                                {
                                // The first contiguousness point is closer to start point of segment

                                // Check if second point is equal to either extremities
                                if (!ContiguousnessPoints[1].IsEqualTo(*Itr, Tolerance))
                                    {
                                    // Second point is different ... we insert
                                    Itr = pio_rPolySegment.m_Points.insert(Itr, ContiguousnessPoints[1]);
                                    MyPointsUsageItr = MyPointsUsage.insert(MyPointsUsageItr, UNKNOWN);
                                    PrevItr = Itr;
                                    --PrevItr;
                                    TheLiteSegment.SetStartPoint(*PrevItr);
                                    TheLiteSegment.SetEndPoint(*Itr);
                                    OtherItr = pio_rPolySegment.m_Points.begin();
                                    PrevOtherItr = OtherItr;
                                    ++OtherItr;
                                    MyOtherPointsUsageItr = MyPointsUsage.begin();
                                    ++MyOtherPointsUsageItr;
                                    }

                                // Indicate this point is the end of a contiguousness region
                                *MyPointsUsageItr = ON_POINT;

                                // Check if first point is equal to either extremities
                                if (!ContiguousnessPoints[0].IsEqualTo(*PrevItr, Tolerance))
                                    {
                                    // First point is different ... we insert
                                    Itr = pio_rPolySegment.m_Points.insert(Itr, ContiguousnessPoints[0]);
                                    MyPointsUsageItr = MyPointsUsage.insert(MyPointsUsageItr, UNKNOWN);
                                    PrevItr = Itr;
                                    --PrevItr;
                                    TheLiteSegment.SetStartPoint(*PrevItr);
                                    TheLiteSegment.SetEndPoint(*Itr);
                                    OtherItr = pio_rPolySegment.m_Points.begin();
                                    PrevOtherItr = OtherItr;
                                    ++OtherItr;
                                    MyOtherPointsUsageItr = MyPointsUsage.begin();
                                    ++MyOtherPointsUsageItr;

                                    }
                                }
                            else
                                {
                                // The second contiguousness point is closer to start point of segment
                                // Check if first point is equal to either extremities
                                if (!ContiguousnessPoints[0].IsEqualTo(*Itr, Tolerance))
                                    {
                                    // First point is different ... we insert
                                    Itr = pio_rPolySegment.m_Points.insert(Itr, ContiguousnessPoints[0]);
                                    MyPointsUsageItr = MyPointsUsage.insert(MyPointsUsageItr, UNKNOWN);
                                    PrevItr = Itr;
                                    --PrevItr;
                                    TheLiteSegment.SetStartPoint(*PrevItr);
                                    TheLiteSegment.SetEndPoint(*Itr);
                                    OtherItr = pio_rPolySegment.m_Points.begin();
                                    PrevOtherItr = OtherItr;
                                    ++OtherItr;
                                    MyOtherPointsUsageItr = MyPointsUsage.begin();
                                    ++MyOtherPointsUsageItr;
                                    }

                                // Indicate this point is the end of a contiguousness region
                                *MyPointsUsageItr = ON_POINT;

                                // Check if second point is equal to either extremities
                                if (!ContiguousnessPoints[1].IsEqualTo(*PrevItr, Tolerance))
                                    {
                                    // Second point is different ... we insert
                                    Itr = pio_rPolySegment.m_Points.insert(Itr, ContiguousnessPoints[1]);
                                    MyPointsUsageItr = MyPointsUsage.insert(MyPointsUsageItr, UNKNOWN);
                                    PrevItr = Itr;
                                    --PrevItr;
                                    TheLiteSegment.SetStartPoint(*PrevItr);
                                    TheLiteSegment.SetEndPoint(*Itr);
                                    OtherItr = pio_rPolySegment.m_Points.begin();
                                    PrevOtherItr = OtherItr;
                                    ++OtherItr;
                                    MyOtherPointsUsageItr = MyPointsUsage.begin();
                                    ++MyOtherPointsUsageItr;
                                    }

                                }
                            }
                        else
                            *MyPointsUsageItr = ON_POINT;
                        }
                    }
                }
            }
        }

    // The first point flag is a copy of last point flag
    MyPointsUsage[0] = MyPointsUsage[pio_rPolySegment.m_Points.size() - 1];

    // Copy point usage to array of usage
    HArrayAutoPtr<PointUsage> aFlags(new PointUsage[MyPointsUsage.size()]);

    MyPointsUsageItr = MyPointsUsage.begin();
    MyPointsUsageIndex = 0;
    for ( ; MyPointsUsageItr != MyPointsUsage.end(); ++MyPointsUsageItr, ++MyPointsUsageIndex)
        {
        aFlags[MyPointsUsageIndex] = *MyPointsUsageItr;
        }

    // Return flags
    return(aFlags.release());
    }



//-----------------------------------------------------------------------------
// InsertAutoFlirtPoints
// PRIVATE
// This method is for the exclusive use SuperScan2. The method is used
// to insert points at all places where the polygon auto-flirts. As
// a reminder, it is valid for a polygon of segment as for any
// shape to auto-flirt, however the process of SuperScan requires that at
// these auto-flirt points a point exist for both segments. This is the
// purpose of the funtion. The polygon is not modified, the list of points is
// used instead and must already contain a representation of the polygon
// after formatting by InteractsWith...()
//-----------------------------------------------------------------------------
void HVE2DPolygonOfSegments::InsertAutoFlirtPoints(HGF2DPositionCollection& pio_rPoints) const
    {
    HINVARIANTS;

    // The shape may not be empty
    HPRECONDITION(!IsEmpty());

    // Define tolerance
    double Tolerance = GetTolerance();

    // Declare two iterators ... one for previous point and one for current point
    HGF2DPositionCollection::iterator   SelfItr(pio_rPoints.begin());
    HGF2DPositionCollection::iterator   PrevSelfItr = SelfItr;
    for (++SelfItr ; SelfItr != pio_rPoints.end() ; ++SelfItr)
        {
        // Create segment
        HGF2DLiteSegment    SelfSegment(*PrevSelfItr, *SelfItr, Tolerance);

        double SelfXMin = MIN(SelfSegment.GetStartPoint().GetX(), SelfSegment.GetEndPoint().GetX());
        double SelfXMax = MAX(SelfSegment.GetStartPoint().GetX(), SelfSegment.GetEndPoint().GetX());
        double SelfYMin = MIN(SelfSegment.GetStartPoint().GetY(), SelfSegment.GetEndPoint().GetY());
        double SelfYMax = MAX(SelfSegment.GetStartPoint().GetY(), SelfSegment.GetEndPoint().GetY());

        // Declare iterator for test point
        HGF2DPositionCollection::iterator   OtherItr(pio_rPoints.begin());

        for (OtherItr ; OtherItr != pio_rPoints.end() ; ++OtherItr)
            {
            // Check if the point can possibly interact with segment
            bool Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, OtherItr->GetX(), Tolerance) &&
                            HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, OtherItr->GetX(), Tolerance) &&
                            HDOUBLE_GREATER_OR_EQUAL(SelfYMax, OtherItr->GetY(), Tolerance) &&
                            HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, OtherItr->GetY(), Tolerance)
                           );

            if (Result)
                {
                // The point is in general segment area ...

                // Check if this point is on segment
                if (SelfSegment.IsPointOn(*OtherItr))
                    {
                    // The point is on ... check if it is at an extremity
                    if (!SelfSegment.GetStartPoint().IsEqualTo(*OtherItr, Tolerance) &&
                        !SelfSegment.GetEndPoint().IsEqualTo(*OtherItr, Tolerance))
                        {
                        // The point connects upon segment but not on an extremity
                        // We therefore need to insert this point in segment
                        // Set end point of segments to point
                        SelfSegment.SetEndPoint(*OtherItr);

                        SelfItr = pio_rPoints.insert(SelfItr, *OtherItr);
                        PrevSelfItr = SelfItr;
                        --PrevSelfItr;

                        // We have lost hold on point iterator ... restart
                        // in fact the following will restart at second point but
                        // since at least one point has been processed ... it is ok.
                        OtherItr = pio_rPoints.begin();

                        // Update extent
                        SelfXMin = MIN(SelfSegment.GetStartPoint().GetX(), SelfSegment.GetEndPoint().GetX());
                        SelfXMax = MAX(SelfSegment.GetStartPoint().GetX(), SelfSegment.GetEndPoint().GetX());
                        SelfYMin = MIN(SelfSegment.GetStartPoint().GetY(), SelfSegment.GetEndPoint().GetY());
                        SelfYMax = MAX(SelfSegment.GetStartPoint().GetY(), SelfSegment.GetEndPoint().GetY());
                        }
                    }
                }
            }

        // Save previous self iterator
        PrevSelfItr = SelfItr;
        }
    }



//-----------------------------------------------------------------------------
// PRIVATE
//
//-----------------------------------------------------------------------------
void HVE2DPolygonOfSegments::ChangeShape(const HVE2DPolygonOfSegments* const* apPoly,
                                         const HGF2DPositionCollection**  apPoints,
                                         HGF2DLiteExtent* PolyExtents,
                                         PointUsage** Flags,
                                         bool* PolyIn,
                                         size_t& ShapeIndex,
                                         size_t& TestShapeIndex,
                                         size_t& Index,
                                         size_t& PrevIndex,
                                         HGF2DPosition& PreviousPoint,
                                         HGF2DPosition& CurrentPoint,
                                         double Tolerance) const


    {
    bool AbsNotFound = false;

    // Find a valid point on other shape
    AbsNotFound = !ChangeToShape(apPoly,
                                 apPoints,
                                 PolyExtents,
                                 Flags,
                                 PolyIn,
                                 ShapeIndex,
                                 TestShapeIndex,
                                 Index,
                                 PreviousPoint,
                                 Tolerance,
                                 false);


    // Check if a point was found
    if (AbsNotFound)
        {
        // A point was not found... this happens
        // We try to obtain a point on current shape ...
        AbsNotFound = !ChangeToShape(apPoly,
                                     apPoints,
                                     PolyExtents,
                                     Flags,
                                     PolyIn,
                                     ShapeIndex,
                                     TestShapeIndex,
                                     Index,
                                     PreviousPoint,
                                     Tolerance,
                                     false);

        // Check if a point was found
        if(AbsNotFound)
            {

            AbsNotFound = !ChangeToShape(apPoly,
                                         apPoints,
                                         PolyExtents,
                                         Flags,
                                         PolyIn,
                                         ShapeIndex,
                                         TestShapeIndex,
                                         Index,
                                         PreviousPoint,
                                         Tolerance,
                                         true);

            if (AbsNotFound)
                {
                // A point was not found... this happens
                // We try to obtain a point on current shape ...
                AbsNotFound = !ChangeToShape(apPoly,
                                             apPoints,
                                             PolyExtents,
                                             Flags,
                                             PolyIn,
                                             ShapeIndex,
                                             TestShapeIndex,
                                             Index,
                                             PreviousPoint,
                                             Tolerance,
                                             true);


                if (AbsNotFound)
                    {
                    // No point found ...
                    throw HVEDecompositionException();
                    }
                }
            }
        }


    // A point was found on either shape followed ... set the previuos index and current point.
    if (Index >= apPoints[ShapeIndex]->size() - 2)
        {
        // Index overflow ... we take first point
        PrevIndex = apPoints[ShapeIndex]->size() - 2;
        Index = 0;
        }
    else
        {
        // No overflow ... take next point
        PrevIndex = Index;
        Index++;
        }

    CurrentPoint = (*(apPoints[ShapeIndex]))[Index];
    }


//-----------------------------------------------------------------------------
// PRIVATE
//
// OnAccepted - Indicates if ON segment may be considered as a valid change.
//-----------------------------------------------------------------------------
bool HVE2DPolygonOfSegments::ChangeToShape(const HVE2DPolygonOfSegments* const* apPoly,
                                           const HGF2DPositionCollection**  apPoints,
                                           HGF2DLiteExtent* PolyExtents,
                                           PointUsage** Flags,
                                           bool* PolyIn,
                                           size_t& ShapeIndex,
                                           size_t& TestShapeIndex,
                                           size_t& Index,
                                           HGF2DPosition& PreviousPoint,
                                           double Tolerance,
                                           bool OnAccepted) const
    {
    // Declare and initialise return value
    bool PointIsCorrect = false;

    HVE2DShape::SpatialPosition MyPartPosition;
    long LastIndex = -1;
    long LastDesesperateIndex = -1;

    // Change shape followed
    std::swap(ShapeIndex, TestShapeIndex);

    // While point not found or no valid points to be found ...
    do
        {
        // Find the first point that is equat to previous point and has not been used yet
        for (Index = LastIndex + 1;
             ((Index < (apPoints[ShapeIndex]->size() - 1)) &&
              (((long)Index <= LastIndex) || (Flags[ShapeIndex][Index] == USED) || !PreviousPoint.IsEqualTo((*(apPoints[ShapeIndex]))[Index], Tolerance))) ;
             Index++)
            ;

        // Check if a point was found
        if (!(Index < (apPoints[ShapeIndex]->size() -  1)))
            {
            // No point were found ... sometimes occurs
            // This is our extremely rare case
            // We must fix things up before continuing
            // We search for a point that may have been used  but for which the next point has not been used.
            // This is required since sometimes points are used twice.
            bool FoundPoint = false;
            size_t FoundIndex=0;
            for (Index = LastDesesperateIndex + 1; !FoundPoint && (Index < apPoints[ShapeIndex]->size()) ; ++Index)
                {
                if (Index != LastIndex)
                    {

                    if (PreviousPoint.IsEqualTo((*apPoints[ShapeIndex])[Index], Tolerance))
                        {
                        // An equal point was found ... check if next point is used
                        if (Index == apPoints[ShapeIndex]->size() - 2)
                            {
                            FoundPoint = (Flags[ShapeIndex][0] != USED);
                            FoundIndex = Index;
                            }
                        else  if (Index <= apPoints[ShapeIndex]->size() - 2)
                            {
                           // If we were to enter this in the case where the scan searched through the whole list of points and
                            // did not found any (Index == size() -1) we would not be able to obtain the point.
                            FoundPoint = (Flags[ShapeIndex][Index+1] != USED);
                            FoundIndex = Index;
                            }
                        }
                    }
                LastDesesperateIndex = (long)Index;
                }
            if (FoundPoint)
                Index = FoundIndex;
            }

        // Check is a point was found (The last point is not considered as it is a duplicate of first point)
        if (Index < (apPoints[ShapeIndex]->size() - 1))
            {
            // A point was found ... we check if it is satisfactory

            // Get previous and current point for this found point
            HGF2DPosition   TentativeCurrentPoint;
            HGF2DPosition   TentativePreviousPoint((*(apPoints[ShapeIndex]))[Index]);

            if (Index == (apPoints[ShapeIndex]->size() - 2))
                {
                // Tentative previous is next to last point (last non-dup point) ...
                TentativeCurrentPoint = (*(apPoints[ShapeIndex]))[0];
                LastIndex = (long)apPoints[ShapeIndex]->size() - 2;
                }
            else
                {
                TentativeCurrentPoint = (*(apPoints[ShapeIndex]))[Index+1];
                LastIndex = (long)Index;
                }

            // Obtain spatial position
            MyPartPosition = apPoly[TestShapeIndex]->CalculateSpatialPositionOfPositionSegment2(PolyExtents[TestShapeIndex], TentativePreviousPoint, TentativeCurrentPoint, Tolerance);

            PointIsCorrect = ((OnAccepted) || (MyPartPosition != HVE2DShape::S_ON)) &&
                             (!((MyPartPosition == HVE2DShape::S_OUT && PolyIn[ShapeIndex]) ||
                                (MyPartPosition == HVE2DShape::S_IN && !PolyIn[ShapeIndex])));

            }
        }
    while (!PointIsCorrect && (Index < apPoints[ShapeIndex]->size()));


    return(PointIsCorrect);
    }


//-----------------------------------------------------------------------------
// Rasterize
// This method rasterizes (generates scanlines for) the polygon.
//-----------------------------------------------------------------------------
void HVE2DPolygonOfSegments::Rasterize(HGFScanLines& pio_rScanlines) const
    {
    if (pio_rScanlines.GetScanlinesCoordSys() == 0)
        {
        pio_rScanlines.SetScanlinesCoordSys(GetCoordSys());
        }
    HASSERT(pio_rScanlines.GetScanlinesCoordSys() == GetCoordSys());

    // Check if shape is not empty
    if (m_PolySegment.m_Points.size() >= 3)
        {
        if (!pio_rScanlines.LimitsAreSet())
            {
            HGF2DPositionCollection::const_iterator Itr = m_PolySegment.m_Points.begin();
            HGF2DPositionCollection::const_iterator NextItr = Itr;
            NextItr++;

            double YMin = Itr->GetY();
            double YMax = YMin;

            while (Itr->GetY() == NextItr->GetY())
                {
                ++Itr;
                ++NextItr;
                }

            size_t YPolarityChanges = 1;
            int8_t YPolarity = Itr->GetY() < NextItr->GetY() ? 1 : -1;

            for (; NextItr != m_PolySegment.m_Points.end() ; ++NextItr)
                {
                if (Itr->GetY() != NextItr->GetY())
                    {
                    int8_t CurrentPolarity = Itr->GetY() < NextItr->GetY() ? 1 : -1;
                    if (CurrentPolarity != YPolarity)
                        {
                        ++YPolarityChanges;
                        YPolarity = CurrentPolarity;
                        }

                    YMin = MIN(YMin, Itr->GetY());
                    YMax = MAX(YMax, Itr->GetY());
                    }

                // Advance iterator
                Itr = NextItr;
                }

            pio_rScanlines.SetLimits(YMin, YMax, YPolarityChanges);
            }


        pio_rScanlines.AddCrossingPointsForSegment(m_PolySegment.m_Points);
        }
    }


//-----------------------------------------------------------------------------
// PRIVATE METHOD
// Allocates a new polygon of segments in a parallelism preserving relation.
//-----------------------------------------------------------------------------
HVE2DVector* HVE2DPolygonOfSegments::AllocateCopyInParallismPreservingRelatedCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const
    {

    HPRECONDITION(GetCoordSys()->HasParallelismPreservingRelationTo(pi_rpCoordSys));

    HVE2DVector*    pResultVector = NULL;

    // The relation between coordinate system preserves parallelism. This
    // implies that the polysegment converted cannot twist upon itself
    // In other words, the lines of the poly segment cannot result in autocrossing.

    // We obtain a copy of the linear in given coord sys
    HVE2DLinear* pMyPrimaryResultLinear = static_cast<HVE2DLinear*>(m_PolySegment.AllocateCopyInCoordSys(pi_rpCoordSys));

    // Check if result linear is complex or polysegment
    HASSERT(pMyPrimaryResultLinear->IsABasicLinear());

    // A basic linear must be a polysegment
    HASSERT(((HVE2DBasicLinear*)(pMyPrimaryResultLinear))->GetBasicLinearType() == HVE2DPolySegment::CLASS_ID);

    // Cast into a polysegment
    HVE2DPolySegment* pMyPolySegment = static_cast<HVE2DPolySegment*>(pMyPrimaryResultLinear);

    // Check if there are at least 3 segments in result
    if (pMyPolySegment->GetSize() < 3)
        {
        // There are less than 3 segments ... empty shape
        pResultVector = new HVE2DVoidShape(pi_rpCoordSys);
        }
    else
        {
        // There are more than 2 segments ... check if area is not null
        HGF2DLocation   AreaOrigin = pMyPolySegment->GetPoint(pMyPolySegment->GetSize() -2);

        if (HDOUBLE_EQUAL(fabs(pMyPolySegment->CalculateRayArea(AreaOrigin)), 0.0,
                                                                         pMyPolySegment->GetTolerance() *
                                                                         pMyPolySegment->GetTolerance()))
            {
            // Area is null ... result is a void shape
            pResultVector = new HVE2DVoidShape(pi_rpCoordSys);
            }
        else
            {
            // Remove needles that may have formed. It is as fast to attempt
            // removal than to check for their presence.
            pMyPolySegment->RemoveAutoContiguousNeedles(true);

            if (pMyPolySegment->IsAutoContiguous())
                {
                // If there are still autocontiguousness points after all of these operations
                // We have a special case where before change of coordinate system two lines
                // are close to being contiguous but are not and afterwards they become
                // contiguous as a result of a global scaling by a small scale (<1).

                // The net result is in fact a surface containing the result polygons of segments
                // after removal of autocontiguousness segments.

                pResultVector = AllocateComplexShapeFromAutoContiguousPolySegment(*pMyPolySegment);

                }
            else
                {
                // Result linear is valid ...
                // we generate a polygon with it and return it
                pResultVector = new HVE2DPolygonOfSegments(*pMyPolySegment);
                }
            }
        }
    // We destroy the temporary linear
    delete pMyPrimaryResultLinear;

    return pResultVector;
    }

//-----------------------------------------------------------------------------
// PRIVATE METHOD
// Allocates a new polygon of segments in a linearity preserving relation.
//-----------------------------------------------------------------------------
HVE2DVector* HVE2DPolygonOfSegments::AllocateCopyInLinearityPreservingRelatedCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const
    {
    HPRECONDITION(GetCoordSys()->HasLinearityPreservingRelationTo(pi_rpCoordSys));

    HVE2DVector*    pResultVector;

    // The relation between coordinate system preserves linearity. This
    // implies that the polysegment converted can twist upon itself
    // In other words, the lines of the poly segment can result in autocrossing.
    // However with a linearity preserving relation, lines remain lines
    // therefore a polysegment will remain a polysegment. The typical relation
    // is a projective. This means that a single point results in autocrossing conditions.
    // This point can be checked

    // Obtain a copy of the transformation model expressing the relation
    HFCPtr<HGF2DTransfoModel> pTransfoModel = GetCoordSys()->GetTransfoModelTo(pi_rpCoordSys);

    // A linearity preserving relation implies that the model can be represented
    // by a matrix
    HASSERT(pTransfoModel->CanBeRepresentedByAMatrix());

    // Analyse the transformation matrix to determine if our object can twist upon itself.

    // Obtain extent of self
    HGF2DExtent MyExtent(GetExtent());

    // Our polygon extent overlaps the discontinuity location ... This means that
    // The polysegment can twist upon itself (autocrossing possible in result)

    // We obtain a copy of the linear in given coord sys
    HVE2DLinear* pMyPrimaryResultLinear = static_cast<HVE2DLinear*>(m_PolySegment.AllocateCopyInCoordSys(pi_rpCoordSys));

    // Check if result linear is complex or polysegment
    HASSERT(pMyPrimaryResultLinear->IsABasicLinear());

    // A basic linear must be a polysegment
    HASSERT(((HVE2DBasicLinear*)(pMyPrimaryResultLinear))->GetBasicLinearType() == HVE2DPolySegment::CLASS_ID);

    // Cast into a polysegment
    HVE2DPolySegment* pMyPolySegment = static_cast<HVE2DPolySegment*>(pMyPrimaryResultLinear);

    // Check if there are at least 3 segments in result
    if (pMyPolySegment->GetSize() < 3)
        {
        // There are less than 3 segments ... empty shape
        pResultVector = new HVE2DVoidShape(pi_rpCoordSys);
        }
    else
        {
        // There are more than 2 segments ... check if area is not null
        HGF2DLocation   AreaOrigin = pMyPolySegment->GetPoint(pMyPolySegment->GetSize() -2);

        if (HDOUBLE_EQUAL(fabs(pMyPolySegment->CalculateRayArea(AreaOrigin)), 0.0,
                                                                         pMyPolySegment->GetTolerance() *
                                                                         pMyPolySegment->GetTolerance()))
            {
            // Area is null ... result is a void shape
            pResultVector = new HVE2DVoidShape(pi_rpCoordSys);
            }
        else
            {
            // Result linear is valid ...
            // Remove needles that may have formed. It is as fast to attempt
            // removal than to check for their presence.
            pMyPolySegment->RemoveAutoContiguousNeedles(true);

            if (pMyPolySegment->GetSize() < 3)
                {
                // There are less than 3 segments ... empty shape
                pResultVector = new HVE2DVoidShape(pi_rpCoordSys);
                }
            else if (pMyPolySegment->IsAutoContiguous())
                {
                // If there are still autocontiguousness points after all of these operations
                // We have a special case where before change of coordinate system two lines
                // are close to being contiguous but are not and afterwards they become
                // contiguous as a result of a global scaling by a small scale (<1).

                // The net result is in fact a surface containing the result polygons of segments
                // after removal of autocontiguousness segments.

                pResultVector = AllocateComplexShapeFromAutoContiguousPolySegment(*pMyPolySegment);

                }
            // We must now make sure if the polysegment autocrosses itself
            else if (pMyPolySegment->AutoCrosses())
                {
                // The polysegment autocrosses.
                // We will ask it to splits itself into non-autocrossings then generate the appropriate shape
                // The result may be a holed shape or a complex shape.
                pResultVector = HVE2DPolygonOfSegments::CreateShapeFromAutoCrossingPolySegment(*pMyPolySegment);
                }
            else
                {
                // The polysegment does not autocross ... we are ok
                // we generate a polygon with it and return it
                pResultVector = new HVE2DPolygonOfSegments(*pMyPolySegment);
                }
            }
        }
    // We destroy the temporary linear
    delete pMyPrimaryResultLinear;

    return pResultVector;
    }

//-----------------------------------------------------------------------------
// PRIVATE METHOD
// Allocates a new polygon of segments in a general non-linearity preserving relation.
//-----------------------------------------------------------------------------
HVE2DVector* HVE2DPolygonOfSegments::AllocateCopyInGeneralRelatedCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const
    {
    HVE2DVector*    pResultVector = NULL;

    // general case
    // In such case, anything is possible such as transformation of lines into
    // curves with autocrossing points etc.
    // We obtain a copy of the linear in given coord sys
    HVE2DLinear* pMyPrimaryResultLinear = static_cast<HVE2DLinear*>(m_PolySegment.AllocateCopyInCoordSys(pi_rpCoordSys));

    HASSERTDUMP2(pMyPrimaryResultLinear->GetStartPoint().IsEqualTo(pMyPrimaryResultLinear->GetEndPoint()), *this, *pMyPrimaryResultLinear);


    // Check if result linear is complex or polysegment
    if (pMyPrimaryResultLinear->IsABasicLinear())
        {
        // A basic linear must be a polysegment
        HASSERT(((HVE2DBasicLinear*)(pMyPrimaryResultLinear))->GetBasicLinearType() == HVE2DPolySegment::CLASS_ID);

        // Cast into a polysegment
        HVE2DPolySegment* pMyPolySegment = static_cast<HVE2DPolySegment*>(pMyPrimaryResultLinear);

        // There are more than 2 segments ... check if area is not null
        // Remove needles that may have formed. It is as fast to attempt
        // removal than to check for their presence.
        // Save a copy of polysegment for dumping ...
        HDEBUGCODE(HVE2DPolySegment OriginalPolySegment(*pMyPolySegment));

        pMyPolySegment->RemoveAutoContiguousNeedles(true);

        // Check that after removal of needles if it is still auto contiguous
        // then it is not because of autocontiguousness needles
        HDEBUGCODE(HVE2DPolySegment NewPolySegment(*pMyPolySegment));
        HDEBUGCODE(NewPolySegment.RemoveAutoContiguousNeedles(true));
        HASSERTDUMP(!pMyPolySegment->IsAutoContiguous() || NewPolySegment.IsAutoContiguous(), OriginalPolySegment);


        // Check if there are at least 3 segments in result
        if (pMyPolySegment->GetSize() < 3)
            {
            // There are less than 3 segments ... empty shape
            pResultVector = new HVE2DVoidShape(pi_rpCoordSys);
            }
        else
            {
            HGF2DLocation   AreaOrigin = pMyPolySegment->GetPoint(pMyPolySegment->GetSize() -2);

            if (HDOUBLE_EQUAL(fabs(pMyPolySegment->CalculateRayArea(AreaOrigin)), 0.0,
                                                                             pMyPolySegment->GetTolerance() *
                                                                             pMyPolySegment->GetTolerance()))
                {
                // Area is null ... result is a void shape
                pResultVector = new HVE2DVoidShape(pi_rpCoordSys);
                }
            else
                {
                if (pMyPolySegment->IsAutoContiguous())
                    {
                    // If there are still autocontiguousness points after all of these operations
                    // We have a special case where before change of coordinate system two lines
                    // are close to being contiguous but are not and afterwards they become
                    // contiguous as a result of a global scaling by a small scale (<1).

                    // The net result is in fact a surface containing the result polygons of segments
                    // after removal of autocontiguousness segments.

                    pResultVector = AllocateComplexShapeFromAutoContiguousPolySegment(*pMyPolySegment);

                    }
                // We must now make sure if the polysegment autocrosses itself
                else if (pMyPolySegment->AutoCrosses())
                    {
                    // The polysegment autocrosses.
                    // We will ask it to splits itself into non-autocrossings then generate the appropriate shape
                    // The result may be a holed shape or a complex shape.
                    pResultVector = HVE2DPolygonOfSegments::CreateShapeFromAutoCrossingPolySegment(*pMyPolySegment);
                    }
                else
                    {
                    // The polysegment does not autocross ... we are ok
                    // we generate a polygon with it and return it
                    pResultVector = new HVE2DPolygonOfSegments(*pMyPolySegment);
                    }
                }
            }
        }
    else
        {
        // The linear must be complex
        HASSERT(pMyPrimaryResultLinear->IsComplex());

        // Cast
        HVE2DComplexLinear* pMyResultLinear = static_cast<HVE2DComplexLinear*>(pMyPrimaryResultLinear);

        // Check if there are at least 3 segments in result
        if (pMyResultLinear->GetNumberOfLinears() < 3)
            {
            // There are less than 3 segments ... empty shape
            pResultVector = new HVE2DVoidShape(pi_rpCoordSys);
            }
        else
            {
            // There are more than 2 segments ... check if area is not null
            HGF2DLocation   AreaOrigin = pMyResultLinear->GetLinear(pMyResultLinear->GetNumberOfLinears() - 1).GetStartPoint();

            if (HDOUBLE_EQUAL(fabs(pMyResultLinear->CalculateRayArea(AreaOrigin)), 0.0,
                                                                              pMyResultLinear->GetTolerance() *
                                                                              pMyResultLinear->GetTolerance()))
                {
                // Area is null ... result is a void shape
                pResultVector = new HVE2DVoidShape(pi_rpCoordSys);
                }
            else
                {
                // Result linear is valid ...

                // We must now make sure if the linear autocrosses itself
                if (pMyResultLinear->AutoCrosses())
                    {
                    // The polysegment autocrosses.

                    // This is not yet implemented.
// HChk AR !!!!! Difficult to implement ... we will see
                    HASSERT(0);
                    }
                else
                    {
                    // we generate a polygon with it and return it
                    pResultVector = new HVE2DPolygon(*pMyResultLinear);
                    }
                }
            }
        }
    // We destroy the temporary linear
    delete pMyPrimaryResultLinear;

    return pResultVector;
    }

