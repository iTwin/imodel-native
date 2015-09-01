//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hve/src/HVE2DSimpleShape.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HVE2DSimpleShape
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HVE2DSimpleShape.h>

HPM_REGISTER_ABSTRACT_CLASS(HVE2DSimpleShape, HVE2DShape)


#include <Imagepp/all/h/HVE2DPolygon.h>
#include <Imagepp/all/h/HVE2DHoledShape.h>
#include <Imagepp/all/h/HVE2DComplexShape.h>
#include <Imagepp/all/h/HVE2DUniverse.h>
#include <Imagepp/all/h/HVE2DRectangle.h>
#include <Imagepp/all/h/HVE2DVoidShape.h>



//-----------------------------------------------------------------------------
// DecomposeSCS
// PRIVATE
// This method decomposes the different parts of two simple shape interactions
//-----------------------------------------------------------------------------
void HVE2DSimpleShape::DecomposeSCS(const HVE2DSimpleShape&              pi_rSimpleShape,
                                    HVE2DSimpleShape::DecomposeOperation pi_Operation,
                                    HoleList&                            pi_rListOfShapes,
                                    const HGF2DLocationCollection&       pi_rPoints) const
    {
    HPRECONDITION(GetCoordSys() == pi_rSimpleShape.GetCoordSys());

    switch (pi_Operation)
        {
        case HVE2DSimpleShape::DIFF :
            // Perform super scan in DIFF mode
            SuperScanSCS(pi_rSimpleShape, false, true, true, false, pi_rListOfShapes, pi_rPoints);
            break;

        case HVE2DSimpleShape::DIFFFROM :
            // Perform super scan in DIFF mode
            SuperScanSCS(pi_rSimpleShape, true, false, false, true, pi_rListOfShapes, pi_rPoints);
            break;

        case HVE2DSimpleShape::INTERSECT:
            // Perform superscan in intersect mode
            SuperScanSCS(pi_rSimpleShape, true, true, true, true, pi_rListOfShapes, pi_rPoints);

            // If there are no shapes in result both are equal , then either one is
            // the result
            if (pi_rListOfShapes.size() == 0)
                {
                pi_rListOfShapes.push_back(static_cast<HVE2DSimpleShape*>(Clone()));
                }
            break;

        case HVE2DSimpleShape::UNION:
            // Perform superscan in union mode
            SuperScanSCS(pi_rSimpleShape, false, true, false, true, pi_rListOfShapes, pi_rPoints);
            // If there are no shapes in result both are equal , then either one is
            // the result
            if (pi_rListOfShapes.size() == 0)
                {
                pi_rListOfShapes.push_back(static_cast<HVE2DSimpleShape*>(Clone()));
                }
            break;

        }
    }



