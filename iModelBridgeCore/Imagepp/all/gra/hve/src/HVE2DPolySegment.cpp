//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hve/src/HVE2DPolySegment.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HVE2DPolySegment
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGFAngle.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DLiteSegment.h>
#include <Imagepp/all/h/HVE2DPolySegment.h>
#include <Imagepp/all/h/HVE2DSegment.h>

#include <Imagepp/all/h/HFCException.h>

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

    HPRECONDITION(po_pPoints != 0);

    // Declare answer variable
    size_t NumberAutoCrossPoints = 0;

    // Pre-calculate tolerance
    double Tolerance = GetTolerance();

    double SelfXMin;
    double SelfXMax;
    double SelfYMin;
    double SelfYMax;

    bool Result;

    // Check that there are at least 4 points (otherwise autocrossing is impossible)
    if (m_Points.size() >= 4)
        {
        // For every segment of the polysegment
        HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
        HGF2DPositionCollection::const_iterator PreviousItr(Itr);
        ++Itr;
        for (; Itr != m_Points.end() ; ++Itr , ++PreviousItr)
            {
            // Obtain extent of this segment
            SelfXMin = MIN(Itr->GetX(), PreviousItr->GetX());
            SelfXMax = MAX(Itr->GetX(), PreviousItr->GetX());
            SelfYMin = MIN(Itr->GetY(), PreviousItr->GetY());
            SelfYMax = MAX(Itr->GetY(), PreviousItr->GetY());

            // Create a lite segment to represent current segment
            HGF2DLiteSegment TheLiteSegment(*PreviousItr, *Itr, Tolerance);

            // For every other segment of polysegment
            HGF2DPositionCollection::const_iterator OtherItr = Itr;
            HGF2DPositionCollection::const_iterator PreviousOtherItr(OtherItr);
            ++OtherItr;
            for (; OtherItr != m_Points.end() ; ++OtherItr , ++PreviousOtherItr)
                {
                // Check that it is not the same segment
                if (OtherItr != Itr)
                    {
                    // Check if current self segment and current other segment may interact
                    Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, MIN(OtherItr->GetX(), PreviousOtherItr->GetX()), Tolerance) &&
                              HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, MAX(OtherItr->GetX(), PreviousOtherItr->GetX()), Tolerance) &&
                              HDOUBLE_GREATER_OR_EQUAL(SelfYMax, MIN(OtherItr->GetY(), PreviousOtherItr->GetY()), Tolerance) &&
                              HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, MAX(OtherItr->GetY(), PreviousOtherItr->GetY()), Tolerance)
                             );


                    if (Result)
                        {
                        HGF2DPosition CrossPoint;

                        // The two segments may interact ... check if they cross
                        if(HGF2DLiteSegment::CROSS_FOUND == HGF2DLiteSegment(*PreviousOtherItr,
                                                                             *OtherItr,
                                                                             Tolerance).IntersectSegment(TheLiteSegment, &CrossPoint))
                            {
                            // Add new intersect point to list
                            po_pPoints->push_back(HGF2DLocation(CrossPoint, GetCoordSys()));

                            NumberAutoCrossPoints++;
                            }
                        }
                    }
                }
            }
        }

    // Now obtain the intersection at split points

    // The following applies only if there are at least 5 points in polysegment


    // The following applies only if there are at least 5 points in polysegment
    if (m_Points.size() >= 5)
        {
        // For every triplets of points
        HGF2DPositionCollection::const_iterator FirstItr(m_Points.begin());
        HGF2DPositionCollection::const_iterator SecondItr(FirstItr);
        HGF2DPositionCollection::const_iterator ThirdItr(FirstItr);
        ++SecondItr;
        ++ThirdItr;
        ++ThirdItr;

        HGF2DPositionCollection::const_iterator FourthItr(ThirdItr);
        HGF2DPositionCollection::const_iterator FifthItr(ThirdItr);
        HGF2DPositionCollection::const_iterator SixthItr(ThirdItr);
        ++FifthItr;
        ++SixthItr;
        ++SixthItr;

        // Create a small polygon of segments with two segments.
        HVE2DPolySegment FirstPolySegment(GetCoordSys());
        FirstPolySegment.AppendPosition(*FirstItr);
        FirstPolySegment.AppendPosition(*SecondItr);

        HVE2DPolySegment SecondPolySegment(GetCoordSys());
        HGF2DPositionCollection::const_iterator TempItr(FourthItr);
        for (; TempItr != m_Points.end() ; ++TempItr)
            {
            SecondPolySegment.AppendPosition(*TempItr);
            }

        for (; SixthItr != m_Points.end() ; ++FirstItr , ++SecondItr, ++ThirdItr, ++FourthItr, ++FifthItr, ++SixthItr)
            {
            // Add new point to First polysegment
            FirstPolySegment.AppendPosition(*ThirdItr);

            HGF2DLocation SplitPointA(*SecondItr, GetCoordSys());
            HGF2DLocation NextToSplitPointA(*ThirdItr, GetCoordSys());

            HGF2DLocation SplitPointB(*FifthItr, GetCoordSys());
            HGF2DLocation NextToSplitPointB(*SixthItr, GetCoordSys());


            // Ask if these cross at split (second) point
            if (FirstPolySegment.IntersectsAtSplitPointWithPolySegment(SecondPolySegment,
                                                                       SplitPointA,
                                                                       NextToSplitPointA,
                                                                       true))
                {

                po_pPoints->push_back(SplitPointA);

                NumberAutoCrossPoints++;
                }


            if (SecondPolySegment.IntersectsAtSplitPointWithPolySegment(FirstPolySegment,
                                                                        SplitPointB,
                                                                        NextToSplitPointB,
                                                                        true))
                {
                // Check if this point was already added to the list
                bool Found = false;
                HSINTX FindIndex = (HSINTX)NumberAutoCrossPoints - 1;

                for ( ; !Found && (FindIndex >= 0) ; --FindIndex)
                    {
                    Found = (SplitPointB.IsEqualTo((*po_pPoints)[po_pPoints->size() - FindIndex - 1], GetTolerance()));
                    }

                if (!Found)
                    {
                    po_pPoints->push_back(SplitPointB);

                    NumberAutoCrossPoints++;
                    }
                }

            // Remove first point of second polysegment
            SecondPolySegment.RemovePoint(0);
            }
        }


    return(NumberAutoCrossPoints);
    }


//-----------------------------------------------------------------------------
// GetClosestSegment
//-----------------------------------------------------------------------------
HVE2DSegment HVE2DPolySegment::GetClosestSegment(const HGF2DLocation& pi_rPoint) const
    {
    HINVARIANTS;

    // Create recipient location with initialization flag
    bool           ClosestPointInitialized = false;
    HGF2DLocation   ClosestPoint(GetCoordSys());
    HVE2DSegment    ResultSegment(GetCoordSys());

    // Check if self is not NULL
    if (m_Points.size() >= 2)
        {
        // Create a segment ... this segment will be used for all
        // operations thus preventing multiple creation of segments
        HVE2DSegment TheSegment(GetCoordSys());

        // For every segment part of the polysegment
        HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
        HGF2DPositionCollection::const_iterator PreviousItr(Itr);
        ++Itr;

        for (; Itr != m_Points.end() ; ++Itr , ++PreviousItr)
            {
            // Set segment to represent current segment
            // The coordinate system of segment is already the proper one.
            TheSegment.SetRawStartPoint(PreviousItr->GetX(), PreviousItr->GetY());
            TheSegment.SetRawEndPoint(Itr->GetX(), Itr->GetY());

            // Calculate closest point to the segment
            HGF2DLocation TempClosestPoint = TheSegment.CalculateClosestPoint(pi_rPoint);

            // Check if new closest point is closer than before (or first)
            if ((!ClosestPointInitialized) ||
                ((pi_rPoint - TempClosestPoint).CalculateLength() < (pi_rPoint - ClosestPoint).CalculateLength()))
                {
                // The new point is closer (or first) ... retain value
                ClosestPoint = TempClosestPoint;

                // Save segment
                ResultSegment = TheSegment;

                // Inidicate closest point was initialized
                ClosestPointInitialized = true;
                }
            }
        }

    return(ResultSegment);
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

    // Allocate result polysegment
    HAutoPtr<HVE2DPolySegment> pNewPolySegment(new HVE2DPolySegment(GetCoordSys()));

    // Obtain extent
    HGF2DExtent MyExtent(GetExtent());

    // Obtain largest dimension
    double GreatestSize = MAX(MyExtent.GetWidth(), MyExtent.GetHeight());

    // Make sure we have the largest possible (greater than any size part of extent)
    GreatestSize *= 2.0;


    HVE2DSegment ReversalSegment(GetCoordSys());

    // Check there is at least two points in polysegment
    // If there are not two points, then result is empty
    if (GetSize() >= 2)
        {
        // For every segment of the polysegment
        HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
        HGF2DPositionCollection::const_iterator PreviousItr(Itr);
        ++Itr;

        // This iterator points on the next point
        HGF2DPositionCollection::const_iterator NextItr(Itr);
        ++NextItr;


        // Process the first point
        HGF2DLocation FirstPoint(*PreviousItr, GetCoordSys());

        // This location holds the previous point
        HGF2DLocation MyPreviousPoint(*PreviousItr, GetCoordSys());

        double Sweep;
        HGFBearing PerpendicularBearing;

        PerpendicularBearing = CalculatePerpendicularBearingAt(FirstPoint, pi_DirectionToRight, &Sweep);

        FirstPoint += HGF2DDisplacement(PerpendicularBearing,
                                        pi_Offset / cos(Sweep));

        // Check is an alignment was provided for the first point
        if (pi_pFirstPointAlignment)
            {
            // A first point alignement was provided ...
            // We must modify the result first point


            // We obtain the second point
            HGF2DLocation SecondPoint(*Itr, GetCoordSys());

            PerpendicularBearing = CalculatePerpendicularBearingAt(SecondPoint, pi_DirectionToRight, &Sweep);
            SecondPoint += HGF2DDisplacement(PerpendicularBearing,
                                             pi_Offset / cos(Sweep));

            // With these two points we create a line
            HGF2DLine TempLine(FirstPoint, SecondPoint);

            // Recipient for cross point
            HGF2DLocation CrossPoint(GetCoordSys());

            // We intersect with alignment line
            if (TempLine.IntersectLine(*pi_pFirstPointAlignment, &CrossPoint) == HGF2DLine::CROSS_FOUND)
                {
                // A cross was found ... this should be our first point
                FirstPoint = CrossPoint;
                }

            // If no cross was found, then algnement was parallel to segment and
            // thus invalid ... we neglect aligment!
            }

        // We have our first point
        pNewPolySegment->AppendPoint(FirstPoint);

        // This location hold the previous point after transformation
        HGF2DLocation MyPreviousTransformedPoint = FirstPoint;

        // This location holds the previous point before transformation

        // This variable indicates if a reversal was encountered
        bool ReversalEncountered = false;
        HVE2DPolySegment ReversalSection(GetCoordSys());
        HGF2DLocationCollection ReversalCrossPoints;

        // For all points other that first point
        for (; NextItr != m_Points.end() ; ++Itr , ++PreviousItr, ++NextItr)
            {
            // Obtain current point
            HGF2DLocation MyPoint(*Itr, GetCoordSys());

            // Calculate current bearing
            HGFBearing SegmentBearing = (MyPoint - MyPreviousPoint).CalculateBearing();

            // Transform point
            PerpendicularBearing = CalculatePerpendicularBearingAt(MyPoint, pi_DirectionToRight, &Sweep);
            MyPoint += HGF2DDisplacement(PerpendicularBearing,
                                         pi_Offset / cos(Sweep));



            HVE2DSegment MyTempSegment(MyPreviousTransformedPoint, MyPoint);

            // Check if this segment crosses the polysegment
            if (pNewPolySegment->Crosses(MyTempSegment))
                {
                // They do cross,
                HGF2DLocationCollection MyIntersectPoint;

                for (size_t i = 0 ; i < MyIntersectPoint.size() ; ++ i)
                    {
                    if (pNewPolySegment->IsPointOn(MyIntersectPoint[i]))
                        {
                        MyPoint = MyIntersectPoint[i];

                        pNewPolySegment->ShortenTo(MyPoint);
                        }
                    }

                ReversalEncountered = false;

                }
            else
                {
                pNewPolySegment->AppendPoint(MyPoint);

                }


            // Check if bearing of transformed has been modified
            // This only occurs in segment reversal
            HGFBearing TransformedSegmentBearing = (MyPoint - MyPreviousTransformedPoint).CalculateBearing();

            if (ReversalEncountered)
                {

                // Check if the current polysegment crosses the reversal segment
                if (pNewPolySegment->Crosses(ReversalSegment))
                    {
                    // They do cross,
                    HGF2DLocationCollection MyIntersectPoint;

                    pNewPolySegment->Intersect(ReversalSegment, &MyIntersectPoint);

                    // Rewind to start of reversal (Note for some reason sometimes the start of reversal is
                    // not on the segment (possibly after multiple consecutive reversals)
                    if (pNewPolySegment->IsPointOn(ReversalSegment.GetStartPoint()))
                        pNewPolySegment->ShortenTo(ReversalSegment.GetStartPoint());

                    // Add intersect point
                    pNewPolySegment->AppendPoint(MyIntersectPoint[0]);

                    // Re-add new point (Which was removed by shorten)
                    pNewPolySegment->AppendPoint(MyPoint);


                    ReversalEncountered = false;
                    }



                }
            else
                {
                // We are not processing a reversal
                // At this point, the reversal section should be empty
                HASSERT(ReversalSection.GetSize() == 0);



                // Check if bearing are equal
//                if (TransformedSegmentBearing.IsEqualTo(SegmentBearing))
                if (TransformedSegmentBearing.IsEqualTo(SegmentBearing) ||
                    TransformedSegmentBearing.IsBearingWithinSweep(PI /2, SegmentBearing) ||
                    TransformedSegmentBearing.IsBearingWithinSweep(-PI /2, SegmentBearing))

                    {
                    // nothing to do ...
                    }
                else
                    {
                    // The bearing has changed (reversed)
                    ReversalEncountered = true;


                    FirstPoint = MyPreviousTransformedPoint;

                    HGF2DLocation SecondPoint = MyPoint + HGF2DDisplacement(TransformedSegmentBearing + PI,
                                                                            GreatestSize);

                    ReversalSegment.SetStartPoint(FirstPoint);
                    ReversalSegment.SetEndPoint(SecondPoint);
                    }
                }

            // Save transformed and untransformed points
            MyPreviousTransformedPoint = MyPoint;
            MyPreviousPoint = HGF2DLocation(*Itr, GetCoordSys());

            }

        // We have processed all points except last one
        // We will not process it if we are in a reversal
// HMChk AR ... If we are within a reversal we should process it as every other points
        // Process the last point
        HGF2DLocation LastPoint(*Itr, GetCoordSys());

        HGF2DLocation NextToLastPoint(*PreviousItr, GetCoordSys());

        // Check if the first point is identical to last (occurs often)
        if (GetStartPoint().IsEqualTo(GetEndPoint()))
            {
            // First point is equal to last ... asking for the bearing to the polysegment
            // would provide us with the first point bearing not last point
            // We therefore create a segment for the purpose of computing bearing only
            HVE2DSegment TheLastSegment(NextToLastPoint, LastPoint);

            PerpendicularBearing = TheLastSegment.CalculatePerpendicularBearingAt(LastPoint, pi_DirectionToRight, &Sweep);
            }
        else
            {
            // The last point is different from first point ....
            PerpendicularBearing = CalculatePerpendicularBearingAt(LastPoint, pi_DirectionToRight, &Sweep);
            }

        LastPoint += HGF2DDisplacement(PerpendicularBearing,
                                       pi_Offset / cos(Sweep));

        // Check is an alignment was provided for the first point
        if (pi_pLastPointAlignment)
            {
            // A last point alignement was provided ...
            // We must modify the result last point


            // We obtain the second point
            PerpendicularBearing = CalculatePerpendicularBearingAt(NextToLastPoint, pi_DirectionToRight, &Sweep);
            NextToLastPoint += HGF2DDisplacement(PerpendicularBearing,
                                                 pi_Offset / cos(Sweep));

            // With these two points we create a line
            HGF2DLine TempLine(NextToLastPoint, LastPoint);

            // Recipient for cross point
            HGF2DLocation CrossPoint(GetCoordSys());

            // We intersect with alignment line
            if (TempLine.IntersectLine(*pi_pLastPointAlignment, &CrossPoint) == HGF2DLine::CROSS_FOUND)
                {
                // A cross was found ... this should be our first point
                LastPoint = CrossPoint;
                }

            // If no cross was found, then algnement was parallel to segment and
            // thus invalid ... we neglect aligment!
            }

        pNewPolySegment->AppendPoint(LastPoint);
        }

    // NOTE:
    // It is possible that after this process the result polysegment autocrosses
    // This may occur even without reversal ... such
    // autocrossing points are tolerated and are part of the result

    return(pNewPolySegment.release());
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

    // Pre-calculate tolerance
    double Tolerance = GetTolerance();

    // Check that there are at least 3 points (otherwise autocontiguousness is impossible)
    if (m_Points.size() >= 3)
        {
        // In the following we designate the head as the last point of
        // a triplet, the middle point as the middle point of this triplet
        // and the back(althoug not declared) as the first
        // Likewise, the back segment is the first segment of
        // a consecutive pair and the head is the second of this pair

        // For every consecutive segment of the polysegment
        HGF2DPositionCollection::iterator HeadItr(m_Points.begin());
        HGF2DPositionCollection::iterator MiddleItr(HeadItr);
        ++HeadItr;

        // Create the first back segment
        HGF2DLiteSegment TheBackSegment(*MiddleItr, *HeadItr, Tolerance);

        // Pre-create the head segment
        HGF2DLiteSegment TheHeadSegment;
        TheHeadSegment.SetTolerance(Tolerance);

        // Advance to second segment
        ++HeadItr;
        ++MiddleItr;

        // Check if polysegment should be considered closed ...
        if (pi_ClosedProcessing)
            {
            // The polysegment should be considered closed
            HGF2DPositionCollection::iterator BackItr(m_Points.begin());

            for (; m_Points.size() >= 3 && BackItr != m_Points.end() ; ++HeadItr , ++MiddleItr)
                {

                // Check if head iterator overflows
                if (HeadItr == m_Points.end())
                    {
                    // Head iterator overflows ... set to second point (first is equal to last);
                    HeadItr = m_Points.begin();
                    ++HeadItr;

                    // Set middle iterator to first (equal to last point)
                    MiddleItr = m_Points.begin();
                    }

                // Set head segment to represent consecutive segments
                TheHeadSegment.SetStartPoint(*MiddleItr);
                TheHeadSegment.SetEndPoint(*HeadItr);

                if (MiddleItr->IsEqualTo(*HeadItr, Tolerance) || TheHeadSegment.AreContiguous(TheBackSegment))
                    {
                    // Two consecutive segments are contiguous ... we have a needle
                    // We remove the shortest segment by deleting the middle point

                    // Delete point
                    // First check if middle itr is on first point
                    if (MiddleItr == m_Points.begin())
                        {
                        // Since middle iterator is on first point, then first point (and last must be destroyed)
                        m_Points.erase(MiddleItr);
                        m_Points.pop_back();

                        // Add new last point for closing
                        m_Points.push_back(*(m_Points.begin()));

                        // Reset start and end points
                        m_StartPoint = HGF2DLocation(*(m_Points.begin()), GetCoordSys());
                        m_EndPoint = m_StartPoint;
                        ResetTolerance();
                        }
                    else
                        {
                        // The middle iterator is not at first point
                        m_Points.erase(MiddleItr);
                        }

                    // Restart process from start
                    if (m_Points.size() >= 3)
                        {
                        // We restart the process
                        HeadItr = m_Points.begin();
                        MiddleItr = HeadItr;
                        ++HeadItr;
                        BackItr = m_Points.begin();

                        // Reset the back segment
                        TheBackSegment.SetStartPoint(*MiddleItr);
                        TheBackSegment.SetEndPoint(*HeadItr);
                        }
                    else
                        {
                        // Break now because HeadItr or MiddleItr could be invalid because of the erase operation. In VC8, ++HeadItr, ++MiddleItr crash.
                        break;
                        }
                    }
                else
                    {
                    // The head segment becomes the back segment
                    TheBackSegment = TheHeadSegment;
                    ++BackItr;
                    }
                }
            }
        else
            {
            // Not closed processing ...
            for (; m_Points.size() >= 3 && HeadItr != m_Points.end() ; ++HeadItr , ++MiddleItr)
                {
                // Set head segment to represent consecutive segments
                TheHeadSegment.SetStartPoint(*MiddleItr);
                TheHeadSegment.SetEndPoint(*HeadItr);

                if (MiddleItr->IsEqualTo(*HeadItr, Tolerance) || TheHeadSegment.AreContiguous(TheBackSegment))
                    {
                    // Two consecutive segments are contiguous ... we have a needle
                    // We remove the shortest segment by deleting the middle point

                    // Delete point
                    m_Points.erase(MiddleItr);

                    // Restart process from start
                    if (m_Points.size() >= 3)
                        {
                        HeadItr = m_Points.begin();
                        MiddleItr = HeadItr;
                        ++HeadItr;

                        // Reset the back segment
                        TheBackSegment.SetStartPoint(*MiddleItr);
                        TheBackSegment.SetEndPoint(*HeadItr);
                        }
                    else
                        {
                        // Break now because HeadItr or MiddleItr could be invalid because of the erase operation. In VC8, ++HeadItr, ++MiddleItr crash.
                        break;
                        }
                    }
                else
                    {
                    // The head segment becomes the back segment
                    TheBackSegment = TheHeadSegment;
                    }
                }
            }
        }
    }


#if (0)

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

    // Pre-calculate tolerance
    double Tolerance = GetTolerance();

    // Check that there are at least 3 points (otherwise autocontiguousness is impossible)
    if (m_Points.size() >= 3)
        {
        // In the following we designate the head as the last point of
        // a triplet, the middle point as the middle point of this triplet
        // and the back(althoug not declared) as the first
        // Likewise, the back segment is the first segment of
        // a consecutive pair and the head is the second of this pair

        // For every consecutive segment of the polysegment
        HGF2DPositionCollection::iterator HeadItr(m_Points.begin());
        HGF2DPositionCollection::iterator MiddleItr(HeadItr);
        ++HeadItr;

        // Create the first back segment
        HGF2DLiteSegment TheBackSegment(*MiddleItr, *HeadItr, Tolerance);

        // Pre-create the head segment
        HGF2DLiteSegment TheHeadSegment;
        TheHeadSegment.SetTolerance(Tolerance);

        // Advance to second segment
        ++HeadItr;
        ++MiddleItr;


#if (0)
        for (; HeadItr != m_Points.end() ; ++HeadItr , ++MiddleItr)
            {

            // Set head segment to represent consecutive segments
            TheHeadSegment.SetStartPoint(*MiddleItr);
            TheHeadSegment.SetEndPoint(*HeadItr);

            if (TheHeadSegment.AreContiguous(TheBackSegment))
                {
                // Two consecutive segments are contiguous ... we have a needle
                // We remove the shortest segment by deleting the middle point

                // Set the back segment (By setting the head)
                TheHeadSegment.SetStartPoint(TheBackSegment.GetStartPoint());

                // Delete point
                HeadItr = m_Points.erase(MiddleItr);

                // Reset the middle iterator to current back point
                MiddleItr = HeadItr;
                --MiddleItr;

                }

            // The head segment becomes the back segment
            TheBackSegment = TheHeadSegment;
            }
#else
        for (; m_Points.size() >= 3 && HeadItr != m_Points.end() ; ++HeadItr , ++MiddleItr)
            {

            // Set head segment to represent consecutive segments
            TheHeadSegment.SetStartPoint(*MiddleItr);
            TheHeadSegment.SetEndPoint(*HeadItr);

            if (MiddleItr->IsEqualTo(*HeadItr, Tolerance) || TheHeadSegment.AreContiguous(TheBackSegment))
                {
                // Two consecutive segments are contiguous ... we have a needle
                // We remove the shortest segment by deleting the middle point

                // Delete point
                m_Points.erase(MiddleItr);

                // Restart process from start
                if (m_Points.size() >= 3)
                    {
                    HeadItr = m_Points.begin();
                    MiddleItr = HeadItr;
                    ++HeadItr;

                    // Reset the back segment
                    TheBackSegment.SetStartPoint(*MiddleItr);
                    TheBackSegment.SetEndPoint(*HeadItr);
                    }

                }
            else
                {
                // The head segment becomes the back segment
                TheBackSegment = TheHeadSegment;
                }
            }
#endif

        // Check if the polysegment should be considered closed (and there at least 2 segments)
        if (pi_ClosedProcessing && (m_Points.size() >= 3))
            {
            // There still is the first and last segments to check
            HeadItr = m_Points.begin();
            HGF2DPositionCollection::iterator MiddleItr(HeadItr);
            ++HeadItr;
            TheHeadSegment.SetStartPoint(*MiddleItr);
            TheHeadSegment.SetEndPoint(*HeadItr);

            // Obtain
            HGF2DPositionCollection::reverse_iterator EndItr = m_Points.rbegin();
            HGF2DPositionCollection::reverse_iterator NextToEndItr(EndItr);
            ++NextToEndItr;
            TheBackSegment.SetStartPoint(*NextToEndItr);
            TheBackSegment.SetEndPoint(*MiddleItr);

#if (0)
            if (TheHeadSegment.AreContiguous(TheBackSegment))
                {
                // The first and last segments are contiguous ... we remove start/end point
                m_Points.erase(MiddleItr);
                m_Points.pop_back();

                // Check if next to end and second points (now first and last) are identical
                EndItr = m_Points.rbegin();
                HeadItr = m_Points.begin();
                if (EndItr->IsEqualTo(*HeadItr, Tolerance))
                    {
                    // The two points are equal (to a tolerance)
                    // Check if they are exactly equal ...
                    // NO TOLERANCE APPLICATION FOR THIS
                    if (*EndItr != *HeadItr)
                        {
                        // They are very close but not equal ... set last point
                        *EndItr = *HeadItr;
                        }
                    }
                else
                    {
                    // The start and end points are not equal ...
                    // we must force this fact
                    m_Points.push_back(*HeadItr);
                    }

                // Set start and end point
                m_StartPoint = HGF2DLocation(*(m_Points.begin()), GetCoordSys());
                m_EndPoint = m_StartPoint;
                ResetTolerance();
                }
#else
            while (m_Points.size() >= 3 && TheHeadSegment.AreContiguous(TheBackSegment))
                {
                // The first and last segments are contiguous ... we remove start/end point
                m_Points.erase(MiddleItr);
                m_Points.pop_back();

                // Check if next to end and second points (now first and last) are identical
                EndItr = m_Points.rbegin();
                HeadItr = m_Points.begin();
                if (EndItr->IsEqualTo(*HeadItr, Tolerance))
                    {
                    // The two points are equal (to a tolerance)
                    // Check if they are exactly equal ...
                    // NO TOLERANCE APPLICATION FOR THIS
                    if (*EndItr != *HeadItr)
                        {
                        // They are very close but not equal ... set last point
                        *EndItr = *HeadItr;
                        }
                    }
                else
                    {
                    // The start and end points are not equal ...
                    // we must force this fact
                    m_Points.push_back(*HeadItr);
                    }

                // Set start and end point
                m_StartPoint = HGF2DLocation(*(m_Points.begin()), GetCoordSys());
                m_EndPoint = m_StartPoint;
                ResetTolerance();

                if (m_Points.size() >= 3)
                    {
                    HeadItr = m_Points.begin();
                    MiddleItr = HeadItr;
                    ++HeadItr;
                    TheHeadSegment.SetStartPoint(*MiddleItr);
                    TheHeadSegment.SetEndPoint(*HeadItr);

                    EndItr = m_Points.rbegin();
                    NextToEndItr = EndItr;
                    ++NextToEndItr;
                    TheBackSegment.SetStartPoint(*NextToEndItr);
                    TheBackSegment.SetEndPoint(*MiddleItr);
                    }

                }
#endif
            }
        }

    }
#endif


//-----------------------------------------------------------------------------
// Reverse
// Reverses the linear
//-----------------------------------------------------------------------------
void HVE2DPolySegment::Reverse()
    {
    HINVARIANTS;

    HGF2DPositionCollection NewCollection = m_Points;

    // Clear old list
    m_Points.clear();

    // For every point of polysegment
    HGF2DPositionCollection::reverse_iterator Itr;
    for(Itr = NewCollection.rbegin() ; Itr != NewCollection.rend() ; ++Itr)
        {
        m_Points.push_back(*Itr);
        }

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

    // Declare answer variable
    bool Answer = false;

    // Pre-calculate tolerance
    double Tolerance = GetTolerance();

    double SelfXMin;
    double SelfXMax;
    double SelfYMin;
    double SelfYMax;

    bool Result;

    // Check that there are at least 4 points (otherwise autocrossing is impossible)
    if (m_Points.size() >= 4)
        {

        // For every segment of the polysegment
        HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
        HGF2DPositionCollection::const_iterator PreviousItr(Itr);
        ++Itr;
        for (; !Answer && Itr != m_Points.end() ; ++Itr , ++PreviousItr)
            {
            // Obtain extent of this segment
            SelfXMin = MIN(Itr->GetX(), PreviousItr->GetX());
            SelfXMax = MAX(Itr->GetX(), PreviousItr->GetX());
            SelfYMin = MIN(Itr->GetY(), PreviousItr->GetY());
            SelfYMax = MAX(Itr->GetY(), PreviousItr->GetY());

            // Create a lite segment to represent current segment
            HGF2DLiteSegment TheLiteSegment(*PreviousItr, *Itr, Tolerance);

            // For every other segment of polysegment
            HGF2DPositionCollection::const_iterator OtherItr = Itr;
            HGF2DPositionCollection::const_iterator PreviousOtherItr(OtherItr);
            ++OtherItr;
            for (; !Answer && OtherItr != m_Points.end() ; ++OtherItr , ++PreviousOtherItr)
                {
                // Check that it is not the same segment
                if (OtherItr != Itr)
                    {
                    // Check if current self segment and current other segment may interact
                    Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, MIN(OtherItr->GetX(), PreviousOtherItr->GetX()), Tolerance) &&
                              HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, MAX(OtherItr->GetX(), PreviousOtherItr->GetX()), Tolerance) &&
                              HDOUBLE_GREATER_OR_EQUAL(SelfYMax, MIN(OtherItr->GetY(), PreviousOtherItr->GetY()), Tolerance) &&
                              HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, MAX(OtherItr->GetY(), PreviousOtherItr->GetY()), Tolerance)
                             );


                    if (Result)
                        {
                        // The two segments may interact ... check if they cross
                        Answer = HGF2DLiteSegment(*PreviousOtherItr,
                                                  *OtherItr,
                                                  Tolerance).Crosses(TheLiteSegment);
                        }
                    }
                }
            }
        }

    // Check if they did cross ...
    if (!Answer)
        {

        // They did not cross ... this does not mean they do not cross...
        // We have to check for special crossings at link points

        // The following applies only if there are at least 5 points in polysegment
        if (m_Points.size() >= 5)
            {
            // For every triplets of points
            HGF2DPositionCollection::const_iterator FirstItr(m_Points.begin());
            HGF2DPositionCollection::const_iterator SecondItr(FirstItr);
            HGF2DPositionCollection::const_iterator ThirdItr(FirstItr);
            ++SecondItr;
            ++ThirdItr;
            ++ThirdItr;

            HGF2DPositionCollection::const_iterator FourthItr(ThirdItr);
            HGF2DPositionCollection::const_iterator FifthItr(ThirdItr);
            HGF2DPositionCollection::const_iterator SixthItr(ThirdItr);
            ++FifthItr;
            ++SixthItr;
            ++SixthItr;

            // Create a small polygon of segments with two segments.
            HVE2DPolySegment FirstPolySegment(GetCoordSys());
            FirstPolySegment.AppendPosition(*FirstItr);
            FirstPolySegment.AppendPosition(*SecondItr);

            HVE2DPolySegment SecondPolySegment(GetCoordSys());
            HGF2DPositionCollection::const_iterator TempItr(FourthItr);
            for (; TempItr != m_Points.end() ; ++TempItr)
                {
                SecondPolySegment.AppendPosition(*TempItr);
                }

            for (; !Answer && SixthItr != m_Points.end() ; ++FirstItr , ++SecondItr, ++ThirdItr, ++FourthItr, ++FifthItr, ++SixthItr)
                {
                // Add new point to First polysegment
                FirstPolySegment.AppendPosition(*ThirdItr);

                HGF2DLocation SplitPointA(*SecondItr, GetCoordSys());
                HGF2DLocation NextToSplitPointA(*ThirdItr, GetCoordSys());

                HGF2DLocation SplitPointB(*FifthItr, GetCoordSys());
                HGF2DLocation NextToSplitPointB(*SixthItr, GetCoordSys());


                // Ask if these cross at split (second) point
                Answer = FirstPolySegment.IntersectsAtSplitPointWithPolySegment(SecondPolySegment,
                                                                                SplitPointA,
                                                                                NextToSplitPointA,
                                                                                true);

                if (!Answer)
                    {
                    Answer = SecondPolySegment.IntersectsAtSplitPointWithPolySegment(FirstPolySegment,
                                                                                     SplitPointB,
                                                                                     NextToSplitPointB,
                                                                                     true);

                    }

                // Remove first point of second polysegment
                SecondPolySegment.RemovePoint(0);
                }
            }
        }

    return(Answer);
    }



