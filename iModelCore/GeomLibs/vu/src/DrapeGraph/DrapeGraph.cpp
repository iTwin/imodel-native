/*--------------------------------------------------------------------------------------+
|    $RCSfile: DrapeGraph.cpp,v $
|   $Revision: 1.13 $
|       $Date: 2010/11/17 22:01:47 $
|     $Author: Earlin.Lutz $
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include    <vector>
#include    <algorithm>
#include    <Vu/VuMultiClip.h>
#include    <Vu/DrapeGraph.h>
#include    <Vu/vumaskops.fdf>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE


#define     EXPOSE_PUBLISHED_STRUCTURE
#define     EXPOSE_DIALOG_IDS
#define     EXPOSE_FDF
#define     BUFFER_SIZE         10


static int s_debug = 0;
static int s_nullPlaneIndex = -1;

static int s_show = 0;

static DRange3d vu_faceRange (VuP faceSeed)
    {
    auto range = DRange3d::NullRange ();
    VU_FACE_LOOP (currNode, faceSeed)
        {
        DPoint3d xyz;
        vu_getDPoint3d (&xyz, currNode);
        range.Extend (xyz);
        }
    END_VU_FACE_LOOP (currNode, faceSeed)
    return range;
    }

double vu_perimeter (VuP pSeed)
    {
    double perimeter = 0.0;
    VU_FACE_LOOP (pEdge, pSeed)
        {
        perimeter += vu_edgeLengthXY (pEdge);
        }
    END_VU_FACE_LOOP (pEdge, pSeed)
    return perimeter;
    }

bool IsRealSector (VuP pNode)
    {
    return pNode != vu_fsucc (vu_fsucc(pNode));
    }

void CompressDuplicatePoints (DPoint3dP xyz, int &n, double relTol, bool stripWraparound)
    {
    double absTol = relTol * bsiGeom_polylineLength (xyz, n);
    double absTol2 = absTol * absTol;
    if (n == 0)
        return;
    int n1 = 1;
    DPoint3d tailXYZ = xyz[0];
    for (int i = 1; i < n; i++)
        {
        if (xyz[i].DistanceSquared (tailXYZ) > absTol2)
            {
            xyz[n1++] = tailXYZ = xyz[i];
            }
        }
    if (stripWraparound)
        while (n1 > 1 && xyz[n1-1].DistanceSquared (xyz[0]) < absTol2)
            n1--;
    n = n1;
    }

bool stdvector_getDPoint3d (bvector<DPoint3d> *source, DPoint3d *value, int index)
    {
    if (NULL != source && index >= 0 && index < (int)source->size ())
        {
        *value = source->at (index);
        return true;
        }
    value->Zero ();
    return false;
    }

// name accessors to vu user data, always cast to int 
int GetVertexIndex (VuP node)  { return (int)vu_getUserData1 (node);}
int GetPlaneIndex (VuP node)  { return vu_getUserDataPAsInt (node);}

void SetVertexIndex (VuP node, ptrdiff_t value)  { vu_setUserData1 (node, (ptrdiff_t)value);}
void SetPlaneIndex (VuP node, size_t value)  { vu_setUserDataPAsInt (node, (int)value);}

// ASSUME: userData1 is the ONE BASED vertex index.
bool GetLocalNormal (bvector<DPoint3d> *pXYZArray, VuP pNode, DVec3dR normal)
    {
    DPoint3d xyzA0, xyzA1, xyzA2;
    VuP pNodeA0 = vu_fpred (pNode);
    VuP pNodeA1 = pNode;
    VuP pNodeA2 = vu_fsucc (pNode);
    ptrdiff_t index0  = GetVertexIndex (pNodeA0);
    ptrdiff_t  index1 = GetVertexIndex (pNodeA1);
    ptrdiff_t  index2 = GetVertexIndex (pNodeA2);
    if (index0 <= 0 || index1 <= 0 || index2 <= 0)
        return false;
    if (   stdvector_getDPoint3d (pXYZArray, &xyzA0, (int)index0 - 1)
        && stdvector_getDPoint3d (pXYZArray, &xyzA1, (int)index1 - 1)
        &&   stdvector_getDPoint3d (pXYZArray, &xyzA2, (int)index2 - 1)
        )
        {
        normal.CrossProductToPoints(xyzA1, xyzA0, xyzA2);
        return true;
        }
    return false;
    }
bool PlaneMatch (bvector<DPoint3d> *pXYZArray, VuP pNodeA, VuP pNodeB)
    {
    int indexA = GetPlaneIndex (pNodeA);
    int indexB = GetPlaneIndex (pNodeB);
    if (indexA != indexB)
        return false;
    DVec3d normalA, normalB;
    if (GetLocalNormal (pXYZArray, pNodeA, normalA) && GetLocalNormal (pXYZArray, pNodeB, normalB)
        && normalA.IsParallelTo (normalB))
        return true;
    return false;
    }



void IncrementMaskCountsFromBothSides (VuP node, VuMask mask, int &num0, int &num1)
    {
    if (vu_getMask (node, mask))
        num0++;
    if (vu_getMask (vu_edgeMate (node), mask))
        num1++;
    }
// In any edge bundle, reduce to at most one appearance of mask to capture parity change.
void RemoveCancelingParityChanges (VuSetP pGraph, VuMask mask)
    {
    VU_SET_LOOP (nodeA, pGraph)
        {
        VuP nodeB = vu_fsucc (nodeA);
        VuP nodeC = vu_vsucc (nodeB);
        if (IsRealSector (nodeA))
            {
            int numNull = 0;
            int numLeftMask = 0;
            int numRightMask = 0;
            IncrementMaskCountsFromBothSides (nodeA, mask, numLeftMask, numRightMask);
            // walk around the vertex counting null faces ...
            for (VuP nodeA1 = vu_vpred (nodeA);
                        nodeA1 != nodeA && !IsRealSector (nodeA1);
                        nodeA1 = vu_vpred (nodeA1))
                {
                numNull++;
                IncrementMaskCountsFromBothSides (nodeA1, mask, numLeftMask, numRightMask);
                }
            int parityChange = numLeftMask - numRightMask;
            if (parityChange > 2 || parityChange < -2)
                {
                // Not supposed to happen ???
                }
            else
                {
                // Clear everywhere, restore to nodeA or mate per parity.
                vu_clrMask (nodeA, mask);
                vu_clrMask (vu_edgeMate (nodeA), mask);
                int numCleared = 0;
                for (VuP nodeA1 = vu_vpred (nodeA);
                            numCleared < numNull;
                            nodeA1 = vu_vpred (nodeA1), numCleared++)
                    {
                    vu_clrMask (nodeA1, mask);
                    vu_clrMask (vu_edgeMate (nodeA1), mask);
                    }
                if (parityChange == 1)
                    vu_setMask (nodeA, mask);
                else if (parityChange == -1)
                    vu_setMask (nodeC, mask);
                }
            }
        }
    END_VU_SET_LOOP (nodeA, pGraph)
    }



bool GetAdjacentRealSectorAroundVertex (VuP pStart, VuP &pNext, VuMask insideSkipMask, VuMask outsideSkipMask)
    {
    pNext = NULL;
    if (!IsRealSector (pStart) || vu_getMask (pStart, insideSkipMask))
        return false;
    VuP pFirst = vu_vsucc (pStart);
    VU_VERTEX_LOOP (pCurr, pFirst)
        {
        if (IsRealSector (pCurr))
            {
            pNext = pCurr;
            return pNext != pStart && !vu_getMask (pNext, outsideSkipMask);
            }
        }
    END_VU_VERTEX_LOOP (pCurr, pFirst)
    return false;
    }

struct StatVector : bvector<int>
{
void RecordHit (size_t i)
    {
    while (i >= size ())
        push_back (0);
    at(i) ++;
    }

void RecordLogHit (double value)
    {
    if (value <= 1.0)
        RecordHit (0);
    else
        {
        double log = floor (log10 (value));
        int    iLog = (int)log;
        RecordHit ((size_t) iLog);
        }
    }

void Print (char const *title)
    {
    GEOMAPI_PRINTF ("%s\n", title);
    for (size_t i = 0; i < size (); i++)
        if (at(i) != 0)
            GEOMAPI_PRINTF ("   (%d count %d)\n", (int)i, at(i));
    }
};

void ReviewGraph (VuSetP pGraph)
    {
    VuMask visitMask = vu_grabMask (pGraph);
    vu_clearMaskInSet (pGraph, visitMask);
    size_t numRealInteriorSectors = 0;
    size_t numNode = 0;
    size_t numExterior = 0;
    size_t numSkipAllBundles = 0;
    size_t numExteriorBundle = 0;
    size_t numInteriorBundle = 0;
    size_t numExteriorMaskErrors = 0;
    size_t numNullSectors = 0;
    StatVector counters;
    VU_SET_LOOP (pEdgeBase, pGraph)
        {
        numNode++;
        if (vu_getMask (pEdgeBase, VU_EXTERIOR_EDGE))
            {
            numExterior++;
            if (!vu_getMask (vu_fsucc (pEdgeBase), VU_EXTERIOR_EDGE))
                numExteriorMaskErrors++;
            }

        if (!IsRealSector (pEdgeBase))
            numNullSectors++;

        if (vu_getMask (pEdgeBase, visitMask))
            {
            }
        else if (vu_getMask (pEdgeBase, VU_EXTERIOR_EDGE))
            {
            }
        else if (IsRealSector(pEdgeBase))
            {
            numRealInteriorSectors++;
            size_t numSkipThisBundle = 0;
            VuP pCurr = pEdgeBase;
            for (;;)
                {
                VuP pOtherSide = vu_fsucc (vu_vsucc (pCurr));
                vu_setMask (pCurr, visitMask);
                vu_setMask (pOtherSide, visitMask);
                pCurr = vu_vsucc (pCurr);
                if (IsRealSector (pCurr))
                    break;
                numSkipThisBundle++;
                numSkipAllBundles++;
                }

            if (vu_getMask (pCurr, VU_EXTERIOR_EDGE))
                numExteriorBundle++;
            else
                numInteriorBundle++;

            counters.RecordHit (numSkipThisBundle);
            }
        }
    END_VU_SET_LOOP (pEdgeBase, pGraph)
    if (s_debug > 1)
        {
        GEOMAPI_PRINTF ("\nNULL FACE STATE\n");
        GEOMAPI_PRINTF ("    (numNode %d) (numExteriorNode %d) (numNullSectors %d)\n", (int)numNode, (int)numExterior, (int)numNullSectors);
        GEOMAPI_PRINTF ("    (numExteriorMaskErrors %d)\n", (int)numExteriorMaskErrors);
        GEOMAPI_PRINTF ("    (numExteriorBundle %d)\n", (int)numExteriorBundle);
        GEOMAPI_PRINTF ("    (numInteriorBundle %d)\n", (int)numInteriorBundle);
        GEOMAPI_PRINTF ("    (numSkipAllBundles %d)\n",
                            (int)numSkipAllBundles);
        counters.Print ("Bundle Markup");
        }
    vu_returnMask (pGraph, visitMask);
    }


struct AreaSortData
    {
    double m_areaFactor;
    VuP    m_pFaceSeed;
    size_t m_planeIndex;
    AreaSortData (double area, double perimeter, double refArea, VuP pNode, size_t planeIndex)
        {
        m_areaFactor = area * area / (perimeter * perimeter * refArea);
        m_pFaceSeed = pNode;
        m_planeIndex = planeIndex;
        }

    static bool cbReverseSortComparison (
        AreaSortData const &dataA,
        AreaSortData const &dataB
        )
        {
        return dataA.m_areaFactor > dataB.m_areaFactor;
        }

    };


void ApplyTransitionMaskBump (VuP seedEdge, VuMask mask)
    {
    if (vu_getMask (seedEdge, mask))
        {
        VU_FACE_LOOP (currEdge, seedEdge)
            {
            if (vu_getMask (currEdge, mask))
                {
                vu_clrMask (currEdge, mask);
                }
            else
                {
                vu_setMask (vu_edgeMate (currEdge), mask);
                }
            }
        END_VU_FACE_LOOP (currEdge, seedEdge)
        }
    }

// Test that this is a valid edge for transition bump.
// If seedEdge is unmarked it is always ok.
// If seed edge is marked, other (edge,mate) cases are:
//    (0,1) ERROR
//    (1,any) OK -- inside 1 will change to 0 on real mark.
//    (0,0) OK -- outside 0 will change to 1
bool IsValidateForTransitionMaskBump (VuP seedEdge, VuMask mask)
    {
    if (vu_getMask (seedEdge, mask))
        {
        VU_FACE_LOOP (currEdge, seedEdge)
            {
            if (!vu_getMask (currEdge, mask)
                && vu_getMask (vu_edgeMate (currEdge), mask))
                return false;
            }
        END_VU_FACE_LOOP (currEdge, seedEdge)
        }
    return true;
    }


// Recursive search and mark "across edges".
// Apply plane index to all nodes around faces reached by path always having the mask.  Clear the mask along the way.
// Search starts on the other side of the seedNode's edge.
size_t SpreadPlaneIndex (VuP seedNode, VuMask activeFaceMask,
size_t planeIndex,
bvector<VuP> &stack,
PlaneArray &planeArray
)
    {
    static bool s_ignoreNullFaces = true;
    stack.clear ();
    stack.push_back (vu_edgeMate (seedNode));
    size_t numSpread = 0;
    while (stack.size () > 0)
        {
        VuP currFace = stack.back ();
        int numThisFace = vu_countEdgesAroundFace (currFace);
        double area = vu_area (currFace);
        stack.pop_back();
        if (numThisFace == 2 && s_ignoreNullFaces)
            continue;
        if (vu_getMask (currFace, activeFaceMask)
            && IsValidateForTransitionMaskBump (currFace, DrapeGraph::s_PRIMARY_EDGE_MASK))
            {
            size_t newPlaneIndex = planeArray.AddDuplicate (planeIndex);
            ApplyTransitionMaskBump (currFace, DrapeGraph::s_PRIMARY_EDGE_MASK);
            if (s_debug > 0)
                GEOMAPI_PRINTF ("    (PlaneSpread id %d area %le)\n", vu_getIndex (currFace), area);
            VU_FACE_LOOP (currNode, currFace)
                {
                vu_clrMask (currNode, activeFaceMask);
                SetPlaneIndex (currNode, newPlaneIndex);
                numSpread++;
                VuP mate = vu_edgeMate (currNode);
                if (vu_getMask (mate, activeFaceMask))
                    stack.push_back (mate);
                }
            END_VU_FACE_LOOP (currNode, currFace)
            }
        }
    return numSpread;
    }

// Can only return null at a pair of vertices with a joining edge (or edges) and no other connections?
static VuP AdvanceToRealSectorAroundVertex (VuP startNode, int numSkip)
    {
    int n = 0;
    VU_VERTEX_LOOP (currNode, startNode)
        {
        if (n >= numSkip && IsRealSector (currNode))
            return currNode;
        n++;
        }
    END_VU_VERTEX_LOOP (currNode, startNode)
    return NULL;
    }

int CompressShortEdges (VuSetP pGraph, double abstol)
    {
    int numCompress = 0;
    int numStrangeQuark  = 0;
    VuMask deleteMask = vu_grabMask (pGraph);
    vu_clearMaskInSet (pGraph, deleteMask);
    VU_SET_LOOP (baseNode, pGraph)
        {
        if (!vu_getMask (baseNode, deleteMask))
            {
            double a = vu_edgeLengthXY (baseNode);
            // A0, A1, B0, B1 are 4 real sectors
            // A1 is vertex successor of A0, skipping null faces
            // B1 is vertex successor of B0, skipping null faces.
            // A0 = vu_fsucc (B1)
            // B0 = vu_fsucc (A0)
            //    B1           A0
            //    Bb===========Aa
            //    B0           A1
            if (a <= abstol)
                {
                VuP mateNode = vu_edgeMate (baseNode);
                VuP nodeA1 = AdvanceToRealSectorAroundVertex (baseNode, 0);
                VuP nodeB1 = AdvanceToRealSectorAroundVertex (mateNode, 0);
                if (NULL != nodeA1 && NULL != nodeB1)   // NULL can only happen on pair of vertices connecting only to each other.
                                                        // This is strange but plausible.
                    {
                    VuP nodeA0 = vu_fsucc (nodeB1);
                    VuP nodeB0 = vu_fsucc (nodeA1);
                    VuP nodeA1Check = AdvanceToRealSectorAroundVertex (nodeA0, 1);
                    VuP nodeB1Check = AdvanceToRealSectorAroundVertex (nodeB0, 1);
                    if (nodeA1Check != nodeA1 || nodeB1Check != nodeB1)
                        {
                        numStrangeQuark++;
                        }
                    else
                        {
                        // Yank the bundle out at each end ...
                        vu_vertexTwist (pGraph, nodeA0, nodeA1);
                        vu_vertexTwist (pGraph, nodeB0, nodeB1);
                        // A0, B0 are the exposed parts of the rest of the graph.
                        // Rejoin the exposed parts ...
                        vu_vertexTwist (pGraph, nodeA0, nodeB0);
                        // A1, B1 are opposite ends of the bundle
                        // Mark the bundle for deletion ...
                        vu_setMaskAroundVertex (nodeA1, deleteMask);
                        vu_setMaskAroundVertex (nodeB1, deleteMask);
                        // hm... But where is it really now?
                        double xx = vu_getX (nodeA0);
                        double yy = vu_getY (nodeA0);
                        VU_VERTEX_LOOP (node, nodeA0)
                            {
                            vu_setX (node, xx);
                            vu_setY (node, yy);
                            }
                        END_VU_VERTEX_LOOP (node, nodeA0)
                        }
                    numCompress++;
                    }
                }
            }
        }
    END_VU_SET_LOOP (baseNode, pGraph)
    vu_freeMarkedEdges (pGraph, deleteMask);
    vu_returnMask (pGraph, deleteMask);
    return numCompress;
    }
void CollapseSmallFeatures (VuSetP pGraph, double shortEdgeTol, double smallAreaTol, double triangleHeightTol)
    {
    CompressShortEdges (pGraph, shortEdgeTol);
    }


// Apply the barrierMask to all edges in graph.
// REMOVE it from edges that can be crossed in intra-plane scans.
// An intra-plane edge is:
//  * neither side is exterior.
//  * vertex continuity on each end.
// @param [inout] bundleArray receives blocks of node pointers for each annotated edge bundle.
//     bundles are separated by NULL pointers.
void AnnotateIntraplaneEdges
(
VuSetP pGraph,
bvector<DPoint3d> *pXYZArray,
VuMask barrierMask,
bvector<VuP> &bundleArray
)
    {
    VuMask visitMask = vu_grabMask (pGraph);
    vu_clearMaskInSet (pGraph, visitMask);
    vu_setMaskInSet (pGraph, barrierMask);
    VuDebug::ShowVertices (pGraph, "BEFORE BARRIER ANNOTATE");
    VU_SET_LOOP (pNodeA0, pGraph)
        {
        VuDebug::ShowFace (pGraph, pNodeA0, "IntraplaneBase", true, true);
        if (vu_getMask (pNodeA0, visitMask))
            {
            }
        else if (vu_getMask (pNodeA0, VU_EXTERIOR_EDGE))
            {
            }
        else if (IsRealSector(pNodeA0))
            {
            // Simple scenario:
            // A0          B0
            // +-----------+
            // A1          B1
            // Real scenario:
            // A0          B0
            // +-----------+
            // +-----------+
            // +-----------+
            // A1          B1
            //  (With null faces inside the bundle)
            VuP pNodeB0 = vu_fsucc (pNodeA0);
            VuP pNodeB1 = vu_vsucc (pNodeB0);
            size_t numEdgeInBundle = 1;
            while (!IsRealSector (pNodeB1))
                {
                pNodeB1 = vu_vsucc (pNodeB1);
                numEdgeInBundle++;
                }
            VuP pNodeA1 = vu_fsucc (pNodeB1);

            if (vu_getMask (pNodeB1, VU_EXTERIOR_EDGE))
                {
                }
            else if (   PlaneMatch (pXYZArray, pNodeA0, pNodeA1)
                     && PlaneMatch (pXYZArray, pNodeB0, pNodeB1)
                    )
                {
                VuDebug::ShowVertex (pGraph, pNodeA0, "A0 BEFORE PURGE");
                VuDebug::ShowVertex (pGraph, pNodeB1, "B1 BEFORE PURGE");
                VuP pMark = pNodeA0;
                // WE want to delete this edge.  Bridger may not.
                // Make sure that all the interior nodes get a vertex index from a real node.
                // The plane match test says A0,A1 are same index, B0,B1 are same index...
                int vertexIndexA = GetVertexIndex (pNodeA0);
                int vertexIndexB = GetVertexIndex (pNodeB0);
                VuP pBundleNodeA = pNodeA1;
                for (size_t i = 1; i < numEdgeInBundle; i++)
                    {
                    pBundleNodeA = vu_vsucc (pBundleNodeA);
                    SetVertexIndex (pBundleNodeA, vertexIndexA);
                    SetVertexIndex (vu_fsucc (pBundleNodeA), vertexIndexB);
                    }

                for (size_t i = 0; i < numEdgeInBundle; i++)
                    {
                    VuP pMate = vu_edgeMate (pMark);
                    bundleArray.push_back (pMark);
                    vu_clrMask (pMark, barrierMask);
                    vu_clrMask (pMate, barrierMask);

                    vu_setMask (pMark, visitMask);
                    vu_setMask (pMate, visitMask);

                    pMark = vu_vpred (pMark);                  
                    }
                bundleArray.push_back (NULL);
                VuDebug::ShowVertex (pGraph, pNodeA0, "A0 AFTER PURGE");
                VuDebug::ShowVertex (pGraph, pNodeB1, "B1 AFTER PURGE");
                }
            }
        else
            {
            }
        }
    END_VU_SET_LOOP (pNodeA0, pGraph)
    vu_returnMask (pGraph, visitMask);
    VuDebug::ShowVertices (pGraph, "AFTER BARRIER ANNOTATE");
    }

void IncrementMaskCounts
(
VuP pNode,
VuMask mask0,
size_t &numMask0,
VuMask mask1,
size_t &numMask1,
VuMask mask2,
size_t &numMask2
)
    {
    if (vu_getMask (pNode, mask0))
        numMask0++;
    if (vu_getMask (pNode, mask1))
        numMask1++;
    if (vu_getMask (pNode, mask2))
        numMask2++;
    }

// Input a sequence of "A" nodes (as per figure above)
// ASSUME all nodes have good vertex indices.
// ASSUME caller approves ripping extras out.
// Mark all except the first for deletion.
// Clear indicated masks.
// Reapply exterior on each side if successors on that side are exterior.
void SimplifyBundle (bvector<VuP> &bundleArray, size_t i0, size_t numEdge,
    VuMask clearMasks, VuMask deleteMask)
    {
    if (numEdge < 2)
        return;

    for (size_t i = i0; i < i0 + numEdge; i++)
        {
        VuP pA = bundleArray[i];       // i?
        VuP pB = vu_edgeMate (pA);
        if (i > i0)
            {
            vu_setMask (pA, deleteMask);
            vu_setMask (pB, deleteMask);
            }
        vu_clrMask (pA, clearMasks);
        vu_clrMask (pB, clearMasks);
        }

    VuP pA0 = bundleArray[i0];
    VuP pB1 = vu_edgeMate (bundleArray[i0 + numEdge - 1]);

    if (vu_getMask (vu_fsucc (pA0), VU_EXTERIOR_EDGE))
        vu_setMask (pA0, VU_EXTERIOR_EDGE);
    if (vu_getMask (vu_fsucc (pB1), VU_EXTERIOR_EDGE))
        vu_setMask (vu_edgeMate (pA0), VU_EXTERIOR_EDGE);     // ?? vu_edgeMate(pA0)
    }

bool IsOdd (size_t a) {return (a & 0x01) != 0;}
void CollapseBundles
(
VuSetP pGraph,
bvector<VuP> &bundleArray
)
    {
    size_t numBundleNode = bundleArray.size ();
    size_t numExterior = 0;
    size_t numRule = 0;
    size_t numBoundary = 0;
    size_t numEdgeInBundle = 0;
    size_t numValid = 0;
    size_t numInvalid = 0;
    StatVector stats;
    VuMask deleteMask = vu_grabMask (pGraph);
    vu_clearMaskInSet (pGraph, deleteMask);
    VuMask clearMasks = VU_EXTERIOR_EDGE | VU_BOUNDARY_EDGE | VU_RULE_EDGE;
    for (size_t i = 0; i < numBundleNode; i++)
        {
        VuP pNode = bundleArray[i];
        if (pNode == NULL)
            {
            if (IsOdd (numBoundary) || IsOdd (numRule) || IsOdd (numExterior))
                {
                numInvalid++;
                }
            else
                {
                numValid++;
                SimplifyBundle (bundleArray, i - numEdgeInBundle, numEdgeInBundle, clearMasks, deleteMask);
                }
            stats.RecordHit (numEdgeInBundle);
            numBoundary = numRule = numExterior = numEdgeInBundle = 0;
            }
        else
            {
            VuP pMate = vu_edgeMate (pNode);
            numEdgeInBundle++;
            IncrementMaskCounts (pNode, VU_EXTERIOR_EDGE, numExterior, VU_BOUNDARY_EDGE, numBoundary, VU_RULE_EDGE, numRule);
            IncrementMaskCounts (pMate, VU_EXTERIOR_EDGE, numExterior, VU_BOUNDARY_EDGE, numBoundary, VU_RULE_EDGE, numRule);
            }
        }
    vu_freeMarkedEdges (pGraph, deleteMask);
    vu_returnMask (pGraph, deleteMask);
    if (s_debug > 1)
        {
        GEOMAPI_PRINTF (" Bundle Analysis (ArraySize %d) (Valid %d) (Invalid %d)\n",
                        (int)numBundleNode, (int)numValid, (int)numInvalid);
        stats.Print ("Bundle Review");
        }
    }



PlaneData::PlaneData (VuP node00, double ax, double ay, double z00, ptrdiff_t tag, double zMin, double zMax)
    {
    m_ax = ax;
    m_ay = ay;
    m_z00 = z00;
    m_node00 = node00;
    m_mask = 0;
    m_tag = tag;
    m_zMin = zMin;
    m_zMax = zMax;
    }

void PlaneData::SetZLimits (double minZ, double maxZ)
    {
    m_zMin = minZ;
    m_zMax = maxZ;
    }

double PlaneData::EvaluateZ (double x, double y) const
    {
    return m_z00 + m_ax * x + m_ay * y;
    }

double PlaneData::EvaluateRestrictedZ (double x, double y) const
    {
    double z = EvaluateZ (x, y);
    if (z > m_zMax)
        z = m_zMax;
    if (z < m_zMin)
        z = m_zMin;
    return z;
    }

double PlaneData::SumSquaredCoffs () const
    {
    return m_ax * m_ax + m_ay * m_ay;
    }

ptrdiff_t PlaneData::GetTag () const {return m_tag;}

void PlaneData::SetMask (unsigned int mask)
    {
    m_mask |= mask;
    }
void PlaneData::ClearMask (unsigned int mask)
    {
    m_mask &= ~mask;
    }
bool PlaneData::IsMaskSet (unsigned int mask)
    {
    return (mask & m_mask) != 0;
    }

static bool IsVectorInXYPlane (DVec3dR normal)
    {
    // Hm... we're going to blow away near vertial walls.
    static double s_normalizedCrossProductTolerance1 = 1.0e-7;
    static double s_normalizedCrossProductTolerance2 = 1.0e-4;
    double axy = sqrt (normal.x * normal.x + normal.y * normal.y);
    // Waste a test -- doesn't affect the result, but makes it easy to
    //  look for borderline cases in debugger.
    if (fabs (normal.z) <= s_normalizedCrossProductTolerance1 * axy)
        return true;
    if (fabs (normal.z) <= s_normalizedCrossProductTolerance2 * axy)
        return true;
    return false;
    }


ptrdiff_t PlaneArray::GetTag (size_t index)
    {
    if (IsActiveIndex (index))
        return at(index).GetTag ();
    return PlaneData::DefaultTag;
    }
bool PlaneArray::IsActiveIndex (size_t index)
    {
    static int s_suppressDanglingEdges = false;
    // index 0 is suppressed --
    if (s_suppressDanglingEdges)
        return index > 0 && index < size ()
            && !at(index).IsMaskSet (PlaneData::DanglingEdge);
    else
        return index > 0 && index < size ();
    }

double PlaneArray::EvaluateZ (size_t index, double x, double y, double defaultZ, bool &ok)
    {
    ok = true;
    if (IsActiveIndex (index))
        return at(index).EvaluateZ (x, y);
    ok = false;
    return defaultZ;
    }

double PlaneArray::EvaluateRestrictedZ (size_t index, double x, double y, double defaultZ, bool &ok)
    {
    ok = true;
    if (IsActiveIndex (index))
        return at(index).EvaluateRestrictedZ (x, y);
    ok = false;
    return defaultZ;
    }


void PlaneArray::SetMask (size_t index, unsigned int mask)
    {
    if (index < size ())
        at (index).SetMask (mask);
    }

bool PlaneArray::AddByOriginAndNormal
(
DPoint3dCR origin,
DVec3dCR normal,
ptrdiff_t tag,
double zMin,
double zMax,
size_t &index
)
    {
    index = size ();
    double a;
    if (!bsiTrig_safeDivide (&a, 1.0, normal.z, 1.0))
        return false;
    push_back (PlaneData (NULL, -normal.x * a, -normal.y * a, origin.DotProduct (normal) * a, tag, zMin, zMax));
    return true;
    }
size_t PlaneArray::AddDuplicate (size_t i)
    {
    if (i >= size ())
        return s_nullPlaneIndex;
    size_t index = size ();
    PlaneData oldData = at (i);
    push_back (oldData);
    return index;
    }

static size_t CountPlaneMatchAtVertex (VuP pStart, int data)
    {
    int count = 0;
    VU_VERTEX_LOOP (pCurr, pStart)
        {
        if (GetPlaneIndex (pCurr) == data)
            count++;
        }
    END_VU_VERTEX_LOOP (pCurr, pStart)
    return count;
    }


DrapeGraph::DrapeGraph ()
        {
        m_numVertical = 0;
        m_maxPlanes = 0;

        m_pGraph = vu_newVuSet (0);
        vu_setUserDataPIsEdgeProperty (m_pGraph, true);
        vu_setUserDataPIsVertexProperty (m_pGraph, false);
        vu_setDefaultUserDataPAsInt (m_pGraph, -1);
        vu_setDefaultUserData1 (m_pGraph, -1, true, false);
        // UGH -- default userDataP is not taking.  Force plane 0 to be unused by the graph.
        PlaneData defaultPlane (NULL, 0,0,0, -1, -DBL_MAX, DBL_MAX);
        m_indexedPlanes.push_back (defaultPlane);

        static double   s_graphAbsTol = 0.0;
        static double   s_graphRelTol = 1.0e-9;
        vu_setTol (m_pGraph, s_graphAbsTol, s_graphRelTol);
        m_maxZFringe = 0.0;
        }

void DrapeGraph::ExpandZRange (DRange3dR range)
    {
    if (m_maxZFringe > 0.0)
        {
        range.low.z -= m_maxZFringe;
        range.high.z += m_maxZFringe;
        }
    }

void DrapeGraph::SetMaxZShift (double z)
    {
    m_maxZFringe = z;
    }

DrapeGraph::~DrapeGraph ()
    {
    vu_freeVuSet (m_pGraph);
    }

bool DrapeGraph::AddPlaneByOriginAndNormal
(
DPoint3dCR origin,
DVec3dCR normal,
ptrdiff_t tag,
double zMin,
double zMax,
size_t &index
)
    {
    return m_indexedPlanes.AddByOriginAndNormal (origin, normal, tag, zMin, zMax, index);
    }

// Mask usage:
// 1) both sides of original edges are VU_BOUNDARY_EDGE
// 2) outside (simple right by area sign) are outsideMask
// 3) VU_EXTERIOR appears by later analysis (???)
bool DrapeGraph::AddPolygon(DPoint3dP xyz, int n, ptrdiff_t tag, VuMask outsideMask)
    {
    static double s_planarityTolerance = 1e-8;
    static double s_simpleTriangulationTol = 0.3;
    static double s_duplicatePointRelTol = 1e-4;
    static double s_normalZTol = 0.1;
    double area, perimeter, thickness;
    DPoint3d centroid;
    DVec3d normal;
    CompressDuplicatePoints (xyz, n, s_duplicatePointRelTol, true);

    DRange3d polygonRange;
    polygonRange.Init ();
    polygonRange.Extend (xyz, n);
    ExpandZRange (polygonRange);
    
    if (bsiPolygon_centroidAreaPerimeter (xyz, n, &centroid, &normal, &area, &perimeter, &thickness))
        {
        // Would you believe...
        // Observed case n = 3, thickness > 1e-8 of perimeter.
        // Huh? It's a triangle.   (At 12 digit uor!!!)
        if (n == 3 || fabs (thickness) < s_planarityTolerance * perimeter)
            {
            size_t planeIndex;
            if (IsVectorInXYPlane (normal)
                || !AddPlaneByOriginAndNormal (xyz[0], normal, tag, polygonRange.low.z, polygonRange.high.z, planeIndex))
                {
                // It's a vertical wall. Throw it out, ignore it, trash it burn it.  There will be consequences, but not soon.
                return true;
                }
            else
                {
                // UMMM.. we haven't rule out self intersecting...
                // Need to push/pop the graph for fixup?
                if (normal.z < 0)
                    bsiDPoint3d_reverseArrayInPlace (xyz, n);
                VuP pCurr = NULL;
                VuP pLeft, pRight;
                VuMask rightMask = outsideMask | VU_BOUNDARY_EDGE;
                VuMask leftMask  = VU_BOUNDARY_EDGE;
                for (int i = 0; i < n; i++)
                    {
                    vu_splitEdge (m_pGraph, pCurr, &pLeft, &pRight);
                    pCurr = pLeft;
                    vu_setMask (pLeft, leftMask);
                    vu_setMask (pRight, rightMask);
                    SetPlaneIndex (pLeft, planeIndex);
                    SetPlaneIndex (pRight, s_nullPlaneIndex);
                    vu_setDPoint3d (pLeft, &xyz[i]);
                    vu_setDPoint3d (pRight, &xyz[i]);
                    }
                return true;
                }
            }
        else if (  fabs (normal.z) > s_normalZTol
                && thickness < s_simpleTriangulationTol * perimeter
                && bsiGeom_testXYPolygonConvex (xyz, n)
                )
            {
            // Non planar polygon, but it's not to wild, so just make the obvious triangles.
            DPoint3d trianglePoints[3];
            for (int i = 1; i < n - 1; i++)
                {
                trianglePoints[0] = xyz[0];
                trianglePoints[1] = xyz[i];
                trianglePoints[2] = xyz[i+1];
                AddPolygon (trianglePoints, 3, tag, outsideMask);
                }
            return true;
            }
        }
    return false;
    }

void DrapeGraph::AddPolygon (TaggedPolygon &polygon, ptrdiff_t tag, VuMask outsideMask)
    {
    AddPolygon (polygon.GetPointsR (), tag, outsideMask);
    }
// Polygon points may be compressed in place !!!
void DrapeGraph::AddPolygon (bvector<DPoint3d> &xyz, ptrdiff_t tag, VuMask outsideMask)
    {
    static int s_numVertical = 0;
    static int s_numAspect1 = 0;
    static int s_numAspect2 = 0;
    if (xyz.size () > 3)
        {
        AddPolygon (&xyz[0], (int)xyz.size (), tag, outsideMask);
        }
    else if (xyz.size () == 3)
        {
        DVec3d vectorAB, vectorAC;
        DPoint3d xyzA = xyz[0];
        vectorAB.DifferenceOf (xyz[1], xyz[0]);
        vectorAC.DifferenceOf (xyz[2], xyz[0]);
        DRange3d polygonRange;
        polygonRange.Init ();
        polygonRange.Extend (&xyz[0], 3);
        ExpandZRange (polygonRange);
        DVec3d normal;
        double area = 0.5 * normal.NormalizedCrossProduct (vectorAB, vectorAC);
        double lengthAB = vectorAB.MagnitudeXY ();
        double lengthAC = vectorAC.MagnitudeXY ();
        double lengthBC = xyz[1].DistanceXY (xyz[2]);
        double p = lengthAB + lengthBC + lengthAC;
        double mu;
        static double s_mediumMu = 1.0e4;
        static double s_bigMu    = 1.0e8;
        static double s_normalZTrigger = 0.1;
        //  ratio of area to perimiter squared is a measure of sliver.
        // For a circle, area/p^2 = 4pi, around 12.5
        // For square, area/p^2 = 16
        bsiTrig_safeDivide (&mu, p * p, area, DBL_MAX);
        if (IsVectorInXYPlane (normal))
            {
            // Vertical panel
            s_numVertical++;
            }
        else if (mu >= s_mediumMu && fabs(normal.z) < s_normalZTrigger)
            {
            // Pretty steep and bad aspect ratio.
            s_numAspect1++;
            }
        else if (mu >= s_bigMu)
            {
            s_numAspect2++;
            }
        else
            {

            bool reverse = normal.z < 0.0;
            int iB = 1, iC = 2;
            if (reverse)
                {
                iB = 2;
                iC = 1;
                }

            double ax, ay;
            if (bsiSVD_solve2x2 (&ax, &ay,
                            vectorAB.x, vectorAB.y,
                            vectorAC.x, vectorAC.y,
                            vectorAB.z,
                            vectorAC.z))
                {
                size_t planeIndex = m_indexedPlanes.size ();
                VuP nodeALeft, nodeARight;
                VuP nodeBLeft, nodeBRight;
                VuP nodeCLeft, nodeCRight;

                vu_splitEdge (m_pGraph, NULL, &nodeALeft, &nodeARight);
                vu_splitEdge (m_pGraph, nodeALeft, &nodeBLeft, &nodeBRight);
                vu_splitEdge (m_pGraph, nodeBLeft, &nodeCLeft, &nodeCRight);

                // (The split sequence applied VU_BOUNDARY_EDGE, we'll repeat it...)
                vu_setMaskAroundFace (nodeALeft,  VU_BOUNDARY_EDGE);
                vu_setMaskAroundFace (nodeARight, VU_BOUNDARY_EDGE);
                if (0 != outsideMask)
                    vu_setMaskAroundFace (nodeARight, outsideMask);

                vu_setDPoint3dAroundVertex (nodeALeft, const_cast <DPoint3d*>(&xyz[0]));
                vu_setDPoint3dAroundVertex (nodeBLeft, const_cast <DPoint3d*>(&xyz[iB]));
                vu_setDPoint3dAroundVertex (nodeCLeft, const_cast <DPoint3d*>(&xyz[iC]));

                SetPlaneIndex (nodeALeft, planeIndex);
                SetPlaneIndex (nodeBLeft, planeIndex);
                SetPlaneIndex (nodeCLeft, planeIndex);

                SetPlaneIndex (nodeARight, s_nullPlaneIndex);
                SetPlaneIndex (nodeBRight, s_nullPlaneIndex);
                SetPlaneIndex (nodeCRight, s_nullPlaneIndex);

                PlaneData plane (nodeALeft, ax, ay, xyzA.z - ax * xyzA.x - ay * xyzA.y, tag, polygonRange.low.z, polygonRange.high.z);
                m_indexedPlanes.push_back (plane);
                }
            }
        }
    }

void DrapeGraph::AddPolygons (TaggedPolygonVectorR polygons, VuMask outsideMaskA0, VuMask outsideMaskA1, double verticalShiftFraction)
    {
    DRange3d range = PolygonVectorOps::GetRange (polygons);
    double verticalShift = (range.high.z - range.low.z) * verticalShiftFraction;
    SetMaxZShift (verticalShift);
#define DEBUG_ALLOW_SINGLE_POLYGON_EXCLUSION
#ifdef DEBUG_ALLOW_SINGLE_POLYGON_EXCLUSION
    static ptrdiff_t s_excludedPolygonIndex = -1;
    for (size_t i = 0; i < polygons.size (); i++)
        if (i != (size_t)s_excludedPolygonIndex)
            AddPolygon (polygons[i], i,
                    polygons[i].GetIndexA () == 0 ? outsideMaskA0 : outsideMaskA1);
#else
    for (size_t i = 0; i < polygons.size (); i++)
        AddPolygon (polygons[i], i,
                    polygons[i].GetIndexA () == 0 ? outsideMaskA0 : outsideMaskA1);
#endif
    }

// Cluster xy coordinates.
void DrapeGraph::ClusterXY (double tol)
    {
    if (tol > 0.0)
        vu_consolidateClusterCoordinates (m_pGraph, tol);
    }


// The edges associated with a plane must be a closed loop.
// After merge, every edge marked with the plane should lead to a vertex with exactly one outgoing edge of the same plane.
void DrapeGraph::MarkParityErrors ()
    {
    int numError = 0;
    int numPlane = (int) m_indexedPlanes.size ();
    VU_SET_LOOP(pCurr,m_pGraph)
        {
        int intIndex = GetPlaneIndex (pCurr);
        if (intIndex > 0 && intIndex < numPlane)
            {
            size_t index = (size_t)intIndex;
            if (m_indexedPlanes.IsActiveIndex (index))
                {
                if (   CountPlaneMatchAtVertex (pCurr, intIndex) != 1
                    || CountPlaneMatchAtVertex (vu_fsucc(pCurr), intIndex) != 1)
                    {
                    m_indexedPlanes.SetMask (index, PlaneData::DanglingEdge);
                    numError++;
                    }                
                }
            }
        }
    END_VU_SET_LOOP (pCurr, m_pGraph)
    if (s_debug > 0)
        GEOMAPI_PRINTF (" (PARITY ERRORS %d\n", numError);
    }



//
// At each node around a face, set z to the largest z of
//      (a) the default and
//      (b) the z of any active plane at the node's xy.
//
void DrapeGraph::SetZAroundFace (VuP pFaceSeed, bvector<int> &activePlaneIndex, double defaultZ, double maxZ)
    {
    VU_FACE_LOOP (pCurr, pFaceSeed)
        {
        double z = defaultZ;
        DPoint3d xyz;
        vu_getDPoint3d (&xyz, pCurr);
        ptrdiff_t sourceTag = PlaneData::DefaultTag;
        int numHit = 0;
        for (size_t i = 0; i < activePlaneIndex.size (); i++)
            {
            bool ok;
            double zi = m_indexedPlanes.EvaluateZ (activePlaneIndex[i], xyz.x, xyz.y, defaultZ, ok);
            if (zi > z || (numHit == 0 && ok == true))
                {
                numHit++;
                z = zi;
                sourceTag = m_indexedPlanes.GetTag (activePlaneIndex[i]);
                }
            if (z > maxZ)
                z = maxZ;
            }
        xyz.z = z;
        vu_setDPoint3d (pCurr, &xyz);
        
        }
    END_VU_FACE_LOOP (pCurr, pFaceSeed)
    }

void DrapeGraph::SetZAll(double z)
    {
    VU_SET_LOOP (pCurr, m_pGraph)
        {
        DPoint3d xyz;
        vu_getDPoint3d (&xyz, pCurr);
        xyz.z = z;
        vu_setDPoint3d (pCurr, &xyz);
        }
    END_VU_SET_LOOP (pCurr, m_pGraph)
    }

static void PushFaceToSeedStack (VuMask visitMask, VuP pFaceSeed, bvector<VuP> &seedStack)
    {
    vu_setMaskAroundFace (pFaceSeed, visitMask);
    VU_FACE_LOOP (pCurr, pFaceSeed)
        {
        seedStack.push_back (pCurr);
        }
    END_VU_FACE_LOOP (pCurr, pFaceSeed)
    }


static bool Find (bvector<int>&data, int target, size_t &index)
    {
    size_t n = data.size ();
    for (index = 0; index < n; index++)
        if (data[index] == target)
            return true;
    return false;
    }

static void RemoveIndex (bvector<int> data, size_t index)
    {
    size_t n = data.size ();
    if (index < n)
        {
        if (index < n - 1)
            data[index] = data[n-1];
        data.pop_back ();
        }
    }

void DrapeGraph::UpdateActivePlanesForEdgeCrossing (VuP pOldFace, VuP pNewFace,
    bvector<int> &activePlaneIndex,
    bvector<int> &invalidPlaneIndices
    )
    {
    int newPlaneIndex = GetPlaneIndex (pNewFace);
    int oldPlaneIndex = GetPlaneIndex (pOldFace);
    if (vu_getMask (pOldFace, s_ANY_ACTIVE_EDGE_MASK))
        {
        // Crossing a hard edge into a face with an active plane.
        // Add the plane to the active plane list.
        if (newPlaneIndex >= 0)
            {
            size_t invalidPosition;
            if (!m_indexedPlanes.IsActiveIndex (newPlaneIndex))
                {
                // ignore it silently
                }
            else if (Find (invalidPlaneIndices, newPlaneIndex, invalidPosition))
                {
                RemoveIndex (invalidPlaneIndices, invalidPosition);
                if (s_debug > 1)
                    GEOMAPI_PRINTF (" Invalid index %d reentered\n", (int)newPlaneIndex);
                }
            else
                {
                activePlaneIndex.push_back (newPlaneIndex);
                if (s_debug > 1)
                    GEOMAPI_PRINTF ("(ADD PLANE %d count %d)\n", (int)newPlaneIndex, (int)activePlaneIndex.size ());
                }
            }
        }
    else if (vu_getMask (pNewFace, s_ANY_ACTIVE_EDGE_MASK))
        {
        // Leaving the plane of this face.
        // It should be somewhere on the active plane list .. find it and get rid of it.
        // It's most likely to be on top ..
        if (!m_indexedPlanes.IsActiveIndex (oldPlaneIndex))
            {
            // ignore it silently
            }
        else
            {
            size_t numActivePlanes = activePlaneIndex.size ();
            for (size_t k = numActivePlanes; k-- > 0;)
                {
                if (activePlaneIndex[k] == oldPlaneIndex)
                    {
                    activePlaneIndex[k] = activePlaneIndex[numActivePlanes-1];
                    activePlaneIndex.pop_back ();
                    if (s_debug > 1)
                        GEOMAPI_PRINTF ("(REMOVE PLANE %d count %d)\n", (int)oldPlaneIndex, (int)activePlaneIndex.size ());
                    break;
                    }
                }
            if (activePlaneIndex.size () == numActivePlanes)
                {
                m_indexedPlanes.SetMask (oldPlaneIndex, PlaneData::DanglingEdge);
                invalidPlaneIndices.push_back (oldPlaneIndex);
                //GEOMAPI_PRINTF (" Pop error: Index %d not found among %d?\n", (int)oldPlaneIndex, (int)numActivePlanes);
                }
            }
        }
    if (activePlaneIndex.size () > m_maxPlanes)
        m_maxPlanes = activePlaneIndex.size ();
    }

static WChar GetMaskChar (VuP node, VuMask mask, WChar ch)
    {
    return vu_getMask (node, mask) ? ch : L' ';
    }
void DrapeGraph::DumpSearchData (VuP pOldFace, VuP pNewFace, bvector<int>&activePlaneIndex, VuMask entryMask)
    {
    if (s_debug > 1)
        {
        double dx, dy;
        vu_getDXY (&dx, &dy, pOldFace);
        int oldPlaneIndex = GetPlaneIndex (pOldFace);
        int newPlaneIndex = GetPlaneIndex (pNewFace);
        double scale = sqrt (dx * dx + dy * dy);
        bsiTrig_safeDivide (&scale, 1.0, scale, 1.0);
        GEOMAPI_PRINTF (" (Seed %d%c%c #V=%d plane %d A%g) (Mate %d%c%c #V=%d plane %d A%g)\n     (dxdy %g,%g)",
                    vu_getIndex (pOldFace), GetMaskChar (pOldFace, DrapeGraph::s_PRIMARY_EDGE_MASK, L'R'),
                                            GetMaskChar (pOldFace, DrapeGraph::s_BARRIER_EDGE_MASK, L'S'),
                    vu_countEdgesAroundFace (pOldFace), oldPlaneIndex, vu_area (pOldFace),
                    vu_getIndex (pNewFace), GetMaskChar (pNewFace, DrapeGraph::s_PRIMARY_EDGE_MASK, L'R'),
                                            GetMaskChar (pNewFace, DrapeGraph::s_BARRIER_EDGE_MASK, L'S'),
                    vu_countEdgesAroundFace (pNewFace), newPlaneIndex, vu_area (pNewFace),
                    dx * scale, dy * scale);
        if (activePlaneIndex.size () > 0)
            {
            GEOMAPI_PRINTF ("   (active planes ");
            for (size_t i = 0; i < activePlaneIndex.size (); i++)
                GEOMAPI_PRINTF ("%d ", activePlaneIndex[i]);
            GEOMAPI_PRINTF (")");
            }
        VuDebug::ShowFace (m_pGraph, pNewFace, "\nCANDIDATE", false, false);
        GEOMAPI_PRINTF ("\n");
        if (vu_getMask (pOldFace, entryMask))
            GEOMAPI_PRINTF ("  RETREAT");
        }
    }


void DrapeGraph::ApplyVisibleZInConnectedComponent (VuP pExteriorSeed, double minZ, double maxZ, VuMask visitMask, VuMask entryMask)
    {
    bvector <int> activePlaneIndex;
    bvector <int> invalidPlaneIndex;
    bvector<VuP>seedStack;

    SetZAroundFace (pExteriorSeed, activePlaneIndex, minZ, maxZ);
    PushFaceToSeedStack (visitMask, pExteriorSeed, seedStack);
    
    if (s_debug > 1)
        {
        GEOMAPI_PRINTF ("\n (Euler %d)\n", vu_eulerCharacteristic (m_pGraph));
        }
    VuDebug::ShowFace (m_pGraph, pExteriorSeed, "PRIMARY EXTERIOR", true, true);
    // Depth first search, with many subtle implications
    while (seedStack.size () > 0)
        {
        VuP pOldFace = seedStack[seedStack.size() - 1];
        VuP pNewFace = vu_edgeMate (pOldFace);
        seedStack.pop_back ();
        DumpSearchData (pOldFace, pNewFace, activePlaneIndex, entryMask);
        if (vu_getMask (pOldFace, entryMask))
            {
            UpdateActivePlanesForEdgeCrossing (pOldFace, pNewFace, activePlaneIndex, invalidPlaneIndex);
            }
        else
            {
            if (!vu_getMask (pNewFace, visitMask))
                {
                VuDebug::ShowFace (m_pGraph, pNewFace, "ENTER", true, false);
                UpdateActivePlanesForEdgeCrossing (pOldFace, pNewFace, activePlaneIndex, invalidPlaneIndex);
                vu_setMask (pNewFace, entryMask);
                PushFaceToSeedStack (visitMask, pNewFace, seedStack);
                SetZAroundFace (pNewFace, activePlaneIndex, minZ, maxZ);
                }
            }
        }
    if (s_debug > 1 && activePlaneIndex.size () != 0)
        GEOMAPI_PRINTF ("\n(ERROR FINAL ACTIVE PLANE COUNT %d)\n", (int)activePlaneIndex.size ());
    }

void DrapeGraph::MoveNodesToMaxVisibleZ (double maxZ)
    {
    bvector<VuP> seedNodes;
    VuMask visitMask = vu_grabMask (m_pGraph);
    VuMask entryMask = vu_grabMask (m_pGraph);
    vu_clearMaskInSet (m_pGraph, visitMask);
    vu_clearMaskInSet (m_pGraph, entryMask);
    DRange3d range;
    vu_graphRange (m_pGraph, &range);
    double totalArea = (range.high.x - range.low.x) * (range.high.y - range.low.y);
    static double s_triggerFraction = 0.00001;
    double triggerArea = -s_triggerFraction * totalArea ;    // SIGNED area for skipping zero-area faces
    double defaultZ = range.low.z;
    vu_clearMaskInSet (m_pGraph, VU_EXTERIOR_EDGE);

    VU_SET_LOOP (pExteriorCandidate, m_pGraph)
        {
        if (!vu_getMask (pExteriorCandidate, visitMask))
            {
            if (vu_area (pExteriorCandidate) < triggerArea)
                {
                vu_setMaskAroundFace (pExteriorCandidate, VU_EXTERIOR_EDGE);
                ApplyVisibleZInConnectedComponent (pExteriorCandidate, defaultZ, maxZ, visitMask, entryMask);
                }
            }
        }
    END_VU_SET_LOOP (pExteriorCandidate, m_pGraph)

    // First pass teated both primary and secondary panels as transitions.
    // Do exterior markup again for only primaries...
    vu_windingFloodFromNegativeAreaFaces (m_pGraph, DrapeGraph::s_PRIMARY_EDGE_MASK, VU_EXTERIOR_EDGE);
    

    vu_returnMask (m_pGraph, entryMask);
    vu_returnMask (m_pGraph, visitMask);
    }

VuSetP DrapeGraph::GetGraph (){ return m_pGraph;}
DRange3d DrapeGraph::GetRange ()
    {
    DRange3d range;
    vu_graphRange (m_pGraph, &range);
    return range;
    }

static bool GetNormal (VuP pFace, DVec3d &normal)
    {
    DPoint3d pointA, pointB, pointC;

    vu_getDPoint3d (&pointA, pFace);
    vu_getDPoint3d (&pointB, vu_fsucc(pFace));
    vu_getDPoint3d (&pointC, vu_fsucc( vu_fsucc(pFace)));

    DVec3d vectorAB, vectorBC;
    vectorAB.DifferenceOf (pointB, pointA);
    vectorBC.DifferenceOf (pointC, pointB);
    normal.Zero ();
    if (vectorAB.IsParallelTo (vectorBC))
        return false;
    normal.NormalizedCrossProduct (vectorAB, vectorBC);
    return true;
    }

static int AssignColor (VuP pFace, int singleNormalIndex, int mixedNormalIndex, int exteriorIndex)
    {
    if (vu_getMask (pFace, VU_EXTERIOR_EDGE))
        {
        return exteriorIndex;
        }
    int numNormal = 0;  // Increments from 0 to 1 at first good normal.
                        // Increments again for each normal that differs from first good normal.
                        //     Note that late normals that are identical to each other but different from
                        //     first cause repeated incrementing.
    int numSplit = 0;
    DVec3d firstNormal, currNormal, firstNegativeNormal;
    VU_FACE_LOOP (pCurr, pFace)
        {
        if (GetNormal (pCurr, currNormal))
            {
            if (numNormal == 0)
                {
                firstNormal = currNormal;
                firstNegativeNormal.Negate(firstNormal);
                numNormal = 1;
                }
            else
                {
                if (   firstNegativeNormal.IsParallelTo (currNormal)
                    || firstNormal.IsParallelTo (currNormal))
                    {
                    }
                else
                    {
                    numNormal++;
                    }
                }
            
            }
        else
            numSplit++;
        }
    END_VU_FACE_LOOP (pCurr, pFace)

    if (numNormal == 1)
        {
        return singleNormalIndex;
        }
    else
        {
        return mixedNormalIndex;
        }
    }

static void AddIndex
(
char const *name,
bvector<int>* pIndexArray,
bvector<int>* pColorArray,
int index,
int color
)
    {
    static int s_debug1 = 0;
    if (s_debug1 > 0)
        GEOMAPI_PRINTF ("%s %d\n", name, index);
    if (pIndexArray)
        pIndexArray->push_back (index);
    if (pColorArray)
        pColorArray->push_back (color);
    }

static void AssignVertexIndicesFromCoordinateMatch
(
VuSetP pGraph,
bvector<DPoint3d> &xyzArray,
double vertexTolerance
)
    {
    if (s_debug > 0)
        {
        VuDebug::ShowFaces (pGraph, "START VERTEX ASSIGNMENT", true);
        }

    VuArrayP faceArrayP = vu_grabArray (pGraph);
    VuP faceP;
    DPoint3d xyz;
    int indexShift = 1;
    int nullIndex  = 0;
    int vertexIndex;

    /* Mark all vertices unknown */
    VU_SET_LOOP (currP, pGraph)
    {
    SetVertexIndex (currP, nullIndex);
    }
    END_VU_SET_LOOP (currP, pGraph)

    vu_collectInteriorFaceLoops (faceArrayP, pGraph);
    double tol2 = vertexTolerance * vertexTolerance;
    for (vu_arrayOpen (faceArrayP);
     vu_arrayRead (faceArrayP, &faceP);)
    {
        if (vu_countEdgesAroundFace (faceP) < 3)
            continue;
    VU_FACE_LOOP (currP, faceP)
        {
        vertexIndex = GetVertexIndex (currP);
        if (vertexIndex == nullIndex)
        {
        vertexIndex = (int)xyzArray.size () + indexShift;
        vu_getDPoint3d(&xyz, currP);
                xyzArray.push_back (xyz);
                // Save this index of other sectors that have the same xyz.
        VU_VERTEX_LOOP (vertP, currP)
            {
            DPoint3d xyz1;
            vu_getDPoint3d (&xyz1, vertP);
            double a = xyz1.DistanceSquared (xyz);
            if (a < tol2)
                SetVertexIndex (vertP, vertexIndex);
            }
        END_VU_VERTEX_LOOP (vertP, currP)
        }
        }
    END_VU_FACE_LOOP (currP, faceP)
        }
    if (s_debug > 0)
        {
        VuDebug::ShowFaces (pGraph, "END VERTEX ASSIGNMENT", true);
        }

    vu_returnArray (pGraph, faceArrayP);
    }

