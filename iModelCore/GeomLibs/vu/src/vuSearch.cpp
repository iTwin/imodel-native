/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/cluster.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

// local replacement for DSegment3d::ProjectPointToLineXY (to eliminate any use of DSegment3d in vu-only lib)
static bool projectPointToLineXY (DPoint3dR xyzD, double &fractionD,
    DPoint3dCR xyzA, DPoint3dCR xyzB, DPoint3dCR xyzC)
    {
    DVec3d vectorU = DVec3d::FromStartEnd (xyzA, xyzB);
    DVec3d vectorV = DVec3d::FromStartEnd (xyzA, xyzC);
    double dotUU = vectorU.DotProductXY (vectorU);
    double dotUV = vectorU.DotProductXY (vectorV);
    bool stat = DoubleOps::SafeDivide (fractionD, dotUV, dotUU, 0.0);
    xyzD.SumOf (xyzA, vectorU, fractionD);
    return stat;
    }


DPoint3d vu_interpolate (VuP node0, double f, VuP node1)
    {
    DPoint3d xyz0, xyz1, xyz;
    vu_getDPoint3d (&xyz0, node0);
    vu_getDPoint3d (&xyz1, node1);
    xyz.Interpolate (xyz0, f, xyz1);
    return xyz;
    }




struct CrossingState
{
enum CrossingLocation 
    {
    CL_LeftUp  = 0,
    CL_LeftDn  = 1,
    CL_RightUp = 2,
    CL_RightDn = 3
    };

ptrdiff_t m_counter[4];
VuPositionDetail m_position[2];
void Init ()
    {
    for (int i = 0; i < 4; i++)
        m_counter[i] = 0;
    m_position[0] = VuPositionDetail (0);
    m_position[1] = VuPositionDetail (1);
    }

CrossingState (){Init();}

void UpdateCount (CrossingLocation selector)
    {
    m_counter[selector]++;
    }


void GetDetailPair (VuPositionDetailR detail0, VuPositionDetailR detail1)
    {
    detail0 = m_position[0];
    detail1 = m_position[1];
    }
// Update one or both of the position details for as x range extrema of vertx type
void UpdateXRange (VuP node, DPoint3dCR xyz)
    {
    if (!m_position[0].IsVertex () || xyz.x < m_position[0].GetX ())
        m_position[0] = VuPositionDetail::FromVertex (node, xyz);
    if (!m_position[1].IsVertex () || xyz.x > m_position[1].GetX ())
        m_position[1] = VuPositionDetail::FromVertex (node, xyz);
    }

void InitXRange (VuP node)
    {
    m_position[0] = VuPositionDetail::FromVertex (node);
    m_position[1] = VuPositionDetail::FromVertex (node);
    }

void UpdateDetailByEdgeCrossing (size_t selector, VuP node, DPoint3dCR xyz, double f)
    {
    if (selector == 0)
        {
        if (m_position[0].IsUnclassified () || xyz.x > m_position[0].GetX ())
            m_position[0] = VuPositionDetail::FromEdge (node, xyz, f);
        }
    else
        {
        if (m_position[1].IsUnclassified () || xyz.x < m_position[1].GetX ())
            m_position[1] = VuPositionDetail::FromEdge (node, xyz, f);
        }
    }

void UpdateDetailByVertexOnAxis (size_t selector, VuP node, DPoint3dCR xyz)
    {
    if (selector == 0)
        {
        // Look for new xyz to the right of m_position[0]:
        if (m_position[0].IsUnclassified () || xyz.x > m_position[0].GetX ())
            m_position[0] = VuPositionDetail::FromVertex (node);
        }
    else
        {
        // Look for new xyz to the left of m_position[1]:
        if (m_position[1].IsUnclassified () || xyz.x < m_position[1].GetX ())
            m_position[1] = VuPositionDetail::FromVertex (node);
        }
    }

// leftDetail and rightDetail are tricky.
// Caller must flipflop order to get this effect.
// For coordinate purposes, leftDetail is only updated for a DOWNWARD crossing.
//   BUT THE COUNTER IS UPDATED always and is really "CounterClockwise" counter, not "left" counter.
// ASSUME y0, y1 are strictly on opposite sides of xyz.y (so division step of interpolating at xyz.y is safe)
// Return true with detail if the crossing is an EXACT hit
// Otherwise return false and increment account according to x.
bool IsSimpleCrossingExactHit
(
DPoint3dCR xyz,
VuP node0,
double y0,
VuP node1,
double y1,
VuPositionDetailR terminalDetail
)
    {
    // Simple crossing !!!!
    double f = (xyz.y - y0) / (y1 - y0);
    DPoint3d xyzC = vu_interpolate (node0, f, node1);
    if (xyzC.x > xyz.x)
        {
        UpdateDetailByEdgeCrossing (1, node0, xyzC, f);
        UpdateCount (y1 > y0 ? CL_RightUp : CL_RightDn);
        }
    else if (xyzC.x < xyz.x)
        {
        UpdateDetailByEdgeCrossing (0, node0, xyzC, f);
        UpdateCount (y1 > y0 ? CL_LeftUp : CL_LeftDn);
        }
    else
        {
        terminalDetail = VuPositionDetail::FromEdge (node0, xyzC, f);
        return true;
        }
    return false;
    }

// ASSUME y0 is nonzero.
// ASSUME y1 is exactly zero.
// Look at subsequent edges with exact y=0.
// Return true with detail if the ON sequence goes through xyz.x.
// Otherwise return false and increment account according to x of the ON edges
bool IsAxisSequenceExactHit (DPoint3dCR xyz, VuP node0, VuPositionDetailR detail)
    {
    double yRef = xyz.y;
    DPoint3d xyz2;
    double y0, y1, y2;
    y0 = vu_getY (node0);
    VuP node1 = vu_fsucc (node0);
    y1 = vu_getY (node1);
    if (y0 == yRef || y1 != yRef)
        return false;   // NEVER NEVER NEVER OCCURS.    This test ensures the loop will reach some
    
    CrossingState    xBracket;
    xBracket.InitXRange (node0);

    for (VuP node2 = vu_fsucc (node1);
        node1 != node0;  // Really shouldn't hit it -- coordinate tests should exit
        node1 = node2, y1 = y2, node2 = vu_fsucc (node2)
        )
        {
        vu_getDPoint3d (&xyz2, node2);
        y2 = xyz2.y;
        if (y2 > yRef)
            {
            int faceSelect = xBracket.m_position[0].GetX () > xyz.x ? 1 : 0;
            int xSelect = 1 - faceSelect;
            UpdateDetailByVertexOnAxis (faceSelect, xBracket.m_position[xSelect].GetNode (), xBracket.m_position[xSelect].GetXYZ ());
            if (y0 < yRef)
                UpdateCount (xSelect == 0 ? CL_LeftUp : CL_RightUp);
            return false;
            }
        else if (y2 < yRef)
            {
            int faceSelect = xBracket.m_position[0].GetX () > xyz.x ? 1 : 0;
            int xSelect = 1 - faceSelect;
            UpdateDetailByVertexOnAxis (faceSelect, xBracket.m_position[xSelect].GetNode (), xBracket.m_position[xSelect].GetXYZ ());
            if (y0 > yRef)
                UpdateCount (xSelect == 0 ? CL_LeftDn : CL_RightDn);
            return false;
            }
        else
            {
            // node1..node2 is on.  Check for exact hit ...
            DPoint3d xyz1;
            vu_getDPoint3d (&xyz1, node1);
            xBracket.UpdateXRange (node2, xyz2);

            if (xyz1.x == xyz.x)
                {
                detail = VuPositionDetail::FromVertex (node1, xyz1);
                return true;
                }
            else if (xyz2.x == xyz.x)
                {
                detail = VuPositionDetail::FromVertex (node2, xyz2);
                return true;
                }
            else
                {
                // These are nonzero ....
                double d1 = xyz.x - xyz1.x;
                double d2 = xyz.x - xyz2.x;
                if (d1 * d2 < 0.0)
                    {
                    // x coordinates strictly bracket xyz.x, so division is safe.
                    double f = (xyz.x - xyz1.x) / (xyz2.x - xyz1.x);
                    DPoint3d xyzHit;
                    xyzHit.Interpolate (xyz1, f, xyz2);
                    detail = VuPositionDetail::FromEdge (node1, xyzHit, f);
                    return true;
                    }
                }
            }
        }
    return false;
    }


// In planar parity there must be an odd counter to both left and right.
// (Just one would do, we'll be strict and check both)
bool IsInsideByPlanarParity ()
    {
    return ((m_counter[CL_LeftUp] + m_counter[CL_LeftDn]) & 1) == 1
        && ((m_counter[CL_RightUp] + m_counter[CL_RightDn]) & 1) == 1;
    }
};

