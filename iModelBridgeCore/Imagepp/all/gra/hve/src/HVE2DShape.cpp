//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hve/src/HVE2DShape.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HVE2DShape
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HVE2DShape.h>


HPM_REGISTER_ABSTRACT_CLASS(HVE2DShape, HVE2DVector)


#include <Imagepp/all/h/HVE2DSimpleShape.h>
#include <Imagepp/all/h/HVE2DHoledShape.h>
#include <Imagepp/all/h/HVE2DComplexShape.h>
#include <Imagepp/all/h/HVE2DRectangle.h>
#include <Imagepp/all/h/HVE2DPolygonOfSegments.h>
#include <Imagepp/all/h/HVE2DVoidShape.h>
#include <Imagepp/all/h/HVE2DUniverse.h>
#include <Imagepp/all/h/HVE2DSegment.h>
#include <Imagepp/all/h/HGFScanLines.h>

#include <Imagepp/all/h/HGF2DHoledShape.h>
#include <Imagepp/all/h/HGF2DComplexShape.h>

//-----------------------------------------------------------------------------
// Rasterize
// This method rasterizes (generates scanlines) for the shape.
//-----------------------------------------------------------------------------
void HVE2DShape::Rasterize(HGFScanLines& pio_rScanlines) const
    {
    if (pio_rScanlines.GetScanlinesCoordSys() == 0)
        {
        pio_rScanlines.SetScanlinesCoordSys(GetCoordSys());
        }
    HASSERT(pio_rScanlines.GetScanlinesCoordSys() == GetCoordSys());

    // Check if shape is not empty
    if (!IsEmpty())
        {
        // Obtain extent of shape
        HGF2DExtent ShapeExtent = GetExtent();

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
        double XMax = ShapeExtent.GetXMax() +
                       (fabs(ShapeExtent.GetXMax() / 10.0)) + 1.0;


        // For each valid other Y position
        for (double Y = pio_rScanlines.GetFirstScanlinePosition() ; Y < ShapeExtent.GetYMax() ; Y += 1.0)
            {
            // Create a segment
            HVE2DSegment    HorizontalSegment(HGF2DLocation(XMin, Y, GetCoordSys()),
                                              HGF2DLocation(XMax, Y, GetCoordSys()));

            // Create recipient list for cross points
            HGF2DLocationCollection CrossPoints;

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

                HGF2DLocationCollection::const_iterator Itr;
                for (Itr = CrossPoints.begin() ; Itr != CrossPoints.end() ; ++Itr)
                    pio_rScanlines.AddCrossingPoint((HSINTX)Y, Itr->GetX());
                }
            }
        }
    }


/** -----------------------------------------------------------------------------
    This method returns a pointer to a dynamically allocated shape that
    defines an area containing the area defined by one shape which is
    not located in the other shape.
    The method differentiates the given from self.

    It thus performs the differentiation of the two shapes.
    When the result shape is not needed anymore the caller must
    delete it.

    @param pi_rShape Constant reference to a shape to perform
                     differentiation of with self.

    @return A pointer to a dynamically allocated shape containing
            the difference of the two shapes.

    Example:
    @code
    @end

    @see DifferentiateShapeSCS()
    @see IntersectShape()
    @see UnifyShape
    -----------------------------------------------------------------------------
*/
HVE2DShape* HVE2DShape::DifferentiateShape(const HVE2DShape& pi_rShape) const
    {
    HVE2DShape* pMyResultShape;

    // Check if the shapes are expressed in the same coordinate system
    if (GetCoordSys() == pi_rShape.GetCoordSys())
        {
        // Both shapes are expressed in the same coordinate system
        pMyResultShape = DifferentiateShapeSCS(pi_rShape);
        }
    else
        {
        // The given shape is not expressed in the same coordinate system
        // Allocate a copy in the self coordinate system
        HAutoPtr<HVE2DShape> pMyNewShape((HVE2DShape*)pi_rShape.AllocateCopyInCoordSys(
                                             GetCoordSys()));

        // Both shapes are now expressed in the same coordinate system
        pMyResultShape = DifferentiateShapeSCS(*pMyNewShape);
        }

    pMyResultShape->SetStrokeTolerance(m_pStrokeTolerance);

    return (pMyResultShape);

    }


