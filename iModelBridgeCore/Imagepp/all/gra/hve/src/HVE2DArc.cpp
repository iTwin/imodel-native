//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hve/src/HVE2DArc.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HVE2DArc
//-------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGF2DTransfoModel.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>

#include <Imagepp/all/h/HVE2DArc.h>
#include <Imagepp/all/h/HGF2DLine.h>
#include <Imagepp/all/h/HVE2DSegment.h>

HPM_REGISTER_CLASS(HVE2DArc, HVE2DBasicLinear)


/** -----------------------------------------------------------------------------
    Creates an arc of circle based on a start point, end point and third point.

    The three points may not be co-linear since this would result in a circle
    of infinite radius. The third point belongs on the arc and indicates both the
    curvature of circle and direction of rotation.
    The interpretation coordinate system is the one of the start point.

    @param pi_rStartPoint The location of the start point of the arc.

    @param pi_rMiddlePoint A point located upon the arc in between the start
                           and end point.

    @param pi_rEndPoint The location of the end point of the arc.

    @see HGF2DLocation
    -----------------------------------------------------------------------------
*/
HVE2DArc::HVE2DArc(const HGF2DLocation& pi_rStartPoint,
                   const HGF2DLocation& pi_rMiddlePoint,
                   const HGF2DLocation& pi_rEndPoint)
    : HVE2DBasicLinear(pi_rStartPoint, pi_rEndPoint),
      m_Center(pi_rStartPoint)
    {
    SetByPoints(pi_rStartPoint, pi_rMiddlePoint, pi_rEndPoint);
    }

/** -----------------------------------------------------------------------------
    Creates an arc of circle based on a center point, radius, start bearing
    end sweep. The start and end points are computed
    from these settings.
    The radius may not be null nor negative.
    The interpretation coordinate system is obtained from the center point.

    @param pi_rCenter The location of the center point of the circle to
                      which the constructed arc belongs to.

    @param pi_rStartBearing The bearing from center point to start point.

    @param pi_Sweep  The sweep of the arc. A negative value indicates
                       a clockwise rotation.

    @param pi_rRadius  The radius of the circle the arc belongs to.

    @see HGF2DLocation
    @see HGFBearing
    -----------------------------------------------------------------------------
*/
HVE2DArc::HVE2DArc(const HGF2DLocation& pi_rCenter,
                   const HGFBearing&    pi_rStartBearing,
                   double               pi_Sweep,
                   double               pi_Radius)
    : HVE2DBasicLinear(pi_rCenter.GetCoordSys()),
      m_Center(pi_rCenter)

    {
    // Radius may not be 0 or negative
    HPRECONDITION(pi_Radius > 0.0);

    // Sweep must be smaller than 360 degrees
    HPRECONDITION(fabs(pi_Sweep) < 2*PI);

    // Calculate start point
    m_StartPoint = m_Center + HGF2DDisplacement(pi_rStartBearing, pi_Radius);

    // Calculate end point
    m_EndPoint = m_Center + HGF2DDisplacement(pi_rStartBearing + pi_Sweep, pi_Radius);

    if (pi_Sweep > 0.0)
        m_RotationDirection = HGFAngle::CCW;
    else
        m_RotationDirection = HGFAngle::CW;
    }

//-----------------------------------------------------------------------------
// GetExtent
// Returns the extent of the arc
//-----------------------------------------------------------------------------
HGF2DExtent HVE2DArc::GetExtent() const
    {
    // Obtain sweep of arc and start bearing
    double    Sweep = CalculateSweep();
    HGFBearing  StartBearing = CalculateStartBearing();

    // Create an empty extent
    HGF2DExtent TheArcExtent(GetCoordSys());

    // Add extremity points to extent
    TheArcExtent.Add(m_StartPoint);
    TheArcExtent.Add(m_EndPoint);

    // The extent may be greater ...
    // We must extend from radius in the 4 orthogonal directions
    // (North, East, West and South) a point, and if this point is part of the arc
    // add it to extent

    // Calculate radius
    double     Radius = (m_Center - m_StartPoint).CalculateLength();

    // Process the north point
    HGFBearing      NorthBearing(PI/2);

    // Check if north is in arc
    if (StartBearing.IsBearingWithinSweep(Sweep, NorthBearing))
        {
        // Add north point
        TheArcExtent.Add(m_Center + HGF2DDisplacement(NorthBearing, Radius));
        }

    // Process the south point
    HGFBearing      SouthBearing(3 * PI / 2);

    // Check if south is in arc
    if (StartBearing.IsBearingWithinSweep(Sweep, SouthBearing))
        {
        // Add south point
        TheArcExtent.Add(m_Center + HGF2DDisplacement(SouthBearing, Radius));
        }

    // Process the east point
    HGFBearing      EastBearing(0.0);

    // Check if south is in arc
    if (StartBearing.IsBearingWithinSweep(Sweep, EastBearing))
        {
        // Add south point
        TheArcExtent.Add(m_Center + HGF2DDisplacement(EastBearing, Radius));
        }

    // Process the west point
    HGFBearing      WestBearing(PI);

    // Check if south is in arc
    if (StartBearing.IsBearingWithinSweep(Sweep, WestBearing))
        {
        // Add south point
        TheArcExtent.Add(m_Center + HGF2DDisplacement(WestBearing, Radius));
        }

    return(TheArcExtent);
    }

//-----------------------------------------------------------------------------
// Rotate
// Rotates the arc by specified angle around origin
//-----------------------------------------------------------------------------
void HVE2DArc::Rotate(double               pi_Angle,
                      const HGF2DLocation& pi_rOrigin)
    {
    // Create a location in current coordinate system
    HGF2DLocation  Origin(pi_rOrigin, GetCoordSys());

    // Create a similitude
    HGF2DSimilitude Similitude();

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

    // Set new start point
    m_EndPoint.SetX(NewX);
    m_EndPoint.SetY(NewY);

    // Transform coordinates of center point
    NewX = m_Center.GetX();
    NewY = m_Center.GetY();
    Similitude.ConvertDirect(&NewX, &NewY);

    // Set new center point
    m_Center.SetX(NewX);
    m_Center.SetY(NewY);

    // Adjust tolerance
//HChkAR
//    ResetTolerance();

    }


//-----------------------------------------------------------------------------
// SetCoordSysImplementation
//-----------------------------------------------------------------------------
void HVE2DArc::SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    {
    // Call ancester set
    HVE2DBasicLinear::SetCoordSysImplementation(pi_rpCoordSys);

    // Set the coordinate system of center
    m_Center.SetCoordSys(GetCoordSys());
    }


//-----------------------------------------------------------------------------
// CalculateRelativePosition
// Calculates and returns the relative position of given location on arc.
//-----------------------------------------------------------------------------
double HVE2DArc::CalculateRelativePosition(const HGF2DLocation& pi_rPointOnLinear) const
    {
    // The given point must be located on arc
    HPRECONDITION(IsPointOn(pi_rPointOnLinear));

    // The arc must not be null
    HPRECONDITION(!IsNull());

    // The relative position is distance ratio between angle to given and to end point
    // from start point

    HGF2DDisplacement  FromCenterToStart(m_StartPoint - m_Center);
    HGF2DDisplacement  FromCenterToEnd(m_EndPoint - m_Center);

    // Compute full angle sweep
    double  Sweep;
    Sweep = (m_RotationDirection == HGFAngle::CCW ?
             (FromCenterToEnd.CalculateBearing() -
              FromCenterToStart.CalculateBearing()) :
             -(FromCenterToStart.CalculateBearing() -
               FromCenterToEnd.CalculateBearing()));

    // Compute sweep to given point
    HGF2DDisplacement  FromCenterToGiven(pi_rPointOnLinear - m_Center);
    double  PartialSweep;
    PartialSweep = (m_RotationDirection == HGFAngle::CCW ?
                    (FromCenterToGiven.CalculateBearing() -
                     FromCenterToStart.CalculateBearing()) :
                    -(FromCenterToStart.CalculateBearing() -
                      FromCenterToGiven.CalculateBearing()));

    // Relative position is the ratio of the two
    return(PartialSweep / Sweep);
    }


