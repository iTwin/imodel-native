/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE


//! Callback signature for linestring-to-mesh sweep output.
//! @param userData IN caller's context data.
//! @param projectedPointA start point of (swept) line segment
//! @param projectedPointB end point of (swept) line segment
//! @param linestringIndexA index of parent segment
//! @param fractionA fractional position of pointA on parent line segment
//! @param fractionB fractional position of pointB on parent line segment
//! @param facetReadIndex index of facet data in index arrays
//! @param facetNoraml normal to this facet.
typedef void (*MdlMesh_SweptSegmentCollector)
(
void *userData,
DPoint3dCR projectedPointA,
DPoint3dCR projectedPointB,
size_t linestringIndexA,
double fractionA,
double fractionB,
size_t facetReadIndex,
DVec3dCR facetNormal
);


static double s_uvwTol = 1.0e-8;
// ASSUME one of h0, h1 is positive (keep positives)
bool ClipToInterval (double &a, double &b, double &h0, double &h1)
    {
    if (fabs (h0 - h1) <= s_uvwTol)
        {
        // hm.. leave it alone.
        }
    else if (h0 <= h1)
        {
        if (h0 < 0.0)
            {
            double s = -h0 / (h1 - h0);
            if (s > a)
                a = s;
            }
        }
    else
        {
        if (h1 < 0.0)
            {
            double s = -h0 / (h1 - h0);
            if (s < b)
                b = s;
            }
        }
    return b >= a;
    }

static double bsiTransform_multiplyXRow (TransformCR transform, DPoint3dCR point)
    {
    return transform.form3d[0][0] * point.x
           + transform.form3d[0][1] * point.y
            + transform.form3d[0][2] * point.z
              + transform.form3d[0][3];
    }

static double bsiTransform_multiplyYRow (TransformCR transform, DPoint3dCR point)
    {
    return transform.form3d[1][0] * point.x
           + transform.form3d[1][1] * point.y
            + transform.form3d[1][2] * point.z
              + transform.form3d[1][3];
    }

static void bsiDRange2d_fromTransformedPoints (DRange2dR range, TransformCR transform, DPoint3dCP points, size_t n)
    {
    range.Init ();
    for (size_t i = 0; i < n; i++)
        {
        range.Extend (  bsiTransform_multiplyXRow (transform, points[i]),
                        bsiTransform_multiplyYRow (transform, points[i])
                     );
        }
    }

struct TransformedRange
{
Transform m_worldToLocal;
DRange2d m_localRange;

TransformedRange (TransformCR worldToLocal, DRange2dCR localRange)
    : m_worldToLocal (worldToLocal),
      m_localRange (localRange)
    {
    }
bool AnyPointsInLocalXY (bvector<DPoint3d> &worldPoints)
    {
    for (size_t i = 0, n = worldPoints.size (); i < n; i++)
        {
        DPoint3d uvw;
        m_worldToLocal.Multiply(uvw, worldPoints[i]);
        if (m_localRange.Contains (uvw.x, uvw.y))
            return true;        
        }
    return false;
    }
void TransformToLocal (bvector<DPoint3d> &localPoints, DRange2dR dataRange, DPoint3dCP worldPoints, size_t count)
    {
    localPoints.clear ();
    for (size_t i = 0; i < count; i++)
        {
        DPoint3d uvw;
        m_worldToLocal.Multiply(uvw, worldPoints[i]);
        localPoints.push_back (uvw);
        }
    dataRange.InitFrom (&localPoints[0], (int) count);
    }

void InitLocalRange (DPoint3dCP points, size_t n, DRange2dR range)
    {
    range.Init ();
    for (size_t i = 0; i < n; i++)
        {
        DPoint3d uvw;
        m_worldToLocal.Multiply(uvw, points[i]);
        range.Extend (uvw.x, uvw.y);
        }
    }
};