void AssignVerticesAtMixedInteriorExterior
(
VuSetP pGraph,
bvector<DPoint3d> &xyzArray
)
    {
    if (s_debug > 0)
        {
        VuDebug::ShowFaces (pGraph, "START MIXED VERTEX ASSIGNMENT", true);
        }
    VuMask visitMask = vu_grabMask (pGraph);
    vu_clearMaskInSet (pGraph, visitMask);
    if (s_debug > 1000)
        VuDebug::ShowElement (pGraph);

    VU_SET_LOOP (pSeed, pGraph)
        {
        if (!vu_getMask (pSeed, visitMask))
            {
            vu_setMaskAroundVertex (pSeed, visitMask);
            int numNullVertex = 0;
            int numSector = 0;
            DPoint3d xyzMax, xyz;
            xyzMax.z = -DBL_MAX;
            VU_VERTEX_LOOP (pCurr, pSeed)
                {
                int vertexIndex = GetVertexIndex (pCurr);
                if (vertexIndex == 0)
                    {
                    vu_getDPoint3d (&xyz, pCurr);
                    if (numNullVertex == 0 || xyz.z > xyzMax.z)
                        xyzMax = xyz;
                    numNullVertex++;
                    }
                numSector++;
                }
            END_VU_VERTEX_LOOP (pCurr, pSeed)

            if (numNullVertex > 0 && numNullVertex < numSector)
                {
                xyzArray.push_back (xyzMax);
                int newIndex = (int)xyzArray.size ();   // 1 shifted
                VU_VERTEX_LOOP (pCurr, pSeed)
                    {
                    if (GetVertexIndex (pCurr) == 0)
                        SetVertexIndex (pCurr, newIndex);
                    }
                END_VU_VERTEX_LOOP (pCurr, pSeed)
                }
            }
        }
    END_VU_SET_LOOP (pSeed, pGraph)
    if (s_debug > 0)
        {
        VuDebug::ShowFaces (pGraph, "END MIXED VERTEX ASSIGNMENT", true);
        }

    vu_returnMask (pGraph, visitMask);
    }