/** -----------------------------------------------------------------------------
    Sets the arc of circle based on a start point, end point and third point.

    The three points may not be co-linear since this would result in a circle
    of infinite radius. The third point belongs on the arc and indicates both the
    curvature of circle and direction of rotation.
    The interpretation coordinate system remains unchanged.

    @param pi_rStartPoint The location of the start point of the arc.

    @param pi_rMiddlePoint A point located upon the arc in between the start
                           and end point.

    @param pi_rEndPoint The location of the end point of the arc.

    @see HGF2DLocation
    -----------------------------------------------------------------------------
*/
void HVE2DArc::SetByPoints(const HGF2DLocation& pi_rStartPoint,
                           const HGF2DLocation& pi_rMiddlePoint,
                           const HGF2DLocation& pi_rEndPoint)
    {
    // The middle point must not be on the same line as extremity points
    // since this would result in an infinite arc
    HPRECONDITION((pi_rStartPoint - pi_rEndPoint).CalculateBearing() !=
                  (pi_rStartPoint - pi_rMiddlePoint).CalculateBearing());

    // Define rigth angle for calculation
    double RightAngle = PI / 2.0;

    // Set extremity points
    m_StartPoint = pi_rStartPoint.ExpressedIn(GetCoordSys());
    m_EndPoint = pi_rEndPoint.ExpressedIn(GetCoordSys());

    HGF2DLocation   MiddlePoint(pi_rMiddlePoint, GetCoordSys());

    // Obtain end point in coordsys of start point
    HGF2DLocation   EndPoint(pi_rEndPoint, pi_rStartPoint.GetCoordSys());

    // The position of radius is found from third point

    // We first obtain intermediate points to arc startpoint->middlepoint
    HGF2DDisplacement   FromStartToMiddle(MiddlePoint - pi_rStartPoint);
    HGF2DLocation    Inter1 = pi_rStartPoint + FromStartToMiddle / 2;

    // Create first line
    HGF2DLine   FirstLine(Inter1, FromStartToMiddle.CalculateBearing() + RightAngle);

    // We first obtain intermediate points to arc middlepoint->endpoint (in same CS)
    HGF2DDisplacement   FromMiddleToEnd(EndPoint - MiddlePoint);
    HGF2DLocation    Inter2(pi_rMiddlePoint + FromMiddleToEnd / 2,
                            pi_rStartPoint.GetCoordSys());

    // Create second line
    HGF2DLine   SecondLine(Inter2, FromMiddleToEnd.CalculateBearing() + RightAngle);

    // The center is obtained by intersection of the two lines
    HGF2DLine::CrossState Status = FirstLine.IntersectLine(SecondLine, &m_Center);

    // The lines must have crossed
    HASSERT(Status == HGF2DLine::CROSS_FOUND);

    // Now we need the rotation direction

    // We compute angle differences (sweep) from start to middle, then start to end
    double  DiffAngle1 = (MiddlePoint - m_Center).CalculateBearing() -
                           (m_StartPoint - m_Center).CalculateBearing();
    double  DiffAngle2 = (EndPoint - m_Center).CalculateBearing() -
                           (m_StartPoint - m_Center).CalculateBearing();

    // If sweep from start to middle is smaller than total arc sweep
    if (CalculateNormalizedTrigoValue(DiffAngle1) < CalculateNormalizedTrigoValue(DiffAngle2))
        // The rotation direction is the one already calculate in start to middle sweep
        m_RotationDirection = ((DiffAngle1 > 0) ? HGFAngle::CW : HGFAngle::CCW);
    else
        {
        // Since partial sweep is greater than total,
        // the direction is reverse of diffangle 1
        if (DiffAngle1 >= 0)
            m_RotationDirection = HGFAngle::CW;
        else
            m_RotationDirection = HGFAngle::CCW;
        }
    }




//-----------------------------------------------------------------------------
// AreContiguous
// Indicates if the linear is contiguous to the given
//-----------------------------------------------------------------------------
bool HVE2DArc::AreContiguous(const HVE2DVector& pi_rVector) const
    {
    bool   Answer = false;

    // No need to check if either vector is null
    if (!IsNull() && !pi_rVector.IsNull())
        {
        // Check if the given vector is a basic linear
        if ((pi_rVector.GetMainVectorType() == HVE2DLinear::CLASS_ID) &&
            (((HVE2DLinear*)(&pi_rVector))->IsABasicLinear()))
            {
            // Check if basic linear is an arc
            if (((HVE2DBasicLinear*)(&pi_rVector))->GetBasicLinearType() == HVE2DArc::CLASS_ID)
                {
                // It is an arc
                HVE2DArc* pTheArc = (HVE2DArc*)(&pi_rVector);

                // Check if they share the same coordinate system
                // or if it is shape preserving
                if ((GetCoordSys() == pTheArc->GetCoordSys()) ||
                    (GetCoordSys()->HasShapePreservingRelationTo(pTheArc->GetCoordSys())))
                    {
                    // The two arcs have the same coordinate system or
                    // are properly related ... call arc specific method
                    Answer = AreArcsContiguous(*pTheArc);
                    }
                else
                    {
                    // They are not related by a Shape preserving model ... we must allocate a copy
                    // in corect coordinate system
                    HVE2DVector* pTempVector = pTheArc->AllocateCopyInCoordSys(GetCoordSys());

                    // Obtain answer
                    Answer = pTempVector->AreContiguous(*this);

                    // Destroy temporary copy
                    delete pTempVector;

                    }
                }
            // Check if the basic linear is a segment
            else if (((HVE2DBasicLinear*)(&pi_rVector))->GetBasicLinearType() ==
                     HVE2DSegment::CLASS_ID)
                {
                // It is a segment

                // We check if it is not linearity preserving
                // if linearity is preserved they may not ne contiguous
                if (!GetCoordSys()->HasLinearityPreservingRelationTo(pi_rVector.GetCoordSys()))
                    {
                    // They are not related by a linearity preserving model ...
                    // we must allocate a copy in corect coordinate system
                    HVE2DVector* pTempVector = pi_rVector.AllocateCopyInCoordSys(GetCoordSys());

                    // Obtain answer
                    Answer = pTempVector->AreContiguous(*this);

                    // Destroy temporary copy
                    delete pTempVector;
                    }
                }
            else
                {
                // Since it is not an arc nor a segment, the question is asked to the given
                Answer = pi_rVector.AreContiguous(*this);
                }
            }
        else
            {
            // Since it is not a basic linear, the question is asked to the given
            Answer = pi_rVector.AreContiguous(*this);
            }
        }

    return(Answer);
    }


//-----------------------------------------------------------------------------
// AreContiguousAt
// Indicates if the linear is contiguous to the given at given point
//-----------------------------------------------------------------------------
bool HVE2DArc::AreContiguousAt(const HVE2DVector& pi_rVector,
                                const HGF2DLocation& pi_rPoint) const
    {
    // The given point must be located on both vectors
    HPRECONDITION(IsPointOn(pi_rPoint) && pi_rVector.IsPointOn(pi_rPoint));

    bool   Answer = false;

    // No need to check if vectors are null since point is located on vectors

    // Check if the given vector is an arc
    if ((pi_rVector.GetMainVectorType() == HVE2DLinear::CLASS_ID) &&
        (((HVE2DLinear*)(&pi_rVector))->IsABasicLinear()) &&
        (((HVE2DBasicLinear*)(&pi_rVector))->GetBasicLinearType() == HVE2DArc::CLASS_ID))
        {
        // It is an arc
        // Since both points are located on the arcs, then
        // being contiguous is a sufficient condition to insure that they are
        // contiguous AT given point
        Answer = AreContiguous(pi_rVector);
        }
    // Check if the given vector is a segment
    else if ((pi_rVector.GetMainVectorType() == HVE2DLinear::CLASS_ID) &&
             (((HVE2DLinear*)(&pi_rVector))->IsABasicLinear()) &&
             (((HVE2DBasicLinear*)(&pi_rVector))->GetBasicLinearType() == HVE2DSegment::CLASS_ID))
        {
        // We check if it is not linearity preserving
        // if linearity is preserved they may not be contiguous
        if (!GetCoordSys()->HasLinearityPreservingRelationTo(pi_rVector.GetCoordSys()))
            {
            // They are not related by a linearity preserving model ... we must allocate a copy
            // In correct coordinate system
            HVE2DVector* pTempVector = pi_rVector.AllocateCopyInCoordSys(GetCoordSys());

            // Obtain answer
            Answer = pTempVector->AreContiguous(*this);

            // Destroy temporary copy
            delete pTempVector;
            }
        }
    else
        {
        // The given is not a arc ...
        Answer = pi_rVector.AreContiguousAt(*this, pi_rPoint);
        }


    return(Answer);
    }



//-----------------------------------------------------------------------------
// Crosses
// Indicates if the two linear cross each other
//-----------------------------------------------------------------------------
bool HVE2DArc::Crosses(const HVE2DVector& pi_rVector) const
    {
    bool   Answer;

    // Check if the given vector is a basic linear
    if ((pi_rVector.GetMainVectorType() == HVE2DLinear::CLASS_ID) &&
        (((HVE2DLinear*)(&pi_rVector))->IsABasicLinear()))
        {
        // Check if it is an arc
        if (((HVE2DBasicLinear*)(&pi_rVector))->GetBasicLinearType() == HVE2DArc::CLASS_ID)
            {
            // It is an arc
            HVE2DArc* pTheArc = (HVE2DArc*)(&pi_rVector);

            // Check if they share the same coordinate system
            // or if it is shape preserving
            if ((GetCoordSys() == pTheArc->GetCoordSys()) ||
                (GetCoordSys()->HasShapePreservingRelationTo(pTheArc->GetCoordSys())))
                {
                // The two arcs have the same coordinate system or
                // have shape preserving relation
                Answer = AreArcsCrossing(*pTheArc);
                }
            else
                {
                // They are not related by a Shape preserving model ... we must allocate a copy
                // In correct coordinate system
                HVE2DVector* pTempVector = pTheArc->AllocateCopyInCoordSys(GetCoordSys());

                // Obtain answer
                Answer = pTempVector->Crosses(*this);

                // Destroy temporary copy
                delete pTempVector;
                }
            }
        // Check if it is a segment
        else if (((HVE2DBasicLinear*)(&pi_rVector))->GetBasicLinearType() ==
                 HVE2DSegment::CLASS_ID)
            {
            // It is an segment
            HVE2DSegment* pTheSegment = (HVE2DSegment*)(&pi_rVector);

            // Check if they share the same coordinate system
            // or if it is linearity preserving
            if ((GetCoordSys() == pTheSegment->GetCoordSys()) ||
                (GetCoordSys()->HasLinearityPreservingRelationTo(pTheSegment->GetCoordSys())))
                {
                // The two arcs have the same coordinate system
                Answer = AreArcAndSegmentCrossing(*pTheSegment);
                }
            else
                {
                // They are not related by a linearity preserving model ...
                // we must allocate a copy in correct coordinate system
                HVE2DVector* pTempVector = pTheSegment->AllocateCopyInCoordSys(GetCoordSys());

                // Obtain answer
                Answer = pTempVector->Crosses(*this);

                // Destroy temporary copy
                delete pTempVector;
                }
            }
        else
            {
            // Since it is not an arc nor a segment, the question is asked to the given
            Answer = pi_rVector.Crosses(*this);
            }
        }
    else
        {
        // Since it is not a basic linear, the question is asked to the given
        Answer = pi_rVector.Crosses(*this);
        }

    return(Answer);
    }


