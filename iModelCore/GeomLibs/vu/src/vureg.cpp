/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/* special manipulations for triangles */

typedef struct regularizationState
    {
    VuMask upEdgeMask;                /* During initialization, each edge is marked as
                                                up or down.  Upward scans then use this
                                                to recognize peaks */
    VuMask downwardExtremumMask;                /* Distinguishes down mins from up mins in rsP->minArrayP */

    VuArrayP minArrayP; /* Array of vu's which are minima eventually sorted by (lexical) y */

    VuArrayP leftArrayP;        /* lower vu on left-side edges */
    VuArrayP rightArrayP;       /* lower vu on right side edges */
    VuArrayP peakArrayP;        /* active peaks */

    VuSetP graphP;
    int    errors;
    } RegularizationState;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void vureg_markUpEdges
(
VuSetP graphP,          /* => graph whose upedges are to be marked */
VuP     faceP,          /* => optional face for single-face markup. */
VuMask  mask            /* => mask to install in each upedge ( and clear in downedge ) */
)
    {
    VuP nextP;
    if (faceP)
        {
        VU_FACE_LOOP( currP, faceP )
            {
            nextP = VU_FSUCC( currP );
            if ( VU_BELOW( currP, nextP ) )
                {
                VU_SETMASK( currP, mask );
                }
            else
                {
                VU_CLRMASK( currP, mask );
                }
            }
        END_VU_FACE_LOOP( currP, faceP )
        }
    else
        {
        VU_SET_LOOP( currP, graphP )
            {
            nextP = VU_FSUCC( currP );
            if ( VU_BELOW( currP, nextP ) )
                {
                VU_SETMASK( currP, mask );
                }
            else
                {
                VU_CLRMASK( currP, mask );
                }
            }
        END_VU_SET_LOOP( currP, graphP )
        }
    }

static int  s_downwardExtremumMaskForSorting;
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int vureg_compareLexicalUV
(
const VuP * node0PP,
const VuP * node1PP
)
    {
    int result = vu_compareLexicalUV( node0PP, node1PP, NULL);
    if ( ( result == 0 ) && (*node0PP != *node1PP) )
        {
        if (VU_GETMASK ( *node0PP, s_downwardExtremumMaskForSorting ) )
            return -1;
        if (VU_GETMASK ( *node1PP, s_downwardExtremumMaskForSorting ) )
            return 1;
        }
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void checkLocalMinimaCandidate
(
RegularizationState *rsP,
VuP                 currP,   /* Edge to examine (during face or set loop) */
VuMask              upEdgeMask
)
    {
    VuP nextP;
    VuP secondP;
    double cross;
    nextP = VU_FSUCC( currP );
    if ( ! VU_GETMASK( currP , upEdgeMask ) && VU_GETMASK( nextP, upEdgeMask ) )
        {
        /* This is a min. Record it and set the up/down bit */
        secondP = VU_FSUCC(nextP);
        cross = vu_cross( currP, nextP, secondP );
        if (currP == secondP && VU_VSUCC(currP) != currP)
            {
            // This sector is one end of a null face.  Just ignore it -- nothing
            // can possibly be inserted.
            // This assignment is just so there is a place to stop in the debugger.
            cross = 0.0;
            }
        else
            {
            vu_arrayAdd( rsP->minArrayP , nextP );
            if ( VU_VSUCC(nextP) == nextP ||  cross <= 0.0 )
                {
                VU_SETMASK( nextP, rsP->downwardExtremumMask );
                }
            }
        }
    else if ( VU_GETMASK( currP , upEdgeMask ) && !VU_GETMASK( nextP, upEdgeMask ) )
        {
        /* This is a max. Set the up/down bit */
        secondP = VU_FSUCC(nextP);
        cross = vu_cross( currP, nextP, secondP );
        if ( VU_VSUCC(nextP) != nextP && cross > 0.0 )
            {
            VU_SETMASK( nextP, rsP->downwardExtremumMask );
            }
        }
    }
/*PF*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int vureg_collectAndMarkLocalMinimaInMarkedGraph
(
RegularizationState *rsP,
VuP                 faceP   /* => OPTIONAL face.  If NULL, mark entire graph. */
)
    {
    VuMask upEdgeMask = rsP->upEdgeMask;
    int count = 0;
    /** Min sorting needs to know what mask bit distinguishes downward mins. */
    vu_arrayClear (rsP->minArrayP);
    vu_arrayClear (rsP->leftArrayP);
    vu_arrayClear (rsP->rightArrayP);

    if (faceP)
        {
        VU_FACE_LOOP (currP, faceP)
            {
            checkLocalMinimaCandidate (rsP, currP, upEdgeMask);
            }
        END_VU_FACE_LOOP (currP, faceP)
        }
    else
        {
        VU_SET_LOOP( currP, rsP->graphP )
            {
            checkLocalMinimaCandidate (rsP, currP, upEdgeMask);
            }
        END_VU_SET_LOOP ( currP, rsP->graphP );
        }

    s_downwardExtremumMaskForSorting = rsP->downwardExtremumMask;
    vu_arraySort0 (rsP->minArrayP, vureg_compareLexicalUV);
    return count;
    }