/*-----------------------------------------------------------------*//**
* Construct "index array" style image of vu topology and coordinates.
* ON INPUT -- all nodes assumed to be labeled with "ONE BASED" vertex ids.
* @param pGraph => graph to read.
* @param pIndexArray <= vertex indices are added here.
* @param pXYZArray <= vertex coordinates are added here.
* @bsimethod                EarlinLutz      10/01
+----------------------------------------------------------------------*/
static void CollectIndexedMeshArrays
(
VuSetP      pGraph,
bvector<int>    &indexArray,
bvector<DPoint3d> &xyzArray,
bvector<int>    *pColorIndexArray,
int singleNormalIndex,
int mixedNormalIndex,
int exteriorColorIndex,
VuMask      skipMask
)
    {
    StatVector faceAreaStats;
    int nullIndex = 0;
    VuMask visitMask = vu_grabMask (pGraph);
    vu_clearMaskInSet (pGraph, visitMask);
    VuMask allSkipMasks = skipMask | VU_EXTERIOR_EDGE;    
    size_t numFace = 0;
    size_t numIndex = 0;
    double area;
    VU_SET_LOOP (faceP, pGraph)
        {
        if (!vu_getMask (faceP, visitMask))
            {
            vu_setMaskAroundFace (faceP, visitMask);
            if (   vu_countEdgesAroundFace (faceP) >= 3
                && !vu_getMask (faceP, allSkipMasks)
                && (area = vu_area (faceP)) > 0.0
               )
                {
                faceAreaStats.RecordLogHit (area);
                int color = AssignColor (faceP, singleNormalIndex, mixedNormalIndex, exteriorColorIndex);
            VU_FACE_LOOP (currP, faceP)
                {
                int vertexIndex = GetVertexIndex (currP);
                    if (vertexIndex == 0)
                        {
                        // Shouldn't happen -- except at exteriors.  Force a new vertex.
                        DPoint3d xyz;
                        vu_getDPoint3d (&xyz, currP);
                        vertexIndex = 1 + (int)xyzArray.size ();
                        xyzArray.push_back (xyz);
                        }
                    indexArray.push_back (vertexIndex);
                    if (pColorIndexArray)
                        pColorIndexArray->push_back (color);
                    numIndex++;
                }
            END_VU_FACE_LOOP (currP, faceP)
                indexArray.push_back (nullIndex);
                if (NULL != pColorIndexArray)
                    pColorIndexArray->push_back(nullIndex);
                numFace++;
                }
            }
    }
    END_VU_SET_LOOP (faceP, pGraph)
    if (s_debug > 0)
        {
        GEOMAPI_PRINTF ("(FaceOutput (Faces %d) (Indices %d))\n", (int)numFace, (int)numIndex);
        faceAreaStats.Print ("Face Area Distribution");
        }
    vu_returnMask (pGraph, visitMask);
    }

