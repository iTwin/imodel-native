/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "mtgintrn.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

template <typename T>
static void InitializeIndexFromNodes(MTGGraph &graph, bvector <T> &index, T defaultValue)
    {
    size_t numNodes = graph.GetNodeIdCount();
    index.clear ();
    for (size_t i = 0; i < numNodes; i++)
        {
        index.push_back (defaultValue);
        }
    }
static DPoint3d ConditionalXYZ(bvector<DPoint3d> const &xyz, int indexA, int indexB, DPoint3dCR defaultXYZ)
    {
    if (indexA >= 0 && indexA < xyz.size ())
        return xyz[(size_t)indexA];
    if (indexB >= 0 && indexB < xyz.size())
        return xyz[(size_t)indexB];
    return defaultXYZ;
    }
// Static methods for DooSabin subdivision.
// DS is:
// <ul>
// <li> Each face shrinks to a new face with same vertex count.
// <li> New face for each edge
// <li> New face at each vertex
// <li> Each sector of old graph:
//   <ul>
//   <li> owns one new vertex
//   <li> the new vertex coordinates are computed from only the face of the original sector.
//   <li> owns 2 new edges:
//      <ul>
//      <li> Along its original outbound edge, a parallel edge of the shrunken face
//      <li> Perpendicular to that original edge, a crossing edge.
//      </ul>
//   </ul>
// </ul>
struct DooSabin
{
MTGFacets oldFacets;
MTGFacets newFacets;
static DPoint3d AverageVertexCoordinatesAroundFace (MTGFacets &facets, MTGNodeId seedNode)
    {
    DPoint3dWeightedSum sum;
    DPoint3d xyz;
    MTGGraph *graph = &facets.graphHdr;
    MTGARRAY_FACE_LOOP(faceNode, graph, seedNode)
        {
        facets.NodeToVertexCoordinates(faceNode, xyz);
        sum.AddUnitWeight(xyz);
        }
    MTGARRAY_END_FACE_LOOP(faceNode, graph, seedNode)
        DPoint3d centroid;
    sum.GetWeightedAverage(centroid);
    return centroid;
    }
public:
static DPoint3d NewXYZInSector(MTGFacets &facets, MTGNodeId nodeB)
    {
    MTGGraph *graph = &facets.graphHdr;
    DPoint3d centroid, xyzA, xyzB, xyzC, xyz;
    // B is the sector node
    // A is its fpred.
    // C is its fsucc.
    MTGNodeId nodeA = graph->FPred (nodeB);
    MTGNodeId nodeC = graph->FSucc (nodeB);

    // centroid is simple average of all vertices around the face
    // edge points are simple average of its 2 vertices
    // sector point is simple average of original and those 2.
    DPoint3dWeightedSum sum;
    centroid.Zero ();
    facets.NodeToVertexCoordinates(nodeA, xyzA);
    facets.NodeToVertexCoordinates(nodeB, xyzB);
    facets.NodeToVertexCoordinates(nodeC, xyzC);
    DPoint3d edgeAB = DPoint3d::FromInterpolate (xyzA, 0.5, xyzB);
    DPoint3d edgeBC = DPoint3d::FromInterpolate(xyzB, 0.5, xyzC);
    centroid = AverageVertexCoordinatesAroundFace(facets, nodeB);
    sum.Zero ();
    sum.AddUnitWeight (centroid);
    sum.AddUnitWeight(edgeAB);
    sum.AddUnitWeight(edgeBC);
    sum.AddUnitWeight(xyzB);
    sum.GetWeightedAverage (xyz);
    return xyz;
    }
static double DistanceBetweenEdges(MTGFacets &facetsA, MTGNodeId nodeA0, MTGFacets &facetsB, MTGNodeId nodeB0)
    {
    MTGNodeId nodeA1 = facetsA.graphHdr.EdgeMate (nodeA0);
    MTGNodeId nodeB1 = facetsB.graphHdr.EdgeMate(nodeB0);
    DSegment3d segmentA, segmentB;

    facetsA.NodeToVertexCoordinates(nodeA0, segmentA.point[0]);
    facetsA.NodeToVertexCoordinates(nodeA1, segmentA.point[1]);

    facetsB.NodeToVertexCoordinates(nodeB0, segmentB.point[0]);
    facetsB.NodeToVertexCoordinates(nodeB1, segmentB.point[1]);

    double fractionA, fractionB;
    DPoint3d pointA, pointB;
    DSegment3d::ClosestApproachBounded (fractionA, fractionB, pointA, pointB, segmentA, segmentB);
    return pointA.Distance (pointB);
    }

// Add subdivision facets to newFacets.
// maxChange is measured from new cross edges to old edge.
void BuildNewFacets (double &maxChange)
    {
    maxChange = 0.0;
    MTGGraph *oldGraph = &oldFacets.graphHdr;
    MTGGraph *newGraph = &newFacets.graphHdr;
    MTGGraph::ScopedMask visitMask (*oldGraph);
    // for each oldNode, oldSectorToNewInteriorFace[oldNode] will be:
    //    .. if oldNode is exterior: MTG_NULL_NODEID
    //    .. otherwise: the corresponding INSIDE node of the new face.
    // Step 1: make them all null:
    bvector<MTGNodeId> oldSectorToNewInteriorFace;
    MTGARRAY_SET_LOOP (nodeA, oldGraph)
        {
        oldSectorToNewInteriorFace.push_back (MTG_NULL_NODEID);
        }
    MTGARRAY_END_SET_LOOP(nodeA, oldGraph)
    // Step 2: make each new face 
    MTGARRAY_SET_LOOP (oldFaceSeed, oldGraph)
        {
        if (!visitMask.IsSetAt (oldFaceSeed)
            && !oldGraph->HasMaskAt(oldFaceSeed, MTG_EXTERIOR_MASK))
            {
            visitMask.SetAroundFace (oldFaceSeed);
            MTGNodeId newFaceSeed = MTG_NULL_NODEID;    // 
            MTGARRAY_FACE_LOOP(oldNodeB, oldGraph, oldFaceSeed)
                {
                DPoint3d xyz = NewXYZInSector (oldFacets, oldNodeB);
                MTGNodeId newNodeB, newNodeB1;  // inside and outside in new face.
                newGraph->SplitEdge (newNodeB, newNodeB1, newFaceSeed);
                newFaceSeed = newNodeB;
                oldSectorToNewInteriorFace[oldNodeB] = newNodeB;
                int newPointIndex = (int) newFacets.AddPoint (xyz);
                newGraph->TrySetLabel(newNodeB, newFacets.vertexLabelOffset, newPointIndex);
                newGraph->TrySetLabel(newNodeB1, newFacets.vertexLabelOffset, newPointIndex);
                }
            MTGARRAY_END_FACE_LOOP(oldNodeB, oldGraph, oldFaceSeed)
            }
        }
    MTGARRAY_END_SET_LOOP (oldFaceSeed, oldGraph)
    // Step 3: add cross edges.
    // For each node:
    //     EXTERIOR --> do nothing
    //     INTERIOR with INTERIOR MATE --> build cross edge
    //     INTERIOR with EXTERIOR MATE --> mark new parallel edge as exterior.
    MTGARRAY_SET_LOOP(oldNodeA, oldGraph)
        {
        MTGNodeId oldMateA = oldGraph->EdgeMate (oldNodeA);
        if (oldGraph->HasMaskAt(oldNodeA, MTG_EXTERIOR_MASK))
            {

            }
        else if (oldGraph->HasMaskAt (oldMateA, MTG_EXTERIOR_MASK))
            {
            MTGNodeId newNodeA = oldSectorToNewInteriorFace[oldNodeA];    // inside the new face
            MTGNodeId newMateA = newGraph->EdgeMate (newNodeA); // outside the new face
            newGraph->SetMaskAt(newNodeA, MTG_BOUNDARY_MASK);
            newGraph->SetMaskAt (newMateA, MTG_EXTERIOR_MASK | MTG_BOUNDARY_MASK);
            }
        else
            {
            auto exteriorCount = oldGraph->CountMaskAroundVertex(oldNodeA, MTG_EXTERIOR_MASK);
            MTGNodeId oldNodeB = oldGraph->VPred (oldNodeA);
            MTGNodeId newNodeA = oldSectorToNewInteriorFace[oldNodeA];    // inside the new face
            MTGNodeId newNodeB = oldSectorToNewInteriorFace [oldNodeB];   // inside the other new face
            MTGNodeId newNodeA1 = newGraph->VPred (newNodeA);   // outside new face on A side
            MTGNodeId newNodeB1 = newGraph->VSucc (newNodeB);  // outside new face on B side
            MTGNodeId newNodeA2, newNodeB2;
            newGraph->Join (newNodeA2, newNodeB2, newNodeA1, newNodeB1, MTG_NULL_MASK, MTG_NULL_MASK);
            if (exteriorCount > 0)
                newGraph->SetMaskAt (newNodeB2, MTG_EXTERIOR_MASK);
            maxChange = DoubleOps::Max (maxChange, DistanceBetweenEdges (oldFacets, oldNodeA, newFacets, newNodeB2));
            }
        }
    MTGARRAY_END_SET_LOOP(oldNodeA, oldGraph)
    }
static PolyfaceHeaderPtr SubdividePolyface(PolyfaceQueryCR source, uint32_t maxDepth, double maxChange, int boundaryAction)
    {
    DooSabin context;
    jmdlMTGFacets_setNormalMode (&context.newFacets, MTG_Facets_VertexOnly, 0, 0);
    if (PolyfaceToMTG_FromPolyfaceConnectivity(&context.oldFacets, source))
        {
        double maxChange1;
        if (maxDepth == 0 || maxDepth > 4)
            maxDepth = 4;
        context.BuildNewFacets (maxChange1);
        for (uint32_t i = 1; i < maxDepth && maxChange1 > maxChange; i++)
            {
            std::swap (context.oldFacets, context.newFacets);
            context.newFacets.ClearGraphAndArrays();
            context.BuildNewFacets(maxChange1);
            }
        auto out = PolyfaceHeader::CreateVariableSizeIndexed();
        AddMTGFacetsToIndexedPolyface(&context.newFacets, *out, MTG_EXTERIOR_MASK, MTG_BOUNDARY_MASK);
        return out;
        }
    return nullptr;
    }
};

