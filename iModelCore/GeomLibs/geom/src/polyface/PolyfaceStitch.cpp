/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "pf_halfEdgeArray.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE



bool TryCountSharedEdgesSpecialCases (
PolyfaceQueryCR mesh,
    size_t &numPolygonEdge, size_t &numMatedPair,
    size_t &num1, size_t &num2, size_t &num3, size_t &num4,
    size_t &numMore, size_t &numCollapsed, bool ignoreSliverFaces,
    size_t &num0Visible,
    size_t &num1OrMoreVisible
    )
    {
    uint32_t indexStyle = mesh.GetMeshStyle ();
    if (indexStyle == MESH_ELM_STYLE_COORDINATE_QUADS || indexStyle == MESH_ELM_STYLE_COORDINATE_TRIANGLES)
        {
        // Repeat 3 times:  Everything is visible.  We are not looking for coordinate overlap.  We are not looking at edge lengths.
        size_t numVertex = (size_t)mesh.GetPointCount ();
        size_t numFacet;
        size_t numPerFacet;
        if (indexStyle == MESH_ELM_STYLE_TRIANGLE_GRID)
            {
            numPerFacet = 3;
            }
        else
            {
            numPerFacet = 4;
            }
        numFacet = numVertex / numPerFacet;
        numPolygonEdge = numVertex;   // yes, its just one to one.  Each vertex is the base of a one edge...
        numMatedPair = 0;
        num1 = numPolygonEdge;
        num2 = num3 = num4 = numMore = 0;
        numCollapsed = 0;
        num0Visible = 0;
        num1OrMoreVisible = numPolygonEdge;
        return true;
        }
    else if (indexStyle == MESH_ELM_STYLE_TRIANGLE_GRID || indexStyle == MESH_ELM_STYLE_QUAD_GRID)
        {
        size_t numPerRow = (size_t)mesh.GetNumPerRow ();
        size_t numVertex = (size_t)mesh.GetPointCount ();
        if (numPerRow >= 2 && numVertex >= 4)
            {
            size_t numRow = numVertex / numPerRow;
            size_t numEdgePerRow = numPerRow - 1;
            size_t numEdgeRow = numRow - 1;
            size_t numQuad = numEdgePerRow * numEdgeRow;
            size_t numTri  = 2 * numQuad;
            if (indexStyle == MESH_ELM_STYLE_TRIANGLE_GRID)
                {
                numPolygonEdge = 3 * numTri;
                num1 = 2 * (numEdgePerRow + numEdgeRow);
                numMatedPair = num2 = numEdgeRow * (numPerRow - 2) + numEdgePerRow * (numRow - 2) + numQuad;
                num3 = num4 = numMore = 0;
                numCollapsed = 0;
                num0Visible = 0;
                num1OrMoreVisible = num1 + num2;
                }
            else
                {
                numPolygonEdge = 4 * numQuad;
                num1 = 2 * (numEdgePerRow + numEdgeRow);
                numMatedPair = num2 = numEdgeRow * (numPerRow - 2) + numEdgePerRow * (numRow - 2);
                num3 = num4 = numMore = 0;
                numCollapsed = 0;
                num0Visible = 0;
                num1OrMoreVisible = num1 + num2;
                }
            return true;
            }
        }
  return false;
  }

/**
Count (half)edge matching.
*/
void PolyfaceQuery::CountSharedEdges (size_t &numPolygonEdge, size_t &numMatedPair,
    size_t &num1, size_t &num2, size_t &num3, size_t &num4,
    size_t &numMore, size_t &numCollapsed, bool ignoreSliverFaces,
    size_t &num0Visible,
    size_t &num1OrMoreVisible
    ) const
    {
    if (TryCountSharedEdgesSpecialCases (*this, numPolygonEdge, numMatedPair,
            num1, num2, num3, num4, numMore, numCollapsed, ignoreSliverFaces,
            num0Visible, num1OrMoreVisible))
        return;
    size_t counts[10];
    HalfEdgeArray halfEdges (this);
    halfEdges.BuildArray (*this, false, ignoreSliverFaces);
    halfEdges.SortForEdgeMatching ();
#define CollectStrayEdges
#ifdef CollectStrayEdges
    bvector<bvector<DSegment3d>> segments;
    halfEdges.CountEdgeSharing (this, &segments, numMatedPair, counts, 6, numCollapsed, num0Visible, num1OrMoreVisible);
#else
    halfEdges.CountEdgeSharing (nullptr, nullptr, numMatedPair, counts, 6, numCollapsed, num0Visible, num1OrMoreVisible);
#endif
    numPolygonEdge = counts[0];
    num1 = counts[1];
    num2 = counts[2];
    num3 = counts[3];
    num4 = counts[4];
    numMore = counts[5];
    }

