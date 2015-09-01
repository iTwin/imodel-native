//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DVector.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGFAngle.h>
#include <Imagepp/all/h/HGF2DVector.h>
#include <Imagepp/all/h/HGF2DDisplacement.h>
#include <Imagepp/all/h/HGF2DLinear.h>
#include <Imagepp/all/h/HGFLiteTolerance.h>

#define DEFAULT_STROKE_TOLERANCE_EXTENT_RATIO (100.0)

//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
void HGF2DVector::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    po_rOutput << "BEGIN Dumping a HGF2DVector object" << endl;
    HDUMP0("BEGIN Dumping a HGF2DVector object\n");

    // Dump the end points
    char    DumString[256];
    if (m_AutoToleranceActive)
        {
        sprintf(DumString, "Auto tolerance : ACTIVE, Tolerance = %5.15lf", m_Tolerance);
        }
    else
        {
        sprintf(DumString, "Auto tolerance : NON-ACTIVE, Tolerance = %5.15lf", m_Tolerance);
        }
    po_rOutput << DumString << endl;
    HDUMP0(DumString);
    HDUMP0("\n");
    po_rOutput << "END (HGF2DVector)" << endl;
    HDUMP0("END (HGF2DVector)\n");
#endif
    }

//-----------------------------------------------------------------------------
// CalculatePerpendicularBearingAt
// This method returns the bearing perpendicular to the given point
// so that it goes in the right when facing the given direction of
// linear
//-----------------------------------------------------------------------------
HGFBearing HGF2DVector::CalculatePerpendicularBearingAt(const HGF2DPosition& pi_rPoint,
                                                  HGF2DVector::ArbitraryDirection
                                                  pi_DirectionToRight,
                                                  double* po_pSweep) const
    {
    // The sweep angle recipient is optional

    // The point must be on the vector
    HPRECONDITION(IsPointOn(pi_rPoint));

    // Calculate direction
    // Obtain bearing at point in both direction
    HGFBearing AlphaBearing = CalculateBearing(pi_rPoint, HGF2DVector::ALPHA);
    HGFBearing BetaBearing = CalculateBearing(pi_rPoint, HGF2DVector::BETA);

    HGFBearing ResultBearing;
    double AngleToApply;

    if (pi_DirectionToRight == HGF2DVector::ALPHA)
        {
        double DeltaBearing = BetaBearing - AlphaBearing;
        AngleToApply = DeltaBearing / 2.0;

        ResultBearing  = AlphaBearing + AngleToApply;
        }
    else
        {
        double DeltaBearing = AlphaBearing - BetaBearing;
        AngleToApply = DeltaBearing / 2.0;

        ResultBearing  = BetaBearing + AngleToApply;
        }

    // Check if sweep is asked for
    if (po_pSweep)
        {
        // The sweep is desired
        *po_pSweep = fabs(AngleToApply) - (PI / 2.0);
        }

    return(ResultBearing);
    }

/** -----------------------------------------------------------------------------
    This method determines of the given linear connects on the self
    vector by one (or both) of its extremity points.

    @param pi_rLinear    Constant reference to linear object.

    @return true if the linear connects on the vector, and false otherwise.

    Example
    @code
    @end

    @see HGF2DLinear
    -----------------------------------------------------------------------------
*/
bool HGF2DVector::IsConnectedBy(const HGF2DLinear& pi_rLinear) const
    {
    return (IsPointOn(pi_rLinear.GetStartPoint()) || IsPointOn(pi_rLinear.GetEndPoint()));
    }




