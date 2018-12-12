//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DShape.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HGF2DShape
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <ImagePP/all/h/HGF2DShape.h>
#include <ImagePP/all/h/HGF2DDisplacement.h>


// HPM_REGISTER_ABSTRACT_CLASS(HGF2DShape, HGF2DVector)


#include <ImagePP/all/h/HGF2DSimpleShape.h>
#include <ImagePP/all/h/HGF2DHoledShape.h>
#include <ImagePP/all/h/HGF2DSegment.h>
#include <ImagePP/all/h/HGFScanlines.h>


//-----------------------------------------------------------------------------
// Rasterize
// This method rasterizes (generates scanlines) for the shape.
//-----------------------------------------------------------------------------
void HGF2DShape::Rasterize(HGFScanLines& pio_rScanlines) const
    {

    // Check if shape is not empty
    if (!IsEmpty())
        {
        // Obtain extent of shape
        HGF2DLiteExtent ShapeExtent = GetExtent();

        if (!pio_rScanlines.LimitsAreSet())
            {
            // HChk MR: For now, 6 is arbitrary
            pio_rScanlines.SetLimits(ShapeExtent.GetYMin(), ShapeExtent.GetYMax(), 6);
            }

        // Increase X dimension extent to include surely all of the shape
        // One tenth of the value is removed/added to be sure oll of the shape is
        // more than included (hence the number 10.0)
        double XMin = ShapeExtent.GetXMin() -
                       (fabs(ShapeExtent.GetXMin() / 10.0)) - 1.0;
        double XMax = ShapeExtent.GetXMax() + (fabs(ShapeExtent.GetXMax() / 10.0)) + 1.0;


        // For each valid other Y position
        for (double Y = pio_rScanlines.GetFirstScanlinePosition() ; Y < ShapeExtent.GetYMax() ; Y += 1.0)
            {
            // Create a segment
            HGF2DSegment    HorizontalSegment(HGF2DPosition(XMin, Y), HGF2DPosition(XMax, Y));

            // Create recipient list for cross points
            HGF2DPositionCollection CrossPoints;

            // Perform intersection with shape
            Intersect(HorizontalSegment, &CrossPoints);

            // Check if segment is contiguous to shape
            if (AreContiguous(HorizontalSegment))
                {
                // The segment is contiguous ... we obtain contiguousness points
                ObtainContiguousnessPoints(HorizontalSegment, &CrossPoints);
                }

            // Perform intersection with shape
            if (CrossPoints.size() > 0)
                {
                // There must be an even number of points
                HASSERT(CrossPoints.size() % 2 == 0);

                HGF2DPositionCollection::const_iterator Itr;
                for (Itr = CrossPoints.begin() ; Itr != CrossPoints.end() ; ++Itr)
                    pio_rScanlines.AddCrossingPoint((HSINTX)Y, Itr->GetX());
                }
            }
        }
    }




/** -----------------------------------------------------------------------------
    This method calculates and returns the spatial position of given
    vector relative to the area defined by the shape. There are four
    possible answer resulting from operation: (HGF2DShape::S_OUT,
    HGF2DShape::S_IN, HGF2DShape::S_PARTIALY_IN,
    HGF2DShape::S_ON). The first one indicates that the whole of the given
    vector is located outside the area defined by the shape. This does not
    exclude the possibility that the given vector may share parts of the
    shape boundary (flirting, countiguousness). If HGF2DShape::S_IN is
    returned, then the vector is located completely inside the area
    enclosed by the shape. This does not exclude the possibility that the given
    vector may share parts of the shape boundary (flirting, contiguousness).
    If HGF2DShape::S_PARTIALY_IN is returned, then the object is located
    partly inside and partly outside the area defined by the shape. Finally
    HGF2DShape::S_ON is returned is the path of the given vector is completely
    located on the shape boundary, with no in nor out parts.

    @param pi_rVector The vector of which spatial position of must be determined.

    @return The spatial position of vector.

    Example:
    @code
    @end

    @see HGF2DVector
    -----------------------------------------------------------------------------
*/
HGF2DShape::SpatialPosition HGF2DShape::CalculateSpatialPositionOf(
    const HGF2DVector& pi_rVector) const
    {
    HGF2DShape::SpatialPosition     ThePosition = HGF2DShape::S_OUT;

    // Check if their extents overlap
    if (GetExtent().OutterOverlaps(pi_rVector.GetExtent(),
                                   MIN(GetTolerance(), pi_rVector.GetTolerance())))
        {

        // Check if vector is made of multiple entities
        if (pi_rVector.GetMainVectorType() == HGF2DLinear::CLASS_ID ||
            (pi_rVector.GetMainVectorType() == HGF2DShape::CLASS_ID &&
             ((HGF2DShape*)&pi_rVector)->IsSimple()))
            {
            // We have a single component vector
            ThePosition = CalculateSpatialPositionOfSingleComponentVector(pi_rVector);
            }
        else
            {
            // We have a possibly disjoint multiple component vector
            ThePosition = CalculateSpatialPositionOfMultipleComponentVector(pi_rVector);
            }
        }

    return (ThePosition);
    }



