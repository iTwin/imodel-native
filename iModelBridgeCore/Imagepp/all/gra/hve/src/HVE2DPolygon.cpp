//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hve/src/HVE2DPolygon.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HVE2DPolygon
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HVE2DPolygon.h>
#include <Imagepp/all/h/HGF2DPolygonOfSegments.h>

HPM_REGISTER_CLASS(HVE2DPolygon, HVE2DSimpleShape)


#include <Imagepp/all/h/HVE2DRectangle.h>
#include <Imagepp/all/h/HVE2DSegment.h>
//-----------------------------------------------------------------------------
// Constructor from rectangle
//-----------------------------------------------------------------------------
HVE2DPolygon::HVE2DPolygon(const HVE2DRectangle& pi_rRectangle)
    : HVE2DSimpleShape(pi_rRectangle.GetCoordSys()),
      m_ComplexLinear(pi_rRectangle.GetCoordSys())
    {
    if (!pi_rRectangle.IsEmpty())
        {
        double     XMin;
        double     XMax;
        double     YMin;
        double     YMax;

        // Extract rectangle definition
        pi_rRectangle.GetRectangle(&XMin, &YMin, &XMax, &YMax);

        // Create first segment
        HVE2DSegment            MyTempSegment(pi_rRectangle.GetCoordSys());
        MyTempSegment.SetRawStartPoint(XMin,YMin);
        MyTempSegment.SetRawEndPoint(XMax,YMin);

        // Add first segment to complex linear
        m_ComplexLinear.AppendLinear(MyTempSegment);

        // Create second segment
        MyTempSegment.SetRawStartPoint(XMax, YMin);
        MyTempSegment.SetRawEndPoint(XMax, YMax);

        // Add second segment to complex linear
        m_ComplexLinear.AppendLinear(MyTempSegment);

        // Create third segment
        MyTempSegment.SetRawStartPoint(XMax, YMax);
        MyTempSegment.SetRawEndPoint(XMin, YMax);

        // Add third segment to complex linear
        m_ComplexLinear.AppendLinear(MyTempSegment);

        // Create last segment
        MyTempSegment.SetRawStartPoint(XMin, YMax);
        MyTempSegment.SetRawEndPoint(XMin, YMin);

        // Add last segment to complex linear
        m_ComplexLinear.AppendLinear(MyTempSegment);

        }

    // Extract tolerance settings from rectangle
    SetAutoToleranceActive(pi_rRectangle.IsAutoToleranceActive());
    SetTolerance(pi_rRectangle.GetTolerance());
    }

//-----------------------------------------------------------------------------
// SetCoordSysImplementation
// Changes the interpretation coordinate system
//-----------------------------------------------------------------------------
void HVE2DPolygon::SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    {
    // Call ancester set
    HVE2DShape::SetCoordSysImplementation(pi_rpCoordSys);

    // Change coordinate system of linear
    m_ComplexLinear.HVE2DComplexLinear::SetCoordSys(GetCoordSys());
    }


//-----------------------------------------------------------------------------
// AllocateCopyInCoordSys
// This method allocates dynamically a copy of the polygon
//-----------------------------------------------------------------------------
HVE2DVector* HVE2DPolygon::AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const
    {
    HVE2DVector*    pResultVector;

    if (GetCoordSys() == pi_rpCoordSys)
        {
        pResultVector = new HVE2DPolygon(*this);
        }
    else
        {
        // We obtain a copy of the linear in given coord sys
        HVE2DComplexLinear* pMyResultLinear = (HVE2DComplexLinear*)(m_ComplexLinear.AllocateCopyInCoordSys(pi_rpCoordSys));

        // we generate a polygon with it and return it
        pResultVector = new HVE2DPolygon(*pMyResultLinear);

        pResultVector->SetStrokeTolerance(m_pStrokeTolerance);

        // We destroy the temporary linear
        delete pMyResultLinear;
        }

    return(pResultVector);
    }

