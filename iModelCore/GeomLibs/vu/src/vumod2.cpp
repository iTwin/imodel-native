/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/*----------------------------------------------------------------------+
|                                                                       |
| name          vu_splitEdgeAtV                                         |
|                                                                       |
| author        earlinLutz                                      10/94   |
|                                                                       |
+----------------------------------------------------------------------*/
static void     vu_splitEdgeAtV (
VuSetP          graphP,         /* Parent set where the new nodes are added */
VuP             startP,         /* Starting vu of edge to split */
double          v,              /* v coordinate for new point. */
VuP            *leftP,          /* <= new vu on left of edge */
VuP            *rightP          /* <= new vu on right of edge */
)
    {
    double          t, u, v0;
    VuP             endP = VU_FSUCC (startP);
    v0 = VU_V (startP);
    t = (v - v0) / (VU_V (VU_FSUCC (startP)) - v0);
    u = VU_U (startP) + t * (VU_U (endP) - VU_U (startP));
    vu_splitEdge (graphP, startP, leftP, rightP);
    VU_SET_UV (*leftP, u, v);
    VU_SET_UV (*rightP, u, v);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          searchSortedArrayAbove                                  |
|                                                                       |
| author        earlinLutz                                      10/94   |
|                                                                       |
+----------------------------------------------------------------------*/
static int      searchSortedArrayAbove (       /* <= the index of the least entry strictly larger than a
                                                   This value is nv if all entries are a or less */
double          a,              /* => test key */
double         *v,              /* => array of keys */
int             nv              /* => number of keys */
)
    {
    int             i;
    for (i = 0; i < nv && v[i] <= a; i++)
        {
        }
    return i;
    }


/*---------------------------------------------------------------------------------**//**
* @description Insert horizontal edges across a face that is known to be monotone in the v-coordinate.
* @param graphP         IN OUT  graph header
* @param sliceArrayP    OUT     array to receive the sliced faces, one node per new face
* @param startP         IN      any starter node in the face
* @param v              IN      ascending array of altitudes (v-coordinates of horizontal split lines)
* @param maskP          IN      array of masks to install on new edges
* @param nv             IN      number of altitudes/masks
* @param fixedMask      IN      additional mask to apply to new edges
* @group "VU Meshing"
* @bsimethod                                                    EarlinLutz      10/94
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_splitMonotoneFaceOnSortedHorizontalLines
(
VuSetP          graphP,
VuArrayP        sliceArrayP,
VuP             startP,
double          *v,
VuMask          *maskP,
int             nv,
VuMask          fixedMask
)
    {
    VuP             nextP, leftP, rightP;
    VuP             insideP, outsideP;
    VuP             newLeftP, newRightP;
    double          v0, v1, vLine;
    int             i;
    VuMask          mask;

    /*    Look for a local min (which should also be a global min) */
    nextP = VU_FSUCC (startP);
    if (VU_BELOW (nextP, startP))
        {
        do
            {
            startP = nextP;
            nextP = VU_FSUCC (nextP);
            }
        while (VU_BELOW (nextP, startP));
        }
    else
        {
        for (nextP = VU_FPRED (startP);
             VU_BELOW (nextP, startP);
             startP = nextP, nextP = VU_FPRED (nextP))
            {
            }
        ;
        }


    i = searchSortedArrayAbove (VU_V (startP), v, nv);
    /*    Walk upwards on both sides, inserting and joining
       cross lines at each v value  */
    leftP = rightP = startP;
    vu_arrayAdd (sliceArrayP, leftP);

    while (i < nv)
        {

        mask = maskP[i] | fixedMask;
        vLine = v[i++];

        /*      Search and subdivide on the left */
        v1 = VU_V (leftP);
        nextP = leftP;
        do
            {
            leftP = nextP;
            nextP = VU_FPRED (leftP);
            v0 = v1;
            v1 = VU_V (nextP);
            }
        while (v1 < vLine && VU_BELOW (leftP, nextP));

        if (v1 > vLine)
            {
            /*      subdivide the left edge */
            vu_splitEdgeAtV (graphP, nextP, vLine, &insideP, &outsideP);
            leftP = insideP;
            }
        else
            {
            /*      Continue moving as long as the edge moves
               strictly to the right */
            do
                {
                leftP = nextP;
                nextP = VU_FPRED (nextP);
                }
            while (VU_V (nextP) == v1 && VU_U (nextP) > VU_U (leftP));
            }

        /*      Search and subdivide on the right */
        v1 = VU_V (rightP);
        nextP = rightP;
        do
            {
            rightP = nextP;
            nextP = VU_FSUCC (rightP);
            v0 = v1;
            v1 = VU_V (nextP);
            }
        while (v1 < vLine && VU_BELOW (rightP, nextP));

        if (v1 > vLine)
            {
            /*      subdivide the left edge */
            vu_splitEdgeAtV (graphP, rightP, vLine, &insideP, &outsideP);
            rightP = insideP;
            }
        else
            {
            rightP = nextP;
            }

        if (leftP != rightP
            && VU_V (leftP) == vLine
            && VU_V (rightP) == vLine)
            {
            vu_join (graphP, leftP, rightP, &newLeftP, &newRightP);
            VU_SETMASK(newLeftP,mask);
            VU_SETMASK(newRightP,mask);
            leftP = newLeftP;
            vu_arrayAdd (sliceArrayP, newLeftP);
            }
        else
            {
            i = nv;             /* forces loop exit */
            }
        }
    }

