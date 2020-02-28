/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>


BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#define fc_hugeVal 1e37
                 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      01/2011
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
* @bsimethod                                                    RayBentley      03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ConvexClipPlaneSet::ConvexClipPlaneSet (ClipPlaneCP planes, size_t n) : T_ClipPlanes (n)
    {
    memcpy (&front(), planes, n * sizeof (ClipPlane));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConvexClipPlaneSet::IsPointInside (DPoint3dCR point) const
    {
    for (ClipPlaneCR plane: *this)
        if (!plane.IsPointOnOrInside (point))
            return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2013
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
* @bsimethod                                                    RayBentley      03/2013
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
* @bsimethod                                                    RayBentley      03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClipPlaneSet::IsPointInside (DPoint3dCR point) const
    {
    for (ConvexClipPlaneSet convexSet: *this)
        if (convexSet.IsPointInside (point))
            return true;

    return false;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/07
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
* @bsimethod                                                    RayBentley      01/2011
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
* @bsimethod                                                    RayBentley      03/2013
+---------------+---------------+---------------+---------------+---------------+------*/                                               
bool ClipPlaneSet::IsPointOnOrInside (DPoint3dCR point, double tolerance) const
    {
    for (ConvexClipPlaneSetCR convexSet: *this)
        if (convexSet.IsPointOnOrInside (point, tolerance))
            return true;

    return false;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2013
+---------------+---------------+---------------+---------------+---------------+------*/                                               
bool ClipPlaneSet::IsSphereInside (DPoint3dCR point, double radius) const
    {
    for (ConvexClipPlaneSetCR convexSet: *this)
        if (convexSet.IsSphereInside (point, radius))
            return true;

    return false;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2013
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
* @bsimethod                                                    RayBentley      03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipPlaneSet::AppendIntervals (DSegment3dCR segment, bvector<DSegment1d> &intervals) const
    {
    double f0, f1;
    for (ConvexClipPlaneSetCR convexSet : *this)
        {
        if (convexSet.ClipBoundedSegment (segment.point[0], segment.point[1], f0, f1, -1.0))
            {
            intervals.push_back (DSegment1d (f0, f1));
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2013
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
* @bsimethod                                                    RayBentley      03/2013
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
* @bsimethod                                                    RayBentley      03/2013
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
* @bsimethod                                                    RayBentley      03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ConvexClipPlaneSet::TransformInPlace (TransformCR transform) 
    {
    for (ClipPlaneR plane: *this)
        plane.TransformInPlace (transform);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipPlaneSet::TransformInPlace (TransformCR transform) 
    {
    for (ConvexClipPlaneSetR convexSet: *this)
        convexSet.TransformInPlace (transform);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ConvexClipPlaneSet::MultiplyPlanesTimesMatrix(DMatrix4dCR matrix)
    {
    for (ClipPlaneR plane: *this)
        plane.MultiplyPlaneTimesMatrix(matrix);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipPlaneSet::MultiplyPlanesTimesMatrix(DMatrix4dCR matrix)
    {
    for (ConvexClipPlaneSetR convexSet: *this)
        convexSet.MultiplyPlanesTimesMatrix(matrix);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool ConvexClipPlaneSet::ClipBoundedSegment (DPoint3dCR pointA, DPoint3dCR pointB, double &fraction0, double &fraction1, double planeSign) const
    {
    fraction0 = 0.0;
    fraction1 = 1.0;

    for (ClipPlaneCR plane: *this)
        {
        double hA = planeSign * plane.EvaluatePoint (pointA);
        double hB = planeSign * plane.EvaluatePoint (pointB);
        if (hB > hA)    // STRICLY
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
        else if (hA > hB)
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
* @bsimethod                                                    EarlinLutz      03/2016
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
* @bsimethod                                                    EarlinLutz      03/2016
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
* @bsimethod                                                    EarlinLutz      03/2016
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
    for (size_t i = 0; curve.AdvanceToBezier(segment, i, false);)
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
            for (size_t i = 0; i + 1 < allRootsOnSegment.size (); i++)
                {
                double f0 = allRootsOnSegment[i];
                double f1 = allRootsOnSegment[i+1];
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
* @bsimethod                                                    EarlinLutz      04/2012
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
* @bsimethod                                                    RayBentley      03/2013
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
* @bsimethod                                                    EarlinLutz      04/2012
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
* @bsimethod                                                    EarlinLutz      01/2017
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
* @bsimethod                                                    EarlinLutz      01/2017
+--------------------------------------------------------------------------------------*/
bool ConvexClipPlaneSet::Add (ValidatedClipPlane const &plane)
    {
    if (plane.IsValid ())
        push_back (plane.Value ());
    return plane.IsValid ();
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
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
                convexSet.push_back (ClipPlane (inwardNormal, distance, !isOriginalEdge));
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
* @bsimethod                                                    Earlin.Lutz     01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ConvexClipPlaneSet::AppendCrossings (CurveVectorCR curves, bvector<CurveLocationDetailPair> &crossings) const
    {
    for (auto &plane: *this)
        {
        plane.AppendCrossings (curves, crossings);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ConvexClipPlaneSet::AppendCrossings (ICurvePrimitiveCR curve, bvector<CurveLocationDetailPair> &crossings) const
    {
    for (auto &plane: *this)
        {
        plane.AppendCrossings (curve, crossings);
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPlaneSet::ClipPlaneSet (ClipPlaneCP planes, size_t nPlanes)
    {
    push_back (ConvexClipPlaneSet (planes, nPlanes));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPlaneSet::ClipPlaneSet (ConvexClipPlaneSetCR convexSet) 
    {
    push_back (convexSet);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2013
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
* @bsimethod                                                    RayBentley      03/2013
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
* @bsimethod                                                    RayBentley      03/2013
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
* @bsimethod                                                    Earlin.Lutz     11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ClipPlaneSet::AppendCrossings (CurveVectorCR curves, bvector<CurveLocationDetailPair> &crossings) const
    {
    for (ConvexClipPlaneSetCR planeSet: *this)
        {
        planeSet.AppendCrossings (curves, crossings);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     11/17
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
* @bsimethod                                                    Earlin.Lutz     11/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void AppendPrimitiveStartEnd (CurveVectorCR curves, bvector<CurveLocationDetailPair> &crossings)
    {
    for (auto &cp : curves)
        AppendPrimitiveStartEnd (*cp, crossings);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     11/17
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
* @bsimethod                                                    Earlin.Lutz     11/17
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
* @bsimethod                                                    Earlin.Lutz     11/17
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
                    auto inClip = clipSet.IsPointOnOrInside (xyz, DoubleOps::SmallMetricDistance());
                    auto inMask= maskSet != nullptr ?  maskSet->IsPointOnOrInside (xyz, DoubleOps::SmallMetricDistance()) : false;
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
* @bsimethod                                                    Earlin.Lutz     11/17
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
* @bsimethod                                                    Earlin.Lutz     11/17
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

struct PolyfaceClipContext
{
BVectorCache<DPoint3d> m_currentCandidates;
BVectorCache<DPoint3d> m_nextCandidates;

bvector<DPoint3d> m_currentCandidate;
BVectorCache<DPoint3d> m_shards;

bvector<DPoint3d> m_inside;
bvector<DPoint3d> m_work1;
bvector<DPoint3d> m_work2;
double m_distanceTolerance;

ClipPlaneSetCR m_clipSet;
ClipPlaneSetCP m_maskSet;
PolyfaceClipContext (ClipPlaneSetCR clipSet, ClipPlaneSetCP maskSet) : m_clipSet (clipSet), m_maskSet(maskSet)
    {
    m_distanceTolerance = 0;
    }

void ClipAndCollect (bvector<DPoint3d> &polygon, ClipPlaneSetCR clipset, BVectorCache<DPoint3d> &insideShards, BVectorCache<DPoint3d> &outsideShards)
    {
    m_currentCandidates.ClearToCache ();
    m_currentCandidates.PushCopy (polygon);
    // m_candidates contains polygon content not yet found to be IN a clip set . . 
    for (auto convexSet : clipset)
        {
        while (m_currentCandidates.SwapBackPop (m_currentCandidate))
            {
            convexSet.ConvexPolygonClipInsideOutside (m_currentCandidate, m_inside, m_shards, m_work1, m_work2, true, m_distanceTolerance);
            if (m_inside.size () > 0)
                insideShards.PushCopy (m_inside);
            // shards become candidates for further clip plane sets
            m_nextCandidates.MoveAllFrom (m_shards);
            }
        m_currentCandidates.MoveAllFrom (m_nextCandidates);
        }
    for (auto &outside : m_currentCandidates)
        outsideShards.PushCopy (outside);
    }

};
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     11/17
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPlaneContainment ClipPlaneSet::ClassifyPolyfaceInSetDifference
(
PolyfaceQueryCR polyface,
ClipPlaneSetCR clipSet,
ClipPlaneSetCP maskSet
)
    {
    auto visitor = PolyfaceVisitor::Attach (polyface);
    PolyfaceClipContext context (clipSet, maskSet);
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
static void AddPolygonsToMesh (PolyfaceHeaderPtr *mesh, BVectorCache<DPoint3d> &shards)
    {
    if (mesh != nullptr)
        {
        for (auto &shard : shards)
            (*mesh)->AddPolygon (shard);
        }
    }

static void AddTrianglesToMesh(PolyfaceHeaderPtr *mesh, bvector<DTriangle3d> &triangles, bool reverse, bvector<DPoint3d> &work)
    {
    if (mesh != nullptr)
        {
        for (auto &t : triangles)
            {
            work.clear ();
            if (reverse)
                {
                work.push_back (t.point[2]);
                work.push_back (t.point[1]);
                work.push_back (t.point[0]);
                }
            else
                {
                work.push_back (t.point[0]);
                work.push_back (t.point[1]);
                work.push_back (t.point[2]);
                }
            (*mesh)->AddPolygon (work);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     11/17
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
    PolyfaceClipContext context (clipSet, maskSet);
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
* @bsimethod                                                    Earlin.Lutz     05/18
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
* @bsimethod                                                    Earlin.Lutz     11/17
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
    if (inside != nullptr)
        *inside = PolyfaceHeader::CreateVariableSizeIndexed ();
    if (outside != nullptr)
        *outside = PolyfaceHeader::CreateVariableSizeIndexed ();
    auto visitor = PolyfaceVisitor::Attach (polyface);
    PolyfaceClipContext context (clipSet, nullptr);
    BVectorCache<DPoint3d> insideA;
    BVectorCache<DPoint3d> outsideA;

    // the mesh contributes faces, clipped in a direct way by the clip plane set . . .
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        insideA.ClearToCache ();
        outsideA.ClearToCache ();
        context.ClipAndCollect (visitor->Point (), clipSet, insideA, outsideA);
        AddPolygonsToMesh (outside, outsideA);
        AddPolygonsToMesh (inside, insideA);
        }
    if (constructNewFacetsOnClipSetPlanes)
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
                                clippedLoopsXYZ.push_back (clippedXYZ);
                                }
                            }
                        if (clippedLoopsXYZ.size () > 0)
                            {
                            bvector<DTriangle3d> triangles;
    //                        PolygonOps::FixupAndTriangulateSpaceLoops (clippedLoopsXYZ, triangles);
                            PolygonOps::FixupAndTriangulateProjectedLoops (clippedLoopsXYZ, localToWorld, worldToLocal, triangles);
                            AddTrianglesToMesh (inside, triangles, true, work);
                            AddTrianglesToMesh (outside, triangles, false, work);
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
* @bsimethod                                                    Earlin.Lutz     11/17
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
        // The fragemnts are presumed quasi disjoint !!!!
        // Use each one individually -- don't fuss with parity region
        CurveVectorPtr clipper;
        CurveVectorPtr result = CurveVector::Create (CurveVector::BOUNDARY_TYPE_UnionRegion);
        for (auto &fragment : allFragments)
            {
            auto clipper = CurveVector::CreateLinear (fragment, CurveVector::BOUNDARY_TYPE_Outer, false);
            clipper->TransformInPlace (worldToLocal);
            auto clippedLocalRegion = CurveVector::AreaIntersection (*localRegion, *clipper);
            if (clippedLocalRegion.IsValid ())
                result->Add (clippedLocalRegion);
            }
        if (result->size () > 0)
            {
            result->TransformInPlace (localToWorld);
            return result;    
            }
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     11/17
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

END_BENTLEY_GEOMETRY_NAMESPACE
