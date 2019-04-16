/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/*********************************************************************
** This file has two categories of operations, both relating to
** in/out relations among multiple loops.
** The first deals with 'merging' -- detection of intersections
** among preexisting independent loops.
** The second implements boundary parity marking -- setting the
** external bits in loops that are 'external' according to
** alternating in/out parity rules.
**********************************************************************/

/**********************************************************************
** Operations for merging overlapping vu graphs.
**
**
** TERMINOLOGY AND COMMENTS
** raw vu graph -- vu graph in which there may be intersections and
**      containments other than those indicated by the vu structure.
**      For instance, a microstation surface has a collection of unoriented,
**      possibly self-intersecting loops.  This file provides operations to
*       find the intersections and augment the graph structure.
**
** upchain - a subset of a face loop, with each successive node being higher
**      than its predecessor.
**
** Efficiency notes:
**      This algorithm is quite efficient in the usual case where intersections
**      are the exception rather than the rule.  A more detailed statement of this
**      assumption is that there are
**      a moderate number of loops which may have a large number of closely
**      spaced points in long upchains.  Hence a single horizontal line will typically
**      intersect a small number of chains.   If an upward sweep touches each VU
**      O(1) times, and inspects all chains at its horizontal level each time,
**      total time is O(NVU*NC).   Pathological zigzag cases where chains are
**      only 1 vu long will generate NVU*NVU time.
**
** When dealing with segment-segment intersections, the gross check for whether
** segments along a chain have overlapping v values is 'efficient', i.e. tries to
** find REJECT cases using only v comparisons,  Once there is v overlap, further
** search uses normal-direction distances from the 'upper line' to the lower, which
** can also be efficiently updated if the 'lower' chain has many short on the same
** side of the upper one.  When that fails and a full intersection test MUST be done,
** there is a substantial amount of overhead in doing toleranced endpoint-to-endpoint
** comparisons.  Since this is supposed to happen relatively infrequently, there is
** no effort to have the segment-segment computation get access to whatever
** fragmentary computations (e.g. some vector extractions and cross products)
** might already have been done by the higher level sweep.
**
***********************************************************************/

#define VUMERGE_NO_INTERSECTION         0
#define VUMERGE_BASEPOINT_INTERSECTION  1
#define VUMERGE_SIMPLE_INTERSECTION     2


/* The squared sine of an angle that is large enought to assure reasonable
   numerics.  Here take theta= 0.1 radians = 5.6 degrees */
#define SQUARED_SINE_OF_NONZERO_ANGLE 0.0001
#define QUICK_OUT_F1  (-0.001)
#define QUICK_OUT_F2  (0.001)
#define QUICK_OUT_F3  (0.999)
#define QUICK_OUT_F4  (1.001)

#define CROSS(A,B) ( A.x*B.y - A.y*B.x)
#define DOT(A,B)   ( A.x*B.x + A.y*B.y)
#define MAG2(A)    ( A.x*A.x + A.y*A.y)

/* Test doubled edge bit.  Assumes mpP var is defined in context */
#define NONDOUBLED_EDGE( edgeP ) ( !VU_GETMASK ( edgeP, mpP->mDoubledEdge ) )
#define DOUBLED_EDGE( edgeP ) ( VU_GETMASK ( edgeP, mpP->mDoubledEdge ) )


struct StackFrame
    {
    VuP m_nodeP;
    int m_exterior;
    StackFrame(VuP nodeP, int exterior)
        {
        m_nodeP = nodeP;
        m_exterior = exterior;
        }
    StackFrame ()
        {
        m_nodeP = NULL;
        m_exterior = 0;
        }
    };