/** -----------------------------------------------------------------------------
    This method is a service protected method provided to all descendants
    to be used in the determination of intersect points at split point
    in their definition. The test point provided must be located ON the self
    vector. The method determines if this point is also shared by the
    given vector, and if so if this corresponds to the crossing of the
    self boundary by the given vectorr boundary. If the test point is
    part of a contiguousness region between the two vectors, the path
    of each vector will be followed along this contiguousness region to discover
    if the vector boundary do leave in crossing directions. The third and
    fourth parameters are used only in solving intersection once and only
    once when the next split point would also be located inside the same contiguousness
    region as the test point. If the last parameter is true the next point
    provided must be located ON the self vector boundary. In this case if
    both test point and next points are located in the same contiguousness
    region, the function will indicate that there is no intersection. If the
    last parameter is false, the next point is ignored and contiguousness
    region intersection analysis will proceed until resolution. Note that the
    vector given must either be expressed in the same coordinate system as self,
    or if not, then the coordinate system must be related through a direction
    preserving transformation model (see HGF2DTransfoModel::PreservesDirection())

    @param pi_rObject Constant reference to a HGF2DVector to find cross points with.

    @param pi_rTestPoint A constant reference to an HGF2DPosition which must
                         be located on the self vector at which resolution
                         of possible intersection is needed.

    @param pi_rNextPoint A constant reference to an HGF2DPosition which if
                         pi_ProcessNext is true and is part of the same
                         contiguousness region as test point will abort
                         analysis and result in a no intersection answer.
                         Note that if there is no contiguousness at test
                         point, or if the points are part of different
                         contiguousness regions, then the analysis may
                         result in an intersection determination.

    @param pi_ProcessNext A flag indicating if the pi_rNextPoint parameter
                          must be ignored or not. If false, the next
                          point is ignored.

    @return true if an intersection at split point is located.

    Example
    @code
    @end

    @see IsPointOn()
    @see Intersect()
    -----------------------------------------------------------------------------
*/
bool HGF2DVector::IntersectsAtSplitPoint(const HGF2DVector& pi_rVector,
                                          const HGF2DPosition& pi_rTestPoint,
                                          const HGF2DPosition& pi_rNextEndPoint,
                                          bool pi_ProcessNext) const
    {
    // The test point must be on self
    HPRECONDITION(IsPointOn(pi_rTestPoint));

    // If a second point is given, it must be on also
    HPRECONDITION(!pi_ProcessNext || IsPointOn(pi_rNextEndPoint));

    bool Answer = false;

    // Obtain tolerance
    double Tolerance = MIN(GetTolerance(), pi_rVector.GetTolerance());

    // Check if end point is located on the given
    if (pi_rVector.IsPointOn(pi_rTestPoint, HGF2DVector::EXCLUDE_EXTREMITIES, Tolerance) &&
        !pi_rVector.IsNull())
        {
        if (!IsAtAnExtremity(pi_rTestPoint, Tolerance))
            {
            HGFBearing  Bearing0;
            HGFBearing  Bearing1;
            HGFBearing  Bearing2;
            HGFBearing  Bearing3;


            double Acc0=0.0;
            double Acc1=0.0;
            double Acc2=0.0;
            double Acc3=0.0;

            HGF2DPosition   FirstContiguousnessPoint;
            HGF2DPosition   SecondContiguousnessPoint;


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
                    HGF2DPosition   NextFirstContiguousnessPoint;
                    HGF2DPosition   NextSecondContiguousnessPoint;

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

                        // Obtain bearings and accelerations at contiguousness point
                        HGFBearing    Bearing0A(pi_rVector.
                                                CalculateBearing(FirstContiguousnessPoint,
                                                                 HGF2DVector::ALPHA));
                        HGFBearing    Bearing0B(pi_rVector.
                                                CalculateBearing(FirstContiguousnessPoint,
                                                                 HGF2DVector::BETA));
                        HGFBearing    Bearing2A(CalculateBearing(FirstContiguousnessPoint,
                                                                 HGF2DVector::ALPHA));
                        HGFBearing    Bearing2B(CalculateBearing(FirstContiguousnessPoint,
                                                                 HGF2DVector::BETA));
                        double  Acc0A = pi_rVector.
                                                        CalculateAngularAcceleration(FirstContiguousnessPoint,
                                                                                     HGF2DVector::ALPHA);
                        double  Acc0B = pi_rVector.CalculateAngularAcceleration(FirstContiguousnessPoint, HGF2DVector::BETA);
                        double  Acc2A = CalculateAngularAcceleration(FirstContiguousnessPoint, HGF2DVector::ALPHA);
                        double  Acc2B = CalculateAngularAcceleration(FirstContiguousnessPoint, HGF2DVector::BETA);

                        bool   PossibleCrossing = true;

                        // Two of the preceeding are equal and are to be discarded
                        if ((Bearing0A.IsEqualTo(Bearing2A)) && (HDOUBLE_EQUAL_EPSILON(Acc0A, Acc2A)))
                            {
                            Bearing0 = Bearing0B;
                            Bearing2 = Bearing2B;
                            Bearing1 = pi_rVector.CalculateBearing(SecondContiguousnessPoint,
                                                                   HGF2DVector::ALPHA);
                            Bearing3 = CalculateBearing(SecondContiguousnessPoint,
                                                        HGF2DVector::ALPHA);
                            Reference1 = Bearing0A;
                            Reference2 = CalculateBearing(SecondContiguousnessPoint,
                                                          HGF2DLinear::BETA);
                            Acc0 = Acc0B;
                            Acc2 = Acc2B;
                            Acc1 = pi_rVector.CalculateAngularAcceleration(SecondContiguousnessPoint,
                                                                           HGF2DVector::ALPHA);
                            Acc3 = CalculateAngularAcceleration(SecondContiguousnessPoint,
                                                                HGF2DVector::ALPHA);
                            }
                        else if ((Bearing0B.IsEqualTo(Bearing2A)) && (HDOUBLE_EQUAL_EPSILON(Acc0B, Acc2A)))
                            {
                            Bearing0 = Bearing0A;
                            Bearing2 = Bearing2B;
                            Bearing1 = pi_rVector.CalculateBearing(SecondContiguousnessPoint,
                                                                   HGF2DVector::BETA);
                            Bearing3 = CalculateBearing(SecondContiguousnessPoint,
                                                        HGF2DLinear::ALPHA);
                            Reference1 = Bearing0B;
                            Reference2 = CalculateBearing(SecondContiguousnessPoint,
                                                          HGF2DLinear::BETA);
                            Acc0 = Acc0A;
                            Acc2 = Acc2B;
                            Acc1 = pi_rVector.CalculateAngularAcceleration(SecondContiguousnessPoint,
                                                                           HGF2DVector::BETA);
                            Acc3 = CalculateAngularAcceleration(SecondContiguousnessPoint,
                                                                HGF2DVector::ALPHA);
                            }
                        else if ((Bearing0A.IsEqualTo(Bearing2B)) && (HDOUBLE_EQUAL_EPSILON(Acc0A, Acc2B)))
                            {
                            Bearing0 = Bearing0B;
                            Bearing2 = Bearing2A;
                            Bearing1 = pi_rVector.CalculateBearing(SecondContiguousnessPoint,
                                                                   HGF2DVector::ALPHA);
                            Bearing3 = CalculateBearing(SecondContiguousnessPoint,
                                                        HGF2DLinear::BETA);
                            Reference1 = Bearing0A;
                            Reference2 = CalculateBearing(SecondContiguousnessPoint,
                                                          HGF2DLinear::ALPHA);
                            Acc0 = Acc0B;
                            Acc2 = Acc2A;
                            Acc1 = pi_rVector.CalculateAngularAcceleration(SecondContiguousnessPoint,
                                                                           HGF2DVector::ALPHA);
                            Acc3 = CalculateAngularAcceleration(SecondContiguousnessPoint,
                                                                HGF2DVector::BETA);
                            }
                        else if ((Bearing0B.IsEqualTo(Bearing2B)) && (HDOUBLE_EQUAL_EPSILON(Acc0B, Acc2B)))
                            {
                            Bearing0 = Bearing0A;
                            Bearing2 = Bearing2A;
                            Bearing1 = pi_rVector.CalculateBearing(SecondContiguousnessPoint,
                                                                   HGF2DVector::BETA);
                            Bearing3 = CalculateBearing(SecondContiguousnessPoint,
                                                        HGF2DLinear::BETA);
                            Reference1 = Bearing0B;
                            Reference2 = CalculateBearing(SecondContiguousnessPoint,
                                                          HGF2DLinear::ALPHA);
                            Acc0 = Acc0A;
                            Acc2 = Acc2A;
                            Acc1 = pi_rVector.CalculateAngularAcceleration(SecondContiguousnessPoint,
                                                                           HGF2DVector::BETA);
                            Acc3 = CalculateAngularAcceleration(SecondContiguousnessPoint,
                                                                HGF2DVector::BETA);
                            }
                        else
                            {
                            // This is a rare case that can only happen on a duplicate point,
                            // therefore a point defined twice
                            // for a linear. Since at this point, these can only possess
                            // duplicates at start and end points
                            // we are located at the extremity of a linear ....
                            // No crossing possible
                            PossibleCrossing = false;
                            }

                        if (PossibleCrossing)
                            {
                            double B3MinusRef2 = CalculateNormalizedTrigoValue(Bearing3 - Reference2);
                            double B1MinusRef2 = CalculateNormalizedTrigoValue(Bearing1 - Reference2);
                            double B2MinusRef1 = CalculateNormalizedTrigoValue(Bearing2 - Reference1);
                            double B0MinusRef1 = CalculateNormalizedTrigoValue(Bearing0 - Reference1);

                            if ((!HDOUBLE_EQUAL_EPSILON(B3MinusRef2, B1MinusRef2) || (Acc3 != Acc1)) &&
                                (!HDOUBLE_EQUAL_EPSILON(B2MinusRef1, B0MinusRef1) || (Acc2 == Acc0)))
                                {

                                bool   FirstAnswer = (HDOUBLE_SMALLER_EPSILON(B3MinusRef2, B1MinusRef2) && !HDOUBLE_EQUAL_EPSILON(B3MinusRef2, 0.0)) ||
                                                      ((HDOUBLE_EQUAL_EPSILON(B3MinusRef2, B1MinusRef2) || HDOUBLE_EQUAL_EPSILON(B3MinusRef2, 0.0)) && (Acc3 < Acc1));

                                bool   SecondAnswer = (HDOUBLE_SMALLER_EPSILON(B0MinusRef1, B2MinusRef1) && !HDOUBLE_EQUAL_EPSILON(B0MinusRef1, 0.0)) ||
                                                       ((HDOUBLE_EQUAL_EPSILON(B0MinusRef1, B2MinusRef1) || HDOUBLE_EQUAL_EPSILON(B0MinusRef1, 0.0)) && (Acc0 < Acc2));


                                Answer = ((!FirstAnswer && SecondAnswer) || (FirstAnswer && !SecondAnswer));
                                }
                            else
                                {
                                Answer = false;
                                }
                            }
                        }
                    }
                }
            else
                {
                // Obtain bearings at point
                Bearing0 = pi_rVector.CalculateBearing(pi_rTestPoint, HGF2DVector::ALPHA);
                Bearing1 = pi_rVector.CalculateBearing(pi_rTestPoint, HGF2DVector::BETA);
                Bearing2 = CalculateBearing(pi_rTestPoint, HGF2DVector::ALPHA);
                Bearing3 = CalculateBearing(pi_rTestPoint, HGF2DVector::BETA);

                double B3MinusB0 = CalculateNormalizedTrigoValue(Bearing3 - Bearing0);
                double B1MinusB0 = CalculateNormalizedTrigoValue(Bearing1 - Bearing0);
                double B2MinusB0 = CalculateNormalizedTrigoValue(Bearing2 - Bearing0);


                bool   FirstAnswer = (B3MinusB0 < B1MinusB0) ||
                                      (((B3MinusB0 == B1MinusB0) || (B3MinusB0 == 0.0)) &&
                                       (CalculateAngularAcceleration(pi_rTestPoint, HGF2DVector::BETA) <
                                        pi_rVector.CalculateAngularAcceleration(pi_rTestPoint, HGF2DVector::BETA)));

                bool   SecondAnswer = ((B2MinusB0 < B1MinusB0)) ||
                                       (((B2MinusB0 == B1MinusB0) || (B2MinusB0 == 0.0)) &&
                                        (CalculateAngularAcceleration(pi_rTestPoint, HGF2DVector::ALPHA) <
                                         pi_rVector.CalculateAngularAcceleration(pi_rTestPoint, HGF2DVector::BETA)));


                Answer = ((!FirstAnswer && SecondAnswer) || (FirstAnswer && !SecondAnswer));
                }
            }
        }

    return(Answer);
    }