//-----------------------------------------------------------------------------
// SuperScanSCS
// PRIVATE
// This method decomposes the different parts of two simple shape interaction
// Prior to entry in the present function, the shapes have been pre-conditioned
// so that none of their components properly cross the other shape. The linears
// have all been split so that a component can flirt or be contiguous but
// not cross.
// NOTE:
//   Due to the fact the algorithm is highly performance critical and very
//   much monolytic, more than 25 lines of code have been used here
//-----------------------------------------------------------------------------
void HVE2DSimpleShape::SuperScanSCS(const HVE2DSimpleShape& pi_rSimpleShape,
                                    bool                   pi_WantInPtsOfShape1,
                                    bool                   pi_ScanShape1CW,
                                    bool                   pi_WantInPtsOfShape2,
                                    bool                   pi_ScanShape2CW,
                                    HoleList&               pi_rListOfShapes,
                                    const HGF2DLocationCollection& pi_rPoints) const
    {
    HPRECONDITION(GetCoordSys() == pi_rSimpleShape.GetCoordSys());

    HGF2DLocation MyStartPoint(GetCoordSys());
    HGF2DLocation CurrentPoint(GetCoordSys());


    // Obtain a copy of linears in proper direction
    // Obtain copy of linear in proper direction
    HAutoPtr<HVE2DComplexLinear>    pMySelfLinear(AllocateLinear(pi_ScanShape1CW ? HVE2DSimpleShape::CW : HVE2DSimpleShape::CCW));
    HAutoPtr<HVE2DComplexLinear>    pMyGivenLinear(pi_rSimpleShape.AllocateLinear(pi_ScanShape2CW ? HVE2DSimpleShape::CW : HVE2DSimpleShape::CCW));

    // Split those at all intersection points
    pMySelfLinear->SplitAtAllOnPointsSCS(pi_rPoints);
    pMyGivenLinear->SplitAtAllOnPointsSCS(pi_rPoints);

    // Now a special process is required for start points of one complex
    // located on the other
    HGF2DLocationCollection GivenAdditionalPoints;
    HVE2DComplexLinear::LinearList::const_iterator    SelfItr;
    for (SelfItr = pMySelfLinear->GetLinearList().begin() ;
         SelfItr != pMySelfLinear->GetLinearList().end() ;
         ++SelfItr)
        {
        if (pMyGivenLinear->IsPointOn((*SelfItr)->GetStartPoint()))
            GivenAdditionalPoints.push_back((*SelfItr)->GetStartPoint());
        }

    HGF2DLocationCollection SelfAdditionalPoints;
    HVE2DComplexLinear::LinearList::const_iterator    GivenItr;
    for (GivenItr = pMyGivenLinear->GetLinearList().begin() ;
         GivenItr != pMyGivenLinear->GetLinearList().end() ;
         ++GivenItr)
        {
        if (pMySelfLinear->IsPointOn((*GivenItr)->GetStartPoint()))
            SelfAdditionalPoints.push_back((*GivenItr)->GetStartPoint());
        }

    // Split those at all additional points
    if (SelfAdditionalPoints.size() > 0)
        pMySelfLinear->SplitAtAllOnPointsSCS(SelfAdditionalPoints);
    if (GivenAdditionalPoints.size() > 0)
        pMyGivenLinear->SplitAtAllOnPointsSCS(GivenAdditionalPoints);

    // Allocate a list of flags the same size as new self linear
    size_t      NumberOfFlags = pMySelfLinear->GetNumberOfLinears();
    HArrayAutoPtr<bool>      pMyFlags(new bool[NumberOfFlags]);

    // Initialize all flags to false
    for (size_t FlagIndex = 0 ; FlagIndex < NumberOfFlags ; FlagIndex++)
        pMyFlags[FlagIndex] = false;

    // Set up linear flipper
    const HVE2DShape* const             apShape[2] = {this, &pi_rSimpleShape};
    HVE2DComplexLinear*                 apLinear[2] = {pMySelfLinear.get(), pMyGivenLinear.get()};
    bool                               PolyIn[2] = {pi_WantInPtsOfShape1, pi_WantInPtsOfShape2};

    // Do Until all parts of self have been processed ...
    size_t      Index = 0;
    bool       AllSet;

    // Reference to current linear
    const HVE2DLinear* pCurrentLinear;

    HGF2DLocation   DumPoint(GetCoordSys());

    do
        {
        // Check if this part of self is properly positioned
        pCurrentLinear = &(pMySelfLinear->GetLinear(Index));

        HVE2DShape::SpatialPosition MyPartPosition;
        MyPartPosition = pi_rSimpleShape.CalculateSpatialPositionOfNonCrossingLinear(*pCurrentLinear);

        if ((MyPartPosition == HVE2DShape::S_OUT && pi_WantInPtsOfShape1) ||
            (MyPartPosition == HVE2DShape::S_IN && !pi_WantInPtsOfShape1))
            {
            // This part is mis-positioned
            // Indicate this part has been processed (discarded)
            pMyFlags[Index] = true;
            }
        else if(MyPartPosition == HVE2DShape::S_ON)
            {
            // The part overlays part of the given shape
            // We discard it, while remembering that even
            // if it cannot be used as a starting linear
            // it may be part of the solution
            pMyFlags[Index] = true;
            }
        else
            {
            // This is a part we want for our result
            pMyFlags[Index] = true;

            // We create a new complex linear
            HVE2DComplexLinear MyNewLinear(GetCoordSys());

            // Append the current part of self
            MyNewLinear.AppendLinear(*pCurrentLinear);

            // Followed shape is self
            HSINTX  ShapeIndex = 0;
            HSINTX  TestShapeIndex = 1;

            // Obtain start point
            MyStartPoint = pCurrentLinear->GetStartPoint();

            // Set current point to end point of kept part
            CurrentPoint = pCurrentLinear->GetEndPoint();

            // Do until we have come back to the start point
            do
                {
                // Increment index
                Index++;

                // Adjust index for passage out of valid range
                if (Index >= apLinear[ShapeIndex]->GetNumberOfLinears())
                    Index = 0;

                // Check validity of this part
                pCurrentLinear = &(apLinear[ShapeIndex]->GetLinear(Index));

                MyPartPosition = apShape[TestShapeIndex]->CalculateSpatialPositionOfNonCrossingLinear(*pCurrentLinear);


                if ((MyPartPosition == HVE2DShape::S_OUT && PolyIn[ShapeIndex]) ||
                    (MyPartPosition == HVE2DShape::S_IN && !PolyIn[ShapeIndex]))
                    {
                    // Indicate this part has been processed (discarded) if it is on self
                    if (apShape[ShapeIndex] == this)
                        pMyFlags[Index] = true;

                    // Change followed shape
                    HSINTX  DummyLong = ShapeIndex;
                    ShapeIndex = TestShapeIndex;
                    TestShapeIndex = DummyLong;

                    // Find the linear which has current point as start point
                    for (Index = 0;
                         (Index < apLinear[ShapeIndex]->GetNumberOfLinears() &&
                          !CurrentPoint.IsEqualTo((pCurrentLinear = &(apLinear[ShapeIndex]->GetLinear(Index)))->GetStartPoint())) ;
                         Index++)
                        ;


                    HASSERT(Index < apLinear[ShapeIndex]->GetNumberOfLinears());

                    // Indicate this part has been processed (discarded) if it is on self
                    if (apShape[ShapeIndex] == this)
                        pMyFlags[Index] = true;


                    }
                else if (MyPartPosition == HVE2DShape::S_ON)
                    {
                    // This part is located ON the test shape
                    // We continue on current only if the others part is
                    // not properly positioned

                    // Find the linear which has current point as start point
                    size_t  DumIndex;
                    for (DumIndex = 0;
                         (DumIndex < apLinear[TestShapeIndex]->GetNumberOfLinears() &&
                          !CurrentPoint.IsEqualTo(apLinear[TestShapeIndex]->GetLinear(DumIndex).GetStartPoint())) ;
                         DumIndex++)
                        ;

                    HASSERT(DumIndex < apLinear[TestShapeIndex]->GetNumberOfLinears());

                    // Obtain its position
                    MyPartPosition = apShape[ShapeIndex]->CalculateSpatialPositionOfNonCrossingLinear(apLinear[TestShapeIndex]->GetLinear(DumIndex));

                    // Determine if this part is properly positioned
                    if ((MyPartPosition == HVE2DShape::S_OUT && !PolyIn[TestShapeIndex]) ||
                        (MyPartPosition == HVE2DShape::S_IN && PolyIn[TestShapeIndex]))
                        {
                        // It is properly positionned ... so we change
                        // Indicate this part has been processed (discarded) if it is on self
                        if (apShape[ShapeIndex] == this)
                            pMyFlags[Index] = true;

                        // Change followed shape
                        HSINTX  DummyLong = ShapeIndex;
                        ShapeIndex = TestShapeIndex;
                        TestShapeIndex = DummyLong;

                        Index = DumIndex;

                        // Indicate that new part is taken (if on self)
                        if (apShape[ShapeIndex] == this)
                            pMyFlags[Index] = true;

                        pCurrentLinear = &(apLinear[ShapeIndex]->GetLinear(Index));
                        }
                    else
                        {
                        // We stay on current linear ... indicate this part
                        // has been taken (if on self)
                        if (apShape[ShapeIndex] == this)
                            pMyFlags[Index] = true;

                        }

                    }
                else
                    {
                    // Indicate this part has been processed (taken) if it is on self
                    if (apShape[ShapeIndex] == this)
                        pMyFlags[Index] = true;
                    }


                HASSERT(pCurrentLinear->GetStartPoint() != pCurrentLinear->GetEndPoint());

                // Append this linear to our new linear
                MyNewLinear.AppendLinear(*pCurrentLinear);

                CurrentPoint = pCurrentLinear->GetEndPoint();

                }
            while (!CurrentPoint.IsEqualTo(MyStartPoint));

            // Check if start and end points are exactly equal
            if (MyNewLinear.GetStartPoint() != MyNewLinear.GetEndPoint())
                {
                // These are different by less than an epsilon ... adjust
                MyNewLinear.AdjustEndPointTo(MyNewLinear.GetStartPoint());
                }

            // Create a polygon with created linear
            HVE2DSimpleShape* pMyNewShape = new HVE2DPolygon(MyNewLinear);

            // Append this polygon to list of shapes
            pi_rListOfShapes.push_back(pMyNewShape);
            }

        // Check if all flags are set and if not find first not set
        AllSet = true;
        for (Index = 0 ; ((Index < NumberOfFlags) && (AllSet = pMyFlags[Index])) ; Index++)
            ;

        }
    while (!AllSet);

    // It may happen that there is no result shape in the case, the shapes are identical

    }