/*PF*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void vureg_pinpointCrossing
(
double *uCrossP,        /* <= u coordinate where v crosses the edge */
VuP bottomP,            /* => VU at bottom of edge */
VuP topP,               /* => VU at top of edge */
double v,               /* => v of scan line */
double uError           /* => u to return if the edge is horizontal */
)
    {
    double v0 = VU_V( bottomP );
    double v1 = VU_V( topP );
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
        double u0 = VU_U( bottomP );
        double u1 = VU_U( topP );
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


/*---------------------------------------------------------------------------------**//**
* Test if the edge base0P..top0P is left/at/right of the edge base1P..top1P, using the midpoint of their common y range as the primary
* comparison point. If they share no y range, use vertical comparison.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int     vureg_compareEdgesHorizontally
(
VuP             base0P,         /* => base of first edge */
VuP             top0P,          /* => top of first edge */
VuP             base1P,         /* => base of second edge */
VuP             top1P           /* => top of second edge */
)
    {
    double vmin, vmax;
    int result = 0;
    double u0,u1,v0,v1;
    double vmid;

    vmin = VU_V(base0P);
    if (VU_V(base1P) > vmin)
        vmin = VU_V(base1P);

    vmax = VU_V(top0P);
    if (VU_V(top1P) < vmax)
        vmax = VU_V(top1P);

    if (vmax < vmin)
        {
        /* There is no horizontal overlap.   Use vertical relationship */
        v0 = VU_V(base0P);
        v1 = VU_V(base1P);
        if (v0 < v1)
            result = -1;
        else if(v1 < v0)
            result = 1;
        }
    else
        {
        vmid = 0.5 * (vmin + vmax);
        if (vmid > vmin && vmid < vmax)
            {
            vureg_pinpointCrossing( &u0, base0P, top0P, vmid,
                            0.5*(VU_U(base0P) + VU_U(top0P)));
            vureg_pinpointCrossing( &u1, base1P, top1P, vmid,
                            0.5*(VU_U(base1P) + VU_U(top1P)));
            }
        else
            {
            // Gotta think about this.  vmid is the average.  The average should be "in the middle",
            //   but it came out wrong.  Floating point range limits strike.  The numbers are 0 or 1 bit apart,
            //   and the average came out the same as one of the limits.
            // Do fussy logic to get u values BEFORE averaging
            double u0_base, u1_base;
            double u0_top, u1_top;
            double v;
            // Take u values from vertex of one edge, interior of other.
            if (VU_V(base0P) < VU_V(base1P))
                {
                v = VU_V(base1P);
                vureg_pinpointCrossing( &u0_base, base0P, top0P, v, VU_U(base0P));
                u1_base = VU_U(base1P);
                }
            else
                {
                v = VU_V(base0P);
                u0_base = VU_U(base0P);
                vureg_pinpointCrossing( &u1_base, base1P, top1P, v, VU_U(base1P));
                }

            if (VU_V(top1P) < VU_V(top0P))
                {
                v = VU_V(top1P);
                vureg_pinpointCrossing( &u0_top, base0P, top0P, v, VU_U(top0P));
                u1_top = VU_U(top1P);
                }
            else
                {
                v = VU_V(top0P);
                u0_top = VU_U(top0P);
                vureg_pinpointCrossing( &u1_top, base1P, top1P, v, VU_U(top1P));
                }

            u0 = 0.5 * (u0_base + u0_top);
            u1 = 0.5 * (u1_base + u1_top);
            }

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
|  ** Upedges are marked with rsP->upEdgeMark                           |
|  ** Every edge in rightArrayP is an upedge.                           |
|  ** The FPRED of every edge in leftArrayP is NOT an upedge            |
|  ** Peaks are ultimately reached by both a left and a right chain.    |
|       Record them into the peakArrayP when reached by a right edge    |
|  ** When drawing on an increasing sequence of mins, the bases of      |
|       all active left and right chains are below all unprocessed mins |
|  ** An upward min generates a new left and a new right chain          |
|  ** When a downward min is inserted between a left and right chain:   |
|       *** all peaks between those chains can die.                     |
|       *** the two edges at the peak are new bases (one right, one     |
|               left)                                                   |
+----------------------------------------------------------------------*/
/*PF*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void vureg_advanceRightEdgesToMin
(
RegularizationState*    rsP,
VuP*                    firstRightPP, /* <= (possibly null) first edge encountered
                                                by rightward ray from minP */
DPoint2d*               uvP,          /* <= (possibly unset) uv at right intersect */
VuP                     minP          /* => vu of scan line */
)
    {
    VuP rightP, nextRightP;
    VuArrayP rightArrayP = rsP->rightArrayP;
    VuP firstRightP;
    double uCross, uFirst;
    double vScan,uMin;
    double uDefault;

    vScan = VU_V(minP);
    uMin  = VU_U(minP);
    uFirst = uMin;
    firstRightP = NULL;
    /* For each right chain ... */
    for ( vu_arrayOpen (rightArrayP); vu_arrayRead (rightArrayP, &rightP);)
        {
        nextRightP = VU_FSUCC(rightP);
        /* Move up until the chain either tops out or passes the min */
        while ( VU_BELOW( nextRightP, minP ) && VU_GETMASK( nextRightP, rsP->upEdgeMask) )
            {
            rightP = nextRightP;
            nextRightP = VU_FSUCC(nextRightP);
            }

        if (VU_BELOW( nextRightP, minP))
            {
            /* nextRightP is a peak. Take the chain out of the active array;
               if the peak is upward, record it */
            if (!VU_GETMASK( nextRightP, rsP->downwardExtremumMask))
                vu_arrayAdd (rsP->peakArrayP, nextRightP);

            vu_arrayRemoveCurrent (rsP->rightArrayP);
            }
        else
            {
            /* rightP .. nextRightP brackets the scanline */
            uDefault = VU_U(rightP);
            vureg_pinpointCrossing ( &uCross, rightP, nextRightP, vScan, uDefault);
            vu_arrayReplaceCurrent ( rightArrayP, rightP);
            if  (
                     uMin < uCross
                &&
                    (
                        NULL == firstRightP
                    ||  -1 == vureg_compareEdgesHorizontally(
                                rightP, nextRightP,
                                firstRightP, VU_FSUCC(firstRightP)
                                )
                    )
                )
                {
                /* This is nearer than the previous best */
                uFirst = uCross;
                firstRightP = rightP;
                }
            }
        }

    if (firstRightP)
        {
        *firstRightPP = firstRightP;
        uvP->x = uFirst;
        uvP->y = vScan;
        }
    else
        {
        *firstRightPP = NULL;
        }
    }

/*PF*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void vureg_advanceLeftEdgesToMin
(
RegularizationState*    rsP,
VuP*                    firstLeftPP, /* <= (possibly null) first edge encountered
                                                by leftward ray from minP */
DPoint2d*               uvP,          /* <= (possibly unset) uv at left intersect */
VuP                     minP          /* => vu of scan line */
)
    {
    VuP leftP, nextLeftP, farLeftP;
    VuArrayP leftArrayP = rsP->leftArrayP;
    VuP firstLeftP;
    double uCross, uFirst;
    double vScan,uMin;
    double uDefault;

    vScan = VU_V(minP);
    uMin  = VU_U(minP);
    uFirst = uMin;
    firstLeftP = NULL;
    /* For each left chain ... */
    for ( vu_arrayOpen (leftArrayP); vu_arrayRead (leftArrayP, &leftP);)
        {
        nextLeftP = VU_FPRED(leftP);
        farLeftP = VU_FPRED(nextLeftP);
        /* Move up until the chain either tops out or passes the min */
        while ( VU_BELOW( nextLeftP, minP ) && !VU_GETMASK( farLeftP, rsP->upEdgeMask) )
            {
            leftP = nextLeftP;
            nextLeftP = VU_FPRED(nextLeftP);
            farLeftP  = VU_FPRED(nextLeftP);
            }

        if (VU_BELOW( nextLeftP, minP))
            {
            /* nextLeftP is a peak. Take the chain out of the active array */
            vu_arrayRemoveCurrent (rsP->leftArrayP);
            }
        else
            {
            /* leftP .. nextLeftP brackets the scanline */
            uDefault = VU_U(leftP);
            vureg_pinpointCrossing ( &uCross, leftP, nextLeftP, vScan, uDefault);
            vu_arrayReplaceCurrent ( leftArrayP, leftP);
            if  (
                     uMin > uCross
                &&
                    (
                        NULL == firstLeftP
                    ||  1 == vureg_compareEdgesHorizontally(
                                leftP, nextLeftP,
                                firstLeftP, VU_FPRED(firstLeftP)
                                )
                    )
                )

                {
                /* This is nearer than the previous best */
                uFirst = uCross;
                firstLeftP = leftP;
                }
            }
        }

    if (firstLeftP)
        {
        *firstLeftPP = firstLeftP;
        uvP->x = uFirst;
        uvP->y = vScan;
        }
    else
        {
        *firstLeftPP = NULL;
        }
    }

