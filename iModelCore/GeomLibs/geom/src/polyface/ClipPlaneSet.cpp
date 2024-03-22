/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

#include "./ClipPlaneSetPolygonClipContext.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#define fc_hugeVal 1e37

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool testRayIntersections (double& tNear, DPoint3dCR origin, DVec3dCR direction, ConvexClipPlaneSetCR planes)
    {
    tNear        = -fc_hugeVal;
    double tFar  =  fc_hugeVal;

    for (ClipPlaneCR plane: planes)
        {
        double     vD = plane.DotProduct (direction), vN = plane.EvaluatePoint (origin);

        if (0.0 == vD)
            {
            // Ray is parallel... No need to continue testing if outside halfspace.
            if (vN < 0.0)
                return false;
            }
        else
            {
            double      rayDistance = -vN / vD;

            if (vD < 0.0)
                {
                if (rayDistance < tFar)
                   tFar = rayDistance;
                }
            else
                {
                if (rayDistance > tNear)
                    tNear = rayDistance;
                }
            }
        }
    return tNear <= tFar;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ConvexClipPlaneSet::ConvexClipPlaneSet (ClipPlaneCP planes, size_t n) : T_ClipPlanes (n)
    {
    memcpy (&front(), planes, n * sizeof (ClipPlane));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ConvexClipPlaneSet::ConvexClipPlaneSet(DRange3dCR range,
bool includeLowX,
bool includeHighX,
bool includeLowY,
bool includeHighY,
bool includeLowZ,
bool includeHighZ
)
    {
    if (!range.IsNull ())
        {
        if (includeLowX)
            push_back (ClipPlane (DVec3d::From (1,0,0), range.low.x));
        if (includeHighX)
            push_back(ClipPlane(DVec3d::From(-1, 0, 0), -range.high.x));
        if (includeLowY)
            push_back(ClipPlane(DVec3d::From(0, 1, 0), range.low.y));
        if (includeHighY)
            push_back(ClipPlane(DVec3d::From(0, -1, 0), -range.high.y));
        if (includeLowZ)
            push_back(ClipPlane(DVec3d::From(0,0, 1), range.low.z));
        if (includeHighZ)
            push_back(ClipPlane(DVec3d::From(0,0,-1), -range.high.z));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConvexClipPlaneSet::IsPointInside (DPoint3dCR point) const
    {
    for (ClipPlaneCR plane: *this)
        if (!plane.IsPointOnOrInside (point))
            return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConvexClipPlaneSet::IsPointOnOrInside (DPoint3dCR point, double tolerance) const
    {
    double      interiorTolerance = fabs (tolerance); // Interior tolerance should always be positive. (TFS# 246598).

    for (ClipPlaneCR plane: *this)
        if (!plane.IsPointOnOrInside (point, (plane.GetIsInterior() ? interiorTolerance : tolerance)))
            return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ValidatedDPoint3d ConvexClipPlaneSet::FindAnyVertex() const
    {
    if (this->size() < 3)
        return ValidatedDPoint3d(DPoint3d::From(0, 0, 0), false);
    size_t n = this->size();
    RotMatrix matrix;
    DPoint3d rhs, xyz;
    for (size_t i = 0; i < n; i++)
        {
        matrix.form3d[0][0] = (*this)[i].m_normal.x;
        matrix.form3d[0][1] = (*this)[i].m_normal.y;
        matrix.form3d[0][2] = (*this)[i].m_normal.z;
        rhs.x = (*this)[i].m_distance;
        for (size_t j = i + 1; j < n; j++)
            {
            if ((*this)[i].m_normal.IsParallelTo ((*this)[j].m_normal))
                continue;
            matrix.form3d[1][0] = (*this)[j].m_normal.x;
            matrix.form3d[1][1] = (*this)[j].m_normal.y;
            matrix.form3d[1][2] = (*this)[j].m_normal.z;
            rhs.y = (*this)[j].m_distance;
            for (size_t k = j + 1; k < n; k++)
                {
                matrix.form3d[2][0] = (*this)[k].m_normal.x;
                matrix.form3d[2][1] = (*this)[k].m_normal.y;
                matrix.form3d[2][2] = (*this)[k].m_normal.z;
                rhs.z = (*this)[k].m_distance;
                if (matrix.Solve(xyz, rhs))
                    {
                    bool allIn = true;
                    for (size_t m = 0; m < n; m++)
                        {
                        if (i != m && j != m && k != m)
                            {
                            if ((*this)[m].EvaluatePoint(xyz) < 0)
                                {
                                allIn = false;
                                break;
                                }
                            }
                        }
                    if (allIn)
                        return ValidatedDPoint3d(xyz, true);
                    }
                }
            }
        }
    return ValidatedDPoint3d(DPoint3d::From(0, 0, 0), false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConvexClipPlaneSet::IsSphereInside (DPoint3dCR point, double radius) const
    {
    // Note - The sphere logic differ from "PointOnOrInside" only in the handling of interior planes.
    // For a sphere we don't negate the tolerance on interior planes - we have to look for true containment (TFS# 439212).
    for (ClipPlaneCR plane: *this)
        if (!plane.IsPointOnOrInside (point, radius))
            return false;

    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClipPlaneSet::IsPointInside (DPoint3dCR point) const
    {
    for (ConvexClipPlaneSet convexSet: *this)
        if (convexSet.IsPointInside (point))
            return true;

    return false;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ClipPlaneSet::TestRayIntersect (DPoint3dCR point, DVec3dCR direction) const
    {
    double      tNear;

    for (ConvexClipPlaneSetCR planeSet: *this)
        if (testRayIntersections (tNear, point, direction, planeSet))
            return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ClipPlaneSet::GetRayIntersection (double& nearest, DPoint3dCR point, DVec3dCR direction) const
    {

    nearest = -fc_hugeVal;

    for (ConvexClipPlaneSetCR planeSet: *this)
        {
        if (planeSet.IsPointInside (point))
            {
            nearest = 0.0;
            }
        else
            {
            double      tNear;

            if (testRayIntersections (tNear, point, direction, planeSet) && tNear > nearest)
                nearest = tNear;
            }
        }

    return nearest > -fc_hugeVal;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClipPlaneSet::IsPointOnOrInside (DPoint3dCR point, double tolerance) const
    {
    for (ConvexClipPlaneSetCR convexSet: *this)
        if (convexSet.IsPointOnOrInside (point, tolerance))
            return true;

    return false;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClipPlaneSet::IsSphereInside (DPoint3dCR point, double radius) const
    {
    for (ConvexClipPlaneSetCR convexSet: *this)
        if (convexSet.IsSphereInside (point, radius))
            return true;

    return false;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClipPlaneSet::IsAnyPointInOrOn(DSegment3dCR segment) const
    {
    double f0, f1;
    for (ConvexClipPlaneSetCR convexSet : *this)
        {
        if (convexSet.ClipBoundedSegment (segment.point[0], segment.point[1], f0, f1, -1.0))
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipPlaneSet::AppendIntervals (DSegment3dCR segment, bvector<DSegment1d> &intervals, ClipPlaneCP planeToSkip) const
    {
    double f0, f1;
    for (ConvexClipPlaneSetCR convexSet : *this)
        {
        if (convexSet.ClipBoundedSegment (segment.point[0], segment.point[1], f0, f1, -1.0, planeToSkip))
            {
            intervals.push_back (DSegment1d (f0, f1));
            }
        }
    }
void ClipPlaneSet::AppendIntervals(DSegment3dCR segment, bvector<DRange1d> &intervals) const
    {
    double f0, f1;
    for (ConvexClipPlaneSetCR convexSet : *this)
        {
        if (convexSet.ClipBoundedSegment(segment.point[0], segment.point[1], f0, f1, -1.0))
            {
            intervals.push_back(DRange1d::From(f0, f1));    // f0, f1 are probably orded correctly, but ::From ensures it.
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClipPlaneSet::IsAnyPointInOrOn(DEllipse3dCR arc) const
    {
    for (ConvexClipPlaneSetCR convexSet : *this)
        {
        if (convexSet.AppendIntervals (arc, nullptr, -1.0))
            return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipPlaneSet::AppendIntervals (DEllipse3dCR arc, bvector<DSegment1d> &intervals) const
    {
    for (ConvexClipPlaneSetCR convexSet : *this)
        {
        convexSet.AppendIntervals (arc, &intervals, -1.0);
        }
    }
void ClipPlaneSet::AppendIntervals(MSBsplineCurveCR curve, bvector<DSegment1d> &intervals) const
    {
    for (ConvexClipPlaneSetCR convexSet : *this)
        {
        convexSet.AppendIntervals(curve, &intervals);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClipPlaneSet::IsAnyPointInOrOn(MSBsplineCurveCR curve) const
    {
    for (ConvexClipPlaneSetCR convexSet : *this)
        {
        if (convexSet.AppendIntervals (curve, nullptr))
            return true;
        }
    return false;
    }




/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConvexClipPlaneSet::TransformInPlace (TransformCR transform)
    {
    for (ClipPlaneR plane: *this)
        plane.TransformInPlace (transform);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipPlaneSet::TransformInPlace (TransformCR transform)
    {
    for (ConvexClipPlaneSetR convexSet: *this)
        convexSet.TransformInPlace (transform);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConvexClipPlaneSet::MultiplyPlanesTimesMatrix(DMatrix4dCR matrix)
    {
    for (ClipPlaneR plane: *this)
        plane.MultiplyPlaneTimesMatrix(matrix);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipPlaneSet::MultiplyPlanesTimesMatrix(DMatrix4dCR matrix)
    {
    for (ConvexClipPlaneSetR convexSet: *this)
        convexSet.MultiplyPlanesTimesMatrix(matrix);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool ConvexClipPlaneSet::ClipBoundedSegment (DPoint3dCR pointA, DPoint3dCR pointB, double &fraction0, double &fraction1, double planeSign,
ClipPlaneCP planeToSkip) const
    {
    fraction0 = 0.0;
    fraction1 = 1.0;

    for (ClipPlaneCR plane: *this)
        {
        if (planeToSkip != nullptr && planeToSkip->IsEqual (plane))
            continue;
        double hA = planeSign * plane.EvaluatePoint (pointA); // signed altitude of pointA wrt plane
        double hB = planeSign * plane.EvaluatePoint (pointB); // signed altitude of pointB wrt plane
        if (hB > hA) // Strictly
            {
            if (hA > 0.0)
                return false;
            if (hB > 0.0)
                {
                double fraction = -hA / (hB - hA);
                if (fraction < fraction0)
                    return false;
                if (fraction < fraction1)
                    fraction1 = fraction;
                }
            }
        else if (hA > hB) // Strictly
            {
            if (hB > 0.0)
                return false;
            if (hA > 0.0)
                {
                double fraction = -hA / (hB - hA);
                if (fraction > fraction1)
                    return false;
                if (fraction > fraction0)
                    fraction0 = fraction;
                }
            }
        else
            {
            // Strictly equal evaluations
            if (hA > 0.0)
                return false;
            }
        }
    return fraction1 >= fraction0;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool ConvexClipPlaneSet::AppendIntervals (DEllipse3dCR arc, bvector<DSegment1d> *intervals, double planeSign) const
    {
    double tol = Angle::SmallAngle ();
    size_t maxIntersections = 2 * size () + 2;
    ScopedArray<double> intersectionFractions (maxIntersections);
    size_t totalIntersections = 0;
    intersectionFractions.GetData ()[totalIntersections++] = 0.0;
    for (ClipPlaneCR plane : *this)
        {
        double singleArcFractions[3];
        int numIntersections = plane.SimpleIntersectionFractions (arc, singleArcFractions, true);
        for (int i = 0; i < numIntersections; i++)
            intersectionFractions.GetData()[totalIntersections++] = singleArcFractions[i];
        }
    intersectionFractions.GetData()[totalIntersections++] = 1.0;
    std::sort (intersectionFractions.GetData(),
                intersectionFractions.GetData () + totalIntersections);
    for (size_t i = 0; i + 1 < totalIntersections; i++)
        {
        double f0 = intersectionFractions.GetData()[i];
        double f1 = intersectionFractions.GetData()[i + 1];
        double f = 0.5 * (f0 + f1);
        DPoint3d xyz;
        arc.FractionParameterToPoint (xyz, f);
        if (IsPointOnOrInside(xyz, tol))
            {
            if (intervals == nullptr)
                return true;
            intervals->push_back (DSegment1d (f0, f1));
            }
        }

    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
static void FormDotProducts (DPoint4dCR plane, DPoint4dCP poles, size_t n, double * products, DRange1dR range)
    {
    range = DRange1d ();
    for (size_t i = 0; i < n; i++)
        {
        double a = plane.DotProduct (poles[i]);
        products[i] = a;
        range.Extend (a);
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool ConvexClipPlaneSet::AppendIntervals (MSBsplineCurveCR curve, bvector<DSegment1d> *intervals) const
    {
    double poleCoffs[MAX_BEZIER_ORDER];
    double roots[MAX_BEZIER_ORDER];
    bvector<double> allRootsOnSegment;
    // Choice:
    // (a) loop over planes as outer loop, clipping the whole bspline to each.
    // (b) loop over segments as outer loop, clipping to all planes.
    // choose (b) to avoid repeatedly going through the saturation.
    // don't saturate a segment until the looser range test of unsaturated poles crosses.
    BCurveSegment segment;
    for (size_t bezierIndex = 0; curve.AdvanceToBezier(segment, bezierIndex, false);)
        {
        allRootsOnSegment.clear ();
        allRootsOnSegment.push_back (0.0);
        size_t order = segment.GetOrder ();
        bool isSaturated = false;
        DPoint4dCP poles = segment.GetPoleP ();
        bool allOut = false;
        for (ClipPlaneCR plane : *this)
            {
            DPoint4d planeCoffs = plane.GetDPlane4d ();
            DRange1d poleRange;
            FormDotProducts (planeCoffs, poles, order, poleCoffs, poleRange);
            if (!isSaturated && poleRange.Contains (0.0))
                {
                // recompute pole range with saturated knots ...
                segment.SaturateKnots ();
                FormDotProducts(planeCoffs, poles, order, poleCoffs, poleRange);
                isSaturated = true;
                }

            if (poleRange.Contains (0.0))
                {
                int numRoots;
                if (bsiBezier_univariateRoots (roots, &numRoots, poleCoffs, (int)order))
                    {
                    for (int k = 0; k < numRoots; k++)
                        allRootsOnSegment.push_back (roots[k]);
                    }
                }
            else
                {
                if (poleRange.Low () < 0.0)
                    {
                    allOut = true;
                    break;  // break from the plane loop -- we're all out
                    }
                }
            }
        if (allOut)
            {
            // This segment is all out
            }
        else if (allRootsOnSegment.size () == 1)
            {
            // nothing but the original 0.0 value.  This segment is all "in"
            if (intervals == nullptr)
                return true;
            auto knotRangeInCurve = segment.KnotRange ();
            intervals->push_back (
                    DSegment1d (
                        curve.KnotToFraction (knotRangeInCurve.Low ()),
                        curve.KnotToFraction(knotRangeInCurve.High ())
                        ));
            }
        else
            {
            allRootsOnSegment.push_back (1.0);
            std::sort (allRootsOnSegment.begin (), allRootsOnSegment .end ());
            for (size_t rootIndex = 0; rootIndex + 1 < allRootsOnSegment.size (); rootIndex++)
                {
                double f0 = allRootsOnSegment[rootIndex];
                double f1 = allRootsOnSegment[rootIndex+1];
                double f = 0.5 * ( f0 + f1);
                DPoint3d xyz;
                segment.FractionToPoint (xyz, f);
                if (IsPointOnOrInside(xyz, 0.0))
                    {
                    if (nullptr == intervals)
                        return true;
                    intervals->push_back(
                        DSegment1d(
                            segment.FractionToKnot (f0),
                            segment.FractionToKnot (f1)
                            ));
                    }
                }
            }
        }
    if (nullptr == intervals)
        return false;
    return intervals->size () > 0;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool ConvexClipPlaneSet::ClipUnBoundedSegment (DPoint3dCR pointA, DPoint3dCR pointB, double &fraction0, double &fraction1, double planeSign) const
    {
    fraction0 = -DBL_MAX;
    fraction1 =  DBL_MAX;

    for (ClipPlaneCR plane: *this)
        {
        double hA = planeSign * plane.EvaluatePoint (pointA);
        double hB = planeSign * plane.EvaluatePoint (pointB);
        double fraction;
        if (bsiTrig_safeDivide (&fraction, -hA, hB - hA, 0.0))
            {
            if (hB > hA)    // INBOUND
                {
                if (fraction > fraction1)
                    return false;
                if (fraction > fraction0)
                    fraction0 = fraction;
                }
            else if (hA > hB) // OUTBOUND
                {
                if (fraction < fraction0)
                    return false;
                if (fraction < fraction1)
                    fraction1 = fraction;
                }
            }
        else
            {
            // Exactly or nearly parallel with the plane.
            // Call it out if both are positive.
            if (hA < 0.0 && hB < 0.0)
                return false;
            }
        }
    return fraction1 >= fraction0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPlaneContainment ConvexClipPlaneSet::ClassifyPointContainment (DPoint3dCP points, size_t nPoints, bool onIsOutside) const
    {
    bool        allInside = true;
    double      onTolerance = onIsOutside ? 1.0E-8 : - 1.0E-8;
    double      interiorTolerance = 1.0E-8; // Interior tolerance should always be positive. (TFS# 246598).

    for (ClipPlaneCR plane: *this)
        {
        size_t  nOutside = 0;

        for (size_t j = 0; j < nPoints; j++)
            {
            if (plane.EvaluatePoint (points[j]) < (plane.GetIsInterior() ? interiorTolerance : onTolerance))
                {
                nOutside++;
                allInside = false;
                }
            }

        if (nOutside == nPoints)
            return ClipPlaneContainment_StronglyOutside;
        }

    return allInside ? ClipPlaneContainment_StronglyInside : ClipPlaneContainment_Ambiguous;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
ConvexClipPlaneSet ConvexClipPlaneSet::FromXYBox (double x0, double y0, double x1, double y1)
    {
    ConvexClipPlaneSet  convexSet;

    convexSet.push_back (ClipPlane (DVec3d::From (-1, 0,0),  -x1));
    convexSet.push_back (ClipPlane (DVec3d::From ( 1, 0,0),  x0));
    convexSet.push_back (ClipPlane (DVec3d::From ( 0,-1,0),  -y1));
    convexSet.push_back (ClipPlane (DVec3d::From ( 0, 1,0),  y0));

    return convexSet;
    }

ConvexClipPlaneSet ConvexClipPlaneSet::FromXYPolyLine (bvector<DPoint3d> const &points, bvector<BoolTypeForVector> const &interior, bool leftIsInside)
    {
    ConvexClipPlaneSet  convexSet;
    for (size_t i0 = 0; i0 + 1 < points.size (); i0++)
        {
        DVec3d edgeVector = DVec3d::FromStartEnd (points[i0], points[i0 + 1]);
        DVec3d perp = DVec3d::FromCCWPerpendicularXY (edgeVector);
        perp.z = 0.0;
        if (!leftIsInside)
            perp.Negate ();
        bool hidden = false;
        if (interior.size () > i0)
            hidden = interior[i0];
        if (perp.Normalize ())
            convexSet.push_back (ClipPlane (perp, points[i0], hidden, false));
        }

    return convexSet;
    }

ClipPlaneSet  ClipPlaneSet::FromSweptPolygon (DPoint3dCP points, size_t n, DVec3dCP direction)
    {
    return FromSweptPolygon (points, n, direction, nullptr);
    }
static bool AppendPlaneThroughXYSegment
(
ConvexClipPlaneSet &convexSet,
bool toLeft,
DPoint3dCR xyzA,
DPoint3dCR xyzB,
bool invisible
)
    {
    auto validatedPerp = DVec3d::FromUnitPerpendicularXY (xyzB - xyzA);
    if (validatedPerp.IsValid ())
        {
        DVec3d perp = validatedPerp.Value ();
        if (!toLeft)
            perp.Negate ();
        convexSet.push_back (ClipPlane (perp, xyzA, invisible));
        }
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool ClipPlaneSetWithIndexedRangeHeap::InitFromMatchedXYPointVectors
(
bvector<DPoint3d> const &pointsA,
bvector<DPoint3d> const &pointsB
)
    {
    size_t n = std::min (pointsA.size (), pointsB.size ());
    for (size_t i = 0; i + 1 < n; i++)
        {
        DPoint3d xyzA0 = pointsA[i];
        DPoint3d xyzA1 = pointsA[i+1];
        DPoint3d xyzB0 = pointsB[i];
        DPoint3d xyzB1 = pointsB[i+1];
        double crossA01B0 = xyzA0.CrossProductToPointsXY (xyzA1, xyzB0);
        double crossA1B10 = xyzA1.CrossProductToPointsXY(xyzB1, xyzB0);
        if (crossA01B0 * crossA1B10 > 0.0)
            {
            bool toLeft = crossA01B0 > 0.0;
            size_t clipperIndex = this->size ();
            this->push_back (ConvexClipPlaneSet ());
            AppendPlaneThroughXYSegment(this->back (), toLeft, xyzA0, xyzA1, false);
            AppendPlaneThroughXYSegment(this->back(), toLeft, xyzA1, xyzB1, false);
            AppendPlaneThroughXYSegment(this->back(), toLeft, xyzB1, xyzB0, false);
            AppendPlaneThroughXYSegment(this->back(), toLeft, xyzB0, xyzA0, false);
            DRange3d range = DRange3d::From (xyzA0, xyzA1);
            range.Extend (xyzB0);
            range.Extend (xyzB1);
            this->m_ranges.AddRange (range, clipperIndex, i);
            }
        }
    if (this->size () > 0)
        {
        this->m_heap.Build (1, &this->m_ranges, 0, this->size () - 1);
        return true;
        }
    return false;
    }
void ClipPlaneSetWithIndexedRangeHeap::AppendDSegment3dIntervals
(
DSegment3dCR segment,
bvector<DSegment1d> &intervals
)
    {
    DRange3d range = DRange3d::From (segment.point[0], segment.point[1]);
    m_heap.CollectInRange (range, 2, m_work);
    double f0, f1;
    size_t clipperIndex, geometryIndex;
    DRange3d clipperRange;
    m_hitData.Accumulate (m_work.size ());
    for (auto indexInMyRanges : m_work)
        {
        if (m_ranges.GetData (indexInMyRanges, clipperRange, clipperIndex, geometryIndex))
            {
            m_ranges.GetData (clipperIndex, clipperRange, clipperIndex, geometryIndex);
            if (at(clipperIndex).ClipBoundedSegment(segment.point[0], segment.point[1], f0, f1, -1.0))
                {
                intervals.push_back(DSegment1d(f0, f1));
                }
            }
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool ClipPlaneSet::AddMatchedXYPointVectors
(
    bvector<DPoint3d> const &pointsA,
    bvector<DPoint3d> const &pointsB
)
    {
    size_t n = std::min(pointsA.size(), pointsB.size());
    for (size_t i = 0; i + 1 < n; i++)
        {
        DPoint3d xyzA0 = pointsA[i];
        DPoint3d xyzA1 = pointsA[i + 1];
        DPoint3d xyzB0 = pointsB[i];
        DPoint3d xyzB1 = pointsB[i + 1];
        double crossA01B0 = xyzA0.CrossProductToPointsXY(xyzA1, xyzB0);
        double crossA1B10 = xyzA1.CrossProductToPointsXY(xyzB1, xyzB0);
        if (crossA01B0 * crossA1B10 > 0.0)
            {
            bool toLeft = crossA01B0 > 0.0;
            this->push_back(ConvexClipPlaneSet());
            AppendPlaneThroughXYSegment(this->back(), toLeft, xyzA0, xyzA1, false);
            AppendPlaneThroughXYSegment(this->back(), toLeft, xyzA1, xyzB1, false);
            AppendPlaneThroughXYSegment(this->back(), toLeft, xyzB1, xyzB0, false);
            AppendPlaneThroughXYSegment(this->back(), toLeft, xyzB0, xyzA0, false);
            }
        }
    if (this->size() > 0)
        {
        return true;
        }
    return false;
    }

static DPoint3d LexicalXYZSelectLow (DPoint3dCR point0, DPoint3dCR point1)
    {
    if (point0.x < point1.x)
        return point0;
    if (point1.x < point0.x)
        return point1;

    if (point0.y < point1.y)
        return point0;
    if (point1.y < point0.y)
        return point1;

    if (point0.z < point1.z)
        return point0;
    if (point1.z < point0.z)
        return point1;
    return point0;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void ConvexClipPlaneSet::AddSweptPolyline
(
bvector<DPoint3d> const &points,  //!< [in] polyline points
DVec3d upVector,          //!< [in] upward vector (e.g. towards eye at infinity)
Angle  tiltAngle            //!< [in] angle for tilt of planes.
)
    {
    bool reverse = false;
    static double s = -1.0;
    if (points.size () > 3 && points.front ().AlmostEqual (points.back ()))
        {
        DVec3d polygonNormal = PolygonOps::AreaNormal (points);
        if (s * polygonNormal.DotProduct (upVector) < 0.0)
            reverse = true;
        }
    for (size_t i = 0; i + 1 < points.size (); i++)
        if (reverse)
            Add (ClipPlane::FromEdgeAndUpVector (points[i+1], points[i], upVector, tiltAngle));
        else
            Add (ClipPlane::FromEdgeAndUpVector (points[i], points[i+1], upVector, tiltAngle));
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool ConvexClipPlaneSet::Add (ValidatedClipPlane const &plane)
    {
    if (plane.IsValid ())
        push_back (plane.Value ());
    return plane.IsValid ();
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
ClipPlaneSet  ClipPlaneSet::FromSweptPolygon (DPoint3dCP points, size_t n, DVec3dCP direction, bvector<bvector<DPoint3d>> *shapes)
    {
    static double s_graphRelTol = 1.0e-9;
    static double s_edgeRelTol  = 1.0e-8;
    static double s_graphAbsTol = 0.0;
    static double s_edgeAbsTol = 0.0;


    Transform localToWorld, worldToLocal;
    DVec3d sweepDirection = DVec3d::From (0,0,1);
    if (NULL != direction)
        {
        double a = sweepDirection.Normalize (*direction);
        if (a == 0.0)
            sweepDirection = DVec3d::From (0,0,1);
        }
    if (n <= 2)
        return ClipPlaneSet();

    localToWorld = Transform::From (RotMatrix::From1Vector (sweepDirection, 2, true), points[0]);
    worldToLocal.InverseOf (localToWorld);

    VuSetP graphP = vu_newVuSet (0);
    if (NULL == graphP)
        return ClipPlaneSet();

    //static double s_relTol = 1.0e-8;


    vu_setTol (graphP, s_graphAbsTol, s_graphRelTol);

    vu_loopFromDPoint3dArrayXYTol (graphP, &worldToLocal, points, (int)n, s_edgeAbsTol, s_edgeRelTol);

    vu_mergeOrUnionLoops (graphP, VUUNION_PARITY);

    vu_regularizeGraph (graphP);
    vu_markAlternatingExteriorBoundaries(graphP,true);

    vu_triangulateMonotoneInteriorFaces (graphP, false);
    vu_flipTrianglesToImproveQuadraticAspectRatio (graphP);
    vu_removeEdgesToExpandConvexInteriorFaces (graphP);

    VuMask visitMask = vu_grabMask (graphP);
    ClipPlaneSet      clipSet;
    size_t numNegative = 0;
    size_t numPositive = 0;
    VU_SET_LOOP (faceSeed, graphP)
        {
        if (   !vu_getMask (faceSeed, visitMask)
            && !vu_getMask (faceSeed, VU_EXTERIOR_EDGE))
            {
            vu_setMaskAroundFace (faceSeed, visitMask);
            double area = vu_area (faceSeed);
            if (area < 0.0)
                numNegative++;
            else
                numPositive++;
            ConvexClipPlaneSet convexSet;
            if (shapes != nullptr)
                shapes->push_back (bvector<DPoint3d> ());
            VU_FACE_LOOP (currVertex, faceSeed)
                {
                DPoint3d xyz0, xyz1, uvw0, uvw1;
                vu_getDPoint3d (&uvw0, currVertex);
                vu_getDPoint3d (&uvw1, vu_fsucc (currVertex));
                localToWorld.Multiply (xyz0, uvw0);
                if (shapes != nullptr)
                    shapes->back().push_back (xyz0);
                localToWorld.Multiply (xyz1, uvw1);
                DVec3d edgeVector = DVec3d::FromStartEnd (xyz0, xyz1);
                DVec3d inwardNormal = DVec3d::FromCrossProduct (sweepDirection, edgeVector);
                inwardNormal.Normalize();

                DPoint3d xyzOrigin = LexicalXYZSelectLow (xyz0, xyz1);
                double distance = inwardNormal.DotProduct (xyzOrigin.x, xyzOrigin.y, xyzOrigin.z);
                bool isOriginalEdge = (vu_getMask (currVertex, VU_BOUNDARY_EDGE) != 0);
                convexSet.push_back (ClipPlane (inwardNormal, distance, !isOriginalEdge, !isOriginalEdge));
                }
            END_VU_FACE_LOOP (currVertex, faceSeed)
            clipSet.push_back (convexSet);
            }
        }
    END_VU_SET_LOOP (faceSeed, graphP)
    vu_returnMask (graphP, visitMask);

    vu_freeVuSet (graphP);
    return clipSet;
    }

void ConvexClipPlaneSet::ClipPointsOnOrInside
(
bvector<DPoint3d> const &points,   //!< [in] input points
bvector<DPoint3d> *inOrOn,      //!< [out] points that are in or outside.
bvector<DPoint3d> *out      //!< [out] points that are outside.
) const
    {
    if (inOrOn)
        inOrOn->clear ();
    if (out)
        out->clear ();
    for (auto &xyz : points)
        {
        if (IsPointOnOrInside (xyz, 0.0))
            {
            if (inOrOn != nullptr)
                inOrOn->push_back (xyz);
            }
        else
            {
            if (out != nullptr)
                out->push_back (xyz);
            }
        }
    }
void ConvexClipPlaneSet::ConvexPolygonClip
(
bvector<DPoint3d> const &input, //!< [in] points of a convex polygon
bvector<DPoint3d> &output,      //!< [out] clipped polygon
bvector<DPoint3d> &work,         //!< [inout] extra polygon
int onPlaneHandling
) const
    {
    output = input;
    for (auto &plane : *this)
        {
        if (output.empty ())
            break;
        plane.ConvexPolygonClipInPlace (output, work, onPlaneHandling);
        }
    }
// DEPRECATED SHORT ARG LIST
void ConvexClipPlaneSet::ConvexPolygonClip
(
bvector<DPoint3d> const &input, //!< [in] points of a convex polygon
bvector<DPoint3d> &output,      //!< [out] clipped polygon
bvector<DPoint3d> &work         //!< [inout] extra polygon
) const
    {
    return ConvexPolygonClip (input, output, work, 0);
    }

#define CheckAreaXY_not
// EDL Dec 7 2016.
// superficially bad area split observed when a vertical facet (edge on from above) is split.
// a1=-2.9864408788819741e-008
// a2=0
// this is artificially near zero.
#ifdef CheckAreaXY
double Check(double a0, double a1)
    {
    double dx = fabs (a1 - a0);
    bool sameArea = DoubleOps::AlmostEqual (a0, a1);
    BeAssert (sameArea);
    return dx;
    }
#endif
void ConvexClipPlaneSet::ConvexPolygonClipInsideOutside
(
bvector<DPoint3d> const &input, //!< [in] points of a convex polygon
bvector<DPoint3d> &inside,      //!< [out] clipped polygon (inside the convex set).  May be aliased as the input.
BVectorCache<DPoint3d> &outside,      //!< [out] clipped polygons (outside the convex set)
bvector<DPoint3d> &work1,
bvector<DPoint3d> &work2,           //!< [out] work vector -- copy of input.
bool clearOutside,
double distanceTolerance
) const
    {
#ifdef CheckAreaXY
    double area0 = PolygonOps::AreaXY (input);
    double area1 = 0.0;
#endif
    work1 = input; // This is the ever-shrinking inside polygon
    work2 = input;
    if (clearOutside)
        outside.ClearToCache ();
    size_t outsideCount0 = outside.size ();
    DRange1d altitudeRange;
    for (auto &plane : *this)
        {
        if (work1.empty())
            break;
        outside.PushFromCache ();
        plane.ConvexPolygonSplitInsideOutside (work1, inside, outside.back (), altitudeRange);
#ifdef CheckAreaXY
Check ( PolygonOps::AreaXY (work1),
        PolygonOps::AreaXY (inside) + PolygonOps::AreaXY (outside.back ())
        );
        area1 += PolygonOps::AreaXY (outside.back ());
#endif
        if (altitudeRange.low >= -distanceTolerance && altitudeRange.low <= 0.0)
            {
            // leave unclipped IN
            outside.PopToCache();
            }
        else if (altitudeRange.high < distanceTolerance && altitudeRange.high >= 0.0)
            {
            // leave unclipped OUT
            inside.clear ();
            outside.back ().clear ();
            outside.back ().swap (work1);
            }
        else
            {
            inside.swap (work1);
            if (outside.back ().empty ())
                outside.PopToCache();
            }
        }

#ifdef CheckAreaXY
        area1 += PolygonOps::AreaXY (work1);
        Check (area0, area1);
#endif

    if (work1.empty ())
        {
        // nothing is inside.  If more than one shard was pushed, get rid of those and just push the single original
        if (outside.size () > outsideCount0 + 1)
            {
            while (outside.size () > outsideCount0)
                outside.PopToCache ();
            outside.PushCopy (work2);
            }
        inside.clear ();
        }
    else
        {
        inside.swap (work1);
        }
    }

int ConvexClipPlaneSet::ReloadSweptConvexPolygon
(
bvector<DPoint3d> const &points,      // polygon points
DVec3dCR sweepDirection,        // direction for sweep
int sideSelect
)
    {
    clear ();
    size_t n = points.size ();
    if (n <= 2)
        return 0;

    DVec3d planeNormal = PolygonOps::AreaNormal (points);
    bool isCCW = sweepDirection.DotProduct (planeNormal) > 0.0;

    size_t delta = isCCW ? 1 : n - 1;
    for (size_t i = 0; i < n; i++)
        {
        size_t i1 = (i+delta) % n;
        DPoint3d xyz0, xyz1;
        xyz0 = points[i];
        xyz1 = points[i1];
        if (xyz0.AlmostEqual (xyz1))
            continue;
        DVec3d edgeVector = DVec3d::FromStartEnd (xyz0, xyz1);
        DVec3d inwardNormal = DVec3d::FromCrossProduct (sweepDirection, edgeVector);
        inwardNormal.Normalize ();
        double distance = inwardNormal.DotProduct (xyz0.x, xyz0.y, xyz0.z);
        push_back (ClipPlane (inwardNormal, distance, false));
        }
    if (sideSelect != 0.0)
        {
        planeNormal.Normalize ();
        double a = sweepDirection.DotProduct (planeNormal) * (double) sideSelect;
        if (a < 0.0)
            planeNormal.Negate ();
        DPoint3d xyz0 = points[0];
        double distance = planeNormal.DotProduct (xyz0.x, xyz0.y, xyz0.z);
        push_back (ClipPlane (planeNormal, distance, false));
        }
    return isCCW ? 1 : -1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConvexClipPlaneSet::AppendCrossings (CurveVectorCR curves, bvector<CurveLocationDetailPair> &crossings) const
    {
    for (auto &plane: *this)
        {
        plane.AppendCrossings (curves, crossings);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ConvexClipPlaneSet::AppendCrossings (ICurvePrimitiveCR curve, bvector<CurveLocationDetailPair> &crossings) const
    {
    for (auto &plane: *this)
        {
        plane.AppendCrossings (curve, crossings);
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPlaneSet::ClipPlaneSet (ClipPlaneCP planes, size_t nPlanes)
    {
    push_back (ConvexClipPlaneSet (planes, nPlanes));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPlaneSet::ClipPlaneSet (ConvexClipPlaneSetCR convexSet)
    {
    push_back (convexSet);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPlaneContainment ClipPlaneSet::ClassifyPointContainment (DPoint3dCP points, size_t nPoints, bool onIsOutside) const
    {
    for (ConvexClipPlaneSetCR convexSet: *this)
        {
        ClipPlaneContainment thisStatus;

        if (ClipPlaneContainment_StronglyOutside != (thisStatus = convexSet.ClassifyPointContainment (points, nPoints, onIsOutside)))
            return thisStatus;
        }

    return ClipPlaneContainment_StronglyOutside;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
*
*  Doesn't attempt to find true minimum range for set - only handles aligned planes.
*  (this is all that is required by our current (internal) users.)
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ConvexClipPlaneSet::GetRange (DRange3dR range, TransformCP transform) const
    {
    RotMatrix   idMatrix = RotMatrix::FromIdentity ();
    DRange3d    s_bigRange = DRange3d::From (-1.0E20, -1.0E20, -1.0E20, 1.0E20, 1.0E20, 1.0E20);

    range = s_bigRange;
    for (ClipPlane clipPlane: *this)       // Not using reference - copy intentionally.
        {
        if (NULL != transform)
            clipPlane.TransformInPlace (*transform);

        double      *minP = &range.low.x, *maxP = &range.high.x;
        for (size_t i=0; i<3; i++, minP++, maxP++)
            {
            DVec3d      direction;

            idMatrix.GetColumn (direction, (int) i);

            if (clipPlane.GetNormal().IsParallelTo (direction))
                {
                if (clipPlane.GetNormal().DotProduct (direction) > 0.0)
                    {
                    if (clipPlane.GetDistance() > *minP)
                        *minP = clipPlane.GetDistance();
                    }
                else
                    {
                    if (-clipPlane.GetDistance() < *maxP)
                        *maxP = -clipPlane.GetDistance();
                    }
                }
            }
        }
    return !range.IsEqual (s_bigRange);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ClipPlaneSet::GetRange (DRange3dR range, TransformCP transform) const
    {
    range.Init ();

    for (ConvexClipPlaneSetCR planeSet: *this)
        {
        DRange3d        thisRange;

        if (planeSet.GetRange (thisRange, transform))
            range.Extend (thisRange);
        }
    return !range.IsNull();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipPlaneSet::AppendCrossings (CurveVectorCR curves, bvector<CurveLocationDetailPair> &crossings) const
    {
    for (ConvexClipPlaneSetCR planeSet: *this)
        {
        planeSet.AppendCrossings (curves, crossings);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipPlaneSet::AppendCrossings (ICurvePrimitiveCR curve, bvector<CurveLocationDetailPair> &crossings) const
    {
    for (ConvexClipPlaneSetCR planeSet: *this)
        {
        planeSet.AppendCrossings (curve, crossings);
        }
    }

static void AppendPrimitiveStartEnd (ICurvePrimitiveCR curve, bvector<CurveLocationDetailPair> &crossings);

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void AppendPrimitiveStartEnd (CurveVectorCR curves, bvector<CurveLocationDetailPair> &crossings)
    {
    for (auto &cp : curves)
        AppendPrimitiveStartEnd (*cp, crossings);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void AppendPrimitiveStartEnd (ICurvePrimitiveCR curve, bvector<CurveLocationDetailPair> &crossings)
    {
    auto cv = curve.GetChildCurveVectorCP ();
    if (cv)
        {
        AppendPrimitiveStartEnd (*cv, crossings);
        }
    else
        {
        DPoint3d xyz0, xyz1;
        curve.GetStartEnd (xyz0, xyz1);
        crossings.push_back (CurveLocationDetailPair (&curve, 0.0, xyz0));
        crossings.push_back (CurveLocationDetailPair (&curve, 1.0, xyz1));
        }
    }
bool CurveLocationDetailPair::cb_compareCurveFraction (CurveLocationDetailPairCR dataA, CurveLocationDetailPairCR dataB)
    {
    ptrdiff_t a = dataA.detailA.curve - dataB.detailB.curve;
    if (a != 0)
        return a < 0;
    if (dataA.detailA.fraction < dataB.detailA.fraction)
        return true;
    if (dataA.detailA.fraction > dataB.detailA.fraction)
        return false;
    if (dataA.detailB.fraction < dataB.detailB.fraction)
        return true;
    if (dataA.detailB.fraction > dataB.detailB.fraction)
        return false;
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* To be called when simple curve crossings are "all out".
* Need to decide if the clip sets are "completely in" the region.
* CLAIM: take any plane from the clip sets.
* Intersect the plane with the curve vectors.
* Classify that curve.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ClipPlaneContainment ClassifyRegionContainment
(
CurveVectorCR curves,
ClipPlaneSetCR clipSet,
ClipPlaneSetCP maskSet
)
    {
    for (auto &convexSet : clipSet)
        {
        for (auto &plane : convexSet)
            {
            auto dplane = plane.GetDPlane3d ();
            auto lines = curves.PlaneSection (dplane);
            if (!lines.IsValid ())
                return ClipPlaneContainment::ClipPlaneContainment_StronglyOutside;
            auto c = ClipPlaneSet::ClassifyCurvePrimitiveInSetDifference (*lines, clipSet, maskSet);
            // this is odd.  We really should just return c.
            // but then the compiler says the for(auto &plane) line is unreachable.
            // Removing (commenting) either of this return or the final one (outside both loops) quiets the compiler.
            if (c == ClipPlaneContainment::ClipPlaneContainment_Ambiguous
                || c == ClipPlaneContainment::ClipPlaneContainment_StronglyOutside
                || c == ClipPlaneContainment::ClipPlaneContainment_StronglyInside
                )
                return c;
            }
        }
    return ClipPlaneContainment::ClipPlaneContainment_Ambiguous;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ClipPlaneContainment ClassifyCrossings
(
bvector<CurveLocationDetailPair> &crossings,
ClipPlaneSetCR clipSet,
ClipPlaneSetCP maskSet,
bool considerRegions,
CurveVectorCP curves
)
    {
    std::sort (crossings.begin (), crossings.end (), CurveLocationDetailPair::cb_compareCurveFraction);
    size_t numIn = 0;
    size_t numOut = 0;
    bvector<double>fractions;
    double tol = DoubleOps::SmallMetricDistance ();
    if (curves != nullptr)
        tol = curves->ResolveTolerance (tol);
    if (crossings.size () > 0)
        {
        for (size_t i0 = 0;
                i0 < crossings.size () && (numIn == 0 || numOut == 0);)
            {
            auto curve = crossings[i0].detailA.curve;
            fractions.clear ();
            fractions.push_back (crossings[i0].detailA.fraction);
            size_t i1;
            for (i1 = i0 + 1; i1 < crossings.size () && crossings[i1].detailA.curve == curve;)
                {
                fractions.push_back (crossings[i1].detailA.fraction);
                i1++;
                }
            size_t numInterval = fractions.size () - 1;
            for (size_t i = 0; i < numInterval; i++)
                {
                double fraction0 = fractions[i];
                double fraction1 = fractions[i + 1];
                if (!DoubleOps::AlmostEqualFraction (fraction0, fraction1))
                    {
                    double midFraction = 0.5 * (fraction0 + fraction1);
                    DPoint3d xyz;
                    curve->FractionToPoint (midFraction, xyz);
                    auto inClip = clipSet.IsPointOnOrInside (xyz, tol);
                    auto inMask= maskSet != nullptr ?  maskSet->IsPointOnOrInside (xyz, tol) : false;
                    if (inClip && !inMask)
                        {
                        numIn++;
                        }
                    else
                        {
                        numOut++;
                        }
                    }
                }
            //  advance to next block !!!
            i0 = i1;
            }
        }

    if (numIn == 0 && numOut == 0)
       return ClipPlaneContainment::ClipPlaneContainment_StronglyOutside;

    if (numIn == 0 && numOut > 0)
        {
        if (!considerRegions || (curves == nullptr) || !curves->IsAnyRegionType ())
            return ClipPlaneContainment::ClipPlaneContainment_StronglyOutside;
        //
        return ClassifyRegionContainment (*curves, clipSet, maskSet);
        }
    if (numIn > 0 && numOut == 0)
        return ClipPlaneContainment::ClipPlaneContainment_StronglyInside;
    return ClipPlaneContainment::ClipPlaneContainment_Ambiguous;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GEOMDLLIMPEXP ClipPlaneContainment ClipPlaneSet::ClassifyCurveVectorInSetDifference
(
CurveVectorCR curves,
ClipPlaneSetCR clipSet,
ClipPlaneSetCP maskSet,
bool treatRegions
)
    {
    bvector<CurveLocationDetailPair> crossings;
    clipSet.AppendCrossings (curves, crossings);
    if (maskSet != nullptr)
        maskSet->AppendCrossings (curves, crossings);
    AppendPrimitiveStartEnd (curves, crossings);
    return ClassifyCrossings (crossings, clipSet, maskSet, treatRegions, &curves);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GEOMDLLIMPEXP ClipPlaneContainment ClipPlaneSet::ClassifyCurvePrimitiveInSetDifference
(
ICurvePrimitiveCR curve,
ClipPlaneSetCR clipSet,
ClipPlaneSetCP maskSet
)
    {
    bvector<CurveLocationDetailPair> crossings;
    clipSet.AppendCrossings (curve, crossings);
    if (maskSet != nullptr)
        maskSet->AppendCrossings (curve, crossings);
    AppendPrimitiveStartEnd (curve, crossings);
    return ClassifyCrossings (crossings, clipSet, maskSet, false, nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPlaneContainment ClipPlaneSet::ClassifyPolyfaceInSetDifference
(
PolyfaceQueryCR polyface,
ClipPlaneSetCR clipSet,
ClipPlaneSetCP maskSet
)
    {
    auto visitor = PolyfaceVisitor::Attach (polyface);
    ClipPlaneSetPolygonClipContext context (clipSet, maskSet);
    BVectorCache<DPoint3d> insideA;
    BVectorCache<DPoint3d> outsideA;
    BVectorCache<DPoint3d> insideB;
    BVectorCache<DPoint3d> outsideB;
    size_t numIn = 0;
    size_t numOut = 0;
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        insideA.ClearToCache ();
        outsideA.ClearToCache ();
        context.ClipAndCollect (visitor->Point (), clipSet, insideA, outsideA);
        if (!maskSet)
            {
            numIn += insideA.size ();
            numOut += outsideA.size ();
            }
        else
            {
            numOut += outsideA.size ();
            // insides need second split by masks . .
            for (auto &shard : insideA)
                {
                context.ClipAndCollect (shard, *maskSet, insideB, outsideB);
                // (inside, outside are reversed in the mask holes!!)
                numOut += insideB.size ();
                numIn += outsideB.size ();
                }
            }
        if (numIn > 0 && numOut > 0)
            return ClipPlaneContainment::ClipPlaneContainment_Ambiguous;
        }
    if (numIn > 0 && numOut == 0)
        return ClipPlaneContainment::ClipPlaneContainment_StronglyInside;
    if (numIn == 0 && numOut > 0)
        return ClipPlaneContainment::ClipPlaneContainment_StronglyOutside;
    return ClipPlaneContainment::ClipPlaneContainment_Ambiguous;
    }

// add polygons -- also interpolate from visitor if active.
static void AddPolygonsToMesh(PolyfaceHeaderPtr &mesh, BVectorCache<DPoint3d> &shards, PolyfaceVisitorR visitor)
    {
    IndexedParameterMap map;
    if(map.ConstructMapping (visitor.Point ()))
        {
        for (auto &shard : shards)
            mesh->AddPolygon(shard, visitor, map);
        }
    }

static void AddPolygonsToMesh (PolyfaceHeaderPtr *mesh, BVectorCache<DPoint3d> &shards)
    {
    if (mesh != nullptr)
        {
        for (auto &shard : shards)
            (*mesh)->AddPolygon (shard);
        }
    }


static void AddTrianglesToMesh(PolyfaceHeaderPtr *mesh,
    bvector<DTriangle3d> &triangles,    // world coordinates, to save
    bool reverse,                       // reverse coordinate order (does not affect normal)
    bvector<DPoint3d> &work,            // work array for coordinates
    TransformCR worldToParameterSpace,  // transform for parameters.
    DVec3dCR normal,                    // normal (same for all)
    bool compressNormal                 // if true, make only one normal.  If false, make same number of normals as points and params
    )
    {
    if (mesh != nullptr)
        {
        DVec3d myNormal = normal;
        if (reverse)
            myNormal = normal;
        for (auto &t : triangles)
            {
            work.clear();
            work.push_back(t.point[0]);
            work.push_back(t.point[1]);
            work.push_back(t.point[2]);
            (*mesh)->AddPolygon(work, worldToParameterSpace, normal, compressNormal, reverse);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipPlaneSet::ClipToSetDifference
(
PolyfaceQueryCR polyface,
ClipPlaneSetCR clipSet,
ClipPlaneSetCP maskSet,
PolyfaceHeaderPtr *inside,
PolyfaceHeaderPtr *outside
)
    {
    if (inside != nullptr)
        *inside = PolyfaceHeader::CreateVariableSizeIndexed ();
    if (outside != nullptr)
        *outside = PolyfaceHeader::CreateVariableSizeIndexed ();
    auto visitor = PolyfaceVisitor::Attach (polyface);
    ClipPlaneSetPolygonClipContext context (clipSet, maskSet);
    BVectorCache<DPoint3d> insideA;
    BVectorCache<DPoint3d> outsideA;
    BVectorCache<DPoint3d> insideB;
    BVectorCache<DPoint3d> outsideB;
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        insideA.ClearToCache ();
        outsideA.ClearToCache ();
        context.ClipAndCollect (visitor->Point (), clipSet, insideA, outsideA);
        if (!maskSet)
            {
            AddPolygonsToMesh (outside, outsideA);
            AddPolygonsToMesh (inside, insideA);
            }
        else
            {
            // outside of clipper is done ..
            AddPolygonsToMesh (outside, outsideA);
                // insides need second split by masks . .
            for (auto &shard : insideA)
                {
                context.ClipAndCollect (shard, *maskSet, insideB, outsideB);
                // YES  -- inside/outside names are swapped because this is the result of a mask clip
                AddPolygonsToMesh (outside, insideB);
                AddPolygonsToMesh (inside, outsideB);
                }
            }
        }
    if (inside != nullptr)
        (*inside)->Compress ();
    if (outside != nullptr)
        (*outside)->Compress ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipPlaneSet::SweptPolygonClipPolyface
(
PolyfaceQueryCR polyface,
bvector<DPoint3d> &polygon,
DVec3dCR sweepDirection,
bool constructNewFacetsOnClipSetPlanes,
PolyfaceHeaderPtr *inside,
PolyfaceHeaderPtr *outside
)
    {
    bvector<BoolTypeForVector> interiorFlag; // empty ==> all active.
    ClipPlaneSet clipper = ClipPlaneSet::FromSweptPolygon (polygon.data (), polygon.size (), &sweepDirection);
    ClipPlaneSet::ClipPlaneSetIntersectPolyface (polyface, clipper, constructNewFacetsOnClipSetPlanes, inside, outside);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipPlaneSet::ClipPlaneSetIntersectPolyface
(
    PolyfaceQueryCR polyface,
    ClipPlaneSetCR clipSet,
    bool constructNewFacetsOnClipSetPlanes,
    PolyfaceHeaderPtr *inside,
    PolyfaceHeaderPtr *outside
)
    {
    ClipPlaneSetIntersectPolyface (polyface, clipSet,
            inside, true, constructNewFacetsOnClipSetPlanes,
            outside, true, constructNewFacetsOnClipSetPlanes,
            nullptr);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipPlaneSet::ClipPlaneSetIntersectPolyface
(
PolyfaceQueryCR polyface,
ClipPlaneSetCR clipSet,
PolyfaceHeaderPtr *inside,
bool keepPolyfaceInsideParts,
bool keepCutFacesWithInside,
PolyfaceHeaderPtr *outside,
bool keepPolyfaceOutsideParts,
bool keepCutFacesWithOutside,
bvector<bvector<DPoint3d>> *cutEdges
)
    {
    if (inside != nullptr)
        {
        *inside = PolyfaceHeader::CreateVariableSizeIndexed ();
        (*inside)->ActivateVectorsForIndexingCR (polyface);
        }
    if (outside != nullptr)
        {
        *outside = PolyfaceHeader::CreateVariableSizeIndexed ();
        (*outside)->ActivateVectorsForIndexingCR(polyface);
        }
    auto visitor = PolyfaceVisitor::Attach (polyface);
    ClipPlaneSetPolygonClipContext context (clipSet, nullptr);
    BVectorCache<DPoint3d> insideA;
    BVectorCache<DPoint3d> outsideA;

    if (keepPolyfaceInsideParts || keepPolyfaceOutsideParts)
        {
        // the mesh contributes faces, clipped in a direct way by the clip plane set . . .
        for (visitor->Reset (); visitor->AdvanceToNextFace ();)
            {
            insideA.ClearToCache ();
            outsideA.ClearToCache ();
            context.ClipAndCollect (visitor->Point (), clipSet, insideA, outsideA);
            if (outside)
                AddPolygonsToMesh (*outside, outsideA, *visitor);
            if (inside)
                AddPolygonsToMesh (*inside, insideA, *visitor);
            }
        }

    if (keepCutFacesWithInside || keepCutFacesWithOutside || cutEdges != nullptr)
        {
        // Each infinite plane of the clip plane set creates a section with the mesh,
        // and the sections are clipped by the other clip planes.
        // unused - DRange3d range = polyface.PointRange ();
        bvector<DPoint3d> work;
        bvector<DPoint3d> clippedXYZ;
        for (auto &convexSet : clipSet)
            {
            for (auto &plane : convexSet)
                {
                if (plane.IsVisible ())
                    {
                    auto dplane = plane.GetDPlane3d ();
                    auto localToWorld = plane.GetLocalToWorldTransform (false);
                    Transform worldToLocal;
                    worldToLocal.InverseOf (localToWorld);
                    auto section = polyface.PlaneSlice (dplane, true);
                    if (section.IsValid ())
                        {
                        bvector<bvector<bvector<DPoint3d>>> regions;
                        bvector<bvector<DPoint3d>> clippedLoopsXYZ;
                        section->CollectLinearGeometry (regions);
                        auto parityRegion = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);
                        for (auto &region : regions)
                            {
                            for (auto &loop : region)
                                {
                                convexSet.ConvexPolygonClip (loop, clippedXYZ, work, 1);
                                if (clippedXYZ.size () > 0)
                                    clippedLoopsXYZ.push_back (clippedXYZ);
                                }
                            }
                        if (clippedLoopsXYZ.size () > 0)
                            {
                            bvector<DTriangle3d> triangles;
    //                        PolygonOps::FixupAndTriangulateSpaceLoops (clippedLoopsXYZ, triangles);
                            PolygonOps::FixupAndTriangulateProjectedLoops (clippedLoopsXYZ, localToWorld, worldToLocal, triangles);
                            DVec3d normal0 = DVec3d::FromMatrixColumn (localToWorld, 2);
                            DVec3d normal1 = DVec3d::FromScale (normal0, -1.0);
                            AddTrianglesToMesh (inside, triangles, true, work,
                                    worldToLocal, normal1, true);
                            AddTrianglesToMesh (outside, triangles, false, work,
                                    worldToLocal, normal0, true);
                            }
                        }
                    }
                }
            }
        }

    if (inside != nullptr)
        (*inside)->Compress ();
    if (outside != nullptr)
        (*outside)->Compress ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipPlaneSet::ClipPlaneSetSectionPolyface
(
    PolyfaceQueryCR polyface,
    ClipPlaneSetCR clipSet,
    PolyfaceHeaderPtr *planeSections,
    bvector<bvector<DPoint3d>> *linestrings,
    ValidatedDouble &colinearEdgeTolerance
)
    {
    if (planeSections != nullptr)
        {
        *planeSections = PolyfaceHeader::CreateVariableSizeIndexed();
        (*planeSections)->ActivateVectorsForIndexingCR(polyface);
        }
    if (linestrings)
        linestrings->clear();
    // Each infinite plane of the clip plane set creates a section with the mesh,
    // and the sections are clipped by the other clip planes.
    // unused - DRange3d range = polyface.PointRange ();
    bvector<DPoint3d> work;
    bvector<DPoint3d> clippedXYZ;
    for (auto &convexSet : clipSet)
        {
        for (auto &plane : convexSet)
            {
            if (plane.IsVisible())
                {
                auto dplane = plane.GetDPlane3d();
                auto localToWorld = plane.GetLocalToWorldTransform(false);
                Transform worldToLocal;
                worldToLocal.InverseOf(localToWorld);
                auto section = polyface.PlaneSlice(dplane, true);
                if (section.IsValid())
                    {
                    bvector<bvector<bvector<DPoint3d>>> regions;
                    bvector<bvector<DPoint3d>> clippedLoopsXYZ;
                    section->CollectLinearGeometry(regions);
                    for (auto &region : regions)
                        {
                        for (auto &loop : region)
                            {
                            convexSet.ConvexPolygonClip(loop, clippedXYZ, work, 1);
                            if (colinearEdgeTolerance.IsValid ()){
                                PolylineOps::CompressColinearPoints (clippedXYZ,
                                        colinearEdgeTolerance.Value (), true, true);
                                }
                            if (clippedXYZ.size() > 0)
                                {
                                clippedLoopsXYZ.push_back(clippedXYZ);
                                if (linestrings != nullptr)
                                    linestrings->push_back (clippedXYZ);
                                }
                            }
                        }
                    if (clippedLoopsXYZ.size() > 0)
                        {
                        bvector<DTriangle3d> triangles;
                        //                        PolygonOps::FixupAndTriangulateSpaceLoops (clippedLoopsXYZ, triangles);
                        PolygonOps::FixupAndTriangulateProjectedLoops(clippedLoopsXYZ, localToWorld, worldToLocal, triangles);
                        DVec3d normal0 = DVec3d::FromMatrixColumn(localToWorld, 2);
                        DVec3d normal1 = DVec3d::FromScale(normal0, -1.0);
                        AddTrianglesToMesh(planeSections, triangles, true, work,
                            worldToLocal, normal1, true);

                        }
                    }
                }
            }
        }
    if (planeSections)
        (*planeSections)->Compress ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipPlaneSet::ClipPlaneSetPolyfaceIntersectionEdges
(
PolyfaceQueryCR polyface,
ClipPlaneSetCR clipSet,
bvector<bvector<DPoint3d>> &linestrings
)
    {
    linestrings.clear();
    // Each infinite plane of the clip plane set creates a section with the mesh,
    // and the sections are clipped by the other clip planes.
    bvector<DPoint3d> work;
    bvector<DPoint3d> clippedXYZ;
    bvector<DSegment1d> segmentFractions;
    bvector<DSegment3d> segments;
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (polyface);
    bvector<DPoint3d> &points = visitor->Point ();
    bvector<DPoint3d> workPoints;
    for (visitor->Reset(); visitor->AdvanceToNextFace();)
        {
        for (auto &convexSet : clipSet)
            {
            for (auto &plane : convexSet)
                {
                if (plane.IsVisible())
                    {
                    convexSet.AppendPolygonPlaneIntersectionEdgesInConvexSet(plane, points, segments, workPoints);
                    }
                }
            }
        }
    PolylineOps::AssemblePolylines(segments, linestrings);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr ClipPlaneSet::ClipPlanarRegion
(
CurveVectorCR planarRegion,
TransformR    localToWorld,
TransformR    worldToLocal
) const
    {
    DRange3d localRange;
    auto localRegion = planarRegion.CloneInLocalCoordinates (LOCAL_COORDINATE_SCALE_01RangeLargerAxis,
            localToWorld, worldToLocal, localRange);
    if (localRegion.IsValid ())
        {
        bvector<DPoint3d> rectangle {
            DPoint3d::From (0,0,0),
            DPoint3d::From (1,0,0),
            DPoint3d::From (1,1,0),
            DPoint3d::From (0,1,0),
            DPoint3d::From (0,0,0)};
        localToWorld.Multiply (rectangle);
        bvector<DPoint3d> work;
        bvector<DPoint3d> fragment;
        bvector<bvector<DPoint3d>> allFragments;
        // Assemble the (union of) fragments from ConvexClipPlaneSet clips
        for (auto &convexSet : *this)
            {
            convexSet.ConvexPolygonClip (rectangle, fragment, work);
            if (fragment.size () > 2)
                {
                allFragments.push_back (bvector<DPoint3d> ());
                allFragments.back ().swap (fragment);
                }
            }
        if (allFragments.size () == 0)
            return nullptr;
        // The fragments are presumed quasi disjoint !!!!
        // Use each one individually -- don't fuss with parity region
        CurveVectorPtr result = CurveVector::Create (CurveVector::BOUNDARY_TYPE_UnionRegion);
        for (auto &fragmentA : allFragments)
            {
            auto clipper = CurveVector::CreateLinear (fragmentA, CurveVector::BOUNDARY_TYPE_Outer, false);
            clipper->TransformInPlace (worldToLocal);
            auto clippedLocalRegion = CurveVector::AreaIntersection (*localRegion, *clipper);
            if (clippedLocalRegion.IsValid ())
                result->Add (clippedLocalRegion);
            }
        if (result->size () > 0)
            {
            result->FlattenNestedUnionRegions();
            result->TransformInPlace (localToWorld);
            return result;
            }
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool curveIsInsideClipper
(
CurveVectorCR curve,
ClipPlaneSetCR clipper
)
    {
    if (curve.empty())
        return false;
    // either all of curve is inside or outside the clipper so we only need to test some
    // point inside the curve, i.e., not an endpoint. We pick the mid point (fraction = 0.5)
    DPoint3d midPoint;
    curve[0]->FractionToPoint(0.5, midPoint);
    return clipper.IsPointInside(midPoint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClipPlaneSet::ClipPath
(
CurveVectorCR curve,
bvector<CurveVectorPtr>* pClippedCurves,
bvector<CurveLocationDetailPair>* pClippedDetailPairs
) const
    {
    if (!curve.IsOpenPath() || curve.empty())
        return false;
    if (pClippedCurves)
        pClippedCurves->clear();
    if (pClippedDetailPairs)
        pClippedDetailPairs->clear();

    // find intersections of curve and clip plane set (as CurveLocationDetailPair vector)
    // intersection can be a point or a curve (if part of the input curve is on one of the planes of the clip plane set)
    bvector<CurveLocationDetailPair> clippedDetailPairsTemp;
    bvector<CurveLocationDetail> clippedDetails;
    this->AppendCrossings(curve, clippedDetailPairsTemp);

    // flat the CurveLocationDetailPair vector to a CurveLocationDetail vector (now we only have intersection points)
    for (CurveLocationDetailPair &detailPair : clippedDetailPairsTemp)
        {
        clippedDetails.push_back(detailPair.detailA);
        if (!detailPair.SameCurveAndFraction())
            clippedDetails.push_back(detailPair.detailB);
        }

    // sort the CurveLocationDetail vector by curve primitive index and fraction
    CurveLocationDetail::SortByIndexAndFraction(clippedDetails, &curve);

    // create first and last details (we need to check them; they might be inside the clipped area)
    DPoint3d startPoint, endPoint;
    curve.GetStartEnd(startPoint, endPoint);
    CurveLocationDetail start(curve[0].get(), 0.0, startPoint);
    CurveLocationDetail end(curve[curve.size()-1].get(), 1.0, endPoint);
    CurveLocationDetail detailA, detailB;
    CurveVectorPtr clippedCurve;
    detailA = start; // used to check the first pair

    // check pairs of details to specify the curve between the details in inside or outside the clipper
    for (size_t i = 0; i < clippedDetails.size(); i++)
        {
        detailB = clippedDetails.at(i);
        clippedCurve = curve.CloneBetweenDirectedFractions(detailA, detailB);
        if (clippedCurve.IsValid() && curveIsInsideClipper(*clippedCurve, *this))
            {
            if (pClippedCurves)
                pClippedCurves->push_back(clippedCurve);
            if (pClippedDetailPairs)
                pClippedDetailPairs->push_back(CurveLocationDetailPair(detailA, detailB));
            }
        detailA = detailB;
        }

    detailB = end; // used to check the last pair
    clippedCurve = curve.CloneBetweenDirectedFractions(detailA, detailB);
    if (clippedCurve.IsValid() && curveIsInsideClipper(*clippedCurve, *this))
        {
        if (pClippedCurves)
            pClippedCurves->push_back(clippedCurve);
        if (pClippedDetailPairs)
            pClippedDetailPairs->push_back(CurveLocationDetailPair(detailA, detailB));
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClipPlaneSet::ClipCurveVector
(
CurveVectorCR curve,
bvector<CurveVectorPtr>& clippedCurves
) const
    {
    if (curve.IsAnyRegionType())
        {
        Transform localToWorld;
        Transform worldToLocal;
        CurveVectorPtr clippedCurve = this->ClipPlanarRegion(curve, localToWorld, worldToLocal);
        if (clippedCurve.IsValid())
            clippedCurves.push_back(clippedCurve);
        return true;
        }
    return ClipPath(curve, &clippedCurves);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr ClipPlaneSet::ClipAndMaskPlanarRegion
(
ClipPlaneSetCP outsideClip,
ClipPlaneSetCP holeClip,
CurveVectorCR planarRegion
)
    {
    CurveVectorPtr outerRegion, innerRegion;
    Transform localToWorld, worldToLocal;
    Transform localToWorldB, worldToLocalB;
    if (outsideClip)
        outerRegion = outsideClip->ClipPlanarRegion (planarRegion, localToWorld, worldToLocal);
    if (holeClip)
        innerRegion = holeClip->ClipPlanarRegion (planarRegion, localToWorldB, worldToLocalB);
    if (outsideClip && holeClip)
        {
        if (outerRegion.IsValid ())
            {
            if (!innerRegion.IsValid ())
                return outerRegion;
            // both are valid now ..
            outerRegion->TransformInPlace (worldToLocal);
            innerRegion->TransformInPlace (worldToLocal);
            auto result = CurveVector::AreaDifference (*outerRegion, *innerRegion);
            result->TransformInPlace (localToWorld);
            return result;
            }
        }
    else if (outsideClip)
        {
        return outerRegion;
        }
    else if (holeClip)
        {
        if (!innerRegion.IsValid ())
            return planarRegion.Clone ();
        outerRegion = planarRegion.Clone ();
        outerRegion->TransformInPlace (worldToLocal);
        innerRegion->TransformInPlace (worldToLocal);
        auto result = CurveVector::AreaDifference (*outerRegion, *innerRegion);
        result->TransformInPlace (localToWorld);
        return result;
        }
    else
        {
        // both missing. return clone.
        return planarRegion.Clone ();
        }
    return nullptr;
    }

/**
* Successively clip faces of the range against the clipper.
* Return true (immediately) when any of these clip steps returns non-empty clip.
* optionally return the representative clipped face.
* Note that this specifically tests only faces.
* If the clip is a closed clipper COMPLETELY INSIDE THE RANGE the return is false.
*/
bool ClipPlaneSet::IsAnyRangeFacePointInside(DRange3dCR range, bvector<DPoint3d> *clippedFacePoints) const
    {
    DPoint3d corners[8];
    int cornerIndices[][4] = {
        {1, 0, 2, 3},
        {4, 5, 7, 6},
        {0, 1, 5, 4},
        {1, 3, 7, 5},
        {3, 2, 6, 7},
        {2, 0, 4, 6} };
    range.Get8Corners(corners);
    bvector<DPoint3d> facePoints;
    bvector<DPoint3d> clippedPoints;
    bvector<DPoint3d> work;
    for (int i = 0; i < 6; i++)
        {
        facePoints.clear();
        for (int j = 0; j < 4; j++)
            facePoints.push_back(corners[cornerIndices[i][j]]);
        for (auto &convexClipper : *this)
            {
            convexClipper.ConvexPolygonClip(facePoints, clippedPoints, work, 1);
            if (clippedPoints.size() > 0)
                {
                if (clippedFacePoints != nullptr)
                    clippedPoints.swap(*clippedFacePoints);
                return true;
                }
            }
        }
    return false;
    }


void ClipPlaneSetCompoundClipContext::ClipPolylineToBooleanCombinationOfLineClips
(
ClipPlaneSetsWithOpcodes &clippers, //!< [in] array of clippers with annotation for how they are to be applied.
bvector<DPoint3d> const & points,
bvector<bvector<DPoint3d>> &clippedChains
)
    {
    DSegment3d segment;
    clippedChains.clear ();
    for (size_t i = 0; i + 1 < points.size(); i++)
        {
        segment.point[0] = points[i];
        segment.point[1] = points[i+1];
        BooleanCombinationOfLineClips(clippers, segment, nullptr, &insideSegments);
        for (auto &insideSegment : insideSegments)
            {
            if (clippedChains.empty()
                || !clippedChains.back ().back ().AlmostEqual (insideSegment.point[0]))
                {
                // save as start of a new chain ...
                clippedChains.push_back (bvector<DPoint3d>());
                clippedChains.back ().push_back (insideSegment.point[0]);
                clippedChains.back ().push_back (insideSegment.point[1]);
                }
            else
                {
                // back of prior chain matches
                clippedChains.back().push_back(insideSegment.point[1]);
                }
            }
        }
    }

void ClipPlaneSetCompoundClipContext::BooleanCombinationOfLineClips
(
ClipPlaneSetsWithOpcodes &clippers, //!< [in] array of clippers with annotation for how they are to be applied.
DSegment3dCR segment,               //!< [in] segment to clip
bvector<DRange1d> *insideIntervals, //!< [out] intervals that are inside
bvector<DSegment3d> *insideSegs    //! [out] segments that are inside
)
    {
    if (insideIntervals)
        insideIntervals->clear();
    if (insideSegs)
        insideSegs->clear();
    workA.clear ();
    workB.clear ();
    workC.clear ();
    result.clear ();
    DRange1d initialInterval = clippers.InitialDRange1d ();
    if (!initialInterval.IsNull())
        result.push_back(initialInterval);
    for (int i = 0; i < clippers.m_clipPlaneSets.size(); i++)
        {
        ClipPlaneSetCP clipper = clippers.m_clipPlaneSets[i];
        workA.clear();
        clipper->AppendIntervals(segment, workA);
        DRange1d::SimplifyInPlace(workA);
        int opcode = clippers.m_opcodes[i];
        if (opcode == 1)
            {
            DRange1d::IntersectSorted(result, workA, workC);
            result.swap(workC);
            }
        else if (opcode == -1)
            {
            DRange1d::DifferenceSorted(result, workA, workC);
            result.swap(workC);
            }
        else if (opcode == 2)
            {
            DRange1d::UnionSorted(result, workA, workC);
            result.swap(workC);
            }
        else if (opcode == -2)
            {
            workC.clear();
            workC.push_back(DRange1d::From(0.0, 1.0));
            DRange1d::DifferenceSorted(workC, workA, workB);
            DRange1d::UnionSorted(result, workB, workC);
            result.swap(workC);
            }
        else if (opcode == 3)
            {
            DRange1d::ParitySorted(result, workA, workC);
            result.swap(workC);
            }
        else if (opcode == -3)
            {
            workC.clear();
            workC.push_back(DRange1d::From(0.0, 1.0));
            DRange1d::DifferenceSorted(workC, workA, workB);
            DRange1d::ParitySorted(result, workB, workC);
            result.swap(workC);
            }
        else if (opcode == 4)
            {
            DRange1d::DifferenceSorted(result, workA, workC);
            result.swap(workC);
            }
        else if (opcode == -4)
            {
            workC.clear();
            workC.push_back(DRange1d::From(0.0, 1.0));
            DRange1d::DifferenceSorted(workC, workA, workB);
            DRange1d::ParitySorted(result, workB, workC);
            result.swap(workC);
            }
        // ALL OTHER OPCODES IGNORE THIS CLIPPER !!!!
        }

    if (!result.empty())
        {
        if (insideIntervals)
            {
            for (auto &a : result)
                insideIntervals->push_back(a);
            }
        if (insideSegs)
            {
            for (auto &a : result)
                insideSegs->push_back(DSegment3d::FromFractionInterval(segment, a.low, a.high));
            }
        }
    }
// Move mesh pointers from source to dest.
// . . but ignore empties.
// . . clear the source array
static void MoveMeshes (bvector<PolyfaceHeaderPtr> &source, bool clearDestination, bvector<PolyfaceHeaderPtr> &dest)
    {
    if (clearDestination)
        dest.clear ();
    for (auto & s : source)
        {
        if (s.IsValid () && s->PointIndex().size () > 0)
            dest.push_back (s);
        }
    source.clear ();
    }
static void CollectClips
(
ClipPlaneSetCP clipper,
bvector<PolyfaceHeaderPtr> const &candidates,
bvector<PolyfaceHeaderPtr> &insideParts,
bvector<PolyfaceHeaderPtr> &outsideParts
)
    {
    insideParts.clear();
    outsideParts.clear();
    for (auto &candidate : candidates)
        {
        if (candidate.IsValid())
            {
            PolyfaceHeaderPtr insideA, outsideA;
            ClipPlaneSet::ClipPlaneSetIntersectPolyface(*candidate, *clipper, false, &insideA, &outsideA);
            insideParts.push_back(insideA);
            outsideParts.push_back(outsideA);
            }
        }
    }
void ClipPlaneSetCompoundClipContext::BooleanCombinationOfMeshClips
(
ClipPlaneSetsWithOpcodes &clippers, //!< [in] array of clippers with annotation for how they are to be applied.
PolyfaceHeaderPtr &mesh,
bvector<PolyfaceHeaderPtr> &clipResult
)
    {
    clipResult.clear ();
    // multiple meshes of inside and outside parts are carried from step to step.
    // (NOT combined -- caller can do that all at once at end)
    // The set of Ptr's "in the arrays" varies over the computation, but
    // the contents of each contents does NOT vary -- hence the input mesh
    // can be placed in the arrays and eventually replaced.
    bvector<PolyfaceHeaderPtr> insideParts;
    bvector<PolyfaceHeaderPtr> outsideParts;
    // These are always loaded and cleared within a step.
    bvector<PolyfaceHeaderPtr> newInsidePartsA, newInsidePartsB;
    bvector<PolyfaceHeaderPtr> newOutsidePartsA, newOutsidePartsB;
    bvector<PolyfaceHeaderPtr> work;
    if (clippers.m_initialState)
        insideParts.push_back (mesh);
    else
        outsideParts.push_back (mesh);

    for (int i = 0; i < clippers.m_clipPlaneSets.size(); i++)
        {
        ClipPlaneSetCP clipper = clippers.m_clipPlaneSets[i];
        int opcode = clippers.m_opcodes[i];

        if (opcode == 1 || opcode == -1)
            {
            if (opcode > 0)
                CollectClips (clipper, insideParts, newInsidePartsA, newOutsidePartsA);
            else
                CollectClips(clipper, insideParts, newOutsidePartsA, newInsidePartsA);
            // intersect.  clip insideParts and shuffle shards to outside
            MoveMeshes (newInsidePartsA, true, insideParts);
            MoveMeshes(newOutsidePartsA, false, outsideParts);
            }
        if (opcode == 2 || opcode == -2)
            {
            // union.  clip outside and shuffle . .
            if (opcode > 0)
                CollectClips(clipper, outsideParts, newInsidePartsA, newOutsidePartsA);
            else
                CollectClips(clipper, outsideParts, newOutsidePartsA, newInsidePartsA);
            MoveMeshes(newInsidePartsA, false, insideParts);
            MoveMeshes(newOutsidePartsA, true, outsideParts);
            }
        if (opcode == 3 || opcode == -3)
            {
            // parity.
            CollectClips(clipper, insideParts, newInsidePartsA, newOutsidePartsA);
            CollectClips(clipper, outsideParts, newInsidePartsB, newOutsidePartsB);
            if (opcode > 0)
                {
                MoveMeshes(newOutsidePartsA, true, insideParts);
                MoveMeshes(newInsidePartsB, false, insideParts);

                MoveMeshes(newInsidePartsA, true, outsideParts);
                MoveMeshes(newOutsidePartsB, false, outsideParts);
                }
            else
                {
                MoveMeshes(newInsidePartsA, true, insideParts);
                MoveMeshes(newOutsidePartsB, false, insideParts);

                MoveMeshes(newOutsidePartsA, true, outsideParts);
                MoveMeshes(newInsidePartsB, false, outsideParts);
                }
            }
        if (opcode == 4 || opcode == -4)
            {
            // subtract outside parts.
            //  A - B == A intersect ~B
            // just the reverse of (1) and (-1)
            if (opcode < 0)
                CollectClips (clipper, insideParts, newInsidePartsA, newOutsidePartsA);
            else
                CollectClips(clipper, insideParts, newOutsidePartsA, newInsidePartsA);
            // intersect.  clip insideParts and shuffle shards to outside
            MoveMeshes (newInsidePartsA, true, insideParts);
            MoveMeshes(newOutsidePartsA, false, outsideParts);
            }

        // ALL OTHER OPCODES IGNORE THIS CLIPPER !!!!
        }
    clipResult.clear ();
    clipResult.swap (insideParts);
    }

GEOMDLLIMPEXP void ConvexClipPlaneSet::AppendPolygonPlaneIntersectionEdgesInConvexSet
(
ClipPlaneCR sectionPlane,
bvector<DPoint3d> &points,
bvector<DSegment3d> &segments,
bvector<DPoint3d> &workPoints
) const
    {
    double f0, f1;
    sectionPlane.PolygonCrossings(points, workPoints);
    if (workPoints.size() == 2)
        {
        auto segment = DSegment3d::From(workPoints[0], workPoints[1]);
        if (ClipBoundedSegment(segment.point[0], segment.point[1], f0, f1, -1.0, &sectionPlane))
            {
            segments.push_back(DSegment3d::FromFractionInterval(segment, f0, f1));
            }
        }
    }

END_BENTLEY_GEOMETRY_NAMESPACE
