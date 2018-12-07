//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF2DLiteSegment.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HGF2DLiteSegment
//-----------------------------------------------------------------------------

#include <ImageppInternal.h>

#include <ImagePP/all/h/HGF2DLiteSegment.h>
#include <ImagePP/all/h/HGF2DDisplacement.h>
#include <ImagePP/all/h/HGF2DLiteLine.h>


/** -----------------------------------------------------------------------------
    This method checks that self segment is crossed at junction point
    between the two given segment.
    The end point of the first segment must be equal to the start point of
    the second segment, and this point must be located on the self segment
    without being equal to an extremity.
    Finaly neither given segments must be contiguous to self.

    @param pi_rFirstSegment IN Constant reference to the first HGF2DLiteSegment.

    @param pi_rSecondSegment IN Constant reference to the second HGF2DLiteSegment.

    @return true if the segment intersects at the split point between the
            two given, and false otherwise.

    -----------------------------------------------------------------------------
*/
bool HGF2DLiteSegment::IntersectsAtSplitPoint(const HGF2DLiteSegment& pi_rFirstSegment,
                                               const HGF2DLiteSegment& pi_rSecondSegment) const
    {
    // The end point of first segment must be equal(Exactly) to start point of second segment
    HPRECONDITION(pi_rFirstSegment.GetEndPoint() == pi_rSecondSegment.GetStartPoint());

    // The split point must be on self
    HPRECONDITION(IsPointOn(pi_rFirstSegment.GetEndPoint()));

    // The split point must be different from an extremity of self
    HPRECONDITION(!pi_rFirstSegment.GetEndPoint().IsEqualTo(GetStartPoint(), m_Tolerance));
    HPRECONDITION(!pi_rFirstSegment.GetEndPoint().IsEqualTo(GetEndPoint(), m_Tolerance));

    // Neither segment must be contiguous to self
    HPRECONDITION(!AreContiguous(pi_rFirstSegment));
    HPRECONDITION(!AreContiguous(pi_rSecondSegment));

    // We first obtain the parametrized line equation at split point perpendicular to self
    // This equation is of the form Line = Anchor + K*(P - Anchor) in vectorial form
    double DeltaY = GetEndPoint().GetY() - pi_rFirstSegment.GetEndPoint().GetY();
    double DeltaX = GetEndPoint().GetX() - pi_rFirstSegment.GetEndPoint().GetX();

    // THE FOLLOWING ENABLES OBTAINING THE PERPENDICULAR
    double SwapVar = DeltaX;
    DeltaX = -DeltaY;
    DeltaY = SwapVar;

    // The anchor is the junction point
    double AnchorX = pi_rFirstSegment.GetEndPoint().GetX();
    double AnchorY = pi_rFirstSegment.GetEndPoint().GetY();

    // Compute slope square length
    double SlopeSquareLength = (DeltaX * DeltaX + DeltaY * DeltaY);

    // Obtain K value for each other points
    double K1;
    double K2;

    // In order to obtain the best precision we select according to delta values
    if (fabs(DeltaY) > fabs(DeltaX))
        {
        // Compute for start point of first segment
        double TempK1 = (DeltaX * (pi_rFirstSegment.GetStartPoint().GetY() - AnchorY) +
                          DeltaY * (pi_rFirstSegment.GetStartPoint().GetX() - AnchorX))
                         / (-SlopeSquareLength);
        double TempY1 = pi_rFirstSegment.GetStartPoint().GetY() + TempK1 * DeltaX;
        K1 = (TempY1 - AnchorY) / DeltaY;

        // Compute for end point of second segment
        double TempK2 = (DeltaX * (pi_rSecondSegment.GetEndPoint().GetY() - AnchorY) +
                          DeltaY * (pi_rSecondSegment.GetEndPoint().GetX() - AnchorX))
                         / (-SlopeSquareLength);

        double TempY2 = pi_rSecondSegment.GetEndPoint().GetY() + TempK2 * DeltaX;
        K2 = (TempY2 - AnchorY) / DeltaY;
        }
    else
        {
        // Compute for start point of first segment
        double TempK1 = (DeltaX * (pi_rFirstSegment.GetStartPoint().GetY() - AnchorY) +
                          DeltaY * (pi_rFirstSegment.GetStartPoint().GetX() - AnchorX))
                         / (-SlopeSquareLength);

        double TempX1 = pi_rFirstSegment.GetStartPoint().GetX() - TempK1 * DeltaY;
        K1 = (TempX1 - AnchorX) / DeltaX;

        // Compute for end point of second segment
        double TempK2 = (DeltaX * (pi_rSecondSegment.GetEndPoint().GetY() - AnchorY) +
                          DeltaY * (pi_rSecondSegment.GetEndPoint().GetX() - AnchorX))
                         / (-SlopeSquareLength);

        double TempX2 = pi_rSecondSegment.GetEndPoint().GetX() - TempK2 * DeltaY;
        K2 = (TempX2 - AnchorX) / DeltaX;
        }

    // If the two K values have different signs then the segments cross at split point
    return(K1 * K2 < 0.0);
    }

