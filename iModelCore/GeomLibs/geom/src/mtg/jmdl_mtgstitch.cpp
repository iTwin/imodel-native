/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "mtgintrn.h"
#include <stdlib.h>
#include <limits.h>
#define DEBUG_CODE(_code_) _code_
#define DEBUG_CODEA(_code_)
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


typedef struct _sortKey
    {
    MTGNodeId   nodeId;
    int         pointIndex;
    double      sortCoordinate;
    } MTG_SortKey;

/*----------------------------------------------------------------------+
|FUNC           cb_qsort_compareSortCoordinates                         |
|AUTHOR         RaghavanKunigahalli                     8/96            |
+----------------------------------------------------------------------*/
static bool  cb_std_compareSortCoordinates
(
const MTG_SortKey    &elem1,
const MTG_SortKey    &elem2
)
    {
    return elem1.sortCoordinate < elem2.sortCoordinate;
    }

/*----------------------------------------------------------------------+
|FUNC           samePoint                                               |
|AUTHOR         RaghavanKunigahalli                     8/96            |
+----------------------------------------------------------------------*/
static bool    samePoint
(
const DPoint3d *pPoint1,
const DPoint3d *pPoint2,
double epsilon
)
    {
    return (fabs(pPoint1->x - pPoint2->x) <= epsilon)
        && (fabs(pPoint1->y - pPoint2->y) <= epsilon)
        && (fabs(pPoint1->z - pPoint2->z) <= epsilon);
    }


/**
* Construct parallel arrays of node and vertex ids for matching vertices.
* @param    pFacetHeader => facets to examine.
* @param pNodeIdArray <= array of nodeId's grouped by vertex cluster.
* @param pVertexIdArray <= parallel array of vertex ids.
* @param absTol => absolute tolerance for coordinate comparison.
* @param relTol => relative tolerance for coordinate comparison.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_createVertexClusterArrays
(
MTGFacets           *pFacetHeader,
EmbeddedIntArray    *pNodeIdArray,
EmbeddedIntArray    *pVertexIdArray,
double              absTol,
double              relTol
)
    {
    int         pivotIndex, testIndex, currVertexId, nodeCount = 0;
    double      epsilon, epsilon2;
    MTG_SortKey sortInfo, *pTestKey, *pPivotKey;
    DPoint3d    randomVector, testPoint, pivotPoint;

    bvector<int> nodeIdArray;
    bvector<int> vertexIdArray;
    bvector <DPoint3d>      &vertexArray = pFacetHeader->vertexArrayHdr;
    MTGGraph                *pGraph;

    static double   s_defaultRelTol = 1.0e-6;
    static double   s_defaultAbsTol = 1.0e-14;

    if (!pFacetHeader)
        return;

    pGraph = &pFacetHeader->graphHdr;

    randomVector.Init ( 0.133333333334, 1.414213562373, 1.666666666667);
    randomVector.Normalize ();

    // compute tolerances
    if (absTol <= 0.0)
        absTol = 0.0;
    if (relTol <= 0.0)
        relTol = 0.0;
    if (absTol == 0.0 && relTol == 0.0)
        {
        relTol = s_defaultRelTol;
        absTol = s_defaultAbsTol;
        }
    epsilon = absTol;
    if (relTol > 0.0)
        {
        epsilon += relTol * bsiDPoint3d_getLargestCoordinateDifference (vertexArray.data (), (int) vertexArray.size ());
        }
    epsilon2 = sqrt (3.0) * epsilon;   // l_infinity vertex tol mapped to linear l_2 tol along random vector

    bvector <MTG_SortKey> dotArray;

    // sort vertices by magnitude*cos(angle to randomVector) in increasing order
    MTGARRAY_SET_LOOP (currNodeId, pGraph)
        {
        jmdlMTGGraph_getLabel (pGraph, &currVertexId, currNodeId, pFacetHeader->vertexLabelOffset);
        testPoint = vertexArray[(size_t)currVertexId];

        sortInfo.nodeId = currNodeId;
        sortInfo.pointIndex = currVertexId;
        sortInfo.sortCoordinate = testPoint.DotProduct (randomVector);
        dotArray.push_back (sortInfo);
        nodeCount++;
        }
    MTGARRAY_END_SET_LOOP (currNodeId, pGraph)

    std::sort (dotArray.begin (), dotArray.end (), cb_std_compareSortCoordinates);
    
    /* Sorting along the single direction does a good but not complete job of bringing
        geometrically clustered points together in the sort array.
        Do linear search "within epsilon" in the sort order to complete the filtering.
        Push nodeId's back to NULL as nodes are gathered into clusters. */
    for (pivotIndex = 0, pPivotKey = dotArray.data (); pivotIndex < nodeCount; pivotIndex++, pPivotKey++)
        {
        if (pPivotKey->nodeId != MTG_NULL_NODEID)
            {
            /* The nodeId is still there, therefore this is a the start of a new cluster. */
            double testLimit = pPivotKey->sortCoordinate + epsilon2;
            pivotPoint = vertexArray[(size_t)pPivotKey->pointIndex];
            nodeIdArray.push_back (pPivotKey->nodeId);
            vertexIdArray.push_back (pPivotKey->pointIndex);

            for (testIndex = pivotIndex + 1,
                 pTestKey = pPivotKey + 1;
                 testIndex < nodeCount && pTestKey->sortCoordinate <= testLimit;
                 testIndex++,
                 pTestKey++
                 )
                {
                if (pTestKey->nodeId != MTG_NULL_NODEID)
                    {
                    testPoint = vertexArray[(size_t)pTestKey->pointIndex];
                    if (samePoint (&pivotPoint, &testPoint, epsilon))
                        {
                        nodeIdArray.push_back (pTestKey->nodeId);       // nodeId from test, but...
                        vertexIdArray.push_back (pPivotKey->pointIndex); // vertex index from base.  Contiguous vertex indices are the cluster.
                        pTestKey->nodeId = MTG_NULL_NODEID;
                        }
                    }
                }
            }
        }

    if (pNodeIdArray)
        pNodeIdArray->swap (nodeIdArray);
    if (pVertexIdArray)
        pVertexIdArray->swap (vertexIdArray);
    }