//-----------------------------------------------------------------------------
// IsAutoContiguous
// Indicates if the linear is contiguous to itself
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::IsAutoContiguous() const
    {
    HINVARIANTS;

    // Declare answer variable
    bool Answer = false;

    // Pre-calculate tolerance
    double Tolerance = GetTolerance();

    double SelfXMin;
    double SelfXMax;
    double SelfYMin;
    double SelfYMax;

    bool Result;

    // Check that there are at least 4 points (otherwise autocrossing is impossible)
    if (m_Points.size() >= 4)
        {

        // For every segment of the polysegment
        HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
        HGF2DPositionCollection::const_iterator PreviousItr(Itr);
        ++Itr;
        for (; !Answer && Itr != m_Points.end() ; ++Itr , ++PreviousItr)
            {
            // Obtain extent of this segment
            SelfXMin = MIN(Itr->GetX(), PreviousItr->GetX());
            SelfXMax = MAX(Itr->GetX(), PreviousItr->GetX());
            SelfYMin = MIN(Itr->GetY(), PreviousItr->GetY());
            SelfYMax = MAX(Itr->GetY(), PreviousItr->GetY());

            // Create a lite segment to represent current segment
            HGF2DLiteSegment TheLiteSegment(*PreviousItr, *Itr, Tolerance);

            // For every other segment of polysegment
            HGF2DPositionCollection::const_iterator OtherItr(m_Points.begin());
            HGF2DPositionCollection::const_iterator PreviousOtherItr(OtherItr);
            ++OtherItr;
            for (; !Answer && OtherItr != m_Points.end() ; ++OtherItr , ++PreviousOtherItr)
                {
                // Check that it is not the same segment
                if (OtherItr != Itr)
                    {
                    // Check if current self segment and current other segment may interact
                    Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, MIN(OtherItr->GetX(), PreviousOtherItr->GetX()), Tolerance) &&
                              HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, MAX(OtherItr->GetX(), PreviousOtherItr->GetX()), Tolerance) &&
                              HDOUBLE_GREATER_OR_EQUAL(SelfYMax, MIN(OtherItr->GetY(), PreviousOtherItr->GetY()), Tolerance) &&
                              HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, MAX(OtherItr->GetY(), PreviousOtherItr->GetY()), Tolerance)
                             );


                    if (Result)
                        {
                        // The two segments may interact ... check if they cross
                        Answer = HGF2DLiteSegment(*PreviousOtherItr,
                                                  *OtherItr,
                                                  Tolerance).AreContiguous(TheLiteSegment);
                        }
                    }
                }
            }
        }

    return(Answer);
    }


//-----------------------------------------------------------------------------
// Constructor (by list of positions)
//-----------------------------------------------------------------------------
HVE2DPolySegment::HVE2DPolySegment(const HGF2DPositionCollection& pi_rListOfPoints,
                                   const HFCPtr<HGF2DCoordSys>&   pi_rpCoordSys)
    : HVE2DBasicLinear(pi_rpCoordSys),
      m_Points(pi_rListOfPoints),
      m_ExtentUpToDate(false),
      m_Extent(pi_rpCoordSys)
    {
    // The list of points must contain 2 or more points
    // 0 or 1 point is illegal
    HPRECONDITION(pi_rListOfPoints.size() >= 2);

    // We set start point
    m_StartPoint = HGF2DLocation(*(pi_rListOfPoints.begin()), pi_rpCoordSys);

    // Set end point
    m_EndPoint = HGF2DLocation(*(pi_rListOfPoints.rbegin()), pi_rpCoordSys);

    // Reset tolerance
    ResetTolerance();

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

    // We set start point
    m_StartPoint = *(pi_rListOfPoints.begin());

    // Set end point
    m_EndPoint = *(pi_rListOfPoints.rbegin());
    m_EndPoint.ChangeCoordSys(GetCoordSys());   // Just make sure...

    // We copy the locations to the position collection
    HGF2DLocationCollection::const_iterator Itr(pi_rListOfPoints.begin());

    for(; Itr != pi_rListOfPoints.end() ; ++Itr)
        {
        // Obtain point expressed in current coordinate system
        HGF2DLocation ThePoint(*Itr, GetCoordSys());

        // Add to list of positions
        m_Points.push_back(HGF2DPosition(ThePoint.GetX(), ThePoint.GetY()));
        }

    // Reset tolerance
    ResetTolerance();

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

    // Preallocate points
    m_Points.reserve(pi_BufferLength / 2);

    // Copy points
    for (size_t Index = 0 ; Index < pi_BufferLength ; Index += 2)
        {
        m_Points.push_back(HGF2DPosition(pi_aBuffer[Index], pi_aBuffer[Index + 1]));
        }

    // Set start and end points
    m_StartPoint = HGF2DLocation(m_Points.begin()->GetX(),
                                 m_Points.begin()->GetY(),
                                 pi_rpCoordSys);
    m_EndPoint = HGF2DLocation(m_Points.rbegin()->GetX(),
                               m_Points.rbegin()->GetY(),
                               pi_rpCoordSys);

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
    HPRECONDITION(m_Points.size() != 0);

    // The point must be located on polysegment
    HASSERTSUPERDEBUG(IsPointOn(pi_rPoint));

    // Declare a variable for the answer
    HGFBearing TheBearing;

    // Stop loop variable
    bool Found = false;

    // Make sure point is expressed in polysegment coordinate system
    HGF2DLocation ThePoint(pi_rPoint, GetCoordSys());

    // Transform into a position
    HGF2DPosition ThePosition(pi_rPoint.GetX(), pi_rPoint.GetY());

    // Extract raw values
    double X = ThePosition.GetX();
    double Y = ThePosition.GetY();

    // We must first find out the first segment which contains
    // the point

    // For every segment of polysegment
    HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
    HGF2DPositionCollection::const_iterator PreviousItr(Itr);
    ++Itr;

    for (; !Found && Itr != m_Points.end() ; ++Itr , ++PreviousItr)
        {
        // Check if current segment and point may interact
        bool Result = (HDOUBLE_GREATER_OR_EQUAL(X, MIN(Itr->GetX(), PreviousItr->GetX()), GetTolerance()) &&
                        HDOUBLE_SMALLER_OR_EQUAL(X, MAX(Itr->GetX(), PreviousItr->GetX()), GetTolerance()) &&
                        HDOUBLE_GREATER_OR_EQUAL(Y, MIN(Itr->GetY(), PreviousItr->GetY()), GetTolerance()) &&
                        HDOUBLE_SMALLER_OR_EQUAL(Y, MAX(Itr->GetY(), PreviousItr->GetY()), GetTolerance())
                       );

        if (Result)
            {
            // point and segment are in the same general area

            // Create segment
            HGF2DLiteSegment TheSegment(*PreviousItr, *Itr, GetTolerance());

            // Check if point is on this segment
            if (TheSegment.IsPointOn(ThePosition))
                {
                // The point is on this segment ... obtain bearing
                // of the segment

                // Check if the point is on end point and BETA direction desired
                if ((pi_Direction == HVE2DVector::BETA) && ThePosition.IsEqualTo(*Itr, GetTolerance()))
                    {
                    // Since we want BETA direction of end point of current component,
                    // We really want the BETA direction of next component start point instead
                    HGF2DPositionCollection::const_iterator OtherIterator(Itr);

                    // Advance iterator one component
                    ++OtherIterator;

                    // If the preceeding component was not the last one
                    if (OtherIterator != m_Points.end())
                        {
                        // Make sure new current is not null
                        while (OtherIterator != m_Points.end() && ((*Itr) == (*OtherIterator)))
                            ++OtherIterator;

                        if (OtherIterator != m_Points.end())
                            {

                            // Current iterator is now next one
                            Itr = OtherIterator;
                            PreviousItr = Itr;
                            --PreviousItr;
                            }
                        }
                    }

                // First we need locations of start and end points
                HGF2DLocation StartPoint(*PreviousItr, GetCoordSys());
                HGF2DLocation EndPoint(*Itr, GetCoordSys());

                // The bearing depends on the arbitrary direction
                // desired
                if (pi_Direction == HVE2DVector::BETA)
                    {
                    // From start to end point
                    TheBearing = (EndPoint - StartPoint).CalculateBearing();
                    }
                else
                    {
                    // From end to start point
                    TheBearing = (StartPoint - EndPoint).CalculateBearing();
                    }

                // Indicate the bearing has been found
                Found = true;
                }
            }
        }

    // The segment must have been found
    HASSERT(Found);

    return(TheBearing);
    }

//-----------------------------------------------------------------------------
// GetExtent
// Returns the extent of the polysegment
//-----------------------------------------------------------------------------
inline HGF2DExtent HVE2DPolySegment::GetExtent() const
    {
    HINVARIANTS;

    // Check if extent is calculated and up to date
    if (!m_ExtentUpToDate)
        {
        // The extent is not up to date ...

        // Reset extent to undefined
        m_Extent = HGF2DExtent(GetCoordSys());

        // For every point of polysegment
        HGF2DPositionCollection::const_iterator Itr;
        for(Itr = m_Points.begin() ; Itr != m_Points.end() ; ++Itr)
            {
            // Augment extent by point
            m_Extent.Add(HGF2DLocation(*Itr, GetCoordSys()));
            }

        // Indicate extent is up to date
        m_ExtentUpToDate = true;
        }

    return(m_Extent);
    }

//-----------------------------------------------------------------------------
// Move
// This method moves the polysegment by the specified displacement
//-----------------------------------------------------------------------------
void HVE2DPolySegment::Move(const HGF2DDisplacement& pi_rDisplacement)
    {
    HINVARIANTS;

    // Check if auto tolerance is active
    if (IsAutoToleranceActive())
        {
        double Tolerance = HGLOBAL_EPSILON;

        // For every part ... merge extents
        for(size_t Index = 0 ; Index < m_Points.size()  ; Index++)
            {
            m_Points[Index][HGF2DPosition::X] += pi_rDisplacement.GetDeltaX();
            m_Points[Index][HGF2DPosition::Y] += pi_rDisplacement.GetDeltaY();

            // Adjust tolerance
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(m_Points[Index][HGF2DPosition::X]));
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(m_Points[Index][HGF2DPosition::Y]));
            }

        SetTolerance(Tolerance);
        }
    else
        {
        // For every part ... merge extents
        for(size_t Index = 0 ; Index < m_Points.size()  ; Index++)
            {
            m_Points[Index][HGF2DPosition::X] += pi_rDisplacement.GetDeltaX();
            m_Points[Index][HGF2DPosition::Y] += pi_rDisplacement.GetDeltaY();
            }
        }

    m_ExtentUpToDate = false;

    // Call ancestor to update extremities
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

    // Obtain scale origin in self coordinate system
    HGF2DLocation   ScaleOrigin(pi_rScaleOrigin, GetCoordSys());
    double ScaleOriginX = ScaleOrigin.GetX();
    double ScaleOriginY = ScaleOrigin.GetY();

    // Depending if auto tolerance is active
    if (IsAutoToleranceActive())
        {
        double Tolerance = HGLOBAL_EPSILON;

        // For every part ... scale
        for(size_t Index = 0 ; Index < m_Points.size() ; Index++)
            {
            m_Points[Index][HGF2DPosition::X] = ScaleOriginX + (pi_ScaleFactor * (m_Points[Index][HGF2DPosition::X] - ScaleOriginX)); 
            m_Points[Index][HGF2DPosition::Y] = ScaleOriginY + (pi_ScaleFactor * (m_Points[Index][HGF2DPosition::Y] - ScaleOriginY));

            // Adjust tolerance
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(m_Points[Index][HGF2DPosition::X]));
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(m_Points[Index][HGF2DPosition::Y]));
            }

        SetTolerance(Tolerance);
        }
    else
        {
        // For every part ... scale
        for(size_t Index = 0 ; Index < m_Points.size() ; Index++)
            {
            m_Points[Index][HGF2DPosition::X] =  ScaleOriginX + (pi_ScaleFactor * (m_Points[Index][HGF2DPosition::X] - ScaleOriginX)); 
            m_Points[Index][HGF2DPosition::Y] =  ScaleOriginY + (pi_ScaleFactor * (m_Points[Index][HGF2DPosition::Y] - ScaleOriginY)); 
            }
        }

    m_ExtentUpToDate = false;

    // Call ancester to update extremities
    HVE2DLinear::Scale(pi_ScaleFactor, pi_rScaleOrigin);

    HINVARIANTS;

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
    if (m_Points.size() == 0)
        {
        // It is the first point ... we must set start point
        m_StartPoint = ThePoint;
        }

    // Add position
    m_Points.push_back(ThePoint.GetPosition());

    // In all case, this point becomes the end point
    m_EndPoint = ThePoint;

    // Indicate extent in no more up to date
    m_ExtentUpToDate = false;

    // Calculate tolerance
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
    if (m_Points.size() == 0)
        {
        // It is the first point ... we must set start point
        m_StartPoint = HGF2DLocation(pi_rNewPoint, GetCoordSys());
        }

    // Add position
    m_Points.push_back(pi_rNewPoint);

    // In all case, this point becomes the end point
    m_EndPoint = HGF2DLocation(pi_rNewPoint, GetCoordSys());

    // Indicate extent in no more up to date
    m_ExtentUpToDate = false;

    // Calculate tolerance
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
    HPRECONDITION((pi_Index >= 0) && (pi_Index < m_Points.size()));

    // Obtain the iterator for designated position
    HGF2DPositionCollection::iterator Itr = m_Points.begin();
    size_t Counter = 0;
    while (Counter < pi_Index)
        {
        ++Itr;
        ++Counter;
        }

    // Erase point
    m_Points.erase(Itr);

    if(!m_Points.empty()) 
        {
        // Reset start and end point if required
        if (pi_Index == 0)
            {
            m_StartPoint = HGF2DLocation(m_Points.front(), GetCoordSys());
            }
        if (pi_Index == m_Points.size())
            {
            m_EndPoint = HGF2DLocation(m_Points.back(), GetCoordSys());
            }
        }

    // Indicate extent in no more up to date
    m_ExtentUpToDate = false;

    // Calculate tolerance
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

    // Create a similitude
    HGF2DSimilitude Similitude;

    // Set rotation
    Similitude.AddRotation(pi_Angle, Origin.GetX(), Origin.GetY());

    // Transform coordinates of start point
    double NewX = m_StartPoint.GetX();
    double NewY = m_StartPoint.GetY();
    Similitude.ConvertDirect(&NewX, &NewY);

    // Set new start point
    m_StartPoint.SetX(NewX);
    m_StartPoint.SetY(NewY);

    // Transform coordinates of end point
    NewX = m_EndPoint.GetX();
    NewY = m_EndPoint.GetY();
    Similitude.ConvertDirect(&NewX, &NewY);

    // Set new end point
    m_EndPoint.SetX(NewX);
    m_EndPoint.SetY(NewY);

    // For every point in list...
    HGF2DPositionCollection::iterator Itr;

    for (Itr = m_Points.begin() ; Itr != m_Points.end() ; ++Itr)
        {
        // Extract values
        NewX = Itr->GetX();
        NewY = Itr->GetY();

        // Apply rotation
        Similitude.ConvertDirect(&NewX, &NewY);

        // Set new values
        Itr->SetX(NewX);
        Itr->SetY(NewY);
        }

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
    if (m_Points.size() >= 2)
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
    if (m_Points.size() >= 2)
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
    if (m_Points.size() >= 2)
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
        // Check if this model between coordinate systems is linearity preserving
        if (GetCoordSys()->HasLinearityPreservingRelationTo(pi_rpCoordSys))
            {
            // The model preserves linearity ... we can transform the points directly

            // We will compute the appropriate tolerance on the fly (default is global epsilon)
            double Tolerance = HGLOBAL_EPSILON;

            // Create recipient polysegment
            HAutoPtr<HVE2DPolySegment> pMyResultPolySegment(new HVE2DPolySegment(pi_rpCoordSys));

            // For all points ...
            HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
            while (Itr != m_Points.end())
                {
                // Transform position to a location
                HGF2DLocation Point(*Itr, GetCoordSys());
                ++Itr;

                // Transform point
                Point.ChangeCoordSys(pi_rpCoordSys);

                Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(Point.GetX()));
                Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(Point.GetY()));

                // HChk MR
                // We apply the currently calculated Tolerance when comparing two
                // positions together. This is better than using the default Epsilon,
                // but I'm not sure that it's OK.

                // Add point to new PolySegment IFF it is not a double point
                if (pMyResultPolySegment->m_Points.size() == 0)
                    {
                    pMyResultPolySegment->m_Points.push_back(Point.GetPosition());
                    }
                else
                    {
                    if (Point.GetPosition().IsEqualTo(*pMyResultPolySegment->m_Points.rbegin(), Tolerance))
                        {
                        if (Itr == m_Points.end())
                            {
                            // Special case: Points are equal, but we can't remove the last
                            // point. Therefore, we remove the one just before.
                            pMyResultPolySegment->m_Points.pop_back();
                            pMyResultPolySegment->m_Points.push_back(Point.GetPosition());
                            }
                        }
                    else
                        {
                        pMyResultPolySegment->m_Points.push_back(Point.GetPosition());
                        }
                    }
                }

            // Transform start and end points
            pMyResultPolySegment->m_StartPoint = m_StartPoint.ExpressedIn(pi_rpCoordSys);
            pMyResultPolySegment->m_EndPoint   = m_EndPoint.ExpressedIn(pi_rpCoordSys);

            // Reset tolerance of new polysegment
            pMyResultPolySegment->SetTolerance(MIN(HMAX_EPSILON, Tolerance));

            // Activate auto tolerance determination
            pMyResultPolySegment->SetAutoToleranceActive(true);

            // Move result polysegment to return variable
            pMyResultVector.reset(pMyResultPolySegment.release());
            }
        else
            {
            // The model does not preserve linearity
            // We process more completely
            pMyResultVector.reset(AllocateCopyInComplexCoordSys(pi_rpCoordSys));
            }

        pMyResultVector->SetStrokeTolerance(m_pStrokeTolerance);
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

    // Create recipient location with initialization flag
    bool           ClosestPointInitialized = false;
    HGF2DLocation   ClosestPoint(GetCoordSys());

    // Check if self is not NULL
    if (m_Points.size() >= 2)
        {
        // Create a segment ... this segment will be used for all
        // operations thus preventing multiple creation of segments
        HVE2DSegment TheSegment(GetCoordSys());

        // For every segment part of the polysegment
        HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
        HGF2DPositionCollection::const_iterator PreviousItr(Itr);
        ++Itr;

        for (; Itr != m_Points.end() ; ++Itr , ++PreviousItr)
            {
            // Set segment to represent current segment
            // The coordinate system of segment is already the proper one.
            TheSegment.SetRawStartPoint(PreviousItr->GetX(), PreviousItr->GetY());
            TheSegment.SetRawEndPoint(Itr->GetX(), Itr->GetY());

            // Calculate closest point to the segment
            HGF2DLocation TempClosestPoint = TheSegment.CalculateClosestPoint(pi_rPoint);

            // Check if new closest point is closer than before (or first)
            if ((!ClosestPointInitialized) ||
                ((pi_rPoint - TempClosestPoint).CalculateLength() < (pi_rPoint - ClosestPoint).CalculateLength()))
                {
                // The new point is closer (or first) ... retain value
                ClosestPoint = TempClosestPoint;

                // Inidicate closest point was initialized
                ClosestPointInitialized = true;
                }
            }
        }

    return(ClosestPoint);
    }


//-----------------------------------------------------------------------------
// CalculateLength
// Calculates and returns the length of polysegment
//-----------------------------------------------------------------------------
double HVE2DPolySegment::CalculateLength() const
    {
    HINVARIANTS;

    // Recipient variable
    double PolySegmentLength = 0.0;

    // Check if there are at least two points in segment
    if (m_Points.size() >= 2)
        {
        // For every point pair in polysegment...
        HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
        HGF2DPositionCollection::const_iterator PreviousItr(Itr);
        ++Itr;
        for ( ; Itr != m_Points.end() ; ++Itr, ++PreviousItr)
            {
            // Evaluate Delta X
            double DeltaX(Itr->GetX() - PreviousItr->GetX());

            // Evaluate Delta Y
            // Note that distance unit conversion is performed since Y units
            // may be different from X unit in which answer is evaluated
            double DeltaY(Itr->GetY() - PreviousItr->GetY());

            // Obtain length of this segment and add to total length
            PolySegmentLength += sqrt((DeltaX * DeltaX) + (DeltaY * DeltaY));
            }
        }

    return(PolySegmentLength);
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

//HChk AR ... Should we impose this condition??????
    // The linear must not be NULL
//    HPRECONDITION(CalculateLength() > 0.0);

    // Declare result variable
    HGF2DLocation   ResultPoint(GetCoordSys());

    // Check if poly segment is not empty
    if (m_Points.size() >= 2)
        {

        // Check for extremity relative position
        // These are needed for extreme precision
        // NOTE: In the following the EXACT compare operation is voluntary
        if (pi_RelativePos == 0.0)
            {
            // Start point is asked for
            ResultPoint = m_StartPoint;
            }
        // NOTE: In the following the EXACT compare operation is voluntary
        else if (pi_RelativePos == 1.0)
            {
            // End point is asked for
            ResultPoint = m_EndPoint;
            }
        else
            {
            // General case ...

            // Obtain the desired relative length
            double  MyPosLength = (CalculateLength() * pi_RelativePos);

            // Loop till the segment on which the point is is found ...
            // It is found when the relative length is exceeded (pos length is negative)
            // For every point pair in polysegment...
            HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
            HGF2DPositionCollection::const_iterator PreviousItr(Itr);
            ++Itr;
            for ( ; (Itr != m_Points.end() && MyPosLength > 0.0) ; ++Itr, ++PreviousItr)
                {
                // Evaluate Delta X
                double DeltaX(Itr->GetX() - PreviousItr->GetX());

                // Evaluate Delta Y
                // Note that distance unit conversion is performed since Y units
                // may be different from X unit in which answer is evaluated
                double DeltaY = (Itr->GetY() - PreviousItr->GetY());

                // Remove the segment length
                MyPosLength -= sqrt((DeltaX * DeltaX) + (DeltaY * DeltaY));
                }

            // Since the loop may leave with a EPSILON sized positive distance,
            // due to caculation errors when at last point, we truncate.
            // Check if all segments have been processed
            if (Itr == m_Points.end() && (MyPosLength >= 0.0))
                {
                // We are indeed pass the end of polysegment
                // there has been calculation errors ... adjust
                // Backtrack
                --PreviousItr;
                --Itr;

                // Set positional length to 0.0
                MyPosLength = 0.0;

                }
            else
                {
                // In the other case, we have bypassed the proper segment
                // Backtrack
                --PreviousItr;
                --Itr;
                }

            // The component segment is found ... we complete operation

            // First create necessary points
            HGF2DLocation SegmentStartPoint((*PreviousItr), GetCoordSys());
            HGF2DLocation SegmentEndPoint((*Itr), GetCoordSys());

            // Precalculate displacement
            HGF2DDisplacement SegmentDisplacement(SegmentEndPoint - SegmentStartPoint);

            // Declare adjusted relative position variable
            double SegmentRelativePos;

            // We check if it is the first and only segment
            if ((m_Points.size() == 2))
                {
                // It is the first segment ... to increase precision we use the relative position
                // provided as parameter
                SegmentRelativePos = pi_RelativePos;
                }
            else
                {
                // with adjusted relative position
                // Calculate relative position within segment (what remains)
                SegmentRelativePos = (1.0 + (MyPosLength / SegmentDisplacement.CalculateLength()));
                }

            // With the adjusted relative position ... calculate point
            ResultPoint = SegmentStartPoint + (SegmentRelativePos * SegmentDisplacement);
            }
        }

    return(ResultPoint);
    }

//-----------------------------------------------------------------------------
// CalculateRelativePosition
// Calculates and returns the relative position of given location on polysegment.
//-----------------------------------------------------------------------------
double HVE2DPolySegment::CalculateRelativePosition(const HGF2DLocation& pi_rPoint) const
    {
    HINVARIANTS;

    // The given point must be located on polysegment
    HASSERTSUPERDEBUG(IsPointOn(pi_rPoint));

    // The polysegment must not be null
    HPRECONDITION(CalculateLength() > 0.0);

    // Obtain the total length
    double MyTotalLength(CalculateLength());

    // Declare a positional distance (in the same units as total length)
    double MyPosLength = 0.0;

    // Obtain point is self coordinate system
    HGF2DLocation ThePoint(pi_rPoint, GetCoordSys());

    // Extract raw values of point
    double X = ThePoint.GetX();
    double Y = ThePoint.GetY();

    // Declare loop stop variable
    bool PointOnFound = false;

    // Loop till the segment on which the point is located (ON) is found
    // We must check for every component segment
    HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
    HGF2DPositionCollection::const_iterator PreviousItr(Itr);
    ++Itr;
    for (; !PointOnFound && Itr != m_Points.end() ; ++Itr , ++PreviousItr)
        {
        // We must now check if point is located ON

        // Check if current segment and segment may interact
        bool Result = (HDOUBLE_GREATER_OR_EQUAL(X, MIN(Itr->GetX(), PreviousItr->GetX()), GetTolerance()) &&
                        HDOUBLE_SMALLER_OR_EQUAL(X, MAX(Itr->GetX(), PreviousItr->GetX()), GetTolerance()) &&
                        HDOUBLE_GREATER_OR_EQUAL(Y, MIN(Itr->GetY(), PreviousItr->GetY()), GetTolerance()) &&
                        HDOUBLE_SMALLER_OR_EQUAL(Y, MAX(Itr->GetY(), PreviousItr->GetY()), GetTolerance())
                       );


        if (Result)
            {
            // Current segment and given polysegment may interact ...

            // Create a lite segment to represent current segment
            HGF2DLiteSegment TheLiteSegment(*PreviousItr, *Itr, GetTolerance());

            // Check if point is on segment
            // Note that tolerance is part of segment
            PointOnFound = TheLiteSegment.IsPointOn(HGF2DPosition(X, Y));
            }

        // Calculate length of segment

        // Evaluate Delta X
        double DeltaX(Itr->GetX() - PreviousItr->GetX());

        // Evaluate Delta Y
        // Note that distance unit conversion is performed since Y units
        // may be different from X unit in which answer is evaluated
        double DeltaY = (Itr->GetY() - PreviousItr->GetY());

        // Obtain length of segment
        double SegmentLength = sqrt((DeltaX * DeltaX) + (DeltaY * DeltaY));

        // If this was not the segment on which point is ...
        // We add length to total length
        if (!PointOnFound)
            {
            // Increment current positional length by whole segment length
            MyPosLength += SegmentLength;
            }
        else
            {
            // If we are on segment point is on we add part of length only

            // The part is the relative position on segment
            // Since computations are difficult we rely on segment
            // for calculation
            HVE2DSegment TheSegment(HGF2DLocation((*PreviousItr), GetCoordSys()),
                                    HGF2DLocation((*Itr), GetCoordSys()));

            MyPosLength += SegmentLength * TheSegment.CalculateRelativePosition(ThePoint);
            }
        }

    // There must have been a segment with point ON
    HASSERT(PointOnFound);
    
    // Return relative position (ratio of lengths)
    double result = (MyPosLength / MyTotalLength);
    return (result <= 1.0 ? result : 1.0);
}

