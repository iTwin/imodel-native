/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/polyface/IPolyfaceConstruction_Add.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
// Problem: point-to-point equality tests ... options:
// 1) (old) use bitwise equality. But the stroking code did NOT enforce this.
// 2) enforce equality in the stroke code
// 3) use AlmostEqual (which uses smallAngle reltol)
// Nov 30, 2017 EDL use AlmostEqual in IsVisibleJoint and IsSmoothClosure.

void UpdateRefPoint
(
DPoint3dR xyzA,
bvector<DPoint3d> &xyzB,
size_t index,
bool applyBToA,
bool applyAToB
)
    {
    if (applyBToA)
        xyzA = xyzB[index];
    if (applyAToB)
        xyzB[index] = xyzA;
    }

// Return scale factors ax,ay to be applied to distance parameters so that dx,dy scales to the expected parameter space range.
static bool SetParamScales (FacetParamMode mode, double dx, double dy, double &ax, double &ay)
    {
    ax = DoubleOps::ValidatedDivide (1.0, dx, 1.0);
    ay = DoubleOps::ValidatedDivide (1.0, dy, 1.0);
    if (mode == FACET_PARAM_01BothAxes)
        {
        return true;
        }
    else if (mode == FACET_PARAM_01LargerAxis)
        {
        if (dy > dx)
            ax = dx / dy;
        else if (dx > dy)
            ay = dy / dx;
        return true;
        }
    else if (mode == FACET_PARAM_Distance)
        {
        ax = ay = 1.0;
        return true;
        }
    return false;
    }
   
// Append points, curve tangents, and trig parameters for a full ellipse scaled radially
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static size_t AppendFullEllipseStrokes
(
DEllipse3dCR ellipse,
size_t numChord,
bvector<DPoint3d> *points = NULL,
bvector<DVec3d> *tangents = NULL,
bvector<DPoint2d> *trigPoints = NULL,
double radiusFraction = 1.0
)
    {
    if (numChord > 0)
        {
        double dTheta = ellipse.sweep / numChord;
        double finalAngle = ellipse.IsFullEllipse () ? ellipse.start : ellipse.start + ellipse.sweep;
        for (size_t i = 0; i <= numChord; i++)
            {
            double theta = ellipse.start + i * dTheta;
            if (i == numChord)
                theta = finalAngle;     // Prevent bit drop
                
            double c = radiusFraction * cos (theta);
            double s = radiusFraction * sin (theta);
            if (points)
                {
                DPoint3d xyz;
                xyz.SumOf (ellipse.center, ellipse.vector0, c, ellipse.vector90, s);
                points->push_back (xyz);
                }
            if (tangents)
                {
                DVec3d tangent;
                tangent.SumOf (ellipse.vector0, -s, ellipse.vector90, c);
                tangents->push_back (tangent);
                }
            if (trigPoints)
                {
                trigPoints->push_back (DPoint2d::From (c, s));
                }
            }
        }
    return numChord;
    }


