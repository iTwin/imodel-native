//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hve/src/HVE2DPolySegment.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HVE2DPolySegment
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <ImagePP/all/h/HGFAngle.h>
#include <ImagePP/all/h/HGF2DSimilitude.h>
#include <ImagePP/all/h/HGF2DLiteSegment.h>
#include <ImagePP/all/h/HVE2DPolySegment.h>
#include <ImagePP/all/h/HVE2DSegment.h>

#include <ImagePP/all/h/HFCException.h>

HPM_REGISTER_CLASS(HVE2DPolySegment, HVE2DLinear)



/** -----------------------------------------------------------------------------
    Finds and returns all auto intersection points.

    KNOWN BUG: Duplicate auto intersection points are not usually returned. If
               the polysegment autocrosses many times at the same location
               it is quite possible that less crossing (but at least 1) will be reported
               and added to cross points

    KNOWN BUG : If autocrossing occurs at an autocontiguous region then 2 cross points
                may be reported for each of these autocontioguousness region where
                autocrossing occurs. The location of these cross points may be different
                by still be part of the same autocontiguous region.

    @param po_pPoints IN OUT A pointer to a collection of points to which will be
                      added auto intersection points. All points in the collection
                      prior to call are untouched and remain in the list.

    @return The number of new intersection points found

    @see IntersectsAtSplitPointWithPolySegment()
    -----------------------------------------------------------------------------
*/
size_t HVE2DPolySegment::AutoIntersect(HGF2DLocationCollection* po_pPoints) const
    {
    HINVARIANTS;
    
    HGF2DPositionCollection points;
    GetPolySegmentPeer().AutoIntersect(&points);
    
    for (auto currentPoint: points)
        po_pPoints->push_back(HGF2DLocation(currentPoint, GetCoordSys()));
    
    return points.size();
    }


//-----------------------------------------------------------------------------
// GetClosestSegment
//-----------------------------------------------------------------------------
HVE2DSegment HVE2DPolySegment::GetClosestSegment(const HGF2DLocation& pi_rPoint) const
    {
    HINVARIANTS;

    HGF2DPosition point = pi_rPoint.ExpressedIn(GetCoordSys()).GetPosition();
    HGF2DSegment closestSegment = GetPolySegmentPeer().GetClosestSegment(point);
    
    return HVE2DSegment(HGF2DLocation(closestSegment.GetStartPoint(), GetCoordSys()), HGF2DLocation(closestSegment.GetEndPoint(), GetCoordSys()));
    
    }





//-----------------------------------------------------------------------------
// AllocateParallelCopy
//-----------------------------------------------------------------------------
HVE2DPolySegment*  HVE2DPolySegment::AllocateParallelCopy(double                          pi_Offset,
                                                          HVE2DVector::ArbitraryDirection pi_DirectionToRight,
                                                          const HGF2DLine*                pi_pFirstPointAlignment,
                                                          const HGF2DLine*                pi_pLastPointAlignment) const

    {
        
    // The offset distance is greater than 0.0
    HPRECONDITION(pi_Offset > 0.0);

    
    
    HAutoPtr<HGF2DLiteLine> firstPointLine;
    if (nullptr != pi_pFirstPointAlignment)
        {
        HGF2DLine firstPointLineCopy = *pi_pFirstPointAlignment;
        firstPointLineCopy.ChangeCoordSys(GetCoordSys());
        firstPointLine = new HGF2DLiteLine(firstPointLineCopy.GetSlope(), firstPointLineCopy.GetIntercept());
        }   

    HAutoPtr<HGF2DLiteLine> lastPointLine;
    if (nullptr != pi_pLastPointAlignment)
        {
        HGF2DLine lastPointLineCopy = *pi_pLastPointAlignment;
        lastPointLineCopy.ChangeCoordSys(GetCoordSys());
        lastPointLine = new HGF2DLiteLine(lastPointLineCopy.GetSlope(), lastPointLineCopy.GetIntercept());
        }  
    HAutoPtr<HGF2DPolySegment> newPolySegment(GetPolySegmentPeer().AllocateParallelCopy(pi_Offset, 
                                                                                          (HGF2DVector::ArbitraryDirection)pi_DirectionToRight, 
                                                                                          firstPointLine.get(), 
                                                                                          lastPointLine.get()));

    return new HVE2DPolySegment(newPolySegment.release(), GetCoordSys());
    }


//-----------------------------------------------------------------------------
// RemoveAutoContiguousNeedles
// PROTECTED
// This method removes the autocontiguousness needles that can be found in the
// polysegment. Needles are formed when two consecutive segments are contiguous
// to one another. Removal of this needles calls for the removal of the
// shortest of the two segments
//-----------------------------------------------------------------------------
void HVE2DPolySegment::RemoveAutoContiguousNeedles(bool pi_ClosedProcessing)
    {
    HINVARIANTS;

    // If closed processing is required, then the polysegment must auto-close
    HPRECONDITION(!pi_ClosedProcessing || GetStartPoint().IsEqualTo(GetEndPoint()));

    GetPolySegmentPeer().RemoveAutoContiguousNeedles(pi_ClosedProcessing);

    // This may modify the start and end point so we reset
    SetLinearStartPoint(HGF2DLocation(GetPolySegmentPeer().GetStartPoint(), GetCoordSys()));
    SetLinearEndPoint(HGF2DLocation(GetPolySegmentPeer().GetEndPoint(), GetCoordSys()));

    // Indicate extent is not up to date anymore
    m_ExtentUpToDate = false;

    // Adjust tolerance
    ResetTolerance();
    }





//-----------------------------------------------------------------------------
// Reverse
// Reverses the linear
//-----------------------------------------------------------------------------
void HVE2DPolySegment::Reverse()
    {
    HINVARIANTS;
    
    // This call should reverse the peer
    HVE2DLinear::Reverse();
    }

//-----------------------------------------------------------------------------
// AutoCrosses
// Indicates if the linear crosses itself
// This function is incomplete. It may determine obvious auto crossing
// but may miss some crossing at junction points
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::AutoCrosses() const
    {
//HChk AR ... need to complete complex linear

    HINVARIANTS;

    return GetPolySegmentPeer().AutoCrosses();
    }



//-----------------------------------------------------------------------------
// IsAutoContiguous
// Indicates if the linear is contiguous to itself
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::IsAutoContiguous() const
    {
    HINVARIANTS;

    return GetPolySegmentPeer().IsAutoContiguous();
    }