//-----------------------------------------------------------------------------
// CalculateBearing
// This method returns the bearing at specified position
//-----------------------------------------------------------------------------
HGFBearing HVE2DPolygon::CalculateBearing(const HGF2DLocation& pi_rPositionPoint,
                                          HVE2DVector::ArbitraryDirection pi_Direction) const
    {
    // The point must be located on polygon
    HPRECONDITION(IsPointOn(pi_rPositionPoint));

    HGFBearing  ReturnValue;

    // Check if it is the end point and direction is ALPHA
    if ((pi_rPositionPoint.IsEqualTo(m_ComplexLinear.GetEndPoint())) && (pi_Direction == HVE2DVector::ALPHA))
        {
        // The given point is the end point of complex ... special processing

        // Create iterator on complex linear
        HVE2DComplexLinear::LinearList::const_reverse_iterator MyIterator = m_ComplexLinear.GetLinearList().rbegin();

        // Obtain bearing from last linear in complex
        ReturnValue = (*MyIterator)->CalculateBearing(pi_rPositionPoint, pi_Direction);
        }
    else
        {
        // Normal operation ...
        ReturnValue = m_ComplexLinear.CalculateBearing(pi_rPositionPoint, pi_Direction);
        }

    return(ReturnValue);
    }


//-----------------------------------------------------------------------------
// CalculateAngularAcceleration
// This method returns the angular acceleration at specified position
//-----------------------------------------------------------------------------
double HVE2DPolygon::CalculateAngularAcceleration(const HGF2DLocation& pi_rPositionPoint,
                                                  HVE2DVector::ArbitraryDirection pi_Direction) const
    {
    // The point must be located on polygon
    HPRECONDITION(IsPointOn(pi_rPositionPoint));

    double  ReturnValue;

    // Check if it is the end point and direction is ALPHA
    if ((pi_rPositionPoint == m_ComplexLinear.GetEndPoint()) && (pi_Direction == HVE2DVector::ALPHA))
        {
        // The given point is the end point of complex ... special processing

        // Create iterator on complex linear
        HVE2DComplexLinear::LinearList::const_reverse_iterator MyIterator = m_ComplexLinear.GetLinearList().rbegin();

        // Obtain bearing from last linear in complex
        ReturnValue = (*MyIterator)->CalculateAngularAcceleration(pi_rPositionPoint, pi_Direction);
        }
    else
        {
        // Normal operation ...
        ReturnValue = m_ComplexLinear.CalculateAngularAcceleration(pi_rPositionPoint, pi_Direction);
        }

    return(ReturnValue);
    }



