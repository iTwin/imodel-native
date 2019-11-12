//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HVE2DSegment
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>


#include <ImagePP/all/h/HGF2DTransfoModel.h>
#include <ImagePP/all/h/HGF2DSimilitude.h>

#include <ImagePP/all/h/HVE2DSegment.h>

HPM_REGISTER_CLASS(HVE2DSegment, HVE2DLinear)


#include <ImagePP/all/h/HVE2DComplexLinear.h>


//-----------------------------------------------------------------------------
// Rotate
// Rotates the segment
//-----------------------------------------------------------------------------
void HVE2DSegment::Rotate(double               pi_Angle,
                          const HGF2DLocation& pi_rOrigin)
    {
    // Create a location in current coordinate system
    HGF2DLocation  Origin(pi_rOrigin, GetCoordSys());

    GetSegmentPeer().Rotate(pi_Angle, Origin.GetPosition());

    // We need to extract both points as setting will reset the peer (loosing change to end point)
    HGF2DPosition newStartPoint = GetSegmentPeer().GetStartPoint();
    HGF2DPosition newEndPoint = GetSegmentPeer().GetEndPoint();

    SetStartPoint(HGF2DLocation(newStartPoint, GetCoordSys()));
    SetEndPoint(HGF2DLocation(newEndPoint, GetCoordSys()));

    // Adjust tolerance
    ResetTolerance();

    // clear peer
    ClearPeer();
    }

//-----------------------------------------------------------------------------
// AreContiguous
// Indicates if the linear is contiguous to the given
//-----------------------------------------------------------------------------
bool HVE2DSegment::AreContiguous(const HVE2DVector& pi_rVector) const
    {
    bool   Answer = false;

    // Check if the given vector is a segment
    if ((pi_rVector.GetMainVectorType() == HVE2DLinear::CLASS_ID) &&
        (((HVE2DLinear*)(&pi_rVector))->IsABasicLinear()) &&
        (((HVE2DBasicLinear*)(&pi_rVector))->GetBasicLinearType() == HVE2DSegment::CLASS_ID))
        {
        // It is a segment
        HVE2DSegment* pTheSegment = (HVE2DSegment*)(&pi_rVector);

        // Check if segments are not null
        if ((!GetStartPoint().IsEqualToSCS(GetEndPoint(), GetTolerance())) &&
            (!pTheSegment->GetStartPoint().IsEqualToSCS(pTheSegment->GetEndPoint(), GetTolerance())))
            {
            // Check if they share the same coordinate system
            if (GetCoordSys() == pTheSegment->GetCoordSys())
                {
                // The two segments have the same coordinate system
                Answer = AreSegmentsContiguousSCS(*pTheSegment);
                }
            else
                {
                // The two segments have different coordinate system

                // We check if it is linearity preserving
                if (GetCoordSys()->HasLinearityPreservingRelationTo(pTheSegment->GetCoordSys()))
                    {
                    // They are related through a linearity preserving model ... go ahead
                    Answer = AreSegmentsContiguous(*pTheSegment);
                    }
                else
                    {
                    // They are not related by a linearity preserving model ... we must allocate a copy
                    // In correct coordinate system
                    HVE2DVector* pTempVector = pTheSegment->AllocateCopyInCoordSys(GetCoordSys());

                    // Obtain answer
                    Answer = pTempVector->AreContiguous(*this);

                    // Destroy temporary copy
                    delete pTempVector;
                    }
                }
            }
        }
    else
        {
        // Since it is not a segment, the question is asked to the given
        Answer = pi_rVector.AreContiguous(*this);
        }


    return(Answer);
    }


//-----------------------------------------------------------------------------
// AreContiguousAt
// Indicates if the linear is contiguous to the given at given point
//-----------------------------------------------------------------------------
bool HVE2DSegment::AreContiguousAt(const HVE2DVector& pi_rVector,
                                    const HGF2DLocation& pi_rPoint) const
    {
    // The given point must be located on both vectors
    HPRECONDITION(IsPointOn(pi_rPoint) && pi_rVector.IsPointOn(pi_rPoint));

    bool   Answer = false;

    // Check if the given vector is a segment
    if ((pi_rVector.GetMainVectorType() == HVE2DLinear::CLASS_ID) &&
        (((HVE2DLinear*)(&pi_rVector))->IsABasicLinear()) &&
        (((HVE2DBasicLinear*)(&pi_rVector))->GetBasicLinearType() == HVE2DSegment::CLASS_ID))
        {
        // It is a segment
        // Since both points are located on the segments, then
        // being contiguous is a sufficient condition to insure that they are
        // contiguous AT given point

        // It is a segment
        HVE2DSegment* pTheSegment = (HVE2DSegment*)(&pi_rVector);

        // Check if segments are not null
        if ((!GetStartPoint().IsEqualToSCS(GetEndPoint(), GetTolerance())) &&
            (!pTheSegment->GetStartPoint().IsEqualToSCS(pTheSegment->GetEndPoint(), GetTolerance())))
            {
            // Check if they share the same coordinate system
            if (GetCoordSys() == pTheSegment->GetCoordSys())
                {
                // The two segments have the same coordinate system
                Answer = AreSegmentsContiguousSCS(*pTheSegment);
                }
            else
                {
                // The two segments have different coordinate system

                // We check if it is linearity preserving
                if (GetCoordSys()->HasLinearityPreservingRelationTo(pTheSegment->GetCoordSys()))
                    {
                    // They are related through a linearity preserving model ... go ahead
                    Answer = AreSegmentsContiguous(*pTheSegment);
                    }
                else
                    {
                    // They are not related by a linearity preserving model ... we must allocate a copy
                    // In correct coordinate system
                    HVE2DVector* pTempVector = pTheSegment->AllocateCopyInCoordSys(GetCoordSys());

                    // Obtain answer
                    Answer = pTempVector->AreContiguous(*this);

                    // Destroy temporary copy
                    delete pTempVector;
                    }
                }
            }
        }
    else
        {
        // The given is not a segment ...
        Answer = pi_rVector.AreContiguousAt(*this, pi_rPoint);
        }

    return(Answer);
    }