/*----------------------------------------------------------------------+
|FUNC           jmdlMTGChain_collapseVertices                           |
|AUTHOR         RaghavanKunigahalli                     9/96            |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void jmdlMTGFacets_relableVertexClustersFromArrays
(
MTGFacets *     pFacetHeader,
EmbeddedIntArray      *pNodeIdArray,
EmbeddedIntArray      *pVertexIdArray
)
    {
    int baseVertexId = 0, currNodeId, currVertexId, baseLabel = 0;
    size_t i, count = pNodeIdArray->size ();
    baseVertexId = -1;
    for (i = 0; i < count; i++)
        {
        currVertexId = pVertexIdArray->at(i);
        currNodeId   = pNodeIdArray->at(i);

        if (i == 0 || currVertexId != baseVertexId)
            {
            /* We are at the beginning of a new cluster. Grab it's label to spread to successors in the cluster. */
            baseVertexId = currVertexId;
            jmdlMTGGraph_getLabel (&pFacetHeader->graphHdr, &baseLabel, currNodeId, pFacetHeader->vertexLabelOffset);
            }
        else
            {
            /* Another node in the same cluster. Drop of the base node's label. */
            jmdlMTGGraph_setLabel (&pFacetHeader->graphHdr, currNodeId, pFacetHeader->vertexLabelOffset, baseLabel);
            }
        }
    }