//-----------------------------------------------------------------------------
// CalculateRayArea
// Calculates and returns the area of the ray extended from given point to
// the given polysegment
//-----------------------------------------------------------------------------
double HVE2DPolySegment::CalculateRayArea(const HGF2DLocation& pi_rPoint) const
    {
    HINVARIANTS;

    // Create recipient area in the X dimension units squared
    double         MyTotalArea = 0.0;

    // Check that there are at least two points
    if (m_Points.size() >= 2)
        {
        // Create changeable ray point in self coordinate system
        HGF2DLocation   MyPoint(pi_rPoint, GetCoordSys());

        // For every segment of polysegment we calculate ray area
        HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
        HGF2DPositionCollection::const_iterator PreviousItr(Itr);
        ++Itr;
        for (; Itr != m_Points.end() ; ++Itr , ++PreviousItr)
            {
            // Create needed locations
            HGF2DLocation SegmentStartPoint((*PreviousItr), GetCoordSys());
            HGF2DLocation SegmentEndPoint((*Itr), GetCoordSys());

            // Calculate ray area for this segment
            MyTotalArea += ((SegmentStartPoint.GetX() * ((SegmentEndPoint - MyPoint).GetDeltaY())) / 2);

            // Change ray point to start of current segment
            MyPoint = SegmentStartPoint;
            }
        }

    return (MyTotalArea);
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

    // Nothing to do as whole span is preserved.
    if (0.0 >= startRelativePos && 1.0 <= endRelativePos)
        return;
    
    // Cannot continue if this condition happens...
    if (startRelativePos > endRelativePos)
        throw HFCUndefinedException(); 

    // The relative positions given must be between 0.0 and 1.0
    HPRECONDITION(HDOUBLE_GREATER_OR_EQUAL(startRelativePos, 0.0, GetTolerance()) && 
                  HDOUBLE_SMALLER_OR_EQUAL(startRelativePos, 1.0, GetTolerance()));
    HPRECONDITION(HDOUBLE_GREATER_OR_EQUAL(endRelativePos, 0.0, GetTolerance()) && 
                  HDOUBLE_SMALLER_OR_EQUAL(endRelativePos, 1.0, GetTolerance()));

    // The polysegment must not be null length or empty
    HPRECONDITION(CalculateLength() > 0.0);

    // Obtain the total length
    const double totalLength(CalculateLength());

    // Obtain length to new start point
    const double startLength(totalLength * startRelativePos);

    // Obtain length to new end point
    const double endLength(totalLength * endRelativePos);

    
    // Start our search for start position's segment at begining of the range.
    double lengthToStartSegmentEndPoint = 0.0;
    double startSegmentLength = 0.0;

    const HGF2DPositionCollection::iterator startPosSegmentIt 
        = adjacent_find(m_Points.begin(), m_Points.end(), 
                        FindSegmentContainingPosition(startSegmentLength, lengthToStartSegmentEndPoint, startLength));


    // Start our search for end position's segment where search for start position's segment left.
    double lengthToEndSegmentEndPoint 
        = (m_Points.end() != startPosSegmentIt ? (lengthToStartSegmentEndPoint - startSegmentLength) : lengthToStartSegmentEndPoint);
    double endSegmentLength = 0.0;

    const HGF2DPositionCollection::iterator endPosSegmentIt 
        = adjacent_find(startPosSegmentIt, m_Points.end(), 
                        FindSegmentContainingPosition(endSegmentLength, lengthToEndSegmentEndPoint, endLength));

    
    if (startPosSegmentIt == endPosSegmentIt && m_Points.end() != startPosSegmentIt)
        {
        // NTERAY:  Not sure if we really need this special case as the "else" case should 
        //          under my assumption perform the same job. Was it an optimization?
        // Start and end are on the same segment

        HGF2DLocation startPoint(startPosSegmentIt[0], GetCoordSys());
        HGF2DLocation endPoint(startPosSegmentIt[1], GetCoordSys());
        
        // Evaluate the displacement vector from start to end of segment
        const HGF2DDisplacement segmentDisplacement(endPoint - startPoint);
        
        HASSERT(startSegmentLength == endSegmentLength);
        HASSERT(lengthToStartSegmentEndPoint <= lengthToEndSegmentEndPoint);

        // TODO: There should be a case to handle this as if it happens, there will a a division by 0.0 hard-error.
        HASSERT(0.0 <= startSegmentLength && 0.0 <= endSegmentLength);

        // Compute the new end point
        endPoint = startPoint + 
                   segmentDisplacement * ((endLength - lengthToEndSegmentEndPoint + endSegmentLength) / 
                               startSegmentLength);
                               
        // Compute the new start point
        startPoint += segmentDisplacement * ((startLength - lengthToStartSegmentEndPoint + startSegmentLength) / 
                               startSegmentLength);
                               
        // We may now discard all points
        m_Points.clear();
        
        // We add the only segment remaining
        m_Points.push_back(startPoint.GetPosition());
        m_Points.push_back(endPoint.GetPosition());
        }
    else 
        {
        if (m_Points.end() != endPosSegmentIt)
            {
            const HGF2DPositionCollection::iterator newEnd = endPosSegmentIt + 2;

            // This condition should be met as adjacent_find would have returned end() otherwise.
            // Do not take any chances...
            HASSERT(m_Points.end() >= newEnd); 
            // TODO: There should be a case to handle this as if it happens, there will a a division by 0.0 hard-error.
            HASSERT(0.0 <= endSegmentLength);

            // Compute the relative position on this segment
            const double segmentRelPos = (endLength - lengthToEndSegmentEndPoint + endSegmentLength) / 
                                            endSegmentLength;        
            
            // Calculate deltas for segment
            const double deltaX = (endPosSegmentIt[1].GetX() - endPosSegmentIt[0].GetX());
            const double deltaY = (endPosSegmentIt[1].GetY() - endPosSegmentIt[0].GetY());

            // We modify the end point
            endPosSegmentIt[1].SetX(endPosSegmentIt[0].GetX() + (deltaX * segmentRelPos));
            endPosSegmentIt[1].SetY(endPosSegmentIt[0].GetY() + (deltaY * segmentRelPos));

            // Better to check this again than crash...
            if (m_Points.end() >= newEnd) 
                m_Points.erase(newEnd, m_Points.end());
            }

        if (m_Points.end() != startPosSegmentIt)
            {
            // TODO: There should be a case to handle this as if it happens, there will a a division by 0.0 hard-error.
            HASSERT(0.0 <= startSegmentLength);

            // Compute the relative position on this segment
            const double segmentRelPos = (startLength - lengthToStartSegmentEndPoint + startSegmentLength) / 
                                            startSegmentLength;        
        
            // Calculate deltas for segment
            const double deltaX(startPosSegmentIt[1].GetX() - startPosSegmentIt[0].GetX());
            const double deltaY(startPosSegmentIt[1].GetY() - startPosSegmentIt[0].GetY());

            // We modify the start point
            startPosSegmentIt[0].SetX(startPosSegmentIt[0].GetX() + (deltaX * segmentRelPos));
            startPosSegmentIt[0].SetY(startPosSegmentIt[0].GetY() + (deltaY * segmentRelPos));

            m_Points.erase(m_Points.begin(), startPosSegmentIt);
            }
        }


    // TODO: We should have some code here that ensure that we did not create new 0.0 length segments
    // as it may create division by 0.0 issues. See here-above.   

    // We update start and end points
    m_StartPoint = HGF2DLocation(m_Points[0], GetCoordSys());
    m_EndPoint = HGF2DLocation(m_Points[m_Points.size() - 1], GetCoordSys());

    // Indicate extent is no more up to date
    m_ExtentUpToDate = false;

    // NOTE: Active this in order to validate good behavior of the method.
    //HPOSTCONDITION(HDOUBLE_EQUAL(CalculateLength().GetValue(), (endLength - startLength).GetValue(), GetTolerance()));

    // Reset tolerance
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

    // Select extremity processing
    HGF2DLiteSegment::ExtremityProcessing SegmentExtremityProcessing = (pi_ExtremityProcessing == HVE2DVector::INCLUDE_EXTREMITIES ?
                                                                        HGF2DLiteSegment::INCLUDE_EXTREMITIES :
                                                                        HGF2DLiteSegment::EXCLUDE_EXTREMITIES);
    // Obtain tolerance
    double Tolerance = pi_Tolerance;

    if (Tolerance < 0.0)
        Tolerance = GetTolerance();

    // Declare answer variable
    bool   Answer = false;

    // To check if a point is on polysegment, we must check if
    // it is on any of the component segment

    // Check if polysegment is not NULL
    if (m_Points.size() >= 2)
        {
        // Obtain extremes of polysegment
        if (!m_ExtentUpToDate)
            GetExtent();

        double XMin = m_Extent.GetXMin();
        double XMax = m_Extent.GetXMax();
        double YMin = m_Extent.GetYMin();
        double YMax = m_Extent.GetYMax();

        // Extract raw coordinate values of point
        double X = pi_rTestPoint.GetX();
        double Y = pi_rTestPoint.GetY();

        // A point is on if it is within the extended extent.
        if (HDOUBLE_GREATER_OR_EQUAL(X, XMin, Tolerance) &&
            HDOUBLE_SMALLER_OR_EQUAL(X, XMax, Tolerance) &&
            HDOUBLE_GREATER_OR_EQUAL(Y, YMin, Tolerance) &&
            HDOUBLE_SMALLER_OR_EQUAL(Y, YMax, Tolerance))
            {
            // Point and polysegment are in the same general area

            HGF2DLiteSegment TheLiteSegment;
            TheLiteSegment.SetTolerance (Tolerance);

            HGF2DPosition ThePoint;

            // We must check for every component segment
            HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
            HGF2DPositionCollection::const_iterator PreviousItr(Itr);
            ++Itr;

            HGF2DPositionCollection::const_iterator BackItr = m_Points.end();
            --BackItr;

            for (; !Answer && Itr != m_Points.end() ; ++Itr , ++PreviousItr)
                {
                // Obtain extent of this segment
                XMin = MIN(Itr->GetX(), PreviousItr->GetX());
                XMax = MAX(Itr->GetX(), PreviousItr->GetX());
                YMin = MIN(Itr->GetY(), PreviousItr->GetY());
                YMax = MAX(Itr->GetY(), PreviousItr->GetY());

                // Check if current segment and segment may interact
                bool Result = (HDOUBLE_GREATER_OR_EQUAL(X, XMin, Tolerance) &&
                                HDOUBLE_SMALLER_OR_EQUAL(X, XMax, Tolerance) &&
                                HDOUBLE_GREATER_OR_EQUAL(Y, YMin, Tolerance) &&
                                HDOUBLE_SMALLER_OR_EQUAL(Y, YMax, Tolerance)
                               );


                if (Result)
                    {
                    // Current segment and given polysegment may interact ...
                    TheLiteSegment.SetStartPoint(*PreviousItr);
                    TheLiteSegment.SetEndPoint(*Itr);
                    ThePoint.SetX(X);
                    ThePoint.SetY(Y);

                    // Check if point is on segment
                    // Note that tolerance is part of segment
                    // For the last and first segment, we check extremity processing state
                    if (PreviousItr == m_Points.begin() && Itr == BackItr)
                        Answer = TheLiteSegment.IsPointOn(ThePoint, SegmentExtremityProcessing);
                    else if (PreviousItr == m_Points.begin())
                        Answer = TheLiteSegment.IsPointOn(ThePoint, SegmentExtremityProcessing) || (ThePoint.IsEqualTo(*Itr, Tolerance));
                    else if (Itr == BackItr)
                        Answer = TheLiteSegment.IsPointOn(ThePoint, SegmentExtremityProcessing) || (ThePoint.IsEqualTo(*PreviousItr, Tolerance));
                    else
                        Answer = TheLiteSegment.IsPointOn(ThePoint);
                    }
                }
            }
        }


    return(Answer);
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
    HPRECONDITION(m_StartPoint.IsEqualTo(pi_rPoint, GetTolerance()));

    // The polysegment must not be nul
    HPRECONDITION(m_Points.size() >= 2);

    // Set start point
    m_StartPoint.Set(pi_rPoint);

    // Adjust point collection
    m_Points[0] = m_StartPoint.GetPosition();

    // Tolerance is not adjusted since change is only minor
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
    HPRECONDITION(m_EndPoint.IsEqualTo(pi_rPoint, GetTolerance()));

    // The polysegment must not be nul
    HPRECONDITION(m_Points.size() >= 2);

    // Adjust end point
    m_EndPoint.Set(pi_rPoint);

    // Adjust point collection
    m_Points[m_Points.size() - 1] = m_EndPoint.GetPosition();

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
    HPRECONDITION(m_Points.size() >= 2);

    // NOTE : Tolerance is unused since drop is exact

    // Allocate space in list
    po_pPoints->reserve(m_Points.size() - 1);

    // Add points except end point
    HGF2DPositionCollection::const_iterator Itr;

    // We extract next to last iterator position
    HGF2DPositionCollection::const_iterator ItrEnd(m_Points.end());
    --ItrEnd;

    // Copy points
    for (Itr = m_Points.begin() ; Itr != ItrEnd ; ++Itr)
        {
        po_pPoints->push_back(HGF2DLocation((*Itr), GetCoordSys()));
        }

    // Check if end point must be included
    if (pi_EndPointProcessing == HVE2DLinear::INCLUDE_END_POINT)
        {
        // We must add end point
        po_pPoints->push_back(m_EndPoint);
        }

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

    // Declare answer variable
    bool Answer = false;

    // Pre-calculate tolerance
    double Tolerance = MIN(GetTolerance(), pi_rPolySegment.GetTolerance());

    // Check if polysegments are not NULL
    if (m_Points.size() >= 2 && pi_rPolySegment.m_Points.size() >= 2)
        {

        // Obtain extremes of self and given poly segments
        HGF2DExtent PolyExtent(GetExtent());
        HGF2DExtent GivenPolyExtent(pi_rPolySegment.GetExtent());

        double SelfXMin = PolyExtent.GetXMin();
        double SelfXMax = PolyExtent.GetXMax();
        double SelfYMin = PolyExtent.GetYMin();
        double SelfYMax = PolyExtent.GetYMax();

        double GivenXMin = GivenPolyExtent.GetXMin();
        double GivenXMax = GivenPolyExtent.GetXMax();
        double GivenYMin = GivenPolyExtent.GetYMin();
        double GivenYMax = GivenPolyExtent.GetYMax();


        // Check if extents outter overlap
        bool Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                        HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                       );


        if (Result)
            {

            // Polysegments may interact

            // For every segment of the self polysegment
            HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
            HGF2DPositionCollection::const_iterator PreviousItr(Itr);
            ++Itr;

            for (; !Answer && Itr != m_Points.end() ; ++Itr , ++PreviousItr)
                {
                // Obtain extent of this segment
                SelfXMin = MIN(Itr->GetX(), PreviousItr->GetX());
                SelfXMax = MAX(Itr->GetX(), PreviousItr->GetX());
                SelfYMin = MIN(Itr->GetY(), PreviousItr->GetY());
                SelfYMax = MAX(Itr->GetY(), PreviousItr->GetY());

                // Check if current segment and segment may interact
                Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                          HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                         );


                if (Result)
                    {
                    // Current segment and given polysegment may interact ...

                    // Create a lite segment to represent current segment
                    HGF2DLiteSegment TheLiteSegment(*PreviousItr, *Itr, Tolerance);

                    // For every segment of given polysegment
                    HGF2DPositionCollection::const_iterator GivenItr(pi_rPolySegment.m_Points.begin());
                    HGF2DPositionCollection::const_iterator PreviousGivenItr(GivenItr);
                    ++GivenItr;
                    for (; !Answer && GivenItr != pi_rPolySegment.m_Points.end() ; ++GivenItr , ++PreviousGivenItr)
                        {
                        // Check if current self segment and current given segment may interact
                        Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, MIN(GivenItr->GetX(), PreviousGivenItr->GetX()), Tolerance) &&
                                  HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, MAX(GivenItr->GetX(), PreviousGivenItr->GetX()), Tolerance) &&
                                  HDOUBLE_GREATER_OR_EQUAL(SelfYMax, MIN(GivenItr->GetY(), PreviousGivenItr->GetY()), Tolerance) &&
                                  HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, MAX(GivenItr->GetY(), PreviousGivenItr->GetY()), Tolerance)
                                 );


                        if (Result)
                            {
                            // The two segments may interact ... check if contiguous
                            Answer = HGF2DLiteSegment(*PreviousGivenItr,
                                                      *GivenItr,
                                                      Tolerance).AreContiguous(TheLiteSegment);
                            }
                        }
                    }
                }
            }
        }

    return(Answer);
    }

//-----------------------------------------------------------------------------
// IsContiguousToPolySegmentLRCS
// PRIVATE
// Indicates if the polysegment is contiguous to the given polysegment
// They must already be expressed in coordinate systems which are related
// through a linearity preserving transformation model
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::IsContiguousToPolySegmentLRCS(const HVE2DPolySegment& pi_rPolySegment) const
    {
    HINVARIANTS;

    // They must already be expressed in coordinate systems which are related
    // through a linearity preserving transformation model
    HPRECONDITION(GetCoordSys()->HasLinearityPreservingRelationTo(pi_rPolySegment.GetCoordSys()));

    // Declare answer variable
    bool Answer = false;

    // Pre-calculate tolerance
    double Tolerance = MIN(GetTolerance(), pi_rPolySegment.GetTolerance());

    // Check if polysegments are not NULL
    if (m_Points.size() >= 2 && pi_rPolySegment.m_Points.size() >= 2)
        {

        // Obtain extremes of self and given poly segments
        HGF2DExtent PolyExtent(GetExtent());
        HGF2DExtent GivenPolyExtent(pi_rPolySegment.GetExtent());
        GivenPolyExtent.ChangeCoordSys(GetCoordSys());

        double SelfXMin = PolyExtent.GetXMin();
        double SelfXMax = PolyExtent.GetXMax();
        double SelfYMin = PolyExtent.GetYMin();
        double SelfYMax = PolyExtent.GetYMax();

        double GivenXMin = GivenPolyExtent.GetXMin();
        double GivenXMax = GivenPolyExtent.GetXMax();
        double GivenYMin = GivenPolyExtent.GetYMin();
        double GivenYMax = GivenPolyExtent.GetYMax();


        // Check if polysegments outter overlap
        bool Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                        HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                       );


        if (Result)
            {

            // Polysegments may interact

            // For every segment of the self polysegment
            HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
            HGF2DPositionCollection::const_iterator PreviousItr(Itr);
            ++Itr;
            for (; !Answer && Itr != m_Points.end() ; ++Itr , ++PreviousItr)
                {
                // Obtain extent of this segment
                SelfXMin = MIN(Itr->GetX(), PreviousItr->GetX());
                SelfXMax = MAX(Itr->GetX(), PreviousItr->GetX());
                SelfYMin = MIN(Itr->GetY(), PreviousItr->GetY());
                SelfYMax = MAX(Itr->GetY(), PreviousItr->GetY());

                // Check if current segment and segment may interact
                Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                          HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                         );


                if (Result)
                    {
                    // Current segment and given polysegment may interact ...

                    // Create a lite segment to represent current segment
                    HGF2DLiteSegment TheLiteSegment(*PreviousItr, *Itr, Tolerance);

                    // For every segment of given polysegment
                    HGF2DPositionCollection::const_iterator GivenItr(pi_rPolySegment.m_Points.begin());
                    HGF2DPositionCollection::const_iterator PreviousGivenItr(GivenItr);
                    ++GivenItr;
                    for (; !Answer && GivenItr != pi_rPolySegment.m_Points.end() ; ++GivenItr , ++PreviousGivenItr)
                        {
                        // Create location in self coordinate system
                        HGF2DLocation CurGivenSegStartLocation(*PreviousGivenItr, pi_rPolySegment.GetCoordSys());
                        CurGivenSegStartLocation.ChangeCoordSys(GetCoordSys());
                        HGF2DLocation CurGivenSegEndLocation(*GivenItr, pi_rPolySegment.GetCoordSys());
                        CurGivenSegEndLocation.ChangeCoordSys(GetCoordSys());

                        HGF2DPosition CurGivenSegStartPosition(CurGivenSegStartLocation.GetPosition());
                        HGF2DPosition CurGivenSegEndPosition(CurGivenSegEndLocation.GetPosition());

                        // Check if current self segment and current given segment may interact
                        Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, MIN(CurGivenSegEndPosition.GetX(), CurGivenSegStartPosition.GetX()), Tolerance) &&
                                  HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, MAX(CurGivenSegEndPosition.GetX(), CurGivenSegStartPosition.GetX()), Tolerance) &&
                                  HDOUBLE_GREATER_OR_EQUAL(SelfYMax, MIN(CurGivenSegEndPosition.GetY(), CurGivenSegStartPosition.GetY()), Tolerance) &&
                                  HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, MAX(CurGivenSegEndPosition.GetY(), CurGivenSegStartPosition.GetY()), Tolerance)
                                 );


                        if (Result)
                            {
                            // The two segments may interact ... check if contiguous
                            Answer = HGF2DLiteSegment(CurGivenSegStartPosition,
                                                      CurGivenSegEndPosition,
                                                      Tolerance).AreContiguous(TheLiteSegment);
                            }
                        }
                    }
                }
            }
        }

    return(Answer);
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

    // Declare answer variable
    bool Answer = false;

    // Pre-calculate tolerance
    double Tolerance = MIN(GetTolerance(), pi_rSegment.GetTolerance());

    // Check if polysegment is not NULL
    if (m_Points.size() >= 2)
        {
        // Obtain extremes of poly segment
        HGF2DExtent PolyExtent(GetExtent());

        double SelfXMin = PolyExtent.GetXMin();
        double SelfXMax = PolyExtent.GetXMax();
        double SelfYMin = PolyExtent.GetYMin();
        double SelfYMax = PolyExtent.GetYMax();

        // Obtain extremes of segment
        double GivenXMin = MIN(pi_rSegment.GetStartPoint().GetX(),
                                pi_rSegment.GetEndPoint().GetX());
        double GivenXMax = MAX(pi_rSegment.GetStartPoint().GetX(),
                                pi_rSegment.GetEndPoint().GetX());
        double GivenYMin = MIN(pi_rSegment.GetStartPoint().GetY(),
                                pi_rSegment.GetEndPoint().GetY());
        double GivenYMax = MAX(pi_rSegment.GetStartPoint().GetY(),
                                pi_rSegment.GetEndPoint().GetY());


        // Check if segment and polysegment extents outter overlap
        bool Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                        HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                       );


        if (Result)
            {
            // Polysegment and segment may interact

            // Create a lite segment to represent segment
            HGF2DLiteSegment TheLiteSegment(pi_rSegment.GetStartPoint().GetPosition(),
                                            pi_rSegment.GetEndPoint().GetPosition(),
                                            Tolerance);


            // For every segment of the polysegment
            HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
            HGF2DPositionCollection::const_iterator PreviousItr(Itr);
            ++Itr;
            for (; !Answer && Itr != m_Points.end() ; ++Itr , ++PreviousItr)
                {
                // Obtain extent of this segment
                SelfXMin = MIN(Itr->GetX(), PreviousItr->GetX());
                SelfXMax = MAX(Itr->GetX(), PreviousItr->GetX());
                SelfYMin = MIN(Itr->GetY(), PreviousItr->GetY());
                SelfYMax = MAX(Itr->GetY(), PreviousItr->GetY());

                // Check if current segment and segment may interact
                Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                          HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                         );


                if (Result)
                    {
                    // The two segments may interact ... check if contiguous
                    Answer = HGF2DLiteSegment(*PreviousItr,
                                              *Itr,
                                              Tolerance).AreContiguous(TheLiteSegment);
                    }
                }
            }
        }

    return(Answer);

    }