//-----------------------------------------------------------------------------
// DifferentiateFromShapeSCS
// This method create a new shape as the difference between self and given.
// The coordinate system of both shapes must be the same
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DSimpleShape::DifferentiateFromShapeSCS(const HVE2DShape& pi_rShape) const
    {
    HPRECONDITION(GetCoordSys() == pi_rShape.GetCoordSys());

    HAutoPtr<HVE2DShape>     pMyResultShape;

    if (IsEmpty())
        {
        // Since self is empty, the result is given
        pMyResultShape = static_cast<HVE2DShape*>(pi_rShape.Clone());
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
        pMyResultShape = DifferentiateFromSimpleShapeSCS((*(HVE2DSimpleShape*)(&pi_rShape)));
        }

    return (pMyResultShape.release());
    }

//-----------------------------------------------------------------------------
// DifferentiateFromSimpleShapeSCS
// PRIVATE METHOD
// This method creates a new shape as the difference between self and given.
// The coordinate systems of both shapes must be the same
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DSimpleShape::DifferentiateFromSimpleShapeSCS(const HVE2DSimpleShape& pi_rSimpleShape) const
    {
    // The two shape must share the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rSimpleShape.GetCoordSys());

    // Neither shape may be empty
    HPRECONDITION(!IsEmpty() && !pi_rSimpleShape.IsEmpty());

    HAutoPtr<HVE2DShape>     pMyResultShape;

    // We separate the process depending on the fact that they cross or not

    // We first compare if their extent overlap
    if (!GetExtent().InnerOverlaps(pi_rSimpleShape.GetExtent(), MIN(GetTolerance(), pi_rSimpleShape.GetTolerance())))
        {
        // The two simple shape cannot possibly intersect ... therefore, the difference is given
        pMyResultShape = static_cast<HVE2DShape*>(pi_rSimpleShape.Clone());
        }
    else
        {
        HGF2DLocationCollection IntersectPoints;

        // Check if they cross and if they do ontain intersection points.
        if (Intersect(pi_rSimpleShape, &IntersectPoints) > 0)
            {
            // The shape intersect each other ... long process
            // We append contiguousness points if any
            if (AreContiguous(pi_rSimpleShape))
                ObtainContiguousnessPoints(pi_rSimpleShape, &IntersectPoints);

            pMyResultShape = DifferentiateFromCrossingSimpleShapeSCS(pi_rSimpleShape, IntersectPoints);
            }
        else
            {
            // Even though their extent is not disjoint, they may still be disjoint
            // We therefore evaluate their spatial position
            HVE2DShape::SpatialPosition     TheGivenPosition = CalculateSpatialPositionOfNonCrossingSimpleShape(pi_rSimpleShape);

            // The shapes cannot be partialy in since they do not intersect
            HASSERT(TheGivenPosition != HVE2DShape::S_PARTIALY_IN);

            if ((TheGivenPosition == HVE2DShape::S_IN) || (TheGivenPosition == HVE2DShape::S_ON))
                {
                // The given is completely located inside self ... the result is therefore empty
                pMyResultShape = new HVE2DVoidShape(GetCoordSys());
                }
            else
                {
                // The given shape is OUT, but this does not imply that self is also OUT ... check
                HVE2DShape::SpatialPosition     TheSelfPosition = pi_rSimpleShape.CalculateSpatialPositionOf(*this);

                // The only possible results are IN or OUT
                HASSERT(TheSelfPosition == HVE2DShape::S_IN || TheSelfPosition == HVE2DShape::S_OUT);

                if (TheSelfPosition == HVE2DShape::S_OUT)
                    {
                    // The two simple shape are disjoint ... therefore, the difference is given
                    pMyResultShape = static_cast<HVE2DShape*>(pi_rSimpleShape.Clone());
                    }
                else
                    {
                    // Self cannot be S_PARTIALY_IN nor S_ON the given
                    // (since the given is not S_PARTIALY_IN nor S_ON self), then
                    // Self is located IN the given ...

                    // We check if they are nevertheless contiguous
                    if (AreContiguous(pi_rSimpleShape))
                        {
                        // Since they are contiguous ... we process as if they crossed
                        // We append contiguousness points
                        ObtainContiguousnessPoints(pi_rSimpleShape, &IntersectPoints);

                        pMyResultShape = DifferentiateFromCrossingSimpleShapeSCS(pi_rSimpleShape, IntersectPoints);
                        }
                    else
                        {
                        // The result is therefore a holed polygon
                        // with given as outter shape and self as hole
                        HAutoPtr<HVE2DHoledShape> pMyResultHoledShape(new HVE2DHoledShape(pi_rSimpleShape));
                        pMyResultHoledShape->AddHole(*this);

                        pMyResultShape = pMyResultHoledShape.release();
                        }
                    }
                }
            }
        }

    return (pMyResultShape.release());

    }