//-----------------------------------------------------------------------------
// Crosses
// Indicates if the two linear cross each other
//-----------------------------------------------------------------------------
bool HVE2DSegment::Crosses(const HVE2DVector& pi_rVector) const
    {
    bool   Answer;

    // Check if the given vector is a segment
    if ((pi_rVector.GetMainVectorType() == HVE2DLinear::CLASS_ID) &&
        (((HVE2DLinear*)(&pi_rVector))->IsABasicLinear()) &&
        (((HVE2DBasicLinear*)(&pi_rVector))->GetBasicLinearType() == HVE2DSegment::CLASS_ID))
        {
        // It is a segment
        HVE2DSegment* pTheSegment = (HVE2DSegment*)(&pi_rVector);

        // Check if they share the same coordinate system
        if (GetCoordSys() == pTheSegment->GetCoordSys())
            {
            // The two segments have the same coordinate system
            Answer = AreSegmentsCrossingSCS(*pTheSegment);
            }
        else
            {
            // The two segments have different coordinate system

            // We check if it is linearity preserving
            if (GetCoordSys()->HasLinearityPreservingRelationTo(pTheSegment->GetCoordSys()))
                {
                // They are related through a linearity preserving model ... go ahead
                Answer = AreSegmentsCrossing(*pTheSegment);
                }
            else
                {
                // They are not related by a linearity preserving model ... we must allocate a copy
                // In correct coordinate system
                HVE2DVector* pTempVector = pTheSegment->AllocateCopyInCoordSys(GetCoordSys());

                // Obtain answer
                Answer = pTempVector->Crosses(*this);

                // Destroy temporary copy
                delete pTempVector;
                }
            }
        }
    else
        {
        // Since it is not a segment, the question is asked to the given
        Answer = pi_rVector.Crosses(*this);
        }

    return(Answer);
    }


//-----------------------------------------------------------------------------
// AreAdjacent
// Indicates if the linear is adjacent to the given
//-----------------------------------------------------------------------------
bool HVE2DSegment::AreAdjacent(const HVE2DVector& pi_rVector) const
    {
    bool   Answer;

    // Check if the given vector is a segment
    if ((pi_rVector.GetMainVectorType() == HVE2DLinear::CLASS_ID) &&
        (((HVE2DLinear*)(&pi_rVector))->IsABasicLinear()) &&
        (((HVE2DBasicLinear*)(&pi_rVector))->GetBasicLinearType() == HVE2DSegment::CLASS_ID))
        {
        // It is a segment
        HVE2DSegment* pTheSegment = (HVE2DSegment*)(&pi_rVector);

        // Check if they share the same coordinate system
        if (GetCoordSys() == pTheSegment->GetCoordSys())
            {
            // The two segments have the same coordinate system
            Answer = AreSegmentsAdjacentSCS(*pTheSegment);
            }
        else
            {
            // The two segments have different coordinate system

            // We check if it is linearity preserving
            if (GetCoordSys()->HasLinearityPreservingRelationTo(pTheSegment->GetCoordSys()))
                {
                // They are related through a linearity preserving model ... go ahead
                Answer = AreSegmentsAdjacent(*pTheSegment);
                }
            else
                {
                // They are not related by a linearity preserving model ... we must allocate a copy
                // In correct coordinate system
                HVE2DVector* pTempVector = pTheSegment->AllocateCopyInCoordSys(GetCoordSys());

                // Obtain answer
                Answer = pTempVector->AreAdjacent(*this);

                // Destroy temporary copy
                delete pTempVector;
                }
            }

        }
    else
        {
        // Since it is not a segment, the question is asked to the given
        Answer = pi_rVector.AreAdjacent(*this);
        }

    return(Answer);
    }

//-----------------------------------------------------------------------------
// Intersect
// Finds intersection point with vector
//-----------------------------------------------------------------------------
size_t HVE2DSegment::Intersect(const HVE2DVector& pi_rVector,
                               HGF2DLocationCollection* po_pCrossPoints) const
    {
    HPRECONDITION(po_pCrossPoints != 0);

    size_t  NumberOfCrossPoints = 0;

    // Check if the given is a segment
    if (pi_rVector.GetMainVectorType() == HVE2DLinear::CLASS_ID && (*(HVE2DLinear*)(&pi_rVector)).IsABasicLinear() &&
        ((*(HVE2DBasicLinear*)(&pi_rVector)).GetBasicLinearType() == HVE2DSegment::CLASS_ID))
        {
        // The given vector is a segment
        HVE2DSegment*   pMySegment = (HVE2DSegment*)(&pi_rVector);

        // We create a recipient location
        HGF2DLocation   MyCrossPoint(GetCoordSys());

        // Check if they share the same coordinate system
        if (GetCoordSys() == pMySegment->GetCoordSys())
            {
            // The two segments have the same coordinate system
            if (HVE2DSegment::CROSS_FOUND == IntersectSegmentSCS(*pMySegment, &MyCrossPoint))
                {
                po_pCrossPoints->push_back(MyCrossPoint);
                NumberOfCrossPoints = 1;
                }
            }
        else
            {
            // The two segments have different coordinate system

            // We check if it is linearity preserving
            if (GetCoordSys()->HasLinearityPreservingRelationTo(pMySegment->GetCoordSys()))
                {
                // They are related through a linearity preserving model ... go ahead
                if (HVE2DSegment::CROSS_FOUND == IntersectSegment(*pMySegment, &MyCrossPoint))
                    {
                    po_pCrossPoints->push_back(MyCrossPoint);
                    NumberOfCrossPoints = 1;
                    }
                }
            else
                {
                // They are not related by a linearity preserving model ... we must allocate a copy
                // In correct coordinate system
                HVE2DVector* pTempVector = pMySegment->AllocateCopyInCoordSys(GetCoordSys());

                // Intersect
                pTempVector->Intersect(*this, po_pCrossPoints);

                // Destroy temporary copy
                delete pTempVector;
                }
            }
        }
    else
        {
        // We have not a segment ... we ask the vector to perform the process
        NumberOfCrossPoints = pi_rVector.Intersect(*this, po_pCrossPoints);
        }

    return (NumberOfCrossPoints);
    }

