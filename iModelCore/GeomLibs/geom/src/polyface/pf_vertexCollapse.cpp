/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <bsibasegeomPCH.h>
#include <Geom/cluster.h>
#include "pf_halfEdgeArray.h"

// Structure to record a vector of indices with one singleton.
struct IndexVectorAndSingleton
    {
    MTGNodeId index0;
    bvector<MTGNodeId> indices;
    void Clear ()
        {
        index0 = MTGGraph::NullNodeId;
        indices.clear ();
        }
    };

static double ConstructTolerance (DRange3dCR range, double abstol, double rangeFractionTol)
    {
    double diagonal = range.low.Distance (range.high);
    if (abstol <= 0.0)
        abstol = 1.0e-14;
    if (rangeFractionTol > 0.0)
        {
        abstol = DoubleOps::Max (abstol, rangeFractionTol * diagonal);
        }
    return abstol;
    }

size_t PolyfaceHeader::DecimateByEdgeCollapse (double abstol, double rangeFractionTol)
    {
    ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ();
    bvector<DPoint3d> newXYZ;  
    bvector<int> blockedIndex;
    bvector<size_t> oldToNew;
    size_t oldNumXYZ = m_point.size ();
    abstol = ConstructTolerance (PointRange (), abstol, rangeFractionTol);
    if (oldNumXYZ < 3)
        return 0;

    bvector<DPoint3d> &point = Point ();
    bsiDPoint3dArray_findClusters (point, blockedIndex, &newXYZ, abstol, false, true, &oldToNew);
    bool sharedParamIndex = m_param.size () == m_point.size () && m_paramIndex.size () == 0;
    bool sharedNormalIndex = m_normal.size () == m_point.size () && m_normalIndex.size () == 0;
    bool sharedColorIndex = m_intColor.size () == m_point.size () && m_intColor.size () == 0;

    size_t num0 = m_point.size ();
    size_t num1 = newXYZ.size ();
    if (sharedParamIndex || sharedNormalIndex || sharedColorIndex)
        {
        // point collapse carries params.
        bvector<DPoint2d> newParam;
        if (sharedParamIndex)
            newParam.resize (newXYZ.size ());       // all zeros

        bvector<DVec3d> newNormal;
        if (sharedNormalIndex)
            newNormal.resize (newXYZ.size ());       // all zeros

        bvector<uint32_t> newColor;
        if (sharedColorIndex)
            newColor.resize (newXYZ.size ());       // all zeros

        for (size_t k = 0; k < m_pointIndex.size (); k++)
            {
            int index1 = m_pointIndex[k];
            if (index1 != 0)
                {
                int index0 = abs (index1) - 1;
                int s = index1 > 0 ? 1 : -1;
                // update the pointIndex (which is also param index)
                size_t newIndex0 = oldToNew[index0];
                m_pointIndex[k]  = s * (1 +  (int)newIndex0);
                // copy from old param to new.   This will gradually replace all the zeros.
                if (sharedParamIndex)
                    newParam[newIndex0] = m_param[index0];
                if (sharedNormalIndex)
                    newNormal[newIndex0] = m_normal[index0];
                if (sharedColorIndex)
                    newColor[newIndex0] = m_intColor[index0];
                }
            }
        if (sharedParamIndex)
            m_param.swap (newParam);
        if (sharedNormalIndex)
            m_normal.swap (newNormal);
        if (sharedColorIndex)
            m_intColor.swap (newColor);
        }
    else
        {
        // REMARK: this loop works with the reference to the inplace m_pointIndex.
        for (auto &index1: m_pointIndex)
            {
            if (index1 != 0)
                {
                int index0 = abs (index1) - 1;
                int s = index1 > 0 ? 1 : -1;
                index1 = s * (1 + (int)oldToNew[index0]);
                }
            }
        }    
    m_point.ClearAndAppend (newXYZ);
    return num0 - num1;
    }