static bool sortAscending (VuP a, VuP b){return vu_getZ(a) < vu_getZ(b);}
static bool sortDescending (VuP a, VuP b){return vu_getZ(b) < vu_getZ(a);}

// In vertex loop of nodeA, collect representative nodes with altitudes strictly between those of nodeA and nodeB.
// (In context, we expect nodeB to be in the same loop, but that is not required.)
static void loadIntermediateAltitudes
(
VuP nodeA,
VuP nodeB,
bvector<VuP>&altitudes,
double tolerance,
bool ascending
)
    {
    altitudes.clear ();
    double zA = vu_getZ (nodeA);
    double zB = vu_getZ (nodeB);
    double z0 = (zA < zB ? zA : zB) + tolerance;
    double z1 = (zA < zB ? zB : zA) - tolerance;
    VU_VERTEX_LOOP (currNode, nodeA)
        {
        if (IsRealSector (currNode) && !vu_getMask (currNode, VU_EXTERIOR_EDGE))
            {
            double z = vu_getZ (currNode);
            if (z > z0 && z < z1)
                altitudes.push_back (currNode);
            }
        }
    END_VU_VERTEX_LOOP (currNode, nodeA)

    if (ascending)
        std::sort (altitudes.begin (), altitudes.end (), sortAscending);
    else
        std::sort (altitudes.begin (), altitudes.end (), sortDescending);
    if (altitudes.size () > 1)
        {
        size_t iLast = 0;
        for (size_t i = 1; i < altitudes.size (); i++)
            {
            if (GetPlaneIndex (altitudes[i]) != GetPlaneIndex (altitudes[iLast]))
                altitudes[++iLast] = altitudes[i];
            }
        altitudes.resize (iLast+1);
        }
    }

