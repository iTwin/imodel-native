/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
// REMARK:  PolyfaceSection.h is included into PolyhfaceClip.h -- shared support classes.

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static bool cb_dpoint4dLT_w (DPoint4dCR dataA, DPoint4dCR dataB)
    {
    return dataA.w < dataB.w;
    }

struct SegmentFrame
{
Transform m_worldToLocal;
size_t m_index;   // index of first point in original arrays.
DSegment3d m_segment;

// Create world to local transform from world to a (skewed) local system where:
// 1) x coordinate is parameter along line
// 2) y coordinate is along given vectorY.
// 3) z is perpendicular
// return false if pointA, pointB are parallel to vectorY.
bool Init (size_t index, DPoint3dCR pointA, DPoint3dCR pointB, DVec3dCR direction)
    {
    m_index = index;
    DVec3d vectorX = DVec3d::FromStartEnd (pointA, pointB);
    double lengthX = vectorX.Magnitude ();
    DVec3d vectorY = direction;
    DVec3d vectorZ;
    vectorY.ScaleToLength (lengthX);  // but of course it is not perpendicular !!!
    vectorZ.GeometricMeanCrossProduct (vectorX, vectorY);
    Transform localToWorld = Transform::FromOriginAndVectors (pointA, vectorX, vectorY, vectorZ);
    m_segment.Init (pointA, pointB);
    if (m_worldToLocal.InverseOf (localToWorld))
        {
        return true;
        }
    m_worldToLocal.InitIdentity ();
    return false;
    }

// worldPoints are assumed to have 2 wrap points -- i.e closure plus first edge. (And these must be bitwise copies )
void ProcessPolygon
(
bvector<DPoint3d> const &worldPoints,   // input polygon
bvector<DPoint3d> &localPoints,         // work space for transformed points
bvector<DPoint4d> &cutPoints,           // work space for sorting cut points
bvector<DSegment3d> &segments           // output segments.
)
    {
    cutPoints.clear ();
    size_t numEdge = worldPoints.size ();
    m_worldToLocal.Multiply (localPoints, worldPoints);
#ifdef PreTestToleranceEdgs
    // Look for "on" edge ...
    for (size_t i = 0; i < numEdge; i++)
        {
        size_t i1 = i + 1;
        if (i1 >= numEdge)
            i1 -= numEdge;
        if (fabs (localPoints[i].z) < zTol
            && fabs (localPoints[i1].z < zTol)
            {
            // EDGE i is ON!!!
            return;
            }
        }
#endif
    // Look for "simple" crossings .. 0 or negative z sign product with nonzero end z
    // Note that the start point is considered part of the first edge but not the last
    for (size_t i = 0; i < numEdge; i++)
        {
        size_t i1 = i + 1;
        if (i1 >= numEdge)
            i1 -= numEdge;
        if (localPoints[i1].z != 0.0 
          && localPoints[i].z * localPoints[i1].z <= 0.0)
            {
            double f = (-localPoints[i].z) / (localPoints[i1].z - localPoints[i].z);   // fraction along the edge that crosses
           double x = DoubleOps::Interpolate (localPoints[i].x, f, localPoints[i1].x);               // distance along the x axis that contains cut lines
            DPoint3d xyz = DPoint3d::FromInterpolate (worldPoints[i], f, worldPoints[i1]);
            cutPoints.push_back (DPoint4d::From (xyz, x));
            }
        }
    std::sort (cutPoints.begin (), cutPoints.end (), cb_dpoint4dLT_w);

    for (size_t i = 0; i + 1 < cutPoints.size (); i += 2)
        {
        double f0 = cutPoints[i].w;
        double f1 = cutPoints[i+1].w;
        DSegment3d fullSegment;
        cutPoints[i].GetXYZ (fullSegment.point[0]);
        cutPoints[i + 1].GetXYZ (fullSegment.point[1]);
        double g;
        DSegment3d clippedSegment = fullSegment;
        if (f0 < 1.0 && f1 > 0.0 && f1 > f0)
            {
            double df = f1 - f0;
            if (f0 < 0.0)
                {
                DoubleOps::SafeDivide (g, -f0, df, 0.0);
                clippedSegment.point[0].Interpolate (fullSegment.point[0], g, fullSegment.point[1]);
                }
            if (f1 > 1.0)
                {
                DoubleOps::SafeDivide (g, 1.0 - f0, df, 1.0);
                clippedSegment.point[1].Interpolate (fullSegment.point[0], g, fullSegment.point[1]);
                }
                segments.push_back (clippedSegment);
            }
        }
    }
};

struct LocalProjectedLinestring
{
DVec3d m_direction;
bvector<DPoint3d> m_viewedPoints;
bvector<DPoint3d> m_worldPoints;
bvector<DVec3d> m_perpVector;
bvector<SegmentFrame> m_segmentFrame;
Transform m_worldToViewPlane;

Transform m_worldToPrincipalBox; 
DRange3d m_principalBoxRange;   // range of the linestring in the principal orientation.

bvector<DPoint3d> m_localPolygonPoints;
bvector<DPoint4d> m_cutPoints;      // xyz is space coordinate.  w is fraction along edge.
double m_tol;

LocalProjectedLinestring (double tol)
    {
    m_tol = tol;
    }

bool Load (bvector<DPoint3d> const &spacePoints, DVec3dCR direction)
    {
    m_worldPoints = spacePoints;
    m_viewedPoints = spacePoints;
    m_direction = direction;

    if (spacePoints.size () < 2)
        return false;
    if (!m_worldToViewPlane.InitFromProjectionToPlane (spacePoints[0], direction))    // This is singular  !!!
        return false;

    m_worldToViewPlane.Multiply (m_viewedPoints, m_worldPoints);
#ifdef UsePrincipalAxes
    DVec3d moments;
    Transform principalToViewplane, viewplaneToPrincipalBox;
    DPoint3dOps::PrincipalAxes (m_viewedPoints, principalToViewplane, viewplaneToPrincipalBox, moments);
    m_worldToPrincipalBox.ProductOf (viewplaneToPrincipalBox, m_worldToViewplane);
    assert (moments.x >= moments.y && moments.y >= moments.z);
    m_worldToPrincipalBox.Multiply (m_viewedPoints, m_worldPoints);
#else
    m_worldToPrincipalBox = m_worldToViewPlane;
#endif
    m_principalBoxRange.Init ();
    m_principalBoxRange.Extend (m_viewedPoints);
    m_segmentFrame.clear ();
    // Build transforms to a local frame for each non-degenerate line ...
    for (size_t i = 0; i + 1 < m_viewedPoints.size (); i++)
        {
        SegmentFrame segmentData;
        if (segmentData.Init (i, spacePoints[i], spacePoints[i+1], direction))
            m_segmentFrame.push_back (segmentData);
        }
    //m_tol = Angle::SmallAngle () * m_viewedDrapeLineRange.LargestCoordinate ();
    return m_segmentFrame.size () > 0;
    }


void ProcessPolygon (bvector<DPoint3d> const &polygonPoints, bvector <DSegment3d> &segments)
    {
    m_worldToPrincipalBox.Multiply (m_viewedPoints, polygonPoints);
    DRange3d viewedPolygonRange;
    viewedPolygonRange.Init ();
    viewedPolygonRange.Extend (m_viewedPoints);
    // Quick exit for complete misses 
    if (viewedPolygonRange.IntersectsWith (m_principalBoxRange, 0.0, 2))
        {
        for (size_t i = 0; i < m_segmentFrame.size (); i++)
            {
            m_segmentFrame[i].ProcessPolygon (polygonPoints, m_viewedPoints, m_cutPoints, segments);
            }
        }
    }
};


struct SectionGraph
{
// Graph nodes index to m_allVertexData via VertexDataLabelTag.
// m_allVertexData has xyz, fraction, readIndex, vertexIndex.
// vertexIndex is into the map -- map xyz should match vertexData.mXYZ;
MTGGraph m_graph;
int      m_vertexDataLabel;
static const int VertexDataLabelTag = 1010;
static const int DefaultVertexDataLabel = -1;

PolyfaceCoordinateMapPtr m_coordinateMap;
bvector<MTGNodeId>    m_anyNodeAtVertex;
bvector <VertexData> m_allVertexData;   // Built up as edges are added to the graph.
                                        // Edges refer to here (only) through their VertexDataLabel

MTGNodeId FindOrSetPriorNodeAtVertex (size_t vertexIndex, MTGNodeId nodeId)
    {
    size_t numRegistered = m_anyNodeAtVertex.size ();
    assert (vertexIndex <= numRegistered + 1);  // we expect the map lookup to generate new vertex indices sequentially,
                                                // and they will 
    while (numRegistered <= vertexIndex)
        {
        m_anyNodeAtVertex.push_back (MTG_NULL_NODEID);
        numRegistered++;
        }

    if (m_anyNodeAtVertex[vertexIndex] == MTG_NULL_NODEID)
        {
        m_anyNodeAtVertex[vertexIndex] = nodeId;
        }
    return m_anyNodeAtVertex[vertexIndex];
    }

MTGNodeId FindPriorNodeAtVertex (size_t vertexIndex)
    {
    size_t numRegistered = m_anyNodeAtVertex.size ();
    
    return vertexIndex < numRegistered ? m_anyNodeAtVertex[vertexIndex] : MTG_NULL_NODEID;
    }


// On input:  graph has isolated edges indexeing into m_allVertexData.
// (m_allVertexData[i].m_vertexIndex is not used yet)
// Assign vertex indices and twist the edges together.
void JoinEdges ()
    {
    for (size_t i = 0, n = m_allVertexData.size (); i < n; i++)
        m_allVertexData[i].m_vertexIndex = SIZE_MAX;
    MTGARRAY_SET_LOOP (nodeA, &m_graph)
        {
        int iIndex;
        if (m_graph.TryGetLabel (nodeA, m_vertexDataLabel, iIndex)
            && iIndex >= 0)
            {
            size_t index = (size_t)iIndex;
            size_t vertexIndex = m_coordinateMap->AddPoint (m_allVertexData[index].mXYZ);
            MTGNodeId nodeB = FindOrSetPriorNodeAtVertex (vertexIndex, nodeA);
            m_allVertexData[index].m_vertexIndex = vertexIndex;
            if (m_graph.IsValidNodeId (nodeB))
                m_graph.VertexTwist (nodeA, nodeB);            
            }
        }
    MTGARRAY_END_SET_LOOP (nodeA, &m_graph)    
    }

SectionGraph (PolyfaceCoordinateMapPtr coordinateMap) : m_coordinateMap (coordinateMap)
    {
    m_vertexDataLabel = m_graph.DefineLabel (VertexDataLabelTag, MTG_LabelMask_VertexProperty, DefaultVertexDataLabel);
    }

// Append to global array of vertex data. Return index where copied.
size_t AddVertexData (VertexData const & data)
    {
    size_t index = m_allVertexData.size ();
    m_allVertexData.push_back (data);
    return index;
    }
    
void AddEdge (VertexData const &vertexA, VertexData const &vertexB)
    {
    MTGNodeId nodeA, nodeB;
    // Make an edge...  just hanging out there.
    m_graph.CreateEdge (nodeA, nodeB);
    m_graph.TrySetLabel (nodeA, m_vertexDataLabel, (int)AddVertexData (vertexA));
    m_graph.TrySetLabel (nodeB, m_vertexDataLabel, (int)AddVertexData (vertexB));
    }

void AddEdge (size_t readIndex, DPoint3dCR xyzA, DPoint3dCR xyzB)
    {
    MTGNodeId nodeA, nodeB;
    // Make an edge...  just hanging out there.
    m_graph.CreateEdge (nodeA, nodeB);
    m_graph.TrySetLabel (nodeA, m_vertexDataLabel, (int)AddVertexData (VertexData::FromReadIndexAndCoordinates (readIndex, xyzA)));
    m_graph.TrySetLabel (nodeB, m_vertexDataLabel, (int)AddVertexData (VertexData::FromReadIndexAndCoordinates (readIndex, xyzB)));
    }

size_t PurgeDuplicates ()
    {
    // 
    MTGMask deleteMask = m_graph.GrabMask ();
    MTGMask edgeVisitedMask = m_graph.GrabMask ();
    m_graph.ClearMask (deleteMask);
    m_graph.ClearMask (edgeVisitedMask);
    size_t numMark = 0;
    MTGARRAY_SET_LOOP (nodeA0, &m_graph)
        {
        if (!m_graph.GetMaskAt (nodeA0, deleteMask)
            && !m_graph.GetMaskAt (nodeA0, edgeVisitedMask)
            )
            {
            m_graph.SetMaskAroundEdge (nodeA0, edgeVisitedMask);
            MTGNodeId nodeB0 = m_graph.FSucc (nodeA0);
            for (MTGNodeId nodeA1 = nodeA0;
                (nodeA1 = m_graph.VSucc (nodeA1)) != nodeA0;
                )
                {
                if (!m_graph.GetMaskAt (nodeA1, deleteMask))
                    {
                    MTGNodeId nodeB1 = m_graph.FSucc (nodeA1);
                    if (m_graph.AreNodesInSameVertexLoop (nodeB0, nodeB1))
                        {
                        m_graph.SetMaskAroundEdge (nodeA1, deleteMask);
                        numMark++;
                        }
                    }
                }
            }
        }
    MTGARRAY_END_SET_LOOP (nodeA0, &m_graph)
    size_t numDelete = m_graph.DropMaskedEdges (deleteMask);
    assert (numMark == numDelete);
    m_graph.DropMask (deleteMask);
    m_graph.DropMask (edgeVisitedMask);
    return numDelete;
    }

VertexDataArray m_vertexAroundFacet;           // Vertices around one facet.
VertexDataArray m_simpleCrossings; // Points with true up or down crossings.  To be paired by later sorting.
bvector<VertexDataPair> m_edges;   // Pairs of points for "on" edges.

// Slice a the single loop (in the visitor) to a single plane.
// Append data to the end of the data arrays in the target mesh.
// (All data arrays are assumed the same length in the target.)
void CutVisitorPolygonWithSinglePlane (PolyfaceVisitorR visitor, ClipPlaneR plane, double zTol)
    {
    double zPos = fabs (zTol);
    double zNeg = -fabs (zTol);

    size_t i0 = 0;
    size_t n0 = visitor.Point ().size ();     // number of points (and edges) in facets.  After setup, there will be two more cycles to simplify indexing.
    if (n0 < 3)
        return;

    m_vertexAroundFacet.SetReplicatedAltitudes (i0, n0, visitor.Point (), visitor.IndexPosition (), plane, 2);
    m_vertexAroundFacet.SetTolerancedAltitudeFlag (zTol);
    m_simpleCrossings.clear ();
    m_edges.clear ();
    if (m_vertexAroundFacet.aMax < zNeg) // All OUT
        return;

    if (m_vertexAroundFacet.aMin > zPos) // All IN
        return;

    // Collect all ON edges directly ...
    for (size_t k = 0; k < n0; k++)
        {
        if (m_vertexAroundFacet[k].m_intData == 0 && m_vertexAroundFacet[k+1].m_intData == 0)
            {
            m_edges.push_back (VertexDataPair (m_vertexAroundFacet[k], m_vertexAroundFacet[k+1]));
            }
        }

    // For each vertex:  is this vertex the last vertex before a true crossing?
    // "ON" edges have already been identified.  We walk past ON vertices if not convenient here.
    // Look for: (1) a simple crossing strictly within the following edge.  Record the fractional point.
    //           (2) 1 or more ON edges followed by an opposite sign.  Record the final ON vertex as an up or down.
    //           (2) 1 or more ON edges folowed by the starting sign.  Ignore.
    for (size_t k = 0; k < n0; k++)
        {
        int s0 = m_vertexAroundFacet[k].m_intData;
        if (m_vertexAroundFacet[k].m_intData != 0)
            {
            int s01 = m_vertexAroundFacet[k+1].m_intData * s0;
            if (s01 > 0)
                {
                // no crossing
                }
            else if (s01 < 0)
                {
                // simple crossing.  Division is safe !!
                m_simpleCrossings.push_back (VertexData::FromInterpolatedCrossing (m_vertexAroundFacet[k], 0.0, m_vertexAroundFacet[k+1]));
                }
            else
                {
                // Find the first following vertex that is not "on" (in worst case we may come back to the start?)
                size_t k0 = k + 1;
                while (m_vertexAroundFacet[k0 + 1].m_intData == 0)
                    k0++;
                int s02 = s0 * m_vertexAroundFacet[k0 + 1].m_intData;
                if (s02 > 0)
                    {
                    // cross and return.  ignore !!!
                    }
                else
                    {
                    m_simpleCrossings.push_back (VertexData::FromExactCrossing (m_vertexAroundFacet[k0], s0));
                    }
                }
            }
        }
    
    size_t numSimpleCrossings = m_simpleCrossings.size ();
    size_t numEdges = m_edges.size ();
    if (numSimpleCrossings > 0 || numEdges > 0)
        {
        SortAndSimplifyCrossings (m_simpleCrossings, m_edges);
        for (size_t i = 0; i < numSimpleCrossings; i += 2)
            AddEdge (m_simpleCrossings[i], m_simpleCrossings[i+1]);
        for (size_t i = 0; i < numEdges; i++)
            AddEdge (m_edges[i].m_dataA, m_edges[i].m_dataB);
        }
    }




void AddPlaneSectionCut
(
PolyfaceQueryCR source,
PolyfaceCoordinateMap &destMap,
DPlane3dCR plane
)
    {
    PolyfaceVisitorPtr visitorPtr = PolyfaceVisitor::Attach (source, true);
    PolyfaceVisitor & visitor = *visitorPtr.get ();
    ClipPlane clipPlane = ClipPlane (plane);
    static double sRelTol = 1.0e-12;
    double zTol = sRelTol * visitor.GetClientPolyfaceQueryCR ().LargestCoordinate ();
    for (visitor.Reset ();visitor.AdvanceToNextFace ();)
        {
        CutVisitorPolygonWithSinglePlane (visitor, clipPlane, zTol);
        }
    }

void AddDrapedLinestring
(
PolyfaceQueryCR source,
PolyfaceCoordinateMap &destMap,
bvector<DPoint3d> const &spacePoints,
DVec3dCR direction
)
    {
    PolyfaceVisitorPtr visitorPtr = PolyfaceVisitor::Attach (source, true);
    PolyfaceVisitor & visitor = *visitorPtr.get ();
    static double sRelTol = 1.0e-12;
    double zTol = sRelTol * visitor.GetClientPolyfaceQueryCR ().LargestCoordinate ();
    LocalProjectedLinestring draper (zTol);
    draper.Load (spacePoints, direction);
    bvector<DSegment3d> segments;
    for (visitor.Reset ();visitor.AdvanceToNextFace ();)
        {
        segments.clear ();
        draper.ProcessPolygon (visitor.Point (), segments);
        for (size_t i = 0; i < segments.size (); i++)
            AddEdge (visitor.GetReadIndex (), segments[i].point[0], segments[i].point[1]);
        }
    }



bool AppendXYZ (MTGNodeId nodeId, bvector<DPoint3d> &dest)
    {
    int index;
    DPoint3d xyz;
    if (m_graph.TryGetLabel (nodeId, m_vertexDataLabel, index)
        && m_coordinateMap->TryGetPoint ((size_t)index, xyz)
        )
        {
        dest.push_back (xyz);
        return true;
        }
    return false;
    }

// Single chain is built up in these two (for eventual transfer to output curve vector)
bvector <FacetEdgeLocationDetail> m_chainLocation;
bvector <DPoint3d> m_chainXYZ;
void AppendOutputVertex (MTGNodeId nodeId)
    {
    int vertexDataIndex;
    if (m_graph.TryGetLabel (nodeId, m_vertexDataLabel, vertexDataIndex)
        && vertexDataIndex >= 0)
        {
        size_t index = (size_t) vertexDataIndex;
        m_chainXYZ.push_back (m_allVertexData[index].mXYZ);
        m_chainLocation.push_back (FacetEdgeLocationDetail (m_allVertexData[index].m_readIndex, m_allVertexData[index].m_edgeFraction));
        }
    }
    
void ClearChain ()
    {
    m_chainLocation.clear ();
    m_chainXYZ.clear ();
    }
    
ICurvePrimitivePtr ChainToCurvePrimitive (size_t minimumSize, bool markEdgeFractions)
    {
    if (m_chainXYZ.size () < minimumSize)
        return NULL;
    ICurvePrimitivePtr linestring = ICurvePrimitive::CreateLineString (m_chainXYZ);
    if (markEdgeFractions)
        {
        FacetEdgeLocationDetailVectorPtr edgeData = FacetEdgeLocationDetailVector::Create ();
        for (size_t i = 0; i < m_chainLocation.size (); i++)
            edgeData->Add (m_chainLocation[i].m_readIndex, m_chainLocation[i].m_fraction);
        linestring->SetCurvePrimitiveInfo (edgeData);
        }
    return linestring;
    }
    
// Collect a linestring with all edges reachable from startNode forward.
// Apply barrier mask to both sides of each edge.
// Stop on hitting vertexBarrierMask or edgeVisitedMask
// Add closure if returned to start.
// use xyz as work space.
ICurvePrimitivePtr CollectChainFrom (MTGNodeId startNode, MTGMask edgeVisitedMask, MTGMask barrierMask, bvector<DPoint3d>&xyz,
    size_t minimumSize, bool markEdgeFractions)
    {
    ClearChain ();
    MTGNodeId currNode = startNode;
    AppendOutputVertex (currNode);
    while (!m_graph.GetMaskAt (currNode, edgeVisitedMask))
        {
        m_graph.SetMaskAroundEdge (currNode, edgeVisitedMask);
        currNode = m_graph.FSucc (currNode);
        AppendOutputVertex (currNode);
        if (currNode == startNode || m_graph.GetMaskAt (currNode, barrierMask))
            break;
        }
    return ChainToCurvePrimitive (minimumSize, markEdgeFractions);
    }

static void Check (size_t value, char const* name)
    {
    }

    
CurveVectorPtr CollectChains (bool formRegions, bool markEdgeFractions)
    {
    bvector<MTGNodeId> vertexSeed;
    m_graph.CollectVertexLoops (vertexSeed);
    MTGMask barrierVertexMask = m_graph.GrabMask ();
    m_graph.ClearMask (barrierVertexMask);
    size_t totalVerts = vertexSeed.size ();
    size_t interiorVerts = 0;
    size_t otherVerts = 0;
    // We want unusual vertices -- anything with other than 2 nodes -- at front.
    // Walk throught the array, swapping simple verts to the (growing downward) pile of simples at the end.
    // (The vert that is swapped forward is always unknown and is then reinspected on the next pass)
    // Indices [0..otherVerts-1] are confirmed "other"
    // Indices [totalVerts-interiorVerts..totalVerts-1] are confirmed "interior"
    // Indices [otherVerts..totalVerts-interiorVerts-1] are untested.
    // Index [otherVerts] is being tested
    // Indices 
    while (otherVerts + interiorVerts < totalVerts)
        {
        MTGNodeId seed = vertexSeed[otherVerts];
        size_t numThisVertex = m_graph.CountNodesAroundVertex (seed);
        if (numThisVertex == 2)
            {
            interiorVerts++;
            std::swap (vertexSeed[otherVerts], vertexSeed[totalVerts - interiorVerts]);
            }
        else
            {
            m_graph.SetMaskAroundVertex (seed, barrierVertexMask);
            otherVerts++;
            }
        }
        
    MTGMask edgeVisitedMask = m_graph.GrabMask ();
    m_graph.ClearMask (edgeVisitedMask);
    bvector<DPoint3d> chainXYZ;
    CurveVector::BoundaryType topType;
    size_t minSize;
    if (otherVerts == 0 && formRegions)
        {
        // The graph has nothing but closed loops !!! (All cheer)
        topType = CurveVector::BOUNDARY_TYPE_ParityRegion;
        minSize = 3;
        }
    else
        {
        topType = CurveVector::BOUNDARY_TYPE_None;
        minSize = 2;
        }
    
    size_t numFace = m_graph.CountFaceLoops ();
    size_t numVertex = m_graph.CountVertexLoops ();
    Check (numFace, "faces");
    Check (numVertex, "vertices");
    ICurvePrimitivePtr thisLoop = NULL;
    CurveVectorPtr allLoops = CurveVector::Create (topType);
    
    bvector<ICurvePrimitivePtr> linestrings;
    for (size_t i = 0, n = vertexSeed.size (); i < n; i++)
        {
        MTGNodeId seed = vertexSeed[i];
        if (!m_graph.GetMaskAt (seed, edgeVisitedMask))
            {
            thisLoop = CollectChainFrom (seed,edgeVisitedMask, barrierVertexMask, chainXYZ, minSize, markEdgeFractions);
            if (thisLoop.IsValid ())
                linestrings.push_back (thisLoop);
            }
        }
    CurveVectorPtr result = NULL;
    bool resultIsRegion = otherVerts == 0 && formRegions;
    // The linestrings are primitives that don't care whether they are open or closed.
    // Closure arises if they are placed in parents that announce closure ...
    if (linestrings.size () == 0)
        {
        }
    else if (linestrings.size () == 1)
        {
        result = CurveVector::Create (linestrings[0],
                    resultIsRegion ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Open);
        }
    else // many children
        {
        if (resultIsRegion)
            {
            // Multiple loops ...
            result = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);
            for (size_t i = 0; i < linestrings.size (); i++)
                {
                CurveVectorPtr child = CurveVector::Create (linestrings[i], CurveVector::BOUNDARY_TYPE_Outer);
                result->push_back (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*child));
                }
            }
        else
            {
            // Multiple isolated primitives ...
            result = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
            for (size_t i = 0; i < linestrings.size (); i++)
                {
                result->push_back (linestrings[i]);
                }
            }
        }