//-----------------------------------------------------------------------------
// IsPointIn
// This method checks if the point is located inside the polygon area
//-----------------------------------------------------------------------------
bool HVE2DPolygon::IsPointIn(const HGF2DLocation& pi_rPoint, double pi_Tolerance) const
    {
    bool   Answer = false;

    // Set Tolerance
    double Tolerance = pi_Tolerance;
    if (pi_Tolerance == HVE_USE_INTERNAL_EPSILON)
        Tolerance = GetTolerance();

    // Obtain point expresed in appropriate coordinate system
    HGF2DLocation   TempPoint(pi_rPoint, GetCoordSys());

    if (GetExtent().IsPointIn(TempPoint))
        {
        size_t  NumberOfCrossPoints = 0;

        double X = TempPoint.GetX();
        double Y = TempPoint.GetY();

        // Evaluate the external X coordinate
        double     PolyXMin = GetExtent().GetXMin();
        double     TheX = (PolyXMin - fabs(PolyXMin) - 1.0);

        // This could be optimised with the use of a real horizontal segment
        HVE2DSegment    MyHorizontalSegment(pi_rPoint, HGF2DLocation(TheX, pi_rPoint.GetY(), GetCoordSys()));

        HVE2DComplexLinear::LinearList::const_iterator Itr;
        for (Itr = m_ComplexLinear.GetLinearList().begin() ; Itr != m_ComplexLinear.GetLinearList().end() ; Itr++)
            {
            HGF2DExtent LinearExtent((*Itr)->GetExtent());

            // Check if the segment can possibly intersect
            double XMin = LinearExtent.GetXMin();
            double XMax = LinearExtent.GetXMax();
            double YMin = LinearExtent.GetYMin();
            double YMax = LinearExtent.GetYMax();

            // A linear which is all farther than x value of point cannot interact
            if ((XMin < X) || HDOUBLE_EQUAL(XMin, X, Tolerance))
                {
                // The linear is possibly corectly positioned
                double XStart = (*Itr)->GetStartPoint().GetX();
                double XEnd = (*Itr)->GetEndPoint().GetX();
                double YStart = (*Itr)->GetStartPoint().GetY();
                double YEnd = (*Itr)->GetEndPoint().GetY();

                // if the Y values are both greater or smaller than that of point
                // cannot intersect either
                if (HDOUBLE_SMALLER(YMin, Y, Tolerance) && HDOUBLE_GREATER(YMax, Y, Tolerance))
                    {
                    // The linear possibly crosses
                    // If the maximum X is smaller than X of point, there shurely is a crossing

                    if HDOUBLE_SMALLER_OR_EQUAL(XMax, X, Tolerance)
                        {
                        // There is at least one crossing, but there may be more

                        // Check if the Y of start and Y of end are not all in positive Y or
                        // not all in negative Ys
                        if ((HDOUBLE_SMALLER(YStart, Y, Tolerance) && HDOUBLE_GREATER(YEnd, Y, Tolerance)) ||
                            (HDOUBLE_GREATER(YStart, Y, Tolerance) && HDOUBLE_SMALLER(YEnd, Y, Tolerance)))
                            // There is an odd number of cross points
                            NumberOfCrossPoints++;
                        // One of the extremity can possibly connect on horizontal segment
                        else if ((HDOUBLE_EQUAL(YStart, Y, Tolerance) || HDOUBLE_EQUAL(YEnd, Y, Tolerance)))
                            {
                            // Check that not both extremity are located on horizontal segment
                            if (!(HDOUBLE_EQUAL(YStart, Y, Tolerance) && HDOUBLE_EQUAL(YEnd, Y, Tolerance)))
                                {
                                if ((HDOUBLE_EQUAL(YStart, Y, Tolerance) && (YEnd > Y)) ||
                                    (HDOUBLE_EQUAL(YEnd, Y, Tolerance) && (YStart > Y)))
                                    NumberOfCrossPoints++;

                                }

                            }
                        }
                    else
                        {
                        // This case is more complicated
                        // We first obtain intersection points between linear and intersection point
                        HGF2DLocationCollection CrossPoints;
                        NumberOfCrossPoints += (*Itr)->Intersect(MyHorizontalSegment, &CrossPoints);

                        // Check if on of the extremities is located on horizontal segment
                        // One of the extremity can possibly connect on horizontal segment
                        if ((HDOUBLE_EQUAL(YStart, Y, Tolerance) && (XStart < X)) ||
                            (HDOUBLE_EQUAL(YEnd, Y, Tolerance) && (XEnd < X)))
                            {
                            // Check that it does not connect by both points
                            if (!((HDOUBLE_EQUAL(YStart, Y, Tolerance) && (XStart < X)) &&
                                  (HDOUBLE_EQUAL(YEnd, Y, Tolerance) && (XEnd < X))))
                                {
                                // Only one extremity connects upon horizontal segment
                                // Depending on which
                                if (HDOUBLE_EQUAL(YStart, Y, Tolerance) && HDOUBLE_SMALLER(XStart, X, Tolerance))
                                    {
                                    if ((*Itr)->TendsToTheLeftOfSCS(MyHorizontalSegment, (*Itr)->GetStartPoint(), HVE2DVector::BETA))
                                        NumberOfCrossPoints++;
                                    }
                                else
                                    {
                                    if ((*Itr)->TendsToTheLeftOfSCS(MyHorizontalSegment, (*Itr)->GetEndPoint(), HVE2DVector::ALPHA))
                                        NumberOfCrossPoints++;
                                    }

                                }
                            }
                        }
                    }
                // Check if extent of linear touches (is contiguous) to horizontal segment
                else if (HDOUBLE_GREATER(YMax, Y, Tolerance) && HDOUBLE_EQUAL(YMin, Y, Tolerance) &&
                         ((HDOUBLE_EQUAL(YStart, Y, Tolerance) && HDOUBLE_SMALLER(XStart, X, Tolerance)) ||
                          (HDOUBLE_EQUAL(YEnd, Y, Tolerance) && HDOUBLE_SMALLER(XEnd, X, Tolerance))))
                    {
                    // The linear has greater Y values that point, but connects by one point in lower X

                    // Check that it does not connect by both points
                    if (!((HDOUBLE_EQUAL(YStart, Y, Tolerance) && HDOUBLE_SMALLER(XStart, X, Tolerance)) &&
                          (HDOUBLE_EQUAL(YEnd, Y, Tolerance) && HDOUBLE_SMALLER(XEnd, X, Tolerance))))
                        NumberOfCrossPoints++;
                    }
                }
            }

        Answer = ((NumberOfCrossPoints % 2 == 1) && (!IsPointOnSCS(pi_rPoint, INCLUDE_EXTREMITIES, Tolerance)));
        }

    return (Answer);
    }