// In/out test by HORIZONTAL scan line.
// Returns true if classification logic succeeds.
// @param [out] hitDetail describes incidence on face.
//! <ul>
//! <li>Topo_None   = point it outside.
//! <li>Topo_Vertex = point is at vertex.
//! <li>Topo_Edge   = point is along edge
//! <li>Topo_Face   = point is inside face
//! </ul>
//! @param [out] leftDetail For Topo_Face case, returns closest edge or vertex hit at left.
//! @param [out] rightDetail For Topo_Face case, returns closest edge or vertex hit at right.
bool vu_isPointInOrOnByHorizontalScan (VuSetP pGraph, VuP nodeA, DPoint3d xyz, VuPositionDetail &hitDetail, VuPositionDetail &leftDetail, VuPositionDetail &rightDetail)
    {
    double yRef = xyz.y;
    double y0, y1;
    CrossingState crossingState;
    hitDetail = VuPositionDetail ();
    leftDetail = rightDetail = hitDetail;
    

    VU_FACE_LOOP (node0, nodeA)
        {
        y0 = vu_getY (node0);
        VuP node1 = vu_fsucc (node0);
        y1 = vu_getY (node1);
        if (y0 == yRef)
            {
            // Ignore.  This will get handled "from" an off-axis start point.  "Just skip" here eliminates termination and wraparound logic elsehwere.
            }
        else if (y0 < yRef)
            {
            // Walk ahead to positive point.
            if (y1 > yRef)
                {
                if (crossingState.IsSimpleCrossingExactHit (xyz, node0, y0, node1, y1, hitDetail))
                    return true;
                }
            else if (y1 == yRef)
                {
                if (crossingState.IsAxisSequenceExactHit (xyz, node0, hitDetail))
                    return true;
                }
            }
        else
            {
            if (y1 < yRef)
                {
                if (crossingState.IsSimpleCrossingExactHit (xyz, node0, y0, node1, y1, hitDetail))
                    return true;
                }
            else if (y1 == yRef)
                {
                if (crossingState.IsAxisSequenceExactHit (xyz, node0, hitDetail))
                    return true;
                }
            }
        }
    END_VU_FACE_LOOP (node0, nodeA)
    
    if (crossingState.IsInsideByPlanarParity ())
        {
        // Summary detail says face ..
        hitDetail = VuPositionDetail::FromFace (nodeA, xyz);
        // Left and right edges.
        crossingState.GetDetailPair (leftDetail, rightDetail);
        return true;
        }
    return false;
    }


bool resolveToEdge (DPoint3dCR xyzA, DPoint3dCR xyzB, DPoint3dCR xyz, double &fraction)
    {
    // xyz and nodeA are both on the (possibly extended) edge.
    DVec3d vectorAtoX, vectorAtoB;
    vectorAtoX.DifferenceOf (xyz, xyzA);
    vectorAtoB.DifferenceOf (xyzB, xyzA);
    double numerator   = vectorAtoX.DotProductXY (vectorAtoB);
    double denominator = vectorAtoB.DotProductXY (vectorAtoB);

    if (numerator == 0.0)
        {
        fraction = 0.0;
        return true;
        }           
    else if (numerator > 0.0 && numerator <= denominator)
        {
        fraction = numerator / denominator;
        return true;
        }
    return false;
    }
    
