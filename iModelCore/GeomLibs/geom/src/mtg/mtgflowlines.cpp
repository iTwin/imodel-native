/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/mtg/mtgflowlines.cpp $
|    $RCSfile: mtgflowlines.cpp,v $
|   $Revision: 1.1 $
|       $Date: 2010/09/03 15:43:44 $
|     $Author: Earlin.Lutz $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|									|
|   Include Files   							|
|									|
+----------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "mtgintrn.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
enum Topo {Topo_None, Topo_Vertex, Topo_Edge, Topo_Face};

//! Detail data for:
//! <ul>
//! <li>NodeId () -- a relevant node</li>
//! <li>XYZ () -- coordinate</li>
//! <li>Topo () -- indicats if the node is representing face, edge, vertex.</li>
//! <li>EdgeFraction () -- position along edge.
//! </ul>
//! Note that the XYZ () coordinate saved in the MTGPositionDetail
//! is not necessarily the same as the xyz of the node.  In fact, the
//!    MTGPositionDetail does not know its containing graph or facet set
//!    needed to find node coordinates.   MTGPositionDetail is just
//!    a data carrier.
//!
struct MTGPositionDetail
    {
    private:
    MTGNodeId m_nodeId;
    double    m_edgeFraction;
    DPoint3d  m_xyz;
    Topo      m_topology;
    //! Construct with node, coordinates, and topology indicator.
    MTGPositionDetail (MTGNodeId nodeId, DPoint3dCR xyz, Topo topo)
        {
        m_nodeId = nodeId;
        m_xyz    = xyz;
        m_edgeFraction = DBL_MAX;
        m_topology = topo;
        }

    //! Construct with edge-specific data.
    MTGPositionDetail (MTGNodeId nodeId, DPoint3dCR xyz, double edgeFraction)
        {
        m_nodeId = nodeId;
        m_xyz    = xyz;
        m_edgeFraction = edgeFraction;
        m_topology = Topo_Edge;
        }

    public:
    MTGPositionDetail ()
        {
        m_nodeId = MTG_NULL_NODEID;
        m_xyz.Zero ();
        m_edgeFraction = DBL_MAX;
        m_topology = Topo_None;
        }

    //! Construct and return as representative of face.
    static MTGPositionDetail FromFace (MTGNodeId nodeId, DPoint3dCR xyz)
        {return MTGPositionDetail (nodeId, xyz, Topo_Face);}

    //! Construct and return as representative of edge.
    static MTGPositionDetail FromEdge (MTGNodeId nodeId, DPoint3dCR xyz, double edgeFraction)
        {return MTGPositionDetail (nodeId, xyz, edgeFraction);}

    //! Construct and return as representative of vertex.
    static MTGPositionDetail FromVertex (MTGNodeId nodeId, DPoint3dCR xyz)
        {return MTGPositionDetail (nodeId, xyz, Topo_Vertex);}

    //! @return true if there is a non-null nodeid.
    bool HasNodeId () const { return m_nodeId != MTG_NULL_NODEID;}
    //! @return edge fraction.
    double EdgeFraction () const { return m_edgeFraction;}
    //! @return topology type
    Topo TopoType () const {return m_topology;}

    //! @return true if topology type is face
    bool IsFace () const {return m_topology == Topo_Face;}
    //! @return true if topology type is edge
    bool IsEdge () const {return m_topology == Topo_Edge;}
    //! @return true if topology type is vertex
    bool IsVertex () const {return m_topology == Topo_Vertex;}

    //! @return node id
    MTGNodeId NodeId () const { return m_nodeId;}
    //! @return coordinates
    DPoint3d XYZ () const { return m_xyz;}
    //! @return vector to coordinates in the other MTGPositionDetail
    DVec3d VectorTo (MTGPositionDetail const &other) const
        {
        return other.m_xyz - m_xyz;
        }

    //! @return MTGPositionDetail for this instance's edge mate;
    //!    The returned MTGPositionDetail's edgeFraction is {1 - this->EdgeFraction ())
    //!    to properly identify the "same" position relative to the other side.
    MTGPositionDetail EdgeMate (MTGGraph & graph) const
        {
        MTGPositionDetail result = *this;
        if (m_nodeId == MTG_NULL_NODEID)
            return result;
        result.m_nodeId = graph.EdgeMate (m_nodeId);
        if (m_edgeFraction != DBL_MAX)
            result.m_edgeFraction = 1.0 - m_edgeFraction;
        return result;
        }

    //! @return x coordinate
    double X () const { return m_xyz.x;}
    //! @return y coordinate
    double Y () const { return m_xyz.y;}
    //! @return z coordinate
    double Z () const { return m_xyz.z;}

    // If candidateKey is less than resultKey, replace resultPos and resultKey
    // by the candidate data.
    static bool UpdateMinimizer
            (
            MTGPositionDetail &resultPos, double &resultKey,
            MTGPositionDetail const &candidatePos, double candidateKey)
        {
        if (candidateKey < resultKey)
            {
            resultKey = candidateKey;
            resultPos = candidatePos;
            return true;
            }
        return false;
        }
    };