// Get a central point.
// method = 1 is polygon centroid
// method = other is average of all given points
bool CentralPoint(bvector<DPoint3d> const &data, int method, DPoint3dR xyzCenter)
    {
    if (data.size () == 0)
        {
        xyzCenter.Zero ();
        return false;
        }
    if (method == 1)
        {
        DVec3d normal;
        double area;
        PolygonOps::CentroidNormalAndArea(data, xyzCenter, normal, area);
        return true;
        }
    // default -- simple sum of vertices ...
    DPoint3dWeightedSum sum;
    sum.Zero ();
    sum.AddUnitWeight (data);
    return sum.GetWeightedAverage (xyzCenter);
    }

GEOMDLLIMPEXP PolyfaceHeaderPtr MTGFacets_CloneCatmullClarkSubdivision(MTGFacets &in, double &maxChange, int boundaryAction)
    {
    // For each old interior node (and its outgoing edge), index to the new parallel (shrunken) edge
    // The interior edge node is INSIDE the shrunken face
    bvector<MTGNodeId> oldNodeToPointInFace;
    bvector<MTGNodeId> oldNodeToPointAtVertex;
    bvector<MTGNodeId> oldNodeToPointOnEdge;
    MTGGraph *graph = &in.graphHdr;
    InitializeIndexFromNodes <int> (*graph, oldNodeToPointInFace, -1);
    InitializeIndexFromNodes <int>(*graph, oldNodeToPointAtVertex, -1);
    InitializeIndexFromNodes <int>(*graph, oldNodeToPointOnEdge, -1);
    PolyfaceHeaderPtr meshB = PolyfaceHeader::CreateVariableSizeIndexed ();
    bvector<int> &meshBPointIndex = meshB->PointIndex ();
    bvector<DPoint3d> &meshBPoint = meshB->Point ();
    bvector<MTGNodeId> allFaceLoops, allVertexLoops;
    graph->CollectFaceLoops (allFaceLoops);
    graph->CollectVertexLoops (allVertexLoops);
    bvector<DPoint3d> xyzAroundFace;
    DPoint3d centroid, xyzA, xyzB, xyzE;
    maxChange = 0.0;
    // step one:
    //    face centroid points (based on original facet coordinates)
    //    midEdge points (based on original facet coordinates)
    for (auto &faceSeed : allFaceLoops)
        {
        if (!graph->HasMaskAt (faceSeed, MTG_EXTERIOR_MASK))
            {
            in.NodeToVertexCoordinatesAroundFace (faceSeed, xyzAroundFace, false);
            CentralPoint (xyzAroundFace, 0, centroid);
            int centroidIndex = (int)meshBPoint.size ();
            meshBPoint.push_back (centroid);

            MTGARRAY_FACE_LOOP (nodeA, graph, faceSeed)
                {
                oldNodeToPointInFace[nodeA] = centroidIndex;
                auto nodeB = graph->FSucc(nodeA);
                in.NodeToVertexCoordinates (nodeA, xyzA);
                in.NodeToVertexCoordinates (nodeB, xyzB);
                xyzE.Interpolate (xyzA, 0.5, xyzB);
                int edgePointIndex = oldNodeToPointOnEdge[nodeA];
                if (edgePointIndex < 0)
                    {
                    auto nodeB1 = graph->EdgeMate (nodeA);
                    edgePointIndex = (int) meshBPoint.size ();
                    meshBPoint.push_back (xyzE);
                    oldNodeToPointOnEdge[nodeA] = edgePointIndex;
                    oldNodeToPointOnEdge[nodeB1] = edgePointIndex;
                    }
                }
            MTGARRAY_END_FACE_LOOP (nodeA, graph, faceSeed)
            }
        }
    // step two: vertex points
    // weighted average of face points, original midege, and original vertex
    bvector<MTGNodeId> interiorNodesAtVertex;
    DPoint3dWeightedSum xyzEdgeAverage, xyzBoundaryEdgeAverage, xyzCentroidAverage;
    DPoint3d xyzVertex;
    // For an interior vertex, the number of incident facets and edges is equal.
    // For exterior vertex, it is not.
    // So averages have to be accumulated separately.
    for (auto &vertexSeed : allVertexLoops)
        {
        interiorNodesAtVertex.clear ();
        xyzEdgeAverage.Zero ();
        xyzCentroidAverage.Zero ();
        xyzBoundaryEdgeAverage.Zero ();
        MTGARRAY_VERTEX_LOOP(nodeA, graph, vertexSeed)
            {
            if (oldNodeToPointInFace[nodeA] >= 0)
                {
                interiorNodesAtVertex.push_back (nodeA);
                xyzA = meshBPoint [oldNodeToPointInFace[nodeA]];
                xyzCentroidAverage.AddWeighted (xyzA, 1.0);
                }
            auto nodeB = graph->FSucc(nodeA);
            auto nodeB1 = graph->VSucc(nodeB);
            bool isBoundary = graph->HasMaskAt(nodeA, MTG_EXTERIOR_MASK)
                || graph->HasMaskAt(nodeB1, MTG_EXTERIOR_MASK);
            if (oldNodeToPointOnEdge [nodeA] >= 0)
                {
                xyzE = meshBPoint[oldNodeToPointOnEdge[nodeA]];
                xyzEdgeAverage.AddWeighted (xyzE, 1.0);
                if (isBoundary)
                    xyzBoundaryEdgeAverage.AddWeighted(xyzE, 1.0);
                }
            else if (oldNodeToPointOnEdge[nodeB1] >= 0)
                {
                xyzE = meshBPoint[oldNodeToPointOnEdge[nodeB1]];
                xyzEdgeAverage.AddWeighted(xyzE, 1.0);
                if (isBoundary)
                    xyzBoundaryEdgeAverage.AddWeighted(xyzE, 1.0);
                }
            }
        MTGARRAY_END_VERTEX_LOOP(nodeA, graph, vertexSeed)

        if (interiorNodesAtVertex.size () > 0)
            {
            in.NodeToVertexCoordinates(interiorNodesAtVertex.front (), xyzVertex);
            auto xyzVertexOriginal = xyzVertex;
            xyzEdgeAverage.GetWeightedAverage (xyzE);
            xyzCentroidAverage.GetWeightedAverage(centroid);
            double n = (double)interiorNodesAtVertex.size();
            double vertexWeight = (n - 3.0) / n;
            double edgeWeight = 2.0 / n;
            double centroidWeight = 1.0 / n;
            if (boundaryAction == 0)
                {
                xyzVertex = xyzVertexOriginal;
                }
            else if (xyzBoundaryEdgeAverage.HasNonZeroWeight())
                {
                // The usual on-boundary case ... only the vertex and average of boundary edges contribute
                xyzBoundaryEdgeAverage.GetWeightedAverage (xyzE);
                xyzVertex.SumOf (xyzVertex, 0.5, xyzE, 0.5);
                }
            else if (n < 3)
                xyzVertex.SumOf (xyzVertex, 0.5, xyzE, 0.5);
            else
                xyzVertex.SumOf (xyzVertex, vertexWeight, xyzE, edgeWeight, centroid, centroidWeight);
            int vertexPointIndex = (int)meshBPoint.size ();
            meshBPoint.push_back (xyzVertex);
            maxChange = DoubleOps::Max (maxChange, xyzVertexOriginal.Distance (xyzVertex));
            for (auto nodeA : interiorNodesAtVertex)
                {
                auto nodeC = graph->FPred (nodeA);
                int edgeAPointIndex = oldNodeToPointOnEdge[nodeA];
                int edgeCPointIndex = oldNodeToPointOnEdge[nodeC];
                int centroidIndex   = oldNodeToPointInFace[nodeA];
                if (centroidIndex >= 0 && edgeAPointIndex >= 0 && edgeCPointIndex >= 0)
                    {
                    meshBPointIndex.push_back (vertexPointIndex + 1);
                    meshBPointIndex.push_back(edgeAPointIndex + 1);
                    meshBPointIndex.push_back(centroidIndex + 1);
                    meshBPointIndex.push_back(edgeCPointIndex + 1);
                    meshBPointIndex.push_back (0);
                    }
                }
            }
        }
    // edges ...  the edges already index to coodinates.
    //  ... just fix the existing points up.
    MTGARRAY_SET_LOOP(nodeA, graph)
        {
        auto nodeB = graph->EdgeMate(nodeA);
        bool isBoundary = graph->HasMaskAt (nodeA, MTG_EXTERIOR_MASK)
                    || graph->HasMaskAt (nodeB, MTG_EXTERIOR_MASK);
        if (nodeA > nodeB)
            {
            int edgePointIndex = -1;
            if (oldNodeToPointOnEdge[nodeA] >= 0)
                edgePointIndex = oldNodeToPointOnEdge[nodeA];
            else if (oldNodeToPointOnEdge[nodeB] > 0)
                edgePointIndex = oldNodeToPointOnEdge[nodeB];
            if (isBoundary)
                {
                // leave the edge point at the midpoint -- no shift towards centroid
                }
            else if (edgePointIndex >= 0)
                {
                DPoint3d xyzOldMidEdge = meshBPoint[(uint32_t)edgePointIndex];
                // For an exterior edge, act as if the outside has a sliver 
                // quad whose centroid is mid-edge
                xyzA.SumOf(
                    ConditionalXYZ(meshBPoint, oldNodeToPointInFace[nodeA], -1, xyzOldMidEdge), 0.5,
                    ConditionalXYZ(meshBPoint, oldNodeToPointInFace[nodeB], -1, xyzOldMidEdge), 0.5);
                DPoint3d xyzNewMidEdge = DPoint3d::FromSumOf(xyzA, 0.5, xyzOldMidEdge, 0.5);
                meshBPoint[(uint32_t)edgePointIndex] = xyzNewMidEdge;
                }
            }

        }
    MTGARRAY_END_SET_LOOP(nodeA, graph)
    return meshB;
    }

