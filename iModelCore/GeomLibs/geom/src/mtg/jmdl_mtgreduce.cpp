/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/mtg/jmdl_mtgreduce.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include    <assert.h>
#include    "mtgintrn.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/*----------------------------------------------------------------------+
|                                                                       |
|   Local Macros                                                        |
|                                                                       |
+----------------------------------------------------------------------*/
/*
Superface loop macro trio:
    * "Superfaces" have edges ("superedges") with >=1 of the edge masks in eMask.
    * _index is undeclared and iterates over superface nodeIds starting w/ start.
    * bMask marks the boundary of the traversed side of the MTG.
    * Superfaces are traversed by vPreds at each vertex until a bMasked or
        eMasked edge is encountered; then the fSucc is taken to the next
        superface vertex.
    * If bMasked node that is not eMasked is encountered, the previous superedge
        is dangling and the superface cannot be closed on one side of the MTG
        (triggers assert).
    * Infinite loops are halted after all nodes traversed (triggers assert).
    * All 3 macros must be used together, in the order below:
*/
#define MTG_SUPERFACE_LOOP_BEGIN(_index,pGraph,start,eMask,bMask)           \
    {                                                                       \
    int         _nodeCt = jmdlMTGGraph_getNodeCount (pGraph);               \
    bool        _bDone = false;                                             \
    MTGNodeId   _index = start;                                             \
    while (!_bDone)                                                         \
        {                                                                   \
        while (!_bDone && !jmdlMTGGraph_getMask (pGraph, _index, eMask | bMask))    \
            {
            // VPred loop:
            //  * code for non-superedge nodes (_index) goes here
            //  * break will NOT exit the loop macro

#define MTG_SUPERFACE_LOOP_MIDDLE(_index,pGraph,start,eMask,bMask)          \
            _index = jmdlMTGGraph_getVPred (pGraph, _index);                \
            if (_index == start || --_nodeCt < 0) _bDone = true;            \
            }                                                               \
        if (!_bDone)                                                        \
            {
            // FSucc loop:
            //  * code for superedge nodes (_index) goes here
            //  * break will exit the loop macro

#define MTG_SUPERFACE_LOOP_END(_index,pGraph,start,eMask,bMask)             \
            assert (jmdlMTGGraph_getMask (pGraph, _index, eMask));          \
            _index = jmdlMTGGraph_getFSucc (pGraph, _index);                \
            if (_index == start || --_nodeCt < 0) _bDone = true;            \
            }                                                               \
        }                                                                   \
    assert (_nodeCt >= 0);                                                  \
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   Local Typedefs                                                      |
|                                                                       |
+----------------------------------------------------------------------*/
// Circle finder global variable/constant bundle
typedef struct _GlobalCircleContext
    {
    // constant per MTG
    MTGFacets               *pFacets;       // connectivity + coordinates
    MTGGraph                *pGraph;        // connectivity only
    const MTG_ReduceMasks   *pMasks;        // circle, edge, boundary, visited
    double                  eps2;           // sin^2(angle) tolerance
    double                  eps2big;        // larger sin^2(angle) tolerance

    // constant per recursion
    MTGNodeId               startNodeId;    // root node of first edge

    // varies per recursion
    int                     nodeCt;         // # nodes b4 break infinite loop
    } GlobalCircleContext;

// Circle finder local variable bundle: must be lean---recursive func arg
typedef struct _LocalCircleContext
    {
    // (ptrs into) previous pass
    DPoint3d    *pPrevVector;   // prev edge (head at prevNodeId's vertex)
    double      prevLen2;       // sqrd length of prev edge
    DPoint3d    *pPrevNormal;   // prev edge pair's normal
    double      prevNormalLen2; // sqrd length of prev edge pair's normal
    MTGNodeId   prevNodeId;     // at start of last superedge

    // running variables
    double      minS2;          // sqr of sine of min interior angle
    double      maxS2;          // sqr of sine of max interior angle
    int         edgeCt;         // # edges in the path so far
    } LocalCircleContext;

/*----------------------------------------------------------------------+
|                                                                       |
|   Global Constants and Variables                                      |
|                                                                       |
+----------------------------------------------------------------------*/
enum
    {
    MIN_CIRCLE_EDGES = 9,       // min edges on circular cycle (8 or greater)
    MAX_CIRCLE_EDGES = 1000     // max edges on circular cycle (stack insurance)
    };

// mininum interior angle for polygon approximating a circle (greater than 3pi/4)
static const double COS_MIN_CIRCLE_ANGLE = cos (msGeomConst_2pi * (0.5 - 1.0 / MIN_CIRCLE_EDGES));
static const double SQR_COS_MIN_CIRCLE_ANGLE = COS_MIN_CIRCLE_ANGLE * COS_MIN_CIRCLE_ANGLE;

GlobalCircleContext GCC;

/*======================================================================+
|                                                                       |
|   Private Utility Routines                                            |
|                                                                       |
+======================================================================*/
/**
* Sets bridge mask on a bridge edge and superface interior mask around start
* vertex of edge if it is not a supervertex.
*
* @param    pGraph  <=> connectivity
* @param pMasks      => node masks used :
*                           preset  : boundary, vertex
*                           set     : bridge, temp2
* @param root        => start node of edge to mask
* @bsihdr                                       DavidAssaf      10/99
+---------------+---------------+---------------+---------------+------*/
static void    maskBridgeEdge
(
MTGGraph                *pGraph,
const MTG_ReduceMasks   *pMasks,
MTGNodeId               root
)
    {
    jmdlMTGGraph_setMaskAroundEdge (pGraph, root, pMasks->bridge);
    if (!jmdlMTGGraph_getMask (pGraph, root, pMasks->vertex))
        jmdlMTGGraph_setMaskAroundManifoldVertex (pGraph, root, pMasks->temp2,
            pMasks->boundary);
    }

/**
* Recursive routine that finds a path starting from a given edge attached to
* but not part of a superface loop, and continuing along edges interior to the
* superface to all accessible, unvisited superface holes.
*
* On input, superfaces should be marked with the vertex/edge masks and the
* superface loop associated with the given node should be marked with the face
* mask temp2.  All boundary-masked edges must also be superface edges.
* Superfaces with the temp1 (to signify nonplanar faces) or with exterior mask
* are not processed.
*
* On output, any bridge edges found have the given bridge mask, and both bridges
* and the holes they connect are masked with temp2.
*
* @param    pGraph  <=> connectivity
* @param pMasks      => node masks used (in called functions):
*                           preset  : boundary, bridge, edge, exterior, temp1, temp2, vertex, visited
*                           set     : (bridge), temp2, visited
* @param root        => start node of edge to consider
* @return true if a bridge was found at this edge; false otherwise
* @bsihdr                                       DavidAssaf      10/99
+---------------+---------------+---------------+---------------+------*/
static bool    recursive_findBridge
(
MTGGraph                *pGraph,
const MTG_ReduceMasks   *pMasks,
MTGNodeId               root
)
    {
    MTGNodeId   mate = jmdlMTGGraph_getEdgeMate (pGraph, root);
    MTGMask     mateMask = jmdlMTGGraph_getMask (pGraph, mate, MTG_ALL_MASK_BITS);
    MTGMask     faceMask = pMasks->temp2;   // superface interior

    jmdlMTGGraph_setMask (pGraph, root, pMasks->visited);

    // this is a superedge and can't be a bridge
    if (mateMask & pMasks->edge)
        return false;

    // mate at visited vertex
    if (mateMask & pMasks->visited)
        return false;

    // mate at unvisited supervertex on this superface loop
    if (mateMask & faceMask)
        {
        jmdlMTGGraph_setMask (pGraph, mate, pMasks->visited);
        return false;
        }

    // mate at unvisited supervertex on hole superface loop (found bridge)
    if (mateMask & pMasks->vertex)
        {
        jmdlMTGGraph_setMask (pGraph, mate, pMasks->visited);

        // set face mask around exterior of hole loop
        jmdlMTGGraph_setFaceMaskAroundSuperFace (pGraph, mate, faceMask,
            pMasks->edge, pMasks->boundary);

        // set masks on this bridge edge
        maskBridgeEdge (pGraph, pMasks, root);

        // search paths leading out from hole for more bridges
        MTG_SUPERFACE_LOOP_BEGIN (node, pGraph, mate, pMasks->edge, pMasks->boundary)
            {
            // don't reprocess this bridge
            if (node != mate)
                recursive_findBridge (pGraph, pMasks, node);
            }
        MTG_SUPERFACE_LOOP_MIDDLE (node, pGraph, mate, pMasks->edge, pMasks->boundary)
        MTG_SUPERFACE_LOOP_END (node, pGraph, mate, pMasks->edge, pMasks->boundary)

        return true;
        }

    // mate at unvisited non-supervertex (found POSSIBLE bridge)
    else
        {
        bool    bBridge = false;

        // prevent bridge loops
        jmdlMTGGraph_setMaskAroundManifoldVertex (pGraph, mate, pMasks->visited,
            pMasks->boundary);

        // process other edges at mate's non-supervertex
        MTG_MANIFOLD_VERTEX_LOOP_BEGIN (node, pGraph, mate, pMasks->boundary)
            {
            if ((mate != node)
                && recursive_findBridge (pGraph, pMasks, node)
                && !bBridge)
                {
                // original edge is part of the bridge
                bBridge = true;
                maskBridgeEdge (pGraph, pMasks, root);
                }
            }
        MTG_MANIFOLD_VERTEX_LOOP_END (node, pGraph, mate, pMasks->boundary)
        return bBridge;
        }
    }

/**
* Recursive routine that finds a circle (a superedge cycle) containing a given
* superedge.
*
* On input, superfaces should be marked with the edge mask.  All boundary-masked
* edges must also be superface edges; boundary masked edges demarcate the extent
* of manifold portions of the MTG.  Superfaces with exterior mask are not
* processed.  Previously found circles may also be masked with the circle mask.
*
* On output, the circular superedge cycle has the given circle (half-edge) mask.
*
* Assumption: two circles will not cross in such a way as to make traversal of
* the superface loop ambiguous.  In other words, circle cycles may not share a
* vertex unless the cycles are on opposite sides of the same circle.
*
* Heuristic: planarity only tested locally against previous edge pair's normal
* (analogous to how planar superfaces are detected in
* jmdlMTGFacets_coalesceCoplanarTriangles).
*
* Before the first call to this function, the global context must be set with
* the current node ID and max node count; the local context must be set with an
* edge count of 0.
*
* Uses GlobalCircleContext (GCC), which includes node mask bundle.  Following
* are the node masks used in this function:
*   preset  : boundary, edge, exterior, punctured
*   set     : circle
*
* @param Locals         <=> local context
* @param pCurrVertex     => currNodeId's vertex = prevNodeId's edgemate's vertex
* @param currNodeId      => start node of superedge to consider
* @see #jmdlMTGFacets_maskCircles
* @return true if a circle was found at this edge; false otherwise
* @bsihdr                                       DavidAssaf      11/99
+---------------+---------------+---------------+---------------+------*/
static bool    recursive_findCircle
(
LocalCircleContext  Locals,
const DPoint3d      *pCurrVertex,
MTGNodeId           currNodeId
)
    {
    // uses GlobalCircleContext (GCC)

    /* Info defined in each pass: ----<-----------------+
    |                                 |                 |
    |      prevNormal ^               | passes >= 2     |
    |                  \              |                 |
    |                ^  *---          | <---------------+
    |    prevVector /  / . prevNodeId | |               |
    |              /  /               | | passes >= 1   |
    | currNormal ^   /                | |               |
    |             \ /                 | | <-------------+
    |   currVertex * . currNodeId     | | |             |
    |              |                  | | |             |
    |            | |                  | | |             |
    | currVector | |                  | | | passes >=0  |
    |            v * nextVertex       | | |             |
    |               \                 | | |             |
    |                                 | | |             |
    +---------------------------------<-<-<------------*/

    DPoint3d    currNormal;
    double      currNormalLen2 = 0.0;
    MTGMask     mask = jmdlMTGGraph_getMask (GCC.pGraph, currNodeId, MTG_ALL_MASK_BITS);

    // found a cycle; check its length
    if (Locals.edgeCt > 0 && currNodeId == GCC.startNodeId)
        return (Locals.edgeCt > MIN_CIRCLE_EDGES);

    // Found non-superedge (e.g. bridge) or previously marked circle edge
    else if (!(mask & GCC.pMasks->edge) || (mask & GCC.pMasks->circle))
        return false;

    // error conditions:
    else if ((mask & GCC.pMasks->exterior)      // wrong side of MTG
        || Locals.edgeCt > MAX_CIRCLE_EDGES     // blew finite recursion stack
        || --GCC.nodeCt < 0)                    // infinite loop
        {
        assert (Locals.edgeCt <= MAX_CIRCLE_EDGES);
        assert (GCC.nodeCt >= 0);
        return false;
        }

    DPoint3d    currVector, nextVertex;
    double      currLen2, sine2 = 0.0;
    MTGNodeId   nextId, edgeMateId = jmdlMTGGraph_getEdgeMate (GCC.pGraph, currNodeId);

    // compute nextVertex, currVector, currLen2
    if (!jmdlMTGFacets_getNodeCoordinates (GCC.pFacets, &nextVertex, edgeMateId))
        return false;
    bsiDPoint3d_subtractDPoint3dDPoint3d (&currVector, &nextVertex, pCurrVertex);
    currLen2 = bsiDPoint3d_magnitudeSquared (&currVector);

    // initialize for safety's sake
    if (Locals.edgeCt == 0)
        {
        currNormal.x = currNormal.y = currNormal.z = 0.0;
        currNormalLen2 = 0.0;
        }

    // prevVector, prevNodeId defined; currNormal computable
    if (Locals.edgeCt > 0)
        {
        // found interior angle too small
        double dot = bsiDPoint3d_dotProduct (&currVector, Locals.pPrevVector);
        if (dot >= 0.0 || (dot * dot < SQR_COS_MIN_CIRCLE_ANGLE * currLen2 * Locals.prevLen2))
            return false;

        // compute currNormal and currNormalLen2
        bsiDPoint3d_crossProduct (&currNormal, Locals.pPrevVector, &currVector);
        currNormalLen2 = bsiDPoint3d_magnitudeSquared (&currNormal);

        // found interior angle too big (colinear edges)
        if (currNormalLen2 <= GCC.eps2 * currLen2 * Locals.prevLen2)
            {
            // Keep going just in case this was a bridge junction, and not a
            // full polygon side.  Lengthen this edge to the next vertex.
            nextId = jmdlMTGGraph_getFSucc (GCC.pGraph, currNodeId);
            bsiDPoint3d_subtract (Locals.pPrevVector, &currVector);
            Locals.prevLen2 = bsiDPoint3d_magnitudeSquared (Locals.pPrevVector);

            if (recursive_findCircle (Locals, &nextVertex, nextId))
                {
                // mask half-edge with circle mask
                jmdlMTGGraph_setMask (GCC.pGraph, currNodeId, GCC.pMasks->circle);
                return true;
                }

            return false;
            }

        // square of sine of angle between current and previous edges
        sine2 = currNormalLen2 / (currLen2 * Locals.prevLen2);

        // define min/maxS2 for next pass if undefined here
        if (Locals.edgeCt == 1)
            Locals.minS2 = Locals.maxS2 = sine2;
        }

    // prevNormal, min/maxS2 defined
    if (Locals.edgeCt > 1)
        {
        // Check for out-of-plane edge: skip dot product test in bsiDPoint3d_normalEqualTolerance()
        // to allow for diametrically opposed normals within tolerance, i.e.,
        // allow 3 consecutive edges that show a slight inflection point.
        DPoint3d cross;
        bsiDPoint3d_crossProduct (&cross, Locals.pPrevNormal, &currNormal);
        if (bsiDPoint3d_magnitudeSquared (&cross)
                > GCC.eps2 * Locals.prevNormalLen2 * currNormalLen2)
            return false;

        /*
        A superedge cycle approximating a circle should have
        (*)                         M - m < eps,
        where M, m are the max, min interior angles of this superface, resp.,
        which we have limited above to being in the range (3pi/4, pi).

        Since we already have the squares of the sines of these angles from the
        squared magnitude of the cross product, we can enforce (*) by checking

        (**)    (sin^2(M) - sin^2(m))^2 <= eps^2 * sin^2(2M)
                                        =  eps^2 * (4 sin^2(M) (1 - sin^2(M))),

        for then, by the Mean Value Theorem and the fact that sin^2 is
        decreasing on (3pi/2, 2pi), we have

                (M - m)^2 < csc^2(2M) (sin^2(M) - sin^2(m))^2 <= eps^2.

        Although (**) is not a necessary condition for (*), we use it
        heuristically as such, and disallow a circle if (**) is not satisfied.
        */

        // check range of min/maxS2, if it's expanded
        if ((sine2 < Locals.minS2) || (sine2 > Locals.maxS2))
            {
            double deltaS2, fraction2, lminS2, lmaxS2;

            if (sine2 < Locals.minS2)
                {
                lminS2 = sine2;
                lmaxS2 = Locals.maxS2;
                }
            else    // sine2 > Locals.maxS2
                {
                lminS2 = Locals.minS2;
                lmaxS2 = sine2;
                }

            deltaS2 = lmaxS2 - lminS2;

            // sin^2(M) = min {lminS2, lmaxS2}
            fraction2 = 4.0 * lminS2 * (1.0 - lminS2);

            // found edge pair whose int angle differs too much from the rest
            if (deltaS2 * deltaS2 > GCC.eps2big * fraction2)
                return false;

            // save min/maxS2 for next pass
            Locals.minS2 = lminS2;
            Locals.maxS2 = lmaxS2;
            }
        }

    // save current info for next pass
    bsiDPoint3d_negateInPlace (&currVector);
    Locals.pPrevVector = &currVector;
    Locals.prevLen2 = currLen2;
    Locals.pPrevNormal = &currNormal;
    Locals.prevNormalLen2 = currNormalLen2;
    Locals.prevNodeId = currNodeId;
    Locals.edgeCt++;

    // loop over reachable edges at nextVertex (MUST mimic superface traversal)
    nextId = jmdlMTGGraph_getFSucc (GCC.pGraph, currNodeId);
    do
        {
        // don't cross another circle
        if (jmdlMTGGraph_getMask (GCC.pGraph, nextId, GCC.pMasks->circle))
            break;

        if (recursive_findCircle (Locals, &nextVertex, nextId))
            {
            // mask half-edge with circle mask
            jmdlMTGGraph_setMask (GCC.pGraph, currNodeId, GCC.pMasks->circle);
            return true;
            }

        // don't vPred over a boundary
        if (jmdlMTGGraph_getMask (GCC.pGraph, nextId, GCC.pMasks->boundary))
            break;
        else
            nextId = jmdlMTGGraph_getVPred (GCC.pGraph, nextId);
        }
        while (nextId != edgeMateId);

    return false;
    }

/**
* In the facet set in which circular superedge loops have been masked by
* jmdlMTGFacets_maskCircles, detect faces that form part of the curved surface
* of a generalized cone with (not necessarily orthogonal) circular cap.
*
* On input, superfaces should be marked with the edge mask and circular
* superedge cycles should be masked with the circle (half-edge) mask.  All
* boundary-masked edges must also be superface edges; boundary masked edges
* demarcate the extent of manifold portions of the MTG.  Superfaces with
* exterior mask are not processed.
*
* On output, the superedges between circular cap and apex are masked with the
* cylsurf mask if they are determined to form a conical surface in between.
*
* Called by the cylinder masker.
*
* @param    pFacets     <=> connectivity + geometry
* @param    pMasks       => node masks used:
*                               preset  : boundary, bridge, circle, edge, exterior
*                               set     : cylsurf
* @see #jmdlMTGFacets_maskCircles
* @see #jmdlMTGFacets_maskCylinders
* @return false on error, true otherwise
* @bsihdr                                       DavidAssaf      01/00
+---------------+---------------+---------------+---------------+------*/
static bool    maskCones
(
MTGFacets               *pFacets,
const MTG_ReduceMasks   *pMasks
)
    {
    if (!pFacets || !pMasks)
        return false;

    MTGGraph            *pGraph = &pFacets->graphHdr;
    EmbeddedIntArray    *pConeVerts = jmdlEmbeddedIntArray_grab ();
    MTGMask             mask,
                        visitedMask = jmdlMTGGraph_grabMask (pGraph),
                        coneMask = jmdlMTGGraph_grabMask (pGraph);
    MTGNodeId           mateId;
    bool                bStatus = false;
    int                 vertId, *pConeVert = NULL, nConeVert, i, i2, apexId;

    // Note: cone apex candidate array: even = (-)(vertId+1); odd = nodeId

    if (!pConeVerts || !visitedMask || !coneMask)
        goto wrapup;
    jmdlMTGGraph_clearMaskInSet (pGraph, visitedMask | coneMask);

    // loop over all nodes
    MTGARRAY_SET_LOOP (nId, pGraph)
        {
        mask = jmdlMTGGraph_getMask (pGraph, nId, MTG_ALL_MASK_BITS);

        // look for interior, unvisited, circle-masked edge
        if ((mask & pMasks->circle) && !(mask & (visitedMask | pMasks->exterior)))
            {
            // (assume that a cone edge does not lie in a circle loop)

            jmdlMTGGraph_setVertexMaskAroundMaskedSuperFace (pGraph, nId,
                visitedMask, 0, 0, pMasks->circle, pMasks->boundary);

            // unmask cone candidates on this side
            jmdlMTGGraph_clearFaceMaskAroundMaskedSuperFace (pGraph, nId,
                visitedMask, 0, pMasks->circle | pMasks->boundary | pMasks->bridge,
                pMasks->circle, pMasks->boundary);

            mateId = jmdlMTGGraph_getEdgeMate (pGraph, nId);
            if (jmdlMTGGraph_getMask (pGraph, mateId, pMasks->circle))
                {
                // unmask cone candidates on other side
                jmdlMTGGraph_clearFaceMaskAroundMaskedSuperFace (pGraph, mateId,
                    visitedMask, 0, pMasks->circle | pMasks->boundary | pMasks->bridge,
                    pMasks->circle, pMasks->boundary);
                }

            // cone candidates = non-visited superedges at (both sides of) circle
            jmdlMTGGraph_setVertexMaskAroundMaskedSuperFace (pGraph, nId,
                coneMask, pMasks->edge, visitedMask, pMasks->circle, pMasks->boundary);

            // fill apex candidate list with candidates at 1st vertex
            jmdlEmbeddedIntArray_empty (pConeVerts);
            MTG_MANIFOLD_VERTEX_LOOP_BEGIN (nnId, pGraph, nId, pMasks->boundary)
                {
                if (jmdlMTGGraph_getMask (pGraph, nnId, coneMask))
                    {
                    // vertId + 1 so can mark a vertId by negation
                    mateId = jmdlMTGGraph_getEdgeMate (pGraph, nnId);
                    if (jmdlMTGFacets_getNodeVertexIndex (pFacets, &vertId, mateId))
                        jmdlEmbeddedIntArray_add2Int (pConeVerts, vertId + 1, mateId);
                    }
                }
            MTG_MANIFOLD_VERTEX_LOOP_END (nnId, pGraph, nId, pMasks->boundary)
            if (nConeVert = jmdlEmbeddedIntArray_getCount (pConeVerts) / 2)
                pConeVert = jmdlEmbeddedIntArray_getPtr (pConeVerts, 0);

            // loop over other circle vertices to whittle down apex candidates
            MTG_SUPERFACE_LOOP_BEGIN (nnId, pGraph, nId, pMasks->circle, pMasks->boundary)
            MTG_SUPERFACE_LOOP_MIDDLE (nnId, pGraph, nId, pMasks->circle, pMasks->boundary)
                {
                if (nnId != nId)
                    {
                    // loop over unvisited superedges at this vertex
                    MTG_MANIFOLD_VERTEX_LOOP_BEGIN (nnnId, pGraph, nnId, pMasks->boundary)
                        {
                        // look for unvisited candidate edges
                        if (jmdlMTGGraph_getMask (pGraph, nnnId, coneMask))
                            {
                            // confirm far vertex as cone apex candidate
                            mateId = jmdlMTGGraph_getEdgeMate (pGraph, nnnId);
                            if (jmdlMTGFacets_getNodeVertexIndex (pFacets, &vertId, mateId))
                                {
                                ++vertId;   // never zero

                                // if far vertex is in list, mark it
                                for (i = 0; i < nConeVert; i++)
                                    {
                                    i2 = 2 * i;
                                    if (pConeVert[i2] == vertId)
                                        {
                                        pConeVert[i2] = -vertId;
                                        break;
                                        }
                                    else
                                        {
                                        // can we have 2 indices to same vertex?
#ifndef NDEBUG
                                        DPoint3d            v0 = {0,0,0}, v1 = {0,0,0};
                                        assert (jmdlEmbeddedDPoint3dArray_getDPoint3d (&pFacets->vertexArrayHdr, &v0, abs(pConeVert[i2])-1));
                                        assert (jmdlEmbeddedDPoint3dArray_getDPoint3d (&pFacets->vertexArrayHdr, &v1, vertId-1));
                                        assert (!bsiDPoint3d_pointEqualTolerance (&v0, &v1, 1.0e-8));
#endif
                                        }
                                    }
                                }
                            }
                        }
                    MTG_MANIFOLD_VERTEX_LOOP_END (nnnId, pGraph, nnId, pMasks->boundary)

                    // compress and reset cone apex candidate list
                    i = 0;
                    while (i < nConeVert)
                        {
                        i2 = 2 * i;
                        if (pConeVert[i2] > 0)
                            {
                            if (i < nConeVert - 1)  // avoid unnecessary calls
                                {
                                // order important: first nodeIds, then vertIds
                                jmdlEmbeddedIntArray_replaceByLast (pConeVerts, i2 + 1);
                                jmdlEmbeddedIntArray_replaceByLast (pConeVerts, i2);
                                }
                            nConeVert--;
                            }
                        else
                            {
                            pConeVert[i2] *= -1;    // make it > 0
                            i++;
                            }
                        }
                    }

                // no more apex candidates for this circle: exit superface loop
                if (!nConeVert)
                    break;
                }
            MTG_SUPERFACE_LOOP_END (nnId, pGraph, nId, pMasks->circle, pMasks->boundary)

            // on a manifold there can be only 0, 1 or 2 cones at this circle
            assert ((0 <= nConeVert) && (nConeVert <= 2));

            // loop over apices
            for (i = 0; i < nConeVert; i++)
                {
                apexId = pConeVert[2 * i + 1];

                // cone-masking: mark apex edges with mates at the circle
                MTG_MANIFOLD_VERTEX_LOOP_BEGIN (nnnId, pGraph, apexId, pMasks->boundary)
                    {
                    mateId = jmdlMTGGraph_getEdgeMate (pGraph, nnnId);
                    if (jmdlMTGGraph_getMask (pGraph, mateId, coneMask))
                        jmdlMTGGraph_setMaskAroundEdge (pGraph, nnnId, pMasks->cylsurf);
                    }
                MTG_MANIFOLD_VERTEX_LOOP_END (nnnId, pGraph, apexId, pMasks->boundary)
                }

            // remove candidate mask from circle vertices
            jmdlMTGGraph_clearVertexMaskAroundSuperFace (pGraph, nId, coneMask,
                pMasks->circle, pMasks->boundary);
            }
        }
    MTGARRAY_END_SET_LOOP (nId, pGraph)
    bStatus = true;

wrapup:
    jmdlMTGGraph_dropMask (pGraph, visitedMask | coneMask);
    jmdlEmbeddedIntArray_drop (pConeVerts);
    return bStatus;
    }

/**
* In the facet set in which circular superedge loops have been masked by
* jmdlMTGFacets_maskCircles, find the first edge (around the vertex at the
* given node) attached to a circular cap and flag all edges at the vertex which
* touch the circular cap.  Such edges are candidates for cylinder side edges.
*
* On input, superfaces should be marked with the edge mask and circular
* superedge cycles should be masked with the circle (half-edge) mask.  All
* boundary-masked edges must also be superface edges; boundary masked edges
* demarcate the extent of manifold portions of the MTG.
*
* On output, ALL cylinder candidate edges at the vertex are masked with the
* temp2 mask.  If pbCap is false and a cylinder candidate edge is found, then the
* new cap is masked with temp1 and pbCap is set to true.  If pbCap is true, a
* cylinder candidate edge is one that hits a temp1-masked cap.
*
* Called by the cylinder masker.
*
* @param    pGraph      <=> connectivity
* @param    pbCap       <=> input: if cap found already; output: if new cap found
* @param    pCapId      <= nodeId of new cap
* @param    pMasks       => node masks used:
*                               preset  : boundary, circle, edge, temp1, visited
*                               set     : temp1, temp2, visited
* @param    nodeId       => vertex around which to search for edges
* @see #jmdlMTGFacets_maskCylinders
* @return true if find a cylinder edge; false otherwise
* @bsihdr                                       DavidAssaf      01/00
+---------------+---------------+---------------+---------------+------*/
static bool    findCylinderSideAtVertex
(
MTGGraph                *pGraph,
bool                    *pbCap,
MTGNodeId               *pCapId,
const MTG_ReduceMasks   *pMasks,
MTGNodeId               nodeId      // circle-masked
)
    {
    MTGMask     mask,
                capMask = pMasks->temp1,    // 2nd circle cap
                cylMask = pMasks->temp2;    // cylsurf candidate
    MTGNodeId   mateId;
    bool        bHitCap = false, bHitOtherCap;

    // look around vertex...
    MTG_MANIFOLD_VERTEX_LOOP_BEGIN (nId, pGraph, nodeId, pMasks->boundary)
        {
        mask = jmdlMTGGraph_getMask (pGraph, nId, MTG_ALL_MASK_BITS);

        // ...for unvisited superedges
        if ((mask & pMasks->edge) && !(mask & pMasks->visited))
            {
            mateId = jmdlMTGGraph_getEdgeMate (pGraph, nId);

            // if haven't found circular cap yet, look for one
            if (!*pbCap)
                {
                // loop over nodes at superedge's far vertex until find cap
                MTG_MANIFOLD_VERTEX_LOOP_BEGIN (nnId, pGraph, mateId, pMasks->boundary)
                    {
                    mask = jmdlMTGGraph_getMask (pGraph, nnId, MTG_ALL_MASK_BITS);

                    // if superedge is at unvisited circle, capMask the circle's
                    // vertices and cylMask the near vertex of the superedge
                    if ((mask & pMasks->circle) && !(mask & pMasks->visited))
                        {
                        *pbCap = bHitCap = true;
                        *pCapId = nnId;
                        jmdlMTGGraph_setVertexMaskAroundSuperFace (pGraph, nnId,
                            capMask, pMasks->circle, pMasks->boundary);
                        jmdlMTGGraph_setMask (pGraph, nId, cylMask);
                        break;
                        }
                    }
                MTG_MANIFOLD_VERTEX_LOOP_END (nnId, pGraph, mateId, pMasks->boundary)

                jmdlMTGGraph_setMask (pGraph, nId, pMasks->visited);
                }

            // if this superedge is connected to the cap, flag it
            else if (jmdlMTGGraph_getMask (pGraph, mateId, capMask))
                {
                jmdlMTGGraph_setMask (pGraph, nId, cylMask | pMasks->visited);
                bHitCap = true;
                }

            // superedge could be on second cylinder
            else
                {
                bHitOtherCap = false;

                // check far vert for unvisited circle mask
                MTG_MANIFOLD_VERTEX_LOOP_BEGIN (nnId, pGraph, mateId, pMasks->boundary)
                    {
                    mask = jmdlMTGGraph_getMask (pGraph, nnId, MTG_ALL_MASK_BITS);

                    if ((mask & pMasks->circle) && !(mask & pMasks->visited))
                        {
                        // get the cylinder edge candidate next time around
                        bHitOtherCap = true;
                        break;
                        }
                    }
                MTG_MANIFOLD_VERTEX_LOOP_END (nnId, pGraph, mateId, pMasks->boundary)

                // no possibility of this superedge being a cylinder side
                if (!bHitOtherCap)
                    jmdlMTGGraph_setMask (pGraph, nId, pMasks->visited);
                }
            }
        }
    MTG_MANIFOLD_VERTEX_LOOP_END (nId, pGraph, nodeId, pMasks->boundary)

    return bHitCap;
    }

/*======================================================================+
|                                                                       |
|   Major Public Code Section : MTG Superface support                   |
|                                                                       |
+======================================================================*/

/**
* For every planar superface in a graph, mark edges (with bridge mask) that
* connect "hole" superfaces with their "containing" superface.
*
* On input, planar superfaces should be marked with the vertex/edge masks.
* All boundary-masked edges must also be superface edges; boundary masked edges
* demarcate the extent of manifold portions of the MTG.  Superfaces with the
* temp1 mask (to signify nonplanar faces) or with exterior mask are not
* processed.
*
* On output, planar superfaces with hole(s) are masked with the punctured face
* mask (this includes bridge edges and hole loops).
*
* @param    pGraph  <=> connectivity
* @param pMasks     <=> node masks used (in called functions):
*                           preset      : boundary, edge, exterior, temp1, vertex
*                           grabbed     : temp2, visited
*                           set         : (bridge), punctured, temp2, (visited)
*                           freed       : temp2, visited
* @return false if error, true otherwise
* @bsihdr                                       DavidAssaf      10/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGGraph_createBridgesInSet
(
MTGGraph        *pGraph,
MTG_ReduceMasks *pMasks
)
    {
    MTGMask mask, manifoldMask, superLoopMask, badNodeMask, avoidMask, faceMask;

    if (!pGraph || !pMasks)
        return false;

    avoidMask = pMasks->temp1;      // superfaces to avoid bridging
    faceMask = pMasks->temp2 = jmdlMTGGraph_grabMask (pGraph);  // superface interior
    pMasks->visited = jmdlMTGGraph_grabMask (pGraph);
    if (!faceMask || !pMasks->visited)
        {
        jmdlMTGGraph_dropMask (pGraph, faceMask | pMasks->visited);
        return false;
        }
    manifoldMask = pMasks->boundary | pMasks->exterior;
    superLoopMask = pMasks->edge | pMasks->bridge;
    badNodeMask = avoidMask | manifoldMask | superLoopMask | pMasks->visited;
    jmdlMTGGraph_clearMaskInSet (pGraph, faceMask | pMasks->visited);

    // loop over all nodes
    MTGARRAY_SET_LOOP (nodeId, pGraph)
        {
        mask = jmdlMTGGraph_getMask (pGraph, nodeId, MTG_ALL_MASK_BITS);

        // look for interior, unvisited, nonbridge, nonsuperedge at a supervertex
        if ((mask & pMasks->vertex) && !(mask & badNodeMask))
            {
            // restrict domain of recursion (following existent bridges)
            jmdlMTGGraph_setFaceMaskAroundSuperFace (pGraph, nodeId, faceMask,
                superLoopMask, pMasks->boundary);

            // find bridges starting with this interior edge
            if (recursive_findBridge (pGraph, pMasks, nodeId))
                {
                // additionally mask with punctured flag if found a bridge
                jmdlMTGGraph_setFaceMaskAroundSuperFace (pGraph, nodeId,
                    pMasks->punctured, superLoopMask, pMasks->boundary);
                }

            // clear superface mask (including existent bridges)
            jmdlMTGGraph_clearFaceMaskAroundSuperFace (pGraph, nodeId, faceMask,
                superLoopMask, pMasks->boundary);
            }
        }
    MTGARRAY_END_SET_LOOP (nodeId, pGraph)

    jmdlMTGGraph_dropMask (pGraph, faceMask | pMasks->visited);
    pMasks->temp2 = pMasks->visited = 0;
    return true;
    }


/**
* Search the punctured superface loop at the given node for other superface
* loops attached by bridge edges and return the number of interior superfaces
* (holes) found.
*
* Optionally return a list of superedge nodes for the holes and (separately) a
* node on the exterior superface with the largest range box diagonal.
*
* On input, the input superface loop must be marked with the punctured face
* mask.  In addition, planar superfaces should be marked with the edge mask.
* All boundary-masked edges must also be superface edges; boundary masked edges
* demarcate the extent of manifold portions of the MTG.  It is assumed that
* punctured edges that are not edge-masked (bridges) form trees that:
* <UL>
* <LI> join the exterior superface to its interior superfaces (holes)
* <LI> are disjoint except possibly at their root
* <LI> may only touch the exterior superface loop at their root
* </UL>
*
* The input node is not guaranteed to be included among the output node(s).
*
* @param    pFacets     <=> connectivity + geometry
* @param pHoleIds       <= one superedge nodeId per hole found (or NULL)
* @param pExtNodeId     <= superedge nodeId on exterior superface loop (or NULL)
* @param pMasks          => node masks used:
*                               preset  : boundary, edge, punctured
* @param nodeId          => node on start superface loop
* @see #jmdlMTGGraph_createBridgesInSet
* @return number of holes found or negative if error
* @bsihdr                                       DavidAssaf      10/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      jmdlMTGFacets_collectAndNumberHolesInSuperFace
(
MTGFacets               *pFacets,   // temporary visit mask grabbed, set, dropped
EmbeddedIntArray        *pHoleIds,
MTGNodeId               *pExtNodeId,
const MTG_ReduceMasks   *pMasks,
MTGNodeId               nodeId
)
    {
    if (!pFacets || !pMasks)
        return -1;

    MTGGraph                *pGraph = &pFacets->graphHdr;
    EmbeddedDPoint3dArray   *pVerts;
    EmbeddedIntArray        *pSuperFaceIds;
    DRange3d                range;
    MTGMask                 visitedMask, evMask;
    double                  diag2, maxDiag2 = 0.0;      // for ext superface
    int                     nHole = -1, nVert, maxIdx = -1;  // index of ext superface

    // candidate superface must be flagged as punctured
    if (!jmdlMTGGraph_getMask (pGraph, nodeId, pMasks->punctured))
        return -1;

    pVerts = jmdlEmbeddedDPoint3dArray_grab ();
    pSuperFaceIds = jmdlEmbeddedIntArray_grabOrEmpty (pHoleIds);
    visitedMask = jmdlMTGGraph_grabMask (pGraph);   // needed only in this fn
    if (!pVerts || !pSuperFaceIds || !visitedMask)
        goto wrapup;
    evMask = pMasks->edge | visitedMask;
    jmdlMTGGraph_clearMaskInSet (pGraph, visitedMask);

    // count edge-superfaces in the punctured-superface
    MTG_SUPERFACE_LOOP_BEGIN (nId, pGraph, nodeId, pMasks->punctured, pMasks->boundary)
    MTG_SUPERFACE_LOOP_MIDDLE (nId, pGraph, nodeId, pMasks->punctured, pMasks->boundary)
        {
        // superedge of unvisited edge-superface
        if (pMasks->edge == jmdlMTGGraph_getMask (pGraph, nId, evMask))
            {
            // compute size of this edge-superface
            if (3 <= (nVert = jmdlMTGFacets_getSuperFaceCoordinates (pFacets,
                                pVerts, nId, pMasks->edge, pMasks->boundary)))
                {
                // add superedge node to hole list
                jmdlEmbeddedIntArray_addInt (pSuperFaceIds, nId);

                bsiDRange3d_initFromArray (&range,
                    jmdlEmbeddedDPoint3dArray_getPtr (pVerts, 0), nVert);
                diag2 = bsiDRange3d_extentSquared (&range);

                // update exterior superface info
                if (diag2 > maxDiag2)
                    {
                    maxIdx = jmdlEmbeddedIntArray_getCount (pSuperFaceIds) - 1;
                    maxDiag2 = diag2;
                    }

                // don't count this edge-superface again
                jmdlMTGGraph_setFaceMaskAroundSuperFace (pGraph, nId, visitedMask,
                    pMasks->edge, pMasks->boundary);
                }
            }
        }
    MTG_SUPERFACE_LOOP_END (nId, pGraph, nodeId, pMasks->punctured, pMasks->boundary)

    // pop exterior superface's node from hole list
    if (pExtNodeId)
        jmdlEmbeddedIntArray_getInt (pSuperFaceIds, pExtNodeId, maxIdx);
    jmdlEmbeddedIntArray_replaceByLast (pSuperFaceIds, maxIdx);
    nHole = jmdlEmbeddedIntArray_getCount (pSuperFaceIds);

wrapup:
    jmdlMTGGraph_dropMask (pGraph, visitedMask);
    jmdlEmbeddedIntArray_dropIfGrabbed (pSuperFaceIds, pHoleIds);
    jmdlEmbeddedDPoint3dArray_drop (pVerts);
    return nHole;
    }


/**
* Collect the coordinates of the vertices on the superface loop at the given
* node.
*
* Optional manifold mask restricts traversal to one side of MTG; use zero if
* geometry is 2-manifold.
*
* @param    pFacets     <=> connectivity + geometry
* @param pVerts         <=> array to fill with coordinates
* @param nodeId          => node on the desired superface loop
* @param edgeMask        => superedges have one or more of these masks
* @param boundaryMask    => edge mask preset along MTG boundary (optional)
* @return # vertices stored (= # superedges), or negative if error
* @bsihdr                                       DavidAssaf      11/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      jmdlMTGFacets_getSuperFaceCoordinates
(
const MTGFacets         *pFacets,
EmbeddedDPoint3dArray   *pVerts,
MTGNodeId               nodeId,
MTGMask                 edgeMask,
MTGMask                 boundaryMask
)
    {
    if (!pFacets || !pVerts)
        return -1;

    const MTGGraph  *pGraph = &pFacets->graphHdr;
    DPoint3d        currVert;

    jmdlEmbeddedDPoint3dArray_empty (pVerts);

    // loop over all nodes in superface loop
    MTG_SUPERFACE_LOOP_BEGIN (nId, pGraph, nodeId, edgeMask, boundaryMask)
    MTG_SUPERFACE_LOOP_MIDDLE (nId, pGraph, nodeId, edgeMask, boundaryMask)
        {
        // get vertex coord from superedge node
        if (!jmdlMTGFacets_getNodeCoordinates (pFacets, &currVert, nId)
            || (false == jmdlEmbeddedDPoint3dArray_addDPoint3d (pVerts, &currVert))
            ) return -1;
        }
    MTG_SUPERFACE_LOOP_END (nId, pGraph, nodeId, edgeMask, boundaryMask)

    return jmdlEmbeddedDPoint3dArray_getCount (pVerts);
    }


/**
* Search the given node and its vertex loop for a node for which the given masks
* are present and absent.  Use zero if either mask is unneeded.
*
* Optional boundary mask restricts traversal to one side of MTG; use zero if
* geometry is 2-manifold.
*
* @param    pGraph      => connectivity
* @param nodeId         => node in the vertex loop
* @param presentMask    => mask to filter returned node
* @param absentMask     => mask to reject returned node
* @param boundaryMask   => edge mask preset along MTG boundary (optional)
* @return first masked node reached, or MTG_NULL_NODEID
* @bsihdr                                       DavidAssaf      11/01
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId jmdlMTGGraph_findMaskAroundManifoldVertex
(
const MTGGraph  *pGraph,
MTGNodeId       nodeId,
MTGMask         presentMask,
MTGMask         absentMask,
MTGMask         boundaryMask
)
    {
    if (pGraph)
        {
        MTG_MANIFOLD_VERTEX_LOOP_BEGIN (nId, pGraph, nodeId, boundaryMask)
            {
            if ((!presentMask || jmdlMTGGraph_getMask (pGraph, nId, presentMask))
                &&
                (!absentMask || !jmdlMTGGraph_getMask (pGraph, nId, absentMask)))
                {
                return nId;
                }
            }
        MTG_MANIFOLD_VERTEX_LOOP_END (nId, pGraph, nodeId, boundaryMask)
        }

    return MTG_NULL_NODEID;
    }


/**
* Set the given mask around the vertex loop of the given node.
*
* Optional boundary mask restricts traversal to one side of MTG; use zero if
* geometry is 2-manifold.
*
* @param    pGraph      <=> connectivity
* @param nodeId          => node in the vertex loop
* @param mask            => mask to set
* @param boundaryMask    => edge mask preset along MTG boundary (optional)
* @bsihdr                                       DavidAssaf      11/01
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGGraph_setMaskAroundManifoldVertex
(
MTGGraph        *pGraph,
MTGNodeId       nodeId,
MTGMask         mask,
MTGMask         boundaryMask
)
    {
    if (pGraph)
        {
        MTG_MANIFOLD_VERTEX_LOOP_BEGIN (nId, pGraph, nodeId, boundaryMask)
            {
            jmdlMTGGraph_setMask (pGraph, nId, mask);
            }
        MTG_MANIFOLD_VERTEX_LOOP_END (nId, pGraph, nodeId, boundaryMask)
        }
    }


/**
* Clear the given mask around the vertex loop of the given node.
*
* Optional boundary mask restricts traversal to one side of MTG; use zero if
* geometry is 2-manifold.
*
* @param    pGraph      <=> connectivity
* @param nodeId          => node in the vertex loop
* @param mask            => mask to clear
* @param boundaryMask    => edge mask preset along MTG boundary (optional)
* @bsihdr                                       DavidAssaf      11/01
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGGraph_clearMaskAroundManifoldVertex
(
MTGGraph        *pGraph,
MTGNodeId       nodeId,
MTGMask         mask,
MTGMask         boundaryMask
)
    {
    if (pGraph)
        {
        MTG_MANIFOLD_VERTEX_LOOP_BEGIN (nId, pGraph, nodeId, boundaryMask)
            {
            jmdlMTGGraph_clearMask (pGraph, nId, mask);
            }
        MTG_MANIFOLD_VERTEX_LOOP_END (nId, pGraph, nodeId, boundaryMask)
        }
    }


/**
* Search the the superface loop containing the given node for a superedge node
* for which the given masks are present and absent.  Use zero if either mask is
* unneeded.
*
* Optional boundary mask restricts traversal to one side of MTG; use zero if
* geometry is 2-manifold.
*
* @param    pGraph      => connectivity
* @param nodeId         => node in the superface loop
* @param presentMask    => mask to filter returned node
* @param absentMask     => mask to reject returned node
* @param edgeMask       => superedges have one or more of these masks
* @param boundaryMask   => edge mask preset along MTG boundary (optional)
* @return node ID of first masked node reached or MTG_NULL_NODEID
* @bsihdr                                       DavidAssaf      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId    jmdlMTGGraph_findEdgeMaskAroundSuperFace
(
const MTGGraph  *pGraph,
MTGNodeId       nodeId,
MTGMask         presentMask,
MTGMask         absentMask,
MTGMask         edgeMask,
MTGMask         boundaryMask
)
    {
    if (pGraph)
        {
        MTG_SUPERFACE_LOOP_BEGIN (nId, pGraph, nodeId, edgeMask, boundaryMask)
        MTG_SUPERFACE_LOOP_MIDDLE (nId, pGraph, nodeId, edgeMask, boundaryMask)
            {
            if ((!presentMask || jmdlMTGGraph_getMask (pGraph, nId, presentMask))
                &&
                (!absentMask || !jmdlMTGGraph_getMask (pGraph, nId, absentMask)))
                {
                return nId;
                }
            }
        MTG_SUPERFACE_LOOP_END (nId, pGraph, nodeId, edgeMask, boundaryMask)
        }

    return MTG_NULL_NODEID;
    }


/**
* Sets the given mask around superedges in the superface loop containing the
* given node.
*
* Optional boundary mask restricts traversal to one side of MTG; use zero if
* geometry is 2-manifold.
*
* @param    pGraph      <=> connectivity
* @param nodeId          => node in the superface loop
* @param mask            => edge mask to set
* @param edgeMask        => superedges have one or more of these masks
* @param boundaryMask    => edge mask preset along MTG boundary (optional)
* @bsihdr                                       DavidAssaf      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGGraph_setEdgeMaskAroundSuperFace
(
MTGGraph        *pGraph,
MTGNodeId       nodeId,
MTGMask         mask,
MTGMask         edgeMask,
MTGMask         boundaryMask
)
    {
    if (pGraph)
        {
        MTG_SUPERFACE_LOOP_BEGIN (nId, pGraph, nodeId, edgeMask, boundaryMask)
        MTG_SUPERFACE_LOOP_MIDDLE (nId, pGraph, nodeId, edgeMask, boundaryMask)
            {
            jmdlMTGGraph_setMaskAroundEdge (pGraph, nId, mask);
            }
        MTG_SUPERFACE_LOOP_END (nId, pGraph, nodeId, edgeMask, boundaryMask)
        }
    }


/**
* Clears the given mask around superedges in the superface loop containing the
* given node.
*
* Optional boundary mask restricts traversal to one side of MTG; use zero if
* geometry is 2-manifold.
*
* @param    pGraph      <=> connectivity
* @param nodeId          => node in the superface loop
* @param mask            => edge mask to clear
* @param edgeMask        => superedges have one or more of these masks
* @param boundaryMask    => edge mask preset along MTG boundary (optional)
* @bsihdr                                       DavidAssaf      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGGraph_clearEdgeMaskAroundSuperFace
(
MTGGraph        *pGraph,
MTGNodeId       nodeId,
MTGMask         mask,
MTGMask         edgeMask,
MTGMask         boundaryMask
)
    {
    if (pGraph)
        {
        MTG_SUPERFACE_LOOP_BEGIN (nId, pGraph, nodeId, edgeMask, boundaryMask)
        MTG_SUPERFACE_LOOP_MIDDLE (nId, pGraph, nodeId, edgeMask, boundaryMask)
            {
            jmdlMTGGraph_clearMaskAroundEdge (pGraph, nId, mask);
            }
        MTG_SUPERFACE_LOOP_END (nId, pGraph, nodeId, edgeMask, boundaryMask)
        }
    }


/**
* Search the the superface loop containing the given node for a node for which
* the given masks are present and absent.  Use zero if either mask is unneeded.
*
* Optional boundary mask restricts traversal to one side of MTG; use zero if
* geometry is 2-manifold.
*
* @param    pGraph      => connectivity
* @param nodeId         => node in the superface loop
* @param presentMask    => mask to filter returned node
* @param absentMask     => mask to reject returned node
* @param edgeMask       => superedges have one or more of these masks
* @param boundaryMask   => edge mask preset along MTG boundary (optional)
* @return node ID of first masked node reached or MTG_NULL_NODEID
* @bsihdr                                       DavidAssaf      11/01
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId    jmdlMTGGraph_findFaceMaskAroundSuperFace
(
const MTGGraph  *pGraph,
MTGNodeId       nodeId,
MTGMask         presentMask,
MTGMask         absentMask,
MTGMask         edgeMask,
MTGMask         boundaryMask
)
    {
    if (pGraph)
        {
        MTG_SUPERFACE_LOOP_BEGIN (nId, pGraph, nodeId, edgeMask, boundaryMask)
            {
            if ((!presentMask || jmdlMTGGraph_getMask (pGraph, nId, presentMask))
                &&
                (!absentMask || !jmdlMTGGraph_getMask (pGraph, nId, absentMask)))
                {
                return nId;
                }
            }
        MTG_SUPERFACE_LOOP_MIDDLE (nId, pGraph, nodeId, edgeMask, boundaryMask)
            {
            if ((!presentMask || jmdlMTGGraph_getMask (pGraph, nId, presentMask))
                &&
                (!absentMask || !jmdlMTGGraph_getMask (pGraph, nId, absentMask)))
                {
                return nId;
                }
            }
        MTG_SUPERFACE_LOOP_END (nId, pGraph, nodeId, edgeMask, boundaryMask)
        }

    return MTG_NULL_NODEID;
    }


/**
* Set the given mask on all nodes of the superface loop containing the
* given node.
*
* Optional boundary mask restricts traversal to one side of MTG; use zero if
* geometry is 2-manifold.
*
* @param    pGraph      <=> connectivity
* @param nodeId          => node in the superface loop
* @param mask            => face mask to set
* @param edgeMask        => superedges have one or more of these masks
* @param boundaryMask    => edge mask preset along MTG boundary (optional)
* @bsihdr                                       DavidAssaf      10/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGGraph_setFaceMaskAroundSuperFace
(
MTGGraph        *pGraph,
MTGNodeId       nodeId,
MTGMask         mask,
MTGMask         edgeMask,
MTGMask         boundaryMask
)
    {
    if (pGraph)
        {
        MTG_SUPERFACE_LOOP_BEGIN (nId, pGraph, nodeId, edgeMask, boundaryMask)
            {
            jmdlMTGGraph_setMask (pGraph, nId, mask);
            }
        MTG_SUPERFACE_LOOP_MIDDLE (nId, pGraph, nodeId, edgeMask, boundaryMask)
            {
            jmdlMTGGraph_setMask (pGraph, nId, mask);
            }
        MTG_SUPERFACE_LOOP_END (nId, pGraph, nodeId, edgeMask, boundaryMask)
        }
    }


/**
* Clear the given mask on all nodes of the superface loop containing the
* given node.
*
* Optional boundary mask restricts traversal to one side of MTG; use zero if
* geometry is 2-manifold.
*
* @param    pGraph      <=> connectivity
* @param nodeId          => node in the superface loop
* @param mask            => face mask to clear
* @param edgeMask        => superedges have one or more of these masks
* @param boundaryMask    => edge mask preset along MTG boundary (optional)
* @bsihdr                                       DavidAssaf      10/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGGraph_clearFaceMaskAroundSuperFace
(
MTGGraph        *pGraph,
MTGNodeId       nodeId,
MTGMask         mask,
MTGMask         edgeMask,
MTGMask         boundaryMask
)
    {
    if (pGraph)
        {
        MTG_SUPERFACE_LOOP_BEGIN (nId, pGraph, nodeId, edgeMask, boundaryMask)
            {
            jmdlMTGGraph_clearMask (pGraph, nId, mask);
            }
        MTG_SUPERFACE_LOOP_MIDDLE (nId, pGraph, nodeId, edgeMask, boundaryMask)
            {
            jmdlMTGGraph_clearMask (pGraph, nId, mask);
            }
        MTG_SUPERFACE_LOOP_END (nId, pGraph, nodeId, edgeMask, boundaryMask)
        }
    }


/**
* Search the the superface loop containing the given node for a supervertex
* containing a node for which the given masks are present and absent.  Use zero if
* either mask is unneeded.
*
* Optional boundary mask restricts traversal to one side of MTG; use zero if
* geometry is 2-manifold.
*
* @param    pGraph      => connectivity
* @param nodeId         => node in the superface loop
* @param presentMask    => mask to filter returned node
* @param absentMask     => mask to reject returned node
* @param edgeMask       => superedges have one or more of these masks
* @param boundaryMask   => edge mask preset along MTG boundary (optional)
* @return node ID of first masked node reached or MTG_NULL_NODEID
* @bsihdr                                       DavidAssaf      11/01
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId    jmdlMTGGraph_findVertexMaskAroundSuperFace
(
const MTGGraph  *pGraph,
MTGNodeId       nodeId,
MTGMask         presentMask,
MTGMask         absentMask,
MTGMask         edgeMask,
MTGMask         boundaryMask
)
    {
    MTGNodeId   testId;

    if (pGraph)
        {
        MTG_SUPERFACE_LOOP_BEGIN (nId, pGraph, nodeId, edgeMask, boundaryMask)
        MTG_SUPERFACE_LOOP_MIDDLE (nId, pGraph, nodeId, edgeMask, boundaryMask)
            {
            testId = jmdlMTGGraph_findMaskAroundManifoldVertex (pGraph, nId,
                        presentMask, absentMask, boundaryMask);
            if (testId != MTG_NULL_NODEID)
                return testId;
            }
        MTG_SUPERFACE_LOOP_END (nId, pGraph, nodeId, edgeMask, boundaryMask)
        }

    return MTG_NULL_NODEID;
    }


/**
* Sets the given mask around all supervertices in the superface loop containing
* the given node.
*
* Optional boundary mask restricts traversal to one side of MTG; use zero if
* geometry is 2-manifold.
*
* @param    pGraph      <=> connectivity
* @param nodeId          => node in the superface loop
* @param mask            => vertex mask to set
* @param edgeMask        => superedges have one or more of these masks
* @param boundaryMask    => edge mask preset along MTG boundary (optional)
* @bsihdr                                       DavidAssaf      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGGraph_setVertexMaskAroundSuperFace
(
MTGGraph        *pGraph,
MTGNodeId       nodeId,
MTGMask         mask,
MTGMask         edgeMask,
MTGMask         boundaryMask
)
    {
    if (pGraph)
        {
        MTG_SUPERFACE_LOOP_BEGIN (nId, pGraph, nodeId, edgeMask, boundaryMask)
        MTG_SUPERFACE_LOOP_MIDDLE (nId, pGraph, nodeId, edgeMask, boundaryMask)
            {
            jmdlMTGGraph_setMaskAroundManifoldVertex (pGraph, nId, mask, boundaryMask);
            }
        MTG_SUPERFACE_LOOP_END (nId, pGraph, nodeId, edgeMask, boundaryMask)
        }
    }


/**
* Clears the given mask around all supervertices of the superface loop
* containing the given node.
*
* Optional boundary mask restricts traversal to one side of MTG; use zero if
* geometry is 2-manifold.
*
* @param    pGraph      <=> connectivity
* @param nodeId          => node in the superface loop
* @param mask            => vertex mask to clear
* @param edgeMask        => superedges have one or more of these masks
* @param boundaryMask    => edge mask preset along MTG boundary (optional)
* @bsihdr                                       DavidAssaf      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGGraph_clearVertexMaskAroundSuperFace
(
MTGGraph        *pGraph,
MTGNodeId       nodeId,
MTGMask         mask,
MTGMask         edgeMask,
MTGMask         boundaryMask
)
    {
    if (pGraph)
        {
        MTG_SUPERFACE_LOOP_BEGIN (nId, pGraph, nodeId, edgeMask, boundaryMask)
        MTG_SUPERFACE_LOOP_MIDDLE (nId, pGraph, nodeId, edgeMask, boundaryMask)
            {
            jmdlMTGGraph_clearMaskAroundManifoldVertex (pGraph, nId, mask, boundaryMask);
            }
        MTG_SUPERFACE_LOOP_END (nId, pGraph, nodeId, edgeMask, boundaryMask)
        }
    }


/**
* Set the given mask around all edges at nodes (for which the given masks are
* present and absent) around supervertices of the superface loop containing the
* given node.  Use zero if either mask is unneeded.
*
* Optional manifold mask restricts traversal to one side of MTG; use zero if
* geometry is 2-manifold.
*
* @param    pGraph      <=> connectivity
* @param nodeId          => node in the superface loop
* @param mask            => edge mask to set
* @param presentMask     => mask to filter target nodes
* @param absentMask      => mask to reject target nodes
* @param edgeMask        => superedges have one or more of these masks
* @param boundaryMask    => edge mask preset along MTG boundary (optional)
* @bsihdr                                       DavidAssaf      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGGraph_setEdgeMaskAroundMaskedSuperFaceVertices
(
MTGGraph        *pGraph,
MTGNodeId       nodeId,
MTGMask         mask,
MTGMask         presentMask,
MTGMask         absentMask,
MTGMask         edgeMask,
MTGMask         boundaryMask
)
    {
    if (pGraph)
        {
        MTG_SUPERFACE_LOOP_BEGIN (nId, pGraph, nodeId, edgeMask, boundaryMask)
        MTG_SUPERFACE_LOOP_MIDDLE (nId, pGraph, nodeId, edgeMask, boundaryMask)
            {
            MTG_MANIFOLD_VERTEX_LOOP_BEGIN (nnId, pGraph, nId, boundaryMask)
                {
                if ((!presentMask || jmdlMTGGraph_getMask (pGraph, nnId, presentMask))
                    &&
                    (!absentMask || !jmdlMTGGraph_getMask (pGraph, nnId, absentMask)))
                    {
                    jmdlMTGGraph_setMaskAroundEdge (pGraph, nnId, mask);
                    }
                }
            MTG_MANIFOLD_VERTEX_LOOP_END (nnId, pGraph, nId, boundaryMask)
            }
        MTG_SUPERFACE_LOOP_END (nId, pGraph, nodeId, edgeMask, boundaryMask)
        }
    }


/**
* Sets the given mask on all nodes for which the given masks are present/absent
* around the superface loop containing the given node.  Use zero if either mask
* is unneeded.
*
* Optional manifold mask restricts traversal to one side of MTG; use zero if
* geometry is 2-manifold.
*
* @param    pGraph      <=> connectivity
* @param nodeId          => node in the superface loop
* @param mask            => face mask to set
* @param presentMask     => mask to filter target nodes
* @param absentMask      => mask to reject target nodes
* @param edgeMask        => superedges have one or more of these masks
* @param boundaryMask    => edge mask preset along MTG boundary (optional)
* @bsihdr                                       DavidAssaf      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGGraph_setFaceMaskAroundMaskedSuperFace
(
MTGGraph        *pGraph,
MTGNodeId       nodeId,
MTGMask         mask,
MTGMask         presentMask,
MTGMask         absentMask,
MTGMask         edgeMask,
MTGMask         boundaryMask
)
    {
    if (pGraph)
        {
        MTG_SUPERFACE_LOOP_BEGIN (nId, pGraph, nodeId, edgeMask, boundaryMask)
            {
            if ((!presentMask || jmdlMTGGraph_getMask (pGraph, nId, presentMask))
                &&
                (!absentMask || !jmdlMTGGraph_getMask (pGraph, nId, absentMask)))
                {
                jmdlMTGGraph_setMask (pGraph, nId, mask);
                }
            }
        MTG_SUPERFACE_LOOP_MIDDLE (nId, pGraph, nodeId, edgeMask, boundaryMask)
            {
            if ((!presentMask || jmdlMTGGraph_getMask (pGraph, nId, presentMask))
                &&
                (!absentMask || !jmdlMTGGraph_getMask (pGraph, nId, absentMask)))
                {
                jmdlMTGGraph_setMask (pGraph, nId, mask);
                }
            }
        MTG_SUPERFACE_LOOP_END (nId, pGraph, nodeId, edgeMask, boundaryMask)
        }
    }


/**
* Clear the given mask on all nodes for which the given masks are present/absent
* around the superface loop containing the given node.  Use zero if either mask
* is unneeded.
*
* Optional manifold mask restricts traversal to one side of MTG; use zero if
* geometry is 2-manifold.
*
* @param    pGraph      <=> connectivity
* @param nodeId          => node in the superface loop
* @param mask            => face mask to clear
* @param presentMask     => mask to filter target nodes
* @param absentMask      => mask to reject target nodes
* @param edgeMask        => superedges have one or more of these masks
* @param boundaryMask    => edge mask preset along MTG boundary (optional)
* @bsihdr                                       DavidAssaf      04/02
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGGraph_clearFaceMaskAroundMaskedSuperFace
(
MTGGraph        *pGraph,
MTGNodeId       nodeId,
MTGMask         mask,
MTGMask         presentMask,
MTGMask         absentMask,
MTGMask         edgeMask,
MTGMask         boundaryMask
)
    {
    if (pGraph)
        {
        MTG_SUPERFACE_LOOP_BEGIN (nId, pGraph, nodeId, edgeMask, boundaryMask)
            {
            if ((!presentMask || jmdlMTGGraph_getMask (pGraph, nId, presentMask))
                &&
                (!absentMask || !jmdlMTGGraph_getMask (pGraph, nId, absentMask)))
                {
                jmdlMTGGraph_clearMask (pGraph, nId, mask);
                }
            }
        MTG_SUPERFACE_LOOP_MIDDLE (nId, pGraph, nodeId, edgeMask, boundaryMask)
            {
            if ((!presentMask || jmdlMTGGraph_getMask (pGraph, nId, presentMask))
                &&
                (!absentMask || !jmdlMTGGraph_getMask (pGraph, nId, absentMask)))
                {
                jmdlMTGGraph_clearMask (pGraph, nId, mask);
                }
            }
        MTG_SUPERFACE_LOOP_END (nId, pGraph, nodeId, edgeMask, boundaryMask)
        }
    }


/**
* Sets the given mask on all nodes for which the given masks are present and absent
* around a supervertex in the superface loop containing the given node.  Use
* zero if either mask is unneeded.
*
* Optional manifold mask restricts traversal to one side of MTG; use zero if
* geometry is 2-manifold.
*
* @param    pGraph      <=> connectivity
* @param nodeId          => node in the superface loop
* @param mask            => mask to set
* @param presentMask     => mask to filter target nodes
* @param absentMask      => mask to reject target nodes
* @param edgeMask        => superedges have one or more of these masks
* @param boundaryMask    => edge mask preset along MTG boundary (optional)
* @bsihdr                                       DavidAssaf      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGGraph_setVertexMaskAroundMaskedSuperFace
(
MTGGraph        *pGraph,
MTGNodeId       nodeId,
MTGMask         mask,
MTGMask         presentMask,
MTGMask         absentMask,
MTGMask         edgeMask,
MTGMask         boundaryMask
)
    {
    if (pGraph)
        {
        MTG_SUPERFACE_LOOP_BEGIN (nId, pGraph, nodeId, edgeMask, boundaryMask)
        MTG_SUPERFACE_LOOP_MIDDLE (nId, pGraph, nodeId, edgeMask, boundaryMask)
            {
            MTG_MANIFOLD_VERTEX_LOOP_BEGIN (nnId, pGraph, nId, boundaryMask)
                {
                if ((!presentMask || jmdlMTGGraph_getMask (pGraph, nnId, presentMask))
                    &&
                    (!absentMask || !jmdlMTGGraph_getMask (pGraph, nnId, absentMask)))
                    {
                    jmdlMTGGraph_setMask (pGraph, nnId, mask);
                    }
                }
            MTG_MANIFOLD_VERTEX_LOOP_END (nnId, pGraph, nId, boundaryMask)
            }
        MTG_SUPERFACE_LOOP_END (nId, pGraph, nodeId, edgeMask, boundaryMask)
        }
    }

/*======================================================================+
|                                                                       |
|   Major Public Code Section : MTG Reduction routines                  |
|                                                                       |
+======================================================================*/

/**
* Drop edges that form trees in the graph.
* <P>
* Algorithm: start at leaf and delete edges that connect it to the next branch.
*
* @param    pGraph      <=> topology
* @return # edges dropped
* @bsihdr                                       DavidAssaf      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int  jmdlMTGGraph_dropTrees
(
MTGGraph    *pGraph
)
    {
    MTGNodeId   nextId, dangleId;
    int         nDropped = 0;

    if (pGraph)
        {
        // drop dangling edges and their branches
        MTGARRAY_SET_LOOP (nodeId, pGraph)
            {
            dangleId = nodeId;
            while (dangleId == jmdlMTGGraph_getVSucc (pGraph, dangleId))
                {
                nextId = jmdlMTGGraph_getFSucc (pGraph, dangleId);
                jmdlMTGGraph_dropEdge (pGraph, dangleId);
                dangleId = nextId;
                nDropped++;
                }
            }
        MTGARRAY_END_SET_LOOP (nodeId, pGraph)
        }

    return nDropped;
    }


/**
* Create a superface set out of the given facet set by masking edges and
* vertices which separate unions of adjacent coplanar triangles.  The resultant
* superfaces are planar within the given angular tolerance.
*
* Bridge edges to holes in these superfaces will be masked with the given bridge
* mask (but not masked with the given edge/vertex masks).  Planar superfaces
* with hole(s) are masked with the punctured face mask (this includes bridge
* edges and hole loops).
*
* The coalesced superfaces are just faces if edges that don't receive either
* edge/bridge mask are dropped from the graph (as will be done if bCleanUp is
* true).
*
* On input, boundary-masked edges should demarcate the extent of manifold
* portions of the MTG.  If this is the case, superfaces will be guaranteed to
* consist of triangles from only one side of the MTG---the side without the
* exterior mask.  Use zero boundary mask if facet geometry is guaranteed to be
* 2-manifold.
*
* @param    pFacets     <=> connectivity + geometry
* @param    pMasks      <=> node masks used (in called functions):
*                               preset  : boundary, exterior
*                               grabbed : temp1
*                               set     : (bridge), edge, (punctured), temp1, vertex
*                               freed   : temp1
* @param    bCleanUp     => true to remove trees, non-superedges/bridges in MTG
* @param    eps2         => max sin^2(angle) allowed between coplanar normals
* @return false on error, true otherwise
* @bsihdr                                       DavidAssaf      10/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_coalesceCoplanarTriangles
(
MTGFacets           *pFacets,
MTG_ReduceMasks     *pMasks,
bool                bCleanUp,
double              eps2
)
    {
    if (!pFacets || !pMasks)
        return false;

    MTGGraph    *pGraph = &pFacets->graphHdr;
    MTGMask     mask, visitedMask, avoidMask;
    MTGNodeId   mateId;
    DPoint3d    n0, n1;     // adjacent triangle normals
    double      d0, d1;     // adjacent normal squared magnitudes (dot products)

    // init masks (avoidMask = superfaces to avoid bridging)
    avoidMask = pMasks->temp1 = jmdlMTGGraph_grabMask (pGraph);
    visitedMask = jmdlMTGGraph_grabMask (pGraph);   // only used in this fn
    if (!avoidMask || !visitedMask)
        {
        jmdlMTGGraph_dropMask (pGraph, avoidMask | visitedMask);
        return false;
        }

    // set and clear only the masks used here
    MTGARRAY_SET_LOOP (nodeId, pGraph)
        {
        jmdlMTGGraph_setMask (pGraph, nodeId, pMasks->edge | pMasks->vertex);
        jmdlMTGGraph_clearMask (pGraph, nodeId, avoidMask | visitedMask);
        }
    MTGARRAY_END_SET_LOOP (nodeId, pGraph)

    // loop over all nodes
    MTGARRAY_SET_LOOP (nodeId, pGraph)
        {
        mask = jmdlMTGGraph_getMask (pGraph, nodeId, MTG_ALL_MASK_BITS);

        // look for interior, unvisited, nonboundary edge
        if (!(mask & (visitedMask | pMasks->boundary | pMasks->exterior)))
            {
            jmdlMTGGraph_setMaskAroundEdge (pGraph, nodeId, visitedMask);
            mateId = jmdlMTGGraph_getEdgeMate (pGraph, nodeId);

            // mark nontriangular faces so we don't bridge them later
            if (!jmdlMTGFacets_getSafeTriangleNormal (pFacets, &n0, &d0, NULL, nodeId, eps2))
                {
                if (!jmdlMTGGraph_getMask (pGraph, nodeId, avoidMask))
                    jmdlMTGGraph_setMaskAroundFace (pGraph, nodeId, avoidMask);
                }
            else if (!jmdlMTGFacets_getSafeTriangleNormal (pFacets, &n1, &d1, NULL, mateId, eps2))
                {
                if (!jmdlMTGGraph_getMask (pGraph, mateId, avoidMask))
                    jmdlMTGGraph_setMaskAroundFace (pGraph, mateId, avoidMask);
                }

            // clear edge/vert flags on edges abutting coplanar triangles
            else if (bsiDPoint3d_normalEqualTolerance (&n0, d0, &n1, d1, eps2))
                {
                jmdlMTGGraph_clearMaskAroundEdge (pGraph, nodeId, pMasks->edge);

                // clear vertex flag if this was last adjacent masked edge
                if (MTG_NULL_NODEID ==
                        jmdlMTGGraph_findMaskAroundManifoldVertex (pGraph,
                            nodeId, pMasks->edge, 0, pMasks->boundary))
                    {
                    jmdlMTGGraph_clearMaskAroundManifoldVertex (pGraph, nodeId,
                        pMasks->vertex, pMasks->boundary);
                    }

                if (MTG_NULL_NODEID ==
                        jmdlMTGGraph_findMaskAroundManifoldVertex (pGraph,
                            mateId, pMasks->edge, 0, pMasks->boundary))
                    {
                    jmdlMTGGraph_clearMaskAroundManifoldVertex (pGraph, mateId,
                        pMasks->vertex, pMasks->boundary);
                    }
                }
            }
        }
    MTGARRAY_END_SET_LOOP (nodeId, pGraph)

    jmdlMTGGraph_dropMask (pGraph, visitedMask);

    // mask bridge edges (to hole loops) in coalesced faces
    jmdlMTGGraph_createBridgesInSet (pGraph, pMasks);

    if (bCleanUp)
        {
        int nDropped;

        // drop unmasked edges
        jmdlMTGGraph_dropByMask (pGraph, pMasks->edge | pMasks->bridge, 0);

        // drop danglers and orphans (can be created by large eps2?)
        nDropped = jmdlMTGGraph_dropTrees (pGraph);

        // assert (!nDropped);
        }

    jmdlMTGGraph_dropMask (pGraph, avoidMask);
    pMasks->temp1 = 0;
    return true;
    }


/**
* In the facet set of superedges (denoted by the given edge mask), detect and
* mask superedge cycles (not necessarily superfaces) which approximate circles.
*
* On input, superfaces should be marked with the edge mask.  All boundary-masked
* edges must also be superface edges; boundary masked edges demarcate the extent
* of manifold portions of the MTG.  Superfaces with exterior mask are not
* processed.
*
* On output, each circular superedge cycle is masked with the given circle
* (half-edge) mask.
*
* @param    pFacets     <=> connectivity + geometry
* @param    pMasks      <=> node masks used (in called functions):
*                               preset  : boundary, edge, exterior, punctured
*                               set     : (circle)
* @param    eps2         => min squared radian difference in angle between edges
* @see #jmdlMTGFacets_getMaskedCircleInfo
* @return false if error, true otherwise
* @bsihdr                                       DavidAssaf      11/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_maskCircles
(
MTGFacets               *pFacets,
const MTG_ReduceMasks   *pMasks,
double                  eps2
)
    {
    // uses GlobalCircleContext (GCC)

    if (!pFacets || !pMasks)
        return false;

    MTGGraph            *pGraph = &pFacets->graphHdr;
    DPoint3d            v;
    int                 nc = jmdlMTGGraph_getNodeCount (pGraph);
    LocalCircleContext  Locals;

    // setup local context
    memset (&Locals, 0, sizeof (Locals));
    Locals.prevNodeId = MTG_NULL_NODEID;
    Locals.minS2 = 1.0;

    // setup global constants
    memset (&GCC, 0, sizeof (GCC));
    GCC.pFacets = pFacets;
    GCC.pGraph = pGraph;
    GCC.pMasks = pMasks;
    GCC.eps2 = eps2;
    GCC.eps2big = sqrt (eps2);  // give it some slop

    // Mask circles
    MTGARRAY_SET_LOOP (nId, pGraph)
        {
        if (jmdlMTGFacets_getNodeCoordinates (pFacets, &v, nId))
            {
            GCC.startNodeId = nId;
            GCC.nodeCt = nc;
            recursive_findCircle (Locals, &v, nId);
            }
        }
    MTGARRAY_END_SET_LOOP (nId, pGraph)

    return true;
    }


/**
* Gets information on the circle approximated by the circular superedge cycle at
* nodeId.  The normal will be set closest to the optional vector.  The returned
* information can be used directly by mdlEllipse_create, for example (provided
* the matrix is correctly converted to MDL).
*
* On input, the circular superedge cycle should be masked with the circle
* (half-edge) mask.
*
* @param    pFacets      => connectivity + geometry
* @param    pCenter     <= center of circle (or NULL)
* @param    pRadius     <= radius of circle (or NULL)
* @param    pMatrix     <= normalized orientation (col[2] = normal) (or NULL)
* @param    pMasks       => node masks used:
*                               preset  : boundary, circle
* @param    nodeId       => node on circular superedge cycle
* @param    pDirection   => determines normal direction (or NULL)
* @see #jmdlMTGFacets_maskCircles
* @return false on error, true otherwise
* @bsihdr                                       DavidAssaf      11/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_getMaskedCircleInfo
(
const MTGFacets         *pFacets,
DPoint3d                *pCenter,
double                  *pRadius,
RotMatrix               *pMatrix,
const MTG_ReduceMasks   *pMasks,
MTGNodeId               nodeId,
const DPoint3d          *pDirection
)
    {
    if (!pFacets || !pMasks)
        return false;

    if (!pCenter && !pRadius && !pMatrix)
        return true;

    EmbeddedDPoint3dArray   *pVerts = jmdlEmbeddedDPoint3dArray_grab ();
    DPoint3d                *pV, n, c, v0, v1;
    double                  nVertRecip;
    bool                    bStatus = false;
    int                     i, nVert;

    // get coords of this circle-masked superface
    nVert = jmdlMTGFacets_getSuperFaceCoordinates (pFacets, pVerts, nodeId,
                pMasks->circle, pMasks->boundary);

    // don't allow degeneracies/errors */
    if (nVert > 2)
        {
        bStatus = true;

        nVertRecip = 1.0 / nVert;
        pV = jmdlEmbeddedDPoint3dArray_getPtr (pVerts, 0);

        // compute center and normal
        c.x = c.y = c.z = n.x = n.y = n.z = 0.0;
        for (i = 0; i < nVert; i++)
            {
            // average vertex is the LSQ approx to the vertices
            v0 = pV[i];
            v1 = (i == nVert - 1) ? pV[0] : pV[i + 1];
            bsiDPoint3d_addDPoint3dInPlace (&c, &v0);

            // "areaNormal" from bsputil_extractNormal (assumed well-conditioned)
            n.x += (v0.y - v1.y) * (v0.z + v1.z);
            n.y += (v0.z - v1.z) * (v0.x + v1.x);
            n.z += (v0.x - v1.x) * (v0.y + v1.y);
            }
        bsiDPoint3d_scaleInPlace (&c, nVertRecip);
        if (pCenter)
            *pCenter = c;

        // compute RMSE (this was minimized above, and approximates avg radius)
        if (pRadius)
            {
            double r = 0.0;
            for (i = 0; i < nVert; i++)
                {
                bsiDPoint3d_subtractDPoint3dDPoint3d (&v0, pV + i, &c);
                r += bsiDPoint3d_magnitudeSquared (&v0);
                }
            *pRadius = sqrt (r * nVertRecip);
            }

        // compute rotation matrix (cols b4 normalize: radius, radius, normal)
        if (pMatrix)
            {
            DVec3d    xAxis;

            // compute x axis (*pV is circle vertex at nodeId)
            bsiDPoint3d_subtractDPoint3dDPoint3d (&xAxis, pV, &c);

            // flip normal to be close to given direction
            if (pDirection && (bsiDPoint3d_dotProduct (&n, pDirection) < 0))
                bsiDPoint3d_negateInPlace (&n);

            // dummy y-axis is computed in squareAndNormalizeColumns
            bsiRotMatrix_initFromColumnVectors (pMatrix, &xAxis, &xAxis, (DVec3d *) &n);
            bStatus = bsiRotMatrix_squareAndNormalizeColumns (pMatrix,
                        pMatrix, 2, 0);
            }
        }

    jmdlEmbeddedDPoint3dArray_drop (pVerts);
    return bStatus;
    }