Public bool vu_faceContainsPoint (VuSetP pGraph, VuP nodeA, DPoint3dCR xyz, VuP &edgeHitNode, double &edgeFraction)
    {
    DPoint3d xyzA, xyzB, xyzC;
    vu_getDPoint3d (&xyzA, nodeA);
    VuP nodeB = vu_fsucc (nodeA);
    vu_getDPoint3d (&xyzB, nodeB); 
    VuP nodeC = vu_fsucc (nodeB);
    vu_getDPoint3d (&xyzC, nodeC);
    int numIn = 0;  // Incremented at any simple containment
    int numOn = 0;  // Incremented at zero area between xyz and nodeA..nodeC.
                    // Decremented at zero area between xyz and nodeA..nodeB.
                    // Bitwise copy of area values ensures proper cancelation on foldback and increases by 2
                    // if the "on" edge is true interior.
    //double b0 = 0.0;
    double a0 = xyz.CrossProductToPointsXY (xyzA, xyzB);
    edgeHitNode = NULL;
    edgeFraction = 0.0;
    if (a0 == 0.0 && resolveToEdge (xyzA, xyzB, xyz, edgeFraction))
        {
        edgeHitNode = nodeA;
        return true;
        }
    while (nodeB != nodeA)
        {
        double a1 = xyz.CrossProductToPointsXY (xyzB, xyzC);
        double a2 = xyz.CrossProductToPointsXY (xyzC, xyzA);
        double b = a0 + a1 + a2;

        if (a1 == 0.0)
            {
            if (b > 0.0)
                {
                if (a0 >= 0.0 && a2 >= 0.0)
                    {
                    edgeFraction = a0 / b;
                    edgeHitNode = nodeB;
                    return true;
                    }
                }
            else if (b < 0.0)
                {
                if (a0 <= 0.0 && a2 <= 0.0)
                    {
                    edgeFraction = a0 / b;
                    edgeHitNode = nodeB;
                    return true;
                    }
                }
            else
                {
                if (resolveToEdge (xyzB, xyzC, xyz, edgeFraction))
                    {
                    edgeHitNode = nodeB;
                    return true;
                    }
                }
            }
        else if (b > 0.0)
            {
            if (a2 == 0.0)
                {
                if (a0 > 0.0 && a1 > 0.0)
                    numOn++;
                }
            else if (a2 > 0.0 && a1 > 0.0)
                {
                numIn++;
                }
            }
        else
            {
            if (a2 == 0.0)
                {
                if (a0 > 0.0 && a1 > 0.0)
                    numOn--;
                }
            else if (a2 < 0.0 && a1 < 0.0 && a2 < 0.0)
                {
                numIn++;
                }
            }
        nodeB = nodeC;
        xyzB = xyzC;
        nodeC = vu_fsucc (nodeB);
        vu_getDPoint3d (&xyzC, nodeC);
        a0 = -a2;   // Ensure bitwise equality on zero test with reverse order !!!
        }

    numIn += (numOn / 2);
    return 1 == (numIn & 0x01);
    }

//! LINEAR SEARCH for any face that contains xyz by parity rules.
//! @param [in] skipMask mask for faces to skip.
Public VuP vu_findContainingFace_linearSearch (VuSetP pGraph, DPoint3dCR xyz, VuMask skipMask)
    {
    VuP faceNode = NULL;
    VuMask visitMask = vu_grabMask (pGraph);
    vu_clearMaskInSet (pGraph, visitMask);

    VuPositionDetail primaryHit, leftHit, rightHit;
    VU_SET_LOOP (faceSeed, pGraph)
        {
        if (!vu_getMask (faceSeed, visitMask))
            {
            vu_setMaskAroundFace (faceSeed, visitMask);
            if (!vu_getMask (faceSeed, skipMask))
                {
                if (vu_isPointInOrOnByHorizontalScan (pGraph, faceSeed, xyz, primaryHit, leftHit, rightHit))
                    {
                    faceNode = faceSeed;
                    goto cleanup;
                    }
                }
            }
        }
    END_VU_SET_LOOP (faceSeed, pGraph)

cleanup:
    vu_returnMask (pGraph, visitMask);
    return faceNode;
    }

//! LINEAR SEARCH for for closests vertex
//! @param [in] skipMask mask for faces to skip.
Public VuP vu_findClosestVertexXY_linearSearch (VuSetP pGraph, DPoint3dCR xyz, VuMask skipMask)
    {
    VuP nearNode = NULL;
    double a2Min = DBL_MAX;
    VU_SET_LOOP (node, pGraph)
        {
        if (!vu_getMask (node, skipMask))
            {
            DPoint3d xyz1;
            vu_getDPoint3d (&xyz1, node);
            double a2 = xyz.DistanceSquaredXY (xyz1);
            if (a2 < a2Min)
                {
                a2Min = a2;
                nearNode = node;
                }   
            }
        }
    END_VU_SET_LOOP (node, pGraph)
    return nearNode;
    }

//! LINEAR SEARCH for for closests vertex OR FACE
//! @param [in] skipMask mask for faces to skip.
//! @param [in] any vertex this close is a hit even if the edge comes closer.
Public VuPositionDetail vu_findClosestEdgeOrVertexXY_linearSearch (VuSetP pGraph, DPoint3dCR xyz, VuMask skipMask, double vertexProximityTolerance)
    {
    VuPositionDetail result;
    double a2Min = DBL_MAX;
    double b2Min = vertexProximityTolerance * vertexProximityTolerance;
    VU_SET_LOOP (tailNode, pGraph)
        {
        VuP edgeMate = vu_edgeMate (tailNode);
        if (!vu_getMask (tailNode, skipMask)
            && (vu_getMask (edgeMate, skipMask)
                || (size_t)tailNode < (size_t)edgeMate))
            {
            DPoint3d xyz0, xyz1, xyz2;
            vu_getDPoint3d (&xyz0, tailNode);
            VuP headNode = vu_fsucc (tailNode);
            vu_getDPoint3d (&xyz1, headNode);
            double dot0 = xyz0.DotProductToPointsXY (xyz1, xyz);
            double dot1 = xyz0.DotProductToPointsXY (xyz1, xyz1);
            double tailDistance2 = xyz0.DistanceSquaredXY (xyz);
            double headDistance2 = xyz1.DistanceSquaredXY (xyz);
            double lambda;
            if (!DoubleOps::SafeDivide (lambda, dot0, dot1, 0.0))
                {
                // Degenerate edge.   Just let it be a vertex.
                if (tailDistance2 < a2Min)
                    {
                    result = VuPositionDetail::FromVertex (tailNode);
                    a2Min = tailDistance2;
                    }
                }
            else if (lambda < 0.5 && tailDistance2 < b2Min)
                {
                if (tailDistance2 < a2Min)
                    {
                    result = VuPositionDetail::FromVertex (tailNode, xyz0);
                    a2Min = tailDistance2;
                    }
                }
            else if (lambda > 0.5 && headDistance2 < b2Min)
                {
                if (headDistance2 < a2Min)
                    {
                    result = VuPositionDetail::FromVertex (tailNode, xyz0);
                    a2Min = headDistance2;
                    }
                }
            else
                {
                // Clamp to bounded edge ...
                if (lambda < 0.0)
                    lambda = 0.0;
                if (lambda > 1.0)
                    lambda = 1.0;
                xyz2.Interpolate (xyz0, lambda, xyz1);
                double a2 = xyz.DistanceSquaredXY (xyz2);
                if (a2 < a2Min)
                    {
                    result = VuPositionDetail::FromEdge (tailNode, xyz2, lambda);
                    a2Min = a2;
                    }
                }
            }
        }
    END_VU_SET_LOOP (tailNode, pGraph)
    return result;
    }