/*-----------------------------------------------------------------*//**
* Append vertical panels.   Assumes all nodes have user pointer set as vertex index.
* Any fully interior edge (or banana bundle) with distinct indices around either vertex produces a panel.
* @param pGraph => graph to read.  "user pointer" fields of all nodes are used
*        as workspace.
* @param pIndexArray <= vertex indices are added here.
* @param pXYZArray <= vertex coordinates are added here.
* @bsimethod                EarlinLutz      10/01
+----------------------------------------------------------------------*/
static void CollectSidePanels
(
VuSetP      pGraph,
bvector<int>    *pIndexArray,
bvector<DPoint3d> *pXYZArray,
bvector<int>*    pColorIndexArray,
int colorIndex,
double vertexTolerance,
VuMask barrierMask
)
    {
    int vertexIndexA0, vertexIndexA1, vertexIndexB0, vertexIndexB1;
    int nullIndex  = 0;
    static int doIntermediates = 1;
    bvector<VuP> altitudeA, altitudeB;   // Altitudes of intermediate points on drop lines.

    VuMask visitMask = vu_grabMask (pGraph);
    vu_clearMaskInSet (pGraph, visitMask);
    vu_clearMaskInSet (pGraph, DrapeGraph::s_WELD_EDGE_MASK);
    VuMask insideSkipMask = VU_EXTERIOR_EDGE;
    VuMask outsideSkipMask = 0;
    int num0 = 0;
    int num1 = 0;
    int num2 = 0;
    int num3 = 0;
    // Start at A0.
    // Naively, A1=A0.vsucc, B0=A1.fsucc, B1=B0.vsucc.
    // Actual vsucc steps must allow for sliver faces hidden in the edge.
    //    B1             A0
    //   +-----------------+
    //    B0             A1
    size_t numTriangle = 0;
    size_t numQuad     = 0;
    size_t numExteriorMaskErorr = 0;
    VuMask compositeSkipMask = barrierMask | VU_EXTERIOR_EDGE;
    VU_SET_LOOP (pNodeA0, pGraph)
        {
        num0++;
        if (!vu_getMask (pNodeA0, visitMask))
            {
            num1++;
            VuP pNodeA1, pNodeB0, pNodeB1;
            vu_setMask (pNodeA0, visitMask);
            if (!IsRealSector (pNodeA0))
                {
                num3++;
                }
            else if (   GetAdjacentRealSectorAroundVertex (pNodeA0, pNodeA1, insideSkipMask, outsideSkipMask)
                && (pNodeB0 = vu_fsucc(pNodeA1),  GetAdjacentRealSectorAroundVertex (pNodeB0, pNodeB1, outsideSkipMask, insideSkipMask))
                && vu_fsucc (pNodeB1) == pNodeA0
                )
                {
                if (barrierMask
                    && vu_getMask (pNodeA0, compositeSkipMask)
                    && vu_getMask (pNodeB1, compositeSkipMask)
                    )
                    {
//                    && vu_getMask (pNodeB0, compositeSkipMask)
//                    && vu_getMask (pNodeA1, compositeSkipMask)
//                    )
//                    {
                    numExteriorMaskErorr++;
                    }
                else
                    {
                    num2++;
                    if (doIntermediates)
                        {
                        loadIntermediateAltitudes (pNodeA0, pNodeA1, altitudeA, vertexTolerance, true);
                        loadIntermediateAltitudes (pNodeB1, pNodeB0, altitudeB, vertexTolerance, false);
                        }
                    vu_setMask (pNodeA0, visitMask);
                    vu_setMask (pNodeB0, visitMask);
                    if (vu_getMask (pNodeA1, VU_EXTERIOR_EDGE))
                        vu_setMask (pNodeA1, DrapeGraph::s_WELD_EDGE_MASK);

                    // vertexIndices are already 1-shifted !!!
                    vertexIndexA0 = GetVertexIndex (pNodeA0);
                    vertexIndexB0 = GetVertexIndex (pNodeB0);
                    vertexIndexA1 = GetVertexIndex (pNodeA1);
                    vertexIndexB1 = GetVertexIndex (pNodeB1);
                    if (   vertexIndexA0 == 0
                        || vertexIndexA1 == 0
                        || vertexIndexB0 == 0
                        || vertexIndexB1 == 0
                        )
                        {
                        GEOMAPI_PRINTF ("Panel in  (A1 %d:%d A0 %d:%d)\n",
                                vu_getIndex (pNodeA1), vertexIndexA1,
                                vu_getIndex (pNodeA0), vertexIndexA0);
                        GEOMAPI_PRINTF ("Panel out (B1 %d:%d B0 %d:%d)\n",
                                vu_getIndex (pNodeB1), vertexIndexB1,
                                vu_getIndex (pNodeB0), vertexIndexB0);

                        }
                    else if (vertexIndexA0 != vertexIndexA1 || vertexIndexB0 != vertexIndexB1)
                        {
                        AddIndex ("Panel A0", pIndexArray, pColorIndexArray, vertexIndexA0, colorIndex);
                        AddIndex ("      B1", pIndexArray, pColorIndexArray, vertexIndexB1, colorIndex);
                        if (vertexIndexB0 != vertexIndexB1)
                            {
                            for (size_t i = 0; i < altitudeB.size (); i++)
                                {
                                AddIndex ("        ", pIndexArray, pColorIndexArray, GetVertexIndex (altitudeB[i]), colorIndex);
                                }
                            AddIndex ("      B0", pIndexArray, pColorIndexArray, vertexIndexB0, colorIndex);
                            }
                        if (vertexIndexA1 != vertexIndexA0)
                            {
                            AddIndex ("      A1", pIndexArray, pColorIndexArray, vertexIndexA1, colorIndex);
                            for (size_t i = 0; i < altitudeA.size (); i++)
                                {
                                AddIndex ("        ", pIndexArray, pColorIndexArray, GetVertexIndex (altitudeA[i]), colorIndex);
                                }
                            }
                        if (vertexIndexA1 != vertexIndexA0 && vertexIndexB0 != vertexIndexB1)
                            numQuad++;
                        else
                            numTriangle++;
                        AddIndex ("      00", pIndexArray, pColorIndexArray, nullIndex, colorIndex);
                        }
                    }
                }
            }
        }
    END_VU_SET_LOOP (pNodeA0, pGraph);
    if (s_debug > 0)
        {
        GEOMAPI_PRINTF ("(PanelOutput (numVu %d) (numEdge %d) (numPanelCandidate %d) (numX %d) (numExtFail %d)\n", (int)num0, (int)num1, (int)num2, (int)num3, (int)numExteriorMaskErorr);
        GEOMAPI_PRINTF ("(PanelOutput (numQuad %d) (numTriangle %d))\n", (int)numQuad, (int)numTriangle);
        }
    vu_returnMask (pGraph, visitMask);
    }