/**
* In the facet set in which circular superedge loops have been masked by
* jmdlMTGFacets_maskCircles, detect faces that form part of the curved surface
* of a generalized cylinder with (not necessarily parallel, congruent or both
* nondegenerate) circular caps.
*
* On input, superfaces should be marked with the edge mask and circular
* superedge cycles should be masked with the circle (half-edge) mask.  All
* boundary-masked edges must also be superface edges; boundary masked edges
* demarcate the extent of manifold portions of the MTG.  Superfaces with
* exterior mask are not processed.
*
* On output, the superedges between circular caps are masked with the cylsurf
* mask if they are determined to form a cylindrical surface between the caps.
*
* @param    pFacets     <=> connectivity + geometry
* @param    pMasks      <=> node masks used (in called functions):
*                               preset  : boundary, bridge, circle, (edge), exterior
*                               grabbed : temp1, temp2, visited
*                               set     : cylsurf, temp1, temp2, visited
*                               freed   : temp1, temp2, visited
* @see #jmdlMTGFacets_maskCircles
* @see #jmdlMTGFacets_getMaskedCylinderInfo
* @return false on error, true otherwise
* @bsihdr                                       DavidAssaf      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_maskCylinders
(
MTGFacets           *pFacets,
MTG_ReduceMasks     *pMasks
)
    {
    if (!pFacets || !pMasks)
        return false;

    MTGGraph    *pGraph = &pFacets->graphHdr;
    MTGMask     mask, capMask, cylMask;
    MTGNodeId   capId = 0, mateId;
    bool        bCap, bHitCap = false, bStatus = false;

    // mask cones first
    if (!maskCones (pFacets, pMasks))
        return false;

    // init local masks
    pMasks->visited = jmdlMTGGraph_grabMask (pGraph);
    capMask = pMasks->temp1 = jmdlMTGGraph_grabMask (pGraph);   // 2nd circle cap
    cylMask = pMasks->temp2 = jmdlMTGGraph_grabMask (pGraph);   // cylsurf candidate
    if (!pMasks->visited || !capMask || !cylMask)
        goto wrapup;
    jmdlMTGGraph_clearMaskInSet (pGraph, pMasks->visited | capMask | cylMask);

    // loop over all nodes
    MTGARRAY_SET_LOOP (nId, pGraph)
        {
        mask = jmdlMTGGraph_getMask (pGraph, nId, MTG_ALL_MASK_BITS);

        // look for interior, unvisited, circle-masked edge
        if ((mask & pMasks->circle) && !(mask & (pMasks->visited | pMasks->exterior)))
            {
            // (assume that a cylinder edge does not lie in a circle loop)

            jmdlMTGGraph_setVertexMaskAroundMaskedSuperFace (pGraph, nId,
                pMasks->visited, 0, 0, pMasks->circle, pMasks->boundary);

            // unmask cylinder candidates on this side
            jmdlMTGGraph_clearFaceMaskAroundMaskedSuperFace (pGraph, nId,
                pMasks->visited, 0, pMasks->circle | pMasks->boundary | pMasks->cylsurf | pMasks->bridge,
                pMasks->circle, pMasks->boundary);

            mateId = jmdlMTGGraph_getEdgeMate (pGraph, nId);
            if (jmdlMTGGraph_getMask (pGraph, mateId, pMasks->circle))
                {
                // unmask cylinder candidates on other side
                jmdlMTGGraph_clearFaceMaskAroundMaskedSuperFace (pGraph, mateId,
                    pMasks->visited, 0, pMasks->circle | pMasks->boundary | pMasks->cylsurf | pMasks->bridge,
                    pMasks->circle, pMasks->boundary);
                }

            // find ALL cylinders at this base circle (look on both sides of circle)
            do
                {
                bCap = false;   // target cap exists?

                // loop over vertices of this circle
                MTG_SUPERFACE_LOOP_BEGIN (nnId, pGraph, nId, pMasks->circle, pMasks->boundary)
                MTG_SUPERFACE_LOOP_MIDDLE (nnId, pGraph, nId, pMasks->circle, pMasks->boundary)
                    {
                    bHitCap = findCylinderSideAtVertex (pGraph, &bCap, &capId,
                                pMasks, nnId);

                    // this vertex is not adj to a cap circle's verts
                    if (!bHitCap)
                        break;    // look for next cap
                    }
                MTG_SUPERFACE_LOOP_END (nnId, pGraph, nId, pMasks->circle, pMasks->boundary)

                // a capMasked circle is reachable from this circle
                if (bCap)
                    {
                    // the other circle is actually a cylinder cap
                    if (bHitCap)
                        {
                        // mark cylinder edges attached to circle verts
                        jmdlMTGGraph_setEdgeMaskAroundMaskedSuperFaceVertices (pGraph,
                            nId, pMasks->cylsurf, cylMask, 0, pMasks->circle,
                            pMasks->boundary);
                        }

                    // remove temporary vertex masks around caps
                    jmdlMTGGraph_clearVertexMaskAroundSuperFace (pGraph, nId,
                        cylMask, pMasks->circle, pMasks->boundary);
                    jmdlMTGGraph_clearVertexMaskAroundSuperFace (pGraph, capId,
                        capMask, pMasks->circle, pMasks->boundary);
                    }
                }
            while (bCap);
            }
        }
    MTGARRAY_END_SET_LOOP (nId, pGraph)
    bStatus = true;

wrapup:
    jmdlMTGGraph_dropMask (pGraph, pMasks->visited | capMask | cylMask);
    pMasks->visited = pMasks->temp1 = pMasks->temp2 = 0;
    return bStatus;
    }


/**
* Gets information on the (circular) cylindrical surface containing the
* cylsurf-masked superedge at nodeId.  The end vertices of this superedge are
* scanned for circle-masked nodes, and the first such node found defines the
* corresponding circular cap: it is important therefore that any circle-masked
* nodes at a vertex refer to the same circle.  If exactly one end vertex lacks a
* circle-masked node, information is returned for the corresponding cone.
*
* The translation from base to top cap is returned in the point parameters, so
* there is no translation component in the returned transform.  The returned
* information can be used directly by mdlSurface_project, for example (if the
* transform is correctly converted to MDL).
*
* Rule lines for the cylinder will be parallel to the shortest cylinder edge at
* the given node's vertex.
*
* @param    pFacets      => connectivity + geometry
* @param    pOrigin     <= center of base cap (or NULL)
* @param    pTarget     <= center of top cap (or NULL)
* @param    pTransform  <= transformation from base cap to top cap (or NULL)
* @param    pBaseId     <= circle-masked node at base cap (or NULL)
* @param    pTopId      <= circle-masked node at top cap or MTG_NULL_NODEID if cone (or NULL)
* @param    pMasks       => node masks used:
*                               preset  : boundary, circle, cylsurf
* @param    nodeId       => cylsurf-masked node on a cylinder face at the base cap
* @see #jmdlMTGFacets_maskCylinders
* @see #jmdlMTGFacets_maskCircles
* @return false on error, true otherwise
* @bsihdr                                       DavidAssaf      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGFacets_getMaskedCylinderInfo
(
const MTGFacets         *pFacets,
DPoint3d                *pOrigin,
DPoint3d                *pTarget,
Transform               *pTransform,
MTGNodeId               *pBaseId,
MTGNodeId               *pTopId,
const MTG_ReduceMasks   *pMasks,
MTGNodeId               nodeId
)
    {
    if (!pFacets || !pMasks)
        return false;

    const MTGGraph  *pGraph = &pFacets->graphHdr;
    MTGNodeId       baseId, topId, mateId = jmdlMTGGraph_getEdgeMate (pGraph, nodeId);
    DPoint3d        target, origin;
    bool            bCone = false, bStatus = false;
    origin.Zero ();
    target.Zero ();
    // find IDs on caps: baseId & topId are on same side of circle caps as
    // nodeId & mateId b/c manifold vertex traversal macros search by vPreds.
    baseId = jmdlMTGGraph_findMaskAroundManifoldVertex (pGraph, nodeId,
                pMasks->circle, 0, pMasks->boundary);
    topId = jmdlMTGGraph_findMaskAroundManifoldVertex (pGraph, mateId,
                pMasks->circle, 0, pMasks->boundary);

    // get origin and target
    if ((baseId != MTG_NULL_NODEID) && (topId != MTG_NULL_NODEID))
        {
        // cylinder
        bStatus = (   jmdlMTGFacets_getMaskedCircleInfo (pFacets, &origin,
                        0, 0, pMasks, baseId, 0)
                   && jmdlMTGFacets_getMaskedCircleInfo (pFacets, &target,
                        0, 0, pMasks, topId, 0));
        }
    else if ((baseId != MTG_NULL_NODEID) && (topId == MTG_NULL_NODEID))
        {
        // cone
        bStatus = jmdlMTGFacets_getMaskedCircleInfo (pFacets, &origin,
                    0, 0, pMasks, baseId, 0);
        if (bStatus)
            {
            jmdlMTGFacets_getNodeCoordinates (pFacets, &target, mateId);
            bCone = true;
            }
        }
    else if ((baseId == MTG_NULL_NODEID) && (topId != MTG_NULL_NODEID))
        {
        // inverted cone
        bStatus = jmdlMTGFacets_getMaskedCircleInfo (pFacets, &origin,
                    0, 0, pMasks, topId, 0);
        if (bStatus)
            {
            jmdlMTGFacets_getNodeCoordinates (pFacets, &target, nodeId);
            baseId = topId;
            topId = MTG_NULL_NODEID;
            bCone = true;
            }
        }

    // get baseId, target, origin, transform
    if (bStatus)
        {
        if (pBaseId)
            *pBaseId = baseId;

        if (pTopId)
            *pTopId = topId;

        if (pTarget)
            *pTarget = target;

        if (pOrigin)
            *pOrigin = origin;

        // get cap transformation
        if (pTransform)
            {
            RotMatrix   rot, baseRot;
            DPoint3d    proj;
            double      baseRad;

            // get base rotation and radius
            bsiDPoint3d_subtractDPoint3dDPoint3d (&proj, &target, &origin);
            bStatus = jmdlMTGFacets_getMaskedCircleInfo (pFacets, 0, &baseRad,
                            &baseRot, pMasks, baseId, &proj);
            if (!bStatus)
                goto wrapup;

            // cone rotation ("squish it to a point!")
            if (bCone)
                {
                DVec3d    n;

                // equivalent to using topRad = 0 and topRot = [0|0|normal]
                bsiRotMatrix_getColumn (&baseRot, &n, 2);
                bsiRotMatrix_initFromScaledOuterProduct (&rot, &n, &n, 1.0);
                }

            // cylinder rotation
            else
                {
                RotMatrix   scale, topRot;
                DPoint3d    vBase, vTop;
                double      dist2, minDist2 = DBL_MAX, ratioRad, topRad;
                MTGNodeId   mateId, minTopId = MTG_NULL_NODEID;
                MTGMask     stopMask = pMasks->boundary | pMasks->circle;

                // Get top rotation and radius:
                // Find smallest cylsurf edge on this cylinder to use for
                // initializing the top cap's rotation; this minimizes twist in
                // the rule lines, which will be parallel to this edge.
                jmdlMTGFacets_getNodeCoordinates (pFacets, &vBase, nodeId);
                MTG_MANIFOLD_VERTEX_LOOP_BEGIN (nId, pGraph, nodeId, stopMask)
                    {
                    mateId = jmdlMTGGraph_getEdgeMate (pGraph, nId);
                    jmdlMTGFacets_getNodeCoordinates (pFacets, &vTop, mateId);
                    dist2 = bsiDPoint3d_distanceSquared (&vBase, &vTop);
                    if (dist2 < minDist2)
                        {
                        minDist2 = dist2;
                        minTopId = mateId;
                        }
                    }
                MTG_MANIFOLD_VERTEX_LOOP_END (nId, pGraph, baseId, stopMask)
                if (minTopId != MTG_NULL_NODEID)
                    {
                    // reset topId to a circle node at far vertex of min edge
                    topId = jmdlMTGGraph_findMaskAroundManifoldVertex (pGraph,
                                minTopId, pMasks->circle, 0, pMasks->boundary);

                    }
                bStatus = jmdlMTGFacets_getMaskedCircleInfo (pFacets, 0,
                            &topRad, &topRot, pMasks, topId, &proj);
                if (!bStatus)
                    goto wrapup;

                // get matrix to rotate/scale from base cap to top cap
                ratioRad = topRad / baseRad;
                bsiRotMatrix_initFromScaleFactors (&scale, ratioRad, ratioRad, 1.0);

                bsiRotMatrix_transposeInPlace (&baseRot);
                bsiRotMatrix_multiplyRotMatrixRotMatrix (&rot, &scale, &baseRot);
                bsiRotMatrix_multiplyRotMatrixRotMatrix (&rot, &topRot, &rot);
                }

            // ignore translation component
            bsiTransform_initFromMatrix (pTransform, &rot);
            }
        }

wrapup:
    assert (bStatus);   // illegally masked cylinder/circle
    return bStatus;
    }

/*======================================================================+
|                                                                       |
|   Deprecated functions for Triforma build                             |
|                                                                       |
+======================================================================*/