//-----------------------------------------------------------------------------
// CalculateSpatialPositionOfSingleComponentVector
// This method returns the spatial position relative to shape of given vector
//-----------------------------------------------------------------------------
HGF2DShape::SpatialPosition HGF2DShape::CalculateSpatialPositionOfSingleComponentVector(
    const HGF2DVector& pi_rVector) const
    {
    // The given vector must be composed of a single entity
    HPRECONDITION(pi_rVector.GetMainVectorType() == HGF2DLinear::CLASS_ID ||
                  (pi_rVector.GetMainVectorType() == HGF2DShape::CLASS_ID &&
                   ((HGF2DShape*)&pi_rVector)->IsSimple()));

    HGF2DShape::SpatialPosition     ThePosition = HGF2DShape::S_OUT;

    // Check if their extents overlap
    if (GetExtent().OutterOverlaps(pi_rVector.GetExtent(),
                                   MIN(GetTolerance(), pi_rVector.GetTolerance())))
        {
        // The extents overlap ... check if they cross ?
        if (Crosses(pi_rVector))
            {
            // Since they do cross, the vector is PARTIALY IN (passes through the shape boundary)
            ThePosition = HGF2DShape::S_PARTIALY_IN;
            }
        else
            {
            // They do not cross
            // We first determine of which type is the vector
            if (pi_rVector.GetMainVectorType()== HGF2DShape::CLASS_ID)
                {
                // The vector is a shape ... We cast as a shape
                HGF2DSimpleShape*     pMyShape = (HGF2DSimpleShape*)(&pi_rVector);

                ThePosition = CalculateSpatialPositionOfNonCrossingSimpleShape(*pMyShape);
                }
            else
                {
                // We have a linear ... we cast
                HGF2DLinear*    pMyLinear = (HGF2DLinear*)(&pi_rVector);

                ThePosition = CalculateSpatialPositionOfNonCrossingLinear(*pMyLinear);
                }
            }
        }

    return (ThePosition);
    }


//-----------------------------------------------------------------------------
// CalculateSpatialPositionOfMultipleComponentVector
// PRIVATE METHOD
// This method returns the spatial position relative to shape of given vector
//-----------------------------------------------------------------------------
HGF2DShape::SpatialPosition HGF2DShape::CalculateSpatialPositionOfMultipleComponentVector(
    const HGF2DVector& pi_rVector) const
    {
    // The given vector must be composed of posibly multiple entities
    HPRECONDITION(pi_rVector.GetMainVectorType() != HGF2DLinear::CLASS_ID &&
                  !(pi_rVector.GetMainVectorType() == HGF2DShape::CLASS_ID &&
                    ((HGF2DShape*)&pi_rVector)->IsSimple()));

    HGF2DShape::SpatialPosition     ThePosition;

    // There are only two posible multiple component shapes
    if (((HGF2DShape*)&pi_rVector)->IsComplex())
        {
        // It is a complex shape
        ThePosition = CalculateSpatialPositionOfComplexShape(*((HGF2DShape*)&pi_rVector));
        }
    else
        {
        // It is a holed shape
        ThePosition = CalculateSpatialPositionOfHoledShape(*((HGF2DHoledShape*)&pi_rVector));
        }

    return(ThePosition);
    }


