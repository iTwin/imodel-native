/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/XYBucketSearch.h>


#include <Geom/cluster.h>
#include <Vu/VuApi.h>
//#include <assert.h>

#define     BUFFER_SIZE         10
//#define DEBUG_CODE(__contents__) __contents__
#define DEBUG_CODE(__contents__)

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
static int s_noisy = 0;
/*---------------------------------------------------------------------------------**//**
@description Delete all edges with specified mask on both sides.
* @bsimethod                                    EarlinLutz                  05/04
+---------------+---------------+---------------+---------------+---------------+------*/
Public void jmdlMTGFacets_dropDoubleMaskedEdges
(
MTGFacets       *pFacets,
MTGMask         mask
)
    {
    MTGGraph *pGraph = &pFacets->graphHdr;
    MTGNodeId mateNodeId;
    if (s_noisy)
        jmdlMTGGraph_printFaceLoops (pGraph);
    MTGARRAY_SET_LOOP (baseNodeId, pGraph)
        {
        mateNodeId = jmdlMTGGraph_getEdgeMate (pGraph, baseNodeId);
        if (jmdlMTGGraph_getMask (pGraph, baseNodeId, mask)
            && jmdlMTGGraph_getMask (pGraph, mateNodeId, mask))
                jmdlMTGGraph_dropEdge (pGraph, baseNodeId);
        }
    MTGARRAY_END_SET_LOOP (baseNodeId, pGraph)
    }



// +1 all exteriors are forward.
// -1 all exteriors are reversed.
// 0 other
// numSourcePoints = number of points in original polygon (pretriangulation)
// indices = signed, one-based indices (caller guarantees all nonzero)
static int  IndexOrderState (EmbeddedIntArrayP indices, int k0, int numIndices, int numSourcePoints, int &numExteriorForward, int &numExteriorReversed, int &numInterior, int &numOther)
    {
    numExteriorForward = numExteriorReversed = numOther = numInterior = 0;

    for (int i = 0; i < numIndices; i++)
        {
        int j = (i + 1) % numIndices;
        int indexI, indexJ;
        if (   jmdlEmbeddedIntArray_getInt (indices, &indexI, k0 + i)
            && jmdlEmbeddedIntArray_getInt (indices, &indexJ, k0 + j))
            {
            if (indexI < 0)
                numInterior++;
            else
                {
                // convert to zero based:
                int indexJ0 = abs (indexJ) - 1;
                int indexI0 = abs (indexI) - 1;
                if (indexJ0 == (indexI0 + 1) % numSourcePoints)
                    numExteriorForward++;
                else if (indexI0 == (indexJ0 + 1) % numSourcePoints)
                    numExteriorReversed++;
                else 
                    numOther++;
                }
            }
        }
    if (numOther > 0)
        return 0;
    if (numExteriorForward > 0 && numExteriorReversed == 0)
        return 1;
    if (numExteriorReversed > 0 && numExteriorForward == 0)
        return -1;
    return 0;
    }

static double s_xyUnitVectorTol = 1.0e-9;
static double s_abstol = 1.0e-8;
// Caller promises unit length so z tolerance is zimple....
bool IsXYUnitVector (DVec3dCR vector)
    {
    return fabs (vector.z) < s_xyUnitVectorTol;
    }
struct NodeData
{
MTGNodeId m_nodeId;
DPoint3d m_xyz;
bool IsSameNode (NodeData const &other) const { return m_nodeId == other.m_nodeId;}
bool IsSameNode (MTGNodeId other) const { return m_nodeId == other;}

NodeData ()
  : m_nodeId (MTG_NULL_NODEID)
  {
  m_xyz.Zero ();
  }
NodeData (MTGFacets *facets, MTGNodeId nodeId)
    {
    m_nodeId = nodeId;
    jmdlMTGFacets_getNodeCoordinates (facets, &m_xyz, m_nodeId);
    }
};

//
//         A0<---------A1
//          |
//          |
//         B0-------->B1
// nodes A0,B0 are vertically "left of the opening"
// There may be additional nodes along A0..B0...
//   A0 and B0 may be the same node !!
// A1, B1 are adjacent

