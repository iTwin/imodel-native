/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/polyface/ClipPlaneSet.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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

ConvexClipPlaneSet ConvexClipPlaneSet::FromXYPolyLine (bvector<DPoint3d> &points, bvector<bool> &interior, bool leftIsInside)
    {
    ConvexClipPlaneSet  convexSet;
    for (size_t i0 = 0; i0 + 1 < points.size (); i0++)
        {
        DVec3d edgeVector = DVec3d::FromStartEnd (points[i0], points[i0 + 1]);
        DVec3d perp = DVec3d::FromCCWPerpendicularXY (edgeVector);
        if (!leftIsInside)
            perp.Negate ();
        if (perp.Normalize ())
            convexSet.push_back (ClipPlane (perp, points[i0], interior[i0], interior[i0]));
        }

    return convexSet;
    }

ClipPlaneSet  ClipPlaneSet::FromSweptPolygon (DPoint3dCP points, size_t n, DVec3dCP direction)
    {
    return FromSweptPolygon (points, n, direction, nullptr);
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
                double distance = inwardNormal.DotProduct (xyz0.x, xyz0.y, xyz0.z);
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
bvector<DPoint3d> &work         //!< [inout] extra polygon
) const
    {
    output = input;
    for (auto &plane : *this)
        {
        if (output.empty ())
            break;
        plane.ConvexPolygonClipInPlace (output, work);
        }
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
bool clearOutside
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
    for (auto &plane : *this)
        {
        if (work1.empty())
            break;
        outside.PushFromCache ();
        plane.ConvexPolygonSplitInsideOutside (work1, inside, outside.back ());
#ifdef CheckAreaXY
Check ( PolygonOps::AreaXY (work1),
        PolygonOps::AreaXY (inside) + PolygonOps::AreaXY (outside.back ())
        );
        area1 += PolygonOps::AreaXY (outside.back ());
#endif
        inside.swap (work1);
        if (outside.back ().empty ())
            outside.PopToCache();
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
* @bsimethod                                                    RayBentley      03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPlaneSet::ClipPlaneSet (ClipPlaneCP planes, size_t nPlanes) : T_ConvexClipPlaneSets (1)
    {
    front() = ConvexClipPlaneSet (planes, nPlanes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ClipPlaneSet::ClipPlaneSet (ConvexClipPlaneSetCR convexSet) : T_ConvexClipPlaneSets (1)
    {
    front() = convexSet;
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



END_BENTLEY_GEOMETRY_NAMESPACE