//-----------------------------------------------------------------------------
// CalculateSpatialPositionOfComplexShape
// PRIVATE METHOD
// This method returns the spatial position relative to shape of given vector
//-----------------------------------------------------------------------------
HGF2DShape::SpatialPosition HGF2DShape::CalculateSpatialPositionOfComplexShape(
    const HGF2DShape& pi_rShape) const
    {
    // Their extent must overlap
    HPRECONDITION(GetExtent().OutterOverlaps(pi_rShape.GetExtent(),
                                             MIN(GetTolerance(), pi_rShape.GetTolerance())));

    // The shape must be complex
    HPRECONDITION(pi_rShape.IsComplex());

    HGF2DShape::SpatialPosition     ThePosition = HGF2DShape::S_ON;

    // The shape is complex ... we ask the spatial position of every component shape
    // until resolution
    HGF2DShape::ShapeList::const_iterator  MyIterator = pi_rShape.GetShapeList().begin();

    // Loop till all components have been processed or partialy in
    while ((MyIterator != pi_rShape.GetShapeList().end()) &&
           (ThePosition != HGF2DShape::S_PARTIALY_IN))
        {
        // We ask the spatial position of current shape
        HGF2DShape::SpatialPosition TempPosition = CalculateSpatialPositionOf(**MyIterator);

        // If the current component position is different from on
        if (TempPosition != HGF2DShape::S_ON)
            {
            // If the previous result was on
            if (ThePosition == HGF2DShape::S_ON)
                {
                // Previous position was ON ... set current component position
                ThePosition = TempPosition;
                }
            else
                {
                // Previous position is either IN or OUT
                // Check if different component position
                if (TempPosition != ThePosition)
                    ThePosition = HGF2DShape::S_PARTIALY_IN;
                }
            }

        ++MyIterator;
        }

    return (ThePosition);
    }


//-----------------------------------------------------------------------------
// CalculateSpatialPositionOfHoledShape
// PRIVATE METHOD
// This method returns the spatial position relative to shape of given vector
//-----------------------------------------------------------------------------
HGF2DShape::SpatialPosition HGF2DShape::CalculateSpatialPositionOfHoledShape(
    const HGF2DHoledShape& pi_rHoledShape) const
    {
    // Their extent must overlap
    HPRECONDITION(GetExtent().OutterOverlaps(pi_rHoledShape.GetExtent(),
                                             MIN(GetTolerance(), pi_rHoledShape.GetTolerance())));

    HGF2DShape::SpatialPosition     ThePosition = HGF2DShape::S_ON;

    // We obtain spatial position of holed shape base
    ThePosition = CalculateSpatialPositionOf(pi_rHoledShape.GetBaseShape());

    // We now know the spatial position of outter shape.
    // If all components of holed shape are IN or ON then holed is IN
    // If all components of holed shape are OUT or ON then holed is OUT
    // If all componenents are ON then holed is ON

    // Check if holed has holes
    if (pi_rHoledShape.HasHoles())
        {
        // For every hole
        HGF2DShape::HoleList::const_iterator Itr = pi_rHoledShape.GetHoleList().begin();

        // loop till all holes processed or shape is partialy in
        while (Itr != pi_rHoledShape.GetHoleList().end() &&
               (ThePosition != HGF2DShape::S_PARTIALY_IN))
            {
            // Obtain spatial position of holed
            HGF2DShape::SpatialPosition HolePosition = CalculateSpatialPositionOf(**Itr);

            //If the hole position is not on
            if (HolePosition != HGF2DShape::S_ON)
                {
                // If the previous result was on
                if (ThePosition == HGF2DShape::S_ON)
                    {
                    // Previous position was ON ... set current hole position
                    ThePosition = HolePosition;
                    }
                else
                    {
                    // Previous position is either IN or OUT
                    // Check if different hole position
                    if (HolePosition != ThePosition)
                        ThePosition = HGF2DShape::S_PARTIALY_IN;
                    }
                }

            ++Itr;
            }
        }

    return (ThePosition);
    }