// Create a VuPositionDetail for specified fraction along any unmasked edge.
static VuPositionDetail vu_findAnyUnmaskedEdge (VuSetP pGraph, double edgeFraction, VuMask skipMask)
    {
    VU_SET_LOOP (edgeNode, pGraph)
        {
        if (!vu_getMask (edgeNode, skipMask))
            {
            DPoint3d xyz = vu_interpolate (edgeNode, edgeFraction, vu_fsucc (edgeNode));
            return VuPositionDetail::FromEdge (edgeNode, xyz, edgeFraction);
            }
        }
    END_VU_SET_LOOP (edgeNode, pGraph)
    return VuPositionDetail ();
    }
#ifdef CompileUnusedFunction
static void NodeCoordinatesOnRay (VuP node, DRay3d ray, DPoint3dR nodeXYZ, double &u, double &v)
    {
    vu_getDPoint3d (&nodeXYZ, node);
    DVec3d vec;
    vec.DifferenceOf (nodeXYZ, ray.origin);
    u = ray.direction.DotProductXY (vec);
    v = ray.direction.CrossProductXY (vec);
    }
#endif
static void PointCoordinatesOnRay (DPoint3d xyz, DRay3d ray, double &u, double &v)
    {
    DVec3d vec;
    vec.DifferenceOf (xyz, ray.origin);
    u = ray.direction.DotProductXY (vec);
    v = ray.direction.CrossProductXY (vec);
    }

struct NodeDPoint3dDPoint2d
{
VuP      m_node;
DPoint3d m_xyz;
DPoint2d m_uv;
NodeDPoint3dDPoint2d (VuP node, DPoint3dCR xyz, DPoint2dCR uv)
    : m_node (node), m_xyz(xyz), m_uv(uv)
    {
    }

NodeDPoint3dDPoint2d ()
    {
    m_xyz.Zero ();
    m_uv.Zero ();
    m_node = NULL;
    }

static NodeDPoint3dDPoint2d FromNodeAndRay (VuP node, DRay3dCR ray)
    {
    DPoint3d xyz;
    vu_getDPoint3d (&xyz, node);
    DPoint2d uv;
    DVec3d vector;
    vector.DifferenceOf (xyz, ray.origin);
    uv.x = ray.direction.DotProductXY (vector);
    uv.y = ray.direction.CrossProductXY (vector);
    return NodeDPoint3dDPoint2d (node, xyz, uv);
    }

VuP GetNode () const { return m_node;}

DPoint3d GetXYZ () const { return m_xyz;}
DPoint2d GetUV  () const { return m_uv; }

double GetX () const { return m_xyz.x;}
double GetY () const { return m_xyz.y;}
double GetZ () const { return m_xyz.z;}

double GetU () const { return m_uv.x; }
double GetV () const { return m_uv.y; }

int    ClassifyU (double target, double tol) const
    {
    if (fabs (m_uv.x - target) < tol)
        return 0;
    return m_uv.x > target ? 1 : -1;
    }

int    ClassifyV (double target, double tol) const
    {
    if (fabs (m_uv.y - target) < tol)
        return 0;
    return m_uv.y > target ? 1 : -1;
    }

};