#ifdef CompileDecimateByEdgeCollapseWithBoundaryControl
// EDL Feb 15 2017
// this method was implemented in a "half edge" data structure where half edges directly corresponded to readIndex.
// It was tested minimally in that form.
// Feb 2017 recoded with MTGFacets, but not debugged.  I think the cluster logic is decent, but the coordinate updates
// in the polyface versus the MTGFacets are murky.
// Abandoning in that state -- ifdefs in this source and in polyface.h
struct EdgeCandidate
{
MTGNodeId m_nodeId;     // Read index for the half edge.
bool   m_isReversed:1;  // True if using this edge in the reverse sense.
EdgeCandidate (MTGNodeId nodeId, bool isReversed)
    : m_nodeId (nodeId), m_isReversed (isReversed)
    {}
EdgeCandidate():m_nodeId(MTGGraph::NullNodeId), m_isReversed (false){}
};

// coordinates, parents and strength of a vertex
struct ClusterData
{
DPoint3d m_xyz;
size_t  m_parentClusterIndex;
MTGNodeId  m_nodeId;
double  m_a;
VuMask m_masks;
size_t  m_newPointIndex;
static const VuMask BoundaryVertexMask = 0x01;
static const VuMask FixedVertexMask = 0x02;
static const VuMask CollapsedVertexMask = 0x04;

void SetMask (VuMask mask)
    { m_masks |= mask;}
bool HasMask (VuMask mask)
    { return 0 != (m_masks & mask);}
uint32_t HasMask01 (VuMask mask)
    { return (0 == (m_masks & mask)) ? 0 : 1;}
void ClearMask (VuMask mask)
    { m_masks &= ~mask;}

ClusterData (DPoint3dCR xyz, MTGNodeId nodeId, size_t parentClusterIndex, double a)
    : m_xyz(xyz),
    m_nodeId (nodeId),
    m_parentClusterIndex (parentClusterIndex),
    m_a (a),
    m_masks(0)
    {
    }

ClusterData () : m_parentClusterIndex (0), m_nodeId(MTGGraph::NullNodeId), m_a (0.0) {}

};

struct ClusterVector : bvector<ClusterData>
{
size_t FixupParent (size_t iStart)
    {
    ClusterData &data = at(iStart);
    if (data.m_parentClusterIndex != iStart)
        data.m_parentClusterIndex = FixupParent(data.m_parentClusterIndex);
    return data.m_parentClusterIndex;
    }
void FixupAllParents ()
    {
    for (size_t i = 0, n = size (); i < n; i++)
        FixupParent (i);
    }
// ASSUME m_parent chains are only one deep (i.e. FixupAllParents has been called.
void PackPoints (bvector<DPoint3d> &points)
    {
    points.clear ();

    for (size_t i = 0, n = size (); i < n; i++)
        {
        size_t j = at(i).m_parentClusterIndex;
        if (j == i)    // This is the root of a cluster.
            {
            at(i).m_newPointIndex = points.size ();
            points.push_back (at(i).m_xyz);
            }
        }
    
    for (size_t i = 0, n = size (); i < n; i++)
        {
        size_t j = at(i).m_parentClusterIndex;
        if (j != i)
            {
            at(i).m_newPointIndex = at(j).m_newPointIndex;
            points.push_back (at(i).m_xyz);
            }
        }
    }
};

typedef ValidatedValue<ClusterData> ValidatedClusterData;