// DEPRECATED
void PolyfaceQuery::CountSharedEdges (size_t &numPolygonEdge, size_t &numMatedPair,
    size_t &num1, size_t &num2, size_t &num3, size_t &num4,
    size_t &numMore, size_t &numCollapsed, bool ignoreSliverFaces
    ) const
    {
    size_t num0Vis = 0;
    size_t num1Vis = 0;
    CountSharedEdges (numPolygonEdge, numMatedPair, num1, num2, num3, num4, numMore, numCollapsed, ignoreSliverFaces, num0Vis, num1Vis);
    }


// DEPRECATED
void PolyfaceQuery::CountSharedEdges (size_t &numPolygonEdge, size_t &numMatedPair,
    size_t &num1, size_t &num2, size_t &num3, size_t &num4,
    size_t &numMore, size_t &numCollapsed) const
    {
    return CountSharedEdges (numPolygonEdge, numMatedPair, num1, num2, num3, num4, numMore, numCollapsed, false);
    }

bool TryCollectSegmentsSpecialCases (PolyfaceQueryCR mesh, bvector<DSegment3d> &segments, bool omitInvisibles)
    {
    uint32_t indexStyle = mesh.GetMeshStyle ();
    if (indexStyle == MESH_ELM_STYLE_COORDINATE_QUADS || indexStyle == MESH_ELM_STYLE_COORDINATE_TRIANGLES)
        {
        DPoint3dCP points = mesh.GetPointCP ();
        size_t numPerFacet = indexStyle = MESH_ELM_STYLE_COORDINATE_QUADS ? 4 : 3;
        size_t numVertex = (size_t)mesh.GetPointCount ();
        for (size_t i0 = 0; i0 + numPerFacet < numVertex; i0 += numPerFacet)
            {
            for (size_t i = 0; i < numPerFacet; i++)
                {
                segments.push_back (DSegment3d::From (points[i0 + i], points[i0 + i + 1]));
                }
            }
        }
    if (indexStyle == MESH_ELM_STYLE_TRIANGLE_GRID || indexStyle == MESH_ELM_STYLE_QUAD_GRID)
        {
        DPoint3dCP points = mesh.GetPointCP ();
        size_t numPerRow = (size_t)mesh.GetNumPerRow ();
        size_t numVertex = (size_t)mesh.GetPointCount ();
        if (numPerRow >= 2 && numVertex >= 4)
            {
            size_t numRow = numVertex / numPerRow;
            // horizontals ...
            for (size_t j = 0; j < numRow; j++)
                {
                size_t i0 = j * numPerRow;
                for (size_t i = 0; i + 1 < numPerRow; i++)
                    segments.push_back (DSegment3d::From (points[i0 + i], points[i0 + i + 1]));
                }

            // verticals ...
            for (size_t i = 0; i < numPerRow; i++)
                {
                size_t j0 = i;
                for (size_t j = 0; j + 1 < numRow; j++, j0 += numPerRow)
                    segments.push_back (DSegment3d::From (points[j0], points[j0 + numPerRow]));
                }

            // diagonals ...
            for (size_t j = 0; j < numRow; j++)
                {
                size_t i0 = j * numPerRow;
                size_t i1 = i0 + numPerRow + 1; // upper right of first quad
                for (size_t i = 0; i + 1 < numPerRow; i++)
                    segments.push_back (DSegment3d::From (points[i0 + i], points[i1 + i]));
                }
            }
        return true;
        }
    return false;
    }