/**
* DEPRECATED
* @bsihdr                                       DavidAssaf      12/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlMTGGraph_vertexHasAmbiguousCircle
(
const MTGGraph          *pGraph,
MTGNodeId               *pCapId,
const MTG_ReduceMasks   *pMasks,
MTGNodeId               nodeId
)
    {
    if (!pGraph || !pMasks) return false;

    MTGNodeId   capId = MTG_NULL_NODEID;
    MTGMask     mask;
    bool        bPuncturedCapFound = false;
    int         nCap = 0;

    /*
    If more than one unique circle at this vertex, the cap is ambiguous.
    Multiple punctured caps are not treated as ambiguous.
    */
    MTGARRAY_VERTEX_LOOP (nId, pGraph, nodeId)
        {
        mask = jmdlMTGGraph_getMask (pGraph, nId, pMasks->circle | pMasks->punctured);
        if (mask & pMasks->circle)
            {
            if (mask & pMasks->punctured)
                {
                if (!bPuncturedCapFound)
                    {
                    nCap++;
                    bPuncturedCapFound = true;
                    }
                }
            else
                {
                nCap++;
                }
            capId = nId;
            }
        }
    MTGARRAY_END_VERTEX_LOOP (nId, pGraph, nodeId)

    if (nCap > 1) return true;

    if (pCapId)
        *pCapId = (nCap == 1) ? capId : MTG_NULL_NODEID;

    return false;
    }