struct EdgePairNodes
{
EdgePairNodes (MTGFacets *facets)
    : m_facets (facets), m_graph (jmdlMTGFacets_getGraph (facets))
    {
    abstol = s_abstol;
    }
private:
MTGFacets *m_facets;
MTGGraph *m_graph;
double abstol;

NodeData m_A0, m_A1, m_B0, m_B1;
public:

MTGNodeId NodeIdA0 () const { return m_A0.m_nodeId;}
MTGNodeId NodeIdA1 () const { return m_A1.m_nodeId;}
MTGNodeId NodeIdB0 () const { return m_B0.m_nodeId;}
MTGNodeId NodeIdB1 () const { return m_B1.m_nodeId;}

NodeData NodeDataA0 () const { return m_A0;}
NodeData NodeDataA1 () const { return m_A1;}
NodeData NodeDataB0 () const { return m_B0;}
NodeData NodeDataB1 () const { return m_B1;}




NodeData LoadNodeData (MTGNodeId nodeId)
    {
    return NodeData (m_facets, nodeId);
    }

NodeData LoadAdjacentNodeData (MTGNodeId nodeId, bool forward)
    {
    return NodeData (m_facets, forward ? jmdlMTGGraph_getFSucc (m_graph, nodeId) : jmdlMTGGraph_getFPred (m_graph, nodeId));
    }


bool SameStart (EdgePairNodes const & other) const { return NodeIdA0() == other.NodeIdA0 ();}
bool StartsFromEdge () const
    {
    return m_A0.m_nodeId != m_B0.m_nodeId;
    }
void LoadA0A1FromA1 (MTGNodeId nodeId)
    {
    m_A1 = NodeData (m_facets, nodeId);
    m_A0 = NodeData (m_facets, jmdlMTGGraph_getFSucc (m_graph, nodeId));
    }

void LoadA0A1FromA0 (MTGNodeId nodeId)
    {
    m_A0 = NodeData (m_facets, nodeId);
    m_A1 = NodeData (m_facets, jmdlMTGGraph_getFPred (m_graph, nodeId));
    }


void LoadB0B1FromB0 (MTGNodeId nodeId)
    {
    m_B0 = NodeData (m_facets, nodeId);
    m_B1 = NodeData (m_facets, jmdlMTGGraph_getFSucc (m_graph, nodeId));
    }

void AdvanceB0B1 ()
    {
    m_B0 = m_B1;
    m_B1 = NodeData (m_facets, jmdlMTGGraph_getFSucc (m_graph, m_B0.m_nodeId));
    }
bool IsA0A1ZeroLengthXY () {return m_A0.m_xyz.DistanceXY (m_A1.m_xyz) <= abstol;}
bool IsB0B1ZeroLengthXY () {return m_B0.m_xyz.DistanceXY (m_B1.m_xyz) <= abstol;}

// Get plane normal using vectors A0A1 and B0B1
bool GetA01B01PlaneNormal (DVec3d &unitNormal, DVec3dR vectorA0A1, DVec3dR vectorB0B1)
    {
    vectorA0A1.DifferenceOf (m_A1.m_xyz, m_A0.m_xyz);
    vectorB0B1.DifferenceOf (m_B1.m_xyz, m_B0.m_xyz);
    double d;
    if (vectorA0A1.IsParallelTo (vectorB0B1))
        {
        if (!m_A0.IsSameNode (m_B0))
            {
            DVec3d edge;
            edge.DifferenceOf (m_B0.m_xyz, m_A0.m_xyz);
            d = unitNormal.NormalizedCrossProduct (vectorA0A1, edge);
            return !vectorA0A1.IsParallelTo (edge);
            }
        }
    d = unitNormal.NormalizedCrossProduct (vectorA0A1, vectorB0B1);
    return !vectorA0A1.IsParallelTo (vectorB0B1);
    }

void PrintPlaneData(char const * name)
    {
    DVec3d normal, vectorA, vectorB;
    GetA01B01PlaneNormal (normal, vectorA, vectorB);
    DEBUG_CODE(printf ("  *** cusp candidate %s\n");)
    DEBUG_CODE(printf ("  (A0A1 %d %d) (B0B1 %d %d) (A0 %g,%g,%g) (A0A1 %g,%g,%g)  (B0B1 %g,%g,%g) (normal %g,%g,%g)\n",
            m_A0.m_nodeId, m_A1.m_nodeId, m_B0.m_nodeId, m_B1.m_nodeId,
            m_A0.m_xyz.x, m_A0.m_xyz.y, m_A0.m_xyz.z,
            vectorA.x, vectorA.y, vectorA.z,
            vectorA.x, vectorA.y, vectorA.z,
            normal.x, normal.y, normal.z
            );)
    }

bool IsVerticalPanelVectorPair (DVec3d &unitNormal)
    {
    DVec3d vectorA, vectorB;
    if (!GetA01B01PlaneNormal (unitNormal, vectorA, vectorB))
        return false;
    if (IsXYUnitVector (unitNormal))
        {
        DVec3d xyPlaneVector;
        xyPlaneVector.Init (-unitNormal.y, unitNormal.x, 0.0);
        double dotA = vectorA.DotProduct (xyPlaneVector);
        double dotB = vectorB.DotProduct (xyPlaneVector);
        return dotA * dotB > 0.0;   // A and B depart in same diretction
        }
    return false;
    }


// Search forward from startNodeId to see if it can be m_A0.m_nodeId ...
// Return true with m_A0.m_nodeId etc set up with coordinates.
// On false return all nodes and coordinates are unspecified.
// To confirm ...
//   A0, A1 distant in XY
//   chain A0...B0 all have zero distanceXY 
//   B0, B1 distant
// A0A1 and B0B1 in opposing directions.

bool LoadVerticalPanelStartConfiguration (MTGNodeId startNodeId, DVec3d &unitNormal)
    {
    LoadA0A1FromA1 (startNodeId);
    if (IsA0A1ZeroLengthXY ())
        return false;
    LoadB0B1FromB0 (m_A0.m_nodeId);
    while (IsB0B1ZeroLengthXY ())
        AdvanceB0B1 ();
    return IsVerticalPanelVectorPair (unitNormal);
    }

// Load with A0==B0 -- i.e. A0A1, B0B1 are edges of a triangle at base point A0==B0
bool LoadTriangleFromSharedEdgePoint (MTGNodeId startNodeId, DVec3d &unitNormal)
    {
    LoadA0A1FromA0 (startNodeId);
    LoadB0B1FromB0 (startNodeId);
    DVec3d vectorA, vectorB;
    return GetA01B01PlaneNormal (unitNormal, vectorA, vectorB);
    }


// Search around a face for a vertical panel start configuration
bool FindVerticalPanelStartConfigurationAroundFace (MTGNodeId faceSeedNodeId)
    {
    DVec3d unitNormal;
    MTGARRAY_FACE_LOOP (currNodeId, m_graph, faceSeedNodeId)
        {
        if (LoadVerticalPanelStartConfiguration (currNodeId, unitNormal))
            return true;
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, m_graph, faceSeedNodeId)
    return false;
    }

void LoadCoplanarNodes (NodeData startNode, DVec3d const &normal, bool forward, bool requireConvex, bvector <NodeData> &nodeList, bool &returnedToStart)
    {
    nodeList.clear ();
    NodeData dataP = startNode;
    NodeData dataQ;
    DVec3d edgeVector1;
    DVec3d edgeVector0, cross01;
    edgeVector0.Zero ();
    returnedToStart = false;
    DEBUG_CODE(printf ("LoadCoplanar start at %d (%g,%g,%g) (normal %g,%g,%g)\n", dataP.m_nodeId, dataP.m_xyz.x, dataP.m_xyz.y, dataP.m_xyz.z, normal.x, normal.y, normal.z);)
    for (;; dataP = dataQ, edgeVector0 = edgeVector1)
        {
        dataQ = LoadAdjacentNodeData (dataP.m_nodeId, forward);
        if (dataQ.IsSameNode (startNode))
            {
            returnedToStart = true;
            break;
            }
        edgeVector1.DifferenceOf (dataQ.m_xyz, dataP.m_xyz);
        DEBUG_CODE(printf ("    (list size %d) candidate %s node %d (xyz %g,%g,%g) (vector %g,%g,%g)\n",
                  nodeList.size (), forward ? "succ" : "pred", dataQ.m_nodeId, dataQ.m_xyz.x, dataQ.m_xyz.y,  dataQ.m_xyz.z, edgeVector1.x, edgeVector1.y, edgeVector1.z);)
        if (!edgeVector1.IsPerpendicularTo (normal))
            break;
        if (requireConvex && nodeList.size () > 0)
            {
            cross01.CrossProduct (edgeVector1, edgeVector0);  // 1 cross 0 ?? confused..
            double d = cross01.DotProduct (normal);
            if (!forward)
                d = -d;
            DEBUG_CODE(printf ("    convex test dot %g\n", d);)
            if (d < 0.0 && !edgeVector0.IsParallelTo (edgeVector1))
                break;
            }
        nodeList.push_back (dataQ);
        }
    }
};
static int s_numCandidate = 0;