//-----------------------------------------------------------------------------
// CalculateSpatialPositionOfNonCrossingSimpleShape
// PRIVATE METHOD
// This method returns the spatial position relative to shape of given shape
//-----------------------------------------------------------------------------
HGF2DShape::SpatialPosition HGF2DShape::CalculateSpatialPositionOfNonCrossingSimpleShape(
    const HGF2DSimpleShape& pi_rSimpleShape) const
    {
    // The shapes must not cross
    HPRECONDITION(!Crosses(pi_rSimpleShape));

    // Their extent must overlap
    HPRECONDITION(GetExtent().OutterOverlaps(pi_rSimpleShape.GetExtent(),
                                             MIN(GetTolerance(), pi_rSimpleShape.GetTolerance())));


    // Obtain tolerance
    double Tolerance = MIN(GetTolerance(), pi_rSimpleShape.GetTolerance());

    HGF2DShape::SpatialPosition     ThePosition = HGF2DShape::S_OUT;

    HFCPtr<HGF2DLinear> pMyLinear = pi_rSimpleShape.GetLinear();

    // We check if the start point is NOT ON
    if (!IsPointOn((pMyLinear->GetStartPoint()), INCLUDE_EXTREMITIES, Tolerance))
        {
        // The point is either IN or OUT as the rest of the shape
        ThePosition = (IsPointIn(pMyLinear->GetStartPoint(), Tolerance) ?
                       HGF2DShape::S_IN : HGF2DShape::S_OUT);
        }
    else
        {
        // Since the first point is ON, the linear either flirts or overlays
        // the shape ... we ask for the spatial position of the linear
        // which performs more complex study of the geometry
        ThePosition = CalculateSpatialPositionOfNonCrossingLinear(*pMyLinear);
        }

    return (ThePosition);
    }