struct LoopSubdivision
{
// Return the weight factor for a single far vertex for vertex averaging.
static double VertexBetaFactor(double n)
    {
    double divN = 1.0 / n;
    static int s_option = 1;
    if (s_option == 1)
        {
        double q = (3.0 + 2.0 * cos(msGeomConst_2pi * divN));
        return (0.625 - q * q / 64.0) / n;
        }
    else
        {
        if (n > 3)
            return 3.0 / (8.0 * n);
        else
            return 3.0 / 16.0;
        }
    }
};

struct SubdivisionData
    {
    MTGFacets m_mtgFacets;
    bvector< DPoint3dSizeSize> m_newPoints;
    PolyfaceQueryCR m_mesh;
    SubdivisionData (PolyfaceQueryCR mesh) : m_mesh (mesh) {}

    bool InitMTG()
        {
        if (PolyfaceToMTG_FromPolyfaceConnectivity(&m_mtgFacets, m_mesh))
            {
            SetupNewPointSums();
            return true;
            }
        return false;
        }
    void SetupNewPointSums()
        {
        // Make new points with empty sums.
        m_newPoints.clear();
        for (auto &xyz : m_mtgFacets.vertexArrayHdr)
            {
            m_newPoints.push_back(DPoint3dSizeSize(xyz, 0, 0));
            }
        }
    bool ComputeVertexSums_LoopWeights (int boundaryAction)
        {
        MTGGraph * graph = &m_mtgFacets.graphHdr;
        MTGARRAY_SET_LOOP(vertexSeedNode, graph)
            {
            ptrdiff_t vertexIndex ;
            DPoint3dWeightedSum sumAllNeighbors;
            DPoint3dWeightedSum sumBoundaryNeighbors;
            // Is this an interior sector that has not had a sum computed?
            if (!graph->HasMaskAt(vertexSeedNode, MTG_EXTERIOR_MASK)
                && m_mtgFacets.NodeToVertexIndex(vertexSeedNode, vertexIndex)
                && m_newPoints[vertexIndex].GetTagA() == 0
                )
                {
                DPoint3d oldXYZ = m_mtgFacets.vertexArrayHdr[vertexIndex];
                MTGARRAY_VERTEX_LOOP(nodeA, graph, vertexSeedNode)
                    {
                    auto nodeB = graph->FSucc(nodeA);
                    auto nodeC = graph->VSucc(nodeB);
                    bool exteriorB = graph->HasMaskAt(nodeB, MTG_EXTERIOR_MASK);
                    bool exteriorC = graph->HasMaskAt(nodeC, MTG_EXTERIOR_MASK);
                    ptrdiff_t vertexIndexQ;
                    if (!exteriorB && !exteriorC)
                        {
                        // simple interior edge ...
                        m_mtgFacets.NodeToVertexIndex(nodeB, vertexIndexQ);
                        sumAllNeighbors.AddWeighted(m_mtgFacets.vertexArrayHdr[vertexIndexQ], 1.0);
                        }
                    else if (exteriorB && !exteriorC)
                        {
                        m_mtgFacets.NodeToVertexIndex(nodeC, vertexIndexQ);
                        sumAllNeighbors.AddWeighted(m_mtgFacets.vertexArrayHdr[vertexIndexQ], 1.0);
                        sumBoundaryNeighbors.AddWeighted(m_mtgFacets.vertexArrayHdr[vertexIndexQ], 1.0);
                        }
                    else if (!exteriorB && exteriorC)
                        {
                        m_mtgFacets.NodeToVertexIndex(nodeC, vertexIndexQ);
                        sumAllNeighbors.AddWeighted(m_mtgFacets.vertexArrayHdr[vertexIndexQ], 1.0);
                        sumBoundaryNeighbors.AddWeighted(m_mtgFacets.vertexArrayHdr[vertexIndexQ], 1.0);
                        }
                    }
                MTGARRAY_END_VERTEX_LOOP(nodeA, graph, vertexSeedNode)

                if (sumBoundaryNeighbors.HasNonZeroWeight())
                    {
                    if (boundaryAction == 1)
                        {
                        sumBoundaryNeighbors.DeWeightInPlace();
                        DPoint3d newXYZ = DPoint3d::FromSumOf(oldXYZ, 0.75,
                            sumBoundaryNeighbors.GetXYZ(), 0.25);
                        m_newPoints[vertexIndex].Set(newXYZ, 1);
                        }
                    else
                        {
                        m_newPoints[vertexIndex].Set (oldXYZ);
                        }
                    }
                else if (sumAllNeighbors.HasNonZeroWeight())
                    {
                    // simple interior node.
                    double n = sumAllNeighbors.GetW ();
                    double beta = LoopSubdivision::VertexBetaFactor(n);
                    // sumAllNeighbors.DeWeightInPlace();
                    DPoint3d newXYZ = DPoint3d::FromSumOf(
                        oldXYZ, 1.0 - n * beta,
                        sumAllNeighbors.GetXYZ(), beta);
                    // DPoint3d newXYZ1 = DPoint3d::FromInterpolate (oldXYZ, s_vertexFraction, newXYZ);
                    m_newPoints[vertexIndex].Set(newXYZ, 2);
                    }
                }
            }
        MTGARRAY_END_SET_LOOP(vertexSeedNode, graph)
        return true;
        }

    // Walk the face starting at nodeA.
    // Sum all vertex coordinates EXCLUDING
    //   nodeA
    //   nodeB
    //   all nodes with mask
    DPoint3dWeightedSum SumUnmaskedComplement(MTGNodeId nodeA, MTGNodeId nodeB, MTGMask mask)
        {
        DPoint3dWeightedSum sum;
        MTGGraph *graph = &m_mtgFacets.graphHdr;
        DPoint3d xyz;
        MTGARRAY_FACE_LOOP(node, graph, nodeA)
            {
            if (node != nodeA
                && node != nodeB
                && !graph->HasMaskAt(node, mask))
                {
                m_mtgFacets.NodeToVertexCoordinates(node, xyz);
                sum.AddWeighted(xyz, 1.0);
                }
            }
        MTGARRAY_END_FACE_LOOP(node, graph, nodeA)
        return sum;
        }
    // split every edge.
    // Set coordinates as mean of original verts.
    // Set m_newPoints coordinates as
    //     interior: (left=1)(before=3)(after=3)(right=1)
    //     boundary: midpoint of before and after.
    // Set mask on the split nodes.
    // assume usual vertex label
    // assume m_mtgFacets.vertexArrayHdr and m_newPoint have corresponding sizes.
    void SplitEdges_LoopWeights(MTGMask mask)
        {
        // edge weights
        static double aMid = 6.0 / 8.0;     // original midpoint -- in turn (0.5, 0.5) of ends
        static double aFar = 1.0 / 8.0;   // vertices far from edge
        MTGGraph * graph = &m_mtgFacets.graphHdr;
#ifdef PrintSmallGraphs
        static int s_smallGraph
        if (m_mtgFacets.graphHdr.GetActiveNodeCount() < 30)
            {
            jmdlMTGFacets_printFaceLoops(&m_mtgFacets);
            jmdlMTGFacets_printVertexLoops(&m_mtgFacets);
            }
#endif
        MTGNodeId nodeA1, nodeB1;
        // DPoint3d xyzA, xyzB, xyzMid;
        MTGGraph::ScopedMask visitMaskA(m_mtgFacets.graphHdr);
        MTGMask visitMask = visitMaskA.Get();
        MTGARRAY_SET_LOOP(nodeA, graph)
            {
            if (!graph->HasMaskAt(nodeA, visitMask) && !graph->HasMaskAt(nodeA, MTG_EXTERIOR_MASK))
                {
                auto nodeB = graph->EdgeMate(nodeA);
                auto nodeA2 = graph->FSucc (nodeA);
                auto nodeB2 = graph->FSucc (nodeB);

                if (graph->SplitEdge(nodeA1, nodeB1, nodeA))
                    {
                    DPoint3d xyzA, xyzB, xyzMid;
                    m_mtgFacets.NodeToVertexCoordinates(nodeA, xyzA);
                    m_mtgFacets.NodeToVertexCoordinates(nodeA2, xyzB);
                    xyzMid.Interpolate(xyzA, 0.5, xyzB);
                    auto vertexIndex = m_mtgFacets.AddPoint(xyzMid);

                    graph->SetMaskAt(nodeA1, mask);
                    graph->SetMaskAt(nodeB1, mask);
                    graph->TrySetLabel(nodeA1, m_mtgFacets.vertexLabelOffset, (int)vertexIndex);
                    graph->TrySetLabel(nodeB1, m_mtgFacets.vertexLabelOffset, (int)vertexIndex);
                    graph->SetMaskAt(nodeA, visitMask);
                    graph->SetMaskAt(nodeB, visitMask);
                    graph->SetMaskAt(nodeA1, visitMask);
                    graph->SetMaskAt(nodeB1, visitMask);

                    if (!graph->HasMaskAt(nodeB, MTG_EXTERIOR_MASK))
                        {
                        // On each side,  get the average of all others.
                        // (For the expected triangles this is "just" the other vertex)
                        auto sumLeft = SumUnmaskedComplement(nodeA, nodeA2, mask);
                        auto sumRight = SumUnmaskedComplement(nodeB, nodeB2, mask);
                        sumLeft.DeWeightInPlace();
                        sumRight.DeWeightInPlace();
                        m_newPoints.push_back(
                            DPoint3dSizeSize(
                                DPoint3d::FromSumOf(
                                    xyzMid, aMid,
                                    sumLeft.GetXYZ(), aFar,
                                    sumRight.GetXYZ(), aFar
                                ),
                                3, 0
                            ));

                        }
                    else
                        {
                        m_newPoints.push_back (DPoint3dSizeSize(xyzMid, 4, 0));
                        }
                    }
                }
            }
        MTGARRAY_END_SET_LOOP(nodeA, graph)
        }

    // For each INTERNAL node with splitMask and FSucc.Fsucc also has splitMask -- join!
    // ("Join" carefully transfers indices!!)
    void CutCorners(
        MTGMask splitMask,      // mask previously set on edge splits
        MTGMask joinMask        // mask to set on join edge.
        )
        {
        MTGGraph * graph = &m_mtgFacets.graphHdr;
        MTGARRAY_SET_LOOP(nodeA, graph)
            {
            if (graph->HasMaskAt(nodeA, splitMask) && !graph->HasMaskAt(nodeA, MTG_EXTERIOR_MASK))
                {
                auto nodeB = graph->FSucc(nodeA);
                auto nodeC = graph->FSucc(nodeB);
                MTGNodeId nodeP, nodeQ;
                graph->Join(nodeP, nodeQ, nodeA, nodeC, joinMask, joinMask);
                }
            }
        MTGARRAY_END_SET_LOOP(nodeA, graph)
        }

    bool InstallNewPointCoordinates(double &maxChange)
        {
        if (m_mtgFacets.vertexArrayHdr.size () != m_newPoints.size ())
            return false;
        for (size_t i = 0; i < m_mtgFacets.vertexArrayHdr.size(); i++)
            {
            DPoint3d xyz0 = m_mtgFacets.vertexArrayHdr[i];
            DPoint3d xyz1 = m_newPoints[i].Get();
            maxChange = DoubleOps::MaxAbs (maxChange, xyz0.Distance (xyz1));
            m_mtgFacets.vertexArrayHdr[i] = xyz1;
            }
        return true;
        }
    bool RunLoopSubdivision(double &maxChange, int boundaryAction)
        {
#ifdef CompilePrintChecks
        static int s_printDebug = 0;
        if (s_printDebug)
            {
            jmdlMTGFacets_printFaceLoops(&splitter.m_mtgFacets);
            jmdlMTGFacets_printVertexLoops(&splitter.m_mtgFacets);
            }
#endif
        maxChange = 0.0;
        SetupNewPointSums ();
        MTGGraph::ScopedMask edgeMask(m_mtgFacets.graphHdr);
        MTGGraph::ScopedMask strutMask(m_mtgFacets.graphHdr);
        ComputeVertexSums_LoopWeights(boundaryAction);
        SplitEdges_LoopWeights(edgeMask.Get());
        CutCorners(edgeMask.Get(), strutMask.Get());
        bool ok = InstallNewPointCoordinates (maxChange);
#ifdef CompilePrintChecks
        if (s_printDebug)
            {
            jmdlMTGFacets_printFaceLoops(&splitter.m_mtgFacets);
            jmdlMTGFacets_printVertexLoops(&splitter.m_mtgFacets);
            }
#endif
        return ok;
        }

    static PolyfaceHeaderPtr Subdivide_LoopWeights(PolyfaceQueryCR mesh, uint32_t maxDepth, double maxChange, int boundaryAction)
        {
        SubdivisionData splitter (mesh);
        if (splitter.InitMTG())
            {
            if (maxChange <= 0.0)
                maxChange = DBL_MAX;
            double maxChange1;
            if (maxDepth == 0 || maxDepth > 4)
                maxDepth = 4;
            splitter.RunLoopSubdivision(maxChange1, boundaryAction);
            PolyfaceHeaderPtr out = PolyfaceHeader::CreateVariableSizeIndexed();
            AddMTGFacetsToIndexedPolyface(&splitter.m_mtgFacets, *out, MTG_EXTERIOR_MASK, MTG_BOUNDARY_MASK);
            for (uint32_t i = 1; i < maxDepth && maxChange1 > maxChange; i++)
                {
                SubdivisionData splitter1(*out);
                if (splitter1.InitMTG())
                    {
                    splitter1.RunLoopSubdivision(maxChange1, boundaryAction);
                    out = PolyfaceHeader::CreateVariableSizeIndexed();
                    AddMTGFacetsToIndexedPolyface(&splitter1.m_mtgFacets, *out, MTG_EXTERIOR_MASK, MTG_BOUNDARY_MASK);
                    }
                }
            return out;
            }
        return nullptr;
        }   
};