typedef MTGPositionDetail & MTGPositionDetailR;
typedef MTGPositionDetail const & MTGPositionDetailCR;
typedef MTGPositionDetail * MTGPositionDetailP;
typedef MTGPositionDetail const * MTGPositionDetailCP;


// Hold an MTGFacets for repeated flow line calculations.
struct FlowLinesContext
{
private:
// REFERENCE (not owned) to subject facets.
MTGFacets & m_facets;
// REFERENCE (not owned) to graph of subject facets
MTGGraph & m_graph;
public:

FlowLinesContext (MTGFacets & facets)
    : m_facets (facets), m_graph (facets.graphHdr)
    {
    }

~FlowLinesContext ()
    {
    }

private:
bool GetNormalAtNode (MTGNodeId nodeId, DVec3dR normal)
    {
    if (m_graph.GetMaskAt (nodeId, MTG_EXTERIOR_MASK))
        return false;
    MTGNodeId nodeIdA = m_graph.FSucc (nodeId);
    MTGNodeId nodeIdB = m_graph.FPred (nodeId);
    DPoint3d xyzOrigin, xyzA, xyzB;
    xyzOrigin = m_facets.GetXYZ (nodeId);
    xyzA = m_facets.GetXYZ(nodeIdA);
    xyzB = m_facets.GetXYZ(nodeIdB);
    normal.CrossProductToPoints (xyzOrigin, xyzA, xyzB);
    return true;
    }

bool MinZAroundFace (MTGNodeId faceNodeId, MTGPositionDetailR minZDetail)
    {
    minZDetail = MTGPositionDetail ();
    if (faceNodeId == MTG_NULL_NODEID)
        return false;

    double minZ = DBL_MAX;
    MTGARRAY_FACE_LOOP (currNodeId, &m_graph, faceNodeId)
        {
        DPoint3d xyz = m_facets.GetXYZ (currNodeId);
        if (xyz.z < minZ)
            minZDetail = MTGPositionDetail::FromVertex (currNodeId, xyz);
        }
    MTGARRAY_END_FACE_LOOP (currNodeId, &m_graph, faceNodeId)
    return minZDetail.HasNodeId ();
    }

MTGPositionDetail MinZAlongEdge (MTGNodeId tailNodeId)
    {
    if (tailNodeId == MTG_NULL_NODEID)
        return MTGPositionDetail ();
    DPoint3d xyz0 = m_facets.GetXYZ (tailNodeId);
    MTGNodeId headNodeId = m_graph.FSucc (tailNodeId);
    DPoint3d xyz1 = m_facets.GetXYZ(headNodeId);
    return xyz0.z <= xyz1.z
            ? MTGPositionDetail::FromVertex (tailNodeId, xyz0)
            : MTGPositionDetail::FromVertex (headNodeId, xyz1);
    }

MTGPositionDetail MinZAroundFaceOnSteepestDescent (MTGPositionDetailCR seedPos)
    {
    DVec3d faceNormal;
    DVec3d cutNormal, descentVector;
    auto zVec = DVec3d::From (0,0,1);
    MTGPositionDetail result = MTGPositionDetail ();
    if (!GetNormalAtNode (seedPos.NodeId (), faceNormal))
        return result;
    if (faceNormal.IsParallelTo (zVec))
        return result;

    cutNormal.CrossProduct (faceNormal, zVec);
    descentVector.NormalizedCrossProduct (cutNormal, faceNormal);
    DPoint3d xyzA, xyzB;
    MTGNodeId seedNodeId = seedPos.NodeId ();
    DPoint3d seedXYZ = seedPos.XYZ ();
    MTGARRAY_FACE_LOOP (nodeIdA, &m_graph, seedNodeId)
        {
        MTGNodeId nodeIdB = m_graph.FSucc (nodeIdA);
        xyzA = m_facets.GetXYZ (nodeIdA);
        xyzB = m_facets.GetXYZ(nodeIdB);
        double hA = xyzA.DotDifference (seedXYZ, cutNormal);
        double hB = xyzB.DotDifference (seedXYZ, cutNormal);
        if (hA * hB < 0.0)
            {
            double f = -hA / (hB - hA);
            DPoint3d xyz;
            xyz.Interpolate (xyzA, f, xyzB);
            if (!result.HasNodeId () || xyz.z < result.Z ())
                result = MTGPositionDetail::FromEdge (nodeIdA, xyz, f);
            }
        }
    MTGARRAY_END_FACE_LOOP (nodeIdA, &m_graph, seedNodeId)
    return result;
    }

double AngleFromXYPlane (MTGPositionDetailCR positionA, MTGPositionDetail positionB)
    {
    if (!positionA.HasNodeId () || !positionB.HasNodeId ())
        return DBL_MAX;

    DVec3d vector = positionA.VectorTo (positionB);
    double dxy = vector.Magnitude ();
    double dz  = vector.z;
    if (dxy == 0.0 && dz == 0.0)
        return DBL_MAX;
    return Angle::Atan2 (dz, dxy);
    }

// Search both adjacent faces and edges for steepest descent progress.
MTGPositionDetail MinZFromEdgeOnSteepestDescent (MTGPositionDetailCR seedPos)
    {
    MTGPositionDetail matePos = seedPos.EdgeMate (m_graph);
    MTGPositionDetail candidate [3] =
            {
            MinZAroundFaceOnSteepestDescent (seedPos),
            MinZAroundFaceOnSteepestDescent (matePos),
            MinZAlongEdge (seedPos.NodeId ())
            };
    double minAngle = AngleFromXYPlane (seedPos, candidate[0]);
    MTGPositionDetail result = candidate[0];
    for (int i = 1; i < 3; i++)
        {
        MTGPositionDetail::UpdateMinimizer (
                        result, minAngle,
                        candidate[i], AngleFromXYPlane (seedPos, candidate[i]));
        }
    return result;
    }

// Search all adjacent faces and edges for steepest descent progress.
MTGPositionDetail MinZFromVertexOnSteepestDescent (MTGPositionDetailCR seedPos)
    {
    MTGPositionDetail result = MTGPositionDetail ();
    if (!seedPos.HasNodeId ())
        return result;
    double minAngle = DBL_MAX;
    MTGNodeId seedNodeId = seedPos.NodeId ();
    MTGARRAY_VERTEX_LOOP (nodeId, &m_graph, seedNodeId)
        {
        DPoint3d xyz = m_facets.GetXYZ (nodeId);
        MTGPositionDetail vertexPos = MTGPositionDetail::FromVertex (nodeId, xyz);
        MTGPositionDetail edgeCandidate = MinZAlongEdge (nodeId);
        MTGPositionDetail faceCandidate = MinZAroundFaceOnSteepestDescent (vertexPos);
        double edgeAngle = AngleFromXYPlane (vertexPos, edgeCandidate);
        double faceAngle = AngleFromXYPlane (vertexPos, faceCandidate);
        MTGPositionDetail::UpdateMinimizer (result, minAngle, edgeCandidate, edgeAngle);
        MTGPositionDetail::UpdateMinimizer (result, minAngle, faceCandidate, faceAngle);
        }
    MTGARRAY_END_VERTEX_LOOP (nodeId, &m_graph, seedNodeId)

    return result;
    }



MTGPositionDetail ChooseFlowTarget (MTGPositionDetailCR start)
    {
    MTGPositionDetail next = start;
    if (start.IsFace ())
        {
        next = MinZAroundFaceOnSteepestDescent (start);
        }
    else if (start.IsEdge ())
        {
        // Make sure it is not close to vertex?
        next = MinZFromEdgeOnSteepestDescent (start);
        }
    else if (start.IsVertex ())
        {
        next = MinZFromVertexOnSteepestDescent (start);
        }
    return next;
    }
// Trace gravity flow from seed point.
// Seed point is assumed to be on-face -- caller is responsible for this.
void FollowFlowFromNodeAndPoint (MTGPositionDetailCR seedPos, bvector<DPoint3d> &path)
    {
    path.clear ();
    path.push_back (seedPos.XYZ ());
    MTGPositionDetail currPos = seedPos;
 
    for (;;)
        {
        MTGPositionDetail nextPos = ChooseFlowTarget (currPos);        
        if (!nextPos.HasNodeId ())
            return;
        if (nextPos.Z () >= currPos.Z ())
            break;
        path.push_back (nextPos.XYZ ());
        currPos = nextPos;
        }
    }

// Test if face contains xy projection of testXYZ ...
bool FaceContainsPoint (MTGNodeId faceSeedNodeId, DPoint3dCR testXYZ, MTGPositionDetailR faceDetail)
    {
    faceDetail = MTGPositionDetail ();
    MTGNodeId nodeIdA = m_graph.FSucc (faceSeedNodeId);
    MTGNodeId nodeIdB = m_graph.FSucc (nodeIdA);
    DPoint3d xyzOrigin = m_facets.GetXYZ (faceSeedNodeId);
    DPoint3d xyzA = m_facets.GetXYZ (nodeIdA);
    DVec3d vectorQ = testXYZ - xyzOrigin;
    // Note.. xy cross products produce twice the area of triangles.
    //   All areas are doubled, doubling will cancel in ratios.
    int numPositiveHit = 0;
    int numNegativeHit = 0;
    double areaSum = 0.0;
    while (nodeIdB != faceSeedNodeId)
        {
        DPoint3d xyzB = m_facets.GetXYZ (nodeIdB);
        DVec3d vectorA, vectorB;
        vectorA.DifferenceOf (xyzA, xyzOrigin);
        vectorB.DifferenceOf (xyzB, xyzOrigin);
        double areaOriginAB = vectorA.CrossProductXY (vectorB);
        double areaOriginAQ = vectorA.CrossProductXY (vectorQ);
        double areaOriginQB = vectorQ.CrossProductXY (vectorB);
        areaSum += areaOriginAB;
        double lambdaA, lambdaB, lambdaAB;
        if (DoubleOps::SafeDivide (lambdaA, areaOriginAQ, areaOriginAB, 0.0)
            && DoubleOps::SafeDivide (lambdaB, areaOriginQB, areaOriginAB, 0.0))
            {
            lambdaAB = 1.0 - lambdaA - lambdaB;
            if (lambdaA >= 0.0 && lambdaB >= 0.0 && lambdaAB >= 0.0)
                {
                DPoint3d piercePoint;
                piercePoint.SumOf (xyzOrigin, lambdaAB, xyzA, lambdaB, xyzB, lambdaA);
                faceDetail = MTGPositionDetail::FromFace (faceSeedNodeId, piercePoint);
                if (areaOriginAB > 0.0)
                    numPositiveHit++;
                else
                    numNegativeHit++;
                }
            }
        nodeIdA = nodeIdB;
        xyzA = xyzB;
        nodeIdB = m_graph.FSucc (nodeIdB);
        }
    return ((numPositiveHit + numNegativeHit) & 0x01) != 0;
    }

MTGPositionDetail FindContainingFace (DPoint3dCR seedXYZ)
    {
    MTGPositionDetail faceDetail = MTGPositionDetail ();
    MTGMask visitMask = m_graph.GrabMask ();
    m_graph.ClearMask (visitMask);
    MTGARRAY_SET_LOOP (seedNodeId, &m_graph)
        {
        if (!m_graph.GetMaskAt (seedNodeId, visitMask | MTG_EXTERIOR_MASK))
            {
            m_graph.SetMaskAroundFace (seedNodeId, visitMask);
            if (FaceContainsPoint (seedNodeId, seedXYZ, faceDetail))
                goto cleanup;
            }
        }
    MTGARRAY_END_SET_LOOP (seedNodeId, &m_graph)
cleanup:
    m_graph.DropMask (visitMask);
    return faceDetail;
    }

public:
void CollectFlowPaths (bvector<DPoint3d> const &startPoints, bvector<bvector<DPoint3d>> &flowPaths)
    {
    bvector<DPoint3d> currentPath;
    flowPaths.clear ();
    for (DPoint3d xyz : startPoints)
        {
        MTGPositionDetail pierceData = FindContainingFace (xyz);
        if (pierceData.HasNodeId ())
            {
            FollowFlowFromNodeAndPoint (pierceData, currentPath);
            if (currentPath.size () > 1)
                flowPaths.push_back (currentPath);
            }
        }
    }

};

