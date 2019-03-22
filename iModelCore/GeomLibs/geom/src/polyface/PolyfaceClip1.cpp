/*----------------------------------------------------------------------+
|
|     $Source: geom/src/polyface/PolyfaceClip1.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <map>
#include "IndexedClipEdge.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE



/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
struct TaggedClipPoint
{
DPoint3d   m_point;
double     m_altitude;
ClipPlaneP m_tag;

TaggedClipPoint ()
    {
    m_point.x = m_point.y = m_point.z = 0.0;
    m_altitude = 0.0;
    m_tag = 0;
    }
TaggedClipPoint (double x, double y, ClipPlaneP tag)
    {
    m_point.x = x;
    m_point.y = y;
    m_point.z = 0.0;
    m_tag     = tag;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void Interpolate(TaggedClipPoint const &dataA, double fraction, TaggedClipPoint const &dataB, ClipPlaneP tag)
    {
    m_point.Interpolate (dataA.m_point, fraction, dataB.m_point);
    m_tag = tag;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void CalculateAltitudes(bvector<TaggedClipPoint> &points, DPlane3dCR plane, double s)
    {
    size_t n = points.size ();
    for (size_t i = 0; i < n; i++)
        {
        DPoint3d xyz = points[i].m_point;
        points[i].m_altitude = s * plane.Evaluate (xyz);
        }
    }


// Clip a polygon (WITHOUT START/END DUPLICATION) to a plane.
// Negative evaluations (after multiplying by the s parameter) are "in"
// tag clip edges.
// Return clipped polyon in points.
static void ConvexClip
(
bvector<TaggedClipPoint> &points,
bvector<TaggedClipPoint> &work,
DPlane3dCR plane,
double planeSign,
ClipPlaneP tag
)
    {
    size_t n = points.size ();
    TaggedClipPoint clipPoint;
    TaggedClipPoint::CalculateAltitudes (points, plane, planeSign);
    size_t numIn = 0;
    size_t numOut = 0;
    for (size_t i = 0; i < n; i++)
        {
        //DPoint3d xyz = points[i].m_point;
        if (points[i].m_altitude <= 0.0)
            numIn++;
        else
            numOut++;
        }

    if (numIn == 0)
        {
        points.clear ();
        return;
        }
    if (numOut == 0)
        return;

    if (n == 0)
        return;

    work.clear ();
    size_t i1, i0 = n - 1;
    double h1, h0 = points[i0].m_altitude;
    for (i1 = 0; i1 < n; i0 = i1++, h0 = h1)
        {
        //DPoint3d xyz1 = points[i1].m_point;
        h1 = points[i1].m_altitude;
        if (h0 == 0.0)
            {
            if (h1 <= 0.0)
                work.push_back (points[i1]);
            }
        else if (h0 < 0.0)
            {
            if (h1 <= 0.0) // IN to IN
                {
                work.push_back (points[i1]);
                }
            else    // IN to OUT
                {
                double s = -h0 / (h1 - h0);
                clipPoint.Interpolate (points[i0], s, points[i1], tag);
                work.push_back (clipPoint);
                }
            }
        else    // h0 OUT
            {
            if (h1 == 0.0)
                {
                work.push_back (points[i1]);
                }
            if (h1 < 0.0) // OUT to IN
                {
                double s = -h0 / (h1 - h0);
                clipPoint.Interpolate (points[i0], s, points[i1], points[i0].m_tag);
                work.push_back (clipPoint);
                work.push_back (points[i1]);
                }
            else // OUT to OUT
                {
                }
            }
        }
    points.swap (work);
    }
};



/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
struct VuClipClassifier
{
private:
Transform m_worldToLocal, m_localToWorld;
bvector<DPoint3d> m_worldPoints;
bvector<DPoint3d> m_localPoints;
VuSetP m_graph;
bvector<VuP> m_seedNodes;
bvector<VuP> m_searchStack;
static const VuMask m_clipEdge = VU_GRID_EDGE;
static const VuMask m_clipEdgeSingleExterior = VU_RULE_EDGE;
static const VuMask m_clipEdgeMultipleExterior = VU_U_SLICE;

DRange3d m_localPolygonRange;
DRange3d m_localClipRange;
double   m_maxRangeDimension;
double   m_clipFringe;

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void CollectMaskedNodes(bvector<VuP>&nodes, VuMask mask)
    {
    nodes.clear ();
    VU_SET_LOOP (node, m_graph)
        {
        if (vu_getMask (node, mask))
            nodes.push_back (node);
        }
    END_VU_SET_LOOP (node, m_graph)
    }


public:
VuClipClassifier ()
    {
    m_graph = vu_newVuSet (0);
    vu_setUserDataPIsEdgeProperty (m_graph, true);
    m_clipFringe = -1.0;
    m_localClipRange.Init ();
    }

~VuClipClassifier ()
    {
    vu_freeVuSet (m_graph);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
Transform GetLocalToWorld(){ return m_localToWorld;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
Transform GetWorldToLocal(){ return m_worldToLocal;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
VuSetP GetGraph() { return m_graph;}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void WorldToLocal(DPlane3dCR worldPlane, DPlane3dR localPlane)
    {
    localPlane = worldPlane;
    m_worldToLocal.Multiply (localPlane.origin);
    m_worldToLocal.MultiplyMatrixOnly (localPlane.normal);
    }

// Save a polygon to be clipped.
// Return false if unable to set up transformation.
bool InitializeForPolygon (bvector<DPoint3d> &points)
    {
    static double s_borderFraction = 0.1;
    vu_reinitializeVuSet (m_graph);
    m_worldPoints.clear ();
    m_localPoints.clear ();
    if (!PolygonOps::CoordinateFrame (&points, m_localToWorld, m_worldToLocal))
        return false;
    m_worldPoints = points;
    m_localPoints = m_worldPoints;
    DPoint3dOps::Multiply (&m_localPoints, m_worldToLocal);
    m_localPolygonRange = DPoint3dOps::Range (&m_localPoints);
    m_maxRangeDimension = DoubleOps::Max (m_localPolygonRange.high.x - m_localPolygonRange.low.x,
                                          m_localPolygonRange.high.y - m_localPolygonRange.low.y);
    m_clipFringe = s_borderFraction * m_maxRangeDimension;
    m_localClipRange = m_localPolygonRange;
    m_localClipRange.low.x  -= m_clipFringe;
    m_localClipRange.low.y  -= m_clipFringe;
    m_localClipRange.high.x += m_clipFringe;
    m_localClipRange.high.y += m_clipFringe;
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void GetClipee (bvector<TaggedClipPoint>&points, ClipPlaneP tag, bool addClosure)
    {
    points.clear ();
    for (size_t i = 0; i < m_localPoints.size (); i++)
        {
        DPoint3d xyz = m_localPoints[i];
        points.push_back (TaggedClipPoint (xyz.x, xyz.y, tag));
        }
    if (addClosure)
        {
        TaggedClipPoint data = points.front ();
        points.push_back (data);
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void GetClipRectangle(bvector<DPoint3d>&points, bool addClosure)
    {
    points.push_back (DPoint3d::From (m_localClipRange.low.x,  m_localClipRange.low.y));
    points.push_back (DPoint3d::From (m_localClipRange.high.x, m_localClipRange.low.y));
    points.push_back (DPoint3d::From (m_localClipRange.high.x, m_localClipRange.high.y));
    points.push_back (DPoint3d::From (m_localClipRange.low.x,  m_localClipRange.high.y));
    if (addClosure)
        points.push_back (DPoint3d::From (m_localClipRange.low.x,  m_localClipRange.low.y));
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void GetClipRectangle(bvector<TaggedClipPoint>&points, ClipPlaneP tag)
    {
    points.clear ();
    points.push_back (TaggedClipPoint (m_localClipRange.low.x,  m_localClipRange.low.y,  tag));
    points.push_back (TaggedClipPoint (m_localClipRange.high.x, m_localClipRange.low.y,  tag));
    points.push_back (TaggedClipPoint (m_localClipRange.high.x, m_localClipRange.high.y, tag));
    points.push_back (TaggedClipPoint (m_localClipRange.low.x,  m_localClipRange.high.y, tag));
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool WorldPlaneToLocalClipSegment(DPlane3dCR worldPlane, DSegment3dR localSegment)
    {
    if (m_localClipRange.IsNull ())
        return false;

    double s_unitTol = 1.0e-12;
    DPoint3d localOrigin;
    DVec3d   localNormal;
    m_worldToLocal.Multiply (localOrigin, worldPlane.origin);
    // We know the transformation is orthogonal -- just multiply the normal
    m_worldToLocal.MultiplyMatrixOnly (localNormal, worldPlane.normal);
    DVec3d localTangent = DVec3d::FromCrossProduct (localNormal.x, localNormal.y, localNormal.z, 0,0,1);
    DVec3d descentVector = DVec3d::FromCrossProduct (localNormal, localTangent);
    double lambda;
    if (DoubleOps::SafeDivideDistance (lambda, -localOrigin.z, descentVector.z, 0.0))
        {
        DPoint3d inplaneOrigin;
        inplaneOrigin.SumOf (localOrigin, descentVector, lambda);
        DRange1d lineRange = DRange1d::FromLowHigh (-DBL_MAX, DBL_MAX);
        localTangent.Normalize ();
        if (fabs (localTangent.x) > s_unitTol)
            {
            double a = 1.0 / localTangent.x;
            double s0 = (m_localClipRange.low.x  - inplaneOrigin.x) * a;
            double s1 = (m_localClipRange.high.x  - inplaneOrigin.x) * a;
            lineRange.IntersectInPlace (
                    a > 0.0 ? DRange1d (s0, s1) : DRange1d (s1, s0));
            }

        if (fabs (localTangent.x) > s_unitTol)
            {
            double a = 1.0 / localTangent.y;
            double s0 = (m_localClipRange.low.y  - inplaneOrigin.y) * a;
            double s1 = (m_localClipRange.high.y - inplaneOrigin.y) * a;
            lineRange.IntersectInPlace (
                    a > 0.0 ? DRange1d (s0, s1) : DRange1d (s1, s0));
            }

        if (lineRange.Length () > m_clipFringe)
            {
            localSegment.point[0].SumOf (inplaneOrigin, localTangent, lineRange.low);
            localSegment.point[1].SumOf (inplaneOrigin, localTangent, lineRange.high);
            return true;
            }
        }
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void AddPolygonToGraph()
    {
    // Build and fixup the polygon by itself ...
    vu_stackPush (m_graph);
    VuP pLoop = NULL;
    for (DPoint3dR xyz: m_localPoints)
        {
        VuP node0, node1;
        vu_splitEdge (m_graph, pLoop, &node0, &node1, VU_BOUNDARY_EDGE, VU_BOUNDARY_EDGE);
        vu_setDPoint3d (node0, &xyz);
        vu_setDPoint3d (node1, &xyz);
        pLoop = node0;
        }
    vu_mergeOrUnionLoops (m_graph, VUUNION_UNION);
    // The graph started as a polygon -- no question that it is connected !!
    //vu_regularizeGraph (mpGraph);
    //vu_floodFromNegativeAreaFaces (m_graph, VU_BOUNDARY_EDGE, VU_EXTERIOR_EDGE);
    vu_stackPop (m_graph);

    // Hmmm.   NOW we might be disconnected in the larger world with clippers.
    vu_mergeOrUnionLoops (m_graph, VUUNION_UNION);
    vu_regularizeGraph (m_graph);
    vu_parityFloodFromNegativeAreaFaces  (m_graph, VU_BOUNDARY_EDGE, VU_EXTERIOR_EDGE);
    vu_windingFloodFromNegativeAreaFaces (m_graph, m_clipEdgeSingleExterior, m_clipEdgeMultipleExterior);
    }

bvector<TaggedClipPoint> m_clipPointA;
bvector<TaggedClipPoint> m_clipPointB;

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void AddConvexClip (ConvexClipPlaneSetP clipPlanes)
    {
    //GetClipRectangle (m_clipPointA, NULL);
    GetClipee (m_clipPointA, NULL, false);

    vu_stackPush (m_graph);
    for (size_t planeIndex = 0; m_clipPointA.size () > 2 && planeIndex < clipPlanes->size(); planeIndex++)
        {
        ClipPlaneP      currPlaneP = &clipPlanes->at (planeIndex);
        DPlane3d        localPlane;

        WorldToLocal (currPlaneP->GetDPlane3d (), localPlane);
        TaggedClipPoint::ConvexClip (m_clipPointA, m_clipPointB, localPlane, -1.0, currPlaneP);
        }

    if (m_clipPointA.size () > 1)
        {
        VuP pLoop = NULL;
        for(TaggedClipPoint& cp : m_clipPointA)
            {
            VuP node0, node1;
            DPoint3d xyz = cp.m_point;
            vu_splitEdge (m_graph, pLoop, &node0, &node1, m_clipEdge, m_clipEdge | m_clipEdgeSingleExterior);
            vu_setDPoint3d (node0, &xyz);
            vu_setDPoint3d (node1, &xyz);
            vu_setUserDataP (node0, cp.m_tag);
            // vu_setUserDataP (node1, cp.m_tag); // NO NO NO That is outside -- it's the trailing edge
            pLoop = node0;
            }
        // Hm.. we started with a rectiangle, in CCW order.
        // The clip steps reduce the size but preserve order and convexity.
        // SO... the exterior masking should be right.
        }
    
    vu_stackPop (m_graph);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void GetClassifications(VuP node, bool &insidePolygon, bool &insideClip)
    {
    insidePolygon = 0 == vu_getMask (node, VU_EXTERIOR_EDGE);
    insideClip    = 0 == vu_getMask (node, m_clipEdgeMultipleExterior);
    }

// Return an array of all interior faces ...
void CollectInteriorFaces (bvector<VuP> &faceArray)
    {
    faceArray.clear ();
    VuMask visitMask = vu_grabMask (m_graph);
    vu_clearMaskInSet (m_graph, visitMask);
    VU_SET_LOOP (seed, m_graph)
        {
        if (!vu_getMask (seed, visitMask))
            {
            //double area = vu_area (seed);
            vu_setMaskAroundFace (seed, visitMask);
            if (!vu_getMask (seed, VU_EXTERIOR_EDGE))
                faceArray.push_back (seed);
            }
        }
    END_VU_SET_LOOP (seed, m_graph)
    vu_returnMask (m_graph, visitMask);
    }
};






/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
struct PolyfaceVUClipContext
{
private:
PolyfaceVisitor&            m_visitor;
// unused - IPolyfaceVisitorFilterP     m_filter;
PolyfaceCoordinateMapP      m_insideMap;
PolyfaceCoordinateMapP      m_outsideMap;
PolyfaceHeaderP             m_insideMesh;
PolyfaceHeaderP             m_outsideMesh;
ClipEdgeDataArray           m_globalEdgeData;
ClipEdgeDataArray           m_localEdgeData;
bvector<VuP>                m_interiorFaces;
bmap <ClipPlaneCP, int>     m_planeIndexMap;



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t MapInsideIndexToOutsideMesh(size_t insideIndex)
    {
    DPoint3d xyz = m_insideMesh->Point() [insideIndex];
    return m_outsideMap->AddPoint (xyz);
    }
// insideIndices contains zero-based indices in the inside mesh !!
// HMMM.. This means inside mesh is required !!!!  Can't just get outside !!!
void EmitLoop (bvector<size_t> insideIndices, bool reverse = false)
    {
    // chain indices are zero based?
    size_t n = insideIndices.size ();
    if (n < 3)
        return;
    if (NULL != m_insideMesh)
        {
        if (reverse)
            for (size_t i = n; i-- > 0;)
                m_insideMesh->PointIndex ().push_back ((int) (insideIndices[i])+ 1);
        else
            for (size_t i = 0; i < n; i++)
                m_insideMesh->PointIndex ().push_back ((int) (insideIndices[i]) + 1);
        m_insideMesh->PointIndex ().push_back (0);
        }

    if (NULL != m_outsideMesh)
        {
        // Ouside face is reversed from inside ...
        if (!reverse)
            for (size_t i = n; i-- > 0;)
                m_outsideMesh->PointIndex ().push_back ((int) (MapInsideIndexToOutsideMesh (insideIndices[i]) + 1));
        else
            for (size_t i = 0; i < n; i++)
                m_outsideMesh->PointIndex ().push_back ((int) (MapInsideIndexToOutsideMesh (insideIndices[i]) + 1));
        m_outsideMesh->PointIndex ().push_back (0);
        }
    }


void EmitLoop (bvector<DPoint3d> xyz, bool reverse = false)
    {
    // chain indices are zero based?
    size_t n = xyz.size ();
    if (n < 3)
        return;
    if (NULL != m_insideMesh)
        {
        if (reverse)
            for (size_t i = n; i-- > 0;)
                {
                size_t index = m_insideMap->AddPoint (xyz[i]);
                m_insideMesh->PointIndex ().push_back ((int) (index+ 1));
                }
        else
            for (size_t i = 0; i < n; i++)
                {
                size_t index = m_insideMap->AddPoint (xyz[i]);
                m_insideMesh->PointIndex ().push_back ((int) (index+ 1));
                }
        m_insideMesh->PointIndex ().push_back (0);
        }

    if (NULL != m_outsideMesh)
        {
        // Ouside face is reversed from inside ...
        if (!reverse)
            for (size_t i = n; i-- > 0;)
                {
                size_t index = m_outsideMap->AddPoint (xyz[i]);
                m_insideMesh->PointIndex ().push_back ((int) (index+ 1));
                }
        else
            for (size_t i = 0; i < n; i++)
                {
                size_t index = m_outsideMap->AddPoint (xyz[i]);
                m_insideMesh->PointIndex ().push_back ((int) (index+ 1));
                }
        m_outsideMesh->PointIndex ().push_back (0);
        }
    }



// insideIndices contains zero-based indices in the inside mesh !!
// HMMM.. This means inside mesh is required !!!!  Can't just get outside !!!
void AssembleLoop (bvector<size_t> insideIndices, bvector<bvector<DPoint3d>> &insideLoops, bvector<bvector<DPoint3d>> &outsideLoops, bool addClosure = true )
    {
    // chain indices are zero based?
    size_t n = insideIndices.size ();
    if (n < 3)
        return;
    if (NULL != m_insideMesh)
        {
        insideLoops.push_back (bvector<DPoint3d> ());
        for (size_t i = 0; i < n; i++)
            insideLoops.back ().push_back (m_insideMesh->Point() [insideIndices[i]]);
        auto xyz0 = insideLoops.back ().front();
        if (addClosure)
            insideLoops.back ().push_back (xyz0);
        }

    if (NULL != m_outsideMesh)
        {
        outsideLoops.push_back (bvector<DPoint3d> ());
        for (size_t i = 0; i < n; i++)
            outsideLoops.back ().push_back (m_insideMesh->Point() [insideIndices[i]]);
        auto xyz0 = outsideLoops.back ().front();
        if (addClosure)
            outsideLoops.back ().push_back (xyz0);
        }
    }

// Return true if segments are colinear.
// @param [out] longIndex index of longer segment.
// @param [out] shortSegmentFractions fractional positions of short segment ends on long segment.
bool AreSegmentsColinear
(
DSegment3dCR segment0,
DSegment3dCR segment1,
int&          longIndex,
DSegment1dR  shortSegmentFractions
)
    {
    longIndex = 0;
    shortSegmentFractions = DSegment1d (0,0);

    DSegment3d segments[2];
    segments[0] = segment0;
    segments[1] = segment1;
    DVec3d vector[2];
    vector[0] = DVec3d::FromStartEnd (segment0.point[0], segment0.point[1]);
    vector[1] = DVec3d::FromStartEnd (segment1.point[0], segment1.point[1]);
    DPoint3d projectedPoint0, projectedPoint1;
    double   fraction0, fraction1;
    if (vector[0].Magnitude () > vector[1].Magnitude())
        longIndex = 1;
    int shortIndex = 1 - longIndex;
    if (   segments[longIndex].ProjectPoint (projectedPoint0, fraction0, segments[shortIndex].point[0])
        && segments[longIndex].ProjectPoint (projectedPoint1, fraction1, segments[shortIndex].point[1])
        && DPoint3dOps::AlmostEqual (segments[shortIndex].point[0], projectedPoint0)
        && DPoint3dOps::AlmostEqual (segments[shortIndex].point[1], projectedPoint1)
        )
        {
        shortSegmentFractions = DSegment1d(fraction0, fraction1);
        return true;
        }
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool GetSegment(PolyfaceHeaderR mesh, size_t startIndex, size_t endIndex, DSegment3dR segment)
    {
    bvector<DPoint3d> const &points = mesh.Point ();
    size_t n = points.size ();
    if (startIndex < n && endIndex < n)
        {
        segment.point[0] = points[startIndex];
        segment.point[1] = points[endIndex];
        return true;
        }
    return false;
    }

void ExtractCutLoops
(
bvector<bvector<DPoint3d>> *cutLoops,
bvector<bvector<DPoint3d>> *cutChains
)
    {
    auto sorter = bsiMTGFragmentSorter_new ();
    DPoint3dCP points = m_insideMesh->GetPointCP ();
    size_t numPoints = m_insideMesh->GetPointCount ();
    int index = 0;
    for (auto &data : m_globalEdgeData)
        {
        if (data.m_vertexA >= 0 && data.m_vertexB >= 0
            && data.m_vertexA < (int)numPoints && data.m_vertexB < (int)numPoints
            )
            {
            DPoint3d xyzA = points[data.m_vertexA];
            DPoint3d xyzB = points[data.m_vertexB];
            bsiMTGFragmentSorter_addFragment (sorter, index, &xyzA, &xyzB);
            }
        index++;
        }
    bvector<int> loopIndices, chainIndices;
    bsiMTGFragmentSorter_sortAndExtractSignedLoopsAndChains (sorter, &loopIndices, &chainIndices, cutLoops, cutChains);
    bsiMTGFragmentSorter_free (sorter);
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void AnalyzeCutPlaneLoops(size_t numComplexFaces)
    {
    numComplexFaces = 0;
    m_globalEdgeData.SortByPlaneVertexAVertexB ();

    size_t beginIndex = 0;
    size_t endIndex;
    bvector<size_t> startIndices;
    bvector<size_t> endIndices;
    bvector<size_t> chainIndices;
    bool reverseLoops = true;
    for (beginIndex = 0;
            beginIndex < (endIndex = m_globalEdgeData.FindEndOfPlaneCluster (beginIndex));
            beginIndex = endIndex)
        {
        bvector<bvector<DPoint3d>> insideLoops, outsideLoops;
        startIndices.clear ();
        endIndices.clear ();
        if (m_globalEdgeData.AnalyzeCutPlane (beginIndex, endIndex, startIndices, endIndices))
            {
            m_globalEdgeData.SetFlag (beginIndex, endIndex, false);
            // Pull out obvious 
            for (size_t index = beginIndex; index < endIndex; index++)
                {
                if (m_globalEdgeData.GetFlag (index))
                    continue;
                chainIndices.clear ();
                if (startIndices.size () == 1 && endIndices.size () == 1)
                    {
                    // Single path leaves and returns to the cut plane.  The single edge joining the start, return closes the loops.
                    m_globalEdgeData.ExtendChain (beginIndex, endIndex, startIndices[0], chainIndices, true);
                    //EmitLoop (chainIndices, reverseLoops);
                    AssembleLoop (chainIndices, insideLoops, outsideLoops);
                    }
                else if (startIndices.size () == 2 && endIndices.size () == 2)
                    {
                    DSegment3d segments[2];
                    // Two paths leaving and returning.  Decide if this is two separate loops (each with a closure edge)
                    // or one loop that weaves from one path to the other.
                    if (GetSegment (*m_insideMesh, startIndices[0], endIndices[0], segments[0])
                        && GetSegment (*m_insideMesh, startIndices[1], endIndices[1], segments[1]))
                        {
                        int longIndex;
                        DSegment1d shortSegmentFractions (0.0);
                        if (AreSegmentsColinear (segments[0], segments[1], longIndex, shortSegmentFractions))
                            {
                            double f0 = shortSegmentFractions.GetStart ();
                            double f1 = shortSegmentFractions.GetEnd ();
                            if (   f0 < 1.0 && f1 < f0 && 0.0 < f1)
                                {
                                m_globalEdgeData.ExtendChain (beginIndex, endIndex, startIndices[0], chainIndices, true);
                                m_globalEdgeData.ExtendChain (beginIndex, endIndex, startIndices[1], chainIndices, true);
                                //EmitLoop (chainIndices, reverseLoops);
                                AssembleLoop (chainIndices, insideLoops, outsideLoops);

                                }
                            else if (  (f0 > 1.0 && f1 > f0)
                                    || (f1 < 0.0 && f1 > f0)
                                    )
                                {
                                m_globalEdgeData.ExtendChain (beginIndex, endIndex, startIndices[0], chainIndices, true);
                                //EmitLoop (chainIndices, reverseLoops);
                                AssembleLoop (chainIndices, insideLoops, outsideLoops);
                                chainIndices.clear ();
                                m_globalEdgeData.ExtendChain (beginIndex, endIndex, startIndices[1], chainIndices, true);
                                //EmitLoop (chainIndices, reverseLoops);
                                AssembleLoop (chainIndices, insideLoops, outsideLoops);
                                }
                            }
                        else
                            {
                            // Join 2 chains as single face ....
                            m_globalEdgeData.ExtendChain (beginIndex, endIndex, startIndices[0], chainIndices, true);
                            m_globalEdgeData.ExtendChain (beginIndex, endIndex, startIndices[1], chainIndices, true);
                            //EmitLoop (chainIndices, reverseLoops);                            
                            AssembleLoop (chainIndices, insideLoops, outsideLoops);
                            }
                        }
                    }
                else
                    {
                    numComplexFaces++;
                    }
                }

            // Anything left is a complete loop.
            // Holes?
            size_t numLoop = 0;
            for (size_t startIndex = beginIndex; startIndex < endIndex; startIndex++)
                {
                // This is wrong for holeInFace !!!
                if (!m_globalEdgeData.GetFlag (startIndex))
                    {
                    numLoop++;
                    chainIndices.clear ();
                    m_globalEdgeData.ExtendChain (beginIndex, endIndex, startIndex, chainIndices, false);
                    //EmitLoop (chainIndices, reverseLoops);
                    AssembleLoop (chainIndices, insideLoops, outsideLoops);
                    }
                }
            static int s_noisy = 0;
            if (s_noisy)
                {
                GEOMAPI_PRINTF (" AssembleLoopsOnPlane\n");
                for (auto &loop : insideLoops)
                    {
                    GEOMAPI_PRINTF("   [\n");
                    for (auto &xyz : loop)
                        {
                        GEOMAPI_PRINTF("      [%.15lg,%.15lg,%.15lg],\n", xyz.x, xyz.y, xyz.z);
                        }
                    GEOMAPI_PRINTF("   ]\n");
                    }
                }

            if (insideLoops.size () > 0)
                {
                bvector<int> triangleIndices, outerLoopIndices;
                Transform localToWorld, worldToLocal;
                bvector<DPoint3d> xyzOut;
                PolygonOps::FixupAndTriangulateSpaceLoops (triangleIndices, outerLoopIndices, xyzOut, localToWorld, worldToLocal, insideLoops);
                if (s_noisy)
                    {
                    for (size_t i = 0; i < xyzOut.size (); i++)
                        {
                        DPoint3d xyz = xyzOut[i];
                        GEOMAPI_PRINTF("   %d [%.15lg,%.15lg,%.15lg],\n", (int)i, xyz.x, xyz.y, xyz.z);
                        }
                    for (size_t i = 0; i < triangleIndices.size (); i++)
                        {
                        int k = triangleIndices[i];
                        if (i == 0 || k == 0)
                            GEOMAPI_PRINTF ("\n   ");
                        if (k != 0)
                            GEOMAPI_PRINTF (" %d", k);
                        }
                    }
                bvector<DPoint3d> triangleXYZ;
                for (size_t i = 0; i < triangleIndices.size (); i++)
                    {
                    int k = triangleIndices[i];
                    if (k == 0)
                        {
                        EmitLoop (triangleXYZ, reverseLoops);
                        triangleXYZ.clear ();
                        }
                    if (k != 0)
                        triangleXYZ.push_back (xyzOut [(size_t)(abs(k) - 1)]);
                    }
                }
            }
        }
    }



PolyfaceVUClipContext
(
PolyfaceVisitor& visitor,
IPolyfaceVisitorFilter *filter,
PolyfaceCoordinateMapP insideDest, PolyfaceCoordinateMapP outsideDest)
    : m_visitor (visitor),
        /* unused - m_filter (filter),*/
      m_insideMap  (insideDest),
      m_outsideMap (outsideDest)
    {
    m_insideMesh = m_insideMap == NULL ? NULL : &m_insideMap->GetPolyfaceHeaderR ();
    m_outsideMesh = m_outsideMap == NULL ? NULL : &m_outsideMap->GetPolyfaceHeaderR ();
    m_visitor.ClientPointIndex ().SetActive (true);
    m_visitor.ClientNormalIndex ().SetActive (false);
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void AddFace(VuP seed, PolyfaceCoordinateMapP map, PolyfaceHeaderP mesh, VuClipClassifier &m_vuClipper, bool saveCutPlaneEdges)
    {
    if (NULL == map)
        return;
    if (vu_countEdgesAroundFace (seed) < 3)
        return;

    Transform localToWorld = m_vuClipper.GetLocalToWorld();
    m_localEdgeData.clear ();
    VU_FACE_LOOP (node, seed)
        {
        DPoint3d worldPoint, localPoint;;
        vu_getDPoint3d (&localPoint, node);
        localToWorld.Multiply (worldPoint, localPoint);
        size_t meshIndex = map->AddPoint (worldPoint);
        mesh->PointIndex ().push_back ((int)meshIndex + 1);
        int planeIndex = -1;
        ClipPlaneP clipPlaneP = (ClipPlaneP)    vu_getUserDataP (node);
        bmap <ClipPlaneCP, int>::iterator    found;

        if ((found = m_planeIndexMap.find (clipPlaneP)) != m_planeIndexMap.end())
            planeIndex = found->second;

        m_localEdgeData.push_back (ClipEdgeData ((int)meshIndex, (int)meshIndex, planeIndex));
        }
    END_VU_FACE_LOOP (node, seed)
    mesh->PointIndex().push_back (0);

    size_t n = m_localEdgeData.size ();
    if (n > 2 && saveCutPlaneEdges)
        {
        // local edge data only has start vertex id.
        // Pull in tail vertex id from array successor
        for (size_t i1 = 0, i0 = n - 1; i1 < n; i0 = i1++)
            m_localEdgeData[i0].m_vertexB = m_localEdgeData[i1].m_vertexA;
        // Promote edges with plane data to the global array.
        for(ClipEdgeData& data : m_localEdgeData)
            {
            if (data.m_planeIndex != -1)
                {
                m_globalEdgeData.push_back (data);
                }
            }
        }
    }

//! Clip the single face of the visitor to each of the chain of clip plane sets.
//! Add each clipped piece (from each clip plane set) to the growing mapped mesh.
//! The visitor face is unchanged at end.
//! (But the visitor data arrays are expanded and trimmed at intermediate steps.)
void ClipCurrentFaceToChain (ClipPlaneSetP clipPlanes, bool includeVolume = true)
    {
    static bool s_printGraph = false;
    VuClipClassifier m_vuClipper;
    if (m_vuClipper.InitializeForPolygon (m_visitor.Point ()))
        {
        for (ConvexClipPlaneSet& currPlanes: *clipPlanes)
            m_vuClipper.AddConvexClip (&currPlanes);

        if (s_printGraph)
            vu_printFaceLabels (m_vuClipper.GetGraph (), "ConvexClip");
        m_vuClipper.AddPolygonToGraph ();
        if (s_printGraph)
            vu_printFaceLabels (m_vuClipper.GetGraph (), "With Clipped Polygon");
        m_vuClipper.CollectInteriorFaces (m_interiorFaces);
        //size_t numFaces = m_interiorFaces.size ();
        for(VuP face : m_interiorFaces)
            {
            bool insideFacet, insideClip;
            m_vuClipper.GetClassifications (face, insideFacet, insideClip);
            if (insideClip)
                AddFace (face, m_insideMap, m_insideMesh, m_vuClipper, true);
            else
                AddFace (face, m_outsideMap, m_outsideMesh, m_vuClipper, false);
            }
        }
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void SuppressAuxArrays(PolyfaceHeaderP mesh)
    {
    if (NULL != mesh)
        {
        mesh->Normal     ().SetActive (false);
        mesh->Param      ().SetActive (false);
        mesh->IntColor   ().SetActive (false);

        mesh->NormalIndex().SetActive (false);
        mesh->ParamIndex ().SetActive (false);
        mesh->ColorIndex ().SetActive (false);

        mesh->Normal     ().clear ();
        mesh->Param      ().clear ();
        mesh->IntColor   ().clear ();

        mesh->NormalIndex ().clear ();
        mesh->ParamIndex  ().clear ();
        mesh->ColorIndex  ().clear ();
        mesh->SetNumPerFace (0);    // Force indexed data -- override from coordinate.
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void SuppressAuxArrays()
    {
    SuppressAuxArrays (m_insideMesh);
    SuppressAuxArrays (m_outsideMesh);
    }

public:

static void ClipToChain
(
PolyfaceVisitor&        visitor,
IPolyfaceVisitorFilter *filter,
PolyfaceCoordinateMapP  insideDest,
PolyfaceCoordinateMapP  outsideDest,
bool                    resultHasIncompleteCutPlaneFaces,
ClipPlaneSetP           clipPlanes,
bool                    formNewFacesOnClipPlanes,
bvector<bvector<DPoint3d>> *cutLoops,
bvector<bvector<DPoint3d>> *cutChains
)
    {
    PolyfaceVUClipContext context (visitor, filter, insideDest, outsideDest);
    // Fix up (blast!) plane indices..
    int planeCounter = 0;
    for (ConvexClipPlaneSetCR convexSet: *clipPlanes)
        {
        for (ClipPlaneCR plane: convexSet)
            {
            context.m_planeIndexMap[&plane] = planeCounter++;
            }
        }
    for (visitor.Reset ();visitor.AdvanceToNextFace ();)
        {
        if (filter != nullptr && !filter->TestFacet (visitor))
            continue;
        context.ClipCurrentFaceToChain (clipPlanes);
        }

    if (formNewFacesOnClipPlanes)
        context.AnalyzeCutPlaneLoops (resultHasIncompleteCutPlaneFaces);
    if (nullptr != cutLoops && nullptr != cutChains)
        context.ExtractCutLoops (cutLoops, cutChains);
    context.SuppressAuxArrays ();
    }
};

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
//! Visit each face of source. Clip to chain and capture the clipped residual.
GEOMDLLIMPEXP void PolyfaceCoordinateMap::AddClippedPolyface (PolyfaceQueryR source,
    PolyfaceCoordinateMapP insideDest,
    PolyfaceCoordinateMapP outsideDest,
    bool &resultHasIncompleteCutPlaneFaces,
    ClipPlaneSetP clipPlanes,
    bool formNewFacesOnClipPlanes,
    IPolyfaceVisitorFilter *filter,
    bvector<bvector<DPoint3d>> *cutLoops,
    bvector<bvector<DPoint3d>> *cutChains
    )
    {
    resultHasIncompleteCutPlaneFaces = false;
    if (NULL != insideDest)
        {
        insideDest->m_polyface.SetMeshStyle (MESH_ELM_STYLE_INDEXED_FACE_LOOPS);
        insideDest->m_polyface.ActivateVectorsForIndexing (source);
        insideDest->m_polyface.FaceIndex ().SetActive (false);
        }

    if (NULL != outsideDest)
        {
        outsideDest->m_polyface.SetMeshStyle (MESH_ELM_STYLE_INDEXED_FACE_LOOPS);
        outsideDest->m_polyface.ActivateVectorsForIndexing (source);
        outsideDest->m_polyface.FaceIndex ().SetActive (false);
        }

    PolyfaceVisitorPtr visitorPtr = PolyfaceVisitor::Attach (source, true);
    PolyfaceVisitor & visitor = *visitorPtr.get ();

    PolyfaceVUClipContext::ClipToChain (visitor, filter, insideDest, outsideDest,
                resultHasIncompleteCutPlaneFaces,
                clipPlanes, formNewFacesOnClipPlanes, cutLoops, cutChains);
    }

double PolyfaceQuery::BuildConvexClipPlaneSet (ConvexClipPlaneSetR planes)
    {
    auto volume = ValidatedVolume ();
    planes.clear ();
    auto visitor = PolyfaceVisitor::Attach (*this, false);
    double s = 1.0;
    if (volume.IsValid () && volume.Value () < 0.0)
        s = -1.0;
    bvector<DPoint3d> & points = visitor->Point ();
    for (visitor->Reset (); visitor->AdvanceToNextFace ();)
        {
        auto normal = PolygonOps::AreaNormal (points).ValidatedNormalize ();
        if (normal.IsValid ())
            {
            DVec3d signedNormal = -s * normal.Value ();
            ClipPlane plane (signedNormal, points[0]);
            planes.push_back (plane);
            }
        }
    return volume.Value ();
    }

END_BENTLEY_GEOMETRY_NAMESPACE