//-----------------------------------------------------------------------------
// DifferentiateFromCrossingSimpleShapeSCS
// PRIVATE METHOD
// This method creates a new shape as the difference between self and given.
// The coordinate systems of both shapes must be the same
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DSimpleShape::DifferentiateFromCrossingSimpleShapeSCS(const HVE2DSimpleShape& pi_rSimpleShape,
                                                                      const HGF2DLocationCollection& pi_rPoints) const
    {
    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rSimpleShape.GetCoordSys());

    // The two shapes must cross each other or at least be contiguous.
    HPRECONDITION(Crosses(pi_rSimpleShape) || AreContiguous(pi_rSimpleShape));

    // neither shape must be empty
    HPRECONDITION(!IsEmpty() && !pi_rSimpleShape.IsEmpty());

    HAutoPtr<HVE2DShape>     pMyResultShape;

    // Create recipient list
    HVE2DShape::HoleList   MyListOfSimpleShapes;

    // Perform decomposition process
    DecomposeSCS(pi_rSimpleShape, HVE2DSimpleShape::DIFFFROM, MyListOfSimpleShapes, pi_rPoints);

    // In the case of a differentiation, all the different shapes returned are disjoint
    if (MyListOfSimpleShapes.size() == 0)
        {
        // The two shapes were either identical or ...
        pMyResultShape = new HVE2DVoidShape(GetCoordSys());
        }
    else if (MyListOfSimpleShapes.size() > 1)
        {
        // Generate a complex shape
        pMyResultShape = new HVE2DComplexShape(MyListOfSimpleShapes);
        }
    else
        {
        pMyResultShape = static_cast<HVE2DShape*>((*MyListOfSimpleShapes.begin())->Clone());
        }

    // Destroy list
    HVE2DShape::HoleList::iterator    MyIterator = MyListOfSimpleShapes.begin();

    while (MyIterator != MyListOfSimpleShapes.end())
        {
        delete *MyIterator;

        MyIterator++;
        }

    return (pMyResultShape.release());

    }



//-----------------------------------------------------------------------------
// DifferentiateShapeSCS
// This method creates a new shape as the difference between self and given.
// The coordinate systems of both shapes must be the same
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DSimpleShape::DifferentiateShapeSCS(const HVE2DShape& pi_rShape) const
    {
    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rShape.GetCoordSys());

    HAutoPtr<HVE2DShape>     pMyResultShape;

    // We check if self is empty or given is universe
    if (IsEmpty() || pi_rShape.GetShapeType() == HVE2DUniverse::CLASS_ID)
        {
        // Since self is empty ... result is empty
        pMyResultShape = new HVE2DVoidShape(GetCoordSys());
        }
    else if (pi_rShape.IsEmpty())
        {
        // Since given is empty, result is self
        pMyResultShape = static_cast<HVE2DShape*>(Clone());
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
        pMyResultShape = DifferentiateSimpleShapeSCS((*(HVE2DSimpleShape*)(&pi_rShape)));
        }

    return (pMyResultShape.release());
    }


