//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hve/src/HVE2DSegment.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HVE2DSegment
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGF2DTransfoModel.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>

#include <Imagepp/all/h/HVE2DSegment.h>

HPM_REGISTER_CLASS(HVE2DSegment, HVE2DLinear)


#include <Imagepp/all/h/HVE2DComplexLinear.h>


//-----------------------------------------------------------------------------
// Rotate
// Rotates the segment
//-----------------------------------------------------------------------------
void HVE2DSegment::Rotate(double               pi_Angle,
                          const HGF2DLocation& pi_rOrigin)
    {
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

    // Adjust tolerance
    ResetTolerance();
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
        if ((!m_StartPoint.IsEqualToSCS(m_EndPoint, GetTolerance())) &&
            (!pTheSegment->m_StartPoint.IsEqualToSCS(pTheSegment->m_EndPoint, GetTolerance())))
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

#if (1)
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
        if ((!m_StartPoint.IsEqualToSCS(m_EndPoint, GetTolerance())) &&
            (!pTheSegment->m_StartPoint.IsEqualToSCS(pTheSegment->m_EndPoint, GetTolerance())))
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


#else

    if (!IsNull() && !pi_rVector.IsNull())
        {
        // Check if the given vector is a segment
        if ((pi_rVector.GetMainVectorType() == HVE2DLinear::CLASS_ID) &&
            (((HVE2DLinear*)(&pi_rVector))->IsABasicLinear()) &&
            (((HVE2DBasicLinear*)(&pi_rVector))->GetBasicLinearType() == HVE2DSegment::CLASS_ID))
            {
            // It is a segment
            // Since both points are located on the segments, then
            // being contiguous is a sufficient condition to insure that they are
            // contiguous AT given point
            Answer = AreContiguous(pi_rVector);
            }
        else
            {
            // The given is not a segment ...
            Answer = pi_rVector.AreContiguousAt(*this, pi_rPoint);
            }
        }


#endif
    return(Answer);
    }


#if (0)
//-----------------------------------------------------------------------------
// Flirts
// Indicates if the segment flirts the given
//-----------------------------------------------------------------------------
bool HVE2DSegment::Flirts(const HVE2DVector& pi_rVector) const
    {
    bool   Answer;

    // Check if the given vector is a segment
    if ((pi_rVector.IsALinear()) &&
        (((HVE2DLinear*)(&pi_rVector))->IsABasicLinear()) &&
        (((HVE2DBasicLinear*)(&pi_rVector))->GetBasicLinearType() == HVE2DSegment::CLASS_ID))
        {
        // It is a segment
        HVE2DSegment* pTheSegment = (HVE2DSegment*)(&pi_rVector);

        // Check if they share the same coordinate system
        if (GetCoordSys() == pMySegment->GetCoordSys())
            {
            // The two segments have the same coordinate system
            Answer = AreSegmentsFlirtingSCS(*pTheSegment);
            }
        else
            {
            // The two segments have different coordinate system

            // We check if it is linearity preserving
            if (GetCoordSys()->HasLinearityPreservingRelationTo(pi_rSegment.GetCoordSys()))
                {
                // They are related through a linearity preserving model ... go ahead
                Answer = AreSegmentsFlirting(*pTheSegment);
                }
            else
                {
                // They are not related by a linearity preserving model ... we must allocate a copy
                // In correct coordinate system
                HVE2DVector* pTempVector = pi_rSegment.AllocateInCoordSys(GetCoordSys());

                // Obtain answer
                Answer = pTempVector->Flirts(*this);

                // Destroy temporary copy
                delete pTempVector;
                }
            }
        }
    else
        {
        // Since it is not a segment, the question is asked to the given
        Answer = pi_rVector.Flirts(*this);
        }

    return(Answer);
    }
#endif


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

#if (0)
            // We check if they are in the proper order
            if (CalculateRelativePosition(*(TempPoints.begin())) < CalculateRelativePosition(*(TempPoints.rbegin())))
                {
                // The points are in the proper order ... we copy
                HGF2DLocationCollection::iterator MyIterator = TempPoints.begin();

                while (MyIterator != TempPoints.end())
                    {
                    po_pContiguousnessPoints->push_back(*MyIterator);

                    MyIterator++;
                    }
                }
            else
                {
                // We add these points in reverse order
                HGF2DLocationCollection::reverse_iterator MyIterator = TempPoints.rbegin();

                while (MyIterator != TempPoints.rend())
                    {
                    po_pContiguousnessPoints->push_back(*MyIterator);

                    MyIterator++;
                    }
                }
#else
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

#endif
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
            pMyResultVector = new HVE2DSegment(m_StartPoint.ExpressedIn(pi_rpCoordSys),
                                               m_EndPoint.ExpressedIn(pi_rpCoordSys));
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
    HGF2DLocation   IntermediatePoint(m_StartPoint + ((m_EndPoint - m_StartPoint) * 0.5));

    // Transform start point, end point and intermediate point
    // This is where the coordinates are effectively transformed!