    return result;
    }
};
// NEEDS WORK: Output "ON" faces as complete regions.

GEOMDLLIMPEXP CurveVectorPtr PolyfaceQuery::PlaneSlice (DPlane3dCR sectionPlane, bool formRegions, bool markEdgeFractions) const
    {
    // This polyface only gets its point array built up ... (The PolyfaceCoordinateMap addresse them)
    PolyfaceHeaderPtr section = PolyfaceHeader::CreateVariableSizeIndexed ();
    PolyfaceCoordinateMapPtr sectionMap = PolyfaceCoordinateMap::New (*section.get ());
    SectionGraph chainGraph (sectionMap);
    chainGraph.AddPlaneSectionCut (*this, *sectionMap, sectionPlane);
    chainGraph.JoinEdges ();
    chainGraph.PurgeDuplicates ();
    return chainGraph.CollectChains (formRegions, markEdgeFractions);
    }


CurveVectorPtr PolyfaceQuery::DrapeLinestring (bvector<DPoint3d> &spacePoints, DVec3dCR direction) const
    {
    // This polyface only gets its point array built up ... (The PolyfaceCoordinateMap addresse them)
    PolyfaceHeaderPtr section = PolyfaceHeader::CreateVariableSizeIndexed ();
    PolyfaceCoordinateMapPtr sectionMap = PolyfaceCoordinateMap::New (*section.get ());
    SectionGraph chainGraph (sectionMap);
    chainGraph.AddDrapedLinestring (*this, *sectionMap, spacePoints, direction);
    chainGraph.JoinEdges ();
    chainGraph.PurgeDuplicates ();
    return chainGraph.CollectChains (false, false);
    }
END_BENTLEY_GEOMETRY_NAMESPACE