/** -----------------------------------------------------------------------------
    These methods returns a pointer to a dynamically allocated shape that
    defines an area containing the area defined by both self and the
    given shape. It therefore performs the intersection of the two shapes.
    When the result shape is not needed anymore it must be deleted by
    the caller.

    @param pi_rShape Constant reference to a shape to perform
                     intersection with self.

    @return A pointer to a dynamically allocated shape containing
            the intersection of the two shapes.

    Example:
    @code
    @end

    @see DifferentiateShape()
    @see IntersectShapeSCS()
    @see UnifyShape()
    -----------------------------------------------------------------------------
*/
HVE2DShape* HVE2DShape::IntersectShape(const HVE2DShape& pi_rShape) const
    {
    HVE2DShape* pMyResultShape;

    // Check if the shapes are expressed in the same coordinate system
    if (GetCoordSys() == pi_rShape.GetCoordSys())
        {
        // Both shapes are expressed in the same coordinate system
        pMyResultShape = IntersectShapeSCS(pi_rShape);
        }
    else
        {
        // The given shape is not expressed in the same coordinate system
        // Allocate a copy in the self coordinate system
        HAutoPtr<HVE2DShape> pMyNewShape((HVE2DShape*)pi_rShape.AllocateCopyInCoordSys(
                                             GetCoordSys()));

        // Both shapes are now expressed in the same coordinate system
        pMyResultShape = IntersectShapeSCS(*pMyNewShape);
        }

    pMyResultShape->SetStrokeTolerance(m_pStrokeTolerance);

    return (pMyResultShape);
    }


//-----------------------------------------------------------------------------
// Sets the shape coordinate system
//-----------------------------------------------------------------------------
void HVE2DShape::SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_pCoordSys)
    {
    // Set ancester coord sys
    HVE2DVector::SetCoordSysImplementation(pi_pCoordSys);

    if (m_pStrokeTolerance != NULL)
        {
        m_pStrokeTolerance->ChangeCoordSys(pi_pCoordSys);
        m_pStrokeTolerance->SetCoordSys(GetCoordSys());
        }
    }


/** -----------------------------------------------------------------------------
    This methods returns a pointer to a dynamically allocated
    shape that defines an area containing the area defined by the
    self and the given shape. It therefore performs the union of the
    two shapes. When the result shape is not needed anymore it
    must be deleted by the caller.

    @param pi_rShape Constant reference to a shape to perform
                     union with self.

    @return A pointer to a dynamically allocated shape containing
            the union of the two shapes.

    Example:
    @code
    @end

    @see DifferentiateShape()
    @see IntersectShape()
    @see UnifyShapeSCS()
    -----------------------------------------------------------------------------
*/
HVE2DShape* HVE2DShape::UnifyShape(const HVE2DShape& pi_rShape) const
    {
    HVE2DShape* pMyResultShape;

    // Check if the shapes are expressed in the same coordinate system
    if (GetCoordSys() == pi_rShape.GetCoordSys())
        {
        // Both shapes are expressed in the same coordinate system
        pMyResultShape = UnifyShapeSCS(pi_rShape);
        }
    else
        {
        // The given shape is not expressed in the same coordinate system
        // Allocate a copy in the self coordinate system
        HAutoPtr<HVE2DShape> pMyNewShape((HVE2DShape*)pi_rShape.AllocateCopyInCoordSys(
                                             GetCoordSys()));

        // Both shapes are now expressed in the same coordinate system
        pMyResultShape = UnifyShapeSCS(*pMyNewShape);
        }

    pMyResultShape->SetStrokeTolerance(m_pStrokeTolerance);

    return (pMyResultShape);
    }