/** -----------------------------------------------------------------------------
    This protected method determines if the path of the self vector boundary
    at the given point (which MUST be located ON both
    vectors [see IsPointOn()]) when followed in the given arbitrary
    direction leaves the given vector to the left (relative to self)

    @param pi_rVector Constant reference to other vector object.

    @param pi_rPoint Constant reference to an HGF2DPosition which constains
                     a point that must be located IN both vectors, and at
                     which the tendency of self is determined.

    @param pi_Direction The arbitrary direction indicating in which direction
                        the path on the self boundary must be followed.

    @return true if the self vector path tends to the left, and false otherwise.

    Example
    @code
    @end

    @see IsPointOn()
    -----------------------------------------------------------------------------
*/
bool HGF2DVector::TendsToTheLeftOfSCS(const HGF2DVector&   pi_rVector,
                                       const HGF2DPosition& pi_rPoint,
                                       ArbitraryDirection   pi_Direction) const
    {
    // The given point must be located on both vectors
    HPRECONDITION(IsPointOn(pi_rPoint) && pi_rVector.IsPointOn(pi_rPoint));

    bool Answer = false;

    // Create reference bearing
    HGFBearing    ReferenceBearing = pi_rVector.CalculateBearing(pi_rPoint, HGF2DVector::BETA);

    // obtain bearing at point
    HGFBearing  SelfBearing = CalculateBearing(pi_rPoint, pi_Direction);

    // Calculate angle variation
    double AngleDiff = CalculateNormalizedTrigoValue(SelfBearing - ReferenceBearing);

    // Check if bearing points leftward
    if (AngleDiff < PI)
        {
        // We count
        Answer = true;
        }
    // Check if not equal
    else if (HDOUBLE_EQUAL_EPSILON(AngleDiff, 0.0) || HDOUBLE_EQUAL_EPSILON(AngleDiff, PI))
        {
        // They have equal bearing ... obtain accelerations
        double  PointAcc = CalculateAngularAcceleration(pi_rPoint, pi_Direction);
        double  RefAcc = pi_rVector.CalculateAngularAcceleration(pi_rPoint,
                                                                                 HGF2DVector::BETA);

        // Check if self acceleration is greater
        if (PointAcc > RefAcc)
            // Points leftward ... we count
            Answer = true;
        // Otherwise check if equal acceleration
        else if (HDOUBLE_EQUAL_EPSILON(PointAcc, RefAcc))
            {
            // We have contiguousness

            // Obtain contiguousness points at point
            HGF2DPosition   FirstPoint;
            HGF2DPosition   SecondPoint;
            ObtainContiguousnessPointsAt(pi_rVector,
                                         pi_rPoint,
                                         &FirstPoint,
                                         &SecondPoint);

            // Determine which if different from point
            if (FirstPoint.IsEqualTo(pi_rPoint, GetTolerance()))
                {
                // The second point is the other point
                // obtain bearing at point
                HGFBearing  SelfBearing2 = CalculateBearing(SecondPoint, pi_Direction);

                // Calculate new reference bearing
                ReferenceBearing = pi_rVector.CalculateBearing(SecondPoint, HGF2DVector::BETA);

                // Calculate angle variation
                double AngleDiff2 = CalculateNormalizedTrigoValue(SelfBearing2 - ReferenceBearing);

                // Check if bearing points leftward
                if (AngleDiff2 < PI)
                    {
                    // We count
                    Answer = true;
                    }
                // Else check if equal bearing
                else if (HDOUBLE_EQUAL_EPSILON(AngleDiff2, 0.0) ||
                         HDOUBLE_EQUAL_EPSILON(AngleDiff2, PI))
                    {
                    // They have equal bearing ... obtain accelerations
                    double PointAcc2 = CalculateAngularAcceleration(SecondPoint,
                                                                                    pi_Direction);

                    double RefAcc2 = pi_rVector.CalculateAngularAcceleration(SecondPoint,
                                                                                             HGF2DVector::BETA);


                    // Check if acceleraton is greater
                    if (PointAcc2 > RefAcc2)
                        // Points leftward ... we count
                        Answer = true;
                    }
                }
            else
                {
                // The first point is the other point
                // obtain bearing at point
                HGFBearing  SelfBearing2 = CalculateBearing(FirstPoint, pi_Direction);

                // Calculate new reference bearing
                ReferenceBearing = pi_rVector.CalculateBearing(FirstPoint, HGF2DVector::BETA);

                // Calculate angle variation
                double AngleDiff2 = CalculateNormalizedTrigoValue(SelfBearing2 - ReferenceBearing);

                // Check if bearing points leftward
                if (AngleDiff2 < PI)
                    {
                    // Points leftward ... we count
                    Answer = true;
                    }
                else if (HDOUBLE_EQUAL_EPSILON(AngleDiff2, 0.0) ||
                         HDOUBLE_EQUAL_EPSILON(AngleDiff2, PI))
                    {
                    // They have equal bearing ... obtain accelerations
                    double PointAcc2 = CalculateAngularAcceleration(FirstPoint,
                                                                                    pi_Direction);

                    double RefAcc2 = pi_rVector.CalculateAngularAcceleration(FirstPoint,
                                                                                             HGF2DVector::BETA);

                    // Check if acceleraton of self is greater
                    if (PointAcc2 > RefAcc2)
                        // Points leftward ... we count
                        Answer = true;
                    }
                }
            }
        }

    return(Answer);
    }