/**
* DEPRECATED
* @bsihdr                                       DavidAssaf      01/00
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGGraph_setMaskAroundMaskedVertex
(
MTGGraph        *pGraph,
MTGNodeId       nodeId,
MTGMask         mask,
MTGMask         queryMask,
int             bState
)
    {
    MTGMask     qm;

    if (!pGraph) return;

    MTGARRAY_VERTEX_LOOP (nId, pGraph, nodeId)
        {
        qm = jmdlMTGGraph_getMask (pGraph, nId, queryMask);
        if ((qm && bState) || (!qm && !bState))
            jmdlMTGGraph_setMask (pGraph, nId, mask);
        }
    MTGARRAY_END_VERTEX_LOOP (nId, pGraph, nodeId)
    }


/**
* DEPRECATED
* @bsihdr                                       DavidAssaf      10/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGGraph_setHalfEdgeMaskAroundSuperFace
(
MTGGraph        *pGraph,
MTGNodeId       nodeId,
MTGMask         mask,
MTGMask         edgeMask,
MTGMask         manifoldMask
)
    {
    // if manifoldMask is boundaryMask, then this is equivalent to:
    jmdlMTGGraph_setFaceMaskAroundMaskedSuperFace (pGraph, nodeId, mask,
        edgeMask, 0, edgeMask, manifoldMask);
    }


/**
* DEPRECATED
* @bsihdr                                       DavidAssaf      10/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlMTGGraph_clearHalfEdgeMaskAroundSuperFace
(
MTGGraph        *pGraph,
MTGNodeId       nodeId,
MTGMask         mask,
MTGMask         edgeMask,
MTGMask         manifoldMask
)
    {
    // if manifoldMask is boundaryMask, then this is equivalent to:
    jmdlMTGGraph_clearFaceMaskAroundMaskedSuperFace (pGraph, nodeId, mask,
        edgeMask, 0, edgeMask, manifoldMask);
    }


/**
* DEPRECATED
* @bsihdr                                       DavidAssaf      10/99
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      jmdlMTGGraph_collectAndNumberHolesInSuperFace
(
MTGFacets               *pFacets,
EmbeddedIntArray        *pHoleIds,
MTGNodeId               *pExtNodeId,
const MTG_ReduceMasks   *pMasks,
MTGNodeId               nodeId
)
    {
    return jmdlMTGFacets_collectAndNumberHolesInSuperFace (pFacets, pHoleIds,
                pExtNodeId, pMasks, nodeId);
    }
END_BENTLEY_GEOMETRY_NAMESPACE