//-----------------------------------------------------------------------------
// IsContiguousToSegmentLRCS
// PRIVATE
// Indicates if the polysegment is contiguous to the given segment
// They must already be expressed in the same coordinate system.
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::IsContiguousToSegmentLRCS(const HVE2DSegment& pi_rSegment) const
    {
    HINVARIANTS;

    // They must already be expressed in coordinate systems which are related
    // through a linearity preserving transformation model
    HPRECONDITION(GetCoordSys()->HasLinearityPreservingRelationTo(pi_rSegment.GetCoordSys()));

    // Declare answer variable
    bool Answer = false;

    // Pre-calculate tolerance
    double Tolerance = MIN(GetTolerance(), pi_rSegment.GetTolerance());

    // Obtain coordinates of segment in polysegment coordinate system
    HGF2DLocation SegmentStartLocation(pi_rSegment.GetStartPoint(), GetCoordSys());
    HGF2DLocation SegmentEndLocation(pi_rSegment.GetEndPoint(), GetCoordSys());

    // Transform into positions
    HGF2DPosition SegmentStartPosition(SegmentStartLocation.GetPosition());
    HGF2DPosition SegmentEndPosition(SegmentEndLocation.GetPosition());

    // Check if polysegment is not NULL
    if (m_Points.size() >= 2)
        {

        // Obtain extremes of poly segment
        HGF2DExtent PolyExtent(GetExtent());

        double SelfXMin = PolyExtent.GetXMin();
        double SelfXMax = PolyExtent.GetXMax();
        double SelfYMin = PolyExtent.GetYMin();
        double SelfYMax = PolyExtent.GetYMax();

        // Obtain segment extremes
        double GivenXMin = MIN(SegmentStartPosition.GetX(), SegmentEndPosition.GetX());
        double GivenXMax = MAX(SegmentStartPosition.GetX(), SegmentEndPosition.GetX());
        double GivenYMin = MIN(SegmentStartPosition.GetY(), SegmentEndPosition.GetY());
        double GivenYMax = MAX(SegmentStartPosition.GetY(), SegmentEndPosition.GetY());

        // Check is segment and polysegment extent outter overlap
        bool Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                        HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                       );


        if (Result)
            {
            // Polysegment and segment may interact

            // Create a lite segment to represent segment
            HGF2DLiteSegment TheLiteSegment(SegmentStartPosition,
                                            SegmentEndPosition,
                                            Tolerance);

            // For every segment of the polysegment
            HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
            HGF2DPositionCollection::const_iterator PreviousItr(Itr);
            ++Itr;
            for (; !Answer && Itr != m_Points.end() ; ++Itr , ++PreviousItr)
                {
                // Obtain extent of this segment
                SelfXMin = MIN(Itr->GetX(), PreviousItr->GetX());
                SelfXMax = MAX(Itr->GetX(), PreviousItr->GetX());
                SelfYMin = MIN(Itr->GetY(), PreviousItr->GetY());
                SelfYMax = MAX(Itr->GetY(), PreviousItr->GetY());

                // Check if current segment and segment may interact
                Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                          HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                         );


                if (Result)
                    {
                    // The two segments may interact ... check if contiguous
                    Answer = HGF2DLiteSegment(*PreviousItr,
                                              *Itr,
                                              Tolerance).AreContiguous(TheLiteSegment);
                    }
                }
            }
        }

    return(Answer);
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

        // We check if it is linearity preserving
        if (GetCoordSys()->HasLinearityPreservingRelationTo(pi_rPolySegment.GetCoordSys()))
            {
            // They are related through a linearity preserving model ... go ahead
            Answer = IsContiguousToPolySegmentLRCS(pi_rPolySegment);
            }
        else
            {
            // They are not related by a linearity preserving model ... we must allocate a copy
            // In correct coordinate system
            HAutoPtr<HVE2DVector> pTempVector(pi_rPolySegment.AllocateCopyInCoordSys(GetCoordSys()));

            // Obtain answer
            Answer = pTempVector->AreContiguous(*this);
            }
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

            // We check if it is linearity preserving
            if (GetCoordSys()->HasLinearityPreservingRelationTo(pi_rSegment.GetCoordSys()))
                {
                // They are related through a linearity preserving model ... go ahead
                Answer = IsContiguousToSegmentLRCS(pi_rSegment);
                }
            else
                {
                // They are not related by a linearity preserving model ... we must allocate a copy
                // In correct coordinate system
                HAutoPtr<HVE2DVector> pTempVector(pi_rSegment.AllocateCopyInCoordSys(GetCoordSys()));

                // Obtain answer
                Answer = pTempVector->AreContiguous(*this);
                }
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

    // Declare answer variable
    bool Answer = false;

    // Pre-calculate tolerance
    double Tolerance = MIN(GetTolerance(), pi_rPolySegment.GetTolerance());

    // Transform point into a position in self coordinate system
    HGF2DLocation PointInSelfCoordSys(pi_rPoint, GetCoordSys());
    HGF2DPosition ThePoint(PointInSelfCoordSys.GetPosition());

    // Check if polysegments are not NULL
    if (m_Points.size() >= 2 && pi_rPolySegment.m_Points.size() >= 2)
        {
        // Obtain extremes of self and given poly segments
        HGF2DExtent PolyExtent(GetExtent());
        HGF2DExtent GivenPolyExtent(pi_rPolySegment.GetExtent());

        double SelfXMin = PolyExtent.GetXMin();
        double SelfXMax = PolyExtent.GetXMax();
        double SelfYMin = PolyExtent.GetYMin();
        double SelfYMax = PolyExtent.GetYMax();

        double GivenXMin = GivenPolyExtent.GetXMin();
        double GivenXMax = GivenPolyExtent.GetXMax();
        double GivenYMin = GivenPolyExtent.GetYMin();
        double GivenYMax = GivenPolyExtent.GetYMax();


        // Check if polysegments extent outter overlap
        bool Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                        HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                       );


        if (Result)
            {
            // Polysegments may interact

            // For every segment of the self polysegment
            HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
            HGF2DPositionCollection::const_iterator PreviousItr(Itr);
            ++Itr;
            for (; !Answer && Itr != m_Points.end() ; ++Itr , ++PreviousItr)
                {
                // Obtain extent of this segment
                SelfXMin = MIN(Itr->GetX(), PreviousItr->GetX());
                SelfXMax = MAX(Itr->GetX(), PreviousItr->GetX());
                SelfYMin = MIN(Itr->GetY(), PreviousItr->GetY());
                SelfYMax = MAX(Itr->GetY(), PreviousItr->GetY());

                // Check if current segment and segment may interact
                Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                          HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                         );


                if (Result)
                    {
                    // Current segment and given polysegment may interact ...

                    // Create a lite segment to represent current segment
                    HGF2DLiteSegment TheLiteSegment(*PreviousItr, *Itr, Tolerance);

                    // Check if point is located on current segment
                    if (TheLiteSegment.IsPointOn(ThePoint))
                        {
                        // The point is on current segment ...

                        // For every segment of given polysegment
                        HGF2DPositionCollection::const_iterator GivenItr(pi_rPolySegment.m_Points.begin());
                        HGF2DPositionCollection::const_iterator PreviousGivenItr(GivenItr);
                        ++GivenItr;
                        for (; !Answer && GivenItr != pi_rPolySegment.m_Points.end() ; ++GivenItr , ++PreviousGivenItr)
                            {
                            // Check if current self segment and current given segment may interact
                            Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, MIN(GivenItr->GetX(), PreviousGivenItr->GetX()), Tolerance) &&
                                      HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, MAX(GivenItr->GetX(), PreviousGivenItr->GetX()), Tolerance) &&
                                      HDOUBLE_GREATER_OR_EQUAL(SelfYMax, MIN(GivenItr->GetY(), PreviousGivenItr->GetY()), Tolerance) &&
                                      HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, MAX(GivenItr->GetY(), PreviousGivenItr->GetY()), Tolerance)
                                     );


                            if (Result)
                                {
                                // The two segments may interact ...
                                HGF2DLiteSegment CurrentGivenSegment(*PreviousGivenItr,
                                                                     *GivenItr,
                                                                     Tolerance);

                                // Check if point is located on this given segment
                                if (CurrentGivenSegment.IsPointOn(ThePoint))
                                    {
                                    // All conditions are met ... check if segments
                                    // are indeed contiguous
                                    Answer = CurrentGivenSegment.AreContiguous(TheLiteSegment);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

    return(Answer);
    }

//-----------------------------------------------------------------------------
// IsContiguousToPolySegmentAtLRCS
// PRIVATE
// Indicates if the polysegment is contiguous to the given polysegment
// They must already be expressed in coordinate systems which are related
// through a linearity preserving transformation model
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::IsContiguousToPolySegmentAtLRCS(const HVE2DPolySegment& pi_rPolySegment,
                                                        const HGF2DLocation&    pi_rPoint) const
    {
    HINVARIANTS;

    // They must already be expressed in coordinate systems which are related
    // through a linearity preserving transformation model
    HPRECONDITION(GetCoordSys()->HasLinearityPreservingRelationTo(pi_rPolySegment.GetCoordSys()));

    // Declare answer variable
    bool Answer = false;

    // Pre-calculate tolerance
    double Tolerance = MIN(GetTolerance(), pi_rPolySegment.GetTolerance());

    // Transform point into a position in self coordinate system
    HGF2DLocation PointInSelfCoordSys(pi_rPoint, GetCoordSys());
    HGF2DPosition ThePoint(PointInSelfCoordSys.GetPosition());

    // Check if polysegments are not NULL
    if (m_Points.size() >= 2 && pi_rPolySegment.m_Points.size() >= 2)
        {
        // Obtain extremes of self polysegment
        HGF2DExtent PolyExtent(GetExtent());

        // Obtain extent of given polysegment expressed in the
        // self coordinate system
        HGF2DExtent GivenPolyExtent(pi_rPolySegment.GetExtent());
        GivenPolyExtent.ChangeCoordSys(GetCoordSys());

        double SelfXMin = PolyExtent.GetXMin();
        double SelfXMax = PolyExtent.GetXMax();
        double SelfYMin = PolyExtent.GetYMin();
        double SelfYMax = PolyExtent.GetYMax();

        double GivenXMin = GivenPolyExtent.GetXMin();
        double GivenXMax = GivenPolyExtent.GetXMax();
        double GivenYMin = GivenPolyExtent.GetYMin();
        double GivenYMax = GivenPolyExtent.GetYMax();


        // Check if extents of polysegments outter overlap
        bool Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                        HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                       );


        if (Result)
            {
            // Polysegments may interact

            // For every segment of the self polysegment
            HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
            HGF2DPositionCollection::const_iterator PreviousItr(Itr);
            ++Itr;
            for (; !Answer && Itr != m_Points.end() ; ++Itr , ++PreviousItr)
                {
                // Obtain extent of this segment
                SelfXMin = MIN(Itr->GetX(), PreviousItr->GetX());
                SelfXMax = MAX(Itr->GetX(), PreviousItr->GetX());
                SelfYMin = MIN(Itr->GetY(), PreviousItr->GetY());
                SelfYMax = MAX(Itr->GetY(), PreviousItr->GetY());

                // Check if current segment and segment may interact
                Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                          HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                         );


                if (Result)
                    {
                    // Current segment and given polysegment may interact ...

                    // Create a lite segment to represent current segment
                    HGF2DLiteSegment TheLiteSegment(*PreviousItr, *Itr, Tolerance);

                    // Check if point is on current self segment
                    if (TheLiteSegment.IsPointOn(ThePoint))
                        {
                        // For every segment of given polysegment
                        HGF2DPositionCollection::const_iterator GivenItr(pi_rPolySegment.m_Points.begin());
                        HGF2DPositionCollection::const_iterator PreviousGivenItr(GivenItr);
                        ++GivenItr;
                        for (; !Answer && GivenItr != pi_rPolySegment.m_Points.end() ; ++GivenItr , ++PreviousGivenItr)
                            {
                            // Create location in self coordinate system
                            HGF2DLocation CurGivenSegStartLocation(*PreviousGivenItr, pi_rPolySegment.GetCoordSys());
                            CurGivenSegStartLocation.ChangeCoordSys(GetCoordSys());
                            HGF2DLocation CurGivenSegEndLocation(*GivenItr, pi_rPolySegment.GetCoordSys());
                            CurGivenSegEndLocation.ChangeCoordSys(GetCoordSys());

                            HGF2DPosition CurGivenSegStartPosition(CurGivenSegStartLocation.GetPosition());
                            HGF2DPosition CurGivenSegEndPosition(CurGivenSegEndLocation.GetPosition());

                            // Check if current self segment and current given segment may interact
                            Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, MIN(CurGivenSegEndPosition.GetX(), CurGivenSegStartPosition.GetX()), Tolerance) &&
                                      HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, MAX(CurGivenSegEndPosition.GetX(), CurGivenSegStartPosition.GetX()), Tolerance) &&
                                      HDOUBLE_GREATER_OR_EQUAL(SelfYMax, MIN(CurGivenSegEndPosition.GetY(), CurGivenSegStartPosition.GetY()), Tolerance) &&
                                      HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, MAX(CurGivenSegEndPosition.GetY(), CurGivenSegStartPosition.GetY()), Tolerance)
                                     );


                            if (Result)
                                {
                                // The two segments may interact ...
                                HGF2DLiteSegment CurrentGivenSegment(CurGivenSegStartPosition,
                                                                     CurGivenSegEndPosition,
                                                                     Tolerance);

                                // Check if point is located on this given segment
                                if (CurrentGivenSegment.IsPointOn(ThePoint))
                                    {
                                    // The two segments may interact ... check if contiguous
                                    Answer = CurrentGivenSegment.AreContiguous(TheLiteSegment);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

    return(Answer);
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

    // Declare answer variable
    bool Answer = false;

    // Pre-calculate tolerance
    double Tolerance = MIN(GetTolerance(), pi_rSegment.GetTolerance());

    // Transform point into a position in self coordinate system
    HGF2DLocation PointInSelfCoordSys(pi_rPoint, GetCoordSys());
    HGF2DPosition ThePoint(PointInSelfCoordSys.GetPosition());

    // Check if polysegment is not NULL
    if (m_Points.size() >= 2)
        {
        // Obtain extremes of poly segment
        HGF2DExtent PolyExtent(GetExtent());

        double SelfXMin = PolyExtent.GetXMin();
        double SelfXMax = PolyExtent.GetXMax();
        double SelfYMin = PolyExtent.GetYMin();
        double SelfYMax = PolyExtent.GetYMax();

        double GivenXMin = MIN(pi_rSegment.GetStartPoint().GetX(),
                                pi_rSegment.GetEndPoint().GetX());
        double GivenXMax = MAX(pi_rSegment.GetStartPoint().GetX(),
                                pi_rSegment.GetEndPoint().GetX());
        double GivenYMin = MIN(pi_rSegment.GetStartPoint().GetY(),
                                pi_rSegment.GetEndPoint().GetY());
        double GivenYMax = MAX(pi_rSegment.GetStartPoint().GetY(),
                                pi_rSegment.GetEndPoint().GetY());


        bool Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                        HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                       );

        if (Result)
            {
            // Polysegment and segment may interact

            // Create a lite segment to represent segment
            HGF2DLiteSegment TheLiteSegment(pi_rSegment.GetStartPoint().GetPosition(),
                                            pi_rSegment.GetEndPoint().GetPosition(),
                                            Tolerance);


            // For every segment of the polysegment
            HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
            HGF2DPositionCollection::const_iterator PreviousItr(Itr);
            ++Itr;
            for (; !Answer && Itr != m_Points.end() ; ++Itr , ++PreviousItr)
                {
                // Obtain extent of this segment
                SelfXMin = MIN(Itr->GetX(), PreviousItr->GetX());
                SelfXMax = MAX(Itr->GetX(), PreviousItr->GetX());
                SelfYMin = MIN(Itr->GetY(), PreviousItr->GetY());
                SelfYMax = MAX(Itr->GetY(), PreviousItr->GetY());

                // Check if current segment and segment may interact
                Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                          HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                         );


                if (Result)
                    {
                    // The two segments may interact ... check if point is on current segment
                    HGF2DLiteSegment CurrentSegment(*PreviousItr, *Itr, Tolerance);

                    // Check if point is located on this segment
                    if (CurrentSegment.IsPointOn(ThePoint))
                        {
                        // The fact that the point is located on both segments is sufficient
                        // to be contiguous at if plain contiguous

                        // Check if contiguous
                        Answer = CurrentSegment.AreContiguous(TheLiteSegment);
                        }
                    }
                }
            }
        }

    return(Answer);
    }

//-----------------------------------------------------------------------------
// IsContiguousToSegmentAtLRCS
// PRIVATE
// Indicates if the polysegment is contiguous to the given segment
// They must already be expressed in the same coordinate system.
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::IsContiguousToSegmentAtLRCS(const HVE2DSegment&  pi_rSegment,
                                                    const HGF2DLocation& pi_rPoint) const
    {
    HINVARIANTS;

    // They must already be expressed in coordinate systems which are related
    // through a linearity preserving transformation model
    HPRECONDITION(GetCoordSys()->HasLinearityPreservingRelationTo(pi_rSegment.GetCoordSys()));

    // The given point must be located on both vectors
    HASSERTSUPERDEBUG(IsPointOn(pi_rPoint) && pi_rSegment.IsPointOn(pi_rPoint));

    // Declare answer variable
    bool Answer = false;

    // Pre-calculate tolerance
    double Tolerance = MIN(GetTolerance(), pi_rSegment.GetTolerance());

    // Obtain coordinates of segment in polysegment coordinate system
    HGF2DLocation SegmentStartLocation(pi_rSegment.GetStartPoint(), GetCoordSys());
    HGF2DLocation SegmentEndLocation(pi_rSegment.GetEndPoint(), GetCoordSys());

    // Transform into positions
    HGF2DPosition SegmentStartPosition(SegmentStartLocation.GetPosition());
    HGF2DPosition SegmentEndPosition(SegmentEndLocation.GetPosition());

    // Transform point into a position in self coordinate system
    HGF2DLocation PointInSelfCoordSys(pi_rPoint, GetCoordSys());
    HGF2DPosition ThePoint(PointInSelfCoordSys.GetPosition());

    // Check if polysegments is not NULL
    if (m_Points.size() >= 2)
        {
        // Obtain extremes of poly segment
        HGF2DExtent PolyExtent(GetExtent());

        double SelfXMin = PolyExtent.GetXMin();
        double SelfXMax = PolyExtent.GetXMax();
        double SelfYMin = PolyExtent.GetYMin();
        double SelfYMax = PolyExtent.GetYMax();

        double GivenXMin = MIN(SegmentStartPosition.GetX(), SegmentEndPosition.GetX());
        double GivenXMax = MAX(SegmentStartPosition.GetX(), SegmentEndPosition.GetX());
        double GivenYMin = MIN(SegmentStartPosition.GetY(), SegmentEndPosition.GetY());
        double GivenYMax = MAX(SegmentStartPosition.GetY(), SegmentEndPosition.GetY());


        bool Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                        HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                       );

        if (Result)
            {
            // Polysegment and segment may interact

            // Create a lite segment to represent segment
            HGF2DLiteSegment TheLiteSegment(SegmentStartPosition,
                                            SegmentEndPosition,
                                            Tolerance);

            // For every segment of the polysegment
            HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
            HGF2DPositionCollection::const_iterator PreviousItr(Itr);
            ++Itr;
            for (; !Answer && Itr != m_Points.end() ; ++Itr , ++PreviousItr)
                {
                // Obtain extent of this segment
                SelfXMin = MIN(Itr->GetX(), PreviousItr->GetX());
                SelfXMax = MAX(Itr->GetX(), PreviousItr->GetX());
                SelfYMin = MIN(Itr->GetY(), PreviousItr->GetY());
                SelfYMax = MAX(Itr->GetY(), PreviousItr->GetY());

                // Check if current segment and segment may interact
                Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                          HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                         );


                if (Result)
                    {
                    // The two segments may interact ... check if point is on current segment
                    HGF2DLiteSegment CurrentSegment(*PreviousItr, *Itr, Tolerance);

                    // Check if point is on this segment
                    if (CurrentSegment.IsPointOn(ThePoint))
                        {
                        // The fact that the point is located on both segments is sufficient
                        // to be contiguous at if plain contiguous

                        // Check if contiguous
                        Answer = CurrentSegment.AreContiguous(TheLiteSegment);
                        }
                    }
                }
            }
        }

    return(Answer);
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

        // We check if it is linearity preserving
        if (GetCoordSys()->HasLinearityPreservingRelationTo(pi_rPolySegment.GetCoordSys()))
            {
            // They are related through a linearity preserving model ... go ahead
            Answer = IsContiguousToPolySegmentAtLRCS(pi_rPolySegment, pi_rPoint);
            }
        else
            {
            // They are not related by a linearity preserving model ... we must allocate a copy
            // In correct coordinate system
            HAutoPtr<HVE2DVector> pTempVector(pi_rPolySegment.AllocateCopyInCoordSys(GetCoordSys()));

            // Obtain answer
            Answer = pTempVector->AreContiguousAt(*this, pi_rPoint);
            }
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

            // We check if it is linearity preserving
            if (GetCoordSys()->HasLinearityPreservingRelationTo(pi_rSegment.GetCoordSys()))
                {
                // They are related through a linearity preserving model ... go ahead
                Answer = IsContiguousToSegmentAtLRCS(pi_rSegment, pi_rPoint);
                }
            else
                {
                // They are not related by a linearity preserving model ... we must allocate a copy
                // In correct coordinate system
                HAutoPtr<HVE2DVector> pTempVector(pi_rSegment.AllocateCopyInCoordSys(GetCoordSys()));

                // Obtain answer from newly allocated vector
                Answer = pTempVector->AreContiguousAt(*this, pi_rPoint);
                }
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

    // Declare answer variable
    bool Answer = false;

    // Pre-calculate tolerance
    double Tolerance = MIN(GetTolerance(), pi_rPolySegment.GetTolerance());

    // Check if polysegments are not NULL
    if (m_Points.size() >= 2 && pi_rPolySegment.m_Points.size() >= 2)
        {
        // Obtain extremes of poly segments
        HGF2DExtent PolyExtent(GetExtent());
        HGF2DExtent GivenPolyExtent(pi_rPolySegment.GetExtent());

        double SelfXMin = PolyExtent.GetXMin();
        double SelfXMax = PolyExtent.GetXMax();
        double SelfYMin = PolyExtent.GetYMin();
        double SelfYMax = PolyExtent.GetYMax();

        double GivenXMin = GivenPolyExtent.GetXMin();
        double GivenXMax = GivenPolyExtent.GetXMax();
        double GivenYMin = GivenPolyExtent.GetYMin();
        double GivenYMax = GivenPolyExtent.GetYMax();

        // Check if their extents outter overlap
        bool Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                        HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                       );


        if (Result)
            {
            // Polysegments may interact

            // For every segment of the self polysegment
            HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
            HGF2DPositionCollection::const_iterator PreviousItr(Itr);
            ++Itr;
            for (; !Answer && Itr != m_Points.end() ; ++Itr , ++PreviousItr)
                {
                // Obtain extent of this segment
                SelfXMin = MIN(Itr->GetX(), PreviousItr->GetX());
                SelfXMax = MAX(Itr->GetX(), PreviousItr->GetX());
                SelfYMin = MIN(Itr->GetY(), PreviousItr->GetY());
                SelfYMax = MAX(Itr->GetY(), PreviousItr->GetY());

                // Check if current segment and segment may interact
                Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                          HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                         );


                if (Result)
                    {
                    // Current segment and given polysegment may interact ...

                    // Create a lite segment to represent current segment
                    HGF2DLiteSegment TheLiteSegment(*PreviousItr, *Itr, Tolerance);

                    // For every segment of given polysegment
                    HGF2DPositionCollection::const_iterator GivenItr(pi_rPolySegment.m_Points.begin());
                    HGF2DPositionCollection::const_iterator PreviousGivenItr(GivenItr);
                    ++GivenItr;
                    for (; !Answer && GivenItr != pi_rPolySegment.m_Points.end() ; ++GivenItr , ++PreviousGivenItr)
                        {
                        // Check if current self segment and current given segment may interact
                        Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, MIN(GivenItr->GetX(), PreviousGivenItr->GetX()), Tolerance) &&
                                  HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, MAX(GivenItr->GetX(), PreviousGivenItr->GetX()), Tolerance) &&
                                  HDOUBLE_GREATER_OR_EQUAL(SelfYMax, MIN(GivenItr->GetY(), PreviousGivenItr->GetY()), Tolerance) &&
                                  HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, MAX(GivenItr->GetY(), PreviousGivenItr->GetY()), Tolerance)
                                 );


                        if (Result)
                            {
                            // The two segments may interact ... check if contiguous
                            Answer = HGF2DLiteSegment(*PreviousGivenItr,
                                                      *GivenItr,
                                                      Tolerance).Crosses(TheLiteSegment);
                            }
                        }
                    }
                }

#if (1)
            // Check if they did cross ...
            if (!Answer)
                {
                // They did not cross ... this does not mean they do not cross...
                // We have to check for special crossings at link points

                // For every pair of points of the self polysegment ...
                Itr = m_Points.begin();
                PreviousItr = Itr;
                ++Itr;

                // Create a location of first current point
                HGF2DLocation FirstPoint(PreviousItr->GetX(),
                                         PreviousItr->GetY(),
                                         GetCoordSys());

                for (; !Answer && Itr != m_Points.end() ; ++Itr , ++PreviousItr)
                    {
                    // Create location for second point
                    HGF2DLocation SecondPoint(Itr->GetX(), Itr->GetY(), GetCoordSys());

                    // Check if they intersect at this split point
                    Answer = IntersectsAtSplitPointWithPolySegment(pi_rPolySegment, FirstPoint, SecondPoint, true);

                    // Move second point to first
                    FirstPoint = SecondPoint;
                    }
                }

            // Check if they did cross ...
            if (!Answer)
                {
                // They did not cross ... this does not mean they do not cross...
                // We have to check for special crossings at link points
                // for given polysegment

                // For every pair of points of the self polysegment ...
                Itr = pi_rPolySegment.m_Points.begin();
                PreviousItr = Itr;
                ++Itr;

                // Create a location of first current point
                HGF2DLocation FirstPoint(PreviousItr->GetX(), PreviousItr->GetY(), GetCoordSys());

                for (; !Answer && Itr != pi_rPolySegment.m_Points.end() ; ++Itr , ++PreviousItr)
                    {
                    // Create location for second point
                    HGF2DLocation SecondPoint(Itr->GetX(), Itr->GetY(), GetCoordSys());

                    // Check if they intersect at this split point
                    Answer = pi_rPolySegment.IntersectsAtSplitPointWithPolySegment(*this,
                                                                                   FirstPoint,
                                                                                   SecondPoint,
                                                                                   true);

                    // Move second point to first
                    FirstPoint = SecondPoint;
                    }
                }
#else
            if (!Answer)
                {
                Answer = IntersectsAtAnySplitPointWithPolySegmentSCS(pi_rPolySegment);
                }
#endif
            }
        }


    return(Answer);
    }


//-----------------------------------------------------------------------------
// CrossesPolySegmentDPCS
// PRIVATE
// Indicates if the two polysegments cross each other
// The two polysegments must be related by a direction preserving relation
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::CrossesPolySegmentDPCS(const HVE2DPolySegment& pi_rPolySegment) const
    {
    HINVARIANTS;

    // They must already be expressed in coordinate systems which are related
    // through a direction preserving transformation model
    HPRECONDITION(GetCoordSys()->HasDirectionPreservingRelationTo(pi_rPolySegment.GetCoordSys()));

    // Declare answer variable
    bool Answer = false;

    // Pre-calculate tolerance
    double Tolerance = MIN(GetTolerance(), pi_rPolySegment.GetTolerance());

    // Check if polysegments are not NULL
    if (m_Points.size() >= 2 && pi_rPolySegment.m_Points.size() >= 2)
        {
        // Obtain extremes of poly segments in self coordinate systems
        HGF2DExtent PolyExtent(GetExtent());
        HGF2DExtent GivenPolyExtent(pi_rPolySegment.GetExtent());
        GivenPolyExtent.ChangeCoordSys(GetCoordSys());

        double SelfXMin = PolyExtent.GetXMin();
        double SelfXMax = PolyExtent.GetXMax();
        double SelfYMin = PolyExtent.GetYMin();
        double SelfYMax = PolyExtent.GetYMax();

        double GivenXMin = GivenPolyExtent.GetXMin();
        double GivenXMax = GivenPolyExtent.GetXMax();
        double GivenYMin = GivenPolyExtent.GetYMin();
        double GivenYMax = GivenPolyExtent.GetYMax();

        // Chek if their extents outter overlap
        bool Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                        HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                       );


        if (Result)
            {
            // Polysegments may interact

            // For every segment of the self polysegment
            HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
            HGF2DPositionCollection::const_iterator PreviousItr(Itr);
            ++Itr;
            for (; !Answer && Itr != m_Points.end() ; ++Itr , ++PreviousItr)
                {
                // Obtain extent of this segment
                SelfXMin = MIN(Itr->GetX(), PreviousItr->GetX());
                SelfXMax = MAX(Itr->GetX(), PreviousItr->GetX());
                SelfYMin = MIN(Itr->GetY(), PreviousItr->GetY());
                SelfYMax = MAX(Itr->GetY(), PreviousItr->GetY());

                // Check if current segment and segment may interact
                Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                          HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                         );


                if (Result)
                    {

                    // Current segment and given polysegment may interact ...

                    // Create a lite segment to represent current segment
                    HGF2DLiteSegment TheLiteSegment(*PreviousItr, *Itr, Tolerance);

                    // For every segment of given polysegment
                    HGF2DPositionCollection::const_iterator GivenItr(pi_rPolySegment.m_Points.begin());
                    HGF2DPositionCollection::const_iterator PreviousGivenItr(GivenItr);
                    ++GivenItr;
                    for (; !Answer && GivenItr != pi_rPolySegment.m_Points.end() ; ++GivenItr , ++PreviousGivenItr)
                        {
                        // Create location in self coordinate system
                        HGF2DLocation CurGivenSegStartLocation(*PreviousGivenItr, pi_rPolySegment.GetCoordSys());
                        CurGivenSegStartLocation.ChangeCoordSys(GetCoordSys());
                        HGF2DLocation CurGivenSegEndLocation(*GivenItr, pi_rPolySegment.GetCoordSys());
                        CurGivenSegEndLocation.ChangeCoordSys(GetCoordSys());

                        HGF2DPosition CurGivenSegStartPosition(CurGivenSegStartLocation.GetPosition());
                        HGF2DPosition CurGivenSegEndPosition(CurGivenSegEndLocation.GetPosition());

                        // Check if current self segment and current given segment may interact
                        Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, MIN(CurGivenSegEndPosition.GetX(), CurGivenSegStartPosition.GetX()), Tolerance) &&
                                  HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, MAX(CurGivenSegEndPosition.GetX(), CurGivenSegStartPosition.GetX()), Tolerance) &&
                                  HDOUBLE_GREATER_OR_EQUAL(SelfYMax, MIN(CurGivenSegEndPosition.GetY(), CurGivenSegStartPosition.GetY()), Tolerance) &&
                                  HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, MAX(CurGivenSegEndPosition.GetY(), CurGivenSegStartPosition.GetY()), Tolerance)
                                 );


                        if (Result)
                            {
                            // The two segments may interact ... check if contiguous
                            Answer = HGF2DLiteSegment(CurGivenSegStartPosition,
                                                      CurGivenSegEndPosition,
                                                      Tolerance).Crosses(TheLiteSegment);
                            }
                        }
                    }
                }

            // Check if they did cross ...
            if (!Answer)
                {
                // They did not cross ... this does not mean they do not cross...
                // We have to check for special crossings at link points

                // For every pair of points of the self polysegment ...
                Itr = m_Points.begin();
                PreviousItr = Itr;
                ++Itr;

                // Create a location of first current point
                HGF2DLocation FirstPoint(PreviousItr->GetX(), PreviousItr->GetY(), GetCoordSys());

                for (; !Answer && Itr != m_Points.end() ; ++Itr , ++PreviousItr)
                    {
                    // Create location for second point
                    HGF2DLocation SecondPoint(Itr->GetX(), Itr->GetY(), GetCoordSys());

                    // Check if they intersect at this split point
                    Answer = IntersectsAtSplitPoint(pi_rPolySegment, FirstPoint, SecondPoint, true);

                    // Move second point to first
                    FirstPoint = SecondPoint;
                    }

                }

            // Check if they did cross ...
            if (!Answer)
                {
                // They did not cross ... this does not mean they do not cross...
                // We have to check for special crossings at link points
                // for given polysegment

                // For every pair of points of the self polysegment ...
                Itr = pi_rPolySegment.m_Points.begin();
                PreviousItr = Itr;
                ++Itr;

                // Create a location of first current point
                HGF2DLocation FirstPoint(PreviousItr->GetX(), PreviousItr->GetY(), pi_rPolySegment.GetCoordSys());

                for (; !Answer && Itr != pi_rPolySegment.m_Points.end() ; ++Itr , ++PreviousItr)
                    {
                    // Create location for second point
                    HGF2DLocation SecondPoint(Itr->GetX(), Itr->GetY(), pi_rPolySegment.GetCoordSys());

                    // Check if they intersect at this split point
                    Answer = pi_rPolySegment.IntersectsAtSplitPoint(*this, FirstPoint, SecondPoint, true);

                    // Move second point to first
                    FirstPoint = SecondPoint;
                    }
                }
            }
        }

    return(Answer);
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

    // Declare answer variable
    bool Answer = false;

    // Pre-calculate tolerance
    double Tolerance = MIN(GetTolerance(), pi_rSegment.GetTolerance());

    // Check if polysegment is not NULL
    if (m_Points.size() >= 2)
        {
        // Obtain extremes of poly segment
        HGF2DExtent PolyExtent(GetExtent());

        double SelfXMin = PolyExtent.GetXMin();
        double SelfXMax = PolyExtent.GetXMax();
        double SelfYMin = PolyExtent.GetYMin();
        double SelfYMax = PolyExtent.GetYMax();

        double GivenXMin = MIN(pi_rSegment.GetStartPoint().GetX(),
                                pi_rSegment.GetEndPoint().GetX());
        double GivenXMax = MAX(pi_rSegment.GetStartPoint().GetX(),
                                pi_rSegment.GetEndPoint().GetX());
        double GivenYMin = MIN(pi_rSegment.GetStartPoint().GetY(),
                                pi_rSegment.GetEndPoint().GetY());
        double GivenYMax = MAX(pi_rSegment.GetStartPoint().GetY(),
                                pi_rSegment.GetEndPoint().GetY());

        // Check if their extent outter overlap
        bool Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                        HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                       );


        if (Result)
            {
            // Polysegment and segment may interact

            // Create a lite segment to represent segment
            HGF2DLiteSegment TheLiteSegment(pi_rSegment.GetStartPoint().GetPosition(),
                                            pi_rSegment.GetEndPoint().GetPosition(),
                                            Tolerance);


            // For every segment of the polysegment
            HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
            HGF2DPositionCollection::const_iterator PreviousItr(Itr);
            ++Itr;
            for (; !Answer && Itr != m_Points.end() ; ++Itr , ++PreviousItr)
                {
                // Obtain extent of this segment
                SelfXMin = MIN(Itr->GetX(), PreviousItr->GetX());
                SelfXMax = MAX(Itr->GetX(), PreviousItr->GetX());
                SelfYMin = MIN(Itr->GetY(), PreviousItr->GetY());
                SelfYMax = MAX(Itr->GetY(), PreviousItr->GetY());

                // Check if current segment and segment may interact
                Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                          HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                         );


                if (Result)
                    {
                    // The two segments may interact ... check if they cross
                    Answer = HGF2DLiteSegment(*PreviousItr, *Itr, Tolerance).Crosses(TheLiteSegment);
                    }
                }

            // Check if they did cross ...
            if (!Answer)
                {
                // They did not cross ... this does not mean they do not cross...
                // We have to check for special crossings at link points

                // For every pair of points of the polysegment ...
                Itr = m_Points.begin();
                PreviousItr = Itr;
                ++Itr;

                // Create a location of first current point
                HGF2DLocation FirstPoint(PreviousItr->GetX(), PreviousItr->GetY(), GetCoordSys());

                for (; !Answer && Itr != m_Points.end() ; ++Itr , ++PreviousItr)
                    {
                    // Create location for second point
                    HGF2DLocation SecondPoint(Itr->GetX(), Itr->GetY(), GetCoordSys());

                    // Check if they intersect at this split point
                    Answer = IntersectsAtSplitPoint(pi_rSegment, FirstPoint, SecondPoint, true);

                    // Move second point to first
                    FirstPoint = SecondPoint;
                    }
                }
            }
        }

    return(Answer);
    }


//-----------------------------------------------------------------------------
// CrossesSegmentDPCS
// PRIVATE
// Indicates if the polysegment and segment cross each other
// The two polysegments must be related by a direction preserving relation
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::CrossesSegmentDPCS(const HVE2DSegment& pi_rSegment) const
    {
    HINVARIANTS;

    // They must already be expressed in coordinate systems which are related
    // through a linearity preserving transformation model
    HPRECONDITION(GetCoordSys()->HasDirectionPreservingRelationTo(pi_rSegment.GetCoordSys()));

    // Declare answer variable
    bool Answer = false;

    // Pre-calculate tolerance
    double Tolerance = MIN(GetTolerance(), pi_rSegment.GetTolerance());

    // Obtain coordinates of segment in polysegment coordinate system
    HGF2DLocation SegmentStartLocation(pi_rSegment.GetStartPoint(), GetCoordSys());
    HGF2DLocation SegmentEndLocation(pi_rSegment.GetEndPoint(), GetCoordSys());

    // Transform into positions
    HGF2DPosition SegmentStartPosition(SegmentStartLocation.GetPosition());
    HGF2DPosition SegmentEndPosition(SegmentEndLocation.GetPosition());

    // Check if polysegment is not NULL
    if (m_Points.size() >= 2)
        {
        // Obtain extremes of poly segment
        HGF2DExtent PolyExtent(GetExtent());

        double SelfXMin = PolyExtent.GetXMin();
        double SelfXMax = PolyExtent.GetXMax();
        double SelfYMin = PolyExtent.GetYMin();
        double SelfYMax = PolyExtent.GetYMax();

        double GivenXMin = MIN(SegmentStartPosition.GetX(), SegmentEndPosition.GetX());
        double GivenXMax = MAX(SegmentStartPosition.GetX(), SegmentEndPosition.GetX());
        double GivenYMin = MIN(SegmentStartPosition.GetY(), SegmentEndPosition.GetY());
        double GivenYMax = MAX(SegmentStartPosition.GetY(), SegmentEndPosition.GetY());

        // Check if their extents outter overlap
        bool Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                        HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                       );

        if (Result)
            {
            // Polysegment and segment may interact

            // Create a lite segment to represent segment
            HGF2DLiteSegment TheLiteSegment(SegmentStartPosition,
                                            SegmentEndPosition,
                                            Tolerance);

            // For every segment of the polysegment
            HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
            HGF2DPositionCollection::const_iterator PreviousItr(Itr);
            ++Itr;
            for (; !Answer && Itr != m_Points.end() ; ++Itr , ++PreviousItr)
                {
                // Obtain extent of this segment
                SelfXMin = MIN(Itr->GetX(), PreviousItr->GetX());
                SelfXMax = MAX(Itr->GetX(), PreviousItr->GetX());
                SelfYMin = MIN(Itr->GetY(), PreviousItr->GetY());
                SelfYMax = MAX(Itr->GetY(), PreviousItr->GetY());

                // Check if current segment and segment may interact
                Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                          HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                         );


                if (Result)
                    {
                    // The two segments may interact ... check if they cross
                    Answer = HGF2DLiteSegment(*PreviousItr, *Itr, Tolerance).Crosses(TheLiteSegment);
                    }
                }

            // Check if they did cross ...
            if (!Answer)
                {
                // They did not cross ... this does not mean they do not cross...
                // We have to check for special crossings at link points

                // For every pair of points of the polysegment ...
                Itr = m_Points.begin();
                PreviousItr = Itr;
                ++Itr;

                // Create a location of first current point
                HGF2DLocation FirstPoint(PreviousItr->GetX(), PreviousItr->GetY(), GetCoordSys());

                for (; !Answer && Itr != m_Points.end() ; ++Itr , ++PreviousItr)
                    {
                    // Create location for second point
                    HGF2DLocation SecondPoint(Itr->GetX(), Itr->GetY(), GetCoordSys());

                    // Check if they intersect at this split point
                    Answer = IntersectsAtSplitPoint(pi_rSegment, FirstPoint, SecondPoint, true);

                    // Move second point to first
                    FirstPoint = SecondPoint;
                    }
                }
            }
        }

    return(Answer);
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

        // We check if it is direction preserving
        if (GetCoordSys()->HasDirectionPreservingRelationTo(pi_rPolySegment.GetCoordSys()))
            {
            // They are related through a linearity preserving model ... go ahead
            Answer = CrossesPolySegmentDPCS(pi_rPolySegment);
            }
        else
            {
            // They are not related by a linearity preserving model ... we must allocate a copy
            // In correct coordinate system
            HAutoPtr<HVE2DVector> pTempVector(pi_rPolySegment.AllocateCopyInCoordSys(GetCoordSys()));

            // Obtain answer from newly allocated vector
            Answer = pTempVector->Crosses(*this);
            }
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

        // We check if it is linearity preserving
        if (GetCoordSys()->HasDirectionPreservingRelationTo(pi_rSegment.GetCoordSys()))
            {
            // They are related through a linearity preserving model ... go ahead
            Answer = CrossesSegmentDPCS(pi_rSegment);
            }
        else
            {
            // They are not related by a linearity preserving model ... we must allocate a copy
            // In correct coordinate system
            HAutoPtr<HVE2DVector> pTempVector(pi_rSegment.AllocateCopyInCoordSys(GetCoordSys()));

            // Obtain answer from newly allocated vector
            Answer = pTempVector->Crosses(*this);
            }
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

    // Declare answer variable
    bool Answer = false;

    // Pre-calculate tolerance
    double Tolerance = MIN(GetTolerance(), pi_rPolySegment.GetTolerance());

    // Check if polysegments are not NULL
    if (m_Points.size() >= 2 && pi_rPolySegment.m_Points.size() >= 2)
        {
        // Obtain extremes of self and given poly segments
        HGF2DExtent PolyExtent(GetExtent());
        HGF2DExtent GivenPolyExtent(pi_rPolySegment.GetExtent());

        double SelfXMin = PolyExtent.GetXMin();
        double SelfXMax = PolyExtent.GetXMax();
        double SelfYMin = PolyExtent.GetYMin();
        double SelfYMax = PolyExtent.GetYMax();

        double GivenXMin = GivenPolyExtent.GetXMin();
        double GivenXMax = GivenPolyExtent.GetXMax();
        double GivenYMin = GivenPolyExtent.GetYMin();
        double GivenYMax = GivenPolyExtent.GetYMax();


        // Check if extents outter overlap
        bool Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                        HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                       );


        if (Result)
            {

            // Polysegments may interact

            // For every segment of the self polysegment
            HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
            HGF2DPositionCollection::const_iterator PreviousItr(Itr);
            ++Itr;

            for (; !Answer && Itr != m_Points.end() ; ++Itr , ++PreviousItr)
                {
                // Obtain extent of this segment
                SelfXMin = MIN(Itr->GetX(), PreviousItr->GetX());
                SelfXMax = MAX(Itr->GetX(), PreviousItr->GetX());
                SelfYMin = MIN(Itr->GetY(), PreviousItr->GetY());
                SelfYMax = MAX(Itr->GetY(), PreviousItr->GetY());

                // Check if current segment and segment may interact
                Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                          HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                         );


                if (Result)
                    {
                    // Current segment and given polysegment may interact ...

                    // Create a lite segment to represent current segment
                    HGF2DLiteSegment TheLiteSegment(*PreviousItr, *Itr, Tolerance);

                    // For every segment of given polysegment
                    HGF2DPositionCollection::const_iterator GivenItr(pi_rPolySegment.m_Points.begin());
                    HGF2DPositionCollection::const_iterator PreviousGivenItr(GivenItr);
                    ++GivenItr;
                    for (; !Answer && GivenItr != pi_rPolySegment.m_Points.end() ; ++GivenItr , ++PreviousGivenItr)
                        {
                        // Check if current self segment and current given segment may interact
                        Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, MIN(GivenItr->GetX(), PreviousGivenItr->GetX()), Tolerance) &&
                                  HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, MAX(GivenItr->GetX(), PreviousGivenItr->GetX()), Tolerance) &&
                                  HDOUBLE_GREATER_OR_EQUAL(SelfYMax, MIN(GivenItr->GetY(), PreviousGivenItr->GetY()), Tolerance) &&
                                  HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, MAX(GivenItr->GetY(), PreviousGivenItr->GetY()), Tolerance)
                                 );


                        if (Result)
                            {
                            // The two segments may interact ... check if contiguous
                            Answer = HGF2DLiteSegment(*PreviousGivenItr,
                                                      *GivenItr,
                                                      Tolerance).AreAdjacent(TheLiteSegment);
                            }
                        }
                    }
                }
            }
        }

    return(Answer);
    }