struct  LocalizedIndexRange : TransformedRange
{
size_t m_pointIndex0;
size_t m_sortIndex0;
size_t m_numPoint;
DRange2d m_viewRange;

LocalizedIndexRange (
    size_t pointIndex0,
    size_t sortIndex0,
    size_t numPoint,
    DRange2dCR viewRange,
    TransformCR worldToLocal,
    DRange2d localRange) :
    m_pointIndex0 (pointIndex0),
    m_sortIndex0 (sortIndex0), 
    m_numPoint (numPoint),
    m_viewRange (viewRange),
    TransformedRange (worldToLocal, localRange)
    {
    }

// Given an (inclusive) index range i0In..i1In for a sorted sequence of doubles.
// Find the (possibly smaller) index range i0Out..i1Out the data interval that overlaps aLow,aHigh.
static bool OverlapIndices
(
bvector<double> &sortedData,
size_t i0In,
size_t i1In,
double aLow,
double aHigh,
size_t &i0Out,
size_t &i1Out
)
    {
    double a0 = sortedData[i0In];
    double a1 = sortedData[i1In];
    if (a0 >= aHigh || a1 <= aLow)
        return false;
    i0Out = i0In;
    i1Out = i1In;
    // push each index beyond its limit ....
    // (Should this be binary search?   It's all going to be preceded by gross range checks, so
    //    "many" pairings are eliminated and the additional benefit of binary may be limited)
    while (i1Out > i0In && sortedData[i1Out] >= aHigh)
        i1Out--;
    while (i0Out < i1In && sortedData[i0Out] <= aLow)
        i0Out++;
    // expand each end
    if (i1Out < i1In)
        i1Out++;
    if (i0Out > i0In)
        i0Out--;
    return true;
    }

bool LocalIndexOverlap
(
bvector<double> &sortedValues,
DRange2dR viewRange,
DPoint3dCP worldXYZ,
size_t numXYZ,
size_t &i0Out,
size_t &i1Out
)
    {
    if (!m_viewRange.IntersectsWith (viewRange))
        return false;
    DRange2d localRange;
    InitLocalRange (worldXYZ, numXYZ, localRange);
    size_t sortIndexA, sortIndexB;
    if (!OverlapIndices (sortedValues, m_sortIndex0, m_sortIndex0 + m_numPoint - 1, localRange.low.x, localRange.high.x,
                sortIndexA, sortIndexB))
        return false;
    i0Out = m_pointIndex0 + (sortIndexA - m_sortIndex0);
    i1Out = m_pointIndex0 + (sortIndexB - m_sortIndex0);
    return true;    
    }
};


struct Evaluator_IntToDouble {GEOMAPI_VIRTUAL bool Evaluate (size_t i, double &value) = 0;};

struct Evaluator_DPoint3dArrayIndexToLocalX : Evaluator_IntToDouble
{
DPoint3dCP m_points;
size_t m_count;
Transform m_worldToView;
Transform m_worldToLocal;

Evaluator_DPoint3dArrayIndexToLocalX (DPoint3dCP points, size_t n, TransformCR worldToView)
    : m_points(points), m_count (n), m_worldToView (worldToView)
    {
    }

// Compute the "viewed" ray through xyz0 and xyz1.
// Return false if (a) either one is a disconnect, (b) the viewed points are identical.
bool SetLocalCoordinates (DPoint3dCR xyz0, DPoint3dCR xyz1)
    {
    if (xyz0.IsDisconnect() || xyz1.IsDisconnect())
        return false;
    DPoint3d uvw0, uvw1;
    m_worldToView.Multiply(uvw0, xyz0);
    m_worldToView.Multiply(uvw1, xyz1);
    DVec3d uVector, vVector, wVector;
    uvw1.z = uvw0.z;
    double a = uVector.NormalizedDifference (uvw1, uvw0);
    if (a == 0.0)
        return false;
    vVector.UnitPerpendicularXY (uVector);
    wVector.CrossProduct (uVector, vVector);
    Transform localToView, viewToLocal;
    localToView.InitFromOriginAndVectors (uvw0, uVector, vVector, wVector);
    viewToLocal.InvertRigidBodyTransformation(localToView);
    m_worldToLocal.InitProduct (viewToLocal, m_worldToView);
    return true;
    }

bool Evaluate (size_t i, double &value) override
    {
    if (i >= m_count)
        return false;
    value = bsiTransform_multiplyXRow (m_worldToLocal, m_points[i]);
    return true;
    }
};