//-----------------------------------------------------------------------------
// CalculateSpatialPositionOfNonCrossingLinear
// This method returns the spatial position relative to shape of given linear
//-----------------------------------------------------------------------------
HGF2DShape::SpatialPosition HGF2DShape::CalculateSpatialPositionOfNonCrossingLinear(
    const HGF2DLinear& pi_rLinear) const
    {
    // The two vectors must not cross
    HPRECONDITION(!Crosses(pi_rLinear));

    HGF2DShape::SpatialPosition     ThePosition = HGF2DShape::S_OUT;

    // Calculate Tolerance
    double Tolerance = MIN(pi_rLinear.GetTolerance(), GetTolerance());

    // Check if their extents overlap
    if (GetExtent().OutterOverlaps(pi_rLinear.GetExtent(), Tolerance))
        {
        // The vectors do overlap ... study
        // First try resolution with start point
        if (!IsPointOn(pi_rLinear.GetStartPoint(), INCLUDE_EXTREMITIES, Tolerance))
            {
            // The point is either IN or OUT as the rest of the shape
            ThePosition = (IsPointIn(pi_rLinear.GetStartPoint(), Tolerance) ?
                           HGF2DShape::S_IN : HGF2DShape::S_OUT);
            }
        // If failed then try resolution with end point
        else if (!IsPointOn(pi_rLinear.GetEndPoint(), INCLUDE_EXTREMITIES, Tolerance))
            {
            // The point is either IN or OUT as the rest of the shape
            ThePosition = (IsPointIn(pi_rLinear.GetEndPoint(), Tolerance) ?
                           HGF2DShape::S_IN : HGF2DShape::S_OUT);
            }
        else
            {
            // At this point, the linear either connects to the shape at both
            // start and end point, or overlays the shape
            // Check if the basic linear is not NULL
            // This condition is very important, since the present may be called recursively
            // in the case of multiple flirting points (see below)
            if (pi_rLinear.IsNull())
                ThePosition = HGF2DShape::S_ON; // Since linear is null and both points are on the shape then the result is ON
            else
                {
                // We determine if the linear is contiguous
                if (AreContiguous(pi_rLinear))
                    {
                    // The linear is contiguous
                    // We ask for extended contiguousness points.
                    HGF2DPositionCollection     MyListOfPoints;

                    if (ObtainContiguousnessPoints(pi_rLinear, &MyListOfPoints))
                        {
                    
                        // There are an even number of contiguousness points
                        HASSERT(MyListOfPoints.size() % 2 == 0);

                        if (MyListOfPoints.size() == 0)
                            {
                            // This can only occur if the linear goes fully around one of the shape component it is therefore ON
                            ThePosition = HGF2DShape::S_ON;
                            }
                        else 
                            {
                            // There are some contiguousness points; the linear is either unclosed or is partially out or in
                            // but we do not know which one yet.

                            // We do not know either the order of the contiguousness points obtained relative to linear.
                            // Note that if the linear autocloses then the start/end point will may not be included in the 
                            // list of contiguousness points even if the point is part of the contiguoussness region.

                            // What we will do is sort contiguousness points according to relative position
                            HGF2DPositionCollection OrderedListOfPoints;
                            OrderedListOfPoints.push_back(pi_rLinear.GetStartPoint());
                            
                            double relativePosition = -1.0;
                            HGF2DPositionCollection::iterator selectedPtIndex;
                            HGF2DPositionCollection::iterator ptIndex = MyListOfPoints.begin();
                            while(MyListOfPoints.size() > 0)
                                {
                                double currentRelativePosition = pi_rLinear.CalculateRelativePosition(*ptIndex);
                                if (relativePosition < 0.0 || currentRelativePosition < relativePosition)
                                    {
                                    relativePosition = currentRelativePosition;
                                    selectedPtIndex = ptIndex;
                                    }
                                ptIndex++;
                                if (ptIndex == MyListOfPoints.end())
                                    {
                                    OrderedListOfPoints.push_back(*selectedPtIndex);
                                    MyListOfPoints.erase(selectedPtIndex);
                                    relativePosition = -1.0;
                                    ptIndex = MyListOfPoints.begin();
                                    }
                                }

                            // We add the last point as relativePosition of last point + half to 1.0
                            // The purpose being that if the linear is self close we have no problem with relative position of last point
                            // that would happen to be the first point at the same time.
                            OrderedListOfPoints.push_back(pi_rLinear.CalculateRelativePoint((pi_rLinear.CalculateRelativePosition(OrderedListOfPoints[OrderedListOfPoints.size() - 1]) + 1.0)/2.0));


                            ThePosition = HGF2DShape::S_ON;

                            for (size_t indexCurrentPoint = 0 ; (HGF2DShape::S_ON == ThePosition) && (indexCurrentPoint < OrderedListOfPoints.size() - 1); indexCurrentPoint++)
                                {
                                HAutoPtr<HGF2DLinear> pMyLinearCopy((HGF2DLinear*)pi_rLinear.Clone());

                                if (!OrderedListOfPoints[indexCurrentPoint].IsEqualTo(OrderedListOfPoints[indexCurrentPoint+1], Tolerance))
                                    {
                                    // We shorten it to point
                                    pMyLinearCopy->Shorten(OrderedListOfPoints[indexCurrentPoint], OrderedListOfPoints[indexCurrentPoint + 1]);

                                    // Obtain the point located halfway on the portion of linear
                                    ThePosition = CalculateSpatialPositionOf(pMyLinearCopy->CalculateRelativePoint(0.5));
                                    }
                                }
                            }
                        }
                    else
                        {
                        // There are no contiguousness points ... possible only for
                        // autoclosing basic linears
                        // located completely on shape
                        ThePosition = HGF2DShape::S_ON;
                        }
                    }
                else
                    {
                    // Since the linear is not contiguous, then any point other than
                    // extremity points will resolve
                    // The point is either IN or OUT as the rest of the linear
                    if (!IsPointOn(pi_rLinear.CalculateRelativePoint(0.5)))
                        ThePosition = (IsPointIn(pi_rLinear.CalculateRelativePoint(0.5), Tolerance) ?
                                       HGF2DShape::S_IN : HGF2DShape::S_OUT);
                    else
                        {
                        // Highly improbable !
                        // The start, end and mid points are all flirt points!
                        // We resolve by recusivity of splited parts
                        // We obtain a copy of the linear
                        HAutoPtr<HGF2DLinear> pMyLinearCopy((HGF2DLinear*)pi_rLinear.Clone());

                        // Shorten by half
                        pMyLinearCopy->ShortenTo(0.5);

                        // Obtain spatial position of this part
                        ThePosition = CalculateSpatialPositionOfNonCrossingLinear(*pMyLinearCopy);

                        // Check if a solution was not found
                        if (ThePosition == HGF2DShape::S_ON)
                            {
                            // No solution found ... try with second half
                            HAutoPtr<HGF2DLinear> pMyLinearCopy2((HGF2DLinear*)pi_rLinear.Clone());

                            // Shorten by half
                            pMyLinearCopy2->ShortenFrom(0.5);

                            // Obtain spatial position of this part
                            ThePosition = CalculateSpatialPositionOfNonCrossingLinear(*pMyLinearCopy2);

                            // The result position is the best that could be found, but note that
                            // if S_ON, there should have been a contiguousness reported !
                            // This may occur in rare cases where the tolerance is two small and
                            // mathematical errors upon contiguousness computations indicate false while true would be more
                            // adequate. In all case the result is ON
                            // HASSERT(ThePosition != HGF2DShape::S_ON);
                            }
                        }
                    }
                }
            }
        }

    return (ThePosition);
    }