/*PF*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void vureg_advancePeaks
(
RegularizationState*    rsP,
VuP*                    topPeakPP,/* <= (possibly null) highest peak in the
                                        channel between leftP and rightP */
VuP                     minP,   /* => vu of scan line */
VuP                     leftP,  /* => left bounding vu */
VuP                     rightP  /* => right bounding vu */
)
    {
    VuArrayP peakArrayP = rsP->peakArrayP;
    VuP topPeakP, peakP;
    double uLeft, uRight;
    double vScan;
    double uDefault;
    int outOfChannel;
    static int killpeaks = 0;

    topPeakP = NULL;
    /* For each left chain ... */
    for ( vu_arrayOpen (peakArrayP); vu_arrayRead (peakArrayP, &peakP);)
        {
        if (  ( !leftP  || VU_BELOW( leftP,  peakP))
           && ( !rightP || VU_BELOW( rightP, peakP))
           && ( !topPeakP || VU_BELOW( topPeakP, peakP))
           )
           {
           outOfChannel = false;
           if ( leftP )
                {
                vScan = VU_V(peakP);
                uDefault = VU_U(peakP);
                vureg_pinpointCrossing ( &uLeft,
                        leftP, VU_FPRED(leftP), vScan, uDefault);
                if ( uLeft >= uDefault )
                    outOfChannel = true;
                }
           if ( rightP )
                {
                vScan = VU_V(peakP);
                uDefault = VU_U(peakP);
                vureg_pinpointCrossing ( &uRight,
                        rightP, VU_FSUCC(rightP), vScan, uDefault);
                if ( uRight <= uDefault )
                    outOfChannel = true;
                }
           if ( outOfChannel )
               {
               }
           else
               {
               topPeakP = peakP;
               }
           }
        }

    *topPeakPP = topPeakP;
    if (killpeaks)
        *topPeakPP = NULL;
}