// Examine points {i0<=i<=iLimit}
// Evaluate function at i0 and beyond.
// Return FINAL index at which function is evaluatable and monotone.
// The evaluator must fail for "some" large index -- else this will be an infinite loop.
static size_t Search_MonotoneFunction (Evaluator_IntToDouble &evaluator, size_t i0)
    {
    size_t i1 = i0;
    double a0, a1;
    if (!evaluator.Evaluate (i1, a0))
        return i0;
    for (;;)
        {
        if (!evaluator.Evaluate (i1 + 1, a1) || a1 < a0)
            return i1;
        a0 = a1;
        i1++;
        }
    }


// Assign blocks of points to individual index ranges.
static void SplitLinestringsToLocalizedRanges (
DRange2dR compositeViewRange,
bvector<LocalizedIndexRange> &localRanges,
bvector<double>&localBreakCoordinates,
TransformCR worldToView,
DPoint3dCP points,
size_t n
)
    {
    compositeViewRange.Init ();
    localRanges.clear ();
    localBreakCoordinates.clear ();
        
    DPoint3d xyz0, xyz1;
    Evaluator_DPoint3dArrayIndexToLocalX evaluator (points, n, worldToView);
    for (size_t pointIndex0 = 0; pointIndex0 + 1 < n;)
        {
        // Points i0 and i1 give a direction and are the first two points of a monotone chain ....
        xyz0 = points[pointIndex0];
        size_t pointIndex1 = pointIndex0 + 1;
        xyz1 = points[pointIndex1];
        if (!evaluator.SetLocalCoordinates (xyz0, xyz1))
            {
            pointIndex0 = pointIndex1;
            continue;
            }
        pointIndex1 = Search_MonotoneFunction (evaluator, pointIndex1);
        size_t numPoint = pointIndex1 - pointIndex0 + 1;

        Transform worldToLocal = evaluator.m_worldToLocal;
        DRange2d viewRange, localRange;
        viewRange.Init ();
        localRange.Init ();
        size_t sortIndex0 = localBreakCoordinates.size ();
        for (size_t i = pointIndex0; i <= pointIndex1; i++)
            {
            DPoint3d uvwView, uvwLocal;
            worldToView.Multiply(uvwView, points[i]);
            worldToLocal.Multiply(uvwLocal, points[i]);
            viewRange.Extend (uvwView.x, uvwView.y);
            compositeViewRange.Extend (uvwView.x, uvwView.y);
            localRange.Extend (uvwLocal.x, uvwLocal.y);
            localBreakCoordinates.push_back (uvwLocal.x);
            }
        double dx = localRange.high.x - localRange.low.x;
        double dy = localRange.high.y - localRange.low.y;
        static double s_rangeFraction = 1.0e-8;
        // The local range might be a straight line  -- spread it a little so range intersectors don't think there's no intersection with anything.
        double ey = s_rangeFraction * dx; 
        if (dy < ey)
            {
            localRange.high.y += ey;
            localRange.low.y  -= ey;
            }
        LocalizedIndexRange localizedIndexRange (pointIndex0, sortIndex0, numPoint, viewRange, worldToLocal, localRange);
        localRanges.push_back (localizedIndexRange);
#ifdef NOISY
        printf (" (Monotone block (pointIndexRange %d %d) (sortIndex0 %d ) (ViewArea %g) (LocalArea %g)\n",
                    pointIndex0, pointIndex1, sortIndex0,
                    viewRange.area (),
                    localRange.area ()
                    );
#endif
        pointIndex0 = pointIndex1;
        }
    }