static bool FaceCountainsNode (MTGGraph *graph, MTGNodeId start, MTGNodeId target)
    {
    MTGARRAY_FACE_LOOP (node, graph, start)
        {
        if (node == target)
            return true;
        }
    MTGARRAY_END_FACE_LOOP (node, graph, start)
    return false;
    }
static size_t AddVerticalPlanarPanels (MTGFacets *facets, MTGNodeId faceSeedNodeId, MTGMask visitMask)
    {
    bvector <NodeData> nodeListA, nodeListB;
    EdgePairNodes cuspA (facets), cuspB (facets), cuspC (facets);
    EdgePairNodes baseTriangle (facets);
    MTGGraph * graph = jmdlMTGFacets_getGraph (facets);
    size_t numAdded = 0;
DEBUG_CODE(
    MTGARRAY_FACE_LOOP (node, graph, faceSeedNodeId)
        {
        NodeData data (facets, node);
        printf ("   node %d  (%16.2f,%16.2f, %16.2f)\n", data.m_nodeId, data.m_xyz.x, data.m_xyz.y, data.m_xyz.z);
        }
    MTGARRAY_END_FACE_LOOP (node, graph, faceSeedNodeId)
)
    if (cuspA.FindVerticalPanelStartConfigurationAroundFace (faceSeedNodeId)
      && cuspB.FindVerticalPanelStartConfigurationAroundFace (cuspA.NodeIdB0 ())
      && !cuspA.SameStart (cuspB)
      && cuspC.FindVerticalPanelStartConfigurationAroundFace (cuspB.NodeIdB0 ())
      && cuspC.SameStart (cuspA)
      )
      {
      cuspA.PrintPlaneData ("cusp A");
      cuspB.PrintPlaneData ("cusp B");
      cuspC.PrintPlaneData ("cusp C");
      // hello world !!!
      s_numCandidate++;
      NodeData baseData = cuspA.LoadNodeData (cuspA.NodeIdA0 ());
      DVec3d unitNormal;
      bool returnedToStart;
      for (;;)
          {
          if (!baseTriangle.LoadTriangleFromSharedEdgePoint (baseData.m_nodeId, unitNormal))
              return numAdded;
          if (!IsXYUnitVector (unitNormal))
              {
              EdgePairNodes restartPosition (facets);
              if (!restartPosition.FindVerticalPanelStartConfigurationAroundFace (baseData.m_nodeId))
                  {
                  // bad dog...
                  return numAdded;
                  }
              baseData = cuspA.LoadNodeData (restartPosition.NodeIdA0 ());
              if (!baseTriangle.LoadTriangleFromSharedEdgePoint (baseData.m_nodeId, unitNormal))
                  return numAdded;
              }
          NodeData A0 = baseTriangle.NodeDataA0 ();
          baseTriangle.LoadCoplanarNodes (A0, unitNormal, true, true, nodeListB, returnedToStart);
          if (nodeListB.size () == 0)
              return numAdded;

          if (returnedToStart)
              {
              jmdlMTGGraph_clearMaskAroundFace (graph, A0.m_nodeId, MTG_EXTERIOR_MASK);
              jmdlMTGGraph_clearMaskAroundFace (graph, A0.m_nodeId, MTG_DIRECTED_EDGE_MASK);
              numAdded++;
              return numAdded;      // the crowd cheers wildly
              }
          else
              {
              baseTriangle.LoadCoplanarNodes (A0, unitNormal, false, true, nodeListA, returnedToStart);
              MTGNodeId newNodeIdBtoA, newNodeIdAtoB;
              MTGNodeId startNodeId = baseTriangle.NodeIdA0 ();
              MTGNodeId nodeIdA = nodeListA.back ().m_nodeId;
              MTGNodeId nodeIdB = nodeListB.back ().m_nodeId;
              int vertexIndexA, vertexIndexB;
              jmdlMTGFacets_getNodeVertexIndex (facets, &vertexIndexA, nodeIdA);
              jmdlMTGFacets_getNodeVertexIndex (facets, &vertexIndexB, nodeIdB);
              DEBUG_CODE(printf ("   JOIN %d %d\n", nodeIdA, nodeIdB);)
              jmdlMTGFacets_addIndexedEdge (facets,
                                &newNodeIdAtoB, &newNodeIdBtoA,
                                nodeIdA, nodeIdB,
                                visitMask, visitMask,
                                vertexIndexA, vertexIndexB,
                                0,0
                                );
              DEBUG_CODE(printf ("   New Edge (nodes %d %d) (vertexIndices %d %d)\n", newNodeIdAtoB, newNodeIdBtoA, vertexIndexA, vertexIndexB);)
              jmdlMTGGraph_clearMaskAroundFace (graph, startNodeId, MTG_EXTERIOR_MASK);
              jmdlMTGGraph_setMaskAroundFace (graph, startNodeId, MTG_DIRECTED_EDGE_MASK);
              jmdlMTGGraph_setMaskAroundFace (graph, startNodeId, visitMask);
              // hmph.. I'm not sure which side is the face just split off.  So look for the start node.
              if (FaceCountainsNode (graph, newNodeIdAtoB, startNodeId))
                  baseData = cuspA.LoadNodeData (newNodeIdBtoA);
              else
                  baseData = cuspA.LoadNodeData (newNodeIdAtoB);
              numAdded++;
              }
          }
      }
    return numAdded;
    }