// append (cos, sin, theta), with test to make force pure horizontal/vertical.
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void AppendTrigPoint (bvector<DPoint3d>&data, double radians)
    {
    static double sZeroTol = 4.5e-15;
    double c = cos (radians);
    double s = sin (radians);
    if (fabs (c) < sZeroTol)
        {
        c = 0.0;
        s = s > 0.0 ? 1.0 : -1.0;
        }
    else if (fabs (s) < sZeroTol)
        {
        s = 0.0;
        c = c > 0.0 ? 1.0 : -1.0;
        }
    data.push_back (DPoint3d::FromXYZ (c, s, radians));
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool GetSinglePrimitiveFacetPointCount(
    IFacetOptionsR options,
    CurveVectorPtr curves,
    size_t readIndex,
    size_t &count
    )
    {
    DSegment3d segment;
    ICurvePrimitivePtr primitive = (*curves)[readIndex];
    if (primitive->TryGetLine (segment))
        {
        count = options.SegmentStrokeCount (segment);
        return true;
        }
    DEllipse3d ellipse;
    if (primitive->TryGetArc (ellipse))
        {
        count = options.EllipseStrokeCount (ellipse);
        return true;
        }
    return false;
    }

// Test if all curves at given readIndex are compatible as single primitives, and
//    return count for faceting.
static bool AreCompatiblePrimitives
    (
    IFacetOptionsR options,
    bvector<CurveVectorPtr> const &contours,
    size_t readIndex,
    size_t &maxCount
    )
    {
    maxCount = 0;
    for (size_t i = 0, n = contours.size (); i < n; i++)
        {
        size_t thisCount = 0;
        if (!GetSinglePrimitiveFacetPointCount (options, contours[i], readIndex, thisCount))
            return false;
        if (thisCount > maxCount)
            maxCount = thisCount;
        }
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void AddRuleLineNormals(IPolyfaceConstructionR builder,
    DPoint3dCR point0, DVec3dCR tangent0,
    DPoint3dCR point1, DVec3dCR tangent1,
    size_t &normalIndexA, size_t &normalIndexB)
    {
    static double s_magnitudeTolerance = 1.0e-8;
    DVec3d ruleVector = DVec3d::FromStartEnd (point1, point0);
    DVec3d normalA, normalB;
    double magT0 = tangent0.Magnitude ();
    double magT1 = tangent1.Magnitude ();
    double magRule = ruleVector.Magnitude ();
    normalA.NormalizedCrossProduct (tangent0, ruleVector);
    normalB.NormalizedCrossProduct (tangent1, ruleVector);
    // If just one of the tangents is near zero, this pulls the other normal ....
    if (magT0 < s_magnitudeTolerance * magRule)
        normalA = normalB;
    if (magT1 < s_magnitudeTolerance * magRule)
        normalB = normalA;

    normalA.Negate ();
    normalB.Negate ();
    normalIndexA = builder.FindOrAddNormal (normalA);
    normalIndexB = builder.FindOrAddNormal (normalB);
    }


static bool IsVisibleRuleEdge
(
size_t index,
bool isRightEdge,
bvector<size_t>   &indexA,
bvector<DVec3d>   &tangentA,
bvector<ICurvePrimitiveP> *curveA,
bvector<size_t>   &indexB,
bvector<DVec3d>   &tangentB,
bvector<ICurvePrimitiveP> *curveB,
size_t numQuad,      // Can index to here inclusive
bool alwaysSmooth
)
    {
    size_t neighborIndex;
    if (isRightEdge)
        {
        if (index + 1 > numQuad)
            {
            neighborIndex = 0;
            if (indexA[0] != indexA[numQuad] || indexB[0] != indexB[numQuad])
                return true;
            }
        else
            neighborIndex = index + 1;
        }
    else
        {
        if (index == 0)
            {
            neighborIndex = numQuad;
            if (indexA[0] != indexA[numQuad] || indexB[0] != indexB[numQuad])
                return true;
            }
        else
            neighborIndex = index - 1;
        }
    assert (neighborIndex < tangentA.size () && neighborIndex < tangentB.size ());
    assert (index < tangentA.size () && index < tangentB.size ());
    if (curveA != nullptr && curveB != nullptr)
        {
        if (index < curveA->size () && neighborIndex < curveA->size ())
            {
            if (curveA->at(index) != curveA->at(neighborIndex))
                return true;
            }
        if (index < curveB->size () && neighborIndex < curveB->size ())
            {
            if (curveB->at(index) != curveB->at(neighborIndex))
                return true;
            }
        }
    // If points only appear once internally, the tangent at index applies on both sides and it is smooth.
    if (indexA[index] != indexA[neighborIndex] && indexB[index] != indexB[neighborIndex])
        return false;
    if (alwaysSmooth)
        return false;
    // neighbor quad is degenerate -- have to look at tangents
    if (indexA[index] == indexA[neighborIndex] && !tangentA[index].IsParallelTo (tangentA[neighborIndex]))
        return true;
    if (indexB[index] == indexB[neighborIndex] && !tangentB[index].IsParallelTo (tangentB[neighborIndex]))
        return true;
    return false;
    }
// Test if quad with indices at i,i+1 is degenerate.
// Assumes all indices valid.
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool isDegenerateQuad
(
bvector<size_t> &indexA,
bvector<size_t> &indexB,
size_t i
)
    {
    return indexA[i] == indexA[i+1]
        && indexB[i] == indexB[i+1];
    }

struct EdgeCandidate
{
size_t m_indexA, m_indexB;
EdgeCandidate (size_t indexA, size_t indexB) : m_indexA (indexA), m_indexB (indexB) {}
bool UndirectedMatch (EdgeCandidate const &other) const
    {
    return (m_indexA == other.m_indexA && m_indexB == other.m_indexB)
        || (m_indexB == other.m_indexA && m_indexA == other.m_indexB);
    }
static void AnnounceCandidate (bvector<EdgeCandidate> &edges, size_t indexA, size_t indexB, bool visible)
    {
    // Only record if visible and does not match either of the top two edges.
    if (visible)
        {
        size_t n = edges.size ();
        EdgeCandidate candidate (indexA, indexB);
        if (n > 1 && edges[n-1].UndirectedMatch (candidate))
            return;
        if (n > 2 && edges[n-2].UndirectedMatch (candidate))
            return;
        edges.push_back (candidate);
        }
    }
};
// Emit quads between point strings.
// Caller must double first, last point indices to obtain closure.
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void AddSmoothRuledQuads (IPolyfaceConstructionR builder,
    bool visibleA,
    double fractionA,
    bvector<DPoint3d> &pointA,
    bvector<size_t>   &indexA,
    bvector<DVec3d>   &tangentA,
    bvector<DPoint2d> &uvA,
    bvector<ICurvePrimitiveP> *curveA,
    bool visibleB,
    double fractionB,
    bvector<DPoint3d> &pointB,
    bvector<size_t>   &indexB,
    bvector<DVec3d>   &tangentB,
    bvector<DPoint2d> &uvB,
    bvector<ICurvePrimitiveP> *curveB,
    double              nominalLength = 0.0,
    bool alwaysSmooth = false
    )
    {
    if (indexA.empty() || indexB.empty())
        {
        BeAssert (false);
        return;
        }



    bvector<EdgeCandidate> candidates;
    size_t numQuad = pointA.size () - 1;
    //static bool s_alwaysVisible = false;
    for (size_t i0 = 0; i0 < numQuad; i0++)
        {
        size_t i1 = i0 + 1;
        if (!isDegenerateQuad (indexA, indexB, i0))
            {
            bool leftVisible = IsVisibleRuleEdge (i0, false, indexA, tangentA, curveA, indexB, tangentB, curveB, numQuad, alwaysSmooth);
            bool rightVisible = IsVisibleRuleEdge (i1, true,  indexA, tangentA, curveA, indexB, tangentB, curveB, numQuad, alwaysSmooth);
            builder.AddPointIndexQuad (
                        indexA[i0], visibleA,
                        indexA[i1], rightVisible,
                        indexB[i1], visibleB,
                        indexB[i0], leftVisible
                        );
            EdgeCandidate::AnnounceCandidate (candidates, indexA[i0], indexB[i0], leftVisible);
            EdgeCandidate::AnnounceCandidate (candidates, indexA[i1], indexB[i1], rightVisible);
            }
        }

    if (builder.NeedNormals ())
        {
        size_t normalIndexA0, normalIndexB0, normalIndexA1, normalIndexB1;
        AddRuleLineNormals (builder, pointA[0], tangentA[0], pointB[0], tangentB[0], normalIndexA0, normalIndexB0);
        for (size_t i0 = 0; i0 < numQuad; i0++, normalIndexA0 = normalIndexA1, normalIndexB0 = normalIndexB1)
            {
            size_t i1 = i0 + 1;
            AddRuleLineNormals (builder, pointA[i1], tangentA[i1], pointB[i1], tangentB[i1], normalIndexA1, normalIndexB1);
            if (!isDegenerateQuad (indexA, indexB, i0))
                builder.AddNormalIndexQuad (normalIndexA0, normalIndexA1, normalIndexB1, normalIndexB0);
            }
        }

    if (builder.NeedParams ())
        {
        // TODO in callers --- do remap from overall parameters !!!
        // builder.RemapPseudoDistanceParams (params, distanceRange, paramRange, 1.0, 1.0);
        size_t paramIndexA0 = builder.FindOrAddParam (uvA[0]);
        size_t paramIndexB0 = builder.FindOrAddParam (uvB[0]);
        size_t paramIndexA1, paramIndexB1;
        for (size_t i0 = 0; i0 < numQuad;
                i0++,
                paramIndexA0 = paramIndexA1,
                paramIndexB0 = paramIndexB1
                )
            {
            size_t i1 = i0 + 1;
            paramIndexA1 = builder.FindOrAddParam (uvA[i1]);
            paramIndexB1 = builder.FindOrAddParam (uvB[i1]);
            if (!isDegenerateQuad (indexA, indexB, i0))
                builder.AddParamIndexQuad (paramIndexA0, paramIndexA1, paramIndexB1, paramIndexB0);
            }
        }
    if (builder.GetFacetOptionsR().GetEdgeChainsRequired())
        {
        bvector<PolyfaceEdgeChain> &chains = builder.GetClientMeshR().EdgeChain ();
        bool reverseEdgeChains = true;

        if (visibleA)
            {
            chains.push_back (PolyfaceEdgeChain (
                CurveTopologyId (CurveTopologyId::Type::SweepProfile, (uint32_t)0)));
            chains.back ().AddZeroBasedIndices (indexA);
            }

        if (visibleB)
            {
            chains.push_back (PolyfaceEdgeChain (
                CurveTopologyId (CurveTopologyId::Type::SweepProfile, (uint32_t)1)));
            chains.back ().AddZeroBasedIndices (indexB);
            }

        bvector<size_t> lateralIndex;
        size_t i = 0;
        for (EdgeCandidate const &candidate : candidates)
            {
            chains.push_back (PolyfaceEdgeChain (
                CurveTopologyId (CurveTopologyId::Type::SweepLateral, (uint32_t) i)));
            lateralIndex.clear ();
            if (reverseEdgeChains)
                {
                lateralIndex.push_back (candidate.m_indexB);
                lateralIndex.push_back (candidate.m_indexA);
                }
            else
                {
                lateralIndex.push_back (candidate.m_indexA);
                lateralIndex.push_back (candidate.m_indexB);
                }
            chains.back ().AddZeroBasedIndices (lateralIndex);
            i++;
            }
        }
    }

// Emit quads between point strings.
// Caller must double first, last point indices to obtain closure.
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void AddSmoothRuledQuads1 (IPolyfaceConstructionR builder,
    bvector<DPoint3d> &pointA,
    bvector<size_t>   &indexA,
    bvector<DVec3d>   &tangentA,
    bvector<DPoint2d> &paramA,
    bvector<ICurvePrimitiveP> *curveA,  // OPTIONAL curve pointer
    bvector<DPoint3d> &pointB,
    bvector<size_t>   &indexB,
    bvector<DVec3d>   &tangentB,
    bvector<DPoint2d> &paramB,
    bvector<ICurvePrimitiveP> *curveB,  // OPTIONAL curve pointer
    double              nominalLength,
    double            maxEdgeLength,
    double            angleRadians,
    bool alwaysSmooth = false
    )
    {
    if (maxEdgeLength > 0.0 || angleRadians > 0)
        {
        double minDistance, maxDistance;
        size_t minIndex, maxIndex;
        double minAngle, maxAngle;
        DPoint3dOps::MinMaxDistance (pointA, pointB, minDistance, minIndex, maxDistance, maxIndex);
        DVec3dOps::MinMaxAngle (tangentA, tangentB, minAngle, minIndex, maxAngle, maxIndex);
        size_t numEdge = builder.GetFacetOptionsR ().DistanceAndTurnStrokeCount (maxDistance, maxAngle);

        if (numEdge > 1)
            {
            bvector<DPoint3d> pointC = pointA;
            bvector<size_t>   indexC = indexA;
            bvector<DVec3d>   tangentC = tangentA;
            bvector<DPoint3d> pointD;
            bvector<size_t>   indexD;            
            bvector<DVec3d>   tangentD;
            bvector<DPoint2d> paramC = paramA;
            bvector<DPoint2d> paramD = paramA;  // first step will recompute immediately

            for (size_t i = 1; i <= numEdge; i++)
                {
                double fractionC = (i - 1) / (double)numEdge;
                double fractionD = i / (double)numEdge;
                for (size_t k = 0; k < paramA.size (); k++)
                    paramD[k].Interpolate (paramA[k], fractionD, paramB[k]);
                if (i == numEdge)
                    {
                    pointD = pointB;
                    indexD = indexB;
                    tangentD = tangentB;
                    fractionD = 1.0;    // prevent bit drop in division
                    }
                else
                    {
                    DPoint3dOps::InterpolateAll (pointD, pointA, fractionD, pointB);
                    DVec3dOps::InterpolateAll (tangentD, tangentA, fractionD, tangentB);
                    indexD.clear ();
                    builder.FindOrAddPoints (pointD, pointD.size (), 0, indexD);
                    // assign indices.
                    }
                AddSmoothRuledQuads (builder,
                    i == 1, fractionC,  pointC, indexC, tangentC, paramC, curveA,
                    i == numEdge, fractionD, pointD, indexD, tangentD, paramD, curveB,
                    nominalLength, alwaysSmooth);
                pointC.swap (pointD);
                indexC.swap (indexD);
                tangentC.swap (tangentD);
                paramC.swap (paramD);
                }
            return;
            }
        }
    // fall out if no splitting along the sweep ..
    AddSmoothRuledQuads (builder, 
        true, 0.0, pointA, indexA, tangentA, paramA, curveA,
        true, 1.0, pointB, indexB, tangentB, paramB, curveA, nominalLength,
        alwaysSmooth);
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool AreCompatibleBsplineCurves (bvector<CurveVectorPtr> const &contours, size_t readIndex)
    {
    size_t numBezier0 = 0, numBezier;
    for (size_t i = 0, n = contours.size (); i < n; i++)
        {
        if (readIndex >= contours[i]->size ())
            return false;
        MSBsplineCurveCP bcurve = contours[i]->at (readIndex)->GetProxyBsplineCurveCP ();
        if (NULL == bcurve)
            return false;
        numBezier = bcurve->CountDistinctBeziers ();
        if (i == 0)
            numBezier0 = numBezier;
        else
            {
            if (numBezier != numBezier0)
                return false;
            }
        }
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool AreCompatibleLineStrings
(
bvector<CurveVectorPtr> const &contours,
size_t readIndex,
size_t &pointCount
)
    {
    size_t numPoints0 = 0;
    pointCount = 0;
    for (size_t i = 0, n = contours.size (); i < n; i++)
        {
        if (readIndex >= contours[i]->size ())
            return false;
        bvector<DPoint3d> const * linestring = contours[i]->at (readIndex)->GetLineStringCP ();
        if (NULL == linestring)
            return false;
        if (i == 0)
            numPoints0 = linestring->size ();
        else
            {
            if (linestring->size () != numPoints0)
                return false;
            }
        }
    pointCount = numPoints0;
    return true;
    }

#ifdef Compile___EvaluateBlockedBezier
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void EvaluateBlockedBezier(
        IPolyfaceConstructionR builder, 
        bvector<size_t> &xyzIndex,
        bvector <DPoint3d> &xyz,
        bvector <DVec3d>   &tangent,
        bvector<DPoint3d> *xyzAccumulator,
        bvector<DPoint4d> const &poles,
        size_t index0, int order, size_t numParam)
    {
    xyzIndex.clear ();
    xyz.clear ();
    tangent.clear ();
    bsiBezierDPoint4d_appendEvaluations (&xyz, &tangent, NULL, &poles[index0], order, numParam);
    if (xyzAccumulator != NULL)
        DPoint3dOps::Append (xyzAccumulator, &xyz);
    builder.FindOrAddPoints (xyz, xyz.size (), 0, xyzIndex);
    }
#endif

/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
struct StrokeGrid
{
    bvector<DPoint3dDoubleUVCurveArrays> m_grid;
    bvector<bvector<size_t>   > m_pointIndexGrid;
    IPolyfaceConstructionR m_builder;
StrokeGrid (IPolyfaceConstructionR builder, size_t numContours)
    : m_builder (builder)
    {
    m_grid.resize (numContours);
    m_pointIndexGrid.resize (numContours);
    }


void AppendPointAndTangent (ICurvePrimitiveP curve, size_t contourIndex, DPoint3dCR xyz, DVec3dCR tangent)
    {
    if (contourIndex < m_grid.size ())
        {
        m_grid[contourIndex].m_xyz.push_back (xyz);
        m_grid[contourIndex].m_vectorU.push_back (tangent);
        m_grid[contourIndex].m_curve.push_back (curve);
        m_pointIndexGrid[contourIndex].push_back (m_builder.FindOrAddPoint (xyz));
        }
    }

/*--------------------------------------------------------------------------------**//**
* Append numEdge edges within segment to contourIndex
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void AppendSegmentStrokesToContour(ICurvePrimitiveP curve, size_t contourIndex, DSegment3dCR segment, size_t numEdge)
    {
    if (contourIndex < m_grid.size ())
        {
        DPoint3d xyzi;
        DVec3d tangenti;
        for (size_t i = 0; i <= numEdge; i++)
            {
            double f = (double) i / (double)numEdge;
            segment.FractionParameterToTangent (xyzi, tangenti, f);
            AppendPointAndTangent (curve, contourIndex, xyzi, tangenti);
            }
        }
    }

// given string of xyzB "up the grid"
// (optionally) make the front of each contour match xyzB
// copy the back of each contour to xyzB.
// (to enforce bitwise equality at closure?  maybe not used)
void ApplyStartEnd (bvector<DPoint3d> &xyzB, bool enforceStart)
    {
    assert (xyzB.size () == m_grid.size ());
    if (enforceStart)
        for (size_t i = 0; i < m_grid.size (); i++)
            m_grid[i].m_xyz.front () = xyzB[i];
    for (size_t i = 0; i < m_grid.size (); i++)
        xyzB[i] = m_grid[i].m_xyz.back ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void AnnounceGridToBuilder (bool applyTolerances)
    {
    double maxLength = 0.0;
    double maxEdgeLength = 0.0;
    double angleTol = 0.0;
    if (applyTolerances)
        {
        maxEdgeLength = m_builder.GetFacetOptionsR ().GetMaxEdgeLength ();
        angleTol = m_builder.GetFacetOptionsR ().GetAngleTolerance ();
        }
    for (size_t i = 0; i < m_grid.size (); i++)
        maxLength = DoubleOps::Max (maxLength, PolylineOps::Length (m_grid[i].m_xyz));
    size_t numI = m_grid[0].m_xyz.size ();
    size_t numJ = m_grid.size ();
    bvector<double> xDist, yDist;
    xDist.push_back (0.0);
    yDist.push_back (0.0);
    double s = 0.0;
    for (size_t i = 1; i < numI; i++)
        {
        for (size_t j = 0; j < numJ; j++)
            s += m_grid[j].m_xyz[i-1].Distance (m_grid[j].m_xyz[i]);
        xDist.push_back (s);    // Sum of x sizes in this stack of quads
        }
    s = 0.0;
    for (size_t j = 1; j < numJ; j++)
        {
        for (size_t i = 0; i < numI; i++)
            s += m_grid[j-1].m_xyz[i].Distance (m_grid[j].m_xyz[i]);
        yDist.push_back (s);    // Sum of x sizes in this stack of quads
        }
    double ax, ay;
    SetParamScales (m_builder.GetFacetOptionsR().GetParamMode (), xDist.back (), yDist.back (), ax, ay);
    for (double &x: xDist)
        x *= ax;
    for (double &y: yDist)
        y *= ay;
    bvector<DPoint2d> uvA, uvB;
    for (size_t i = 0; i < numI; i++)
        uvA.push_back (DPoint2d::From (xDist[i], yDist[0]));
    for (size_t iA = 0, iB = 1; iB < numJ; iA = iB++)
        {
        uvB.clear ();
        for (size_t i = 0; i < numI; i++)
            uvB.push_back (DPoint2d::From (xDist[i], yDist[iB]));
        AddSmoothRuledQuads1 (m_builder,
            m_grid[iA].m_xyz, m_pointIndexGrid[iA], m_grid[iA].m_vectorU, uvA, &m_grid[iA].m_curve,
            m_grid[iB].m_xyz, m_pointIndexGrid[iB], m_grid[iB].m_vectorU, uvB, &m_grid[iB].m_curve,
            maxLength,
            maxEdgeLength,
            angleTol,
            false
            );
        uvA.swap (uvB);
        }
    DRange2d distanceRange;
    distanceRange.Init ();
    distanceRange.Extend (0,0);
    distanceRange.Extend (xDist.back (), yDist.back ());
    m_builder.SetCurrentFaceParamDistanceRange (distanceRange);
    m_builder.EndFace_internal ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
* append point coordinates from a contour to dest (removing duplicates)
+--------------------------------------------------------------------------------------*/
void GetStrokesFromContour(size_t contourIndex, bvector<DPoint3d> *dest)
    {
    if (contourIndex < m_grid.size () && dest != NULL)
        {
        bvector<DPoint3d> const & source = m_grid[contourIndex].m_xyz;
        size_t n = source.size ();
        if (n < 1)
            return;
        size_t i = 0;
        if (dest->size () == 0)
            {
            dest->push_back (source.front ());
            i++;
            }
        for ( ;i < n; i++)
            {
            if (!source[i].IsEqual (dest->back ()))
                dest->push_back (source[i]);
            }
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void AddBezierEvaluationsAndIndices(
StrokeGrid &strokeGrid,
size_t contourIndex,
BCurveSegment &segment,
ICurvePrimitiveP parentCurve,
size_t numParam
)
    {
    size_t numA = m_grid[contourIndex].m_xyz.size ();
    bsiBezierDPoint4d_appendEvaluations (&m_grid[contourIndex].m_xyz, &m_grid[contourIndex].m_vectorU, NULL, segment.GetPoleP (), (int)segment.GetOrder (), numParam);
    for (size_t i = numA; i < m_grid[contourIndex].m_xyz.size (); i++)
        {
        m_pointIndexGrid[contourIndex].push_back (m_builder.FindOrAddPoint (m_grid[contourIndex].m_xyz[i]));
        m_grid[contourIndex].m_curve.push_back (parentCurve);
        }
    }

};


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void AddRuledBetweenCorrespondingPrimitives
(
StrokeGrid &strokeGrid,
bvector<CurveVectorPtr> const &contours,
size_t readIndex,
size_t numEdge
)
    {
    size_t numContour = contours.size ();
    size_t numPoint = numEdge + 1;

    if (numEdge < 1 || numContour < 2)
        return;
    double df = 1.0 / (numPoint - 1);
    for (size_t contourIndex = 0; contourIndex < numContour; contourIndex++)
        {
        for (size_t pointIndex = 0; pointIndex <= numEdge; pointIndex++)
            {
            DPoint3d xyz;
            DVec3d  tangent;
            auto curve = contours[contourIndex]->at(readIndex).get ();
            curve->FractionToPoint (
                    pointIndex * df, xyz, tangent);
            strokeGrid.AppendPointAndTangent (curve, contourIndex, xyz, tangent);
            }
        }
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool TryGetSegmentInLineString
(
ICurvePrimitiveCR prim,
DSegment3dR segment,
size_t segmentIndex
)
    {
    bvector<DPoint3d> const* points = prim.GetLineStringCP();
    if (NULL != points && segmentIndex + 1 < points->size ())
        {
        segment = DSegment3d::From (points->at(segmentIndex), points->at(segmentIndex+1));
        return true;
        }
    return false;

    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool AddRuledBetweenCorrespondingLineStrings
(
StrokeGrid &strokeGrid,
size_t minEdges,
bvector<CurveVectorPtr> const &contours,
size_t readIndex
)
    {
    IFacetOptionsR options = strokeGrid.m_builder.GetFacetOptionsR ();
    DSegment3d segment;
    size_t numContour = contours.size ();
    if (numContour < 2)
        return false;
    size_t numPoint = 0;
    if (!AreCompatibleLineStrings (contours, readIndex, numPoint))
        return false;

    for (size_t segmentIndex = 0; segmentIndex + 1 < numPoint; segmentIndex++)
        {
        size_t maxStrokes = minEdges;
        // Find max number of strokes required on this segmentIndex going up the contours
        for (size_t contourIndex = 0; contourIndex < numContour; contourIndex++)
            {
            if (readIndex >= contours[contourIndex]->size ())
                return false;
            if (!TryGetSegmentInLineString (*contours[contourIndex]->at(readIndex), segment, segmentIndex))
                return false;
            size_t numStroke = options.SegmentStrokeCount (segment);
            if (numStroke > maxStrokes)
                maxStrokes = numStroke;
            }

        // Stroke this segment from each contour
        for (size_t contourIndex = 0; contourIndex < numContour; contourIndex++)
            {
            ICurvePrimitiveP curve = contours[contourIndex]->at(readIndex).get ();
            TryGetSegmentInLineString (*curve, segment, segmentIndex);
            strokeGrid.AppendSegmentStrokesToContour (curve, contourIndex, segment, maxStrokes);
            }
        }
    return true;
    }

//   CurveVectorPtr flavor !!!!
static bool AddRuledBetweenCorrespondingBsplineCurves
    (
    StrokeGrid &strokeGrid,
    size_t minEdges,
    bvector<CurveVectorPtr> const &contours,
    size_t readIndex
    )
    {
    IFacetOptionsR options = strokeGrid.m_builder.GetFacetOptionsR ();
    size_t numContour = contours.size ();

    bvector <bvector<DPoint3d> > curvePoint (numContour);
    bvector <bvector<DVec3d> >   curveTangent (numContour);
    bvector <bvector <size_t> >  curvePointIndex (numContour);
    bvector <size_t>             curveBezierIndex (numContour);

    if (numContour < 2)
        return false;

    // Lots of possibilities for mismatch between curves.
    // Error strategy is to go through as if it's good but check for errors after each sweep up the contrours.
    // Setup up curve pointers and bezier reades for each contour ..
    bvector<BCurveSegment>segments;
    bvector<MSBsplineCurveCP>curves;
    bvector<ICurvePrimitiveP> primitives;
    segments.reserve (numContour);
    curves.reserve (numContour);
    size_t numOK = 0;
    for (size_t i = 0; i < numContour; i++)
        {
        segments.push_back (BCurveSegment ());
        CurveVectorP contour = contours[i].get ();
        curveBezierIndex[i] = 0;
        if (readIndex < contour->size ())
            {
            curves.push_back (contour->at (readIndex)->GetProxyBsplineCurveCP());
            primitives.push_back (contour->at(readIndex).get ());
            if (curves.back () != NULL)
                numOK++;
            }
        }
    if (numOK != numContour)
        return false;

    for (size_t bezierIndex = 0; true; bezierIndex++)
        {
        // Advance all the beziers ...
        size_t numAdvanced = 0;
        for (size_t i = 0; i < numContour; i++)
            {
            if (curves[i]->AdvanceToBezier (segments[i], curveBezierIndex[i]))
                numAdvanced++;
            }
        // confirm tha tall advanced over the same knot replications ...
        bool knotsMatch = true;
        for (size_t i = 1; i < numContour && knotsMatch; i++)
             if (curveBezierIndex[i] != curveBezierIndex[1])
                knotsMatch = false;
        if (numAdvanced != numContour || !knotsMatch)
            break;      // numAdvanced == 0 is the expected exit condition.
        // Get largest stroke point count among the beziers ...        
        size_t maxCount = minEdges;
        for (size_t contourIndex = 0; contourIndex < numContour; contourIndex++)
            {
            size_t thisCount = 0;
            if (!options.BezierStrokeCount (segments[contourIndex].GetPoleP (), segments[contourIndex].GetOrder (), thisCount))
                return false;
            if (thisCount > maxCount)
                maxCount = thisCount;
            }

        // Stroke each bezier (to exactly the right number of points !!!!)
        for (size_t contourIndex = 0; contourIndex < numContour; contourIndex++)
            {
            strokeGrid.AddBezierEvaluationsAndIndices (strokeGrid, contourIndex, segments[contourIndex], primitives[contourIndex], maxCount);
            }
        }
                         
    return true;
    }

// Compute tangents from point to point.
// return with points doubled for incoming, outgoing tangents.
// return false if less than 2 points in input.
static bool ExpandPolygonTangents
(
bvector<DPoint3d> &polygonPoint,
bvector<DPoint3d> &point,
bvector<DVec3d> &tangent
)
    {
    point.clear ();
    tangent.clear ();
    if (polygonPoint.size () < 2)
        return false;
    size_t numPoint = polygonPoint.size ();
    for (size_t i = 1; i < numPoint; i++)
        {
        DPoint3d pointA = polygonPoint[i-1];
        DPoint3d pointB = polygonPoint[i];
        if (pointA.IsEqual (pointB))
            continue;
        DVec3d edgeTangent = DVec3d::FromStartEndNormalize (pointA, pointB);
        point.push_back (pointA);
        tangent.push_back (edgeTangent);
        point.push_back (pointB);
        tangent.push_back (edgeTangent);
        }
    return true;
    }



/*--------------------------------------------------------------------------------**//**
* TFS#11626 Edges at tangeny are inconsistently visible for rotation versus linear sweep.
* Linear goes through compatible primitives.
* Rotational goes through arrays of point+tangent.
* Add option to make all double points visible.
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool IsVisibleJoint(DPoint3dCR pointA, DPoint3dCR pointB, DVec3dCR tangentA, DVec3dCR tangentB,
            ICurvePrimitiveCP curveA, ICurvePrimitiveCP curveB)
    {
    if (!pointA.AlmostEqual (pointB))
        return false;   // simple interior point within curve -- not visible
    if (curveA != curveB)
        return true;    // head-to-tail junction
    return !tangentA.IsParallelTo (tangentB);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool AppendBsplineStrokes(
IPolyfaceConstruction &builder,
ICurvePrimitivePtr curve,
bvector<DPoint3d> &xyz,
bvector<DVec3d> *tangent,
bvector<double> *fractions = nullptr,
bvector<ICurvePrimitiveP> *curves = nullptr
)
    {
    MSBsplineCurveCP bcurve = curve->GetProxyBsplineCurveCP ();
    if (NULL == bcurve)
        return false;
    size_t num0 = xyz.size ();
    bcurve->AddStrokes (builder.GetFacetOptionsR (), xyz, tangent, fractions);
    if (curves != nullptr)
        for (size_t i = num0; i < xyz.size (); i++)
            curves->push_back (curve.get ());
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IPolyfaceConstruction::SetCurrentFaceParamDistanceRange (DRange2dCR distanceRange)
    {
    FacetFaceData faceData = GetFaceData ();
    faceData.m_paramDistanceRange = distanceRange;
    SetFaceData (faceData);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IPolyfaceConstruction::SynchOptions ()
    {
    PolyfaceHeaderR polyface = GetClientMeshR ();
    IFacetOptionsR   options  = GetFacetOptionsR ();
    m_needParams = options.GetParamsRequired ();
    m_needNormals = options.GetNormalsRequired ();

    polyface.Param ().SetActive (m_needParams);
    polyface.ParamIndex ().SetActive (m_needParams);

    polyface.Normal ().SetActive (m_needNormals);
    polyface.NormalIndex ().SetActive (m_needNormals);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IPolyfaceConstruction::ToggleIndexOrderAndNormalReversal ()
    {
    m_state.m_reverseNewFacetIndexOrder = !m_state.m_reverseNewFacetIndexOrder;
    m_state.m_reverseNewNormals = !m_state.m_reverseNewNormals;
    }

//! Ask if facets are to be reversed as received.
//! @remarks This is affected by PushState/PopState operations.
bool IPolyfaceConstruction::GetReverseNewFacetIndexOrder () const
    {
    return m_state.m_reverseNewFacetIndexOrder;
    }

//! Ask if normals are to be reversed as received.
//! @remarks This is affected by PushState/PopState operations.
bool IPolyfaceConstruction::GetReverseNewNormals () const
    {
    return m_state.m_reverseNewNormals;
    }


//! Get the world to local placement transform.
//! @return false if the transform is an identity.
//! @remarks This is affected by PushState/PopState operations.
bool IPolyfaceConstruction::GetWorldToLocal (TransformR transform) const
    {
    transform = m_state.m_worldToLocal;
    return m_state.m_isTransformed;
    }

//! Ask if the local to world transform is nontrivial
//! @remarks This is affected by PushState/PopState operations.
bool IPolyfaceConstruction::IsTransformed () const
    {
    return m_state.m_isTransformed;
    }

//! Get the local to world placement transform.
//! @return false if the transform is an identity.
//! @remarks This is affected by PushState/PopState operations.
bool IPolyfaceConstruction::GetLocalToWorld (TransformR transform) const
    {
    transform = m_state.m_localToWorld;
    return m_state.m_isTransformed;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      09/2012
+--------------------------------------------------------------------------------------*/
bool IPolyfaceConstruction::GetLocalToWorldNormals (RotMatrixR matrix) const
    {
    matrix = m_state.m_localToWorldNormals;
    return m_state.m_isTransformed;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DPoint3d IPolyfaceConstruction::MultiplyByLocalToWorld (DPoint3dCR localPoint) const
    {
    if (!m_state.m_isTransformed)
        return localPoint;
    DPoint3d worldPoint;
    m_state.m_localToWorld.Multiply (worldPoint, localPoint);
    return worldPoint;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DVec3d IPolyfaceConstruction::MultiplyNormalByLocalToWorld (DVec3dCR localNormal) const
    {
    DVec3d worldNormal = localNormal;    
    if (m_state.m_isTransformed)
        m_state.m_localToWorldNormals.Multiply (worldNormal, localNormal);
        
    if (GetReverseNewNormals ())
        worldNormal.Negate ();
    worldNormal.Normalize ();

    return worldNormal;
    }





//! Set the local to world transform.
//! @return false if the given transform is not invertible.
//! @remarks This is affected by PushState/PopState operations.
bool IPolyfaceConstruction::SetLocalToWorld (TransformCR transform)
    {
    Transform inverse;
    if (inverse.InverseOf (transform))
        {
        m_state.m_localToWorld = transform;
        m_state.m_worldToLocal = inverse;
        RotMatrix matrixPart = RotMatrix::From (m_state.m_localToWorld);
        RotMatrix inverseMatrixPart = RotMatrix::From (inverse);
        m_state.m_localToWorldNormals.TransposeOf (inverseMatrixPart);
        double det = matrixPart.Determinant ();
        m_state.m_localToWorldScale = pow (fabs (det), 1.0 / 3.0) * (det >= 0.0 ? 1.0 : -1.0);
        m_state.m_worldToLocalScale = 1.0 / m_state.m_localToWorldScale;    // InverseOf guarantees that this is not divide by zero !!!
        m_state.m_isTransformed = !m_state.m_localToWorld.IsIdentity ();
        
        return true;
        }
    return false;
    }

//! Apply (right multiply) the local to world transform.
//! @return false if the given transform is not invertible.
bool IPolyfaceConstruction::ApplyLocalToWorld (TransformCR relativeTransform)
    {
    Transform transform;
    transform.InitProduct (m_state.m_localToWorld, relativeTransform);
    return SetLocalToWorld (transform);
    }

//! Push the current transform and revesal state.
void IPolyfaceConstruction::PushState (bool initializeCurrentState)
    {
    m_stateStack.push_back (m_state);
    if (initializeCurrentState)
        m_state = ConstructionState ();
    }

//! Pop the current transform and revesal state.
bool IPolyfaceConstruction::PopState ()
    {
    size_t n = m_stateStack.size ();
    if (n > 0)
        {
        m_state = m_stateStack[n-1];
        m_stateStack.pop_back ();
        return true;
        }
    return false;
    }

//! Set the current reversal state.
void IPolyfaceConstruction::SetReverseNewFacetIndexOrder (bool reverse)
    {
    m_state.m_reverseNewFacetIndexOrder = reverse;
    }

//! Set the current reversal state.
void IPolyfaceConstruction::SetReverseNewNormals (bool reverse)
    {
    m_state.m_reverseNewNormals = reverse;
    }


//! Get the (average) scale factor of the local to world transform.
//! @remarks This is affected by PushState/PopState operations.
double IPolyfaceConstruction::GetLocalToWorldScale () const
    {
    return m_state.m_localToWorldScale;
    }

//! Get the (average) scale factor of the world to local transform.
//! @remarks This is affected by PushState/PopState operations.
double IPolyfaceConstruction::GetWorldToLocalScale () const
    {
    return m_state.m_worldToLocalScale;
    }

IPolyfaceConstruction::ConstructionState::ConstructionState ()
    {
    m_reverseNewFacetIndexOrder = false;
    m_reverseNewNormals = false;
    m_isTransformed = false;
    m_worldToLocal = Transform::FromIdentity ();
    m_localToWorld = Transform::FromIdentity ();
    m_localToWorldScale = m_worldToLocalScale = 1.0;
    m_localToWorldNormals = RotMatrix::FromIdentity ();
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
IPolyfaceConstruction::IPolyfaceConstruction()
    : m_state ()
    {
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IPolyfaceConstruction::InitializeConstructionStateAndStack ()
    {
    m_state = ConstructionState ();
    m_stateStack.clear ();
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IPolyfaceConstruction::InitializeCurrentConstructionState ()
    {
    m_state = ConstructionState ();
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool IPolyfaceConstruction::NeedNormals () {return m_needNormals;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool IPolyfaceConstruction::NeedParams ()  {return m_needParams;}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IPolyfaceConstruction::FindOrAddPoints (bvector<DPoint3d> &point, size_t n, size_t numWrap, bvector<size_t> &index)
    {
    for (size_t i = 0; i < n; i++)
        index.push_back (FindOrAddPoint (point[i]));
    for (size_t i = 0; i < numWrap; i++)
        index.push_back (index[i]);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IPolyfaceConstruction::FindOrAddNormals (bvector<DVec3d> &normal, size_t n, size_t numWrap, bvector<size_t> &index)
    {
    for (size_t i = 0; i < n; i++)
        index.push_back (FindOrAddNormal (normal[i]));
    for (size_t i = 0; i < numWrap; i++)
        index.push_back (index[i]);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IPolyfaceConstruction::FindOrAddParams (bvector<DPoint2d> &param, size_t n, size_t numWrap, bvector<size_t> &index)
    {
    for (size_t i = 0; i < n; i++)
        index.push_back (FindOrAddParam (param[i]));
    for (size_t i = 0; i < numWrap; i++)
        index.push_back (index[i]);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IPolyfaceConstruction::AddPointIndexFan (size_t centerIndex, bvector<size_t> &index, size_t numChord, bool reverse)
    {
    if (!reverse)
        {
        for (size_t i = 0; i < numChord; i++)
            AddPointIndexTriangle (index[i], true, index[i+1], false, centerIndex, false);
        }
    else
        {
        for (size_t i = numChord; i > 0; i--)
            AddPointIndexTriangle (index[i], true, index[i-1], false, centerIndex, false);
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IPolyfaceConstruction::AddPointIndexStrip
(
bool visibleLeft,
bvector<size_t> &indexA,
bool visibleA,
bvector<size_t> &indexB,
bool visibleB,
bool visibleRight,
size_t numQuad,
bool reverse
)
    {
    if (reverse)
        {
        AddPointIndexStrip (visibleLeft, indexB, visibleB, indexA, visibleA, visibleRight, numQuad, false);
        return;
        }
    size_t nA = indexA.size ();
    size_t nB = indexB.size ();
    size_t n = std::min (nA, nB);
    if (n == 0)
        return;
    if (numQuad >= n)
        numQuad = n - 1;
    for (size_t i = 0; i < numQuad; i++)
        {
        AddPointIndexQuad (
                indexA[i], visibleA,
                indexA[i+1], i + 2 == n ? visibleRight : false,
                indexB[i+1], visibleB,
                indexB[i], i == 0 ? visibleLeft : false);
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IPolyfaceConstruction::AddNormalIndexFan (size_t centerIndex, bvector<size_t> &index, size_t numChord, bool reverse)
    {
    if (!reverse)
        {
        for (size_t i = 0; i < numChord; i++)
            AddNormalIndexTriangle (index[i], index[i+1], centerIndex);
        }
    else
        {
        for (size_t i = numChord; i > 0; i--)
            AddNormalIndexTriangle (index[i], index[i-1], centerIndex);
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IPolyfaceConstruction::AddNormalIndexPlanarFan (DVec3dCR vectorA, DVec3dCR vectorB, bool reverseCrossProduct, size_t numChord)
    {
    DVec3d normal;
    normal.NormalizedCrossProduct (vectorA, vectorB);
    if (reverseCrossProduct)
        normal.Negate ();
    size_t normalIndex = FindOrAddNormal (normal);
    AddNormalIndexPlanarFan (normalIndex, numChord);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IPolyfaceConstruction::AddNormalIndexPlanarFan (size_t normalIndex, size_t numChord)
    {
    for (size_t i = 0; i < numChord; i++)
        AddNormalIndexTriangle (normalIndex, normalIndex, normalIndex);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IPolyfaceConstruction::AddNormalIndexPlanarStrip (size_t normalIndex, size_t numQuad)
    {
    for (size_t i = 0; i < numQuad; i++)
        AddNormalIndexQuad (normalIndex, normalIndex, normalIndex, normalIndex);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IPolyfaceConstruction::AddParamIndexStrip (bvector<size_t> &indexA, bvector<size_t> &indexB, size_t numQuad, bool reverse)
    {
    if (reverse)
        {
        AddParamIndexStrip (indexB, indexA, numQuad, false);
        return;
        }
        
    size_t nA = indexA.size ();
    size_t nB = indexB.size ();
    size_t n = std::min (nA, nB);
    if (n == 0)
        return;
    if (numQuad >= n)
        numQuad = n - 1;

    for (size_t i = 0; i < numQuad; i++)
        AddParamIndexQuad (indexA[i], indexA[i+1], indexB[i+1], indexB[i]);

    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IPolyfaceConstruction::AddParamIndexFan (size_t centerIndex, bvector<size_t> &index, size_t numChord, bool reverse)
    {
    if (!reverse)
        {
        for (size_t i = 0; i < numChord; i++)
            AddParamIndexTriangle (index[i], index[i+1], centerIndex);
        }
    else
        {
        for (size_t i = numChord; i > 0; i--)
            AddParamIndexTriangle (index[i], index[i-1], centerIndex);
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool TryTransformFromPseudoDistanceRectangle(
FacetParamMode mode,
DRange2dCR baseRange,
double xDistanceFactor,
double yDistanceFactor,
DRange2dR distanceRange,
DRange2dR targetRange,
TransformR transform
)
    {
    double dx = (baseRange.high.x - baseRange.low.x) * xDistanceFactor;
    double dy = (baseRange.high.y - baseRange.low.y) * yDistanceFactor;
    distanceRange = DRange2d::From (0,0, dx, dy);
    bool stat = false;
    if (mode == FACET_PARAM_01BothAxes)
        {
        targetRange = DRange2d::From (0,0,1,1);
        stat = true;
        }
    else if (mode == FACET_PARAM_01LargerAxis)
        {
        double ax = 1.0;
        double ay = 1.0;
        if (dy > dx)
            ax = dx / dy;
        else if (dx > dy)
            ay = dy / dx;
        targetRange = DRange2d::From (0,0, ax, ay);
        stat = true;
        }
    else if (mode == FACET_PARAM_Distance)
        {
        targetRange = distanceRange;
        stat = true;
        }
    else
        {
        targetRange = baseRange; /// should never happen.
        stat = false;
        }
    transform.InitIdentity ();
    return stat && Transform::TryRangeMapping (baseRange, targetRange, transform);
    }

//! @param [in,out] params parameters to remap.
//! @param [in] xDistanceFactor scale factor to turn input x coordinates to distance (if distance requested either as final result or for larger axis scaling)
//! @param [in] yDistanceFactor scale factor to turn input y coordinates to distance (if distance requested either as final result or for larger axis scaling)
bool IPolyfaceConstruction::RemapPseudoDistanceParams
(
bvector<DPoint2d>&params,
DRange2dR distanceRange,
DRange2dR paramRange,
double xDistanceFactor,
double yDistanceFactor,
Transform *transformOut
)
    {
    size_t n = params.size ();
    bool stat = false;
    if (nullptr != transformOut)
        *transformOut = Transform::FromIdentity ();
    if (n == 0)
        {
        stat = true;
        }
    else if (n == 1)
        {
        stat = true;
        params[0].Zero ();
        }
    else
        {
        DRange2d baseRange = DRange2d::From (params);
        Transform transform;
        stat = TryTransformFromPseudoDistanceRectangle (GetFacetOptionsR().GetParamMode (),
                        baseRange,
                        xDistanceFactor, yDistanceFactor,
                        distanceRange, paramRange, transform);
        if (nullptr != transformOut)
            *transformOut = transform;
        if (stat)
            DPoint2dOps::Multiply (&params, transform);
        }
    return stat;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IPolyfaceConstruction::AddFullDisk (DEllipse3dCR ellipse, size_t numPerQuadrant)
    {
    SynchOptions ();
    bvector<DPoint3d> pointB;
    bvector<DPoint2d> trigPointB;
    DRange2d distanceRange, paramRange;
    DEllipse3d fullEllipse = ellipse;
    fullEllipse.MakeFullSweep ();
    size_t numChord;
    double magnitude0 = ellipse.vector0.Magnitude ();
    double magnitude90 = ellipse.vector90.Magnitude ();
    
    bool reversed = false;
    size_t numRadius = GetFacetOptionsR ().DistanceStrokeCount (DoubleOps::Max (magnitude0, magnitude90));

    double radiusFraction = 1.0 / (double)numRadius;                
    if (numPerQuadrant > 0)
        numChord = 4 * numPerQuadrant;
    else 
        numChord = GetFacetOptionsR ().FullEllipseStrokeCount (fullEllipse);
        
    AppendFullEllipseStrokes (ellipse, numChord, &pointB, NULL, &trigPointB, radiusFraction);

    //double df = 1.0 / numChord;
    bvector<size_t>pointIndexB;
    FindOrAddPoints (pointB, numChord, 1, pointIndexB);
    size_t centerPointIndex = FindOrAddPoint (ellipse.center);

    
    AddPointIndexFan (centerPointIndex, pointIndexB, numChord, reversed);

    DVec3d normal;
    normal.NormalizedCrossProduct (ellipse.vector0, ellipse.vector90);
    if (reversed)
        normal.Negate ();
    size_t normalIndex = FindOrAddNormal (normal);
    
    if (NeedNormals ())
        AddNormalIndexPlanarFan (normalIndex, numChord);

    DPoint2d centerParam;
    centerParam.Zero ();
    Transform uvTransform = Transform::FromIdentity ();
    bvector<size_t> paramIndexB;
    
    if (NeedParams ())
        {
        centerParam.Zero ();
        trigPointB.push_back (centerParam);
        RemapPseudoDistanceParams (trigPointB, distanceRange, paramRange, magnitude0, magnitude90, &uvTransform);
        SetCurrentFaceParamDistanceRange (distanceRange);
        FindOrAddParams (trigPointB, trigPointB.size (), 0, paramIndexB);
        // Force wraparound, but keep the center point at back.
        size_t centerParamIndex = paramIndexB.back ();
        paramIndexB.back () = paramIndexB.front ();
        paramIndexB.push_back (centerParamIndex);
        AddParamIndexFan (centerParamIndex, paramIndexB, numChord, reversed);
        }
        
    if (numRadius > 1)
        {
        bvector<DPoint3d> pointA;
        bvector<DPoint2d> trigPointA;
        bvector<size_t>   pointIndexA;
        bvector<size_t>   paramIndexA;
        for (size_t i = 2; i <= numRadius; i++)
            {
            pointA.swap (pointB);
            trigPointA.swap (trigPointB);
            pointIndexA.swap (pointIndexB);
            paramIndexA.swap (paramIndexB);
            pointB.clear ();
            trigPointB.clear ();
            paramIndexB.clear ();
            pointIndexB.clear ();        
            AppendFullEllipseStrokes (ellipse, numChord, &pointB, NULL, &trigPointB, i * radiusFraction);
            FindOrAddPoints (pointB, numChord, 1, pointIndexB);
            AddPointIndexStrip (false, pointIndexB, false, pointIndexA, i == numRadius, false, numChord, reversed);

            if (NeedNormals ())
                AddNormalIndexPlanarStrip (normalIndex, numChord);

            if (NeedParams ())
                {
                FindOrAddParams (trigPointB, trigPointB.size (), 0, paramIndexB);
                paramIndexB.back () = paramIndexB.front ();
                AddParamIndexStrip (paramIndexB, paramIndexA, numChord, reversed);    // how many really?
                }
            }
        }        
    EndFace_internal ();
    }


// Assign chord count at an interior circle.
// force chord count to be a multiple of 4.
size_t ProportionalChordCount (size_t numOuter, size_t radialStep, size_t numRadius)
    {
    if (radialStep == numRadius)
        return numOuter;
    size_t n = radialStep * (numOuter + numRadius - 1) / numRadius;
    size_t numPerQuadrant = (n + 3) / 4;
    if (numPerQuadrant < 1)
        numPerQuadrant = 1;
    return 4 * numPerQuadrant;
    }
#define MAX_RADIAL_LAYER 40
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IPolyfaceConstruction::AddFullDiskTriangles (DEllipse3dCR ellipse, size_t numPerQuadrant)
    {
    SynchOptions ();
    bvector<DPoint3d> pointB, trigPointBXYZ;
    bvector<DPoint2d> trigPointB;
    DRange2d distanceRange, paramRange;
    DEllipse3d fullEllipse = ellipse;
    fullEllipse.MakeFullSweep ();
    size_t numChord;
    double magnitude0 = ellipse.vector0.Magnitude ();
    double magnitude90 = ellipse.vector90.Magnitude ();
    
    // Precompute uv transform and transformed center parameter ...
    trigPointB.push_back (DPoint2d::From (-1.0, -1.0));
    trigPointB.push_back (DPoint2d::From ( 1.0,  1.0));
    Transform uvTransform = Transform::FromIdentity ();

    RemapPseudoDistanceParams (trigPointB, distanceRange, paramRange, magnitude0, magnitude90, &uvTransform);
    trigPointB.clear ();
    DPoint2d centerParam;
    trigPointB.push_back (DPoint2d::FromZero ());
    DPoint2dOps::Multiply (&trigPointB, uvTransform);
    centerParam = trigPointB.front ();
    trigPointB.clear ();


    bool reversed = false;
    size_t numRadius = GetFacetOptionsR ().DistanceStrokeCount (DoubleOps::Max (magnitude0, magnitude90));
    if (numRadius > MAX_RADIAL_LAYER)
        numRadius = MAX_RADIAL_LAYER;
    double radiusFraction = 1.0 / (double)numRadius;                
    if (numPerQuadrant > 0)
        numChord = 4 * numPerQuadrant;
    else 
        numChord = GetFacetOptionsR ().FullEllipseStrokeCount (fullEllipse);


    size_t numChordB = ProportionalChordCount (numChord, 1, numRadius);

    AppendFullEllipseStrokes (ellipse, numChordB, &pointB, NULL, &trigPointB, radiusFraction);
    DPoint3dOps::AppendXY0 (trigPointBXYZ, trigPointB);

    uvTransform.Multiply (trigPointB, trigPointB);
    
    bvector<size_t>pointIndexB;
    FindOrAddPoints (pointB, numChordB, 1, pointIndexB);
    size_t centerPointIndex = FindOrAddPoint (ellipse.center);

    AddPointIndexFan (centerPointIndex, pointIndexB, numChordB, reversed);

    DVec3d normal;
    normal.NormalizedCrossProduct (ellipse.vector0, ellipse.vector90);
    if (reversed)
        normal.Negate ();

    bool needNormals = NeedNormals ();
    size_t normalIndex = needNormals ? FindOrAddNormal (normal) : 0;
    
    if (needNormals)
        AddNormalIndexPlanarFan (normalIndex, numChordB);

    bvector<size_t> paramIndexB;
    
    bool needParams = NeedParams ();
    if (needParams)
        {
        SetCurrentFaceParamDistanceRange (distanceRange);
        FindOrAddParams (trigPointB, trigPointB.size (), 0, paramIndexB);
        // Force wraparound . . .
        size_t centerParamIndex = FindOrAddParam (centerParam);
        paramIndexB.push_back (paramIndexB.front ());
        AddParamIndexFan (centerParamIndex, paramIndexB, numChordB, reversed);
        }
        
    if (numRadius > 1)
        {
        bvector<DPoint3d> trigPointAXYZ;
        bvector<DTriangle3d> trigTriangle;
        for (size_t i = 2; i <= numRadius; i++)
            {
            trigPointAXYZ.swap (trigPointBXYZ);
            size_t numChordB = ProportionalChordCount (numChord, i, numRadius);
            trigPointB.clear ();

            AppendFullEllipseStrokes (ellipse, numChordB, nullptr, nullptr, &trigPointB, i * radiusFraction);
            trigPointBXYZ.clear ();
            DPoint3dOps::AppendXY0 (trigPointBXYZ, trigPointB);

            PolylineOps::GreedyTriangulationBetweenLinestrings (trigPointBXYZ, trigPointAXYZ, trigTriangle);
            for (DTriangle3d const &triangle : trigTriangle)
                {
                // Create a single-triangle fan . . .
                pointIndexB.clear ();
                paramIndexB.clear ();
                DPoint2d uv;
                DPoint3d xyz;
                for (size_t k = 0; k < 3; k++)
                    {
                    uv.Init (triangle.point[k].x, triangle.point[k].y);
                    ellipse.EvaluateTrigPairs (&xyz, &uv, 1);
                    pointIndexB.push_back (FindOrAddPoint (xyz));
                    if (needParams)
                        {
                        uvTransform.Multiply (uv, uv);
                        paramIndexB.push_back (FindOrAddParam (uv));
                        }
                    }
                AddPointIndexTriangle (
                    pointIndexB[0],  true,
                    pointIndexB[1], true,
                    pointIndexB[2], true
                    );
                if (needParams)
                    AddParamIndexTriangle (paramIndexB[0], paramIndexB[1], paramIndexB[2]);
                if (needNormals)
                        AddNormalIndexPlanarFan (normalIndex, 1);
                }
            }
        }        
    EndFace_internal ();
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool IPolyfaceConstruction::AddSweptNGon (size_t n, double rOuter, double z0, double z1,
    bool bottomCap,
    bool topCap
    )
    {
    bool needParams = NeedParams ();
    if (n < 3)
        return false;
    double dtheta = msGeomConst_2pi / (double)n;
    bvector<size_t> index0;
    bvector<size_t> index1;
    bvector<DPoint2d>baseUV;
    bvector<DPoint2d>topUV;
    bvector<DPoint2d>sideUV;
    double edgeLength = 2 * rOuter * sin (0.5 * dtheta);
    double dz = z1 - z0;
    for (size_t i = 0; i < n; i++)
        {
        double theta = i * dtheta;
        double c = cos (theta);
        double s = sin (theta);
        
        double rc = rOuter * c;
        double rs = rOuter * s;

        DPoint3d xyz0 = DPoint3d::FromXYZ (rc, rs, z0);
        DPoint3d xyz1 = DPoint3d::FromXYZ (rc, rs, z1);
        baseUV.push_back (DPoint2d::From (rc, -rs));
        topUV.push_back (DPoint2d::From (rc, rs));
        sideUV.push_back (DPoint2d::From (i * edgeLength, 0.0));
        sideUV.push_back (DPoint2d::From (i * edgeLength, dz));
        index0.push_back (FindOrAddPoint (xyz0));
        index1.push_back (FindOrAddPoint (xyz1));
        }
    // closure point in uv space
    sideUV.push_back (DPoint2d::From (n * edgeLength, 0.0));
    sideUV.push_back (DPoint2d::From (n * edgeLength, 1.0));
    DRange2d sideDistanceRange, sideParamRange, topDistanceRange, topParamRange, bottomDistanceRange, bottomParamRange;
    if (needParams)
        {
        RemapPseudoDistanceParams (sideUV, sideDistanceRange, sideParamRange);
        RemapPseudoDistanceParams (topUV, topDistanceRange, topParamRange);
        RemapPseudoDistanceParams (baseUV, bottomDistanceRange, bottomParamRange);
        }
    bvector<int> &pointIndex = GetClientMeshR ().PointIndex ();

    for (size_t i = 0; i < n; i++)
        {
        size_t iA = i;
        size_t iB = i + 1;
        if (iB >= (size_t)n)
            iB -= n;
        AddPointIndexQuad (index0[iA], true, index0[iB], true,
                            index1[iB], true, index1[iA], true);
        }
    SetCurrentFaceParamDistanceRange (sideDistanceRange);
    EndFace_internal ();
    // NEEDS WORK -- capture large polygon for triangulation.
    // NEEDS WORK -- params and normals.
    if (bottomCap)
        {
        // bottom face -- indices appear CCW from below.
        for (size_t i = 0; i < n; i++)
            pointIndex.push_back ((int)(index0[n - i - 1] + 1));
        pointIndex.push_back (0);
        SetCurrentFaceParamDistanceRange (bottomDistanceRange);
        EndFace_internal ();
        }

    if (topCap)
        {
        // top face -- indices appear CCW from above.
        for (size_t i = 0; i < n; i++)
            pointIndex.push_back ((int)(index1[i] + 1));
        pointIndex.push_back (0);
        SetCurrentFaceParamDistanceRange (topDistanceRange);
        EndFace_internal ();
        }

#ifdef TriangulateEndCap

    // NEEDS WORK -- params and normals.
    if (bottomCap)
        {
        // bottom face -- indices appear CCW from below.
        for (size_t i1 = n - 1; i1 > 1; i1--)
            {
            AddPointIndexTriangle (
                index0[0],  i1 == n - 1,
                index0[i1], true,
                index0[i1-1], i1 == 2
                );
            }
        SetCurrentFaceParamDistanceRange (bottomDistanceRange);
        EndFace_internal ();
        }

    if (topCap)
        {
        // top face -- indices appear CCW from above.
        for (size_t i0 = 1; i0 + 1 < n; i0++)
            {
            AddPointIndexTriangle (
                index1[0],  i0 == 1,
                index1[i0], true,
                index1[i0+1], i0 == n - 2
                );
            }
        SetCurrentFaceParamDistanceRange (topDistanceRange);
        EndFace_internal ();
        }
#endif
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IPolyfaceConstruction::AddFullSphere
(
DPoint3dCR center,
double radius,
size_t numPerQuadrantNS,
size_t numPerQuadrantEW
)
    {
    AddEllipsoidPatch (center, radius, radius, radius, 2 * numPerQuadrantNS, 4 * numPerQuadrantEW,
            0.0, msGeomConst_2pi,
            -msGeomConst_piOver2,
            msGeomConst_pi, false);
    }

static bool isVisible (size_t i, size_t n, size_t visibleInterval)
    {
    if (visibleInterval <= 1)
        return true;
    if (visibleInterval > n)
        return false;
    if (i == 0)
        return true;
    if (i + 1 > n)
        return true;
    return i % visibleInterval == 0;
    }
static void CreateTrianglesInCircle (
IPolyfaceConstruction &builder,
bool reverse,
DPoint3dCR center,
bvector<size_t> &pointIndex,
DVec3dCR normal,
double parameterRadius,
bvector<DPoint3d> theta, // (cos,sin,theta)
bvector<size_t> index   // work vector.
)
    {
    size_t numTheta = pointIndex.size ();
    size_t centerPointIndex = builder.FindOrAddPoint (center);
    for (size_t i = 0; i + 1 < numTheta; i++)
        {
        if (reverse)
            builder.AddPointIndexTriangle (centerPointIndex, false, pointIndex[i+1], true, pointIndex[i], false);
        else
            builder.AddPointIndexTriangle (centerPointIndex, false, pointIndex[i], true, pointIndex[i+1], false);
        }
    if (builder.NeedParams ())
        {
        index.clear ();
        size_t centerIndex = builder.FindOrAddParam (DPoint2d::From(0,0));
        for (size_t i = 0; i < numTheta; i++)
            {
            DPoint2d xy = DPoint2d::From (parameterRadius * theta[i].x, parameterRadius * theta[i].y);
            index.push_back (builder.FindOrAddParam (xy));
            }
        index.push_back (index[0]); // maybe theta already has dups?
        for (size_t i = 0; i + 1 < numTheta; i++)
            {
            if (reverse)
                builder.AddParamIndexTriangle (centerIndex, index[i+1], index[i]);
            else
                builder.AddParamIndexTriangle (centerIndex, index[i], index[i+1]);
            }
        }
    if (builder.NeedNormals ())
        {
        index.clear ();
        size_t normalIndex = builder.FindOrAddNormal (normal);
        for (size_t i = 0; i + 1 < numTheta; i++)
            {
            builder.AddNormalIndexTriangle (normalIndex, normalIndex, normalIndex);// no need to reverse normal indices
            }
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IPolyfaceConstruction::AddEllipsoidPatch
(
DPoint3dCR center,
double radiusX,
double radiusY,
double radiusPole,
size_t numEastWestEdge,
size_t numNorthSouthEdge,
double theta0,
double thetaSweep,
double phi0,
double phiSweep,
bool capped,
int  orientationSelect
)
    {
    static double s_poleTolerance = 1.0e-8; // big tolerance to compress poles to zero.
    //static size_t s_minPerQuadrant = 1;
    static size_t s_defaultThetaVisibleCount = 1000;  /// hmph -- everything hidden?
    static size_t s_defaultPhiVisibleCount   = 1000;  /// hmph -- everything hidden?
    double largerEquatorRadius = DoubleOps::MaxAbs (radiusX, radiusY);
    DEllipse3d equator = DEllipse3d::From (0,0,0, radiusX, 0,0, 0,radiusY,0, theta0, thetaSweep);
    DEllipse3d largestMeridian = DEllipse3d::From (0,0,0,
                largerEquatorRadius, 0, 0,
                0,radiusPole,0,
                phi0, phiSweep
                );
    if (!Angle::IsFullCircle (thetaSweep))
        capped = false;
    SynchOptions ();

    DEllipse3d scaledMeridian = largestMeridian;
    DEllipse3d scaledEquator  = equator;
    double scale = GetLocalToWorldScale ();
    scaledMeridian.vector0.Scale (scale);
    scaledMeridian.vector90.Scale (scale);
    scaledEquator.vector0.Scale (scale);
    scaledEquator.vector90.Scale (scale);
    if (numNorthSouthEdge == 0)
        numNorthSouthEdge = GetFacetOptionsR ().EllipseStrokeCount (scaledMeridian);
    if (numEastWestEdge == 0)
        numEastWestEdge = GetFacetOptionsR ().EllipseStrokeCount (scaledEquator);

    if (numNorthSouthEdge < 2)
        numNorthSouthEdge = 2;
    if (numEastWestEdge < 4)
        numEastWestEdge = 4;

    double dTheta = thetaSweep / (double)(numEastWestEdge);
    double dPhi   = phiSweep  / (double)(numNorthSouthEdge);

    // Store <x,y,z>=<cos(q),sin(q),q>
    bvector<DPoint3d> theta;
    bvector<DPoint3d> phi;

        
        
        
    // Evaluate angle functions in first quadrant.  Use rotations to copy to others.
    for (size_t i = 0; i <= numEastWestEdge; i++)
        AppendTrigPoint (theta, theta0 + dTheta * (double)i);

    for (size_t j = 0; j <= numNorthSouthEdge; j++)
        AppendTrigPoint (phi, phi0 + dPhi* (double)j);

    size_t numPhi = phi.size ();
    size_t numTheta = theta.size ();

    size_t thetaVisibleCount = s_defaultThetaVisibleCount;
    size_t phiVisibleCount   = s_defaultPhiVisibleCount;



    bvector <size_t> pointIndex;
    bvector <size_t> cap0PointIndex;
    bvector <size_t> cap1PointIndex;
    // Add points ... Let the builder map take care of duplicates ...
    for (size_t j = 0; j < numPhi; j++)
        {
        for (size_t i = 0; i < numTheta; i++)
            {
            DPoint3d xyz;
            xyz.x = center.x + radiusX * theta[i].x * phi[j].x;
            xyz.y = center.y + radiusY * theta[i].y * phi[j].x;
            xyz.z = center.z + radiusPole * phi[j].y;
            size_t index = FindOrAddPoint (xyz);
            pointIndex.push_back (index);
            if (j == 0)
                cap0PointIndex.push_back (index);
            else if (j == numPhi - 1)
                cap1PointIndex.push_back (index);
            }
        }

    for (size_t j = 0; j < numPhi - 1; j++)
        {
        size_t i0 = j * numTheta;
        size_t i1 = i0 + numTheta;
        for (size_t i = 0; i + 1 < numTheta; i++)
            {
            AddPointIndexQuad (
                    pointIndex[i0+i], isVisible (j, numPhi, phiVisibleCount),
                    pointIndex[i0+i+1], isVisible (i+1, numTheta, thetaVisibleCount),
                    pointIndex[i1+i+1], isVisible (j+1, numPhi, phiVisibleCount),
                    pointIndex[i1+i], isVisible (i, numTheta, thetaVisibleCount));
            }
        }

    if (NeedParams ())
        {
        // PARAMS
        pointIndex.clear ();
        // NEEDS WORK -- distance scale
        for (size_t j = 0; j < numPhi; j++)
            {
            for (size_t i = 0; i < numTheta; i++)
                {
                DPoint2d xy;
                if (orientationSelect == 0)
                    {
                    xy.x = theta[i].z;
                    xy.y = phi[j].z;
                    }
                else
                    {
                    xy.x = -phi[j].z;
                    xy.y = theta[i].z;
                    }
                pointIndex.push_back (FindOrAddParam (xy));
                }
            }

        for (size_t j = 0; j < numPhi - 1; j++)
            {
            size_t i0 = j * numTheta;
            size_t i1 = i0 + numTheta;
            for (size_t i = 0; i + 1 < numTheta; i++)
                {
                AddParamIndexQuad (pointIndex[i0+i], pointIndex[i0+i+1], pointIndex[i1+i+1], pointIndex[i1+i]);
                }
            }
        }

    if (NeedNormals ())
        {
        // NORMALS
        pointIndex.clear ();
        for (size_t j = 0; j < numPhi; j++)
            {
            for (size_t i = 0; i < numTheta; i++)
                {
                DVec3d normal;
                normal.x = theta[i].x * phi[j].x;
                normal.y = theta[i].y * phi[j].x;
                normal.z = phi[j].y;
                pointIndex.push_back (FindOrAddNormal (normal));
                }
            }

        for (size_t j = 0; j < numPhi - 1; j++)
            {
            size_t i0 = j * numTheta;
            size_t i1 = i0 + numTheta;
            for (size_t i = 0; i + 1 < numTheta; i++)
                {
                AddNormalIndexQuad (pointIndex[i0+i], pointIndex[i0+i+1], pointIndex[i1+i+1], pointIndex[i1+i]);
                }
            }
        }
    EndFace_internal ();

    DPoint3d trigPoint;
    trigPoint = phi[0];
    // parameterize as (r cos, r sin)
    if (fabs (trigPoint.x) > s_poleTolerance)
        {
        if (capped)
            {
            CreateTrianglesInCircle (*this, true,  // base is from below, needs reverse
                DPoint3d::From (0,0, radiusPole * trigPoint.y),
                cap0PointIndex,
                DVec3d::From (0,0,-1),
                trigPoint.x,
                theta,
                pointIndex
                );
            EndFace_internal ();
            }
        AddEdgeChainZeroBased (CurveTopologyId::Type::SweepProfile, 0, cap0PointIndex);
        }
        
    trigPoint = phi[numNorthSouthEdge];
    if (fabs (trigPoint.x) > s_poleTolerance)
        {
        if (capped)
            {
            CreateTrianglesInCircle (*this, false,  // top is from above, no reverse
                DPoint3d::From (0,0, radiusPole * trigPoint.y),
                cap1PointIndex,
                DVec3d::From (0,0,1),
                trigPoint.x,
                theta,
                pointIndex
                );
            EndFace_internal ();
            }
        AddEdgeChainZeroBased (CurveTopologyId::Type::SweepProfile, 1, cap1PointIndex);
        }
    }


static bool AllPhysicallyClosed (bvector<CurveVectorPtr> const &curves)
    {
    for (size_t i = 0; i < curves.size (); i++)
        if (!curves[i]->AreStartEndAlmostEqual ())
            return false;
    return true;
    }
// Return common boundary type (if any).  Return BOUNDARY_TYPE_None if mixed or mismatched sizes.
static CurveVector::BoundaryType CommonBoundaryType (bvector<CurveVectorPtr> const &curves)
    {
    if (curves.size () == 0)                    
        return CurveVector::BOUNDARY_TYPE_None;
    size_t firstSize = curves[0]->size ();
    size_t numOpen = 0;
    size_t numClosed = 0;
    size_t numParity = 0;
    size_t numUnion = 0;

    for (size_t i = 0; i < curves.size (); i++)
        {
        if (curves[i]->size () != firstSize)
            return CurveVector::BOUNDARY_TYPE_None;
        CurveVector::BoundaryType b = curves[i]->GetBoundaryType();
        if (b == CurveVector::BOUNDARY_TYPE_Open)
            numOpen++;
        else if (b == CurveVector::BOUNDARY_TYPE_Outer || b == CurveVector::BOUNDARY_TYPE_Inner)
            numClosed++;
        else if (b == CurveVector::BOUNDARY_TYPE_ParityRegion)
            numParity++;
        else if (b == CurveVector::BOUNDARY_TYPE_UnionRegion)
            numUnion++;
        }

    size_t numBoundary = curves.size ();
    if (numParity == numBoundary)
        return CurveVector::BOUNDARY_TYPE_ParityRegion;
    if (numUnion == numBoundary)
        return CurveVector::BOUNDARY_TYPE_UnionRegion;
    if (numClosed == numBoundary)
        return CurveVector::BOUNDARY_TYPE_Outer;
    return CurveVector::BOUNDARY_TYPE_Open;
    }



/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
struct CorrespondingCurveState
{
DVec3d m_primaryNormal;
size_t m_primaryNormalIndex;
bvector<DPoint3d>m_basePolygon;
bvector<DPoint3d>m_topPolygon;
IPolyfaceConstruction &m_builder;
bool m_capped;

CorrespondingCurveState (IPolyfaceConstruction &builder, bool capped)
    : m_builder (builder), m_capped(capped)
    {
    m_primaryNormal.Zero ();
    m_primaryNormalIndex = SIZE_MAX;
    }
//------------------------------------------------------------------------------
// Internal ruled surface constructin for a single curve vector of type OPEN, INNER, or OUTER.
// Caller confirms matching types and passes appropriate flags and cap collectors
//------------------------------------------------------------------------------
bool AddRuledBetweenCorrespondingChains
(
bvector<CurveVectorPtr> const &contours,
bool reverse
)
    {

    bvector<DPoint3d> startPoints;      // tail point of prior primitives, to be enforced on succesor (cyclicly also)
    bvector<DVec3d> startTangents;
    if (contours.size () < 2)
        return false;

//    bool closed = contours[0]->IsClosedPath ();
    // ASSUMPTION:  Looking at the final point tangent is enough to get a twist angle.
    for (size_t i = 0; i < contours.size (); i++)
        {
        if (contours[i]->empty())        // Degenerate chain - This happens on XGraphics from Petrobras that has a boundary with point strings....
            return false;       

        DPoint3d xyz;
        DVec3d tangent;
        contours[i]->back ()->FractionToPoint (1.0, xyz, tangent);
        startPoints.push_back (xyz);
        startTangents.push_back (tangent);
        }

    double maxTwist = 0.0;
    for (size_t i = 1; i < startTangents.size (); i++)
      maxTwist = DoubleOps::Max (maxTwist, startTangents[i-1].AngleTo (startTangents[i]));

    IFacetOptionsR options = m_builder.GetFacetOptionsR ();
    //size_t numContour = contours.size ();
    int errors = 0;
    if (reverse)
        m_builder.ToggleIndexOrderAndNormalReversal ();

    size_t numPrimitive = contours[0]->size ();
    StrokeGrid strokeGrid (m_builder, contours.size ());
    size_t minEdges = m_builder.GetFacetOptionsR ().DistanceAndTurnStrokeCount (0.0, maxTwist);
    for (size_t primitiveIndex = 0; primitiveIndex < numPrimitive; primitiveIndex++)
        {
        // line & ellipse sequences set up based on the maximum count ....
        size_t edgeCount;
        if (AreCompatiblePrimitives (options, contours, primitiveIndex, edgeCount))
            {
            if (edgeCount < minEdges)
                edgeCount = minEdges;
            AddRuledBetweenCorrespondingPrimitives (strokeGrid, contours, primitiveIndex, edgeCount);
            }
        else if (AreCompatibleBsplineCurves (contours, primitiveIndex))
            {
            AddRuledBetweenCorrespondingBsplineCurves (strokeGrid, minEdges, contours, primitiveIndex);
            }
        else if (AreCompatibleLineStrings (contours, primitiveIndex, edgeCount))
            {
            AddRuledBetweenCorrespondingLineStrings (strokeGrid, minEdges, contours, primitiveIndex);
            }
        else
            errors++;
        }
    //strokeGrid.ApplyStartEnd (startPoints, enforceStart);     // TODO: enforce closure ?????
    strokeGrid.AnnounceGridToBuilder (true);
    strokeGrid.GetStrokesFromContour (0, &m_basePolygon);
    strokeGrid.GetStrokesFromContour (contours.size () - 1, &m_topPolygon);

    if (reverse)
       m_builder.ToggleIndexOrderAndNormalReversal ();
    if (m_capped)
        {
        DPoint3dOps::AppendDisconnect (&m_basePolygon);
        DPoint3dOps::AppendDisconnect (&m_topPolygon);
        }
    m_builder.EndFace_internal ();
    return errors == 0;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void EmitCaps(bool reversed)
    {
    if (!m_capped)
        return;
    // We define:
    //   standard (nonreversed) configuration has
    //     basePolygon on xy plane appears counterclockwise and is swept in positive z direction.
    //     hence the basePolyon has to be output in reversed order and top as given.
    //     sides are positive order.
    if (!reversed)
        {
        m_builder.ToggleIndexOrderAndNormalReversal ();
        m_builder.AddTriangulation (m_basePolygon);
        m_builder.EndFace_internal ();
        m_builder.ToggleIndexOrderAndNormalReversal ();
        m_builder.AddTriangulation (m_topPolygon);
        m_builder.EndFace_internal ();
        }
    else
        {
        m_builder.AddTriangulation (m_basePolygon);
        m_builder.ToggleIndexOrderAndNormalReversal ();
        m_builder.EndFace_internal ();
        m_builder.AddTriangulation (m_topPolygon);
        m_builder.ToggleIndexOrderAndNormalReversal ();
        m_builder.EndFace_internal ();
        }
    }

// Test the SINGLE CLOSED LOOP in the base contour for its orientation relative to the target contour
// In a normal (non reversed) loop
//    1) The base loop appears counterclockwise on its plane when viewed so the sweep is in positive z direction
//    2) base loop gets reversed on output so it becomes counter clockwise when viewed below.
//    3) top loop already appears counterclockwise and is NOT reversed.
//    4) quads on side are not reversed.
// In a reversed loop
//    1) The base loop appears clockwise on its plane when viewed so the sweep is in positive z direction
//    2) base loop is not reversed on output.
//    3) top loop IS reversed.
//    4) quads on side are reversed.


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool NeedReverse(CurveVectorCR contourA, CurveVectorCR contourB)
    {
    bool reverse = false;
    bvector<DPoint3d> pointsA;
    bvector<DPoint3d> pointsB;
    bvector<DVec3d> tangents;
    DVec3d normalA, normalB;
    double areaA, areaB;
    DPoint3d centroidA, centroidB;
    size_t numLoop;
    bvector<DPoint3d> basePolygon;
    m_builder.StrokeWithDoubledPointsAtCorners (contourA, pointsA, tangents, numLoop);
    m_builder.StrokeWithDoubledPointsAtCorners (contourB, pointsB, tangents, numLoop);
    if (  PolygonOps::CentroidNormalAndArea (pointsA, centroidA, normalA, areaA)
       && PolygonOps::CentroidNormalAndArea (pointsB, centroidB, normalB, areaB)
        )
        {
        reverse = centroidB.DotDifference (centroidA, normalA) < 0.0;
        }
    return reverse;
    }
};


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CollectChildAreaNormals(IPolyfaceConstruction &builder, CurveVector &parent, bvector<DVec3d> &normals, size_t &maxMagnitudeIndex)
    {
    bvector<DPoint3d> points;
    bvector<DVec3d> tangents;
    normals.clear ();
    maxMagnitudeIndex = SIZE_MAX;
    for (size_t i = 0; i < parent.size (); i++)
        {
        points.clear ();
        tangents.clear ();
        CurveVectorCP childLoop = parent[i]->GetChildCurveVectorCP ();
        if (NULL == childLoop)
            return false;
        if (!childLoop->IsClosedPath ())
            return false;
        size_t numLoop;
        builder.StrokeWithDoubledPointsAtCorners (*childLoop, points, tangents, numLoop);
        DVec3d polygonNormal = PolygonOps::AreaNormal (points);
        normals.push_back (polygonNormal);
        }
    if (normals.size () == 0)
        return false;
    double a, aMax;
    aMax = normals[0].Magnitude ();
    maxMagnitudeIndex = 0;
    for (size_t i = 0; i < normals.size (); i++)
        {
        a = normals[i].Magnitude ();
        if (a > aMax)
            {
            aMax = a;
            maxMagnitudeIndex = i;
            }
        }
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
// collect child vectors from specified index of top level vector.
// Return true if in each of the parents index is valid and resolves to a child curve vector.
bool GatherChildrenAtIndex
(
bvector<CurveVectorPtr> const &parents,
size_t index,
bvector<CurveVectorPtr> &children
)
    {
    size_t numParent = parents.size ();
    children.clear ();
    for (size_t i = 0; i < numParent; i++)
        {
        CurveVectorPtr child ((CurveVectorP)parents[i]->at(index)->GetChildCurveVectorCP ());

        if (NULL == child.get ())
            return false;
        children.push_back (child);
        }
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool IPolyfaceConstruction::AddRuledBetweenCorrespondingCurves
    (
    bvector<CurveVectorPtr> const &contours,
    bool capped
    )
    {
    GetClientMeshR ().SetTwoSided (!capped);
    size_t numContours = contours.size ();
    bool stat = false;
    if (numContours < 2)
        return false;
    size_t numChildren = contours[0]->size ();

    SynchOptions ();
    CorrespondingCurveState context (*this, capped);
    CurveVector::BoundaryType boundaryType = CommonBoundaryType (contours);
    if (boundaryType == CurveVector::BOUNDARY_TYPE_Open)
        {
        bool reversed = false;
        if (AllPhysicallyClosed (contours))
            reversed = context.NeedReverse (*contours[0], *contours[1]);
        stat = context.AddRuledBetweenCorrespondingChains (contours, reversed);
        }
    else if (boundaryType == CurveVector::BOUNDARY_TYPE_Outer
            || boundaryType == CurveVector::BOUNDARY_TYPE_Inner
            )
        {
        bool reversed = context.NeedReverse (*contours[0], *contours[1]);
        stat = context.AddRuledBetweenCorrespondingChains (contours, reversed);
        if (stat)
            context.EmitCaps (reversed);
        }
    else if (boundaryType == CurveVector::BOUNDARY_TYPE_ParityRegion)
        {
        bvector<DVec3d> normals;
        size_t maxNormalIndex;
        bool reversedPrimary = false;
        // All loop orientations follow the [0] contour loops
        if (CollectChildAreaNormals (*this, *contours[0], normals, maxNormalIndex))
            {
            bool outerLoopReversal = context.NeedReverse (
                        *contours[0]->at (maxNormalIndex)->GetChildCurveVectorCP (),
                        *contours[1]->at (maxNormalIndex)->GetChildCurveVectorCP ());
            bool currentLoopReversal;
            bvector<CurveVectorPtr> oneContourSet;
            DVec3d referenceNormal = normals[maxNormalIndex];
            for (size_t i = 0; i < numChildren; i++)
                {
                GatherChildrenAtIndex (contours, i, oneContourSet);
                // loops OTHER THAN the biggest have to get their normals opposite the main.
                currentLoopReversal = outerLoopReversal;
                if (i != maxNormalIndex)
                    {
                    if (normals[i].DotProduct (referenceNormal) > 0.0)
                        currentLoopReversal = !outerLoopReversal;
                    else
                        currentLoopReversal = outerLoopReversal;
                    }
                context.AddRuledBetweenCorrespondingChains (oneContourSet, currentLoopReversal);
                }
            if (capped)
                context.EmitCaps (reversedPrimary);
            }
        }
    else if (boundaryType == CurveVector::BOUNDARY_TYPE_UnionRegion)
        {
        bvector<CurveVectorPtr> oneContourSet;
        // Process children at each index ...
        for (size_t ich = 0; ich < numChildren; ich++)
            {
            GatherChildrenAtIndex (contours, ich, oneContourSet);
            AddRuledBetweenCorrespondingCurves (oneContourSet, capped);
            }
        }
    return stat;
    }


    
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IPolyfaceConstruction::AddRuled (DEllipse3dR ellipse0, DEllipse3dR ellipse1, bool cap)
    {
    SynchOptions ();
    double maxEdgeLength = GetFacetOptionsR ().GetMaxEdgeLength ();
    double angleTol      = GetFacetOptionsR ().GetAngleTolerance ();
    bvector<DPoint3d> point[2];
    bvector<DVec3d>   capTangent[2];
    bvector<DPoint2d> capParam[2];
    bvector<size_t> pointIndex[2];
    DEllipse3d fullEllipse[2];
    DRange2d capDistanceRange[2], capParamRange[2];
    size_t numCapChord[2];
    fullEllipse[0] = ellipse0;
    fullEllipse[1] = ellipse1;
    
    double length0 = ellipse0.ArcLength ();
    double length1 = ellipse1.ArcLength ();
    double nominalLength = length0 > length1 ? length0 : length1;
    static double s_startDegrees = 90.0;
    for (size_t i = 0; i < 2; i++)
        {
        fullEllipse[i].MakeFullSweep ();
        fullEllipse[i].start = Angle::DegreesToRadians (s_startDegrees);
        numCapChord[i] = GetFacetOptionsR ().FullEllipseStrokeCount (fullEllipse[i]);
        }
    size_t numChord = numCapChord[0] > numCapChord[1] ? numCapChord[0] : numCapChord[1];

    for (int i = 0; i < 2; i++)
        {
        AppendFullEllipseStrokes (fullEllipse[i], numChord, &point[i], &capTangent[i], &capParam[i]);
        FindOrAddPoints (point[i], numChord, 1, pointIndex[i]);
        }

    bvector<DPoint2d> paramA, paramB;
    double df = numChord == 0 ? 1.0 : 1.0 / (double)numChord;

    for (size_t i = 0; i < point[0].size (); i++)
        {
        paramA.push_back (DPoint2d::From (i * df, 0.0));
        paramB.push_back (DPoint2d::From (i * df, 1.0));
        }    

    if (cap)
        {
        for (int i = 0; i < 2; i++)
            {
            if (fullEllipse[i].IsNearZeroRadius ())
                continue;
            DPoint2d centerParam;
            centerParam.Zero ();
            size_t centerPointIndex = FindOrAddPoint (fullEllipse[i].center);
            bool reverse = (i == 0);
            AddPointIndexFan (centerPointIndex, pointIndex[i], numChord, reverse);
            if (NeedParams ())
                {
                bvector<size_t> paramIndex;
                capParam[i].push_back (centerParam);
                RemapPseudoDistanceParams (capParam[i], capDistanceRange[i], capParamRange[i], fullEllipse[i].vector0.Magnitude (), fullEllipse[i].vector90.Magnitude());
                FindOrAddParams (capParam[i], capParam[i].size (), 0, paramIndex);
                // Force index wraparound, but keep the center point at back.
                size_t centerParamIndex = paramIndex.back ();
                paramIndex.back () = paramIndex.front ();
                paramIndex.push_back (centerParamIndex);
                FindOrAddParams (capParam[i], numChord, 1, paramIndex);
                AddParamIndexFan (centerParamIndex, paramIndex, numChord, reverse);
                SetCurrentFaceParamDistanceRange (capDistanceRange[i]);
                }
            if (NeedNormals ())
                AddNormalIndexPlanarFan (fullEllipse[i].vector0, fullEllipse[i].vector90, reverse, numChord);
            EndFace_internal ();    
            }
        }
    AddSmoothRuledQuads1 (*this,
        point[0], pointIndex[0], capTangent[0], paramA, nullptr,
        point[1], pointIndex[1], capTangent[1], paramB, nullptr,
        nominalLength, maxEdgeLength, angleTol, true);
    EndFace_internal ();
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IPolyfaceConstruction::FindOrAddPoints (DPoint3dCP points, size_t n, bvector<size_t> &index)
    {
    index.clear();
    for (size_t i = 0; i < n; i++)
        index.push_back (FindOrAddPoint (points[i]));
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IPolyfaceConstruction::FindOrAddNormals (DVec3dCP normals, size_t n, bvector<size_t> &index)
    {
    index.clear();
    for (size_t i = 0; i < n; i++)
        index.push_back (FindOrAddNormal (normals[i]));
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IPolyfaceConstruction::FindOrAddParams (DPoint2dCP params, size_t n, bvector<size_t> &index)
    {
    index.clear();
    for (size_t i = 0; i < n; i++)
        index.push_back (FindOrAddParam (params[i]));
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void AddTriangle (IPolyfaceConstructionR builder,
    size_t k0, bool visible0,
    size_t k1, bool visible1,
    size_t k2, bool visible2,
    bvector<size_t> &pointIndex,
    bvector<size_t> &normalIndex,
    bvector<size_t> &paramIndex
    )
    {
    builder.AddPointIndexTriangle ( pointIndex[k0], visible0,
                        pointIndex[k1], visible1,
                        pointIndex[k2], visible2
                        );
    if (normalIndex.size () > 0)
        builder.AddNormalIndexTriangle (normalIndex[k0], normalIndex[k1], normalIndex[k2]);
    if (paramIndex.size () > 0)
        builder.AddParamIndexTriangle (paramIndex[k0], paramIndex[k1], paramIndex[k2]);
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IPolyfaceConstruction::AddRowMajorQuadGrid (DPoint3dCP points, DVec3dCP normals, DPoint2dCP params, size_t numPerRow, size_t numRow, bool forceTriangles)
    {
    static bool s_foldVisible = true;
    bvector<size_t> pointIndex, paramIndex, normalIndex;
    size_t n = numPerRow * numRow;
    pointIndex.reserve (n);
    FindOrAddPoints (points, n, pointIndex);
    if (NeedNormals () && NULL != normals)
        {
        normalIndex.reserve (n);
        FindOrAddNormals (normals, n, normalIndex);
        }
    if (NeedParams () && NULL != params)
        {
        paramIndex.reserve (n);
        FindOrAddParams (params, n, paramIndex);
        }
    
    for (size_t j1 = 1; j1 < numRow; j1++)
        {
        size_t j0 = j1 - 1;
        for (size_t i1 = 1; i1 < numPerRow; i1++)
            {
            size_t i0 = i1 - 1;
            // k0..k3 are indices into the index arrays ...
            size_t k0 = j0 * numPerRow + i0;
            size_t k1 = j0 * numPerRow + i1;
            size_t k2 = j1 * numPerRow + i1;
            size_t k3 = j1 * numPerRow + i0;
            bool visible01 = j0 == 0;
            bool visible12 = i1 == numPerRow - 1;
            bool visible23 = j1 == numRow - 1;
            bool visible30 = i0 == 0;
            if (pointIndex[k0] == pointIndex[k2])   // degenerate quad becomes facing triangles (013) and (123)
                {
                AddTriangle (*this, k0, visible01, k1, s_foldVisible, k3, visible30, pointIndex, normalIndex, paramIndex);
                AddTriangle (*this, k1, visible12, k2, visible23, k3, s_foldVisible, pointIndex, normalIndex, paramIndex);
                }
            else if (pointIndex[k1] == pointIndex[k3])  // degenerate quad becomes facing triangles 
                {
                AddTriangle (*this, k0, visible01, k1, visible12, k2, s_foldVisible, pointIndex, normalIndex, paramIndex);
                AddTriangle (*this, k2, visible23, k3, visible30, k0, s_foldVisible, pointIndex, normalIndex, paramIndex);
                }
            else    // full quad
                {
                AddPointIndexQuad ( pointIndex[k0], j0 == 0,
                                    pointIndex[k1], i1 == numPerRow - 1,
                                    pointIndex[k2], j1 == numRow - 1,
                                    pointIndex[k3], i0 == 0
                                    );
                if (normalIndex.size () > 0)
                    AddNormalIndexQuad (normalIndex[k0], normalIndex[k1], normalIndex[k2], normalIndex[k3]);
                if (paramIndex.size () > 0)
                    AddParamIndexQuad (paramIndex[k0], paramIndex[k1], paramIndex[k2], paramIndex[k3]);
                }
            }
        }
    }


// Add facets to a mesh.
// Facets approximate a tube around a centerline.
// The centerline curve (bspline) should be planar or nearly so.
//  (If it is not, the successive circular sections may pinch in strange ways)
//! @param [in] builder mesh builder
//! @param [in] centerlineCurve tube centerline
//! @param [in] radius tube radius
//! @param [in] numEdgePerSection number of edges around each section circle
//! @param [in] numSectionEdge number of edges along curve.
void IPolyfaceConstruction::AddTubeMesh
(
MSBsplineCurveCR centerlineCurve,
double radius,
int numEdgePerSection,  // points per section
int numSectionEdge      //
)
    {
    if (numEdgePerSection <= 0)
        numEdgePerSection = 24;
    if (numSectionEdge <= 0)
        numSectionEdge = 12;
    DVec3d tangentA, tangentB;
    // Evalaute tangents at more or less random points to get normal to the plane of the curve.
    DPoint3d pointA, pointB;
    centerlineCurve.FractionToPoint (pointA, tangentA, 0.0);
    centerlineCurve.FractionToPoint (pointB, tangentB, 0.12318);

    DVec3d planeNormal;
    if (tangentA.IsParallelTo (tangentB))
        {
        DVec3d vecX, vecY, vecZ;
        tangentA.GetNormalizedTriad (vecX, vecY, vecZ);
        planeNormal = vecX;
        }
    else
        planeNormal.NormalizedCrossProduct (tangentA, tangentB);
    
    // Build the sections ...
    bvector<DPoint3d> pointGrid;
    bvector<DVec3d> normalGrid;
    bvector<DPoint2d> paramGrid;
    // build grid of points on the circles.
    // Each circle gets numEdge + 1 points.  (The builder will find the duplicated wraparound point.)
    for (int i = 0; i <= numSectionEdge; i++)
        {
        double v = i / (double)numSectionEdge;
        double curveFraction = i / (double)(numSectionEdge);
        DPoint3d centerlinePoint;
        DVec3d   centerlineTangent;
        centerlineCurve.FractionToPoint (centerlinePoint, centerlineTangent, curveFraction);
        DVec3d vector0, vector90;
        vector90.NormalizedCrossProduct (centerlineTangent, planeNormal);
        vector0.NormalizedCrossProduct (vector90, centerlineTangent);
        DEllipse3d circle = DEllipse3d::FromScaledVectors (centerlinePoint, vector0, vector90, radius, radius, 0.0, Angle::TwoPi ());

        for (size_t k = 0; k <= (size_t)numEdgePerSection; k++)
            {
            DPoint3d xyz;
            double u = k / (double)numEdgePerSection;
            DVec3d sectionTangent, sectionBinormal;
            double hoopFraction = (double)k/(double)numEdgePerSection;
            circle.FractionParameterToDerivatives (xyz, sectionTangent, sectionBinormal, hoopFraction);
            DVec3d normal;
            normal.NormalizedCrossProduct (sectionTangent, centerlineTangent);
            pointGrid.push_back (xyz);
            normalGrid.push_back (normal);
            paramGrid.push_back (DPoint2d::From (u, v));
            }

        // To adjust to space curvature, rebuild out-of-plane normal with local direction
        planeNormal = vector0;
        }
    AddRowMajorQuadGrid (&pointGrid[0],
                GetFacetOptionsR().GetNormalsRequired () ? &normalGrid[0] : NULL,
                GetFacetOptionsR().GetParamsRequired ()  ? &paramGrid[0] : NULL,
                numEdgePerSection + 1, numSectionEdge+1);
    EndFace_internal ();
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IPolyfaceConstruction::AddTriStrip (DPoint3dCP points, DVec3dCP normals, DPoint2dCP params,size_t n, bool firstTriangle012)
    {
    bvector<size_t> index;
    index.reserve (n);
	
// Odd count:
//    1--3--5
//    |\ |\ |\
//    | \| \| \
//    0--2--4--6
// Even count:
//    1--3--5--7
//    |\ |\ |\ |
//    | \| \| \|
//    0--2--4--6
// Treat each position 2,3..n-1 as FINAL point of a triangle.
// There are two triangle configurations.
// Shift indices indicate position shift (BACKWARDS)
    size_t shift[2][3] =
        {
        {0,1,2}, {0,2,1}
        };
// Tell if specific edges are visible as top/bottom.
    bool viz0 [2] = {false, true};
    bool viz2 [2] = {true, false};
    FindOrAddPoints (points, n, index);
    size_t firstOutputK = 2;
    static double s_duplicateParamTolerance = 1.0e-8;
    if (NULL != params
        && params[0].IsEqual (params[1], s_duplicateParamTolerance))
        firstOutputK++;
// 
    size_t select0 = firstTriangle012 ? 0 : 1;
    for (size_t k2 = 2, select = select0; k2 < n; k2 += 1, select = 1 - select)
        {
        size_t i0 = k2 - shift[select][0];
        size_t i1 = k2 - shift[select][1];
        size_t i2 = k2 - shift[select][2];
        if (k2 >= firstOutputK)
            AddPointIndexTriangle (
                            index[i0], viz0[select] || k2 == n - 1,
                            index[i1], k2 == 2,
                            index[i2], viz2[select] || k2 == n - 2
                            );
        }

    if (NeedNormals () && NULL != normals)
        {
        FindOrAddNormals (normals, n, index);
        for (size_t k2 = 2, select = select0; k2 < n; k2++, select = 1 - select)
            {
            size_t i0 = k2 - shift[select][0];
            size_t i1 = k2 - shift[select][1];
            size_t i2 = k2 - shift[select][2];
            if (k2 >= firstOutputK)
                AddNormalIndexTriangle (
                                index[i0],
                                index[i1],
                                index[i2]
                                );
            }
        }
    
    if (NeedParams () && NULL != params)
        {
        FindOrAddParams (params, n, index);
        for (size_t k2 = 2, select = select0; k2 < n; k2++, select = 1 - select)
            {
            size_t i0 = k2 - shift[select][0];
            size_t i1 = k2 - shift[select][1];
            size_t i2 = k2 - shift[select][2];
            if (k2 >= firstOutputK)
                AddParamIndexTriangle (
                                index[i0],
                                index[i1],
                                index[i2]
                                );
            }
        }
    }

// Add triangulations of two supplied point strings.  In nonreversed case, pointA is "as is", pointB is reversed.
// In reversed case, pointA is reversed, pointB is "as is".
void IPolyfaceConstruction::AddTriangulationPair (bvector <DPoint3d> &pointA, bool reverseA, bvector <DPoint3d> &pointB, bool reverseB,
bool enableTriangulation,
bool edgeChainsPermitted,
CurveTopologyId::Type chainType
)
    {
    if (enableTriangulation)
        {
        if (reverseA)
            ToggleIndexOrderAndNormalReversal ();
        AddTriangulation (pointA);
        if (reverseA)
            ToggleIndexOrderAndNormalReversal ();
        if (reverseB)
            ToggleIndexOrderAndNormalReversal ();
        AddTriangulation (pointB);
        if (reverseB)
            ToggleIndexOrderAndNormalReversal ();
        }

    if (edgeChainsPermitted && GetFacetOptionsR ().GetEdgeChainsRequired ())
        {
        AddEdgeChains (chainType, 0, pointA);
        AddEdgeChains (chainType, 1, pointB);
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IPolyfaceConstruction::AddLinearSweep
(
bvector<DPoint3d> &pointA,
bvector<DVec3d> &tangentA,
DVec3dCR step
)
    {
    AddLinearSweep (pointA, &tangentA, step, false);
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IPolyfaceConstruction::AddLinearSweep
(
bvector<DPoint3d> &pointA,
bvector<DVec3d> *tangentA,
DVec3dCR step,
bool capped
)
    {
    size_t n = pointA.size ();
    if (NULL == tangentA)
        {
        bvector<DVec3d> expandedTangents;
        bvector<DPoint3d> expandedPoints;
        ExpandPolygonTangents (pointA, expandedPoints, expandedTangents);
        AddLinearSweep (expandedPoints, &expandedTangents, step, capped);
        return;
        }
    SynchOptions ();
    if (n < 2)
        return;

    DVec3d polygonNormal = PolygonOps::AreaNormal (pointA);
    bvector<DPoint3d> pointB;
    int dir = polygonNormal.DotProduct (step) > 0 ? 1 : -1;

    //double contourDistance = 0.0;
    static double s_pointRelTol = 1.0e-8;
    static double s_pointAbsTol = 1.0e-10;
    double pointTolerance = s_pointAbsTol + s_pointRelTol * PolylineOps::Length (pointA);
    double accumulatedDistance = 0.0;
    double stepLength = step.Magnitude ();
    size_t pointIndexA0 = 0, pointIndexB0 = 0, pointIndexA1 = 0, pointIndexB1 = 0;
    size_t normalIndex0 = 0, normalIndex1 = 0;  // Normal is identical at contours A, B.
    size_t paramIndexA0 = 0, paramIndexB0 = 0, paramIndexA1 = 0, paramIndexB1 = 0;
    DPoint3d xyzB1;
    DVec3d normal1;
    bool needNormals = NeedNormals ();
    bool needParams = NeedParams ();
    bool visible0 = false;
    bool visible1 = false;
    size_t lastIndex = n - 1;
    bool visible01 = pointA[0].AlmostEqual (pointA[lastIndex])
        && !IsVisibleJoint (pointA[0], pointA[lastIndex],
            tangentA->at(0), tangentA->at(lastIndex), nullptr, nullptr);

    if (dir == 1)
        ToggleIndexOrderAndNormalReversal ();

    for (size_t i = 0; i < n; i++)
        {
        xyzB1.SumOf (pointA[i], step);
        pointB.push_back (xyzB1);
        }

    for (size_t i = 0; i < n; i++)
        {
        xyzB1 = pointB[i];
        normal1.NormalizedCrossProduct (tangentA->at(i), step);
        pointIndexA1 = FindOrAddPoint (pointA[i]);
        pointIndexB1 = FindOrAddPoint (xyzB1);
        double du = 0.0;
        size_t visibiltyTestIndex;
        if (i == 0)
            {
            visibiltyTestIndex = n - 1;
            }
        else
            {
            du = pointA[i-1].Distance (pointA[i]);
            accumulatedDistance += du;
            visibiltyTestIndex = i + 1;
            if (i == n - 1)
                visibiltyTestIndex = 0;
            }

        if (i > 0 && i < n - 1)
            visible1 = IsVisibleJoint (pointA[i], pointA[i+1], tangentA->at(i), tangentA->at(i+1), nullptr, nullptr);
        else
            visible1 = visible01;

        if (needParams)
            {
            paramIndexA1 = FindOrAddParam (DPoint2d::From (accumulatedDistance, 0.0));
            paramIndexB1 = FindOrAddParam (DPoint2d::From (accumulatedDistance, stepLength));
            }

        if (needNormals)
            normalIndex1 = FindOrAddNormal (normal1);

        if (du > pointTolerance)
            {
            // Needs work --- hide smooth?
            AddPointIndexQuad (pointIndexA0, true, pointIndexA1, visible1, pointIndexB1, true, pointIndexB0, visible0);
            if (needNormals)
                AddNormalIndexQuad (normalIndex0, normalIndex1, normalIndex1, normalIndex0);
            if (needParams)
                AddParamIndexQuad (paramIndexA0, paramIndexA1, paramIndexB1, paramIndexB0);
            }

        visible0 = visible1;
        pointIndexA0 = pointIndexA1;
        pointIndexB0 = pointIndexB1;
        normalIndex0 = normalIndex1;
        paramIndexA0 = paramIndexA1;
        paramIndexB0 = paramIndexB1;
        }

    // We currently have simple distances in UOR's.
    EndFace_internal ();

    if (dir == 1)
        ToggleIndexOrderAndNormalReversal ();

    AddTriangulationPair (pointA, dir == 1, pointB, dir != 1, capped, false);
    // ?? Edge Chains ?? Maybe they only happen within SolidPrimitive?


    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t Remap(bvector<int32_t> &data, bvector<size_t> const &oldToNew)
    {
    size_t errors = 0;
    size_t numMapped = oldToNew.size ();
    for (size_t i = 0, n = data.size (); i < n; i++)
        {
        int32_t old =  data[i];
        if (old >= 0 && old < (int32_t)numMapped)
            {
            data[i] = (int32_t)oldToNew[(size_t)old];
            }
        else
            errors++;
        }
    return errors;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool IPolyfaceConstruction::Add (PolyfaceHeaderR mesh)
    {
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (mesh, true);
    bvector<size_t>oldPointToNewPoint;
    bvector<size_t>oldNormalToNewNormal;
    bvector<size_t>oldParamToNewParam;
    bvector<size_t>oldIntColorToNewIntColor;

    size_t numErrors = 0;
    // build tables from old to in all the data arrays..
    DPoint3dCP pPoint = mesh.GetPointCP ();
    for (size_t i = 0, n = mesh.GetPointCount (); i < n; i++)
        oldPointToNewPoint.push_back (FindOrAddPoint (pPoint[i]));

    DPoint2dCP pParam = mesh.GetParamCP ();
    for (size_t i = 0, n = mesh.GetParamCount (); i < n; i++)
        oldParamToNewParam.push_back (FindOrAddParam (pParam[i]));

    DVec3dCP pNormal = mesh.GetNormalCP ();
    for (size_t i = 0, n = mesh.GetNormalCount (); i < n; i++)
        oldNormalToNewNormal.push_back (FindOrAddNormal (pNormal[i]));


    // colors.  Only one index set.
    uint32_t const* pIntColor = mesh.GetIntColorCP ();
    for (size_t i = 0, n = mesh.GetColorCount (); i < n; i++)
        oldIntColorToNewIntColor.push_back (FindOrAddIntColor (pIntColor[i]));

    bvector<int32_t> &vPointIndex = visitor->ClientPointIndex ();
    bvector<int32_t> &vNormalIndex = visitor->ClientNormalIndex ();

    bvector<int32_t> &vParamIndex = visitor->ClientParamIndex ();
    bvector<int32_t> &vColorIndex = visitor->ClientColorIndex ();
    bvector<bool> &vVisible = visitor->Visible ();
                                                                                                                      
    visitor->Reset ();
    for (;visitor->AdvanceToNextFace ();)
        {
        size_t numErrorsThisFace = 0;
        numErrorsThisFace += Remap (vPointIndex, oldPointToNewPoint);
        numErrorsThisFace += Remap (vParamIndex, oldParamToNewParam);
        numErrorsThisFace += Remap (vColorIndex, oldIntColorToNewIntColor);
        numErrorsThisFace += Remap (vNormalIndex, oldNormalToNewNormal);
        numErrors += numErrorsThisFace;
        if (numErrorsThisFace == 0)
            {
            FacetFaceDataCP faceData = visitor->GetFaceDataCP();

            if (nullptr != faceData)
                SetFaceData(*faceData);

            for (size_t i = 0, n = vPointIndex.size (); i < n; i++)
                AddPointIndex (vPointIndex[i], vVisible [i]);
            AddPointIndexTerminator ();
            if (vNormalIndex.size () > 0)
                {
                for (size_t i = 0, n = vNormalIndex.size (); i < n; i++)
                    AddNormalIndex (vNormalIndex[i]);
                AddNormalIndexTerminator ();
                }
            if (vParamIndex.size () > 0)
                {
                for (size_t i = 0, n = vParamIndex.size (); i < n; i++)
                    AddParamIndex (vParamIndex[i]);
                AddParamIndexTerminator ();
                }
            if (vColorIndex.size () > 0)
                {
                for (size_t i = 0, n = vColorIndex.size (); i < n; i++)
                    AddColorIndex (vColorIndex[i]);
                AddColorIndexTerminator ();
                }
            if (nullptr != faceData)
                EndFace();
            }
        }
    return numErrors == 0;
    }



// Compute transforms rotated point data and determine if loops must be reversed.
static bool ComputeRotationalSweepLoopSense
(
bvector<DPoint3d> const &points,
DPoint3dCR origin, 
DVec3dCR axis,
double sweepRadians
)
    {
    DVec3d polygonNormal = PolygonOps::AreaNormal (points);

    double aMax = 0.0;
    DVec3d maxSweepVector;
    for (size_t i = 0, n = points.size (); i < n; i++)
        {
        DVec3d radialVector = DVec3d::FromStartEnd (origin, points[i]);
        DVec3d sweepVector;
        sweepVector.CrossProduct (axis, radialVector);
        double a = sweepVector.Magnitude ();
        if (a > aMax)
            {
            maxSweepVector = sweepVector;
            aMax = a;
            }
        }
    double b = sweepRadians * polygonNormal.DotProduct (maxSweepVector);
    return b > 0.0;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IPolyfaceConstruction::AddRotationalSweepLoop
(
bvector<DPoint3d> &pointA,
bvector<DVec3d> &tangentA,
DPoint3dCR origin,
DVec3dCR rotationAxis,
double   totalSweepRadians,
bool     reverse,
double   nominalBaseCurveLength,
bvector<DPoint3d> *startCapPointAccumulator,
bvector<DPoint3d> *endCapPointAccumulator,
bvector <ICurvePrimitiveP> *curve
)
    {
    DVec3d axis = rotationAxis;
    if (totalSweepRadians < 0.0)
        {
        totalSweepRadians = - totalSweepRadians;
        axis.Negate ();
        }
    if (totalSweepRadians > Angle::TwoPi ())
        totalSweepRadians = Angle::TwoPi ();
    SynchOptions ();
    size_t n = pointA.size ();
    if (n < 2)
        return;

    DVec3d xVec, yVec, zVec;
    if (!axis.GetNormalizedTriad (xVec, yVec, zVec))
        return;


    //DVec3d polygonNormal = PolygonOps::AreaNormal (pointA);


    Transform localToWorld = Transform::FromOriginAndVectors (origin, xVec, yVec, zVec);
    Transform worldToLocal;
    worldToLocal.InverseOf (localToWorld);

    bvector<DPoint3d> workPoint;
    bvector<DPoint2d>workParam;
    bvector<DVec3d>baseNormal;
    bvector<bool>baseCurveBreak;
    // true sweep vector per point is axis cross radial vector.
    // but that is zero if point is on or near axis.
    // and it flips direction if point is just "behind" the axis.
    // ASSUME (demand) the contour is supposed to be planar.
    // use the max xy radial vector when actual is small
    DVec3d maximalRadialVector = DVec3d::FromZero ();
    double aMax = 0.0;
    for (size_t i = 0; i < n; i++)
        {
        DVec3d radialVector = DVec3d::FromStartEnd (origin, pointA[i]);
        DVec3d localRadialVector;
        worldToLocal.MultiplyMatrixOnly (localRadialVector, radialVector);
        double a = localRadialVector.MagnitudeXY ();
        if (a > aMax)
            {
            aMax = a;
            maximalRadialVector = radialVector;
            }
        }
    double smallRadialTol = 0.01 * aMax;
    for (size_t i = 0; i < n; i++)
        {
        workPoint.push_back (pointA[i]);
        DVec3d radialVector = DVec3d::FromStartEnd (origin, pointA[i]);
        DVec3d localRadialVector;
        worldToLocal.MultiplyMatrixOnly (localRadialVector, radialVector);
        if (localRadialVector.MagnitudeXY () < smallRadialTol)
            radialVector = maximalRadialVector;
        DVec3d sweepVector, normalVector;
        sweepVector.CrossProduct (axis, radialVector);
        normalVector.NormalizedCrossProduct (sweepVector, tangentA[i]);
        baseNormal.push_back (normalVector);
        }
    size_t lastIndex = n - 1;
    baseCurveBreak.push_back (
        !pointA[0].AlmostEqual (pointA[lastIndex])
        || IsVisibleJoint (
                pointA[0], pointA[lastIndex],
                tangentA[0], tangentA[lastIndex],
                    curve ? curve->at (0) : nullptr,
                    curve ? curve->at (lastIndex) : nullptr
                ));
    for (size_t i = 1; i < lastIndex; i++)
        baseCurveBreak.push_back (IsVisibleJoint (pointA[i], pointA[i+1], tangentA[i], tangentA[i+1],
                    curve ? curve->at (i) : nullptr,
                    curve ? curve->at (i+1) : nullptr));

    baseCurveBreak.push_back (baseCurveBreak[0]);
    DPoint3dOps::Multiply (&workPoint, worldToLocal);
    // hm .. this aMax computation is probably redundant.  The usage above is "newer" so leave the old thing in place ..
    for (size_t i = 0; i < n; i++)
        {
        double a = workPoint[i].MagnitudeXY ();
        if (a > aMax)
            aMax = a;
        }
    double polylineLength = PolylineOps::Length (pointA);
    double curveParameterScale = 
                nominalBaseCurveLength == 0.0
                ? 1.0
                : (nominalBaseCurveLength / polylineLength);
    DPoint2d param;
    param.Zero ();
    workParam.push_back (param);
    for (size_t i = 1; i < n; i++)
        {
        param.x += pointA[i-1].Distance (pointA[i]) * curveParameterScale;
        workParam.push_back (param);
        }
    Transform paramTransform;
    DRange2d distanceRange, targetRange;
    TryTransformFromPseudoDistanceRectangle (GetFacetOptionsR().GetParamMode (),
        DRange2d::From (0,0, nominalBaseCurveLength, aMax * totalSweepRadians),
        1.0, 1.0,
        distanceRange, targetRange, paramTransform
        );
    SetCurrentFaceParamDistanceRange (distanceRange);
    DEllipse3d ellipse = DEllipse3d::From (0,0,0, aMax,0,0, 0,aMax,0, 0.0, totalSweepRadians);
    size_t numStep = GetFacetOptionsR ().EllipseStrokeCount (ellipse);

    bvector <size_t> pointIndexA,  pointIndexB;
    bvector <size_t> normalIndexA, normalIndexB;
    bvector <size_t> paramIndexA,  paramIndexB;
    bool closedSweep = bsiTrig_isAngleFullCircle (totalSweepRadians) ? true : false;
    double angleStep = totalSweepRadians / (double)(numStep);
    double paramYStep = angleStep * aMax;
    bool needNormals = NeedNormals ();
    bool needParams = NeedParams ();
    bool needEdgeChains = GetFacetOptionsR().GetEdgeChainsRequired ();
    bvector<PolyfaceEdgeChain> &chains = GetClientMeshR().EdgeChain ();
    bvector<size_t> edgeChainIndex;
    if (needEdgeChains)
        {
        for (size_t j = 0; j < n; j++)
            edgeChainIndex.push_back (SIZE_MAX);
        }

    if (reverse)
        ToggleIndexOrderAndNormalReversal ();
    for (size_t step = 0; step <= numStep; step++)
        {
        pointIndexB.clear ();
        paramIndexB.clear ();
        normalIndexB.clear ();
        Transform stepTransform;
        RotMatrix stepMatrix = RotMatrix::FromVectorAndRotationAngle (axis, angleStep * step);
        stepTransform = Transform::FromMatrixAndFixedPoint (stepMatrix, origin);
        
        workPoint = pointA;
        DPoint3dOps::Multiply (&workPoint, stepTransform);
        DVec3d normal;
        for (size_t i = 0; i < n; i++)
            {
            if (needNormals)
                {
                normal.Multiply (stepMatrix, baseNormal[i]);
                normalIndexB.push_back (FindOrAddNormal (normal));
                }
            pointIndexB.push_back (FindOrAddPoint (workPoint[i]));
            if (needParams)
                {
                workParam[i].y += paramYStep;
                DPoint2d uv = workParam[i];
                paramTransform.Multiply (&uv, &uv, 1);
                paramIndexB.push_back (FindOrAddParam (uv));
                }
            }

        if (step > 0)
            {
            for (size_t j1 = 1; j1 < n; j1++)
                {
                size_t j0 = j1 - 1;
                auto jA0 = pointIndexA[j0];
                auto jB0 = pointIndexB[j0];
                auto jA1 = pointIndexA[j1];
                auto jB1 = pointIndexB[j1];
                //
                //   jA1---<---jB1
                //    |        |
                //    |        |
                //   jA0--->---jB0
                if (jA0 != jA1)     // filter out duplicate points along contour
                    {
                    // degenerate to triangles if needed --- this messes up paramter space, but nobody but me cares.
                    if (jA0 != jB0 && jA1 != jB1)
                        {
                        AddPointIndexQuad (
                                pointIndexA[j0], baseCurveBreak[j0],
                                pointIndexB[j0], step == numStep && !closedSweep,
                                pointIndexB[j1], baseCurveBreak[j1],
                                pointIndexA[j1], step == 1 && !closedSweep
                                );
                        if (needNormals)
                            AddNormalIndexQuad (normalIndexA[j0], normalIndexB[j0], normalIndexB[j1], normalIndexA[j1]);
                        if (needParams)
                            AddParamIndexQuad (paramIndexA[j0], paramIndexB[j0], paramIndexB[j1], paramIndexA[j1]);
                        }
                    else if (jA0 != jB0)
                        {
                        // skip jA1
                        AddPointIndexTriangle (
                            pointIndexA[j0], baseCurveBreak[j0],
                            pointIndexB[j0], step == numStep && !closedSweep,
                            pointIndexB[j1], step == 1 && !closedSweep
                        );
                        if (needNormals)
                            AddNormalIndexTriangle (normalIndexA[j0], normalIndexB[j0], normalIndexA[j1]);
                        if (needParams)
                            AddParamIndexTriangle (paramIndexA[j0], paramIndexB[j0], paramIndexA[j1]);
                        }
                    else if (jA1 != jB1)
                        {
                        // skip IndexB[j0]
                        AddPointIndexTriangle (
                            pointIndexA[j0], step == numStep && !closedSweep,
                            pointIndexB[j1], baseCurveBreak[j1],
                            pointIndexA[j1], step == 1 && !closedSweep
                        );
                        if (needNormals)
                            AddNormalIndexTriangle (normalIndexA[j0], normalIndexB[j1], normalIndexA[j1]);
                        if (needParams)
                            AddParamIndexTriangle (paramIndexA[j0], paramIndexB[j1], paramIndexA[j1]);
                        }
                    }
                }
            if (needEdgeChains)
                {
                uint32_t numLateral = (uint32_t)n;
                if (pointIndexA.front () == pointIndexA.back ()
                    && pointIndexB.front () == pointIndexB.back ())
                        {
                        numLateral--;
                        edgeChainIndex.back () = SIZE_MAX;
                        }
                for (uint32_t j = 0; j < numLateral; j++)
                    {
                    size_t chainIndex = edgeChainIndex[j];
                    if (baseCurveBreak[j])
                        {
                        if (chainIndex == SIZE_MAX)
                            {
                            // This lateral does not have an active edge chain.
                            /// Create a new one, record its index, and put the pointA index in ...
                            chainIndex = chains.size ();
                            chains.push_back (PolyfaceEdgeChain (CurveTopologyId (CurveTopologyId::Type::SweepLateral, j)));
                            chains.back ().AddZeroBasedIndex ((int32_t)pointIndexA[j]);
                            edgeChainIndex[j] = chainIndex;
                            }
                        // Add the pointB index to the evolving chain . . 
                        chains[chainIndex].AddZeroBasedIndex((int32_t)pointIndexB[j]);
                        }
                    else
                        {
                        // there's nothing here for subsequent layers to connect to.
                        edgeChainIndex[j] = SIZE_MAX;
                        }
                    }
                }
            }
        paramIndexA = paramIndexB;
        normalIndexA = normalIndexB;
        pointIndexA = pointIndexB;
        }
    if (reverse)
        ToggleIndexOrderAndNormalReversal ();
    EndFace_internal ();

    if (NULL != startCapPointAccumulator)
        DPoint3dOps::Append (startCapPointAccumulator, &pointA);
    if (NULL != endCapPointAccumulator)
        DPoint3dOps::Append (endCapPointAccumulator, &workPoint);
    }

// ugh.   FractionToPoint names are slightly different for various types...
static void FractionToPoint (DSegment3dCR geometry, double f, DPoint3dR xyz, DVec3dR tangent)
    {
    geometry.FractionParameterToTangent (xyz, tangent, f);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void FractionToPoint(DEllipse3dCR geometry, double f, DPoint3dR xyz, DVec3dR tangent)
    {
    DVec3d deflection;
    geometry.FractionParameterToDerivatives (xyz, tangent, deflection, f);
    }
#ifdef CompileAll

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void FractionToPoint(ICurvePrimitive const & geometry, double f, DPoint3dR xyz, DVec3dR tangent)
    {
    geometry.FractionToPoint (f, xyz, tangent);
    }
#endif



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
template <typename T>
static bool StrokeByUniformFractions
(
T const &geometry,
size_t count,
bvector<DPoint3d> &points,
bvector<DVec3d>   &tangents
)
    {
    if (count > 0)
        {
        double df = 1.0 / (double)count;
        DPoint3d xyz;
        DVec3d tangent;
        for (size_t i = 0; i <= count; i++)
            {
            double f = i * df;
            if (i == count)
                f = 1.0;    // prevent numerical fuzz
            FractionToPoint (geometry, f, xyz, tangent);
            points.push_back (xyz);
            tangents.push_back (tangent);
            }
        }
    return count > 0;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
template <typename T>
static bool StrokeByUniformFractions
(
ICurvePrimitiveCP curve,
T const &geometry,
double f0,
double f1,
size_t count,
DPoint3dDoubleUVCurveArrays &pointTangentCurve
)
    {
    if (count > 0)
        {
        double df = 1.0 / (double)count;
        DPoint3d xyz;
        DVec3d tangent;
        for (size_t i = 0; i <= count; i++)
            {
            double f = i * df;
            if (i == count)
                f = 1.0;    // prevent numerical fuzz
            FractionToPoint (geometry, f, xyz, tangent);
            pointTangentCurve.Add (xyz, DoubleOps::Interpolate (f0, f, f1), tangent, const_cast <ICurvePrimitive*>(curve));
            }
        }
    return count > 0;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
template <typename T>
static bool StrokeByUniformFractions
(
T const &geometry,
size_t count,
bvector<DPoint3d> &points
)
    {
    if (count > 0)
        {
        double df = 1.0 / (double)count;
        DPoint3d xyz;
        DVec3d tangent;
        for (size_t i = 0; i <= count; i++)
            {
            double f = i * df;
            if (i == count)
                f = 1.0;    // prevent numerical fuzz
            FractionToPoint (geometry, f, xyz, tangent);
            if (points.size () == 0
                || !DPoint3dOps::AlmostEqual (xyz, points.back ()))
                points.push_back (xyz);
            }
        }
    return count > 0;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2014
+--------------------------------------------------------------------------------------*/
bool IPolyfaceConstruction::StrokeWithDoubledPointsAtCorners
(
CurveVectorCR curves,
bvector<bvector<DPoint3d> >&points,
bvector<bvector<DVec3d> >  &tangents,
bvector<double> &curveLengths
)
    {
    IFacetOptionsR options = GetFacetOptionsR ();
    CurveVectorCP child = NULL;
    DSegment3d segment;
    DEllipse3d ellipse;
    bvector<DPoint3d>const *linestring;
    bool makeNewLoop = true;
    for (size_t i = 0; i < curves.size (); i++)
        {
        ICurvePrimitivePtr primitive = curves.at(i);
        if (NULL != (child = primitive->GetChildCurveVectorCP ()))
            {
            StrokeWithDoubledPointsAtCorners (*child, points, tangents, curveLengths);
            makeNewLoop = true;
            }
        else
            {
            if (makeNewLoop)
                {
                points.push_back (bvector<DPoint3d> ());
                tangents.push_back (bvector<DVec3d> ());
                curveLengths.push_back (0);
                makeNewLoop = false;
                }
            if (primitive->TryGetLine (segment))
                {
                StrokeByUniformFractions<DSegment3d> (segment,
                    options.SegmentStrokeCount (segment),
                    points.back (), tangents.back ());
                curveLengths.back () += segment.Length ();
                }
            else if (NULL != (linestring = primitive->GetLineStringCP ()))
                {
                for (size_t i = 1, n = linestring->size (); i < n; i++)
                    {
                    DSegment3d segment;
                    segment.point[0] = linestring->at(i-1);
                    segment.point[1] = linestring->at(i);
                    StrokeByUniformFractions<DSegment3d> (segment,
                        options.SegmentStrokeCount (segment),
                    points.back (), tangents.back ());
                    curveLengths.back () += segment.Length ();
                    }
                }
            else if (primitive->TryGetArc (ellipse))
                {
                StrokeByUniformFractions <DEllipse3d>(ellipse,
                    options.EllipseStrokeCount (ellipse),
                    points.back (), tangents.back ());
                if (ellipse.IsFullEllipse () && !points.back ().empty ())    // enforce bitwise closure
                    points.back ().back () = points.back ().front ();
                double a;
                primitive->Length (a);
                curveLengths.back () += a;
                }
            else if (AppendBsplineStrokes (*this, primitive, points.back (), &tangents.back ()))
                {
                double a;
                primitive->Length (a);
                curveLengths.back () += a;
                }
            else
                return false;
            }
        }
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2014
+--------------------------------------------------------------------------------------*/
bool IPolyfaceConstruction::StrokeWithDoubledPointsAtCorners
(
CurveVectorCR curves,
bvector<DPoint3dDoubleUVCurveArrays> &pointTangentCurve,
bvector<double> &curveLength
)
    {
    IFacetOptionsR options = GetFacetOptionsR ();
    CurveVectorCP child = NULL;
    DSegment3d segment;
    DEllipse3d ellipse;
    bvector<DPoint3d>const *linestring;
    bool makeNewLoop = true;
    for (size_t i = 0; i < curves.size (); i++)
        {
        ICurvePrimitivePtr primitive = curves.at(i);
        if (NULL != (child = primitive->GetChildCurveVectorCP ()))
            {
            StrokeWithDoubledPointsAtCorners (*child, pointTangentCurve, curveLength);
            makeNewLoop = true;
            }
        else
            {
            if (makeNewLoop)
                {
                pointTangentCurve.push_back (DPoint3dDoubleUVCurveArrays());
                curveLength.push_back (0);
                makeNewLoop = false;
                }
            if (primitive->TryGetLine (segment))
                {
                StrokeByUniformFractions<DSegment3d> (primitive.get (), segment, 0.0, 1.0,
                    options.SegmentStrokeCount (segment), pointTangentCurve.back ());
                curveLength.back () += segment.Length ();
                }
            else if (NULL != (linestring = primitive->GetLineStringCP ()))
                {
                size_t n = linestring->size ();
                double df = n > 1 ? 1.0 / (double)(n-1) : 0.0;
                for (size_t i = 1, n = linestring->size (); i < n; i++)
                    {
                    DSegment3d segment;
                    segment.point[0] = linestring->at(i-1);
                    segment.point[1] = linestring->at(i);
                    StrokeByUniformFractions<DSegment3d> (primitive.get (), segment, (i - 1) * df, i * df,
                        options.SegmentStrokeCount (segment), pointTangentCurve.back ());
                    curveLength.back () += segment.Length ();
                    }
                }
            else if (primitive->TryGetArc (ellipse))
                {
                StrokeByUniformFractions <DEllipse3d>(primitive.get (), ellipse, 0.0, 1.0,
                    options.EllipseStrokeCount (ellipse), pointTangentCurve.back ());
                if (ellipse.IsFullEllipse () && !pointTangentCurve.back ().m_xyz.empty ())    // enforce bitwise closure
                    pointTangentCurve.back ().m_xyz.back () = pointTangentCurve.back ().m_xyz.front ();
                double a;
                primitive->Length (a);
                curveLength.back () += a;
                }
            else if (AppendBsplineStrokes (*this,
                    primitive,
                    pointTangentCurve.back ().m_xyz,
                    &pointTangentCurve.back ().m_vectorU,
                    &pointTangentCurve.back ().m_f,
                    &pointTangentCurve.back ().m_curve))
                {
                double a;
                primitive->Length (a);
                curveLength.back () += a;
                }
            else
                return false;
            }
        }
    return true;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool IPolyfaceConstruction::StrokeWithDoubledPointsAtCorners
(
CurveVectorCR curves,
bvector<DPoint3d> &points,
bvector<DVec3d>   &tangents,
size_t &numLoop
)
    {
    IFacetOptionsR options = GetFacetOptionsR ();
    CurveVectorCP child = NULL;
    DSegment3d segment;
    DEllipse3d ellipse;
    bvector<DPoint3d>const *linestring;
    //int errors = 0;
    numLoop = 1;
    for (size_t i = 0; i < curves.size (); i++)
        {
        ICurvePrimitivePtr primitive = curves.at(i);
        if (primitive->TryGetLine (segment))
            {
            StrokeByUniformFractions<DSegment3d> (segment,
                options.SegmentStrokeCount (segment),
                points, tangents);
            }
        else if (NULL != (linestring = primitive->GetLineStringCP ()))
            {
            for (size_t i = 1, n = linestring->size (); i < n; i++)
                {
                DSegment3d segment;
                segment.point[0] = linestring->at(i-1);
                segment.point[1] = linestring->at(i);
                StrokeByUniformFractions<DSegment3d> (segment,
                    options.SegmentStrokeCount (segment),
                    points, tangents);
                }
            }
        else if (primitive->TryGetArc (ellipse))
            {
            StrokeByUniformFractions <DEllipse3d>(ellipse,
                options.EllipseStrokeCount (ellipse),
                points, tangents);
            }
        else if (AppendBsplineStrokes (*this, primitive, points, &tangents))
            {
            }
        else if (NULL != (child = primitive->GetChildCurveVectorCP ()))
            {
            size_t numLoop1;
            StrokeWithDoubledPointsAtCorners (*child, points, tangents, numLoop1);
            numLoop += numLoop1;
            DPoint3dOps::AppendDisconnect (&points);
            DVec3dOps::AppendDisconnect (&tangents);
            }
        else return false;
        }
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool IPolyfaceConstruction::Stroke
(
CurveVectorCR curves,
bvector<DPoint3d> &points,
size_t &numLoop
)
    {
    IFacetOptionsR options = GetFacetOptionsR ();
    CurveVectorCP child = NULL;
    DSegment3d segment;
    DEllipse3d ellipse;
    bvector<DPoint3d>const *linestring;
    //int errors = 0;
    numLoop = 1;
    for (size_t i = 0; i < curves.size (); i++)
        {
        ICurvePrimitivePtr primitive = curves.at(i);
        if (primitive->TryGetLine (segment))
            {
            StrokeByUniformFractions<DSegment3d> (segment,
                options.SegmentStrokeCount (segment),
                points);
            }
        else if (NULL != (linestring = primitive->GetLineStringCP ()))
            {
            for (size_t i = 1, n = linestring->size (); i < n; i++)
                {
                DSegment3d segment;
                segment.point[0] = linestring->at(i-1);
                segment.point[1] = linestring->at(i);
                StrokeByUniformFractions<DSegment3d> (segment,
                    options.SegmentStrokeCount (segment),
                    points);
                }
            }
        else if (primitive->TryGetArc (ellipse))
            {
            StrokeByUniformFractions <DEllipse3d>(ellipse,
                options.EllipseStrokeCount (ellipse),
                points);
            }
        else if (AppendBsplineStrokes (*this, primitive, points, NULL))
            {
            }
        else if (NULL != (child = primitive->GetChildCurveVectorCP ()))
            {
            size_t numLoop1;
            Stroke (*child, points, numLoop1);
            numLoop += numLoop1;
            DPoint3dOps::AppendDisconnect (&points);
            }
        else return false;
        }
    return true;
    }


// to be called for curves confirmed as outer or parity region (with correct outer/inner)
static void AddRotationalSweep_singleRegion (IPolyfaceConstructionR builder,
CurveVectorPtr curve,
DPoint3dCR origin,
DVec3dCR axis,
double   totalSweepRadians,
bool     capped
)
    {
    if(curve.IsValid ())
        {
        bvector<DPoint3dDoubleUVCurveArrays> loopData;
        bvector<double> curveLengths;
        builder.StrokeWithDoubledPointsAtCorners (*curve, loopData, curveLengths);
        for (auto &loop : loopData)
            {
            if (loop.m_xyz.size () != loop.m_vectorU.size ()
                || loop.m_xyz.size () != loop.m_curve.size ())
                    GEOMAPI_PRINTF ("loop sizes %d %d %d\n", (int)loop.m_xyz.size (),(int)loop.m_vectorU.size (),(int)loop.m_curve.size ());
            }
        bool reverse = curve->IsAnyRegionType () ? ComputeRotationalSweepLoopSense(loopData[0].m_xyz, origin, axis, totalSweepRadians)
                : false;

        bvector<DPoint3d> endCapPoints;
        bvector<DPoint3d> startCapPoints;

        for (size_t i = 0; i < loopData.size (); i++)
            {
            builder.AddRotationalSweepLoop (
                loopData[i].m_xyz,
                loopData[i].m_vectorU,
                origin, axis, totalSweepRadians, reverse, curveLengths[i], &startCapPoints, &endCapPoints,
                &loopData[i].m_curve);
            DPoint3dOps::AppendDisconnect (&startCapPoints);
            DPoint3dOps::AppendDisconnect (&endCapPoints);
            }

        builder.AddTriangulationPair(startCapPoints, reverse, endCapPoints, !reverse,
            capped,
            !Angle::IsFullCircle (totalSweepRadians), CurveTopologyId::Type::SweepProfile);

        }
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IPolyfaceConstruction::AddRotationalSweep
(
CurveVectorPtr curve,
DPoint3dCR origin,
DVec3dCR axis,
double   totalSweepRadians,
bool     capped
)
    {
    if (curve->IsUnionRegion () && curve->IsParityRegion ())    // We don't trust the loops on anything more than a single....
        {
        LocalCoordinateSelect frameType = LOCAL_COORDINATE_SCALE_UnitAxesAtStart;
        Transform localToWorld, worldToLocal;
        DRange3d localRange;
        CurveVectorPtr fixup = curve->CloneInLocalCoordinates (frameType, localToWorld, worldToLocal, localRange);
        // fixup as viewed in xy plane
        bool ok = fixup->FixupXYOuterInner (false);
        if (!ok)
            capped = false;
        // go back to world
        fixup->TransformInPlace (localToWorld);
        if (fixup->IsUnionRegion ())
            {
            for (size_t i = 0; i < fixup->size (); i++)
                AddRotationalSweep_singleRegion (*this, fixup->at(i)->GetChildCurveVectorP (), origin, axis, totalSweepRadians, capped);
            }
        else
            {
            AddRotationalSweep_singleRegion (*this, fixup->at(0)->GetChildCurveVectorP (), origin, axis, totalSweepRadians, capped);
            }
        }
    else
        AddRotationalSweep_singleRegion (*this, curve, origin, axis, totalSweepRadians, capped);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IPolyfaceConstruction::AddRegion (CurveVectorCR region)
    {
    if (region.IsUnionRegion ())
        {
        for (size_t i = 0; i < region.size (); i++)
            {
            CurveVectorCP child = region[i]->GetChildCurveVectorCP ();
            if (NULL != child)
                AddRegion (*child);
            }
        }
    else if (region.IsAnyRegionType ())
        {
        DEllipse3d ellipse;
        double maxEdgeLength = GetFacetOptionsR ().GetMaxEdgeLength ();
        if (region.IsEllipticDisk (ellipse))
            {
            if (maxEdgeLength == 0.0)
                AddFullDisk (ellipse, 0);
            else
                AddFullDiskTriangles (ellipse, 0);
            }
        else
            {
            bvector<DPoint3d> points;
            size_t numLoop;
            Stroke (region, points, numLoop);
            AddTriangulation (points);
            }
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void IPolyfaceConstruction::AddTriangles (bvector<DTriangle3d> const &triangles, bool reverse, bvector<DTriangle3d> const *paramTriangles)
    {
    bool needNormals = NeedNormals ();
    bool needParams = NeedParams ();
    SynchOptions();
    if (reverse)
        ToggleIndexOrderAndNormalReversal ();
    for (size_t i = 0; i < triangles.size (); i++)
        {
        auto &triangle = triangles[i];
        auto normal = DVec3d::FromCrossProductToPoints (triangle.point[0], triangle.point[1], triangle.point[2]).ValidatedNormalize ();
        if (normal.IsValid ())
            {
            auto pointIndexA = FindOrAddPoint (triangle.point[0]);
            auto pointIndexB = FindOrAddPoint (triangle.point[1]);
            auto pointIndexC = FindOrAddPoint (triangle.point[2]);
            AddPointIndexTriangle (pointIndexA, true, pointIndexB, true, pointIndexC, true);

            if (needNormals)
                {
                auto normalIndex = FindOrAddNormal (normal);
                AddNormalIndexTriangle (normalIndex, normalIndex, normalIndex);
                }
            if (needParams)
                {
                DPoint2d uv0 = DPoint2d::From (0,0);
                DPoint2d uv1 = DPoint2d::From (1,0);
                DPoint2d uv2 = DPoint2d::From (0,1);
                if (paramTriangles != nullptr && i < paramTriangles->size ())
                    {
                    DTriangle3d paramTriangle = paramTriangles->at(i);
                    uv0 = DPoint2d::From (paramTriangle.point[0]);
                    uv1 = DPoint2d::From (paramTriangle.point[1]);
                    uv2 = DPoint2d::From (paramTriangle.point[2]);
                    }
                auto paramIndex0 = FindOrAddParam (uv0);
                auto paramIndex1 = FindOrAddParam (uv1);
                auto paramIndex2 = FindOrAddParam (uv2);
                AddParamIndexTriangle (paramIndex0, paramIndex1, paramIndex2);
                }
            }
        }
    if (reverse)
        ToggleIndexOrderAndNormalReversal ();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
IPolyfaceConstructionPtr IPolyfaceConstruction::New (IFacetOptionsR options, double pointMatchTolerance, double paramMatchTolerance, double normalMatchTolerance)
    {
    auto instance = new PolyfaceConstruction (options, pointMatchTolerance, paramMatchTolerance, normalMatchTolerance);
    instance->SynchOptions();
    return instance;
    }

IPolyfaceConstructionPtr IPolyfaceConstruction::Create (IFacetOptionsR options, double pointMatchTolerance, double paramMatchTolerance, double normalMatchTolerance)
    {
    auto instance = new PolyfaceConstruction (options, pointMatchTolerance, paramMatchTolerance, normalMatchTolerance);
    instance->SynchOptions();
    return instance;
    }


END_BENTLEY_GEOMETRY_NAMESPACE