struct LocalSearchTable
{
bvector<double> m_localBreakCoordinates;
bvector<LocalizedIndexRange> m_localRanges;
Transform m_worldToView;
DRange2d m_linestringsInViewRange;
LocalSearchTable (TransformCR worldToView)
    {
    m_worldToView = worldToView;
    }


// @param worldXYZ facet coordinates.
// @return number of pairs
size_t CollectIndexPairs (DPoint3dCP worldXYZ, size_t numXYZ, bvector<size_t> &indexPairs)
    {
    indexPairs.clear ();
    DRange2d faceRangeInView;
    bsiDRange2d_fromTransformedPoints (faceRangeInView, m_worldToView, worldXYZ, numXYZ);
    if (!faceRangeInView.IntersectsWith (m_linestringsInViewRange))
        return 0;

    size_t i0, i1;
    for (size_t rangeSelect = 0, numRange = m_localRanges.size ();
            rangeSelect < numRange;
            rangeSelect++
        )
        {
        if (m_localRanges[rangeSelect].LocalIndexOverlap (m_localBreakCoordinates, faceRangeInView, worldXYZ, numXYZ, i0, i1))
            {
            indexPairs.push_back (i0);
            indexPairs.push_back (i1);
            }
        }
    return indexPairs.size () / 2;
    }

void LoadLineString (DPoint3dCP points, size_t n)
    {
    SplitLinestringsToLocalizedRanges (m_linestringsInViewRange, m_localRanges, m_localBreakCoordinates, m_worldToView, points, n);
    }
};

struct TransformPair
{
Transform worldToView;
Transform viewToWorld;
};

struct AnnotatedSegment
{
DPoint3d m_pointA;
DPoint3d m_pointB;
size_t   m_index[2];
double  m_parameter[2];
DVec3d  m_normal;

size_t LineIndex () {return m_index[0];}
size_t MeshIndex () {return m_index[1];}

void Init (DPoint3dCR pointA, DPoint3dCR pointB, size_t indexA, size_t indexB, double parameterA, double parameterB, DVec3dCR normal)
    {
    m_pointA = pointA;
    m_pointB = pointB;
    m_index[0] = indexA;
    m_index[1] = indexB;
    m_parameter[0] = parameterA;
    m_parameter[1] = parameterB;
    m_normal = normal;
    }

void Init (DPoint4dCR pointA, DPoint4dCR pointB, size_t indexA, size_t indexB, double parameterA, double parameterB, DVec3dCR facetNormal)
    {
    m_pointA.Init (pointA.x, pointA.y, pointA.z);
    m_pointB.Init (pointB.x, pointB.y, pointB.z);
    m_index[0] = indexA;
    m_index[1] = indexB;
    m_parameter[0] = parameterA;
    m_parameter[1] = parameterB;
    m_normal = facetNormal;
    }

void SetNormal (DVec3dCR normal){m_normal = normal;};
void AdvanceTailToFraction (double f)
    {
    if (f >= m_parameter[1])
        {
        m_parameter[0] = m_parameter[1];
        m_pointA = m_pointB;
        }
    else if (f > m_parameter[0])
        {
        double g;
        DoubleOps::SafeDivide (g, f - m_parameter[0], m_parameter[1] - m_parameter[0], 0.0);
        m_parameter[0] = (1.0 - g) * m_parameter[0] + g * m_parameter[1];
        m_pointA.Interpolate (m_pointA, g, m_pointB);
        }
    }
};
typedef AnnotatedSegment &AnnotatedSegmentR;


static bool cb_sort_compareAnnotatedSegmentBySourceOrder (AnnotatedSegment dataA, AnnotatedSegment dataB)
    {
    if (dataA.m_index[0] < dataB.m_index[0])
        return true;
    if (dataA.m_index[0] > dataB.m_index[0])
        return false;
    if (dataA.m_parameter[0] < dataB.m_parameter[0])
        return true;
    return false;
    }