//-----------------------------------------------------------------------------
// ObtainContiguousnessPoints
// Finds contiguousness point with vector
//-----------------------------------------------------------------------------
size_t HVE2DSegment::ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                HGF2DLocationCollection* po_pContiguousnessPoints) const
    {
    HPRECONDITION(po_pContiguousnessPoints != 0);

    size_t  NumberOfNewPoints;

    // Check if the given is a segment
    if (pi_rVector.GetMainVectorType() == HVE2DLinear::CLASS_ID && (*(HVE2DLinear*)(&pi_rVector)).IsABasicLinear() &&
        ((*(HVE2DBasicLinear*)(&pi_rVector)).GetBasicLinearType() == HVE2DSegment::CLASS_ID))
        {
        // The given vector is a segment
        HVE2DSegment*   pMySegment = (HVE2DSegment*)(&pi_rVector);

        // Check if they share the same coordinate system
        if (GetCoordSys() == pMySegment->GetCoordSys())
            {
            // The two segments have the same coordinate system
            // We obtain contiguousness points with segment
            NumberOfNewPoints = ObtainContiguousnessPointsWithSegmentSCS(*pMySegment, po_pContiguousnessPoints);
            }
        else
            {
            // The two segments have different coordinate system

            // We check if it is linearity preserving
            if (GetCoordSys()->HasLinearityPreservingRelationTo(pMySegment->GetCoordSys()))
                {
                // They are related through a linearity preserving model ... go ahead
                NumberOfNewPoints = ObtainContiguousnessPointsWithSegment(*pMySegment, po_pContiguousnessPoints);
                }
            else
                {
                // They are not related by a linearity preserving model ... we must allocate a copy
                // In correct coordinate system
                HVE2DVector* pTempVector = pMySegment->AllocateCopyInCoordSys(GetCoordSys());

                // Obtain answer
                NumberOfNewPoints = ObtainContiguousnessPoints(*pTempVector, po_pContiguousnessPoints);

                // Destroy temporary copy
                delete pTempVector;
                }
            }
        }
    else
        {
        HGF2DLocationCollection TempPoints;

        // We have not a segment ... we ask the vector to perform the process
        if ((NumberOfNewPoints = pi_rVector.ObtainContiguousnessPoints(*this, &TempPoints)) != 0)
            {
            // There are some contiguousness points ...


            // We have contiguousness points, however these are ordered according to the
            // vector that provided them. We need to re-order them according to segment
            // In order to do this we first re-order the points within pairs
            HGF2DLocationCollection::iterator MyItr = TempPoints.begin();
            HGF2DLocationCollection::iterator MySecondItr;

            while (MyItr != TempPoints.end())
                {
                // Position second itr to next point
                MySecondItr = MyItr;
                ++MySecondItr;

                // Check if they are in order
                if (CalculateRelativePosition(*MySecondItr) < CalculateRelativePosition(*MyItr))
                    {
                    // The pair is in improper order ... swap
                    HGF2DLocation TempPoint(*MyItr);
                    *MyItr = *MySecondItr;
                    *MySecondItr = TempPoint;
                    }

                // Advance to next pair
                ++MyItr;
                ++MyItr;
                }

            // Now we need to sort the pairs
            MyItr = TempPoints.begin();
            while (MyItr != TempPoints.end())
                {
                // Position second itr to next pair
                MySecondItr = MyItr;
                ++MySecondItr;
                ++MySecondItr;

                // Make sure there is another pair
                while (MySecondItr != TempPoints.end())
                    {
                    // Check if they are in order
                    if (CalculateRelativePosition(*MySecondItr) < CalculateRelativePosition(*MyItr))
                        {
                        // The pairs are in improper order ... swap
                        HGF2DLocation TempPoint(*MyItr);
                        *MyItr = *MySecondItr;
                        *MySecondItr = TempPoint;

                        // We need a third iterator not to change MyItr
                        HGF2DLocationCollection::iterator MyThirdItr(MyItr);
                        ++MyThirdItr;

                        // Advance in second pair
                        HGF2DLocationCollection::iterator MyFourthItr(MySecondItr);
                        ++MyFourthItr;

                        // Swap second point of pairs
                        TempPoint  = *MyThirdItr;
                        *MyThirdItr = *MyFourthItr;
                        *MyFourthItr = TempPoint;
                        }

                    // Advance to next pair
                    ++MySecondItr;
                    ++MySecondItr;
                    }

                // Advance to next pair
                ++MyItr;
                ++MyItr;
                }

            // Now we copy to recipient list
            HGF2DLocationCollection::iterator MyIterator = TempPoints.begin();

            while (MyIterator != TempPoints.end())
                {
                po_pContiguousnessPoints->push_back(*MyIterator);

                MyIterator++;
                }

            }
        }

    return (NumberOfNewPoints);
    }