struct MTGHealingContext
{
public:
// return the number of triangles added.
static size_t FillHolesByLocalRetriangulation (MTGFacets* facets, bool purgeDoubleExteriors = true)
    {
    size_t numAdded = 0;
    // delete double exterior
    if (purgeDoubleExteriors)
        jmdlMTGFacets_dropDoubleMaskedEdges (facets, MTG_EXTERIOR_MASK);
    // Find and close exterior faces ...
    MTGGraphP graph = jmdlMTGFacets_getGraph (facets);
    EdgePairNodes edgePair (facets);
    MTGMask visitMask = jmdlMTGGraph_grabMask (graph);
    jmdlMTGGraph_clearMaskInSet (graph, visitMask);

    MTGARRAY_SET_LOOP (seedNodeId, graph)
        {
        if (jmdlMTGGraph_getMask (graph, seedNodeId, MTG_EXTERIOR_MASK)
            && ! jmdlMTGGraph_getMask (graph, seedNodeId, visitMask))
            {
            jmdlMTGGraph_setMaskAroundFace (graph, seedNodeId, visitMask);
            if ((int)graph->CountNodesAroundFace (seedNodeId) < 4)
                {
                jmdlMTGGraph_clearMaskAroundFace (graph, seedNodeId, MTG_EXTERIOR_MASK);
                jmdlMTGGraph_setMaskAroundFace   (graph, seedNodeId, MTG_DIRECTED_EDGE_MASK);
                numAdded++;
                }
            else
                numAdded += AddVerticalPlanarPanels (facets, seedNodeId, visitMask);
            }
        }
    MTGARRAY_END_SET_LOOP (seedNodeId, graph)


    jmdlMTGGraph_dropMask (graph, visitMask);
    return numAdded;
    }

static void AccessSector (MTGFacets *facets, MTGGraph *graph, MTGNodeId seedNodeId,
MTGNodeId &leftNodeId,
MTGNodeId &rightNodeId,
DPoint3dR  xyzSeed,
DPoint3dR  xyzLeft,
DPoint3dR  xyzRight,
DVec3dR    leftVector,
DVec3dR    rightVector
)
    {
    leftNodeId = jmdlMTGGraph_getFPred (graph, seedNodeId);
    rightNodeId = jmdlMTGGraph_getFSucc (graph, seedNodeId);
    jmdlMTGFacets_getNodeCoordinates (facets, &xyzSeed, seedNodeId);
    jmdlMTGFacets_getNodeCoordinates (facets, &xyzLeft, leftNodeId);
    jmdlMTGFacets_getNodeCoordinates (facets, &xyzRight, rightNodeId);
    leftVector.DifferenceOf (xyzLeft, xyzSeed);
    rightVector.DifferenceOf (xyzRight, xyzSeed);
    }    

static void EvaluatePinchedSectorCandidate (MTGFacets *facets, MTGGraph *graph, MTGNodeId seedNodeId, double angleTol, bvector<MTGNodeId> &candidates)
    {
    if (jmdlMTGGraph_getMask (graph, seedNodeId, MTG_EXTERIOR_MASK))
        {
        DPoint3d xyzLeft, xyzRight, xyzSeed;
        DVec3d leftVector, rightVector;
        MTGNodeId leftNodeId, rightNodeId;
        AccessSector (facets, graph, seedNodeId, leftNodeId, rightNodeId, xyzSeed, xyzLeft, xyzRight, leftVector, rightVector);
        if (leftVector.DotProduct (rightVector) > 0.0)
            {
            double radians = leftVector.AngleTo(rightVector);
            if (radians < angleTol)
                candidates.push_back (seedNodeId);
            }
        }
    }

static void SetLabelAroundVertex (MTGGraph *graph, MTGNodeId seed, int labelIndex, int value)
    {
    MTGARRAY_VERTEX_LOOP (curr, graph, seed)
        {
        jmdlMTGGraph_setLabel (graph, curr, labelIndex, value);
        }
    MTGARRAY_END_VERTEX_LOOP (curr, graph, seed)
    }

// join leftNodeId to rightNodeId.
// Copy vertex data in indicated direction.
static void Pinch (MTGFacets *facets, MTGGraph *graph, MTGNodeId leftNodeId, MTGNodeId rightNodeId, int dataMoveDirection)
    {
    int vertexIndex;
    int vertexLabel = facets->vertexLabelOffset;

    // hm.. should vertexIndex come from an interior node?
    if (dataMoveDirection < 0)    // move data from right to left
        {
        jmdlMTGFacets_getNodeVertexIndex (facets, &vertexIndex, rightNodeId);
        SetLabelAroundVertex (graph, leftNodeId, vertexLabel, vertexIndex);
        }
    else if (dataMoveDirection > 0) // move data from left to right
        {
        jmdlMTGFacets_getNodeVertexIndex (facets, &vertexIndex, leftNodeId);
        SetLabelAroundVertex (graph, rightNodeId, vertexLabel, vertexIndex);
        }
    jmdlMTGGraph_vertexTwist (graph, leftNodeId, rightNodeId);
    }
public:
// Look for exterior vertices with (forward) parallel outbound directions.
// Add and join successor vertices (creating degenerate 2-edge faces)
//
// (Why leave them as is?  The transfer to polyface will not make the exterior polygons, so it has the effect of merging it all together without requiring
//   tricky logic to delete and merge edges)
static size_t SimplifySlivers (MTGFacets *facets, double distanceTol, double angleTol = 1.0e-9)
    {
    MTGGraphP graph = jmdlMTGFacets_getGraph (facets);

    bvector<MTGNodeId> candidates;
    size_t numChanged = 0;
    if (s_noisy)
        jmdlMTGGraph_printFaceLoops (graph);

    MTGARRAY_SET_LOOP (seedNodeId, graph)
        {
        EvaluatePinchedSectorCandidate (facets, graph, seedNodeId, angleTol, candidates);
        }
    MTGARRAY_END_SET_LOOP (seedNodeId, graph)

    DPoint3d xyzLeft, xyzRight, xyzSeed;
    DVec3d leftVector, rightVector;
    MTGNodeId leftNodeId, rightNodeId;

    MTGNodeId nodeIdA, nodeIdB;
    for (;candidates.size () > 0;)
        {
        MTGNodeId seedNodeId = candidates.back ();
        candidates.pop_back ();
        AccessSector (facets, graph, seedNodeId, leftNodeId, rightNodeId, xyzSeed, xyzLeft, xyzRight, leftVector, rightVector);
        if (leftNodeId != rightNodeId)
            {
            double leftDistance = leftVector.Magnitude ();
            double rightDistance = rightVector.Magnitude ();
            if (fabs (leftDistance - rightDistance) <= distanceTol)
                {
                // coincident vertices at far end ..
                // umm...reassign vertex index?
                Pinch (facets, graph, leftNodeId, rightNodeId, -1);
                EvaluatePinchedSectorCandidate (facets, graph, rightNodeId, angleTol, candidates);
                }
            else if (leftDistance < rightDistance)  // SPLIT RIGHT
                {
                jmdlMTGGraph_splitEdge (graph, &nodeIdA, &nodeIdB, seedNodeId);
                Pinch (facets, graph, leftNodeId, nodeIdA, 1);
                EvaluatePinchedSectorCandidate (facets, graph, nodeIdA, angleTol, candidates);
                }
            else // right distance < leftDistance.  SPLIT LEFT
                {
                jmdlMTGGraph_splitEdge (graph, &nodeIdA, &nodeIdB, leftNodeId);
                Pinch (facets, graph, nodeIdA, rightNodeId, 1);
                EvaluatePinchedSectorCandidate (facets, graph, rightNodeId, angleTol, candidates);
                }
            numChanged++;
            }
        }
    return numChanged;
    }

public:
// return the number of triangles added.
static size_t AddVerticalExteriorEdges (MTGFacets* facets)
    {
    size_t numAdded = 0;
    // delete double exterior
    jmdlMTGFacets_dropDoubleMaskedEdges (facets, MTG_EXTERIOR_MASK);
    // Find and close exterior faces ...
    MTGGraphP graph = jmdlMTGFacets_getGraph (facets);

    bvector<DPoint3d> candidateXYZ;
    bvector<MTGNodeId> candidateNodeId;
    bvector<int> clusterIndex;
    static double abstol = 1.0e-8;
    MTGARRAY_SET_LOOP (seedNodeId, graph)
        {
        if (jmdlMTGGraph_getMask (graph, seedNodeId, MTG_EXTERIOR_MASK))
            {
            int numOn, numOff;
            jmdlMTGGraph_countMasksAroundVertex (graph, &numOn, &numOff, seedNodeId, MTG_EXTERIOR_MASK);
            if (numOn == 1 && numOff >= 1)
                {
                DPoint3d xyz;
                jmdlMTGFacets_getNodeCoordinates (facets, &xyz, seedNodeId);
                xyz.z = 0.0;
                candidateXYZ.push_back (xyz);
                candidateNodeId.push_back (seedNodeId);
                }
            }
        }
    MTGARRAY_END_SET_LOOP (seedNodeId, graph)

    bsiDPoint3dArray_findClusters (candidateXYZ, clusterIndex, abstol, false, false);

    for (size_t i0 = 0, i1 = 0; i1 < clusterIndex.size (); i1++)
        {
        if (clusterIndex[i1] == -1)
            {
            // i0 .. i1-1 are indices of a point cluster ...
            size_t clusterSize = i1 - i0;
            if (clusterSize == 2)
                {
                MTGNodeId nodeId0, nodeId1;
                nodeId0 = candidateNodeId[clusterIndex[i0]];
                nodeId1 = candidateNodeId[clusterIndex[i0 + 1]];
                MTGNodeId newNodeId0, newNodeId1;
                int vertexIndex0, vertexIndex1;
                jmdlMTGFacets_getNodeVertexIndex (facets, &vertexIndex0, nodeId0);
                jmdlMTGFacets_getNodeVertexIndex (facets, &vertexIndex1, nodeId1);
                jmdlMTGFacets_addIndexedEdge (facets,
                                &newNodeId0, &newNodeId1,
                                nodeId0, nodeId1,
                                MTG_EXTERIOR_MASK, MTG_EXTERIOR_MASK,
                                vertexIndex0, vertexIndex1,
                                0,0
                                );
                }
            i0 = i1 + 1;
            }
        }


    return numAdded;
    }




public:
// return the number of triangles added.
static size_t FillHolesBySpaceTriangulation (MTGFacets* facets)
    {
    size_t numAdded = 0;
    EmbeddedDPoint3dArrayP pFringeXYZArray = jmdlEmbeddedDPoint3dArray_grab ();
    bvector<MTGNodeId> fringeVertexId;
    bvector< bvector<int> > newTriangleVertexId;   // save up triples during scan, add to mtg at end.
    EmbeddedIntArray *fringeIndexArray = jmdlEmbeddedIntArray_grab ();
    // delete double exterior
    jmdlMTGFacets_dropDoubleMaskedEdges (facets, MTG_EXTERIOR_MASK);
    // Find and close exterior faces ...
    MTGGraphP graph = jmdlMTGFacets_getGraph (facets);
    MTGMask visitMask = jmdlMTGGraph_grabMask (graph);
    jmdlMTGGraph_clearMaskInSet (graph, visitMask);
    int numFailures = 0;

    MTGARRAY_SET_LOOP (seedNodeId, graph)
        {
        if (jmdlMTGGraph_getMask (graph, seedNodeId, MTG_EXTERIOR_MASK)
            && ! jmdlMTGGraph_getMask (graph, seedNodeId, visitMask))
            {
            jmdlMTGGraph_setMaskAroundFace (graph, seedNodeId, visitMask);
            
            jmdlEmbeddedDPoint3dArray_empty (pFringeXYZArray);
            fringeVertexId.clear ();
            MTGARRAY_FACE_LOOP (currNodeId, graph, seedNodeId)
                {
                DPoint3d xyz;
                jmdlMTGFacets_getNodeCoordinates (facets, &xyz, currNodeId);
                jmdlEmbeddedDPoint3dArray_addDPoint3d (pFringeXYZArray, &xyz);
                int vertexIndex;    // In the facets.
                jmdlMTGFacets_getNodeVertexIndex (facets, &vertexIndex, currNodeId);
                fringeVertexId.push_back (vertexIndex);
                }
            MTGARRAY_END_FACE_LOOP (currNodeId, graph, seedNodeId)

            int nA = 0;
            int nB = 0;
            int nOther = 0;
            int fringeSize = jmdlEmbeddedDPoint3dArray_getCount (pFringeXYZArray);
            DPoint3dP pFringeXYZBuffer = jmdlEmbeddedDPoint3dArray_getPtr (pFringeXYZArray, 0);
            // TRUE ==> signed, one based indices that distinguish direction clearly.
            if (SUCCESS != vu_triangulateSpacePolygon (fringeIndexArray, pFringeXYZBuffer, fringeSize, 0.0, 3, TRUE))
                {
                numFailures++;
                }
            else
                {
// Force all edges visible -- have to confirm planarity before allowing hidden edges....
                int numTriangleIndices = jmdlEmbeddedIntArray_getCount (fringeIndexArray);
                int *pFringeIndex = jmdlEmbeddedIntArray_getPtr (fringeIndexArray, 0);                

                int k;
                DEBUG_CODE(printf ("Triangulation %d indices\n  ", numTriangleIndices);)
                int fringeIndex;
                int k0 = 0;

                for (k = 0; k < numTriangleIndices; k++)
                    {
                    fringeIndex = pFringeIndex[k];
                    DEBUG_CODE(printf (" %3d", fringeIndex);)
                    if (fringeIndex == 0)
                        {
                        int numExtA, numExtB, numInt, numOther;
                        int orientation = IndexOrderState (fringeIndexArray, k0, k - k0, fringeSize, numExtA, numExtB, numInt, numOther);
                        DEBUG_CODE(printf ("(orientation %d)\n  ", orientation);)
                        if (orientation == 1)
                            nA++;
                        else if (orientation == -1)
                            nB++;
                        else
                            nOther++;
                        k0 = k + 1;
                        }
                    }
                DEBUG_CODE(printf ("\n (Face orientations: (POS %d) (NEG %d) (OTHER %d))\n", nA, nB, nOther);)

                bool reverseAll = (nB > 0 && nA == 0);
                k0 = 0;
                for (k = 0; k < numTriangleIndices; k++)
                    {
                    fringeIndex = pFringeIndex[k];
                    if (fringeIndex == 0)
                        {
                        if (k - k0 == 3)
                            {
                            newTriangleVertexId.push_back (bvector<int> ());
                            bvector<int> &newTriangle = newTriangleVertexId.back ();
                            // indices k0 < i < k are a triangle.
                            if (reverseAll)
                                {
                                for (int i = k - 1; i >= k0; i--)
                                    {
                                    int j = abs (pFringeIndex[i]) - 1;
                                    int vertexIndex = fringeVertexId[j];
                                    newTriangle.push_back (vertexIndex);
                                    }
                                }
                            else
                                {
                                for (int i = k0; i < k; i++)
                                    {
                                    int j = abs (pFringeIndex[i]) - 1;
                                    int vertexIndex = fringeVertexId[j];
                                    newTriangle.push_back (vertexIndex);
                                    }
                                }
                            }
                        k0 = k + 1;
                        }
                    }
                }
            }
        }
    MTGARRAY_END_SET_LOOP (seedNodeId, graph)

    // triangleVertexId contains index triples for (original) vertices of all triangles
    // needed to fill all the voids (except those where triangulation failed -- too much nonplanarity?),
    // oriented consistently with the fringes of the holes they fill.
    bvector<int> newTriangle;
    for (size_t i = 0; i < newTriangleVertexId.size (); i++)
        {
        newTriangle = newTriangleVertexId[i];
        //Frank: reverse the normal of the triangle
        size_t size = newTriangle.size();
        for(size_t i = 0; i < size/2; i++)
            {
            int index = newTriangle[i];
            newTriangle[i] = newTriangle[size-1-i];
            newTriangle[size-1-i] = index;
            }
        // Just jamb it in (without reusing the exterior edges).
        jmdlMTGFacets_addIndexedFace (facets, &newTriangle[0], NULL, (int)newTriangle.size (),
                        0, NULL, 0);
        numAdded++;
        }

    jmdlMTGGraph_dropMask (graph, visitMask);
    jmdlEmbeddedIntArray_drop (fringeIndexArray);
    jmdlEmbeddedDPoint3dArray_drop (pFringeXYZArray);
    return numAdded;
    }
};