/**
Find all unique edges, optionally limited to those with positive indices.
*/
void PolyfaceQuery::CollectSegments (bvector<DSegment3d> &segments, bool omitInvisibles) const
    {
    if (TryCollectSegmentsSpecialCases (*this, segments, omitInvisibles))
        return;
    HalfEdgeArray halfEdges (this);
    halfEdges.BuildArray (*this, omitInvisibles);
    halfEdges.SortForEdgeMatching ();
    halfEdges.CollectSegments (segments);
    }

//! Mark edges invisible (negative index) if dihedral angle between normals is small.
bool PolyfaceHeader::MarkInvisibleEdges (double smoothAngle, DVec3dCP silhouetteVector)
    {
    uint32_t indexStyle = GetMeshStyle ();
    if (!ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ())
        return false;
    if (indexStyle == MESH_ELM_STYLE_COORDINATE_QUADS || indexStyle == MESH_ELM_STYLE_COORDINATE_TRIANGLES)
        return false;

    HalfEdgeArray halfEdges (this);
    halfEdges.BuildArray (*this, false);
    halfEdges.SortForEdgeMatching ();

    bool stat = false;
    if (!Normal ().Active ())
        {
        BuildPerFaceNormals ();
        stat = halfEdges.MarkVisibility (smoothAngle, *this, silhouetteVector);
        ClearNormals (false);
        }
    else
        {
        // There are normals -- these may blur near-smooth dihedral angles into appearing copylanar.
        // Temporarily replace them by per-face normals.
        BlockedVector<DVec3d> savedNormals = Normal ();
        BlockedVectorInt    savedNormalIndex = NormalIndex ();
        BuildPerFaceNormals ();
        stat = halfEdges.MarkVisibility (smoothAngle, *this, silhouetteVector);
        // Restore the originals . . 
        Normal () = savedNormals;
        NormalIndex () = savedNormalIndex;
        }

    return stat;
    }
    
//! Mark edges invisible (negative index) if multiple conditions are met:
//  1) dihedral angle between normals is small
//  2) the edge is a diagonal -- in each of its two incident facets it is at least edgeLengthFactor larger than any other edge.
//  3) no more than maxPerFace edges in the facet.
//! For typical use, maxPerFacet=3 and edgeLengthFactor = 1.001 restricts this to diagonals in quads.
//! 
bool PolyfaceHeader::MarkDiagonalEdgesInvisible
(
double smoothAngle,
double edgeLengthFactor,
uint32_t maxEdgesInFacet
)
    {
    uint32_t indexStyle = GetMeshStyle();
    if (!ConvertToVariableSizeSignedOneBasedIndexedFaceLoops())
        return false;
    if (indexStyle == MESH_ELM_STYLE_COORDINATE_QUADS || indexStyle == MESH_ELM_STYLE_COORDINATE_TRIANGLES)
        return false;

    HalfEdgeArray halfEdges(this);
    halfEdges.BuildArray(*this, false, false, false, edgeLengthFactor, maxEdgesInFacet);
    halfEdges.SortForEdgeMatching();

    bool stat = false;
    if (!Normal().Active())
        {
        BuildPerFaceNormals();
        stat = halfEdges.MarkVisibility(smoothAngle, *this, nullptr);
        ClearNormals(false);
        }
    else
        {
        // There are normals -- these may blur near-smooth dihedral angles into appearing copylanar.
        // Temporarily replace them by per-face normals.
        BlockedVector<DVec3d> savedNormals = Normal();
        BlockedVectorInt    savedNormalIndex = NormalIndex();
        BuildPerFaceNormals();
        stat = halfEdges.MarkVisibility(smoothAngle, *this, nullptr);
        // Restore the originals . . 
        Normal() = savedNormals;
        NormalIndex() = savedNormalIndex;
        }

    return stat;
    }

bool PolyfaceHeader::MarkTopologicalBoundariesVisible (bool preserveOtherVisibility)
    {
    if (!ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ())
        return false;
    HalfEdgeArray halfEdges (this);
    halfEdges.BuildArray (*this, false);
    halfEdges.SortForEdgeMatching ();
    return halfEdges.MarkTopologicalBoundariesVisible (*this, preserveOtherVisibility);
    }
 

void PolyfaceHeader::MarkAllEdgesVisible ()
    {
    if (!ConvertToVariableSizeSignedOneBasedIndexedFaceLoops ())
        return;
    bvector<int> &pointIndex = PointIndex ();
    // (If no indices, this array will be empty.)
    // (zeros and prior positives are unchanged ...)
    for (size_t i = 0, n = pointIndex.size (); i < n; i++)
        pointIndex[i] = abs (pointIndex[i]);
    }    