#ifdef CompileXYLexicalCompare
static bool cb_compare_DPoint2d_XYLexical (DPoint2d dataA, DPoint2d dataB)
    {
    if (dataA.x < dataB.x)
        return true;
    if (dataA.x > dataB.y)
        return false;
    if (dataA.y < dataB.y)
        return true;
    return false;
    }
#endif
static bool cb_sort_compareGPA_a_userData (GraphicsPoint dataA, GraphicsPoint dataB)
    {
    if (dataA.a < dataB.a)
        return true;
    if (dataA.a > dataB.a)
        return false;
    if (dataA.userData < dataB.userData)
        return true;
    if (dataA.userData > dataB.userData)
        return false;
    return true;
    }
//
// Clip a segment to the frustum of a triangle.
// push the clipped part to the AnnotatedSegments
static bool ClipToTriangle
(
DSegment3dCR segment,
DVec3dCR facetNormal,
TransformCR worldToTriangle,
TransformCR triangleToWorld,
size_t sourceIndex,
size_t facetIndex,
AnnotatedSegment &clippedSegment
)
    {
    static double uvwTol = 1.0e-8;
    double myZero = -uvwTol;
//    double myOne  = 1.0 + uvwTol;
    DPoint3d uvw[2];
    double zz[2];
    worldToTriangle.Multiply (uvw, segment.point, 2);
    if (uvw[0].x < myZero && uvw[1].x < myZero)
        return false;
    if (uvw[0].y < myZero && uvw[1].y < myZero)
        return false;
    zz[0] = 1.0 - uvw[0].x - uvw[0].y;
    zz[1] = 1.0 - uvw[1].x - uvw[1].y;
    if (zz[0] < myZero && zz[1] < myZero)
        return false;
    double a = 0.0;
    double b = 1.0;
    if (   ClipToInterval (a, b, uvw[0].x, uvw[1].x)
        && ClipToInterval (a, b, uvw[0].y, uvw[1].y)
        && ClipToInterval (a, b, zz[0], zz[1])
        )
        {
        DSegment3d fullProjection;
        DSegment3d clippedProjection;
        triangleToWorld.Multiply (fullProjection.point[0], uvw[0].x, uvw[0].y, 0.0);
        triangleToWorld.Multiply (fullProjection.point[1], uvw[1].x, uvw[1].y, 0.0);
        clippedProjection.point[0].Interpolate (*(&fullProjection.point[0]), a, *(&fullProjection.point[1]));
        clippedProjection.point[1].Interpolate (*(&fullProjection.point[0]), b, *(&fullProjection.point[1]));
        clippedSegment.Init (
                clippedProjection.point[0], clippedProjection.point[1],
                sourceIndex, facetIndex, a, b, facetNormal);
        return true;
        }
    return false;
    }