//-----------------------------------------------------------------------------
// ObtainContiguousnessPointsAt
// Finds contiguousness point with vector
//-----------------------------------------------------------------------------
void HVE2DSegment::ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
                                                const HGF2DLocation& pi_rPoint,
                                                HGF2DLocation* po_pFirstContiguousnessPoint,
                                                HGF2DLocation* po_pSecondContiguousnessPoint) const
    {
    HPRECONDITION(po_pFirstContiguousnessPoint != 0);
    HPRECONDITION(po_pSecondContiguousnessPoint != 0);

    // The vectors must be contiguous at given point
    HPRECONDITION(AreContiguousAt(pi_rVector, pi_rPoint));

    // Check if the given is a segment
    if (pi_rVector.GetMainVectorType() == HVE2DLinear::CLASS_ID && (*(HVE2DLinear*)(&pi_rVector)).IsABasicLinear() &&
        ((*(HVE2DBasicLinear*)(&pi_rVector)).GetBasicLinearType() == HVE2DSegment::CLASS_ID))
        {
        // The given vector is a segment
        HVE2DSegment*   pMySegment = (HVE2DSegment*)(&pi_rVector);

        // Check if they share the same coordinate system
        if (GetCoordSys() == pMySegment->GetCoordSys())
            {
            HGF2DLocationCollection ContiguousnessPoints;

            ObtainContiguousnessPointsWithSegmentSCS(*pMySegment,
                                                     &ContiguousnessPoints);

            HASSERT(ContiguousnessPoints.size() == 2);

            *po_pFirstContiguousnessPoint = ContiguousnessPoints[0];
            *po_pSecondContiguousnessPoint = ContiguousnessPoints[1];

            }
        else if (GetCoordSys()->HasLinearityPreservingRelationTo(pMySegment->GetCoordSys()))
            {

            HGF2DLocationCollection ContiguousnessPoints;

            ObtainContiguousnessPointsWithSegment(*pMySegment,
                                                  &ContiguousnessPoints);

            HASSERT(ContiguousnessPoints.size() == 2);

            *po_pFirstContiguousnessPoint = ContiguousnessPoints[0];
            *po_pSecondContiguousnessPoint = ContiguousnessPoints[1];

            // The two segments have the same coordinate system
            // We obtain contiguousness points with segment
            }
        else
            {
            // They are not related by a linearity preserving model ... we must allocate a copy
            // In correct coordinate system
            HVE2DVector* pTempVector = pMySegment->AllocateCopyInCoordSys(GetCoordSys());

            // Obtain answer
            ObtainContiguousnessPointsAt(*pTempVector,
                                         pi_rPoint,
                                         po_pFirstContiguousnessPoint,
                                         po_pSecondContiguousnessPoint);

            // Destroy temporary copy
            delete pTempVector;
            }
        }
    else
        {
        // We have not a segment ... we ask the vector to perform the process
        pi_rVector.ObtainContiguousnessPointsAt(*this,
                                                pi_rPoint,
                                                po_pFirstContiguousnessPoint,
                                                po_pSecondContiguousnessPoint);

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
// AreContiguousAtAndGet
// Checks if contiguous and if yes finds contiguousness point with vector
//-----------------------------------------------------------------------------
bool HVE2DSegment::AreContiguousAtAndGet(const HVE2DVector& pi_rVector,
                                          const HGF2DLocation& pi_rPoint,
                                          HGF2DLocation* po_pFirstContiguousnessPoint,
                                          HGF2DLocation* po_pSecondContiguousnessPoint) const
    {
    HPRECONDITION(po_pFirstContiguousnessPoint != 0);
    HPRECONDITION(po_pSecondContiguousnessPoint != 0);

    bool Answer = false;

    // Check if the given is a segment
    if (pi_rVector.GetMainVectorType() == HVE2DLinear::CLASS_ID && (*(HVE2DLinear*)(&pi_rVector)).IsABasicLinear() &&
        ((*(HVE2DBasicLinear*)(&pi_rVector)).GetBasicLinearType() == HVE2DSegment::CLASS_ID))
        {
        // The given vector is a segment
        HVE2DSegment*   pMySegment = (HVE2DSegment*)(&pi_rVector);

        // Check if they share the same coordinate system
        if (GetCoordSys() == pMySegment->GetCoordSys())
            {
            // The two segments have the same coordinate system
            // We obtain contiguousness points with segment
            Answer = AreContiguousAtAndGetWithSegmentSCS(*pMySegment,
                                                         po_pFirstContiguousnessPoint,
                                                         po_pSecondContiguousnessPoint);
            }
        else if (GetCoordSys()->HasLinearityPreservingRelationTo(pMySegment->GetCoordSys()))
            {
            Answer = AreContiguousAtAndGetWithSegment(*pMySegment,
                                                      po_pFirstContiguousnessPoint,
                                                      po_pSecondContiguousnessPoint);

            }
        else
            {
            // They are not related by a linearity preserving model ... we must allocate a copy
            // In correct coordinate system
            HVE2DVector* pTempVector = pMySegment->AllocateCopyInCoordSys(GetCoordSys());

            // Obtain answer
            Answer = AreContiguousAtAndGet(*pTempVector,
                                           pi_rPoint,
                                           po_pFirstContiguousnessPoint,
                                           po_pSecondContiguousnessPoint);

            // Destroy temporary copy
            delete pTempVector;
            }
        }
    else
        {
        // We have not a segment ... we ask the vector to perform the process
        Answer = pi_rVector.AreContiguousAtAndGet(*this,
                                                  pi_rPoint,
                                                  po_pFirstContiguousnessPoint,
                                                  po_pSecondContiguousnessPoint);

        if (Answer)
            {
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

    return(Answer);
    }



//-----------------------------------------------------------------------------
// AllocateCopyInCoordSys
// Returns a dynamically allocated copy of the segment in a different coordinate
// system
//-----------------------------------------------------------------------------
HVE2DVector* HVE2DSegment::AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const
    {
    HVE2DVector*    pMyResultVector;

    // Check if it is the same coordinate system
    if (pi_rpCoordSys == GetCoordSys())
        {
        pMyResultVector = new HVE2DSegment(*this);
        }
    else
        {
        // Check if this model is translation, stretch, affine, similitude or identity
        if (GetCoordSys()->HasLinearityPreservingRelationTo(pi_rpCoordSys))
            {
            // The model preserves linearity ... we can transform the points directly
            pMyResultVector = new HVE2DSegment(GetStartPoint().ExpressedIn(pi_rpCoordSys),
                                               GetEndPoint().ExpressedIn(pi_rpCoordSys));
            }
        else
            {
            // The model either does not preserve linearity
            // We process more completely
            pMyResultVector = AllocateCopyInComplexCoordSys(pi_rpCoordSys);
            }

        pMyResultVector->SetStrokeTolerance(m_pStrokeTolerance);
        }

    return(pMyResultVector);
    }


//-----------------------------------------------------------------------------
// AppendItselfInCoordSys
// Adds to the given complex linear a transformed version of segment
//-----------------------------------------------------------------------------
void HVE2DSegment::AppendItselfInCoordSys(HVE2DComplexLinear& pio_rResultComplex,
                                          const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const
    {
    double StrokeTolerance (GetTolerance());

    HFCPtr<HGFTolerance> pStrokeTolerance = GetStrokeTolerance();

    pStrokeTolerance->ChangeCoordSys(GetCoordSys());

    if(pStrokeTolerance->GetLinearTolerance() > 0.0)
        {
        StrokeTolerance = pStrokeTolerance->GetLinearTolerance();
        }


    // Obtain the segment intermediate point
    HGF2DLocation   IntermediatePoint(GetStartPoint() + ((GetEndPoint() - GetStartPoint()) * 0.5));

    // Transform start point, end point and intermediate point
    // This is where the coordinates are effectively transformed!
// HChk AR !!!!!! The complex coordinate system may have domain limitations
//                in such case an error will be thrown ... check if it is here!!!!
    HVE2DSegment    TransformedSegment(GetStartPoint().ExpressedIn(pi_rpCoordSys),
                                       GetEndPoint().ExpressedIn(pi_rpCoordSys));
    HGF2DLocation   NewIntermediatePoint(IntermediatePoint, pi_rpCoordSys);

    // Check if tolerance is respected

    // Obtain distance from transformed segment
    double TheDistanceFromTransformed = (TransformedSegment.CalculateClosestPoint(NewIntermediatePoint)
                                              - NewIntermediatePoint).CalculateLength();

    if (HDOUBLE_EQUAL(TheDistanceFromTransformed, 0.0, GetTolerance()))
        {
        // The epsilon is respected ... we add this last segment to linear
        pio_rResultComplex.AppendLinear(TransformedSegment);
        }
    else
        {
        // Since the tolerance is not respected, we split the segment into two smaller ones
        HVE2DSegment    FirstSegment(GetStartPoint(), IntermediatePoint);
        FirstSegment.SetStrokeTolerance(m_pStrokeTolerance);

        FirstSegment.AppendItselfInCoordSys(pio_rResultComplex, pi_rpCoordSys);

        HVE2DSegment    SecondSegment(IntermediatePoint, GetEndPoint());
        SecondSegment.SetStrokeTolerance(m_pStrokeTolerance);

        SecondSegment.AppendItselfInCoordSys(pio_rResultComplex, pi_rpCoordSys);
        }
    }


//-----------------------------------------------------------------------------
// AllocateCopyInComplexCoordSys
// Returns a dynamically allocated copy of the segment transformed according
// in the given coordinate system
//-----------------------------------------------------------------------------
HVE2DVector* HVE2DSegment::AllocateCopyInComplexCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const
    {
    HVE2DVector*    pResultVector;

    // Allocate complex linear to receive result
    HVE2DComplexLinear*  pNewComplex = new HVE2DComplexLinear(GetCoordSys());

    // Add transformed segment to complex
    AppendItselfInCoordSys(*pNewComplex, pi_rpCoordSys);

    // Check if there are more than one linear in result
    if (pNewComplex->GetNumberOfLinears() > 1)
        {
        // The result is a complex linear
        pResultVector = pNewComplex;
        }
    else
        {
        // Since there is only one linear in complex,
        // the result was a simple segment
        pResultVector = new HVE2DSegment((*(HVE2DSegment*)(*(pNewComplex->GetLinearList().begin()))));

        delete pNewComplex;
        }

    pResultVector->SetStrokeTolerance(m_pStrokeTolerance);

    return (pResultVector);

    }



/** -----------------------------------------------------------------------------
    This method calculates and returns the crossing point between the
    given line.
    It is possible that no crossing point exists. In that case, the
    state of the returned point is undefined, and the returned state indicates
    if crossing could be performed, and for what reason it could not.
    Two reasons are possible for not crossing. If the segment and line
    are parallel, or if they are disjoint. The parallel condition is detected
    before checking if they are disjoint, so a paralllel status does not imply
    that the segment and line are disjoint or not. It follows that if segment
    is on top of the line, they do not cross.

    The point returned in interpreted in the same coordinate system as the segment.

    @param pi_rLine Constant reference to line to find crossing point with.

    @param po_pPoint Pointer to HGF2DLocation that receives the crossing
                     point if there is one.

    @return The status of the crossing operation. This status is
            HVE2DSegment::CROSS_FOUND if a crossing point is found,
            HVE2DSegment::PARALLEL if the segment is parallel to line given
            or HVE2DSegment::NO_CROSS if no crossing point exists between the segment
            and the given line. Note that the value of the HGF2DLocation pointed
            to by po_pPoint is undefined if the returned value is different
            from HVE2DSegment::CROSS_FOUND.

    Example
    @code
    @end

    @see HGF2DLine
    @see IntersectSegment
    -----------------------------------------------------------------------------
*/
HVE2DSegment::CrossState HVE2DSegment::IntersectLine(const HGF2DLine& pi_rLine,
                                                     HGF2DLocation* po_pPoint,
                                                     double pi_Tolerance) const
    {
    HPRECONDITION(po_pPoint != 0);

    // Check if they share the same coordinate system or they are related
    // through a linearity preserving model
    HPRECONDITION((GetCoordSys() == pi_rLine.GetCoordSys()) ||
                  (GetCoordSys()->GetTransfoModelTo(pi_rLine.GetCoordSys())->PreservesLinearity()));

    // Obtain tolerance
    double Tolerance = pi_Tolerance;
    if (Tolerance < 0.0)
        Tolerance = GetTolerance();

    // Create return object set initialy to no crosses
    HVE2DSegment::CrossState    Status = HVE2DSegment::NO_CROSS;

    // Get intersection point between the given line and the line the segment belongs to
    HGF2DLine::CrossState LineStatus = pi_rLine.IntersectLine(HGF2DLine(GetStartPoint(), GetEndPoint()), po_pPoint);

    // Check if the lines did cross
    if (LineStatus == HGF2DLine::CROSS_FOUND)
        {
        // The lines did cross .. check if the cross point is part of the segment
        // but not on an extremity
        if (IsPointOnLineOnSegment(*po_pPoint, Tolerance) &&
            (!GetStartPoint().IsEqualTo(*po_pPoint, Tolerance)) &&
            (!GetEndPoint().IsEqualTo(*po_pPoint, Tolerance)))
            // The point is part of the segment ... a cross has been found
            Status = HVE2DSegment::CROSS_FOUND;
        }
    // In the case no cross was found, find if this is because they are parallel
    else if (LineStatus == HGF2DLine::PARALLEL)
        // The line and segment are parallel to each other
        Status = HVE2DSegment::PARALLEL;

    return (Status);
    }

/** -----------------------------------------------------------------------------
    This method calculates and returns the crossing point between the
    given segment.
    It is possible that no crossing point exists. In that case, the
    state of the returned point is undefined, and the returned state indicates
    if crossing could be performed, and for what reason it could not.
    Two reasons are possible for not crossing. If the segments
    are parallel, or if they are disjoint. The parallel condition is detected
    before checking if they are disjoint, so a paralllel status does not imply
    that the segments are disjoint or not. It follows that if the segment
    is on top of the other, they do not cross.

    The point returned in interpreted in the same coordinate system as the segment.

    @param pi_rSegment Constant reference to segment to find crossing point with.

    @param po_pPoint Pointer to HGF2DLocation that receives the crossing
                     point if there is one.

    @return The status of the crossing operation. This status is
            HVE2DSegment::CROSS_FOUND if a crossing point is found,
            HVE2DSegment::PARALLEL if the segments are parallel
            or HVE2DSegment::NO_CROSS if no crossing point exists between the segments
            Note that the value of the HGF2DLocation pointed
            to by po_pPoint is undefined if the returned value is different
            from HVE2DSegment::CROSS_FOUND.

    Example
    @code
    HFCPtr<HGF2DCoordSys>    pMyWorld(new HGF2DCoordSys());
    HGF2DLocation        MyFirstPoint(10, 10, pMyWorld);
    HGF2DLocation        MySecondPoint(15, 16,  pMyWorld);
    HVE2DSegment        MySeg1(MyFirstPoint, MySecondPoint);
    ...
    HGF2DLocation          MyThirdPoint(15, 16, pMyWorld);
    HGF2Dlocation        MyFourthPoint(12, -3,  pMyWorld);
    HVE2DSegment        MySeg2(MyThirdPoint, MyFourthPoint);
    HGF2DLocaton         MyCrossPoint;
    HVE2DSegment::CrossState MyCrossState = MySeg1.IntersectSegment(MySeg2,
                                                        &MyCrossPoint);
    @end

    @see HGF2DLine
    @see IntersectLine()
    -----------------------------------------------------------------------------
*/
HVE2DSegment::CrossState HVE2DSegment::IntersectSegment(const HVE2DSegment& pi_rSegment,
                                                        HGF2DLocation* po_pPoint) const
    {
    HPRECONDITION(po_pPoint != 0);

    // Check if they share the same coordinate system or they are related
    // through a linearity preserving model
    HPRECONDITION((GetCoordSys() == pi_rSegment.GetCoordSys()) ||
                  (GetCoordSys()->HasLinearityPreservingRelationTo(pi_rSegment.GetCoordSys())));

    // Compute effective tolerance (smallest of the two given segment)
    double Tolerance = MIN(GetTolerance(), pi_rSegment.GetTolerance());

    // Create return object set initialy to no crosses
    HVE2DSegment::CrossState    Status = HVE2DSegment::NO_CROSS;

    // Get intersection point between the lines the segments belong to
    HGF2DLine::CrossState LineStatus = HGF2DLine(GetStartPoint(), GetEndPoint()).IntersectLine(HGF2DLine(pi_rSegment.GetStartPoint(), pi_rSegment.GetEndPoint()), po_pPoint);

    // Check if the lines did cross
    if (LineStatus == HGF2DLine::CROSS_FOUND)
        {
        // The lines did cross .. check if the cross point is part of both segment
        if (IsPointOnLineOnSegment(*po_pPoint, Tolerance) && pi_rSegment.IsPointOn(*po_pPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance) &&
            (!po_pPoint->IsEqualTo(GetStartPoint(), Tolerance)) && (!po_pPoint->IsEqualTo(GetEndPoint(), Tolerance)) &&
            (!po_pPoint->IsEqualTo(pi_rSegment.GetStartPoint(), Tolerance)) && (!po_pPoint->IsEqualTo(pi_rSegment.GetEndPoint(), Tolerance)) &&
            (!AreContiguous(pi_rSegment)) && !LinksTo(pi_rSegment))
            // The point is part of both segment ... a cross has been found
            Status = HVE2DSegment::CROSS_FOUND;
        }
    // In the case no cross was found, find if this is because the segments are parallel
    else if (LineStatus == HGF2DLine::PARALLEL)
        // The segments are parallel to each other
        Status = HVE2DSegment::PARALLEL;

    return (Status);
    }

//-----------------------------------------------------------------------------
// IntersectSegmentSCS
// Calculates and returns the intersection point with anotehr segment if there is one
//-----------------------------------------------------------------------------
HVE2DSegment::CrossState HVE2DSegment::IntersectSegmentSCS(const HVE2DSegment& pi_rSegment,
                                                           HGF2DLocation* po_pPoint) const
    {
    HPRECONDITION(po_pPoint != 0);

    // They must be expressed in the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rSegment.GetCoordSys());

    // Create return object set initialy to no crosses
    HVE2DSegment::CrossState    Status = HVE2DSegment::NO_CROSS;

    bool IntersectsAtExtremity = false;

    Status = IntersectSegmentExtremityIncludedSCS(pi_rSegment, po_pPoint, &IntersectsAtExtremity);

    if ((HVE2DSegment::CROSS_FOUND == Status) && IntersectsAtExtremity)
        {
        Status = HVE2DSegment::NO_CROSS;
        }

    return (Status);
    }


//-----------------------------------------------------------------------------
// IntersectSegmentExtremityIncludedSCS
// Calculates and returns the intersection point with another segment if there is one
// Contrary to the IntersectSegment method, an intersection is detected when the found
// cross point is located at any of the extremities of any segment.
//-----------------------------------------------------------------------------
HVE2DSegment::CrossState HVE2DSegment::IntersectSegmentExtremityIncludedSCS(const HVE2DSegment& pi_rSegment,
                                                                            HGF2DLocation* po_pPoint,
                                                                            bool* po_pIntersectsAtExtremity) const
    {
    HPRECONDITION(po_pPoint != 0);

    // They must be expressed in the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rSegment.GetCoordSys());

    HGF2DPosition intersectPoint;
    HGF2DSegment::CrossState crossState = GetSegmentPeer().IntersectSegmentExtremityIncluded(pi_rSegment.GetSegmentPeer(), 
                                                                                             &intersectPoint, 
                                                                                             po_pIntersectsAtExtremity);
    
    // Create return object set initialy to no crosses
    HVE2DSegment::CrossState    Status = (HVE2DSegment::CrossState)(crossState);

    *po_pPoint = HGF2DLocation(intersectPoint, GetCoordSys());

    


    return (Status);
    }





//-----------------------------------------------------------------------------
// ObtainContiguousnessPointsWithSegment
// Calculates and returns the contiguousness points with another segment if
// there is any
//-----------------------------------------------------------------------------
size_t HVE2DSegment::ObtainContiguousnessPointsWithSegment(const HVE2DSegment& pi_rSegment,
                                                           HGF2DLocationCollection* po_pContiguousnessPoints) const
    {
    HPRECONDITION(po_pContiguousnessPoints != 0);

    // The segments must be contiguous
    HPRECONDITION(AreSegmentsContiguous(pi_rSegment));

    // When contiguous together, two segments will have exactly 2
    // contiguousness points

    // Compute effective tolerance (smallest of the two given segment)
    double Tolerance = MIN(GetTolerance(), pi_rSegment.GetTolerance());

    // Save initial number of points in list
    size_t  InitialNumberOfPoints = po_pContiguousnessPoints->size();


    // We check if the start points are equal
    if (GetStartPoint().IsEqualTo(pi_rSegment.GetStartPoint(), Tolerance))
        {
        // The two start points are on top of each other
        // It is therefore one of the contiguousness points
        po_pContiguousnessPoints->push_back(GetStartPoint());

        // The second point is either one of the end points

        // Check if the end points are equal
        if (GetEndPoint().IsEqualTo(pi_rSegment.GetEndPoint(), Tolerance))
            po_pContiguousnessPoints->push_back(GetEndPoint());
        else
            {
            // Since the end points are not equal, then the one that is on the other segment
            // is the result
            if (IsPointOn(pi_rSegment.GetEndPoint(), HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                po_pContiguousnessPoints->push_back(pi_rSegment.GetEndPoint());
            else
                po_pContiguousnessPoints->push_back(GetEndPoint());
            }
        }
    else if (GetEndPoint().IsEqualTo(pi_rSegment.GetEndPoint(), Tolerance))
        {
        // The two end points are on top of each other

        // The other point is either one of the start point
        // the one that is on the other segment is the result
        if (IsPointOn(pi_rSegment.GetStartPoint(), HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
            po_pContiguousnessPoints->push_back(pi_rSegment.GetStartPoint());
        else
            po_pContiguousnessPoints->push_back(GetStartPoint());

        // The end point is therefore one of the contiguousness points
        po_pContiguousnessPoints->push_back(GetEndPoint());

        }
    else if (GetStartPoint().IsEqualTo(pi_rSegment.GetEndPoint(), Tolerance))
        {
        // The self start point in on top of the given end point
        // It is therefore one of the contiguousness points
        po_pContiguousnessPoints->push_back(GetStartPoint());

        // The second point is either one of the other point

        // Check if the other extremity points are equal
        if (GetEndPoint().IsEqualTo(pi_rSegment.GetStartPoint(), Tolerance))
            po_pContiguousnessPoints->push_back(GetEndPoint());
        else
            {
            // Since the those points are not equal, then the one that is on the other segment
            // is the result
            if (IsPointOn(pi_rSegment.GetStartPoint(), HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                po_pContiguousnessPoints->push_back(pi_rSegment.GetStartPoint());
            else
                po_pContiguousnessPoints->push_back(GetEndPoint());
            }
        }
    else if (GetEndPoint().IsEqualTo(pi_rSegment.GetStartPoint(), Tolerance))
        {
        // The self end point in on top of the given start point

        // The second point is either one of the other extremity point
        // the one that is on the other segment is the result
        if (IsPointOn(pi_rSegment.GetEndPoint(), HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
            po_pContiguousnessPoints->push_back(pi_rSegment.GetEndPoint());
        else
            po_pContiguousnessPoints->push_back(GetStartPoint());

        // The end point is therefore one of the contiguousness points
        po_pContiguousnessPoints->push_back(GetEndPoint());

        // The second point is either one of the other point

        }
    else
        {
        // General case, no extremity is on the other
        // Two points are on the other segment and they are the result

        // We start with segment start point
        if (pi_rSegment.IsPointOn(GetStartPoint(), HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
            po_pContiguousnessPoints->push_back(GetStartPoint());

        // Check if the segments are oriented in the same direction
        if (CalculateBearing(GetStartPoint(), HVE2DVector::BETA).IsEqualTo(pi_rSegment.CalculateBearing(pi_rSegment.GetStartPoint(), HVE2DVector::BETA)))
            {
            // The two segments are oriented likewise ...
            // we check in start to end order
            if (IsPointOn(pi_rSegment.GetStartPoint(), HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                po_pContiguousnessPoints->push_back(pi_rSegment.GetStartPoint());
            if (IsPointOn(pi_rSegment.GetEndPoint(), HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                po_pContiguousnessPoints->push_back(pi_rSegment.GetEndPoint());
            }
        else
            {
            // The two segments are oriented differently
            // we check in end to start order
            if (IsPointOn(pi_rSegment.GetEndPoint(), HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                po_pContiguousnessPoints->push_back(pi_rSegment.GetEndPoint());
            if (IsPointOn(pi_rSegment.GetStartPoint(), HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                po_pContiguousnessPoints->push_back(pi_rSegment.GetStartPoint());
            }

        // The last possible point is end point
        if (pi_rSegment.IsPointOn(GetEndPoint(), HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
            po_pContiguousnessPoints->push_back(GetEndPoint());
        }

    return (po_pContiguousnessPoints->size() - InitialNumberOfPoints);
    }


//-----------------------------------------------------------------------------
// AreContiguousAtAndGetWithSegment
// Finds if the segments are contiguous then
// Calculates and returns the contiguousness points with another segment if
// there is any
//-----------------------------------------------------------------------------
bool HVE2DSegment::AreContiguousAtAndGetWithSegment(const HVE2DSegment& pi_rSegment,
                                                     HGF2DLocation* po_pFirstContiguousnessPoint,
                                                     HGF2DLocation* po_pSecondContiguousnessPoint) const
    {
    HPRECONDITION(po_pFirstContiguousnessPoint != 0);
    HPRECONDITION(po_pSecondContiguousnessPoint != 0);


    HGF2DPosition firstPoint;
    HGF2DPosition secondPoint;

    bool answer = GetSegmentPeer().AreContiguousAtAndGetWithSegment(pi_rSegment.GetSegmentPeer(), &firstPoint, &secondPoint);

    *po_pFirstContiguousnessPoint = HGF2DLocation(firstPoint, GetCoordSys());
    *po_pSecondContiguousnessPoint = HGF2DLocation(secondPoint, GetCoordSys());
    return answer;


    }



//-----------------------------------------------------------------------------
// AreContiguousAtAndGetWithSegmentSCS
// Finds if the segments are contiguous then
// Calculates and returns the contiguousness points with another segment if
// there is any
//-----------------------------------------------------------------------------
bool HVE2DSegment::AreContiguousAtAndGetWithSegmentSCS(const HVE2DSegment& pi_rSegment,
                                                        HGF2DLocation* po_pFirstContiguousnessPoint,
                                                        HGF2DLocation* po_pSecondContiguousnessPoint) const
    {
    // The two segments must share the same coordiante system
    HPRECONDITION(GetCoordSys() == pi_rSegment.GetCoordSys());

    HPRECONDITION(po_pFirstContiguousnessPoint != 0);
    HPRECONDITION(po_pSecondContiguousnessPoint != 0);

    HGF2DPosition firstPoint;
    HGF2DPosition secondPoint;
    
    bool returnValue = GetSegmentPeer().AreContiguousAtAndGetWithSegment(pi_rSegment.GetSegmentPeer(), &firstPoint, &secondPoint);
    
    *po_pFirstContiguousnessPoint = HGF2DLocation(firstPoint, GetCoordSys());
    *po_pSecondContiguousnessPoint = HGF2DLocation(secondPoint, GetCoordSys());
    
    return returnValue;
    

    }

//-----------------------------------------------------------------------------
// ObtainContiguousnessPointsWithSegmentSCS
// Calculates and returns the contiguousness points with another segment if
// there is any
//-----------------------------------------------------------------------------
size_t HVE2DSegment::ObtainContiguousnessPointsWithSegmentSCS(const HVE2DSegment& pi_rSegment,
                                                              HGF2DLocationCollection* po_pContiguousnessPoints) const
    {
    // The two segments must have the same coordsys
    HPRECONDITION(GetCoordSys() == pi_rSegment.GetCoordSys());

    HPRECONDITION(po_pContiguousnessPoints != 0);

    // The segments must be contiguous
    HPRECONDITION(AreSegmentsContiguousSCS(pi_rSegment));


    HGF2DPositionCollection tempCollection;
    GetSegmentPeer().ObtainContiguousnessPointsWithSegment(pi_rSegment.GetSegmentPeer(), &tempCollection);
    
    for (auto currentPoint : tempCollection)
        {
        po_pContiguousnessPoints->push_back(HGF2DLocation(currentPoint, GetCoordSys()));
        }
        
    return tempCollection.size();
        

    }








//-----------------------------------------------------------------------------
// CalculateClosestPoint
// Calculates the closest point on segment to given point
//-----------------------------------------------------------------------------
HGF2DLocation HVE2DSegment::CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const
    {
    return HGF2DLocation (GetSegmentPeer().CalculateClosestPoint(pi_rPoint.GetPosition()), GetCoordSys());

    }


//-----------------------------------------------------------------------------
// CalculateLength
// Calculates and returns the length of segment
//-----------------------------------------------------------------------------
double HVE2DSegment::CalculateLength() const
    {
    return GetSegmentPeer().CalculateLength();
    }


//-----------------------------------------------------------------------------
// IsPointOn
// Checks if the point is located on the segment
//-----------------------------------------------------------------------------
bool   HVE2DSegment::IsPointOn(const HGF2DLocation& pi_rTestPoint,
                                HVE2DVector::ExtremityProcessing    pi_ExtremityProcessing,
                                double pi_Tolerance) const
    {
    bool   Answer = false;

    // Check if the two share the same coordinate system
    if (GetCoordSys() == pi_rTestPoint.GetCoordSys())
        Answer = IsPointOnSCS(pi_rTestPoint, pi_ExtremityProcessing, pi_Tolerance);
    else
        Answer = IsPointOnSCS(HGF2DLocation(pi_rTestPoint, GetCoordSys()), pi_ExtremityProcessing, pi_Tolerance);

    return(Answer);
    }

//-----------------------------------------------------------------------------
// IsPointOnSCS
// Checks if the point is located on the segment
//-----------------------------------------------------------------------------
bool   HVE2DSegment::IsPointOnSCS(const HGF2DLocation& pi_rTestPoint,
                                   HVE2DVector::ExtremityProcessing    pi_ExtremityProcessing,
                                   double pi_Tolerance) const
    {
    // The two must share the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rTestPoint.GetCoordSys());

    return GetSegmentPeer().IsPointOn(pi_rTestPoint.GetPosition(), (HGF2DVector::ExtremityProcessing)pi_ExtremityProcessing, pi_Tolerance);


    }



//-----------------------------------------------------------------------------
// IsPointOnLineOnSegment
// Static method
// Checks if the point is located on the segment knowing it is on the line
//-----------------------------------------------------------------------------
bool   HVE2DSegment::IsPointOnLineOnSegment(const HGF2DLocation& pi_rTestPoint,
                                             double pi_Tolerance) const
    {
    HGF2DLocation   MyPoint(pi_rTestPoint, GetCoordSys());
    double X = MyPoint.GetX();
    double Y = MyPoint.GetY();

    // Obtain extremes of segment
    double XMin = MIN(GetStartPoint().GetX(), GetEndPoint().GetX());
    double XMax = MAX(GetStartPoint().GetX(), GetEndPoint().GetX());
    double YMin = MIN(GetStartPoint().GetY(), GetEndPoint().GetY());
    double YMax = MAX(GetStartPoint().GetY(), GetEndPoint().GetY());

    // Obtain tolerance
    double Tolerance = pi_Tolerance;
    if (pi_Tolerance < 0.0)
        Tolerance = GetTolerance();

    // A point is on if it is within the extended extent.
    // No epsilon is applied, therefore a point located within an
    // epsilon of distance of one of the extremities will
    // not be recognized as on segment
    return (HDOUBLE_GREATER_OR_EQUAL(X, XMin, Tolerance) &&
            HDOUBLE_SMALLER_OR_EQUAL(X, XMax, Tolerance) &&
            HDOUBLE_GREATER_OR_EQUAL(Y, YMin, Tolerance) &&
            HDOUBLE_SMALLER_OR_EQUAL(Y, YMax, Tolerance));
    }

//-----------------------------------------------------------------------------
// IsPointOnLineOnSegmentSCS
// Static method
// Checks if the point is located on the segment knowing it is on the line
//-----------------------------------------------------------------------------
bool   HVE2DSegment::IsPointOnLineOnSegmentSCS(const HGF2DLocation& pi_rTestPoint,
                                                double pi_Tolerance) const
    {
    // The given point must be expressed in the same coordinate system as segment
    HPRECONDITION(pi_rTestPoint.GetCoordSys() == GetCoordSys());

    double X = pi_rTestPoint.GetX();
    double Y = pi_rTestPoint.GetY();

    // Obtain extremes of segment
    double XMin = MIN(GetStartPoint().GetX(), GetEndPoint().GetX());
    double XMax = MAX(GetStartPoint().GetX(), GetEndPoint().GetX());
    double YMin = MIN(GetStartPoint().GetY(), GetEndPoint().GetY());
    double YMax = MAX(GetStartPoint().GetY(), GetEndPoint().GetY());

    // Obtain tolerance
    double Tolerance = pi_Tolerance;
    if (pi_Tolerance < 0.0)
        Tolerance = GetTolerance();

    // A point is on if it is within the extended extent.
    return (HDOUBLE_GREATER_OR_EQUAL(X, XMin, Tolerance) &&
            HDOUBLE_SMALLER_OR_EQUAL(X, XMax, Tolerance) &&
            HDOUBLE_GREATER_OR_EQUAL(Y, YMin, Tolerance) &&
            HDOUBLE_SMALLER_OR_EQUAL(Y, YMax, Tolerance));
    }








//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
void HVE2DSegment::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    po_rOutput << "Object is a HVE2DSegment" << endl;
    HDUMP0("Object is a HVE2DSegment\n");

    HVE2DBasicLinear::PrintState(po_rOutput);

#endif
    }




//-----------------------------------------------------------------------------
// AreSegmentsContiguous
// Indicates if the two segments are contiguous
//-----------------------------------------------------------------------------
bool HVE2DSegment::AreSegmentsContiguous(const HVE2DSegment& pi_rSegment) const
    {
    // Check if they share the same coordinate system or they are related
    // through a linearity preserving model
    HPRECONDITION((GetCoordSys() == pi_rSegment.GetCoordSys()) ||
                  (GetCoordSys()->HasLinearityPreservingRelationTo(pi_rSegment.GetCoordSys())));

    HVE2DSegment  TempSegment(pi_rSegment.GetStartPoint().ExpressedIn(GetCoordSys()),
                              pi_rSegment.GetEndPoint().ExpressedIn(GetCoordSys()));

    return(AreSegmentsContiguousSCS(TempSegment));

    }



//-----------------------------------------------------------------------------
// AreSegmentsContiguousSCS
// Indicates if the two segments are contiguous
//-----------------------------------------------------------------------------
bool HVE2DSegment::AreSegmentsContiguousSCS(const HVE2DSegment& pi_rSegment) const
    {
    // The two segments must share the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rSegment.GetCoordSys());

    // None of the segments may be null
    HPRECONDITION(!IsNull() && !pi_rSegment.IsNull());

    return GetSegmentPeer().AreSegmentsContiguous(pi_rSegment.GetSegmentPeer());

    }


