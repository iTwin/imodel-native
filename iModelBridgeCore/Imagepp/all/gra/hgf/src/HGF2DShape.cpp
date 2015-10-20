//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DShape.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HGF2DShape
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGF2DShape.h>
#include <Imagepp/all/h/HGF2DDisplacement.h>


// HPM_REGISTER_ABSTRACT_CLASS(HGF2DShape, HGF2DVector)


#include <Imagepp/all/h/HGF2DSimpleShape.h>
#include <Imagepp/all/h/HGF2DHoledShape.h>
#include <Imagepp/all/h/HGF2DSegment.h>
#include <Imagepp/all/h/HGFScanLines.h>


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
            if (!pi_rLinear.IsNull())
                {
                // We determine if the linear is contiguous
                if (AreContiguous(pi_rLinear))
                    {
                    // The linear is contiguous
                    // We ask for extended contiguousness points.
                    HGF2DPositionCollection     MyListOfPoints;

                    if (pi_rLinear.ObtainContiguousnessPoints(*this, &MyListOfPoints))
                        {
                        // There are an even number of contiguousness points
                        HASSERT(MyListOfPoints.size() % 2 == 0);

                        // There are some contiguousness points.
                        // Here we will locate sections of the linear out of the contiguousness region
                        // And check their position. Normally there should be a single region
                        // but there can be many.

                        // Important note: At the moment we know that both the start and end points are on
                        // the shape yet this does not imply that both are located in the same contiguousness region
                        // or that they are even both in a contiguousness region. One of the start or end point may be in 
                        // fact only flirting

                        // We first check if there is only a single region
                        if (MyListOfPoints.size() == 2)
                            {
                            // There is a single region ... two possibilities exist:
                            // A - The contiguousness region is the whole length of the linear then the linear is ON
                            // B - The contiguousness region goes from start point to intemediate point and
                            //     end point connects within this region (typically closes on the start point)
                            //     or the same geometry but with end point par of the region and start point connecting.
                            if (pi_rLinear.GetStartPoint().IsEqualTo(MyListOfPoints[0], Tolerance))
                                {
                                // Start point is at start of contiguousness region ... check if end point is at end
                                if (pi_rLinear.GetEndPoint().IsEqualTo(MyListOfPoints[1]))
                                    ThePosition = HGF2DShape::S_ON;
                                else
                                    {
                                    // The end point connects ... we must obtain the remained of the linear
                                    // We obtain a copy of the linear
                                    HAutoPtr<HGF2DLinear> pMyLinearCopy((HGF2DLinear*)pi_rLinear.Clone());

                                    // We shorten it to second point
                                    pMyLinearCopy->ShortenFrom(MyListOfPoints[1]);

                                    // We obtain the mid point of result linear
                                    HGF2DPosition MyPoint = pMyLinearCopy->CalculateRelativePoint(0.5);

                                    // Check the spatial position of point
                                    if (!IsPointOn(MyPoint, INCLUDE_EXTREMITIES, Tolerance))
                                        {
                                        // The point is either IN or OUT as the rest of the linear
                                        ThePosition = (IsPointIn(MyPoint, Tolerance) ?
                                                       HGF2DShape::S_IN : HGF2DShape::S_OUT);
                                        }
                                    else
                                        {
                                        // highly improbable ... requires that the point selected at random is on (flirts)
                                        // we will leave with the possibility of having a false result.
                                        ThePosition = HGF2DShape::S_ON;
                                        }
                                    }
                                }
                            else
                                {
                                // Start point is not part of region ...
                                HAutoPtr<HGF2DLinear> pMyLinearCopy((HGF2DLinear*)pi_rLinear.Clone());

                                // We shorten it to FIRST point
                                pMyLinearCopy->ShortenTo(MyListOfPoints[0]);

                                // We obtain the mid point of result linear
                                HGF2DPosition MyPoint = pMyLinearCopy->CalculateRelativePoint(0.5);

                                // Check the spatial position of point
                                if (!IsPointOn(MyPoint, INCLUDE_EXTREMITIES, Tolerance))
                                    {
                                    // The point is either IN or OUT as the rest of the linear
                                    ThePosition = (IsPointIn(MyPoint, Tolerance) ?
                                                   HGF2DShape::S_IN : HGF2DShape::S_OUT);
                                    }
                                else
                                    {
                                    // highly improbable ... requires that the point selected at random is on (flirts)
                                    // we will leave with the possibility of having a false result.
                                    ThePosition = HGF2DShape::S_ON;
                                    }
                                }
                            }
                        else
                            {
                            // There are more than two contiguousness regions ... we take a point intermediate in between regions and test
                            // Start point is not part of region ...
                            HAutoPtr<HGF2DLinear> pMyLinearCopy((HGF2DLinear*)pi_rLinear.Clone());

                            // We shorten it from second point to third
                            pMyLinearCopy->Shorten(MyListOfPoints[1], MyListOfPoints[2]);

                            // We obtain the mid point of result linear
                            HGF2DPosition MyPoint = pMyLinearCopy->CalculateRelativePoint(0.5);

                            // Check the spatial position of point
                            if (!IsPointOn(MyPoint, INCLUDE_EXTREMITIES, Tolerance))
                                {
                                // The point is either IN or OUT as the rest of the linear
                                ThePosition = (IsPointIn(MyPoint, Tolerance) ?
                                               HGF2DShape::S_IN : HGF2DShape::S_OUT);
                                }
                            else
                                {
                                // highly improbable ... requires that the point selected at random is on (flirts)
                                // we will leave with the possibility of having a false result.
                                ThePosition = HGF2DShape::S_ON;
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
                            // This may occur in rare cases where the tolerance is too small and
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