// At each edge with mask at tail,
// Move to head.
// Take userData1 from head.
// Copy around vertex (vsucc direction) until done or hit a mask.
static void copyPreferredUserData1AroundVertex
(
VuSetP pGraph,
VuMask mask,
int selector    // 1 = spread from head, other = simple tail spread.
)
    {
    if (selector == 1)
        {
        VU_SET_LOOP (pTail, pGraph)
            {
            if (vu_getMask (pTail, mask))
                {
                VuP pHead = vu_fsucc (pTail);
                int data = (int)vu_getUserData1 (pHead);
                DPoint3d xyz;
                vu_getDPoint3d (&xyz, pHead);
                for (VuP pNext = vu_vsucc (pHead);
                        pNext != pHead && !vu_getMask (pNext, mask);
                        pNext = vu_vsucc (pNext))
                    {
                    vu_setUserData1 (pNext, (ptrdiff_t)data);
                    vu_setDPoint3d (pNext, &xyz);
                    }
                }
            }
        END_VU_SET_LOOP (pTail, pGraph)
        }
    else
        {
        VU_SET_LOOP (pTail, pGraph)
            {
            if (vu_getMask (pTail, mask))
                {
                int data = (int)vu_getUserData1 (pTail);
                DPoint3d xyz;
                vu_getDPoint3d (&xyz, pTail);
                for (VuP pNext = vu_vsucc (pTail);
                        pNext != pTail;
                        pNext = vu_vsucc (pNext))
                    {
                    vu_setUserData1 (pNext, (ptrdiff_t)data);
                    vu_setDPoint3d (pNext, &xyz);
                    }
                }
            }
        END_VU_SET_LOOP (pTail, pGraph)        
        }
    }

