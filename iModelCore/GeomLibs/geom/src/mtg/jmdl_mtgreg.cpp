/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/mtg/jmdl_mtgreg.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "../memory/jmdl_iarray.fdf"
#include "../memory/jmdl_dpnt3.fdf"
#include "../bspline/sortutil.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/*----------------------------------------------------------------------+
|                                                                       |
|   Local type definitions                                              |
|                                                                       |
+----------------------------------------------------------------------*/

typedef struct
    {
    MTGGraph        *pGraph;
    EmbeddedDPoint3dArray *pCoordinateArray;
    bool            reverseCoordinates;
    int             vertexLabelOffset;
    int             errors;


    MTGMask         upEdgeMask;       /* During initialization, each edge is marked as
                                                up or down.  Upward scans then use this
                                                to recognize peaks */
    MTGMask         downwardExtremumMask; /* Distinguishes down mins from up mins in pCoordinateState->pMinArray */

    EmbeddedIntArray      *pMinArray;   /* Array of vu's which are minima eventually sorted by (lexical) y */

    EmbeddedIntArray      *pLeftArray;  /* lower vu on left-side edges */
    EmbeddedIntArray      *pRightArray; /* lower vu on right side edges */
    EmbeddedIntArray      *pPeakArray;  /* active peaks */

    } CoordinateState;

/*----------------------------------------------------------------------+
|                                                                       |
|   Private Global variables                                            |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   Public Global variables                                             |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   External variables                                                  |
|                                                                       |
+----------------------------------------------------------------------*/