/** -----------------------------------------------------------------------------
    Checks the self segment and given segments are adjacent to one another.

    @param pi_rSegment IN Constant reference to the segment to check adjacence.

    @return true if the segments are adjacent.

    @see AreContiguous()
    -----------------------------------------------------------------------------
*/
bool HGF2DLiteSegment::AreAdjacent(const HGF2DLiteSegment& pi_rSegment) const
    {
    // To be adjacent, the two segments must either be contiguous or
    // link to each other and have the same slope (be parrallel)

    bool Result = false;

    // Check if contiguous ==> Adjacent
    if (AreContiguous(pi_rSegment))
        {
        Result = true;
        }
    // They are not contiguous ...check if they link
    else if (LinksTo(pi_rSegment))
        {
        Result = AreParallel(pi_rSegment);


        }

    return(Result);
    }


/** -----------------------------------------------------------------------------
    Checks the self segment and given segments are parallel to one another.

    @param pi_rSegment IN Constant reference to the segment to check parallelitude.

    @return true if the segments are parallel and false otherwise.

    @see AreContiguous()
    -----------------------------------------------------------------------------
*/
bool HGF2DLiteSegment::AreParallel(const HGF2DLiteSegment& pi_rSegment,
                                    double pi_SlopeTolerance) const
    {


    double SelfDx;
    double SelfDy;

    double GivenDx;
    double GivenDy;

    GetDeltas(&SelfDx, &SelfDy);

    pi_rSegment.GetDeltas(&GivenDx, &GivenDy);

    double K1 = (SelfDy * GivenDx - SelfDx * GivenDy);

    // ATTENTION: THE FIRST CONDITION MUST REMAIN SINCE fabs(0) MAY BE SLIGHTLY DIFFERENT THAN 0
    // FOR TOLERANCE == 0 THE CONDITION IS NECESSARY
    bool DoAreParallel = (K1 == 0.0) || (fabs(K1) < pi_SlopeTolerance);

    return DoAreParallel;

    }

/** -----------------------------------------------------------------------------
    Checks the self segment and given segments are contiguous to one another.

    @param pi_rSegment IN Constant reference to the segment to check contiguousness.

    @return true if the segments are contiguous and false otherwise.

    @see AreAdjacent()
    @see ObtainContiguousnessPoints()
    -----------------------------------------------------------------------------
*/
bool HGF2DLiteSegment::AreContiguous(const HGF2DLiteSegment& pi_rSegment) const
    {
    // The process of determining if two segments are contiguous relies eavily
    // on the ability of a segment to check if a point is located on

    // HChk AR Tolerance is not applied everywhere in the same manner
    //         For example, IsPointOn does not require a tolerance.
    //         ObtainContiguousPoints uses bad tolerance...
    double Tolerance = MIN(m_Tolerance, pi_rSegment.m_Tolerance);

    int32_t NumberOfContiguousnessPoints = 0;

    if (!GetStartPoint().IsEqualTo(GetEndPoint(), Tolerance) &&
        !pi_rSegment.GetStartPoint().IsEqualTo(pi_rSegment.GetEndPoint(), Tolerance))
        {
        // We check if the start points are equal
        if (GetStartPoint().IsEqualTo(pi_rSegment.GetStartPoint(), Tolerance))
            {
            // The two start points are on top of each other
            // It is therefore one of the contiguousness points
            NumberOfContiguousnessPoints++;

            // The second point is either one of the end points

            // Check if the end points are equal
            if (GetEndPoint().IsEqualTo(pi_rSegment.GetEndPoint(), Tolerance))
                NumberOfContiguousnessPoints++;
            else
                {
                // Since the end points are not equal, then the one that is on the other segment
                // is the result
                if (IsPointOn(pi_rSegment.GetEndPoint()))
                    NumberOfContiguousnessPoints++;
                else if(pi_rSegment.IsPointOn(GetEndPoint()))
                    NumberOfContiguousnessPoints++;
                }
            }
        else if (GetEndPoint().IsEqualTo(pi_rSegment.GetEndPoint(), Tolerance))
            {
            // The two end points are on top of each other

            // The other point is either one of the start point
            // the one that is on the other segment is the result
            if (IsPointOn(pi_rSegment.GetStartPoint()))
                NumberOfContiguousnessPoints++;
            else if (pi_rSegment.IsPointOn(GetStartPoint()))
                NumberOfContiguousnessPoints++;

            // The end point is therefore one of the contiguousness points
            NumberOfContiguousnessPoints++;

            }
        else if (GetStartPoint().IsEqualTo(pi_rSegment.GetEndPoint(), Tolerance))
            {
            // The self start point in on top of the given end point
            // It is therefore one of the contiguousness points
            NumberOfContiguousnessPoints++;

            // The second point is either one of the other point

            // Check if the other extremity points are equal
            if (GetEndPoint().IsEqualTo(pi_rSegment.GetStartPoint(), Tolerance))
                NumberOfContiguousnessPoints++;
            else
                {
                // Since the those points are not equal, then the one that is on the other segment
                // is the result
                if (IsPointOn(pi_rSegment.GetStartPoint()))
                    NumberOfContiguousnessPoints++;
                else if (pi_rSegment.IsPointOn(GetEndPoint()))
                    NumberOfContiguousnessPoints++;
                }
            }
        else if (GetEndPoint().IsEqualTo(pi_rSegment.GetStartPoint(), Tolerance))
            {
            // The self end point in on top of the given start point

            // The second point is either one of the other extremity point
            // the one that is on the other segment is the result
            if (IsPointOn(pi_rSegment.GetEndPoint()))
                NumberOfContiguousnessPoints++;
            else if (pi_rSegment.IsPointOn(GetStartPoint()))
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
            if (pi_rSegment.IsPointOn(GetStartPoint()))
                NumberOfContiguousnessPoints++;

            // The two segments are oriented likewise ...
            // we check in start to end order
            if (IsPointOn(pi_rSegment.GetStartPoint()))
                NumberOfContiguousnessPoints++;
            if (IsPointOn(pi_rSegment.GetEndPoint()))
                NumberOfContiguousnessPoints++;

            // The last possible point is end point
            if (pi_rSegment.IsPointOn(GetEndPoint()))
                NumberOfContiguousnessPoints++;
            }
        }

    return(NumberOfContiguousnessPoints == 2);
    }