//-----------------------------------------------------------------------------
// AreAdjacent
// Indicates if the linear is adjacent to the given
//-----------------------------------------------------------------------------
bool HVE2DArc::AreAdjacent(const HVE2DVector& pi_rVector) const
    {
    bool   Answer;

    // Check if the given vector is a basic linear
    if ((pi_rVector.GetMainVectorType() == HVE2DLinear::CLASS_ID) &&
        (((HVE2DLinear*)(&pi_rVector))->IsABasicLinear()))
        {
        // Check if it is an arc
        if (((HVE2DBasicLinear*)(&pi_rVector))->GetBasicLinearType() == HVE2DArc::CLASS_ID)
            {
            // It is an arc
            HVE2DArc* pTheArc = (HVE2DArc*)(&pi_rVector);

            // Check if they share the same coordinate system
            // or if shape preserving relation
            if ((GetCoordSys() == pTheArc->GetCoordSys()) ||
                (GetCoordSys()->HasShapePreservingRelationTo(pTheArc->GetCoordSys())))
                {
                // The two arcs have the same coordinate system
                // or are properly related
                Answer = AreArcsAdjacent(*pTheArc);
                }
            else
                {
                // They are not related by a Shape preserving model ... we must allocate a copy
                // In correct coordinate system
                HVE2DVector* pTempVector = pTheArc->AllocateCopyInCoordSys(GetCoordSys());

                // Obtain answer
                Answer = pTempVector->AreAdjacent(*this);

                // Destroy temporary copy
                delete pTempVector;
                }
            }
        // Check if it is a segment
        else if (((HVE2DBasicLinear*)(&pi_rVector))->GetBasicLinearType() ==
                 HVE2DSegment::CLASS_ID)
            {
            // It is an segment
            HVE2DSegment* pTheSegment = (HVE2DSegment*)(&pi_rVector);

            // Check if they share the same coordinate system
            // or if linearity preserving relation
            if ((GetCoordSys() == pTheSegment->GetCoordSys()) ||
                (GetCoordSys()->HasLinearityPreservingRelationTo(pTheSegment->GetCoordSys())))
                {
                // The two arcs have the same coordinate system
                Answer = AreArcAndSegmentAdjacent(*pTheSegment);
                }
            else
                {
                // They are not related by a linearity preserving model ...
                // we must allocate a copy in correct coordinate system
                HVE2DVector* pTempVector = pTheSegment->AllocateCopyInCoordSys(GetCoordSys());

                // Obtain answer
                Answer = pTempVector->AreAdjacent(*this);

                // Destroy temporary copy
                delete pTempVector;
                }
            }
        else
            {
            // Since it is not an arc nor a segment, the question is asked to the given
            Answer = pi_rVector.AreAdjacent(*this);
            }
        }
    else
        {
        // Since it is not a basic linear, the question is asked to the given
        Answer = pi_rVector.AreAdjacent(*this);
        }

    return(Answer);
    }

//-----------------------------------------------------------------------------
// Intersect
// Finds intersection point with vector
//-----------------------------------------------------------------------------
size_t HVE2DArc::Intersect(const HVE2DVector& pi_rVector,
                           HGF2DLocationCollection* po_pCrossPoints) const
    {
    HPRECONDITION(po_pCrossPoints != NULL);

    size_t  NumberOfCrossPoints = 0;

    // Check if the given is an arc
    if (pi_rVector.GetMainVectorType() == HVE2DLinear::CLASS_ID &&
        (*(HVE2DLinear*)(&pi_rVector)).IsABasicLinear() &&
        ((*(HVE2DBasicLinear*)(&pi_rVector)).GetBasicLinearType() == HVE2DArc::CLASS_ID))
        {
        // The given vector is a arc
        HVE2DArc*   pMyArc = (HVE2DArc*)(&pi_rVector);

        // We create a recipient location
        HGF2DLocation   MyCrossPoint1(GetCoordSys());
        HGF2DLocation   MyCrossPoint2(GetCoordSys());

        // Check if they share the same coordinate system or have
        // a shape preserving relation
        if ((GetCoordSys() == pMyArc->GetCoordSys()) ||
            (GetCoordSys()->HasShapePreservingRelationTo(pMyArc->GetCoordSys())))
            {
            // The two arcs have the same coordinate system or are properly related
            // obtain intersection point for two arcs
            if ((NumberOfCrossPoints = IntersectArc(*pMyArc, &MyCrossPoint1, &MyCrossPoint2)) > 0)
                {
                // They did cross ... transfer cross points to collection
                po_pCrossPoints->push_back(MyCrossPoint1);

                if (NumberOfCrossPoints > 1)
                    po_pCrossPoints->push_back(MyCrossPoint2);
                }
            }
        else
            {
            // They are not related by a shape preserving model ... we must allocate a copy
            // In correct coordinate system
            HVE2DVector* pTempVector = pMyArc->AllocateCopyInCoordSys(GetCoordSys());

            // Intersect
            pTempVector->Intersect(*this, po_pCrossPoints);

            // Destroy temporary copy
            delete pTempVector;
            }
        }
    // Check if the given is a segment
    else if (pi_rVector.GetMainVectorType() == HVE2DLinear::CLASS_ID &&
             (*(HVE2DLinear*)(&pi_rVector)).IsABasicLinear() &&
             ((*(HVE2DBasicLinear*)(&pi_rVector)).GetBasicLinearType() == HVE2DSegment::CLASS_ID))
        {
        // The given vector is a arc
        HVE2DSegment*   pMySegment = (HVE2DSegment*)(&pi_rVector);

        // We create a recipient location
        HGF2DLocation   MyCrossPoint1(GetCoordSys());
        HGF2DLocation   MyCrossPoint2(GetCoordSys());

        // Check if they share the same coordinate system or share a
        // linearity preserving relation
        if ((GetCoordSys() == pMySegment->GetCoordSys()) ||
            (GetCoordSys()->HasLinearityPreservingRelationTo(pMySegment->GetCoordSys())))
            {
            // The two vectors have the same coordinate system or are linearity preserving relation
            // Obtain intersection points
            if ((NumberOfCrossPoints = IntersectSegment(*pMySegment,
                                                        &MyCrossPoint1,
                                                        &MyCrossPoint2)) > 0)
                {

                // They did cross ... transfer cross points to collection
                po_pCrossPoints->push_back(MyCrossPoint1);

                if (NumberOfCrossPoints > 1)
                    po_pCrossPoints->push_back(MyCrossPoint2);
                }
            }
        else
            {
            // They are not related by a shape preserving model ... we must allocate a copy
            // In correct coordinate system
            HVE2DVector* pTempVector = pMySegment->AllocateCopyInCoordSys(GetCoordSys());

            // Intersect
            pTempVector->Intersect(*this, po_pCrossPoints);

            // Destroy temporary copy
            delete pTempVector;
            }

        }
    else
        {
        // We have not an arc nor a segment ... we ask the vector to perform the process
        NumberOfCrossPoints = pi_rVector.Intersect(*this, po_pCrossPoints);
        }

    return (NumberOfCrossPoints);
    }

//-----------------------------------------------------------------------------
// ObtainContiguousnessPoints
// Finds contiguousness point with vector
//-----------------------------------------------------------------------------
size_t HVE2DArc::ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                            HGF2DLocationCollection* po_pContiguousnessPoints) const
    {
    HPRECONDITION(po_pContiguousnessPoints != NULL);

    // The two vectors must be contiguous
    HPRECONDITION(AreContiguous(pi_rVector));

    size_t  NumberOfNewPoints;

    // Check if the given is a arc
    if (pi_rVector.GetMainVectorType() == HVE2DLinear::CLASS_ID &&
        (*(HVE2DLinear*)(&pi_rVector)).IsABasicLinear() &&
        ((*(HVE2DBasicLinear*)(&pi_rVector)).GetBasicLinearType() == HVE2DArc::CLASS_ID))
        {
        // The given vector is a arc
        HVE2DArc*   pMyArc = (HVE2DArc*)(&pi_rVector);

        // Check if they share the same coordinate system or are related
        // by a DIRECTION preserving relation
        if ((GetCoordSys() == pMyArc->GetCoordSys()) ||
            (GetCoordSys()->HasDirectionPreservingRelationTo(pMyArc->GetCoordSys())))
            {
            // The two arcs have the same coordinate system or related
            // through a shape preserving model
            // We obtain contiguousness points with arc
            NumberOfNewPoints = ObtainContiguousnessPointsWithArc(*pMyArc,
                                                                  po_pContiguousnessPoints);
            }
        else
            {
            // They are not related by a DIRECTION preserving model ... we must allocate a copy
            // In correct coordinate system
            HVE2DVector* pTempVector = pMyArc->AllocateCopyInCoordSys(GetCoordSys());

            // Obtain answer
            NumberOfNewPoints = ObtainContiguousnessPoints(*pTempVector, po_pContiguousnessPoints);

            // Destroy temporary copy
            delete pTempVector;
            }
        }
    // Check if the given is a segment
    else if (pi_rVector.GetMainVectorType() == HVE2DLinear::CLASS_ID &&
             (*(HVE2DLinear*)(&pi_rVector)).IsABasicLinear() &&
             ((*(HVE2DBasicLinear*)(&pi_rVector)).GetBasicLinearType() == HVE2DSegment::CLASS_ID))
        {
        // They may not be related by a linearity preserving model ... otherwise they would not be
        // contiguous
        HASSERT(!GetCoordSys()->HasLinearityPreservingRelationTo(pi_rVector.GetCoordSys()));

        // we must allocate a copy
        // In correct coordinate system
        HVE2DVector* pTempVector = pi_rVector.AllocateCopyInCoordSys(GetCoordSys());

        // Obtain answer
        NumberOfNewPoints = ObtainContiguousnessPoints(*pTempVector, po_pContiguousnessPoints);

        // Destroy temporary copy
        delete pTempVector;

        }
    else
        {
        HGF2DLocationCollection TempPoints;

        // We have not an arc nor a segment ... we ask the vector to perform the process
        if ((NumberOfNewPoints = pi_rVector.ObtainContiguousnessPoints(*this, &TempPoints)) != 0)
            {
            // There are some contiguousness points ...

            // We check if they are in the proper order
            if (CalculateRelativePosition(*(TempPoints.begin())) <
                CalculateRelativePosition(*(TempPoints.rbegin())))
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
            }
        }

    return (NumberOfNewPoints);
    }