//-----------------------------------------------------------------------------
// AreContiguousAtAndGet
// This method determines if the two vectors are contiguous at given point
// and returns the two contiguouness point of the region
//-----------------------------------------------------------------------------
bool HGF2DVector::AreContiguousAtAndGet(const HGF2DVector& pi_rVector,
                                         const HGF2DPosition& pi_rPoint,
                                         HGF2DPosition* po_pFirstContiguousnessPoint,
                                         HGF2DPosition* po_pSecondContiguousnessPoint) const
    {
    HPRECONDITION(po_pFirstContiguousnessPoint != 0);
    HPRECONDITION(po_pSecondContiguousnessPoint != 0);

    // These intialisation are part of the patch mentionned below. We simply want to make sure
    // that the coordinate values are initialised and not left undefined possibly set by accident to a previous
    // point result that may have been located on the vector.
    po_pFirstContiguousnessPoint->SetX(0.000000001);
    po_pFirstContiguousnessPoint->SetY(0.000000001);
    po_pSecondContiguousnessPoint->SetX(0.000000001);
    po_pSecondContiguousnessPoint->SetY(0.000000001);

    bool   Answer;

    if (Answer = AreContiguousAt(pi_rVector, pi_rPoint))
        {
        ObtainContiguousnessPointsAt(pi_rVector,
                                     pi_rPoint,
                                     po_pFirstContiguousnessPoint,
                                     po_pSecondContiguousnessPoint);

        // The following lines are a patch that is used when the two vectors are contiguous
        // but there is no start and end point to the contiguousness region. This will
        // occur for example with shapes that are identical
        // If this occurs then the returned points will be undefined
        // Although not a perfect solution the patch should allow detection of such case
        // and enable us not to modify the interface of ObtainContiguousnessPointsAt() which unfortunately is public.
        if (!IsPointOn (*po_pFirstContiguousnessPoint) || !IsPointOn(*po_pSecondContiguousnessPoint))
            Answer = false;
        }

    return(Answer);
    }