struct PointSearchContext
{
VuSetP m_graph;
double m_tol;
PointSearchContext (VuSetP graph)
    : m_graph (graph)
    {
    m_tol = vu_getMergeTol (graph);
    }


enum RayClassification {
    RC_NoHits,
    RC_TargetOnVertex,
    RC_TargetOnEdge,
    RC_Bracket,
    RC_TargetBefore,
    RC_TargetAfter
    } ;

VuPositionDetail Panic () const
    {
    return VuPositionDetail (-1);
    }
// From given edge start point, pick vertex or edge side for proceeding along ray.
VuPositionDetail ReAimFromEdge
(
VuPositionDetail edgeHit,
DRay3dCR ray,
double targetDistance  //!< distance to target point
)
    {
    VuP nodeA = edgeHit.GetNode ();
    NodeDPoint3dDPoint2d dataA = NodeDPoint3dDPoint2d::FromNodeAndRay (nodeA, ray);
    NodeDPoint3dDPoint2d dataB = NodeDPoint3dDPoint2d::FromNodeAndRay (vu_edgeMate (nodeA), ray);
    VuPositionDetail result;
    int sideA = dataA.ClassifyV (0.0, m_tol);
    int sideB = dataB.ClassifyV (0.0, m_tol);

    if (sideA * sideB < 0)
        {
        // Simple crossing -- just aim into a face
        if (sideA > 0)
            {
            result = VuPositionDetail::FromFace (dataA.GetNode (), edgeHit.GetXYZ ());
            }
        else
            {
            result = VuPositionDetail::FromFace (dataB.GetNode (), edgeHit.GetXYZ ());
            }
        }
    else if (sideA == 0 || sideB == 0)
        {
        // The usual case is both 0 i.e. ray is clearly along the edge.
        
        int alongA = dataA.ClassifyU (targetDistance, m_tol);
        int alongB = dataB.ClassifyU (targetDistance, m_tol);
        if (alongA == 0 && sideA == 0)
            {
            result = VuPositionDetail::FromVertex (dataA.GetNode ());
            result.SetITag (1);
            }
        else if (alongB == 0 && sideB == 0)
            {
            result = VuPositionDetail::FromVertex (dataB.GetNode ());
            result.SetITag (1);
            }
        else if (alongA * alongB < 0)
            {
            // target is within edge
            // (hmmm.. This is written for the case where both sideA and sideB are zero.
            //    If only one is zero, this computes a close edge point but the strong "on" conclusion might be wrong)
            double edgeFraction = (targetDistance - dataA.GetU ()) / (dataB.GetU () - dataA.GetU ());
            DPoint3d xyz = vu_interpolate (dataA.GetNode (), edgeFraction, dataB.GetNode ());
            result = VuPositionDetail::FromEdge (dataA.GetNode (), xyz, edgeFraction);
            result.SetITag (1);
            }
        else if (alongA < 0 && alongB < 0)
            {
            // target is beyond the edge -- move towards it.
            result = VuPositionDetail::FromVertex (dataA.GetNode (),
                            dataA.GetU () > dataB.GetU () ? dataA.GetXYZ () : dataB.GetXYZ ());
            }
        else   
            {
            // This shouldn't happen -- maybe as if the initial edge point was not within the edge???
            if (   fabs (dataA.GetU ()) < m_tol
                && fabs (dataA.GetV ()) < m_tol
                )
                {
                result = VuPositionDetail::FromVertex (dataA.GetNode (), dataA.GetXYZ());
                }
            else if (  fabs (dataB.GetU ()) < m_tol
                    && fabs (dataB.GetV ()) < m_tol
                    )
                {
                result = VuPositionDetail::FromVertex (dataB.GetNode (), dataB.GetXYZ());
                }
            else
            result = Panic ();
            }
        }
    else
        {
        // Both vertices are to same side of the line.   This can't happen for edge point between nodes.
        result = Panic ();
        }
    return result;
    }


// From given edge start point, pick vertex or edge side for proceeding along ray.
// RAY IS ASSUMED TO START AT THE VERTEX PRECISELY !!!!
VuPositionDetail ReAimFromVertex
(
VuPositionDetail searchBase,
DRay3dCR ray,
double targetDistance  //!< distance to target point
)
    {
    VuP vertexNode = searchBase.GetNode ();
    VuPositionDetail result;
    VU_VERTEX_LOOP (outboundEdge, vertexNode)
        {
        DPoint3d xyzBase;
        vu_getDPoint3d (&xyzBase, outboundEdge);
        NodeDPoint3dDPoint2d data0 = NodeDPoint3dDPoint2d::FromNodeAndRay (vu_fsucc (outboundEdge), ray);
        NodeDPoint3dDPoint2d data1 = NodeDPoint3dDPoint2d::FromNodeAndRay (vu_fpred (outboundEdge), ray);
        double u0 = data0.GetU ();
        //double u1 = data1.GetU ();
        double v0 = data0.GetV ();
        double v1 = data1.GetV ();
        if (fabs (v0) < m_tol)
            {
            if (fabs (u0 - targetDistance) < m_tol)
                {
                // Direct hit at far end
                result = VuPositionDetail::FromVertex (data0.GetNode ());
                result.SetITag (1);
                return result;
                }
            else if (u0 > targetDistance)
                {
                // Direct hig within edge
                double edgeFraction = targetDistance / u0;
                DPoint3d xyz = vu_interpolate (outboundEdge, edgeFraction, data0.GetNode ());
                result = VuPositionDetail::FromEdge (outboundEdge, xyz, edgeFraction);
                return result;
                }
            else if (fabs (u0) <= m_tol)
                {
                // Unexpected direct hit on the base of the search, but call it a hit....
                result = VuPositionDetail::FromVertex (outboundEdge);
                result.SetITag (1);
                return result;
                }
            else if (u0 > m_tol)
                {
                // Advance to vertex  ...
                //double edgeFraction = targetDistance / u0;
                result = VuPositionDetail::FromVertex (data0.GetNode ());
                return result;
                }
            else
                {
                // Search direction is exactly opposite this edge.
                // See if the other side of the sector is turned even beyond that ...
                if (v1 > m_tol)
                    {
                    result = VuPositionDetail::FromFace (outboundEdge, xyzBase);
                    return result;
                    }
                }                
            }
        else if (v0 < -m_tol)
            {
            if (v1 > m_tol)
                {
                // The usual simple entry into an angle < 180
                result = VuPositionDetail::FromFace (outboundEdge, xyzBase);
                return result;
                }
            }
        // NEEDS WORK: angle >= 180 cases !!!!
        }
    END_VU_VERTEX_LOOP (outboundEdge, vertexNode)
    return Panic ();
    }

// Visit all edges around face.
// Return lastBefore and firstAfter describing progress towards target distance on ray.
RayClassification ReAimAroundFace
(
VuP    faceNode,
DRay3dCR ray,
double targetDistance,  //!< distance to target point
VuPositionDetailR lastBefore,   //! last hit at or before target distance
VuPositionDetailR firstAfter    //! first hit at or after target distance
)
    {
    lastBefore = VuPositionDetail ();
    firstAfter = VuPositionDetail ();
    lastBefore.SetDTag (-DBL_MAX);
    firstAfter.SetDTag (DBL_MAX);
    NodeDPoint3dDPoint2d data0, data1, data2;
    data0 = NodeDPoint3dDPoint2d::FromNodeAndRay(faceNode, ray);
    
    VU_FACE_LOOP (node0, faceNode)
        {
        VuP node1 = vu_fsucc (node0);
        data1 = NodeDPoint3dDPoint2d::FromNodeAndRay (node1, ray);
        //double u0 = data0.GetU ();
        double u1 = data1.GetU ();
        double v0 = data0.GetV ();
        double v1 = data1.GetV ();
        if (fabs (v1) < m_tol)
            {
            // Vertex hit ...
            VuPositionDetail vertexHit = VuPositionDetail::FromVertex (node1, data1.GetXYZ ());
            vertexHit.SetDTag (u1);
            if (fabs (u1 - targetDistance) < m_tol)
                {
                firstAfter = lastBefore = vertexHit;
                return RC_TargetOnVertex;
                }
            if (u1 > targetDistance && u1 < firstAfter.GetDTag ())
                firstAfter = vertexHit;
            if (u1 < targetDistance && u1 > lastBefore.GetDTag ())
                lastBefore = vertexHit;            
            }
        else if (v0 * v1 < 0.0)
            {
            // Edge Crossing ...
            double edgeFraction = - v0 / (v1 - v0);
            double uEdge, vEdge;
            DPoint3d xyzEdge = vu_interpolate (node0, edgeFraction, node1);
            PointCoordinatesOnRay (xyzEdge, ray, uEdge, vEdge);
            VuPositionDetail edgeHit = VuPositionDetail::FromEdge (data0.GetNode (), xyzEdge, edgeFraction);
            edgeHit.SetDTag (uEdge);
            if (fabs (uEdge - targetDistance) <= m_tol)
                {
                firstAfter = lastBefore = edgeHit;
                return RC_TargetOnEdge;
                }
            if (uEdge > targetDistance && uEdge < firstAfter.GetDTag ())
                {
                firstAfter = edgeHit;
                firstAfter.SetITag (v0 > 0.0 ? -1 : 1);
                }
            if (uEdge < targetDistance && uEdge > lastBefore.GetDTag ())
                lastBefore = edgeHit;
            }
        data0 = data1;
        }
    END_VU_FACE_LOOP (node0, faceNode)

    ptrdiff_t afterTag = firstAfter.GetITag ();
    firstAfter.SetITag (0);
    lastBefore.SetITag (0);
    if (lastBefore.IsUnclassified ())
        {
        if (firstAfter.IsUnclassified ())
            return PointSearchContext::RC_NoHits;
        return RC_TargetBefore;
        }
    if (firstAfter.IsUnclassified ()
        || (firstAfter.IsEdge () && afterTag < 0))
        {
        return RC_TargetAfter;
        }
    else
        {
        return RC_Bracket;
        }
    }

// Return false if target is reached !!!!
bool ConstructSearchRay (VuPositionDetailCR start, DPoint3dCR target, DRay3dR ray, double &distanceToTarget)
    {
    ray.origin = start.GetXYZ ();
    ray.direction.DifferenceOf (target, start.GetXYZ ());
    ray.direction.z = 0.0;
    distanceToTarget = ray.direction.Normalize ();
    return distanceToTarget >= m_tol;
    }
};