/** -----------------------------------------------------------------------------
    The method returns all the contiguousness points between self and the given
    segment. Contiguousness points are the start and end points of contiguousness
    regions between two segment.  The segment must be contiguous to call these
    methods. The first indicates the start of a contiguousness region, the
    second indicates the end of this same contiguousness region. There will of
    course be exactly 2 contiguousness points returned.

    @param pi_rSegment IN Constant reference to the segment to obtain
                          contiguousness points with.

    @return The number of new contiguousness points found (2)

    @see AreContiguous()
    -----------------------------------------------------------------------------
*/
size_t HGF2DLiteSegment::ObtainContiguousnessPoints(const HGF2DLiteSegment& pi_rSegment,
                                                    HGF2DPositionCollection* po_pContiguousnessPoints) const
    {
    HPRECONDITION(po_pContiguousnessPoints != 0);

    // The segments must be contiguous
    HPRECONDITION(AreContiguous(pi_rSegment));

    // When contiguous, two segments will have exactly 2
    // contiguousness points

    // Save initial number of points in list
    size_t  InitialNumberOfPoints = po_pContiguousnessPoints->size();


    // We check if the start points are equal
    if (GetStartPoint().IsEqualTo(pi_rSegment.GetStartPoint(), m_Tolerance))
        {
        // The two start points are on top of each other
        // It is therefore one of the contiguousness points
        po_pContiguousnessPoints->push_back(GetStartPoint());

        // The second point is either one of the end points

        // Check if the end points are equal
        if (GetEndPoint().IsEqualTo(pi_rSegment.GetEndPoint(), m_Tolerance))
            po_pContiguousnessPoints->push_back(GetEndPoint());
        else
            {
            // Since the end points are not equal, then the one that is on the other segment
            // is the result
            if (IsPointOn(pi_rSegment.GetEndPoint()))
                po_pContiguousnessPoints->push_back(pi_rSegment.GetEndPoint());
            else
                po_pContiguousnessPoints->push_back(GetEndPoint());
            }
        }
    else if (GetEndPoint().IsEqualTo(pi_rSegment.GetEndPoint(), m_Tolerance))
        {
        // The two end points are on top of each other

        // The other point is either one of the start point
        // the one that is on the other segment is the result
        if (IsPointOn(pi_rSegment.GetStartPoint()))
            po_pContiguousnessPoints->push_back(pi_rSegment.GetStartPoint());
        else
            po_pContiguousnessPoints->push_back(GetStartPoint());

        // The end point is therefore one of the contiguousness points
        po_pContiguousnessPoints->push_back(GetEndPoint());

        }
    else if (GetStartPoint().IsEqualTo(pi_rSegment.GetEndPoint(), m_Tolerance))
        {
        // The self start point in on top of the given end point
        // It is therefore one of the contiguousness points
        po_pContiguousnessPoints->push_back(GetStartPoint());

        // The second point is either one of the other point

        // Check if the other extremity points are equal
        if (GetEndPoint().IsEqualTo(pi_rSegment.GetStartPoint(), m_Tolerance))
            po_pContiguousnessPoints->push_back(GetEndPoint());
        else
            {
            // Since the those points are not equal, then the one that is on the other segment
            // is the result
            if (IsPointOn(pi_rSegment.GetStartPoint()))
                po_pContiguousnessPoints->push_back(pi_rSegment.GetStartPoint());
            else
                po_pContiguousnessPoints->push_back(GetEndPoint());
            }
        }
    else if (GetEndPoint().IsEqualTo(pi_rSegment.GetStartPoint(), m_Tolerance))
        {
        // The self end point in on top of the given start point

        // The second point is either one of the other extremity point
        // the one that is on the other segment is the result
        if (IsPointOn(pi_rSegment.GetEndPoint()))
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
        if (pi_rSegment.IsPointOn(GetStartPoint()))
            po_pContiguousnessPoints->push_back(GetStartPoint());

        // Check if the segments are oriented in the same direction
        bool SameDirection;

            {
            // Obtain first line parameters
            double SelfSlope;
            double GivenSlope;

            // Check if segment vertical
            if (GetStartPoint().GetX() == GetEndPoint().GetX())
                SelfSlope = GetStartPoint().GetY() - GetEndPoint().GetY();
            else
                SelfSlope = (GetStartPoint().GetY() - GetEndPoint().GetY()) /
                            (GetStartPoint().GetX() - GetEndPoint().GetX());

            // Obtain second line parameters

            // Check if segment vertical
            if (pi_rSegment.GetStartPoint().GetX() == pi_rSegment.GetEndPoint().GetX())
                GivenSlope = pi_rSegment.GetStartPoint().GetY() - pi_rSegment.GetEndPoint().GetY();
            else
                GivenSlope = (pi_rSegment.GetStartPoint().GetY() - pi_rSegment.GetEndPoint().GetY()) /
                             (pi_rSegment.GetStartPoint().GetX() - pi_rSegment.GetEndPoint().GetX());

            // If the slopes have the same sign, then they are in the same direction
            SameDirection = (SelfSlope * GivenSlope > 0.0);
            }
        if (SameDirection)
            {
            // The two segments are oriented likewise ...
            // we check in start to end order
            if (IsPointOn(pi_rSegment.GetStartPoint()))
                po_pContiguousnessPoints->push_back(pi_rSegment.GetStartPoint());
            if (IsPointOn(pi_rSegment.GetEndPoint()))
                po_pContiguousnessPoints->push_back(pi_rSegment.GetEndPoint());
            }
        else
            {
            // The two segments are oriented differently
            // we check in end to start order
            if (IsPointOn(pi_rSegment.GetEndPoint()))
                po_pContiguousnessPoints->push_back(pi_rSegment.GetEndPoint());
            if (IsPointOn(pi_rSegment.GetStartPoint()))
                po_pContiguousnessPoints->push_back(pi_rSegment.GetStartPoint());
            }

        // The last possible point is end point
        if (pi_rSegment.IsPointOn(GetEndPoint()))
            po_pContiguousnessPoints->push_back(GetEndPoint());
        }

    return (po_pContiguousnessPoints->size() - InitialNumberOfPoints);
    }