//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
void HGF2DShape::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    po_rOutput << "Object is a HGF2DShape" << endl;
    HDUMP0("Object is a HGF2DShape\n");

    HGF2DVector::PrintState(po_rOutput);

#endif
    }


//-----------------------------------------------------------------------------
// @bsimethod                                                   2014/06
//-----------------------------------------------------------------------------
HFCPtr<HGF2DShape> HGF2DShape::AllocTransformInverse(const HGF2DTransfoModel& pi_rModel) const
    {
    HFCPtr<HGF2DTransfoModel> pReversedModel = pi_rModel.Clone();
    pReversedModel->Reverse();
    return AllocTransformDirect(*pReversedModel);
    }

//-----------------------------------------------------------------------------
// Compute the convex hull of the shape
// Returns in po_pConvexHull a list of point in counter-clockwise order representing the convex hull
//
// Reference for the algorithm : https://en.wikibooks.org/wiki/Algorithm_Implementation/Geometry/Convex_hull/Monotone_chain
//
//Laurent.Robert-Veillette                                              03/2016
//-----------------------------------------------------------------------------
void HGF2DShape::GetConvexHull(HGF2DPositionCollection* p_points, HGF2DPositionCollection* po_pConvexHull) const
    {
    HPRECONDITION(po_pConvexHull != nullptr);
    HPRECONDITION(p_points != nullptr);

    int nbPoints = (int)p_points->size();
    po_pConvexHull->resize(2 * nbPoints);

    // Sort the points lexicographically (left points first)
    std::sort(begin(*p_points), end(*p_points), [] (const HGF2DPosition& a, const HGF2DPosition& b)
        {
        return (a.GetX() < b.GetX() || (a.GetX() == b.GetX() && a.GetY() < b.GetY()));
        });

    // Build lower hull
    int k = 0;
    for (int i = 0; i < nbPoints; ++i)
        {
        while (k >= 2 && (*p_points)[i].CrossProduct2D((*po_pConvexHull)[k - 2], (*po_pConvexHull)[k - 1]) <= 0)
            --k;
        (*po_pConvexHull)[k++] = (*p_points)[i];
        }

    // Build upper hull
    for (int i = nbPoints - 2, t = k + 1; i >= 0; --i)
        {
        while (k >= t && (*p_points)[i].CrossProduct2D((*po_pConvexHull)[k - 2], (*po_pConvexHull)[k - 1]) <= 0)
            --k;
        (*po_pConvexHull)[k++] = (*p_points)[i];
        }

    //Removing the unused points and the last one because it is a repetition of the first point.
    (*po_pConvexHull).resize(k - 1);
    }