//
// Clip a segment to the frusta of multiple triangles.
// push the XOR of the clipped parts clipped parts to the AnnotatedSegments
static size_t PushAfterTriangleClipAndXOR
(
MdlMesh_SweptSegmentCollector collector,
void *userData,
DSegment3dCR segment,
bvector<TransformPair> &triangleTransforms,
bvector<GraphicsPoint> sortArray,
size_t sourceIndex,
size_t facetIndex,
DVec3dCR facetNormal
)
    {
    sortArray.clear ();
    for (size_t i = 0, numTriangle = triangleTransforms.size (); i < numTriangle; i++)
        {
        AnnotatedSegment clippedSegment;
        if (ClipToTriangle (segment, facetNormal,
                    triangleTransforms[i].worldToView,
                    triangleTransforms[i].viewToWorld,
                    sourceIndex, facetIndex,
                    clippedSegment))
            {
            GraphicsPoint gp0, gp1;
            bsiGraphicsPoint_initFromDPoint3d (&gp0, &clippedSegment.m_pointA, 1.0,
                                clippedSegment.m_parameter[0], 0, -1);
            bsiGraphicsPoint_initFromDPoint3d (&gp1, &clippedSegment.m_pointB, 1.0,
                                clippedSegment.m_parameter[1], 0,  1);
            sortArray.push_back (gp0);
            sortArray.push_back (gp1);
            }
        }

    size_t numAccept = 0;
    // Pack out repeated parameters.
    if (sortArray.size () >= 2)
        {
        std::sort (sortArray.begin (), sortArray.end (), cb_sort_compareGPA_a_userData);
        double duplicateParameterTol = 1.0e-10;
        size_t numCandidate = sortArray.size ();
        for (size_t i = 0; i < numCandidate;)
            {
            if (i + 1 < numCandidate && fabs (sortArray[i+1].a - sortArray[i].a) < duplicateParameterTol)
                {
                i += 2;
                }
            else
                {
                sortArray[numAccept] = sortArray[i];
                i++;
                numAccept++;
                }
            }
        }
    // Save intervals
    for (size_t i1 = 1; i1 < numAccept; i1 += 2)
        {
        size_t i0 = i1 - 1;
        DPoint3d pointA, pointB;
        sortArray[i0].point.GetXYZ (pointA);
        sortArray[i1].point.GetXYZ (pointB);
        collector (userData,
                pointA, pointB,
                sourceIndex,
                sortArray[i0].a, sortArray[i1].a,
                facetIndex, facetNormal);
        }
    return numAccept;
    }

static void Use (double a){};

// Build skew transform from a triangle's uv coordinates to world, with the triangle out-of-plane
// direction given as a sweepVector (which is typically NOT perpendicular).
bool BuildTriangleTransforms (TransformR triangleToWorld, TransformR worldToTriangle,
            DPoint3dCR origin, DPoint3dCR pointA, DPoint3dCR pointB, DVec3dCR sweepVector)
    {
    DVec3d vectorA, vectorB;
    vectorA.DifferenceOf (pointA, origin);
    vectorB.DifferenceOf (pointB, origin);
    triangleToWorld.InitFromOriginAndVectors (origin, vectorA, vectorB, sweepVector);
    return worldToTriangle.InverseOf(triangleToWorld) ? true : false;
    }

