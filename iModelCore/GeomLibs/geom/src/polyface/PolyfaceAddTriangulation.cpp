/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/cluster.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

void IPolyfaceConstruction::AddEdgeChains (CurveTopologyId::Type type, uint32_t chainIndex, bvector <DPoint3d> const &points, bool addClosure)
    {
    bvector<size_t> pointIndex;
    bvector<PolyfaceEdgeChain> &chains = GetClientMeshR().EdgeChain ();
    for (size_t i = 0, n = points.size (); i < n; i++)
        {
        DPoint3d xyz = points[i];
        bool doOutput = (i + 1 == n);
        if (xyz.IsDisconnect ())
            doOutput = true;
        else
            {
            size_t newIndex = FindOrAddPoint (xyz);
            if (pointIndex.empty () || pointIndex.back () != newIndex)
                pointIndex.push_back (newIndex);
            }
        if (doOutput)
            {
            chains.push_back (PolyfaceEdgeChain (
                CurveTopologyId (type, chainIndex)));
            if (addClosure)
                pointIndex.push_back (pointIndex.front ());
            chains.back ().AddZeroBasedIndices (pointIndex);
            pointIndex.clear ();
            }
        }
    }

void IPolyfaceConstruction::AddEdgeChainZeroBased (CurveTopologyId::Type type, uint32_t chainIndex, bvector <size_t> const &pointIndices)
    {
    bvector<PolyfaceEdgeChain> &chains = GetClientMeshR().EdgeChain ();
    chains.push_back (PolyfaceEdgeChain (
        CurveTopologyId (type, chainIndex)));
    chains.back ().AddZeroBasedIndices (pointIndices);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool IPolyfaceConstruction::AddTriangulation (bvector <DPoint3d> const &originalInPoints)
    {
    // work with a local duplicate free copy of the points, eliminating trailing disconnects ...
    auto inpoints = originalInPoints;
    while (inpoints.size () > 0 && inpoints.back ().IsDisconnect ())
        inpoints.pop_back ();

    // work with a local duplicate free copy of the points....
    if (inpoints.size () < 3)
        return false;
    bvector<DPoint3d> points;
    points.push_back (inpoints.front ());
    for (size_t i = 1; i < inpoints.size (); i++)
        if (!inpoints[i].AlmostEqual (points.back()))
            points.push_back (inpoints[i]);
    while (points.size () > 1 && points.back ().AlmostEqual (points[0]))
        points.pop_back ();
    
    SynchOptions ();
    bvector <DPoint2d> params;
    Transform worldToLocal, localToWorld;    
    bvector <DPoint3d> outPoints = points;   // Possibly augmented by intersections
    if (!PolygonOps::CoordinateFrame (&outPoints[0], points.size (), localToWorld, worldToLocal, LOCAL_COORDINATE_SCALE_UnitAxesAtLowerLeft))
        return false;

    int     destMaxPerFace = GetFacetOptionsR ().GetMaxPerFace ();
    bool    convexRequired = GetFacetOptionsR ().GetConvexFacetsRequired ();
    bool    smoothTriangulation = GetFacetOptionsR ().GetSmoothTriangleFlowRequired ();
    int numPoints = (int)points.size ();
    bvector <int> loopIndex;


    size_t numFacet = 0;
    int maxPerFace = 3;                     // should be GetFacetOptionsR ().GetMaxPerFace (); convexifier problems
    bool buildSimpleIndices = false;
   
    if (destMaxPerFace >= numPoints  && DPoint3dOps::CountDisconnects (points) == 0)
        buildSimpleIndices = true;
        
    if (buildSimpleIndices && convexRequired && !bsiGeom_testPolygonConvex (&points[0], (int)points.size ()))
        buildSimpleIndices = false;

    double maxEdgeLength = GetFacetOptionsR ().GetMaxEdgeLength ();
    if (maxEdgeLength > 0)
        buildSimpleIndices = false;

    if (maxEdgeLength > 0)
        {
        bvector<int> indicesOut;
        bvector<DPoint3d> xyzOut;
        bvector<DPoint3d> xyPoints;
        worldToLocal.Multiply (xyPoints, points);
        static double s_radiansForRemovingQuadDiagonals = 0.10;
        if (SUCCESS != vu_subtriangulateXYPolygon (&loopIndex, &outPoints, &xyPoints[0], (int)xyPoints.size (),
                    maxEdgeLength, maxEdgeLength, maxEdgeLength, smoothTriangulation, s_radiansForRemovingQuadDiagonals, &xyPoints[0]))
            return false;
        localToWorld.Multiply (outPoints, outPoints);
        }            
    else if (buildSimpleIndices)
        {
        outPoints.clear ();
        size_t n = points.size ();
        // strip off trailing points.
        while (n > 1 && points[0].AlmostEqual (points[n-1]))
            n--;
        // build a single loop over all points (one-based indices with 0 terminator), ignoring repeats
        loopIndex.push_back (1);
        outPoints.push_back (points[0]);
        for (size_t i = 1, n = points.size (); i < n; i++)
            {
            if (!points[i].AlmostEqual (outPoints.back ()))
                {
                outPoints.push_back (points[i]);
                loopIndex.push_back ((int)outPoints.size ());   // 1 based index !!!
                }
            }
        if (outPoints.back ().AlmostEqual (outPoints.front ()))
            {
            outPoints.pop_back ();
            loopIndex.pop_back ();
            }
        loopIndex.push_back (0);
        }
    else if (SUCCESS != vu_triangulateProjectedPolygon (&loopIndex, NULL, &outPoints, &localToWorld, &worldToLocal,  &points[0], (int)points.size (), 0.0, true, maxPerFace))
        {
        return false;
        }

    // Remap indices into polyface ...
    DVec3d polygonNormal;
    bvector <size_t> outPointToPolyfacePoint;
    bvector <size_t> outPointToPolyfaceParam;
    FindOrAddPoints (outPoints, outPoints.size (), 0, outPointToPolyfacePoint);
    localToWorld.GetMatrixColumn (polygonNormal, 2);
    // EDL April 10 2018 -- points arrive with a disconnect.  It goes into the params.  But maybe it's never referenced.  When did this appear?  dgnjs regression says it's newish.
    static int s_disconnectHandling = 0;    // 1==> include the disconnect directly (old behavior). 2==>skip (this may cause index errors), other==>insert a local 000
    if (NeedParams())
        {
        for (size_t i = 0, n = outPoints.size (); i < n; i++)
            {
            DPoint3d workPoint = outPoints[i];
            if (workPoint.IsDisconnect ())
                {
                if (s_disconnectHandling == 1)
                    {
                    worldToLocal.Multiply (workPoint);
                    }
                else if (s_disconnectHandling == 2)
                    {
                    continue;
                    }
                else
                    // this ensures there is a point and the indexing does not change.
                    workPoint.Zero ();
                }
            else
                worldToLocal.Multiply (workPoint);

            DPoint2d param;
            param.x = workPoint.x;
            param.y = workPoint.y;
            params.push_back (param);
            }
        DRange2d distanceRange, paramRange;
        RemapPseudoDistanceParams (params, distanceRange, paramRange, 1.0, 1.0);
        SetCurrentFaceParamDistanceRange (distanceRange);

        for (size_t i = 0; i < params.size (); i++)
            outPointToPolyfaceParam.push_back (FindOrAddParam (params[i]));
        }

    for (size_t i0 = 0, i1 = 0, n = loopIndex.size (); i1 < n; i1++)
        {
        if (loopIndex[i1] == 0)
            {
            size_t numThisFace = i1 - i0;
            if (numThisFace == 3)
                {
                size_t  k0 = abs (loopIndex[i0]) - 1;
                size_t  k1 = abs (loopIndex[i0+1]) - 1;
                size_t  k2 = abs (loopIndex[i0+2]) - 1;
                bool    visible0 = loopIndex[i0] > 0;
                bool    visible1 = loopIndex[i0+1] > 0;
                bool    visible2 = loopIndex[i0+2] > 0;
                AddPointIndexTriangle (
                            outPointToPolyfacePoint[k0], visible0,
                            outPointToPolyfacePoint[k1], visible1,
                            outPointToPolyfacePoint[k2], visible2);
                // Well, yes, we expect the normals to be the same, but compute them all separately just in case the original polygon was non-planar.
                if (NeedNormals ())
                    {
                    DVec3d normal;
                    normal.CrossProductToPoints (outPoints[k0], outPoints[k1], outPoints[k2]);
                    normal.Normalize ();
                    size_t normalIndex = FindOrAddNormal (normal);
                    AddNormalIndexTriangle (normalIndex, normalIndex, normalIndex);
                    }
                if (NeedParams ())
                    AddParamIndexTriangle (
                            outPointToPolyfaceParam[k0],
                            outPointToPolyfaceParam[k1],
                            outPointToPolyfaceParam[k2]);
                numFacet++;
                }
            else if (numThisFace == 4)
                {
                size_t  k0 = abs (loopIndex[i0]) - 1;
                size_t  k1 = abs (loopIndex[i0+1]) - 1;
                size_t  k2 = abs (loopIndex[i0+2]) - 1;
                size_t  k3 = abs (loopIndex[i0+3]) - 1;
                
                bool    visible0 = loopIndex[i0] > 0;
                bool    visible1 = loopIndex[i0+1] > 0;
                bool    visible2 = loopIndex[i0+2] > 0;
                bool    visible3 = loopIndex[i0+3] > 0;

                AddPointIndexQuad (
                            outPointToPolyfacePoint[k0], visible0,
                            outPointToPolyfacePoint[k1], visible1,
                            outPointToPolyfacePoint[k2], visible2,
                            outPointToPolyfacePoint[k3], visible3);
                // Well, yes, we expect the normals to be the same, but compute them all separately just in case the original polygon was non-planar.
                if (NeedNormals ())
                    {
                    DVec3d normal;
                    normal.CrossProductToPoints (outPoints[k0], outPoints[k1], outPoints[k2]);
                    normal.Normalize ();
                    size_t normalIndex = FindOrAddNormal (normal);
                    AddNormalIndexQuad (normalIndex, normalIndex, normalIndex, normalIndex);
                    }
                if (NeedParams ())
                    AddParamIndexQuad (
                            outPointToPolyfaceParam[k0],
                            outPointToPolyfaceParam[k1],
                            outPointToPolyfaceParam[k2],
                            outPointToPolyfaceParam[k3]);
                numFacet++;
                }
            else if (numThisFace > 4)
                {
                size_t iStart = i0;
                size_t iEnd   = i1;
                size_t upStep = 1;
                size_t downStep = 0;
                if (GetReverseNewFacetIndexOrder ())
                    {
                    iStart = i1;
                    iEnd   = i0;
                    upStep = 0;
                    downStep = 1;
                    }
                for (size_t i = iStart; i != iEnd; i += upStep)
                    {
                    i -= downStep;
                    size_t k = abs (loopIndex[i]) - 1;
                    bool   visible = loopIndex[i] > 0;
                    AddPointIndex (outPointToPolyfacePoint[k], visible);
                    }
                AddPointIndexTerminator ();

                if (NeedNormals ())
                    {
                    // ugh. Use the overall z vector as normal.  What if it's non planar.
                    size_t normalIndex = FindOrAddNormal (polygonNormal);
                    for (size_t i = iStart; i != iEnd; i += upStep)
                        {
                        i -= downStep;
                        AddNormalIndex  (normalIndex);
                        }
                    AddNormalIndexTerminator ();
                    }

                if (NeedParams ())
                    {
                    for (size_t i = iStart; i != iEnd; i += upStep)
                        {
                        i -= downStep;
                        size_t k = abs (loopIndex[i]) - 1;
                        AddParamIndex (outPointToPolyfaceParam[k]);
                        }
                    AddParamIndexTerminator ();
                    }
                }
            i0 = i1 + 1;
            }
        }
    EndFace ();
    if (GetFacetOptionsR ().GetEdgeChainsRequired ())
        AddEdgeChains (CurveTopologyId::Type::TriangulationBoundary , 0, inpoints, true);
    return numFacet > 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Xiaoyong.Yang                   03/2007
+---------------+---------------+---------------+---------------+---------------+------*/

static void AddEdge (VuSetP graph, DPoint3d xyzA, DPoint3d xyzB,
    VuMask mask = VU_BOUNDARY_EDGE,
    size_t numInteriorPoint = 0)
    {
    VuP nodeA, nodeB;
    vu_makePair (graph, &nodeA, &nodeB);
    vu_setMask (nodeA, mask);
    vu_setMask (nodeB, mask);
    vu_setDPoint3d (nodeA, &xyzA);
    vu_setDPoint3d (nodeB, &xyzB);
    for (size_t i = 0; i < numInteriorPoint; i++)
        {
        double f = (double)i / (double) numInteriorPoint;
        VuP node0, node1;
        vu_splitEdge (graph, nodeB, &node0, &node1);
        DPoint3d xyz;
        xyz.Interpolate (xyzA, f, xyzB);
        vu_setDPoint3d (node0, &xyz);
        vu_setDPoint3d (node1, &xyz);
        }
    }


static void GetSplitCounts
(
DRange3dCR range,
size_t numPoint,
size_t &numX,
size_t &numY
)
    {
    int         totalAround = (int)sqrt ((double)numPoint);

    static int  s_maxAround = 40;
    //static int  s_maxOut = 100;
    //static int  s_split = 15;   // bit mask, 4 bits on.

    if (totalAround > s_maxAround)
        totalAround = s_maxAround;

    double dx = range.high.x - range.low.x;
    double dy = range.high.y - range.low.y;

    double d = dx + dy;
    numX = (size_t)(totalAround * dx / d);
    numY = (size_t)(totalAround * dy / d);
    }

static void AddExpandedRangeEdges (VuSetP graph, DRange3d range, double fraction, size_t numPoints, double minRatio = 0.25)
    {
    double xFringe = fraction * (range.high.x - range.low.x);
    double yFringe = fraction * (range.high.y - range.low.y);
    if (xFringe < minRatio * yFringe)
        xFringe = minRatio * yFringe;
    if (yFringe < minRatio * xFringe)
        yFringe = minRatio * xFringe;
    double x0 = range.low.x - xFringe;
    double x1 = range.high.x + xFringe;
    double y0 = range.low.y - yFringe;
    double y1 = range.high.y + yFringe;
    double z  = range.low.z;

    size_t numX, numY;
    GetSplitCounts (range, numPoints, numX, numY);
    DPoint3d xyz[5] = 
        {
        {x0, y0, z},
        {x1, y0, z},
        {x1, y1, z},
        {x0, y1, z},
        {x0, y0, z},
        };

    size_t splitCount[4] = {numX, numY, numX, numY};
    for (int i = 0; i < 4; i++)
        {
        size_t numInteriorPoints = splitCount[i];
        if (numInteriorPoints > 0)
            numInteriorPoints--;  // to match old behavior?
        AddEdge (graph, xyz[i], xyz[i+1], VU_BOUNDARY_EDGE, numInteriorPoints);
        }
    }
// Look at userDataP
// If already 0 or positive, return it (it is a zero-based index into the vector)
// Otherwise
//   get xyz from the node.
//   save it in the point vector -- the index where saved is returned.
//   save the index as userDataP around the vertex.
static int FindOrAddPoint (VuSetP graph, VuP node, bvector<DPoint3d> &points)
    {
    int index = vu_getUserDataPAsInt (node);
    if (index < 0)
        {
        DPoint3d xyz;
        vu_getDPoint3d (&xyz, node);
        index = (int)points.size ();
        points.push_back (xyz);
        VU_VERTEX_LOOP (sector, node)
            {
            vu_setUserDataPAsInt (sector, index);
            }
        END_VU_VERTEX_LOOP (sector, node)
        }
    return index;        
    }
// Use UserDataPAsInt for vertex numbering.    
PolyfaceHeaderPtr vu_toPolyface (VuSetP graph, VuMask faceExclusionMask)
    {
    PolyfaceHeaderPtr polyface = PolyfaceHeader::CreateVariableSizeIndexed ();
    bvector<DPoint3d> &points = polyface->Point ();
    bvector<int> &pointIndex = polyface->PointIndex ();
    VuMask visitMask = vu_grabMask (graph);
    VU_SET_LOOP (node, graph)
        {
        vu_setUserDataPAsInt (node, -1);
        vu_clrMask (node, visitMask);
        }
    END_VU_SET_LOOP (node, graph)
    
    VuMask skipMask = faceExclusionMask | visitMask;
    
    VU_SET_LOOP (faceSeed, graph)
        {
        if (!vu_getMask (faceSeed, skipMask))
            {
            vu_setMaskAroundFace (faceSeed, visitMask);
            if (vu_countEdgesAroundFace (faceSeed) >= 3)
                {
                VU_FACE_LOOP (sector, faceSeed)
                    {
                    int index = FindOrAddPoint (graph, sector, points);
                    pointIndex.push_back (index + 1);   // all visible !!!
                    }
                END_VU_FACE_LOOP (sector, faceSeed)
                pointIndex.push_back (0);   // terminate face.
                }
            }
        }
    END_VU_SET_LOOP (faceSeed, graph)    
    
    vu_returnMask (graph, visitMask);
    return polyface;
    }    
static double   s_graphAbsTol = 0.0;
static double   s_graphRelTol = 1.0e-9;

//! Create a triangulation of points as viewed in xy.  Add the triangles to the polyface.
//! (Other than sharing vertices with matched xy, this does not coordinate in any way with prior mesh contents.)
//! @param [in] points candidate points
//! @param [in] fringeExpansionFactor fractional factor (usually 0.10 to 0.20) for defining a surrounding rectangle.  The z of this triangle is
//!     at the low z of all the points.
//! @param [in] retainFringeTriangles true to keep the fringe triangles.  If false, any edge that reaches the outer rectangle is deleted.
PolyfaceHeaderPtr PolyfaceHeader::CreateXYTriangulation (bvector <DPoint3d> const &points, double fringeExpansionFactor, bool retainFringeTriangles, bool convexHull)
    {
    //static int          s_noisy = 0;

    if (points.size () < 3)
        return NULL;
    double s_relTol = 1.0e-9;
    
    if (fringeExpansionFactor < 0.01)
        fringeExpansionFactor = 0.01;
    size_t numPoint = points.size ();
    bvector<DPoint3d> localPointA;
    bvector<DPoint3d> localPointB;
    bvector<DPoint3d> hullPoints;
    DRange3d worldRange = DRange3d::From (points);
    DPoint3d localOrigin = DPoint3d::FromInterpolate (worldRange.low, 0.5, worldRange.high);
    for (size_t i = 0; i < numPoint; i++)
        {
        DVec3d uvw = DVec3d::FromStartEnd (localOrigin, points[i]);
        localPointA.push_back (uvw);
        }

    double tolerance = s_relTol * worldRange.low.DistanceXY (worldRange.high);    

    bvector<int>clusterIndices;
    bsiDPoint3dArray_findClusters (&localPointA[0], localPointA.size (), clusterIndices, &localPointB, tolerance, false, false);

    if (localPointB.size () < 3)
        return NULL;


        
    VuSetP graph = vu_newVuSet (0);
    vu_setTol (graph, s_graphAbsTol, s_graphRelTol);
    
    static double s_expansionFraction = 0.10;
    DRange3d localRange = worldRange;
    localRange.low.Subtract (localOrigin);
    localRange.high.Subtract (localOrigin);
    if (convexHull)
        {
        int numHullPoints;
        hullPoints.resize ( localPointB.size () + 1);
        bsiDPoint3dArray_convexHullXY (&hullPoints.front(), &numHullPoints, &localPointB.front (), (int)localPointB.size ());
        hullPoints.resize ((size_t)numHullPoints);
        for (size_t i = 0; i < hullPoints.size (); i++)
            {
            AddEdge (graph, hullPoints[i], hullPoints[(i+1) % numHullPoints], VU_RULE_EDGE);
            }
        }
    
    AddExpandedRangeEdges (graph, localRange, s_expansionFraction, numPoint);
    vu_mergeOrUnionLoops (graph, VUUNION_UNION);
    vu_regularizeGraph (graph);     // It's just a rectangle -- should do nothing?
    vu_splitMonotoneFacesToEdgeLimit (graph, 3);
    vu_parityFloodFromNegativeAreaFaces (graph, VU_BOUNDARY_EDGE, VU_EXTERIOR_EDGE);

    vu_flipTrianglesToImproveQuadraticAspectRatio (graph);
    vu_insertAndRetriangulate (graph, &localPointB[0], (int)localPointB.size (), false);
    
    if (!retainFringeTriangles)
        vu_spreadExteriorMasksToAdjacentFaces (graph, true, VU_EXTERIOR_EDGE, VU_EXTERIOR_EDGE);

    PolyfaceHeaderPtr polyface = vu_toPolyface (graph, VU_EXTERIOR_EDGE);
    bvector<DPoint3d> &outputPoints = polyface->Point ();
    for (size_t i = 0; i < outputPoints.size (); i++)
        {
        outputPoints[i].Add (localOrigin);
        }
    vu_freeVuSet (graph);
    return polyface;
    }


//! Create a triangulation of regions as viewed in xy
PolyfaceHeaderPtr PolyfaceHeader::CreateConstrainedTriangulation
(
bvector<bvector <DPoint3d>> const &loops,
bvector<bvector< DPoint3d>> const *paths,
bvector<DPoint3d> const *isolatedPoints  
)
    {
       
    VuSetP graph = vu_newVuSet (0);
    vu_setTol (graph, s_graphAbsTol, s_graphRelTol);
    DRange3d worldRange = DRange3d::From (loops);
    double localAbsTol = 1.0e-8;
    auto localRange = DRange3d::From (-1,-1,-1,1,1,1);
    BentleyApi::Transform localToWorld, worldToLocal;
    
    if (!Transform::TryUniformScaleXYRangeFit (worldRange, localRange, worldToLocal, localToWorld))
        return nullptr;

    vu_addEdgesXYTol (graph, &worldToLocal, loops, true, localAbsTol, VU_BOUNDARY_EDGE, VU_BOUNDARY_EDGE);
    if (nullptr != paths)
        vu_addEdgesXYTol (graph, &worldToLocal, *paths, false, localAbsTol, VU_RULE_EDGE, VU_RULE_EDGE);


    vu_mergeOrUnionLoops (graph, VUUNION_UNION);
    vu_regularizeGraph (graph);
    vu_parityFloodFromNegativeAreaFaces (graph, VU_BOUNDARY_EDGE, VU_EXTERIOR_EDGE);
    vu_splitMonotoneFacesToEdgeLimit (graph, 3);
    vu_flipTrianglesToImproveQuadraticAspectRatio (graph);
    
    if (nullptr != isolatedPoints)
        {
        bvector<DPoint3d> localPoints;
        worldToLocal.Multiply (localPoints, *isolatedPoints);
        vu_insertAndRetriangulate (graph, &localPoints[0], (int)localPoints.size (), false);
        }
    vu_parityFloodFromNegativeAreaFaces(graph, VU_BOUNDARY_EDGE, VU_EXTERIOR_EDGE);

    PolyfaceHeaderPtr polyface = vu_toPolyface (graph, VU_EXTERIOR_EDGE);
    bvector<DPoint3d> &outputPoints = polyface->Point ();
    for (size_t i = 0; i < outputPoints.size (); i++)
        {
        localToWorld.Multiply (outputPoints[i]);
        }
    vu_freeVuSet (graph);
    return polyface;
    }

PolyfaceHeaderPtr PolyfaceHeader::CreateConstrainedTriangulation
(
CurveVectorCR loops,
CurveVectorCP paths,
bvector<DPoint3d> const *isolatedPoints,
IFacetOptionsP strokeOptions
)
    {
    bvector<bvector<DPoint3d>> loopPoints, pathPoints;
    loops.CollectLinearGeometry (loopPoints, strokeOptions);
    if (nullptr != paths)
        paths->CollectLinearGeometry (pathPoints, strokeOptions);
    return CreateConstrainedTriangulation (loopPoints, &pathPoints, isolatedPoints);
    }


//! Create a triangulation of points.
//! 
static VuSetP CreateDelauney
(
bvector<DPoint3d> const points
)
    {
    VuSetP graph = vu_newVuSet (0);
    DRange3d worldRange = DRange3d::From (points);
    double localAbsTol = 1.0e-8;
    auto localRange = DRange3d::From (-1,-1,-1,1,1,1);
    BentleyApi::Transform localToWorld, worldToLocal;

    if (!Transform::TryUniformScaleXYRangeFit (worldRange, localRange, worldToLocal, localToWorld))
        return nullptr;

    bvector<DPoint3d> localPoints;
    worldToLocal.Multiply (localPoints, points);

    static int s_boundaryMode = 0;
    static double s_primaryExpansionFactor = 2.0;
    static double s_relativeFactor = 0.5;
    if (s_boundaryMode == 0)
        {
        // Start with outer rectangle triangulation.
        AddExpandedRangeEdges (graph, localRange, s_primaryExpansionFactor, localPoints.size (), s_relativeFactor);
        }
    else
        {
        // Trivial triangulation of the convex hull.
        bvector<DPoint3d> xyzHull (points.size () + 2);
        int numOut;
        bsiDPoint3dArray_convexHullXY (&xyzHull[0], &numOut, (DPoint3d*)&localPoints[0], (int)localPoints.size ());    
        xyzHull.resize (numOut);
        vu_addEdgesXYTol (graph, nullptr, xyzHull, true, localAbsTol, VU_BOUNDARY_EDGE, VU_BOUNDARY_EDGE);
        }

    vu_mergeOrUnionLoops (graph, VUUNION_UNION);
    vu_regularizeGraph (graph);
    vu_parityFloodFromNegativeAreaFaces (graph, VU_BOUNDARY_EDGE, VU_EXTERIOR_EDGE);
    vu_splitMonotoneFacesToEdgeLimit (graph, 3);
    // final flip for true delauney condition . . .
    vu_flipTrianglesForIncircle (graph);
    
    vu_insertAndRetriangulate (graph, &localPoints[0], (int)localPoints.size (), false);
    // this should not be needed ... but retriangulate seems wrong..
    vu_flipTrianglesForIncircle (graph);

    if (s_boundaryMode == 0)
        vu_spreadExteriorMasksToAdjacentFaces (graph, true, VU_EXTERIOR_EDGE, VU_EXTERIOR_EDGE);

    vu_transform (graph, &localToWorld);
    return graph;
    }


// nodeA, nodeB, nodeC are 3 nodes in an FSucc chain.
// nodeD is (if needed) nodeA->FPred.
// if this is a add its circumcenter to the points.
// if there is a right turn (or straight) output a point to the left of the midpoint of nodeA..nodeB and then a point to the right of nodeA..nodeD
static void AppendPseudoCenters (VuP nodeA, bvector<DPoint3d> &points)
    {
    static double s_exteriorFactor = 2.0;
    auto nodeB = nodeA->FSucc ();
    auto nodeC = nodeB->FSucc ();
    auto xyzA = nodeA->GetXYZ ();
    auto xyzB = nodeB->GetXYZ ();
    auto xyzC = nodeC->GetXYZ ();
    DVec3d vectorAB, vectorBC;
    vectorAB = xyzB - xyzA;
    vectorBC = xyzC - xyzB;
    static double s_exteriorAngleTol = 1.0e-8;
    double theta = vectorAB.AngleToXY (vectorBC);   // This is signed, goes 0 at straight line, negative for turn to right.
    if (theta > s_exteriorAngleTol)
        {
        auto voronoiPoint = DPoint3d::FromIntersectPerpendicularsXY (xyzB, xyzA, 0.5, xyzC, 0.5);
        if (voronoiPoint.IsValid ())
            points.push_back (voronoiPoint.Value ());
        }
    else
        {
        //  we appear to be on the outside ...
        // output a point some distance away on this edge's left bisector, and perhaps likewise at the FPred edge ....
        points.push_back (DPoint3d::FromInterpolateAndPerpendicularXY (xyzA, 0.5, xyzB, s_exteriorFactor));
        auto xyzD = nodeA->FPred ()->GetXYZ ();
        if (xyzD.CrossProductToPointsXY (xyzA, xyzB) <= 0.0)
            points.push_back (DPoint3d::FromInterpolateAndPerpendicularXY (xyzD, 0.5, xyzA, s_exteriorFactor));
        }
    }

static PolyfaceHeaderPtr CreateVoronoi (VuSetP graph)
    {
    _VuSet::TempMask visitMask (graph);
    auto facets = PolyfaceHeader::CreateVariableSizeIndexed ();
    VU_SET_LOOP (vertexSeed, graph)
        {
        if (!visitMask.IsSetAtNode (vertexSeed))
            {
            vertexSeed->SetMaskAroundVertex (visitMask.Mask ());
            bvector<DPoint3d> points;
            VU_VERTEX_LOOP (sector, vertexSeed)
                {
                AppendPseudoCenters (sector, points);
                }
            END_VU_VERTEX_LOOP (sector, vertexSeed)
            DPoint3dOps::Compress (points, DoubleOps::SmallMetricDistance ());
            facets->AddPolygon (points);
            }
        }
    END_VU_SET_LOOP (vertexSeed, graph)
    facets->Compress ();
    return facets;
    }

static PolyfaceHeaderPtr CreateTwoPointVoronoi (DPoint3dCR point0, double radius0, DPoint3dCR point1, double radius1, int voronoiMetric, double sideFactor, double backFactor)
    {
        double a = sideFactor;    // make voronoi region this far out . ..
        double b = backFactor;
        auto splitPlane = DPlane3d::VoronoiSplitPlane (point0, radius0, point1, radius1, voronoiMetric);
        double f = 0.5;
        if (splitPlane.IsValid ())
            {
            double height0 = splitPlane.Value ().Evaluate (point0);
            double height1 = splitPlane.Value ().Evaluate (point1);
            auto f1 = DoubleOps::InverseInterpolate (height0, 0.0, height1);
            if (f1.IsValid ())
                f = DoubleOps::ClampFraction(f1.Value ());
            }
        bvector<DPoint3d> points {
        DPoint3d::FromInterpolateAndPerpendicularXY (point0, f, point1, -a),
        DPoint3d::FromInterpolateAndPerpendicularXY (point0, f, point1,  a),
        DPoint3d::FromInterpolateAndPerpendicularXY (point0, f - b, point1, -a),
        DPoint3d::FromInterpolateAndPerpendicularXY (point0, f - b, point1,  a),
        DPoint3d::FromInterpolateAndPerpendicularXY (point0, f + b, point1, -a),
        DPoint3d::FromInterpolateAndPerpendicularXY (point0, f + b, point1,  a)
        };
        //     3----------------1-----------------5
        //     |                |                 |
        //     |                |                 |
        //     |             P0 | P1              |
        //     |                |                 |
        //     |                |                 |
        //     2----------------0-----------------4
        int q = 1;  // one-based
        bvector<int> index1 {
            q + 3, q + 2, q + 0, q + 1, 0,
            q + 1, q + 0, q + 4, q + 5, 0
        };
        return PolyfaceHeader::CreateIndexedMesh (0, points, index1);
    }
bool PolyfaceHeader::CreateDelauneyTriangulationAndVoronoiRegionsXY (bvector<DPoint3d> const &points, PolyfaceHeaderPtr &delauney, PolyfaceHeaderPtr &voronoi)
    {
    if (points.size () == 2)
        {
        delauney = nullptr;
        voronoi = CreateTwoPointVoronoi (points[0], 1.0, points[1], 1.0, 0, 20.0, 20.0);
        return true;
        }
    VuSetP graph = CreateDelauney (points);
    if (graph != nullptr)
        {
        delauney = vu_toPolyface (graph, VU_EXTERIOR_EDGE);
        voronoi = CreateVoronoi (graph);
        vu_freeVuSet (graph);
        return true;
        }
    return false;
    }


// make userData1 = point index for vertices that match points in an array
void InstallPointIndices (VuSetP graph, bvector<DPoint3d> const &points)
    {
    VuPositionDetail searcher;
    VU_SET_LOOP (node, graph)
        {
        node->SetUserData1 (-1);
        }
    END_VU_SET_LOOP (node, graph)
    // fix back indices.
    for (size_t i = 0; i < points.size (); i++)
        {
        searcher = vu_moveTo (graph, searcher, points[i]);
        if (searcher.IsVertex ())
            {
            auto seedNode = searcher.GetNode ();
            VU_VERTEX_LOOP (node, seedNode)
                {
                node->SetUserData1 (i);
                }
            END_VU_VERTEX_LOOP (node, seedNode)
            }
        }
    }
bool check (int numBoundary, int numTotal, ptrdiff_t userData1)
    {
    if (numBoundary < numTotal && userData1 >= 0)
        return true;
    return false;
    }
PolyfaceHeaderPtr CreateVoronoi
(
VuSetP graph,
bvector<DPoint3d> const &points,
bvector<double> const &radii,
int voronoiMetric,
bvector<NeighborIndices> *cellData = nullptr,  //!< [out] optional array giving [siteIndex==pointIndex, auxIndex==facetRead, neighborIndex==array of indices of adjacent cells within cellData array]
VuMask exteriorMask = VU_EXTERIOR_EDGE
)
    {
    PolyfaceHeaderPtr voronoi = PolyfaceHeader::CreateVariableSizeIndexed ();
    InstallPointIndices (graph, points);
    _VuSet::TempMask visited (graph);
    ConvexClipPlaneSet planes;
    DRange3d range = graph->Range ();
    static double s_rangeExpansionFactor = 1.0;
    double dx = range.XLength () * s_rangeExpansionFactor;
    double dy = range.YLength () * s_rangeExpansionFactor;
    dx = dy = std::max (dx, dy);
    range.low.x -= dx;
    range.high.x += dx;
    range.low.y -= dy;
    range.high.y += dy;
    bvector<DPoint3d> outerBox, clip1, clip2;
    outerBox.push_back (DPoint3d::From (range.low.x, range.low.y));
    outerBox.push_back (DPoint3d::From (range.high.x, range.low.y));
    outerBox.push_back (DPoint3d::From (range.high.x, range.high.y));
    outerBox.push_back (DPoint3d::From (range.low.x, range.high.y));
    static bool s_interior = false;
    static double s_sign = -1.0;
    size_t errors = 0;
    if (nullptr != cellData)
        cellData->clear ();
    int useEdgeNeighborClip = voronoiMetric != 1.0;
    VU_SET_LOOP (vertexSeed, graph)
        {
        if (!visited.IsSetAtNode (vertexSeed))
            {
            int numBoundary = vu_countMaskAroundVertex (vertexSeed, exteriorMask);
            int numTotal = vu_countEdgesAroundVertex (vertexSeed);
            ptrdiff_t userData1 = vertexSeed->GetUserData1 ();
            check (numBoundary, numTotal, userData1);
            visited.SetAroundVertex (vertexSeed);
            if (userData1 >= 0)
                {
                planes.clear ();
                if (cellData != nullptr)
                    cellData->push_back (NeighborIndices((size_t)userData1));


                VU_VERTEX_LOOP (outboundEdge, vertexSeed)
                    {
                    size_t indexA = (size_t)outboundEdge->GetUserData1 ();
                    size_t indexB = (size_t)outboundEdge->FSucc ()->GetUserData1 ();
                    if (indexA < points.size () && indexB < points.size ())
                        {
                        auto plane  = DPlane3d::VoronoiSplitPlane (points[indexA], radii[indexA], points[indexB], radii[indexB], voronoiMetric);
                        auto plane1 = DPlane3d::VoronoiSplitPlane (points[indexB], radii[indexB], points[indexA], radii[indexA], voronoiMetric);
                        if (!plane.Value ().origin.AlmostEqual (plane1.Value ().origin))
                            errors++;
                        if (plane.IsValid ())
                            {
                            DPlane3d plane1 = plane.Value ();
                            plane1.normal = s_sign * plane1.normal;
                            planes.push_back (ClipPlane (plane1, false, s_interior));
                            }
                        if (useEdgeNeighborClip)
                            {
                            // is there a triangle across the edge?
                            auto nodeB = outboundEdge->FSucc ();
                            auto nodeC0 = nodeB->EdgeMate ();
                            auto nodeC1 = nodeC0->FSucc ();
                            auto nodeC2 = nodeC1->FSucc ();
                            auto nodeC3 = nodeC2->FSucc ();
                            size_t indexC = (size_t)nodeC2->GetUserData1 ();
                            if (nodeC3 == nodeC0 && indexC < points.size ())
                                {
                                auto plane2 = DPlane3d::VoronoiSplitPlane (points[indexA], radii[indexA], points[indexC], radii[indexC], voronoiMetric);
                                if (plane2.IsValid ())
                                    {
                                    DPlane3d plane1 = plane2.Value ();
                                    plane1.normal = s_sign * plane1.normal;
                                    planes.push_back (ClipPlane (plane1, false, s_interior));
                                    }
                                }
                            }
                        // Store the neighbor point index .. later it will become a cellData index.
                        if (nullptr != cellData)
                            cellData->back ().AddNeighbor (indexB, SIZE_MAX);
                        }
                    }
                END_VU_VERTEX_LOOP (outboundEdge, vertexSeed)
                planes.ConvexPolygonClip (outerBox, clip1, clip2);
                DPoint3dOps::Compress (clip1, DoubleOps::SmallMetricDistance ());
                if (clip1.size () > 2)
                    {
                    size_t readIndex = voronoi->PointIndex().size ();
                    voronoi->AddPolygon (clip1);
                    if (nullptr != cellData)
                        cellData->back ().SetAuxIndex (readIndex);
                    }
                }
            }
        }
    END_VU_SET_LOOP (vertexSeed, graph)

    if (nullptr != cellData)
        {
        size_t numPoint = points.size ();
        size_t numCell  = cellData->size ();
        // CrossReference the neighbors
        bvector<size_t> pointIndexToCellIndex;
        for (size_t i = 0;i < numPoint; i++)
            pointIndexToCellIndex.push_back (SIZE_MAX);
        for (size_t i = 0; i < numCell; i++)
            {
            auto k = cellData->at(i).GetSiteIndex ();
            if (k < numPoint)
                pointIndexToCellIndex[k] = i;
            }
        for (auto &cell : *cellData)
            {
            for (auto &neighborPair : cell.Neighbors ())
                {
                if (neighborPair.siteIndex < numPoint)
                    neighborPair.neighborIndex = pointIndexToCellIndex[neighborPair.siteIndex];
                }
            }
        }
    //voronoi->Compress ();
    return voronoi;
    }

bool PolyfaceHeader::CreateDelauneyTriangulationAndVoronoiRegionsXY
(
bvector<DPoint3d> const &points,
bvector<double> const &radii,
int voronoiMetric,
PolyfaceHeaderPtr &delauney,
PolyfaceHeaderPtr &voronoi,
bvector<NeighborIndices> *cellData  //!< [out] optional array giving [siteIndex==pointIndex, auxIndex==facetRead, neighborIndex==array of indices of adjacent cells within cellData array]

)
    {
    /* NO -- CreateVoronoi is fixed to handle n=2 case ..
    if (points.size () == 2)
        {
        delauney = nullptr;
        voronoi = CreateTwoPointVoronoi (points[0], radii[0],  points[1], radii[1], voronoiMetric, 20.0, 20.0);
        return true;
        }
    */
    VuSetP graph = CreateDelauney (points);
    if (graph != nullptr)
        {
        delauney = vu_toPolyface (graph, VU_EXTERIOR_EDGE);
        voronoi = CreateVoronoi (graph, points, radii, voronoiMetric, cellData);
        vu_freeVuSet (graph);
        return true;
        }
    return false;
    }



END_BENTLEY_GEOMETRY_NAMESPACE