/*---------------------------------------------------------------------------------**//**
* Join a downward min to the selected vu. Invariants: downmins are only joined to things below. This implies a direction of the new edge.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int     vureg_join
(
RegularizationState *rsP,
VuP             *startPP,               /* => start (at minP) of new edge */
VuP             *endPP,                 /* => end of new edge */
VuP             minP,                   /* => downward min being joined */
VuP             farP                    /* => far vu of join */
)
    {
    VuP endP;
    int status;

    if (   vu_nodeInSector(minP, farP)
        && vu_nodeInSector(farP, minP)
       )
        {
        vu_join (rsP->graphP, minP, farP, startPP, endPP);
        endP = *endPP;
        VU_SETMASK (endP, rsP->upEdgeMask);
        status = SUCCESS;
        }
    else
        {
        vu_panic (rsP->graphP);
        status = ERROR;
        }
    return status;
    }
/*PF*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void vureg_insertDownwardMin
(
RegularizationState *rsP,
VuP minP                        /* => Pointer to the downward min node */
)
    {
    VuP rightP, leftP, peakP;
    DPoint2d uvRight, uvLeft;
    VuP startP, endP;
    //VuP highP = NULL;

    vureg_advanceRightEdgesToMin (rsP, &rightP, &uvRight, minP);
    vureg_advanceLeftEdgesToMin  (rsP, &leftP,  &uvLeft,  minP);

    vureg_advancePeaks (rsP, &peakP, minP, leftP, rightP);

    if (peakP)
        {
        if (SUCCESS == vureg_join (rsP, &startP, &endP, minP, peakP))
            {
            vu_arrayAdd( rsP->leftArrayP, startP);
            vu_arrayAdd( rsP->rightArrayP, minP);
            }
        leftP = rightP = NULL;  /* kills off later logic.  dangerous */
        }

    if (rightP && leftP)
        {
        /* Ignore the lower of the bracket pair */
        if (VU_BELOW( leftP, rightP))
            {
            leftP = NULL;
            }
        else
            {
            rightP = NULL;
            }
        }

    if (rightP)
        {
        if (SUCCESS == vureg_join (rsP, &startP, &endP, minP, rightP))
            {
            vu_arrayAdd (rsP->rightArrayP, minP);
            vu_arrayAdd (rsP->leftArrayP, startP);
            vu_arrayReplaceMatched (rsP->leftArrayP, rightP, endP);
            }
        }
    else if (leftP )
        {
        if (SUCCESS == vureg_join (rsP, &startP, &endP, minP, leftP))
            {
            vu_arrayAdd (rsP->rightArrayP, minP);
            vu_arrayAdd (rsP->leftArrayP,  endP);
            /* and the old left will come up to the right of the min */
            }
        }
    else if (!peakP)
        {
        vu_arrayAdd (rsP->rightArrayP, minP);
        vu_arrayAdd (rsP->leftArrayP,  minP);
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void vureg_insertUpwardMin
(
RegularizationState *rsP,
VuP minP                        /* => Pointer to the downward min node */
)
    {
    vu_arrayAdd (rsP->rightArrayP, minP);
    vu_arrayAdd (rsP->leftArrayP, minP);
    }


/*PF*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void vureg_closeSweep
(
RegularizationState *rsP
)
    {
    vu_returnMask ( rsP->graphP, rsP->upEdgeMask );
    vu_returnMask ( rsP->graphP, rsP->downwardExtremumMask );
    vu_returnArray ( rsP->graphP, rsP->minArrayP );
    vu_returnArray ( rsP->graphP, rsP->rightArrayP );
    vu_returnArray ( rsP->graphP, rsP->leftArrayP );
    vu_returnArray ( rsP->graphP, rsP->peakArrayP );
    }
/*PF*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int vureg_openSweep
(
RegularizationState *rsP,
VuSetP graphP
)
    {
    int status;
    rsP->graphP = graphP;
    rsP->upEdgeMask = vu_grabMask (graphP);
    rsP->downwardExtremumMask = vu_grabMask (graphP);

    rsP->minArrayP = vu_grabArray( graphP );
    rsP->rightArrayP = vu_grabArray ( graphP );
    rsP->leftArrayP = vu_grabArray ( graphP );
    rsP->peakArrayP = vu_grabArray ( graphP );

    vu_clearMaskInSet( graphP, rsP->upEdgeMask | rsP->downwardExtremumMask );
    rsP->errors = 0;

    if ( rsP->downwardExtremumMask &&
         rsP->peakArrayP
       )
        {
        status = SUCCESS;
        }
    else
        {
        vureg_closeSweep ( rsP );
        status = ERROR;
        }
    return status;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void vureg_halfRegularizeFromSeeds
(
RegularizationState *rsP
)
    {
    int forceExit = -1; /* Useful in debuggers. */
    VuP minP;

    vu_arrayOpen ( rsP->minArrayP );
    while (    rsP->errors == 0
            && forceExit != 0
            && vu_arrayRead ( rsP->minArrayP, &minP ) )
        {
        if ( VU_GETMASK ( minP, rsP->downwardExtremumMask ) )
            {
            vureg_insertDownwardMin ( rsP, minP );
            forceExit--;
            }
        else
            {
            vureg_insertUpwardMin ( rsP, minP );
            }
        }
    }

/*PF*/
/*---------------------------------------------------------------------------------**//**
* Sweep from bottom to top to connect each downward min to a node below (without crossing any lines of the polygon)
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int vureg_halfRegularizeGraph
(
VuSetP graphP,
bool    bOneFaceAtATime
)
    {
    RegularizationState regstate;
    int status = ERROR;
    if ( SUCCESS == vureg_openSweep ( &regstate, graphP ) )
        {
        if (bOneFaceAtATime)
            {
            VuP currFaceP;
            VuArrayP faceArrayP = vu_grabArray (graphP);
            vu_collectInteriorFaceLoops (faceArrayP, graphP);
            for (vu_arrayOpen (faceArrayP); vu_arrayRead (faceArrayP, &currFaceP); )
                {
                vureg_markUpEdges( regstate.graphP, currFaceP, regstate.upEdgeMask );
                vureg_collectAndMarkLocalMinimaInMarkedGraph( &regstate, currFaceP);
                vureg_halfRegularizeFromSeeds (&regstate);
                }
            vu_returnArray (graphP, faceArrayP);
            }
        else
            {
            vureg_markUpEdges (regstate.graphP, NULL, regstate.upEdgeMask);
            vureg_collectAndMarkLocalMinimaInMarkedGraph (&regstate, NULL);
            vureg_halfRegularizeFromSeeds (&regstate);
            }
        vureg_closeSweep ( &regstate );
        }
    status = regstate.errors == 0 ? SUCCESS : ERROR;
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* Make multiple attempts to regularize the graph.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void    vureg_halfRegularizeGraphMultipleAttemptsExt
(
VuSetP          graphP,         /* <=> graph to be regularized */
bool            bOneFaceAtATime, /* => if true, assume faces are connected. */
int             attempts        /* => maximum number of tries */
)
    {
    if (attempts <= 0)
        attempts = 1;

    while (attempts-- > 0)
        {
        if (SUCCESS == vureg_halfRegularizeGraph (graphP, bOneFaceAtATime))
            {
            attempts = 0;
            }
        else
            {
            vu_panic (graphP);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description Make multiple attempts to regularize the graph by sweeping up.
* @remarks Regularization of (the y-coordinates of) a graph entails inserting edges so that each
*       face has a single local minimum and a single local maximum, and so that holes have a bridge to their surrounding face.
* @param graphP     IN OUT  graph header
* @param attempts   IN      maximum number of attempts
* @group "VU Meshing"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vureg_halfRegularizeGraphMultipleAttempts
(
VuSetP          graphP,
int             attempts
)
    {
    vureg_halfRegularizeGraphMultipleAttemptsExt (graphP, false, attempts);
    }

/*---------------------------------------------------------------------------------**//**
* @description Regularize the graph by sweeping up and down.
* @remarks Regularization of (the y-coordinates of) a graph entails inserting edges so that each
*       face has a single local minimum and a single local maximum, and so that holes have a bridge to their surrounding face.
* @remarks This is a critical step in triangulation.   Once the faces are regularized, triangulation
*       edges are easy to add in a single bottom-to-top sweep.
* @remarks This routine calls ~mvureg_halfRegularizeGraphMultipleAttempts twice---the second time
*       on a locally 180-degree rotated graph.
* @param graphP     IN OUT  graph header
* @group "VU Meshing"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vureg_regularizeGraph
(
VuSetP graphP
)
    {
    static int sweepUp = 1;
    static int sweepDown = 1;
    static int attemptLimit = 3;

    if ( sweepUp )
        {
        vureg_halfRegularizeGraphMultipleAttemptsExt (graphP, false, attemptLimit);
        }

    if ( sweepDown )
        {
        vu_rotate180 (graphP );
        vureg_halfRegularizeGraphMultipleAttemptsExt (graphP, false, attemptLimit);
        vu_rotate180 (graphP );
        }
    }

/*---------------------------------------------------------------------------------**//**
* @description Regularize each individual face of the graph by sweeping up and down.
* @remarks Regularization of (the y-coordinates of) a graph entails inserting edges so that each
*       face has a single local minimum and a single local maximum, and so that holes have a bridge to their surrounding face.
* @remarks This routine calls ~mvureg_halfRegularizeGraphMultipleAttempts twice---the second time
*       on a locally 180-degree rotated graph.
* @remarks This routine assumes some prior process has connected holes to parents.
        Per-face logic respects periodics: each face is assumed consistent, but cross-edge seam is preserved.
* @param graphP     IN OUT  graph header
* @group "VU Meshing"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void vureg_regularizeConnectedInteriorFaces
(
VuSetP graphP
)
    {
    static int sweepUp = 1;
    static int sweepDown = 1;
    static int attemptLimit = 3;

    if ( sweepUp )
        {
        vureg_halfRegularizeGraphMultipleAttemptsExt (graphP, true, attemptLimit);
        }

    if ( sweepDown )
        {
        vu_rotate180 (graphP );
        vureg_halfRegularizeGraphMultipleAttemptsExt (graphP, true, attemptLimit);
        vu_rotate180 (graphP );
        }
    }
END_BENTLEY_GEOMETRY_NAMESPACE