//-----------------------------------------------------------------------------
// DifferentiateSimpleShapeSCS
// PRIVATE METHOD
// This method creates a new shape as the difference between self and given.
// The coordinate systems of both shapes must be the same
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DSimpleShape::DifferentiateSimpleShapeSCS(const HVE2DSimpleShape& pi_rSimpleShape) const
    {
    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rSimpleShape.GetCoordSys());

    // The shapes cannot be empty
    HPRECONDITION(!IsEmpty() && !pi_rSimpleShape.IsEmpty());

    HAutoPtr<HVE2DShape>     pMyResultShape;

    // We separate the process depending on the fact that they cross or not
    // We compare if their extent overlap
    if (!GetExtent().InnerOverlaps(pi_rSimpleShape.GetExtent(), MIN(GetTolerance(), pi_rSimpleShape.GetTolerance())))
        {
        // The two simple shape cannot possibly intersect ... therefore, the difference is self
        pMyResultShape = static_cast<HVE2DShape*>(Clone());
        }
    else
        {
        // Their extents do overlap, we check if the cross and if they do extract crossing points
        HGF2DLocationCollection IntersectPoints;
        if (Intersect(pi_rSimpleShape, &IntersectPoints) > 0)
            {
            // The shape intersect each other ... long process
            // We append contiguousness points if any
            if (AreContiguous(pi_rSimpleShape))
                ObtainContiguousnessPoints(pi_rSimpleShape, &IntersectPoints);

            pMyResultShape = DifferentiateCrossingSimpleShapeSCS(pi_rSimpleShape, IntersectPoints);
            }
        else
            {
            // Even though their extent is not disjoint, they may still be disjoint
            // We therefore evaluate their spatial position
            HVE2DShape::SpatialPosition     TheGivenPosition = CalculateSpatialPositionOf(pi_rSimpleShape);

            // The shapes cannot be partialy in since they do not cross
            HASSERT(TheGivenPosition != HVE2DShape::S_PARTIALY_IN);

            if (TheGivenPosition == HVE2DShape::S_ON)
                {
                // The two shapes are identical ... result is empty
                pMyResultShape = new HVE2DVoidShape(GetCoordSys());
                }
            else if (TheGivenPosition == HVE2DShape::S_IN)
                {
                // The given is completely located inside self ...

                // We check if they are contiguous
                if (AreContiguous(pi_rSimpleShape))
                    {
                    // We append contiguousness points
                    ObtainContiguousnessPoints(pi_rSimpleShape, &IntersectPoints);


                    pMyResultShape = DifferentiateCrossingSimpleShapeSCS(pi_rSimpleShape, IntersectPoints);
                    }
                else
                    {
                    //the result is therefore a holed polygon
                    // with self as outter shape and given as hole
                    HAutoPtr<HVE2DHoledShape> pMyResultHoledShape(new HVE2DHoledShape(*this));
                    pMyResultHoledShape->AddHole(pi_rSimpleShape);

                    pMyResultShape = pMyResultHoledShape.release();
                    }
                }
            else
                {
                // The given shape is OUT, but this does not imply that self is also OUT ... check
                HVE2DShape::SpatialPosition     TheSelfPosition = pi_rSimpleShape.CalculateSpatialPositionOf(*this);

                // The only possible results are in or out
                HASSERT(TheSelfPosition == HVE2DShape::S_IN || TheSelfPosition == HVE2DShape::S_OUT);

                if (TheSelfPosition == HVE2DShape::S_OUT)
                    {
                    // The two simple shape are disjoint ... therefore, the difference is self
                    pMyResultShape = static_cast<HVE2DShape*>(Clone());
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

    return (pMyResultShape.release());

    }

//-----------------------------------------------------------------------------
// DifferentiateCrossingSimpleShapeSCS
// PRIVATE METHOD
// This method creates a new shape as the difference between self and given.
// The coordinate systems of both shapes must be the same
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DSimpleShape::DifferentiateCrossingSimpleShapeSCS(const HVE2DSimpleShape& pi_rSimpleShape,
                                                                  const HGF2DLocationCollection& pi_rPoints) const
    {
    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rSimpleShape.GetCoordSys());

    // The shapes must cross each other or at least be contiguous.
    HPRECONDITION(Crosses(pi_rSimpleShape) || AreContiguous(pi_rSimpleShape));

    // Neither shapes must be empty
    HPRECONDITION(!IsEmpty() && !pi_rSimpleShape.IsEmpty());

    HAutoPtr<HVE2DShape>     pMyResultShape;


    // Create recipient list
    HVE2DShape::HoleList   MyListOfSimpleShapes;

    // Perform decomposition process
    DecomposeSCS(pi_rSimpleShape, HVE2DSimpleShape::DIFF, MyListOfSimpleShapes, pi_rPoints);

    // In the case of a differentiation, all the different shapes returned are disjoint
    if (MyListOfSimpleShapes.size() == 0)
        {
        pMyResultShape = new HVE2DVoidShape(GetCoordSys());
        }
    else if (MyListOfSimpleShapes.size() > 1)
        {
        // Generate a complex shape
        pMyResultShape = new HVE2DComplexShape(MyListOfSimpleShapes);
        }
    else
        {
        pMyResultShape = static_cast<HVE2DShape*>((*MyListOfSimpleShapes.begin())->Clone());
        }

    // Destroy list
    HVE2DShape::HoleList::iterator    MyIterator = MyListOfSimpleShapes.begin();

    while (MyIterator != MyListOfSimpleShapes.end())
        {
        delete *MyIterator;

        MyIterator++;
        }

    return (pMyResultShape.release());

    }


//-----------------------------------------------------------------------------
// IntersectShapeSCS
// This method creates a new shape as the intersection between self and given.
// The coordinate systems of both shapes must be the same
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DSimpleShape::IntersectShapeSCS(const HVE2DShape& pi_rShape) const
    {
    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rShape.GetCoordSys());

    HAutoPtr<HVE2DShape>     pMyResultShape;

    if (IsEmpty() || pi_rShape.IsEmpty())
        {
        // Since at least one shape is empty, the result is empty
        pMyResultShape = new HVE2DVoidShape(GetCoordSys());
        }
    else if (pi_rShape.GetShapeType() == HVE2DUniverse::CLASS_ID)
        {
        pMyResultShape = static_cast<HVE2DShape*>(Clone());
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
        pMyResultShape = IntersectSimpleShapeSCS((*(HVE2DSimpleShape*)(&pi_rShape)));
        }

    return (pMyResultShape.release());

    }


//-----------------------------------------------------------------------------
// IntersectSimpleShapeSCS
// PRIVATE METHOD
// This method creates a new shape as the intersection between self and given.
// The coordinate systems of both shapes must be the same
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DSimpleShape::IntersectSimpleShapeSCS(const HVE2DSimpleShape& pi_rSimpleShape) const
    {
    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rSimpleShape.GetCoordSys());

    // Neither shape should be empty
    HPRECONDITION(!IsEmpty() && !pi_rSimpleShape.IsEmpty());

    HAutoPtr<HVE2DShape>     pMyResultShape;

    // We separate the process depending on the fact that they cross or not

    // We first compare if their extent overlap
    if (!GetExtent().InnerOverlaps(pi_rSimpleShape.GetExtent(), MIN(GetTolerance(), pi_rSimpleShape.GetTolerance())))
        {
        // The two simple shape cannot possibly intersect ... therefore, the intersection is empty
        pMyResultShape = new HVE2DVoidShape(GetCoordSys());
        }
    else
        {
        HGF2DLocationCollection IntersectPoints;

        if (Intersect(pi_rSimpleShape, &IntersectPoints) > 0)
            {
            // The shape intersect each other ... long process
            // We append contiguousness points if any
            if (AreContiguous(pi_rSimpleShape))
                ObtainContiguousnessPoints(pi_rSimpleShape, &IntersectPoints);

            pMyResultShape = IntersectCrossingSimpleShapeSCS(pi_rSimpleShape, IntersectPoints);
            }
        else
            {
            // Even though their extent is not disjoint, they may still be disjoint
            // We therefore evaluate their spatial position
            HVE2DShape::SpatialPosition     TheGivenPosition = CalculateSpatialPositionOf(pi_rSimpleShape);

            // The shapes cannot be partialy in since did not intersect
            HASSERT(TheGivenPosition != HVE2DShape::S_PARTIALY_IN);

            if ((TheGivenPosition == HVE2DShape::S_IN) || (TheGivenPosition == HVE2DShape::S_ON))
                {
                // The given is completely located inside self ... the result is therefore given
                pMyResultShape = static_cast<HVE2DShape*>(pi_rSimpleShape.Clone());
                }
            else
                {
                // The given shape is OUT, but this does not imply that self is also OUT ... check
                HVE2DShape::SpatialPosition     TheSelfPosition = pi_rSimpleShape.CalculateSpatialPositionOf(*this);

                // Self can only be either inside or outside
                HASSERT(TheSelfPosition == HVE2DShape::S_IN || TheSelfPosition == HVE2DShape::S_OUT);

                if (TheSelfPosition == HVE2DShape::S_OUT)
                    {
                    // The two shapes are technically disjoint, but they still can be contiguous
                    // Even if contiguous ... result is empty
                    // The two simple shape are disjoint ... therefore, the intersection is empty
                    pMyResultShape = new HVE2DVoidShape(GetCoordSys());
                    }
                else
                    {
                    // Self cannot be S_PARTIALY_IN nor S_ON the given
                    // (since the given is not S_PARTIALY_IN nor S_ON self), then
                    // Self is located S_IN the given ... the result is therefore self
                    pMyResultShape = static_cast<HVE2DShape*>(Clone());
                    }
                }
            }
        }

    return (pMyResultShape.release());

    }


