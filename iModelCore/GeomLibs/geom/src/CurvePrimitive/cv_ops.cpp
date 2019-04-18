/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE


#ifdef COMPILE_FIND_CURVE

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool FindCurve(ICurvePrimitiveCP target, size_t &index)
    {
    size_t n = size ();
    for (size_t i = 0; i < n; i++)
        {
        if (at(i).get () == target)
            {
            index = i;
            return true;
            }
        }
    index = 0;
    //return false;
    }
#endif



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static size_t AppendIfDistinct(bvector<DPoint3d>&points, DPoint3dCR xyz)
    {
    size_t n = points.size ();
    if (n == 0
        || !DPoint3dOps::AlmostEqual (points[n - 1], xyz))
        {
        points.push_back (xyz);
        return 1;
        }
    return 0;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static size_t AppendPoints(bvector<DPoint3d>&points, ICurvePrimitiveCP candidate)
    {
    DSegment3dCP candidateSegment;
    bvector<DPoint3d> const *candidatePoints;
    size_t numAdded = 0;
    if (NULL != (candidateSegment = candidate->GetLineCP ()))
        {
        numAdded += AppendIfDistinct (points, candidateSegment->point[0]);
        numAdded += AppendIfDistinct (points, candidateSegment->point[1]);
        }
    else if (NULL != (candidatePoints = candidate->GetLineStringCP ()))
        {
        for (size_t i = 0; i < candidatePoints->size (); i++)
            numAdded += AppendIfDistinct (points, candidatePoints->at(i));
        }
    return numAdded;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool AppendEllipse(DEllipse3dR baseEllipse, DEllipse3dCR nextEllipse)
    {
    // Test for simple equality of center, vectors, and angle ranges, and sweeps in same direction....
    if (   DPoint3dOps::AlmostEqual (baseEllipse.center, nextEllipse.center)
        && DVec3dOps::AlmostEqual (baseEllipse.vector0, nextEllipse.vector0)
        && DVec3dOps::AlmostEqual (baseEllipse.vector90, nextEllipse.vector90)
        && Angle::NearlyEqualAllowPeriodShift (baseEllipse.start + baseEllipse.sweep, nextEllipse.start)
        && baseEllipse.sweep * nextEllipse.sweep >= 0.0
        )
        {
        baseEllipse.sweep += nextEllipse.sweep;
        return true;
        }
    return false;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
typedef bool(*CompatiblePrimitiveFunction) (CurveVectorCR source, size_t indexA, size_t indexB);


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool CompatibleLinear(CurveVectorCR source, size_t indexA, size_t indexB)
    {
    if (indexA >= source.size ())
        return false;
    if (indexB >= source.size ())
        return false;
    if (   NULL == source.at(indexA)->GetLineCP ()
        && NULL == source.at(indexA)->GetLineStringCP ())
        return false;
    if (   NULL == source.at(indexB)->GetLineCP ()
        && NULL == source.at(indexB)->GetLineStringCP ())
        return false;
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool CompatibleArcs(CurveVectorCR source, size_t indexA, size_t indexB)
    {
    size_t n = source.size ();
    if (indexA >= n || indexB >= n)
        return false;
    DEllipse3dCP pEllipseA = source.at (indexA)->GetArcCP ();
    DEllipse3dCP pEllipseB = source.at (indexB)->GetArcCP ();
    if (NULL == pEllipseA || NULL == pEllipseB)
        return false;
    DEllipse3d ellipseA = *pEllipseA;
    DEllipse3d ellipseB = *pEllipseB;
    return AppendEllipse (ellipseA, ellipseB);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool CompatibleBsplines(CurveVectorCR source, size_t indexA, size_t indexB)
    {
    static bool s_allowBsplineConsolidation = false;
    if (!s_allowBsplineConsolidation)
        return false;
    size_t n = source.size ();
    if (indexA >= n || indexB >= n)
        return false;
    MSBsplineCurveCP bcurveA = source.at (indexA)->GetBsplineCurveCP ();
    MSBsplineCurveCP bcurveB = source.at (indexB)->GetBsplineCurveCP ();
    if (NULL == bcurveA || NULL == bcurveB)
        return false;
    // TO DO -- check for matched end of A and start of B?
    DPoint3d endA, startB;
    bcurveA->FractionToPoint (endA, bcurveA->FractionToKnot (1.0));
    bcurveB->FractionToPoint (startB, bcurveB->FractionToKnot (0.0));
    if (DPoint3dOps::AlmostEqual (endA, startB))
        return true;
    return false;    
    }


// Count Line and LineString curves starting at baseIndex and looking forward (always) and backwards (if at start)
static bool CompatiblePrimitiveRange (CurveVectorCR source, size_t baseIndex, CompatiblePrimitiveFunction F, size_t &newBase, size_t &numPrimitive, bool &hasWrap)
    {
    size_t n = source.size ();
    newBase = baseIndex;
    numPrimitive = 0;
    hasWrap = false;
    if (baseIndex >= n)
        return false;
    numPrimitive = 1;
    for (size_t i = baseIndex; F (source, i, i + 1);)
        {
        numPrimitive++;
        i++;
        }
    if (baseIndex == 0 && source.IsClosedPath ())
        {
        size_t highestPossible = baseIndex + numPrimitive;
        size_t i = n;
        while (i > highestPossible && F(source, i - 1, i % n))
            {
            i--;
            }
        if (i < n)
            {
            newBase = i;
            numPrimitive += (n - i);
            hasWrap = true;
            }
        }
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void CurveVector::SimplifyLinestrings (double distanceTolerance, bool eliminateOverdraw, bool wrap, bool xyOnly)
    {
    if (distanceTolerance <= 0.0)
        distanceTolerance = DoubleOps::ComputeTolerance (FastMaxAbs (), 0.0, Angle::SmallAngle ());
    bvector<DPoint3d> *points;
    if (   wrap && size () == 1
        && IsClosedPath ()
        && at(0).IsValid ()
        &&  NULL != (points = at(0)->GetLineStringP ()))
        PolylineOps::CompressColinearPoints (*points, distanceTolerance, eliminateOverdraw, true, xyOnly);
    else
        {
        for (size_t i = 0; i < size (); i++)
            {
            if (!at(i).IsValid ())
                continue;

            CurveVectorP childVector;

            if (NULL != (childVector = const_cast <CurveVectorP> (at(i)->GetChildCurveVectorCP ())))
                childVector->SimplifyLinestrings (distanceTolerance, eliminateOverdraw, wrap, xyOnly);
            else if (NULL != (points = at(i)->GetLineStringP ()))
                PolylineOps::CompressColinearPoints (*points, distanceTolerance, eliminateOverdraw, false, xyOnly);
            }
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void CurveVector::ConsolidateAdjacentPrimitives (bool doSimplifyLinestrings, bool xyOnly)
    {
    double tolerance = DoubleOps::MaxAbs (
        DoubleOps::ComputeTolerance (FastMaxAbs (), 0.0, Angle::SmallAngle ()),
        DoubleOps::SmallMetricDistance ()
        );

    if (size () > 1)
        {
        size_t baseIndex = 0;
        size_t numCompressedPrimitives = 0;
        size_t sourceCount = size ();   // True count, does not change.
        size_t stopCount = sourceCount; // Can be reduced if tail is pulled into at(0) by wraparound.
        for (; baseIndex < stopCount;)
            {
            bool hasWrap;
            size_t blockBase;
            size_t blockCount;
            CurveVectorP childVector;
            if (CompatiblePrimitiveRange (*this, baseIndex, CompatibleLinear, blockBase, blockCount, hasWrap)
                && blockCount > 1
                )
                {
                bvector<DPoint3d> points;
                for (size_t k = 0; k < blockCount; k++)
                    {
                    size_t i = (blockBase + k) % sourceCount;
                    AppendPoints (points, at(i).get ());
                    }
                ICurvePrimitivePtr newLineString = ICurvePrimitive::CreateLineString (&points[0], points.size ());
                at(numCompressedPrimitives++) = newLineString;
                baseIndex = (blockBase + blockCount) % sourceCount;
                if (baseIndex == 0)
                    baseIndex = sourceCount;
                if (hasWrap)
                    stopCount = blockBase;
                }
            else if (CompatiblePrimitiveRange (*this, baseIndex, CompatibleArcs, blockBase, blockCount, hasWrap)
                && blockCount > 1
                )
                {
                DEllipse3d baseEllipse = *at(blockBase)->GetArcCP ();
                DEllipse3dCP nextEllipse;
                for (size_t k = 0; k < blockCount; k++)
                    {
                    size_t i = (blockBase + k) % sourceCount;
                    nextEllipse = at(i)->GetArcCP ();
                    AppendEllipse (baseEllipse, *nextEllipse);
                    }
                at(numCompressedPrimitives++) = ICurvePrimitive::CreateArc (baseEllipse);
                baseIndex = (blockBase + blockCount) % sourceCount;
                if (baseIndex == 0)
                    baseIndex = sourceCount;
                if (hasWrap)
                    stopCount = blockBase;
                }
            else if (CompatiblePrimitiveRange (*this, baseIndex, CompatibleBsplines, blockBase, blockCount, hasWrap)
                && blockCount > 1)
                {
                MSBsplineCurve baseBspline, nextBspline; 
                baseBspline.CopyFrom (*at(blockBase)->GetBsplineCurveCP ());
                for (size_t k = 1; k < blockCount; k++)
                    {
                    size_t i = (blockBase + k) % sourceCount;
                    nextBspline = *at(i)->GetBsplineCurveCP ();
                    baseBspline.AppendCurves (baseBspline, nextBspline, false, true);
                    }
                at(numCompressedPrimitives++) = ICurvePrimitive::CreateBsplineCurve (baseBspline);
                baseBspline.ReleaseMem ();
                baseIndex = (blockBase + blockCount) % sourceCount;
                if (baseIndex == 0)
                    baseIndex = sourceCount;
                if (hasWrap)
                    stopCount = blockBase;
                }
            else
                {
                if (NULL != (childVector = const_cast <CurveVectorP>(at(baseIndex)->GetChildCurveVectorCP ())))
                    {
                    childVector->ConsolidateAdjacentPrimitives ();
                    }
                if (numCompressedPrimitives != baseIndex)
                    at(numCompressedPrimitives++) = at (baseIndex);
                else
                    numCompressedPrimitives++;
                baseIndex++;
                }
            }
        
        resize (numCompressedPrimitives);
        }

    if (doSimplifyLinestrings)
        SimplifyLinestrings (tolerance, true, true, xyOnly);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void CurveVector::ConsolidateAdjacentPrimitives ()
    {
    ConsolidateAdjacentPrimitives (true);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void CurveVector::AppendCurvePlaneIntersections
(
DPlane3dCR plane,
bvector<CurveLocationDetailPair> &intersections,
double tolerance
) const
    {
    intersections.clear ();
    size_t nPrimitive = size ();
    tolerance = ResolveTolerance (tolerance);    
    for (size_t iPrimitive = 0; iPrimitive < nPrimitive; iPrimitive++)
        at(iPrimitive)->AppendCurvePlaneIntersections (plane, intersections, tolerance);
    
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void UpdateMaxDistance
(
DPoint3dCR basePoint,
DPoint3dCR candidatePoint,
double &maxDistance,
DPoint3dR farthestPoint
)
    {
    double d = basePoint.Distance (candidatePoint);
    if (d > maxDistance)
        {
        maxDistance = d;
        farthestPoint = candidatePoint;
        }
    }

// Find a ray for sorting.
static bool ComputeSortRay
(
bvector <CurveLocationDetailPair> &curveIntersections,
// First index
size_t i0,
// One beyond final index
size_t i1,
DRay3dR sortRay
)
    {
    if (i0 + 2 >= i1)
        return false;
    DPoint3d point0 = curveIntersections[i0].detailA.point;
    DPoint3d point1 = point0;
    // Find most distant point.
    // Vector to most distant point is at least half the total span.
    double dMax = 0.0;
    for (size_t i = i0 + 1; i < i1; i++)
        {
        UpdateMaxDistance (point0, curveIntersections[i1].detailA.point, dMax, point1);
        UpdateMaxDistance (point0, curveIntersections[i1].detailB.point, dMax, point1);
        }

    sortRay.origin = point0;
    sortRay.direction = DVec3d::FromStartEnd (point0, point1);
    if (dMax == 0.0 || !sortRay.direction.Normalize ())
        return false;
    return true;
    }

// Assign all "a" values along ray.
// Swap all pairs so lower "a" is first.
static void ComputeSortCoordinates
(
bvector <CurveLocationDetailPair> &curveIntersections,
size_t i0,
size_t i1,
DRay3d sortRay
)
    {
    for (size_t i = i0; i < i1; i++)
        {
        curveIntersections[i].detailA.a =
            sortRay.DirectionDotVectorToTarget (curveIntersections[i].detailA.point);
        curveIntersections[i].detailB.a =
            sortRay.DirectionDotVectorToTarget (curveIntersections[i].detailB.point);
        if (curveIntersections[i].detailB.a < curveIntersections[i].detailA.a)
            std::swap (curveIntersections[i].detailA, curveIntersections[i].detailB);
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool cb_compareDetailAa(
CurveLocationDetailPair const &pairA,
CurveLocationDetailPair const &pairB
)
    {
    return pairA.detailA.a < pairB.detailA.a;
    }

// Move entries around so that single point pairs are first.
// Return number of single point pairs.
static size_t CollectSinglePoints (bvector <CurveLocationDetailPair> &pairs)
    {
    size_t i1 = pairs.size ();
    // Move all single point intersections to front.
    // Every pass moves i forward or i1 backwards.
    for (size_t i = 0; i < i1;)
        {
        if (pairs[i].SameCurveAndFraction ())
            {
            i++;
            }
        else
            {
            if (i < i1 - 1)
                std::swap (pairs[i], pairs[i1 - 1]);
            i1--;
            }
        }
    return i1;
    // 0..i1 are the single points.
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool AssembleSimpleParitySegments
(
bvector <CurveLocationDetailPair> &curveIntersections,
bvector <CurveLocationDetailPair> &segments
)
    {
    size_t i2 = curveIntersections.size();
    DRay3d sortRay;
    // Use ALL pairs to look for good sort ray ...
    ComputeSortRay (curveIntersections, 0, i2, sortRay);
    // Shuffle single points at start
    size_t i1 = CollectSinglePoints (curveIntersections);
    ComputeSortCoordinates (curveIntersections, 0, i2, sortRay);
    std::sort (curveIntersections.begin (), curveIntersections.begin () + i1, cb_compareDetailAa);
    for (size_t i = 0; i + 1 < i1; i += 2)
        {
        segments.push_back
            (
            CurveLocationDetailPair (
                curveIntersections[i].detailA,
                curveIntersections[i+1].detailB
                )
            );
        }
    // NEEDS_WORK WRONG BAD KLUDGE -- what about vertex hits????
    for (size_t i = i1; i < i2; i++)
        segments.push_back (curveIntersections[i]);
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurveVector::AppendClosedCurvePlaneIntersections
(
DPlane3dCR plane,
bvector<CurveLocationDetailPair> &intersections,
double tolerance
) const
    {
    BoundaryType btype = GetBoundaryType ();
    bool stat = false;
    if (btype == BOUNDARY_TYPE_Outer
        || btype == BOUNDARY_TYPE_Inner
        || btype == BOUNDARY_TYPE_ParityRegion
        )
        {
        bvector <CurveLocationDetailPair> curveIntersections;
        AppendCurvePlaneIntersections (plane, curveIntersections, tolerance);
        stat = AssembleSimpleParitySegments (curveIntersections, intersections);
        }
    else if (btype == BOUNDARY_TYPE_UnionRegion)
        {
        for (size_t i = 0; i < size (); i++)
            {
            CurveVectorCP child = at(i)->GetChildCurveVectorCP ();
            stat &= child->AppendClosedCurvePlaneIntersections (plane, intersections, tolerance);
            }
        }
    return stat;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr CurveVector::PlaneSection
(
DPlane3dCR plane,
double tolerance
) const
    {
    bvector<CurveLocationDetailPair> pairs;
    AppendClosedCurvePlaneIntersections (plane, pairs, tolerance);
    size_t numIntersections = pairs.size ();
    if (numIntersections == 1)
        {
        return ICurvePrimitive::CreateLine (pairs.front ().GetDSegment3d ());
        }
    else
        {
        CurveVectorPtr segments = CurveVector::Create (BOUNDARY_TYPE_None);
        for (size_t i = 0; i < numIntersections; i++)
            {
            segments->push_back (ICurvePrimitive::CreateLine (pairs[i].GetDSegment3d ()));
            }
        return ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*segments);
        }
    }

void ConvertToIntervalsOnRay (DRay3dCR ray, bvector<CurveLocationDetailPair> &pairs, bvector<DRange1d> &ranges)
    {
    for (CurveLocationDetailPair &pair : pairs)
        {
        double s0 = ray.DirectionDotVectorToTarget (pair.detailA.point);
        double s1 = ray.DirectionDotVectorToTarget (pair.detailB.point);
        ranges.push_back (DRange1d::From (s0, s1));
        }
    DRange1d::SimplifyInPlace (ranges);
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      10/2015
+--------------------------------------------------------------------------------------*/
bool CurveCurve::TransverseRegionIntersectionSegments
(
CurveVectorCR regionA,     //!< first source set
CurveVectorCR regionB,     //!< second source set
bvector<DSegment3d> &segments   //!< Destination for segments.
)
    {
    DPlane3d planeA, planeB;
    double areaA, areaB;
    DRay3d ray;
    if (regionA.CentroidNormalArea (planeA.origin, planeA.normal, areaA)
        && regionB.CentroidNormalArea (planeB.origin, planeB.normal, areaB)
        && ray.InitFromPlanePlaneIntersection (planeA, planeB)
        )
        {
        bvector<CurveLocationDetailPair> pairA, pairB, intersection;
        double tolerance = 1.0e-5;
        regionA.AppendClosedCurvePlaneIntersections (planeB, pairA, tolerance);
        regionB.AppendClosedCurvePlaneIntersections (planeA, pairB, tolerance);
        bvector<DRange1d> intervalA, intervalB, intervalAB;
        ConvertToIntervalsOnRay (ray, pairA, intervalA);
        ConvertToIntervalsOnRay (ray, pairB, intervalB);
        DRange1d::IntersectSorted (intervalA, intervalB, intervalAB);
        for (DRange1d &range : intervalAB)
            {
            segments.push_back (DSegment3d::From (ray.FractionParameterToPoint (range.low), ray.FractionParameterToPoint (range.high)));
            }
        return true;
        }
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CompleteFrame(DPoint3dCR origin, DVec3dCR tangent, DVec3dCR perp, TransformR frame)
    {
    static double s_magnitudeFraction = 1.0e-6;
    static double s_minAngle = 1.0e-8;
    if (perp.Magnitude () < s_magnitudeFraction * tangent.Magnitude ())
        return false;
    double theta = perp.SmallerUnorientedAngleTo (tangent);
    if (theta < s_minAngle)
        return false;
    RotMatrix matrix;
    if (matrix.SquareAndNormalizeColumns (RotMatrix::From2Vectors (tangent, perp), 0, 1))
        {
        frame.InitFrom (matrix, origin);
        return true;
        }
    return false;
    }

bool GetNonZeroTangent (DVec3dR tangent, DVec3dCR derivative1, DVec3dCR derivative2, DVec3dCR derivative3)
    {
    double a1 = derivative1.Magnitude ();
    double a2 = derivative2.Magnitude ();
    double a3 = derivative3.Magnitude ();
    double a = a1 + a2 + a3;
    if (a == 0.0)
        return false;
    static double s_reltol = 1.0e-15;    // really tight -- any hint of D1 wins.
    if (a1 >= s_reltol * a)
        {
        tangent = derivative1;
        return true;
        }
    if (a2 >= s_reltol * a)
        {
        tangent = derivative2;
        return true;
        }
    if (a3 >= s_reltol * a)
        {
        tangent = derivative3;
        return true;
        }
    return false;
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool GetNeighborhoodFrenetFrame(CurveVectorCR curves, size_t i, TransformR frame)
    {
    if (i >= curves.size ())
        return false;
    ICurvePrimitiveCP curveA = curves.at (i).get ();
    DPoint3d pointA, pointB;
    DVec3d   derivativeA1, derivativeA2, derivativeA3;
    DVec3d   derivativeB1, derivativeB2, derivativeB3;
    if (curveA->FractionToPoint (0.0, pointA, derivativeA1, derivativeA2, derivativeA3))
        {
        if (CompleteFrame (pointA, derivativeA1, derivativeA2, frame))
            return true;
        DVec3d tangentA, tangentB;
        if (GetNonZeroTangent (tangentA, derivativeA1, derivativeA2, derivativeA3))
            {
            // Need to look forward for non-parallel tangent to clarify plane
            for (size_t iB = i + 1, n = curves.size(); iB < n; iB++)
                {
                ICurvePrimitiveCP curveB = curves.at (iB).get ();
                if (curveB->FractionToPoint (0.0, pointB, derivativeB1, derivativeB2, derivativeB3)
                    && GetNonZeroTangent (tangentB, derivativeB1, derivativeB2, derivativeB3)
                    )
                    {
                    if (CompleteFrame (pointA, tangentA, DVec3d::FromStartEnd (pointA, pointB), frame))
                        return true;
                    if (CompleteFrame (pointA, tangentA, tangentB, frame))
                        return true;
                    }
                }
            }
        }
    return false;
    }
bool CurveVector::GetAnyFrenetFrame (TransformR frame) const
    {
    return GetAnyFrenetFrame (frame, 0);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurveVector::GetAnyFrenetFrame (TransformR frame, int searchPreference) const
    {
    // legacy behavior requests to use end tangent as "2nd vector" ...
    if (searchPreference == 1 || searchPreference == 2)
        {
        DPoint3d xyz0, xyz1;
        DVec3d   tangent0, tangent1;
        if (GetStartEnd (xyz0, xyz1, tangent0, tangent1))
            {
            RotMatrix orientation;
            if (tangent0.IsParallelTo (tangent1))
                {
                if (searchPreference == 1)
                    {
                    // really strong preference to look no further than end tangent even if it is parallel to start tangent.
                    orientation.InitFrom1Vector (tangent0, 0, false);
                    orientation.SquareAndNormalizeColumns (orientation, 0, 2);
                    frame.InitFrom (orientation, xyz0);
                    return true;
                    }                
                }
            else
                {
                orientation.InitFrom2Vectors (tangent0, tangent1);
                orientation.SquareAndNormalizeColumns (orientation, 0, 2);
                frame.InitFrom (orientation, xyz0);
                return true;
                }
            }
        }
    
    CurveVectorCP child;
    for (size_t i = 0, n = size (); i < n; i++)
        {
        if (NULL != (child = at(i)->GetChildCurveVectorCP ()))
            {
            if (child->GetAnyFrenetFrame (frame))
                return true;
            }
        else if (GetNeighborhoodFrenetFrame (*this, i, frame))
            return true;
        }
    // No really good frenet frame found.  Take any single curve's best estimate ...
    // (This is probably just a single line)
    for (size_t i = 0, n = size (); i < n; i++)
        {
        if (at(i).get()->FractionToFrenetFrame (0.0, frame))
            return true;
        }
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurveVector::IsPlanar (TransformR localToWorld, TransformR worldToLocal, DRange3dR range) const
    {
    if (GetAnyFrenetFrame (localToWorld)
        && worldToLocal.InverseOf (localToWorld)
        && GetRange (range, worldToLocal)
        )
        {
        return range.IsAlmostZeroZ ();
        }
    return false;
    }

bool CurveVector::IsPlanarWithDefaultNormal (TransformR localToWorld, TransformR worldToLocal, DRange3dR range, DVec3dCP normal) const
    {
    if (GetAnyFrenetFrame (localToWorld)
        && worldToLocal.InverseOf (localToWorld)
        && GetRange (range, worldToLocal)
        )
        {
        double dx = range.XLength ();
        double dy = range.YLength ();
        double dz = range.ZLength ();
        double maxAbs = range.LargestCoordinate ();
        bool zeroZ = DoubleOps::AlmostEqual (maxAbs, maxAbs + dz);
        bool zeroY = DoubleOps::AlmostEqual (maxAbs, maxAbs + dy);
        bool zeroX = DoubleOps::AlmostEqual (maxAbs, maxAbs + dx);
        // "Usual case" --- nonzero in both x and y, zero in z ...
        if (!zeroX && !zeroY && zeroZ)
            return true;
     
        // single line will fail to have a unique transform.  Try again using the given normal and the x axis ...
        if (!zeroX && zeroY && zeroZ)
            {
            // rebuild the frenet frame with its existing xVector and something else, preferably given normal ...
            DVec3d xVector, yVector, zVector;
            DPoint3d origin;
            localToWorld.GetOriginAndVectors (origin, xVector, yVector, zVector);
            RotMatrix axes;
            if (NULL != normal && !normal->IsParallelTo (xVector))
                {
                axes = RotMatrix::FromColumnVectors (xVector, yVector, *normal);
                axes.SquareAndNormalizeColumns (axes, 0, 2);
                }
            else
                {
                DVec3d vector = xVector;
                vector.GetNormalizedTriad (zVector, yVector,xVector);
                axes = RotMatrix::FromColumnVectors (xVector, yVector, zVector);
                }
            localToWorld.InitFrom (axes, origin);
            // OK, we've rebuilt the local coordinates. If it inverts, it ships ...
            if (worldToLocal.InverseOf (localToWorld)
                && GetRange (range, worldToLocal)
                )
                {
                double maxAbs = range.LargestCoordinate ();
                double dz = range.high.z - range.low.z;
                return DoubleOps::AlmostEqual (maxAbs, maxAbs + dz);
                }
            }
        }
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
CurveVectorPtr CurveVector::CloneInLocalCoordinates
(
LocalCoordinateSelect frameType,
TransformR localToWorld,
TransformR worldToLocal,
DRange3dR localRange
) const
    {
    CurveVectorPtr result;
    BentleyApi::Transform frame;
    if (!GetAnyFrenetFrame (frame))
        return result;


    localToWorld = frame;
    worldToLocal.InverseOf (localToWorld);

    result = Clone ();
    result->TransformInPlace (worldToLocal);
    result->GetRange (localRange);
    BentleyApi::Transform localToNewLocal = BentleyApi::Transform::FromIdentity ();
    BentleyApi::Transform::CorrectCoordinateFrameXYRange (localToWorld, worldToLocal, localRange, frameType, &localToNewLocal);
    if (!localToNewLocal.IsIdentity ())
        result->TransformInPlace (localToNewLocal);
    return result;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurveVector::TryUVFractionToXYZ
    (
    double uFraction,
    double vFraction,
    DPoint3dR xyz,
    DVec3dR dXdu,
    DVec3dR dXdv
    ) const
    {
    BentleyApi::Transform localToWorld, worldToLocal;
    DRange3d localRange;
    CurveVectorPtr localCurve = CloneInLocalCoordinates (LOCAL_COORDINATE_SCALE_01RangeBothAxes,
                localToWorld, worldToLocal, localRange);
    if (localCurve.IsValid ())
        {
        localToWorld.Multiply (xyz, uFraction, vFraction, 0.0);
        localToWorld.GetMatrixColumn (dXdu, 0);
        localToWorld.GetMatrixColumn (dXdv, 1);
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool CurveVector::ContainsNonLinearPrimitive () const
    {
    if (IsUnionRegion () || IsParityRegion ())
        {
        for (ICurvePrimitivePtr curve: *this)
            {
            if (curve.IsNull () || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType ())
                continue;

            if (curve->GetChildCurveVectorCP ()->ContainsNonLinearPrimitive ())
                return true;
            }

        return false;
        }

    for (ICurvePrimitivePtr curve: *this)
        {
        if (!curve.IsValid ())
            continue;

        switch (curve->GetCurvePrimitiveType ())
            {
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
                return true;
            }
        }

    return false;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