//-----------------------------------------------------------------------------
// ObtainContiguousnessPointsAt
// Finds contiguousness point with vector
//-----------------------------------------------------------------------------
void HVE2DArc::ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
                                            const HGF2DLocation& pi_rPoint,
                                            HGF2DLocation* po_pFirstContiguousnessPoint,
                                            HGF2DLocation* po_pSecondContiguousnessPoint) const
    {
    HPRECONDITION(po_pFirstContiguousnessPoint != NULL);
    HPRECONDITION(po_pSecondContiguousnessPoint != NULL);

    // The vectors must be contiguous at given point
    HPRECONDITION(AreContiguousAt(pi_rVector, pi_rPoint));

    // Check if the given is a arc
    if (pi_rVector.GetMainVectorType() == HVE2DLinear::CLASS_ID &&
        (*(HVE2DLinear*)(&pi_rVector)).IsABasicLinear() &&
        ((*(HVE2DBasicLinear*)(&pi_rVector)).GetBasicLinearType() == HVE2DArc::CLASS_ID))
        {
        // The given vector is a arc
        HVE2DArc*   pMyArc = (HVE2DArc*)(&pi_rVector);

        // Check if they share the same coordinate system or that they
        // are related through a direction preserving relation
        if ((GetCoordSys() == pMyArc->GetCoordSys()) ||
            (GetCoordSys()->HasDirectionPreservingRelationTo(pMyArc->GetCoordSys())))
            {
#if (0)
//            ??????
//            WRONG!

            HGF2DLocationCollection ContiguousnessPoints;

            ObtainContiguousnessPointsWithArc(*pMyArc, &ContiguousnessPoints);

            HASSERT(ContiguousnessPoints.size() == 2);

            *po_pFirstContiguousnessPoint = ContiguousnessPoints[0];
            *po_pSecondContiguousnessPoint = ContiguousnessPoints[1];
#else
            // The two arcs have the same coordinate system or related
            // through a shape preserving model
            // We obtain contiguousness points with arc
            ObtainContiguousnessPointsAtWithArc(*pMyArc,
                                                pi_rPoint,
                                                po_pFirstContiguousnessPoint,
                                                po_pSecondContiguousnessPoint);
#endif
            }
        else
            {

            // They are not related by a direction preserving model ... we must allocate a copy
            // In correct coordinate system
            HVE2DVector* pTempVector = pMyArc->AllocateCopyInCoordSys(GetCoordSys());

            // Obtain answer
            ObtainContiguousnessPointsAt(*pTempVector,
                                         pi_rPoint,
                                         po_pFirstContiguousnessPoint,
                                         po_pSecondContiguousnessPoint);

            // Destroy temporary copy
            delete pTempVector;
            }
        }
    // Check if the given is a segment
    else if (pi_rVector.GetMainVectorType() == HVE2DLinear::CLASS_ID &&
             (*(HVE2DLinear*)(&pi_rVector)).IsABasicLinear() &&
             ((*(HVE2DBasicLinear*)(&pi_rVector)).GetBasicLinearType() == HVE2DSegment::CLASS_ID))
        {
        // They may not related by a linearity preserving model ... otherwise they would not be
        // contiguous
        HASSERT(!GetCoordSys()->HasLinearityPreservingRelationTo(pi_rVector.GetCoordSys()));

        // we must allocate a copy
        // In correct coordinate system
        HVE2DVector* pTempVector = pi_rVector.AllocateCopyInCoordSys(GetCoordSys());

        // Obtain answer
        ObtainContiguousnessPointsAt(*pTempVector,
                                     pi_rPoint,
                                     po_pFirstContiguousnessPoint,
                                     po_pSecondContiguousnessPoint);

        // Destroy temporary copy
        delete pTempVector;
        }
    else
        {
        // We have not an arc nor a circle ... we ask the vector to perform the process
        pi_rVector.ObtainContiguousnessPointsAt(*this,
                                                pi_rPoint,
                                                po_pFirstContiguousnessPoint,
                                                po_pSecondContiguousnessPoint);

        // We check if the two points are in the proper order
        if (CalculateRelativePosition(*po_pFirstContiguousnessPoint) >
            CalculateRelativePosition(*po_pSecondContiguousnessPoint))
            {
            // Not in the proper order ... we swap
            HGF2DLocation   SwapLocation(*po_pFirstContiguousnessPoint);
            *po_pFirstContiguousnessPoint = *po_pSecondContiguousnessPoint;
            *po_pSecondContiguousnessPoint = SwapLocation;
            }
        }

    }


//-----------------------------------------------------------------------------
// AllocateCopyInCoordSys
// Returns a dynamically allocated copy of the arc in a different coordinate
// system
//-----------------------------------------------------------------------------
HVE2DVector* HVE2DArc::AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const
    {
    HVE2DVector*    pMyResultVector;

    // Check if it is the same coordinate system
    if (pi_rpCoordSys == GetCoordSys())
        {
        pMyResultVector = new HVE2DArc(*this);
        }
    else
        {
        // Check if this model is shape preserving
        if (GetCoordSys()->HasShapePreservingRelationTo(pi_rpCoordSys))
            {
            // Calculate a third point
            HGF2DLocation MiddlePoint(CalculateRelativePoint(0.5));

            // The model preserves Shape ... we can transform the points directly
            pMyResultVector = new HVE2DArc(m_StartPoint.ExpressedIn(pi_rpCoordSys),
                                           MiddlePoint,
                                           m_EndPoint.ExpressedIn(pi_rpCoordSys));
            }
        else
            {
            // The model does not preserve Shape
            // We process more completely
            pMyResultVector = AllocateCopyInComplexCoordSys(pi_rpCoordSys);
            }
        }

    return(pMyResultVector);
    }