/** -----------------------------------------------------------------------------
    This method determined if the given point is located on the segment. This
    method applies automatically the segment tolerance.
    Extremities of the segment can be excluded of the check by using the
    optional parameter pi_ExtremityProcessing.

    @param pi_rTestPoint IN Constant reference to test point.

    @param pi_ExtremityProcessing IN OPTIONAL Defaults to GFD2DLiteSegment::INCLUDE_EXTREMITIES
                                    The other possible value is HGF2DLiteSegment::EXCLUDE_EXTREMITIES.
                                    Indicates if extremities of the segment should be included
                                    or excluded from the test.

    @return true if point is located on the segment and false otherwise.
    -----------------------------------------------------------------------------
*/
bool   HGF2DLiteSegment::IsPointOn(const HGF2DPosition& pi_rTestPoint, ExtremityProcessing pi_ExtremityProcessing) const
    {
    // To check if a point is on segment, it must first be included in the outter
    // extent of segment.
    // If this is true, then the distance from point to the line
    // the segment is part of must be smaller than tolerance
    // The computation of distance are base on a mathematical equation
    // Found in "Formules et tables de mathematique"
    // Experience has proven that this equation is far more
    // precise than any other one studied
    bool   Answer = false;

    // Obtain extremes of segment
    double XMin = MIN(GetStartPoint().GetX(), GetEndPoint().GetX());
    double XMax = MAX(GetStartPoint().GetX(), GetEndPoint().GetX());
    double YMin = MIN(GetStartPoint().GetY(), GetEndPoint().GetY());
    double YMax = MAX(GetStartPoint().GetY(), GetEndPoint().GetY());

    double X = pi_rTestPoint.GetX();
    double Y = pi_rTestPoint.GetY();

    // A point is on if it is within the extended extent.
    if (HDOUBLE_GREATER_OR_EQUAL(X, XMin, m_Tolerance) &&
        HDOUBLE_SMALLER_OR_EQUAL(X, XMax, m_Tolerance) &&
        HDOUBLE_GREATER_OR_EQUAL(Y, YMin, m_Tolerance) &&
        HDOUBLE_SMALLER_OR_EQUAL(Y, YMax, m_Tolerance))
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
#if (0)
            // Non vertical and non horizontal segment
            // Obtain slope and intercept of line
            double SelfSlope;
            double SelfIntercept;
            bool InvertSlope = false;

            SelfSlope = ((GetStartPoint().GetY() - GetEndPoint().GetY())) /
                        (GetStartPoint().GetX() - GetEndPoint().GetX());

            if (SelfSlope > 1.0)
                {
                InvertSlope = true;
                SelfSlope = ((GetStartPoint().GetX() - GetEndPoint().GetX())) /
                            (GetStartPoint().GetY() - GetEndPoint().GetY());

                SelfIntercept = GetStartPoint().GetX() - (GetStartPoint().GetY() * SelfSlope);
                }
            else
                {
                SelfIntercept = GetStartPoint().GetY() - (GetStartPoint().GetX() * SelfSlope);
                }

            // Calculate distance from point to line
            double DistanceToLine;
            if (InvertSlope)
                {
                DistanceToLine = (SelfSlope * pi_rTestPoint.GetY() - pi_rTestPoint.GetX() +
                                  SelfIntercept) /
                                 sqrt(SelfSlope * SelfSlope + 1);
                }
            else
                {
                DistanceToLine = (SelfSlope * pi_rTestPoint.GetX() - pi_rTestPoint.GetY() +
                                  SelfIntercept) /
                                 sqrt(SelfSlope * SelfSlope + 1);
                }

            // Check if distance is smaller than epsilon.
            // Being within the extent and closer than epsilon of line, the
            // point will be ON
            Answer = HDOUBLE_EQUAL(DistanceToLine, 0.0, m_Tolerance);
#else
            double x1;
            double y1;
            double x2;
            double y2;

            x1 = GetEndPoint().GetX() - GetStartPoint().GetX();
            y1 = GetEndPoint().GetY() - GetStartPoint().GetY();
            x2 = X - GetStartPoint().GetX();
            y2 = Y - GetStartPoint().GetY();

            double Det = ((x1*y2) - (x2*y1)); // Determinant is 2 times the area of the triangle formed by three points.

            double Length1 = sqrt (x1*x1 + y1*y1);


            double AreaTolerance = Length1 * GetTolerance();

            Answer = (HNumeric<double>::EQUAL (Det, 0.0, AreaTolerance));
#endif
            }

        // Check if extremities must be excluded
        if ((Answer) && (pi_ExtremityProcessing == HGF2DLiteSegment::EXCLUDE_EXTREMITIES))
            {
            // The caller wishes to exclude extremities from operation
            // We check it is different from extremity
            Answer = (!GetStartPoint().IsEqualTo(pi_rTestPoint, m_Tolerance) &&
                      !GetEndPoint().IsEqualTo(pi_rTestPoint, m_Tolerance));
            }

        if ((!Answer) && (pi_ExtremityProcessing == HGF2DLiteSegment::INCLUDE_EXTREMITIES))
            {
            Answer = (GetStartPoint().IsEqualTo(pi_rTestPoint, m_Tolerance) ||
                      GetEndPoint().IsEqualTo(pi_rTestPoint, m_Tolerance));
            }
        }

    return(Answer);
    }