PolyfaceHeaderPtr PolyfaceHeader::CloneSubdivided(PolyfaceQueryCR source,
int subdivisionMethod,
uint32_t maxDepth,
double maxChange,
int boundaryAction
)
    {
    if (maxDepth > 4)
        maxDepth = 4;
    if (maxDepth == 0 && maxChange <= 0.0)
        return source.CloneAsVariableSizeIndexed(source);

    if (subdivisionMethod == TaggedNumericData::SubdivisionMethod::ChooseBasedOnFacets)
        {
        size_t minPerFace, maxPerFace;
        source.CollectPerFaceCounts(minPerFace, maxPerFace);
        if (minPerFace == 3 && maxPerFace == 3)
            subdivisionMethod = TaggedNumericData::SubdivisionMethod::Loop;
        else if (minPerFace == 4 && maxPerFace == 4)
            subdivisionMethod = TaggedNumericData::SubdivisionMethod::CatmullClark;
        else
            subdivisionMethod = TaggedNumericData::SubdivisionMethod::DooSabin;
        }
    if (subdivisionMethod == TaggedNumericData::SubdivisionMethod::Loop)
        {
        size_t minPerFace, maxPerFace;
        source.CollectPerFaceCounts(minPerFace, maxPerFace);
        if (minPerFace == 3 && maxPerFace == 3)
            {
            return SubdivisionData::Subdivide_LoopWeights (source, maxDepth, maxChange, boundaryAction);
            }
        else
            {
            auto triangulatedSource = source.Clone ();
            triangulatedSource->Triangulate ();
            return SubdivisionData::Subdivide_LoopWeights(*triangulatedSource, maxDepth, maxChange, boundaryAction);
            }
        }
    else if (subdivisionMethod == TaggedNumericData::SubdivisionMethod::DooSabin)
        {
        return DooSabin::SubdividePolyface(source, maxDepth, maxChange, boundaryAction);
        }
    else
        {
        // DEFAULT to CatmullClark
        // REMARK: This rebuilds the MTG at each level.
        // For possible performance improvement:
        //    * rewrite the single level clone step as MTG-to-MTG
        //    * build PolyfaceHeader only at the end.

        // unroll the first subdivision because source is a PolyfaceQuery and later rounds have a PolyfaceHeader
        MTGFacets mtgMesh0;
        if (!PolyfaceToMTG_FromPolyfaceConnectivity(&mtgMesh0, source))
            return nullptr;
        double maxChange1;
        if (maxDepth == 0)
            maxDepth = 4;
        auto currentMesh = MTGFacets_CloneCatmullClarkSubdivision(mtgMesh0, maxChange1, boundaryAction);
        for (uint32_t i = 1; i < maxDepth  && maxChange1 > maxChange; i++)
            {
            MTGFacets mtgMesh;
            if (!PolyfaceToMTG_FromPolyfaceConnectivity(&mtgMesh, *currentMesh))
                break;
            currentMesh = MTGFacets_CloneCatmullClarkSubdivision(mtgMesh, maxChange1, boundaryAction);
            }
        return currentMesh;
        }
    }

END_BENTLEY_GEOMETRY_NAMESPACE