// In the graph (which is in an MTGFacets)
//<ul>
//<li>Every node has readIndex as a label (part of the normal MTGFacets)
//<li>Every node has a clusterIndex as a label.(access through m_clusterIndexLabel
//</ul>
//
// As clusters are merged . . .
// Child clusters point to parent (which has final coordinates)
struct VertexClusterContext
{
PolyfaceHeaderR m_mesh;
MTGFacets m_facets;
MTGGraphP m_graph;  // from m_facets.
int m_clusterIndexLabel;
int m_readIndexLabel;

ClusterVector m_cluster;

VertexClusterContext (PolyfaceHeaderR mesh)
    : m_mesh (mesh)
    {
    m_graph = m_facets.GetGraphP ();
    m_clusterIndexLabel = m_graph->DefineLabel (1001, MTG_LabelMask_VertexProperty,-1);
    }

ValidatedDPoint3d NodeToClusterXYZ (MTGNodeId nodeId)
    {
    int clusterId;
    if (    m_graph->TryGetLabel (nodeId, m_clusterIndexLabel, clusterId))
        return ClusterIndexToXYZ (clusterId);
    return ValidatedDPoint3d ();
    }

ValidatedDPoint3d ClusterIndexToXYZ (size_t clusterIndex)
    {
    if (clusterIndex < m_cluster.size ())
        return ValidatedDPoint3d (m_cluster[(size_t)clusterIndex].m_xyz, true);
    return ValidatedDPoint3d ();
    }

size_t NodeToClusterIndex (MTGNodeId nodeId)
    {
    int clusterId;
    m_graph->TryGetLabel (nodeId, m_clusterIndexLabel, clusterId);
    return clusterId >= 0 ? (size_t) clusterId : SIZE_MAX;
    }

size_t NodeToReadIndex (MTGNodeId nodeId)
    {
    int index;
    m_graph->TryGetLabel (nodeId, m_readIndexLabel, index);
    return index < SIZE_MAX ? (size_t) index : SIZE_MAX;
    }

bool IsValidClusterId (size_t clusterId) {return clusterId < m_cluster.size ();}

ValidatedClusterData OriginalVertexData (MTGNodeId nodeId)
    {
    DPoint3d xyz;
    if (m_facets.NodeToVertexCoordinates (nodeId, xyz))
        {
        return ValidatedClusterData (ClusterData (xyz, nodeId, SIZE_MAX, 0.0),
                true);
        }        
    return ValidatedClusterData ();
    }
// unconditional collapse -- to be called by policy-aware callers. . . 
void CollapseCluster (size_t parentClusterIndex, size_t childClusterIndex, bool addEdgeCandidates)
    {
    if (parentClusterIndex < m_cluster.size () && childClusterIndex < m_cluster.size () && childClusterIndex != parentClusterIndex)
        {
        auto &child = m_cluster[childClusterIndex];
        child.m_parentClusterIndex = parentClusterIndex;
        parentClusterIndex = m_cluster.FixupParent (childClusterIndex);
        auto & parent = m_cluster[parentClusterIndex];
        child.SetMask (ClusterData::CollapsedVertexMask);
        child.m_xyz = parent.m_xyz;
        FreezeCluster (childClusterIndex, addEdgeCandidates);
        }
    }


void GetClusterIndices (EdgeCandidate const &edge, size_t &clusterIndex0, size_t &clusterIndex1)
    {
    MTGNodeId edgeBase0, edgeBase1;
    if (edge.m_isReversed)
        {
        edgeBase0 = m_graph->FSucc (edge.m_nodeId);
        edgeBase1 = edge.m_nodeId;
        }
    else
        {
        edgeBase0 = edge.m_nodeId;
        edgeBase1 = m_graph->FSucc (edge.m_nodeId);
        }
    
    clusterIndex0 = NodeToClusterIndex (edgeBase0);    
    clusterIndex1 = NodeToClusterIndex (edgeBase1);
    }

ValidatedDSegment3d NewEdgeCoordinates (MTGNodeId nodeId)
    {
    auto tail = NodeToClusterXYZ (nodeId);
    auto head = NodeToClusterXYZ (m_graph->FSucc (nodeId));
    if (tail.IsValid () && head.IsValid ())
        {
        return ValidatedDSegment3d (
            DSegment3d::From (tail.Value (), head.Value ()),
            true);
        }
    else
        return ValidatedDSegment3d ();
    }

ValidatedDVec3d NewEdgeVector (MTGNodeId nodeId)
    {
    auto tail = NodeToClusterXYZ (nodeId);
    auto head = NodeToClusterXYZ (m_graph->FSucc (nodeId));
    if (tail.IsValid () && head.IsValid ())
        {
        return ValidatedDVec3d (
            DVec3d::FromStartEnd (tail.Value (), head.Value ()), true);
        }
    else
        return ValidatedDVec3d ();
    }


double SumInteriorDihedralAngles (IndexVectorAndSingleton &edges)
    {
// The neighbord data seeded at iC has these half edges:
// 
//           iC | iB
//     iD       |       iA
//--------------+-----------------
    double sum = 0.0;
    for (MTGNodeId iC : edges.indices)
        {
        if (!m_graph->HasMaskAt (iC, MTG_BOUNDARY_MASK))
            {
            MTGNodeId iB  = m_graph->EdgeMate (iC);
            MTGNodeId iA = m_graph->FSucc (iB);
            MTGNodeId iD = m_graph->FPred (iC);
            auto vectorD = NewEdgeVector (iD); // inbound
            auto vectorC = NewEdgeVector (iC); // outbound
            auto vectorA = NewEdgeVector (iA); // outbound
            auto normalCD = DVec3d::FromCrossProduct (vectorD, vectorC);
            auto normalAC = DVec3d::FromCrossProduct (vectorA, vectorC);
            sum += normalCD.AngleTo (normalAC);
            }
        }
    return sum;
    }

MinimumValuePriorityQueue<size_t> m_clusterCandidates;
MinimumValuePriorityQueue<EdgeCandidate> m_edgeCandidates;
// Visit sectors at a node id.
// report outbound edges in data vector.
// report the (at most one) inbound as the singleton
void GetNodesAroundVertex (MTGNodeId nodeId, IndexVectorAndSingleton &data)
    {
    data.Clear ();
    if (!m_graph->IsValidNodeId (nodeId))
        return;
    MTGNodeId nodeIdA = m_graph->FindMaskAroundVertex (nodeId, MTG_BOUNDARY_MASK);
    if (m_graph->IsValidNodeId (nodeIdA))
        {
        data.index0 = m_graph->EdgeMate (nodeIdA);
        MTGNodeId nodeIdB = nodeIdA;
        do
            {
            nodeIdB = m_graph->VPred (nodeIdB); 
            data.indices.push_back (nodeIdB);
            } while (nodeIdB != nodeIdA && !m_graph->HasMaskAt (nodeIdB, MTG_BOUNDARY_MASK));
        }
    else
        {
        MTGARRAY_VERTEX_LOOP (nodeIdB, m_graph, nodeIdA)
            {
            data.indices.push_back (nodeIdB);
            }
        MTGARRAY_END_VERTEX_LOOP (nodeIdB, m_graph, nodeIdA)
        }
    }

IndexVectorAndSingleton m_vertexNeighborhoodA;  // for temporary use at non-recursive sites

void AnnounceEdgeCandidate (MTGNodeId index, bool reversed)
    {
    if (m_graph->IsValidNodeId (index))
        {
        EdgeCandidate candidate(index, reversed);
        size_t pointIndex0, pointIndex1;
        GetClusterIndices (candidate, pointIndex0, pointIndex1);
        bool fixed0 = m_cluster[pointIndex0].HasMask (ClusterData::FixedVertexMask);
        bool fixed1 = m_cluster[pointIndex1].HasMask (ClusterData::FixedVertexMask);
        if (fixed0 && fixed1)
            {
            // IGNORE edge between fixed vertices.
            }
        else if (fixed0 || fixed1)
            {
            DPoint3d xyz0 = ClusterIndexToXYZ (pointIndex0);
            DPoint3d xyz1 = ClusterIndexToXYZ (pointIndex1);
            m_edgeCandidates.Insert (candidate, xyz0.Distance (xyz1));
            }
        else
            {
            // IGNORE edge between floating vertices
            }
        }
    }

void FreezeCluster (size_t clusterIndex, bool addEdgeCandidates)
    {
    m_cluster[clusterIndex].SetMask (ClusterData::FixedVertexMask);
    if (addEdgeCandidates)
        {
        GetNodesAroundVertex (m_cluster[clusterIndex].m_nodeId, m_vertexNeighborhoodA);
        AnnounceEdgeCandidate (m_vertexNeighborhoodA.index0, true);
        for (auto index : m_vertexNeighborhoodA.indices)
            AnnounceEdgeCandidate (index, false);
        }
    }

bool RemoveAndFreezeTopCluster (bool addEdgeCandidates)
    {
    if (m_clusterCandidates.IsEmpty ())
        return false;
    size_t clusterIndex;
    double a;
    m_clusterCandidates.RemoveMin (clusterIndex, a);
    FreezeCluster (clusterIndex, addEdgeCandidates);
    return true;
    }

// remove vertices with priority <= value from the vertexCandidates heap
size_t FreezeVerticesFromHeap (double value, bool addEdgeCandidates)
    {
    size_t numFreeze = 0;
    while (!m_clusterCandidates.IsEmpty () && m_clusterCandidates.MinValue () < value)
        {
        numFreeze++;
        RemoveAndFreezeTopCluster (addEdgeCandidates);
        }
    return numFreeze;
    }


void FixupPointAndPointIndexArrays ()
    {
    m_cluster.FixupAllParents ();  // Now all clusters know their real parent directly.
    MTGARRAY_SET_LOOP (nodeId, m_graph)
        {
        auto finalCluster = m_cluster.FixupParent (NodeToClusterIndex (nodeId));
        m_facets.SetPointIndex (nodeId, finalCluster);
        }
    MTGARRAY_END_SET_LOOP (nodeId, m_graph)

    m_cluster.PackPoints (m_mesh.Point ());
    // make the polyface PointIndex () entries go to the new array also ..
    bvector<int> &pointIndex = m_mesh.PointIndex ();
    MTGARRAY_SET_LOOP_TESTED (nodeId, m_graph, !m_graph->HasMaskAt (nodeId, MTG_EXTERIOR_MASK))
        {
        auto k = m_cluster.FixupParent (NodeToClusterIndex (nodeId));
        size_t readIndex = NodeToReadIndex (nodeId);
        if (readIndex < pointIndex.size ())
            pointIndex[readIndex] = 1 + (int)m_cluster[k].m_newPointIndex;  // one based !!!
        }
    MTGARRAY_END_SET_LOOP_TESTED (nodeId, m_graph)

    }

size_t CollapseEdgesFromHeap (double collapseDistance, bool addEdgeCandidates)
    {
    EdgeCandidate candidate;
    size_t numCollapse = 0;
    double originalDistance;    // ORIGINAL distance -- may have changed due to other collapses.
    while (m_edgeCandidates.RemoveMin (candidate, originalDistance)) // hmm.. we'll continue evaluating.  Can we exit early when distances are large?
        {
        size_t pointIndex0, pointIndex1;
        GetClusterIndices (candidate, pointIndex0, pointIndex1);
        bool fixed0 = m_cluster[pointIndex0].HasMask (ClusterData::FixedVertexMask);
        BeAssert (fixed0);
        bool fixed1 = m_cluster[pointIndex1].HasMask (ClusterData::FixedVertexMask);
        if (fixed0 && fixed1)
            {
            // IGNORE edge between fixed vertices.
            }
        else 
            {
            DPoint3d xyz0 = ClusterIndexToXYZ (pointIndex0);
            DPoint3d xyz1 = ClusterIndexToXYZ (pointIndex1);
            double distance = xyz0.Distance (xyz1);
            if (distance <= collapseDistance)
                {
                if (fixed0)
                    {
                    numCollapse++;
                    CollapseCluster (pointIndex0, pointIndex1, addEdgeCandidates);
                    }
                else
                    {
                    numCollapse++;
                    CollapseCluster (pointIndex1, pointIndex0, addEdgeCandidates);
                    }
                }
            }
        }
    m_edgeCandidates.Clear ();
    return numCollapse;
    }
double EvaluateVertexNeighborhood (IndexVectorAndSingleton &nbd)
    {
    static double boundaryVertexBaseValue = -10.0;
    static double interiorVertexBaseValue = -5.0;
    static double boundaryEdgeAngleScale = -1.0 / Angle::Pi ();
    static double dihedralAngleScale = -1.0 / Angle::Pi ();
    if (nbd.indices.size () == 0)
        return 0.0;
    double interiorSum = SumInteriorDihedralAngles (nbd);

    if (m_graph->IsValidNodeId (nbd.index0))
        {
        // this is a bounday vertex.
        // Its two indcident boundary half edges are:
        MTGNodeId leftHalfEdge = nbd.index0;
        MTGNodeId rightHalfEdge = nbd.indices.back ();
        auto leftVector = NewEdgeVector (leftHalfEdge);
        auto rightVector = NewEdgeVector (rightHalfEdge);
        double exteriorAngle = 0.0;
        if (leftVector.IsValid () && rightVector.IsValid ())
            {
            exteriorAngle = leftVector.Value ().AngleTo (rightVector.Value ());
            }
        return boundaryVertexBaseValue + exteriorAngle * boundaryEdgeAngleScale + interiorSum * dihedralAngleScale;
        }
    return interiorVertexBaseValue + interiorSum * dihedralAngleScale;
    }

void SetNeighborHoodMask (IndexVectorAndSingleton &nbd, MTGMask mask, bool setVector, bool setSingleton)
    {
    if (setVector)
        for (auto nodeId : nbd.indices)
            m_graph->SetMaskAt (nodeId, mask);
    if (setSingleton)
        m_graph->SetMaskAt (nbd.index0, mask);
    }

bool BuildMeshAndClusterHeap ()
    {
    if (!PolyfaceToMTG_FromPolyfaceConnectivity (&m_facets, m_mesh))
        return NULL;
    if (!m_graph->TrySearchLabelTag (MTG_LABEL_TAG_POLYFACE_READINDEX, m_readIndexLabel))
        return NULL;

    IndexVectorAndSingleton vertexNeighborhood;
    auto visitMask = m_graph->GrabMask ();
    m_graph->ClearMask (visitMask);
    size_t errors = 0;
    m_cluster.clear ();
    // At each distinct vertex of the mesh .... 
    MTGMask excludeMask = visitMask | MTG_EXTERIOR_MASK;
    MTGARRAY_SET_LOOP_TESTED (nodeId, m_graph, !m_graph->HasMaskAt (nodeId, excludeMask))
        {
        GetNodesAroundVertex (nodeId, vertexNeighborhood);
        SetNeighborHoodMask (vertexNeighborhood, visitMask, true, false);
        auto seedData = OriginalVertexData(nodeId);
        if (!seedData.IsValid ())
            errors++;
        else
            {
            // Create a newPoint[] for these coordinates.
            // Index the vertex neighborhood back to this newPoint[]
            size_t newClusterIndex = m_cluster.size ();
            auto seedData1 = seedData.Value ();
            seedData1.m_parentClusterIndex = newClusterIndex;
            m_cluster.push_back (seedData1);
            for (MTGNodeId edgeNode : vertexNeighborhood.indices)
                {
                auto data = OriginalVertexData (edgeNode);
                if (!data.IsValid () || data.Value ().m_parentClusterIndex != data.Value ().m_parentClusterIndex)
                    errors++;
                else
                    {
                    m_facets.SetPointIndex (edgeNode, newClusterIndex);
                    }
                }
            }
        }
    MTGARRAY_END_SET_LOOP_TESTED (nodeId, m_graph)

    if (errors == 0)
        {
        m_clusterCandidates.Clear ();
        m_graph->ClearMask (visitMask);

        MTGARRAY_SET_LOOP_TESTED (nodeId, m_graph, !m_graph->HasMaskAt (nodeId, excludeMask))
            {
            GetNodesAroundVertex (nodeId, vertexNeighborhood);
            SetNeighborHoodMask (vertexNeighborhood, visitMask, true, false);
            if (vertexNeighborhood.indices.size () == 0)
                continue;
            m_clusterCandidates.Insert (nodeId, EvaluateVertexNeighborhood (vertexNeighborhood));
            }
        MTGARRAY_END_SET_LOOP_TESTED (nodeId, m_graph)
        }
    m_graph->DropMask (visitMask);
    // FINALLY READY TO FLY ....
    return errors == 0;
    }

size_t RunCollapse (double shortEdgeLength)
    {
    m_edgeCandidates.Clear ();
    static double s_fixedVertexPriority = -10.25;   // boundary vertex with angle 45 degrees or greater is fixed
    FreezeVerticesFromHeap (s_fixedVertexPriority, true); // Freeze boundary vertices with angle 45 degrees or greater.   Outbound edges go into the m_edgeCandidates heap.
    size_t numCollapse = 0;
    do 
        {
        numCollapse += CollapseEdgesFromHeap (shortEdgeLength, true);
        RemoveAndFreezeTopCluster (true);
        } while (!m_clusterCandidates.IsEmpty ());
    return numCollapse;
    }
};