//-----------------------------------------------------------------------------
// ObtainContiguousnessPoints
// Finds contiguousness points with vector
// If start point is in a contiguousness region, then this point
// will be located in first contiguousness region
//-----------------------------------------------------------------------------
size_t HVE2DPolygon::ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                HGF2DLocationCollection* po_pContiguousnessPoints) const
    {
    // Save number of points in location collection
    size_t  NumberOfNewPointsFound;
    size_t  CollectionInitialNumberOfPoints = po_pContiguousnessPoints->size();

    // Ask complex linear to obtain contiguousness points
    if ((NumberOfNewPointsFound = m_ComplexLinear.ObtainContiguousnessPoints(pi_rVector, po_pContiguousnessPoints)) > 0)
        {
        // There are some contiguousness points ... check first and last point found
        if ((*po_pContiguousnessPoints)[CollectionInitialNumberOfPoints].IsEqualTo((*po_pContiguousnessPoints)[po_pContiguousnessPoints->size() - 1]))
            {
            // The first and last ones are equal ... we must remove those ...

            // We first check if there are some remaining points found ....
            if (NumberOfNewPointsFound > 2)
                {
                // There are more than 2 new contiguousness points ... the first one become the
                // previous to last
                (*po_pContiguousnessPoints)[CollectionInitialNumberOfPoints] = (*po_pContiguousnessPoints)[po_pContiguousnessPoints->size() - 2];
                }

            // We remove the last 2 entries
            po_pContiguousnessPoints->pop_back();
            po_pContiguousnessPoints->pop_back();

            // We adjust the number of new points found
            NumberOfNewPointsFound -= 2;
            }

        }

    return(NumberOfNewPointsFound);
    }


//-----------------------------------------------------------------------------
// ObtainContiguousnessPointsAt
// Finds contiguousness points with vector at specified point
//-----------------------------------------------------------------------------
void HVE2DPolygon::ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
                                                const HGF2DLocation& pi_rPoint,
                                                HGF2DLocation* po_pFirstContiguousnessPoint,
                                                HGF2DLocation* po_pSecondContiguousnessPoint) const
    {
    // The two vectors must be contiguous at specified point
    HPRECONDITION(AreContiguousAt(pi_rVector, pi_rPoint));

    // Ask complex linear to obtain contiguousness points at
    m_ComplexLinear.ObtainContiguousnessPointsAt(pi_rVector,
                                                 pi_rPoint,
                                                 po_pFirstContiguousnessPoint,
                                                 po_pSecondContiguousnessPoint);

    // The preceeding piece of code pre-supposed that the
    // contiguousness points are returned in increasing ALPHA
    // and some other assumptions which should not have been expected ...
    // Check if first contiguousness point is the start point
    if ((*po_pFirstContiguousnessPoint).IsEqualTo(m_ComplexLinear.GetStartPoint()) ||
        (*po_pSecondContiguousnessPoint).IsEqualTo(m_ComplexLinear.GetStartPoint()))
        {
        // We obtain all contiguousness points
        HGF2DLocationCollection     AllContiguousnessPoints;
        ObtainContiguousnessPoints(pi_rVector, &AllContiguousnessPoints);

        if(AllContiguousnessPoints.size() > 1)
            {
            // We know from nature of method that the start point is always located
            // in first returned contiguousness region
            *po_pFirstContiguousnessPoint = AllContiguousnessPoints[0];
            *po_pSecondContiguousnessPoint = AllContiguousnessPoints[1];
            }
        }
    }