//-----------------------------------------------------------------------------
// IntersectCrossingSimpleShapeSCS
// PRIVATE METHOD
// This method creates a new shape as the intersection between self and given.
// The coordinate systems of both shapes must be the same
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DSimpleShape::IntersectCrossingSimpleShapeSCS(const HVE2DSimpleShape& pi_rSimpleShape,
                                                              const HGF2DLocationCollection& pi_rPoints) const
    {
    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rSimpleShape.GetCoordSys());

    // The two shapes must cross
    HPRECONDITION(Crosses(pi_rSimpleShape) || AreContiguous(pi_rSimpleShape));

    // Neither shape should be empty
    HPRECONDITION(!IsEmpty() && !pi_rSimpleShape.IsEmpty());

    HAutoPtr<HVE2DShape>     pMyResultShape;

    // Create recipient list
    HVE2DShape::HoleList   MyListOfSimpleShapes;

    // Perform decomposition process
    DecomposeSCS(pi_rSimpleShape, HVE2DSimpleShape::INTERSECT, MyListOfSimpleShapes, pi_rPoints);

    // There should always be at least one shape in result
    HPOSTCONDITION(MyListOfSimpleShapes.size() != 0);

    // In the case of a intersection, all the different shapes returned are disjoint
    if (MyListOfSimpleShapes.size() > 1)
        {
        // Generate a complex shape
        pMyResultShape = new HVE2DComplexShape(MyListOfSimpleShapes);
        }
    else
        {
        pMyResultShape = static_cast<HVE2DShape*>((*MyListOfSimpleShapes.begin())->Clone());
        }

    // Destroy list
    HVE2DShape::HoleList::iterator    MyIterator = MyListOfSimpleShapes.begin();

    while (MyIterator != MyListOfSimpleShapes.end())
        {
        delete *MyIterator;

        MyIterator++;
        }

    return (pMyResultShape.release());

    }