/** -----------------------------------------------------------------------------
    Checks if given points are located on different sides of the segment.

    @param pi_rPoint1 IN Constant reference to first test point.

    @param pi_rPoint2 IN Constant reference to second test point.


    @return true if points are located on different sides of segment.
    -----------------------------------------------------------------------------
*/
bool HGF2DLiteSegment::ArePointsOnDifferentSides(const HGF2DPosition& pi_rPoint1, const HGF2DPosition& pi_rPoint2) const
    {
    bool ResultPoint1;
    bool ResultPoint2;

    // Check if segment vertical
    if (HDOUBLE_EQUAL(GetStartPoint().GetX(), GetEndPoint().GetX(), m_Tolerance))
        {
        if (HDOUBLE_EQUAL(pi_rPoint1.GetX(), GetStartPoint().GetX(), m_Tolerance) ||
            HDOUBLE_EQUAL(pi_rPoint2.GetX(), GetStartPoint().GetX(), m_Tolerance))
            {
            // Arbitrary. Same value = not on different sides.
            ResultPoint1 = true;
            ResultPoint2 = true;
            }
        else
            {
            ResultPoint1 = HDOUBLE_GREATER(pi_rPoint1.GetX(), GetStartPoint().GetX(), m_Tolerance);
            ResultPoint2 = HDOUBLE_GREATER(pi_rPoint2.GetX(), GetStartPoint().GetX(), m_Tolerance);
            }
        }
    else
        {
        double SelfSlope = (GetStartPoint().GetY() - GetEndPoint().GetY()) /
                            (GetStartPoint().GetX() - GetEndPoint().GetX());
        double SelfIntercept = GetStartPoint().GetY() - (GetStartPoint().GetX() * SelfSlope);

        if (SelfSlope > 1.0 || SelfSlope < -1.0)
            {
            double CalculatedX1 = (pi_rPoint1.GetY() - SelfIntercept) / SelfSlope;
            double CalculatedX2 = (pi_rPoint2.GetY() - SelfIntercept) / SelfSlope;

            if (HDOUBLE_EQUAL(pi_rPoint1.GetX(), CalculatedX1, m_Tolerance) ||
                HDOUBLE_EQUAL(pi_rPoint2.GetX(), CalculatedX2, m_Tolerance))
                {
                // Arbitrary. Same value = not on different sides.
                ResultPoint1 = true;
                ResultPoint2 = true;
                }
            else
                {
                ResultPoint1 = HDOUBLE_GREATER(pi_rPoint1.GetX(), CalculatedX1, m_Tolerance);
                ResultPoint2 = HDOUBLE_GREATER(pi_rPoint2.GetX(), CalculatedX2, m_Tolerance);
                }
            }
        else
            {
            double CalculatedY1 = SelfSlope * pi_rPoint1.GetX() + SelfIntercept;
            double CalculatedY2 = SelfSlope * pi_rPoint2.GetX() + SelfIntercept;

            if (HDOUBLE_EQUAL(pi_rPoint1.GetY(), CalculatedY1, m_Tolerance) ||
                HDOUBLE_EQUAL(pi_rPoint2.GetY(), CalculatedY2, m_Tolerance))
                {
                // Arbitrary. Same value = not on different sides.
                ResultPoint1 = true;
                ResultPoint2 = true;
                }
            else
                {
                ResultPoint1 = HDOUBLE_GREATER(pi_rPoint1.GetY(), CalculatedY1, m_Tolerance);
                ResultPoint2 = HDOUBLE_GREATER(pi_rPoint2.GetY(), CalculatedY2, m_Tolerance);
                }
            }
        }

    return (ResultPoint1 != ResultPoint2);
    }