//-----------------------------------------------------------------------------
// Crosses
// This method checks if the polygon crosses with given vector.
//-----------------------------------------------------------------------------
bool HVE2DPolygon::Crosses(const HVE2DVector& pi_rVector) const
    {
    // Check if complex linear crosses
    bool Answer = m_ComplexLinear.Crosses(pi_rVector);

    // If the complex does not cross
    if (!Answer)
        {
        // It does not cross, but there still may be a crossing at auto-closing point
        // We check if end point of complex is located on vector
        // We exclude from the test the extremities of vector, since then it would not cross
        if ((GetCoordSys() == pi_rVector.GetCoordSys()) ||
            GetCoordSys()->HasDirectionPreservingRelationTo(pi_rVector.GetCoordSys()))
            {
            Answer = IntersectsAtSplitPoint(pi_rVector, m_ComplexLinear.GetEndPoint(), m_ComplexLinear.GetEndPoint(), false);
            }
        else
            {
            // The coordinate system do not have a direction preserving
            // relation

            // Allocate a copy of vector in another coordinate system
            HVE2DVector* pTempVector = pi_rVector.AllocateCopyInCoordSys(GetCoordSys());

            // Obtain answer
            Answer = IntersectsAtSplitPoint(*pTempVector, m_ComplexLinear.GetEndPoint(), m_ComplexLinear.GetEndPoint(), false);

            // Destroy temporary copy
            delete pTempVector;
            }
        }

    return (Answer);

    }

//-----------------------------------------------------------------------------
// Intersect
// This method intersects with given vector and returns the cross points
//-----------------------------------------------------------------------------
size_t HVE2DPolygon::Intersect(const HVE2DVector& pi_rVector,
                               HGF2DLocationCollection* po_pCrossPoints) const
    {
    size_t NumberOfNewCrossPoints;

    // Ask linear for intersect points
    NumberOfNewCrossPoints = m_ComplexLinear.Intersect(pi_rVector, po_pCrossPoints);

    // Now it is quite possible that a vector crosses at the junction point between
    // complex extremities. It is also possible that the vector is contiguous
    // over this junction and does cross somewhere
    if ((GetCoordSys() == pi_rVector.GetCoordSys()) ||
        GetCoordSys()->HasDirectionPreservingRelationTo(pi_rVector.GetCoordSys()))
        {
        if (IntersectsAtSplitPoint(pi_rVector, m_ComplexLinear.GetEndPoint(), m_ComplexLinear.GetEndPoint(), false))
            {
            po_pCrossPoints->push_back(m_ComplexLinear.GetEndPoint());

            NumberOfNewCrossPoints++;
            }
        }
    else
        {
        // The coordinate system do not have a direction preserving
        // relation

        // Allocate a copy of vector in another coordinate system
        HVE2DVector* pTempVector = pi_rVector.AllocateCopyInCoordSys(GetCoordSys());

        if (IntersectsAtSplitPoint(*pTempVector, m_ComplexLinear.GetEndPoint(), m_ComplexLinear.GetEndPoint(), false))
            {
            po_pCrossPoints->push_back(m_ComplexLinear.GetEndPoint());

            NumberOfNewCrossPoints++;
            }

        // Destroy temporary copy
        delete pTempVector;
        }

    return(NumberOfNewCrossPoints);
    }


//-----------------------------------------------------------------------------
// AllocateFence
// Allocates a fence representing polygon 
//-----------------------------------------------------------------------------
HGF2DShape* HVE2DPolygon::GetLightShape() const
{
    // Allocate a new polygon fence
    HAutoPtr<HGF2DPolygonOfSegments > pNewFence;

    // Check if polygon is not empty
    if (!IsEmpty())
        {
        // Get access to list of points in polygon fence
        HGF2DPositionCollection ListOfPoints;

        HGF2DLocationCollection Points;

        m_ComplexLinear.Drop(&Points, GetTolerance());

        HGF2DLocationCollection::iterator Iterator;
        for (Iterator = Points.begin() ; Iterator != Points.end() ; Iterator++)
            {
            ListOfPoints.push_back(HGF2DPosition(Iterator->GetX(), Iterator->GetY()));
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
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
void HVE2DPolygon::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    HVE2DSimpleShape::PrintState(po_rOutput);

    po_rOutput << "BEGIN Dumping a HVE2DPolygonOfSegments object" << endl;
    HDUMP0("BEGIN Dumping a HVE2DPolygonOfSegments object\n");

    m_ComplexLinear.PrintState(po_rOutput);

    po_rOutput << "END (HVE2DPolygonOfSegments)" << endl;
    HDUMP0("END (HVE2DPolygonOfSegments)\n");
#endif
    }