static size_t s_maxInteriorDepth = 0;
static size_t s_maxExteriorDepth = 0;
/*----------------------------------------------------------------------+
|                                                                       |
| name          vu_spreadBoundaryParityWithinBoundaryRegion             |
|                                                                       |
| author        earlinLutz                                      10/94   |
|                                                                       |
| Depth first search to spread boundary parity to all faces reachable   |
| from seedP without crossing boundary edges.                           |
|                                                                       |
+----------------------------------------------------------------------*/
static void vu_spreadBoundaryParityWithinBoundaryRegion
(
VuP seedP,      /* => Start node for search. */
int exterior,   /* => 1 to set the exterior bit ON, 0 to set the exterior bit OFF */
VuMask *mPass0
)
    {
    VuP mateP;

    bvector<StackFrame> stack;
    stack.push_back (StackFrame(seedP, exterior));
    while (stack.size () > 0)
        {
        if (stack.size () > s_maxInteriorDepth)
            s_maxInteriorDepth = stack.size ();
        StackFrame frame = stack[stack.size () - 1];
        seedP = frame.m_nodeP;
        exterior = frame.m_exterior;
        stack.resize (stack.size () - 1);
        if (VU_GETMASK (seedP, *mPass0))
            continue;
    /* Mark this face */
    VU_FACE_LOOP(currP,seedP)
        {
        VU_WRITEMASK(currP,VU_EXTERIOR_EDGE,exterior);
        VU_SETMASK(currP,*mPass0);
        }
    END_VU_FACE_LOOP(currP,seedP)

    /* Recurse across nonboundary edges to unmarked neighbors */
    VU_FACE_LOOP(currP,seedP)
        {
        if(!VU_GETMASK(currP,VU_BOUNDARY_EDGE))
            {
            mateP = VU_EDGE_MATE(currP);
            if(!VU_GETMASK(mateP,*mPass0))
                {
                    stack.push_back (StackFrame (mateP, exterior));
                }
            }
        }
    END_VU_FACE_LOOP(currP,seedP)
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vu_spreadBoundaryParityOutsideBoundaryRegion            |
|                                                                       |
| author        earlinLutz                                      10/94   |
|                                                                       |
| Depth first search to spread boundary parity to all faces reachable   |
| from seedP, including those across boundary edges (in which case      |
| the parity flips as the boundary is crossed).                         |
|                                                                       |
+----------------------------------------------------------------------*/
static void vu_spreadBoundaryParityOutsideBoundaryRegion
(
VuP seedP,      /* => Start node for search. */
int exterior,   /* => 1 to set the exterior bit ON, 0 to set the exterior bit OFF */
VuMask *mPass0,
VuMask *mPass1
)
    {
    VuP mateP;
    if(!(*mPass0 && *mPass1)) return;


    bvector<StackFrame> stack;
    stack.push_back (StackFrame(seedP, exterior));
    while (stack.size () > 0)
        {
        if (stack.size () > s_maxExteriorDepth)
            s_maxExteriorDepth = stack.size ();
        StackFrame frame = stack[stack.size () - 1];
        seedP = frame.m_nodeP;
        exterior = frame.m_exterior;
        stack.resize (stack.size () - 1);
        vu_setMaskAroundFace (seedP, *mPass1);


    /* Recurse across nonboundary edges to unmarked neighbors */
    VU_FACE_LOOP(currP,seedP)
        {
        mateP = VU_EDGE_MATE(currP);
        if(VU_GETMASK(currP,VU_BOUNDARY_EDGE))
            {
            if(!VU_GETMASK(mateP,*mPass0))
                {/* Cross into the fresh adjacent region. */
                vu_spreadBoundaryParityWithinBoundaryRegion (mateP, exterior, mPass0);
                    stack.push_back (StackFrame (mateP, !exterior));
                    //vu_spreadBoundaryParityOutsideBoundaryRegion(mateP,!exterior, mPass0, mPass1);
                }
            }
        else
            {
            if(!VU_GETMASK(mateP,*mPass1))
                {/* Countinue looking for ways out of the current region */
                    stack.push_back (StackFrame (mateP, exterior));
                    //vu_spreadBoundaryParityOutsideBoundaryRegion(mateP,exterior, mPass0, mPass1);
                }
            }
        }
    END_VU_FACE_LOOP(currP,seedP)
        }
    }


/*======================================================================+
|                                                                       |
|   Major Public Code Section                                           |
|                                                                       |
+======================================================================*/
/*---------------------------------------------------------------------------------**//**
* @description Perform a merge operation on (disjoint) face loops in the graph, with duplicate edges culled using the parity rule.
* @remarks The graph may have intersections and containments other than those indicated by explicit graph topology.
*       This function finds the intersections/containments using the given rule and augments the graph structure accordingly.
* @remarks This function uses the graph tolerances set by ~mvu_setTol.
* @remarks This is a shortcut for calling ~mvu_merge2002 with ~tVuMergeType VUUNION_PARITY and with tolerances extracted by ~mvu_getTol.
* @param graphP     IN OUT  graph header
* @group "VU Booleans"
* @bsimethod                                                    EarlinLutz      10/94
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_mergeLoops
(
VuSetP graphP
)
    {
    vu_mergeOrUnionLoops (graphP, VUUNION_PARITY);
    }

/*---------------------------------------------------------------------------------**//**
* @description Set the VU_EXTERIOR_EDGE mask on the edges of each face of the graph that has negative area.
* @remarks First the VU_EXTERIOR_EDGE mask is cleared throughout the graph (~mvu_clearMaskInSet).
* @param graphP                 IN OUT  graph header
* @param suppressTrivialFaces   IN      whether to ignore faces with less than three edges
* @group "VU Node Masks"
* @see vu_setMaskByArea, vu_markFaceAndComputeArea
* @bsimethod                                                    EarlinLutz      10/94
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_markExteriorFacesByArea
(
VuSetP  graphP,
bool    suppressTrivialFaces
)
    {
    double area;
    VuMask visitedMask = vu_grabMask ( graphP );
    vu_clearMaskInSet( graphP, visitedMask | VU_EXTERIOR_EDGE);
    VU_SET_LOOP ( currP, graphP )
        {
        if ( ! VU_GETMASK( currP, visitedMask ) )
            {
            /* Here we are in a new face */
            area = vu_markFaceAndComputeArea ( currP, visitedMask );
            if ( area <= 0.0
                && ! (suppressTrivialFaces && VU_FSUCC( VU_FSUCC( currP) ) == currP )
                )
                vu_markFace ( currP, VU_EXTERIOR_EDGE );
            }
        }
    END_VU_SET_LOOP (currP, graphP )
    vu_returnMask ( graphP , visitedMask );
    }