static double s_absTol = 1.0e-8;
size_t PolyfaceQuery::HealVerticalPanels (PolyfaceQueryCR polyface, bool tryVerticalPanels, bool trySpaceTriangulation, PolyfaceHeaderPtr &healedPolyface, bool simplifySlivers)
    {
    healedPolyface = NULL;
    MTGFacets* facets = jmdlMTGFacets_grab ();
    bvector<MTGNodeId> meshVertexToNodeId;
    bvector<size_t>nodeIdToReadIndex;
    size_t facetsAdded = 0;

    if (PolyfaceToMTG (facets, &meshVertexToNodeId, &nodeIdToReadIndex, polyface, true, Angle::SmallAngle (), Angle::SmallAngle ()))
        {
        if (simplifySlivers)
            {
            DRange3d range;
            static double s_squeezeRelTol = 1.0e-7;
            jmdlMTGFacets_getRange (facets, &range);
            double squeezeTol = s_absTol + s_squeezeRelTol * range.low.Distance (range.high);
            facetsAdded += MTGHealingContext::SimplifySlivers (facets, squeezeTol);
            }

        if (tryVerticalPanels)
            {
            facetsAdded += MTGHealingContext::FillHolesByLocalRetriangulation (facets);
            MTGHealingContext::AddVerticalExteriorEdges (facets);
            facetsAdded += MTGHealingContext::FillHolesByLocalRetriangulation (facets, false);
            static int s_finalFill = 1;
            if (s_finalFill)
                facetsAdded += MTGHealingContext::FillHolesByLocalRetriangulation (facets, false);
            }
        if (trySpaceTriangulation)
            facetsAdded += MTGHealingContext::FillHolesBySpaceTriangulation (facets);
        }

    if (facetsAdded > 0)
        {
        healedPolyface = PolyfaceHeader::CreateVariableSizeIndexed ();
        AddMTGFacetsToIndexedPolyface (facets, *healedPolyface, MTG_EXTERIOR_MASK, MTG_DIRECTED_EDGE_MASK);
        }

    jmdlMTGFacets_drop (facets);
    return facetsAdded;
    }