static void CollectWeldTriangulation
(
VuSetP             pGraph,
bvector<int>      *pIndexArray,
bvector<DPoint3d> *pXYZArray,
bvector<int>*      pColorIndexArray,
int                colorIndex
)
    {
    static int s_show = 0;
    static int s_copyType = 0;
    VuDebug::MoveToNewOutputRange ();
    if ((s_show & 0x01) != 0)
        VuDebug::ShowElement (pGraph);
    VuDebug::ShowFaces (pGraph, "START WELD ANALYSIS", true, (s_show & 0x10) == 0x10);
    // Needs work:  What if there is a vertical drop in the weld?
    copyPreferredUserData1AroundVertex (pGraph, DrapeGraph::s_WELD_EDGE_MASK, s_copyType);

    // Delete all edges that are just not part of the weld ...
    vu_freeEdgesByMaskCount (pGraph, DrapeGraph::s_WELD_EDGE_MASK, TRUE, FALSE, TRUE);  // only single sided weld masking makes sense.

    // We have observed extra dangling edges.  Not sure why.  Get rid of danglers.
    // (Why are the extras?  Are there any missing?)
    //vu_deleteDanglingEdges (pGraph);
    if ((s_show & 0x02) != 0)
        VuDebug::ShowElement (pGraph);
    VuDebug::ShowFaces (pGraph, "After weld tearout.", true, (s_show & 0x20) != 0);

    vu_setMaskInSet (pGraph, VU_BOUNDARY_EDGE); // Everything in the weld is a boundary

    // This should be just head to tail joints with same xy, possible z mess..
    vu_mergeOrUnionLoops (pGraph, VUUNION_UNION);

    vu_regularizeGraph (pGraph);
    vu_splitMonotoneFacesToEdgeLimit (pGraph, 3);
    vu_flipTrianglesToImproveQuadraticAspectRatio (pGraph);

    // Deduce inside/outside from active loops only which was placed on outside of original faces ...
    vu_parityFloodFromNegativeAreaFaces (pGraph, VU_BOUNDARY_EDGE, VU_EXTERIOR_EDGE);

    if ((s_show & 0x04) != 0)
        VuDebug::ShowElement (pGraph);
    VuDebug::ShowFaces (pGraph, "After triangulation.", true, (s_show & 0x40) != 0);

    // Output triangles (order reversed because we are making a lower face from a graph
    // constructed as viewed above)
    VuMask visitMask = vu_grabMask (pGraph);

    vu_clearMaskInSet (pGraph, visitMask);
    size_t numInterior = 0;
    size_t numExterior = 0;
    size_t numThisFace = 0;
    VU_SET_LOOP (faceSeed, pGraph)
        {
        numThisFace = vu_countEdgesAroundFace (faceSeed);
        if (0 == vu_getMask (faceSeed, visitMask) && numThisFace > 2)
            {
            vu_setMaskAroundFace (faceSeed, visitMask);

            if (0 == vu_getMask (faceSeed, VU_EXTERIOR_EDGE))
                {
                VU_REVERSE_FACE_LOOP (faceNode, faceSeed)
                    {
                    int vertexIndexA = GetVertexIndex (faceNode);
                    if (vertexIndexA <= 0)
                        {
                        // Shouldn't happen.  All the verts were matched up.
                        // But something in the regularize or triangulate made a bare VU with no labels.
                        // Look for a good one...
                        VU_VERTEX_LOOP (currNode, faceNode)
                            {
                            int vertexIndexB = GetVertexIndex (currNode);
                            if (vertexIndexA <= 0 && vertexIndexB > 0)
                                vertexIndexA = 0;
                            }
                        END_VU_VERTEX_LOOP (currNode, faceNode)
                        }
                    AddIndex ("Underpane", pIndexArray, pColorIndexArray, vertexIndexA, colorIndex);
                    }
                END_VU_REVERSE_FACE_LOOP (faceNode, faceSeed)
                AddIndex ("UnderPanel0", pIndexArray, pColorIndexArray, 0, colorIndex);
                numInterior++;
                }
            else
                {
                numExterior++;
                }
            }
        }
    END_VU_SET_LOOP (faceSeed, pGraph)
    vu_returnMask (pGraph, visitMask);
    }

enum FacetColorIdentifier
    {
    FC_SingleNormal = 2,
    FC_MixedNormal = 1,
    FC_SidePanel = 4,
    FC_Exterior  = 5
    };

// INPUT: mask1 is set on (entire) exterior faces.
//   BUT  exterior markup may have create artificial interior slivers.
// Set mask2 as the expected exterior...
//   Any facet with more than 2 edges and mask1 set gets mask2. (That's easy)
//   In addition, adjacent null faces (whether or not marked with mask1) get mask2.
//  (But faces that are slivers with mask1 but a whose adjacent faces are not mask1 are left unmarked)
static void markEffectiveExterior (VuSetP graph, VuMask mask1, VuMask mask2, bool markSlivers)
    {
    VuMask visitMask = vu_grabMask (graph);
    vu_clearMaskInSet (graph, mask2);
    vu_clearMaskInSet (graph, visitMask);

    // phase 1: mark "large" faces . . .
    VU_SET_LOOP(seed, graph)
        {
        if (!vu_getMask (seed, visitMask))
            {
            vu_setMaskAroundFace (seed, visitMask);
            if (vu_getMask (seed, mask1))
                {
                int n = vu_countEdgesAroundFace (seed);
                if (n > 2)
                    {
                    vu_setMaskAroundFace (seed, mask2);
                    vu_setMaskAroundFace (seed, visitMask);
                    }
                }
            }
        }
    END_VU_SET_LOOP (seed, graph)

    if (markSlivers)
        {
        // phase 2: bubble out through adjacent slivers
        VU_SET_LOOP(vertexSeed, graph)
            {
            if (vu_getMask (vertexSeed, mask2))
                {
                // we are in a mask2 face. walk around the vertex as long as it is non-mask2 slivers.
                VuP firstCandidate = vu_vpred (vertexSeed);
                VU_REVERSE_VERTEX_LOOP (sliverCandidate, firstCandidate)
                    {
                    if (vu_getMask (sliverCandidate, mask2))
                        break;
                    if (vu_fsucc (vu_fsucc (sliverCandidate)) == sliverCandidate)
                        {
                        vu_setMaskAroundFace (sliverCandidate, mask2);
                        }
                    else
                        break;
                    }
                END_VU_REVERSE_VERTEX_LOOP (sliverCandidate, firstCandidate)
                }
            }
        END_VU_SET_LOOP (vertexSeed, graph)
        }
    vu_returnMask (graph, visitMask);
    }