/*======================================================================+
|                                                                       |
|   Private Utility Routines                                            |
|                                                                       |
+======================================================================*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGReg_openCoordinateQueries                        |
|                                                                       |
| author        earlinLutz                                      10/99   |
|                                                                       |
| Init a coordinate state for coordinate-only queries.                  |
+----------------------------------------------------------------------*/
static bool        jmdlMTGReg_openCoordinateQueries
(
CoordinateState *pCoordinateState,
MTGGraph            *pGraph,
int                 vertexLabelOffset,
EmbeddedDPoint3dArray     *pCoordinateArray,
bool                reverseCoordinates
)
    {
    pCoordinateState->pGraph = pGraph;

    pCoordinateState->vertexLabelOffset = vertexLabelOffset;

    pCoordinateState->reverseCoordinates = reverseCoordinates;
    pCoordinateState->pCoordinateArray = pCoordinateArray;

    /* Zero out everything else */
    pCoordinateState->upEdgeMask = MTG_NULL_MASK;
    pCoordinateState->downwardExtremumMask = MTG_NULL_MASK;

    pCoordinateState->pMinArray    = NULL;
    pCoordinateState->pRightArray  = NULL;
    pCoordinateState->pLeftArray   = NULL;
    pCoordinateState->pPeakArray   = NULL;

    pCoordinateState->errors = 0;
    return true;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGReg_closeCoordinateQueries                       |
|                                                                       |
| author        earlinLutz                                      10/99   |
|                                                                       |
+----------------------------------------------------------------------*/
static bool        jmdlMTGReg_closeCoordinateQueries
(
CoordinateState *pCoordinateState
)
    {
    return true;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGReg_below                                        |
|                                                                       |
| author        earlinLutz                                      10/99   |
|                                                                       |
+----------------------------------------------------------------------*/
static void        jmdlMTGReg_panic
(
CoordinateState *pCoordinateState
)
    {
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGReg_getUV                                        |
|                                                                       |
| author        earlinLutz                                      10/99   |
|                                                                       |
+----------------------------------------------------------------------*/
static     bool    jmdlMTGReg_getUV
(
CoordinateState *pCoordinateState,
double              *pU,
double              *pV,
int                 *pVertexIndex,
MTGNodeId           nodeId
)
    {
    DPoint3d xyz;
    if (jmdlMTGGraph_getLabel (pCoordinateState->pGraph, pVertexIndex, nodeId, pCoordinateState->vertexLabelOffset)
        && jmdlEmbeddedDPoint3dArray_getDPoint3d (pCoordinateState->pCoordinateArray, &xyz, *pVertexIndex))
        {
        if (pCoordinateState->reverseCoordinates)
            {
            *pU = -xyz.x;
            *pV = -xyz.y;
            }
        else
            {
            *pU = xyz.x;
            *pV = xyz.y;
            }
        return true;
        }
    jmdlMTGReg_panic (pCoordinateState);
    return false;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGReg_getU                                         |
|                                                                       |
| author        earlinLutz                                      10/99   |
|                                                                       |
+----------------------------------------------------------------------*/
static     double  jmdlMTGReg_getU
(
CoordinateState *pCoordinateState,
MTGNodeId           nodeId
)
    {
    double u, v;
    int index;
    if (jmdlMTGReg_getUV (pCoordinateState, &u, &v, &index, nodeId))
        {
        return u;
        }
    return 0.0;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGReg_getV                                         |
|                                                                       |
| author        earlinLutz                                      10/99   |
|                                                                       |
+----------------------------------------------------------------------*/
static     double  jmdlMTGReg_getV
(
CoordinateState *pCoordinateState,
MTGNodeId           nodeId
)
    {
    double u, v;
    int index;
    if (jmdlMTGReg_getUV (pCoordinateState, &u, &v, &index, nodeId))
        {
        return v;
        }
    return 0.0;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGReg_below                                        |
|                                                                       |
| author        earlinLutz                                      10/99   |
|                                                                       |
+----------------------------------------------------------------------*/
static bool        jmdlMTGReg_below
(
CoordinateState *pCoordinateState,
MTGNodeId           node0Id,
MTGNodeId           node1Id
)
    {
    double      u0, v0, u1, v1;
    MTGNodeId   index0, index1;
    if (   jmdlMTGReg_getUV (pCoordinateState, &u0, &v0, &index0, node0Id)
        && jmdlMTGReg_getUV (pCoordinateState, &u1, &v1, &index1, node1Id))
        {
        if (v0 < v1)
            return true;
        if (v1 < v0)
            return false;
        if (u0 < u1)
            return true;
        return false;
        }
    return false;
    }
#ifdef CompileExtraCallbacks
/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGReg_below                                        |
|                                                                       |
| author        earlinLutz                                      10/99   |
|                                                                       |
+----------------------------------------------------------------------*/
static bool        jmdlMTGReg_coordinateBelowNode
(
CoordinateState *pCoordinateState,
double              u0,
double              v0,
MTGNodeId           node1Id
)
    {
    double      u1, v1;
    MTGNodeId   index1;
    if (   jmdlMTGReg_getUV (pCoordinateState, &u1, &v1, &index1, node1Id))
        {
        if (v0 < v1)
            return true;
        if (v1 < v0)
            return false;
        if (u0 < u1)
            return true;
        return false;
        }
    return false;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGReg_below                                        |
|                                                                       |
| author        earlinLutz                                      10/99   |
|                                                                       |
+----------------------------------------------------------------------*/
static bool        jmdlMTGReg_nodeBelowCoordinate
(
CoordinateState *pCoordinateState,
MTGNodeId           node0Id,
double              u1,
double              v1
)
    {
    double      u0, v0;
    MTGNodeId   index0;
    if (   jmdlMTGReg_getUV (pCoordinateState, &u0, &v0, &index0, node0Id))
        {
        if (v0 < v1)
            return true;
        if (v1 < v0)
            return false;
        if (u0 < u1)
            return true;
        return false;
        }
    return false;
    }
#endif

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGReg_cross                                        |
|                                                                       |
| author        earlinLutz                                      10/99   |
|                                                                       |
+----------------------------------------------------------------------*/
static double      jmdlMTGReg_cross
(
CoordinateState *pCoordinateState,
MTGNodeId           node0Id,
MTGNodeId           node1Id,
MTGNodeId           node2Id
)
    {
    double          u0, u1, u2, v0, v1, v2;
    double          du0, du1, dv0, dv1;
    int index0, index1, index2;
    if (   jmdlMTGReg_getUV (pCoordinateState, &u0, &v0, &index0, node0Id)
        && jmdlMTGReg_getUV (pCoordinateState, &u1, &v1, &index1, node1Id)
        && jmdlMTGReg_getUV (pCoordinateState, &u2, &v2, &index2, node2Id)
        )
        {
        du0 = u1 - u0;
        dv0 = v1 - v0;
        du1 = u2 - u1;
        dv1 = v2 - v1;
        return du0 * dv1 - du1 * dv0;
        }
    return 0.0;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGReg_nodeInSector                                 |
|                                                                       |
| author        earlinLutz                                      10/99   |
|                                                                       |
+----------------------------------------------------------------------*/
static bool        jmdlMTGReg_nodeInSector
(
CoordinateState *pCoordinateState,
MTGNodeId           nodeId,
MTGNodeId           sectorNodeId
)
    {
    MTGGraph *pGraph = pCoordinateState->pGraph;

    MTGNodeId   succNodeId = jmdlMTGGraph_getFSucc (pGraph, sectorNodeId);
    MTGNodeId   predNodeId = jmdlMTGGraph_getFPred (pGraph, sectorNodeId);
    double      sectorCross;

    double      succCross   = jmdlMTGReg_cross (pCoordinateState, sectorNodeId, succNodeId,   nodeId);
    double      predCross   = jmdlMTGReg_cross (pCoordinateState, predNodeId,   sectorNodeId, nodeId);

    bool        inSector;

    if (jmdlMTGGraph_getVSucc (pCoordinateState->pGraph, sectorNodeId) == sectorNodeId)
        {
        /* Special case: The sector is 180 degrees. Call it in.
           (Is this safe?  I think so.  The callers should have
           prechecked the ON cases, (I think.)  Everything else is in.
         */
        inSector = true;
        }
    /* Use pred and succ cross products alone first -- this gives
        the right answer on 180 degree sectors without ever looking
        at the sector cross product (which is zero or nearly so)
    */
    else if ( predCross > 0.0 && succCross > 0.0 )
        inSector = true;
    else if (predCross <= 0.0 && succCross <= 0.0 )
        inSector = false;
    /* Signs are mixed */
    else
        {
        sectorCross = jmdlMTGReg_cross (pCoordinateState, predNodeId, sectorNodeId, succNodeId);
        inSector = (sectorCross < 0.0);
        }

    return inSector;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGReg_compareLexicalUV                             |
|                                                                       |
| author        earlinLutz                                      10/99   |
|                                                                       |
+----------------------------------------------------------------------*/
static int     jmdlMTGReg_compareLexicalUV
(
CoordinateState *pCoordinateState,
MTGNodeId           node0Id,
MTGNodeId           node1Id
)
    {
    double      u0, v0, u1, v1;
    MTGNodeId   index0, index1;

    if (   jmdlMTGReg_getUV (pCoordinateState, &u0, &v0, &index0, node0Id)
        && jmdlMTGReg_getUV (pCoordinateState, &u1, &v1, &index1, node1Id))
        {
        if (v0 < v1)
            return -1;

        if (v1 < v0)
            return 1;

        if (u0 < u1)
            return -1;

        if (u1 < u0)
            return 1;
        return 0;
        }
    return 0;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGReg_markUpEdges                                  |
|                                                                       |
| author        earlinLutz                                      10/99   |
|                                                                       |
+----------------------------------------------------------------------*/
static void jmdlMTGReg_markUpEdges
(
CoordinateState *pCoordinateState,
MTGMask mask            /* => mask to install in each upedge ( and clear in downedge ) */
)
    {
    MTGGraph *pGraph = pCoordinateState->pGraph;

    MTGNodeId nextP;
    MTGARRAY_SET_LOOP( currNodeId, pGraph )
        {
        nextP = jmdlMTGGraph_getFSucc (pGraph, currNodeId );
        if ( jmdlMTGReg_below (pCoordinateState,  currNodeId, nextP ) )
            {
            jmdlMTGGraph_setMask (pGraph, currNodeId, mask );
            }
        else
            {
            jmdlMTGGraph_clearMask (pGraph, currNodeId, mask );
            }
        }
    MTGARRAY_END_SET_LOOP( currNodeId, pGraph )
    }

typedef struct
    {
    CoordinateState *pCoordinateState;
    MTGMask mask;
    } MTGReg_MaskedSortContext;
/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGReg_compareLexicalUVWithMask                     |
|                                                                       |
| author        earlinLutz                                      10/99   |
|                                                                       |
| Lexical comparison in parameter space, resolve ties making masked     |
| nodes below unmasked.                                                 |
+----------------------------------------------------------------------*/
static int jmdlMTGReg_compareLexicalUVWithMask
(
const MTGNodeId * pNode0Id,
const MTGNodeId * pNode1Id,
const MTGReg_MaskedSortContext    *pContext
)
    {
    CoordinateState *pCoordinateState = pContext->pCoordinateState;
    MTGGraph  *pGraph  = pCoordinateState->pGraph;

    int result = jmdlMTGReg_compareLexicalUV(pCoordinateState, *pNode0Id, *pNode1Id);

    if ( ( result == 0 ) && (*pNode0Id != *pNode1Id) )
        {
        int mask = pContext->mask;
        if (jmdlMTGGraph_getMask (pGraph, *pNode0Id, mask) )
            return -1;
        if (jmdlMTGGraph_getMask (pGraph, *pNode1Id, mask) )
            return 1;
        }
    return result;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGReg_sortMasked.                                  |
|                                                                       |
| author        earlinLutz                                      10/99   |
|                                                                       |
| Sort the node indices in the int array.  Use simple lexical           |
| first, then look for indicated mask to break ties -- masked node      |
| precedes unmasked.                                                    |
+----------------------------------------------------------------------*/
static void jmdlMTGReg_sortMasked
(
CoordinateState *pCoordinateState,
EmbeddedIntArray            *pIntArray,
MTGMask             tieBreakMask
)
    {
    MTGReg_MaskedSortContext context;


    int num = jmdlEmbeddedIntArray_getCount (pIntArray);
    int *pBuffer = jmdlEmbeddedIntArray_getPtr (pIntArray, 0);

    context.pCoordinateState = pCoordinateState;
    context.mask      = tieBreakMask;

    mdlUtil_dlmQuickSort
                    (
                    pBuffer,
                    pBuffer + num - 1,
                    sizeof (int),
                    (PFToolsSortCompare)jmdlMTGReg_compareLexicalUVWithMask,
                    &context
                    );
    }
/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGReg_collectAndMarkLocalMinimaInMarkedGraph       |
|                                                                       |
| author        earlinLutz                                      10/99   |
|                                                                       |
+----------------------------------------------------------------------*/
static int jmdlMTGReg_collectAndMarkLocalMinimaInMarkedGraph
(
CoordinateState *pCoordinateState
)
    {
    MTGGraph *pGraph = pCoordinateState->pGraph;

    MTGNodeId nextP, secondP;
    MTGMask upEdgeMask = pCoordinateState->upEdgeMask;
    double cross;
    int count = 0;

    jmdlEmbeddedIntArray_empty( pCoordinateState->pMinArray );

    MTGARRAY_SET_LOOP(currNodeId, pGraph )
        {
        nextP = jmdlMTGGraph_getFSucc (pGraph, currNodeId );
        if  (  !jmdlMTGGraph_getMask (pGraph, currNodeId, upEdgeMask)
            &&  jmdlMTGGraph_getMask (pGraph, nextP,      upEdgeMask)
            )
            {
            /* This is a min. Record it and set the up/down bit */
            secondP = jmdlMTGGraph_getFSucc (pGraph,nextP);
            cross = jmdlMTGReg_cross (pCoordinateState, currNodeId, nextP, secondP );
            jmdlEmbeddedIntArray_addInt( pCoordinateState->pMinArray , nextP );
            if ( jmdlMTGGraph_getVSucc (pGraph,nextP) == nextP ||  cross <= 0.0 )
                {
                jmdlMTGGraph_setMask (pGraph, nextP, pCoordinateState->downwardExtremumMask );
                }
            }
        else if (   jmdlMTGGraph_getMask (pGraph, currNodeId, upEdgeMask)
                && !jmdlMTGGraph_getMask (pGraph, nextP, upEdgeMask )
                )
            {
            /* This is a max. Set the up/down bit */
            secondP = jmdlMTGGraph_getFSucc (pGraph, nextP);
            cross = jmdlMTGReg_cross (pCoordinateState, currNodeId, nextP, secondP );
            if ( jmdlMTGGraph_getVSucc (pGraph, nextP) != nextP && cross > 0.0 )
                {
                jmdlMTGGraph_setMask (pGraph, nextP, pCoordinateState->downwardExtremumMask );
                }
            }
        }
    MTGARRAY_END_SET_LOOP (currNodeId, pGraph);

    jmdlMTGReg_sortMasked (
                pCoordinateState,
                pCoordinateState->pMinArray,
                pCoordinateState->downwardExtremumMask
                );
    return count;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGReg_pinpointCrossing                                     |
|                                                                       |
| author        earlinLutz                                      10/99   |
|                                                                       |
+----------------------------------------------------------------------*/
static void jmdlMTGReg_pinpointCrossing
(
CoordinateState *pCoordinateState,
double *uCrossP,        /* <= u coordinate where v crosses the edge */
MTGNodeId bottomP,              /* => VU at bottom of edge */
MTGNodeId topP,         /* => VU at top of edge */
double v,               /* => v of scan line */
double uError           /* => u to return if the edge is horizontal */
)
    {

    double v0 = jmdlMTGReg_getV (pCoordinateState, bottomP );
    double v1 = jmdlMTGReg_getV (pCoordinateState, topP );
    double dv = v1 - v0;

    if ( dv == 0.0 )
        {
        /* This is not supposed to happen. I'll return the error value just
                to be complete */
        *uCrossP = uError;
        }
    else
        {
        double s = ( v - v0 ) / dv;
        double u0 = jmdlMTGReg_getU (pCoordinateState, bottomP );
        double u1 = jmdlMTGReg_getU (pCoordinateState, topP );
        /* Interpolate with the nearer end as the 0 point.  */
        if ( s < 0.5 )
            {
            *uCrossP = u0 + s * (u1 - u0);
            }
        else
            {
            s = ( v1 - v ) / dv;
            *uCrossP = u1 + s * (u0 - u1);
            }
        }
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGReg_compareEdgesHorizontally                     |
|                                                                       |
| author        earlinLutz                                      10/99   |
|                                                                       |
| Test if the edge base0P..top0P is left/at/right of the edge           |
| base1P..top1P, using the midpoint of their common y range as the      |
| primary comparison point.                                             |
| If they share no y range, use vertical comparison.                    |
|                                                                       |
+----------------------------------------------------------------------*/
static int     jmdlMTGReg_compareEdgesHorizontally
(
CoordinateState     *pCoordinateState,
MTGNodeId               base0P,         /* => base of first edge */
MTGNodeId               top0P,          /* => top of first edge */
MTGNodeId               base1P,         /* => base of second edge */
MTGNodeId               top1P           /* => top of second edge */
)
    {
    double vmin, vmax;
    int result = 0;

    double u0, u1, v0, v1, vmid;

    vmin = jmdlMTGReg_getV (pCoordinateState, base0P);
    if (jmdlMTGReg_getV (pCoordinateState,base1P) > vmin)
        vmin = jmdlMTGReg_getV (pCoordinateState,base1P);

    vmax = jmdlMTGReg_getV (pCoordinateState,top0P);
    if (jmdlMTGReg_getV (pCoordinateState,top1P) < vmax)
        vmax = jmdlMTGReg_getV (pCoordinateState,top1P);

    if (vmax < vmin)
        {
        /* There is no horizontal overlap.   Use vertical relationship */
        v0 = jmdlMTGReg_getV (pCoordinateState, base0P);
        v1 = jmdlMTGReg_getV (pCoordinateState, base1P);
        if (v0 < v1)
            result = -1;
        else if(v1 < v0)
            result = 1;
        }
    else
        {
        vmid = 0.5 * (vmin + vmax);
        jmdlMTGReg_pinpointCrossing (pCoordinateState, &u0, base0P, top0P, vmid,
                        0.5*(jmdlMTGReg_getU (pCoordinateState,base0P) + jmdlMTGReg_getU (pCoordinateState,top0P)));
        jmdlMTGReg_pinpointCrossing (pCoordinateState, &u1, base1P, top1P, vmid,
                        0.5*(jmdlMTGReg_getU (pCoordinateState,base1P) + jmdlMTGReg_getU (pCoordinateState,top1P)));
        if (u0 < u1)
            {
            result = -1;
            }
        else if (u0 > u1)
            {
            result = 1;
            }
        }
    return result;
    }


/*----------------------------------------------------------------------+
| Invariants:                                                           |
|  ** Upedges are marked with pCoordinateState->upEdgeMark                              |
|  ** Every edge in pRightArray is an upedge.                           |
|  ** The FPRED of every edge in pLeftArray is NOT an upedge            |
|  ** Peaks are ultimately reached by both a left and a right chain.    |
|       Record them into the pPeakArray when reached by a right edge    |
|  ** When drawing on an increasing sequence of mins, the bases of      |
|       all active left and right chains are below all unprocessed mins |
|  ** An upward min generates a new left and a new right chain          |
|  ** When a downward min is inserted between a left and right chain:   |
|       *** all peaks between those chains can die.                     |
|       *** the two edges at the peak are new bases (one right, one     |
|               left)                                                   |
+----------------------------------------------------------------------*/
/*PF*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGReg_advanceRightEdgesToMin                               |
|                                                                       |
| author        earlinLutz                                      10/99   |
|                                                                       |
+----------------------------------------------------------------------*/
static void jmdlMTGReg_advanceRightEdgesToMin
(
CoordinateState*        pCoordinateState,
MTGNodeId               *pFirstRightNodeId, /* <= (possibly null) first edge encountered
                                                by rightward ray from minNodeId */
DPoint2d                *uvP,           /* <= (possibly unset) uv at right intersect */
MTGNodeId               minNodeId       /* => vu of scan line */
)
    {
    MTGGraph *pGraph = pCoordinateState->pGraph;

    MTGNodeId rightNodeId, nextRightNodeId;
    EmbeddedIntArray * pRightArray = pCoordinateState->pRightArray;
    MTGNodeId firstRightNodeId;
    double uCross, uFirst = DBL_MAX;
    double vScan,uMin;
    double uDefault;
    int    i;

    vScan = jmdlMTGReg_getV (pCoordinateState, minNodeId);
    uMin  = jmdlMTGReg_getU (pCoordinateState, minNodeId);
    firstRightNodeId = MTG_NULL_NODEID;
    /* For each right chain ... */
    for ( i = 0; jmdlEmbeddedIntArray_getInt (pRightArray, &rightNodeId, i); i++)
        {
        nextRightNodeId = jmdlMTGGraph_getFSucc (pGraph,rightNodeId);
        /* Move up until the chain either tops out or passes the min */
        while ( jmdlMTGReg_below (pCoordinateState,  nextRightNodeId, minNodeId ) && jmdlMTGGraph_getMask (pGraph, nextRightNodeId, pCoordinateState->upEdgeMask) )
            {
            rightNodeId = nextRightNodeId;
            nextRightNodeId = jmdlMTGGraph_getFSucc (pGraph,nextRightNodeId);
            }

        if (jmdlMTGReg_below (pCoordinateState,  nextRightNodeId, minNodeId))
            {
            /* nextRightNodeId is a peak.
                Take the chain out of the active array; if the peak is upward, record it */
            if (!jmdlMTGGraph_getMask (pGraph, nextRightNodeId, pCoordinateState->downwardExtremumMask))
                jmdlEmbeddedIntArray_addInt (pCoordinateState->pPeakArray, nextRightNodeId);

            jmdlVArrayInt_replaceByLast (pCoordinateState->pRightArray, i);
            i--;
            }
        else
            {
            /* rightNodeId .. nextRightNodeId brackets the scanline */
            uDefault = jmdlMTGReg_getU (pCoordinateState,rightNodeId);
            jmdlMTGReg_pinpointCrossing (pCoordinateState, &uCross, rightNodeId, nextRightNodeId, vScan, uDefault);
            jmdlVArrayInt_set (pRightArray, rightNodeId, i);     // WAS: "replace current"
            if  (
                     uMin < uCross
                &&
                    (
                        MTG_NULL_NODEID == firstRightNodeId
                    ||  -1 == jmdlMTGReg_compareEdgesHorizontally
                                (
                                pCoordinateState,
                                rightNodeId,
                                nextRightNodeId,
                                firstRightNodeId,
                                jmdlMTGGraph_getFSucc (pGraph, firstRightNodeId)
                                )
                    )
                )
                {
                /* This is nearer than the previous best */
                uFirst = uCross;
                firstRightNodeId = rightNodeId;
                }
            }
        }

    if (firstRightNodeId != MTG_NULL_NODEID)
        {
        *pFirstRightNodeId = firstRightNodeId;
        uvP->x = uFirst;
        uvP->y = vScan;
        }
    else
        {
        *pFirstRightNodeId = MTG_NULL_NODEID;
        }
    }

/*PF*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGReg_advanceLeftEdgesToMin                                |
|                                                                       |
| author        earlinLutz                                      10/99   |
|                                                                       |
+----------------------------------------------------------------------*/
static void jmdlMTGReg_advanceLeftEdgesToMin
(
CoordinateState*        pCoordinateState,
MTGNodeId               *pFirstLeftNodeId, /* <= (possibly null) first edge encountered
                                                by leftward ray from minNodeId */
DPoint2d*               uvP,          /* <= (possibly unset) uv at left intersect */
MTGNodeId               minNodeId          /* => vu of scan line */
)
    {
    MTGGraph *pGraph = pCoordinateState->pGraph;

    MTGNodeId leftNodeId, nextLeftNodeId, farLeftNodeId;
    EmbeddedIntArray * pLeftArray = pCoordinateState->pLeftArray;
    MTGNodeId firstLeftNodeId;
    double uCross, uFirst = DBL_MAX;
    double vScan,uMin;
    double uDefault;
    int     i;

    vScan = jmdlMTGReg_getV (pCoordinateState, minNodeId);
    uMin  = jmdlMTGReg_getU (pCoordinateState, minNodeId);
    firstLeftNodeId = MTG_NULL_NODEID;
    /* For each left chain ... */
    for ( i = 0; jmdlEmbeddedIntArray_getInt (pLeftArray, &leftNodeId, i); i++)
        {
        nextLeftNodeId = jmdlMTGGraph_getFPred (pGraph,leftNodeId);
        farLeftNodeId = jmdlMTGGraph_getFPred (pGraph,nextLeftNodeId);
        /* Move up until the chain either tops out or passes the min */
        while ( jmdlMTGReg_below (pCoordinateState,  nextLeftNodeId, minNodeId ) && !jmdlMTGGraph_getMask (pGraph, farLeftNodeId, pCoordinateState->upEdgeMask) )
            {
            leftNodeId = nextLeftNodeId;
            nextLeftNodeId = jmdlMTGGraph_getFPred (pGraph,nextLeftNodeId);
            farLeftNodeId  = jmdlMTGGraph_getFPred (pGraph,nextLeftNodeId);
            }

        if (jmdlMTGReg_below (pCoordinateState,  nextLeftNodeId, minNodeId))
            {
            /* nextLeftNodeId is a peak. Take the chain out of the active array */
            jmdlVArrayInt_replaceByLast (pCoordinateState->pLeftArray, i);
            i--;
            }
        else
            {
            /* leftNodeId .. nextLeftNodeId brackets the scanline */
            uDefault = jmdlMTGReg_getU (pCoordinateState,leftNodeId);
            jmdlMTGReg_pinpointCrossing (pCoordinateState, &uCross, leftNodeId, nextLeftNodeId, vScan, uDefault);
            jmdlVArrayInt_set ( pLeftArray, leftNodeId, i); // WAS: "replaceCurrent"
            if  (
                     uMin > uCross
                &&
                    (
                        MTG_NULL_NODEID == firstLeftNodeId
                    ||  1 == jmdlMTGReg_compareEdgesHorizontally
                                (
                                pCoordinateState,
                                leftNodeId,
                                nextLeftNodeId,
                                firstLeftNodeId,
                                jmdlMTGGraph_getFPred (pGraph,firstLeftNodeId)
                                )
                    )
                )

                {
                /* This is nearer than the previous best */
                uFirst = uCross;
                firstLeftNodeId = leftNodeId;
                }
            }
        }

    if (firstLeftNodeId != MTG_NULL_NODEID)
        {
        *pFirstLeftNodeId = firstLeftNodeId;
        uvP->x = uFirst;
        uvP->y = vScan;
        }
    else
        {
        *pFirstLeftNodeId = MTG_NULL_NODEID;
        }
    }

/*PF*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGReg_advancePeaks                                 |
|                                                                       |
| author        earlinLutz                                      10/99   |
|                                                                       |
+----------------------------------------------------------------------*/
static void jmdlMTGReg_advancePeaks
(
CoordinateState     *pCoordinateState,
MTGNodeId               *pTopPeakNodeId,    /* <= (possibly null) highest peak in the
                                            channel between leftNodeId and rightNodeId
                                            */
MTGNodeId               minNodeId,      /* => vu of scan line       */
MTGNodeId               leftNodeId,     /* => left bounding vu      */
MTGNodeId               rightNodeId     /* => right bounding vu     */
)
    {
    MTGGraph *pGraph = pCoordinateState->pGraph;

    EmbeddedIntArray * pPeakArray = pCoordinateState->pPeakArray;
    MTGNodeId topPeakNodeId, peakNodeId;
    double uLeft, uRight;
    double vScan;
    double uDefault;
    int i;
    int outOfChannel;
    static int killpeaks = 0;

    topPeakNodeId = MTG_NULL_NODEID;
    /* For each left chain ... */
    for ( i = 0; jmdlEmbeddedIntArray_getInt (pPeakArray, &peakNodeId, i); i++)
        {
        if (  ( leftNodeId == MTG_NULL_NODEID    || jmdlMTGReg_below (pCoordinateState,  leftNodeId,  peakNodeId))
           && ( rightNodeId == MTG_NULL_NODEID   || jmdlMTGReg_below (pCoordinateState,  rightNodeId, peakNodeId))
           && ( topPeakNodeId == MTG_NULL_NODEID || jmdlMTGReg_below (pCoordinateState,  topPeakNodeId, peakNodeId))
           )
           {
           outOfChannel = false;
           if ( leftNodeId != MTG_NULL_NODEID )
                {
                vScan = jmdlMTGReg_getV (pCoordinateState, peakNodeId);
                uDefault = jmdlMTGReg_getU (pCoordinateState, peakNodeId);
                jmdlMTGReg_pinpointCrossing
                                (
                                pCoordinateState,
                                &uLeft,
                                leftNodeId,
                                jmdlMTGGraph_getFPred (pGraph,leftNodeId),
                                vScan,
                                uDefault
                                );

                if ( uLeft >= uDefault )
                    outOfChannel = true;
                }
           if ( rightNodeId != MTG_NULL_NODEID )
                {
                vScan = jmdlMTGReg_getV (pCoordinateState, peakNodeId);
                uDefault = jmdlMTGReg_getU (pCoordinateState, peakNodeId);
                jmdlMTGReg_pinpointCrossing
                            (
                            pCoordinateState,
                            &uRight,
                            rightNodeId,
                            jmdlMTGGraph_getFSucc (pGraph,rightNodeId),
                            vScan,
                            uDefault
                            );

                if ( uRight <= uDefault )
                    outOfChannel = true;
                }
           if ( outOfChannel )
               {
               }
           else
               {
               topPeakNodeId = peakNodeId;
               }
           }
        }

    *pTopPeakNodeId = topPeakNodeId;
    if (killpeaks)
        *pTopPeakNodeId = MTG_NULL_NODEID;
}

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGReg_join                                         |
|                                                                       |
| author        earlinLutz                                      10/99   |
|                                                                       |
| Join a downward min to the selected vu.                               |
| Invariants:                                                           |
|  downmins are only joined to things below.  This implies              |
|       a direction of the new edge.                                    |
+----------------------------------------------------------------------*/
static bool    jmdlMTGReg_join
(
CoordinateState *pCoordinateState,
MTGNodeId               *pStartNodeId,          /* => start (at minNodeId) of new edge */
MTGNodeId               *pEndNodeId,                    /* => end of new edge */
MTGNodeId               minNodeId,                      /* => downward min being joined */
MTGNodeId               farNodeId                       /* => far vu of join */
)
    {
    MTGGraph *pGraph = pCoordinateState->pGraph;

    MTGNodeId endNodeId;
    bool    boolstat;

    if (   jmdlMTGReg_nodeInSector(pCoordinateState, minNodeId, farNodeId)
        && jmdlMTGReg_nodeInSector(pCoordinateState, farNodeId, minNodeId)
       )
        {
        jmdlMTGGraph_join   (
                            pGraph,
                            pStartNodeId,
                            pEndNodeId,
                            minNodeId,
                            farNodeId,
                            MTG_NULL_MASK,
                            MTG_NULL_MASK
                            );

        endNodeId = *pEndNodeId;
        jmdlMTGGraph_setMask (pGraph,endNodeId, pCoordinateState->upEdgeMask);
        boolstat = true;
        }
    else
        {
        jmdlMTGReg_panic (pCoordinateState);
        boolstat = false;
        }
    return boolstat;
    }
/*PF*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGReg_insertDownwardMin                            |
|                                                                       |
| author        earlinLutz                                      10/99   |
|                                                                       |
+----------------------------------------------------------------------*/
static void jmdlMTGReg_insertDownwardMin
(
CoordinateState *pCoordinateState,
MTGNodeId minNodeId                     /* => Pointer to the downward min node */
)
    {
    MTGNodeId rightNodeId, leftNodeId, peakNodeId;
    DPoint2d uvRight, uvLeft;
    MTGNodeId startNodeId, endNodeId;

    jmdlMTGReg_advanceRightEdgesToMin (pCoordinateState, &rightNodeId, &uvRight, minNodeId);
    jmdlMTGReg_advanceLeftEdgesToMin  (pCoordinateState, &leftNodeId,  &uvLeft,  minNodeId);

    jmdlMTGReg_advancePeaks (pCoordinateState, &peakNodeId, minNodeId, leftNodeId, rightNodeId);

    if (peakNodeId != MTG_NULL_NODEID)
        {
        if (jmdlMTGReg_join (pCoordinateState, &startNodeId, &endNodeId, minNodeId, peakNodeId))
            {
            jmdlEmbeddedIntArray_addInt( pCoordinateState->pLeftArray, startNodeId);
            jmdlEmbeddedIntArray_addInt( pCoordinateState->pRightArray, minNodeId);
            }
        leftNodeId = rightNodeId = MTG_NULL_NODEID;  /* kills off later logic.  dangerous */
        }

    if (   rightNodeId != MTG_NULL_NODEID
        && leftNodeId  != MTG_NULL_NODEID
        )
        {
        /* Ignore the lower of the bracket pair */
        if (jmdlMTGReg_below (pCoordinateState,  leftNodeId, rightNodeId))
            {
            leftNodeId = MTG_NULL_NODEID;
            }
        else
            {
            rightNodeId = MTG_NULL_NODEID;
            }
        }

    if (rightNodeId != MTG_NULL_NODEID)
        {
        if (jmdlMTGReg_join (pCoordinateState, &startNodeId, &endNodeId, minNodeId, rightNodeId))
            {
            jmdlEmbeddedIntArray_addInt (pCoordinateState->pRightArray, minNodeId);
            jmdlEmbeddedIntArray_addInt (pCoordinateState->pLeftArray, startNodeId);
            jmdlVArrayInt_replaceMatched (pCoordinateState->pLeftArray, rightNodeId, endNodeId);
            }
        }
    else if (leftNodeId != MTG_NULL_NODEID)
        {
        if (jmdlMTGReg_join (pCoordinateState, &startNodeId, &endNodeId, minNodeId, leftNodeId))
            {
            jmdlEmbeddedIntArray_addInt (pCoordinateState->pRightArray, minNodeId);
            jmdlEmbeddedIntArray_addInt (pCoordinateState->pLeftArray,  endNodeId);
            /* and the old left will come up to the right of the min */
            }
        }
    else if (peakNodeId == MTG_NULL_NODEID)
        {
        jmdlEmbeddedIntArray_addInt (pCoordinateState->pRightArray, minNodeId);
        jmdlEmbeddedIntArray_addInt (pCoordinateState->pLeftArray,  minNodeId);
        }
    }
/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGReg_insertUpwardMin                                      |
|                                                                       |
| author        earlinLutz                                      10/99   |
|                                                                       |
+----------------------------------------------------------------------*/
static void jmdlMTGReg_insertUpwardMin
(
CoordinateState *pCoordinateState,
MTGNodeId minNodeId                     /* => Pointer to the downward min node */
)
    {
    jmdlEmbeddedIntArray_addInt (pCoordinateState->pRightArray, minNodeId);
    jmdlEmbeddedIntArray_addInt (pCoordinateState->pLeftArray, minNodeId);
    }


/*PF*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGReg_closeSweep                                   |
|                                                                       |
| author        earlinLutz                                      10/99   |
|                                                                       |
+----------------------------------------------------------------------*/
static void jmdlMTGReg_closeSweep
(
CoordinateState *pCoordinateState
)
    {
    jmdlMTGGraph_dropMask ( pCoordinateState->pGraph, pCoordinateState->upEdgeMask );
    jmdlMTGGraph_dropMask ( pCoordinateState->pGraph, pCoordinateState->downwardExtremumMask );
    jmdlEmbeddedIntArray_drop (pCoordinateState->pMinArray );
    jmdlEmbeddedIntArray_drop (pCoordinateState->pRightArray );
    jmdlEmbeddedIntArray_drop (pCoordinateState->pLeftArray );
    jmdlEmbeddedIntArray_drop (pCoordinateState->pPeakArray );
    }
/*PF*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGReg_openSweep                                    |
|                                                                       |
| author        earlinLutz                                      10/99   |
|                                                                       |
+----------------------------------------------------------------------*/
static bool        jmdlMTGReg_openSweep
(
CoordinateState *pCoordinateState,
MTGGraph            *pGraph,
int                 vertexLabelOffset,
EmbeddedDPoint3dArray     *pCoordinateArray,
bool                reverseCoordinates
)
    {
    bool    boolstat = false;

    pCoordinateState->pGraph = pGraph;

    pCoordinateState->upEdgeMask = jmdlMTGGraph_grabMask (pGraph);
    pCoordinateState->downwardExtremumMask = jmdlMTGGraph_grabMask (pGraph);

    pCoordinateState->pMinArray = jmdlEmbeddedIntArray_grab ();
    pCoordinateState->pRightArray = jmdlEmbeddedIntArray_grab ();
    pCoordinateState->pLeftArray = jmdlEmbeddedIntArray_grab ();
    pCoordinateState->pPeakArray = jmdlEmbeddedIntArray_grab ();

    pCoordinateState->vertexLabelOffset = vertexLabelOffset;

    pCoordinateState->reverseCoordinates = reverseCoordinates;
    pCoordinateState->pCoordinateArray = pCoordinateArray;

    jmdlMTGGraph_clearMaskInSet( pGraph, pCoordinateState->upEdgeMask | pCoordinateState->downwardExtremumMask );
    pCoordinateState->errors = 0;

    if ( pCoordinateState->downwardExtremumMask &&
         pCoordinateState->pPeakArray
       )
        {
        boolstat = true;
        }
    else
        {
        jmdlMTGReg_closeSweep ( pCoordinateState );
        boolstat = false;
        }

    return boolstat;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGReg_halfRegularizeGraph                          |
|                                                                       |
| author        earlinLutz                                      10/99   |
|                                                                       |
| Sweep from bottom to top to connect each downward min to a node below |
| (without crossing any lines of the polygon)                           |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int jmdlMTGReg_halfRegularizeGraph
(
MTGGraph            *pGraph,
int                 vertexLabelOffset,
EmbeddedDPoint3dArray     *pCoordinateArray,
bool                reverseCoordinates
)
    {

    CoordinateState regstate;
    MTGNodeId minNodeId;
    int i;
    bool    boolstat = false;
    int forceExit;
    forceExit = -1;


    if (jmdlMTGReg_openSweep ( &regstate, pGraph, vertexLabelOffset, pCoordinateArray, reverseCoordinates))
        {
        jmdlMTGReg_markUpEdges( &regstate, regstate.upEdgeMask );
        jmdlMTGReg_collectAndMarkLocalMinimaInMarkedGraph( &regstate );
        i = 0;
        while (    regstate.errors == 0
                && forceExit != 0
                && jmdlEmbeddedIntArray_getInt ( regstate.pMinArray, &minNodeId, i) )
            {
            if ( jmdlMTGGraph_getMask (regstate.pGraph, minNodeId, regstate.downwardExtremumMask ) )
                {
                jmdlMTGReg_insertDownwardMin ( &regstate, minNodeId );
                forceExit--;
                }
            else
                {
                jmdlMTGReg_insertUpwardMin ( &regstate, minNodeId );
                }
            i++;
            }
        boolstat = regstate.errors == 0 ? true : false;
        jmdlMTGReg_closeSweep ( &regstate );
        }
    return boolstat;
    }
/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGReg_halfRegularizeGraphMultipleAttempts          |
|                                                                       |
| author        earlinLutz                                      10/99   |
|                                                                       |
| Make multiple attempts to regularize the graph.                       |
+----------------------------------------------------------------------*/
static bool        jmdlMTGReg_halfRegularizeGraphMultipleAttempts
(
MTGGraph            *pGraph,
int                 vertexLabelOffset,
EmbeddedDPoint3dArray     *pCoordinateArray,
bool                reverseCoordinates,
int                 attempts
)
    {
    bool    boolstat = false;
    if (attempts <= 0)
        attempts = 1;

    while (attempts-- > 0)
        {
        if (jmdlMTGReg_halfRegularizeGraph (pGraph, vertexLabelOffset, pCoordinateArray, reverseCoordinates))
            {
            attempts = 0;
            boolstat = true;
            }
        else
            {
            jmdlMTGReg_panic (NULL);
            }
        }
    return boolstat;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGReg_regularize                                   |
|                                                                       |
| author        earlinLutz                                      10/99   |
|                                                                       |
| Bi-directional regularization sweeps                                  |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool         jmdlMTGReg_regularize
(
MTGGraph            *pGraph,
int                 vertexLabelOffset,
EmbeddedDPoint3dArray     *pCoordinateArray
)
    {
    static int sweepUp = 1;
    static int sweepDown = 1;
    static int attemptLimit = 1;
    bool    boolstat = true;

    if ( sweepUp )
        {
        boolstat = jmdlMTGReg_halfRegularizeGraphMultipleAttempts(pGraph, vertexLabelOffset, pCoordinateArray, false, attemptLimit);
        }

    if ( sweepDown && boolstat )
        {
        boolstat = jmdlMTGReg_halfRegularizeGraphMultipleAttempts(pGraph, vertexLabelOffset, pCoordinateArray, true, attemptLimit);
        }

    return boolstat;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGTri_triangulateFaceByVerticalSweep               |
|                                                                       |
| author        earlinLutz                                      10/97   |
|                                                                       |
+----------------------------------------------------------------------*/
static void    jmdlMTGTri_triangulateFaceByVerticalSweep
(
CoordinateState *pCoordinateState,
MTGNodeId             startNodeId               /* Initial vu. MUST be a lexical min!!! */
)
    {
    MTGGraph        *pGraph = pCoordinateState->pGraph;

    MTGNodeId             leftNodeId = jmdlMTGGraph_getFPred (pGraph, startNodeId);
    MTGNodeId             rightNodeId = jmdlMTGGraph_getFSucc (pGraph, startNodeId);
    MTGNodeId             node0Id, node1Id, node2Id;
    MTGNodeId             A, B;

    /*    Loop invariant: startNodeId is the bottom node. */
    while (leftNodeId != rightNodeId
           && rightNodeId != startNodeId
           && jmdlMTGGraph_getFSucc (pGraph, rightNodeId) != leftNodeId
    /* These conditions should never happen !! */
           && jmdlMTGReg_cross (pCoordinateState, leftNodeId, startNodeId, rightNodeId) > 0
           && jmdlMTGReg_below (pCoordinateState, startNodeId, leftNodeId)
           && jmdlMTGReg_below (pCoordinateState, startNodeId, rightNodeId)
        )
        {
        if (jmdlMTGReg_below (pCoordinateState, leftNodeId, rightNodeId))
            {
            /*      Triangulate to all left side edges that
               are below rightNodeId */

            /*      Phase 1: move upward, adding back edges
               when prior nodes are visible. */
            node0Id = leftNodeId;
            node1Id = startNodeId;
            node2Id = rightNodeId;
            /*      Invariant: the path from node0Id back to node1Id is concave.
               Each loop pass moves node0Id up the left side, filling in
               edges as needed.  The right side edge
               (following startNodeId) is never altered.
             */
            while (node0Id != node2Id && jmdlMTGReg_below (pCoordinateState, node0Id, rightNodeId))
                {
                while (node2Id != rightNodeId
                       && node2Id != node0Id
                       && node2Id != node1Id
                       && jmdlMTGReg_cross (pCoordinateState, node0Id, node1Id, node2Id) > 0)
                    {
                    jmdlMTGGraph_join (pGraph, &A, &B, node0Id, node2Id, MTG_NULL_MASK, MTG_NULL_MASK);
                    node0Id = A;
                    node1Id = jmdlMTGGraph_getFSucc (pGraph, node0Id);
                    node2Id = jmdlMTGGraph_getFSucc (pGraph, node1Id);
                    }
                node2Id = node1Id;
                node1Id = node0Id;
                node0Id = jmdlMTGGraph_getFPred (pGraph, node0Id);
                }
            /*      Phase 2: Fan out edges from rightNodeId to the
               left side. node0Id.node1Id.node2Id describes a pair of
               adjacent edges at the bottom. */
            leftNodeId = node1Id;
            node2Id = rightNodeId;
            node1Id = jmdlMTGGraph_getFPred (pGraph, node2Id);
            node0Id = jmdlMTGGraph_getFPred (pGraph, node1Id);
            while (jmdlMTGGraph_getFSucc (pGraph, node2Id) != node0Id && node0Id != leftNodeId)
                {
                jmdlMTGGraph_join (pGraph, &A, &B, node0Id, node2Id, MTG_NULL_MASK, MTG_NULL_MASK);
                node1Id = A;
                node0Id = jmdlMTGGraph_getFPred (pGraph, node1Id);
                }
            /*      Finish off with the last stroke from the
               left node to the right, except when already
               topped out */
            if (jmdlMTGGraph_getFSucc (pGraph, node2Id) != node0Id)
                {
                jmdlMTGGraph_join (pGraph, &A, &B, node0Id, node2Id, MTG_NULL_MASK, MTG_NULL_MASK);
                node0Id = A;
                }
            startNodeId = node0Id;
            rightNodeId = jmdlMTGGraph_getFSucc (pGraph, startNodeId);
            leftNodeId = jmdlMTGGraph_getFPred (pGraph, startNodeId);

            }
        else
            {
            /*      Triangulate to all right side edges that
               are below leftNodeId */

            /*      Phase 1: move upward, adding back edges
               when prior nodes are visible. */
            node0Id = leftNodeId;
            node1Id = startNodeId;
            node2Id = rightNodeId;
            /*      Invariant: the path up to node1Id is concave.
               Each loop pass advances node1Id, filling in
               edges as needed. Note that the
               startNodeId edge may get hidden, so the
               bottom node must be referenced as
               jmdlMTGGraph_getFSucc (pGraph, leftNodeId) rather than as startNodeId.
             */
            while (node0Id != node2Id && jmdlMTGReg_below (pCoordinateState, node2Id, leftNodeId))
                {
                while (node0Id != leftNodeId
                       && node2Id != node0Id
                       && node2Id != node1Id
                       && jmdlMTGReg_cross (pCoordinateState, node0Id, node1Id, node2Id) > 0)
                    {
                    jmdlMTGGraph_join (pGraph, &A, &B, node0Id, node2Id, MTG_NULL_MASK, MTG_NULL_MASK);
                    node0Id = jmdlMTGGraph_getFPred (pGraph, A);
                    node1Id = A;
                    }
                node0Id = node1Id;
                node1Id = node2Id;
                node2Id = jmdlMTGGraph_getFSucc (pGraph, node2Id);
                }
            /*      Phase 2: Fan out edges from leftNodeId to the
               right side. node0Id.node1Id.node2Id describes a pair of
               adjacent edges at the bottom. */
            rightNodeId = node1Id;
            node0Id = leftNodeId;
            node1Id = jmdlMTGGraph_getFSucc (pGraph, node0Id);
            node2Id = jmdlMTGGraph_getFSucc (pGraph, node1Id);
            while (jmdlMTGGraph_getFSucc (pGraph, node2Id) != node0Id && node2Id != rightNodeId)
                {
                jmdlMTGGraph_join (pGraph, &A, &B, node0Id, node2Id, MTG_NULL_MASK, MTG_NULL_MASK);
                node0Id = A;
                node1Id = node2Id;
                node2Id = jmdlMTGGraph_getFSucc (pGraph, node2Id);
                }
            /*      Finish off with the last stroke from the
               left node to the right, except when already
               topped out */
            if (jmdlMTGGraph_getFSucc (pGraph, node2Id) != node0Id)
                {
                jmdlMTGGraph_join (pGraph, &A, &B, node0Id, node2Id, MTG_NULL_MASK, MTG_NULL_MASK);
                }
            startNodeId = rightNodeId;
            rightNodeId = jmdlMTGGraph_getFSucc (pGraph, startNodeId);
            leftNodeId = jmdlMTGGraph_getFPred (pGraph, startNodeId);
            }
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGReg_searchRegularizedLimits                      |
|                                                                       |
| author        earlinLutz                                      10/99   |
|                                                                       |
+----------------------------------------------------------------------*/
static bool       jmdlMTGReg_searchMinMaxArea
(
CoordinateState *pCoordinateState,
MTGNodeId       *pMinNodeId,
MTGNodeId       *pMaxNodeId,
double          *pArea,
MTGNodeId       seedNodeId
)
    {
    MTGGraph *pGraph = pCoordinateState->pGraph;
    bool    oldBelow, newBelow;

    MTGNodeId minNodeId, maxNodeId, currNodeId, loopStartNodeId;

    currNodeId = jmdlMTGGraph_getFSucc (pGraph, seedNodeId);
    loopStartNodeId = jmdlMTGGraph_getFSucc (pGraph, currNodeId);
    minNodeId = maxNodeId = MTG_NULL_NODEID;

    oldBelow = jmdlMTGReg_below (pCoordinateState, currNodeId, seedNodeId);

    if (pMinNodeId)
        *pMinNodeId = MTG_NULL_NODEID;

    if (pMaxNodeId)
        *pMaxNodeId = MTG_NULL_NODEID;

    if (pArea)
        *pArea = 0.0;

    MTGARRAY_FACE_LOOP (nextNodeId, pGraph, loopStartNodeId)
        {
        newBelow = jmdlMTGReg_below (pCoordinateState, nextNodeId, currNodeId);
        if (newBelow != oldBelow)
            {
            if (newBelow)
                {
                if (maxNodeId != MTG_NULL_NODEID)
                    return false;
                maxNodeId = currNodeId;
                }
            else
                {
                if (minNodeId != MTG_NULL_NODEID)
                    return false;
                minNodeId = currNodeId;
                }
            }
        currNodeId = nextNodeId;
        oldBelow = newBelow;
        }
    MTGARRAY_END_FACE_LOOP (nextNodeId, pGraph, loopStartNodeId)

    if (pArea)
        {
        double area = 0.0;
        MTGNodeId nextNodeId;
        for (currNodeId = jmdlMTGGraph_getFSucc (pGraph, seedNodeId);
            (nextNodeId = jmdlMTGGraph_getFSucc (pGraph, currNodeId)) != seedNodeId;
            currNodeId = nextNodeId)
            {
            area += jmdlMTGReg_cross (pCoordinateState, seedNodeId, currNodeId, nextNodeId);
            }
        area *= 0.5;
        *pArea = area;
        }

    if (pMinNodeId)
        *pMinNodeId = minNodeId;

    if (pMaxNodeId)
        *pMaxNodeId = maxNodeId;

    return true;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          jmdlMTGTri_triangulateAllCCWRegularFacesByVerticalSweep |
|                                                                       |
| author        earlinLutz                                      10/99   |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    jmdlMTGTri_triangulateAllCCWRegularFacesByVerticalSweep
(
MTGGraph            *pGraph,
int                 vertexLabelOffset,
EmbeddedDPoint3dArray     *pCoordinateArray
)
    {
    CoordinateState context;
    int i;
    MTGNodeId seedNodeId, lowNodeId, highNodeId;
    double area;
    int numError = 0;

    if (jmdlMTGReg_openCoordinateQueries (&context, pGraph, vertexLabelOffset, pCoordinateArray, false))
        {
        EmbeddedIntArray *pFaceArray = jmdlEmbeddedIntArray_grab ();
        jmdlMTGGraph_collectAndNumberFaceLoops (pGraph, pFaceArray, NULL);

        for (i = 0; jmdlEmbeddedIntArray_getInt (pFaceArray, &seedNodeId, i); i++)
            {
            if  (    jmdlMTGReg_searchMinMaxArea (&context, &lowNodeId, &highNodeId, &area, seedNodeId)
                 && area > 0.0
                )
                {
                jmdlMTGTri_triangulateFaceByVerticalSweep (&context, lowNodeId);
                }
            }
        jmdlEmbeddedIntArray_drop (pFaceArray);
        jmdlMTGReg_closeCoordinateQueries (&context);
        }

    return numError == 0;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