//-----------------------------------------------------------------------------
// IsAdjacentToPolySegmentLRCS
// PRIVATE
// Indicates if the polysegment adjacent to given polysegment
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::IsAdjacentToPolySegmentLRCS(const HVE2DPolySegment& pi_rPolySegment) const
    {
    HINVARIANTS;

    // They must already be expressed in coordinate systems which are related
    // through a linearity preserving transformation model
    HPRECONDITION(GetCoordSys()->HasLinearityPreservingRelationTo(pi_rPolySegment.GetCoordSys()));

#if (0)
    // For the moment, we consider contiguous segments as adjacent.
    //HChk AR ????
    return(IsContiguousToPolySegmentLRCS(pi_rPolySegment));
#else
    // Declare answer variable
    bool Answer = false;

    // Pre-calculate tolerance
    double Tolerance = MIN(GetTolerance(), pi_rPolySegment.GetTolerance());

    // Check if polysegments are not NULL
    if (m_Points.size() >= 2 && pi_rPolySegment.m_Points.size() >= 2)
        {
        // Obtain extremes of self and given poly segments
        HGF2DExtent PolyExtent(GetExtent());
        HGF2DExtent GivenPolyExtent(pi_rPolySegment.GetExtent());
        GivenPolyExtent.ChangeCoordSys(GetCoordSys());

        double SelfXMin = PolyExtent.GetXMin();
        double SelfXMax = PolyExtent.GetXMax();
        double SelfYMin = PolyExtent.GetYMin();
        double SelfYMax = PolyExtent.GetYMax();

        double GivenXMin = GivenPolyExtent.GetXMin();
        double GivenXMax = GivenPolyExtent.GetXMax();
        double GivenYMin = GivenPolyExtent.GetYMin();
        double GivenYMax = GivenPolyExtent.GetYMax();


        // Check if extents outter overlap
        bool Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                        HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                       );


        if (Result)
            {

            // Polysegments may interact

            // For every segment of the self polysegment
            HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
            HGF2DPositionCollection::const_iterator PreviousItr(Itr);
            ++Itr;

            for (; !Answer && Itr != m_Points.end() ; ++Itr , ++PreviousItr)
                {
                // Obtain extent of this segment
                SelfXMin = MIN(Itr->GetX(), PreviousItr->GetX());
                SelfXMax = MAX(Itr->GetX(), PreviousItr->GetX());
                SelfYMin = MIN(Itr->GetY(), PreviousItr->GetY());
                SelfYMax = MAX(Itr->GetY(), PreviousItr->GetY());

                // Check if current segment and segment may interact
                Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                          HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                         );


                if (Result)
                    {
                    // Current segment and given polysegment may interact ...

                    // Create a lite segment to represent current segment
                    HGF2DLiteSegment TheLiteSegment(*PreviousItr, *Itr, Tolerance);

                    // For every segment of given polysegment
                    HGF2DPositionCollection::const_iterator GivenItr(pi_rPolySegment.m_Points.begin());
                    HGF2DPositionCollection::const_iterator PreviousGivenItr(GivenItr);
                    ++GivenItr;
                    for (; !Answer && GivenItr != pi_rPolySegment.m_Points.end() ; ++GivenItr , ++PreviousGivenItr)
                        {
                        // Create location in self coordinate system
                        HGF2DLocation CurGivenSegStartLocation(*PreviousGivenItr, pi_rPolySegment.GetCoordSys());
                        CurGivenSegStartLocation.ChangeCoordSys(GetCoordSys());
                        HGF2DLocation CurGivenSegEndLocation(*GivenItr, pi_rPolySegment.GetCoordSys());
                        CurGivenSegEndLocation.ChangeCoordSys(GetCoordSys());

                        HGF2DPosition CurGivenSegStartPosition(CurGivenSegStartLocation.GetPosition());
                        HGF2DPosition CurGivenSegEndPosition(CurGivenSegEndLocation.GetPosition());

                        // Check if current self segment and current given segment may interact
                        Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, MIN(CurGivenSegEndPosition.GetX(), CurGivenSegStartPosition.GetX()), Tolerance) &&
                                  HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, MAX(CurGivenSegEndPosition.GetX(), CurGivenSegStartPosition.GetX()), Tolerance) &&
                                  HDOUBLE_GREATER_OR_EQUAL(SelfYMax, MIN(CurGivenSegEndPosition.GetY(), CurGivenSegStartPosition.GetY()), Tolerance) &&
                                  HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, MAX(CurGivenSegEndPosition.GetY(), CurGivenSegStartPosition.GetY()), Tolerance)
                                 );

                        if (Result)
                            {
                            // The two segments may interact ... check if adjacent
                            Answer = HGF2DLiteSegment(CurGivenSegStartPosition,
                                                      CurGivenSegEndPosition,
                                                      Tolerance).AreAdjacent(TheLiteSegment);
                            }
                        }
                    }
                }
            }
        }

    return(Answer);

#endif
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

#if (0)
    // For the moment, we consider contiguous segments as adjacent.
    //HChk AR ????
    return(IsContiguousToSegmentSCS(pi_rSegment));
#else

    // Declare answer variable
    bool Answer = false;

    // Pre-calculate tolerance
    double Tolerance = MIN(GetTolerance(), pi_rSegment.GetTolerance());

    // Check if polysegment is not NULL
    if (m_Points.size() >= 2)
        {
        // Obtain extremes of poly segment
        HGF2DExtent PolyExtent(GetExtent());

        double SelfXMin = PolyExtent.GetXMin();
        double SelfXMax = PolyExtent.GetXMax();
        double SelfYMin = PolyExtent.GetYMin();
        double SelfYMax = PolyExtent.GetYMax();

        // Obtain extremes of segment
        double GivenXMin = MIN(pi_rSegment.GetStartPoint().GetX(),
                                pi_rSegment.GetEndPoint().GetX());
        double GivenXMax = MAX(pi_rSegment.GetStartPoint().GetX(),
                                pi_rSegment.GetEndPoint().GetX());
        double GivenYMin = MIN(pi_rSegment.GetStartPoint().GetY(),
                                pi_rSegment.GetEndPoint().GetY());
        double GivenYMax = MAX(pi_rSegment.GetStartPoint().GetY(),
                                pi_rSegment.GetEndPoint().GetY());


        // Check if segment and polysegment extents outter overlap
        bool Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                        HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                       );


        if (Result)
            {
            // Polysegment and segment may interact

            // Create a lite segment to represent segment
            HGF2DLiteSegment TheLiteSegment(pi_rSegment.GetStartPoint().GetPosition(),
                                            pi_rSegment.GetEndPoint().GetPosition(),
                                            Tolerance);


            // For every segment of the polysegment
            HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
            HGF2DPositionCollection::const_iterator PreviousItr(Itr);
            ++Itr;
            for (; !Answer && Itr != m_Points.end() ; ++Itr , ++PreviousItr)
                {
                // Obtain extent of this segment
                SelfXMin = MIN(Itr->GetX(), PreviousItr->GetX());
                SelfXMax = MAX(Itr->GetX(), PreviousItr->GetX());
                SelfYMin = MIN(Itr->GetY(), PreviousItr->GetY());
                SelfYMax = MAX(Itr->GetY(), PreviousItr->GetY());

                // Check if current segment and segment may interact
                Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                          HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                         );


                if (Result)
                    {
                    // The two segments may interact ... check if contiguous
                    Answer = HGF2DLiteSegment(*PreviousItr,
                                              *Itr,
                                              Tolerance).AreAdjacent(TheLiteSegment);
                    }
                }
            }
        }

    return(Answer);

#endif
    }

//-----------------------------------------------------------------------------
// IsAdjacentToSegmentLRCS
// PRIVATE
// Indicates if the polysegment adjacent to given segment
// They must already be expressed in coordinate systems which are related
// through a linearity preserving transformation model
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::IsAdjacentToSegmentLRCS(const HVE2DSegment& pi_rSegment) const
    {
    HINVARIANTS;

    // They must already be expressed in coordinate systems which are related
    // through a linearity preserving transformation model
    HPRECONDITION(GetCoordSys()->HasLinearityPreservingRelationTo(pi_rSegment.GetCoordSys()));

#if (0)
    // For the moment, we consider contiguous segments as adjacent.
    //HChk AR ????
    return(IsContiguousToSegmentLRCS(pi_rSegment));
#else
    // Declare answer variable
    bool Answer = false;

    // Pre-calculate tolerance
    double Tolerance = MIN(GetTolerance(), pi_rSegment.GetTolerance());

    // Obtain coordinates of segment in polysegment coordinate system
    HGF2DLocation SegmentStartLocation(pi_rSegment.GetStartPoint(), GetCoordSys());
    HGF2DLocation SegmentEndLocation(pi_rSegment.GetEndPoint(), GetCoordSys());

    // Transform into positions
    HGF2DPosition SegmentStartPosition(SegmentStartLocation.GetPosition());
    HGF2DPosition SegmentEndPosition(SegmentEndLocation.GetPosition());

    // Check if polysegment is not NULL
    if (m_Points.size() >= 2)
        {
        // Obtain extremes of poly segment
        HGF2DExtent PolyExtent(GetExtent());

        double SelfXMin = PolyExtent.GetXMin();
        double SelfXMax = PolyExtent.GetXMax();
        double SelfYMin = PolyExtent.GetYMin();
        double SelfYMax = PolyExtent.GetYMax();

        // Obtain segment extremes
        double GivenXMin = MIN(SegmentStartPosition.GetX(), SegmentEndPosition.GetX());
        double GivenXMax = MAX(SegmentStartPosition.GetX(), SegmentEndPosition.GetX());
        double GivenYMin = MIN(SegmentStartPosition.GetY(), SegmentEndPosition.GetY());
        double GivenYMax = MAX(SegmentStartPosition.GetY(), SegmentEndPosition.GetY());

        // Check is segment and polysegment extent outter overlap
        bool Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                        HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                       );


        if (Result)
            {
            // Polysegment and segment may interact

            // Create a lite segment to represent segment
            HGF2DLiteSegment TheLiteSegment(SegmentStartPosition,
                                            SegmentEndPosition,
                                            Tolerance);

            // For every segment of the polysegment
            HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
            HGF2DPositionCollection::const_iterator PreviousItr(Itr);
            ++Itr;
            for (; !Answer && Itr != m_Points.end() ; ++Itr , ++PreviousItr)
                {
                // Obtain extent of this segment
                SelfXMin = MIN(Itr->GetX(), PreviousItr->GetX());
                SelfXMax = MAX(Itr->GetX(), PreviousItr->GetX());
                SelfYMin = MIN(Itr->GetY(), PreviousItr->GetY());
                SelfYMax = MAX(Itr->GetY(), PreviousItr->GetY());

                // Check if current segment and segment may interact
                Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                          HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                         );


                if (Result)
                    {
                    // The two segments may interact ... check if contiguous
                    Answer = HGF2DLiteSegment(*PreviousItr,
                                              *Itr,
                                              Tolerance).AreAdjacent(TheLiteSegment);
                    }
                }
            }
        }

    return(Answer);

#endif
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

        // We check if it is linearity preserving
        if (GetCoordSys()->HasLinearityPreservingRelationTo(pi_rPolySegment.GetCoordSys()))
            {
            // They are related through a linearity preserving model ... go ahead
            Answer = IsAdjacentToPolySegmentLRCS(pi_rPolySegment);
            }
        else
            {
            // They are not related by a linearity preserving model ... we must allocate a copy
            // In correct coordinate system
            HAutoPtr<HVE2DVector> pTempVector(pi_rPolySegment.AllocateCopyInCoordSys(GetCoordSys()));

            // Obtain answer from newly allocated vector
            Answer = pTempVector->AreAdjacent(*this);
            }
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

        // We check if it is linearity preserving
        if (GetCoordSys()->HasLinearityPreservingRelationTo(pi_rSegment.GetCoordSys()))
            {
            // They are related through a linearity preserving model ... go ahead
            Answer = IsAdjacentToSegmentLRCS(pi_rSegment);
            }
        else
            {
            // They are not related by a linearity preserving model ... we must allocate a copy
            // In correct coordinate system
            HAutoPtr<HVE2DVector> pTempVector(pi_rSegment.AllocateCopyInCoordSys(GetCoordSys()));

            // Obtain answer from newly allocated vector
            Answer = pTempVector->AreAdjacent(*this);
            }
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

    // Declare variable for counting new cross points
    size_t  NumberOfNewPoints = 0;

    // Pre-calculate tolerance
    double Tolerance = MIN(GetTolerance(), pi_rSegment.GetTolerance());

    // Check if polysegment is not NULL
    if (m_Points.size() >= 2)
        {
        // Obtain extremes of poly segment
        HGF2DExtent PolyExtent(GetExtent());

        double SelfXMin = PolyExtent.GetXMin();
        double SelfXMax = PolyExtent.GetXMax();
        double SelfYMin = PolyExtent.GetYMin();
        double SelfYMax = PolyExtent.GetYMax();

        double GivenXMin = MIN(pi_rSegment.GetStartPoint().GetX(), pi_rSegment.GetEndPoint().GetX());
        double GivenXMax = MAX(pi_rSegment.GetStartPoint().GetX(), pi_rSegment.GetEndPoint().GetX());
        double GivenYMin = MIN(pi_rSegment.GetStartPoint().GetY(), pi_rSegment.GetEndPoint().GetY());
        double GivenYMax = MAX(pi_rSegment.GetStartPoint().GetY(), pi_rSegment.GetEndPoint().GetY());

        // Check if extents outter overlap
        bool Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                        HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                       );


        if (Result)
            {
            // Polysegment and segment may interact

            // Create a lite segment to represent segment
            HGF2DLiteSegment TheLiteSegment(pi_rSegment.GetStartPoint().GetPosition(),
                                            pi_rSegment.GetEndPoint().GetPosition(),
                                            Tolerance);


            // For every segment of the polysegment
            HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
            HGF2DPositionCollection::const_iterator PreviousItr(Itr);
            ++Itr;
            for (; Itr != m_Points.end() ; ++Itr , ++PreviousItr)
                {
                // Obtain extent of this segment
                SelfXMin = MIN(Itr->GetX(), PreviousItr->GetX());
                SelfXMax = MAX(Itr->GetX(), PreviousItr->GetX());
                SelfYMin = MIN(Itr->GetY(), PreviousItr->GetY());
                SelfYMax = MAX(Itr->GetY(), PreviousItr->GetY());

                // Check if current segment and segment may interact
                Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                          HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                         );


                if (Result)
                    {
                    // The two segments may interact ... check if they cross
                    HGF2DPosition CrossPoint;
                    if (HGF2DLiteSegment::CROSS_FOUND == HGF2DLiteSegment(*PreviousItr, *Itr, Tolerance).IntersectSegment(TheLiteSegment, &CrossPoint))
                        {
                        // They did cross ... add cross point to list
                        po_pCrossPoints->push_back(HGF2DLocation(CrossPoint, GetCoordSys()));

                        // Increment the number of cross point found
                        ++NumberOfNewPoints;
                        }
                    }
                }

            // Now we search for junction iuntersection points

            // For every pair of points of the polysegment ...
            Itr = m_Points.begin();
            PreviousItr = Itr;
            ++Itr;

            // Create a location of first current point
            HGF2DLocation FirstPoint(PreviousItr->GetX(), PreviousItr->GetY(), GetCoordSys());

            for (; Itr != m_Points.end() ; ++Itr , ++PreviousItr)
                {
                // Create location for second point
                HGF2DLocation SecondPoint(Itr->GetX(), Itr->GetY(), GetCoordSys());

                // Check if they intersect at this split point
                if (IntersectsAtSplitPoint(pi_rSegment, FirstPoint, SecondPoint, true))
                    {
                    // They cross
                    po_pCrossPoints->push_back(FirstPoint);

                    ++NumberOfNewPoints;
                    }

                // Move second point to first
                FirstPoint = SecondPoint;
                }
            }
        }

    return(NumberOfNewPoints);
    }

//-----------------------------------------------------------------------------
// IntersectSegmentDPCS
// PRIVATE
// Finds intersection point with segment
// The coordinate systems of vectors must be related by a direction preserving
// relation
//-----------------------------------------------------------------------------
size_t HVE2DPolySegment::IntersectSegmentDPCS(const HVE2DSegment& pi_rSegment,
                                              HGF2DLocationCollection* po_pCrossPoints) const
    {
    HINVARIANTS;

    // The coordinate systems of vectors must be related by a direction
    // preserving relation
    HPRECONDITION(GetCoordSys()->HasDirectionPreservingRelationTo(pi_rSegment.GetCoordSys()));

    size_t  NumberOfNewPoints = 0;

    // Pre-calculate tolerance
    double Tolerance = MIN(GetTolerance(), pi_rSegment.GetTolerance());

    // Obtain coordinates of segment in polysegment coordinate system
    HGF2DLocation SegmentStartLocation(pi_rSegment.GetStartPoint(), GetCoordSys());
    HGF2DLocation SegmentEndLocation(pi_rSegment.GetEndPoint(), GetCoordSys());

    // Transform into positions
    HGF2DPosition SegmentStartPosition(SegmentStartLocation.GetPosition());
    HGF2DPosition SegmentEndPosition(SegmentEndLocation.GetPosition());

    // Check if polysegment is not NULL
    if (m_Points.size() >= 2)
        {

        // Obtain extremes of poly segment
        HGF2DExtent PolyExtent(GetExtent());

        double SelfXMin = PolyExtent.GetXMin();
        double SelfXMax = PolyExtent.GetXMax();
        double SelfYMin = PolyExtent.GetYMin();
        double SelfYMax = PolyExtent.GetYMax();

        double GivenXMin = MIN(SegmentStartPosition.GetX(), SegmentEndPosition.GetX());
        double GivenXMax = MAX(SegmentStartPosition.GetX(), SegmentEndPosition.GetX());
        double GivenYMin = MIN(SegmentStartPosition.GetY(), SegmentEndPosition.GetY());
        double GivenYMax = MAX(SegmentStartPosition.GetY(), SegmentEndPosition.GetY());

        // Check if their extents outter overlap
        bool Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                        HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                       );


        if (Result)
            {
            // Polysegment and segment may interact

            // Create a lite segment to represent segment
            HGF2DLiteSegment TheLiteSegment(SegmentStartPosition,
                                            SegmentEndPosition,
                                            Tolerance);

            // For every segment of the polysegment
            HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
            HGF2DPositionCollection::const_iterator PreviousItr(Itr);
            ++Itr;
            for (; Itr != m_Points.end() ; ++Itr , ++PreviousItr)
                {
                // Obtain extent of this segment
                SelfXMin = MIN(Itr->GetX(), PreviousItr->GetX());
                SelfXMax = MAX(Itr->GetX(), PreviousItr->GetX());
                SelfYMin = MIN(Itr->GetY(), PreviousItr->GetY());
                SelfYMax = MAX(Itr->GetY(), PreviousItr->GetY());

                // Check if current segment and segment may interact
                Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                          HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                         );


                if (Result)
                    {
                    // The two segments may interact ... check if they cross
                    HGF2DPosition CrossPoint;
                    if (HGF2DLiteSegment::CROSS_FOUND == HGF2DLiteSegment(*PreviousItr, *Itr, Tolerance).IntersectSegment(TheLiteSegment, &CrossPoint))
                        {
                        // They did cross ... add cross point to list
                        po_pCrossPoints->push_back(HGF2DLocation(CrossPoint, GetCoordSys()));

                        // Increment the number of cross point found
                        ++NumberOfNewPoints;
                        }
                    }
                }

            // We have to check for special crossings at link points

            // For every pair of points of the polysegment ...
            Itr = m_Points.begin();
            PreviousItr = Itr;
            ++Itr;

            // Create a location of first current point
            HGF2DLocation FirstPoint(PreviousItr->GetX(), PreviousItr->GetY(), GetCoordSys());

            for (; Itr != m_Points.end() ; ++Itr , ++PreviousItr)
                {
                // Create location for second point
                HGF2DLocation SecondPoint(Itr->GetX(), Itr->GetY(), GetCoordSys());

                // Check if they intersect at this split point
                if (IntersectsAtSplitPoint(pi_rSegment, FirstPoint, SecondPoint, true))
                    {
                    // They cross
                    po_pCrossPoints->push_back(FirstPoint);

                    ++NumberOfNewPoints;
                    }

                // Move second point to first
                FirstPoint = SecondPoint;
                }

            }
        }

    return(NumberOfNewPoints);
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

    // Create recipient variable for number of new points
    size_t  NumberOfNewPoints = 0;

    // Pre-calculate tolerance
    double Tolerance = MIN(GetTolerance(), pi_rPolySegment.GetTolerance());

    // Check if polysegments are not NULL
    if (m_Points.size() >= 2 && pi_rPolySegment.m_Points.size() >= 2)
        {
        // Obtain extremes of polysegments
        HGF2DExtent PolyExtent(GetExtent());
        HGF2DExtent GivenPolyExtent(pi_rPolySegment.GetExtent());

        double SelfXMin = PolyExtent.GetXMin();
        double SelfXMax = PolyExtent.GetXMax();
        double SelfYMin = PolyExtent.GetYMin();
        double SelfYMax = PolyExtent.GetYMax();

        double GivenXMin = GivenPolyExtent.GetXMin();
        double GivenXMax = GivenPolyExtent.GetXMax();
        double GivenYMin = GivenPolyExtent.GetYMin();
        double GivenYMax = GivenPolyExtent.GetYMax();

        // Check if their extent outter overlap
        bool Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                        HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                       );


        if (Result)
            {
            // Polysegments may interact

            // For every segment of the polysegment
            HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
            HGF2DPositionCollection::const_iterator PreviousItr(Itr);
            ++Itr;
            for (; Itr != m_Points.end() ; ++Itr , ++PreviousItr)
                {
                // Obtain extent of this segment
                SelfXMin = MIN(Itr->GetX(), PreviousItr->GetX());
                SelfXMax = MAX(Itr->GetX(), PreviousItr->GetX());
                SelfYMin = MIN(Itr->GetY(), PreviousItr->GetY());
                SelfYMax = MAX(Itr->GetY(), PreviousItr->GetY());

                // Check if current segment and segment may interact
                Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                          HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                         );


                if (Result)
                    {
                    // Current segment and given polysegment may interact ...

                    // Create a lite segment to represent current segment
                    HGF2DLiteSegment TheLiteSegment(*PreviousItr, *Itr, Tolerance);

                    // For every segment of given polysegment
                    HGF2DPositionCollection::const_iterator GivenItr(pi_rPolySegment.m_Points.begin());
                    HGF2DPositionCollection::const_iterator PreviousGivenItr(GivenItr);
                    ++GivenItr;
                    for (; GivenItr != pi_rPolySegment.m_Points.end() ; ++GivenItr , ++PreviousGivenItr)
                        {
                        // Check if current self segment and current given segment may interact
                        Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, MIN(GivenItr->GetX(), PreviousGivenItr->GetX()), Tolerance) &&
                                  HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, MAX(GivenItr->GetX(), PreviousGivenItr->GetX()), Tolerance) &&
                                  HDOUBLE_GREATER_OR_EQUAL(SelfYMax, MIN(GivenItr->GetY(), PreviousGivenItr->GetY()), Tolerance) &&
                                  HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, MAX(GivenItr->GetY(), PreviousGivenItr->GetY()), Tolerance)
                                 );


                        if (Result)
                            {
                            // The two segments may interact ... check if they cross
                            HGF2DPosition CrossPoint;
                            if (HGF2DLiteSegment::CROSS_FOUND == HGF2DLiteSegment(*PreviousGivenItr, *GivenItr, Tolerance).IntersectSegment(TheLiteSegment, &CrossPoint))
                                {
                                // They did cross ... add cross point to list
                                po_pCrossPoints->push_back(HGF2DLocation(CrossPoint, GetCoordSys()));

                                // Increment the number of cross point found
                                ++NumberOfNewPoints;
                                }
                            }
                        }
                    }
                }

            // They did not cross ... this does not mean they do not cross...
            // We have to check for special crossings at link points

            // For every pair of points of the polysegment ...
            Itr = m_Points.begin();
            PreviousItr = Itr;
            ++Itr;

            // Declare a work segment
            HVE2DSegment CurrentSegment(GetCoordSys());

            CurrentSegment.SetAutoToleranceActive(IsAutoToleranceActive());
            CurrentSegment.SetTolerance(GetTolerance());

            // Create a location of first current point
            HGF2DLocation FirstPoint(PreviousItr->GetX(), PreviousItr->GetY(), GetCoordSys());

            for (; Itr != m_Points.end() ; ++Itr , ++PreviousItr)
                {
                // Create location for second point
                HGF2DLocation SecondPoint(Itr->GetX(), Itr->GetY(), GetCoordSys());

                // Check if they intersect at this split point
                if (IntersectsAtSplitPointWithPolySegment(pi_rPolySegment, FirstPoint, SecondPoint, true))
                    {
                    // They cross
                    po_pCrossPoints->push_back(FirstPoint);

                    ++NumberOfNewPoints;
                    }

                // For each of self segment ... also search in other polysegment
                CurrentSegment.SetStartPoint(FirstPoint);
                CurrentSegment.SetEndPoint(SecondPoint);

                // For every pair of points of the given polysegment ...
                HGF2DPositionCollection::const_iterator GivenItr(pi_rPolySegment.m_Points.begin());
                HGF2DPositionCollection::const_iterator GivenPreviousItr(GivenItr);
                ++GivenItr;

                // Create a location of first current point
                HGF2DLocation OtherFirstPoint(GivenPreviousItr->GetX(), GivenPreviousItr->GetY(), GetCoordSys());

                for (; GivenItr != pi_rPolySegment.m_Points.end() ; ++GivenItr , ++GivenPreviousItr)
                    {
                    // Create location for second point
                    HGF2DLocation OtherSecondPoint(GivenItr->GetX(), GivenItr->GetY(), GetCoordSys());

                    // Check if they intersect at this split point
                    if (pi_rPolySegment.IntersectsAtSplitPoint(CurrentSegment, OtherFirstPoint, OtherSecondPoint, true))
                        {
                        // They cross
                        po_pCrossPoints->push_back(OtherFirstPoint);

                        ++NumberOfNewPoints;
                        }
                    // Move second point to first
                    OtherFirstPoint = OtherSecondPoint;
                    }
                // Move second point to first
                FirstPoint = SecondPoint;
                }
            }
        }

    return(NumberOfNewPoints);
    }