/*---------------------------------------------------------------------------------**//**
Extract indexed mesh arrays from the drape graph.
+---------------+---------------+---------------+---------------+---------------+------*/
void DrapeGraph::ExtractIndexArrays
(
bvector<DPoint3d> &xyzArray,    //!< REQUIRED
bvector<int>  &indexArray,      //!< REQUIRED
bvector<int>  *colorIndexArray, //!< OPTIONAL 
bool             triangulateFirst,
uint32_t          color,
uint32_t          weight,
int32_t           style,
double          vertexTolerance,
double          baseZ,
VuMask          dtmBarrierMask,
bool            outputBase,
bvector<bvector<DPoint3d>> *exteriorLoops
)
    {
    static bool sMergeIntraplane = true;
    static bool sShowExterior = false;
    static int sideColorIndex = FC_SidePanel;
    static int singleNormalIndex = FC_SingleNormal;
    static int mixedNormalIndex  = FC_MixedNormal;
    static int exteriorIndex     = FC_Exterior;
    VuSetP pGraph = GetGraph ();
    if (triangulateFirst)
        {
        vu_splitMonotoneFacesToEdgeLimit (pGraph, 3);
        vu_flipTrianglesToImproveQuadraticAspectRatio (pGraph);
        }
    VuDebug::ShowFaces (pGraph, "ASSIGN VERTEX INDICES", true, false);
    AssignVertexIndicesFromCoordinateMatch (pGraph, xyzArray, vertexTolerance);
    AssignVerticesAtMixedInteriorExterior (pGraph, xyzArray);

    if (nullptr != exteriorLoops)
        {
        static int s_loopType = 2;
        bvector<bvector<VuP>> loops;
        if (s_loopType == 2)
            {
            VuMask effectiveExteriorMask = vu_grabMask (pGraph);
            markEffectiveExterior (pGraph, VU_EXTERIOR_EDGE, effectiveExteriorMask, false);
            bvector<bvector<VuP>> loopsA, loopsB;
            vu_collectMaskedFaces (pGraph, effectiveExteriorMask, true, loopsA);
            vu_collectMaskedFaces (pGraph, effectiveExteriorMask, true, loopsB);
            vu_searchMaximalUnmaskedFaces (pGraph, effectiveExteriorMask, loops);
            vu_returnMask (pGraph, effectiveExteriorMask);
            }
        else if (s_loopType == 1)
            {
            vu_collectMaskedFaces (pGraph, VU_EXTERIOR_EDGE, false, loops);
            }
        else if (s_loopType == -1)
            {
            vu_collectMaskedFaces (pGraph, VU_EXTERIOR_EDGE, true, loops);
            }
        DPoint3d xyz;
        for (auto &loop : loops)
            {
            if (loop.size () > 2)
                {
                exteriorLoops->push_back (bvector<DPoint3d> ());
                DPoint3d xyzPrevious;
                // Due to sudden altitude shifts, may need to output a vertical edge before each point .
                vu_getDPoint3d(&xyzPrevious, vu_fsucc(loop.back ()));
                for (size_t i = 0; i < loop.size (); i++)
                    {    
                    VuP node = loop[i];                    
                    vu_getDPoint3d (&xyz, node);
                    if (!xyz.AlmostEqual (xyzPrevious))
                        exteriorLoops->back ().push_back (xyzPrevious);
                    exteriorLoops->back ().push_back (xyz);
                    vu_getDPoint3d (&xyzPrevious, vu_fsucc(node));

                    }
                xyz = exteriorLoops->back ().front ();
                exteriorLoops->back ().push_back (xyz);
                }
            }
        }

    if (sMergeIntraplane)
        {
        bvector<VuP> bundleArray;
        ReviewGraph (pGraph);
        VuMask barrierMask = vu_grabMask (pGraph);
        AnnotateIntraplaneEdges (pGraph, &xyzArray, barrierMask, bundleArray);
        CollapseBundles (pGraph, bundleArray);

        vu_expandFacesToBarrierWithDistinctVertices (pGraph, barrierMask);

        VuDebug::ShowVertices (pGraph, "AFTER BRIDGE DELETE");
        vu_deleteDanglingEdges (pGraph);
        VuDebug::ShowVertices (pGraph, "AFTER DANGLER DELETE");
        vu_returnMask (pGraph, barrierMask);
        ReviewGraph (pGraph);
        }
    VuDebug::ShowFaces (pGraph, "START OUTPUT", true, false);
    if (s_show != 0)
        VuDebug::ShowElement (pGraph);
    CollectIndexedMeshArrays (pGraph, indexArray, xyzArray, colorIndexArray, singleNormalIndex, mixedNormalIndex, exteriorIndex, dtmBarrierMask);
    CollectSidePanels (pGraph, &indexArray, &xyzArray, colorIndexArray, sideColorIndex, vertexTolerance, dtmBarrierMask);

    if (outputBase)
        {
        CollectWeldTriangulation (pGraph, &indexArray, &xyzArray, colorIndexArray, exteriorIndex);
        }
    }

// Feb 18, 2016 This logic replicated vumaskops.cpp as vu_exchnageNullFacesToBringMaskInside
void ExchangeNullFacesToBringMaskInside (VuSetP pGraph, VuMask mask)
    {
    if (s_show != 0)
        VuDebug::ShowElement (pGraph);
    size_t numNull = 0;
    size_t numReversed = 0;
    static size_t maxReversed = SIZE_MAX;
    // When two input faces "butt against each other along an edge,
    // the angular sort logic (in vu_merge) has no way of knowing which
    // of the geometrically identical edges is logicaly "first" in the sort order.
    // The caller tells us a mask that is preferred "inside" when present once on each edge.
    // Look for the mask appearing "outside" twice and reverse the edges.
    //
    // If the input partitions the plane and has no overlaps, the resulting graph is "exactly" the original
    // polygons glued together with double edges everywhere.
    VU_SET_LOOP (nodeA, pGraph)
        {
        if (!IsRealSector (nodeA))
            {
            numNull++;
            VuP nodeB = vu_fsucc (nodeA);
            VuP nodeA1 = vu_vsucc (nodeA);
            VuP nodeB1 = vu_vsucc (nodeB);
            if (   vu_getMask (nodeA1, mask)
                && !vu_getMask (nodeA, mask)
                && vu_getMask (nodeB1, mask)
                && !vu_getMask (nodeB, mask))
                {
                VuP nodeB0 = vu_vpred (nodeB);
                VuP nodeA0 = vu_vpred (nodeA);

                vu_vertexTwist (pGraph, nodeB, nodeB0);
                vu_vertexTwist (pGraph, nodeA, nodeA1);

                vu_vertexTwist (pGraph, nodeB, nodeB1);
                vu_vertexTwist (pGraph, nodeA1, nodeA0);
                numReversed++;
                }
            }
        if (numReversed > maxReversed)
            break;
        }
    END_VU_SET_LOOP (nodeA, pGraph)
    if (s_show != 0)
        VuDebug::ShowElement (pGraph);
    }

void DrapeGraph::DoAnalysis (double lowZ, double highZ, double clusterTolerance)
    {
    SetZAll (lowZ);
    ClusterXY (clusterTolerance);
    VuDebug::ShowFaces (m_pGraph, "BEFORE MERGE", false);
    vu_mergeOrUnionLoops (m_pGraph, VUUNION_UNION);
    VuDebug::ShowFaces (m_pGraph, "AFTER MERGE", false);
    ExchangeNullFacesToBringMaskInside (m_pGraph, s_PRIMARY_EDGE_MASK);
    ExchangeNullFacesToBringMaskInside (m_pGraph, s_BARRIER_EDGE_MASK);
    vu_regularizeGraph (m_pGraph);

    VuDebug::ShowFaces (m_pGraph, "BEFORE SMALL FACE FLOOD", true);
    static double s_smallFeatureRelTol = 1.0e-6;
    DRange3d range = GetRange ();
    double shortEdgeLength = s_smallFeatureRelTol * range.low.Distance (range.high);
    // hmm..  small feature collaps changes xy coordinates.
    // We would like to make these changes for smallest vertical change.
    // Don't know how to do that.
    CollapseSmallFeatures (m_pGraph, shortEdgeLength, 0.0, 0.0);
    VuDebug::ShowFaces (m_pGraph, "AFTER SMALL FACE FLOOD", true);

    MarkParityErrors ();
    MoveNodesToMaxVisibleZ (highZ);
    }


// Data carrier for a VuMask and pointer to VuGraph.
//
struct VuMaskOps
{
protected:
VuSetP m_graph;
VuMask m_mask;
public:

VuMaskOps (VuSetP graph, VuMask mask)
    : m_graph(graph),
      m_mask(mask)
    {
    }


VuMask Bits (){return m_mask;}
void ClearInSet () {vu_clearMaskInSet (m_graph, m_mask);}
void SetAroundFace (VuP seed){ vu_setMaskAroundFace (seed, m_mask);}
void SetAroundVertex (VuP seed){ vu_setMaskAroundVertex (seed, m_mask);}
bool IsMasked (VuP node){ return 0 != vu_getMask (node, m_mask);}
};

// VuMaskOps derived class with grab/return actions in constructor and destructor.
struct VuTempMask : public VuMaskOps
{
public:
VuTempMask (VuSetP graph, bool initialValue = false)
    : VuMaskOps (graph, vu_grabMask (graph))
    {
    if (initialValue)
        vu_setMaskInSet (graph, m_mask);
    else
        vu_clearMaskInSet (graph, m_mask);
    }

~VuTempMask ()
    {
    vu_returnMask (m_graph, m_mask);
    }
};

void DrapeGraph::MarkFacesBelow (double capZ, VuMask mask)
    {
    VuTempMask visitMask (m_pGraph, false);

    VU_SET_LOOP (faceSeed, m_pGraph)
        {
        if (!visitMask.IsMasked (faceSeed))
            {
            visitMask.SetAroundFace (faceSeed);
            DRange3d range = vu_faceRange (faceSeed);
            if (range.high.z <= capZ)
                vu_setMaskAroundFace (faceSeed, mask);
            }
        }
    END_VU_SET_LOOP (faceSeed, m_pGraph)
    }

void DrapeGraph::MarkFaces (VuTestCondition &tester, VuMask mask)
    {
    VuTempMask visitMask (m_pGraph, false);

    VU_SET_LOOP (faceSeed, m_pGraph)
        {
        if (!visitMask.IsMasked (faceSeed))
            {
            visitMask.SetAroundFace (faceSeed);
            size_t planeIndex = GetPlaneIndex (faceSeed);
            ptrdiff_t parentTag = m_indexedPlanes.GetTag (planeIndex);
            if (tester.Test (m_pGraph, faceSeed, parentTag))
                vu_setMaskAroundFace (faceSeed, mask);
            }
        }
    END_VU_SET_LOOP (faceSeed, m_pGraph)
    }

END_BENTLEY_GEOMETRY_NAMESPACE