struct SegmentMatchContext
{
VuSetP m_graph;
MTGMask m_forwardMask;
MTGMask m_primaryEdgeMask;
double m_tolerance;
SegmentMatchContext (double tolerance)
    {
    m_graph = vu_newVuSet (0);
    m_forwardMask = VU_RULE_EDGE;
    m_primaryEdgeMask = VU_SEAM_EDGE;
    m_tolerance = tolerance;
    vu_setUserDataPIsEdgeProperty (m_graph, true);
    }
~SegmentMatchContext () 
    {
    vu_freeVuSet (m_graph);
    }
VuP AddSegment (DSegment3dCR segment, int index)
    {
    VuP nodeA, nodeB;
    vu_makePair (m_graph, &nodeA, &nodeB);
    nodeA->SetMask (m_forwardMask);
    nodeA->SetMask (m_forwardMask);
    nodeB->SetMask (m_primaryEdgeMask);
    nodeA->SetXYZ (segment.point[0]);
    nodeB->SetXYZ (segment.point[1]);
    vu_setUserDataPAsInt (nodeA, index);
    vu_setUserDataPAsInt (nodeB, index);
    return nodeA;
    }
bool SameXY (DPoint3d xyzA, DPoint3d xyzB)
    {
    return (xyzA.DistanceXY (xyzB) <= m_tolerance);
    }
void Merge ()
    {
    vu_mergeOrUnionLoops (m_graph, VUUNION_UNION);
    }
// Reset z coordinates to original values (merge smashes them to a single value around a vertex.)
// When vertices are inserted within an edge, record it as a curve location detail in addedPoints, containing:
// <ul>
// <li> fraction -- fractional position along a segemnt.
// <li> point -- interpolated point
// <li> componentIndex = segment index.
// <li> all other fields (curve pointer, numComonent, componentFraction, a) zeros.
// </ul>
void CorrectZCoordinates (bvector<FacetEdgeDetail> const &segments, bvector<CurveLocationDetail> &addedPoints)
    {
    // OUCH !!! merge may have smashed Z coordinates !!!
    VU_SET_LOOP (node, m_graph)
        {
        ptrdiff_t index = (ptrdiff_t)vu_getUserDataPAsInt (node);
        if (index >= 0 && index < (ptrdiff_t)segments.size ())
            {
            DSegment3d segment = segments[index].segment;
            DPoint3d xyz;
            vu_getDPoint3d (&xyz, node);
            if (SameXY (xyz, segment.point[0]))
                xyz.z = segment.point[0].z;
            else if (SameXY (xyz, segment.point[1]))
                xyz.z = segment.point[1].z;
            else
                {
                // This node is "along" an original edge but at a (hopefully) internior point.
                DPoint3d xyz1;
                double fraction1;
                segment.ProjectPointXY (xyz1, fraction1, xyz);
                xyz.z = xyz1.z;     // but leave the x and y parts alone to honor the merge tolerance decisions 
                addedPoints.push_back (
                    CurveLocationDetail (nullptr, fraction1, xyz, (size_t)index, 0, 0.0, 0.0));
                }
            node->SetXYZ (xyz);
            }
        }
    END_VU_SET_LOOP (node, m_graph)
    }
void CreateVerticalPolygons (bvector<bvector<DPoint3d>> &outputPolygons)
    {
    outputPolygons.clear ();
    bvector<DPoint3d> facetXYZ;
    // OUCH !!! merge may have smashed Z coordinates !!!
    VU_SET_LOOP (nodeA, m_graph)
        {
        auto nodeB = nodeA->FSucc ();
        auto nodeC = nodeB->FSucc ();
        // Are we inside a sling, and at the lower-id node?
        // nodeA and nodeB reference two original segments that had different Z.
        // the xyz coordinates of the original A segment are in nodeA and its mate.
        // the xyz coordinates of the original B segment are in nodeB and its mate.
        if (nodeA == nodeC && nodeA->GetId () < nodeB->GetId ())
            {
            auto mateA = nodeA->EdgeMate ();
            auto mateB = nodeB->EdgeMate ();
            facetXYZ.clear ();
            facetXYZ.push_back (nodeA->GetXYZ ());
            facetXYZ.push_back (mateA->GetXYZ ());
            facetXYZ.push_back (nodeB->GetXYZ ());
            facetXYZ.push_back (mateB->GetXYZ ());
            if (!nodeA->HasMask (m_primaryEdgeMask))
                std::reverse (facetXYZ.begin (), facetXYZ.end ());
            DPoint3dOps::CompressCyclic (facetXYZ, m_tolerance);
            if (facetXYZ.size () > 2)
                outputPolygons.push_back (facetXYZ);
            }
        }
    END_VU_SET_LOOP (nodeA, m_graph)
    }
};