struct SearchHistory
{
#define MAX_SAVED 4
VuPositionDetail m_position[MAX_SAVED];
size_t m_numSaved;
size_t m_maxSave;
SearchHistory (size_t maxSave)
    : m_numSaved (0)
    {
    m_maxSave = maxSave;
    if (m_maxSave == 0 || m_maxSave > MAX_SAVED)
        m_maxSave = MAX_SAVED;
    }
static bool MatchNodeAndScope (VuPositionDetail &a, VuPositionDetail &b)
    {
    if (a.GetNode () == b.GetNode ()
        && a.GetTopologyScope () == b.GetTopologyScope ())
        return true;
    return false;
    }
// Return true if position matches a saved position.
bool FindNodeAndScope (VuPositionDetail & a)
    {
    if (a.IsUnclassified ())
        return false;
    for (size_t i = 0; i < m_numSaved; i++)
        {
        if (MatchNodeAndScope (a, m_position[i]))
            return true;
        }
    return false;
    }

// Save (if classified), tossing oldest if necessary
void Save (VuPositionDetail &a)
    {
    if (a.IsUnclassified ())
        return;
    if (m_numSaved == m_maxSave)
        {
        for (size_t i = 1; i < m_numSaved; i++)
            m_position[i-1] = m_position[i];
        m_numSaved--;
        }
    m_position[m_numSaved++] = a;
    }
};

// Return a face, edge, or vertex position detail that contains xyz.
// A successful search is indicated by {GetITag()==1} in the returned detail.
Public VuPositionDetail vu_moveTo (VuSetP pGraph, VuPositionDetailR startPosition, DPoint3dCR xyz, VuPositonDetailAnnouncer * announcer)
    {
    PointSearchContext psc (pGraph);
    SearchHistory history (4);
    VuPositionDetail searchPosition = startPosition;
    searchPosition.SetITag (0);
    if (searchPosition.IsUnclassified ())
        {
        VuPositionDetail seed = vu_findAnyUnmaskedEdge (pGraph, 0.5, 0);
        if (seed.IsUnclassified ())
            return seed;
        searchPosition = seed;
        }


    //double tol = vu_getMergeTol (pGraph);
    
    for (;searchPosition.GetITag () == 0;)
        {
        if (announcer != nullptr)
            {
            bool continueSearch = announcer->AnnounceAndTestPosition (searchPosition);
            if (!continueSearch)
                return searchPosition;
            }
        if (history.FindNodeAndScope (searchPosition))
            return VuPositionDetail ();

        history.Save (searchPosition);
        DRay3d ray;
        double distanceToTarget;
        if (!psc.ConstructSearchRay (searchPosition, xyz, ray, distanceToTarget))
            {
            searchPosition.SetITag (1);
            }
        else if (searchPosition.IsFace ())
            {
            VuPositionDetail lastBefore, firstAfter;
            PointSearchContext::RayClassification rc = psc.ReAimAroundFace (searchPosition.GetNode (), ray, distanceToTarget,
                        lastBefore, firstAfter);
            switch (rc)
                {
                case PointSearchContext::RC_NoHits:
                    {
                    searchPosition = VuPositionDetail (-1);
                    break;
                    }
                case PointSearchContext::RC_TargetOnVertex:
                    {
                    searchPosition = lastBefore;
                    searchPosition.SetITag (1);
                    break;
                    }
                case PointSearchContext::RC_TargetOnEdge:
                    {
                    searchPosition = lastBefore;
                    searchPosition.SetITag (1);
                    break;
                    }
                case PointSearchContext::RC_Bracket:
                    {
                    searchPosition = VuPositionDetail::FromFace (lastBefore.GetNode (), xyz);
                    searchPosition.SetITag (1);
                    break;
                    }
                case PointSearchContext::RC_TargetBefore:
                    {
                    searchPosition = VuPositionDetail::FromFace (searchPosition.GetNode (), xyz);
                    searchPosition.SetITag (1);
                    break;
                    }
                case PointSearchContext::RC_TargetAfter:
                    {
                    searchPosition = lastBefore;
                    break;
                    }                
                }
            }
        else if (searchPosition.IsEdge ())
            {
            searchPosition = psc.ReAimFromEdge (searchPosition, ray, distanceToTarget);
            }
        else if (searchPosition.IsVertex ())
            {
            searchPosition = psc.ReAimFromVertex (searchPosition, ray, distanceToTarget);
            }
        }

    return searchPosition;
    }

double AssessFace(VuP seed)
    {
    double area = vu_area (seed);
    double maxLength = 0.0;
    double ratio;
    VU_FACE_LOOP (edge, seed)
        {
        double length = vu_edgeLengthXY (edge);
        if (length > maxLength)
            maxLength = length;
        }
    END_VU_FACE_LOOP (edge, seed)
    DoubleOps::SafeDivide (ratio, area, maxLength, 0.0);
    return ratio;
    }