size_t PolyfaceHeader::DecimateByEdgeCollapseWithBoundaryControl (double abstol, double rangeFractionTol)
    {
    ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ();
    VertexClusterContext context (*this);
    size_t numCollapse = 0;
    if (context.BuildMeshAndClusterHeap ())
        {
        abstol = ConstructTolerance (PointRange (), abstol, rangeFractionTol);
        numCollapse = context.RunCollapse (abstol);
        if (numCollapse > 0)
            {
            context.FixupPointAndPointIndexArrays ();   // This packs the mesh points, and updates all point indices. (But leaves collapsed facets in place)
            RemoveCollapsedFacetsByPointIndexComparison ();
            }
        }
    return numCollapse;
    }
#endif
// copy data[i0] through data[i0+numCopy-1] to data[j0] onward.
static void CopyIndices (bvector<int>&data, size_t i0, size_t i1, size_t j0)
    {
    if (i0 != j0)
        {
        for (;i0 < i1; i0++, j0++)
            data[j0] = data[i0];
        }
    }

size_t PolyfaceHeader::RemoveCollapsedFacetsByPointIndexComparison ()
    {
    ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ();
    size_t numIndex = m_pointIndex.size ();
    size_t numAccept = 0;
    size_t numDeletedFacets = 0;
    size_t i1;
    for (size_t i0 = 0; i0 < numIndex; i0 = i1)
        {
        i1 = i0 + 1;
        for (; i1 < numIndex && m_pointIndex[i1] != 0;)
            {
            i1++;
            }
        // back up over duplicates at tail
        size_t k1 = i1 - 1;
        while (k1 > i0 && m_pointIndex[k1] == m_pointIndex[i0])
            k1--;
        // count transitions
        size_t numDistinct = 0;
        for (size_t j0 = i0; j0 <= k1;)
            {
            int index0 = m_pointIndex[j0];
            while (++j0 <= k1 && m_pointIndex[j0] == index0)
                {
                }
            numDistinct++;
            }
        if (i1 < numIndex)
             i1++;   // i1 normally identifies the trailing zero.  Increment so copies include it and next step starts "after"
            
        if (numDistinct < 3)
            {
            // this is a degenerate facet
            numDeletedFacets++;
            }
        else
            {
            // accept this facet ... note that if "some" duplicate were found, but 3 or more distinct,
            //   the entire set (including the dup) is copied.
            CopyIndices (m_pointIndex, i0, i1, numAccept);
            if (m_paramIndex.size() == numIndex)
                CopyIndices (m_paramIndex, i0, i1, numAccept);
            if (m_normalIndex.size() == numIndex)
                CopyIndices (m_normalIndex, i0, i1, numAccept);
            if (m_colorIndex.size() == numIndex)
                CopyIndices (m_colorIndex, i0, i1, numAccept);
            if (m_faceIndex.size() == numIndex)
                CopyIndices (m_faceIndex, i0, i1, numAccept);
            numAccept += i1 - i0;
            }
        }
    m_pointIndex.resize (numAccept);
    if (m_paramIndex.size() == numIndex)
        m_paramIndex.resize (numAccept);
    if (m_normalIndex.size() == numIndex)
        m_normalIndex.resize (numAccept);
    if (m_colorIndex.size() == numIndex)
        m_colorIndex.resize (numAccept);
    if (m_faceIndex.size() == numIndex)
        m_faceIndex.resize (numAccept);
    return numDeletedFacets;
    }