//-----------------------------------------------------------------------------
// UnifyShapeSCS
// This method creates a new shape as the union between self and given.
// The coordinate systems of both shapes must be the same
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DSimpleShape::UnifyShapeSCS(const HVE2DShape& pi_rShape) const
    {
    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rShape.GetCoordSys());

    HAutoPtr<HVE2DShape>     pMyResultShape;

    // If self is empty or given is universe
    if (IsEmpty() || pi_rShape.GetShapeType() == HVE2DUniverse::CLASS_ID)
        {
        // Since self is empty (or given is universe... result is given
        pMyResultShape = static_cast<HVE2DShape*>(pi_rShape.Clone());
        }
    else if (pi_rShape.IsEmpty())
        {
        // Since given is empty, result is self
        pMyResultShape = static_cast<HVE2DShape*>(Clone());
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
        pMyResultShape = UnifySimpleShapeSCS((*(HVE2DSimpleShape*)(&pi_rShape)));
        }

    return (pMyResultShape.release());

    }

//-----------------------------------------------------------------------------
// UnifySimpleShapeSCS
// PRIVATE METHOD
// This method creates a new shape as the union between self and given.
// The coordinate systems of both shapes must be the same
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DSimpleShape::UnifySimpleShapeSCS(const HVE2DSimpleShape& pi_rSimpleShape) const
    {
    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rSimpleShape.GetCoordSys());

    // Neither shape should be empty
    HPRECONDITION(!IsEmpty() && !pi_rSimpleShape.IsEmpty());


    HAutoPtr<HVE2DShape>     pMyResultShape;

    // We separate the process depending on the fact that they cross or not

    // We first compare if their extent overlap
    if (!GetExtent().OutterOverlaps(pi_rSimpleShape.GetExtent(), MIN(GetTolerance(),
                                                                     pi_rSimpleShape.GetTolerance())) &&
        !(AreContiguous(pi_rSimpleShape)))
        {
        // The two simple shape cannot possibly intersect ... therefore, the union is a
        // complex shape containing both of them
        HAutoPtr<HVE2DComplexShape> pMyResultComplexShape(new HVE2DComplexShape(GetCoordSys()));

        // Append to this complex shape the two simple shapes
        pMyResultComplexShape->AddShape(*this);
        pMyResultComplexShape->AddShape(pi_rSimpleShape);

        pMyResultShape = pMyResultComplexShape.release();
        }
    else
        {
        // Their extents do overlap, we check if the cross and if they do extract crossing points
        HGF2DLocationCollection IntersectPoints;
        if (Intersect(pi_rSimpleShape, &IntersectPoints) > 0)
            {
            // The shape intersect each other ... long process
            // We append contiguousness points if any
            if (AreContiguous(pi_rSimpleShape))
                ObtainContiguousnessPoints(pi_rSimpleShape, &IntersectPoints);

            pMyResultShape = UnifyCrossingSimpleShapeSCS(pi_rSimpleShape, IntersectPoints);
            }
        else
            {

            // Even though their extent is not disjoint, they may still be disjoint
            // We therefore evaluate their spatial position
            HVE2DShape::SpatialPosition     TheGivenPosition = CalculateSpatialPositionOf(pi_rSimpleShape);

            // The position cannot be partialy in since they do not cross
            HASSERT(TheGivenPosition != HVE2DShape::S_PARTIALY_IN);

            if (TheGivenPosition == HVE2DShape::S_IN || TheGivenPosition == HVE2DShape::S_ON)
                {
                // The given is completely located inside self ... the result is therefore self
                pMyResultShape = static_cast<HVE2DShape*>(Clone());
                }
            else
                {
                // The given shape is OUT, but this does not imply that self is also OUT ... check
                HVE2DShape::SpatialPosition     TheSelfPosition = pi_rSimpleShape.CalculateSpatialPositionOf(*this);

                // The result can be either in or out only
                HASSERT(TheSelfPosition == HVE2DShape::S_IN || TheSelfPosition == HVE2DShape::S_OUT);

                if (TheSelfPosition == HVE2DShape::S_OUT)
                    {
                    // The two shapes are out of each other
                    // They may still be contiguous however
                    if (AreContiguous(pi_rSimpleShape))
                        {
                        // We append contiguousness points if any
                        ObtainContiguousnessPoints(pi_rSimpleShape, &IntersectPoints);

                        pMyResultShape = UnifyCrossingSimpleShapeSCS(pi_rSimpleShape, IntersectPoints);
                        }
                    else
                        {
                        // The two simple shape are disjoint ... therefore, the union is a
                        // complex shape containing both of them
                        HAutoPtr<HVE2DComplexShape> pMyResultComplexShape(new HVE2DComplexShape(GetCoordSys()));

                        // Append to this complex shape the two simple shapes
                        pMyResultComplexShape->AddShape(*this);
                        pMyResultComplexShape->AddShape(pi_rSimpleShape);

                        pMyResultShape = pMyResultComplexShape.release();
                        }
                    }
                else
                    {
                    // Self cannot be S_PARTIALY_IN nor S_ON the given
                    // (since the given is not S_PARTIALY_IN nor S_ON self), then
                    // Self is located S_IN the given ... the result is therefore the given
                    pMyResultShape = static_cast<HVE2DShape*>(pi_rSimpleShape.Clone());
                    }
                }
            }
        }

    return (pMyResultShape.release());

    }