static void mdlMesh_sweepLinestringToPolyfaceMeshExt_unsorted
(
MdlMesh_SweptSegmentCollector collector,
void *userData,
bvector<DPoint3d> const &linestringPoints,
        DVec3dCR                sweepDirection,
PolyfaceQueryCR mesh
)
    {
    TransformPair viewTransforms;
    
    size_t numLinestringPoints = linestringPoints.size ();
    if (numLinestringPoints < 2)
        return;
    DPoint3d pointA;

    RotMatrix matrix;
    matrix.InitFrom1Vector (sweepDirection, 2, true);
    pointA = linestringPoints[0];
    
    viewTransforms.viewToWorld.InitFrom (matrix, pointA);
    viewTransforms.worldToView.InverseOf(*(&viewTransforms.viewToWorld));

    LocalSearchTable rangeFilter (viewTransforms.worldToView);
    rangeFilter.LoadLineString (&linestringPoints[0], numLinestringPoints);

    
    bvector<GraphicsPoint> sortArray;
    bvector <AnnotatedSegment> sweptSegments;
    bvector<TransformPair> triangleTansforms;

    bvector<size_t> indexPairs;
    DVec3d facetNormal;
    facetNormal.Init (0, 0, 1);
    size_t numFacet = 0;
    size_t numTest  = 0;
    size_t numAccept = 0;
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (mesh, false);
    bvector<DPoint3d> &points = visitor->Point ();
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        uint32_t numThisFace = visitor->NumEdgesThisFace ();
        numFacet++;
        if (numThisFace < 3)
            continue;
        if ( 0 == rangeFilter.CollectIndexPairs (&points[0], (size_t)numThisFace, indexPairs))
            continue;
        // Build in/out intervals for each triangle from base point.
        // The facet is the XOR of the triangles.
        // The sweep image is the XOR of the intervals.
        // Cache the (multiple) sets of convex clip planes (represented as transforms between the world and triangle skew system)
        //    so they can be used repeatedly over all source segments.
        triangleTansforms.clear ();
        TransformPair data;
        for (size_t i2 = 2; i2 < (size_t)numThisFace; i2++)
            if (BuildTriangleTransforms (data.viewToWorld, data.worldToView, points[0], points[i2-1], points[i2], sweepDirection))
                triangleTansforms.push_back (data);
        bsiPolygon_polygonNormalAndArea (&facetNormal, NULL, &points[0], (int)numThisFace);            
        DSegment3d sourceSegment;
        for (size_t pairIndex = 0; pairIndex < indexPairs.size (); pairIndex += 2)
            {
            size_t i0 = indexPairs[pairIndex];
            size_t i1 = indexPairs[pairIndex + 1];
            for (size_t i = i0; i < i1; i++)
                {
                numTest++;
                sourceSegment.point[0] = linestringPoints[i];
                sourceSegment.point[1] = linestringPoints[i + 1];
                size_t edgeCount = PushAfterTriangleClipAndXOR
                        (
                        collector, userData,
                        sourceSegment,
                        triangleTansforms,
                        sortArray,
                        i, visitor->GetReadIndex (), facetNormal
                        );
                if (edgeCount > 0)
                    numAccept++;
                }
            }
        }

    double testFraction = (double)numTest / ((double)numFacet * (double)numLinestringPoints);
    double acceptFraction = (double)numAccept / (numTest > 0 ? (double)numTest : 1.0);
    Use (testFraction);
    Use (acceptFraction);
    }



static void cb_collectAsAnnotatedSegments
(
void *userData,
DPoint3dCR projectedPointA,
DPoint3dCR projectedPointB,
size_t linestringIndexA,
double fractionA,
double fractionB,
size_t facetReadIndex,
DVec3dCR facetNormal
)
    {
    bvector <AnnotatedSegment> *sweptSegments = (bvector <AnnotatedSegment> *)userData;
    AnnotatedSegment segment;
    segment.Init (projectedPointA, projectedPointB, linestringIndexA, facetReadIndex, fractionA, fractionB, facetNormal);
    sweptSegments->push_back (segment);
    }


static void bsiDRange3d_fromArrayWithDisconnects
(
DRange3dR range,
EmbeddedDPoint3dArray const *pXYZArray
)
    {
    size_t n = (size_t)jmdlEmbeddedDPoint3dArray_getCount (pXYZArray);
    range.Init ();
    for (size_t i = 0; i < n; i++)
        {
        DPoint3d pointA;
        jmdlEmbeddedDPoint3dArray_getDPoint3d (pXYZArray, &pointA, (int)i);
        if (!pointA.IsDisconnect())
            range.Extend (pointA);
        }
    }
#ifdef CompileWithCollector
static void  mdlMesh_sweepLinestringToPolyfaceMeshExt
(
MdlMesh_SweptSegmentCollector collector,
void *userData,
bvector <DPoint3d> const &linestringPoints,
        DVec3dCR                sweepDirection,
        PolyfaceQueryCR         mesh,
bool    sortAlongLinestring
)
    {
    if (!sortAlongLinestring)
        {
        mdlMesh_sweepLinestringToPolyfaceMeshExt_unsorted (collector, userData,
                linestringPoints, sweepDirection, mesh);
        return;
        }
    else
        {
        bvector <AnnotatedSegment> sweptSegments;
        mdlMesh_sweepLinestringToPolyfaceMeshExt_unsorted (cb_collectAsAnnotatedSegments, &sweptSegments,
                linestringPoints, sweepDirection, mesh);

        std::sort (sweptSegments.begin (), sweptSegments.end (), cb_sort_compareAnnotatedSegmentBySourceOrder);
        size_t numSegment = sweptSegments.size ();
        for (size_t i = 0; i < numSegment; i++)
            {
            AnnotatedSegment & segment = sweptSegments.at (i);
            collector (userData,
                        segment.m_pointA, segment.m_pointB,
                        segment.m_index[0], segment.m_parameter[0], segment.m_parameter[1],
                        segment.m_index[1], segment.m_normal);
            }
        }
    }