/** -----------------------------------------------------------------------------
    This method calculates and returns the spatial position of given
    vector relative to the area defined by the shape. There are four
    possible answer resulting from operation: (HVE2DShape::S_OUT,
    HVE2DShape::S_IN, HVE2DShape::S_PARTIALY_IN,
    HVE2DShape::S_ON). The first one indicates that the whole of the given
    vector is located outside the area defined by the shape. This does not
    exclude the possibility that the given vector may share parts of the
    shape boundary (flirting, countiguousness). If HVE2DShape::S_IN is
    returned, then the vector is located completely inside the area
    enclosed by the shape. This does not exclude the possibility that the given
    vector may share parts of the shape boundary (flirting, contiguousness).
    If HVE2DShape::S_PARTIALY_IN is returned, then the object is located
    partly inside and partly outside the area defined by the shape. Finally
    HVE2DShape::S_ON is returned is the path of the given vector is completely
    located on the shape boundary, with no in nor out parts.

    @param pi_rVector The vector of which spatial position of must be determined.

    @return The spatial position of vector.

    Example:
    @code
    @end

    @see HVE2DVector
    -----------------------------------------------------------------------------
*/
HVE2DShape::SpatialPosition HVE2DShape::CalculateSpatialPositionOf(
    const HVE2DVector& pi_rVector) const
    {
    HVE2DShape::SpatialPosition     ThePosition = HVE2DShape::S_OUT;

    // Check if their extents overlap
    if (GetExtent().OutterOverlaps(pi_rVector.GetExtent(),
                                   MIN(GetTolerance(), pi_rVector.GetTolerance())))
        {

        // Check if vector is made of multiple entities
        if (pi_rVector.GetMainVectorType() == HVE2DLinear::CLASS_ID ||
            (pi_rVector.GetMainVectorType() == HVE2DShape::CLASS_ID &&
             ((HVE2DShape*)&pi_rVector)->IsSimple()))
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
HVE2DShape::SpatialPosition HVE2DShape::CalculateSpatialPositionOfSingleComponentVector(
    const HVE2DVector& pi_rVector) const
    {
    // The given vector must be composed of a single entity
    HPRECONDITION(pi_rVector.GetMainVectorType() == HVE2DLinear::CLASS_ID ||
                  (pi_rVector.GetMainVectorType() == HVE2DShape::CLASS_ID &&
                   ((HVE2DShape*)&pi_rVector)->IsSimple()));

    HVE2DShape::SpatialPosition     ThePosition = HVE2DShape::S_OUT;

    // Check if their extents overlap
    if (GetExtent().OutterOverlaps(pi_rVector.GetExtent(),
                                   MIN(GetTolerance(), pi_rVector.GetTolerance())))
        {
        // The extents overlap ... check if they cross ?
        if (Crosses(pi_rVector))
            {
            // Since they do cross, the vector is PARTIALY IN (passes through the shape boundary)
            ThePosition = HVE2DShape::S_PARTIALY_IN;
            }
        else
            {
            // They do not cross
            // We first determine of which type is the vector
            if (pi_rVector.GetMainVectorType()== HVE2DShape::CLASS_ID)
                {
                // The vector is a shape ... We cast as a shape
                HVE2DSimpleShape*     pMyShape = (HVE2DSimpleShape*)(&pi_rVector);

                ThePosition = CalculateSpatialPositionOfNonCrossingSimpleShape(*pMyShape);
                }
            else
                {
                // We have a linear ... we cast
                HVE2DLinear*    pMyLinear = (HVE2DLinear*)(&pi_rVector);

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
HVE2DShape::SpatialPosition HVE2DShape::CalculateSpatialPositionOfMultipleComponentVector(
    const HVE2DVector& pi_rVector) const
    {
    // The given vector must be composed of posibly multiple entities
    HPRECONDITION(pi_rVector.GetMainVectorType() != HVE2DLinear::CLASS_ID &&
                  !(pi_rVector.GetMainVectorType() == HVE2DShape::CLASS_ID &&
                    ((HVE2DShape*)&pi_rVector)->IsSimple()));

    HVE2DShape::SpatialPosition     ThePosition;

    // There are only two posible multiple component shapes
    if (((HVE2DShape*)&pi_rVector)->IsComplex())
        {
        // It is a complex shape
        ThePosition = CalculateSpatialPositionOfComplexShape(*((HVE2DShape*)&pi_rVector));
        }
    else
        {
        // It is a holed shape
        ThePosition = CalculateSpatialPositionOfHoledShape(*((HVE2DHoledShape*)&pi_rVector));
        }

    return(ThePosition);
    }


//-----------------------------------------------------------------------------
// CalculateSpatialPositionOfComplexShape
// PRIVATE METHOD
// This method returns the spatial position relative to shape of given vector
//-----------------------------------------------------------------------------
HVE2DShape::SpatialPosition HVE2DShape::CalculateSpatialPositionOfComplexShape(
    const HVE2DShape& pi_rShape) const
    {
    // Their extent must overlap
    HPRECONDITION(GetExtent().OutterOverlaps(pi_rShape.GetExtent(),
                                             MIN(GetTolerance(), pi_rShape.GetTolerance())));

    // The shape must be complex
    HPRECONDITION(pi_rShape.IsComplex());

    HVE2DShape::SpatialPosition     ThePosition = HVE2DShape::S_ON;

    // The shape is complex ... we ask the spatial position of every component shape
    // until resolution
    HVE2DShape::ShapeList::const_iterator  MyIterator = pi_rShape.GetShapeList().begin();

    // Loop till all components have been processed or partialy in
    while ((MyIterator != pi_rShape.GetShapeList().end()) &&
           (ThePosition != HVE2DShape::S_PARTIALY_IN))
        {
        // We ask the spatial position of current shape
        HVE2DShape::SpatialPosition TempPosition = CalculateSpatialPositionOf(**MyIterator);

        // If the current component position is different from on
        if (TempPosition != HVE2DShape::S_ON)
            {
            // If the previous result was on
            if (ThePosition == HVE2DShape::S_ON)
                {
                // Previous position was ON ... set current component position
                ThePosition = TempPosition;
                }
            else
                {
                // Previous position is either IN or OUT
                // Check if different component position
                if (TempPosition != ThePosition)
                    ThePosition = HVE2DShape::S_PARTIALY_IN;
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
HVE2DShape::SpatialPosition HVE2DShape::CalculateSpatialPositionOfHoledShape(
    const HVE2DHoledShape& pi_rHoledShape) const
    {
    // Their extent must overlap
    HPRECONDITION(GetExtent().OutterOverlaps(pi_rHoledShape.GetExtent(),
                                             MIN(GetTolerance(), pi_rHoledShape.GetTolerance())));

    HVE2DShape::SpatialPosition     ThePosition = HVE2DShape::S_ON;

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
        HVE2DShape::HoleList::const_iterator Itr = pi_rHoledShape.GetHoleList().begin();

        // loop till all holes processed or shape is partialy in
        while (Itr != pi_rHoledShape.GetHoleList().end() &&
               (ThePosition != HVE2DShape::S_PARTIALY_IN))
            {
            // Obtain spatial position of holed
            HVE2DShape::SpatialPosition HolePosition = CalculateSpatialPositionOf(**Itr);

            //If the hole position is not on
            if (HolePosition != HVE2DShape::S_ON)
                {
                // If the previous result was on
                if (ThePosition == HVE2DShape::S_ON)
                    {
                    // Previous position was ON ... set current hole position
                    ThePosition = HolePosition;
                    }
                else
                    {
                    // Previous position is either IN or OUT
                    // Check if different hole position
                    if (HolePosition != ThePosition)
                        ThePosition = HVE2DShape::S_PARTIALY_IN;
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
HVE2DShape::SpatialPosition HVE2DShape::CalculateSpatialPositionOfNonCrossingSimpleShape(
    const HVE2DSimpleShape& pi_rSimpleShape) const
    {
    // The shapes must not cross
    HPRECONDITION(!Crosses(pi_rSimpleShape));

    // Their extent must overlap
    HPRECONDITION(GetExtent().OutterOverlaps(pi_rSimpleShape.GetExtent(),
                                             MIN(GetTolerance(), pi_rSimpleShape.GetTolerance())));


    // Obtain tolerance
    double Tolerance = MIN(GetTolerance(), pi_rSimpleShape.GetTolerance());

    HVE2DShape::SpatialPosition     ThePosition = HVE2DShape::S_OUT;

    HVE2DComplexLinear MyLinear(pi_rSimpleShape.GetLinear());

    // We check if the start point is NOT ON
    if (!IsPointOn((MyLinear.GetStartPoint()), INCLUDE_EXTREMITIES, Tolerance))
        {
        // The point is either IN or OUT as the rest of the shape
        ThePosition = (IsPointIn(MyLinear.GetStartPoint(), Tolerance) ?
                       HVE2DShape::S_IN : HVE2DShape::S_OUT);
        }
    else
        {
        // Since the first point is ON, the linear either flirts or overlays
        // the shape ... we ask for the spatial position of the linear
        // which performs more complex study of the geometry
        ThePosition = CalculateSpatialPositionOfNonCrossingLinear(MyLinear);
        }

    return (ThePosition);
    }


//-----------------------------------------------------------------------------
// CalculateSpatialPositionOfNonCrossingLinear
// This method returns the spatial position relative to shape of given linear
//-----------------------------------------------------------------------------
HVE2DShape::SpatialPosition HVE2DShape::CalculateSpatialPositionOfNonCrossingLinear(
    const HVE2DLinear& pi_rLinear) const
    {
    // The two vectors must not cross
    HPRECONDITION(!Crosses(pi_rLinear));

    HVE2DShape::SpatialPosition     ThePosition = HVE2DShape::S_OUT;

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
                           HVE2DShape::S_IN : HVE2DShape::S_OUT);
            }
        // If failed then try resolution with end point
        else if (!IsPointOn(pi_rLinear.GetEndPoint(), INCLUDE_EXTREMITIES, Tolerance))
            {
            // The point is either IN or OUT as the rest of the shape
            ThePosition = (IsPointIn(pi_rLinear.GetEndPoint(), Tolerance) ?
                           HVE2DShape::S_IN : HVE2DShape::S_OUT);
            }
        else
            {
            // At this point, the linear either connects to the shape at both
            // start and end point, or overlays the shape
            // We check if the linear is complex
            if (pi_rLinear.IsComplex())
                {
                // We have a complex shape ... we cast
                HVE2DComplexLinear*     pMyComplexLinear = (HVE2DComplexLinear*)(&pi_rLinear);

                // We ask for every part until a non-ON
                // is found ... check spatial position
                HVE2DComplexLinear::LinearList::const_iterator  MyLinearIterator;
                MyLinearIterator = pMyComplexLinear->GetLinearList().begin();

                ThePosition = HVE2DShape::S_ON;

                while ((MyLinearIterator != pMyComplexLinear->GetLinearList().end())
                       && (ThePosition == HVE2DShape::S_ON))
                    {
                    ThePosition = CalculateSpatialPositionOfNonCrossingLinear(**MyLinearIterator);

                    MyLinearIterator++;
                    }

                }
            else
                {
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
                        HGF2DLocationCollection     MyListOfPoints;

                        if (pi_rLinear.ObtainContiguousnessPoints(*this, &MyListOfPoints))
                            {
                            // There are an even number of contiguousness points
                            HASSERT(MyListOfPoints.size() % 2 == 0);

                            // There are some contiguousness points

                            ThePosition = HVE2DShape::S_ON;

                            HGF2DLocation CurrentPoint = pi_rLinear.GetStartPoint();

                            // For every point until resolved
                            HGF2DLocationCollection::iterator     Itr;

                            for (Itr = MyListOfPoints.begin() ;
                                 Itr != MyListOfPoints.end() &&
                                 ThePosition == HVE2DShape::S_ON ; Itr++)
                                {
                                // No need to test if contiguous point is equal to extremity
                                if (!CurrentPoint.IsEqualTo(*Itr, Tolerance))
                                    {
                                    // We obtain a copy of the linear
                                    HAutoPtr<HVE2DLinear> pMyLinearCopy((HVE2DLinear*)pi_rLinear.Clone());

                                    // We shorten it to point
                                    pMyLinearCopy->Shorten(CurrentPoint, *Itr);

                                    // We obtain the mid point of result linear
                                    HGF2DLocation MyPoint = pMyLinearCopy->CalculateRelativePoint(0.5);

                                    // Check the spatial position of point
                                    if (!IsPointOn(MyPoint, INCLUDE_EXTREMITIES, Tolerance))
                                        {
                                        // The point is either IN or OUT as the rest of the linear
                                        ThePosition = (IsPointIn(MyPoint, Tolerance) ?
                                                       HVE2DShape::S_IN : HVE2DShape::S_OUT);
                                        }
                                    }

                                // Change current point
                                ++Itr;

                                // Since there are an even number of points :)
                                HASSERT(Itr != MyListOfPoints.end());

                                CurrentPoint = *Itr;
                                }

                            }
                        else
                            {
                            // There are no contiguousness points ... possible only for
                            // autoclosing basic linears
                            // located completely on shape
                            ThePosition = HVE2DShape::S_ON;
                            }
                        }
                    else
                        {
                        // Since the linear is not contiguous, then any point other than
                        // extremity points will resolve
                        // The point is either IN or OUT as the rest of the linear
                        if (!IsPointOn(pi_rLinear.CalculateRelativePoint(0.5)))
                            ThePosition = (IsPointIn(pi_rLinear.CalculateRelativePoint(0.5), Tolerance) ?
                                           HVE2DShape::S_IN : HVE2DShape::S_OUT);
                        else
                            {
                            // Highly improbable !
                            // The start, end and mid points are all flirt points!
                            // We resolve by recusivity of splited parts
                            // We obtain a copy of the linear
                            HAutoPtr<HVE2DLinear> pMyLinearCopy((HVE2DLinear*)pi_rLinear.Clone());

                            // Shorten by half
                            pMyLinearCopy->ShortenTo(0.5);

                            // Obtain spatial position of this part
                            ThePosition = CalculateSpatialPositionOfNonCrossingLinear(*pMyLinearCopy);

                            // Check if a solution was not found
                            if (ThePosition == HVE2DShape::S_ON)
                                {
                                // No solution found ... try with second half
                                HAutoPtr<HVE2DLinear> pMyLinearCopy2((HVE2DLinear*)pi_rLinear.Clone());

                                // Shorten by half
                                pMyLinearCopy2->ShortenFrom(0.5);

                                // Obtain spatial position of this part
                                ThePosition = CalculateSpatialPositionOfNonCrossingLinear(*pMyLinearCopy2);

                                // The result position is the best that could be found, but note that
                                // if S_ON, there should have been a contiguousness reported !
                                // This may occur in rare cases where the tolerance is two small and
                                // mathematical errors upon contiguousness computations indicate false while true would be more
                                // adequate. In all case the result is ON
                                // HASSERT(ThePosition != HVE2DShape::S_ON);
                                }
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
void HVE2DShape::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    po_rOutput << "Object is a HVE2DShape" << endl;
    HDUMP0("Object is a HVE2DShape\n");

    HVE2DVector::PrintState(po_rOutput);

#endif
    }


//-----------------------------------------------------------------------------
// Helper static method 
// create a shape from a light shape
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DShape::fCreateShapeFromLightShape(const HGF2DShape& pi_rShape, const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    {
    if (pi_rShape.IsSimple())
        {
        const HGF2DSimpleShape& rSimpleShape = static_cast<const HGF2DSimpleShape&>(pi_rShape);
        HGF2DShapeTypeId shapeType = rSimpleShape.GetShapeType();

        if (shapeType == HGF2DRectangleId)
            {
            return new HVE2DRectangle(static_cast<const HGF2DRectangle&>(rSimpleShape), pi_rpCoordSys);
            }
        else if (shapeType == HGF2DPolygonOfSegmentsId)
            {
            return new HVE2DPolygonOfSegments(static_cast<const HGF2DPolygonOfSegments&>(rSimpleShape), pi_rpCoordSys);
            }
        else if (shapeType == HGF2DShapeId_Void)
            {
            return new HVE2DVoidShape(static_cast<const HGF2DVoidShape&>(rSimpleShape), pi_rpCoordSys);
            }
        else 
            {
            HASSERT(shapeType == HGF2DUniverseId);
            return new HVE2DUniverse(static_cast<const HGF2DUniverse&>(rSimpleShape), pi_rpCoordSys);
            }
        }
    else if (pi_rShape.HasHoles())
        {
        return new HVE2DHoledShape(static_cast<const HGF2DHoledShape&>(pi_rShape), pi_rpCoordSys);
        }
    else
        {
        HASSERT(pi_rShape.IsComplex());
        return new HVE2DComplexShape(static_cast<const HGF2DComplexShape&>(pi_rShape), pi_rpCoordSys);
        }
    }