//! For each point in startPoints[]:
//! 1) find a containing facet in top view.
//! 2) follow gravity flow line.
//! 3) Add flowline points to flowLines.

void  MTGFacets_CollectFlowPaths
(
    MTGFacets &facets,
    bvector<DPoint3d> const &startPoints,
    bvector<bvector<DPoint3d>> &paths
)
    {
    paths.clear ();
    FlowLinesContext context (facets);
    context.CollectFlowPaths (startPoints, paths);
    }
/** Backward compatible wrapper for MTGFacets_CollectFlowPaths */
Public void jmdlMTGFacets_collectFlowPaths
(
    MTGFacetsP facets,
    DPoint3dCP points,
    int   numPoints,
    EmbeddedDPoint3dArrayP flowLinesWithDisconnects
)
    {
    flowLinesWithDisconnects->clear ();
    bvector<DPoint3d> seedPoints;
    bvector<bvector<DPoint3d>> flowPaths;
    for (int i = 0; i < numPoints; i++)
        seedPoints.push_back (points[i]);
    MTGFacets_CollectFlowPaths(*facets, seedPoints, flowPaths);
    DPoint3d disconnect;
    disconnect.InitDisconnect ();
    for (auto &path : flowPaths)
        {
        for (auto &xyz : path)
            flowLinesWithDisconnects->push_back (xyz);
        flowLinesWithDisconnects->push_back (disconnect);
        }
    }

END_BENTLEY_GEOMETRY_NAMESPACE