//! Input an array of meshes expected to have boundary segments are separated by "missing side panels" as viewed in a certain direction.
//! return a (separate, new) mesh with the side panels.
PolyfaceHeaderPtr PolyfaceHeader::CreateSidePanelsForViewDirection
(
bvector<PolyfaceHeaderPtr> const &meshes,       //!< [in] array of input meshes
DVec3dCR viewDirection                     //! view direction, e.g. (0,0,1) for usual "missing sides in xy view"
)
    {
    bvector<FacetEdgeDetail> segmentsA;
    bvector<FacetEdgeDetail> allSegments;
    for (auto &meshA : meshes)
        {
        if (meshA.IsValid())
            {
            meshA->CollectEdgeMateData (segmentsA, false, false);
            FacetEdgeDetail::Append (allSegments, segmentsA);
            }
        }
    if (allSegments.size () == 0)
        return nullptr;

    BentleyApi::Transform localFrame = Transform::From (RotMatrix::From1Vector (viewDirection, 2, true), FacetEdgeDetail::GetAnyPoint (allSegments));
    BentleyApi::Transform worldToLocal;
    worldToLocal.InverseOf (localFrame);
    FacetEdgeDetail::TransformInPlace (allSegments, worldToLocal);
    auto result = PolyfaceHeader::CreateVerticalPanelsBetweenSegments (allSegments);
    if (result.IsValid ())
        result->Transform (localFrame);
    return result;
    }

