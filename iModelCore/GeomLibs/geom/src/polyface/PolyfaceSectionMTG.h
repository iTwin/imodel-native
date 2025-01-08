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

void JoinEdges ()
    {
    MTGARRAY_SET_LOOP (nodeA, &m_graph)
        {
        if (auto data = GetVertexData(nodeA))
            {
            MTGNodeId nodeB = FindOrSetPriorNodeAtVertex (data->m_vertexIndex, nodeA);
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

void AddGapEdge(MTGNodeId danglerNodeId, size_t destVertexIndex)
    {
    MTGNodeId destNodeId = FindPriorNodeAtVertex(destVertexIndex);
    int startVertexDataIndex, destVertexDataIndex;
    if (m_graph.IsValidNodeId(danglerNodeId) && m_graph.IsValidNodeId(destNodeId) &&
        m_graph.TryGetLabel(danglerNodeId, m_vertexDataLabel, startVertexDataIndex) &&
        m_graph.TryGetLabel(destNodeId, m_vertexDataLabel, destVertexDataIndex))
        {
        MTGNodeId nodeA, nodeB;
        m_graph.CreateEdge(nodeA, nodeB);
        m_graph.TrySetLabel(nodeA, m_vertexDataLabel, startVertexDataIndex);
        m_graph.TrySetLabel(nodeB, m_vertexDataLabel, destVertexDataIndex);
        m_graph.VertexTwist(nodeA, danglerNodeId);
        m_graph.VertexTwist(nodeB, destNodeId);
        }
    }

std::map<std::pair<size_t, size_t>, MTGNodeId> m_uniqueEdges;

// @param edgeMask applied to both sides of the new edge
void AddEdge (VertexData& vertexA, VertexData& vertexB, MTGMask edgeMask = MTG_NULL_MASK)
    {
    // avoid duplicate or trivial edge
    auto iA = vertexA.m_vertexIndex = m_coordinateMap->AddPoint(vertexA.mXYZ);
    auto iB = vertexB.m_vertexIndex = m_coordinateMap->AddPoint(vertexB.mXYZ);
    if (iA == iB)
        return;
    auto edge = (iA < iB) ? std::pair<size_t, size_t>(iA, iB) : std::pair<size_t, size_t>(iB, iA);
    auto found = m_uniqueEdges.find(edge);
    if (m_uniqueEdges.end() == found)
        {
        MTGNodeId nodeA, nodeB;
        // Make an edge...  just hanging out there.
        m_graph.CreateEdge (nodeA, nodeB);
        m_graph.TrySetLabel (nodeA, m_vertexDataLabel, (int)AddVertexData (vertexA));
        m_graph.TrySetLabel (nodeB, m_vertexDataLabel, (int)AddVertexData (vertexB));
        found = m_uniqueEdges.insert({edge, nodeA}).first;
        }
    // set mask on new edge, or transfer it to existing edge
    if (edgeMask)
        m_graph.SetMaskAroundEdge(found->second, edgeMask);
    }

void AddEdge (size_t readIndex, DPoint3dCR xyzA, DPoint3dCR xyzB)
    {
    auto vdA = VertexData::FromReadIndexAndCoordinates(readIndex, xyzA);
    auto vdB = VertexData::FromReadIndexAndCoordinates(readIndex, xyzB);
    return AddEdge(vdA, vdB);
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
    MTGMask onPlaneMask = (n0 == m_edges.size()) ? MTG_SECTION_EDGE_MASK : MTG_NULL_MASK;

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
            AddEdge (m_edges[i].m_dataA, m_edges[i].m_dataB, onPlaneMask);
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
    if (m_graph.TryGetLabel(nodeId, m_vertexDataLabel, vertexDataIndex) && vertexDataIndex >= 0 && (size_t) vertexDataIndex < m_allVertexData.size())
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
        auto danglerData = GetVertexData(seed);
        auto fSuccData = GetVertexData(m_graph.FSucc(seed));
        if (!danglerData || !fSuccData)
            continue;
        DPoint3dCR danglerVertex = danglerData->mXYZ;
        size_t danglerVertexIndex = danglerData->m_vertexIndex;
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
    if (m_graph.CountNodesAroundVertex(seed) <= 2)
        return;
    auto nearVertexData = GetVertexData(seed);
    if (!nearVertexData)
        return;
    auto farVertexData = GetVertexData(m_graph.FSucc(seed));
    if (!farVertexData)
        return;
    DVec3d edgeDir0 = DVec3d::FromStartEnd(nearVertexData->mXYZ, farVertexData->mXYZ);

    bvector<bpair<double, MTGNodeId>> incidentEdges; // radian angle in [0, 2pi)
    MTGARRAY_VERTEX_LOOP(edgeId, &m_graph, seed)
        {
        farVertexData = GetVertexData(m_graph.FSucc(edgeId));
        if (!farVertexData)
            return;
        double angle = Angle::AdjustToSweep(edgeDir0.SignedAngleTo(DVec3d::FromStartEnd(nearVertexData->mXYZ, farVertexData->mXYZ), refVector), 0.0, msGeomConst_2pi);
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

    std::sort(incidentEdges.begin(), incidentEdges.end(), [](auto const& e0, auto const& e1) { return e0.first < e1.first; });

    // destroy the vertex loop
    for (MTGNodeId edgeId = seed; m_graph.VSucc(edgeId) != edgeId; )
        edgeId = m_graph.YankEdgeFromVertex(edgeId);

    // reconstitute the vertex loop in order: since edge i dangles, it is always inserted in the loop after edge i-1.
    for (size_t i = 1; i < incidentEdges.size(); ++i)
        m_graph.VertexTwist(incidentEdges[i].second, incidentEdges[i - 1].second);
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

// Collect the super-face at the given seed node and mark it with the given `visitMask`.
// @param superFace [out] nodes around the super-face. This method returns true if and only if the first and last nodes are equal.
// @param seed [in] starting node for the super-face
// @param avoidMask [in] super-faces follow fSucc pointers where the next edge at a vertex is the first vPred without this mask
// @param visitMask [in] the collected super-face will be marked with this mask
// @param visitBothSides [in] whether to mark both sides of the super-face edges with `visitMask`, or just the side containing `seed`
// @returns Whether a valid super-face was collected.
bool CollectAndVisitSuperFace(bvector<MTGNodeId>& superFace, MTGNodeId seed, MTGMask avoidMask, MTGMask visitMask, bool visitBothSides)
    {
    superFace.clear();
    seed = m_graph.FindUnmaskedAroundVertexPred(seed, avoidMask | visitMask);
    if (MTG_NULL_NODEID == seed)
        return false;
    superFace.push_back(seed);
    MTGNodeId loopNode = seed;
    size_t maxIters = m_graph.GetActiveNodeCount() / 2  + 1; // insurance
    size_t iter = 0;
    do
        {
        if (visitBothSides)
            m_graph.SetMaskAroundEdge(loopNode, visitMask);
        else
            m_graph.SetMaskAt(loopNode, visitMask);
        loopNode = m_graph.FindUnmaskedAroundVertexPred(m_graph.FSucc(loopNode), avoidMask);
        if (MTG_NULL_NODEID == loopNode)
            {
            BeAssert(!"Unexpected end to superFace!");
            return false;   // return the edges we found so far
            }
        superFace.push_back(loopNode);
        } while (loopNode != seed && ++iter < maxIters);
    return loopNode == seed;
    }

// Mark exterior super-faces in the graph if they are unambiguous.
// @param avoidMask [in] super-faces follow fSucc pointers where the next edge at a vertex is the first vPred without this mask
// @param mask [in] what exterior faces are marked with (e.g., visitMask)
// @param planeNormal [in] the plane in which signed xy-areas are computed. In this plane, exterior and interior
//        faces have xy-areas with opposite sign.
// @param graphIsReversed [out] on a true return, whether the graph's interior face loops are CW with respect to planeNormal
// @returns Whether exterior face loop(s) were marked with the given mask. If false, exterior faces could not be
//        unambiguously determined and no face loops were marked.
bool MarkExteriorSuperFaces(MTGMask avoidMask, MTGMask mask, DVec3dCR planeNormal, bool& graphIsReversed)
    {
    // NOTE: MTG doesn't carry coordinates, so it has no area support like VU. Here we know the graph is planar, so we
    // implement bespoke signed area support (without parity sweep).
    typedef bvector<MTGNodeId> SuperFace;
    typedef std::pair<double, SuperFace> SignedArea;

    graphIsReversed = false;
    bvector<MTGNodeId> superFace;
    bvector<DPoint3d> polygon;
    bvector<SignedArea> superFaceAreas;
    double maxAbsArea = 0.0;

    auto toLocal = RotMatrix::From1Vector(planeNormal, 2, true);
    toLocal.Transpose();

    auto myVisitMask = m_graph.GrabMask();
    m_graph.ClearMask(myVisitMask);
    MTGARRAY_SET_LOOP(nodeId, &m_graph)
        {
        if (!m_graph.HasMaskAt(nodeId, myVisitMask))
            {
            if (CollectAndVisitSuperFace(superFace, nodeId, avoidMask, myVisitMask, false))
                {
                superFace.pop_back(); // don't need wraparound edge
                polygon.clear();
                for (auto const& edge : superFace)
                    {
                    if (auto data = GetVertexData(edge))
                        polygon.push_back(data->mXYZ);
                    }
                toLocal.Multiply(polygon, polygon);
                double area = PolygonOps::AreaXY(polygon);
                superFaceAreas.push_back({area, superFace});
                if (maxAbsArea < fabs(area))
                    maxAbsArea = fabs(area);
                }
            }
        }
    MTGARRAY_END_SET_LOOP(nodeId, &m_graph)
    m_graph.DropMask(myVisitMask);

    double areaTol = this->m_coordinateMap->GetXYZAbsTol();
    double smallestArea = areaTol * areaTol;
    areaTol = smallestArea + this->m_coordinateMap->GetXYZRelTol() * maxAbsArea;

    // after sort, the most negative area face is first
    std::sort(superFaceAreas.begin(), superFaceAreas.end(), [](auto const& a, auto const& b) -> bool { return a.first < b.first; });
    double absArea0 = fabs(superFaceAreas.front().first);
    double absArea1 = fabs(superFaceAreas.back().first);
    graphIsReversed = absArea0 < absArea1;
    if (fabs(absArea0 - absArea1) <= areaTol)
        {
        // no obvious exterior face; break ties by counting signed areas of nontrivial faces
        // HEURISTIC: usually there will be more positive area (interior) faces than negative (exterior).
        size_t numNegative = 0, numPositive = 0;
        std::for_each(superFaceAreas.begin(), superFaceAreas.end(), [&](auto const& a) { if (fabs(a.first) < smallestArea) return; if (a.first < 0) ++numNegative; else ++numPositive; });
        if (numNegative == numPositive)
            return false;   // exterior faces are ambiguous; possibly all face loops are disjoint and have equal area.
        graphIsReversed = numPositive < numNegative;
        }

    // largest face is unambiguous, but if it has positive signed area, the graph's parity is reversed
    if (graphIsReversed)
        {
        std::for_each(superFaceAreas.begin(), superFaceAreas.end(), [](auto& a) { a.first *= -1.0; });
        std::reverse(superFaceAreas.begin(), superFaceAreas.end());
        }

    // negative area (exterior) faces are frontloaded: mark them
    for (auto const& faceArea : superFaceAreas)
        {
        if (faceArea.first >= smallestArea)
            break;
        for (auto const& edge : faceArea.second)
            m_graph.SetMaskAt(edge, mask);
        }
    return true;
    }

void CollectLoopsAtVertex(bvector<ICurvePrimitivePtr>& linestrings, MTGNodeId seed, MTGMask danglerMask, MTGMask visitedMask, MTGMask onPlaneMask, bool markEdgeFractions, bool visitBothSides)
    {
    bvector<MTGNodeId> superFace;
    auto isVisited = [&](MTGNodeId nodeId) -> bool { return m_graph.GetMaskAt(nodeId, visitedMask); };
    auto isSuperFaceOnSection = [&]() -> bool { return onPlaneMask && superFace.size() == std::count_if(superFace.begin(), superFace.end(), [&](auto n) -> bool { return m_graph.GetMaskAt(n, onPlaneMask); }); };
    MTGARRAY_VERTEX_LOOP(nodeId, &m_graph, seed)
        {
        if (!isVisited(nodeId))
            {
            BeAssert(!m_graph.GetMaskAt(nodeId, danglerMask));  // expect all chains to be visited already
            ClearChain();
            bool haveLoop = CollectAndVisitSuperFace(superFace, nodeId, danglerMask, visitedMask, visitBothSides);
            BeAssert(visitBothSides || haveLoop);
            if (haveLoop && isSuperFaceOnSection())
                {
                ; // skip a loop that came from a facet on the clip plane; these can lead to overlapping/duplicate output facets
                }
            else
                {
                for (auto const& edge : superFace)
                    AppendOutputVertex(edge);
                auto linestring = ChainToCurvePrimitive(3, markEdgeFractions);
                if (linestring.IsValid())
                    linestrings.push_back(linestring);
                }
            }
        }
    MTGARRAY_END_VERTEX_LOOP(nodeId, &m_graph, seed)
    };

CurveVectorPtr OutputChainsAsCurveVector(bvector<ICurvePrimitivePtr> const& linestrings, bool hasDanglers, bool formRegions, bool reverseLoops)
    {
    CurveVectorPtr result;
    if (linestrings.empty())
        return result;

    auto IsLoop = [](ICurvePrimitiveCR curve) -> bool { auto pts = curve.GetLineStringCP(); return pts && pts->front().AlmostEqual(pts->back()) && pts->size() > 3; };
    if (formRegions && reverseLoops)
        std::for_each(linestrings.begin(), linestrings.end(), [&](auto& ls) { if (IsLoop(*ls)) ls->ReverseCurvesInPlace(); });

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
            if (region.IsValid() && IsLoop(*linestring))
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
                region = region->at(0)->GetChildCurveVectorP(); // promote parity region to outer loop
            result->push_back(ICurvePrimitive::CreateChildCurveVector_SwapFromSource(*region));
            result->SwapAt(0, result->size() - 1);
            }
        }
    return result;
    }

// The input mesh is typically not well-behaved (gaps, non-manifold topology, duplicate faces, self-intersects), so this method tries really hard to compensate.
// @param [in] formRegions whether to construct a region from any loops assembled from section edges. Default false.
// @param [in] markEdgeFractions whether to populate FacetEdgeLocationDetailVector on each returned CurvePrimitive. Default false.
// @param [in] skipOnPlaneFacets whether output lacks facets coplanar with sectionPlane. Default is false (include them).
// @param [in] refVector when formRegions is true, this optional vector is used for sorting nodes and computing signed areas. Typically this is the cutplane normal. Default nullptr.
CurveVectorPtr CollectChains(bool formRegions = false, bool markEdgeFractions = false, bool skipOnPlaneFacets = false, DVec3dCP refVector = nullptr)
    {
    if (!m_graph.GetActiveNodeCount())
        return nullptr;

    MTGMask onPlaneMask = skipOnPlaneFacets ? MTG_SECTION_EDGE_MASK : MTG_NULL_MASK;
    bool collectLoops = formRegions && refVector;
    auto const& vertexSeeds = m_anyNodeAtVertex;

    // Step 1: add edges to eliminate dangling chains. This improves loop collection.
    if (collectLoops)
        FillGapsInLoops(vertexSeeds, *refVector);

    // Step 2: mark remaining dangling chains and remember their starts. We'll ignore them during loop collection.
    MTGMask danglerMask = m_graph.GrabMask();
    m_graph.ClearMask(danglerMask);
    bvector<MTGNodeId> danglerVertices = MarkDanglingChains(vertexSeeds, danglerMask);

    // Step 3: sort the vertex loops now that we've added all the edges. This avoids collecting overlapping loops.
    if (collectLoops)
        {
        for (auto const& seed : vertexSeeds)
            SortVertexLoop(seed, *refVector);
        }

    // Step 4: collect chains
    bvector<ICurvePrimitivePtr> linestrings;
    MTGMask visitedMask = m_graph.GrabMask();
    m_graph.ClearMask(visitedMask);
    CollectDanglingChains(linestrings, danglerVertices, danglerMask, visitedMask, markEdgeFractions);

    // Step 5: mark exterior loops
    // We collected dangling chains by marking both sides of an edge as visited. But when we collect loops, marking
    // both sides would prevent us from collecting two complete adjacent loops. So we have to mark only one side of
    // each loop, but this means exterior loops would be collected, which we don't want. Therefore, here we mark
    // exterior loops if we can distinguish them; then we can proceed in the next steps to collect single-sided loops,
    // which will all be interior loops. If we can't decide which loops are exterior, we fall back to visiting both
    // sides of each edge, which means collecting adjacent loops may result in open chain(s).
    bool visitBothSides = true;
    bool graphIsReversed = false;
    if (collectLoops && m_graph.CountFaceLoops() > 2)
        visitBothSides = !MarkExteriorSuperFaces(danglerMask, visitedMask, *refVector, graphIsReversed);

    // Step 6: collect remaining loops
    for (auto& seed : vertexSeeds)
        CollectLoopsAtVertex(linestrings, seed, danglerMask, visitedMask, onPlaneMask, markEdgeFractions, visitBothSides);

    bool hasDanglers = danglerVertices.size() > 0;
    CurveVectorPtr result = OutputChainsAsCurveVector(linestrings, hasDanglers, formRegions, graphIsReversed);

    m_graph.DropMask(visitedMask);
    m_graph.DropMask(danglerMask);
    return result;
    }
};

CurveVectorPtr PolyfaceQuery::PlaneSlice (DPlane3dCR sectionPlane, bool formRegions, bool markEdgeFractions, bool skipOnPlaneFacets) const
    {
    // This polyface only gets its point array built up ... (The PolyfaceCoordinateMap addresses them)
    PolyfaceHeaderPtr section = PolyfaceHeader::CreateVariableSizeIndexed ();
    PolyfaceCoordinateMapPtr sectionMap = PolyfaceCoordinateMap::New (*section.get ());
    SectionGraph chainGraph (sectionMap);
    chainGraph.AddPlaneSectionCut (*this, *sectionMap, sectionPlane);
    chainGraph.JoinEdges ();
    return chainGraph.CollectChains(formRegions, markEdgeFractions, skipOnPlaneFacets, &sectionPlane.normal);
    }

CurveVectorPtr PolyfaceQuery::DrapeLinestring (bvector<DPoint3d> &spacePoints, DVec3dCR direction) const
    {
    // This polyface only gets its point array built up ... (The PolyfaceCoordinateMap addresses them)
    PolyfaceHeaderPtr section = PolyfaceHeader::CreateVariableSizeIndexed ();
    PolyfaceCoordinateMapPtr coordMap = PolyfaceCoordinateMap::New (*section.get ());
    SectionGraph chainGraph (coordMap);
    chainGraph.AddDrapedLinestring (*this, *coordMap, spacePoints, direction);
    chainGraph.JoinEdges ();
    return chainGraph.CollectChains();
    }
END_BENTLEY_GEOMETRY_NAMESPACE