struct InsertAndRetriangulateContext
{
private:
VuSetP m_graph;
VuPositionDetail m_searcher;

// Walk face from edgeNode;  insert new edges back to start node from all except
//   immediate successor and predecessor.
// insert all new nodes, and nodes of the existing face, in edgeSet.
void RetriangulateFromBaseVertex (VuP edgeNode, VuMarkedEdgeSetP edgeSet)
    {
    int numNode = vu_countEdgesAroundFace (edgeNode);
    if (nullptr != edgeSet)
        {
        VU_FACE_LOOP (node, edgeNode)
            {
            vu_markedEdgeSetTestAndAdd(edgeSet,node);
            }
        END_VU_FACE_LOOP (node, edgeNode)
        }
    if (vu_area (edgeNode) <= 0.0)
        return;
    int numEdge = numNode - 3;
    VuP farNode = vu_fsucc (edgeNode);
    VuP nearNode = edgeNode;
    double lambda;
    for (int i = 0; i < numEdge; i++)
        {
        farNode = vu_fsucc (farNode);
        VuP newA, newB;
        vu_join (m_graph, nearNode, farNode, &newA, &newB);
        if (nullptr != edgeSet)
            vu_markedEdgeSetTestAndAdd(edgeSet,newA);
        lambda = AssessFace (newB);
        nearNode = newA;
        }
    lambda = AssessFace (nearNode);
    }

public:

InsertAndRetriangulateContext (VuSetP graph)
    : m_graph (graph)
    {
    }

void Reset ()
    {
    m_searcher = VuPositionDetail ();
    }

void panic ()
    {
    }
VuPositionDetail SearchForEdgeOrVertex (DPoint3dCR xyz)
    {
    double distance = DBL_MAX;
    VuPositionDetail position;
    DPoint3d xyzA, xyzB, xyzC;
    double fractionC;
    VU_SET_LOOP (nodeA, m_graph)
        {
        VuP nodeB = vu_fsucc (nodeA);
        vu_getDPoint3d (&xyzA, nodeA);
        vu_getDPoint3d (&xyzB, nodeB);
        if (projectPointToLineXY (xyzC, fractionC, xyzA, xyzB, xyz))
            {
            if (fractionC > 1.0)
                xyzC = xyzB;
            else if (fractionC < 0.0)
                xyzC = xyzA;
            double distanceC = xyz.DistanceXY (xyzC);
            if (distanceC < distance)
                {
                distance = distanceC;
                if (fractionC >= 1.0)
                    position = VuPositionDetail::FromVertex (nodeB, xyzB);
                else if (fractionC < 0.0)
                    position = VuPositionDetail::FromVertex (nodeA, xyzA);
                else
                    position = VuPositionDetail::FromEdge (nodeA, xyzC, fractionC);
                }
            }
        }
    END_VU_SET_LOOP (nodeA, m_graph)
    return position;
    }

// Search the whole graph for the closest vertex or edge point.
VuPositionDetail SearchForVertex (DPoint3dCR xyz)
    {
    double distance = DBL_MAX;
    VuPositionDetail position;
    VU_SET_LOOP (nodeA, m_graph)
        {
        DPoint3d xyzA;
        vu_getDPoint3d (&xyzA, nodeA);
        double distanceA = xyz.DistanceXY (xyzA);
        if (distanceA < distance)
            {
            distance = distanceA;
            position = VuPositionDetail::FromVertex (nodeA, xyzA);
            }
        }
    END_VU_SET_LOOP (nodeA, m_graph)
    return position;
    }
void ResetSearch (DPoint3dCR xyz, int maxDim)
    {
    if (maxDim > 0)
        m_searcher = SearchForEdgeOrVertex (xyz);
    else
        m_searcher = SearchForVertex (xyz);
    }
bool InsertAndRetriangulate (DPoint3dCR xyz, bool newZWins, VuMarkedEdgeSetP edgeSet = nullptr)
    {
    m_searcher = vu_moveTo (m_graph, m_searcher, xyz);
    VuP seedNode = m_searcher.GetNode ();
    bool stat = false;
    if (m_searcher.IsFace ())
        {
        if (!vu_getMask (seedNode, VU_EXTERIOR_EDGE))
            {
            VuP newA, newB;
            vu_join (m_graph, seedNode, NULL, &newA, &newB);
            vu_setDPoint3d (newB, const_cast <DPoint3dP>(&xyz));
            RetriangulateFromBaseVertex (newB, edgeSet);
            m_searcher = VuPositionDetail::FromVertex (newB, xyz);
            }
        stat = true;
        }
    else if (m_searcher.IsEdge ())
        {
        VuP newA, newB;
        vu_splitEdge (m_graph, seedNode, &newA, &newB);
        vu_setDPoint3d (newA, const_cast <DPoint3dP>(&xyz));
        vu_setDPoint3d (newB, const_cast <DPoint3dP>(&xyz));
        RetriangulateFromBaseVertex (newA, edgeSet);
        RetriangulateFromBaseVertex (newB, edgeSet);
        m_searcher = VuPositionDetail::FromVertex (newA, xyz);
        stat = true;
        }
    else if (m_searcher.IsVertex ())
        {
        // There's already a vertex there.  Maybe the z is different.
        if (newZWins)
            vu_setDPoint3dAroundVertex (seedNode, const_cast <DPoint3dP>(&xyz));
        stat = true;
        }
    else
        {
        stat = false;
        }
    return stat;
    }
};

ptrdiff_t IntegerizeInRange (double x, ptrdiff_t x0, ptrdiff_t x1)
    {
    ptrdiff_t ix = (ptrdiff_t)x;
    if (ix < x0)
        ix = x0;
    if (ix > x1)
        ix = x1;
    return ix;
    }

struct BlockDef
{
DPoint3d m_xyz0;  // block center
DPoint3d m_xyz1;
double m_d;       // best distance
size_t m_i1;      // index of best point
BlockDef (double x, double y, double z)
    {
    m_xyz0.Init (x, y, z);
    m_xyz1.Zero ();
    m_d = DBL_MAX;
    m_i1 = SIZE_MAX;
    }
bool IsValid () { return m_i1 != SIZE_MAX;}
void Update (DPoint3dCR xyz, size_t index)
    {
    double d = m_xyz0.DistanceXY(xyz);
    if (d < m_d)
        {
        m_i1 = index;
        m_xyz1 = xyz;
        m_d = d;
        }
    }    
};
// Insert scattered points as seeds ...
static void InsertSeedPoints (InsertAndRetriangulateContext &context, bvector<DPoint3d> & points, bool newZWins, size_t numPerBlock, size_t maxblockPerSide)
    {
    size_t n = points.size ();
    if (n < numPerBlock)
        return;
    DRange3d range;
    range.Init ();
    range.Extend (&points[0], (int)n);
    DVec3d diagonal;
    diagonal.DifferenceOf (range.high, range.low);
    double d = diagonal.x + diagonal.y;
    double fx = diagonal.x / d;
    double fy = diagonal.y / d;
    static double s_minFraction = 0.0625;
    if (fx < s_minFraction)
        fx = s_minFraction;
    if (fy < s_minFraction)
        fy = s_minFraction;
    if (fx < fy)
        {
        fx = fx /fy;
        fy = 1.0;
        }
    else
        {
        fy = fy / fx;
        fx = 1.0;
        }
    // Number of blocks on long side ...
    double s = sqrt ((double)n / ((double)numPerBlock * fx * fy));
    if  (s <= 1.0)
        return;
    ptrdiff_t numX = IntegerizeInRange (s * fx, 1, maxblockPerSide);
    ptrdiff_t numY = IntegerizeInRange (s * fy, 1, maxblockPerSide);
    if (numX == 1 && numY == 1)
        return;
    //size_t numBlock = numX * numY;
    bvector<BlockDef> blocks;
    double dx = diagonal.x / (double)numX;
    double dy = diagonal.y / (double)numY;
    double x0 = range.low.x;
    double y0 = range.low.y;
    // x blocks vary fastest ...
    for (ptrdiff_t iy = 0; iy < numY+1; iy++)
        {
        for (ptrdiff_t ix = 0; ix < numX+1; ix++)
            {
            double x = x0 + (0.5 + ix) * dx;
            double y = y0 + (0.5 + iy) * dy;
            blocks.push_back (BlockDef (x, y, 0.0));
            }
        }
    for (size_t i = 0; i < n; i++)
        {
        double ax = (points[i].x - x0) / dx;
        double ay = (points[i].y - y0) / dy;
        ptrdiff_t ix = IntegerizeInRange (ax, 0, numX);
        ptrdiff_t iy = IntegerizeInRange (ay, 0, numY);
        ptrdiff_t blockIndex = ix + iy * numX;
        blocks[blockIndex].Update (points[i], i);
        }
        
    for (size_t blockIndex = 0; blockIndex < blocks.size (); blockIndex++)
        {
        if (blocks[blockIndex].IsValid ())
            context.InsertAndRetriangulate (blocks[blockIndex].m_xyz1, newZWins);
        }        
    }