//! input: array of line segments, e.g. boundary edges of a mesh with vertical facets missing.
//! output: mesh with vertical panels to fill between segments that match  in z direction.
PolyfaceHeaderPtr PolyfaceHeader::CreateVerticalPanelsBetweenSegments (bvector<FacetEdgeDetail> const &segments)
    {
    DRange3d range = DRange3d::NullRange ();
    for (auto &s: segments)
        {
        range.Extend (s.segment.point[0]);
        range.Extend (s.segment.point[1]);
        }
    double tightMatchedPointTolerance = 1.0e-14 * range.LargestCoordinateXY ();
    SegmentMatchContext context (tightMatchedPointTolerance);
    for (size_t i = 0; i < segments.size (); i++)
        {
        context.AddSegment (segments[i].segment, (int)i);
        }
    context.Merge ();
    bvector<CurveLocationDetail> addedPoints;
    context.CorrectZCoordinates (segments, addedPoints);
    bvector<bvector<DPoint3d>> polygons;
    auto polyface = PolyfaceHeader::CreateVariableSizeIndexed ();
    context.CreateVerticalPolygons (polygons);
    for (auto &polygon : polygons)
        polyface->AddPolygon (polygon);
    return polyface;
    }

/*-----------------------------------------------------------------*//**
* @bsimethod                                                       EarlinLutz      07/18
+---------------+---------------+---------------+---------------+------*/
FacetEdgeDetail::FacetEdgeDetail  (
    DSegment3dCR _segment,
    size_t _readIndex,
    uint32_t _clusterIndex,
    uint32_t _numInCluster
    ) :
    segment (_segment),
    readIndex (_readIndex),
    clusterIndex (_clusterIndex),
    numInCluster (_numInCluster)
    {

    }
//! Apply transform to the segment coordintes
void FacetEdgeDetail::TransformInPlace (TransformCR transform)
    {
    transform.Multiply (segment.point[0]);
    transform.Multiply (segment.point[1]);
    }
//! Apply transtform to all segment coordinates.
void FacetEdgeDetail::TransformInPlace (bvector<FacetEdgeDetail> &edges, TransformCR transform)
    {
    for (auto &edge : edges)
        edge.TransformInPlace (transform);
    }

//! Append (push_back) all from source to dest.
void FacetEdgeDetail::Append (bvector<FacetEdgeDetail> &dest, bvector<FacetEdgeDetail> const &source)
    {
    for (auto &edge : source)
        dest.push_back (edge);
    }

/*-----------------------------------------------------------------*//**
* @bsimethod                                                       EarlinLutz      07/18
+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr PolyfaceHeader::CloneWithTVertexFixup
(
bvector<PolyfaceHeaderPtr> const &meshes,      //!< [in] array of input meshes
IFacetOptions *options,                         //!< [in] facet options.
double onEdgeTolerance                         //!< [in] tolerance for identifying T vertex.  defaults to DoubleOps::SmallMetricDistance ()
)
    {
    if (onEdgeTolerance <= 0.0)
        onEdgeTolerance = DoubleOps::SmallMetricDistance ();
    IFacetOptionsPtr myOptions = options == nullptr ? IFacetOptions::Create () : options;
    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create (*myOptions);
    auto xySearcher = XYBucketSearch::Create ();
    // Collect all points into a single searcher.
    size_t totalPointCount = 0;
    for (auto &mesh : meshes)
        {
        if (mesh.IsValid ())
            {
            bvector<DPoint3d> &meshPoints = mesh->Point ();
            for (auto &xyz : meshPoints)
                xySearcher->AddPoint (xyz, totalPointCount);
            }
        }
    DRange3d range = xySearcher->GetRange ();
    static double s_relTol = 1.0e-11;
    onEdgeTolerance = DoubleOps::Max (onEdgeTolerance,
            range.LargestCoordinate () * s_relTol);
    // Look for point-on-edge incidence while creating new facets.
    bvector<DPoint3d> newFacetPoints;
    bvector<DPoint3d> searchPoint;
    bvector<size_t>   searchId;
    InsertPointsInPolylineContext polylineContext (onEdgeTolerance);

    for (auto &mesh : meshes)
        {
        if (mesh.IsValid ())
            {
            PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach (*mesh, true);
            visitor->SetNumWrap (1);
            bvector<DPoint3d> &facetPoints = visitor->Point ();
            DRange3d searchRange;
            for (visitor->Reset (); visitor->AdvanceToNextFace ();)
                {
                searchRange.InitFrom (facetPoints);
                searchRange.Extend (1.5 * onEdgeTolerance);
                xySearcher->CollectPointsInRangeXYZ (searchRange, searchPoint, searchId);
                if (searchPoint.size () > 0)
                    {
                    polylineContext.InsertPointsInPolyline (facetPoints, newFacetPoints, searchPoint);
                    builder->AddTriangulation (newFacetPoints);
                    }
                else
                    builder->AddTriangulation (facetPoints);
                }
            }
        }
    auto result = builder->GetClientMeshPtr ();
    result->Compress ();
    return result;
    }

//! Input an array of meshes expected to have boundary segments are separated by "missing side panels" as viewed in a certain direction.
//! return a (separate, new) mesh with the side panels added.   Additional midEdge vertices are inserted into the original facets if T vertices are present.
PolyfaceHeaderPtr PolyfaceHeader::CloneWithSidePanelsInserted
(
bvector<PolyfaceHeaderPtr> const &meshes,       //!< [in] array of input meshes
DVec3dCR viewDirection                     //! view direction, e.g. (0,0,1) for usual "missing sides in xy view"
)
    {
    auto sidePanels = PolyfaceHeader::CreateSidePanelsForViewDirection (meshes, viewDirection);
    bvector<PolyfaceHeaderPtr> allMeshes = meshes;
    allMeshes.push_back (sidePanels);
    return PolyfaceHeader::CloneWithTVertexFixup (allMeshes);
    }
END_BENTLEY_GEOMETRY_NAMESPACE