//-----------------------------------------------------------------------------
// Constructor (by provided peer)
//-----------------------------------------------------------------------------
HVE2DPolySegment::HVE2DPolySegment(HGF2DPolySegment* peerPolySegment, const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
: HVE2DBasicLinear(pi_rpCoordSys),
  m_ExtentUpToDate(false),
  m_Extent(pi_rpCoordSys)
    {
    m_VolatilePeer = false;
    m_Peer = peerPolySegment;
    SetLinearStartPoint(HGF2DLocation(peerPolySegment->GetStartPoint(), pi_rpCoordSys));
    SetLinearEndPoint(HGF2DLocation(peerPolySegment->GetEndPoint(), pi_rpCoordSys));

    SetAutoToleranceActive(peerPolySegment->IsAutoToleranceActive());
    SetTolerance(peerPolySegment->GetTolerance());
    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// Constructor (by list of positions)
//-----------------------------------------------------------------------------
HVE2DPolySegment::HVE2DPolySegment(const HGF2DPositionCollection& pi_rListOfPoints,
                                   const HFCPtr<HGF2DCoordSys>&   pi_rpCoordSys)
    : HVE2DBasicLinear(pi_rpCoordSys),
      m_ExtentUpToDate(false),
      m_Extent(pi_rpCoordSys)
    {
    m_VolatilePeer = false;

    CreatePolySegmentPeer(pi_rListOfPoints);
    // The list of points must contain 2 or more points
    // 0 or 1 point is illegal
    HPRECONDITION(pi_rListOfPoints.size() >= 2);
	

	
    // We set start point
    SetLinearStartPoint(HGF2DLocation(*(pi_rListOfPoints.begin()), pi_rpCoordSys));

    // Set end point
    SetLinearEndPoint(HGF2DLocation(*(pi_rListOfPoints.rbegin()), pi_rpCoordSys));
	
    //GetPolySegmentPeer().SetLinearStartPoint(GetStartPoint().GetPosition());
    //GetPolySegmentPeer().SetLinearEndPoint(GetEndPoint().GetPosition());

    // Indicate extent is not up to date anymore
    m_ExtentUpToDate = false;

    // Adjust tolerance
    ResetTolerance();

    GetPolySegmentPeer().SetLinearStartPoint(GetStartPoint().GetPosition());
    GetPolySegmentPeer().SetLinearEndPoint(GetEndPoint().GetPosition());

    HINVARIANTS;
    }


//-----------------------------------------------------------------------------
// Constructor (by collection of locations)
//-----------------------------------------------------------------------------
HVE2DPolySegment::HVE2DPolySegment(const HGF2DLocationCollection& pi_rListOfPoints)
    : HVE2DBasicLinear(pi_rListOfPoints.begin()->GetCoordSys()),
      m_ExtentUpToDate(false),
      m_Extent(pi_rListOfPoints.begin()->GetCoordSys())
    {
    // The list of points must contain 2 or more points
    // 0 or 1 point is illegal
    HPRECONDITION(pi_rListOfPoints.size() >= 2);

    m_VolatilePeer = false;

    CreateEmptyPolySegmentPeer();

    // We set start point
    SetLinearStartPoint(*(pi_rListOfPoints.begin()));

    // Set end point
    SetLinearEndPoint(HGF2DLocation(*(pi_rListOfPoints.rbegin()), GetCoordSys()));

    GetPolySegmentPeer().SetLinearStartPoint(GetStartPoint().GetPosition());
    GetPolySegmentPeer().SetLinearEndPoint(GetEndPoint().GetPosition());

    // We copy the locations to the position collection
    HGF2DLocationCollection::const_iterator Itr(pi_rListOfPoints.begin());

    for(; Itr != pi_rListOfPoints.end() ; ++Itr)
        {
        // Obtain point expressed in current coordinate system
        HGF2DLocation ThePoint(*Itr, GetCoordSys());

        // Add to list of positions
        GetPoints().push_back(HGF2DPosition(ThePoint.GetX(), ThePoint.GetY()));
        }

    // Reset tolerance
    ResetTolerance();

    GetPolySegmentPeer().SetLinearStartPoint(GetStartPoint().GetPosition());
    GetPolySegmentPeer().SetLinearEndPoint(GetEndPoint().GetPosition());

    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// Constructor with an array of values
//-----------------------------------------------------------------------------
HVE2DPolySegment::HVE2DPolySegment(size_t pi_BufferLength,
                                   double pi_aBuffer[],
                                   const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HVE2DBasicLinear(pi_rpCoordSys),
      m_ExtentUpToDate(false),
      m_Extent(pi_rpCoordSys)
    {
    // There must be at least 2 points provided
    HPRECONDITION(pi_BufferLength >= 4);

    // There must be an even number of values
    HPRECONDITION(pi_BufferLength % 2 == 0);

    m_VolatilePeer = false;

    CreateEmptyPolySegmentPeer();

    // Preallocate points
    GetPoints().reserve(pi_BufferLength / 2);

    // Copy points
    for (size_t Index = 0 ; Index < pi_BufferLength ; Index += 2)
        {
        GetPoints().push_back(HGF2DPosition(pi_aBuffer[Index], pi_aBuffer[Index + 1]));
        }

    // Set start and end points
    SetLinearStartPoint(HGF2DLocation(GetPoints().begin()->GetX(),
                                 GetPoints().begin()->GetY(),
                                 pi_rpCoordSys));
    SetLinearEndPoint(HGF2DLocation(GetPoints().rbegin()->GetX(),
                               GetPoints().rbegin()->GetY(),
                               pi_rpCoordSys));


    GetPolySegmentPeer().SetLinearStartPoint(GetStartPoint().GetPosition());
    GetPolySegmentPeer().SetLinearEndPoint(GetEndPoint().GetPosition());

    ResetTolerance();


    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// CalculateBearing
// Calculates and returns the bearing of polysegment at specified point
//-----------------------------------------------------------------------------
HGFBearing HVE2DPolySegment::CalculateBearing(const HGF2DLocation& pi_rPoint,
                                              HVE2DVector::ArbitraryDirection pi_Direction) const
    {
    HINVARIANTS;

    // The segment must not be NULL
    HPRECONDITION(GetPoints().size() != 0);

    // The point must be located on polysegment
    HASSERTSUPERDEBUG(IsPointOn(pi_rPoint));

    return GetPolySegmentPeer().CalculateBearing(pi_rPoint.GetPosition(), (HGF2DVector::ArbitraryDirection)pi_Direction);
    }

//-----------------------------------------------------------------------------
// GetExtent
// Returns the extent of the polysegment
//-----------------------------------------------------------------------------
HGF2DExtent HVE2DPolySegment::GetExtent() const
    {
    HINVARIANTS;

    HGF2DLiteExtent extent = GetPolySegmentPeer().GetExtent();
    if (extent.IsDefined())
        m_Extent = HGF2DExtent(extent.GetXMin(), extent.GetYMin(), extent.GetXMax(), extent.GetYMax(), GetCoordSys());
    else
        m_Extent = HGF2DExtent(GetCoordSys());
    m_ExtentUpToDate = true;
    return m_Extent;
    }

//-----------------------------------------------------------------------------
// Move
// This method moves the polysegment by the specified displacement
//-----------------------------------------------------------------------------
void HVE2DPolySegment::Move(const HGF2DDisplacement& pi_rDisplacement)
    {
    HINVARIANTS;

    // Following will call move on peer
    HVE2DLinear::Move(pi_rDisplacement);    
    }

//-----------------------------------------------------------------------------
// Scale
// This method scales the polysegment by the specified scaling factor
// around the given location
//-----------------------------------------------------------------------------
void HVE2DPolySegment::Scale(double pi_ScaleFactor, const HGF2DLocation& pi_rScaleOrigin)
    {
    // The given scale must be different from 0.0
    HPRECONDITION(pi_ScaleFactor != 0.0);

    // Following will call scale on peer
    HVE2DLinear::Scale(pi_ScaleFactor, pi_rScaleOrigin);    
    HINVARIANTS;

    }


//-----------------------------------------------------------------------------
// Scale
// This method scales the polygon by the specified scaling factor
// around the given location
//-----------------------------------------------------------------------------
void HVE2DPolySegment::Scale(double              pi_ScaleFactorX,
                             double              pi_ScaleFactorY,
                             const HGF2DLocation& pi_rScaleOrigin)
    {
    HINVARIANTS;

    // The given scale factors must be different from 0.0
    HPRECONDITION(pi_ScaleFactorX != 0.0);
    HPRECONDITION(pi_ScaleFactorY != 0.0);

    GetPolySegmentPeer().Scale(pi_ScaleFactorX, pi_ScaleFactorY, pi_rScaleOrigin.ExpressedIn(GetCoordSys()).GetPosition());

    SetLinearStartPoint(HGF2DLocation(GetPolySegmentPeer().GetStartPoint(), GetCoordSys()));
    SetLinearEndPoint(HGF2DLocation(GetPolySegmentPeer().GetEndPoint(), GetCoordSys())); 

    // Indicate extent is not up to date anymore
    m_ExtentUpToDate = false;

    // Adjust tolerance
    ResetTolerance();   

    }

//-----------------------------------------------------------------------------
// SetCoordSysImplementation
//-----------------------------------------------------------------------------
void HVE2DPolySegment::SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_pCoordSys)
    {
    HINVARIANTS;

    HVE2DBasicLinear::SetCoordSysImplementation(pi_pCoordSys);

    m_ExtentUpToDate = false;
    }


//-----------------------------------------------------------------------------
// AppendPoint
// Add a point at the end of polysegment
//-----------------------------------------------------------------------------
void HVE2DPolySegment::AppendPoint(const HGF2DLocation& pi_rNewPoint)
    {
    HINVARIANTS;

    // Obtain a copy of point in self coordinate system
    HGF2DLocation ThePoint(pi_rNewPoint, GetCoordSys());

    // Check if it is the first point
    if (GetPolySegmentPeer().GetSize() == 0)
        {
        // It is the first point ... we must set start point
        SetLinearStartPoint(ThePoint);
        GetPolySegmentPeer().SetLinearStartPoint(GetStartPoint().GetPosition());
        }
        
    GetPolySegmentPeer().AppendPoint(ThePoint.GetPosition());

    // In all case, this point becomes the end point
    SetLinearEndPoint(ThePoint);

    // Indicate extent is not up to date anymore
    m_ExtentUpToDate = false;

    // Adjust tolerance
    ResetTolerance();
    }

//-----------------------------------------------------------------------------
// AppendPosition
// Add a point at the end of polysegment
//-----------------------------------------------------------------------------
void HVE2DPolySegment::AppendPosition(const HGF2DPosition& pi_rNewPoint)
    {
    HINVARIANTS;

    // Check if it is the first point
    if (GetPolySegmentPeer().GetSize() == 0)
        {
        // It is the first point ... we must set start point
        SetLinearStartPoint(HGF2DLocation(pi_rNewPoint, GetCoordSys()));
        GetPolySegmentPeer().SetLinearStartPoint(pi_rNewPoint);

        }
        
    GetPolySegmentPeer().AppendPoint(pi_rNewPoint);

    // In all case, this point becomes the end point
    SetLinearEndPoint(HGF2DLocation(pi_rNewPoint, GetCoordSys()));

    // Indicate extent is not up to date anymore
    m_ExtentUpToDate = false;

    // Adjust tolerance
    ResetTolerance();
    }



/** -----------------------------------------------------------------------------
    Removes a point from the polysegment.

    @param pi_Index IN Index of the point ot remove. The index must be between
                    0 and the number of points in polysegment minus 1.

    @see GetSize()
    @bsimethod                                          AlainRobert 2003/09/02
    -----------------------------------------------------------------------------
*/
void HVE2DPolySegment::RemovePoint(size_t pi_Index)
    {
    HINVARIANTS;

    // Make sure that the index is valid
    HPRECONDITION((pi_Index >= 0) && (pi_Index < GetPolySegmentPeer().GetSize()));

    GetPolySegmentPeer().RemovePoint(pi_Index);

    if (GetPolySegmentPeer().GetSize() != 0)
        {
        // Reset start and end point if required
        if (pi_Index == 0)
            {
            SetLinearStartPoint(HGF2DLocation(GetPolySegmentPeer().GetStartPoint(), GetCoordSys()));
            GetPolySegmentPeer().SetLinearStartPoint(GetStartPoint().GetPosition());

            }
        if (pi_Index == GetPolySegmentPeer().GetSize())
            {
            SetLinearEndPoint(HGF2DLocation(GetPolySegmentPeer().GetEndPoint(), GetCoordSys()));
            GetPolySegmentPeer().SetLinearEndPoint(GetEndPoint().GetPosition());

            }
        }

    // Indicate extent is not up to date anymore
    m_ExtentUpToDate = false;

    // Adjust tolerance
    ResetTolerance();
    }


//-----------------------------------------------------------------------------
// Rotate
// Rotates the polysegment
//-----------------------------------------------------------------------------
void HVE2DPolySegment::Rotate(double               pi_Angle,
                              const HGF2DLocation& pi_rOrigin)
    {
    HINVARIANTS;

    // Create a location in current coordinate system
    HGF2DLocation  Origin(pi_rOrigin, GetCoordSys());

    GetPolySegmentPeer().Rotate(pi_Angle, pi_rOrigin.ExpressedIn(GetCoordSys()).GetPosition());

    SetLinearStartPoint(HGF2DLocation(GetPolySegmentPeer().GetStartPoint(), GetCoordSys()));
    SetLinearEndPoint(HGF2DLocation(GetPolySegmentPeer().GetEndPoint(), GetCoordSys()));    
    GetPolySegmentPeer().SetLinearStartPoint(GetStartPoint().GetPosition());
    GetPolySegmentPeer().SetLinearEndPoint(GetEndPoint().GetPosition());

    // Indicate extent is not up to date anymore
    m_ExtentUpToDate = false;

    // Adjust tolerance
    ResetTolerance();
    }

//-----------------------------------------------------------------------------
// AreContiguous
// Indicates if the polysegment and given vector are contiguous
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::AreContiguous(const HVE2DVector& pi_rVector) const
    {
    HINVARIANTS;


    // Declare recipient variable
    bool   Answer = false;

    // Check if the given vector is a basic linear
    if ((pi_rVector.GetMainVectorType() == HVE2DLinear::CLASS_ID) &&
        ((static_cast<const HVE2DLinear&>(pi_rVector)).IsABasicLinear()))
        {

        // The given is a basic linear ... of what type ?

        // First cast as a basic linear
        const HVE2DBasicLinear& rBasicLinear = static_cast<const HVE2DBasicLinear&>(pi_rVector);

        // Depending on basic linear type
        if (rBasicLinear.GetBasicLinearType() == HVE2DPolySegment::CLASS_ID)
            {
            // The vector is a polysegment

            // Cast as a polysegment
            const HVE2DPolySegment& rGivenPolySegment = static_cast<const HVE2DPolySegment&>(rBasicLinear);

            // Check if they are contiguous
            Answer = IsContiguousToPolySegment(rGivenPolySegment);
            }
        else if (rBasicLinear.GetBasicLinearType() == HVE2DSegment::CLASS_ID)
            {
            // The vector is a segment

            // Cast as a segment
            const HVE2DSegment& rGivenSegment = static_cast<const HVE2DSegment&>(rBasicLinear);

            // Check if contiguous
            Answer = IsContiguousToSegment(rGivenSegment);
            }
        else
            {
            // Unknown basic linear type ... we ask to given for answer
            Answer = pi_rVector.AreContiguous(*this);
            }
        }
    else
        {
        // The given vector is not a linear, or at least not a basic linear
        // We ask to given for answer
        Answer = pi_rVector.AreContiguous(*this);
        }

    return(Answer);
    }

//-----------------------------------------------------------------------------
// AreContiguousAt
// Indicates if the linear is contiguous to the given at given point
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::AreContiguousAt(const HVE2DVector& pi_rVector,
                                        const HGF2DLocation& pi_rPoint) const
    {
    HINVARIANTS;

    // The given point must be located on both vectors
    HASSERTSUPERDEBUG(IsPointOn(pi_rPoint) && pi_rVector.IsPointOn(pi_rPoint));

    // Declare recipient variable
    bool   Answer = false;

    // Check if the given vector is a basic linear
    if ((pi_rVector.GetMainVectorType() == HVE2DLinear::CLASS_ID) &&
        ((static_cast<const HVE2DLinear&>(pi_rVector)).IsABasicLinear()))
        {

        // The given is a basic linear ... of what type ?

        // First cast as a basic linear
        const HVE2DBasicLinear& rBasicLinear = static_cast<const HVE2DBasicLinear&>(pi_rVector);

        // Depending on basic linear type
        if (rBasicLinear.GetBasicLinearType() == HVE2DPolySegment::CLASS_ID)
            {
            // The vector is a polysegment

            // Cast as a polysegment
            const HVE2DPolySegment& rGivenPolySegment = static_cast<const HVE2DPolySegment&>(rBasicLinear);

            // Check if they are contiguous
            Answer = IsContiguousToPolySegmentAt(rGivenPolySegment, pi_rPoint);
            }
        else if (rBasicLinear.GetBasicLinearType() == HVE2DSegment::CLASS_ID)
            {
            // Cast as a segment
            const HVE2DSegment& rGivenSegment = static_cast<const HVE2DSegment&>(rBasicLinear);

            // Check if contiguous
            Answer = IsContiguousToSegmentAt(rGivenSegment, pi_rPoint);
            }
        else
            {
            // Unknown basic linear type ... we ask to given for answer
            Answer = pi_rVector.AreContiguousAt(*this, pi_rPoint);
            }
        }
    else
        {
        // The given vector is not a linear, or at least not a basic linear
        // We ask to given for answer
        Answer = pi_rVector.AreContiguousAt(*this, pi_rPoint);
        }

    return(Answer);
    }

//-----------------------------------------------------------------------------
// Crosses
// Indicates if the polysegment and given vector cross each other
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::Crosses(const HVE2DVector& pi_rVector) const
    {
    HINVARIANTS;

    // Declare recipient variable
    bool   Answer = false;

    // Check if self is not empty
    if (GetPoints().size() >= 2)
        {
        // Check if the given vector is a basic linear
        if ((pi_rVector.GetMainVectorType() == HVE2DLinear::CLASS_ID) &&
            ((static_cast<const HVE2DLinear&>(pi_rVector)).IsABasicLinear()))
            {

            // The given is a linear ... of what type ?

            // First cast as a basic linear
            const HVE2DBasicLinear& rBasicLinear = static_cast<const HVE2DBasicLinear&>(pi_rVector);

            // Depending on basic linear type
            if (rBasicLinear.GetBasicLinearType() == HVE2DPolySegment::CLASS_ID)
                {
                // The vector is a polysegment
                // Cast as a polysegment
                const HVE2DPolySegment& rGivenPolySegment = static_cast<const HVE2DPolySegment&>(rBasicLinear);

                // Check if they cross
                Answer = CrossesPolySegment(rGivenPolySegment);
                }
            else if (rBasicLinear.GetBasicLinearType() == HVE2DSegment::CLASS_ID)
                {
                // The vector is a segment
                // Cast as a segment
                const HVE2DSegment& rGivenSegment = static_cast<const HVE2DSegment&>(rBasicLinear);

                // Check if they cross
                Answer = CrossesSegment(rGivenSegment);
                }
            else
                {
                // Unknown basic linear type ... we ask to given for answer
                Answer = pi_rVector.Crosses(*this);
                }
            }
        else
            {
            // The given vector is not a linear, or at least not a basic linear
            // We ask to given for answer
            Answer = pi_rVector.Crosses(*this);
            }
        }

    return(Answer);
    }

//-----------------------------------------------------------------------------
// AreAdjacent
// Indicates if the polysegment and given vector are adjacent
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::AreAdjacent(const HVE2DVector& pi_rVector) const
    {
    HINVARIANTS;

    // Declare recipient variable
    bool   Answer = false;

    // Check if self is not NULL
    if (GetPoints().size() >= 2)
        {
        // Check if the given vector is a basic linear
        if ((pi_rVector.GetMainVectorType() == HVE2DLinear::CLASS_ID) &&
            ((static_cast<const HVE2DLinear&>(pi_rVector)).IsABasicLinear()))
            {

            // The given is a linear ... of what type ?

            // First cast as a basic linear
            const HVE2DBasicLinear& rBasicLinear = static_cast<const HVE2DBasicLinear&>(pi_rVector);

            // Depending on basic linear type
            if (rBasicLinear.GetBasicLinearType() == HVE2DPolySegment::CLASS_ID)
                {
                // Cast as a polysegment
                // The vector is a polysegment
                const HVE2DPolySegment& rGivenPolySegment = static_cast<const HVE2DPolySegment&>(rBasicLinear);

                // Check if they are adjacent
                Answer = IsAdjacentToPolySegment(rGivenPolySegment);
                }
            else if (rBasicLinear.GetBasicLinearType() == HVE2DSegment::CLASS_ID)
                {
                // Cast as a segment
                // The vector is a segment
                const HVE2DSegment& rGivenSegment = static_cast<const HVE2DSegment&>(rBasicLinear);

                // Check if adjacent
                Answer = IsAdjacentToSegment(rGivenSegment);
                }
            else
                {
                // Unknown basic linear type ... we ask to given for answer
                Answer = pi_rVector.AreAdjacent(*this);
                }
            }
        else
            {
            // The given vector is not a linear, or at least not a basic linear
            // We ask to given for answer
            Answer = pi_rVector.AreAdjacent(*this);
            }
        }

    return(Answer);
    }

//-----------------------------------------------------------------------------
// Intersect
// Finds intersection points with vector
//-----------------------------------------------------------------------------
size_t HVE2DPolySegment::Intersect(const HVE2DVector&       pi_rVector,
                                   HGF2DLocationCollection* po_pCrossPoints) const
    {
    HINVARIANTS;

    // A recipient collection must be provided
    HPRECONDITION(po_pCrossPoints != 0);

    // Declare counter for number of cross points foundÿ
    size_t  NumberOfCrossPoints = 0;

    // Check if self is not NULL
    if (GetPoints().size() >= 2)
        {

        // Check if the given vector is a basic linear
        if ((pi_rVector.GetMainVectorType() == HVE2DLinear::CLASS_ID) &&
            ((static_cast<const HVE2DLinear&>(pi_rVector)).IsABasicLinear()))
            {

            // The given is a linear ... of what type ?

            // First cast as a basic linear
            const HVE2DBasicLinear& rBasicLinear = static_cast<const HVE2DBasicLinear&>(pi_rVector);

            // Depending on basic linear type
            if (rBasicLinear.GetBasicLinearType() == HVE2DPolySegment::CLASS_ID)
                {
                // The given is a polysegment
                // Cast as a polysegment
                const HVE2DPolySegment& rGivenPolySegment = static_cast<const HVE2DPolySegment&>(rBasicLinear);

                // Check if they are adjacent
                NumberOfCrossPoints = IntersectPolySegment(rGivenPolySegment, po_pCrossPoints);
                }
            else if (rBasicLinear.GetBasicLinearType() == HVE2DSegment::CLASS_ID)
                {
                // The given is a segment
                // Cast as a segment
                const HVE2DSegment& rGivenSegment = static_cast<const HVE2DSegment&>(rBasicLinear);

                // Check if they are adjacent
                NumberOfCrossPoints = IntersectSegment(rGivenSegment, po_pCrossPoints);
                }
            else
                {
                // Unknown basic linear type ... we ask to given for answer
                NumberOfCrossPoints = pi_rVector.Intersect(*this, po_pCrossPoints);
                }
            }
        else
            {
            // The given vector is not a linear, or at least not a basic linear
            // We ask to given for answer
            NumberOfCrossPoints = pi_rVector.Intersect(*this, po_pCrossPoints);
            }
        }

    return (NumberOfCrossPoints);
    }

//-----------------------------------------------------------------------------
// ObtainContiguousnessPoints
// Finds contiguousness points with vector
//-----------------------------------------------------------------------------
size_t HVE2DPolySegment::ObtainContiguousnessPoints(const HVE2DVector&       pi_rVector,
                                                    HGF2DLocationCollection* po_pContiguousnessPoints) const
    {
    HINVARIANTS;

    // A recipient collection must be provided
    HPRECONDITION(po_pContiguousnessPoints != 0);

    // The vectors must be contiguous
    HASSERTSUPERDEBUG(AreContiguous(pi_rVector));

    // Declare counter for number of new contiguousness points foundÿ
    size_t  NumberOfNewPoints = 0;

    // Check if the given vector is a basic linear
    if (pi_rVector.GetMainVectorType() == HVE2DLinear::CLASS_ID &&
        (static_cast<const HVE2DLinear&>(pi_rVector)).IsABasicLinear())
        {

        // The given is a basic linear ... of what type ?

        // First cast as a basic linear
        const HVE2DBasicLinear& rBasicLinear = static_cast<const HVE2DBasicLinear&>(pi_rVector);

        // Depending on basic linear type
        if (rBasicLinear.GetBasicLinearType() == HVE2DPolySegment::CLASS_ID)
            {
            // The given is a polysegment
            // Cast as a polysegment
            const HVE2DPolySegment& rGivenPolySegment = static_cast<const HVE2DPolySegment&>(rBasicLinear);

            // Obtain contiguousness points
            NumberOfNewPoints = ObtainContiguousnessPointsWithPolySegment(rGivenPolySegment, po_pContiguousnessPoints);
            }
        else if (rBasicLinear.GetBasicLinearType() == HVE2DSegment::CLASS_ID)
            {
            // The given is a segment
            // Cast as a segment
            const HVE2DSegment& rGivenSegment = static_cast<const HVE2DSegment&>(rBasicLinear);

            // Obtain contiguousness points
            NumberOfNewPoints = ObtainContiguousnessPointsWithSegment(rGivenSegment, po_pContiguousnessPoints);
            }
        else
            {
            // Unknown basic linear type ... we ask to given for answer

            // Since the collection of contiguousness points must
            // be returned in increasing order of relative position
            // we will require a second temporary collection
            HGF2DLocationCollection TempPoints;

            // We have not a known basic linear ... we ask the vector to perform the process
            if ((NumberOfNewPoints = pi_rVector.ObtainContiguousnessPoints(*this, &TempPoints)) != 0)
                {
                // There are some contiguousness points ...

                // We check if they are in the proper order by checking relative position
                // of first and last points found
                if (CalculateRelativePosition(*(TempPoints.begin())) < CalculateRelativePosition(*(TempPoints.rbegin())))
                    {
                    // The points are in the proper order ... we copy
                    HGF2DLocationCollection::iterator MyIterator(TempPoints.begin());

                    while (MyIterator != TempPoints.end())
                        {
                        po_pContiguousnessPoints->push_back(*MyIterator);

                        ++MyIterator;
                        }
                    }
                else
                    {
                    // The points are not in the proper order
                    // We add these points in reverse order
                    HGF2DLocationCollection::reverse_iterator MyIterator(TempPoints.rbegin());

                    while (MyIterator != TempPoints.rend())
                        {
                        po_pContiguousnessPoints->push_back(*MyIterator);

                        ++MyIterator;
                        }
                    }
                }
            }
        }
    else
        {
        // The given vector is not a linear, or at least not a basic linear
        // We ask to given for answer
        // Since the collection of contiguousness points must
        // be returned in increasing order of relative position
        // we will require a second temporary collection
        HGF2DLocationCollection TempPoints;

        if ((NumberOfNewPoints = pi_rVector.ObtainContiguousnessPoints(*this, &TempPoints)) != 0)
            {
            // There are some contiguousness points ...

            // We check if they are in the proper order by checking relative position
            // of first and last points found
            if ((CalculateRelativePosition(*(TempPoints.begin())) < CalculateRelativePosition(*(TempPoints.rbegin()))) || (TempPoints.rbegin()->IsEqualTo(GetEndPoint(), GetTolerance())))
                {
                // The points are in the proper order ... we copy
                HGF2DLocationCollection::iterator MyIterator(TempPoints.begin());

                while (MyIterator != TempPoints.end())
                    {
                    po_pContiguousnessPoints->push_back(*MyIterator);

                    ++MyIterator;
                    }
                }
            else
                {
                // The points are not in the proper order
                // We add these points in reverse order
                HGF2DLocationCollection::reverse_iterator MyIterator(TempPoints.rbegin());

                while (MyIterator != TempPoints.rend())
                    {
                    po_pContiguousnessPoints->push_back(*MyIterator);

                    ++MyIterator;
                    }
                }
            }
        }

    return(NumberOfNewPoints);
    }

//-----------------------------------------------------------------------------
// ObtainContiguousnessPointsAt
// Finds contiguousness point with given vector
//-----------------------------------------------------------------------------
void HVE2DPolySegment::ObtainContiguousnessPointsAt(const HVE2DVector&   pi_rVector,
                                                    const HGF2DLocation& pi_rPoint,
                                                    HGF2DLocation*       po_pFirstContiguousnessPoint,
                                                    HGF2DLocation*       po_pSecondContiguousnessPoint) const
    {
    HINVARIANTS;

    // Recipient variables must be provided
    HPRECONDITION(po_pFirstContiguousnessPoint != 0);
    HPRECONDITION(po_pSecondContiguousnessPoint != 0);

    // The vectors must be contiguous at given point
    HASSERTSUPERDEBUG(AreContiguousAt(pi_rVector, pi_rPoint));

    // Check if the given is a basic linear
    if (pi_rVector.GetMainVectorType() == HVE2DLinear::CLASS_ID &&
        (static_cast<const HVE2DLinear&>(pi_rVector)).IsABasicLinear())
        {
        // The given is a basic linear ... of what type ?

        // First cast as a basic linear
        const HVE2DBasicLinear& rBasicLinear = static_cast<const HVE2DBasicLinear&>(pi_rVector);

        // Depending on basic linear type
        if(rBasicLinear.GetBasicLinearType() == HVE2DPolySegment::CLASS_ID)
            {
            // The given is a polysegment
            // Cast as a polysegment
            const HVE2DPolySegment& rGivenPolySegment = static_cast<const HVE2DPolySegment&>(rBasicLinear);

            // Obtain contiguousness points at point
            ObtainContiguousnessPointsWithPolySegmentAt(rGivenPolySegment,
                                                        pi_rPoint,
                                                        po_pFirstContiguousnessPoint,
                                                        po_pSecondContiguousnessPoint);
            }
        else if (rBasicLinear.GetBasicLinearType() == HVE2DSegment::CLASS_ID)
            {
            // The given is a segment
            // Cast as a segment
            const HVE2DSegment& rGivenSegment = static_cast<const HVE2DSegment&>(rBasicLinear);

            // Obtain contiguousness points at point
            ObtainContiguousnessPointsWithSegmentAt(rGivenSegment,
                                                    pi_rPoint,
                                                    po_pFirstContiguousnessPoint,
                                                    po_pSecondContiguousnessPoint);
            }
        else
            {
            // Unknown basic linear type ... we ask to given for answer

            // We have not a segment ... we ask the vector to perform the process
            // Obtain contiguousness points at point
            pi_rVector.ObtainContiguousnessPointsAt(*this,
                                                    pi_rPoint,
                                                    po_pFirstContiguousnessPoint,
                                                    po_pSecondContiguousnessPoint);

            // Since the contiguousness points must
            // be returned in increasing order of relative position
            // We check if the two points are in the proper order
            if (CalculateRelativePosition(*po_pFirstContiguousnessPoint) > CalculateRelativePosition(*po_pSecondContiguousnessPoint))
                {
                // Not in the proper order ... we swap
                HGF2DLocation   SwapLocation(*po_pFirstContiguousnessPoint);
                *po_pFirstContiguousnessPoint  = *po_pSecondContiguousnessPoint;
                *po_pSecondContiguousnessPoint = SwapLocation;
                }
            }
        }
    else
        {
        // The given vector is not a linear, or at least not a basic linear
        // We ask to given for answer

        // Obtain contiguousness points at point
        pi_rVector.ObtainContiguousnessPointsAt(*this, pi_rPoint, po_pFirstContiguousnessPoint, po_pSecondContiguousnessPoint);

        // Since the contiguousness points must
        // be returned in increasing order of relative position
        // We check if the two points are in the proper order
        if (CalculateRelativePosition(*po_pFirstContiguousnessPoint) > CalculateRelativePosition(*po_pSecondContiguousnessPoint))
            {
            // Not in the proper order ... we swap
            HGF2DLocation   SwapLocation(*po_pFirstContiguousnessPoint);
            *po_pFirstContiguousnessPoint  = *po_pSecondContiguousnessPoint;
            *po_pSecondContiguousnessPoint = SwapLocation;
            }
        }
    }

//-----------------------------------------------------------------------------
// AllocateCopyInCoordSys
// Returns a dynamically allocated copy of the polysegment in a different
// coordinate system
//-----------------------------------------------------------------------------
HVE2DVector* HVE2DPolySegment::AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const
    {
    HINVARIANTS;

    HAutoPtr<HVE2DVector>    pMyResultVector;

    // Check if it is the same coordinate system
    if (pi_rpCoordSys == GetCoordSys())
        {
        pMyResultVector.reset(new HVE2DPolySegment(*this));
        }
    else
        {

        HFCPtr<HGF2DTransfoModel> pModel = GetCoordSys()->GetTransfoModelTo(pi_rpCoordSys);
    
        HFCPtr<HGF2DPolySegment> newPolySegment = GetPolySegmentPeer().AllocPolySegmentTransformDirect(*pModel);
        
        pMyResultVector.reset(new HVE2DPolySegment(new HGF2DPolySegment(*newPolySegment), pi_rpCoordSys));

        }

    return(pMyResultVector.release());
    }

//-----------------------------------------------------------------------------
// CalculateClosestPoint
// Calculates the closest point on polysegment to given point
//-----------------------------------------------------------------------------
HGF2DLocation HVE2DPolySegment::CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const
    {
    HINVARIANTS;
   
    return HGF2DLocation(GetPolySegmentPeer().CalculateClosestPoint(pi_rPoint.ExpressedIn(GetCoordSys()).GetPosition()), GetCoordSys());
    }


//-----------------------------------------------------------------------------
// CalculateLength
// Calculates and returns the length of polysegment
//-----------------------------------------------------------------------------
double HVE2DPolySegment::CalculateLength() const
    {
    HINVARIANTS;
    return GetPolySegmentPeer().CalculateLength();
    }

//-----------------------------------------------------------------------------
// CalculateRelativePoint
// Calculates and returns the location based on the given relative position.
//-----------------------------------------------------------------------------
HGF2DLocation HVE2DPolySegment::CalculateRelativePoint(double pi_RelativePos) const
    {
    HINVARIANTS;

    // The relative position must be between 0.0 and 1.0
    HPRECONDITION((pi_RelativePos >= 0.0) && (pi_RelativePos <= 1.0));

    return HGF2DLocation(GetPolySegmentPeer().CalculateRelativePoint(pi_RelativePos), GetCoordSys());
    }

//-----------------------------------------------------------------------------
// CalculateRelativePosition
// Calculates and returns the relative position of given location on polysegment.
//-----------------------------------------------------------------------------
double HVE2DPolySegment::CalculateRelativePosition(const HGF2DLocation& pi_rPoint) const
    {
    HINVARIANTS;

    return GetPolySegmentPeer().CalculateRelativePosition(pi_rPoint.ExpressedIn(GetCoordSys()).GetPosition());
}

//-----------------------------------------------------------------------------
// CalculateRayArea
// Calculates and returns the area of the ray extended from given point to
// the given polysegment
//-----------------------------------------------------------------------------
double HVE2DPolySegment::CalculateRayArea(const HGF2DLocation& pi_rPoint) const
    {
    HINVARIANTS;

    return GetPolySegmentPeer().CalculateRayArea(pi_rPoint.ExpressedIn(GetCoordSys()).GetPosition());
    }


namespace { //BEGIN_UNNAMED_NAMESPACE

class ComputeSegmentLenght : std::binary_function<HGF2DPosition, HGF2DPosition, double>
    {
public:
    explicit ComputeSegmentLenght () {}

    double operator () (const HGF2DPosition& startPt, const HGF2DPosition& endPt) const
        {
        const double deltaX(endPt.GetX() - startPt.GetX());
        
        // Evaluate Delta Y
        // Note that distance unit conversion is performed since Y units
        // may be different from X unit in which answer is evaluated
        const double deltaY(endPt.GetY() - startPt.GetY());
        
        // Obtain length of segment
        return sqrt((deltaX * deltaX) + (deltaY * deltaY));
        }
    };

class FindSegmentContainingPosition : std::binary_function<HGF2DPosition, HGF2DPosition, bool>
    {
    ComputeSegmentLenght    m_computeSegmentLenght;
    double&                 m_lastSegmentLenght;
    double&                 m_lastPositionLenght;
    const double            m_searchedPositionLenght;

public:
    explicit FindSegmentContainingPosition         (double&                 lastSegmentLenght, 
                                                    double&                 lastPositionLenght, 
                                                    double                  searchedPositionLenght)
        :   m_computeSegmentLenght(),
            m_lastSegmentLenght(lastSegmentLenght),
            m_lastPositionLenght(lastPositionLenght),
            m_searchedPositionLenght(searchedPositionLenght)
        {
        }

    bool operator () (const HGF2DPosition& startPt, const HGF2DPosition& endPt) const
        {
        m_lastSegmentLenght = m_computeSegmentLenght(startPt, endPt);
                                         
        HASSERT(0.0 != m_lastSegmentLenght);

        // Increment position length
        m_lastPositionLenght += m_lastSegmentLenght;

        return m_lastPositionLenght >= m_searchedPositionLenght;
        }
    };

} //END_UNNAMED_NAMESPACE

//-----------------------------------------------------------------------------
// Shorten
// Shortens the polysegment definition by specification of
// relative positions to self.
//-----------------------------------------------------------------------------
void HVE2DPolySegment::Shorten(double startRelativePos, double endRelativePos)
    {
    HINVARIANTS;
    
    GetPolySegmentPeer().Shorten(startRelativePos, endRelativePos);
    SetLinearStartPoint(HGF2DLocation(GetPolySegmentPeer().GetStartPoint(), GetCoordSys()));
    SetLinearEndPoint(HGF2DLocation(GetPolySegmentPeer().GetEndPoint(), GetCoordSys()));

    // Indicate extent is not up to date anymore
    m_ExtentUpToDate = false;

    // Adjust tolerance
    ResetTolerance();
    }

//-----------------------------------------------------------------------------
// Shorten
// Shortens the polysegment definition by specification of a new start
// and end points
//-----------------------------------------------------------------------------
void HVE2DPolySegment::Shorten(const HGF2DLocation& pi_rStartPoint,
                               const HGF2DLocation& pi_rEndPoint)
    {
    HINVARIANTS;

    // The given points must be located on polysegment
    HASSERTSUPERDEBUG(IsPointOn(pi_rStartPoint) && IsPointOn(pi_rEndPoint));

    // Call other function to perform the processing
    Shorten(CalculateRelativePosition(pi_rStartPoint),
            CalculateRelativePosition(pi_rEndPoint));
    }


//-----------------------------------------------------------------------------
// ShortenTo
// Shortens the polysegment definition by specification of a new end point
//-----------------------------------------------------------------------------
void HVE2DPolySegment::ShortenTo(const HGF2DLocation& pi_rNewEndPoint)
    {
    HINVARIANTS;

    // The given point must be located on polysegment
    HASSERTSUPERDEBUG(IsPointOn(pi_rNewEndPoint));

    // Call other function to perform the processing
    ShortenTo(CalculateRelativePosition(pi_rNewEndPoint));
    }

//-----------------------------------------------------------------------------
// ShortenFrom
// Shortens the polysegment definition by specification of a new start point
//-----------------------------------------------------------------------------
void HVE2DPolySegment::ShortenFrom(const HGF2DLocation& pi_rNewStartPoint)
    {
    HINVARIANTS;

    // The given point must be located on complex linear
    HASSERTSUPERDEBUG(IsPointOn(pi_rNewStartPoint));

    // Call other function to perform the processing
    ShortenFrom(CalculateRelativePosition(pi_rNewStartPoint));
    }

//-----------------------------------------------------------------------------
// ShortenFrom
// Shortens the polysegment definition by specification of
// a new start relative point
//-----------------------------------------------------------------------------
void HVE2DPolySegment::ShortenFrom(double pi_StartRelativePos)
    {
    HINVARIANTS;

    // Both relative position must be between 0.0 and 1.0, and the
    // end relative position must be greater than the start relative
    HPRECONDITION((pi_StartRelativePos >= 0.0) && (pi_StartRelativePos <= 1.0));

    // Call other function to perform the processing
    Shorten(pi_StartRelativePos, 1.0);

    }

//-----------------------------------------------------------------------------
// ShortenTo
// Shortens the polysegment definition by specification of
// a new end relative point
//-----------------------------------------------------------------------------
void HVE2DPolySegment::ShortenTo(double pi_EndRelativePos)
    {
    HINVARIANTS;

    // The relative position must be between 0.0 and 1.0
    HPRECONDITION((pi_EndRelativePos >= 0.0) && (pi_EndRelativePos <= 1.0));

    // Call other function to perform the processing
    Shorten(0.0, pi_EndRelativePos);
    }

//-----------------------------------------------------------------------------
// IsPointOn
// Checks if the point is located on the polysegment
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::IsPointOn(const HGF2DLocation& pi_rTestPoint,
                                  HVE2DVector::ExtremityProcessing    pi_ExtremityProcessing,
                                  double pi_Tolerance) const
    {
    HINVARIANTS;

    bool   Answer = false;

    // Check if the two share the same coordinate system
    if (GetCoordSys() == pi_rTestPoint.GetCoordSys())
        {
        Answer = IsPointOnSCS(pi_rTestPoint, pi_ExtremityProcessing, pi_Tolerance);
        }
    else
        {
        Answer = IsPointOnSCS(HGF2DLocation(pi_rTestPoint, GetCoordSys()),
                              pi_ExtremityProcessing,
                              pi_Tolerance);
        }

    return(Answer);
    }

//-----------------------------------------------------------------------------
// IsPointOnSCS
// Checks if the point is located on the poly segment
// The point must be expressed in the same coordinate system as self
// The tolerance must be positive
//-----------------------------------------------------------------------------
bool   HVE2DPolySegment::IsPointOnSCS(const HGF2DLocation& pi_rTestPoint,
                                       HVE2DVector::ExtremityProcessing    pi_ExtremityProcessing,
                                       double pi_Tolerance) const
    {
    HINVARIANTS;

    // The two must share the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rTestPoint.GetCoordSys());

    return GetPolySegmentPeer().IsPointOn(pi_rTestPoint.GetPosition(), (HGF2DVector::ExtremityProcessing)pi_ExtremityProcessing, pi_Tolerance);
    }


//-----------------------------------------------------------------------------
// AdjustStartPointTo
// Adjust the start point to an more exact point located less than
// an epsilon from previous point
//-----------------------------------------------------------------------------
void HVE2DPolySegment::AdjustStartPointTo(const HGF2DLocation& pi_rPoint)
    {
    HINVARIANTS;

    // The adjust point must be virtually identical to start point
    HPRECONDITION(GetStartPoint().IsEqualTo(pi_rPoint, GetTolerance()));

    // The polysegment must not be nul
    HPRECONDITION(GetPoints().size() >= 2);




    GetPolySegmentPeer().AdjustStartPointTo(pi_rPoint.ExpressedIn(GetCoordSys()).GetPosition());
    SetLinearStartPoint(pi_rPoint);

    }

//-----------------------------------------------------------------------------
// AdjustEndPointTo
// Adjust the end point to an more exact point located less than
// an epsilon from previous point
//-----------------------------------------------------------------------------
void HVE2DPolySegment::AdjustEndPointTo(const HGF2DLocation& pi_rPoint)
    {
    HINVARIANTS;

    // The end point must be equal within tolerance to given point
    HPRECONDITION(GetEndPoint().IsEqualTo(pi_rPoint, GetTolerance()));

    // The polysegment must not be nul
    HPRECONDITION(GetPoints().size() >= 2);



    GetPolySegmentPeer().AdjustEndPointTo(pi_rPoint.ExpressedIn(GetCoordSys()).GetPosition());

    SetLinearEndPoint(pi_rPoint);
    // Tolerance is not adjusted since change is only minor
    }


//-----------------------------------------------------------------------------
// Drop
// Returns the description of segments in the form of raw location
// segments
//-----------------------------------------------------------------------------
void HVE2DPolySegment::Drop(HGF2DLocationCollection* po_pPoints,
                            double                   pi_Tolerance,
                            EndPointProcessing       pi_EndPointProcessing) const
    {
    HINVARIANTS;

    // Recipient collection must be provided
    HPRECONDITION(po_pPoints != 0);

    // The polysegment may not be empty (may be null length though)
    HPRECONDITION(GetPoints().size() >= 2);

    // NOTE : Tolerance is unused since drop is exact
    HGF2DPositionCollection positions;
    GetPolySegmentPeer().Drop(&positions, pi_Tolerance, (HGF2DLinear::EndPointProcessing)pi_EndPointProcessing);
    
    for (auto currentPoint : positions)
        po_pPoints->push_back(HGF2DLocation(currentPoint, GetCoordSys()));
    }

//-----------------------------------------------------------------------------
// IsContiguousToPolySegmentSCS
// PRIVATE
// Indicates if the polysegment is contiguous to the given polysegment
// They must already be expressed in the same coordinate system.
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::IsContiguousToPolySegmentSCS(const HVE2DPolySegment& pi_rPolySegment) const
    {
    HINVARIANTS;

    // They must share the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rPolySegment.GetCoordSys());
    return GetPolySegmentPeer().IsContiguousToPolySegment(pi_rPolySegment.GetPolySegmentPeer());
    }


//-----------------------------------------------------------------------------
// IsContiguousToSegmentSCS
// PRIVATE
// Indicates if the polysegment is contiguous to the given segment
// They must already be expressed in the same coordinate system.
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::IsContiguousToSegmentSCS(const HVE2DSegment& pi_rSegment) const
    {
    HINVARIANTS;

    // They must share the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rSegment.GetCoordSys());
    return GetPolySegmentPeer().IsContiguousToSegment(pi_rSegment.GetSegmentPeer());
    }



//-----------------------------------------------------------------------------
// IsContiguousToPolySegment
// PRIVATE
// Indicates if the polysegment is contiguous to the given polysegment
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::IsContiguousToPolySegment(const HVE2DPolySegment& pi_rPolySegment) const
    {
    HINVARIANTS;

    bool Answer = false;

    // Check if they share the same coordinate system
    if (GetCoordSys() == pi_rPolySegment.GetCoordSys())
        {
        // The two polysegments have the same coordinate system
        Answer = IsContiguousToPolySegmentSCS(pi_rPolySegment);
        }
    else
        {
        // The two polysegments have different coordinate system

        // In correct coordinate system
        HAutoPtr<HVE2DVector> pTempVector(pi_rPolySegment.AllocateCopyInCoordSys(GetCoordSys()));

        // Obtain answer
        Answer = pTempVector->AreContiguous(*this);
        }


    return(Answer);
    }

//-----------------------------------------------------------------------------
// IsContiguousToSegment
// PRIVATE
// Indicates if the polysegment is contiguous to the given segment
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::IsContiguousToSegment(const HVE2DSegment& pi_rSegment) const
    {
    HINVARIANTS;

    bool Answer = false;

    // Check if segment is not null
    if (!pi_rSegment.IsNull())
        {
        // Check if they share the same coordinate system
        if (GetCoordSys() == pi_rSegment.GetCoordSys())
            {
            // The two vectors have the same coordinate system
            Answer = IsContiguousToSegmentSCS(pi_rSegment);
            }
        else
            {
            // The two vectors have different coordinate system
            HAutoPtr<HVE2DVector> pTempVector(pi_rSegment.AllocateCopyInCoordSys(GetCoordSys()));

            // Obtain answer
            Answer = pTempVector->AreContiguous(*this);
            }
        }

    return(Answer);
    }




//-----------------------------------------------------------------------------
// IsContiguousToPolySegmentAtSCS
// PRIVATE
// Indicates if the polysegment is contiguous to the given polysegment
// At specified point
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::IsContiguousToPolySegmentAtSCS(const HVE2DPolySegment& pi_rPolySegment,
                                                       const HGF2DLocation&    pi_rPoint) const
    {
    HINVARIANTS;

    // They must share the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rPolySegment.GetCoordSys());

    // The given point must be located on both vectors
    HASSERTSUPERDEBUG(IsPointOn(pi_rPoint) && pi_rPolySegment.IsPointOn(pi_rPoint));

    return GetPolySegmentPeer().IsContiguousToPolySegmentAt(pi_rPolySegment.GetPolySegmentPeer(), pi_rPoint.ExpressedIn(GetCoordSys()).GetPosition());
    }



//-----------------------------------------------------------------------------
// IsContiguousToSegmentAtSCS
// PRIVATE
// Indicates if the polysegment is contiguous to the given segment
// They must already be expressed in the same coordinate system.
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::IsContiguousToSegmentAtSCS(const HVE2DSegment&  pi_rSegment,
                                                   const HGF2DLocation& pi_rPoint) const
    {
    HINVARIANTS;

    // They must share the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rSegment.GetCoordSys());

    // The given point must be located on both vectors
    HASSERTSUPERDEBUG(IsPointOn(pi_rPoint) && pi_rSegment.IsPointOn(pi_rPoint));

    return GetPolySegmentPeer().IsContiguousToSegmentAt(pi_rSegment.GetSegmentPeer(), pi_rPoint.ExpressedIn(GetCoordSys()).GetPosition());
    }



//-----------------------------------------------------------------------------
// IsContiguousToPolySegmentAt
// PRIVATE
// Indicates if the polysegment is contiguous to the given polysegment
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::IsContiguousToPolySegmentAt(const HVE2DPolySegment& pi_rPolySegment,
                                                    const HGF2DLocation&    pi_rPoint) const
    {
    HINVARIANTS;

    // The given point must be located on both vectors
    HASSERTSUPERDEBUG(IsPointOn(pi_rPoint) && pi_rPolySegment.IsPointOn(pi_rPoint));

    bool Answer = false;

    // Check if they share the same coordinate system
    if (GetCoordSys() == pi_rPolySegment.GetCoordSys())
        {
        // The two polysegments have the same coordinate system
        Answer = IsContiguousToPolySegmentAtSCS(pi_rPolySegment, pi_rPoint);
        }
    else
        {
        // The two polysegments have different coordinate systems
        HAutoPtr<HVE2DVector> pTempVector(pi_rPolySegment.AllocateCopyInCoordSys(GetCoordSys()));

        // Obtain answer
        Answer = pTempVector->AreContiguousAt(*this, pi_rPoint);
        }

    return(Answer);
    }

//-----------------------------------------------------------------------------
// IsContiguousToSegmentAt
// PRIVATE
// Indicates if the polysegment is contiguous to the given segment
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::IsContiguousToSegmentAt(const HVE2DSegment&  pi_rSegment,
                                                const HGF2DLocation& pi_rPoint) const
    {
    HINVARIANTS;

    // The given point must be located on both vectors
    HASSERTSUPERDEBUG(IsPointOn(pi_rPoint) && pi_rSegment.IsPointOn(pi_rPoint));

    bool Answer = false;

    // Check if segment is not null
    if (!pi_rSegment.IsNull())
        {
        // Check if they share the same coordinate system
        if (GetCoordSys() == pi_rSegment.GetCoordSys())
            {
            // The two vectors have the same coordinate system
            Answer = IsContiguousToSegmentAtSCS(pi_rSegment, pi_rPoint);
            }
        else
            {
            // The two vectors have different coordinate systems
            HAutoPtr<HVE2DVector> pTempVector(pi_rSegment.AllocateCopyInCoordSys(GetCoordSys()));

            // Obtain answer from newly allocated vector
            Answer = pTempVector->AreContiguousAt(*this, pi_rPoint);
            }
        }

    return(Answer);
    }



//-----------------------------------------------------------------------------
// CrossesPolySegmentSCS
// PRIVATE
// Indicates if the two polysegments cross each other
// The two polysegments are expressed in the same coordinate system
//-----------------------------------------------------------------------------

bool HVE2DPolySegment::CrossesPolySegmentSCS(const HVE2DPolySegment& pi_rPolySegment) const
    {
    HINVARIANTS;

    // They must share the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rPolySegment.GetCoordSys());

    return GetPolySegmentPeer().CrossesPolySegment(pi_rPolySegment.GetPolySegmentPeer());
    }




//-----------------------------------------------------------------------------
// CrossesSegmentSCS
// PRIVATE
// Indicates if the polysegment and segment cross each other
// The two linear are expressed in the same coordinate system
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::CrossesSegmentSCS(const HVE2DSegment& pi_rSegment) const
    {
    HINVARIANTS;

    // They must share the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rSegment.GetCoordSys());

    return GetPolySegmentPeer().CrossesSegment(pi_rSegment.GetSegmentPeer());
    }






//-----------------------------------------------------------------------------
// CrossesPolySegment
// PRIVATE
// Indicates if the two polysegment cross each other
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::CrossesPolySegment(const HVE2DPolySegment& pi_rPolySegment) const
    {
    HINVARIANTS;

    // Create variable for answer
    bool   Answer = false;

    // Check if they share the same coordinate system
    if (GetCoordSys() == pi_rPolySegment.GetCoordSys())
        {
        // The two segments have the same coordinate system
        Answer = CrossesPolySegmentSCS(pi_rPolySegment);
        }
    else
        {
        // The two polysegments have different coordinate systems
        HAutoPtr<HVE2DVector> pTempVector(pi_rPolySegment.AllocateCopyInCoordSys(GetCoordSys()));

        // Obtain answer from newly allocated vector
        Answer = pTempVector->Crosses(*this);
        }

    return(Answer);
    }

//-----------------------------------------------------------------------------
// CrossesSegment
// PRIVATE
// Indicates if the polysegment and the segment cross each other
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::CrossesSegment(const HVE2DSegment& pi_rSegment) const
    {
    HINVARIANTS;

    bool   Answer = false;

    // Check if they share the same coordinate system
    if (GetCoordSys() == pi_rSegment.GetCoordSys())
        {
        // The two segments have the same coordinate system
        Answer = CrossesSegmentSCS(pi_rSegment);
        }
    else
        {
        // The two linears have different coordinate systems
        HAutoPtr<HVE2DVector> pTempVector(pi_rSegment.AllocateCopyInCoordSys(GetCoordSys()));

        // Obtain answer from newly allocated vector
        Answer = pTempVector->Crosses(*this);
        }

    return(Answer);
    }



//-----------------------------------------------------------------------------
// IsAdjacentToPolySegmentSCS
// PRIVATE
// Indicates if the polysegment adjacent to given polysegment
// They must already be expressed in the same coordinate systems
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::IsAdjacentToPolySegmentSCS(const HVE2DPolySegment& pi_rPolySegment) const
    {
    HINVARIANTS;

    // They must share the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rPolySegment.GetCoordSys());

    return GetPolySegmentPeer().IsAdjacentToPolySegment(pi_rPolySegment.GetPolySegmentPeer());
    }



//-----------------------------------------------------------------------------
// IsAdjacentToSegmentSCS
// PRIVATE
// Indicates if the polysegment adjacent to given segment
// They must already be expressed in the same coordinate systems
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::IsAdjacentToSegmentSCS(const HVE2DSegment& pi_rSegment) const
    {
    HINVARIANTS;

    // They must already be expressed in the same coordinate systems
    HPRECONDITION(GetCoordSys() == pi_rSegment.GetCoordSys());

    return GetPolySegmentPeer().IsAdjacentToSegment(pi_rSegment.GetSegmentPeer());
    }




//-----------------------------------------------------------------------------
// IsAdjacentToPolySegment
// PRIVATE
// Indicates if the polysegment is adjacent to given polysegment
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::IsAdjacentToPolySegment(const HVE2DPolySegment& pi_rPolySegment) const
    {
    HINVARIANTS;

    bool Answer = false;

    // Check if they share the same coordinate system
    if (GetCoordSys() == pi_rPolySegment.GetCoordSys())
        {
        // The two polysegments have the same coordinate system
        Answer = IsAdjacentToPolySegmentSCS(pi_rPolySegment);
        }
    else
        {
        // The two polysegments have different coordinate systems
        // They are not related by a linearity preserving model ... we must allocate a copy
        // In correct coordinate system
        HAutoPtr<HVE2DVector> pTempVector(pi_rPolySegment.AllocateCopyInCoordSys(GetCoordSys()));

        // Obtain answer from newly allocated vector
        Answer = pTempVector->AreAdjacent(*this);
        }

    return(Answer);
    }

//-----------------------------------------------------------------------------
// IsAdjacentToSegment
// PRIVATE
// Indicates if the polysegment is adjacent to given segment
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::IsAdjacentToSegment(const HVE2DSegment& pi_rSegment) const
    {
    HINVARIANTS;

    bool Answer = false;

    // Check if they share the same coordinate system
    if (GetCoordSys() == pi_rSegment.GetCoordSys())
        {
        // The two linears have the same coordinate system
        Answer = IsAdjacentToSegmentSCS(pi_rSegment);
        }
    else
        {
        // The two polysegments have different coordinate systems
        // They are not related by a linearity preserving model ... we must allocate a copy
        // In correct coordinate system
        HAutoPtr<HVE2DVector> pTempVector(pi_rSegment.AllocateCopyInCoordSys(GetCoordSys()));

        // Obtain answer from newly allocated vector
        Answer = pTempVector->AreAdjacent(*this);
        }

    return(Answer);
    }



//-----------------------------------------------------------------------------
// IntersectSegmentSCS
// PRIVATE
// Finds intersection point with segment
// The two vectors must share the same coordinate system
//-----------------------------------------------------------------------------
size_t HVE2DPolySegment::IntersectSegmentSCS(const HVE2DSegment& pi_rSegment,
                                             HGF2DLocationCollection* po_pCrossPoints) const
    {
    HINVARIANTS;

    // The two vectors must share the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rSegment.GetCoordSys());

    HGF2DPositionCollection crossPoints;

    size_t numCrossPoints = GetPolySegmentPeer().IntersectSegment(pi_rSegment.GetSegmentPeer(), &crossPoints);

    for (auto point : crossPoints)
        po_pCrossPoints->push_back(HGF2DLocation(point, GetCoordSys()));

    return numCrossPoints;
    }



//-----------------------------------------------------------------------------
// IntersectPolySegmentSCS
// PRIVATE
// Finds intersection point with poly segment
// The two vectors must share the same coordinate system
//-----------------------------------------------------------------------------
size_t HVE2DPolySegment::IntersectPolySegmentSCS(const HVE2DPolySegment& pi_rPolySegment,
                                                 HGF2DLocationCollection* po_pCrossPoints) const
    {
    HINVARIANTS;

    // The two vectors must share the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rPolySegment.GetCoordSys());

    HGF2DPositionCollection crossPoints;

    size_t numCrossPoints = GetPolySegmentPeer().IntersectPolySegment(pi_rPolySegment.GetPolySegmentPeer(), &crossPoints);

    for (auto point : crossPoints)
        po_pCrossPoints->push_back(HGF2DLocation(point, GetCoordSys()));
  
    return numCrossPoints;
    }






//-----------------------------------------------------------------------------
// IntersectPolySegment
// Finds intersection point with polysegment
//-----------------------------------------------------------------------------
size_t HVE2DPolySegment::IntersectPolySegment(const HVE2DPolySegment& pi_rPolySegment,
                                              HGF2DLocationCollection* po_pCrossPoints) const
    {
    HINVARIANTS;

    // Recipient collection must be provided
    HPRECONDITION(po_pCrossPoints != 0);

    // Declare variable for number of new cross points
    size_t  NumberOfCrossPoints = 0;

    // Check if they share the same coordinate system
    if (GetCoordSys() == pi_rPolySegment.GetCoordSys())
        {
        // The two polysegments have the same coordinate system
        NumberOfCrossPoints = IntersectPolySegmentSCS(pi_rPolySegment, po_pCrossPoints);
        }
    else
        {
        // The two polysegments have different coordinate systems
        HAutoPtr<HVE2DVector> pTempVector(pi_rPolySegment.AllocateCopyInCoordSys(GetCoordSys()));

        // Intersect
        NumberOfCrossPoints = pTempVector->Intersect(*this, po_pCrossPoints);
        }

    return(NumberOfCrossPoints);
    }

//-----------------------------------------------------------------------------
// IntersectSegment
// PRIVATE
// Finds intersection point with segment
//-----------------------------------------------------------------------------
size_t HVE2DPolySegment::IntersectSegment(const HVE2DSegment&      pi_rSegment,
                                          HGF2DLocationCollection* po_pCrossPoints) const
    {
    HINVARIANTS;

    // Recipient collection must be provided
    HPRECONDITION(po_pCrossPoints != 0);

    // Declare variable for number of new cross points
    size_t  NumberOfCrossPoints = 0;

    // Check if they share the same coordinate system
    if (GetCoordSys() == pi_rSegment.GetCoordSys())
        {
        // The two have the same coordinate system
        NumberOfCrossPoints = IntersectSegmentSCS(pi_rSegment, po_pCrossPoints);
        }
    else
        {
        // The two have different coordinate systems

        HAutoPtr<HVE2DVector> pTempVector(pi_rSegment.AllocateCopyInCoordSys(GetCoordSys()));

        // Intersect
        NumberOfCrossPoints = pTempVector->Intersect(*this, po_pCrossPoints);
        }

    return(NumberOfCrossPoints);
    }

//-----------------------------------------------------------------------------
// ObtainContiguousnessPointsWithSegmentSCS
// PRIVATE
// Finds contiguousness points with segment
// The polysegment and segment must share the same coordinate system
//-----------------------------------------------------------------------------
size_t HVE2DPolySegment::ObtainContiguousnessPointsWithSegmentSCS(const HVE2DSegment&  pi_rSegment,
                                                                  HGF2DLocationCollection* po_pContiguousnessPoints) const
    {
    HINVARIANTS;

    // The two vectors must share the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rSegment.GetCoordSys());

    // Check that a recipient collection is provided
    HPRECONDITION(po_pContiguousnessPoints != 0);

    HGF2DPositionCollection contigPoints;

    size_t numPoints = GetPolySegmentPeer().ObtainContiguousnessPointsWithSegment(pi_rSegment.GetSegmentPeer(), &contigPoints);

    for (auto point: contigPoints)
        po_pContiguousnessPoints->push_back(HGF2DLocation(point, GetCoordSys()));

    return numPoints;
    }



//-----------------------------------------------------------------------------
// ObtainContiguousnessPointsWithPolySegmentSCS
// PRIVATE
// Finds contiguousness points with polysegment
// The polysegment and segment must share the same coordinate system
//-----------------------------------------------------------------------------
size_t HVE2DPolySegment::ObtainContiguousnessPointsWithPolySegmentSCS(const HVE2DPolySegment&  pi_rPolySegment,
                                                                      HGF2DLocationCollection* po_pContiguousnessPoints) const
    {
    HINVARIANTS;

    // The two vectors must share the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rPolySegment.GetCoordSys());

    // Check that a recipient collection is provided
    HPRECONDITION(po_pContiguousnessPoints != 0);

    HGF2DPositionCollection contigPoints;

    size_t numPoints = GetPolySegmentPeer().ObtainContiguousnessPointsWithPolySegment(pi_rPolySegment.GetPolySegmentPeer(), &contigPoints);

    for (auto point : contigPoints)
        po_pContiguousnessPoints->push_back(HGF2DLocation(point, GetCoordSys()));

    return numPoints;
    }





//-----------------------------------------------------------------------------
// ObtainContiguousnessPointsWithPolySegment
// PRIVATE
// Finds contiguousness points with polysegment
//-----------------------------------------------------------------------------
size_t HVE2DPolySegment::ObtainContiguousnessPointsWithPolySegment(const HVE2DPolySegment&  pi_rPolySegment,
                                                                   HGF2DLocationCollection* po_pContiguousnessPoints) const
    {
    HINVARIANTS;

    // Check that a recipient collection is provided
    HPRECONDITION(po_pContiguousnessPoints != 0);

    // Declare variable to count the number of points found
    size_t NumberOfNewPoints = 0;

    // Check if they share the same coordinate system
    if (GetCoordSys() == pi_rPolySegment.GetCoordSys())
        {
        // The two polysegments have the same coordinate system
        NumberOfNewPoints = ObtainContiguousnessPointsWithPolySegmentSCS(pi_rPolySegment, po_pContiguousnessPoints);
        }
    else
        {
        // The two polysegments have different coordinate systems
        HAutoPtr<HVE2DVector> pTempVector(pi_rPolySegment.AllocateCopyInCoordSys(GetCoordSys()));

        // Obtain answer
        NumberOfNewPoints = pTempVector->ObtainContiguousnessPoints(*this, po_pContiguousnessPoints);
        }

    return(NumberOfNewPoints);
    }

//-----------------------------------------------------------------------------
// ObtainContiguousnessPointsWithSegment
// PRIVATE
// Finds contiguousness points with segment
//-----------------------------------------------------------------------------
size_t HVE2DPolySegment::ObtainContiguousnessPointsWithSegment(const HVE2DSegment&  pi_rSegment,
                                                               HGF2DLocationCollection* po_pContiguousnessPoints) const
    {
    HINVARIANTS;

    // Check that a recipient collection is provided
    HPRECONDITION(po_pContiguousnessPoints != 0);
    // Declare variable to count the number of points found
    size_t NumberOfNewPoints = 0;

    // Check if they share the same coordinate system
    if (GetCoordSys() == pi_rSegment.GetCoordSys())
        {
        // The polysegment and segment have the same coordinate system
        NumberOfNewPoints = ObtainContiguousnessPointsWithSegmentSCS(pi_rSegment, po_pContiguousnessPoints);
        }
    else
        {
        // The polysegment and segment have different coordinate systems

        // They are not related by a linearity preserving model ... we must allocate a copy
        // In correct coordinate system
        HAutoPtr<HVE2DVector> pTempVector(pi_rSegment.AllocateCopyInCoordSys(GetCoordSys()));

        // Obtain answer
        NumberOfNewPoints = pTempVector->ObtainContiguousnessPoints(*this, po_pContiguousnessPoints);
        }

    return(NumberOfNewPoints);
    }


//-----------------------------------------------------------------------------
// ObtainContiguousnessPointsWithSegmentAtSCS
// PRIVATE
// Finds contiguousness points with segment
// The polysegment and segment must share the same coordinate system
//-----------------------------------------------------------------------------
void HVE2DPolySegment::ObtainContiguousnessPointsWithSegmentAtSCS(const HVE2DSegment&  pi_rSegment,
                                                                  const HGF2DLocation& pi_rPoint,
                                                                  HGF2DLocation* po_pFirstContiguousnessPoint,
                                                                  HGF2DLocation* po_pSecondContiguousnessPoint) const
    {
    HINVARIANTS;

    // The two vectors must share the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rSegment.GetCoordSys());

    // Check that recipient locations are provided
    HPRECONDITION(po_pFirstContiguousnessPoint != 0);
    HPRECONDITION(po_pSecondContiguousnessPoint != 0);

    HGF2DPosition FirstContiguousnessPosition;
    HGF2DPosition SecondContiguousnessPosition;

    // Transform point into a position in self coordinate system
    HGF2DLocation PointInSelfCoordSys(pi_rPoint, GetCoordSys());
    HGF2DPosition ThePoint(PointInSelfCoordSys.GetPosition());

    
    GetPolySegmentPeer().ObtainContiguousnessPointsWithSegmentAt(pi_rSegment.GetSegmentPeer(), ThePoint, &FirstContiguousnessPosition, &SecondContiguousnessPosition);

    *po_pFirstContiguousnessPoint = HGF2DLocation(FirstContiguousnessPosition, GetCoordSys());
    *po_pSecondContiguousnessPoint = HGF2DLocation(SecondContiguousnessPosition, GetCoordSys());
    }



//-----------------------------------------------------------------------------
// ObtainContiguousnessPointsWithPolySegmentAtSCS
// PRIVATE
// Finds contiguousness points with polysegment
// The polysegment and segment must share the same coordinate system
//-----------------------------------------------------------------------------
void HVE2DPolySegment::ObtainContiguousnessPointsWithPolySegmentAtSCS(const HVE2DPolySegment&  pi_rPolySegment,
                                                                      const HGF2DLocation& pi_rPoint,
                                                                      HGF2DLocation* po_pFirstContiguousnessPoint,
                                                                      HGF2DLocation* po_pSecondContiguousnessPoint) const
    {
    HINVARIANTS;

    // The two vectors must share the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rPolySegment.GetCoordSys());

    // Check that recipient locations are provided
    HPRECONDITION(po_pFirstContiguousnessPoint != 0);
    HPRECONDITION(po_pSecondContiguousnessPoint != 0);

    // Transform point into a position in self coordinate system
    HGF2DLocation PointInSelfCoordSys(pi_rPoint, GetCoordSys());
    HGF2DPosition ThePoint(PointInSelfCoordSys.GetPosition());

    HGF2DPosition FirstContiguousnessPosition;
    HGF2DPosition SecondContiguousnessPosition;
    GetPolySegmentPeer().ObtainContiguousnessPointsWithPolySegmentAt(pi_rPolySegment.GetPolySegmentPeer(), ThePoint, &FirstContiguousnessPosition, &SecondContiguousnessPosition);

    *po_pFirstContiguousnessPoint = HGF2DLocation(FirstContiguousnessPosition, GetCoordSys());
    *po_pSecondContiguousnessPoint = HGF2DLocation(SecondContiguousnessPosition, GetCoordSys());
    }


//-----------------------------------------------------------------------------
// ObtainContiguousnessPointsWithPolySegmentAt
// PRIVATE
// Finds contiguousness points with polysegment
//-----------------------------------------------------------------------------
void HVE2DPolySegment::ObtainContiguousnessPointsWithPolySegmentAt(const HVE2DPolySegment&  pi_rPolySegment,
                                                                   const HGF2DLocation& pi_rPoint,
                                                                   HGF2DLocation* po_pFirstContiguousnessPoint,
                                                                   HGF2DLocation* po_pSecondContiguousnessPoint) const
    {
    HINVARIANTS;

    // Check that recipient locations are provided
    HPRECONDITION(po_pFirstContiguousnessPoint != 0);
    HPRECONDITION(po_pSecondContiguousnessPoint != 0);

    // Check if they share the same coordinate system
    if (GetCoordSys() == pi_rPolySegment.GetCoordSys())
        {
        // The two polysegments have the same coordinate system
        ObtainContiguousnessPointsWithPolySegmentAtSCS(pi_rPolySegment,
                                                       pi_rPoint,
                                                       po_pFirstContiguousnessPoint,
                                                       po_pSecondContiguousnessPoint);
        }
    else
        {
        // The two polysegments have different coordinate systems

        HAutoPtr<HVE2DVector> pTempVector(pi_rPolySegment.AllocateCopyInCoordSys(GetCoordSys()));

        // Obtain answer
        pTempVector->ObtainContiguousnessPointsAt(*this,
                                                  pi_rPoint,
                                                  po_pFirstContiguousnessPoint,
                                                  po_pSecondContiguousnessPoint);

//HChk AR ???? Not sure the following is valid
        // We check if the two points are in the proper order
        if (CalculateRelativePosition(*po_pFirstContiguousnessPoint) > CalculateRelativePosition(*po_pSecondContiguousnessPoint))
            {
            // Not in the proper order ... we swap
            HGF2DLocation   SwapLocation(*po_pFirstContiguousnessPoint);
            *po_pFirstContiguousnessPoint = *po_pSecondContiguousnessPoint;
            *po_pSecondContiguousnessPoint = SwapLocation;
            }

        }
    }

//-----------------------------------------------------------------------------
// ObtainContiguousnessPointsWithSegmentAt
// PRIVATE
// Finds contiguousness points with segment at specified location
//-----------------------------------------------------------------------------
void HVE2DPolySegment::ObtainContiguousnessPointsWithSegmentAt(const HVE2DSegment&  pi_rSegment,
                                                               const HGF2DLocation& pi_rPoint,
                                                               HGF2DLocation* po_pFirstContiguousnessPoint,
                                                               HGF2DLocation* po_pSecondContiguousnessPoint) const
    {
    HINVARIANTS;

    // Check that recipient locations are provided
    HPRECONDITION(po_pFirstContiguousnessPoint != 0);
    HPRECONDITION(po_pSecondContiguousnessPoint != 0);

    // Check if they share the same coordinate system
    if (GetCoordSys() == pi_rSegment.GetCoordSys())
        {
        // The polysegment and segment have the same coordinate system
        ObtainContiguousnessPointsWithSegmentAtSCS(pi_rSegment, pi_rPoint, po_pFirstContiguousnessPoint, po_pSecondContiguousnessPoint);
        }
    else
        {
        // The polysegment and segment have different coordinate systems
        HAutoPtr<HVE2DVector> pTempVector(pi_rSegment.AllocateCopyInCoordSys(GetCoordSys()));

        // Obtain answer
        pTempVector->ObtainContiguousnessPointsAt(*this,
                                                  pi_rPoint,
                                                  po_pFirstContiguousnessPoint,
                                                  po_pSecondContiguousnessPoint);

//HChk AR ???? Not sure the following is valid
        // We check if the two points are in the proper order
        if (CalculateRelativePosition(*po_pFirstContiguousnessPoint) > CalculateRelativePosition(*po_pSecondContiguousnessPoint))
            {
            // Not in the proper order ... we swap
            HGF2DLocation   SwapLocation(*po_pFirstContiguousnessPoint);
            *po_pFirstContiguousnessPoint = *po_pSecondContiguousnessPoint;
            *po_pSecondContiguousnessPoint = SwapLocation;
            }
        }
    }




//-----------------------------------------------------------------------------
// AppendSegmentInCoordSys
// PRIVATE
// Adds to the self polysegment a transformed version of segment
//-----------------------------------------------------------------------------
void HVE2DPolySegment::AppendSegmentInCoordSys(const HVE2DSegment& pi_rSegment)
    {
    HINVARIANTS;

    HFCPtr<HGFTolerance> pTol (pi_rSegment.GetStrokeTolerance());
    double StrokeTolerance (GetTolerance());

    pTol->ChangeCoordSys(GetCoordSys());

    if (pTol->GetLinearTolerance() > 0.0)
        {
        StrokeTolerance = pTol->GetLinearTolerance();
        }

    // Obtain the segment intermediate point
    HGF2DLocation   IntermediatePoint(pi_rSegment.GetCoordSys());
    IntermediatePoint.SetX((pi_rSegment.GetStartPoint().GetX() + pi_rSegment.GetEndPoint().GetX()) / 2.0);
    IntermediatePoint.SetY((pi_rSegment.GetStartPoint().GetY() + pi_rSegment.GetEndPoint().GetY()) / 2.0);

    // Transform start point, end point and intermediate point
// HChk AR If domain error thrown ... we should do something!!!!
    HGF2DLocation   TransformedEndPoint(pi_rSegment.GetEndPoint(), GetCoordSys());
    HVE2DSegment    TransformedSegment(pi_rSegment.GetStartPoint().ExpressedIn(GetCoordSys()),
                                       TransformedEndPoint);

    HGF2DLocation   NewIntermediatePoint(IntermediatePoint, GetCoordSys());

    // Obtain distance from transformed segment
    double TheDistanceFromTransformed = (TransformedSegment.CalculateClosestPoint(NewIntermediatePoint)
                                              - NewIntermediatePoint).CalculateLength();


    // Check if tolerance is respected
    if (HDOUBLE_EQUAL(TheDistanceFromTransformed, 0.0, StrokeTolerance))
        {
        // The epsilon is respected ... we add this last segment to linear
        GetPolySegmentPeer().AppendPoint(TransformedEndPoint.GetPosition());

        SetLinearEndPoint(TransformedEndPoint);
        GetPolySegmentPeer().SetLinearEndPoint(GetEndPoint().GetPosition());

        }
    else
        {
        // Since the tolerance is not respected, we split the segment into two smaller ones
        HVE2DSegment    FirstSegment(pi_rSegment.GetStartPoint(), IntermediatePoint);
        FirstSegment.SetStrokeTolerance(pi_rSegment.GetStrokeTolerance());

        AppendSegmentInCoordSys(FirstSegment);

        HVE2DSegment    SecondSegment(IntermediatePoint, pi_rSegment.GetEndPoint());
        SecondSegment.SetStrokeTolerance(pi_rSegment.GetStrokeTolerance());

        AppendSegmentInCoordSys(SecondSegment);
        }

    HINVARIANTS;

    }


//-----------------------------------------------------------------------------
// AllocateCopyInComplexCoordSys
// PRIVATE
// Returns a dynamically allocated copy of the segment transformed according
// in the given coordinate system
//-----------------------------------------------------------------------------
HVE2DVector* HVE2DPolySegment::AllocateCopyInComplexCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const
    {
    HINVARIANTS;

    // Allocate polysegment linear to receive result
    HAutoPtr<HVE2DPolySegment>  pNewPolySegment(new HVE2DPolySegment(pi_rpCoordSys));

    // Desactivate auto tolerance
    pNewPolySegment->SetAutoToleranceActive(false);
    pNewPolySegment->SetTolerance(GetTolerance());

    HFCPtr<HGFTolerance> pTol (GetStrokeTolerance());

    pNewPolySegment->SetStrokeTolerance(pTol);


    // Transform start and end points.
// HChk AR If domain error thrown ... we should do something!!!!
    pNewPolySegment->SetLinearStartPoint(GetStartPoint().ExpressedIn(pi_rpCoordSys));
    pNewPolySegment->SetLinearEndPoint(GetEndPoint().ExpressedIn(pi_rpCoordSys));
    pNewPolySegment->GetPolySegmentPeer().SetLinearStartPoint(pNewPolySegment->GetStartPoint().GetPosition());
    pNewPolySegment->GetPolySegmentPeer().SetLinearEndPoint(pNewPolySegment->GetEndPoint().GetPosition());

// HChk AR If domain error thrown ... we should do something!!!!
    // Obtain a transformed value of start point
    HGF2DLocation TransStartPoint(GetStartPoint(), pi_rpCoordSys);

    // Append start point first
    pNewPolySegment->GetPolySegmentPeer().AppendPoint(TransStartPoint.GetPosition());


    // For every segment of self polysegment
    HGF2DPositionCollection::const_iterator Itr = GetPoints().begin();
    HGF2DPositionCollection::const_iterator PreviousItr = Itr;
    ++Itr;
    for (; Itr != GetPoints().end() ; ++Itr , ++PreviousItr)
        {
        // Append this segment to the complex
        HVE2DSegment aSegment(*PreviousItr, *Itr, GetCoordSys());
        aSegment.SetStrokeTolerance(pTol);

        pNewPolySegment->AppendSegmentInCoordSys(aSegment);
        }

    // In some imprecise transformation models there could be a variation
    // between conversion of two identical coordinates ... adjust if applicable
    if (GetStartPoint().IsEqualTo(GetEndPoint()))
        pNewPolySegment->AdjustEndPointTo(pNewPolySegment->GetStartPoint());

    // If the start point and end points were originally equal then they should be equal in the result
    HASSERT(!GetStartPoint().IsEqualTo(GetEndPoint()) || pNewPolySegment->GetStartPoint().IsEqualTo(pNewPolySegment->GetEndPoint()));

    // Reactive auto tolearnce determination if applicable
    pNewPolySegment->SetAutoToleranceActive(IsAutoToleranceActive());
    pNewPolySegment->ResetTolerance();

    return (pNewPolySegment.release());
    }



//-----------------------------------------------------------------------------
// ResetTolerance
// PRIVATE method
// Recalculates the tolerance if needed and permitted
//-----------------------------------------------------------------------------
void HVE2DPolySegment::ResetTolerance()
    {

    GetPolySegmentPeer().ResetTolerance();

    SetTolerance(GetPolySegmentPeer().GetTolerance());

    HINVARIANTS;

    }

//-----------------------------------------------------------------------------
// MakeEmpty
// This method empties the polysegment
//-----------------------------------------------------------------------------
void HVE2DPolySegment::MakeEmpty()
    {
    HINVARIANTS;

    GetPolySegmentPeer().MakeEmpty();


    // Indicate extent is not up to date
    m_ExtentUpToDate = false;

    // Check if auto tolerance is active
    if (IsAutoToleranceActive())
        {
        // Reset auto tolerance to default
        SetTolerance(HGLOBAL_EPSILON);
        }
    }

//-----------------------------------------------------------------------------
// Reserver
// This method pre-allocates points in polysegment
//-----------------------------------------------------------------------------
void HVE2DPolySegment::Reserve(size_t pi_PointsToPreAllocate)
    {
    HINVARIANTS;
    GetPolySegmentPeer().Reserve(pi_PointsToPreAllocate);
    }


//-----------------------------------------------------------------------------
// IntersectsAtAnySplitPointWithPolySegmentSCS
// This method determines if the given linear connects on self
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::IntersectsAtAnySplitPointWithPolySegmentSCS(const HVE2DPolySegment& pi_rPolySegment) const
    {
    // The coordinate systems between polysegments must be the same
    HPRECONDITION(GetCoordSys() == pi_rPolySegment.GetCoordSys());

    return GetPolySegmentPeer().IntersectsAtAnySplitPointWithPolySegment(pi_rPolySegment.GetPolySegmentPeer());
    }


//-----------------------------------------------------------------------------
// IntersectsAtAnySplitPointWithLiteSegmentSCS
// This method determines if the given segment intersects any split point
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::IntersectsAtAnySplitPointWithLiteSegmentSCS(const HGF2DLiteSegment& pi_rSegment) const
    {

    return GetPolySegmentPeer().IntersectsAtAnySplitPointWithLiteSegment(pi_rSegment);
    }



//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the polysegment in the given output stream
// in text format
//-----------------------------------------------------------------------------
void HVE2DPolySegment::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    po_rOutput << "Object is a HVE2DPolySegment" << endl;
    HDUMP0("Object is a HVE2DPolySegment\n");

    HVE2DBasicLinear::PrintState(po_rOutput);

    GetPolySegmentPeer().PrintState(po_rOutput);
#endif
    }

//-----------------------------------------------------------------------------
// Simplify()
// PRIVATE METHOD
// Simplifies the linear
//-----------------------------------------------------------------------------
void HVE2DPolySegment::Simplify(bool processAsClosed)
    {
    HINVARIANTS;

    GetPolySegmentPeer().Simplify(processAsClosed);

    // Make sure that start and end point are still equal
    SetLinearStartPoint(HGF2DLocation(GetPolySegmentPeer().GetStartPoint(), GetCoordSys()));
    SetLinearEndPoint(HGF2DLocation(GetPolySegmentPeer().GetEndPoint(), GetCoordSys()));
    }


// &&AR IS IT NEEDED?
#if (1)

//-----------------------------------------------------------------------------
// SplitIntoNonAutoCrossing
// @description This method is an helper used for the separation into parts
//              of an autocrossing linear. The self linear must autocross
//              in order to call this method. The method operates differently
//              depending upon the value of the pi_ProcessClosed parameter.
//              if set to false (the default), then the method returns a list
//              of polysegments split at every autocross points regardless of
//              the fact that the polysegment may have been originally autoclosing
//              If the pi_ProcessClosed parameter is set to true, then the
//              self polysegment must auto closed. In such case, the method will
//              initially generate a list of polysegment components, then fit
//              components together in order to result in a list of autoclosed
//              polysegment.
//
// @param pio_pListOfResultPolySegments A list of polysegments to which will
//              be appended non autocrossing polysegments resulting from operation.
//
// @param pi_ProcessClosed if set to true, then the self polysegment must be autoclosed
//                         then the method will generate a list of autoclosed
//                         non autocrossing polysegments. If set to false, then
//                         the result is a simple list of polysegments resulting
//                         from the split.
//
// @bsimethod                                          Alain Robert (2003/08/13)
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::SplitIntoNonAutoCrossing(list<HFCPtr<HVE2DPolySegment> >* pio_pListOfResultPolySegments,
                                                bool pi_ProcessClosed) const
    {
    bool result = true;

    // Make sure that self autocrosses
    HASSERTSUPERDEBUG(AutoCrosses());

    // If process closed is set to true, then self must autoclose
    HPRECONDITION(!pi_ProcessClosed || (GetStartPoint() == GetEndPoint()));

    // Obtain all autointersect points
    HGF2DLocationCollection MyAutoIntersectPoints;
    AutoIntersect(&MyAutoIntersectPoints);

    // Make sure that there are intersection points
    HASSERT(MyAutoIntersectPoints.size() > 0);

    // Sort intersection points in increasing order of relative position.
    SortPointsAccordingToRelativePosition(&MyAutoIntersectPoints);

    // Create list of components
    // Create a copy of self
    list<HFCPtr<HVE2DPolySegment> > CurrentListOfPolySegments;

    // Initialize list to current copy
    CurrentListOfPolySegments.push_back(HFCPtr<HVE2DPolySegment>(new HVE2DPolySegment(*this)));

    // For every auto intersect point
    HGF2DLocationCollection::iterator MySplitPointItr;
    MySplitPointItr = MyAutoIntersectPoints.begin();
    while(result && (MySplitPointItr != MyAutoIntersectPoints.end()))
        {

        // For every polysegment in list of components
        list<HFCPtr<HVE2DPolySegment> >::iterator PolySegmentItr;
        PolySegmentItr = CurrentListOfPolySegments.begin();

        for ( ; result && (PolySegmentItr != CurrentListOfPolySegments.end()) ; ++ PolySegmentItr)
            {
            // Check if point is located on the current polysegment
            if ((*PolySegmentItr)->IsPointOn(*MySplitPointItr, EXCLUDE_EXTREMITIES))
                {
                // Create a duplicate of current component
                HFCPtr<HVE2DPolySegment> pNewMiddlePolySegment = new HVE2DPolySegment(**PolySegmentItr);

                // Check if the split point is also start point
                if (MySplitPointItr->IsEqualTo((*PolySegmentItr)->GetStartPoint(), GetTolerance()))
                    {
                    // Reverse the current polysegments
                    (*PolySegmentItr)->Reverse();
                    pNewMiddlePolySegment->Reverse();

                    (*PolySegmentItr)->ShortenTo(*MySplitPointItr);
                    pNewMiddlePolySegment->ShortenFrom(*MySplitPointItr);

                    // Set back points in proper order
                    (*PolySegmentItr)->Reverse();
                    pNewMiddlePolySegment->Reverse();
                    }
                else
                    {
                    // Shorten to current polysegment
                    (*PolySegmentItr)->ShortenFrom(*MySplitPointItr);

                    // Shorten new portion to same point
                    pNewMiddlePolySegment->ShortenTo(*MySplitPointItr);

                    if (HDOUBLE_EQUAL (pNewMiddlePolySegment->CalculateLength(), 0.0, GetTolerance()))
                        {
                        // Since the new polysegment is null length it implies that the
                        // tolerance is unacceptable for the current coordinates and we are started into an infinite loop
                        result = false;
                        }
                    }

                // Insert component to list
                PolySegmentItr = CurrentListOfPolySegments.insert(PolySegmentItr, pNewMiddlePolySegment);

                }
            }

        MySplitPointItr++;


        }


    // Copy list to return list
    list<HFCPtr<HVE2DPolySegment> >::iterator CopyItr = CurrentListOfPolySegments.begin();
    for ( ; result && (CopyItr != CurrentListOfPolySegments.end()) ; ++CopyItr)
        {
        // Copy pointer
        pio_pListOfResultPolySegments->push_back(*CopyItr);
        }


    // If closed processing was asked for then modify list
    if (result && pi_ProcessClosed)
        {
        RecomposeClosedPolySegments(pio_pListOfResultPolySegments);
        }

    return result;
    }



// &&AR IS IT NEEDED?


//-----------------------------------------------------------------------------
// RecomposeClosedPolySegments
// PRIVATE
// Recomposes closed polysegments from a set of split polysegments.
//
// @bsimethod                                          Alain Robert (2003/08/13)
//-----------------------------------------------------------------------------
void HVE2DPolySegment::RecomposeClosedPolySegments(list<HFCPtr<HVE2DPolySegment> >* pio_pListOfResultPolySegments) const
    {
    HPRECONDITION (pio_pListOfResultPolySegments->size() > 0);

    // In such case we first determine if the original polysegment rotation direction
    double TotalAreaSigned = CalculateRayArea(HGF2DLocation(GetStartPoint(), GetCoordSys()));

    bool GoesCW = (TotalAreaSigned < 0.0);

    // In such case, portions part of the list must be fit with each other until all closed.
    // Create a duplicate list of linears
    list<HFCPtr<HVE2DPolySegment> > TempList(*pio_pListOfResultPolySegments);
    pio_pListOfResultPolySegments->clear();

    // Allocate a list of flags
    HArrayAutoPtr<bool> pFlags(new bool[TempList.size()]);

    // Initialize to false to indicate all parts are unused
    memset(pFlags, 0, TempList.size() * sizeof(bool));

    // Until all components have been accounted for ...
    size_t i = 0;
    size_t i2 = 0;
    list<HFCPtr<HVE2DPolySegment> >::iterator Itr = TempList.begin();
    while(Itr != TempList.end())
        {
        // Check if this component was already used
        if (!pFlags[i])
            {
            // Indicate that the component has been used.
            pFlags[i] = true;

            // Create New result polysegment
            HFCPtr<HVE2DPolySegment> pNewPolySegment = new HVE2DPolySegment(**Itr);

            while(!(pNewPolySegment->GetStartPoint().IsEqualTo(pNewPolySegment->GetEndPoint(), GetTolerance())))
                {
                // For all next segments search for a connecting
                list<HFCPtr<HVE2DPolySegment> >::iterator SecondItr = Itr;
                ++SecondItr;
                i2 = i+1;

                bool FoundLinked = false;
                size_t FoundIndex = 0;
                bool FoundIsStart=false;
                list<HFCPtr<HVE2DPolySegment> >::iterator FoundItr;

                while(SecondItr != TempList.end())
                    {
                    // Check if already used
                    if (!pFlags[i2])
                        {

                        // Check if they connect
                        if (((*SecondItr)->GetStartPoint().IsEqualTo(pNewPolySegment->GetEndPoint(), GetTolerance())) ||
                            ((*SecondItr)->GetEndPoint().IsEqualTo(pNewPolySegment->GetEndPoint(), GetTolerance())))
                            {
                            if (!FoundLinked)
                                {
                                FoundLinked = true;
                                FoundIndex = i2;
                                FoundItr = SecondItr;
                                FoundIsStart = ((*SecondItr)->GetStartPoint().IsEqualTo(pNewPolySegment->GetEndPoint(), GetTolerance()));
                                }
                            else
                                {
                                // One already found ... check if new is a better choice
                                // Create composite of current and previously found
                                HVE2DPolySegment TempPolySegment(*pNewPolySegment);
                                if (FoundIsStart)
                                    {
                                    for (size_t j = 1 ; j < (*FoundItr)->GetSize() ; ++j)
                                        TempPolySegment.AppendPosition((*FoundItr)->GetPosition(j));
                                    }
                                else
                                    {
                                    for (int32_t j = (int32_t)((*FoundItr)->GetSize() - 2) ; j >= 0 ; --j)
                                        TempPolySegment.AppendPosition((*FoundItr)->GetPosition(j));
                                    }

                                HVE2DPolySegment OtherPolySegment(**SecondItr);
                                if (!((*SecondItr)->GetStartPoint().IsEqualTo(pNewPolySegment->GetEndPoint(), GetTolerance())))
                                    OtherPolySegment.Reverse();

                                HGFBearing    ReferenceBearing = OtherPolySegment.CalculateBearing(pNewPolySegment->GetEndPoint(), HVE2DVector::BETA);

                                // obtain bearing at point
                                HGFBearing  SelfBearing1 = TempPolySegment.CalculateBearing(pNewPolySegment->GetEndPoint(), HVE2DVector::BETA);
                                HGFBearing  SelfBearing2 = TempPolySegment.CalculateBearing(pNewPolySegment->GetEndPoint(), HVE2DVector::ALPHA);

                                double AngleDiff = CalculateNormalizedTrigoValue(SelfBearing1 - ReferenceBearing);

                                double SelfAngleDiff = CalculateNormalizedTrigoValue(SelfBearing1 - SelfBearing2);


                                // Check if bearing points leftward
                                if ((SelfAngleDiff > AngleDiff) == ((i%2 == 0) ? GoesCW : !GoesCW))
                                    {
                                    FoundIndex = i2;
                                    FoundItr = SecondItr;
                                    FoundIsStart = ((*SecondItr)->GetStartPoint().IsEqualTo(pNewPolySegment->GetEndPoint(), GetTolerance()));
                                    }
                                }
                            }
                        }

                    ++SecondItr;
                    ++i2;

                    }

                // One polysegment must have been found
                HASSERT(FoundLinked);

                // Link new polysegment to new component found
                if (FoundIsStart)
                    {
                    for (size_t j = 1 ; j < (*FoundItr)->GetSize() ; ++j)
                        pNewPolySegment->AppendPosition((*FoundItr)->GetPosition(j));
                    }
                else
                    {
                    for (int32_t j = (int32_t)((*FoundItr)->GetSize() - 2) ; j >= 0 ; --j)
                        pNewPolySegment->AppendPosition((*FoundItr)->GetPosition(j));
                    }

                pFlags[FoundIndex] = true;

                }

            // Adjust end point to make sure it is exactly closed
            pNewPolySegment->AdjustEndPointTo(pNewPolySegment->GetStartPoint());

            // Add result linear to output list
            pio_pListOfResultPolySegments->push_back(pNewPolySegment);
            }

        // Found next polysegment that has not been used yet!
        Itr = TempList.begin();
        i = 0;
        while((pFlags[i] == true) && Itr != TempList.end())
            {
            ++Itr;
            ++i;
            }
        }
    }



// &&AR IS IT NEEDED?

//-----------------------------------------------------------------------------
// SortPointsAccordingToRelativePosition
// @description This method sorts the provided list of points according to
//              increasing order of relative position.
//
// @param pio_pListOfPointsOnLinear A list of points that must all be on linear
//                                  to sort.
//
// @bsimethod                                          Alain Robert (2003/08/13)
//-----------------------------------------------------------------------------
void HVE2DPolySegment::SortPointsAccordingToRelativePosition(HGF2DLocationCollection* pio_pListOfPointsOnLinear) const
    {

    // Sort intersection points in increasing order of relative position.
    HGF2DLocationCollection::iterator FirstPointItr;
    HGF2DLocationCollection::iterator SecondPointItr;

    FirstPointItr = pio_pListOfPointsOnLinear->begin();

    // For every points ...
    while(FirstPointItr != pio_pListOfPointsOnLinear->end())
        {
        SecondPointItr = FirstPointItr;
        SecondPointItr++;

        // From next point till end of list
        while(SecondPointItr!= pio_pListOfPointsOnLinear->end())
            {
            // Check if second point has a smaller relative position than first
            if (CalculateRelativePosition(*FirstPointItr) > CalculateRelativePosition(*SecondPointItr))
                {
                std::swap(*FirstPointItr, *SecondPointItr);
                }
            ++SecondPointItr;
            }
        ++FirstPointItr;
        }
    }

#endif