//-----------------------------------------------------------------------------
// IntersectPolySegmentDPCS
// PRIVATE
// Finds intersection point with polysegment
// The coordinate systems of vectors must be related by a direction preserving
// relation
//-----------------------------------------------------------------------------
size_t HVE2DPolySegment::IntersectPolySegmentDPCS(const HVE2DPolySegment& pi_rPolySegment,
                                                  HGF2DLocationCollection* po_pCrossPoints) const
    {
    HINVARIANTS;

    // The coordinate systems of vectors must be related by a direction
    // preserving relation
    HPRECONDITION(GetCoordSys()->HasDirectionPreservingRelationTo(pi_rPolySegment.GetCoordSys()));

    // Declare recipient variable for number of new cross points
    size_t  NumberOfNewPoints = 0;

    // Pre-calculate tolerance
    double Tolerance = MIN(GetTolerance(), pi_rPolySegment.GetTolerance());

    // Check if polysegments are not NULL
    if (m_Points.size() >= 2 && pi_rPolySegment.m_Points.size() >= 2)
        {

        // Obtain extremes of polysegments expressed in self coordinate system
        HGF2DExtent PolyExtent(GetExtent());
        HGF2DExtent GivenPolyExtent(pi_rPolySegment.GetExtent());
        GivenPolyExtent.ChangeCoordSys(GetCoordSys());

        double SelfXMin = PolyExtent.GetXMin();
        double SelfXMax = PolyExtent.GetXMax();
        double SelfYMin = PolyExtent.GetYMin();
        double SelfYMax = PolyExtent.GetYMax();

        double GivenXMin = GivenPolyExtent.GetXMin();
        double GivenXMax = GivenPolyExtent.GetXMax();
        double GivenYMin = GivenPolyExtent.GetYMin();
        double GivenYMax = GivenPolyExtent.GetYMax();

        // Check if their extent outter overlap
        bool Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                        HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                       );


        if (Result)
            {
            // Polysegments may interact

            // For every segment of the polysegment
            HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
            HGF2DPositionCollection::const_iterator PreviousItr(Itr);
            ++Itr;

            for (; Itr != m_Points.end() ; ++Itr , ++PreviousItr)
                {
                // Obtain extent of this segment
                SelfXMin = MIN(Itr->GetX(), PreviousItr->GetX());
                SelfXMax = MAX(Itr->GetX(), PreviousItr->GetX());
                SelfYMin = MIN(Itr->GetY(), PreviousItr->GetY());
                SelfYMax = MAX(Itr->GetY(), PreviousItr->GetY());

                // Check if current segment and segment may interact
                Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                          HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                         );


                if (Result)
                    {
                    // Current segment and given polysegment may interact ...

                    // Create a lite segment to represent current segment
                    HGF2DLiteSegment TheLiteSegment(*PreviousItr, *Itr, Tolerance);

                    // For every segment of given polysegment
                    HGF2DPositionCollection::const_iterator GivenItr(pi_rPolySegment.m_Points.begin());
                    HGF2DPositionCollection::const_iterator PreviousGivenItr(GivenItr);
                    ++GivenItr;

                    for (; GivenItr != pi_rPolySegment.m_Points.end() ; ++GivenItr , ++PreviousGivenItr)
                        {
                        // Create location in self coordinate system
                        HGF2DLocation CurGivenSegStartLocation(*PreviousGivenItr, pi_rPolySegment.GetCoordSys());
                        CurGivenSegStartLocation.ChangeCoordSys(GetCoordSys());
                        HGF2DLocation CurGivenSegEndLocation(*GivenItr, pi_rPolySegment.GetCoordSys());
                        CurGivenSegEndLocation.ChangeCoordSys(GetCoordSys());

                        HGF2DPosition CurGivenSegStartPosition(CurGivenSegStartLocation.GetPosition());
                        HGF2DPosition CurGivenSegEndPosition(CurGivenSegEndLocation.GetPosition());

                        // Check if current self segment and current given segment may interact
                        Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, MIN(CurGivenSegEndPosition.GetX(), CurGivenSegStartPosition.GetX()), Tolerance) &&
                                  HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, MAX(CurGivenSegEndPosition.GetX(), CurGivenSegStartPosition.GetX()), Tolerance) &&
                                  HDOUBLE_GREATER_OR_EQUAL(SelfYMax, MIN(CurGivenSegEndPosition.GetY(), CurGivenSegStartPosition.GetY()), Tolerance) &&
                                  HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, MAX(CurGivenSegEndPosition.GetY(), CurGivenSegStartPosition.GetY()), Tolerance)
                                 );


                        if (Result)
                            {
                            // The two segments may interact ... check if they cross
                            HGF2DPosition CrossPoint;
                            if (HGF2DLiteSegment::CROSS_FOUND == HGF2DLiteSegment(CurGivenSegStartPosition, CurGivenSegEndPosition, Tolerance).IntersectSegment(TheLiteSegment, &CrossPoint))
                                {
                                // They did cross ... add cross point to list
                                po_pCrossPoints->push_back(HGF2DLocation(CrossPoint, GetCoordSys()));

                                // Increment the number of cross point found
                                ++NumberOfNewPoints;
                                }
                            }
                        }
                    }
                }

            // We have to check for special crossings at link points

            // Declare a work segment
            HVE2DSegment CurrentSegment(GetCoordSys());

            // For every pair of points of the polysegment ...
            Itr = m_Points.begin();
            PreviousItr = Itr;
            ++Itr;

            // Create a location of first current point
            HGF2DLocation FirstPoint(PreviousItr->GetX(), PreviousItr->GetY(), GetCoordSys());

            for (; Itr != m_Points.end() ; ++Itr , ++PreviousItr)
                {
                // Create location for second point
                HGF2DLocation SecondPoint(Itr->GetX(), Itr->GetY(), GetCoordSys());

                // Check if they intersect at this split point
                if (IntersectsAtSplitPoint(pi_rPolySegment, FirstPoint, SecondPoint, true))
                    {
                    // They cross
                    po_pCrossPoints->push_back(FirstPoint);

                    ++NumberOfNewPoints;
                    }


                // For each of self segment ... also search in other polysegment
                CurrentSegment.SetStartPoint(FirstPoint);
                CurrentSegment.SetEndPoint(SecondPoint);

                // For every pair of points of the given polysegment ...
                HGF2DPositionCollection::const_iterator GivenItr(pi_rPolySegment.m_Points.begin());
                HGF2DPositionCollection::const_iterator GivenPreviousItr(GivenItr);
                ++GivenItr;

                // Create a location of first current point
                HGF2DLocation OtherFirstPoint(GivenPreviousItr->GetX(), GivenPreviousItr->GetY(), pi_rPolySegment.GetCoordSys());

                for (; GivenItr != pi_rPolySegment.m_Points.end() ; ++GivenItr , ++GivenPreviousItr)
                    {
                    // Create location for second point
                    HGF2DLocation OtherSecondPoint(GivenItr->GetX(), GivenItr->GetY(), pi_rPolySegment.GetCoordSys());

                    // Check if they intersect at this split point
                    if (pi_rPolySegment.IntersectsAtSplitPoint(CurrentSegment, OtherFirstPoint, OtherSecondPoint, true))
                        {
                        // They cross
                        po_pCrossPoints->push_back(OtherFirstPoint);

                        ++NumberOfNewPoints;
                        }

                    // Move second point to first
                    OtherFirstPoint = OtherSecondPoint;
                    }

                // Move second point to first
                FirstPoint = SecondPoint;
                }
            }
        }

    return(NumberOfNewPoints);
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

        // We check if it is linearity preserving
        if (GetCoordSys()->HasDirectionPreservingRelationTo(pi_rPolySegment.GetCoordSys()))
            {
            // They are related through a direction preserving model ... go ahead
            NumberOfCrossPoints = IntersectPolySegmentDPCS(pi_rPolySegment, po_pCrossPoints);
            }
        else
            {
            // They are not related by a direction preserving model ... we must allocate a copy
            // In correct coordinate system
            HAutoPtr<HVE2DVector> pTempVector(pi_rPolySegment.AllocateCopyInCoordSys(GetCoordSys()));

            // Intersect
            NumberOfCrossPoints = pTempVector->Intersect(*this, po_pCrossPoints);
            }
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

        // We check if it is direction preserving
        if (GetCoordSys()->HasDirectionPreservingRelationTo(pi_rSegment.GetCoordSys()))
            {
            // They are related through a direction preserving model ... go ahead
            NumberOfCrossPoints = IntersectSegmentDPCS(pi_rSegment, po_pCrossPoints);
            }
        else
            {
            // They are not related by a direction preserving model ... we must allocate a copy
            // In correct coordinate system
            HAutoPtr<HVE2DVector> pTempVector(pi_rSegment.AllocateCopyInCoordSys(GetCoordSys()));

            // Intersect
            NumberOfCrossPoints = pTempVector->Intersect(*this, po_pCrossPoints);
            }
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

    // Pre-calculate tolerance
    double Tolerance = MIN(GetTolerance(), pi_rSegment.GetTolerance());

    // Save initial number of points
    size_t  InitialNumberOfPoints = po_pContiguousnessPoints->size();

    // Obtain extent of segment
    double GivenXMin = MIN(pi_rSegment.GetStartPoint().GetX(), pi_rSegment.GetEndPoint().GetX());
    double GivenXMax = MAX(pi_rSegment.GetStartPoint().GetX(), pi_rSegment.GetEndPoint().GetX());
    double GivenYMin = MIN(pi_rSegment.GetStartPoint().GetY(), pi_rSegment.GetEndPoint().GetY());
    double GivenYMax = MAX(pi_rSegment.GetStartPoint().GetY(), pi_rSegment.GetEndPoint().GetY());

    // No need to check extent overlap since we already know they are
    // contiguous

    // Create a lite segment to represent segment
    HGF2DLiteSegment TheLiteSegment(pi_rSegment.GetStartPoint().GetPosition(),
                                    pi_rSegment.GetEndPoint().GetPosition(),
                                    Tolerance);

    // For every pair of points of the polysegment ...
    HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
    HGF2DPositionCollection::const_iterator PreviousItr(Itr);
    ++Itr;

    for (; Itr != m_Points.end() ; ++Itr , ++PreviousItr)
        {
        // Check if current segment and segment may interact
        bool Result = (HDOUBLE_GREATER_OR_EQUAL(MAX(Itr->GetX(), PreviousItr->GetX()), GivenXMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(MIN(Itr->GetX(), PreviousItr->GetX()), GivenXMax, Tolerance) &&
                        HDOUBLE_GREATER_OR_EQUAL(MAX(Itr->GetY(), PreviousItr->GetY()), GivenYMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(MIN(Itr->GetY(), PreviousItr->GetY()), GivenYMax, Tolerance)
                       );


        if (Result)
            {
            // The two segments may interact ... check if contiguous

            // Create lite segment for current segment
            HGF2DLiteSegment PolySegment(*PreviousItr, *Itr, Tolerance);

            // Check if they are contiguous
            if (PolySegment.AreContiguous(TheLiteSegment))
                {
                // These two segments are effectively contiguous

                // Declare a position collection to receive contiguousness points
                HGF2DPositionCollection ContiguousnessPositions;

                // Obtain contiguousness points
                PolySegment.ObtainContiguousnessPoints(TheLiteSegment, &ContiguousnessPositions);

                // There must be exactly 2 points
                HASSERT(ContiguousnessPositions.size() == 2);

                // We check if first returned point is equal to last point obtained previously
                // if there were points added
                if (po_pContiguousnessPoints->size() > InitialNumberOfPoints)
                    {
                    // New contiguousness points were added
                    // We have previous points ... compare first of new one
                    // with last of previous
                    if ((*(po_pContiguousnessPoints->rbegin())).IsEqualTo(HGF2DLocation(ContiguousnessPositions[0], GetCoordSys()),
                                                                          Tolerance))
                        {
                        // These two are equal ... we remove previous last one
                        po_pContiguousnessPoints->pop_back();
                        }
                    else
                        {
                        // The two points are different ... we add first point
                        po_pContiguousnessPoints->push_back(HGF2DLocation(ContiguousnessPositions[0], GetCoordSys()));
                        }

                    // We add second point in all cases
                    po_pContiguousnessPoints->push_back(HGF2DLocation(ContiguousnessPositions[1], GetCoordSys()));
                    }
                else
                    {
                    // No new points were added previously ...
                    // We add the two points as are
                    po_pContiguousnessPoints->push_back(HGF2DLocation(ContiguousnessPositions[0], GetCoordSys()));
                    po_pContiguousnessPoints->push_back(HGF2DLocation(ContiguousnessPositions[1], GetCoordSys()));
                    }
                }
            }
        }

    return (po_pContiguousnessPoints->size() - InitialNumberOfPoints);
    }

//-----------------------------------------------------------------------------
// ObtainContiguousnessPointsWithSegmentLRCS
// PRIVATE
// Finds contiguousness points with segment
// The polysegment and segment must have coordinate systems related
// by a linearity preserving relation
//-----------------------------------------------------------------------------
size_t HVE2DPolySegment::ObtainContiguousnessPointsWithSegmentLRCS(const HVE2DSegment&  pi_rSegment,
                                                                   HGF2DLocationCollection* po_pContiguousnessPoints) const
    {
    HINVARIANTS;

    // The polysegment and segment must have coordinate systems related
    // by a linearity preserving relation
    HPRECONDITION(GetCoordSys()->HasLinearityPreservingRelationTo(pi_rSegment.GetCoordSys()));

    // Check that a recipient collection is provided
    HPRECONDITION(po_pContiguousnessPoints != 0);

    // Pre-calculate tolerance
    double Tolerance = MIN(GetTolerance(), pi_rSegment.GetTolerance());

    // Obtain coordinates of segment in polysegment coordinate system
    HGF2DLocation SegmentStartLocation(pi_rSegment.GetStartPoint(), GetCoordSys());
    HGF2DLocation SegmentEndLocation(pi_rSegment.GetEndPoint(), GetCoordSys());

    // Transform into positions
    HGF2DPosition SegmentStartPosition(SegmentStartLocation.GetPosition());
    HGF2DPosition SegmentEndPosition(SegmentEndLocation.GetPosition());

    // Save initial number of points
    size_t  InitialNumberOfPoints = po_pContiguousnessPoints->size();

    // Obtain extent of segment
    double GivenXMin = MIN(SegmentStartPosition.GetX(), SegmentEndPosition.GetX());
    double GivenXMax = MAX(SegmentStartPosition.GetX(), SegmentEndPosition.GetX());
    double GivenYMin = MIN(SegmentStartPosition.GetY(), SegmentEndPosition.GetY());
    double GivenYMax = MAX(SegmentStartPosition.GetY(), SegmentEndPosition.GetY());

    // No need to check extent overlap since we already know they are
    // contiguous

    // Create a lite segment to represent segment
    HGF2DLiteSegment TheLiteSegment(SegmentStartPosition,
                                    SegmentEndPosition,
                                    Tolerance);

    // For every pair of points of the polysegment ...
    HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
    HGF2DPositionCollection::const_iterator PreviousItr(Itr);
    ++Itr;

    for (; Itr != m_Points.end() ; ++Itr , ++PreviousItr)
        {
        // Check if current segment and segment may interact
        bool Result = (HDOUBLE_GREATER_OR_EQUAL(MAX(Itr->GetX(), PreviousItr->GetX()), GivenXMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(MIN(Itr->GetX(), PreviousItr->GetX()), GivenXMax, Tolerance) &&
                        HDOUBLE_GREATER_OR_EQUAL(MAX(Itr->GetY(), PreviousItr->GetY()), GivenYMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(MIN(Itr->GetY(), PreviousItr->GetY()), GivenYMax, Tolerance)
                       );


        if (Result)
            {
            // The two segments may interact ... check if contiguous

            // Create lite segment for current segment
            HGF2DLiteSegment PolySegment(*PreviousItr, *Itr, Tolerance);

            // Check if they are contiguous
            if (PolySegment.AreContiguous(TheLiteSegment))
                {
                // These two segments are effectively contiguous

                // Declare a position collection to receive contiguousness points
                HGF2DPositionCollection ContiguousnessPositions;

                // Obtain contiguousness points
                PolySegment.ObtainContiguousnessPoints(TheLiteSegment, &ContiguousnessPositions);

                // There must be exactly 2 points
                HASSERT(ContiguousnessPositions.size() == 2);

                // We check if first returned point is equal to last point obtained previously
                // if there were points added
                if (po_pContiguousnessPoints->size() > InitialNumberOfPoints)
                    {
                    // New contiguousness points were added
                    // We have previous points ... compare first of new one
                    // with last of previous
                    if ((*(po_pContiguousnessPoints->rbegin())).IsEqualTo(HGF2DLocation(ContiguousnessPositions[0], GetCoordSys()),
                                                                          Tolerance))
                        {
                        // These two are equal ... we remove previous last one
                        po_pContiguousnessPoints->pop_back();
                        }
                    else
                        {
                        // The two points are different ... we add first point
                        po_pContiguousnessPoints->push_back(HGF2DLocation(ContiguousnessPositions[0], GetCoordSys()));
                        }

                    // We add second point in all cases
                    po_pContiguousnessPoints->push_back(HGF2DLocation(ContiguousnessPositions[1], GetCoordSys()));
                    }
                else
                    {
                    // No new points were added previously ...
                    // We add the two points as are
                    po_pContiguousnessPoints->push_back(HGF2DLocation(ContiguousnessPositions[0], GetCoordSys()));
                    po_pContiguousnessPoints->push_back(HGF2DLocation(ContiguousnessPositions[1], GetCoordSys()));
                    }
                }
            }
        }

    return (po_pContiguousnessPoints->size() - InitialNumberOfPoints);
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

    // Pre-calculate tolerance
    double Tolerance = MIN(GetTolerance(), pi_rPolySegment.GetTolerance());

    // Save initial number of points
    size_t  InitialNumberOfPoints = po_pContiguousnessPoints->size();

    // Obtain extent of given polysegment
    HGF2DExtent GivenPolyExtent(pi_rPolySegment.GetExtent());

    double GivenXMin = GivenPolyExtent.GetXMin();
    double GivenXMax = GivenPolyExtent.GetXMax();
    double GivenYMin = GivenPolyExtent.GetYMin();
    double GivenYMax = GivenPolyExtent.GetYMax();

    // No need to check extent overlap since we already know they are
    // contiguous

    // For every pair of points of the polysegment ...
    HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
    HGF2DPositionCollection::const_iterator PreviousItr(Itr);
    ++Itr;

    for (; Itr != m_Points.end() ; ++Itr , ++PreviousItr)
        {
        // Obtain extent of this segment
        double SelfXMin = MIN(Itr->GetX(), PreviousItr->GetX());
        double SelfXMax = MAX(Itr->GetX(), PreviousItr->GetX());
        double SelfYMin = MIN(Itr->GetY(), PreviousItr->GetY());
        double SelfYMax = MAX(Itr->GetY(), PreviousItr->GetY());

        // Check if current segment and given polysegment may interact
        bool Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                        HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                       );


        if (Result)
            {
            // The two segments may interact ... check if contiguous

            // Create lite segment for current segment
            HGF2DLiteSegment PolySegment(*PreviousItr, *Itr, Tolerance);

            // For every segment of given polysegment
            HGF2DPositionCollection::const_iterator GivenItr(pi_rPolySegment.m_Points.begin());
            HGF2DPositionCollection::const_iterator PreviousGivenItr = GivenItr;
            ++GivenItr;
            for (; GivenItr != pi_rPolySegment.m_Points.end() ; ++GivenItr , ++PreviousGivenItr)
                {
                // Check if current self segment and current given segment may interact
                Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, MIN(GivenItr->GetX(), PreviousGivenItr->GetX()), Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, MAX(GivenItr->GetX(), PreviousGivenItr->GetX()), Tolerance) &&
                          HDOUBLE_GREATER_OR_EQUAL(SelfYMax, MIN(GivenItr->GetY(), PreviousGivenItr->GetY()), Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, MAX(GivenItr->GetY(), PreviousGivenItr->GetY()), Tolerance)
                         );

                if (Result)
                    {
                    // The two segments may interact ...

                    // Create given segment
                    HGF2DLiteSegment GivenSegment(*PreviousGivenItr, *GivenItr, Tolerance);

                    // Check if contiguous
                    if (PolySegment.AreContiguous(GivenSegment))
                        {
                        // These two segments are effectively contiguous

                        // Declare a position collection to receive contiguousness points
                        HGF2DPositionCollection ContiguousnessPositions;

                        // Obtain contiguousness points
                        PolySegment.ObtainContiguousnessPoints(GivenSegment, &ContiguousnessPositions);

                        // There must be exactly 2 points
                        HASSERT(ContiguousnessPositions.size() == 2);

                        // We check if first returned point is equal to last point obtained previously
                        // if there were points added
                        if (po_pContiguousnessPoints->size() > InitialNumberOfPoints)
                            {
                            // New contiguousness points were added
                            // We have previous points ... compare first of new one
                            // with last of previous
                            if ((*(po_pContiguousnessPoints->rbegin())).IsEqualTo(HGF2DLocation(ContiguousnessPositions[0], GetCoordSys()),
                                                                                  Tolerance))
                                {
                                // These two are equal ... we remove previous last one
                                po_pContiguousnessPoints->pop_back();

                                // We add second point
                                po_pContiguousnessPoints->push_back(HGF2DLocation(ContiguousnessPositions[1], GetCoordSys()));
                                }
                            // Compare second previous with last found
                            else if ((*po_pContiguousnessPoints)[po_pContiguousnessPoints->size() - 2].IsEqualTo(HGF2DLocation(ContiguousnessPositions[1], GetCoordSys()),
                                     Tolerance))
                                {
                                // These two are equal ... we change value of next to last
                                (*po_pContiguousnessPoints)[po_pContiguousnessPoints->size() - 2] = HGF2DLocation(ContiguousnessPositions[0], GetCoordSys());
                                }
                            else
                                {
                                // The two points are different ... we add points
                                po_pContiguousnessPoints->push_back(HGF2DLocation(ContiguousnessPositions[0], GetCoordSys()));
                                po_pContiguousnessPoints->push_back(HGF2DLocation(ContiguousnessPositions[1], GetCoordSys()));
                                }
                            }
                        else
                            {
                            // No new points were added previously ...
                            // We add the two points as are
                            po_pContiguousnessPoints->push_back(HGF2DLocation(ContiguousnessPositions[0], GetCoordSys()));
                            po_pContiguousnessPoints->push_back(HGF2DLocation(ContiguousnessPositions[1], GetCoordSys()));
                            }
                        }
                    }
                }
            }
        }

    return (po_pContiguousnessPoints->size() - InitialNumberOfPoints);
    }

