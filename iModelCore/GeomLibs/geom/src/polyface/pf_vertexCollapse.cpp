/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/polyface/pf_vertexCollapse.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <bsibasegeomPCH.h>
#include <Geom/cluster.h>
#include "pf_halfEdgeArray.h"

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

    size_t num0 = m_point.size ();
    size_t num1 = newXYZ.size ();
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
    
    m_point.ClearAndAppend (newXYZ);
    return num0 - num1;
    }

struct EdgeCandidate
{
size_t m_readIndex;     // Read index for the half edge.
bool   m_isReversed:1;  // True if using this edge in the reverse sense.
EdgeCandidate (size_t readIndex, bool isReversed)
    : m_readIndex (readIndex), m_isReversed (isReversed)
    {}
EdgeCandidate():m_readIndex(SIZE_MAX), m_isReversed (false){}
};

// coordinates, parents and strength of a vertex
struct ClusterData
{
DPoint3d m_xyz;
size_t  m_parentClusterIndex;
size_t  m_halfEdgeIndex;
double  m_a;
Neighbor::MaskType m_masks;
size_t  m_newPointIndex;
static const Neighbor::MaskType BoundaryVertexMask = 0x01;
static const Neighbor::MaskType FixedVertexMask = 0x02;
static const Neighbor::MaskType CollapsedVertexMask = 0x04;

void SetMask (Neighbor::MaskType mask)
    { m_masks |= mask;}
bool HasMask (Neighbor::MaskType mask)
    { return 0 != (m_masks & mask);}
uint32_t HasMask01 (Neighbor::MaskType mask)
    { return (0 == (m_masks & mask)) ? 0 : 1;}
void ClearMask (Neighbor::MaskType mask)
    { m_masks &= ~mask;}

ClusterData (DPoint3dCR xyz, size_t halfEdgeIndex, size_t parent, double a)
    : m_xyz(xyz),
    m_halfEdgeIndex (halfEdgeIndex),
    m_parentClusterIndex (parent),
    m_a (a),
    m_masks(0)
    {
    }

ClusterData () : m_parentClusterIndex (0), m_halfEdgeIndex(SIZE_MAX), m_a (0.0) {}

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

struct VertexClusterContext
{
PolyfaceHeaderR m_mesh;
NeighborVector m_neighbors;
ClusterVector m_cluster;

VertexClusterContext (PolyfaceHeaderR mesh)
    : m_mesh (mesh)
    {
    }

ValidatedDPoint3d ReadIndexToClusterXYZ (size_t readIndex)
    {
    size_t pointIndex = m_neighbors.PointIndex (readIndex);
    if (pointIndex < m_cluster.size ())
        return ValidatedDPoint3d (m_cluster[pointIndex].m_xyz, true);
    return ValidatedDPoint3d ();
    }

ValidatedDPoint3d ClusterIndexToXYZ (size_t pointIndex)
    {
    if (pointIndex < m_cluster.size ())
        return ValidatedDPoint3d (m_cluster[pointIndex].m_xyz, true);
    return ValidatedDPoint3d ();
    }


ValidatedClusterData OriginalVertexData (size_t readIndex)
    {
    auto originalIndex = OriginalVertexIndex (readIndex);
    if (originalIndex.IsValid ())
        {
        return ValidatedClusterData (
                ClusterData (
                        m_mesh.Point()[originalIndex],
                        readIndex, 0, 0.0),
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

size_t VertexIndex (size_t readIndex)
    {
    return m_neighbors.PointIndex (readIndex);
    }

ValidatedSize OriginalVertexIndex (size_t readIndex)
    {
    if (m_neighbors.IsValidReadIndex (readIndex))
        return ValidatedSize (abs (m_mesh.PointIndex()[readIndex]) - 1, true);
    return ValidatedSize (0, false);
    }

void GetVertexIndices (EdgeCandidate const &edge, size_t &pointIndex0, size_t &pointIndex1)
    {
    size_t edgeBase0, edgeBase1;
    if (edge.m_isReversed)
        {
        edgeBase0 = m_neighbors.FSucc (edge.m_readIndex);
        edgeBase1 = edge.m_readIndex;
        }
    else
        {
        edgeBase0 = edge.m_readIndex;
        edgeBase1 = m_neighbors.FSucc (edge.m_readIndex);
        }
    
    pointIndex0 = m_neighbors.PointIndex (edgeBase0);    
    pointIndex1 = m_neighbors.PointIndex (edgeBase1);
    }

ValidatedDSegment3d NewEdgeCoordinates (size_t index)
    {
    auto tail = ReadIndexToClusterXYZ (index);
    auto head = ReadIndexToClusterXYZ (m_neighbors.FSucc (index));
    if (tail.IsValid () && head.IsValid ())
        {
        return ValidatedDSegment3d (
            DSegment3d::From (tail.Value (), head.Value ()),
            true);
        }
    else
        return ValidatedDSegment3d ();
    }

ValidatedDVec3d NewEdgeVector (size_t index)
    {
    auto tail = ReadIndexToClusterXYZ (index);
    auto head = ReadIndexToClusterXYZ (m_neighbors.FSucc (index));
    if (tail.IsValid () && head.IsValid ())
        {
        return ValidatedDVec3d (
            DVec3d::FromStartEnd (tail.Value (), head.Value ()), true);
        }
    else
        return ValidatedDVec3d ();
    }


double SumInteriorDihedralAngles (NeighborVector::IndexVectorAndSingleton &edges)
    {
// The neighbord data seeded at iC has these half edges:
// 
//           iC | iB
//     iD       |       iA
//--------------+-----------------
    double sum = 0.0;
    for (size_t iC : edges.indices)
        {
        size_t iB  = m_neighbors.ManifoldMate (iC);
        if (m_neighbors.IsValidReadIndex (iB))
            {
            size_t iA = m_neighbors.FSucc (iB);
            size_t iD = m_neighbors.FPred (iC);
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


NeighborVector::IndexVectorAndSingleton m_vertexNeighborhoodA;  // for temporary use at non-recursive sites

void AnnounceEdgeCandidate (size_t index, bool reversed)
    {
    if (m_neighbors.IsValidReadIndex (index))
        {
        EdgeCandidate candidate(index, reversed);
        size_t pointIndex0, pointIndex1;
        GetVertexIndices (candidate, pointIndex0, pointIndex1);
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
        m_neighbors.GetAroundVertex (m_cluster[clusterIndex].m_halfEdgeIndex, m_vertexNeighborhoodA);
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
    for (size_t i = 0; i < m_neighbors.size (); i++)
        {
        if (m_neighbors.IsValidReadIndex (i))
            {
            auto finalCluster = m_cluster.FixupParent (m_neighbors.PointIndex (i));
            m_neighbors.SetPointIndex (i, finalCluster);
            }
        }
    m_cluster.PackPoints (m_mesh.Point ());
    // make the polyface PointIndex () entries go to the new array also ..
    bvector<int> &pointIndex = m_mesh.PointIndex ();
    for (size_t i = 0; i < m_neighbors.size (); i++)
        {
        if (m_neighbors.IsValidReadIndex (i))
            {
            auto k = m_cluster.FixupParent (m_neighbors.PointIndex(i));
            pointIndex[i] = 1 + (int)m_cluster[k].m_newPointIndex;  // one based !!!
            }
        }

    }

size_t CollapseEdgesFromHeap (double collapseDistance, bool addEdgeCandidates)
    {
    EdgeCandidate candidate;
    size_t numCollapse = 0;
    double originalDistance;    // ORIGINAL distance -- may have changed due to other collapses.
    while (m_edgeCandidates.RemoveMin (candidate, originalDistance)) // hmm.. we'll continue evaluating.  Can we exit early when distances are large?
        {
        size_t pointIndex0, pointIndex1;
        GetVertexIndices (candidate, pointIndex0, pointIndex1);
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
double EvaluateVertexNeighborhood (NeighborVector::IndexVectorAndSingleton &nbd)
    {
    static double boundaryVertexBaseValue = -10.0;
    static double interiorVertexBaseValue = -5.0;
    static double boundaryEdgeAngleScale = -1.0 / Angle::Pi ();
    static double dihedralAngleScale = -1.0 / Angle::Pi ();
    if (nbd.indices.size () == 0)
        return 0.0;
    double interiorSum = SumInteriorDihedralAngles (nbd);

    if (m_neighbors.IsValidReadIndex (nbd.index0))
        {
        // this is a bounday vertex.
        // Its two indcident boundary half edges are:
        size_t leftHalfEdge = nbd.index0;
        size_t rightHalfEdge = nbd.indices.back ();
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

bool BuildMeshAndClusterHeap ()
    {
    size_t num1, num2, numX;
    if (!m_mesh.BuildNeighborVector (m_neighbors, num1, num2, numX)
         || numX > 0
         )
         return false;

    NeighborVector::IndexVectorAndSingleton vertexNeighborhood;
    auto visitMask = NeighborVector::s_maskInternalA;
    m_neighbors.ClearMask (visitMask);
    size_t errors = 0;
    m_cluster.clear ();
    // At each distinct vertex of the mesh .... 
    for (size_t i = 0; i < m_neighbors.size (); i++)
        {
        if (m_neighbors.IsValidReadIndex (i) && !m_neighbors.HasMask (i, visitMask))
            {
            m_neighbors.GetAroundVertex (i, vertexNeighborhood);
            m_neighbors.SetMask (vertexNeighborhood, visitMask, true, false);
            auto seedData = OriginalVertexData(i);
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
                for (size_t k : vertexNeighborhood.indices)
                    {
                    auto data = OriginalVertexData (k);
                    if (!data.IsValid () || data.Value ().m_parentClusterIndex != data.Value ().m_parentClusterIndex)
                        errors++;
                    else
                        {
                        m_neighbors.SetPointIndex (k, newClusterIndex);
                        }
                    }
                }
            }
        }

    if (errors > 0)
        return false;

    m_clusterCandidates.Clear ();
    m_neighbors.ClearMask (visitMask);
    for (size_t i = 0; i < m_neighbors.size (); i++)
        {
        if (m_neighbors.IsValidReadIndex (i) && !m_neighbors.HasMask (i, visitMask))
            {
            m_neighbors.SetMask (vertexNeighborhood, visitMask, true, false);
            m_neighbors.GetAroundVertex (i, vertexNeighborhood);
            if (vertexNeighborhood.indices.size () == 0)
                continue;
            m_clusterCandidates.Insert (m_neighbors.PointIndex (i), EvaluateVertexNeighborhood (vertexNeighborhood));
            }
        }
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