//-----------------------------------------------------------------------------
// operator==
//-----------------------------------------------------------------------------
bool HGF2DLiteSegment::operator==(const HGF2DLiteSegment& pi_rObj) const
    {
    return ((GetStartPoint().IsEqualTo(pi_rObj.GetStartPoint()) || GetStartPoint().IsEqualTo(pi_rObj.GetEndPoint())) &&
            (GetEndPoint().IsEqualTo(pi_rObj.GetEndPoint()) || GetEndPoint().IsEqualTo(pi_rObj.GetStartPoint())));
    }






/** -----------------------------------------------------------------------------
    This method calculates and returns the crossing point between the given
    segment.

    It is possible that no crossing point exists. In that case, the state of
    the returned point is undefined, and the returned state indicates if crossing
    could be performed, and for what reason it could not. Two reasons are possible
    for not crossing. If the segments are parallel, or if they are disjoint.
    If their extents outer overlap, then there is automatically a NO_CROSS
    state returned. In this, the lite segment differs greatly from the
    HVE2DSegment from which it was spawned. If the outer extents overlap,
    then the parallel condition is detected before checking if they are disjoint,
    so a parallel status does not imply that the segments are disjoint
    or not. It follows that two segment one on top of the other do not cross.

    @param pi_rSegment IN Constant reference to segment to intersect with.

    @param po_pPoint OUT Pointer to an HGF2DPosition that receives the cross
                     point if any.


    @return The status of the crossing operation. This status is
            HGF2DLiteSegment::CROSS_FOUND if a crossing point is found,
            HGF2DLiteSegment::PARALLEL if the segment is parallel to the segment
            or line given or HGF2DLiteSegment::NO_CROSS if no crossing point exists
            between the segment and the given segment. Note that the value of the
            HGF2DPosition pointed to by po_pPoint is undefined if the returned
            value is different from HGF2DLiteSegment::CROSS_FOUND.
    -----------------------------------------------------------------------------
*/
HGF2DLiteSegment::CrossState HGF2DLiteSegment::IntersectSegment(
    const HGF2DLiteSegment& pi_rSegment,
    HGF2DPosition* po_pPoint) const
    {
    HPRECONDITION(po_pPoint != 0);

    // Create return object set initialy to no crosses
    HGF2DLiteSegment::CrossState    Status = HGF2DLiteSegment::NO_CROSS;
    bool IntersectsAtExtremity = false;

    Status = IntersectSegmentExtremityIncluded(pi_rSegment, po_pPoint, &IntersectsAtExtremity);

    if (Status == CROSS_FOUND && IntersectsAtExtremity)
        Status = NO_CROSS;

    return (Status);
    }