//-----------------------------------------------------------------------------
// ObtainContiguousnessPointsWithPolySegmentLRCS
// PRIVATE
// Finds contiguousness points with poly segment
// The polysegments must have coordinate systems related
// by a linearity preserving relation
//-----------------------------------------------------------------------------
size_t HVE2DPolySegment::ObtainContiguousnessPointsWithPolySegmentLRCS(const HVE2DPolySegment&  pi_rPolySegment,
                                                                       HGF2DLocationCollection* po_pContiguousnessPoints) const
    {
    HINVARIANTS;

    // The polysegments must have coordinate systems related
    // by a linearity preserving relation
    HPRECONDITION(GetCoordSys()->HasLinearityPreservingRelationTo(pi_rPolySegment.GetCoordSys()));

    // The two vectors must share the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rPolySegment.GetCoordSys());

    // Check that a recipient collection is provided
    HPRECONDITION(po_pContiguousnessPoints != 0);

    // Pre-calculate tolerance
    double Tolerance = MIN(GetTolerance(), pi_rPolySegment.GetTolerance());

    // Save initial number of points
    size_t  InitialNumberOfPoints = po_pContiguousnessPoints->size();

    // Obtain extent of given polysegment in self coordinate system
    HGF2DExtent GivenPolyExtent(pi_rPolySegment.GetExtent());
    GivenPolyExtent.ChangeCoordSys(GetCoordSys());

    double GivenXMin = GivenPolyExtent.GetXMin();
    double GivenXMax = GivenPolyExtent.GetXMax();
    double GivenYMin = GivenPolyExtent.GetYMin();
    double GivenYMax = GivenPolyExtent.GetYMax();

    // No need to check extent overlap since we already know they are
    // contiguous

    // For every pair of points of the polysegment ...
    HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
    HGF2DPositionCollection::const_iterator PreviousItr = Itr;
    ++Itr;

    for (; Itr != m_Points.end() ; ++Itr , ++PreviousItr)
        {
        // Obtain extent of this segment
        double SelfXMin = MIN(Itr->GetX(), PreviousItr->GetX());
        double SelfXMax = MAX(Itr->GetX(), PreviousItr->GetX());
        double SelfYMin = MIN(Itr->GetY(), PreviousItr->GetY());
        double SelfYMax = MAX(Itr->GetY(), PreviousItr->GetY());

        // Check if current segment and given polysegment may interact
        bool Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                        HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                       );


        if (Result)
            {
            // The two segments may interact ... check if contiguous

            // Create lite segment for current segment
            HGF2DLiteSegment PolySegment(*PreviousItr, *Itr, Tolerance);

            // For every segment of given polysegment
            HGF2DPositionCollection::const_iterator GivenItr(pi_rPolySegment.m_Points.begin());
            HGF2DPositionCollection::const_iterator PreviousGivenItr = GivenItr;
            ++GivenItr;
            for (; GivenItr != pi_rPolySegment.m_Points.end() ; ++GivenItr , ++PreviousGivenItr)
                {
                // Create location in self coordinate system
                HGF2DLocation CurGivenSegStartLocation(*PreviousGivenItr, pi_rPolySegment.GetCoordSys());
                CurGivenSegStartLocation.ChangeCoordSys(GetCoordSys());
                HGF2DLocation CurGivenSegEndLocation(*GivenItr, pi_rPolySegment.GetCoordSys());
                CurGivenSegEndLocation.ChangeCoordSys(GetCoordSys());

                HGF2DPosition CurGivenSegStartPosition(CurGivenSegStartLocation.GetPosition());
                HGF2DPosition CurGivenSegEndPosition(CurGivenSegEndLocation.GetPosition());

                // Check if current self segment and current given segment may interact
                Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, MIN(CurGivenSegEndPosition.GetX(), CurGivenSegStartPosition.GetX()), Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, MAX(CurGivenSegEndPosition.GetX(), CurGivenSegStartPosition.GetX()), Tolerance) &&
                          HDOUBLE_GREATER_OR_EQUAL(SelfYMax, MIN(CurGivenSegEndPosition.GetY(), CurGivenSegStartPosition.GetY()), Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, MAX(CurGivenSegEndPosition.GetY(), CurGivenSegStartPosition.GetY()), Tolerance)
                         );


                if (Result)
                    {
                    // The two segments may interact ...

                    // Create given segment
                    HGF2DLiteSegment GivenSegment(CurGivenSegStartPosition, CurGivenSegEndPosition, Tolerance);

                    // Check if contiguous
                    if (PolySegment.AreContiguous(GivenSegment))
                        {
                        // These two segments are effectively contiguous

                        // Declare a position collection to receive contiguousness points
                        HGF2DPositionCollection ContiguousnessPositions;

                        // Obtain contiguousness points
                        PolySegment.ObtainContiguousnessPoints(GivenSegment, &ContiguousnessPositions);

                        // There must be exactly 2 points
                        HASSERT(ContiguousnessPositions.size() == 2);

                        // We check if first returned point is equal to last point obtained previously
                        // if there were points added
                        if (po_pContiguousnessPoints->size() > InitialNumberOfPoints)
                            {
                            // New contiguousness points were added
                            // We have previous points ... compare first of new one
                            // with last of previous
                            if ((*(po_pContiguousnessPoints->rbegin())).IsEqualTo(HGF2DLocation(ContiguousnessPositions[0], GetCoordSys()),
                                                                                  Tolerance))
                                {
                                // These two are equal ... we remove previous last one
                                po_pContiguousnessPoints->pop_back();

                                // We add second point
                                po_pContiguousnessPoints->push_back(HGF2DLocation(ContiguousnessPositions[1], GetCoordSys()));

                                }
                            // Compare second previous with last found
                            else if ((*po_pContiguousnessPoints)[po_pContiguousnessPoints->size() - 2].IsEqualTo(HGF2DLocation(ContiguousnessPositions[1], GetCoordSys()),
                                     Tolerance))
                                {
                                // These two are equal ... we change value of next to last
                                (*po_pContiguousnessPoints)[po_pContiguousnessPoints->size() - 2] = HGF2DLocation(ContiguousnessPositions[0], GetCoordSys());
                                }
                            else
                                {
                                // The two points are different ... we add points
                                po_pContiguousnessPoints->push_back(HGF2DLocation(ContiguousnessPositions[0], GetCoordSys()));
                                po_pContiguousnessPoints->push_back(HGF2DLocation(ContiguousnessPositions[1], GetCoordSys()));
                                }
                            }
                        else
                            {
                            // No new points were added previously ...
                            // We add the two points as are
                            po_pContiguousnessPoints->push_back(HGF2DLocation(ContiguousnessPositions[0], GetCoordSys()));
                            po_pContiguousnessPoints->push_back(HGF2DLocation(ContiguousnessPositions[1], GetCoordSys()));
                            }
                        }
                    }
                }
            }
        }

    return (po_pContiguousnessPoints->size() - InitialNumberOfPoints);
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

        // We check if it is linearity preserving
        if (GetCoordSys()->HasLinearityPreservingRelationTo(pi_rPolySegment.GetCoordSys()))
            {
            // They are related through a linearity preserving model ... go ahead
            NumberOfNewPoints = ObtainContiguousnessPointsWithPolySegmentLRCS(pi_rPolySegment, po_pContiguousnessPoints);
            }
        else
            {
            // They are not related by a linearity preserving model ... we must allocate a copy
            // In correct coordinate system
            HAutoPtr<HVE2DVector> pTempVector(pi_rPolySegment.AllocateCopyInCoordSys(GetCoordSys()));

            // Obtain answer
            NumberOfNewPoints = pTempVector->ObtainContiguousnessPoints(*this, po_pContiguousnessPoints);
            }
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

        // We check if it is linearity preserving
        if (GetCoordSys()->HasLinearityPreservingRelationTo(pi_rSegment.GetCoordSys()))
            {
            // They are related through a linearity preserving model ... go ahead
            NumberOfNewPoints = ObtainContiguousnessPointsWithSegmentLRCS(pi_rSegment, po_pContiguousnessPoints);
            }
        else
            {
            // They are not related by a linearity preserving model ... we must allocate a copy
            // In correct coordinate system
            HAutoPtr<HVE2DVector> pTempVector(pi_rSegment.AllocateCopyInCoordSys(GetCoordSys()));

            // Obtain answer
            NumberOfNewPoints = pTempVector->ObtainContiguousnessPoints(*this, po_pContiguousnessPoints);
            }
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

    // Pre-calculate tolerance
    double Tolerance = MIN(GetTolerance(), pi_rSegment.GetTolerance());

    double GivenXMin = MIN(pi_rSegment.GetStartPoint().GetX(), pi_rSegment.GetEndPoint().GetX());
    double GivenXMax = MAX(pi_rSegment.GetStartPoint().GetX(), pi_rSegment.GetEndPoint().GetX());
    double GivenYMin = MIN(pi_rSegment.GetStartPoint().GetY(), pi_rSegment.GetEndPoint().GetY());
    double GivenYMax = MAX(pi_rSegment.GetStartPoint().GetY(), pi_rSegment.GetEndPoint().GetY());

    // No need to check extent overlap since we already know they are
    // contiguous

    bool   Found = false;

    // Create a lite segment to represent segment
    HGF2DLiteSegment TheLiteSegment(pi_rSegment.GetStartPoint().GetPosition(),
                                    pi_rSegment.GetEndPoint().GetPosition(),
                                    Tolerance);

    // For every pair of points of the polysegment ...
    HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
    HGF2DPositionCollection::const_iterator PreviousItr(Itr);
    ++Itr;

    for (; (!Found) && (Itr != m_Points.end()) ; ++Itr , ++PreviousItr)
        {
        // Check if current segment and segment may interact
        bool Result = (HDOUBLE_GREATER_OR_EQUAL(MAX(Itr->GetX(), PreviousItr->GetX()), GivenXMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(MIN(Itr->GetX(), PreviousItr->GetX()), GivenXMax, Tolerance) &&
                        HDOUBLE_GREATER_OR_EQUAL(MAX(Itr->GetY(), PreviousItr->GetY()), GivenYMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(MIN(Itr->GetY(), PreviousItr->GetY()), GivenYMax, Tolerance)
                       );


        if (Result)
            {
            // The two segments may interact ...

            // Create lite segment for current segment
            HGF2DLiteSegment PolySegment(*PreviousItr, *Itr, Tolerance);

            // Check if point is located on current segment
            if (PolySegment.IsPointOn(ThePoint))
                {
                // The point is on current segment ...

                // Check if they are contiguous
                if (PolySegment.AreContiguous(TheLiteSegment))
                    {
                    // These two segments are effectively contiguous

                    // Declare a position collection to receive contiguousness points
                    HGF2DPositionCollection NewContiguousnessPositions;

                    // Obtain contiguousness points
                    PolySegment.ObtainContiguousnessPoints(TheLiteSegment, &NewContiguousnessPositions);

                    // There must be exactly 2 points
                    HASSERT(NewContiguousnessPositions.size() == 2);

                    // Copy to recipient list
                    FirstContiguousnessPosition = NewContiguousnessPositions[0];
                    SecondContiguousnessPosition = NewContiguousnessPositions[1];

                    // Indicate we have found the original contiguousness region
                    Found = true;
                    }
                }
            }
        }

    // Make sure a segment has been found
    HASSERT(Found);

    // Backtrack iterators
    --Itr;
    --PreviousItr;


    // We have found a component which is contiguous at given point ...
    // The contiguousness may however extend past this component in both direction

    // We first back track to obtain start of contiguousness region
    // Obtain a copy of iterator
    HGF2DPositionCollection::const_iterator  MyOtherItr(Itr);
    HGF2DPositionCollection::const_iterator  MyOtherPreviousItr(PreviousItr);

    HGF2DLocation AdditionalFirstPoint(GetCoordSys());
    HGF2DLocation AdditionalSecondPoint(GetCoordSys());

    while ((MyOtherPreviousItr->IsEqualTo(FirstContiguousnessPosition, Tolerance) ||
            MyOtherPreviousItr->IsEqualTo(SecondContiguousnessPosition, Tolerance)) &&
           (MyOtherPreviousItr != m_Points.begin()))
        {
        // Rewind one component
        --MyOtherPreviousItr;
        --MyOtherItr;

        if (pi_rSegment.IsPointOn(HGF2DLocation(*MyOtherItr, GetCoordSys()), HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
            {
            HVE2DSegment CurrentSegment(HGF2DLocation(*MyOtherPreviousItr, GetCoordSys()), HGF2DLocation(*MyOtherItr, GetCoordSys()));

            if (CurrentSegment.AreContiguousAt(pi_rSegment, HGF2DLocation(*MyOtherItr, GetCoordSys())))
                {
                // They are still contiguous ... obtain contiguousness points
                CurrentSegment.ObtainContiguousnessPointsAt(pi_rSegment,
                                                            HGF2DLocation(*MyOtherItr, GetCoordSys()),
                                                            &AdditionalFirstPoint,
                                                            &AdditionalSecondPoint);

                // Remove those identical to end point
                if (AdditionalFirstPoint.GetPosition().IsEqualTo(*MyOtherItr, Tolerance))
                    {
                    if (FirstContiguousnessPosition.IsEqualTo(*MyOtherItr, Tolerance))
                        FirstContiguousnessPosition = AdditionalSecondPoint.GetPosition();
                    else
                        SecondContiguousnessPosition = AdditionalSecondPoint.GetPosition();
                    }
                else
                    {
                    if (FirstContiguousnessPosition.IsEqualTo(*MyOtherItr, Tolerance))
                        FirstContiguousnessPosition = AdditionalFirstPoint.GetPosition();
                    else
                        SecondContiguousnessPosition = AdditionalFirstPoint.GetPosition();
                    }
                }
            }
        }

    // We forward track in the same fashion
    MyOtherItr = Itr;
    MyOtherPreviousItr = PreviousItr;

    ++MyOtherItr;
    ++MyOtherPreviousItr;

    while ((MyOtherItr != m_Points.end()) &&
           (MyOtherPreviousItr->IsEqualTo(FirstContiguousnessPosition, Tolerance) ||
            MyOtherPreviousItr->IsEqualTo(SecondContiguousnessPosition, Tolerance)))
        {
        if (pi_rSegment.IsPointOn(HGF2DLocation(*MyOtherPreviousItr, GetCoordSys()), HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
            {
            HVE2DSegment CurrentSegment(HGF2DLocation(*MyOtherPreviousItr, GetCoordSys()), HGF2DLocation(*MyOtherItr, GetCoordSys()));

            if (CurrentSegment.AreContiguousAt(pi_rSegment, HGF2DLocation(*MyOtherPreviousItr, GetCoordSys())))
                {

                // They are still contiguous ... obtain contiguousness points
                CurrentSegment.ObtainContiguousnessPointsAt(pi_rSegment,
                                                            HGF2DLocation(*MyOtherPreviousItr, GetCoordSys()),
                                                            &AdditionalFirstPoint,
                                                            &AdditionalSecondPoint);

                // Remove those identical to end point
                if (AdditionalFirstPoint.GetPosition().IsEqualTo(*MyOtherPreviousItr, Tolerance))
                    {
                    if (FirstContiguousnessPosition.IsEqualTo(*MyOtherPreviousItr, Tolerance))
                        FirstContiguousnessPosition = AdditionalSecondPoint.GetPosition();
                    else
                        SecondContiguousnessPosition = AdditionalSecondPoint.GetPosition();
                    }
                else
                    {
                    if (FirstContiguousnessPosition.IsEqualTo(*MyOtherPreviousItr, Tolerance))
                        FirstContiguousnessPosition = AdditionalFirstPoint.GetPosition();
                    else
                        SecondContiguousnessPosition = AdditionalFirstPoint.GetPosition();
                    }
                }
            }

        ++MyOtherItr;
        ++MyOtherPreviousItr;
        }

    // The two first points are returned
    *po_pFirstContiguousnessPoint = HGF2DLocation(FirstContiguousnessPosition, GetCoordSys());
    *po_pSecondContiguousnessPoint = HGF2DLocation(SecondContiguousnessPosition, GetCoordSys());

    }

//-----------------------------------------------------------------------------
// ObtainContiguousnessPointsWithSegmentAtLRCS
// PRIVATE
// Finds contiguousness points with segment
// The polysegment and segment must have coordinate systems related
// by a linearity preserving relation
//-----------------------------------------------------------------------------
void HVE2DPolySegment::ObtainContiguousnessPointsWithSegmentAtLRCS(const HVE2DSegment&  pi_rSegment,
                                                                   const HGF2DLocation& pi_rPoint,
                                                                   HGF2DLocation* po_pFirstContiguousnessPoint,
                                                                   HGF2DLocation* po_pSecondContiguousnessPoint) const
    {
    HINVARIANTS;

    // The polysegment and segment must have coordinate systems related
    // by a linearity preserving relation
    HPRECONDITION(GetCoordSys()->HasLinearityPreservingRelationTo(pi_rSegment.GetCoordSys()));

    // Check that recipient locations are provided
    HPRECONDITION(po_pFirstContiguousnessPoint != 0);
    HPRECONDITION(po_pSecondContiguousnessPoint != 0);

    HGF2DPosition FirstContiguousnessPosition;
    HGF2DPosition SecondContiguousnessPosition;

    // Transform point into a position in self coordinate system
    HGF2DLocation PointInSelfCoordSys(pi_rPoint, GetCoordSys());
    HGF2DPosition ThePoint(PointInSelfCoordSys.GetPosition());

    // Pre-calculate tolerance
    double Tolerance = MIN(GetTolerance(), pi_rSegment.GetTolerance());

    // Obtain coordinates of segment in polysegment coordinate system
    HGF2DLocation SegmentStartLocation(pi_rSegment.GetStartPoint(), GetCoordSys());
    HGF2DLocation SegmentEndLocation(pi_rSegment.GetEndPoint(), GetCoordSys());

    // Transform into positions
    HGF2DPosition SegmentStartPosition(SegmentStartLocation.GetPosition());
    HGF2DPosition SegmentEndPosition(SegmentEndLocation.GetPosition());

    // Obtain extent of segment
    double GivenXMin = MIN(SegmentStartPosition.GetX(), SegmentEndPosition.GetX());
    double GivenXMax = MAX(SegmentStartPosition.GetX(), SegmentEndPosition.GetX());
    double GivenYMin = MIN(SegmentStartPosition.GetY(), SegmentEndPosition.GetY());
    double GivenYMax = MAX(SegmentStartPosition.GetY(), SegmentEndPosition.GetY());

    // No need to check extent overlap since we already know they are
    // contiguous

    bool   Found = false;

    // Create a lite segment to represent segment
    HGF2DLiteSegment TheLiteSegment(SegmentStartPosition,
                                    SegmentEndPosition,
                                    Tolerance);

    // For every pair of points of the polysegment ...
    HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
    HGF2DPositionCollection::const_iterator PreviousItr(Itr);
    ++Itr;

    for (; (!Found) && (Itr != m_Points.end()) ; ++Itr , ++PreviousItr)
        {
        // Check if current segment and segment may interact
        bool Result = (HDOUBLE_GREATER_OR_EQUAL(MAX(Itr->GetX(), PreviousItr->GetX()), GivenXMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(MIN(Itr->GetX(), PreviousItr->GetX()), GivenXMax, Tolerance) &&
                        HDOUBLE_GREATER_OR_EQUAL(MAX(Itr->GetY(), PreviousItr->GetY()), GivenYMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(MIN(Itr->GetY(), PreviousItr->GetY()), GivenYMax, Tolerance)
                       );


        if (Result)
            {
            // The two segments may interact ...

            // Create lite segment for current segment
            HGF2DLiteSegment PolySegment(*PreviousItr, *Itr, Tolerance);

            // Check if point is located on current segment
            if (PolySegment.IsPointOn(ThePoint))
                {
                // The point is on current segment ...

                // Check if they are contiguous
                if (PolySegment.AreContiguous(TheLiteSegment))
                    {
                    // These two segments are effectively contiguous

                    // Declare a position collection to receive contiguousness points
                    HGF2DPositionCollection NewContiguousnessPositions;

                    // Obtain contiguousness points
                    PolySegment.ObtainContiguousnessPoints(TheLiteSegment, &NewContiguousnessPositions);

                    // There must be exactly 2 points
                    HASSERT(NewContiguousnessPositions.size() == 2);

                    // Copy to recipient list
                    FirstContiguousnessPosition = NewContiguousnessPositions[0];
                    SecondContiguousnessPosition = NewContiguousnessPositions[1];

                    // Indicate we have found the original contiguousness region
                    Found = true;
                    }
                }
            }
        }

    // Make sure a segment has been found
    HASSERT(Found);

    // Backtrack iterators
    --Itr;
    --PreviousItr;


    // We have found a component which is contiguous at given point ...
    // The contiguousness may however extend past this component in both direction

    // We first back track to obtain start of contiguousness region
    // Obtain a copy of iterator
    HGF2DPositionCollection::const_iterator  MyOtherItr(Itr);
    HGF2DPositionCollection::const_iterator  MyOtherPreviousItr(PreviousItr);

    HGF2DLocation AdditionalFirstPoint(GetCoordSys());
    HGF2DLocation AdditionalSecondPoint(GetCoordSys());

    while ((MyOtherPreviousItr->IsEqualTo(FirstContiguousnessPosition, Tolerance) ||
            MyOtherPreviousItr->IsEqualTo(SecondContiguousnessPosition, Tolerance)) &&
           (MyOtherPreviousItr != m_Points.begin()))
        {
        // Rewind one component
        --MyOtherPreviousItr;
        --MyOtherItr;

        if (pi_rSegment.IsPointOn(HGF2DLocation(*MyOtherItr, GetCoordSys()), HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
            {
            HVE2DSegment CurrentSegment(HGF2DLocation(*MyOtherPreviousItr, GetCoordSys()), HGF2DLocation(*MyOtherItr, GetCoordSys()));

            if (CurrentSegment.AreContiguousAt(pi_rSegment, HGF2DLocation(*MyOtherItr, GetCoordSys())))
                {
                // They are still contiguous ... obtain contiguousness points
                CurrentSegment.ObtainContiguousnessPointsAt(pi_rSegment,
                                                            HGF2DLocation(*MyOtherItr, GetCoordSys()),
                                                            &AdditionalFirstPoint,
                                                            &AdditionalSecondPoint);

                // Remove those identical to end point
                if (AdditionalFirstPoint.GetPosition().IsEqualTo(*MyOtherItr, Tolerance))
                    {
                    if (FirstContiguousnessPosition.IsEqualTo(*MyOtherItr, Tolerance))
                        FirstContiguousnessPosition = AdditionalSecondPoint.GetPosition();
                    else
                        SecondContiguousnessPosition = AdditionalSecondPoint.GetPosition();
                    }
                else
                    {
                    if (FirstContiguousnessPosition.IsEqualTo(*MyOtherItr, Tolerance))
                        FirstContiguousnessPosition = AdditionalFirstPoint.GetPosition();
                    else
                        SecondContiguousnessPosition = AdditionalFirstPoint.GetPosition();
                    }
                }
            }
        }

    // We forward track in the same fashion
    MyOtherItr = Itr;
    MyOtherPreviousItr = PreviousItr;

    ++MyOtherItr;
    ++MyOtherPreviousItr;

    while ((MyOtherItr != m_Points.end()) &&
           (MyOtherPreviousItr->IsEqualTo(FirstContiguousnessPosition, Tolerance) ||
            MyOtherPreviousItr->IsEqualTo(SecondContiguousnessPosition, Tolerance)))
        {
        if (pi_rSegment.IsPointOn(HGF2DLocation(*MyOtherPreviousItr, GetCoordSys()), HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
            {
            HVE2DSegment CurrentSegment(HGF2DLocation(*MyOtherPreviousItr, GetCoordSys()), HGF2DLocation(*MyOtherItr, GetCoordSys()));

            if (CurrentSegment.AreContiguousAt(pi_rSegment, HGF2DLocation(*MyOtherPreviousItr, GetCoordSys())))
                {

                // They are still contiguous ... obtain contiguousness points
                CurrentSegment.ObtainContiguousnessPointsAt(pi_rSegment,
                                                            HGF2DLocation(*MyOtherPreviousItr, GetCoordSys()),
                                                            &AdditionalFirstPoint,
                                                            &AdditionalSecondPoint);

                // Remove those identical to end point
                if (AdditionalFirstPoint.GetPosition().IsEqualTo(*MyOtherPreviousItr, Tolerance))
                    {
                    if (FirstContiguousnessPosition.IsEqualTo(*MyOtherPreviousItr, Tolerance))
                        FirstContiguousnessPosition = AdditionalSecondPoint.GetPosition();
                    else
                        SecondContiguousnessPosition = AdditionalSecondPoint.GetPosition();
                    }
                else
                    {
                    if (FirstContiguousnessPosition.IsEqualTo(*MyOtherPreviousItr, Tolerance))
                        FirstContiguousnessPosition = AdditionalFirstPoint.GetPosition();
                    else
                        SecondContiguousnessPosition = AdditionalFirstPoint.GetPosition();
                    }
                }
            }

        ++MyOtherItr;
        ++MyOtherPreviousItr;
        }

    // The two first points are returned
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

    // Pre-calculate tolerance
    double Tolerance = MIN(GetTolerance(), pi_rPolySegment.GetTolerance());

    HGF2DPosition FirstContiguousnessPosition;
    HGF2DPosition SecondContiguousnessPosition;

    // Obtain extent of polysegment
    HGF2DExtent GivenPolyExtent(pi_rPolySegment.GetExtent());

    double GivenXMin = GivenPolyExtent.GetXMin();
    double GivenXMax = GivenPolyExtent.GetXMax();
    double GivenYMin = GivenPolyExtent.GetYMin();
    double GivenYMax = GivenPolyExtent.GetYMax();

    // No need to check extent overlap since we already know they are
    // contiguous

    bool   Found = false;


    // For every pair of points of the polysegment ...
    HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
    HGF2DPositionCollection::const_iterator PreviousItr(Itr);
    ++Itr;

    for (; (!Found) && (Itr != m_Points.end()) ; ++Itr , ++PreviousItr)
        {
        // Obtain extent of this segment
        double SelfXMin = MIN(Itr->GetX(), PreviousItr->GetX());
        double SelfXMax = MAX(Itr->GetX(), PreviousItr->GetX());
        double SelfYMin = MIN(Itr->GetY(), PreviousItr->GetY());
        double SelfYMax = MAX(Itr->GetY(), PreviousItr->GetY());

        // Check if current segment and given polysegment may interact
        bool Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                        HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                       );


        if (Result)
            {
            // The two segments may interact ... check if contiguous

            // Create lite segment for current segment
            HGF2DLiteSegment PolySegment(*PreviousItr, *Itr, Tolerance);

            // Check if point is located on current segment
            if (PolySegment.IsPointOn(ThePoint))
                {
#if (0)
                // For every segment of given polysegment
                HGF2DPositionCollection::const_iterator GivenItr(pi_rPolySegment.m_Points.begin());
                HGF2DPositionCollection::const_iterator PreviousGivenItr(GivenItr);
                ++GivenItr;
                for (; (!Found) && GivenItr != pi_rPolySegment.m_Points.end() ; ++GivenItr , ++PreviousGivenItr)
                    {
                    // Check if current self segment and current given segment may interact
                    Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, MIN(GivenItr->GetX(), PreviousGivenItr->GetX()), Tolerance) &&
                              HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, MAX(GivenItr->GetX(), PreviousGivenItr->GetX()), Tolerance) &&
                              HDOUBLE_GREATER_OR_EQUAL(SelfYMax, MIN(GivenItr->GetY(), PreviousGivenItr->GetY()), Tolerance) &&
                              HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, MAX(GivenItr->GetY(), PreviousGivenItr->GetY()), Tolerance)
                             );


                    if (Result)
                        {
                        // The self segment and given poly segment may interact ...

                        // Create given segment
                        HGF2DLiteSegment GivenSegment(*PreviousGivenItr, *GivenItr, Tolerance);

                        // Check if point is located on current segment
                        if (GivenSegment.IsPointOn(ThePoint))
                            {
                            // The point is on current segment ...
                            // Check if contiguous
                            if (PolySegment.AreContiguous(GivenSegment))
                                {
                                // These two segments are effectively contiguous

                                // Declare a position collection to receive contiguousness points
                                HGF2DPositionCollection NewContiguousnessPositions;

                                // Obtain contiguousness points
                                PolySegment.ObtainContiguousnessPoints(GivenSegment, &NewContiguousnessPositions);

                                // There must be exactly 2 points
                                HASSERT(NewContiguousnessPositions.size() == 2);


                                // Copy to recipient list
                                FirstContiguousnessPosition = NewContiguousnessPositions[0];
                                SecondContiguousnessPosition = NewContiguousnessPositions[1];

                                // Indicate we have found the original contiguousness region
                                Found = true;
                                }
                            }
                        }
                    }
#else
                // Create a full fledged segment
                HVE2DSegment FullPolySegment(HGF2DLocation(*PreviousItr, GetCoordSys()),
                                             HGF2DLocation(*Itr, GetCoordSys()));
                FullPolySegment.SetAutoToleranceActive(false);
                FullPolySegment.SetTolerance(Tolerance);

                if (FullPolySegment.AreContiguousAt(pi_rPolySegment, PointInSelfCoordSys))
                    {
                    HGF2DLocation TheFirstPoint;
                    HGF2DLocation TheSecondPoint;
                    FullPolySegment.ObtainContiguousnessPointsAt(pi_rPolySegment, PointInSelfCoordSys, &TheFirstPoint, &TheSecondPoint);

                    FirstContiguousnessPosition = TheFirstPoint.GetPosition();
                    SecondContiguousnessPosition = TheSecondPoint.GetPosition();

                    // Indicate we have found the original contiguousness region
                    Found = true;

                    }
#endif
                }
            }
        }


    // Make sure a segment has been found
    HASSERT(Found);

    // Backtrack iterators
    --Itr;
    --PreviousItr;


    // We have found a component which is contiguous at given point ...
    // The contiguousness may however extend past this component in both direction

    // We first back track to obtain start of contiguousness region
    // Obtain a copy of iterator
    HGF2DPositionCollection::const_iterator  MyOtherItr(Itr);
    HGF2DPositionCollection::const_iterator  MyOtherPreviousItr(PreviousItr);

    HGF2DLocation AdditionalFirstPoint(GetCoordSys());
    HGF2DLocation AdditionalSecondPoint(GetCoordSys());

    while ((MyOtherPreviousItr->IsEqualTo(FirstContiguousnessPosition, Tolerance) ||
            MyOtherPreviousItr->IsEqualTo(SecondContiguousnessPosition, Tolerance)) &&
           (MyOtherPreviousItr != m_Points.begin()))
        {
        // Rewind one component
        --MyOtherPreviousItr;
        --MyOtherItr;

        if (pi_rPolySegment.IsPointOn(HGF2DLocation(*MyOtherItr, GetCoordSys()), HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
            {
            HVE2DSegment CurrentSegment(HGF2DLocation(*MyOtherPreviousItr, GetCoordSys()), HGF2DLocation(*MyOtherItr, GetCoordSys()));

            if (CurrentSegment.AreContiguousAt(pi_rPolySegment, HGF2DLocation(*MyOtherItr, GetCoordSys())))
                {
                // They are still contiguous ... obtain contiguousness points
                CurrentSegment.ObtainContiguousnessPointsAt(pi_rPolySegment,
                                                            HGF2DLocation(*MyOtherItr, GetCoordSys()),
                                                            &AdditionalFirstPoint,
                                                            &AdditionalSecondPoint);

                // Remove those identical to end point
                if (AdditionalFirstPoint.GetPosition().IsEqualTo(*MyOtherItr, Tolerance))
                    {
                    if (FirstContiguousnessPosition.IsEqualTo(*MyOtherItr, Tolerance))
                        FirstContiguousnessPosition = AdditionalSecondPoint.GetPosition();
                    else
                        SecondContiguousnessPosition = AdditionalSecondPoint.GetPosition();
                    }
                else
                    {
                    if (FirstContiguousnessPosition.IsEqualTo(*MyOtherItr, Tolerance))
                        FirstContiguousnessPosition = AdditionalFirstPoint.GetPosition();
                    else
                        SecondContiguousnessPosition = AdditionalFirstPoint.GetPosition();
                    }
                }
            }
        }

    // We forward track in the same fashion
    MyOtherItr = Itr;
    MyOtherPreviousItr = PreviousItr;

    ++MyOtherItr;
    ++MyOtherPreviousItr;

    while ((MyOtherItr != m_Points.end()) &&
           (MyOtherPreviousItr->IsEqualTo(FirstContiguousnessPosition, Tolerance) ||
            MyOtherPreviousItr->IsEqualTo(SecondContiguousnessPosition, Tolerance)))
        {
        if (pi_rPolySegment.IsPointOn(HGF2DLocation(*MyOtherPreviousItr, GetCoordSys()), HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
            {
            HVE2DSegment CurrentSegment(HGF2DLocation(*MyOtherPreviousItr, GetCoordSys()), HGF2DLocation(*MyOtherItr, GetCoordSys()));

            if (CurrentSegment.AreContiguousAt(pi_rPolySegment, HGF2DLocation(*MyOtherPreviousItr, GetCoordSys())))
                {

                // They are still contiguous ... obtain contiguousness points
                CurrentSegment.ObtainContiguousnessPointsAt(pi_rPolySegment,
                                                            HGF2DLocation(*MyOtherPreviousItr, GetCoordSys()),
                                                            &AdditionalFirstPoint,
                                                            &AdditionalSecondPoint);

                // Remove those identical to end point
                if (AdditionalFirstPoint.GetPosition().IsEqualTo(*MyOtherPreviousItr, Tolerance))
                    {
                    if (FirstContiguousnessPosition.IsEqualTo(*MyOtherPreviousItr, Tolerance))
                        FirstContiguousnessPosition = AdditionalSecondPoint.GetPosition();
                    else
                        SecondContiguousnessPosition = AdditionalSecondPoint.GetPosition();
                    }
                else
                    {
                    if (FirstContiguousnessPosition.IsEqualTo(*MyOtherPreviousItr, Tolerance))
                        FirstContiguousnessPosition = AdditionalFirstPoint.GetPosition();
                    else
                        SecondContiguousnessPosition = AdditionalFirstPoint.GetPosition();
                    }
                }
            }

        ++MyOtherItr;
        ++MyOtherPreviousItr;
        }

    // The two first points are returned
    *po_pFirstContiguousnessPoint = HGF2DLocation(FirstContiguousnessPosition, GetCoordSys());
    *po_pSecondContiguousnessPoint = HGF2DLocation(SecondContiguousnessPosition, GetCoordSys());
    }

//-----------------------------------------------------------------------------
// ObtainContiguousnessPointsWithPolySegmentAtLRCS
// PRIVATE
// Finds contiguousness points with poly segment
// The polysegments must have coordinate systems related
// by a linearity preserving relation
//-----------------------------------------------------------------------------
void HVE2DPolySegment::ObtainContiguousnessPointsWithPolySegmentAtLRCS(const HVE2DPolySegment&  pi_rPolySegment,
                                                                       const HGF2DLocation& pi_rPoint,
                                                                       HGF2DLocation* po_pFirstContiguousnessPoint,
                                                                       HGF2DLocation* po_pSecondContiguousnessPoint) const
    {
    HINVARIANTS;

    // The polysegments must have coordinate systems related
    // by a linearity preserving relation
    HPRECONDITION(GetCoordSys()->HasLinearityPreservingRelationTo(pi_rPolySegment.GetCoordSys()));

    // Check that recipient locations are provided
    HPRECONDITION(po_pFirstContiguousnessPoint != 0);
    HPRECONDITION(po_pSecondContiguousnessPoint != 0);

    // Transform point into a position in self coordinate system
    HGF2DLocation PointInSelfCoordSys(pi_rPoint, GetCoordSys());
    HGF2DPosition ThePoint(PointInSelfCoordSys.GetPosition());

    // Pre-calculate tolerance
    double Tolerance = MIN(GetTolerance(), pi_rPolySegment.GetTolerance());

    HGF2DPosition FirstContiguousnessPosition;
    HGF2DPosition SecondContiguousnessPosition;

    // Obtain extent of polysegment
    HGF2DExtent GivenPolyExtent(pi_rPolySegment.GetExtent());
    GivenPolyExtent.ChangeCoordSys(GetCoordSys());

    double GivenXMin = GivenPolyExtent.GetXMin();
    double GivenXMax = GivenPolyExtent.GetXMax();
    double GivenYMin = GivenPolyExtent.GetYMin();
    double GivenYMax = GivenPolyExtent.GetYMax();

    // No need to check extent overlap since we already know they are
    // contiguous

    bool   Found = false;


    // For every pair of points of the polysegment ...
    HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
    HGF2DPositionCollection::const_iterator PreviousItr(Itr);
    ++Itr;

    for (; (!Found) && Itr != m_Points.end() ; ++Itr , ++PreviousItr)
        {
        // Obtain extent of this segment
        double SelfXMin = MIN(Itr->GetX(), PreviousItr->GetX());
        double SelfXMax = MAX(Itr->GetX(), PreviousItr->GetX());
        double SelfYMin = MIN(Itr->GetY(), PreviousItr->GetY());
        double SelfYMax = MAX(Itr->GetY(), PreviousItr->GetY());

        // Check if current segment and given polysegment may interact
        bool Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                        HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                        HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                       );


        if (Result)
            {
            // The two segments may interact ... check if contiguous

            // Create lite segment for current segment
            HGF2DLiteSegment PolySegment(*PreviousItr, *Itr, Tolerance);

            // Check if point is located on current segment
            if (PolySegment.IsPointOn(ThePoint))
                {
                // For every segment of given polysegment
                HGF2DPositionCollection::const_iterator GivenItr(pi_rPolySegment.m_Points.begin());
                HGF2DPositionCollection::const_iterator PreviousGivenItr(GivenItr);
                ++GivenItr;
                for (; (!Found) && GivenItr != pi_rPolySegment.m_Points.end() ; ++GivenItr , ++PreviousGivenItr)
                    {

                    // Create location in self coordinate system
                    HGF2DLocation CurGivenSegStartLocation(*PreviousGivenItr, pi_rPolySegment.GetCoordSys());
                    CurGivenSegStartLocation.ChangeCoordSys(GetCoordSys());
                    HGF2DLocation CurGivenSegEndLocation(*GivenItr, pi_rPolySegment.GetCoordSys());
                    CurGivenSegEndLocation.ChangeCoordSys(GetCoordSys());

                    HGF2DPosition CurGivenSegStartPosition(CurGivenSegStartLocation.GetPosition());
                    HGF2DPosition CurGivenSegEndPosition(CurGivenSegEndLocation.GetPosition());

                    // Check if current self segment and current given segment may interact
                    Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, MIN(CurGivenSegEndPosition.GetX(), CurGivenSegStartPosition.GetX()), Tolerance) &&
                              HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, MAX(CurGivenSegEndPosition.GetX(), CurGivenSegStartPosition.GetX()), Tolerance) &&
                              HDOUBLE_GREATER_OR_EQUAL(SelfYMax, MIN(CurGivenSegEndPosition.GetY(), CurGivenSegStartPosition.GetY()), Tolerance) &&
                              HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, MAX(CurGivenSegEndPosition.GetY(), CurGivenSegStartPosition.GetY()), Tolerance)
                             );


                    if (Result)
                        {
                        // The two segments may interact ...

                        // Create given segment
                        HGF2DLiteSegment GivenSegment(CurGivenSegStartPosition,
                                                      CurGivenSegEndPosition,
                                                      Tolerance);

                        // Check if point is located on current segment
                        if (GivenSegment.IsPointOn(ThePoint))
                            {
                            // The point is on current segment ...
                            // Check if contiguous
                            if (PolySegment.AreContiguous(GivenSegment))
                                {
                                // These two segments are effectively contiguous

                                // Declare a position collection to receive contiguousness points
                                HGF2DPositionCollection NewContiguousnessPositions;

                                // Obtain contiguousness points
                                PolySegment.ObtainContiguousnessPoints(GivenSegment, &NewContiguousnessPositions);

                                // There must be exactly 2 points
                                HASSERT(NewContiguousnessPositions.size() == 2);

                                // Copy to recipient list
                                FirstContiguousnessPosition = NewContiguousnessPositions[0];
                                SecondContiguousnessPosition = NewContiguousnessPositions[1];

                                // Indicate we have found the original contiguousness region
                                Found = true;
                                }
                            }
                        }
                    }
                }
            }
        }


    // Make sure a segment has been found
    HASSERT(Found);

    // Backtrack iterators
    --Itr;
    --PreviousItr;


    // We have found a component which is contiguous at given point ...
    // The contiguousness may however extend past this component in both direction

    // We first back track to obtain start of contiguousness region
    // Obtain a copy of iterator
    HGF2DPositionCollection::const_iterator  MyOtherItr(Itr);
    HGF2DPositionCollection::const_iterator  MyOtherPreviousItr(PreviousItr);

    HGF2DLocation AdditionalFirstPoint(GetCoordSys());
    HGF2DLocation AdditionalSecondPoint(GetCoordSys());

    while ((MyOtherPreviousItr->IsEqualTo(FirstContiguousnessPosition, Tolerance) ||
            MyOtherPreviousItr->IsEqualTo(SecondContiguousnessPosition, Tolerance)) &&
           (MyOtherPreviousItr != m_Points.begin()))
        {
        // Rewind one component
        --MyOtherPreviousItr;
        --MyOtherItr;

        if (pi_rPolySegment.IsPointOn(HGF2DLocation(*MyOtherItr, GetCoordSys()), HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
            {
            HVE2DSegment CurrentSegment(HGF2DLocation(*MyOtherPreviousItr, GetCoordSys()), HGF2DLocation(*MyOtherItr, GetCoordSys()));

            if (CurrentSegment.AreContiguousAt(pi_rPolySegment, HGF2DLocation(*MyOtherItr, GetCoordSys())))
                {
                // They are still contiguous ... obtain contiguousness points
                CurrentSegment.ObtainContiguousnessPointsAt(pi_rPolySegment,
                                                            HGF2DLocation(*MyOtherItr, GetCoordSys()),
                                                            &AdditionalFirstPoint,
                                                            &AdditionalSecondPoint);

                // Remove those identical to end point
                if (AdditionalFirstPoint.GetPosition().IsEqualTo(*MyOtherItr, Tolerance))
                    {
                    if (FirstContiguousnessPosition.IsEqualTo(*MyOtherItr, Tolerance))
                        FirstContiguousnessPosition = AdditionalSecondPoint.GetPosition();
                    else
                        SecondContiguousnessPosition = AdditionalSecondPoint.GetPosition();
                    }
                else
                    {
                    if (FirstContiguousnessPosition.IsEqualTo(*MyOtherItr, Tolerance))
                        FirstContiguousnessPosition = AdditionalFirstPoint.GetPosition();
                    else
                        SecondContiguousnessPosition = AdditionalFirstPoint.GetPosition();
                    }
                }
            }
        }

    // We forward track in the same fashion
    MyOtherItr = Itr;
    MyOtherPreviousItr = PreviousItr;

    ++MyOtherItr;
    ++MyOtherPreviousItr;

    while ((MyOtherItr != m_Points.end()) &&
           (MyOtherPreviousItr->IsEqualTo(FirstContiguousnessPosition, Tolerance) ||
            MyOtherPreviousItr->IsEqualTo(SecondContiguousnessPosition, Tolerance)))
        {
        if (pi_rPolySegment.IsPointOn(HGF2DLocation(*MyOtherPreviousItr, GetCoordSys()), HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
            {
            HVE2DSegment CurrentSegment(HGF2DLocation(*MyOtherPreviousItr, GetCoordSys()), HGF2DLocation(*MyOtherItr, GetCoordSys()));

            if (CurrentSegment.AreContiguousAt(pi_rPolySegment, HGF2DLocation(*MyOtherPreviousItr, GetCoordSys())))
                {

                // They are still contiguous ... obtain contiguousness points
                CurrentSegment.ObtainContiguousnessPointsAt(pi_rPolySegment,
                                                            HGF2DLocation(*MyOtherPreviousItr, GetCoordSys()),
                                                            &AdditionalFirstPoint,
                                                            &AdditionalSecondPoint);

                // Remove those identical to end point
                if (AdditionalFirstPoint.GetPosition().IsEqualTo(*MyOtherPreviousItr, Tolerance))
                    {
                    if (FirstContiguousnessPosition.IsEqualTo(*MyOtherPreviousItr, Tolerance))
                        FirstContiguousnessPosition = AdditionalSecondPoint.GetPosition();
                    else
                        SecondContiguousnessPosition = AdditionalSecondPoint.GetPosition();
                    }
                else
                    {
                    if (FirstContiguousnessPosition.IsEqualTo(*MyOtherPreviousItr, Tolerance))
                        FirstContiguousnessPosition = AdditionalFirstPoint.GetPosition();
                    else
                        SecondContiguousnessPosition = AdditionalFirstPoint.GetPosition();
                    }
                }
            }

        ++MyOtherItr;
        ++MyOtherPreviousItr;
        }

    // The two first points are returned
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

        // We check if it is linearity preserving
        if (GetCoordSys()->HasLinearityPreservingRelationTo(pi_rPolySegment.GetCoordSys()))
            {
            // They are related through a linearity preserving model ... go ahead
            ObtainContiguousnessPointsWithPolySegmentAtLRCS(pi_rPolySegment,
                                                            pi_rPoint,
                                                            po_pFirstContiguousnessPoint,
                                                            po_pSecondContiguousnessPoint);
            }
        else
            {
            // They are not related by a linearity preserving model ... we must allocate a copy
            // In correct coordinate system
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

        // We check if it is linearity preserving
        if (GetCoordSys()->HasLinearityPreservingRelationTo(pi_rSegment.GetCoordSys()))
            {
            // They are related through a linearity preserving model ... go ahead
            ObtainContiguousnessPointsWithSegmentAtLRCS(pi_rSegment, pi_rPoint, po_pFirstContiguousnessPoint, po_pSecondContiguousnessPoint);
            }
        else
            {
            // They are not related by a linearity preserving model ... we must allocate a copy
            // In correct coordinate system
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
        m_Points.push_back(TransformedEndPoint.GetPosition());

        m_EndPoint = TransformedEndPoint;
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
    pNewPolySegment->m_StartPoint = m_StartPoint.ExpressedIn(pi_rpCoordSys);
    pNewPolySegment->m_EndPoint   = m_EndPoint.ExpressedIn(pi_rpCoordSys);

// HChk AR If domain error thrown ... we should do something!!!!
    // Obtain a transformed value of start point
    HGF2DLocation TransStartPoint(m_StartPoint, pi_rpCoordSys);

    // Append start point first
    pNewPolySegment->m_Points.push_back(TransStartPoint.GetPosition());

#if (0)
// HChk AR DOUBTFUL!
    pNewPolySegment->SetAutoToleranceActive(true);
    pNewPolySegment->ResetTolerance();
    pNewPolySegment->SetAutoToleranceActive(false);

    // Compute tolerance
    pNewPolySegment->SetTolerance(pNewPolySegment->GetTolerance() * 1000.0);
#endif

    // For every segment of self polysegment
    HGF2DPositionCollection::const_iterator Itr = m_Points.begin();
    HGF2DPositionCollection::const_iterator PreviousItr = Itr;
    ++Itr;
    for (; Itr != m_Points.end() ; ++Itr , ++PreviousItr)
        {
        // Append this segment to the complex
        HVE2DSegment aSegment(*PreviousItr, *Itr, GetCoordSys());
        aSegment.SetStrokeTolerance(pTol);

        pNewPolySegment->AppendSegmentInCoordSys(aSegment);
        }

    // In some imprecise transformation models there could be a variation
    // between conversion of two identical coordinates ... adjust if applicable
    if (m_StartPoint.IsEqualTo(m_EndPoint))
        pNewPolySegment->AdjustEndPointTo(pNewPolySegment->GetStartPoint());

    // If the start point and end points were originally equal then they should be equal in the result
    HASSERT(!m_StartPoint.IsEqualTo(m_EndPoint) || pNewPolySegment->GetStartPoint().IsEqualTo(pNewPolySegment->GetEndPoint()));

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
inline void HVE2DPolySegment::ResetTolerance()
    {
    HINVARIANTS;

    // Check if auto-tolerance is active
    if (IsAutoToleranceActive())
        {
        // Compute appropriate tolerance (default is global epsilon)
        double Tolerance = HGLOBAL_EPSILON;

        // If coordinates are greater than global tolerance divided by
        // float precision (~= 1E-8 / 1E-15 -> 1E7 ... 1E6 for all cases)
        // Then we use global epsilon * coordinate
        // The tolerance is also a factor of the length of segments

        // Check if there is any points
        if (m_Points.size() >= 1)
            {
            HGF2DPositionCollection::const_iterator Itr = m_Points.begin();
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(Itr->GetX()));
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(Itr->GetY()));

//            HGF2DPositionCollection::const_iterator PrevItr(Itr);
            ++Itr;

            for(; Itr != m_Points.end() ; ++Itr)
                {
                Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(Itr->GetX()));
                Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(Itr->GetY()));

//                double DeltaX = Itr->GetX() - PrevItr->GetX();
//                double DeltaY = Itr->GetY() - PrevItr->GetY();
//                Tolerance = MAX(Tolerance, HEPSILON_EXTENT_MULTIPLICATOR * (sqrt(DeltaX * DeltaX + DeltaY * DeltaY)));
                }
            }

        SetTolerance(MIN(HMAX_EPSILON, Tolerance));
        }
    }

//-----------------------------------------------------------------------------
// MakeEmpty
// This method empties the polysegment
//-----------------------------------------------------------------------------
void HVE2DPolySegment::MakeEmpty()
    {
    HINVARIANTS;

    // Clear all points
    m_Points.clear();

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

    m_Points.reserve(pi_PointsToPreAllocate);
    }


//-----------------------------------------------------------------------------
// IntersectsAtAnySplitPointWithPolySegmentSCS
// This method determines if the given linear connects on self
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::IntersectsAtAnySplitPointWithPolySegmentSCS(const HVE2DPolySegment& pi_rPolySegment) const
    {
    // The coordinate systems between polysegments must be the same
    HPRECONDITION(GetCoordSys() == pi_rPolySegment.GetCoordSys());

    bool Answer = false;

    // Obtain tolerance
    double Tolerance = MIN(GetTolerance(), pi_rPolySegment.GetTolerance());

    // Obtain extremes of poly segments
    HGF2DExtent PolyExtent(GetExtent());
    HGF2DExtent GivenPolyExtent(pi_rPolySegment.GetExtent());

    double SelfXMin = PolyExtent.GetXMin();
    double SelfXMax = PolyExtent.GetXMax();
    double SelfYMin = PolyExtent.GetYMin();
    double SelfYMax = PolyExtent.GetYMax();

    double GivenXMin = GivenPolyExtent.GetXMin();
    double GivenXMax = GivenPolyExtent.GetXMax();
    double GivenYMin = GivenPolyExtent.GetYMin();
    double GivenYMax = GivenPolyExtent.GetYMax();

    // Check if their extents outter overlap
    bool Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                    HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                    HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                    HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                   );


    if (Result)
        {
        // Polysegments may interact

        // For every segment of the self polysegment
        HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
        HGF2DPositionCollection::const_iterator PreviousItr(Itr);
        ++Itr;
        for (; !Answer && Itr != m_Points.end() ; ++Itr , ++PreviousItr)
            {
            // Obtain extent of this segment
            SelfXMin = MIN(Itr->GetX(), PreviousItr->GetX());
            SelfXMax = MAX(Itr->GetX(), PreviousItr->GetX());
            SelfYMin = MIN(Itr->GetY(), PreviousItr->GetY());
            SelfYMax = MAX(Itr->GetY(), PreviousItr->GetY());

            // Check if current segment and segment may interact
            Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                      HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                      HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                      HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                     );


            if (Result)
                {
                // Current segment and given polysegment may interact ...

                // Create a lite segment to represent current segment
                HGF2DLiteSegment TheLiteSegment(*PreviousItr, *Itr, Tolerance);

                Answer = pi_rPolySegment.IntersectsAtAnySplitPointWithLiteSegmentSCS(TheLiteSegment);

                }
            }

        if (!Answer)
            {
            // Reset self extent vars
            SelfXMin = PolyExtent.GetXMin();
            SelfXMax = PolyExtent.GetXMax();
            SelfYMin = PolyExtent.GetYMin();
            SelfYMax = PolyExtent.GetYMax();

            // For every segment of the given polysegment
            HGF2DPositionCollection::const_iterator Itr(pi_rPolySegment.m_Points.begin());
            HGF2DPositionCollection::const_iterator PreviousItr(Itr);
            ++Itr;
            for (; !Answer && Itr != pi_rPolySegment.m_Points.end() ; ++Itr , ++PreviousItr)
                {
                // Obtain extent of this segment
                GivenXMin = MIN(Itr->GetX(), PreviousItr->GetX());
                GivenXMax = MAX(Itr->GetX(), PreviousItr->GetX());
                GivenYMin = MIN(Itr->GetY(), PreviousItr->GetY());
                GivenYMax = MAX(Itr->GetY(), PreviousItr->GetY());

                // Check if current segment and segment may interact
                Result = (HDOUBLE_GREATER_OR_EQUAL(GivenXMax, SelfXMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(GivenXMin, SelfXMax, Tolerance) &&
                          HDOUBLE_GREATER_OR_EQUAL(GivenYMax, SelfYMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(GivenYMin, SelfYMax, Tolerance)
                         );


                if (Result)
                    {
                    // Current segment and self polysegment may interact ...

                    // Create a lite segment to represent current segment
                    HGF2DLiteSegment TheLiteSegment(*PreviousItr, *Itr, Tolerance);

                    Answer = IntersectsAtAnySplitPointWithLiteSegmentSCS(TheLiteSegment);

                    }
                }
            }

        if (!Answer)
            {
            // The previous takes care of intersections at split point for one polysegment only
            // If the two share a split point, then the following will detect.

            // Reset extents
            SelfXMin = PolyExtent.GetXMin();
            SelfXMax = PolyExtent.GetXMax();
            SelfYMin = PolyExtent.GetYMin();
            SelfYMax = PolyExtent.GetYMax();

            GivenXMin = GivenPolyExtent.GetXMin();
            GivenXMax = GivenPolyExtent.GetXMax();                                                                                 
            GivenYMin = GivenPolyExtent.GetYMin();
            GivenYMax = GivenPolyExtent.GetYMax();

            // For every segment of the self polysegment
            HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
            HGF2DPositionCollection::const_iterator PreviousItr(Itr);
            ++PreviousItr;
            ++Itr;
            ++Itr;
            for (; !Answer && Itr != m_Points.end() ; ++Itr , ++PreviousItr)
                {
                // Obtain extent of this segment
                SelfXMin = MIN(Itr->GetX(), PreviousItr->GetX());
                SelfXMax = MAX(Itr->GetX(), PreviousItr->GetX());
                SelfYMin = MIN(Itr->GetY(), PreviousItr->GetY());
                SelfYMax = MAX(Itr->GetY(), PreviousItr->GetY());

                // Check if current segment and segment may interact
                Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                          HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                          HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                         );


                if (Result)
                    {
                    HGF2DLiteSegment TheLiteSegment(*PreviousItr, *Itr);

                    // This self segment is within the given polysegment extent ...
                    // For every segment of the given polysegment
                    HGF2DPositionCollection::const_iterator GivenItr(pi_rPolySegment.m_Points.begin());
                    HGF2DPositionCollection::const_iterator PreviousGivenItr(GivenItr);
                    ++PreviousGivenItr;
                    ++GivenItr;
                    ++GivenItr;
                    for (; !Answer && GivenItr != pi_rPolySegment.m_Points.end() ; ++GivenItr , ++PreviousGivenItr)
                        {
                        HGF2DLiteSegment GivenSegment(*PreviousGivenItr, *GivenItr);

#if (0)
                        // Check if current previous is equal to previous given
                        if (PreviousItr->IsEqualTo(*PreviousGivenItr))
                            {
                            Answer = IntersectsAtSplitPoint(pi_rPolySegment,
                                                            HGF2DLocation(*PreviousItr, GetCoordSys()),
                                                            HGF2DLocation(*Itr, GetCoordSys()),
                                                            false);
                            }
                        else if (GivenSegment.IsPointOn(*PreviousItr) &&
                                 TheLiteSegment.AreContiguous(GivenSegment))
                            {
                            Answer = IntersectsAtSplitPoint(pi_rPolySegment,
                                                            HGF2DLocation(*PreviousItr, GetCoordSys()),
                                                            HGF2DLocation(*Itr, GetCoordSys()),
                                                            false);
//                                                            HGF2DLocation(*Itr, GetCoordSys()),
//                                                            true);
                            }
                        else if (TheLiteSegment.IsPointOn(*PreviousGivenItr) &&
                                 TheLiteSegment.AreContiguous(GivenSegment))
                            {
                            Answer = IntersectsAtSplitPoint(pi_rPolySegment,
                                                            HGF2DLocation(*PreviousGivenItr, GetCoordSys()),
                                                            HGF2DLocation(*GivenItr, GetCoordSys()),
                                                            false);
                            }
#else
                        // Check if segments are contiguous
                        if (TheLiteSegment.AreContiguous(GivenSegment))
                            {
                            // They are contiguous ... we will nevertheless process only if
                            // there is no next segment in given or this next segment is not contiguous
                            // to either previous or next segment of self

                            // Check if there is a next given segment
                            HGF2DPositionCollection::const_iterator NextGivenItr(GivenItr);
                            ++NextGivenItr;
                            if (NextGivenItr != pi_rPolySegment.m_Points.end())
                                {
                                // There is a next given segment ... obtain it
                                HGF2DLiteSegment NextGivenSegment(*GivenItr, *NextGivenItr, Tolerance);

                                // Obtain previous self segment
                                HGF2DPositionCollection::const_iterator PreviousToPreviousItr(PreviousItr);
                                --PreviousToPreviousItr;
                                HGF2DLiteSegment PreviousSegment(*PreviousToPreviousItr, *PreviousItr, Tolerance);

                                if (!NextGivenSegment.AreContiguous(TheLiteSegment) &&
                                    !NextGivenSegment.AreContiguous(PreviousSegment))
                                    {
                                    // The next given segment is neither contiguous to current nor previous self
                                    // we must now check for the next self segment is any
                                    HGF2DPositionCollection::const_iterator NextItr(Itr);
                                    ++NextItr;
                                    if (NextItr != m_Points.end())
                                        {
                                        // Create next self segment
                                        HGF2DLiteSegment NextSegment(*Itr, *NextItr, Tolerance);

                                        if (!NextGivenSegment.AreContiguous(NextSegment))
                                            {
                                            // The next segment is not contiguous to any other ... we process
                                            // There is next given segment ... we process depending on which point is on
                                            if (GivenSegment.IsPointOn(*PreviousItr))
                                                {
                                                // Self point is on given
                                                Answer = IntersectsAtSplitPoint(pi_rPolySegment,
                                                                                HGF2DLocation(*PreviousItr, GetCoordSys()),
                                                                                HGF2DLocation(*Itr, GetCoordSys()),
                                                                                false);
                                                }
                                            else if (TheLiteSegment.IsPointOn(*PreviousGivenItr))
                                                {
                                                // Given point is on self
                                                Answer = IntersectsAtSplitPoint(pi_rPolySegment,
                                                                                HGF2DLocation(*PreviousGivenItr, GetCoordSys()),
                                                                                HGF2DLocation(*GivenItr, GetCoordSys()),
                                                                                false);
                                                }
                                            }
                                        }
                                    else
                                        {
                                        // There is no next self segment ... we process
                                        // There is next given segment ... we process depending on which point is on
                                        if (GivenSegment.IsPointOn(*PreviousItr))
                                            {
                                            // Self point is on given
                                            Answer = IntersectsAtSplitPoint(pi_rPolySegment,
                                                                            HGF2DLocation(*PreviousItr, GetCoordSys()),
                                                                            HGF2DLocation(*Itr, GetCoordSys()),
                                                                            false);
                                            }
                                        else if (TheLiteSegment.IsPointOn(*PreviousGivenItr))
                                            {
                                            // Given point is on self
                                            Answer = IntersectsAtSplitPoint(pi_rPolySegment,
                                                                            HGF2DLocation(*PreviousGivenItr, GetCoordSys()),
                                                                            HGF2DLocation(*GivenItr, GetCoordSys()),
                                                                            false);
                                            }
                                        }


                                    }
                                }
                            else
                                {
                                // There is next given segment ... we process depending on which point is on
                                if (GivenSegment.IsPointOn(*PreviousItr))
                                    {
                                    // Self point is on given
                                    Answer = IntersectsAtSplitPoint(pi_rPolySegment,
                                                                    HGF2DLocation(*PreviousItr, GetCoordSys()),
                                                                    HGF2DLocation(*Itr, GetCoordSys()),
                                                                    false);
                                    }
                                else if (TheLiteSegment.IsPointOn(*PreviousGivenItr))
                                    {
                                    // Given point is on self
                                    Answer = IntersectsAtSplitPoint(pi_rPolySegment,
                                                                    HGF2DLocation(*PreviousGivenItr, GetCoordSys()),
                                                                    HGF2DLocation(*GivenItr, GetCoordSys()),
                                                                    false);
                                    }
                                }

                            }
                        // Not contiguous ... are points equal ?
                        else if (PreviousItr->IsEqualTo(*PreviousGivenItr))
                            {
                            Answer = IntersectsAtSplitPoint(pi_rPolySegment,
                                                            HGF2DLocation(*PreviousItr, GetCoordSys()),
                                                            HGF2DLocation(*Itr, GetCoordSys()),
                                                            false);
                            }

#endif

                        }
                    }
                }
            }
        }

    return(Answer);
    }


//-----------------------------------------------------------------------------
// IntersectsAtAnySplitPointWithLiteSegmentSCS
// This method determines if the given segment intersects any split point
//-----------------------------------------------------------------------------
bool HVE2DPolySegment::IntersectsAtAnySplitPointWithLiteSegmentSCS(const HGF2DLiteSegment& pi_rSegment) const
    {
    bool Answer = false;

    // Obtain tolerance
    double Tolerance = MIN(GetTolerance(), pi_rSegment.GetTolerance());

    // Obtain extremes of poly segments
    HGF2DExtent PolyExtent(GetExtent());


    double SelfXMin = PolyExtent.GetXMin();
    double SelfXMax = PolyExtent.GetXMax();
    double SelfYMin = PolyExtent.GetYMin();
    double SelfYMax = PolyExtent.GetYMax();

    double GivenXMin = MIN(pi_rSegment.GetStartPoint().GetX(), pi_rSegment.GetEndPoint().GetX());
    double GivenXMax = MAX(pi_rSegment.GetStartPoint().GetX(), pi_rSegment.GetEndPoint().GetX());
    double GivenYMin = MIN(pi_rSegment.GetStartPoint().GetY(), pi_rSegment.GetEndPoint().GetY());
    double GivenYMax = MAX(pi_rSegment.GetStartPoint().GetY(), pi_rSegment.GetEndPoint().GetY());

    // Check if their extents outter overlap
    bool Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                    HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                    HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                    HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                   );


    if (Result)
        {
        // Polysegment may interact with segment

        // For every segment of the self polysegment
        HGF2DPositionCollection::const_iterator Itr(m_Points.begin());
        HGF2DPositionCollection::const_iterator PreviousItr(Itr);
        ++PreviousItr;
        ++Itr;
        ++Itr;
        for (; !Answer && Itr != m_Points.end() ; ++Itr , ++PreviousItr)
            {
            // Obtain extent of this segment
            SelfXMin = MIN(Itr->GetX(), PreviousItr->GetX());
            SelfXMax = MAX(Itr->GetX(), PreviousItr->GetX());
            SelfYMin = MIN(Itr->GetY(), PreviousItr->GetY());
            SelfYMax = MAX(Itr->GetY(), PreviousItr->GetY());

            // Check if current segment and segment may interact
            Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                      HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                      HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                      HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                     );


            if (Result)
                {
                // Current segment and given polysegment may interact ...

                // Check if previous given point is on current segment
                if (pi_rSegment.IsPointOn(*PreviousItr))
                    {
                    // Create a lite segment to represent current segment
                    HGF2DLiteSegment TheLiteSegment(*PreviousItr, *Itr, Tolerance);

                    HGF2DPositionCollection::const_iterator PreviousToPreviousItr = PreviousItr;
                    --PreviousToPreviousItr;
                    HGF2DLiteSegment PreviousSegment(*PreviousToPreviousItr, *PreviousItr, Tolerance);


                    // Check if they are contiguous or connected
                    if (!(TheLiteSegment.AreContiguous(pi_rSegment) ||
                          PreviousSegment.AreContiguous(pi_rSegment) ||
                          PreviousSegment.IsPointOn(pi_rSegment.GetStartPoint()) ||
                          PreviousSegment.IsPointOn(pi_rSegment.GetEndPoint()) ||
                          TheLiteSegment.IsPointOn(pi_rSegment.GetStartPoint()) ||
                          TheLiteSegment.IsPointOn(pi_rSegment.GetEndPoint())))
                        {
                        // Obtain previous segment
                        Answer = pi_rSegment.IntersectsAtSplitPoint(PreviousSegment, TheLiteSegment);
                        }
                    }
                }
            }
        }

    return(Answer);
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

    // Indicate number of points
    po_rOutput << "There is " << m_Points.size() << "Points in PolySegment" << endl;
    HDUMP1("There is %" PRIu64 " Points in polysegment", (uint64_t)m_Points.size());

    // Dump all points
    char    DumString[256];
    HGF2DPositionCollection::const_iterator Itr;
    for (Itr = m_Points.begin() ; Itr != m_Points.end() ; Itr++)
        {
        sprintf(DumString, "%5.15lf , %5.15lf", (*Itr).GetX(), (*Itr).GetY());
        HDUMP0(DumString);
        HDUMP0("\n");
        po_rOutput << DumString << endl;
        }

#endif
    }

/** -----------------------------------------------------------------------------
    PRIVATE METHOD
    This method is an override ..

    @see IntersectsAtSplitPointWithPolySegment()
    -----------------------------------------------------------------------------
*/

bool HVE2DPolySegment::IntersectsAtSplitPointWithPolySegment(const HVE2DPolySegment& pi_rVector,
                                                              const HGF2DLocation& pi_rTestPoint,
                                                              const HGF2DLocation& pi_rNextEndPoint,
                                                              bool pi_ProcessNext) const
    {
    // The test point must be on self
    HASSERTSUPERDEBUG(IsPointOn(pi_rTestPoint));

    // If a second point is given, it must be on also
    HASSERTSUPERDEBUG(!pi_ProcessNext || IsPointOn(pi_rNextEndPoint));

    // The following procedure only works if the transformation model between vectors
    // Preserves direction
    HPRECONDITION((GetCoordSys() == pi_rVector.GetCoordSys()) ||
                  GetCoordSys()->HasDirectionPreservingRelationTo(pi_rVector.GetCoordSys()));

    bool Answer = false;

    // Obtain tolerance
    double Tolerance = MIN(GetTolerance(), pi_rVector.GetTolerance());

    // Check if end point is located on the given
    if (pi_rVector.IsPointOn(pi_rTestPoint, HVE2DVector::EXCLUDE_EXTREMITIES, Tolerance) &&
        !pi_rVector.IsNull())
        {
        if (!IsAtAnExtremity(pi_rTestPoint, Tolerance))
            {
            HGFBearing  Bearing0;
            HGFBearing  Bearing1;
            HGFBearing  Bearing2;
            HGFBearing  Bearing3;

            HGF2DLocation   FirstContiguousnessPoint(GetCoordSys());
            HGF2DLocation   SecondContiguousnessPoint(GetCoordSys());


            // Check if the two vectors are contiguous at this point
            if (AreContiguousAtAndGet(pi_rVector,
                                      pi_rTestPoint,
                                      &FirstContiguousnessPoint,
                                      &SecondContiguousnessPoint))
                {
                HGFBearing  Reference1;
                HGFBearing  Reference2;

                // The two vectors are contiguous at this point...
                // We must located the two contiguousness point
                // Obtain contiguousness points around the test point

                // Check if the end point of next component is part of this contiguousness
                // If this is the case, then the process is left to the next component
                bool ContinueProcess = true;
                if ((pi_ProcessNext) &&
                    (pi_rVector.IsPointOn(pi_rNextEndPoint)) &&
                    (AreContiguousAt(pi_rVector, pi_rNextEndPoint)))
                    {
                    HGF2DLocation   NextFirstContiguousnessPoint(GetCoordSys());
                    HGF2DLocation   NextSecondContiguousnessPoint(GetCoordSys());

                    // Obtain contiguousness points around the test point
                    ObtainContiguousnessPointsAt(pi_rVector,
                                                 pi_rNextEndPoint,
                                                 &NextFirstContiguousnessPoint,
                                                 &NextSecondContiguousnessPoint);

                    if ((FirstContiguousnessPoint.IsEqualTo(NextFirstContiguousnessPoint,
                                                            GetTolerance())) ||
                        (SecondContiguousnessPoint.IsEqualTo(NextSecondContiguousnessPoint,
                                                             GetTolerance())))
                        {
                        ContinueProcess = false;
                        }

                    }
                if (ContinueProcess && (!IsAtAnExtremity(FirstContiguousnessPoint, Tolerance) &&
                                        !IsAtAnExtremity(SecondContiguousnessPoint, Tolerance)))
                    {
                    // The next component end point is not part of the contiguousness ...

                    // Check that contiguousness is not located at an extremity of the vector
                    if (!pi_rVector.IsAtAnExtremity(FirstContiguousnessPoint, Tolerance) &&
                        !pi_rVector.IsAtAnExtremity(SecondContiguousnessPoint, Tolerance))
                        {

                        // Obtain relative position of each contiguousness points
                        double SelfRelativeFirst = CalculateRelativePosition(FirstContiguousnessPoint);
                        double SelfRelativeSecond = CalculateRelativePosition(SecondContiguousnessPoint);

                        double GivenRelativeFirst = pi_rVector.CalculateRelativePosition(FirstContiguousnessPoint);
                        double GivenRelativeSecond = pi_rVector.CalculateRelativePosition(SecondContiguousnessPoint);

                        if (SelfRelativeFirst < SelfRelativeSecond)
                            {
                            Bearing2 = CalculateBearing(FirstContiguousnessPoint,
                                                        HVE2DVector::ALPHA);
                            Bearing3 = CalculateBearing(SecondContiguousnessPoint,
                                                        HVE2DVector::BETA);
                            Reference2 = CalculateBearing(SecondContiguousnessPoint, HVE2DVector::ALPHA);
                            }
                        else
                            {
                            Bearing2 = CalculateBearing(FirstContiguousnessPoint,
                                                        HVE2DVector::BETA);
                            Bearing3 = CalculateBearing(SecondContiguousnessPoint,
                                                        HVE2DVector::ALPHA);
                            Reference2 = CalculateBearing(SecondContiguousnessPoint, HVE2DVector::BETA);
                            }


                        if (GivenRelativeFirst < GivenRelativeSecond)
                            {
                            Bearing0 = pi_rVector.CalculateBearing(FirstContiguousnessPoint,
                                                                   HVE2DVector::ALPHA);
                            Bearing1 = pi_rVector.CalculateBearing(SecondContiguousnessPoint,
                                                                   HVE2DVector::BETA);
                            Reference1 = pi_rVector.CalculateBearing(FirstContiguousnessPoint, HVE2DVector::BETA);
                            }
                        else
                            {
                            Bearing0 = pi_rVector.CalculateBearing(FirstContiguousnessPoint,
                                                                   HVE2DVector::BETA);
                            Bearing1 = pi_rVector.CalculateBearing(SecondContiguousnessPoint,
                                                                   HVE2DVector::ALPHA);
                            Reference1 = pi_rVector.CalculateBearing(FirstContiguousnessPoint, HVE2DVector::ALPHA);

                            }


                        double B3MinusRef2 = CalculateNormalizedTrigoValue(Bearing3 - Reference2);
                        double B1MinusRef2 = CalculateNormalizedTrigoValue(Bearing1 - Reference2);
                        double B2MinusRef1 = CalculateNormalizedTrigoValue(Bearing2 - Reference1);
                        double B0MinusRef1 = CalculateNormalizedTrigoValue(Bearing0 - Reference1);

                        bool   FirstAnswer = (B3MinusRef2 < B1MinusRef2);

                        bool   SecondAnswer = (B0MinusRef1 < B2MinusRef1);

                        Answer = ((!FirstAnswer && SecondAnswer) || (FirstAnswer && !SecondAnswer));
                        }
                    }
                }
            else
                {
                // Obtain bearings at point
                Bearing0 = pi_rVector.CalculateBearing(pi_rTestPoint, HVE2DVector::ALPHA);
                Bearing1 = pi_rVector.CalculateBearing(pi_rTestPoint, HVE2DVector::BETA);
                Bearing2 = CalculateBearing(pi_rTestPoint, HVE2DVector::ALPHA);
                Bearing3 = CalculateBearing(pi_rTestPoint, HVE2DVector::BETA);

                double B3MinusB0 = CalculateNormalizedTrigoValue(Bearing3 - Bearing0);
                double B1MinusB0 = CalculateNormalizedTrigoValue(Bearing1 - Bearing0);
                double B2MinusB0 = CalculateNormalizedTrigoValue(Bearing2 - Bearing0);


                bool   FirstAnswer = (B3MinusB0 < B1MinusB0);

                bool   SecondAnswer = (B2MinusB0 < B1MinusB0);


                Answer = ((!FirstAnswer && SecondAnswer) || (FirstAnswer && !SecondAnswer));
                }
            }
        }

    return(Answer);
    }

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
    HPRECONDITION(!pi_ProcessClosed || (m_StartPoint == m_EndPoint));

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
                if (MySplitPointItr->IsEqualTo((*PolySegmentItr)->m_StartPoint, GetTolerance()))
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
    double TotalAreaSigned = CalculateRayArea(HGF2DLocation(m_StartPoint, GetCoordSys()));

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
                                    for (long j = (long)((*FoundItr)->GetSize() - 2) ; j >= 0 ; --j)
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
                    for (long j = (long)((*FoundItr)->GetSize() - 2) ; j >= 0 ; --j)
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
