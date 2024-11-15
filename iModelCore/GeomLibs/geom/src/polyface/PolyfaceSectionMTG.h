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
            double f = (-localPoints[i].z) / (localPoints[i1].z - localPoints[i].z);    // fraction along the edge that crosses
            double x = DoubleOps::Interpolate (localPoints[i].x, f, localPoints[i1].x); // distance along the x axis that contains cut lines
            DPoint3d xyz = DPoint3d::FromInterpolate (worldPoints[i], f, worldPoints[i1]);
            cutPoints.push_back (DPoint4d::From (xyz, x));  // sneaky: use w to store param of projection onto the segment being draped
            }
        }
    std::sort (cutPoints.begin (), cutPoints.end (), cb_dpoint4dLT_w);

    for (size_t i = 0; i + 1 < cutPoints.size (); i += 2)
        {
        double f0 = cutPoints[i].w;
        double f1 = cutPoints[i+1].w;   // f1 >= f0 guaranteed by the sort
        DSegment3d fullSegment;
        cutPoints[i].GetXYZ (fullSegment.point[0]);
        cutPoints[i + 1].GetXYZ (fullSegment.point[1]);
        double g;
        DSegment3d clippedSegment = fullSegment;
        // NOTE: tolerance is consistently NOT used here; it is not needed.
        // When f1==f0 we've draped onto a perpendicular facet; allow this only when the draped segment is nontrivial.
        if (f0 < 1.0 && f1 > 0.0 && (f1 > f0 || clippedSegment.LengthSquared() > 0.0))
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


// On input: graph has isolated edges indexing into m_allVertexData.
// (m_allVertexData[i].m_vertexIndex is not used yet)
// Assign vertex indices and twist the edges together.
void JoinEdges ()
    {
    for (size_t i = 0, n = m_allVertexData.size (); i < n; i++)
        m_allVertexData[i].m_vertexIndex = SIZE_MAX;
    MTGARRAY_SET_LOOP (nodeA, &m_graph)
        {
        if (auto data = GetVertexData(nodeA))
            {
            size_t vertexIndex = m_coordinateMap->AddPoint(data->mXYZ);
            MTGNodeId nodeB = FindOrSetPriorNodeAtVertex (vertexIndex, nodeA);
            data->m_vertexIndex = vertexIndex;
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

void AddGapEdge(MTGNodeId startNodeId, size_t destVertexIndex)
    {
    MTGNodeId destNodeId = FindPriorNodeAtVertex(destVertexIndex);
    int startVertexDataIndex, destVertexDataIndex;
    if (m_graph.IsValidNodeId(startNodeId) && m_graph.IsValidNodeId(destNodeId) &&
        m_graph.TryGetLabel(startNodeId, m_vertexDataLabel, startVertexDataIndex) &&
        m_graph.TryGetLabel(destNodeId, m_vertexDataLabel, destVertexDataIndex))
        {
        MTGNodeId nodeA, nodeB;
        m_graph.CreateEdge(nodeA, nodeB);
        m_graph.TrySetLabel(nodeA, m_vertexDataLabel, startVertexDataIndex);
        m_graph.TrySetLabel(nodeB, m_vertexDataLabel, destVertexDataIndex);
        m_graph.VertexTwist(nodeA, startNodeId);
        m_graph.VertexTwist(nodeB, destNodeId);
        }
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

    for (MTGNodeId& nodeId : m_anyNodeAtVertex)
        {   // find a surviving node at each vertex
        if (m_graph.GetMaskAt(nodeId, deleteMask))
            nodeId = m_graph.FindUnmaskedAroundVertex(nodeId, deleteMask);
        }

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

size_t GetVertexDataIndex(MTGNodeId nodeId)
    {
    int vertexDataIndex;
    if (m_graph.TryGetLabel(nodeId, m_vertexDataLabel, vertexDataIndex) && vertexDataIndex >= 0 && vertexDataIndex < m_allVertexData.size())
        return (size_t) vertexDataIndex;
    return SIZE_MAX;
    }

VertexData* GetVertexData(MTGNodeId nodeId)
    {
    size_t index = GetVertexDataIndex(nodeId);
    if (index < SIZE_MAX)
        return &m_allVertexData[index];
    return nullptr;
    }

bool AppendXYZ (MTGNodeId nodeId, bvector<DPoint3d> &dest)
    {
    if (auto data = GetVertexData(nodeId))
        {
        dest.push_back(data->mXYZ);
        return true;
        }
    return false;
    }

// Single chain is built up in these two (for eventual transfer to output curve vector)
bvector <FacetEdgeLocationDetail> m_chainLocation;
bvector <DPoint3d> m_chainXYZ;
void AppendOutputVertex (MTGNodeId nodeId)
    {
    if (auto data = GetVertexData(nodeId))
        {
        m_chainXYZ.push_back(data->mXYZ);
        m_chainLocation.push_back(FacetEdgeLocationDetail(data->m_readIndex, data->m_edgeFraction));
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

void FillGapsInLoops(bvector<MTGNodeId> const& vertexSeeds, DVec3dCR refVector)
    {
    double absTol = m_coordinateMap->GetXYZAbsTol();
    double relTol = m_coordinateMap->GetXYZRelTol();
    if (relTol <= 0.0)
        return;

    typedef bpair<double, size_t> ParamVertexIndexPair;
    auto compareParams = [](ParamVertexIndexPair const& a, ParamVertexIndexPair const& b) { return a.first < b.first; };

    // order the points in the mesh by dot product with "random" vector
    DPoint3d xyz0;
    m_coordinateMap->TryGetPoint(0, xyz0);
    DVec3d unitSortVec = DVec3d::FromNormalizedCrossProduct(refVector, DVec3d::From(1.45, 0.13, 0.2));
    size_t vertexIndex = 0;
    bvector<ParamVertexIndexPair> paramToVertexIndex;
    for (auto const& xyz : m_coordinateMap->GetPolyfaceHeaderR().Point())
        paramToVertexIndex.push_back(ParamVertexIndexPair(xyz.DotDifference(xyz0, unitSortVec), vertexIndex++));
    std::sort(paramToVertexIndex.begin(), paramToVertexIndex.end(), compareParams);

    // for each dangling vertex in the graph, look for nearby mesh points mapped to distinct graph vertices,
    // and bridge the gap if within tolerance.
    for (auto const& seed: vertexSeeds)
        {
        if (1 != m_graph.CountNodesAroundVertex(seed))
            continue;
        auto degOneData = GetVertexData(seed);
        auto fSuccData = GetVertexData(m_graph.FSucc(seed));
        if (!degOneData || !fSuccData)
            continue;
        DPoint3dCR danglerVertex = degOneData->mXYZ;
        size_t danglerVertexIndex = degOneData->m_vertexIndex;
        size_t danglerFarVertexIndex = fSuccData->m_vertexIndex;

        // sort_zyx() in PolyfaceCoordinateMap.cpp equates points using l-infinity norm:
        // ||a-b|| := a.MaxDiff(b) <= absTol + relTol * (|a.x| + |a.y| + |a.z| + |b.x| + |b.y| + |b.z|)
        double tolFactor = 2.0 * (fabs(danglerVertex.x) + fabs(danglerVertex.y) + fabs(danglerVertex.z));
        double param = danglerVertex.DotDifference(xyz0, unitSortVec);

        // bridge shortest gaps first, then loosen relative tol and repeat
        static const size_t s_maxIter = 4;
        double myRelTol = relTol * 10.0;
        for (size_t iter = 0; iter < s_maxIter && myRelTol <= mgds_fc_epsilon; ++iter, myRelTol *= 10.0)
            {
            double tol = absTol + myRelTol * tolFactor; // approximation to coordMap's tol using new relative tolerance
            double twoTol = tol + tol;
            double minDist = DBL_MAX;
            size_t closestVertexIndex = SIZE_MAX;

            // To find nearby points c to d=danglerVertex, we only have to check nearby projection parameters q to
            // p=param. To see this, suppose |q-param| > 2*tol. Then we have:
            //    2*tol < |q-param| <= DRange3d::From(c,d).DiagonalDistance() <= sqrt(3||c-d||^2) < 2||c-d||.
            // Thus all points c with projection parameter q that satisfy ||c-d|| <= tol are found in the parameter
            // range |q-param| <= 2*tol.
            auto lessThan = [&](ParamVertexIndexPair const& a, ParamVertexIndexPair const& b) { return a.first < b.first - twoTol; };

            // find the first vertex in parametric range of danglerVertex
            auto closeParam = std::lower_bound(paramToVertexIndex.begin(), paramToVertexIndex.end(), ParamVertexIndexPair(param, 0), lessThan);
            if (closeParam != paramToVertexIndex.end())
                {
                // iterate projections in range of param
                for (auto iParam = closeParam; iParam != paramToVertexIndex.end(); ++iParam)
                    {
                    if (iParam->first - param > twoTol)
                        break; // no more vertices close to danglerVertex in parameter space
                    if (iParam->second == danglerVertexIndex || iParam->second == danglerFarVertexIndex)
                        continue; // there's already an edge between this vertex and danglerVertex
                    DPoint3d closeVertex;
                    m_coordinateMap->TryGetPoint(iParam->second, closeVertex);
                    double dist = danglerVertex.MaxDiff(closeVertex); // using coordMap norm
                    if (dist > 0.0 && dist <= tol && dist < minDist)
                        {
                        closestVertexIndex = iParam->second;
                        minDist = dist;
                        }
                    }
                }
            if (closestVertexIndex < SIZE_MAX)
                {
                AddGapEdge(seed, closestVertexIndex);
                break;
                }
            }
        }
    }

bvector<MTGNodeId> MarkDanglingChains(bvector<MTGNodeId> const& vertexSeeds, MTGMask danglerMask)
    {
    bvector<MTGNodeId> danglerVertices;

    // Lambda to return unmasked node at this vertex if it is the only such node.
    // We can't just mark chains starting at degree-1 vertices: this would miss the trunk of a dangling tree.
    auto getIsolatedUnmaskedNodeAtVertex = [&](MTGNodeId vertex, MTGMask mask) -> MTGNodeId
        {
        MTGNodeId found = MTG_NULL_NODEID;
        MTGARRAY_VERTEX_LOOP(currNodeId, &m_graph, vertex)
            {
            if (!m_graph.GetMaskAt(currNodeId, mask))
                {
                if (MTG_NULL_NODEID != found)
                    return MTG_NULL_NODEID; // not isolated
                found = currNodeId;
                }
            }
        MTGARRAY_END_VERTEX_LOOP(currNodeId, &m_graph, vertex)
        return found;
        };

    bool foundDanglingEdge;
    do
        {
        foundDanglingEdge = false;
        for (auto const& seed: vertexSeeds)
            {
            MTGNodeId dangler = getIsolatedUnmaskedNodeAtVertex(seed, danglerMask);
            if (MTG_NULL_NODEID != dangler)
                {
                foundDanglingEdge = true;
                danglerVertices.push_back(dangler);
                do
                    {   // mark the entire *unambiguous* chain
                    m_graph.SetMaskAroundEdge(dangler, danglerMask);
                    dangler = m_graph.FSucc(dangler);
                    } while (m_graph.CountNodesAroundVertex(dangler) == 2);
                }
            }
        } while (foundDanglingEdge);

    return danglerVertices;
    }

// Given a vertex loop containing more than 2 edges, reorder the radial edges by their angle measured from the first edge.
void SortVertexLoop(MTGNodeId seed, DVec3dCR refVector)
    {
    auto nearVertexData = GetVertexData(seed);
    if (!nearVertexData)
        return;
    auto farVertexData = GetVertexData(m_graph.FSucc(seed));
    if (!farVertexData)
        return;
    DVec3d edgeDir0 = DVec3d::FromStartEnd(nearVertexData->mXYZ, farVertexData->mXYZ);

    farVertexData = GetVertexData(m_graph.FSucc(m_graph.VSucc(seed)));
    if (!farVertexData)
        return;
    DVec3d edgeDir1 = DVec3d::FromStartEnd(nearVertexData->mXYZ, farVertexData->mXYZ);
    DVec3d normal = (edgeDir0.SignedAngleTo(edgeDir1, refVector) < 0.0) ? DVec3d::FromScale(refVector, -1) : refVector;

    bvector<bpair<double, MTGNodeId>> incidentEdges; // radian angle in [0, 2pi)
    MTGARRAY_VERTEX_LOOP(edgeId, &m_graph, seed)
        {
        farVertexData = GetVertexData(m_graph.FSucc(edgeId));
        if (!farVertexData)
            return;
        double angle = Angle::AdjustToSweep(edgeDir0.SignedAngleTo(DVec3d::FromStartEnd(nearVertexData->mXYZ, farVertexData->mXYZ), normal), 0.0, msGeomConst_2pi);
        incidentEdges.push_back({angle, edgeId});
        }
    MTGARRAY_END_VERTEX_LOOP(edgeId, &m_graph, seed)

    bool alreadyOrdered = true;
    for (size_t i = 1; i < incidentEdges.size(); ++i)
        {
        if (!(alreadyOrdered = (incidentEdges[i - 1].first <= incidentEdges[i].first)))
            break;
        }
    if (alreadyOrdered)
        return;

    // lambda ignores duplicate angles, because we don't expect them after PurgeDuplicates
    std::sort(incidentEdges.begin(), incidentEdges.end(), [](auto const& e0, auto const& e1) { return e0.first < e1.first; });

    // destroy the vertex loop
    for (MTGNodeId edgeId = seed; m_graph.VSucc(edgeId) != edgeId; )
        edgeId = m_graph.YankEdgeFromVertex(edgeId);

    // reconstitute the vertex loop in order
    for (size_t i = 1; i < incidentEdges.size(); ++i)
        m_graph.VertexTwist(incidentEdges[i - 1].second, incidentEdges[i].second);
    }

void CollectDanglingChains(bvector<ICurvePrimitivePtr>& linestrings, bvector<MTGNodeId> const& danglerVertices, MTGMask danglerMask, MTGMask visitedMask, bool markEdgeFractions)
    {
    auto isChain = [&](MTGNodeId nodeId) -> bool { return m_graph.GetMaskAt(nodeId, danglerMask); };
    auto isVisited = [&](MTGNodeId nodeId) -> bool { return m_graph.GetMaskAt(nodeId, visitedMask); };
    for (auto const& seed: danglerVertices)
        {
        MTGARRAY_VERTEX_LOOP(nodeId, &m_graph, seed)
            {
            if (!isVisited(nodeId))
                {
                ClearChain();
                MTGNodeId chainNode = nodeId;
                AppendOutputVertex(chainNode);
                do
                    {
                    m_graph.SetMaskAroundEdge(chainNode, visitedMask);
                    chainNode = m_graph.FSucc(chainNode);
                    AppendOutputVertex(chainNode);
                    } while (!isVisited(chainNode) && isChain(chainNode) && m_graph.VSucc(chainNode) != chainNode);

                auto chain = ChainToCurvePrimitive(2, markEdgeFractions);
                if (chain.IsValid())
                    linestrings.push_back(chain);
                }
            }
        MTGARRAY_END_VERTEX_LOOP(nodeId, &m_graph, seed)
        }
    }

void CollectLoopsAtVertex(bvector<ICurvePrimitivePtr>& linestrings, MTGNodeId seed, MTGMask danglerMask, MTGMask visitedMask, bool markEdgeFractions)
    {
    auto isChain = [&](MTGNodeId nodeId) -> bool { return m_graph.GetMaskAt(nodeId, danglerMask); };
    auto isVisited = [&](MTGNodeId nodeId) -> bool { return m_graph.GetMaskAt(nodeId, visitedMask); };
    MTGARRAY_VERTEX_LOOP(nodeId, &m_graph, seed)
        {
        if (!isVisited(nodeId))
            {
            BeAssert(!isChain(nodeId));

            ClearChain();
            MTGNodeId loopNode = nodeId;
            AppendOutputVertex(loopNode);
            do
                {
                m_graph.SetMaskAroundEdge(loopNode, visitedMask);
                loopNode = m_graph.FSucc(loopNode);
                loopNode = m_graph.FindUnmaskedAroundVertexPred(loopNode, danglerMask); // skip danglers at the far vertex
                AppendOutputVertex(loopNode);
                } while (!isVisited(loopNode) && loopNode != nodeId);

            BeAssert(loopNode == nodeId);

            auto loop = ChainToCurvePrimitive(3, markEdgeFractions);
            if (loop.IsValid())
                linestrings.push_back(loop);
            }
        }
    MTGARRAY_END_VERTEX_LOOP(nodeId, &m_graph, seed)
    };

CurveVectorPtr OutputChainsAsCurveVector(bvector<ICurvePrimitivePtr> const& linestrings, bool hasDanglers, bool formRegions)
    {
    CurveVectorPtr result;
    if (linestrings.empty())
        return result;
    bool resultIsRegion = !hasDanglers && formRegions;
    if (linestrings.size() == 1)
        {
        // The linestrings are primitives that don't care whether they are open or closed.
        // Closure arises if they are placed in parents that announce closure ...
        auto boundaryType = resultIsRegion ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Open;
        result = CurveVector::Create(linestrings[0], boundaryType);
        }
    else if (resultIsRegion)
        {
        result = CurveVector::Create(CurveVector::BOUNDARY_TYPE_ParityRegion);
        for (size_t i = 0; i < linestrings.size(); i++)
            {
            CurveVectorPtr child = CurveVector::Create(linestrings[i], CurveVector::BOUNDARY_TYPE_Outer);
            result->push_back(ICurvePrimitive::CreateChildCurveVector_SwapFromSource(*child));
            }
        }
    else
        {
        // we have a mix of loops and chains; collect all the loops into a region
        result = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
        auto region = formRegions ? CurveVector::Create(CurveVector::BOUNDARY_TYPE_ParityRegion) : nullptr;
        for (auto const& linestring : linestrings)
            {
            auto pts = linestring->GetLineStringCP();
            if (region.IsValid() && pts && pts->front().AlmostEqual(pts->back()) && pts->size() > 3)
                {
                CurveVectorPtr child = CurveVector::Create(linestring, CurveVector::BOUNDARY_TYPE_Outer);
                region->push_back(ICurvePrimitive::CreateChildCurveVector_SwapFromSource(*child));
                }
            else
                {
                result->push_back(linestring);
                }
            }
        // if we found a region, insert it at front
        if (region.IsValid() && !region->empty())
            {
            if (1 == region->size())
                region->SetBoundaryType(CurveVector::BOUNDARY_TYPE_Outer);
            result->push_back(ICurvePrimitive::CreateChildCurveVector_SwapFromSource(*region));
            result->SwapAt(0, result->size() - 1);
            }
        }
    return result;
    }

// The input mesh is typically not well-behaved (gaps, non-manifold topology, duplicate faces,
// self-intersects), so this method tries really hard to compensate.
// Optional refVector (e.g., cutplane normal) is used when formRegions=true for sorting nodes.
CurveVectorPtr CollectChains(bool formRegions = false, bool markEdgeFractions = false, DVec3dCP refVector = nullptr)
    {
    if (!m_graph.GetActiveNodeCount())
        return nullptr;

    bvector<MTGNodeId> vertexSeeds;
    m_graph.CollectVertexLoops (vertexSeeds);

    // Step 1: fill gaps to eliminate dangling chains (to improve loop collection)
    if (formRegions && refVector)
        FillGapsInLoops(vertexSeeds, *refVector);

    // Step 2: mark remaining dangling chains and remember their starts. We'll ignore them during loop collection.
    MTGMask danglerMask = m_graph.GrabMask();
    m_graph.ClearMask(danglerMask);
    bvector<MTGNodeId> danglerVertices = MarkDanglingChains(vertexSeeds, danglerMask);

    // Step 3: sort the edges around pinch vertices, which lie on multiple loops
    bvector<MTGNodeId> pinchVertices;
    if (formRegions && refVector)
        {
        for (auto const& seed: vertexSeeds)
            {
            size_t numThisVertex = m_graph.CountUnmaskedAroundVertex(seed, danglerMask);
            if (numThisVertex % 2 == 0 && numThisVertex > 2)
                {
                pinchVertices.push_back(seed);
                SortVertexLoop(seed, *refVector);
                }
            }
        }

    // Step 4: collect chains
    bvector<ICurvePrimitivePtr> linestrings;
    MTGMask visitedMask = m_graph.GrabMask();
    m_graph.ClearMask(visitedMask);
    CollectDanglingChains(linestrings, danglerVertices, danglerMask, visitedMask, markEdgeFractions);

    // Step 5: collect loops at pinch vertices
    if (formRegions)
        {
        for (auto& seed : pinchVertices)
            {
            // start at visited node (if present) to preserve parity
            MTGNodeId visitedSeed = m_graph.FindMaskAroundVertex(seed, visitedMask);
            if (MTG_NULL_NODEID != visitedSeed)
                seed = visitedSeed;

            CollectLoopsAtVertex(linestrings, seed, danglerMask, visitedMask, markEdgeFractions);
            }
        }

    // Step 6: collect remaining loops
    for (auto& seed : vertexSeeds)
        CollectLoopsAtVertex(linestrings, seed, danglerMask, visitedMask, markEdgeFractions);

    bool hasDanglers = danglerVertices.size() > 0;
    CurveVectorPtr result = OutputChainsAsCurveVector(linestrings, hasDanglers, formRegions);

    m_graph.DropMask(visitedMask);
    m_graph.DropMask(danglerMask);
    return result;
    }
};

GEOMDLLIMPEXP CurveVectorPtr PolyfaceQuery::PlaneSlice (DPlane3dCR sectionPlane, bool formRegions, bool markEdgeFractions) const
    {
    // This polyface only gets its point array built up ... (The PolyfaceCoordinateMap addresses them)
    PolyfaceHeaderPtr section = PolyfaceHeader::CreateVariableSizeIndexed ();
    PolyfaceCoordinateMapPtr sectionMap = PolyfaceCoordinateMap::New (*section.get ());
    SectionGraph chainGraph (sectionMap);
    chainGraph.AddPlaneSectionCut (*this, *sectionMap, sectionPlane);
    chainGraph.JoinEdges ();
    chainGraph.PurgeDuplicates ();
    return chainGraph.CollectChains(formRegions, markEdgeFractions, &sectionPlane.normal);
    }


CurveVectorPtr PolyfaceQuery::DrapeLinestring (bvector<DPoint3d> &spacePoints, DVec3dCR direction) const
    {
    // This polyface only gets its point array built up ... (The PolyfaceCoordinateMap addresses them)
    PolyfaceHeaderPtr section = PolyfaceHeader::CreateVariableSizeIndexed ();
    PolyfaceCoordinateMapPtr sectionMap = PolyfaceCoordinateMap::New (*section.get ());
    SectionGraph chainGraph (sectionMap);
    chainGraph.AddDrapedLinestring (*this, *sectionMap, spacePoints, direction);
    chainGraph.JoinEdges ();
    chainGraph.PurgeDuplicates ();
    return chainGraph.CollectChains();
    }
END_BENTLEY_GEOMETRY_NAMESPACE