/** -----------------------------------------------------------------------------
    This method calculates and returns the crossing point between the given
    segment.

    This method is identical to the IntersectSegment() method except that it
    returns a cross when the cross point is located upon one of the segments end point
    and optional returns this fact in the optional parameter.

    Refer to IntersectSegment for details.

    @param pi_rSegment IN Constant reference to segment to intersect with.

    @param po_pPoint OUT Pointer to an HGF2DPosition that receives the cross
                     point if any.

    @param po_pIntersectsAtExtremity OUT OPTIONAL Pointer to bool that will receive
                    an indication if the cross point is located on an extremity.
                    If there is no cross the content of the returned value is
                    undefined.

    @return The status of the crossing operation. This status is
            HGF2DLiteSegment::CROSS_FOUND if a crossing point is found,
            HGF2DLiteSegment::PARALLEL if the segment is parallel to the segment
            or line given or HGF2DLiteSegment::NO_CROSS if no crossing point exists
            between the segment and the given segment. Note that the value of the
            HGF2DPosition pointed to by po_pPoint is undefined if the returned
            value is different from HGF2DLiteSegment::CROSS_FOUND.
    -----------------------------------------------------------------------------
*/
HGF2DLiteSegment::CrossState HGF2DLiteSegment::IntersectSegmentExtremityIncluded(
    const HGF2DLiteSegment& pi_rSegment,
    HGF2DPosition* po_pPoint,
    bool* po_pIntersectsAtExtremity) const
    {
    HPRECONDITION(po_pPoint != 0);

    // Create return object set initialy to no crosses
    HGF2DLiteSegment::CrossState    Status = HGF2DLiteSegment::NO_CROSS;

    // Compute effective tolerance (smallest of the two given segment)
    double EffectiveTolerance = MIN(m_Tolerance, pi_rSegment.m_Tolerance);

    // Obtain extremes of self segment
    double XMin1 = MIN(GetStartPoint().GetX(), GetEndPoint().GetX());
    double XMax1 = MAX(GetStartPoint().GetX(), GetEndPoint().GetX());
    double YMin1 = MIN(GetStartPoint().GetY(), GetEndPoint().GetY());
    double YMax1 = MAX(GetStartPoint().GetY(), GetEndPoint().GetY());

    // Obtain extremes of given segment
    double XMin2 = MIN(pi_rSegment.GetStartPoint().GetX(), pi_rSegment.GetEndPoint().GetX());
    double XMax2 = MAX(pi_rSegment.GetStartPoint().GetX(), pi_rSegment.GetEndPoint().GetX());
    double YMin2 = MIN(pi_rSegment.GetStartPoint().GetY(), pi_rSegment.GetEndPoint().GetY());
    double YMax2 = MAX(pi_rSegment.GetStartPoint().GetY(), pi_rSegment.GetEndPoint().GetY());


    // Check if their extent overlaps (outter overlap)
    bool Result = (HDOUBLE_GREATER(XMax1, XMin2, EffectiveTolerance) &&
                    HDOUBLE_SMALLER(XMin1, XMax2, EffectiveTolerance) &&
                    HDOUBLE_GREATER(YMax1, YMin2, EffectiveTolerance) &&
                    HDOUBLE_SMALLER(YMin1, YMax2, EffectiveTolerance));

    if (Result)
        {

        Status = PARALLEL;
        double SelfDx;
        double SelfDy;

        double GivenDx;
        double GivenDy;

        // Obtain the deltas of both segments
        GetDeltas(&SelfDx, &SelfDy);
        pi_rSegment.GetDeltas(&GivenDx, &GivenDy);

        double K1 = (SelfDy * GivenDx - SelfDx * GivenDy);

        // Check if lines are parallel
        if (K1 == 0.0)
            {
            Status = PARALLEL;
            }
        else
            {
            // Since the segments are not parallel, then K1 cannot be 0.0

            double K2 = (SelfDx * (pi_rSegment.GetStartPoint().GetY() - GetStartPoint().GetY()) +
                          SelfDy * (GetStartPoint().GetX() - pi_rSegment.GetStartPoint().GetX())) / K1;

            po_pPoint->SetX(pi_rSegment.GetStartPoint().GetX() + (K2 * GivenDx));
            po_pPoint->SetY(pi_rSegment.GetStartPoint().GetY() + (K2 * GivenDy));


            // The lines do cross ... now we check if the cross point is located
            // on the segments
            double X = po_pPoint->GetX();
            double Y = po_pPoint->GetY();

            if ((((X >= XMin1) || HDOUBLE_EQUAL(X, XMin1, EffectiveTolerance)) &&
                 ((X <= XMax1) || HDOUBLE_EQUAL(X, XMax1, EffectiveTolerance)) &&
                 ((Y >= YMin1) || HDOUBLE_EQUAL(Y, YMin1, EffectiveTolerance)) &&
                 ((Y <= YMax1) || HDOUBLE_EQUAL(Y, YMax1, EffectiveTolerance))) &&
                (((X >= XMin2) || HDOUBLE_EQUAL(X, XMin2, EffectiveTolerance)) &&
                 ((X <= XMax2) || HDOUBLE_EQUAL(X, XMax2, EffectiveTolerance)) &&
                 ((Y >= YMin2) || HDOUBLE_EQUAL(Y, YMin2, EffectiveTolerance)) &&
                 ((Y <= YMax2) || HDOUBLE_EQUAL(Y, YMax2, EffectiveTolerance))))
                {
                if (!AreContiguous(pi_rSegment))
                    {
                    // The point is part of both segment ... a cross has been found
                    Status = CROSS_FOUND;

                    if (NULL != po_pIntersectsAtExtremity)
                        {
                        // We want to know if the segments intersect at an extremity
                        if ((!po_pPoint->IsEqualTo(GetStartPoint(), EffectiveTolerance)) &&
                            (!po_pPoint->IsEqualTo(GetEndPoint(), EffectiveTolerance)) &&
                            (!po_pPoint->IsEqualTo(pi_rSegment.GetStartPoint(), EffectiveTolerance)) &&
                            (!po_pPoint->IsEqualTo(pi_rSegment.GetEndPoint(), EffectiveTolerance)) &&
                            (!LinksTo(pi_rSegment)))
                            {
                            // The intersection is not at an extremity
                            *po_pIntersectsAtExtremity = false;
                            }
                        else
                            {
                            // The intersection is located upon an extremity.
                            *po_pIntersectsAtExtremity = true;
                            }
                        }

                    }
                else
                    {
                    Status = NO_CROSS;
                    }
                }
            else
                Status = NO_CROSS;
            }
        }

    return (Status);
    }