/*----------------------------------------------------------------------+
@description Linearly search the array.
        Remove faces with mask.
        Among unmasked faces, choose the one with min area.  Remove it from the array and return it.
@param arrayP IN array to search
@param mask IN mask for faces to remove immediately
@return pointer with smallest signed area.
@bsimethod                      EarlinLutz          02/04
+----------------------------------------------------------------------*/
static VuP chooseAndRemoveMinAreaFace
(
VuArrayP arrayP,
VuMask mask

)
    {
    double currArea, minArea;
    VuP minP, currP;
    int i, minIndex;
    minP = NULL;
    minArea = FLT_MAX;

    for (i = 0, minIndex = -1; NULL != (currP = vu_arrayGetVuP (arrayP, i)); i++)
        {
        if (vu_getMask (currP, mask))
            {
            vu_arrayRemoveCurrent (arrayP);
            }
        else
            {
            currArea = vu_area (currP);
            if (!minP || currArea < minArea)
                {
                minArea = currArea;
                minP = currP;
                minIndex = i;
                }
            }
        }

    if (NULL != minP)
        {
        currP = vu_arrayRemoveLast (arrayP);
        if (currP != minP)
            vu_arrayReplaceVuP (arrayP, minIndex, currP);
        }
    return minP;
    }

/*---------------------------------------------------------------------------------**//**
* @description Set VU_EXTERIOR_EDGE masks throughout the graph using boundary-crossing parity.
* @remarks Assumed initial condition: some subset of edges has VU_EXTERIOR_EDGE set, while various other
*       edges have VU_BOUNDARY_EDGE mask set.
* @remarks The algorithm proceeds by flood search:
*       <ul>
*       <li>set the exterior mask on edges reachable from an exterior edge without crossing any boundary edge</li>
*       <li>clear the exterior mask on the other side of those boundary edges</li>
*       </ul>
* @param graphP                 IN OUT  graph header
* @param checkExteriorMarks     IN      whether to reset exterior masks (~mvu_markExteriorFacesByArea) beforehand
* @group "VU Node Masks"
* @bsimethod                                                    EarlinLutz      10/94
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vu_markAlternatingExteriorBoundaries
(
VuSetP graphP,
int checkExteriorMarks
)
    {

    VuMask mPass0;      /* Mask bit to identify faces that have been
                                reached in first pass of parity marking */
    VuMask mPass1;      /* Mask bit to identify faces that have been
                                reached in second pass of parity marking */
    VuArrayP candidateArrayP = vu_grabArray(graphP);
    VuP seedP;
    static bool    s_suppressTrivialExteriorFaces = true;


    if( checkExteriorMarks)
        vu_markExteriorFacesByArea(graphP,s_suppressTrivialExteriorFaces);

    mPass0 = vu_grabMask(graphP);
    mPass1 = vu_grabMask(graphP);

    vu_clearMaskInSet(graphP,mPass0 | mPass1);
    vu_collectExteriorFaceLoops (candidateArrayP, graphP);
    // Flood search from negative area seed faces.
    // In normal usage, a connected graph has a single outer negative face, and
    // the first pass marks the entire graph.   In a multi-component graph,
    // the search for min area faces could recompute areas.
    while (NULL != (seedP = chooseAndRemoveMinAreaFace (candidateArrayP, mPass0)))
        {
        if(VU_GETMASK(seedP,VU_EXTERIOR_EDGE) && !VU_GETMASK(seedP,mPass0))
            {
            vu_spreadBoundaryParityWithinBoundaryRegion(seedP,true, &mPass0);
            vu_spreadBoundaryParityOutsideBoundaryRegion(seedP,false, &mPass0, &mPass1);
            }
        }

    vu_returnMask(graphP,mPass0);
    vu_returnMask(graphP,mPass1);

    vu_returnArray (graphP, candidateArrayP);
    }

END_BENTLEY_GEOMETRY_NAMESPACE