/*---------------------------------------------------------------------------------**//**
* @description This method returns the stroke tolerance of the vector. This stroke
*              tolerance is a quadrilateral representing an example surface located
*              usually near the vector location where two points located within
*              would be considered identical for stroke purposes. This stroke tolerance
*              is different from the resolution as returned by GetTolerance() which
*              returns a mathematical resolution for any coordinates of the vector.
*              The stroke tolerance is usually set using the SetStrokeTolerance() method
*              If none has been previously set, then one will be created at
*              the location of the vector extent center with a size of 100th of the
*              width and height of this extent. This arbitrarly created stroke tolerance
*              will not be automatically adjusted after initial generation.
* @return A smart pointer to the tolerance of the vector. This tolerance may not be
*         expressed in the vector coordinate system
* @bsimethod                                                    StephanePoulin  07/2003
+---------------+---------------+---------------+---------------+---------------+------*/
HFCPtr<HGFLiteTolerance> HGF2DVector::GetStrokeTolerance() const
    {
    if (m_pStrokeTolerance == NULL)
        {
        // There are no stroke tolerance set ... we will invent one judiciously
        HGF2DLiteExtent aExtent(GetExtent());

        double CenterX ((aExtent.GetXMin() + aExtent.GetXMax()) / 2.0);
        double CenterY ((aExtent.GetYMin() + aExtent.GetYMax()) / 2.0);
        double TolWidth (aExtent.GetWidth() / DEFAULT_STROKE_TOLERANCE_EXTENT_RATIO);
        double TolHeight(aExtent.GetHeight() / DEFAULT_STROKE_TOLERANCE_EXTENT_RATIO);

        // Make sure that widht or height are not smaller than resolution
        // An acceptable tolerance seems to be around 1/4 a pixels. Adding +/-
        // 1/8 of a pixel to the center gives 1/4 a pixel.
        TolWidth  = MAX(TolWidth  * DEFAULT_PIXEL_TOLERANCE, GetTolerance());
        TolHeight = MAX(TolHeight * DEFAULT_PIXEL_TOLERANCE, GetTolerance());

        m_pStrokeTolerance = new HGFLiteTolerance(CenterX - TolWidth, CenterY - TolHeight,
                                                  CenterX - TolWidth, CenterY + TolHeight,
                                                  CenterX + TolWidth, CenterY + TolHeight,
                                                  CenterX + TolWidth, CenterY - TolHeight);
        }

    return new HGFLiteTolerance (*m_pStrokeTolerance);
    }

/*---------------------------------------------------------------------------------**//**
* @description This method sets the stroke tolerance. Such tolerance is represented
*              as a small shape set into thre HGF tolerance with a specific reference
*              coordinate system. The linear tolerance is extracted after this small shape
*              has been tranformed into the comparison coordinate system. Thie present method
*              can be used to set or unset a stroke tolerance to the vector. Providing a NULL
*              pointer resets the stroke tolerance to undefined. It can be set afterwards
*              or be automatically recomputed the next time GetStrokeTolerance() is called.
*
* @param pi_Tolerance A reference to a smart pointer to a tolerance object. If a null
*                     pointer is provided then the storke is set to undefined.
*
* @see GetStrokeTolerance()
* @see HGFTolerance
* @bsimethod                                                    StephanePoulin  07/2003
+---------------+---------------+---------------+---------------+---------------+------*/
void HGF2DVector::SetStrokeTolerance(const HFCPtr<HGFLiteTolerance> & pi_Tolerance)
    {
    if (pi_Tolerance != NULL)
        m_pStrokeTolerance = new HGFLiteTolerance(*pi_Tolerance);
    else
        m_pStrokeTolerance = NULL;
    }