#endif
struct SweepResultsCollector
{
bvector<DPoint3d> &m_xyz;
bvector<int>      &m_indexA;
bvector<int>      &m_indexB;
SweepResultsCollector (bvector<DPoint3d> &xyz, bvector<int>&indexA, bvector <int> &indexB) :
    m_xyz (xyz),
    m_indexA (indexA),
    m_indexB (indexB)
    {
    m_xyz.clear ();
    m_indexA.clear ();
    m_indexB.clear ();
    }
 
 void Add (DPoint3dCR xyz, size_t indexA, size_t indexB)
    {
    m_xyz.push_back (xyz);
    m_indexA.push_back ((int)indexA);
    m_indexB.push_back ((int)indexB);
    }
 
void FixupTail (size_t indexA, size_t indexB)
    {
    if (m_xyz.size () > 0)
        {
        m_indexA.back () = (int)indexA;
        m_indexB.back () = (int)indexB;
        }
    }
};

void  PolyfaceQuery::SweepLinestringToMesh
(
bvector<DPoint3d> &xyzOut,
bvector<int> &linestringIndexOut,
bvector<int> &meshIndexOut,
bvector <DPoint3d> const &linestringPoints,
DVec3dCR  sweepDirection
)
    {
    bvector <AnnotatedSegment> sweptSegments;
    DRange3d linestringRange;
    bsiDRange3d_fromArrayWithDisconnects (linestringRange, &linestringPoints);

    mdlMesh_sweepLinestringToPolyfaceMeshExt_unsorted (cb_collectAsAnnotatedSegments, &sweptSegments,
                linestringPoints, sweepDirection, *this);

    std::sort (sweptSegments.begin (), sweptSegments.end (), cb_sort_compareAnnotatedSegmentBySourceOrder);
    SweepResultsCollector collector (xyzOut, linestringIndexOut, meshIndexOut);
    DVec3d defaultNormal;
    defaultNormal.Init (0, 0, 1);
    if (sweptSegments.size () > 0)
        {
        double tol = 1.0e-10 * linestringRange.LargestCoordinate ();
        DPoint3d disconnect;
        disconnect.InitDisconnect ();
        double endFraction = 0.0;
        DPoint3d pointBPrevious, pointB;
        pointBPrevious.Zero ();
        int priorLineIndex = -1;
        for (size_t i = 0; i < sweptSegments.size (); i++, pointBPrevious = pointB)
            {
            AnnotatedSegment segment = sweptSegments[i];
            if (i > 0  && priorLineIndex == segment.LineIndex ())
                segment.AdvanceTailToFraction (endFraction);
            endFraction = segment.m_parameter[1];
            priorLineIndex = (int)segment.LineIndex ();

            DPoint3d pointA = segment.m_pointA;
            pointB = segment.m_pointB;
            size_t sourceIndex = segment.m_index[0];
            size_t meshIndex   = segment.m_index[1];
            double d = pointA.Distance (pointBPrevious);
            if (i == 0)
                collector.Add (pointA, sourceIndex, meshIndex);
            else if (i > 0 && d <= tol)
                collector.FixupTail (sourceIndex, meshIndex);
            else
                {
                collector.Add (disconnect, -1, -1);
                collector.Add (pointA, sourceIndex, meshIndex);
                }
            collector.Add (pointB, sourceIndex, meshIndex);
            }
        collector.Add (disconnect, -1, -1);
        }
    }
END_BENTLEY_GEOMETRY_NAMESPACE