//-----------------------------------------------------------------------------
// Compute the minimal bounding box in which a shape is contained. We can also visualize it
// as the oriented extent with the minimal area. i.e. the rotated rectangle contaning all the
// shape while having the minimal area.
//
// Returns the four corner of the best rectangle found
//
// Reference for the algorithm : https://geidav.wordpress.com/2014/01/23/computing-oriented-minimum-bounding-boxes-in-2d/
//
//Laurent.Robert-Veillette                                              03/2016
//-----------------------------------------------------------------------------
void HGF2DShape::GetBestOrientedExtent(HGF2DPositionCollection* po_pMinimalBoxCorners, HGF2DPositionCollection* po_pConvexHull) const
    {
    HPRECONDITION(po_pMinimalBoxCorners != nullptr);
    HPRECONDITION(po_pConvexHull != nullptr);

    //Already a rectangle
    if (po_pConvexHull->size() == 4)
        *po_pMinimalBoxCorners = *po_pConvexHull;

    else
        {
        //Construct the extreme lines with the extent boundary points
        HGF2DLiteExtent SelfExtent(GetExtent());
        HGF2DLiteLine VerticalMinLine(HGF2DPosition(SelfExtent.GetXMin(), 0), HGF2DPosition(SelfExtent.GetXMin(), 1));
        HGF2DLiteLine VerticalMaxLine(HGF2DPosition(SelfExtent.GetXMax(), 0), HGF2DPosition(SelfExtent.GetXMax(), 1));
        HGF2DLiteLine HorizontalMinLine(HGF2DPosition(0, SelfExtent.GetYMin()), HGF2DPosition(1, SelfExtent.GetYMin()));
        HGF2DLiteLine HorizontalMaxLine(HGF2DPosition(0, SelfExtent.GetYMax()), HGF2DPosition(1, SelfExtent.GetYMax()));

        //Initialize the current minimum rectangle area to infity
        double MinArea = std::numeric_limits<double>::max();

        //The minimal vertical line is always associated with index 0 because of the order of the points in the convex hull method
        int VerticalMinIndex = 0;
        int VerticalMaxIndex = 0;
        int HorizontalMinIndex = 0;
        int HorizontalMaxIndex = 0;

        //Associate each line with its corresponding point(s) in the convex hull
        int i = 0;
        double maxX = (*po_pConvexHull)[0].GetX();
        double minY = (*po_pConvexHull)[0].GetY();
        double maxY = minY;
        for_each(begin(*po_pConvexHull), end(*po_pConvexHull), [&] (const HGF2DPosition& pPoint)
            {
            if (pPoint.GetX() > maxX)
                {
                maxX = pPoint.GetX();
                VerticalMaxIndex = i;
                }
            if (pPoint.GetY() < minY)
                {
                minY = pPoint.GetY();
                HorizontalMinIndex = i;
                }
            if (pPoint.GetY() > maxY)
                {
                maxY = pPoint.GetY();
                HorizontalMaxIndex = i;
                }
            ++i;
            });

        double TotalRotationAngle = 0.0; //Can't go over PI / 2 radian
                                         //Rotate the bounding boxes to fit with one side of the convex hull, compute the area and compare to find the minimal one.
        auto nbPoints = po_pConvexHull->size();
        HGF2DPositionCollection CornersMinimalBox;
        CornersMinimalBox.reserve(4);

        while (TotalRotationAngle <= PI / 2)
            {
            HGF2DLiteLine FirstHullLine((*po_pConvexHull)[VerticalMinIndex], (*po_pConvexHull)[(VerticalMinIndex + 1) % nbPoints]);
            HGF2DLiteLine SecondHullLine((*po_pConvexHull)[VerticalMaxIndex], (*po_pConvexHull)[(VerticalMaxIndex + 1) % nbPoints]);
            HGF2DLiteLine ThirdHullLine((*po_pConvexHull)[HorizontalMinIndex], (*po_pConvexHull)[(HorizontalMinIndex + 1) % nbPoints]);
            HGF2DLiteLine FourthHullLine((*po_pConvexHull)[HorizontalMaxIndex], (*po_pConvexHull)[(HorizontalMaxIndex + 1) % nbPoints]);

            //Get the minimum angle between the line of the box and the corresponding line of the convex hull
            double FirstAngle = fabs(VerticalMinLine.CalculateBearing().GetAngle() - FirstHullLine.CalculateBearing().GetAngle());
            double SecondAngle = fabs(VerticalMaxLine.CalculateBearing().GetAngle() - SecondHullLine.CalculateBearing().GetAngle());
            double ThirdAngle = fabs(HorizontalMinLine.CalculateBearing().GetAngle() - ThirdHullLine.CalculateBearing().GetAngle());
            double FourthAngle = fabs(HorizontalMaxLine.CalculateBearing().GetAngle() - FourthHullLine.CalculateBearing().GetAngle());

            FirstAngle = MIN(FirstAngle, PI - FirstAngle);
            SecondAngle = MIN(SecondAngle, PI - SecondAngle);
            ThirdAngle = MIN(ThirdAngle, PI - ThirdAngle);
            FourthAngle = MIN(FourthAngle, PI - FourthAngle);

            //Find minimal rotation angle
            double minAngle = MIN(FirstAngle, MIN(SecondAngle, MIN(ThirdAngle, FourthAngle)));
            BeAssert(minAngle <= PI / 2 && minAngle >= 0);

            //Rotate Counter-Clockwise every line around the point of their index
            VerticalMinLine.Rotate(minAngle, (*po_pConvexHull)[VerticalMinIndex]);
            VerticalMaxLine.Rotate(minAngle, (*po_pConvexHull)[VerticalMaxIndex]);
            HorizontalMinLine.Rotate(minAngle, (*po_pConvexHull)[HorizontalMinIndex]);
            HorizontalMaxLine.Rotate(minAngle, (*po_pConvexHull)[HorizontalMaxIndex]);

            //Increment the corresponding index
            if (HDOUBLE_EQUAL_EPSILON(minAngle, FirstAngle))
                VerticalMinIndex = (++VerticalMinIndex % nbPoints);
            else if (HDOUBLE_EQUAL_EPSILON(minAngle, SecondAngle))
                VerticalMaxIndex = (++VerticalMaxIndex % nbPoints);
            else if (HDOUBLE_EQUAL_EPSILON(minAngle, ThirdAngle))
                HorizontalMinIndex = (++HorizontalMinIndex % nbPoints);
            else if (HDOUBLE_EQUAL_EPSILON(minAngle, FourthAngle))
                HorizontalMaxIndex = (++HorizontalMaxIndex % nbPoints);

            //Intersect the lines to form the rectangle
            HGF2DPosition Corner1, Corner2, Corner3, Corner4;
            VerticalMinLine.IntersectLine(HorizontalMaxLine, &Corner1);
            HorizontalMaxLine.IntersectLine(VerticalMaxLine, &Corner2);
            VerticalMaxLine.IntersectLine(HorizontalMinLine, &Corner3);
            HorizontalMinLine.IntersectLine(VerticalMinLine, &Corner4);

            double width = VerticalMinLine.CalculateShortestDistance(Corner2);
            double height = HorizontalMaxLine.CalculateShortestDistance(Corner3);

            //Angles between lines must always be PI/2 radian
            BeAssert(HDOUBLE_EQUAL_EPSILON(fabs(VerticalMinLine.CalculateBearing().GetAngle() -
                                               HorizontalMinLine.CalculateBearing().GetAngle()), PI / 2));
            BeAssert(HDOUBLE_EQUAL_EPSILON(fabs(VerticalMinLine.CalculateBearing().GetAngle() -
                                               HorizontalMaxLine.CalculateBearing().GetAngle()), PI / 2));
            BeAssert(HDOUBLE_EQUAL_EPSILON(fabs(VerticalMaxLine.CalculateBearing().GetAngle() -
                                               HorizontalMinLine.CalculateBearing().GetAngle()), PI / 2));
            BeAssert(HDOUBLE_EQUAL_EPSILON(fabs(VerticalMaxLine.CalculateBearing().GetAngle() -
                                               HorizontalMaxLine.CalculateBearing().GetAngle()), PI / 2));

            //Compute the area and compare it with the actual minimum
            double ActualArea = width * height;

            if (ActualArea < MinArea)
                {
                MinArea = ActualArea;
                CornersMinimalBox.clear();
                CornersMinimalBox.push_back(Corner1);
                CornersMinimalBox.push_back(Corner2);
                CornersMinimalBox.push_back(Corner3);
                CornersMinimalBox.push_back(Corner4);
                }

            TotalRotationAngle += minAngle;
            }
        *po_pMinimalBoxCorners = CornersMinimalBox;
        }
    }