//-----------------------------------------------------------------------------
// PRIVATE
// GetDeltas
// This private method returns the normalized deltas for the segments
// These deltas are a good representation of the slope of the segment
// used in various calculations.
//-----------------------------------------------------------------------------
void HGF2DLiteSegment::GetDeltas(double* po_pDx,
                                 double* po_pDy) const
    {
    // Recipient variables must be provided
    HPRECONDITION(po_pDx != 0);
    HPRECONDITION(po_pDy != 0);

    // Calculate deltas based on points
    double Dx = GetEndPoint().GetX() - GetStartPoint().GetX();
    double Dy = GetEndPoint().GetY() - GetStartPoint().GetY();

    // Obtain absolute values of both deltas
    double AbsDx = fabs(Dx);
    double AbsDy = fabs(Dy);

    // Normalize deltas according to greatest delta X or Y
    if (AbsDx > AbsDy)
        {
        Dx /= AbsDx;
        Dy /= AbsDx;
        }
    else
        {
        Dx /= AbsDy;
        Dy /= AbsDy;
        }

    // Return result
    *po_pDx = Dx;
    *po_pDy = Dy;


    }


//-----------------------------------------------------------------------------
// CalculateClosestPoint
// Calculates the closest point on segment to given point
//-----------------------------------------------------------------------------
HGF2DPosition HGF2DLiteSegment::CalculateClosestPoint(const HGF2DPosition& pi_rPoint) const
    {
    // Calculate closest point to line the segment is part of
    HGF2DPosition   ClosestPoint(HGF2DLiteLine(GetStartPoint(), GetEndPoint()).CalculateClosestPoint(pi_rPoint));

    // Check if this point is not located on the segment
    if (!IsPointOnLineOnSegment(ClosestPoint))
        {
        // Calculate distances from extremities to point
        double FromStart = GetStartPoint().CalculateLengthTo(pi_rPoint);
        double FromEnd = GetEndPoint().CalculateLengthTo(pi_rPoint);

        // Check if distances are equal
        // KEEP AS OPERATOR==
        if (FromStart == FromEnd)
            {
            // The distances are identical ... may be a precision problem ...
            // Check if length of segment is zero
            if (CalculateLength() == 0.0)
                {
                // We have a null segment ... any point will do
                ClosestPoint =  GetStartPoint();
                }
            else
                {
                // We have a precision problem ...
                // We know that the tentative closest point is very far from either start
                // or end point, and is aligned to it ... we use bearings if not infinity
                if (!BeNumerical::BeFinite(FromStart))
                    ClosestPoint = GetStartPoint();
                else
                    {
                    HGF2DDisplacement displacement1(GetStartPoint().GetX() - ClosestPoint.GetX(), GetStartPoint().GetY() - ClosestPoint.GetY());
                    HGFBearing   FromStartBearing = displacement1.CalculateBearing();
                    HGF2DDisplacement displacement2(GetEndPoint().GetX() - GetStartPoint().GetX(), GetEndPoint().GetY() - GetStartPoint().GetY());
                    HGFBearing   SegmentBearing = displacement2.CalculateBearing();

                    ClosestPoint = ((FromStartBearing.IsEqualTo(SegmentBearing)) ? GetStartPoint() : GetEndPoint());
                    }
                }
            }
        else
            {
            // Since the closest point on line is not located on segment, then the
            // closest point is the closest of the start point or the end point.
            ClosestPoint = ((FromStart < FromEnd) ? GetStartPoint() : GetEndPoint());
            }
        }

    return (ClosestPoint);
    }


//-----------------------------------------------------------------------------
// IsPointOnLineOnSegmentSCS
// Static method
// Checks if the point is located on the segment knowing it is on the line
//-----------------------------------------------------------------------------
bool   HGF2DLiteSegment::IsPointOnLineOnSegment(const HGF2DPosition& pi_rTestPoint) const
    {
    double X = pi_rTestPoint.GetX();
    double Y = pi_rTestPoint.GetY();

    // Obtain extremes of segment
    double XMin = MIN(GetStartPoint().GetX(), GetEndPoint().GetX());
    double XMax = MAX(GetStartPoint().GetX(), GetEndPoint().GetX());
    double YMin = MIN(GetStartPoint().GetY(), GetEndPoint().GetY());
    double YMax = MAX(GetStartPoint().GetY(), GetEndPoint().GetY());

    double Tolerance = GetTolerance();

    // A point is on if it is within the extended extent.
    return (HDOUBLE_GREATER_OR_EQUAL(X, XMin, Tolerance) &&
            HDOUBLE_SMALLER_OR_EQUAL(X, XMax, Tolerance) &&
            HDOUBLE_GREATER_OR_EQUAL(Y, YMin, Tolerance) &&
            HDOUBLE_SMALLER_OR_EQUAL(Y, YMax, Tolerance));
    }


//-----------------------------------------------------------------------------
// CalculateLength
// Calculates and returns the length of segment
//-----------------------------------------------------------------------------
double HGF2DLiteSegment::CalculateLength() const
    {
    double DeltaX(GetEndPoint().GetX()-GetStartPoint().GetX());
    double DeltaY(GetEndPoint().GetY()-GetStartPoint().GetY());

    return sqrt((DeltaX * DeltaX) + (DeltaY * DeltaY));
    }