// HChk AR !!!!!! The complex coordinate system may have domain limitations
//                in such case an error will be thrown ... check if it is here!!!!
    HVE2DSegment    TransformedSegment(m_StartPoint.ExpressedIn(pi_rpCoordSys),
                                       m_EndPoint.ExpressedIn(pi_rpCoordSys));
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
        HVE2DSegment    FirstSegment(m_StartPoint, IntermediatePoint);
        FirstSegment.SetStrokeTolerance(m_pStrokeTolerance);

        FirstSegment.AppendItselfInCoordSys(pio_rResultComplex, pi_rpCoordSys);

        HVE2DSegment    SecondSegment(IntermediatePoint, m_EndPoint);
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
    HGF2DLine::CrossState LineStatus = pi_rLine.IntersectLine(HGF2DLine(m_StartPoint, m_EndPoint), po_pPoint);

    // Check if the lines did cross
    if (LineStatus == HGF2DLine::CROSS_FOUND)
        {
        // The lines did cross .. check if the cross point is part of the segment
        // but not on an extremity
        if (IsPointOnLineOnSegment(*po_pPoint, Tolerance) &&
            (!m_StartPoint.IsEqualTo(*po_pPoint, Tolerance)) &&
            (!m_EndPoint.IsEqualTo(*po_pPoint, Tolerance)))
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
    HGF2DLine::CrossState LineStatus = HGF2DLine(m_StartPoint, m_EndPoint).IntersectLine(HGF2DLine(pi_rSegment.m_StartPoint, pi_rSegment.m_EndPoint), po_pPoint);

    // Check if the lines did cross
    if (LineStatus == HGF2DLine::CROSS_FOUND)
        {
        // The lines did cross .. check if the cross point is part of both segment
        if (IsPointOnLineOnSegment(*po_pPoint, Tolerance) && pi_rSegment.IsPointOn(*po_pPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance) &&
            (!po_pPoint->IsEqualTo(m_StartPoint, Tolerance)) && (!po_pPoint->IsEqualTo(m_EndPoint, Tolerance)) &&
            (!po_pPoint->IsEqualTo(pi_rSegment.m_StartPoint, Tolerance)) && (!po_pPoint->IsEqualTo(pi_rSegment.m_EndPoint, Tolerance)) &&
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

    // Create return object set initialy to no crosses
    HVE2DSegment::CrossState    Status = HVE2DSegment::NO_CROSS;

    // Compute effective tolerance (smallest of the two given segment)
    double Tolerance = MIN(GetTolerance(), pi_rSegment.GetTolerance());

    // Obtain extremes of self segment
    double XMin1 = MIN(m_StartPoint.GetX(), m_EndPoint.GetX());
    double XMax1 = MAX(m_StartPoint.GetX(), m_EndPoint.GetX());
    double YMin1 = MIN(m_StartPoint.GetY(), m_EndPoint.GetY());
    double YMax1 = MAX(m_StartPoint.GetY(), m_EndPoint.GetY());

    // Obtain extremes of given segment
    double XMin2 = MIN(pi_rSegment.m_StartPoint.GetX(), pi_rSegment.m_EndPoint.GetX());
    double XMax2 = MAX(pi_rSegment.m_StartPoint.GetX(), pi_rSegment.m_EndPoint.GetX());
    double YMin2 = MIN(pi_rSegment.m_StartPoint.GetY(), pi_rSegment.m_EndPoint.GetY());
    double YMax2 = MAX(pi_rSegment.m_StartPoint.GetY(), pi_rSegment.m_EndPoint.GetY());


    bool Result = (HDOUBLE_GREATER(XMax1, XMin2, Tolerance) &&
                    HDOUBLE_SMALLER(XMin1, XMax2, Tolerance) &&
                    HDOUBLE_GREATER(YMax1, YMin2, Tolerance) &&
                    HDOUBLE_SMALLER(YMin1, YMax2, Tolerance));

    if (Result)
        {

        HGF2DLine   SelfLine(m_StartPoint, m_EndPoint);
        HGF2DLine   GivenLine(pi_rSegment.m_StartPoint, pi_rSegment.m_EndPoint);


        HGF2DLine::CrossState LineStatus = HGF2DLine::PARALLEL;

        // Check if any of the lines are vertical
        if (SelfLine.IsVertical() || GivenLine.IsVertical())
            {
            // Check if both are vertical ...
            if (SelfLine.IsVertical() && GivenLine.IsVertical())
                {
                // They are both vertical ... parallel
                LineStatus = HGF2DLine::PARALLEL;
                }
            else if (SelfLine.IsVertical() && fabs(GivenLine.GetSlope()) > 1000.0)
                {
                // Self is vertical and given slope is great
                // they cross if segments are not contiguous
                LineStatus = (AreSegmentsContiguousSCS(pi_rSegment) ? HGF2DLine::PARALLEL : HGF2DLine::CROSS_FOUND);
                }
            else if (GivenLine.IsVertical() && fabs(SelfLine.GetSlope()) > 1000.0)
                {
                // Given is vertical and other slope is great
                LineStatus = (AreSegmentsContiguousSCS(pi_rSegment) ? HGF2DLine::PARALLEL : HGF2DLine::CROSS_FOUND);
                }
            else
                {
                // There surely is a cross
                LineStatus = HGF2DLine::CROSS_FOUND;
                }

            // If there is indeed a cross ... obtain it
            if (LineStatus == HGF2DLine::CROSS_FOUND)
                {
                SelfLine.IntersectLine(GivenLine, po_pPoint);
                }
            }
        else if ((fabs(SelfLine.GetSlope()) > 1000.0) && (fabs(GivenLine.GetSlope()) > 1000.0))
            {
            // Both lines are technicaly vertical,
            // Since slope is very big, the computations are highly imprecise
            // We determine if there may be a crossing by coordinate analysis

            // First check if the given coordinates are on opposite sides of
            // line formed by self

            if ((HDOUBLE_GREATER(pi_rSegment.m_StartPoint.GetX(), MIN(m_StartPoint.GetX(), m_EndPoint.GetX()), Tolerance) &&
                 HDOUBLE_SMALLER(pi_rSegment.m_EndPoint.GetX(), MAX(m_StartPoint.GetX(), m_EndPoint.GetX()), Tolerance)) ||
                (HDOUBLE_GREATER(pi_rSegment.m_EndPoint.GetX(), MIN(m_StartPoint.GetX(), m_EndPoint.GetX()), Tolerance) &&
                 HDOUBLE_SMALLER(pi_rSegment.m_StartPoint.GetX(), MAX(m_StartPoint.GetX(), m_EndPoint.GetX()), Tolerance)))
                {
                // Both segments are close to being vertical ...
                LineStatus = (AreSegmentsContiguousSCS(pi_rSegment) ? HGF2DLine::PARALLEL : HGF2DLine::CROSS_FOUND);

                // If there is indeed a cross ... obtain it
                if (LineStatus == HGF2DLine::CROSS_FOUND)
                    {
                    SelfLine.IntersectLine(GivenLine, po_pPoint);
                    }
                }
            }
        else
            {
            // Get intersection point between the lines the segments belong to
            LineStatus = SelfLine.IntersectLine(GivenLine, po_pPoint);
            }

        // Check if it reported parallel
        if ((LineStatus == PARALLEL) && !SelfLine.IsVertical() && !GivenLine.IsVertical())
            {
            // There are some cases where the lines report parallel,
            // but they are not ... notably when slope is extremely small
            // But the slope are not perfectly equal
            // NO EPSILON APLLICATION HERE
//            if ((fabs(SelfLine.GetSlope()) < 1E-3) && (SelfLine.GetSlope() != GivenLine.GetSlope()))
            if ((SelfLine.GetSlope() != GivenLine.GetSlope()))
                {
                // The slopes are indeed very small ... may yet cross


                // First we check if they are contiguous (Higher precision operation)
                if (!AreSegmentsContiguousSCS(pi_rSegment))
                    {
                    // We nevertheless set returned to PARALLEL
                    Status = PARALLEL;

                    // Since they are not contiguous ... they may yet be crossing
                    // Check if one of the slope is equal to 0.0 (Exactly [NO EPSILON])
                    if (SelfLine.GetSlope() == 0.0)
                        {
                        // Self is an horizontal line (perfectly horizontal)
                        // Since slope is very small, the computations are highly imprecise
                        // We determine if there may be a crossing by coordinate analysis

                        // First check if the given coordinates are on opposite sides of
                        // line formed by self

                        if ((HDOUBLE_GREATER(pi_rSegment.m_StartPoint.GetY(), m_StartPoint.GetY(), Tolerance) &&
                             HDOUBLE_SMALLER(pi_rSegment.m_EndPoint.GetY(), m_StartPoint.GetY(), Tolerance)) ||
                            (HDOUBLE_GREATER(pi_rSegment.m_EndPoint.GetY(), m_StartPoint.GetY(), Tolerance) &&
                             HDOUBLE_SMALLER(pi_rSegment.m_StartPoint.GetY(), m_StartPoint.GetY(), Tolerance)))
                            {
                            // The Y coordinate are on opposite sides of self Y coordinate
                            // We assume a cross exists
                            LineStatus = HGF2DLine::CROSS_FOUND;

                            // Obtain coordinate of given on Y of self
                            po_pPoint->SetY(m_StartPoint.GetY());

                            // Since slope is very small, the computations are highly imprecise
                            // We may not compute the X coordinate precisely using slope

                            // Obtain X by calculation
                            po_pPoint->SetX((po_pPoint->GetY() - GivenLine.GetIntercept()) / GivenLine.GetSlope());
                            }
                        }
                    else if (GivenLine.GetSlope() == 0.0)
                        {
                        // Given is an horizontal line (perfectly horizontal)
                        // Since slope is very small, the computations are highly imprecise
                        // We determine if there may be a crossing by coordinate analysis

                        // First check if the given coordinates are on opposite sides of
                        // line formed by self

                        if ((HDOUBLE_GREATER(m_StartPoint.GetY(), pi_rSegment.m_StartPoint.GetY(), Tolerance) &&
                             HDOUBLE_SMALLER(m_EndPoint.GetY(), pi_rSegment.m_StartPoint.GetY(), Tolerance)) ||
                            (HDOUBLE_GREATER(m_EndPoint.GetY(), pi_rSegment.m_StartPoint.GetY(), Tolerance) &&
                             HDOUBLE_SMALLER(m_StartPoint.GetY(), pi_rSegment.m_StartPoint.GetY(), Tolerance)))
                            {
                            // The Y coordinate are on opposite sides of self Y coordinate
                            // We assume a cross exists
                            LineStatus = HGF2DLine::CROSS_FOUND;

                            // Obtain coordinate of given on Y of given
                            po_pPoint->SetY(pi_rSegment.m_StartPoint.GetY());

                            // Obtain X by calculation
                            po_pPoint->SetX((po_pPoint->GetY() - SelfLine.GetIntercept()) / SelfLine.GetSlope());
                            }
                        }
                    else
                        {
                        // Both lines are technicaly horizontal,
                        // Since slope is very small, the computations are highly imprecise
                        // We determine if there may be a crossing by coordinate analysis

                        // First check if the given coordinates are on opposite sides of
                        // line formed by self

                        if ((HDOUBLE_GREATER(pi_rSegment.m_StartPoint.GetY(), MIN(m_StartPoint.GetY(), m_EndPoint.GetY()), Tolerance) &&
                             HDOUBLE_SMALLER(pi_rSegment.m_EndPoint.GetY(), MAX(m_StartPoint.GetY(), m_EndPoint.GetY()), Tolerance)) ||
                            (HDOUBLE_GREATER(pi_rSegment.m_EndPoint.GetY(), MIN(m_StartPoint.GetY(), m_EndPoint.GetY()), Tolerance) &&
                             HDOUBLE_SMALLER(pi_rSegment.m_StartPoint.GetY(), MAX(m_StartPoint.GetY(), m_EndPoint.GetY()), Tolerance)))
                            {

                            // We assume a cross exists
                            LineStatus = HGF2DLine::CROSS_FOUND;

                            // Calculate cross point as if they did cross
                            po_pPoint->SetX((GivenLine.GetIntercept() - SelfLine.GetIntercept()) /
                                            (SelfLine.GetSlope() - GivenLine.GetSlope()));

                            //  if one of the line is quasi-vertical, the computation may become very
                            //  unstable, so we use the line with the smallest slope (absolute)
                            if (fabs(SelfLine.GetSlope()) < fabs(GivenLine.GetSlope()))
                                po_pPoint->SetY((SelfLine.GetSlope() * po_pPoint->GetX()) + SelfLine.GetIntercept());
                            else
                                po_pPoint->SetY((GivenLine.GetSlope() * po_pPoint->GetX()) + GivenLine.GetIntercept());
                            }
                        }
                    }
                }
            }

        double X = po_pPoint->GetX();
        double Y = po_pPoint->GetY();

        // Check if the lines did cross
        if (LineStatus == HGF2DLine::CROSS_FOUND)
            {
            if ((HDOUBLE_GREATER_OR_EQUAL(X, XMin1, Tolerance) &&
                 HDOUBLE_SMALLER_OR_EQUAL(X, XMax1, Tolerance) &&
                 HDOUBLE_GREATER_OR_EQUAL(Y, YMin1, Tolerance) &&
                 HDOUBLE_SMALLER_OR_EQUAL(Y, YMax1, Tolerance)) &&
                (HDOUBLE_GREATER_OR_EQUAL(X, XMin2, Tolerance) &&
                 HDOUBLE_SMALLER_OR_EQUAL(X, XMax2, Tolerance) &&
                 HDOUBLE_GREATER_OR_EQUAL(Y, YMin2, Tolerance) &&
                 HDOUBLE_SMALLER_OR_EQUAL(Y, YMax2, Tolerance)))
                {

                // The point is part of both segment ... a cross has been found
                Status = HVE2DSegment::CROSS_FOUND;

                if (NULL != po_pIntersectsAtExtremity)
                    {
                    if ((!po_pPoint->IsEqualToSCS(m_StartPoint, Tolerance)) && (!po_pPoint->IsEqualToSCS(m_EndPoint, Tolerance)) &&
                        (!po_pPoint->IsEqualToSCS(pi_rSegment.m_StartPoint, Tolerance)) && (!po_pPoint->IsEqualToSCS(pi_rSegment.m_EndPoint, Tolerance)) &&
                        (!LinksTo(pi_rSegment)))
                        {
                        *po_pIntersectsAtExtremity = false;
                        }
                    else
                        {
                        *po_pIntersectsAtExtremity = true;
                        }
                    }
                }
            }
        // In the case no cross was found, find if this is because the segments are parallel
        else if (LineStatus == HGF2DLine::PARALLEL)
            // The segments are parallel to each other
            Status = HVE2DSegment::PARALLEL;
        }

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
    if (m_StartPoint.IsEqualTo(pi_rSegment.m_StartPoint, Tolerance))
        {
        // The two start points are on top of each other
        // It is therefore one of the contiguousness points
        po_pContiguousnessPoints->push_back(m_StartPoint);

        // The second point is either one of the end points

        // Check if the end points are equal
        if (m_EndPoint.IsEqualTo(pi_rSegment.m_EndPoint, Tolerance))
            po_pContiguousnessPoints->push_back(m_EndPoint);
        else
            {
            // Since the end points are not equal, then the one that is on the other segment
            // is the result
            if (IsPointOn(pi_rSegment.m_EndPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                po_pContiguousnessPoints->push_back(pi_rSegment.m_EndPoint);
            else
                po_pContiguousnessPoints->push_back(m_EndPoint);
            }
        }
    else if (m_EndPoint.IsEqualTo(pi_rSegment.m_EndPoint, Tolerance))
        {
        // The two end points are on top of each other

        // The other point is either one of the start point
        // the one that is on the other segment is the result
        if (IsPointOn(pi_rSegment.m_StartPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
            po_pContiguousnessPoints->push_back(pi_rSegment.m_StartPoint);
        else
            po_pContiguousnessPoints->push_back(m_StartPoint);

        // The end point is therefore one of the contiguousness points
        po_pContiguousnessPoints->push_back(m_EndPoint);

        }
    else if (m_StartPoint.IsEqualTo(pi_rSegment.m_EndPoint, Tolerance))
        {
        // The self start point in on top of the given end point
        // It is therefore one of the contiguousness points
        po_pContiguousnessPoints->push_back(m_StartPoint);

        // The second point is either one of the other point

        // Check if the other extremity points are equal
        if (m_EndPoint.IsEqualTo(pi_rSegment.m_StartPoint, Tolerance))
            po_pContiguousnessPoints->push_back(m_EndPoint);
        else
            {
            // Since the those points are not equal, then the one that is on the other segment
            // is the result
            if (IsPointOn(pi_rSegment.m_StartPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                po_pContiguousnessPoints->push_back(pi_rSegment.m_StartPoint);
            else
                po_pContiguousnessPoints->push_back(m_EndPoint);
            }
        }
    else if (m_EndPoint.IsEqualTo(pi_rSegment.m_StartPoint, Tolerance))
        {
        // The self end point in on top of the given start point

        // The second point is either one of the other extremity point
        // the one that is on the other segment is the result
        if (IsPointOn(pi_rSegment.m_EndPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
            po_pContiguousnessPoints->push_back(pi_rSegment.m_EndPoint);
        else
            po_pContiguousnessPoints->push_back(m_StartPoint);

        // The end point is therefore one of the contiguousness points
        po_pContiguousnessPoints->push_back(m_EndPoint);

        // The second point is either one of the other point

        }
    else
        {
        // General case, no extremity is on the other
        // Two points are on the other segment and they are the result

        // We start with segment start point
        if (pi_rSegment.IsPointOn(m_StartPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
            po_pContiguousnessPoints->push_back(m_StartPoint);

        // Check if the segments are oriented in the same direction
        if (CalculateBearing(m_StartPoint, HVE2DVector::BETA).IsEqualTo(pi_rSegment.CalculateBearing(pi_rSegment.m_StartPoint, HVE2DVector::BETA)))
            {
            // The two segments are oriented likewise ...
            // we check in start to end order
            if (IsPointOn(pi_rSegment.m_StartPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                po_pContiguousnessPoints->push_back(pi_rSegment.m_StartPoint);
            if (IsPointOn(pi_rSegment.m_EndPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                po_pContiguousnessPoints->push_back(pi_rSegment.m_EndPoint);
            }
        else
            {
            // The two segments are oriented differently
            // we check in end to start order
            if (IsPointOn(pi_rSegment.m_EndPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                po_pContiguousnessPoints->push_back(pi_rSegment.m_EndPoint);
            if (IsPointOn(pi_rSegment.m_StartPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                po_pContiguousnessPoints->push_back(pi_rSegment.m_StartPoint);
            }

        // The last possible point is end point
        if (pi_rSegment.IsPointOn(m_EndPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
            po_pContiguousnessPoints->push_back(m_EndPoint);
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

    // When contiguous together, two segments will have exactly 2
    // contiguousness points

    // Save initial number of points in list
    size_t  NumberOfPoints = 0;

    // pre-calculate tolerance
    double Tolerance = MIN(pi_rSegment.GetTolerance(), GetTolerance());

    // We check if the start points are equal
    if (m_StartPoint.IsEqualTo(pi_rSegment.m_StartPoint, Tolerance))
        {
        // The two start points are on top of each other
        // It is therefore one of the contiguousness points
        NumberOfPoints++;

        // The second point is either one of the end points

        // Check if the end points are equal
        if (m_EndPoint.IsEqualTo(pi_rSegment.m_EndPoint, Tolerance))
            {
            // It is a second contiguousness point
            NumberOfPoints++;
            *po_pFirstContiguousnessPoint = m_StartPoint;
            *po_pSecondContiguousnessPoint = m_EndPoint;
            }
        else
            {
            // Since the end points are not equal, then the one that is on the other segment
            // is the result
            if (IsPointOn(pi_rSegment.m_EndPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                {
                NumberOfPoints++;
                *po_pFirstContiguousnessPoint = m_StartPoint;
                *po_pSecondContiguousnessPoint = pi_rSegment.m_EndPoint;
                }
            else if(pi_rSegment.IsPointOn(m_EndPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                {
                NumberOfPoints++;
                *po_pFirstContiguousnessPoint = m_StartPoint;
                *po_pSecondContiguousnessPoint = m_EndPoint;
                }
            }
        }
    else if (m_EndPoint.IsEqualTo(pi_rSegment.m_EndPoint, Tolerance))
        {
        // The two end points are on top of each other

        NumberOfPoints++;

        // The other point is either one of the start point
        // the one that is on the other segment is the result
        if (IsPointOn(pi_rSegment.m_StartPoint))
            {
            NumberOfPoints++;
            *po_pFirstContiguousnessPoint = pi_rSegment.m_StartPoint;
            *po_pSecondContiguousnessPoint = m_EndPoint;
            }
        else if (pi_rSegment.IsPointOn(m_StartPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
            {
            NumberOfPoints++;
            *po_pFirstContiguousnessPoint = m_StartPoint;
            *po_pSecondContiguousnessPoint = m_EndPoint;
            }
        }
    else if (m_StartPoint.IsEqualTo(pi_rSegment.m_EndPoint, Tolerance))
        {
        // The self start point in on top of the given end point
        // It is therefore one of the contiguousness points
        NumberOfPoints++;

        // The second point is either one of the other point

        // Check if the other extremity points are equal
        if (m_EndPoint.IsEqualTo(pi_rSegment.m_StartPoint, Tolerance))
            {
            NumberOfPoints++;
            *po_pFirstContiguousnessPoint = m_StartPoint;
            *po_pSecondContiguousnessPoint = m_EndPoint;
            }
        else
            {
            // Since the those points are not equal, then the one that is on the other segment
            // is the result
            if (IsPointOn(pi_rSegment.m_StartPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                {
                NumberOfPoints++;
                *po_pFirstContiguousnessPoint = m_StartPoint;
                *po_pSecondContiguousnessPoint = pi_rSegment.m_StartPoint;
                }
            else if (pi_rSegment.IsPointOn(m_EndPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                {
                NumberOfPoints++;
                *po_pFirstContiguousnessPoint = m_StartPoint;
                *po_pSecondContiguousnessPoint = m_EndPoint;
                }
            }
        }
    else if (m_EndPoint.IsEqualTo(pi_rSegment.m_StartPoint, Tolerance))
        {
        // The self end point in on top of the given start point
        NumberOfPoints++;

        // The second point is either one of the other extremity point
        // the one that is on the other segment is the result
        if (IsPointOn(pi_rSegment.m_EndPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
            {
            NumberOfPoints++;
            *po_pFirstContiguousnessPoint = pi_rSegment.m_EndPoint;
            *po_pSecondContiguousnessPoint = m_EndPoint;
            }
        else if (pi_rSegment.IsPointOn(m_StartPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
            {
            NumberOfPoints++;
            *po_pFirstContiguousnessPoint = m_StartPoint;
            *po_pSecondContiguousnessPoint = m_EndPoint;
            }
        }
    else
        {
        // General case, no extremity is on the other
        // Two points are on the other segment and they are the result

        // We start with segment start point
        if (pi_rSegment.IsPointOn(m_StartPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
            {
            NumberOfPoints++;
            *po_pFirstContiguousnessPoint = m_StartPoint;
            }

        // Check if the segments are oriented in the same direction
        if (CalculateBearing(m_StartPoint, HVE2DVector::BETA).IsEqualTo(pi_rSegment.CalculateBearing(pi_rSegment.m_StartPoint, HVE2DVector::BETA)))
            {
            // The two segments are oriented likewise ...
            // we check in start to end order
            if (IsPointOn(pi_rSegment.m_StartPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                {
                NumberOfPoints++;

                if (NumberOfPoints == 1)
                    *po_pFirstContiguousnessPoint = pi_rSegment.m_StartPoint;
                else
                    *po_pSecondContiguousnessPoint = pi_rSegment.m_StartPoint;
                }
            if (IsPointOn(pi_rSegment.m_EndPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                {
                NumberOfPoints++;

                if (NumberOfPoints == 1)
                    *po_pFirstContiguousnessPoint = pi_rSegment.m_EndPoint;
                else
                    *po_pSecondContiguousnessPoint = pi_rSegment.m_EndPoint;
                }
            }
        else
            {
            // The two segments are oriented differently
            // we check in end to start order
            if (IsPointOn(pi_rSegment.m_EndPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                {
                NumberOfPoints++;

                if (NumberOfPoints == 1)
                    *po_pFirstContiguousnessPoint = pi_rSegment.m_EndPoint;
                else
                    *po_pSecondContiguousnessPoint = pi_rSegment.m_EndPoint;
                }
            if (IsPointOn(pi_rSegment.m_StartPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                {
                NumberOfPoints++;

                if (NumberOfPoints == 1)
                    *po_pFirstContiguousnessPoint = pi_rSegment.m_StartPoint;
                else
                    *po_pSecondContiguousnessPoint = pi_rSegment.m_StartPoint;
                }
            }

        // The last possible point is end point
        if (pi_rSegment.IsPointOn(m_EndPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
            {
            NumberOfPoints++;

            if (NumberOfPoints > 1)
                *po_pSecondContiguousnessPoint = m_EndPoint;
            }
        }


    return (NumberOfPoints == 2);
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

    // When contiguous together, two segments will have exactly 2
    // contiguousness points

    // Save initial number of points in list
    size_t  NumberOfPoints = 0;

    // pre-calculate tolerance
    double Tolerance = MIN(pi_rSegment.GetTolerance(), GetTolerance());

    // We check if the start points are equal
    if (m_StartPoint.IsEqualToSCS(pi_rSegment.m_StartPoint, Tolerance))
        {
        // The two start points are on top of each other
        // It is therefore one of the contiguousness points
        NumberOfPoints++;

        // The second point is either one of the end points

        // Check if the end points are equal
        if (m_EndPoint.IsEqualToSCS(pi_rSegment.m_EndPoint, Tolerance))
            {
            // It is a second contiguousness point
            NumberOfPoints++;
            *po_pFirstContiguousnessPoint = m_StartPoint;
            *po_pSecondContiguousnessPoint = m_EndPoint;
            }
        else
            {
            // Since the end points are not equal, then the one that is on the other segment
            // is the result
            if (IsPointOnSCS(pi_rSegment.m_EndPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                {
                NumberOfPoints++;
                *po_pFirstContiguousnessPoint = m_StartPoint;
                *po_pSecondContiguousnessPoint = pi_rSegment.m_EndPoint;
                }
            else if(pi_rSegment.IsPointOnSCS(m_EndPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                {
                NumberOfPoints++;
                *po_pFirstContiguousnessPoint = m_StartPoint;
                *po_pSecondContiguousnessPoint = m_EndPoint;
                }
            }
        }
    else if (m_EndPoint.IsEqualToSCS(pi_rSegment.m_EndPoint, Tolerance))
        {
        // The two end points are on top of each other

        NumberOfPoints++;

        // The other point is either one of the start point
        // the one that is on the other segment is the result
        if (IsPointOnSCS(pi_rSegment.m_StartPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
            {
            NumberOfPoints++;
            *po_pFirstContiguousnessPoint = pi_rSegment.m_StartPoint;
            *po_pSecondContiguousnessPoint = m_EndPoint;
            }
        else if (pi_rSegment.IsPointOnSCS(m_StartPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
            {
            NumberOfPoints++;
            *po_pFirstContiguousnessPoint = m_StartPoint;
            *po_pSecondContiguousnessPoint = m_EndPoint;
            }
        }
    else if (m_StartPoint.IsEqualToSCS(pi_rSegment.m_EndPoint, Tolerance))
        {
        // The self start point in on top of the given end point
        // It is therefore one of the contiguousness points
        NumberOfPoints++;

        // The second point is either one of the other point

        // Check if the other extremity points are equal
        if (m_EndPoint.IsEqualToSCS(pi_rSegment.m_StartPoint, Tolerance))
            {
            NumberOfPoints++;
            *po_pFirstContiguousnessPoint = m_StartPoint;
            *po_pSecondContiguousnessPoint = m_EndPoint;
            }
        else
            {
            // Since the those points are not equal, then the one that is on the other segment
            // is the result
            if (IsPointOnSCS(pi_rSegment.m_StartPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                {
                NumberOfPoints++;
                *po_pFirstContiguousnessPoint = m_StartPoint;
                *po_pSecondContiguousnessPoint = pi_rSegment.m_StartPoint;
                }
            else if (pi_rSegment.IsPointOnSCS(m_EndPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                {
                NumberOfPoints++;
                *po_pFirstContiguousnessPoint = m_StartPoint;
                *po_pSecondContiguousnessPoint = m_EndPoint;
                }
            }
        }
    else if (m_EndPoint.IsEqualToSCS(pi_rSegment.m_StartPoint, Tolerance))
        {
        // The self end point in on top of the given start point
        NumberOfPoints++;

        // The second point is either one of the other extremity point
        // the one that is on the other segment is the result
        if (IsPointOn(pi_rSegment.m_EndPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
            {
            NumberOfPoints++;
            *po_pFirstContiguousnessPoint = pi_rSegment.m_EndPoint;
            *po_pSecondContiguousnessPoint = m_EndPoint;
            }
        else if (pi_rSegment.IsPointOnSCS(m_StartPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
            {
            NumberOfPoints++;
            *po_pFirstContiguousnessPoint = m_StartPoint;
            *po_pSecondContiguousnessPoint = m_EndPoint;
            }
        }
    else
        {
        // General case, no extremity is on the other
        // Two points are on the other segment and they are the result

        // We start with segment start point
        if (pi_rSegment.IsPointOnSCS(m_StartPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
            {
            NumberOfPoints++;
            *po_pFirstContiguousnessPoint = m_StartPoint;
            }

        // Check if the segments are oriented in the same direction
        if (CalculateBearing(m_StartPoint, HVE2DVector::BETA).IsEqualTo(pi_rSegment.CalculateBearing(pi_rSegment.m_StartPoint, HVE2DVector::BETA)))
            {
            // The two segments are oriented likewise ...
            // we check in start to end order
            if (IsPointOnSCS(pi_rSegment.m_StartPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                {
                NumberOfPoints++;

                if (NumberOfPoints == 1)
                    *po_pFirstContiguousnessPoint = pi_rSegment.m_StartPoint;
                else
                    *po_pSecondContiguousnessPoint = pi_rSegment.m_StartPoint;
                }
            if (IsPointOnSCS(pi_rSegment.m_EndPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                {
                NumberOfPoints++;

                if (NumberOfPoints == 1)
                    *po_pFirstContiguousnessPoint = pi_rSegment.m_EndPoint;
                else
                    *po_pSecondContiguousnessPoint = pi_rSegment.m_EndPoint;
                }
            }
        else
            {
            // The two segments are oriented differently
            // we check in end to start order
            if (IsPointOnSCS(pi_rSegment.m_EndPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                {
                NumberOfPoints++;

                if (NumberOfPoints == 1)
                    *po_pFirstContiguousnessPoint = pi_rSegment.m_EndPoint;
                else
                    *po_pSecondContiguousnessPoint = pi_rSegment.m_EndPoint;
                }
            if (IsPointOnSCS(pi_rSegment.m_StartPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                {
                NumberOfPoints++;

                if (NumberOfPoints == 1)
                    *po_pFirstContiguousnessPoint = pi_rSegment.m_StartPoint;
                else
                    *po_pSecondContiguousnessPoint = pi_rSegment.m_StartPoint;
                }
            }

        // The last possible point is end point
        if (pi_rSegment.IsPointOnSCS(m_EndPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
            {
            NumberOfPoints++;

            if (NumberOfPoints > 1)
                *po_pSecondContiguousnessPoint = m_EndPoint;
            }
        }


    return (NumberOfPoints == 2);
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

    // When contiguous together, two segments will have exactly 2
    // contiguousness points

    // Save initial number of points in list
    size_t  InitialNumberOfPoints = po_pContiguousnessPoints->size();

    // Pre-calculate tolerance
    double Tolerance =  MIN(pi_rSegment.GetTolerance(), GetTolerance());

    // We check if the start points are equal
    if (m_StartPoint.IsEqualToSCS(pi_rSegment.m_StartPoint, Tolerance))
        {
        // The two start points are on top of each other
        // It is therefore one of the contiguousness points
        po_pContiguousnessPoints->push_back(m_StartPoint);

        // The second point is either one of the end points

        // Check if the end points are equal
        if (m_EndPoint.IsEqualToSCS(pi_rSegment.m_EndPoint, Tolerance))
            po_pContiguousnessPoints->push_back(m_EndPoint);
        else
            {
            // Since the end points are not equal, then the one that is on the other segment
            // is the result
            if (IsPointOnSCS(pi_rSegment.m_EndPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                po_pContiguousnessPoints->push_back(pi_rSegment.m_EndPoint);
            else
                po_pContiguousnessPoints->push_back(m_EndPoint);
            }
        }
    else if (m_EndPoint.IsEqualToSCS(pi_rSegment.m_EndPoint, Tolerance))
        {
        // The two end points are on top of each other

        // The other point is either one of the start point
        // the one that is on the other segment is the result
        if (IsPointOnSCS(pi_rSegment.m_StartPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
            po_pContiguousnessPoints->push_back(pi_rSegment.m_StartPoint);
        else
            po_pContiguousnessPoints->push_back(m_StartPoint);

        // The end point is therefore one of the contiguousness points
        po_pContiguousnessPoints->push_back(m_EndPoint);

        }
    else if (m_StartPoint.IsEqualToSCS(pi_rSegment.m_EndPoint, Tolerance))
        {
        // The self start point in on top of the given end point
        // It is therefore one of the contiguousness points
        po_pContiguousnessPoints->push_back(m_StartPoint);

        // The second point is either one of the other point

        // Check if the other extremity points are equal
        if (m_EndPoint.IsEqualToSCS(pi_rSegment.m_StartPoint, Tolerance))
            po_pContiguousnessPoints->push_back(m_EndPoint);
        else
            {
            // Since the those points are not equal, then the one that is on the other segment
            // is the result
            if (IsPointOnSCS(pi_rSegment.m_StartPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                po_pContiguousnessPoints->push_back(pi_rSegment.m_StartPoint);
            else
                po_pContiguousnessPoints->push_back(m_EndPoint);
            }
        }
    else if (m_EndPoint.IsEqualToSCS(pi_rSegment.m_StartPoint, Tolerance))
        {
        // The self end point in on top of the given start point

        // The second point is either one of the other extremity point
        // the one that is on the other segment is the result
        if (IsPointOnSCS(pi_rSegment.m_EndPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
            po_pContiguousnessPoints->push_back(pi_rSegment.m_EndPoint);
        else
            po_pContiguousnessPoints->push_back(m_StartPoint);

        // The end point is therefore one of the contiguousness points
        po_pContiguousnessPoints->push_back(m_EndPoint);

        // The second point is either one of the other point

        }
    else
        {
        // General case, no extremity is on the other
        // Two points are on the other segment and they are the result

        // We start with segment start point
        if (pi_rSegment.IsPointOnSCS(m_StartPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
            po_pContiguousnessPoints->push_back(m_StartPoint);

        // Check if the segments are oriented in the same direction
        if (CalculateBearing(m_StartPoint, HVE2DVector::BETA).IsEqualTo(pi_rSegment.CalculateBearing(pi_rSegment.m_StartPoint, HVE2DVector::BETA)))
            {
            // The two segments are oriented likewise ...
            // we check in start to end order
            if (IsPointOnSCS(pi_rSegment.m_StartPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                po_pContiguousnessPoints->push_back(pi_rSegment.m_StartPoint);
            if (IsPointOnSCS(pi_rSegment.m_EndPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                po_pContiguousnessPoints->push_back(pi_rSegment.m_EndPoint);
            }
        else
            {
            // The two segments are oriented differently
            // we check in end to start order
            if (IsPointOnSCS(pi_rSegment.m_EndPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                po_pContiguousnessPoints->push_back(pi_rSegment.m_EndPoint);
            if (IsPointOnSCS(pi_rSegment.m_StartPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                po_pContiguousnessPoints->push_back(pi_rSegment.m_StartPoint);
            }

        // The last possible point is end point
        if (pi_rSegment.IsPointOnSCS(m_EndPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
            po_pContiguousnessPoints->push_back(m_EndPoint);
        }

    return (po_pContiguousnessPoints->size() - InitialNumberOfPoints);
    }








//-----------------------------------------------------------------------------
// CalculateClosestPoint
// Calculates the closest point on segment to given point
//-----------------------------------------------------------------------------
HGF2DLocation HVE2DSegment::CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const
    {
    // Calculate closest point to line the segment is part of
    HGF2DLocation   ClosestPoint(HGF2DLine(m_StartPoint, m_EndPoint).CalculateClosestPoint(pi_rPoint));

    // Check if this point is not located on the segment
    if (!IsPointOnLineOnSegmentSCS(ClosestPoint))
        {
        // Calculate distances from extremities to point
        double FromStart((m_StartPoint - pi_rPoint).CalculateLength());
        double FromEnd((m_EndPoint - pi_rPoint).CalculateLength());

        // Check if distances are equal
        // KEEP AS OPERATOR==
        if (FromStart == FromEnd)
            {
            // The distances are identical ... may be a precision problem ...
            // Check if length of segment is zero
            if (CalculateLength() == 0.0)
                {
                // We have a null segment ... any point will do
                ClosestPoint =  m_StartPoint;
                }
            else
                {
                // We have a precision problem ...
                // We know that the tentative closest point is very far from either start
                // or end point, and is aligned to it ... we use bearings if not infinity
                if (!BeNumerical::BeFinite(FromStart))
                    ClosestPoint = m_StartPoint;
                else
                    {
                    HGFBearing   FromStartBearing((m_StartPoint - ClosestPoint).CalculateBearing());
                    HGFBearing   SegmentBearing((m_EndPoint - m_StartPoint).CalculateBearing());

                    ClosestPoint = ((FromStartBearing.IsEqualTo(SegmentBearing)) ? m_StartPoint : m_EndPoint);
                    }
                }
            }
        else
            {
            // Since the closest point on line is not located on segment, then the
            // closest point is the closest of the start point or the end point.
            ClosestPoint = ((FromStart < FromEnd) ? m_StartPoint : m_EndPoint);
            }
        }

    return (ClosestPoint);
    }


//-----------------------------------------------------------------------------
// CalculateLength
// Calculates and returns the length of segment
//-----------------------------------------------------------------------------
double HVE2DSegment::CalculateLength() const
    {
    double DeltaX(m_EndPoint.GetX()-m_StartPoint.GetX());
    double DeltaY(m_EndPoint.GetY()-m_StartPoint.GetY());

    return sqrt((DeltaX * DeltaX) + (DeltaY * DeltaY));
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

    // To check if a point is on segment, it must first be included in the outter
    // extent of segment.
    // If this is true, then the distance from point to the line
    // the segment is part of must be smaller than tolerance
    // The computation of distance are base on a mathematical equation
    // Found in "Formules et tables de mathematique"
    // Experience has proven that this equation is far more
    // precise than any other one studied
    bool   Answer = false;

    // Obtain tolerance
    double Tolerance = pi_Tolerance;

    if (Tolerance < 0.0)
        Tolerance = GetTolerance();

    // Obtain extremes of segment
    double XMin = MIN(m_StartPoint.GetX(), m_EndPoint.GetX());
    double XMax = MAX(m_StartPoint.GetX(), m_EndPoint.GetX());
    double YMin = MIN(m_StartPoint.GetY(), m_EndPoint.GetY());
    double YMax = MAX(m_StartPoint.GetY(), m_EndPoint.GetY());

    double X = pi_rTestPoint.GetX();
    double Y = pi_rTestPoint.GetY();

    // A point is on if it is within the extended extent.
    if (HDOUBLE_GREATER_OR_EQUAL(X, XMin, Tolerance) &&
        HDOUBLE_SMALLER_OR_EQUAL(X, XMax, Tolerance) &&
        HDOUBLE_GREATER_OR_EQUAL(Y, YMin, Tolerance) &&
        HDOUBLE_SMALLER_OR_EQUAL(Y, YMax, Tolerance))
        {
        // Check if line is vertical or horizontal
        // The non-application of tolerance is volontary
        if ((XMin == XMax) || (YMin == YMax))
            {
            // Segment is vertical or horizontal... therefore the point is automatically ON
            // Since segment in in extent
            Answer = true;
            }
        else
            {
            // Non vertical and non horizontal segment
            // Obtain slope and intercept of line
            double SelfSlope;

            SelfSlope = ((m_StartPoint.GetY() - m_EndPoint.GetY())) /
                        (m_StartPoint.GetX() - m_EndPoint.GetX());
            double SelfIntercept = m_StartPoint.GetY() - (m_StartPoint.GetX() * SelfSlope);

            // Calculate distance from point to line
            double DistanceToLine = (SelfSlope * pi_rTestPoint.GetX() -
                                          pi_rTestPoint.GetY() + SelfIntercept) /
                                         sqrt(SelfSlope * SelfSlope + 1);

            // Check if distance is smaller than epsilon.
            // Being within the extent and closer than epsilon of line, the
            // point will be ON
            Answer = HDOUBLE_EQUAL(DistanceToLine, 0.0, Tolerance);
            }

        // Check if extremities must be excluded
        if ((Answer) && (pi_ExtremityProcessing == HVE2DVector::EXCLUDE_EXTREMITIES))
            {
            // The caller wishes to exclude extremities from operation
            // We check it is different from extremity
            Answer = (!m_StartPoint.IsEqualToSCS(pi_rTestPoint, Tolerance) &&
                      !m_EndPoint.IsEqualToSCS(pi_rTestPoint, Tolerance));
            }

        if ((!Answer) && (pi_ExtremityProcessing == HVE2DVector::INCLUDE_EXTREMITIES))
            {
            Answer = (m_StartPoint.IsEqualToSCS(pi_rTestPoint, Tolerance) ||
                      m_EndPoint.IsEqualToSCS(pi_rTestPoint, Tolerance));
            }


        }

    return(Answer);
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
    double XMin = MIN(m_StartPoint.GetX(), m_EndPoint.GetX());
    double XMax = MAX(m_StartPoint.GetX(), m_EndPoint.GetX());
    double YMin = MIN(m_StartPoint.GetY(), m_EndPoint.GetY());
    double YMax = MAX(m_StartPoint.GetY(), m_EndPoint.GetY());

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
    double XMin = MIN(m_StartPoint.GetX(), m_EndPoint.GetX());
    double XMax = MAX(m_StartPoint.GetX(), m_EndPoint.GetX());
    double YMin = MIN(m_StartPoint.GetY(), m_EndPoint.GetY());
    double YMax = MAX(m_StartPoint.GetY(), m_EndPoint.GetY());

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

    HVE2DSegment  TempSegment(pi_rSegment.m_StartPoint.ExpressedIn(GetCoordSys()),
                              pi_rSegment.m_EndPoint.ExpressedIn(GetCoordSys()));

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

    size_t  NumberOfContiguousnessPoints = 0;

    // Pre-calculate tolerance
    double Tolerance = MIN(GetTolerance(), pi_rSegment.GetTolerance());

    // Obtain extremes of segment
    double SelfXMin = MIN(m_StartPoint.GetX(), m_EndPoint.GetX());
    double SelfXMax = MAX(m_StartPoint.GetX(), m_EndPoint.GetX());
    double SelfYMin = MIN(m_StartPoint.GetY(), m_EndPoint.GetY());
    double SelfYMax = MAX(m_StartPoint.GetY(), m_EndPoint.GetY());

    double GivenXMin = MIN(pi_rSegment.m_StartPoint.GetX(), pi_rSegment.m_EndPoint.GetX());
    double GivenXMax = MAX(pi_rSegment.m_StartPoint.GetX(), pi_rSegment.m_EndPoint.GetX());
    double GivenYMin = MIN(pi_rSegment.m_StartPoint.GetY(), pi_rSegment.m_EndPoint.GetY());
    double GivenYMax = MAX(pi_rSegment.m_StartPoint.GetY(), pi_rSegment.m_EndPoint.GetY());


    bool Result = (HDOUBLE_GREATER_OR_EQUAL(SelfXMax, GivenXMin, Tolerance) &&
                    HDOUBLE_SMALLER_OR_EQUAL(SelfXMin, GivenXMax, Tolerance) &&
                    HDOUBLE_GREATER_OR_EQUAL(SelfYMax, GivenYMin, Tolerance) &&
                    HDOUBLE_SMALLER_OR_EQUAL(SelfYMin, GivenYMax, Tolerance)
                   );


    if (Result)
        {


        // We check if the start points are equal
        if (m_StartPoint.IsEqualToSCS(pi_rSegment.m_StartPoint, Tolerance))
            {
            // The two start points are on top of each other
            // It is therefore one of the contiguousness points
            NumberOfContiguousnessPoints++;

            // The second point is either one of the end points

            // Check if the end points are equal
            if (m_EndPoint.IsEqualToSCS(pi_rSegment.m_EndPoint, Tolerance))
                NumberOfContiguousnessPoints++;
            else
                {
                // Since the end points are not equal, then the one that is on the other segment
                // is the result
                if (IsPointOnSCS(pi_rSegment.m_EndPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                    NumberOfContiguousnessPoints++;
                else if(pi_rSegment.IsPointOnSCS(m_EndPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                    NumberOfContiguousnessPoints++;
                }
            }
        else if (m_EndPoint.IsEqualToSCS(pi_rSegment.m_EndPoint, Tolerance))
            {
            // The two end points are on top of each other

            // The other point is either one of the start point
            // the one that is on the other segment is the result
            if (IsPointOnSCS(pi_rSegment.m_StartPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                NumberOfContiguousnessPoints++;
            else if (pi_rSegment.IsPointOnSCS(m_StartPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                NumberOfContiguousnessPoints++;

            // The end point is therefore one of the contiguousness points
            NumberOfContiguousnessPoints++;

            }
        else if (m_StartPoint.IsEqualToSCS(pi_rSegment.m_EndPoint, Tolerance))
            {
            // The self start point in on top of the given end point
            // It is therefore one of the contiguousness points
            NumberOfContiguousnessPoints++;

            // The second point is either one of the other point

            // Check if the other extremity points are equal
            if (m_EndPoint.IsEqualToSCS(pi_rSegment.m_StartPoint, Tolerance))
                NumberOfContiguousnessPoints++;
            else
                {
                // Since the those points are not equal, then the one that is on the other segment
                // is the result
                if (IsPointOnSCS(pi_rSegment.m_StartPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                    NumberOfContiguousnessPoints++;
                else if (pi_rSegment.IsPointOnSCS(m_EndPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                    NumberOfContiguousnessPoints++;
                }
            }
        else if (m_EndPoint.IsEqualToSCS(pi_rSegment.m_StartPoint, Tolerance))
            {
            // The self end point in on top of the given start point

            // The second point is either one of the other extremity point
            // the one that is on the other segment is the result
            if (IsPointOnSCS(pi_rSegment.m_EndPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                NumberOfContiguousnessPoints++;
            else if (pi_rSegment.IsPointOnSCS(m_StartPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                NumberOfContiguousnessPoints++;

            // The end point is therefore one of the contiguousness points
            NumberOfContiguousnessPoints++;

            // The second point is either one of the other point

            }
        else
            {
            // General case, no extremity is on the other
            // Two points are on the other segment and they are the result

            // We start with segment start point
            if (pi_rSegment.IsPointOnSCS(m_StartPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                NumberOfContiguousnessPoints++;

            // The two segments are oriented likewise ...
            // we check in start to end order
            if (IsPointOnSCS(pi_rSegment.m_StartPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                NumberOfContiguousnessPoints++;
            if (IsPointOnSCS(pi_rSegment.m_EndPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                NumberOfContiguousnessPoints++;

            // The last possible point is end point
            if (pi_rSegment.IsPointOnSCS(m_EndPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                NumberOfContiguousnessPoints++;
            }
        }


    return(NumberOfContiguousnessPoints == 2);
    }