bool TryIsClosedByEdgePairingSpecialCase (PolyfaceQueryCR mesh, bool &isClosed)
    {
    isClosed = false;
    uint32_t indexStyle = mesh.GetMeshStyle ();
    if (indexStyle == MESH_ELM_STYLE_TRIANGLE_GRID || indexStyle == MESH_ELM_STYLE_QUAD_GRID)
        return true;
    if (indexStyle == MESH_ELM_STYLE_COORDINATE_QUADS || indexStyle == MESH_ELM_STYLE_COORDINATE_TRIANGLES)
        return false;
    return false;
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PolyfaceQuery::IsClosedByEdgePairing () const
    {
    bool isClosed;
    if (TryIsClosedByEdgePairingSpecialCase (*this, isClosed))
        return isClosed;

    size_t numPolygonEdge, numMatedPair, num1, num2, num3, num4, numMore, numCollapsed;
    CountSharedEdges (numPolygonEdge, numMatedPair, num1, num2, num3, num4, numMore, numCollapsed, true);
    if (numMatedPair * 2 == numPolygonEdge)
        return true;        // simple matching with no boundary.
    if (numMatedPair * 2 + numCollapsed == numPolygonEdge)
        return true;        // sphere tiled with quads at poles??
    if (numMatedPair * 2 + numCollapsed + 4 * num4 == numPolygonEdge
            && num4 == numCollapsed)
        return true;        // sphere tiled with triangles at poles.
                            // each collapsed triangle has a collapsed edge and pair of opposing edges
                            // that collapse to a singleton
    return false;
    }





bool PolyfaceToMTG_FromPolyfaceConnectivity
(
MTGFacets*       facets,
PolyfaceQueryCR  polyface,
bool ignoreDegeneracies
)
    {
    HalfEdgeArray halfEdges (&polyface);
    halfEdges.BuildArray (polyface, false, ignoreDegeneracies);
    return halfEdges.BuildGraph (polyface, facets);
    }

bool PolyfaceToMTG_FromPolyfaceConnectivity
(
MTGFacets*       facets,
PolyfaceQueryCR  polyface
)
    {
    return PolyfaceToMTG_FromPolyfaceConnectivity (facets, polyface, false);
    }



struct ConsistentOrientationEnforcer
{
HalfEdgeArray m_halfEdges;
PolyfaceHeaderR m_facets;       // supplied by caller
MeshAnnotationVector &m_messages; // supplied by caller
bvector<bvector<size_t>> &m_componentReadIndices; // supplied by caller
size_t m_numFlips;
size_t m_numErrors;
bvector<size_t> m_readIndexToHalfEdge;
bvector<size_t> m_facetReadIndexStack;

PolyfaceVisitorPtr m_searchVisitor1;


ConsistentOrientationEnforcer
(
PolyfaceHeaderR facets,
bvector<bvector<size_t>> &componentReadIndices,
MeshAnnotationVector &messages
)
    : m_facets(facets), m_numFlips (0), m_numErrors(0), m_messages (messages),
    m_halfEdges (&facets),
    m_componentReadIndices (componentReadIndices)
    {
    }
bool DoSetup ()
    {
    m_halfEdges.BuildArray (m_facets, false, false);
    m_halfEdges.SortForEdgeMatching ();
    return true;
    }

void MarkEdgesAroundFacet (PolyfaceVisitorCR visitor, bool value)
    {
    auto readIndex = visitor.GetReadIndex ();
    auto numThisFace = visitor.NumEdgesThisFace ();
    // This is the seed for a search.  Mark all its edges ..
    for (size_t i = 0; i < numThisFace; i++)
        {
        size_t heI = m_readIndexToHalfEdge [readIndex + i];
        if (heI != SIZE_MAX)
            m_halfEdges[heI].m_visible = true;
        }
    }

void MarkEdgesAroundFacet (Size2 readIndexRange, bool value)
    {
    // This is the seed for a search.  Mark all its edges ..
    for (size_t i = readIndexRange.m_dataA; i < readIndexRange.m_dataB; i++)
        {
        size_t heI = m_readIndexToHalfEdge [i];
        if (heI != SIZE_MAX)
            m_halfEdges[heI].m_visible = true;
        }
    }


// Run ReverseIndicesOneFace to fix up the polyface itself.
// Walk around
void ReverseHalfEdgeVerticesOneFace (Size2 readIndexRange)
    {
    auto numThisFace = readIndexRange.m_dataB - readIndexRange.m_dataA;
    // reverse the half edges .. uh oh, the read indices are wrong now ...
    for (size_t i = 0; i < numThisFace; i++)
        {
        size_t heI = m_readIndexToHalfEdge [readIndexRange.m_dataA + i];
        BeAssert (heI != SIZE_MAX);
        m_halfEdges[heI].ReverseVertices ();
        }
    }

Size2 SectorReadIndexToFacetReadIndexRange (size_t sectorReadIndex)
    {
    auto numPerFacet = m_facets.GetNumPerFace ();
    Size2 range (sectorReadIndex); 
    size_t numIndex = m_facets.PointIndex ().size ();
    if (numPerFacet > 1)
        {
        range.m_dataA = sectorReadIndex - (sectorReadIndex % numPerFacet);
        range.m_dataB = range.m_dataA + numPerFacet;
        }
    while (range.m_dataA > 0 && m_facets.PointIndex ()[range.m_dataA - 1] != 0)
        range.m_dataA--;
    range.m_dataB = sectorReadIndex + 1;
    while (range.m_dataB < numIndex && m_facets.PointIndex ()[range.m_dataB] != 0)
        range.m_dataB++;
    return range;
    }

void DoSearch (size_t seedReadIndex)
    {
    m_facetReadIndexStack.clear ();
    m_facetReadIndexStack.push_back (seedReadIndex);
    m_componentReadIndices.push_back (bvector<size_t>());
    while (!m_facetReadIndexStack.empty ())
        {
        size_t facetReadIndex = m_facetReadIndexStack.back ();
        m_componentReadIndices.back ().push_back (facetReadIndex);

        m_facetReadIndexStack.pop_back ();
        auto facetReadIndexRange = SectorReadIndexToFacetReadIndexRange (facetReadIndex);
        m_searchVisitor1->MoveToFacetByReadIndex (facetReadIndexRange.m_dataA);
        auto numEdges = m_searchVisitor1->NumEdgesThisFace ();
        for (size_t i = 0; i < numEdges; i++)   // for each adjacent edge
            {
            size_t readIndex = facetReadIndexRange.m_dataA + i;
            auto halfEdgeIndexA = m_readIndexToHalfEdge[readIndex];
            BeAssert (halfEdgeIndexA != SIZE_MAX);
            Size2 halfEdgeClusterRange = m_halfEdges.MateRange (halfEdgeIndexA);
            ptrdiff_t numHalfEdgesInCluster = halfEdgeClusterRange.m_dataB - halfEdgeClusterRange.m_dataA;
            if (numHalfEdgesInCluster == 1)
                {
                // true boundary edge.  ignore.
                }
            else if (numHalfEdgesInCluster == 2)
                {
                size_t halfEdgeIndexB = halfEdgeClusterRange.GetFirstDifferentIndex (halfEdgeIndexA);
                size_t sectorReadIndexB  = m_halfEdges[halfEdgeIndexB].m_readIndex;
                Size2 neighborFaceReadIndexRange = SectorReadIndexToFacetReadIndexRange (sectorReadIndexB);
                if (m_halfEdges[halfEdgeIndexB].m_visible)
                    {
                    // We are visiting this facet from a different entry point.
                    // We expect it to be a REVERSED mate ...
                    if (!m_halfEdges[halfEdgeIndexA].IsReversedMateOf (m_halfEdges[halfEdgeIndexB]))
                        {
                        m_messages.Assert (false,
                            "Common Edges with identical orientation",
                                m_halfEdges[halfEdgeIndexA].m_readIndex,
                                m_halfEdges[halfEdgeIndexB].m_readIndex
                                );
                        }
                    }
                else
                    {
                    m_facetReadIndexStack.push_back (neighborFaceReadIndexRange.m_dataA);
                    MarkEdgesAroundFacet (neighborFaceReadIndexRange, true);
                    if (m_halfEdges[halfEdgeIndexA].IsReversedMateOf (m_halfEdges[halfEdgeIndexB]))
                        {
                        // This is as it should be . . .
                        }
                    else
                        {
                        // Reverse the facet itself ..
                        m_facets.ReverseIndicesOneFace (neighborFaceReadIndexRange.m_dataA, neighborFaceReadIndexRange.m_dataB - 1);  // Those args are inclusive of swap range.
                        // And its half edges ...
                        // But note that the half edges' readIndex and nextReadIndex now lead only to "within the facet"
                        // and not to the actual matching half edges parents...
                        ReverseHalfEdgeVerticesOneFace (neighborFaceReadIndexRange);
                        }
                    }
                }
            else
                {
                // nonmanifold edge .. search stops here.
                }
            }
        }
    }
//! ASSUME the polyfaceHeader is the same as the PoyfaceQuery used to build the array !!!!!
bool Go ()
    {
    m_searchVisitor1 = PolyfaceVisitor::Attach (m_facets, false);
    m_readIndexToHalfEdge.clear ();
    m_facetReadIndexStack.clear ();
    m_numFlips = 0;
    m_numErrors = 0;

    size_t numIndex = m_facets.PointIndex ().size ();
    size_t numHalfEdge = m_halfEdges.size ();
    m_readIndexToHalfEdge.resize (numIndex);
    for (size_t i = 0; i < numIndex; i++)
        m_readIndexToHalfEdge[i] = SIZE_MAX;
    for (size_t i = 0; i < numHalfEdge; i++)
        {
        HalfEdge &he = m_halfEdges[i];
        he.m_visible = false;
        if (m_halfEdges[i].m_readIndex < numIndex)
            {
            m_readIndexToHalfEdge[he.m_readIndex] = i;
            }
        else
            BeAssert (he.m_readIndex < numIndex);
        }
    PolyfaceVisitorPtr globalVisitor = PolyfaceVisitor::Attach (m_facets, false);
    for (globalVisitor->Reset (); globalVisitor->AdvanceToNextFace ();)
        {
        size_t readIndex = globalVisitor->GetReadIndex ();
        size_t heIndex0 = m_readIndexToHalfEdge[readIndex];
        if (!m_halfEdges[heIndex0].m_visible)
            {
            MarkEdgesAroundFacet (*globalVisitor, true);
            DoSearch (readIndex);
            }
        }
    return true;
    }
};

bool PolyfaceHeader::OrientAndCollectManifoldComponents
(
bvector<bvector<size_t>> &componentReadIndices,
MeshAnnotationVector &messages//!< [out] array of status messages
)
    {
    if (PointIndex ().size () == 0)
        return false;
    ConsistentOrientationEnforcer enforcer (*this, componentReadIndices, messages);
    if (!enforcer.DoSetup ())
        return false;
    return enforcer.Go ();
    }
#define INDEX_UNORDERED_FACET INT_MAX
#define INDEX_NO_HALF_EDGE SIZE_MAX

// context for choosing a facet order to eliminate long edges and excavate inlets into facets.
// The objective is to produce a facet order so that the last facets are the first to be removed (e.g. from a convex hull)
// in an excavation that removes long exterior edges first, leading to inlets and possibly islands.
struct LongEdgeExcavactionContext
{
HalfEdgeArray m_halfEdges;
PolyfaceHeaderR m_facets;       // supplied by caller
bvector<size_t> &m_readIndexSequence;
bvector<size_t> m_readIndexToHalfEdge;

MinimumValuePriorityQueue<size_t> m_heap;


double m_maxEdgeLength;
LongEdgeExcavactionContext
(
PolyfaceHeaderR facets,
bvector<size_t> &readIndexSequence,
double maxEdgeLength
)
    : m_facets(facets),
    m_halfEdges (&facets),
    m_readIndexSequence (readIndexSequence),
    m_maxEdgeLength (maxEdgeLength)
    {
    }
// Build the half edge array.
// Sort so shared half edges are contiguous
// build the m_readIndexToHalfEdge array.
bool DoSetup ()
    {
    m_halfEdges.BuildArray (m_facets, false, false);
    m_halfEdges.SortForEdgeMatching ();
    auto numHalfEdge = m_halfEdges.size ();
    auto numReadIndex = m_facets.PointIndex().size ();
    for (size_t i = 0; i < numReadIndex; i++)
        m_readIndexToHalfEdge.push_back (INDEX_NO_HALF_EDGE);
    for (size_t i = 0; i < numHalfEdge; i++)
        m_readIndexToHalfEdge [m_halfEdges[i].m_readIndex] = i;
    return true;
    }
double HeapValue (size_t heIndex)
    {
    size_t vertex0 = m_halfEdges[heIndex].m_vertex0 - 1;
    size_t vertex1 = m_halfEdges[heIndex].m_vertex1 - 1;
    return -m_facets.Point()[vertex0].Distance (m_facets.Point()[vertex1]);
    }

void InsertHalfEdgeInHeap (size_t i)
    {
    m_heap.Insert (i, HeapValue(i));
    }


bool IsUnordered (size_t heIndex)
    {
    return m_halfEdges[heIndex].m_nodeId == INDEX_UNORDERED_FACET;
    }
bool IsCrossableEdge (size_t he)
    {
    return fabs (HeapValue (he)) > m_maxEdgeLength;
    }
size_t NextHalfEdgeAroundFacet (size_t halfEdgeIndex)
    {
    return m_readIndexToHalfEdge[m_halfEdges[halfEdgeIndex].m_faceSuccessorReadIndex];
    }

bvector<size_t> m_facetReadIndices;    // scratch space to simplify reversal.
// Walk around the half edges of a facet.
// record the facet id in each half edge.
// push SIZE_MAX on the m_readIndexSequence
// record the (reversed) readIndex sequence in m_readIndexSequence.
// 
void AssignIdAroundFacet (size_t seed, int id, bool saveToOutput)
    {
    // assign the count around the facet ..
#ifdef NOISY_EDGE_REMOVAL
    GEOMAPI_PRINTF (" (ID %d) (", (int)seed);
#endif
    m_facetReadIndices.clear ();
    for (auto he = seed;;)
        {
#ifdef NOISY_EDGE_REMOVAL
        GEOMAPI_PRINTF ("(he %d ri %d)", (int)he, (int)m_halfEdges[he].m_readIndex);
#endif
        m_facetReadIndices.push_back (m_halfEdges[he].m_readIndex);
        m_halfEdges[he].m_nodeId = id;
        he = NextHalfEdgeAroundFacet (he);
        if (he == seed)
            break;
        }
#ifdef NOISY_EDGE_REMOVAL
    GEOMAPI_PRINTF (")\n");
#endif

    if (saveToOutput)
        {
        m_readIndexSequence.push_back (SIZE_MAX);
        for (size_t i = m_facetReadIndices.size (); i-- > 0;)
            m_readIndexSequence.push_back(m_facetReadIndices[i]);
        }
    }

void EnqueueNeighbors (size_t seed)
    {
    // assign the count around the facet ..
    for (auto he = seed;;)
        {
        // visit all edge mates:
        auto range = m_halfEdges.MateRange (he);
        for (size_t i = range.m_dataA; i < range.m_dataB; i++)
            {
            if (IsUnordered (i) && IsCrossableEdge (i))
                InsertHalfEdgeInHeap (i);
            }
        he = NextHalfEdgeAroundFacet (he);
        if (he == seed)
            break;
        }
    }
// m_nodeId = order of removal.  INT_MAX is unremoved
bool Go ()
    {
    m_heap.Clear ();
    m_readIndexSequence.clear ();
    // mark all as unordered ..
    for (auto &he : m_halfEdges)
        he.m_nodeId = INDEX_UNORDERED_FACET;

    // All non-paired halfedges are seeds .  .
    Size2 range (0);
    for (size_t i = 0; i < m_halfEdges.size (); i = range.m_dataB)
        {
        range = m_halfEdges.MateRange (i);
        size_t numInRange = range.NumInRange ();
        // unconditionally insert all exteriors and fans
        if (numInRange != 2 && IsCrossableEdge (i))
            for (size_t j = range.m_dataA; j < range.m_dataB; j++)
                InsertHalfEdgeInHeap (j);
        }
    
    size_t candidate;
    double edgeLength;
    int currentId = 0;
    while (m_heap.RemoveMin (candidate, edgeLength))
        {
        if (IsUnordered (candidate))
            {
#ifdef NOISY_EDGE_REMOVAL
            GEOMAPI_PRINTF (" (candidate %d) (L %g)\n", (int)candidate, -edgeLength);
#endif
            AssignIdAroundFacet (candidate, currentId, false);
            EnqueueNeighbors (candidate);
            currentId++;
            }
        }

// assign ids to everything else ..
    for (size_t he = 0; he < m_halfEdges.size (); he++)
        {
        if (IsUnordered (he))
            AssignIdAroundFacet (he, currentId, true);
        }
    std::reverse (m_readIndexSequence.begin (), m_readIndexSequence.end ());
    return m_readIndexSequence.size () > 0;
    }
};


bool PolyfaceHeader::ConstructOrderingForLongEdgeRemoval
(
bvector<size_t> &readIndexSequence,
double maxEdgeLength
)
    {
    if (PointIndex ().size () == 0)
        return false;
    LongEdgeExcavactionContext context (*this, readIndexSequence, maxEdgeLength);
    if (!context.DoSetup ())
        return false;
    bool stat =  context.Go ();
#ifdef NOISY_EDGE_REMOVAL
    for (auto ri : readIndexSequence)
        {
        if (ri == SIZE_MAX)
            GEOMAPI_PRINTF ("\n");
        else
            GEOMAPI_PRINTF (" %d", (int)ri);
        }
#endif
    return stat;
    }

void RemapReadIndices (bvector<size_t> &readIndexSequence, bvector<int> &data, bvector<int> &work)
    {
    work = data;
    data.clear ();
    for (auto ri : readIndexSequence)
        {
        if (ri == SIZE_MAX)
            data.push_back(0);
        else
            data.push_back(work[ri]);
        }
    }
bool PolyfaceHeader::ExcavateFacetsWithLongBoundaryEdges (double maxEdgeLength)
    {
    if (PointIndex ().size () == 0)
        return false;
        bvector<size_t> newReadIndexOrder;
    bool stat = ConstructOrderingForLongEdgeRemoval (newReadIndexOrder, maxEdgeLength);
    if (stat)
        {
        bvector<int> work;
        RemapReadIndices (newReadIndexOrder, m_pointIndex, work);
        RemapReadIndices (newReadIndexOrder, m_paramIndex, work);
        RemapReadIndices (newReadIndexOrder, m_normalIndex, work);
        RemapReadIndices (newReadIndexOrder, m_colorIndex, work);
        }
    return stat;
    }

bool PolyfaceHeader::SplitByMaxEdgeLength(double splitLength, bvector<PolyfaceHeaderPtr> &splits)
    {
    splits.clear();
    PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*this);

    bvector<DPoint3d> &point = visitor->Point();
    visitor->SetNumWrap(1);
    bvector<ptrdiff_t> readIndex0, readIndex1;
    double aMax = 0.0;
    double aMin = DBL_MAX;
    for (visitor->Reset(); visitor->AdvanceToNextFace();)
        {
        size_t readIndex = visitor->GetReadIndex();
        double maxEdgeLength = 0.0;
        for (size_t i = 1; i < point.size(); i++)
            {
            maxEdgeLength = DoubleOps::Max(maxEdgeLength, point[i - 1].Distance(point[i]));
            }
        aMax = DoubleOps::Max(aMax, maxEdgeLength);
        aMin = DoubleOps::Min(aMin, maxEdgeLength);
        if (maxEdgeLength > splitLength)
            readIndex1.push_back(readIndex);
        else
            readIndex0.push_back(readIndex);
        }
    bvector<ptrdiff_t>splitIndices;
    if (readIndex0.size() > 0 && readIndex1.size() > 0)
        {
        splitIndices = readIndex0;
        splitIndices.push_back(-1);
        for (size_t i = 0; i < readIndex1.size(); i++)
            splitIndices.push_back(readIndex1[i]);
        splitIndices.push_back(-1);
        this->CopyPartitions(splitIndices, splits);
        return true;
        }
    else
        {
        return false;
        }
    }


END_BENTLEY_GEOMETRY_NAMESPACE