//-----------------------------------------------------------------------------
// AppendItselfInCoordSys
// Adds to the given complex linear a transformed version of arc
//-----------------------------------------------------------------------------
void HVE2DArc::AppendItselfInCoordSys(HVE2DComplexLinear& pio_rResultComplex,
                                      const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const
    {
    // Obtain the arc intermediate point
    HGF2DLocation   IntermediatePoint(CalculateRelativePoint(0.5));

    // Transform start point, end point and intermediate point
    HVE2DSegment    TransformedSegment(m_StartPoint.ExpressedIn(pi_rpCoordSys),
                                       m_EndPoint.ExpressedIn(pi_rpCoordSys));
    HGF2DLocation   NewIntermediatePoint(IntermediatePoint, pi_rpCoordSys);

    // Check if tolerance is respected

    // Obtain distance from transformed arc
    double TheDistanceFromTransformed;
    TheDistanceFromTransformed = (TransformedSegment.CalculateClosestPoint(NewIntermediatePoint)
                                  - NewIntermediatePoint).CalculateLength();

    if (HDOUBLE_EQUAL(TheDistanceFromTransformed, 0.0, GetTolerance()))
        {
        // The epsilon is respected ... we add this last segment to linear
        pio_rResultComplex.AppendLinear(TransformedSegment);
        }
    else
        {
        // Since the tolerance is not respected, we split the segment into two smaller ones

        // Duplicate arc
        HVE2DArc    FirstArc(*this);

        // Shorten duplicate to intermediate point
        FirstArc.ShortenTo(IntermediatePoint);

        // Append this arc
        FirstArc.AppendItselfInCoordSys(pio_rResultComplex, pi_rpCoordSys);

        // Another duplicate
        HVE2DArc    SecondArc(*this);

        // Shorten it from same point
        SecondArc.ShortenFrom(IntermediatePoint);

        // Append this arc
        SecondArc.AppendItselfInCoordSys(pio_rResultComplex, pi_rpCoordSys);
        }
    }


//-----------------------------------------------------------------------------
// AllocateCopyInComplexCoordSys
// Returns a dynamically allocated copy of the arc transformed according
// in the given coordinate system
//-----------------------------------------------------------------------------
HVE2DVector* HVE2DArc::AllocateCopyInComplexCoordSys(
    const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const
    {
    HVE2DVector*    pResultVector;

    // Allocate complex linear to receive result
    HVE2DComplexLinear*  pNewComplex = new HVE2DComplexLinear(GetCoordSys());

    // Add transformed arc to complex
    AppendItselfInCoordSys(*pNewComplex, pi_rpCoordSys);

    // The result is a complex linear
    pResultVector = pNewComplex;

    return (pResultVector);

    }



//-----------------------------------------------------------------------------
// IntersectLine
// Calculates and returns the intersection point with a line if there is one
//-----------------------------------------------------------------------------
int32_t HVE2DArc::IntersectLine(const HGF2DLine& pi_rLine,
                               HGF2DLocation* po_pFirstPoint,
                               HGF2DLocation* po_pSecondPoint) const
    {
    HPRECONDITION(po_pFirstPoint != NULL);
    HPRECONDITION(po_pSecondPoint != NULL);

    int32_t FinalNumCross = 0;

    // Check if they share the same coordinate system or they are related
    // through a linearity preserving model
    HPRECONDITION((GetCoordSys() == pi_rLine.GetCoordSys()) ||
                  (GetCoordSys()->HasLinearityPreservingRelationTo(pi_rLine.GetCoordSys())));

    HGF2DLocation   FirstCrossPoint(GetCoordSys());
    HGF2DLocation   SecondCrossPoint(GetCoordSys());
    // Get intersection point between the given line and the circle the arc belongs to
    int32_t NumCross = CalculateCircle().IntersectLine(pi_rLine,
                                                      &FirstCrossPoint,
                                                      &SecondCrossPoint);

    // Check if the line and circle did cross
    if (NumCross > 0)
        {
        // The circle and line did cross .. check if the cross point is part of the arc
        // but not on an extremity
        if (IsPointOnCircleOnArc(FirstCrossPoint) &&
            (!m_StartPoint.IsEqualTo(FirstCrossPoint)) &&
            (!m_EndPoint.IsEqualTo(FirstCrossPoint)))
            {
            // The point is part of the arc ... a cross has been found
            *po_pFirstPoint = FirstCrossPoint;
            FinalNumCross++;
            }

        // Check if there was two cross points
        if (NumCross == 2)
            {
            if (IsPointOnCircleOnArc(SecondCrossPoint) &&
                (!m_StartPoint.IsEqualTo(SecondCrossPoint)) &&
                (!m_EndPoint.IsEqualTo(SecondCrossPoint)))
                {
                // The point is part of the arc ... a cross has been found
                if (FinalNumCross > 0)
                    {
                    *po_pSecondPoint = SecondCrossPoint;
                    }
                else
                    {
                    *po_pFirstPoint = SecondCrossPoint;
                    }
                FinalNumCross++;
                }
            }
        }

    return (FinalNumCross);
    }

//-----------------------------------------------------------------------------
// IntersectSegment
// Calculates and returns the intersection points with a segment if there is one
//-----------------------------------------------------------------------------
int32_t HVE2DArc::IntersectSegment(const HVE2DSegment& pi_rSegment,
                                  HGF2DLocation* po_pFirstPoint,
                                  HGF2DLocation* po_pSecondPoint) const
    {
    HPRECONDITION(po_pFirstPoint != NULL);
    HPRECONDITION(po_pSecondPoint != NULL);


    // Check if they share the same coordinate system or they are related
    // through a linearity preserving model
    HPRECONDITION((GetCoordSys() == pi_rSegment.GetCoordSys()) ||
                  (GetCoordSys()->HasLinearityPreservingRelationTo(pi_rSegment.GetCoordSys())));

    int32_t FinalNumCross = 0;

    HGF2DLocation   FirstCrossPoint(GetCoordSys());
    HGF2DLocation   SecondCrossPoint(GetCoordSys());

    // Get intersection point between the given line and the circle the arc belongs to
    int32_t NumCross = IntersectLine(pi_rSegment.CalculateLine(),
                                    &FirstCrossPoint,
                                    &SecondCrossPoint);

    // Check if the line did cross
    if (NumCross > 0)
        {
        // The arc and line did cross .. check if the cross point is part of the segment
        // but not on an extremity
        if (pi_rSegment.IsPointOn(FirstCrossPoint) &&
            (!pi_rSegment.GetStartPoint().IsEqualTo(FirstCrossPoint)) &&
            (!pi_rSegment.GetEndPoint().IsEqualTo(FirstCrossPoint)))
            {
            // The point is part of the segment ... a cross has been found
            *po_pFirstPoint = FirstCrossPoint;
            FinalNumCross++;
            }

        // Check if there was two cross points
        if (NumCross == 2)
            {
            // Check if second point is valid
            if (pi_rSegment.IsPointOn(SecondCrossPoint) &&
                (!pi_rSegment.GetStartPoint().IsEqualTo(SecondCrossPoint)) &&
                (!pi_rSegment.GetEndPoint().IsEqualTo(SecondCrossPoint)))
                {
                // The point is part of the arc ... a cross has been found
                if (FinalNumCross > 0)
                    {
                    *po_pSecondPoint = SecondCrossPoint;
                    }
                else
                    {
                    *po_pFirstPoint = SecondCrossPoint;
                    }
                FinalNumCross++;
                }
            }
        }

    return (FinalNumCross);
    }


//-----------------------------------------------------------------------------
// IntersectArc
// Calculates and returns the intersection point with another arc if there is one
//-----------------------------------------------------------------------------
int32_t HVE2DArc::IntersectArc(const HVE2DArc& pi_rArc,
                              HGF2DLocation* po_pFirstPoint,
                              HGF2DLocation* po_pSecondPoint) const
    {
    HPRECONDITION(po_pFirstPoint != NULL);
    HPRECONDITION(po_pSecondPoint != NULL);

    int32_t FinalNumCross = 0;

    // Check if they share the same coordinate system or they are related
    // through a Shape preserving model
    HPRECONDITION((GetCoordSys() == pi_rArc.GetCoordSys()) ||
                  (GetCoordSys()->HasShapePreservingRelationTo(pi_rArc.GetCoordSys())));

    // Get intersection point between the circles the arcs belong to
    HGF2DLocation   FirstCrossPoint(GetCoordSys());
    HGF2DLocation   SecondCrossPoint(GetCoordSys());
    int32_t NumCross = CalculateCircle().IntersectCircle(pi_rArc.CalculateCircle(),
                                                        &FirstCrossPoint,
                                                        &SecondCrossPoint);

    // Check if the circles did cross
    if (NumCross > 0)
        {
        // If they did cross ... there must be 2 such points
        HASSERT(NumCross == 2);

        // The circle did cross .. check if the cross point is part of both arc
        if (IsPointOnCircleOnArc(FirstCrossPoint) && pi_rArc.IsPointOn(FirstCrossPoint) &&
            (!FirstCrossPoint.IsEqualTo(m_StartPoint)) &&
            (!FirstCrossPoint.IsEqualTo(m_EndPoint)) &&
            (!FirstCrossPoint.IsEqualTo(pi_rArc.m_StartPoint)) &&
            (!FirstCrossPoint.IsEqualTo(pi_rArc.m_EndPoint)))
            {
            // The point is part of both arc ... a cross has been found
            *po_pFirstPoint = FirstCrossPoint;
            FinalNumCross++;
            }

        // Check if second points is part of arcs
        if (IsPointOnCircleOnArc(SecondCrossPoint) && pi_rArc.IsPointOn(SecondCrossPoint) &&
            (!SecondCrossPoint.IsEqualTo(m_StartPoint)) &&
            (!SecondCrossPoint.IsEqualTo(m_EndPoint)) &&
            (!SecondCrossPoint.IsEqualTo(pi_rArc.m_StartPoint)) &&
            (!SecondCrossPoint.IsEqualTo(pi_rArc.m_EndPoint)))
            {
            // The second point is part of both arc ... a cross has been found
            if (FinalNumCross > 0)
                {
                *po_pSecondPoint = SecondCrossPoint;
                }
            else
                {
                *po_pFirstPoint = SecondCrossPoint;
                }
            FinalNumCross++;
            }

        }

    return (FinalNumCross);
    }




//-----------------------------------------------------------------------------
// ObtainContiguousnessPointsWithArc
// Calculates and returns the contiguousness points with another arc if
// there is any
//-----------------------------------------------------------------------------
size_t HVE2DArc::ObtainContiguousnessPointsWithArc(
    const HVE2DArc& pi_rArc,
    HGF2DLocationCollection* po_pContiguousnessPoints) const
    {
    HPRECONDITION(po_pContiguousnessPoints != NULL);

    // The arcs must be contiguous
    HPRECONDITION(AreArcsContiguous(pi_rArc));

    // The two arcs must have the same coordinate system or be related by a direction
    // preserving relation
    HPRECONDITION((GetCoordSys() == pi_rArc.GetCoordSys()) ||
                  (GetCoordSys()->HasDirectionPreservingRelationTo(pi_rArc.GetCoordSys())));


    // When contiguous together, two arcs will have exactly 2
    // contiguousness points

    // Save initial number of points in list
    size_t  InitialNumberOfPoints = po_pContiguousnessPoints->size();


    // We check if the start points are equal
    if (m_StartPoint.IsEqualTo(pi_rArc.m_StartPoint) &&
        CalculateBearing(m_StartPoint, HVE2DVector::BETA).
        IsEqualTo(pi_rArc.CalculateBearing(pi_rArc.m_StartPoint, HVE2DVector::BETA)))
        {
        // The two start points are on top of each other
        // It is therefore one of the contiguousness points
        po_pContiguousnessPoints->push_back(m_StartPoint);

        // The second point is either one of the end points

        // Check if the end points are equal
        if (m_EndPoint.IsEqualTo(pi_rArc.m_EndPoint))
            po_pContiguousnessPoints->push_back(m_EndPoint);
        else
            {
            // Since the end points are not equal, then the one that is on the other arc
            // is the result
            if (IsPointOn(pi_rArc.m_EndPoint))
                po_pContiguousnessPoints->push_back(pi_rArc.m_EndPoint);
            else
                po_pContiguousnessPoints->push_back(m_EndPoint);
            }
        }
    else if (m_EndPoint.IsEqualTo(pi_rArc.m_EndPoint) &&
             CalculateBearing(m_EndPoint, HVE2DVector::ALPHA).
             IsEqualTo(pi_rArc.CalculateBearing(pi_rArc.m_EndPoint, HVE2DVector::ALPHA)))
        {
        // The two end points are on top of each other

        // The other point is either one of the start point
        // the one that is on the other arc is the result
        if (IsPointOn(pi_rArc.m_StartPoint))
            po_pContiguousnessPoints->push_back(pi_rArc.m_StartPoint);
        else
            po_pContiguousnessPoints->push_back(m_StartPoint);

        // The end point is therefore one of the contiguousness points
        po_pContiguousnessPoints->push_back(m_EndPoint);

        }
    else if (m_StartPoint.IsEqualTo(pi_rArc.m_EndPoint) &&
             CalculateBearing(m_StartPoint, HVE2DVector::BETA).
             IsEqualTo(pi_rArc.CalculateBearing(pi_rArc.m_EndPoint, HVE2DVector::ALPHA)))
        {
        // The self start point in on top of the given end point
        // It is therefore one of the contiguousness points
        po_pContiguousnessPoints->push_back(m_StartPoint);

        // The second point is either one of the other point

        // Check if the other extremity points are equal
        if (m_EndPoint.IsEqualTo(pi_rArc.m_StartPoint))
            po_pContiguousnessPoints->push_back(m_EndPoint);
        else
            {
            // Since the those points are not equal, then the one that is on the other arc
            // is the result
            if (IsPointOn(pi_rArc.m_StartPoint))
                po_pContiguousnessPoints->push_back(pi_rArc.m_StartPoint);
            else
                po_pContiguousnessPoints->push_back(m_EndPoint);
            }
        }
    else if (m_EndPoint.IsEqualTo(pi_rArc.m_StartPoint) &&
             CalculateBearing(m_EndPoint, HVE2DVector::ALPHA).
             IsEqualTo(pi_rArc.CalculateBearing(pi_rArc.m_StartPoint, HVE2DVector::BETA)))
        {
        // The self end point in on top of the given start point

        // The second point is either one of the other extremity point
        // the one that is on the other arc is the result
        if (IsPointOn(pi_rArc.m_EndPoint))
            po_pContiguousnessPoints->push_back(pi_rArc.m_EndPoint);
        else
            po_pContiguousnessPoints->push_back(m_StartPoint);

        // The end point is therefore one of the contiguousness points
        po_pContiguousnessPoints->push_back(m_EndPoint);

        // The second point is either one of the other point

        }
    else
        {
        // General case, no extremity is on the other
        // Note than two arcs can have two contiguousness regions

        bool SelfStartIsOn = pi_rArc.IsPointOn(m_StartPoint);
        bool SelfEndIsOn = pi_rArc.IsPointOn(m_EndPoint);
        bool GivenStartIsOn = IsPointOn(pi_rArc.m_StartPoint);
        bool GivenEndIsOn = IsPointOn(pi_rArc.m_EndPoint);

        // Check if all four extremities are on
        if (SelfStartIsOn && SelfEndIsOn && GivenStartIsOn && GivenEndIsOn)
            {
            // All four points are one the other arc
            // This implies possible dual contiguousness regions
            // We must eleminate the case of linking

            if (m_StartPoint.IsEqualTo(pi_rArc.m_StartPoint))
                {
                po_pContiguousnessPoints->push_back(pi_rArc.m_EndPoint);
                po_pContiguousnessPoints->push_back(m_EndPoint);
                }
            else if (m_StartPoint.IsEqualTo(pi_rArc.m_EndPoint))
                {
                po_pContiguousnessPoints->push_back(pi_rArc.m_StartPoint);
                po_pContiguousnessPoints->push_back(m_EndPoint);
                }
            else if (m_EndPoint.IsEqualTo(pi_rArc.m_StartPoint))
                {
                po_pContiguousnessPoints->push_back(pi_rArc.m_EndPoint);
                po_pContiguousnessPoints->push_back(m_EndPoint);
                }
            else if (m_EndPoint.IsEqualTo(pi_rArc.m_EndPoint))
                {
                po_pContiguousnessPoints->push_back(pi_rArc.m_StartPoint);
                po_pContiguousnessPoints->push_back(m_StartPoint);
                }
            else
                {
                // We have dual contiguousness
                po_pContiguousnessPoints->push_back(m_StartPoint);

                // Compare arc direction
                if (m_RotationDirection == pi_rArc.m_RotationDirection)
                    {
                    po_pContiguousnessPoints->push_back(pi_rArc.m_EndPoint);
                    po_pContiguousnessPoints->push_back(pi_rArc.m_StartPoint);
                    }
                else
                    {
                    po_pContiguousnessPoints->push_back(pi_rArc.m_StartPoint);
                    po_pContiguousnessPoints->push_back(pi_rArc.m_EndPoint);
                    }

                po_pContiguousnessPoints->push_back(m_EndPoint);
                }
            }
        else
            {
            // We only have a single contiguousness

            // We start with arc start point
            if (SelfStartIsOn)
                po_pContiguousnessPoints->push_back(m_StartPoint);

            // Check if the arcs are oriented in the same direction
            if (m_RotationDirection == pi_rArc.m_RotationDirection)
                {
                // The two arcs are oriented likewise ...
                // we check in end to start order
                if (GivenStartIsOn)
                    po_pContiguousnessPoints->push_back(pi_rArc.m_StartPoint);
                if (GivenEndIsOn)
                    po_pContiguousnessPoints->push_back(pi_rArc.m_EndPoint);
                }
            else
                {
                // The two arcs are oriented differently
                // we check in end to start order
                if (GivenEndIsOn)
                    po_pContiguousnessPoints->push_back(pi_rArc.m_EndPoint);
                if (GivenStartIsOn)
                    po_pContiguousnessPoints->push_back(pi_rArc.m_StartPoint);

                }

            // The last possible point is end point
            if (SelfEndIsOn)
                po_pContiguousnessPoints->push_back(m_EndPoint);
            }
        }

    // There is an even number of points added
    HPOSTCONDITION((po_pContiguousnessPoints->size() - InitialNumberOfPoints) % 2 == 0);

    return (po_pContiguousnessPoints->size() - InitialNumberOfPoints);
    }


//-----------------------------------------------------------------------------
// ObtainContiguousnessPointsAtWithArc
// Calculates and returns the contiguousness points with another arc if
// there is any
//-----------------------------------------------------------------------------
void HVE2DArc::ObtainContiguousnessPointsAtWithArc(
    const HVE2DArc& pi_rArc,
    const HGF2DLocation& pi_rPoint,
    HGF2DLocation* po_pFirstContiguousnessPoint,
    HGF2DLocation* po_pSecondContiguousnessPoint) const
    {
    HPRECONDITION(po_pFirstContiguousnessPoint != NULL);
    HPRECONDITION(po_pSecondContiguousnessPoint != NULL);

    // The arcs must be contiguous
    HPRECONDITION(AreArcsContiguousAt(pi_rArc, pi_rPoint));

    // The two arcs must have the same coordinate system or be related by a direction
    // preserving relation
    HPRECONDITION((GetCoordSys() == pi_rArc.GetCoordSys()) ||
                  (GetCoordSys()->HasDirectionPreservingRelationTo(pi_rArc.GetCoordSys())));


    HGF2DLocationCollection ContiguousnessPoints;

    // Obtain all contiguousness points between arcs
    ObtainContiguousnessPointsWithArc(pi_rArc, &ContiguousnessPoints);

    // Some points must be found
    HASSERT(ContiguousnessPoints.size() > 0);

    // The number of points must be even
    HASSERT(ContiguousnessPoints.size() % 2 == 0);

    // For every pair of contiguousness points
    HGF2DLocationCollection::iterator Itr;
    HGF2DLocationCollection::iterator OtherItr;

    bool Found = false;
    for (Itr = ContiguousnessPoints.begin() ;
         !Found && Itr != ContiguousnessPoints.end() ; ++Itr)
        {
        OtherItr = Itr;
        ++OtherItr;

        // Create an arc with these two points
        HVE2DArc TempArc(m_Center,
                         ((*Itr) - m_Center).CalculateBearing(),
                         ((*OtherItr) - m_Center).CalculateBearing(),
                         CalculateRadius(),
                         m_RotationDirection);

        Found = TempArc.IsPointOn(pi_rPoint);

        }

    // The contiguousness region must have been located
    HASSERT(Found);

    *po_pFirstContiguousnessPoint = *Itr;
    *po_pSecondContiguousnessPoint = *OtherItr;
    }



//-----------------------------------------------------------------------------
// CalculateClosestPoint
// Calculates the closest point on arc to given point
//-----------------------------------------------------------------------------
HGF2DLocation HVE2DArc::CalculateClosestPoint(const HGF2DLocation& pi_rPoint) const
    {
    // Calculate closest point to line the arc is part of
    HGF2DLocation   ClosestPoint(CalculateCircle().CalculateClosestPoint(pi_rPoint));

    // Check if this point is not located on the arc
    if (!IsPointOnCircleOnArc(ClosestPoint))
        {
        // The point is not located on the arc ....
        // Calculate distances from extremities to point
        double FromStart((m_StartPoint - pi_rPoint).CalculateLength());
        double FromEnd((m_EndPoint - pi_rPoint).CalculateLength());

        // Since the closest point on line is not located on arc, then the
        // closest point is the closest of the start point or the end point.
        ClosestPoint = ((FromStart < FromEnd) ? m_StartPoint : m_EndPoint);
        }

    return (ClosestPoint);
    }


//-----------------------------------------------------------------------------
// IsPointOn
// Checks if the point is located on the arc
//-----------------------------------------------------------------------------
bool   HVE2DArc::IsPointOn(const HGF2DLocation& pi_rTestPoint,
                            HVE2DVector::ExtremityProcessing    pi_ExtremityProcessing,
                            double                             pi_Tolerance) const
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
// Checks if the point is located on the arc
//-----------------------------------------------------------------------------
bool   HVE2DArc::IsPointOnSCS(const HGF2DLocation& pi_rTestPoint,
                               HVE2DVector::ExtremityProcessing    pi_ExtremityProcessing,
                               double                             pi_Tolerance) const
    {
    // The two must share the same coordinate system
    HPRECONDITION(GetCoordSys() == pi_rTestPoint.GetCoordSys());

    bool   Answer = false;

    // Extract tolerance if internal tolerance must be use obtain it
    double Tolerance = pi_Tolerance;
    if (Tolerance == HVE_USE_INTERNAL_EPSILON)
        Tolerance = GetTolerance();

    // Obtain extremes of arc
    HGF2DExtent ArcExtent(GetExtent());

    double XMin = ArcExtent.GetXMin();
    double XMax = ArcExtent.GetXMax();
    double YMin = ArcExtent.GetYMin();
    double YMax = ArcExtent.GetYMax();

    double X = pi_rTestPoint.GetX();
    double Y = pi_rTestPoint.GetY();

    // A point is on if it is within the extended extent.
    // No epsilon is applied, therefore a point located within an
    // epsilon of distance of one of the extremities will
    // not be recognized as on arc
    if (HDOUBLE_GREATER_OR_EQUAL(X, XMin, Tolerance) &&
        HDOUBLE_SMALLER_OR_EQUAL(X, XMax, Tolerance) &&
        HDOUBLE_GREATER_OR_EQUAL(Y, YMin, Tolerance) &&
        HDOUBLE_SMALLER_OR_EQUAL(Y, YMax, Tolerance))
        {
        double DistanceToCircle;
        DistanceToCircle = CalculateCircle().CalculateShortestDistance(pi_rTestPoint);
        Answer = HDOUBLE_EQUAL(0.0, DistanceToCircle, Tolerance);

        // If the point is in extent and on circle, then it is a possible candidate
        if (Answer)
            {
            // Obtain start bearing of arc
            HGFBearing StartBearing = CalculateStartBearing();

            // Obtain bearing from center to given point
            HGFBearing PointBearing = (pi_rTestPoint - m_Center).CalculateBearing();

            // Check if point bearing is within arc sweep
            Answer = StartBearing.IsBearingWithinSweep(CalculateSweep(), PointBearing);
            }

        // Check if extremities must be excluded
        if ((Answer) && (pi_ExtremityProcessing == HVE2DVector::EXCLUDE_EXTREMITIES))
            {
            // The caller wishes to exclude extremities from operation
            // We check it is different from extremity
            Answer = (!m_StartPoint.IsEqualToSCS(pi_rTestPoint, Tolerance) &&
                      !m_EndPoint.IsEqualToSCS(pi_rTestPoint, Tolerance));
            }
        }

    return(Answer);


    }


//-----------------------------------------------------------------------------
// IsPointOnCircleOnArc
// Static method
// Checks if the point is located on the arc knowing it is on the circle
//-----------------------------------------------------------------------------
bool   HVE2DArc::IsPointOnCircleOnArc(const HGF2DLocation& pi_rTestPoint) const
    {
    HGF2DLocation   MyPoint(pi_rTestPoint, GetCoordSys());

    // Obtain extremes of arc
    HGF2DExtent ArcExtent(GetExtent());

    double XMin = ArcExtent.GetXMin();
    double XMax = ArcExtent.GetXMax();
    double YMin = ArcExtent.GetYMin();
    double YMax = ArcExtent.GetYMax();

    double X = MyPoint.GetX();
    double Y = MyPoint.GetY();

    bool Answer;

    // A point is on if it is within the extended extent.
    // No epsilon is applied, therefore a point located within an
    // epsilon of distance of one of the extremities will
    // not be recognized as on arc
    Answer = (HDOUBLE_GREATER_OR_EQUAL_EPSILON(X, XMin) &&
              HDOUBLE_SMALLER_OR_EQUAL_EPSILON(X, XMax) &&
              HDOUBLE_GREATER_OR_EQUAL_EPSILON(Y, YMin) &&
              HDOUBLE_SMALLER_OR_EQUAL_EPSILON(Y, YMax));

    // If the point is in extent, then it is apossible candidate
    if (Answer)
        {
        // Obtain start bearing of arc
        HGFBearing StartBearing = CalculateStartBearing();

        // Obtain bearing from center to given point
        HGFBearing PointBearing = (MyPoint - m_Center).CalculateBearing();

        // Check if point bearing is within arc sweep
        Answer = StartBearing.IsBearingWithinSweep(CalculateSweep(), PointBearing);
        }

    return(Answer);
    }


//-----------------------------------------------------------------------------
// AreArcsContiguous
// PRIVATE
// Indicates if the two arcs are contiguous
//-----------------------------------------------------------------------------
bool HVE2DArc::AreArcsContiguous(const HVE2DArc& pi_rArc) const
    {
    // The contiguousness determination between 2 arcs is complicated by the matter
    // that two arcs can have 2 contiguousness regions. To complicate matters, also,
    // they may be linked at one extremity points, but be contiguouss on
    // the other end of the arcs.

    // Check if they share the same coordinate system or they are related
    // through a Shape preserving model
    HPRECONDITION((GetCoordSys() == pi_rArc.GetCoordSys()) ||
                  (GetCoordSys()->HasShapePreservingRelationTo(pi_rArc.GetCoordSys())));

    bool   Answer;

    HVE2DArc    TempArc(pi_rArc);

    // Other cases

    // The processing differs if the relation between coordinate systems
    // preserves direction or not
    if ((GetCoordSys() != pi_rArc.GetCoordSys()) &&
        (!GetCoordSys()->HasDirectionPreservingRelationTo(pi_rArc.GetCoordSys())))
        {
        TempArc.SetByPoints(pi_rArc.m_StartPoint.ExpressedIn(GetCoordSys()),
                            pi_rArc.CalculateRelativePoint(0.5).ExpressedIn(GetCoordSys()),
                            pi_rArc.m_EndPoint.ExpressedIn(GetCoordSys()));
        }


    // To be contiguous, they must be located on the same circle,
    // and also their extent must overlap
    Answer = (
                 CalculateCircle().IsEqualTo(TempArc.CalculateCircle()) &&
                 GetExtent().InnerOverlaps(TempArc.GetExtent(),
                                           MIN(GetTolerance(), TempArc.GetTolerance()))
             );

    // If the first conditions are respected ... they are possible candidates
    // for contiguousness
    if (Answer)
        {
        // Obtain self arc parameters
        HGFBearing    SelfStartBearing = CalculateStartBearing();
        HGFBearing    SelfEndBearing = CalculateEndBearing();
        double        SelfSweep = CalculateSweep();

        // Obtain given arc parameters
        HGFBearing    GivenStartBearing = pi_rArc.CalculateStartBearing();
        HGFBearing    GivenEndBearing = pi_rArc.CalculateEndBearing();
        double        GivenSweep = pi_rArc.CalculateSweep();

        Answer = (
                     (SelfStartBearing.IsBearingWithinSweep(SelfSweep, GivenStartBearing) &&
                      (SelfStartBearing != GivenStartBearing)
                     ) ||
                     (SelfStartBearing.IsBearingWithinSweep(SelfSweep, GivenEndBearing) &&
                      (SelfStartBearing != GivenEndBearing)
                     ) ||
                     (GivenStartBearing.IsBearingWithinSweep(GivenSweep, SelfStartBearing) &&
                      (GivenStartBearing != SelfStartBearing)
                     ) ||
                     (GivenStartBearing.IsBearingWithinSweep(GivenSweep, SelfEndBearing) &&
                      (GivenStartBearing != SelfEndBearing)
                     ) ||
                     ((GivenStartBearing == SelfStartBearing) &&
                      (m_RotationDirection == pi_rArc.m_RotationDirection)
                     ) ||
                     ((GivenStartBearing == SelfEndBearing) &&
                      (m_RotationDirection != pi_rArc.m_RotationDirection)
                     ) ||
                     ((GivenEndBearing == SelfStartBearing) &&
                      (m_RotationDirection != pi_rArc.m_RotationDirection)
                     ) ||
                     ((GivenEndBearing == SelfEndBearing) &&
                      (m_RotationDirection == pi_rArc.m_RotationDirection)
                     )
                 );
        }

    return(Answer);

    }

//-----------------------------------------------------------------------------
// AreArcsContiguousAt
// PRIVATE
// Indicates if the two arcs are contiguous at given point
//-----------------------------------------------------------------------------
bool HVE2DArc::AreArcsContiguousAt(const HVE2DArc& pi_rArc,
                                    const HGF2DLocation& pi_rPoint) const
    {
    // Check if they share the same coordinate system or they are related
    // through a Shape preserving model
    HPRECONDITION((GetCoordSys() == pi_rArc.GetCoordSys()) ||
                  (GetCoordSys()->HasShapePreservingRelationTo(pi_rArc.GetCoordSys())));

    // The given point must be located on both vectors
    HPRECONDITION(IsPointOn(pi_rPoint) && pi_rArc.IsPointOn(pi_rPoint));


    bool   Answer;

    HVE2DArc    TempArc(pi_rArc);

    // Other cases

    // The processing differs if the relation between coordinate systems
    // preserves direction or not
    if ((GetCoordSys() != pi_rArc.GetCoordSys()) &&
        (!GetCoordSys()->HasDirectionPreservingRelationTo(pi_rArc.GetCoordSys())))
        {
        TempArc.SetByPoints(pi_rArc.m_StartPoint.ExpressedIn(GetCoordSys()),
                            pi_rArc.CalculateRelativePoint(0.5).ExpressedIn(GetCoordSys()),
                            pi_rArc.m_EndPoint.ExpressedIn(GetCoordSys()));
        }

    // Pre-compute tolerance
    double Tolerance = MIN(GetTolerance(), TempArc.GetTolerance());


    // Check if circles are the same and the extents of arcs overlap
    Answer = (
                 CalculateCircle().IsEqualTo(TempArc.CalculateCircle()) &&
                 GetExtent().InnerOverlaps(TempArc.GetExtent(), Tolerance)
             );

    // If the first conditions are respected ...
    if (Answer)
        {
        // We already know that the point is located on both arcs, that their extent overlap
        // and that they share the same circle.
        // There is only a few cases where there will not be contiguousness at given location
        // These include only the cases where the arcs link at either end points
        // and the given location is exactly located on one of these extremity points
        bool PointIsAtSelfStart;
        bool PointIsAtGivenStart;

        // Check if given location is equal to either point of the self arc
        if ((PointIsAtSelfStart = m_StartPoint.IsEqualTo(pi_rPoint, Tolerance)) ||
            m_EndPoint.IsEqualTo(pi_rPoint, Tolerance))
            {
            // The given point is effectively located at an end point of self arc
            // Check the same for other arc
            if ((PointIsAtGivenStart = TempArc.GetStartPoint().IsEqualTo(pi_rPoint, Tolerance)) ||
                TempArc.GetEndPoint().IsEqualTo(pi_rPoint, Tolerance))
                {
                // The given point is effectively located at extremity of arcs.
                // This does not completely rule out contiguousness ... it depends on the
                // points and the rotation direction

                // Check if point is on start point of self
                if (PointIsAtSelfStart)
                    {
                    // It may still be on either extremity of given arc
                    if (PointIsAtGivenStart)
                        {
                        // Check if arcs rotate in same direction
                        Answer = (m_RotationDirection == TempArc.m_RotationDirection);
                        }
                    else
                        {
                        // The arcs link at end point of given arc
                        // Check if arcs rotate in different direction
                        Answer = (m_RotationDirection != TempArc.m_RotationDirection);
                        }
                    }
                else
                    {
                    // The point is located on end point of self arc
                    // It may still be on either extremity of given arc
                    if (PointIsAtGivenStart)
                        {
                        // Check if arcs rotate in different direction
                        Answer = (m_RotationDirection != TempArc.m_RotationDirection);
                        }
                    else
                        {
                        // The arcs link at end point of given arc
                        // Check if arcs rotate in same direction
                        Answer = (m_RotationDirection == TempArc.m_RotationDirection);
                        }
                    }
                }
            }
        }

    return(Answer);

    }



//-----------------------------------------------------------------------------
// AreArcsAdjacent
// PRIVATE
// Indicates if the two arcs are adjacent
//-----------------------------------------------------------------------------
bool HVE2DArc::AreArcsAdjacent(const HVE2DArc& pi_rArc) const
    {
    // Check if they share the same coordinate system or they are related
    // through a Shape preserving model
    HPRECONDITION((GetCoordSys() == pi_rArc.GetCoordSys()) ||
                  (GetCoordSys()->HasShapePreservingRelationTo(pi_rArc.GetCoordSys())));

    bool   Answer = false;

    HVE2DArc    TempArc(pi_rArc);

    // The processing differs if the they do not have the same coordinate systems
    if (GetCoordSys() != pi_rArc.GetCoordSys())
        {
        // We convert arc to self coordinate system
        TempArc.SetByPoints(pi_rArc.m_StartPoint.ExpressedIn(GetCoordSys()),
                            pi_rArc.CalculateRelativePoint(0.5).ExpressedIn(GetCoordSys()),
                            pi_rArc.m_EndPoint.ExpressedIn(GetCoordSys()));
        }


    // Obtain circles of arcs
    HVE2DCircle SelfCircle = CalculateCircle();
    HVE2DCircle GivenCircle = TempArc.CalculateCircle();


    // Two arcs are adjacent if they are contiguous OR
    // if the arcs are adjacent (by one point only)
    // and this adjacency point is located on both the arcs
    if (GetExtent().OutterOverlaps(TempArc.GetExtent(),
                                   MIN(GetTolerance(), TempArc.GetTolerance())))
        {
        if (AreArcsContiguous(TempArc))
            {
            Answer = true;
            }
        else if (SelfCircle.IsAdjacentToCircle(GivenCircle))
            {
            // Declare a location to receive adjacence point
            HGF2DLocation   AdjacencePoint(GetCoordSys());

            // Obtain adjacence point
            SelfCircle.ObtainCircleAdjacencyPoint(GivenCircle, &AdjacencePoint);

            Answer = IsPointOn(AdjacencePoint) && TempArc.IsPointOn(AdjacencePoint);
            }
        }

    return(Answer);
    }

//-----------------------------------------------------------------------------
// AreArcsCrossing
// PRIVATE
// Indicates if the two arcs are crossing
//-----------------------------------------------------------------------------
bool HVE2DArc::AreArcsCrossing(const HVE2DArc& pi_rArc) const
    {
    // Check if they share the same coordinate system or they are related
    // through a Shape preserving model
    HPRECONDITION((GetCoordSys() == pi_rArc.GetCoordSys()) ||
                  (GetCoordSys()->HasShapePreservingRelationTo(pi_rArc.GetCoordSys())));

    HGF2DLocation   DumPoint1(GetCoordSys());
    HGF2DLocation   DumPoint2(GetCoordSys());

    return(IntersectArc(pi_rArc, &DumPoint1, &DumPoint2) != 0);
    }

//-----------------------------------------------------------------------------
// AreArcAndSegmentCrossing
// PRIVATE
// Indicates if the arc crosses the segment
//-----------------------------------------------------------------------------
bool HVE2DArc::AreArcAndSegmentCrossing(const HVE2DSegment& pi_rSegment) const
    {
    // Check if they share the same coordinate system or they are related
    // through a linearity preserving model
    HPRECONDITION((GetCoordSys() == pi_rSegment.GetCoordSys()) ||
                  (GetCoordSys()->HasLinearityPreservingRelationTo(pi_rSegment.GetCoordSys())));

    HGF2DLocation   DumPoint1(GetCoordSys());
    HGF2DLocation   DumPoint2(GetCoordSys());

    return(IntersectSegment(pi_rSegment, &DumPoint1, &DumPoint2) != 0);
    }

//-----------------------------------------------------------------------------
// AreArcAndSegmentAdjacent
// PRIVATE
// Indicates if the arc is adjacent the segment
//-----------------------------------------------------------------------------
bool HVE2DArc::AreArcAndSegmentAdjacent(const HVE2DSegment& pi_rSegment) const
    {
    // Check if they share the same coordinate system or they are related
    // through a linearity preserving model
    HPRECONDITION((GetCoordSys() == pi_rSegment.GetCoordSys()) ||
                  (GetCoordSys()->HasLinearityPreservingRelationTo(pi_rSegment.GetCoordSys())));

    bool   Answer = false;

    // Obtain circles of arc and line of segment
    HVE2DCircle SelfCircle = CalculateCircle();
    HGF2DLine GivenLine = pi_rSegment.CalculateLine();

    // Declare a location to receive adjacence point
    HGF2DLocation   AdjacencePoint(GetCoordSys());

    // The arc and segment are adjacent if the circle and line are adjacent (by one point only)
    // and this adjacency point is located on both the arc and the segment
    if (GetExtent().OutterOverlaps(pi_rSegment.GetExtent(),
                                   MIN(GetTolerance(), pi_rSegment.GetTolerance())) &&
        SelfCircle.IsAdjacentToLine(GivenLine))
        {
        // Declare a location to receive adjacence point
        HGF2DLocation   AdjacencePoint(GetCoordSys());

        // Obtain adjacence point
        SelfCircle.ObtainLineAdjacencyPoint(GivenLine, &AdjacencePoint);

        Answer = IsPointOn(AdjacencePoint) && pi_rSegment.IsPointOn(AdjacencePoint);
        }

    return(Answer);
    }


//-----------------------------------------------------------------------------
// Drop
// Returns the description of arc in the form of raw location
// segments
//-----------------------------------------------------------------------------
void HVE2DArc::Drop(HGF2DLocationCollection* po_pPoints,
                    double                   pi_rTolerance,
                    EndPointProcessing pi_EndPointProcessing) const
    {
    HPRECONDITION(po_pPoints != NULL);

    // The tolerance may not be null
    HPRECONDITION(pi_rTolerance != 0.0);

    // Calculate length
    double Length = CalculateLength();

    // Evaluate the number of points needed
    uint32_t NumPoints = (uint32_t)(Length / pi_rTolerance);

    // Evaluate the angle sweep of arc
    double TotalSweep = CalculateSweep();

    // Evaluate the step sweep
    double StepSweep = TotalSweep / NumPoints;

    // Obtain start bearing
    HGFBearing StartBearing = CalculateStartBearing();

    // Obtain radius
    double Radius = CalculateRadius();

    // Loop for all needed points
    double AngleCount(0.0);
    for (; HDOUBLE_SMALLER_EPSILON(AngleCount, TotalSweep) ;
         AngleCount += StepSweep)
        {
        // Create new location
        HGF2DLocation NewPoint = m_Center +
                                 HGF2DDisplacement(StartBearing + AngleCount, Radius);

        // Append new point
        po_pPoints->push_back(NewPoint);

        }

    // Check if end point must be added
    if (pi_EndPointProcessing == HVE2DLinear::INCLUDE_END_POINT)
        {
        po_pPoints->push_back(m_EndPoint);
        }
    }


//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
void HVE2DArc::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    char     DumString[256];

    HVE2DBasicLinear::PrintState(po_rOutput);

    po_rOutput << "Object is a HVE2DArc" << endl;
    HDUMP0("Object is a HVE2DArc");
    sprintf(DumString,
            "X = %5.15lf  Y = %5.15lf",
            m_Center.GetX(),
            m_Center.GetY());
    po_rOutput << "Center is at : " << DumString << endl;
    HDUMP1("Center is at %s ", DumString);
    po_rOutput << "RotationDirection is : "
               << (m_RotationDirection == HGFAngle::CW ? "CW" : "CCW");
    HDUMP1("Rotation direction is %s ",
           (m_RotationDirection == HGFAngle::CW ? "CW" : "CCW"));
#endif
    }