// Insert points into interior faces and update triangulations.
Public void vu_insertAndRetriangulate (VuSetP pGraph, DPoint3dCP rawPoints, size_t numRaw, bool newZWins)
    {
//    static size_t s_triangulationPeriod = 20;
    InsertAndRetriangulateContext context (pGraph);
    static size_t s_maxBlockPerSide = 100;
    static size_t s_targetPerBlock = 40;
    bvector<DPoint3d> points;
    bvector<int>clusterIndices;

    DRange3d range;
    range.Init ();
    range.Extend (rawPoints, (int)numRaw);
    DVec3d diagonal;
    diagonal.DifferenceOf (range.high, range.low);
    static double s_relTol = 1.0e-6;
    double abstol = s_relTol * diagonal.MagnitudeXY ();
    bsiDPoint3dArray_findClusters (const_cast<DPoint3dP>(rawPoints), numRaw, clusterIndices, &points, abstol, false, false);
    InsertSeedPoints (context, points, newZWins, s_targetPerBlock, s_maxBlockPerSide);
    vu_flipTrianglesToImproveQuadraticAspectRatio (pGraph);   
#define continuousFlip 1     
#ifdef continuousFlip
    VuMarkedEdgeSetP edgeSetP = vu_markedEdgeSetNew(
            pGraph, VU_ALL_FIXED_EDGES_MASK );
    for (size_t i = 0; i < points.size (); i++)
        {
        context.InsertAndRetriangulate (points[i], newZWins, edgeSetP);
        vu_flipTrianglesToImproveQuadraticAspectRatio (pGraph, edgeSetP);
        }
    vu_markedEdgeSetFree(edgeSetP);
#else
    size_t n = points.size ();
    for (size_t i = 0; i < points.size (); i++)
        {
        if (context.InsertAndRetriangulate (points[i], newZWins))
            {
            }
        else
            {
            context.ResetSearch (points[i], 0);
            if (!context.InsertAndRetriangulate (points[i], newZWins))
                {
                context.ResetSearch (points[i], 1);
                context.InsertAndRetriangulate (points[i], newZWins);
                }
            }
        size_t k = i + 1;
        if (0 == (k % s_triangulationPeriod)
            || k == n)
            {
            vu_flipTrianglesToImproveQuadraticAspectRatio (pGraph);
            context.Reset ();
            }
        }
#endif
    }


Public void vu_insertAndRetriangulate (VuSetP pGraph, TaggedPolygonVectorCR allPoints, bool newZWins)
    {
    static size_t s_triangulationPeriod = 20;
    InsertAndRetriangulateContext context (pGraph);
    size_t numVector = allPoints.size ();
    size_t numSinceFlip = 0;
    for (size_t k = 0; k < numVector; k++)
        {
        bvector <DPoint3d> const  &points = allPoints[k].GetPointsCR ();
        for (size_t i = 0, n = points.size (); i < n; i++)
            {
            if (context.InsertAndRetriangulate (points[i], newZWins))
                {
                }
            else
                {
                context.ResetSearch (points[i], 0);
                if (!context.InsertAndRetriangulate (points[i], newZWins))
                    {
                    context.ResetSearch (points[i], 1);
                    context.InsertAndRetriangulate (points[i], newZWins);
                    }
                }
            numSinceFlip++;
            if (0 == (numSinceFlip % s_triangulationPeriod))
                {
                vu_flipTrianglesToImproveQuadraticAspectRatio (pGraph);
                numSinceFlip = 0;
                context.Reset ();
                }
            }        
        }
    if (numSinceFlip > 0)
        vu_flipTrianglesToImproveQuadraticAspectRatio (pGraph);
    }


Public void vu_collectInteriorFaces (VuSetP graph, TaggedPolygonVectorR polygons)
    {
    polygons.clear ();
    VuMask visitMask = vu_grabMask (graph);
    vu_clearMaskInSet (graph, visitMask);
    VU_SET_LOOP (seed, graph)
        {
        if (!vu_getMask (seed, visitMask))
            {
            vu_setMaskAroundFace (seed, visitMask);
            if (!vu_getMask (seed, VU_EXTERIOR_EDGE))
                {
                polygons.push_back (TaggedPolygon ());
                VU_FACE_LOOP (vertex, seed)
                    {
                    DPoint3d xyz;
                    vu_getDPoint3d (&xyz, vertex);
                    polygons.back ().Add (xyz);                    
                    }
                END_VU_FACE_LOOP (vertex, seed)
                }
            }
        }
    END_VU_SET_LOOP (seed, graph)
    vu_returnMask (graph, visitMask);
    }

Public void vu_collectFacesByMask (VuSetP graph, TaggedPolygonVectorR polygons, VuMask mask, bool value)
    {
    polygons.clear ();
    VuMask visitMask = vu_grabMask (graph);
    vu_clearMaskInSet (graph, visitMask);
    VU_SET_LOOP (seed, graph)
        {
        if (!vu_getMask (seed, visitMask))
            {
            vu_setMaskAroundFace (seed, visitMask);
            bool maskValue = 0 != vu_countMaskAroundFace (seed, mask);
            if (maskValue == value)
                {
                polygons.push_back (TaggedPolygon ());
                VU_FACE_LOOP (vertex, seed)
                    {
                    DPoint3d xyz;
                    vu_getDPoint3d (&xyz, vertex);
                    polygons.back ().Add (xyz);                    
                    }
                END_VU_FACE_LOOP (vertex, seed)
                }
            }
        }
    END_VU_SET_LOOP (seed, graph)
    vu_returnMask (graph, visitMask);
    }



END_BENTLEY_GEOMETRY_NAMESPACE