/*
   Typical 'meshing' operations on a vu graph -- horizontal line slicing,
   triangulation of monotone face.

   These algorithms assume that all vu nodes have the same coordinate space,
   hence coordinates can be copied blindly to opposite sides of edges
   (no seams!!!).

   The cautious numerical coder will notice many exact floating point equality
   tests!!!  These are quite intentional.  The theory is that these numbers are
   generated by COPYING, and from a strictly topological standpoint the ordering
   imposed by the floating point space is just as good as integers. Yes, that is
   frightening to think about.  However, it works remarkably well.  A LONG discussion
   is needed to clarify that!!
*/
#ifdef CompileAll
/*----------------------------------------------------------------------+
|                                                                       |
| name          vu_canTriangulateByFanFromLeft                          |
|                                                                       |
| author        earlinLutz                                      10/94   |
|                                                                       |
+----------------------------------------------------------------------*/
static int      vu_canTriangulateByFanFromLeft /* <= 1 indicates if triangulation from leftP to the
                                                   right side can be a simple fan, i.e. there are no left edges that
                                                   hide others when viewed from leftP */
(
                 VuP leftP,     /* <= First VU above bottom on left side of polygon whose sides both go up */
                 VuP rightP     /* <= Last VU on right side and beloew leftP */
)
    {
    VuP             P1 = VU_FSUCC (leftP);
    VuP             P2 = VU_FSUCC (P1);
    while (P2 != rightP && P2 != leftP)
        {
        if (vu_cross (leftP, P1, P2) <= 0)
            return 0;
        P1 = P2;
        P2 = VU_FSUCC (P2);
        }
    return 1;
    }