//-----------------------------------------------------------------------------
// UnifyCrossingSimpleShapeSCS
// PRIVATE METHOD
// This method create a new shape as the union between self and given.
// The coordinate system of both shapes must be the same
//-----------------------------------------------------------------------------
HVE2DShape* HVE2DSimpleShape::UnifyCrossingSimpleShapeSCS(const HVE2DSimpleShape& pi_rSimpleShape,
                                                          const HGF2DLocationCollection& pi_rPoints) const
    {
    // The two shapes must have the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rSimpleShape.GetCoordSys());

    // The shapes should cross or be contiguous
    HPRECONDITION(Crosses(pi_rSimpleShape) || AreContiguous(pi_rSimpleShape));

    // Neither shape should be empty
    HPRECONDITION(!IsEmpty() && !pi_rSimpleShape.IsEmpty());

    HAutoPtr<HVE2DShape>     pMyResultShape;


    // Create recipient list
    HVE2DShape::HoleList   MyListOfSimpleShapes;

    // Perform decomposition process
    DecomposeSCS(pi_rSimpleShape, HVE2DSimpleShape::UNION, MyListOfSimpleShapes, pi_rPoints);

    // There should always be at least one shape in result
    HPOSTCONDITION(MyListOfSimpleShapes.size() != 0);

    // In the case of a union, if there are more than one simple shape returned, then only one is the outter shape
    // and the rest are holes
    if (MyListOfSimpleShapes.size() > 1)
        {
        bool Found = false;

        // There are many shapes ... find the one containing the others
        // As soon as one contain any other, it has been found
        HVE2DShape::SimpleShapeList::iterator MyShapeIterator = MyListOfSimpleShapes.begin();

        // The loop should never go past the end of the list
        HVE2DShape::SimpleShapeList::iterator MyOtherShapeIterator;
        while (!Found)
            {
            // Set iterator to first shape in list
            MyOtherShapeIterator = MyListOfSimpleShapes.begin();

            // Until shape is found or no more shape in list...
            while (!Found && MyOtherShapeIterator != MyListOfSimpleShapes.end())
                {
                // Do not test if the two shape iterator represent the same shape
                if (MyShapeIterator != MyOtherShapeIterator)
                    Found = ((*MyShapeIterator)->CalculateSpatialPositionOf(**MyOtherShapeIterator) == HVE2DShape::S_IN);

                // Get to next shape in list
                MyOtherShapeIterator++;
                }
            if (!Found)
                MyShapeIterator++;
            }

        // Create holed shape
        HAutoPtr<HVE2DHoledShape> pMyResultHoledShape(new HVE2DHoledShape(**MyShapeIterator));

        // Add all holes and detroy shapes at the same time
        HVE2DShape::SimpleShapeList::iterator MyFinalShapeIterator = MyListOfSimpleShapes.begin();

        // Add all enclosed shape as holes
        while (MyFinalShapeIterator != MyListOfSimpleShapes.end())
            {
            // Check that it is not the enclosing shape before adding
            if (MyShapeIterator != MyFinalShapeIterator)
                pMyResultHoledShape->AddHole(**MyFinalShapeIterator);

            // Delete shape
            delete *MyFinalShapeIterator;

            // Go to next shape in list
            MyFinalShapeIterator++;
            }

        pMyResultShape = pMyResultHoledShape.release();

        }
    else
        {
        // There is a single shape ... return it
        pMyResultShape = (*MyListOfSimpleShapes.begin());
        }


    return (pMyResultShape.release());

    }


//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
void HVE2DSimpleShape::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    HVE2DShape::PrintState(po_rOutput);

    po_rOutput << "Object is a HVE2DSimpleShape" << endl;
    HDUMP0("Object is a HVE2DSimpleShape\n");

#endif
    }