/**
@description Find and delete all isolated triangles which reference two or fewer distinct vertices.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
static void jmdlMTGFacets_deleteDegenerateTriangles
(
MTGFacets           *pFacetHeader
)
    {
    MTGGraph    *pGraph     = &pFacetHeader->graphHdr;
    MTGMask     visitMask   = jmdlMTGGraph_grabMask (pGraph);
    int numAroundFace;
    int numComplexVertex;
    int numDegenerateEdge;
    int vertexIndex[4];
    int nodeIdAroundFace[3];
    jmdlMTGGraph_clearMaskInSet (pGraph, visitMask);

    MTGARRAY_SET_LOOP (seedNodeId, pGraph)
        {
        if (!jmdlMTGGraph_getMask (pGraph, seedNodeId, visitMask))
            {
            jmdlMTGGraph_setMaskAroundFace (pGraph, seedNodeId, visitMask);
            numAroundFace = numComplexVertex = 0;
            MTGARRAY_FACE_LOOP (currNodeId, pGraph, seedNodeId)
                {
                numAroundFace++;
                if ((int)pGraph->CountNodesAroundVertex (currNodeId) != 2)
                    numComplexVertex++;
                }
            MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, seedNodeId)

            if (numAroundFace == 3 && numComplexVertex == 0)
                {
                int i = 0;
                // We have found a completely isolated triangle.
                // If any edge references the same vertex at both ends, delete all the edges.
                MTGARRAY_FACE_LOOP (currNodeId, pGraph, seedNodeId)
                    {
                    jmdlMTGFacets_getNodeVertexIndex (pFacetHeader, &vertexIndex[i], currNodeId);
                    nodeIdAroundFace[i] = currNodeId;
                    i++;
                    }
                MTGARRAY_END_FACE_LOOP (currNodeId, pGraph, seedNodeId)
                vertexIndex[3] = vertexIndex[0];
                numDegenerateEdge = 0;
                for (i = 0; i < 3; i++)
                    {
                    if (vertexIndex[i] == vertexIndex[i+1])
                        numDegenerateEdge++;
                    }
                if (numDegenerateEdge > 0)
                    {
                    for (i = 0; i < 3; i++)
                        {
                        jmdlMTGGraph_dropEdge (pGraph, nodeIdAroundFace[i]);
                        }
                    }
                }
            }
        }
    MTGARRAY_END_SET_LOOP (seedNodeId, pGraph)
    }



struct MTGMerge_EdgeKey
    {
    MTGNodeId   nodeId;
    int         lowVertexIndex;
    int         highVertexIndex;
    double      angle;

    // save vertex indices with possible reversal for (low, high) relationship !!!
    MTGMerge_EdgeKey (MTGNodeId _nodeId, int vertexIndex1, int vertexIndex2)
        {
        nodeId = _nodeId;
        if (vertexIndex1 < vertexIndex2)
            {
            lowVertexIndex  = vertexIndex1;
            highVertexIndex = vertexIndex2;
            }
        else
            {
            lowVertexIndex  = vertexIndex2;
            highVertexIndex = vertexIndex1;
            }
        angle = 0.0;
        }

    bool SameVerticesOnEdge (MTGMerge_EdgeKey const &other) const
        {
        return lowVertexIndex == other.lowVertexIndex
            && highVertexIndex == other.highVertexIndex;
        }
    bool NearlyEqualAnglesAllowPeriodShift (MTGMerge_EdgeKey const &other) const
        {
        return Angle::NearlyEqualAllowPeriodShift (angle, other.angle);
        }
    };

typedef struct MTGMerge_EdgeKey const & MTGMerge_EdgeKeyCR;
typedef struct MTGMerge_EdgeKey const & MTGMerge_EdgeKeyR;

// LT test using lowVertexIndex then highVertexIndex.
static bool compareId (MTGMerge_EdgeKeyCR edgeA, MTGMerge_EdgeKeyCR edgeB)
    {

    if (edgeA.lowVertexIndex < edgeB.lowVertexIndex)
        return  true;
    else if (edgeA.lowVertexIndex > edgeB.lowVertexIndex)
        return  false;
    else
        {
        if (edgeA.highVertexIndex < edgeB.highVertexIndex)
            return  true;
        else if (edgeA.highVertexIndex > edgeB.highVertexIndex)
            return false;
        else
            return  false; // EQUAL
        }
    }

// LT test using angle
static bool compareAngle (MTGMerge_EdgeKeyCR edgeA, MTGMerge_EdgeKeyCR edgeB)
    {

    if (edgeA.angle < edgeB.angle)
        return  true;
    else if (edgeA.angle > edgeB.angle)
        return  false;
    else
        {
        return false; // EQUAL
        }
    }

// Conditional access to bvector, with int to size_t cast.
template <typename T>
static bool TryGet (bvector <T> &source, T *data, int i)
    {
    size_t ii = (size_t)i;
    if (ii >= source.size ())
        return false;
    *data = source[i];
    return true;
    }

static int s_noisy = 0;
// INPUT STATE:
//   Each face exists as an isolated edge loop.
//   (i.e. each node's vertex successor is its "through the face" partner)
struct EdgeStarAssembler
{
private:
MTGFacets *m_pFacets;
MTGGraph &m_graph;
int m_noisy;

bvector<MTGNodeId> faceStartArray;  // one node per original face loop -- i.e. both sides present.
bvector<MTGNodeId> m_partnerThroughFace;        // partner "through" the face.  (At start, these are VSucc of each other.)

bvector < MTGMerge_EdgeKey> m_globalEdgeArray;
bvector <MTGMerge_EdgeKey> m_edgeStarArray;


static bool DelimitEdgeStar (size_t readIndex0, size_t &readIndex1, bvector<MTGMerge_EdgeKey> const &source)
    {
    size_t n = source.size ();
    readIndex1 = readIndex0;
    if (readIndex1 >= n)
        return false;
    readIndex1++;
    for (;readIndex1 < n && source[readIndex1].SameVerticesOnEdge (source[readIndex0]); readIndex1++)
        {
        }
    return true;
    }



// Look forward from i0 for edges with the same vertex indices.
// collect all that do not have mask set
// advance i0 to next block start.
// return false if no candidates.
static void CollectUnMaskedEdgeStarEntries (size_t readIndex0, size_t readIndex1,
bvector<MTGMerge_EdgeKey> const &source,
MTGGraph::ScopedMask &mask,
bvector<MTGMerge_EdgeKey> &dest
)
    {
    dest.clear ();
    for (size_t readIndex = readIndex0; readIndex < readIndex1; readIndex++)
        {
        if (!mask.IsSetAt (source[readIndex].nodeId))
            dest.push_back (source[readIndex]);
        }
    }

MTGGraph::ScopedMask m_nullVolumeMask;
MTGGraph::ScopedMask m_positiveSideOfFaceMask;
MTGGraph::ScopedMask m_duplicateFacetMask;

size_t numNodeId;

bool TryGetEdgeVector (MTGMerge_EdgeKeyCR key, DVec3dR edgeVector)
    {
    DPoint3d xyzA, xyzB;
    if (  TryGet<DPoint3d>  (m_pFacets->vertexArrayHdr, &xyzA, key.lowVertexIndex)
       && TryGet<DPoint3d>  (m_pFacets->vertexArrayHdr, &xyzB, key.highVertexIndex)
       )
        {
        edgeVector = DVec3d::FromStartEnd (xyzA, xyzB);
        return true;
        }
    return false;
    }

double AngleBetweenEdgeNeighborhoods (MTGMerge_EdgeKeyCR keyA, MTGMerge_EdgeKeyCR keyB, DVec3dCR edgeVector)
    {
    // Remark edgeVector is assumed to be from lowVertexIndex towards high -- caller can set it up with the EdgeVector method...
    BeAssert (keyA.lowVertexIndex == keyB.lowVertexIndex);
    BeAssert (keyA.highVertexIndex == keyB.highVertexIndex);
    DVec3d normalA, normalB;

    if (   TryGetFaceNormalAtNode (keyA.nodeId, normalA)
        && TryGetFaceNormalAtNode (keyB.nodeId, normalB)
        )
        {
        return normalA.SignedAngleTo (normalB, edgeVector);
        }
    BeAssert (false);
    return 0.0;
    }

bool ConfirmSimpleLoopsAndRecordFacePartners ()
    {
    m_partnerThroughFace.clear ();
    m_partnerThroughFace.resize (numNodeId, MTG_NULL_NODEID);

    // CONFIRM: all facets are isolated
    // record the around-vertex relationship (which we also understand to be "through" the face)
    MTGARRAY_SET_LOOP (node, &m_graph)
        {
        if (2 != m_graph.CountNodesAroundVertex (node))
            return false;
        m_partnerThroughFace[node] = m_graph.VSucc (node);
        }
    MTGARRAY_END_SET_LOOP (node, &m_graph)
    return true;
    }

// Create the m_globalEdgeArray:
// Each incidence of a face with an edge appears exactly once, with the lower numbered vertex first, along with the
//   node at that end.
bool BuildEdgeArray ()
    {
    // Find all mtg nodes sitting at lower-numbered vertex of their edge.
    // Collect these as representatives of a face incident to the edge.
    MTGARRAY_SET_LOOP (currNodeId, &m_graph)
        {
        ptrdiff_t vertexIndexA, vertexIndexB;
        if (   m_pFacets->NodeToVertexIndex (currNodeId, vertexIndexA)
            && m_pFacets->NodeToVertexIndex (m_graph.FSucc (currNodeId), vertexIndexB)
            && vertexIndexA < vertexIndexB
            )
            {
            m_globalEdgeArray.push_back (MTGMerge_EdgeKey (currNodeId, (int)vertexIndexA, (int)vertexIndexB));
            }
        }
    MTGARRAY_END_SET_LOOP (currNodeId, &m_graph)
    return true;
    }
struct FaceData
{
DVec3d m_normal;
MTGNodeId m_nodeId;
FaceData (DVec3dCR normal, MTGNodeId nodeId)
    {
    m_normal = normal;
    m_nodeId = nodeId;
    }
};

bvector<FaceData> m_faceData;
bvector<size_t> m_nodeToFaceIndex;


// look up the normal -- negate if the node is on the face backside.
bool TryGetFaceNormalAtNode (MTGNodeId nodeId, DVec3dR normal)
    {
    if (nodeId >= (int)m_nodeToFaceIndex.size ())
        return false;
    size_t faceIndex = m_nodeToFaceIndex [nodeId];
    normal = m_faceData[faceIndex].m_normal;
    if (!m_positiveSideOfFaceMask.GetAt (nodeId))
        normal.Negate ();
    return true;
    }

size_t AddFaceData (DVec3dCR normal, MTGNodeId nodeId)
    {
    size_t faceIndex = m_faceData.size ();
    m_faceData.push_back (FaceData (normal, nodeId));
    m_positiveSideOfFaceMask.SetAroundFace (nodeId);
    return faceIndex;
    }


bool BuildNodeToFaceDataArray ()
    {
    m_faceData.clear ();
    m_nodeToFaceIndex.clear ();
    m_nodeToFaceIndex.resize (numNodeId, SIZE_MAX);
    bvector<DPoint3d> xyzAroundFace;
    int vertexIdOffset = m_pFacets->vertexLabelOffset;
    size_t numErrors = 0;
    for (MTGNodeId faceSeed : faceStartArray)
        {
        // faceStartArray has entries for both sides of each original face -- only do this setup from one side ...
        if (m_nodeToFaceIndex[faceSeed] != SIZE_MAX)
            continue;

        if (jmdlMTGFacets_getFaceCoordinates ((jmdlMTGFacets_getGraph (m_pFacets)),
                        &xyzAroundFace, (&m_pFacets->vertexArrayHdr), faceSeed, vertexIdOffset)
            )
            {
            DPoint3d * pCoordinateArray = xyzAroundFace.data ();
            size_t nVertex = xyzAroundFace.size ();
            DVec3d normal;
            bsiGeom_polygonNormal (&normal, NULL, pCoordinateArray, (int)nVertex);
            size_t faceIndex = AddFaceData (normal, faceSeed);
            MTGARRAY_FACE_LOOP (currNodeId, &m_graph, faceSeed)
                {
                m_nodeToFaceIndex[currNodeId] = faceIndex;
                m_nodeToFaceIndex[m_partnerThroughFace[currNodeId]] = faceIndex;
                }
            MTGARRAY_END_FACE_LOOP (currNodeId, &m_graph, faceSeed)
            }
        else
            {
            numErrors++;
            }
        }
    return numErrors == 0;
    }

// direct sort step . . .
void SortEdgeBlockByAngles_doSort (size_t i0, size_t i1)
    {
    std::sort (m_globalEdgeArray.begin () + i0, m_globalEdgeArray.begin () + i1, compareAngle);
    }

// sort within the edge block -- repeat if front, back of block have NearlyEqualAllowPeriodShift
void SortEdgeBlockByAngles (size_t i0, size_t i1)
    {
    size_t n = i1 - i0;
    if (n > 2)
        {
        // Simple sort ...
        SortEdgeBlockByAngles_doSort (i0, i1);
        size_t i2 = i1 - 1;
        size_t numShift = 0;
        double shiftRadians = Angle::TwoPi ();
        while (i2 > i0 && m_globalEdgeArray[i0].NearlyEqualAnglesAllowPeriodShift (m_globalEdgeArray[i2]))
            {
            m_globalEdgeArray[i2].angle -= shiftRadians;
            i2--;
            numShift++;
            }
        if (numShift > 0)
            SortEdgeBlockByAngles_doSort (i0, i1);
        }
    }

bool SameVerticesAroundFaces (MTGNodeId nodeA0, MTGNodeId nodeB0)
    {
    size_t numA = m_graph.CountNodesAroundFace (nodeA0);
    size_t numB = m_graph.CountNodesAroundFace (nodeB0);
    if (numA != numB)
        return false;
    MTGNodeId nodeA = nodeA0, nodeB = nodeB0;
    // edgeTwist plays with vertex loops, leaves face loops completely unchanged.
    for (size_t i = 0; i < numA; i++)
        {
        ptrdiff_t vertexIndexA, vertexIndexB;
        m_pFacets->NodeToVertexIndex (nodeA, vertexIndexA);
        m_pFacets->NodeToVertexIndex (nodeB, vertexIndexB);
        if (vertexIndexA != vertexIndexB)
            return false;
        nodeA = m_graph.FSucc (nodeA);
        nodeB = m_graph.FSucc (nodeB);
        }
    return true;
    }

bool PasteFaces (MTGNodeId nodeA0, MTGNodeId nodeB0)
    {
    size_t numA = m_graph.CountNodesAroundFace (nodeA0);
    size_t numB = m_graph.CountNodesAroundFace (nodeB0);
    if (numA != numB)
        return false;
    MTGNodeId nodeA = nodeA0, nodeB = nodeB0;
    for (size_t i = 0; i < numA; i++)
        {
        jmdlMTGGraph_edgeTwist (&m_graph, nodeA, nodeB);
        if (IsNoisy(99))
            GEOMAPI_PRINTF ("  (Paste %d %d)\n", nodeA, nodeB);
        nodeA = m_graph.FSucc (nodeA);
        nodeB = m_graph.FSucc (nodeB);
        }
    return true;
    }



public:

EdgeStarAssembler (MTGFacets*facets, int noisy = 0)
    : m_pFacets (facets),
    m_graph (*jmdlMTGFacets_getGraph (facets)),
    m_nullVolumeMask (m_graph),
    m_positiveSideOfFaceMask (m_graph),
    m_duplicateFacetMask (m_graph),
    m_noisy(noisy)
    {
    // various null values -- Setup does the real work, and can return error flag
    numNodeId = 0;
    }

bool IsNoisy (int trigger)
    {
    return m_noisy >= trigger;
    }

bool Setup ()
    {
    numNodeId = m_graph.GetNodeIdCount ();   // This may include deleted nodes.
    m_graph.CollectFaceLoops (faceStartArray);
    return ConfirmSimpleLoopsAndRecordFacePartners ()
        && BuildEdgeArray ()
        && BuildNodeToFaceDataArray ();
    }

bool     SortAroundEdges (bool deleteDuplicateFaces = true)
    {
    int vertexIdOffset = m_pFacets->vertexLabelOffset;
    bool    boolstat = true;
    if (IsNoisy (99))
        jmdlMTGFacets_printFaceLoops (m_pFacets, m_nullVolumeMask.Get (), '@');

    std::sort (m_globalEdgeArray.begin (), m_globalEdgeArray.end (), compareId);

    // Compute face normals for each face loop
    bvector <DVec3d> vertexNormalArray;
    bvector <DPoint3d>faceLoopArray;

    vertexNormalArray.resize (numNodeId);
    DVec3d zeroVector = DVec3d::From (0,0,0);
    for (size_t i = 0; i < (size_t) numNodeId; i++)
        vertexNormalArray.push_back (zeroVector);

    const DPoint3d *pCoordinateArray;
    DVec3d normal;
    int errorCount = 0;

    for (MTGNodeId startId : faceStartArray)
        {
        if (jmdlMTGFacets_getFaceCoordinates ((jmdlMTGFacets_getGraph (m_pFacets)),
                        &faceLoopArray, (&m_pFacets->vertexArrayHdr), startId, vertexIdOffset)
            )
            {
            pCoordinateArray = faceLoopArray.data ();
            size_t nVertex = faceLoopArray.size ();
            bsiGeom_polygonNormal (&normal, NULL, pCoordinateArray, (int)nVertex);
            MTGARRAY_FACE_LOOP (currNodeId, &m_graph, startId)
                {
                vertexNormalArray[currNodeId] = normal;
                }
            MTGARRAY_END_FACE_LOOP (currNodeId, &m_graph, startId)
            }
        else
            {
            errorCount++;
            }
        }


    // Find blocks of nodes around common edges
    // Within each block:
    //     compute angle from first panel to each of the others.
    //     sort by angle.
    for (size_t i0 = 0, i1 = 0, numEdge = m_globalEdgeArray.size (); i0 < numEdge; i0 = i1)
        {
        // Find out how many successive nodes are along the same edge, and compute
        // their angle with respect to the edge ....
        m_globalEdgeArray[i0].angle = 0.0;
        DVec3d edgeVector;
        if (!TryGetEdgeVector (m_globalEdgeArray[i0], edgeVector))
            {
            BeAssert (false);
            }
        else
            {
            for (i1 = i0 + 1; i1 < numEdge; i1++)
                {
                if (!m_globalEdgeArray[i0].SameVerticesOnEdge (m_globalEdgeArray[i1]))
                    break;
                m_globalEdgeArray[i1].angle = AngleBetweenEdgeNeighborhoods (m_globalEdgeArray[i0], m_globalEdgeArray[i1], edgeVector);
                }
            }
        SortEdgeBlockByAngles (i0, i1);
        }

    // The facets are still standalone.
    // Within each block (per edge) . . 
    //    find duplicate facets.
    //    apply m_duplicateFacetMask (So it will be seen from other m_globalEdgeArray entries that have other edges of the facet)
    for (size_t i0 = 0, i1 = 0;
         DelimitEdgeStar (i0, i1, m_globalEdgeArray); i0 = i1)
        {
        // n-squared search for duplicate faces .  . .
        for (size_t k0 = i0; k0 + 1 < i1; k0++)
            {
            MTGNodeId node0 = m_globalEdgeArray[k0].nodeId;
            if (m_duplicateFacetMask.IsSetAt (node0))
                continue;
            for (size_t k1 = k0 + 1; k1 < i1; k1++)
                {
                MTGNodeId node1 = m_globalEdgeArray[k1].nodeId;
                if (m_duplicateFacetMask.IsSetAt (node1))
                    continue;
                if (SameVerticesAroundFaces (node0, node1))
                    {
                    m_duplicateFacetMask.SetAroundFace (node1);
                    m_duplicateFacetMask.SetAroundFace (m_graph.VSucc (node1));
                    }
                }
            }
    }

    if (IsNoisy (99))
        jmdlMTGFacets_printFaceLoops (m_pFacets, m_duplicateFacetMask.Get (), '@');
    size_t numEdgeTwist = 0;
    for (size_t i0 = 0, i1 = 0;
         DelimitEdgeStar (i0, i1, m_globalEdgeArray); i0 = i1)
        {
        CollectUnMaskedEdgeStarEntries (i0, i1, m_globalEdgeArray, m_duplicateFacetMask, m_edgeStarArray);
        size_t n = m_edgeStarArray.size ();
        for (size_t j0 = 0; j0 + 1 < n; j0++)
            {
            size_t  j1 = j0 + 1;
            MTGNodeId node0 = m_edgeStarArray[j0].nodeId;
            MTGNodeId node1 = m_edgeStarArray[j1].nodeId;
            jmdlMTGGraph_edgeTwist ((jmdlMTGFacets_getGraph (m_pFacets)), node0, node1);
            if (IsNoisy (99))
                GEOMAPI_PRINTF(" (edgeTwist %d %d)\n", node0, node1);
            numEdgeTwist++;
            }
        }

    if (IsNoisy (99))
        {
        GEOMAPI_PRINTF(" (numEdgeTwist %d)\n", (int)numEdgeTwist);
        jmdlMTGFacets_printFaceLoops (m_pFacets, m_nullVolumeMask.Get (), '@');
        }
    return boolstat;
    }
};


/**
* @param    pFacetHeader <= facet set to be stithed.
* @param absTol => absolute tolerance for vertex consolidation.
* @param relTol => relative tolerance for vertex consolidation.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_stitchFacetsForMultiVolume
(
MTGFacets * pFacetHeader,
double      absTol,
double      relTol
)
    {
    if (!pFacetHeader || pFacetHeader->vertexArrayHdr.size () < 2)
        return false;

    bvector <int> nodeIdArray;
    bvector <int> vertexIdArray;

    jmdlMTGFacets_createVertexClusterArrays (pFacetHeader, &nodeIdArray, &vertexIdArray, absTol, relTol);
    jmdlMTGFacets_relableVertexClustersFromArrays (pFacetHeader, &nodeIdArray, &vertexIdArray);
    jmdlMTGFacets_deleteDegenerateTriangles (pFacetHeader);
    EdgeStarAssembler assembler (pFacetHeader, s_noisy);
    bool stat = false;
    if (assembler.Setup ())
        {
        stat = assembler.SortAroundEdges ();
        }
    return  stat;
    }



/**
* @param    pFacetHeader <= facet set to be stithed.
* @param absTol => absolute tolerance for vertex consolidation.
* @param relTol => relative tolerance for vertex consolidation.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_stitchFacets
(
MTGFacets * pFacetHeader,
double      absTol,
double      relTol
)
    {
    bool    boolstat = true;

    if (!pFacetHeader || pFacetHeader->vertexArrayHdr.size () < 2)
        return false;

    bvector <int> nodeIdArray;
    bvector <int> vertexIdArray;

    jmdlMTGFacets_createVertexClusterArrays (pFacetHeader, &nodeIdArray, &vertexIdArray, absTol, relTol);
    jmdlMTGFacets_relableVertexClustersFromArrays (pFacetHeader, &nodeIdArray, &vertexIdArray);
    jmdlMTGFacets_deleteDegenerateTriangles (pFacetHeader);
    boolstat = jmdlMTGChain_twistEdgeStars (pFacetHeader);

    return  boolstat;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