#endif
static int      count = 0;
/*----------------------------------------------------------------------+
|                                                                       |
| name          check_monotoneV                                         |
|                                                                       |
| author        earlinLutz                                      10/94   |
|                                                                       |
+----------------------------------------------------------------------*/
static int      check_monotoneV
(
VuSetP graphP,
VuP startP
)
    {
    VuP             leftP = startP, rightP = startP;
    int             error = 0;
    count++;
    while (VU_BELOW (leftP, VU_FPRED (leftP)))
        {
        leftP = VU_FPRED (leftP);
        }
    while (VU_BELOW (rightP, VU_FSUCC (rightP)))
        {
        rightP = VU_FSUCC (rightP);
        }
    if (leftP != rightP)
        {
        error = 1;
        }
    if (error)
        vu_panic (graphP);
    return error;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vu_triangulateFromCentroid                              |
|                                                                       |
| author        earlinLutz                                      10/94   |
|                                                                       |
+----------------------------------------------------------------------*/
static int     vu_triangulateFromCentroid
(
VuSetP          graphP,
VuP             startP,         /* => any vu on face */
int             check           /* => if true, check if all are visible
                                        before doing the constrution */
)
    {
    DPoint2d uvCentroid;
    VuP centerP, fringeP, new0P, new1P;
    double area;
    int status = ERROR;
    double dA;
    double du0, dv0, du1, dv1;
    VuP start1P;
    int nPos, nNeg, nEdge;

    if (SUCCESS == vu_centroid (&uvCentroid, &area, &nPos, &nNeg, startP)
        && nPos + nNeg > 3
       )
        {
        status = SUCCESS;
        nEdge = nPos + nNeg + 2;
        if (check && nNeg > 0)
            {
            /* Some edges where reversed when viewd from the
                start point.  We ae non-convex, but still might be
                ok for triangulation from the centroid if inward
                bends are small
            */
            nPos = nNeg = 0;
            start1P = VU_FSUCC(startP);
            du0 = VU_U(startP) - uvCentroid.x;
            dv0 = VU_U(startP) - uvCentroid.y;
            VU_FACE_LOOP (currP, start1P)
                {
                du1 = VU_U(currP) - uvCentroid.x;
                dv1 = VU_V(currP) - uvCentroid.y;
                dA = VU_CROSS_PRODUCT_FROM_COMPONENTS (du0, dv0, du1, dv1);
                if (dA > 0.0)
                    {
                    nPos++;
                    }
                else
                    {
                    nNeg++;
                    }
                du0 = du1;
                dv0 = dv1;
                }
            END_VU_FACE_LOOP (currP, start1P)
            if (nNeg > 0)
                status = ERROR;
            }

        if (SUCCESS == status)
            {
            /* Make the first edge to the fringe */
            vu_makePair (graphP, &centerP, &new1P);
            VU_U (new1P) = VU_U(startP);
            VU_V (new1P) = VU_V(startP);
            VU_U (centerP) = uvCentroid.x;
            VU_V (centerP) = uvCentroid.y;
            vu_vertexTwist (graphP, new1P, startP);
            for (;--nEdge > 0;)
                {
                fringeP = VU_FSUCC( VU_FSUCC( centerP));
                vu_join (graphP, centerP, fringeP, &new0P, &new1P);
                centerP = new0P;
                }
            }
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @description Triangulate a face that is monotone in the v-coordinate.
* @remarks For best results, startP should be a lexical minimum (i.e., it should have the
*       least u-coordinate among those nodes with the least v-coordinate); otherwise, a triangulation
*       is attempted starting at the centroid of the face.
* @param graphP IN OUT  graph header
* @param startP IN      v-minimum node in face
* @group "VU Meshing"
* @bsimethod                                                    EarlinLutz      10/94
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_triangulateByVerticalSweep
(
VuSetP          graphP,
VuP             startP
)
    {
    VuP             leftP = VU_FPRED (startP);
    VuP             rightP = VU_FSUCC (startP);
    VuP             P0, P1, P2;
    VuP             A, B;

    if (check_monotoneV (graphP, startP))
        {
        /* startP failed the min test.  Try to work from the
        centroid */
        vu_triangulateFromCentroid (graphP, startP, true);
        return;
        }

    /*    Loop invariant: startP is the bottom node. */
    while (leftP != rightP
           && rightP != startP
           && VU_FSUCC (rightP) != leftP
    /* These conditions should never happen !! */
           && vu_cross (leftP, startP, rightP) > 0
           && VU_BELOW (startP, leftP)
           && VU_BELOW (startP, rightP)
        )
        {
        if (VU_BELOW (leftP, rightP))
            {
            /*      Triangulate to all left side edges that
               are below rightP */

            /*      Phase 1: move upward, adding back edges
               when prior nodes are visible. */
            P0 = leftP;
            P1 = startP;
            P2 = rightP;
            /*      Invariant: the path from P0 back to P1 is concave.
               Each loop pass moves P0 up the left side, filling in
               edges as needed.  The right side edge
               (following startP) is never altered.
             */
            while (P0 != P2 && VU_BELOW (P0, rightP))
                {
                while (P2 != rightP
                       && P2 != P0
                       && P2 != P1
                       && vu_cross (P0, P1, P2) > 0)
                    {
                    vu_join (graphP, P0, P2, &A, &B);
                    P0 = A;
                    P1 = VU_FSUCC (P0);
                    P2 = VU_FSUCC (P1);
                    }
                P2 = P1;
                P1 = P0;
                P0 = VU_FPRED (P0);
                }
            /*      Phase 2: Fan out edges from rightP to the
               left side. P0.P1.P2 describes a pair of
               adjacent edges at the bottom. */
            leftP = P1;
            P2 = rightP;
            P1 = VU_FPRED (P2);
            P0 = VU_FPRED (P1);
            while (VU_FSUCC (P2) != P0 && P0 != leftP)
                {
                vu_join (graphP, P0, P2, &A, &B);
                P1 = A;
                P0 = VU_FPRED (P1);
                }
            /*      Finish off with the last stroke from the
               left node to the right, except when already
               topped out */
            if (VU_FSUCC (P2) != P0)
                {
                vu_join (graphP, P0, P2, &A, &B);
                P0 = A;
                }
            startP = P0;
            rightP = VU_FSUCC (startP);
            leftP = VU_FPRED (startP);

            }
        else
            {
            /*      Triangulate to all right side edges that
               are below leftP */

            /*      Phase 1: move upward, adding back edges
               when prior nodes are visible. */
            P0 = leftP;
            P1 = startP;
            P2 = rightP;
            /*      Invariant: the path up to P1 is concave.
               Each loop pass advances P1, filling in
               edges as needed. Note that the
               startP edge may get hidden, so the
               bottom node must be referenced as
               VU_FSUCC(leftP) rather than as startP.
             */
            while (P0 != P2 && VU_BELOW (P2, leftP))
                {
                while (P0 != leftP
                       && P2 != P0
                       && P2 != P1
                       && vu_cross (P0, P1, P2) > 0)
                    {
                    vu_join (graphP, P0, P2, &A, &B);
                    P0 = VU_FPRED (A);
                    P1 = A;
                    }
                P0 = P1;
                P1 = P2;
                P2 = VU_FSUCC (P2);
                }
            /*      Phase 2: Fan out edges from leftP to the
               right side. P0.P1.P2 describes a pair of
               adjacent edges at the bottom. */
            rightP = P1;
            P0 = leftP;
            P1 = VU_FSUCC (P0);
            P2 = VU_FSUCC (P1);
            while (VU_FSUCC (P2) != P0 && P2 != rightP)
                {
                vu_join (graphP, P0, P2, &A, &B);
                P0 = A;
                P1 = P2;
                P2 = VU_FSUCC (P2);
                }
            /*      Finish off with the last stroke from the
               left node to the right, except when already
               topped out */
            if (VU_FSUCC (P2) != P0)
                {
                vu_join (graphP, P0, P2, &A, &B);
                }
            startP = rightP;
            rightP = VU_FSUCC (startP);
            leftP = VU_FPRED (startP);
            }
        }
    }
#ifdef CompileAll
/*----------------------------------------------------------------------+
|                                                                       |
| name          vumod_retreatToLocalMin                                 |
|                                                                       |
| author        earlinLutz                                      10/94   |
|                                                                       |
+----------------------------------------------------------------------*/
static VuP      vumod_retreatToLocalMin
(
VuP             startP
)
    {
    VuP             nextP;

    for (nextP = VU_FSUCC(startP); VU_BELOW (nextP, startP);)
        {
        startP = nextP;
        nextP = VU_FSUCC(startP);
        }

    for (nextP = VU_FPRED(startP); VU_BELOW (nextP, startP);)
        {
        startP = nextP;
        nextP = VU_FPRED(startP);
        }

    return startP;
    }
#endif
/*---------------------------------------------------------------------------------**//**
* @nodoc
* @description Slice a graph with the given vertical and horizontal lines and optionally triangulate the resulting partitions.
* @remarks This routine works best if each face of the graph is monotone in either the u- or v-coordinate.
* @param graphP             IN OUT  graph header
* @param uValP              IN      ascending array of u-coordinates of vertical split lines
* @param uMaskP             IN      array of masks to install on new vertical edges
* @param uValCount          IN      number of vertical split lines
* @param vValP              IN      ascending array of v-coordinates of horizontal split lines
* @param vMaskP             IN      array of masks to install on new horizontal edges
* @param vValCount          IN      number of horizontal split lines
* @param faceArrayP         IN OUT  scratch array for original faces
* @param uSliceMinArrayP    IN OUT  scratch array for faces after u-slices
* @param vSliceMinArrayP    IN OUT  scratch array for faces after v-slices
* @param triangulate        IN      whether to triangulate each slice with a v-sweep
* @see vu_splitMonotoneFaceOnSortedHorizontalLines
* @group "VU Meshing"
* @bsimethod                                                    EarlinLutz      10/94
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_sliceAndTriangulate
(
VuSetP          graphP,
double          *uValP,
VuMask          *uMaskP,
int             uValCount,
double          *vValP,
VuMask          *vMaskP,
int             vValCount,
VuArrayP        faceArrayP,
VuArrayP        uSliceMinArrayP,
VuArrayP        vSliceMinArrayP,
bool            triangulate
)
    {
    VuP             faceP;

    /* vu_checkNextInSet (graphP); */
    /*    TEMPORARILY ROTATE THE COORDINATES so the horizontal slice logic can be applied for
       vertical slices.  The {} brackets around this are just to emphasize the range
       of the coordinate flipflip. */
        {
        vu_rotate90CCW (graphP);
        /*      Slice by vertical (constant u) lines */
        vu_collectInteriorFaceLoops (faceArrayP, graphP);
        vu_arrayClear (uSliceMinArrayP);
        for (vu_arrayOpen (faceArrayP); vu_arrayRead (faceArrayP, &faceP);)
            {
            vu_splitMonotoneFaceOnSortedHorizontalLines (
                                             graphP, uSliceMinArrayP, faceP,
                                             uValP, uMaskP, uValCount, VU_U_SLICE );
            }

        vu_rotate90CW (graphP);
        }
    /* vu_checkNextInSet (graphP); */
    /*    Slice by horizontal (constant v) lines */
    vu_arrayClear (vSliceMinArrayP);
    for (vu_arrayOpen (uSliceMinArrayP); vu_arrayRead (uSliceMinArrayP, &faceP);)
        {
        VuP             minUP, maxUP, minVP, maxVP;

        vu_findExtrema (faceP, &minUP, &maxUP, &minVP, &maxVP);
        vu_splitMonotoneFaceOnSortedHorizontalLines (
                                             graphP, vSliceMinArrayP, faceP,
                                             vValP, vMaskP , vValCount, VU_V_SLICE );
        }
    /* vu_checkNextInSet (graphP); */
    if (triangulate)
        {
        for (vu_arrayOpen (vSliceMinArrayP); vu_arrayRead (vSliceMinArrayP, &faceP);)
            {
            /*      This depends on the slicer returning pointers to minima !! */
            double          area = vu_area (faceP);
            if (area > 0)
                vu_triangulateByVerticalSweep (graphP, faceP);
            }
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          vu_isSectorConvexAfterEdgeRemoval                       |
|                                                                       |
| author        earlinLutz                                      03/01   |
|                                                                       |
+----------------------------------------------------------------------*/
static bool    vu_isSectorConvexAfterEdgeRemoval
(
VuP edgeP
)
    {
    VuP leftP = vu_fpred (edgeP);
    VuP rightP = vu_fsucc (vu_vpred (edgeP));
    double cp = vu_cross (leftP, edgeP, rightP);
    DPoint2d vector0, vector1;
    if (cp > 0.0)
        return true;
    if (cp < 0.0)
        return false;
    vu_vector (&vector0, leftP, edgeP);
    vu_vector (&vector1, edgeP, rightP);
    return vector0.DotProduct (vector1) > 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @description Remove edges between interior faces if the union of the faces is convex.
* @param graphP IN OUT  graph header
* @group "VU Edges"
* @bsimethod                                                    EarlinLutz      03/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void    vu_removeEdgesToExpandConvexFaces
(
VuSetP  graphP,
VuMask  barrierMask
)
    {
    VuP mateP;

    VuMask visitMask = vu_grabMask (graphP);
    VuMask deleteMask = vu_grabMask (graphP);
    vu_clearMaskInSet (graphP, visitMask | deleteMask);

    VU_SET_LOOP (currP, graphP)
        {
        /* Is edge unvisited and interior? */
        if (!vu_getMask (currP, visitMask))
            {
            mateP = vu_edgeMate (currP);
            vu_setMask (currP, visitMask);
            vu_setMask (mateP, visitMask);
            if  (   !vu_getMask (currP, barrierMask)
                &&  !vu_getMask (mateP, barrierMask)
                &&  vu_isSectorConvexAfterEdgeRemoval (currP)
                &&  vu_isSectorConvexAfterEdgeRemoval (mateP)
                )
                {
                vu_vertexTwist (graphP, currP, vu_vpred (currP));
                vu_vertexTwist (graphP, mateP, vu_vpred (mateP));
                vu_setMask (currP, deleteMask);
                vu_setMask (mateP, deleteMask);
                }
            }
        }
    END_VU_SET_LOOP (currP, graphP)

    vu_freeMarkedEdges (graphP, deleteMask);

    vu_returnMask (graphP, visitMask);
    vu_returnMask (graphP, deleteMask);
    }

Public GEOMDLLIMPEXP void    vu_removeEdgesToExpandConvexInteriorFaces
(
VuSetP  graphP
)
    {
    vu_removeEdgesToExpandConvexFaces (graphP, VU_EXTERIOR_EDGE);
    }
/*---------------------------------------------------------------------------------**//**
* @description Triangulate monotone faces of the graph.
* @remarks It is assumed that the interior faces of the graph are v-monotone.  If they are u-monotone, pass true for flipCoordinates;
*       the graph will be rotated internally by 90 degrees, triangulated by vertical sweep, then rotated back.
* @param graphP             IN OUT  graph header
* @param flipCoordinates    IN      whether to triangulate by horizontal sweep instead of vertical sweep.
* @group "VU Meshing"
* @bsimethod                                                    EarlinLutz      10/94
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_triangulateMonotoneInteriorFaces
(
VuSetP      graphP,
bool        flipCoordinates
)
    {
    VuP faceP;

    VuArrayP faceArrayP = vu_grabArray ( graphP );

    if ( flipCoordinates ) vu_rotate90CCW ( graphP );

    vu_collectInteriorFaceLoops (faceArrayP, graphP);

    for ( vu_arrayOpen ( faceArrayP ) ; vu_arrayRead( faceArrayP , &faceP ) ; )
        {
        VuP             minUP, maxUP, minVP, maxVP;
        vu_findExtrema (faceP, &minUP, &maxUP, &minVP, &maxVP);
        vu_triangulateByVerticalSweep ( graphP, minVP );
        }

    if ( flipCoordinates ) vu_rotate90CW ( graphP );

    vu_returnArray ( graphP, faceArrayP );
    }

/***********************************************************************+
|                                                                       |
| Statics for recursive search for large faces                          |
|                                                                       |
| s_XXXX = static, not altered by search                                |
| sl_XXXX = may be used as a local variable (without consuming stack    |
|       space) but beware that recursion may change its value.          |
+***********************************************************************/
struct FaceSearchParams
    {
    VuMask permEdgeMask;
    int maxSuperFaceSize;
    int currentSuperFaceId;
    int currentSuperFaceSize;
    int mySize;
/* Limit the search to s_stepLimit total steps to cut off programmer errors
   that cause infinite recursion */
    int stepCount;
    int depth;
    int depthErrors;
    FaceSearchParams (VuMask _permEdgeMask, int _maxEdge)
        {
        permEdgeMask = _permEdgeMask;
        maxSuperFaceSize = _maxEdge;
        currentSuperFaceId = 0;
        currentSuperFaceSize = maxSuperFaceSize + 1; /* Force a new face immediately */
        stepCount = 0;
        depth = 0;
        depthErrors = 0;
        }
    } ;
static int s_depthLimit = 1000;
static int s_stepLimit = 100000;
/*---------------------------------------------------------------------+
|                                                                       |
| name          vumod_searchFaceLimit                                   |
|                                                                       |
| author        earlinLutz                                      10/94   |
|                                                                       |
+----------------------------------------------------------------------*/
static void vumod_searchFaceLimit      /* <= number of edges in augmented superface */
(
VuP             faceP,          /* => Seed point for search */
struct FaceSearchParams *paramP  /* => Passed in for thread safety */
)
    {
    int oldSuperFaceId;
    if (VU_GET_LABEL_AS_INT(faceP) >= 0)
        return;
    if (paramP->stepCount++ > s_stepLimit)
        return;

    paramP->mySize = vu_countEdgesAroundFace (faceP);

    /* Assign myself to the current superface or a new one: */
    if (paramP->mySize + paramP->currentSuperFaceSize - 2 > paramP->maxSuperFaceSize)
        {
        paramP->currentSuperFaceSize = paramP->mySize;
        paramP->currentSuperFaceId++;
        }
    else
        {
        paramP->currentSuperFaceSize += paramP->mySize;
        }

    /* Install the face id.  If an edges is shared with the current superface
        (ME!!!) decrement the edge count to recognize that the shared edge
        was counted on both sides.
    */
    VU_FACE_LOOP (currP, faceP)
        {
        VU_SET_LABEL (currP, paramP->currentSuperFaceId);
        if (VU_GET_LABEL_AS_INT (VU_EDGE_MATE(currP)) == paramP->currentSuperFaceId)
            paramP->currentSuperFaceSize -= 2;
        }
    END_VU_FACE_LOOP (currP, faceP)

    /* Recurse to lower level faces.  They will add themselves to the current
        supernode if possible, increment it if not */
    VU_FACE_LOOP (currP, faceP)
        {
        if (!VU_GETMASK(currP, paramP->permEdgeMask))
            {
            oldSuperFaceId = paramP->currentSuperFaceId;
            paramP->depth++;
            if (paramP->depth < s_depthLimit)
                vumod_searchFaceLimit (VU_EDGE_MATE(currP), paramP);
            else
                paramP->depthErrors++;
            paramP->depth--;
            /* Don't allow a new super face started within the just-completed
               recursion to jump into the next one */
            if (paramP->currentSuperFaceId != oldSuperFaceId)
                paramP->currentSuperFaceSize = paramP->maxSuperFaceSize;
            }
        }
    END_VU_FACE_LOOP (currP, faceP)
    return;
    }

/*---------------------------------------------------------------------------------**//**
* @description Split all v-monotone faces of the graph that have more than maxEdge edges.
* @param graphP     IN OUT  graph header
* @param maxEdge    IN      maximum number of edges for a new face
* @group "VU Meshing"
* @bsimethod                                                    EarlinLutz      10/95
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     vu_splitMonotoneFacesToEdgeLimit
(
VuSetP          graphP,
int             maxEdge
)
    {
    VuMask permEdgeMask = vu_grabMask (graphP);

    VuP faceP;

    VuArrayP faceArrayP = vu_grabArray ( graphP );

    vu_collectInteriorFaceLoops (faceArrayP, graphP);

    /* Mark all current edges as permanent */
    vu_setMaskInSet (graphP, permEdgeMask);

    /* Triangulate all faces with more than maxEdge edges */
    for (vu_arrayOpen (faceArrayP) ; vu_arrayRead(faceArrayP , &faceP) ; )
        {
        VuP             minUP, maxUP, minVP, maxVP;
        if (vu_countEdgesAroundFace(faceP) <= maxEdge)
            {
            vu_arrayRemoveCurrent (faceArrayP);
            }
        else
            {
            vu_findExtrema (faceP, &minUP, &maxUP, &minVP, &maxVP);
            vu_triangulateByVerticalSweep ( graphP, minVP );
            }
        }

    /* faceArray still has a pointer to a node on each original face (which
        is now triangulated).   The triangulation edges do NOT have the permEdgeMask
        marker
    */
    if (   vu_arraySize (faceArrayP) > 0
        && maxEdge > 3
        && permEdgeMask
        )
        {
        struct FaceSearchParams params (maxEdge, permEdgeMask);

        VU_SET_LOOP (currP, graphP)
            {
            VU_SET_LABEL (currP, -1);
            }
        END_VU_SET_LOOP (currP, graphP)
        for (vu_arrayOpen (faceArrayP); vu_arrayRead (faceArrayP, &faceP) ;)
            {
            vumod_searchFaceLimit (faceP, &params);
            }

        /* This sets each edge twice.  Not worth the hassle to optimize it */
        VU_SET_LOOP (currP, graphP)
            {
            VuP mateP = VU_EDGE_MATE(currP);
            int mateId = VU_GET_LABEL_AS_INT (currP);
            int myId = VU_GET_LABEL_AS_INT (mateP);
            if (   myId < 0 || mateId < 0 || myId != mateId
                || VU_GETMASK(currP, permEdgeMask)
                || VU_GETMASK(mateP, permEdgeMask)
                )
                {
                VU_SETMASK(currP, permEdgeMask);
                VU_SETMASK(mateP, permEdgeMask);
                }
            else
                {
                VU_CLRMASK(currP, permEdgeMask);
                VU_CLRMASK(mateP, permEdgeMask);
                }
            }
        END_VU_SET_LOOP (currP, graphP)
        if (params.stepCount < s_stepLimit && params.depthErrors == 0)
                vu_freeNonMarkedEdges (graphP, permEdgeMask);

        }
    vu_returnArray ( graphP, faceArrayP );
    vu_returnMask (graphP, permEdgeMask);
    }

/*---------------------------------------------------------------------------------**//**
* @description Split an edge at the given 3D point and insert its coordinates on both sides.
* @param graphP IN OUT  graph header
* @param leftP  OUT     new node at pointP on same side as edgeP
* @param rightP OUT     new node at pointP on opposite side
* @param edgeP  IN      base node of edge to split
* @param pointP IN      coordinates of new nodes
* @group "VU Edges"
* @see vu_splitEdge
* @bsimethod                                                    EarlinLutz      10/94
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void            vu_splitEdgeAtDPoint3d
(
VuSetP graphP,
VuP *leftP,
VuP *rightP,
VuP edgeP,
DPoint3d *pointP
)
    {
    vu_splitEdge(graphP,edgeP,leftP,rightP);
    VU_UVW (*leftP) = *pointP;
    VU_UVW (*rightP) = *pointP;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
