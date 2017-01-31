/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/gpa/gp_polygon.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#define CLIP_UNKNOWN 0
#define CLIP_IN  -1
#define CLIP_OUT 1

/*----------------------------------------------------------------------+
|                                                                       |
|   Local type definitions                                              |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   Private Global variables                                            |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   Public GEOMDLLIMPEXP Global variables                                             |
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

/*======================================================================+
|                                                                       |
|   Major Public GEOMDLLIMPEXP Code Section                                           |
|                                                                       |
+======================================================================*/

/*----------------------------------------------------------------------+
|FUNC           jmdlGraphicsPointArray_clipToOnePlane                              |
|AUTHOR         EarlinLutz                              4/95            |
+----------------------------------------------------------------------*/
static bool    jmdlGraphicsPointArray_addClippedBezierSegments
(
        GraphicsPointArray  *pDest,         /* <=> receiving array */
        int                     *pReadIndex,    /* <=> index where bezier begins */
        bool                    *pClipped,       /* <=> indicates if any clipping occured. */
        GraphicsPointArray  *pSource,       /* => source array */
const   DPoint4d                *pPlane         /* => the clipping plane */
)
    {
    DPoint4d poleArray[MAX_BEZIER_ORDER];
    DPoint4d leftPoleArray[MAX_BEZIER_ORDER];

    double   funcArray[MAX_BEZIER_ORDER];
    double   rootArray[MAX_BEZIER_ORDER + 2];
    int      numRoot;
    int      iTest;
    static double smallInterval = 1.0e-6;
    double s0, s1, sMax, t, fTest;
    int numPole;
    int i;
    bool    funcStat = false;
    int iRead = *pReadIndex;
    if (jmdlGraphicsPointArray_getBezier (pSource, &iRead, poleArray, &numPole, MAX_BEZIER_ORDER))
        {
        for (i = 0; i < numPole; i++)
            funcArray[i] = bsiDPoint4d_dotProduct (poleArray + i, pPlane);

        funcStat = true;
        *pReadIndex = iRead;

        if (bsiBezier_univariateRoots (rootArray, &numRoot, funcArray, numPole))
            {

            if (numRoot == 0)
                {
                /* The whole thing is on one side or the other.   Just look at the start point to
                            classify everything */
                if (funcArray[0] < 0.0)
                    {
                    jmdlGraphicsPointArray_addDPoint4dBezier (pDest, poleArray, numPole, 1, false);
                    jmdlGraphicsPointArray_markBreak (pDest);
                    }
                else
                    {
                    if (pClipped)
                        *pClipped = true;
                    }
                }
            else
                {
                iTest = (numPole - 1) / 2;
                s0 = 0.0;
                sMax = 1.0 - smallInterval;
                i  = 0;

                /* Invariant: poleArray containsthe poles from s0 to 1 */
                /* Remark: double roots will cause repeated subdivision at local parameter 0.
                    Just let it happen.  The small interval test will ignore the repetition, and
                    doing the 'extra' computation ensures that the interval really creeps to the right
                    if roots are close but not exact */
                for (i = 0; i < numRoot && s0 < sMax; i++)
                    {
                    s1 = rootArray[i];
                    t = (s1 - s0) / (1.0 - s0);
                    bsiBezier_subdivisionPolygons(
                                        (double *)leftPoleArray,
                                        (double *)poleArray,
                                        (double *)poleArray,
                                        numPole,
                                        4,
                                        t);

                    if (s1 - s0 > smallInterval)
                        {
                        bsiBezier_functionAndDerivative
                                    (&fTest, NULL, funcArray, numPole, 1, 0.5 * (s0 + s1));
                        if (fTest <= 0.0)
                            {
                            jmdlGraphicsPointArray_addDPoint4dBezier (pDest, leftPoleArray, numPole, 1, false);
                            jmdlGraphicsPointArray_markBreak (pDest);
                            }
                        else
                            {
                            if (pClipped)
                                *pClipped = true;
                            }
                        }
                    s0 = s1;
                    }

                if (s0 < sMax)
                    {
                    /* The residual pole array is the final interval */
                        bsiBezier_functionAndDerivative
                                    (&fTest, NULL, funcArray, numPole, 1, 0.5 * (s0 + 1.0));
                        if (fTest <= 0.0)
                            {
                            jmdlGraphicsPointArray_addDPoint4dBezier (pDest, poleArray, numPole, 1, false);
                            jmdlGraphicsPointArray_markBreak (pDest);
                            }
                        else
                            {
                            if (pClipped)
                                *pClipped = true;
                            }
                    }
                }
            }
        }
    return funcStat;
    }

/*----------------------------------------------------------------------+
|FUNC           jmdlGraphicsPointArray_clipConicSweepToPlane            |
|AUTHOR         EarlinLutz                              4/95            |
+----------------------------------------------------------------------*/
static void    jmdlGraphicsPointArray_clipDConic4dToPlane
(
        GraphicsPointArray      *pDestHeader,
const   DConic4d    *pConic,
        bool        *pClipped,
const   DPoint4d    *pPlane
)
    {
    DPoint3d trigPoint[2];
    DPoint4d midPoint;
    double clipFraction[4];
    double theta0 = pConic->start;
    double sweep  = pConic->sweep;
    int numBound = 0;
    double fraction;
    double alpha0, alpha1, alphaMid, fMid;

    double tolerance;
    static double s_relTol = 1.0e-8;
    static double s_zeroDot = 1.0e-10;
    static double smallAngle = 1.0e-6;
    int i;
    int numIntersection;
    /* I know that this only returns a single sweep sector ... */
    numIntersection = bsiDConic4d_intersectPlane
                    (
                    pConic,
                    trigPoint,
                    pPlane
                    );

    tolerance = s_relTol *
                ( fabs (bsiDPoint4d_dotProduct (&pConic->center,   pPlane))
                + fabs (bsiDPoint4d_dotProduct (&pConic->vector0,  pPlane))
                + fabs (bsiDPoint4d_dotProduct (&pConic->vector90, pPlane))
                );

    numBound = 0;
    clipFraction[numBound++] = 0.0;
    for (i = 0; i < numIntersection; i++)
        {
        fraction = bsiTrig_normalizeAngleToSweep (trigPoint[i].z, theta0, sweep);
        if (fraction > 0.0 && fraction < 1.0)
            clipFraction [numBound++] = fraction;
        }

    clipFraction[numBound++] = 1.0;
    if (numBound == 4 && clipFraction[1] > clipFraction[2])
        {
        fraction = clipFraction[1];
        clipFraction[1] = clipFraction[2];
        clipFraction[2] = fraction;
        }

    /* Classify each interval by altituded test at its midpoint */
    for (i = 1, alpha0 = theta0; i < numBound; i++, alpha0 = alpha1)
        {
        alpha1 = theta0 + sweep * clipFraction[i];
        if (fabs (alpha1 - alpha0) > smallAngle)
            {
            alphaMid = 0.5 * (alpha0 + alpha1);
            bsiDConic4d_angleParameterToDPoint4d (pConic, &midPoint, alphaMid);
            fMid = bsiDPoint4d_dotProduct (&midPoint, pPlane);
            if (fMid <= s_zeroDot)
                {
                DConic4d clippedConic;
                clippedConic = *pConic;
                clippedConic.start = alpha0;
                clippedConic.sweep = alpha1 - alpha0;
                jmdlGraphicsPointArray_addDConic4d (pDestHeader, &clippedConic);
                jmdlGraphicsPointArray_markBreak (pDestHeader);
                }
            else
                {
                if (pClipped)
                    *pClipped = true;
                }
            }
        }
    jmdlGraphicsPointArray_markBreak (pDestHeader);
    }


/*---------------------------------------------------------------------------------**//**
*
* Complete the clipping process for a single plane.
* Caller is responsible for computing altitude-above-plane as the "a" value in each GraphicsPoint.
* @bsimethod                                                    EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlGraphicsPointArray_clipToOnePlane
(
        GraphicsPointArray  *pInstance,     /* <=> Polyline or polygon. */
        bool                    *pClipped,      /* <=> set true if clipped */
const   DPoint4d                *pPlane         /* => the clipping plane */
)
    {
    double h0,h1;       /* Altitude above plane at 0,1 point of current segment */
    static int createEdges = false;
    int i,n;
    double s;
    GraphicsPoint *pSourceArray;
    DPoint4d newPoint;
    int curveType;

    GraphicsPointArray *pSourceHeader = jmdlGraphicsPointArray_grab ();
    jmdlGraphicsPointArray_append (pSourceHeader, pInstance);

    pSourceArray = jmdlGraphicsPointArray_getPtr (pSourceHeader, 0);
    n = jmdlGraphicsPointArray_getCount (pSourceHeader);

    jmdlGraphicsPointArray_empty (pInstance);

    if (!pSourceHeader)
        {
        /* This really shouldn't happen.  Just let the points return unchanged. */
        }
    else
        {
        int oldState = CLIP_UNKNOWN;
        /* Reminder: The point BEFORE any isolated point in the array
            is marked as a break.  This gives a useful fact for the loop:
            it is only necessary to test for isolated points in the block
            that handles CLIP_UNKNOWN (i.e. edge to current point is not
            considered)
        */
        for ( i=0; i < n; i++)
            {
            curveType = HPOINT_GET_CURVETYPE_BITS (pSourceArray[i].mask);
            if (curveType)
                {
                DConic4d conic;
                int index = i;
                if (curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
                    {
                    if (jmdlGraphicsPointArray_getDConic4d
                                            (
                                            pSourceHeader,
                                            &index,
                                            &conic,
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL))
                        {
                        i = index - 1;
                        jmdlGraphicsPointArray_clipDConic4dToPlane
                                        (pInstance, &conic, pClipped, pPlane);
                        }
                    }
                else
                    {
                    *pClipped = true;       /* NEEDS WORK .... */
                    if (jmdlGraphicsPointArray_addClippedBezierSegments
                                        (pInstance, &index, pClipped, pSourceHeader, pPlane))
                        i = index - 1;
                    }
                oldState = CLIP_UNKNOWN;
                }
            else
                {
                /* Linestring ... */
                *pClipped = true;
                if (oldState == CLIP_IN)
                    {
                    if (pSourceArray[i].a <= 0.0)
                        {
                        jmdlGraphicsPointArray_addDPoint4d (pInstance, &pSourceArray[i].point);
                        }
                    else
                        {
                        /* This segment crosses going outward */
                        h0 = pSourceArray[i-1].a;
                        h1 = pSourceArray[i].a;
                        s = h0 / (h0 - h1);     /* IN to OUT guarantees 0 <= s <= 1 */
                        bsiDPoint4d_interpolate
                                        (
                                        &newPoint,
                                        &pSourceArray[i-1].point,
                                        s,
                                        &pSourceArray[i].point
                                        );
                        jmdlGraphicsPointArray_addDPoint4d( pInstance, &newPoint );
                        if (!createEdges)
                            jmdlGraphicsPointArray_markBreak (pInstance);
                        oldState = CLIP_OUT;
                        }
                    }
                else if (oldState == CLIP_OUT)
                    {
                    if (pSourceArray[i].a <= 0.0)
                        {
                        *pClipped = true;
                        /* This segment crosses going outward */
                        h0 = pSourceArray[i-1].a;
                        h1 = pSourceArray[i].a;
                        s = h0 / (h0 - h1);     /* IN to OUT guarantees 0 <= s <= 1 */
                        bsiDPoint4d_interpolate
                                        (
                                        &newPoint,
                                        &pSourceArray[i-1].point,
                                        s,
                                        &pSourceArray[i].point
                                        );
                        jmdlGraphicsPointArray_addDPoint4d (pInstance, &newPoint);
                        jmdlGraphicsPointArray_addDPoint4d (pInstance, &pSourceArray[i].point);
                        oldState = CLIP_IN;
                        }
                    }
                else
                    {
                    if (!(pSourceArray[i].mask & HPOINT_MASK_BREAK))
                        {
                        /* Test the current point */
                        oldState = (pSourceArray[i].a <= 0.0)
                                 ? CLIP_IN : CLIP_OUT;
                        if (CLIP_IN == oldState)
                            {
                            jmdlGraphicsPointArray_addDPoint4d (pInstance, &pSourceArray[i].point);
                            }
                        }
                    else if (pSourceArray[i].mask & HPOINT_MASK_POINT)
                        {
                        if (pSourceArray[i].a <= 0.0)
                            {
                            jmdlGraphicsPointArray_addDPoint4d (pInstance, &pSourceArray[i].point);
                            jmdlGraphicsPointArray_markPoint (pInstance);
                            }
                        oldState = CLIP_UNKNOWN;
                        }
                    }
                if (pSourceArray[i].mask & HPOINT_MASK_BREAK)
                    {
                    oldState = CLIP_UNKNOWN;
                    jmdlGraphicsPointArray_markBreak (pInstance);
                    }
                }
            }
        jmdlGraphicsPointArray_drop( pSourceHeader );
        }
    }


/*---------------------------------------------------------------------------------**//**
*
* Complete the clipping process for a single plane, treating the geometry as filled figures.
* Caller is responsible for computing altitude-above-plane as the "a" value in each GraphicsPoint.
* @bsimethod                                                    EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlGraphicsPointArray_clipFilledToOnePlane
(
        GraphicsPointArray  *pInstance, /* <=> Polyline or polygon. */
        bool                    *pClipped,  /* <=> set true if clipped */
const   DPoint4d                *pPlane     /*  => the clipping plane */
)
    {
    GPFragments_hdr *pGPGraph       = jmdlGPFragments_grab ();
    GraphicsPointArray      *pScratchArray = jmdlGraphicsPointArray_grab ();
    GraphicsPointArray      *pOutputArray = jmdlGraphicsPointArray_grab ();
    int         iReadPos = 0;

    while (jmdlGPFragments_createPlaneClipFragments (pGPGraph,  &iReadPos, pInstance,pPlane))
        {
        jmdlGPFragments_sortAndJoinAlongMaximalAxis (pGPGraph);
        jmdlGPFragments_joinDanglers (pGPGraph, pScratchArray);

        jmdlGPFragments_extractEdgesToGraphicsPointArray (pGPGraph, pOutputArray);
        jmdlGraphicsPointArray_markMajorBreak (pOutputArray);
        *pClipped = true;
        }

    pOutputArray->arrayMask |= pInstance->arrayMask & HPOINT_ARRAYMASK_FILL;

    jmdlGraphicsPointArray_swap (pOutputArray, pInstance);

    jmdlGraphicsPointArray_drop (pScratchArray);
    jmdlGraphicsPointArray_drop (pOutputArray);
    jmdlGPFragments_drop (pGPGraph);
    }


/*----------------------------------------------------------------------+
|FUNC               jmdlGraphicsPointArray_findBreak                            |
|AUTHOR         EarlinLutz                              6/96            |
| Return the index AFTER the first break starting at i0.                |
+----------------------------------------------------------------------*/
static int jmdlGraphicsPointArray_findBreak     /* <= -1 if all IN, 1 if all OUT, 0 if mixed */
(
const GraphicsPoint *pPoint,
      int           i0,
      int           n
)
    {
    int currMask;
    int i = i0;

    if (n <= 0 || i0 >= n)
        return n;

    do
        {
        currMask = pPoint[i].mask;
        i++;
        } while (i < n && !(currMask & HPOINT_MASK_BREAK));

    return i;
    }


#ifdef PHASE_SUPPORT

/*----------------------------------------------------------------------+
|FUNC               jmdlGraphicsPointArray_clipPhaseToOnePlane                  |
|AUTHOR         EarlinLutz                              6/96            |
| Clip to one plane, carrying along phase distances.                    |
| Definition: Phase distance = accumulated distance from pen down on    |
| original polyline.                                                    |
| Phase distance is in scratchP[].w.                                    |
+----------------------------------------------------------------------*/
void jmdlGraphicsPointArray_clipPhaseToOnePlane
(
GraphicsPointArray         **ppHeader,     /* <=> Polyline or polygon. */
bool            *pClipped,      /* <= set true if clipped */
const DPoint4d  *pPlane         /* => homogeneous plane equations */
)

    {
    int i0, i1, i,n;
    double d0, d1;
    int inAt0 = 0;

    GraphicsPointArray* pSourceHeader = *ppHeader;
    double fraction, h0, h1;
    int *maskP;
    int side;

    DPoint4d *pointP;
    DPoint4d *scratchP;
    DPoint4d newPoint, newPhase;
    n = pSourceHeader->nPoint;

    if (n <= 0)
        {
        }
    else if ((side = jmdlGraphicsPointArray_setScratchXHeight (pSourceHeader, pPlane)) > 0)
        {
        /* All out.  Clear the array. */
        jmdlGraphicsPointArray_clear (*ppHeader);
        }
    else if (side < 0)
        {
        /* Nothing to do -- all IN */
        }
    else
        {
        /* Have to do a real clip */
        GraphicsPointArray *pDestHeader = jmdlGraphicsPointArray_grab ();

        *pClipped = true;
        maskP = pSourceHeader->maskP;
        scratchP = pSourceHeader->scratchP;
        pointP   = pSourceHeader->pointP;
        if (!pDestHeader)
            {
            /* This really shouldn't happen.  Just let the points return unchanged. */
            }
        else
            {
            *ppHeader = pDestHeader;
            newPhase.x = newPhase.y = newPhase.z = newPhase.w = 0.0;
            for ( i0 = 0; i0 < n; i0 = i1)
                {
                i1 = jmdlGraphicsPointArray_findBreak (maskP, i0, n);
                if (i1 > i0 + 1)
                    {
                    inAt0 = 0;
                    h0 = scratchP[i0].x;
                    d0 = scratchP[i0].w;
                    if (h0 <= 0.0)
                        {
                        newPhase.w = d0;
                        newPoint   = pointP[i0];
                        jmdlGraphicsPointArray_addDPoint4dPair (pDestHeader, &newPoint, &newPhase);
                        inAt0 = 1;
                        }

                    /* Lazy intersection -- collect all the crossing points as a single chain */
                    /* This generates some extras for ON edges */
                    for (i = i0 + 1; i < i1; i++)
                        {
                        h1 = scratchP[i].x;
                        d1 = scratchP[i].w;

                        if (!inAt0)    /* h0 > 0.0 */
                            {
                            if (h1 < 0)
                                {
                                fraction = - h0 / ( h1 - h0);
                                bsiDPoint4d_interpolate (&newPoint, pointP + i - 1, fraction, pointP + i);
                                newPhase.w = d0 + bsiDPoint4d_realDistance (pointP + i - 1, &newPoint);
                                jmdlGraphicsPointArray_addDPoint4dPair (pDestHeader, &newPoint, &newPhase);
                                jmdlGraphicsPointArray_addDPoint4dPair (pDestHeader, pointP + i, scratchP + i);
                                inAt0 = 1;
                                }
                            }
                        else /* h0 <= 0, inAt0 */
                            {
                            if (h1 > 0.0)
                                {
                                fraction = - h0 / ( h1 - h0);
                                bsiDPoint4d_interpolate (&newPoint, pointP + i - 1, fraction, pointP + i);
                                newPhase.w = d0 + bsiDPoint4d_realDistance (pointP + i - 1, &newPoint);
                                jmdlGraphicsPointArray_addDPoint4dPair (pDestHeader, &newPoint, &newPhase);
                                jmdlGraphicsPointArray_markBreak (pDestHeader);
                                inAt0 = 0;
                                }
                            else
                                {
                                newPhase.w = d1;
                                jmdlGraphicsPointArray_addDPoint4dPair (pDestHeader, pointP + i, &newPhase);
                                }
                            }
                        h0 = h1;
                        d0 = d1;
                        }
                    if (inAt0)
                        jmdlGraphicsPointArray_markBreak (pDestHeader);
                    }
                }
            }
        }
    }



/*----------------------------------------------------------------------+
|FUNC               jmdlGraphicsPointArray_initPhase                            |
|AUTHOR         EarlinLutz                              6/96            |
| Setup phase data in an GraphicsPointArray array.                                  |
| Definition: Phase distance = accumulated distance from pen down on    |
| original polyline.                                                    |
| Phase distance is in scratchP[].w.                                    |
+----------------------------------------------------------------------*/
void jmdlGraphicsPointArray_initPhase
(
GraphicsPointArray         *pHeader,                    /* <=> Polyline or polygon. */
bool            restartAtBreaks     /* => if true, reset distance to 0 at each break */
)

    {
    int i,n;
    double distance;

    int *maskP;
    DPoint4d *pointP;
    DPoint4d *scratchP;
    n = pHeader->nPoint;

    if (n > 0)
        {
        maskP = pHeader->maskP;
        scratchP = pHeader->scratchP;
        pointP   = pHeader->pointP;
        scratchP[0].w = distance = 0.0;
        for ( i = 1; i < n; i++)
            {
            if ((maskP[i-1] & HPOINT_MASK_BREAK) && restartAtBreaks)
                {
                distance = 0.0;
                }
            else
                {
                distance += bsiDPoint4d_realDistance (&pointP[i], &pointP[i-1]);
                }
            scratchP[i].w = distance;
            }
        }
    }

#endif   // PHASE_SUPPORT


/*---------------------------------------------------------------------------------**//**
* Intersect a planar halfspace, assuming (a) convex faces and (b) no curves.
* Replace instance arrays by clipped.
* @bsimethod                                                    EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlGraphicsPointArray_intersectOnePlane
(
      GraphicsPointArray    *pInstance,
const DPoint4d                  *pPlane
)
    {
    int i0, i1, i;
    GraphicsPointArray* pSourceHeader;
    double fraction, h0, h1, dh;
    GraphicsPoint *pSourceArray = jmdlGraphicsPointArray_getPtr (pInstance, 0);

    int n = jmdlGraphicsPointArray_getCount (pInstance);
    DPoint4d newPoint;

    if (n <= 0 || jmdlGraphicsPointArray_setHeight (pInstance, pPlane) != 0)
        {
        /* All in or all out.  Return empty. */
        jmdlGraphicsPointArray_empty (pInstance);
        }
    else
        {
        /* Have to do a real clip from a copy of the array. */
        pSourceHeader = jmdlGraphicsPointArray_grab ();

        if (!pSourceHeader)
            {
            /* This really shouldn't happen.  */
            }
        else
            {
            jmdlGraphicsPointArray_append (pSourceHeader, pInstance);
            jmdlGraphicsPointArray_empty (pInstance);
            pSourceArray = jmdlGraphicsPointArray_getPtr (pSourceHeader, 0);

            for ( i0 = 0; i0 < n; i0 = i1)
                {
                i1 = jmdlGraphicsPointArray_findBreak (pSourceArray, i0, n);
                if (i1 > i0 + 1)
                    {
                    h0 = pSourceArray[i0].a;
                    /* Lazy intersection -- collect all the crossing points as a single chain */
                    /* This generates some extras for ON edges */
                    for (i = i0 + 1; i < i1; i++)
                        {
                        h1 = pSourceArray[i].a;
                        if (h1 * h0 <= 0.0)
                            {
                            dh = h1 - h0;
                            if (dh != 0.0)
                                fraction = - h0 / dh;
                            else
                                fraction = 0.0;
                            bsiDPoint4d_interpolate
                                        (
                                        &newPoint,
                                        &pSourceArray[i - 1].point,
                                        fraction,
                                        &pSourceArray[i].point
                                        );
                            jmdlGraphicsPointArray_addDPoint4d (pInstance, &newPoint);
                            }
                        h0 = h1;
                        }
                    jmdlGraphicsPointArray_markBreak (pInstance);
                    }
                }
            }
        }
    }

#ifdef CLIP_OUTSIDE
#define MAX_PLANE 32


/*----------------------------------------------------------------------+
|FUNC               jmdlGraphicsPointArray_clipOutsidePlanes                    |
|AUTHOR         EarlinLutz                              6/96            |
| Generate the intersection lines between a plane an an GraphicsPointArray mesh.   |
|NORET                                                                  |
+----------------------------------------------------------------------*/
void        jmdlGraphicsPointArray_clipOutsidePlanes
(
        GraphicsPointArray     **ppHeader,     /* <=> Polyline or polygon. */
        bool        *pClipped,      /* <= set true if clipped */
const   DPoint4d    *planeArrayP,   /*  => homogeneous plane equations */
        int         numPlane        /*  => number of planes */
)

    {
            double      h0[MAX_PLANE], h1[MAX_PLANE];
            double      a0,a1;
            double      s,s0,s1;
            int         i,n, nIn, nOut, n0, n1, n01;
            int         j;
            GraphicsPointArray     *pSourceHeader = *ppHeader;
            int         *maskP;
            DPoint4d    *pointP;
    const   DPoint4d    *pPlane;
            DPoint4d    *scratchP;
            DPoint4d    newPoint;

    n = pSourceHeader->nPoint;
    if (n <= 1) return;


    /*
    **
    ** To maintain terminology with in-clipping, say 'in' for points
    ** in the convex volume.  Our goal here is to find the 'out' points.
    **
    ** A line segment can exit and leave a convex volume at most once.
    ** Hence there can be at most 2 segments OUT segments.  In
    ** the two-segment case, the OUT sections have parameter ranges
    **  0..s0 and s1..1.  If there is just one segment it is 0..s0 or s1..1
    **
    ** Represent this by the two numbers s0.
    ** Initially (before any segment is known to be IN), set s0=0 and s1=1.
    ** During clipping, quick exit to a full segment when s0>=s1.
    **
    */

    if (numPlane > MAX_PLANE)
        numPlane = MAX_PLANE;

    maskP    = pSourceHeader->maskP;
    pointP   = pSourceHeader->pointP;
    scratchP = pSourceHeader->scratchP;
    nIn = nOut = n0 = n1 = n01 = 0;

    /* Stoke the process by classifying point0 */
    for (j = 0, pPlane = planeArrayP; j < numPlane; j++, pPlane++)
        {
        h0[j] =         pPlane->x * pointP->x
                    +   pPlane->y * pointP->y
                    +   pPlane->z * pointP->z
                    +   pPlane->w * pointP->w;
        }

    maskP++;
    pointP++;
    scratchP++;

   /* For each segment:
        Find the s0..s1 values.
        Save them as scratchP->x and scratchP->y

        NB: pointP, maskP, scratchP are at the index i, which is the END
                of the segment being processed.  Hence have to look back at
                maskP[-1] to check properties at the beginning.
   */

    for (i = 0;
        i < n;
        i++, maskP++, pointP++, scratchP++ )
        {

        if (maskP[-1] & HPOINT_MASK_BREAK)
            {
            /* It's a null segment -- evaluate altitudes at the endpoint and skip out */
            for (j = 0, pPlane = planeArrayP; j < numPlane; j++, pPlane++)
                {
                h0[j] =         pPlane->x * pointP->x
                            +   pPlane->y * pointP->y
                            +   pPlane->z * pointP->z
                            +   pPlane->w * pointP->w;
                }
            continue;
            }


        /* Evaluate the endpoint heights */
        for (j = 0, pPlane = planeArrayP; j < numPlane; j++, pPlane++)
            {
            h1[j] =         pPlane->x * pointP->x
                        +   pPlane->y * pointP->y
                        +   pPlane->z * pointP->z
                        +   pPlane->w * pointP->w;
            }


        s0 = 0.0;
        s1 = 1.0;
        for (j = 0; j < numPlane && s0 < s1; j++)
            {
            a0 = h0[j];
            a1 = h1[j];
            if (a0 * a1 >= 0.0)
                {
                /* No crossing */
                if (a0 >= 0.0)
                    {
                    /* fully OUT for this plane.  Force exit from the loop */
                    s0 = s1 = 1.0;
                    }
                else
                    {
                    /* fully IN for this plane.  Limits stay unchanged */
                    }
                }
            else if (a0 < 0.0)
                {
                /* Cross IN to OUT */
                s = -a0 / ( a1 - a0);
                if (s < s1)
                    s1 = s;
                }
            else
                {
                /* Cross OUT to IN */
                s = -a0 / ( a1 - a0);
                if (s > s0)
                    s0 = s;
                }
            }

        if (s0 >= s1)
            {
            /* Force all fully out cases to look alike */
            s0 = s1 = 1.0;
            }
        scratchP->x = s0;
        scratchP->y = s1;

        if (s0 >= s1)
            {
            nOut++;
            }
        else if (s0 == 0.0)
            {   /* 0..s0 OUT segment is empty */
            if (s1 == 1.0)
                {   /* and so is s1..1 */
                nIn++;
                }
            else
                {
                n1++;
                }
            }
        else
            {  /* 0..s0 OUT segment is not empty */
            if (s1 == 1.0)
                {
                n0++;
                }
            else
                {
                /* 2 segments */
                n01++;
                }
            }

        /* make current endpoint next segment's start */
        for (j = 0, pPlane = planeArrayP; j < numPlane; j++, pPlane++)
            {
            h0[j] = h1[j];
            }
        }



    /* Phase II: Output the visible parts */

    if (n0 + n01 + n1 + nIn == 0)
        {
        /* Return untouched */
        }
    else if (nOut + n01 + n0 + n1 == 0)
        {
        /* The whole thing is 'in' the region -- toss it */
        jmdlGraphicsPointArray_clear (pSourceHeader);
        }
    else
        {
        /* Alas, the clipping adds/removes points.  We have to do a real clip
            into a new destination array, then get rid of the original */

        GraphicsPointArray *pDestHeader = jmdlGraphicsPointArray_grab ();

        *pClipped = true;
        maskP = pSourceHeader->maskP;
        scratchP = pSourceHeader->scratchP;
        pointP   = pSourceHeader->pointP;
        if (!pDestHeader)
            {
            /* This really shouldn't happen.  Just let the points return unchanged. */
            }
        else
            {
            int oldState = 0;
            *ppHeader = pDestHeader;
            for ( i=1; i < n; i++)
                {
                if (maskP[i-1] & HPOINT_MASK_BREAK)
                    {
                    jmdlGraphicsPointArray_markBreak (pDestHeader);
                    oldState = 0;
                    }
                else
                    {
                    s0 = scratchP[i].x;
                    s1 = scratchP[i].y;
                    if (s0 > 0.0)
                        {
                        /* Be sure the beam is on from the start ... */
                        if (!oldState)
                            jmdlGraphicsPointArray_addDPoint4d (pDestHeader, pointP + i - 1);
                        if (s0 == 1.0)
                            {
                            jmdlGraphicsPointArray_addDPoint4d (pDestHeader, pointP + i);
                            oldState = 1;
                            }
                        else
                            {
                            bsiDPoint4d_interpolate (&newPoint, pointP + i - 1, s0, pointP + i);
                            jmdlGraphicsPointArray_addDPoint4d (pDestHeader, &newPoint);
                            jmdlGraphicsPointArray_markBreak (pDestHeader);
                            oldState = 0;
                            }
                        }

                    if (s1 < 1.0)
                        {
                        if (oldState)
                            jmdlGraphicsPointArray_markBreak (pDestHeader);
                        bsiDPoint4d_interpolate (&newPoint, pointP + i - 1, s1, pointP + i);
                        jmdlGraphicsPointArray_addDPoint4d (pDestHeader, &newPoint);
                        jmdlGraphicsPointArray_addDPoint4d (pDestHeader, pointP + i);
                        oldState = 1;
                        }
                    else if (s0 <= 0.0)
                        {
                        oldState = 0;
                        }
                    }
                }
            jmdlGraphicsPointArray_drop( pSourceHeader );
            }
        }
    }
#endif



/*---------------------------------------------------------------------------------**//**
* Clip hpoints to a region bounded by planes.
* @param pInstance <=> polyline or polyline data to be clipped.
* @param pClipped <=> set true if any clipping occurs.  NOT CHANGED if no clipping, i.e.
*                   can be used as ongoing summary for multiple steps.
* @param pPlane => array of homogeneous plane coordinates.
*               Each plane defines a halfspace, with the sign convention that the
*               positive halfspace is OUT, negative IN.
* @param nPlane => number of planes.
* clipType => 0 for intersection of halfspaces (negative parts), 1 for outside.
* @bsimethod                                                    EarlinLutz      09/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void jmdlGraphicsPointArray_clipToPlanes
(
      GraphicsPointArray    *pInstance,
      bool                      *pClipped,
const DPoint4d                  *pPlane,
      int                       nPlane,
      int                       clipType
)
    {
    int iPlane;
    static int s_enableClipFilled = -1;
    static int s_bypassClip = 0;
    int         clipFilled = jmdlGraphicsPointArray_getArrayMask (pInstance, HPOINT_ARRAYMASK_FILL);

    if (s_bypassClip)
        return;

    if (s_enableClipFilled == 0)
        clipFilled = 0;
    else if (s_enableClipFilled == 1)
        clipFilled = 1;


    if (nPlane <= 0)
        {
        /* leave it alone */
        }
    else if (clipType == 1)
        {
        //jmdlGraphicsPointArray_clipOutsidePlanes (ppHeader, pClipped, pPlane, nPlane);
        // NEEDS WORK -- outside clip
        }
    else if (clipType == 2)
        {
        for ( iPlane = 0; iPlane < nPlane; iPlane++)
            {
            // NEEDS WORK -- phase clip
            //jmdlGraphicsPointArray_clipPhaseToOnePlane (ppHeader, pClipped, &pPlane[iPlane]);
            }
        }
    else
        {
        int inOut;
        for ( iPlane = 0; iPlane < nPlane; iPlane++)
            {
            inOut = jmdlGraphicsPointArray_setHeight ( pInstance, &pPlane[iPlane] );
            if (inOut > 0)
                {
                /* All out.  We are done. */
                jmdlGraphicsPointArray_empty (pInstance);
                }
            else if (inOut < 0)
                {
                /* All in here. Continue with further planes. */
                }
            else
                {
                /* Real work to do on this plane. */
                if (clipFilled)
                    jmdlGraphicsPointArray_clipFilledToOnePlane (pInstance, pClipped, &pPlane[iPlane]);
                else
                    jmdlGraphicsPointArray_clipToOnePlane (pInstance, pClipped, &pPlane[iPlane]);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Form the dot product between the xy1 components of the DPoint3d and the xyw components
* of the DPoint4d.
*
* @param        =>
* @return
* @bsimethod                                                    EarlinLutz      06/98
+---------------+---------------+---------------+---------------+---------------+------*/
static double  jmdlGraphicsPointArray_dotProductXY1XYW
(
const DPoint3d  *pA,
const DPoint4d  *pB
)
    {
    return  pA->x * pB->x + pA->y * pB->y + pA->z * pB->w;
    }

/*---------------------------------------------------------------------------------**//**
* Form the dot product between the xy1 components of the DPoint3d and the xyw components
* of the DPoint4d.
*
* @param        =>
* @return
* @bsimethod                                                    EarlinLutz      06/98
+---------------+---------------+---------------+---------------+---------------+------*/
static int compareGraphicsPointA
(
const void *pvA,
const void *pvB
)
    {
    GraphicsPointCP pA = (GraphicsPointCP)pvA;
    GraphicsPointCP pB = (GraphicsPointCP)pvB;

    if (pA->a > pB->a)
        return  1;
    if (pA->a < pB->a)
        return  -1;
    return  0;;
    }

/*---------------------------------------------------------------------------------**//**
* Add a point to the GraphicsPointArray array, with given "a" value and userData set
* to its index in the array.
* @bsimethod                                                    EarlinLutz      06/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGraphicsPointArray_addWithScratchParameter
(
        GraphicsPointArray  *pInstance,
const   DPoint4d                *pPoint,
        double                  a
)
    {
    GraphicsPoint gpoint;
    int idNew = jmdlGraphicsPointArray_getCount (pInstance);
    bsiGraphicsPoint_initFromDPoint4d (&gpoint,pPoint, a, 0, idNew);
    jmdlGraphicsPointArray_addDPoint4d (pInstance, pPoint);
    }


/*---------------------------------------------------------------------------------**//**
* Clip hpoints to the frustum obtained by sweeping a polygon in the z direction.
* @param   **ppHeader  <=> Polyline data.  This procedure has the authority to clip
*                           in place or into a new header.  In the latter case the given
*                           header is returned to jmdlGraphicsPointArray_drop(.)
* @param    *pClipped  <=> set true the data is altered in any way.  Left unchanged
*                           otherwise.
* @param    pPointArray => array of points treated as a closed polygon.
* @param    numPoint    => number of points.
* @param    clipType    =>  0 for clip to inside of polygon, with xor rules.
*                           1 for clip to outisde, with xor rules.
* @bsimethod                                                    EarlinLutz      06/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_clipToXYPolygon
(
      GraphicsPointArray    *pInstance,
      bool                      *pClipped,
const DPoint3d                  *pPolygonPoint,
      int                       numPolygonPoint,
      int                       clipType
)
    {
    int     numLinePoint = jmdlGraphicsPointArray_getCount (pInstance);

    static double s_normalizationTol = 1.0e-10;
    static double s_relTol = 1.0e-10;
    DPoint4d    linePoint;
    DPoint3d    xyzPoint;
    DPoint4d hPoint0, hPoint1;
    DPoint3d realPoint0, realPoint1;
    double h1, dh, h0;
    int i, j, j0, j1, k, m;
#define BLOCK_SIZE 100
    DPoint3d basePoint[BLOCK_SIZE];
    DPoint3d vector[BLOCK_SIZE];
    double   UU[BLOCK_SIZE];
    double   hSave[BLOCK_SIZE];
    int mask;
    double hTol;
    double uv;
    int numInBlock;
    DPoint4d scratchPoint;
    int numAdded = 0;
    double ds;
    DPoint3d testPoint;
    int parity, i0, i1, im;

    memset (&scratchPoint, 0, sizeof (DPoint4d));

    if (numPolygonPoint > 2)
        {
        /* Deal with the polygon in blocks to reuse cross products */
        j0 = 0;

        do
            {
            jmdlGraphicsPointArray_getDPoint4dWithMask (pInstance, &hPoint0, &mask, 0);

            j1 = j0 + BLOCK_SIZE - 1;
            if (j1 >= numPolygonPoint)
                j1 = numPolygonPoint;
            hTol = 0.0;
            for (j = j0; j < j1; j++)
                {
                k = j + 1;
                if (k >= numPolygonPoint)
                    k = 0;
                m = j - j0;
                basePoint[m] = pPolygonPoint[j];
                vector[m].x = - (pPolygonPoint[k].y - pPolygonPoint[j].y);
                vector[m].y =    pPolygonPoint[k].x - pPolygonPoint[j].x;
                vector[m].z =
                      pPolygonPoint[j].x * pPolygonPoint[k].y
                    - pPolygonPoint[j].y * pPolygonPoint[k].x;
                UU[m]       = vector[m].x * vector[m].x + vector[m].y * vector[m].y;
                hSave[m] = jmdlGraphicsPointArray_dotProductXY1XYW (&vector[m], &hPoint0);
                if (fabs (hSave[m]) > hTol)
                    hTol = fabs (hSave[m]);
                }

            numInBlock = j1 - j0;
            hTol *= s_relTol;
            for (i = 1; i < numLinePoint; i++, hPoint0 = hPoint1)
                {
                /* Find all intersections of edge [i-1..i] with the polygon.
                    Record them as additional GraphicsPointArray with the edge's parameter
                    space mapped to i-1..i stored as the scratch x component.
                */
                /* hSave contains the altitudes of point i-1 (hPoint0) relative to all edges */
                jmdlGraphicsPointArray_getDPoint4dWithMask (pInstance, &hPoint1, &mask, i);
                for (m = 0; m < numInBlock ; m++)
                    {
                    h0 = hSave[m];
                    h1 = hSave[m] = jmdlGraphicsPointArray_dotProductXY1XYW (&vector[m], &hPoint1);
                    dh = h0 - h1;
                    if (fabs (dh) < hTol)
                        {
                        }
                    else if (h1 * h0 <= 0.0)
                        {
                        /* (-h1, h0) are the homogeneous coordinates of the intersection
                                point wrt to the line.  Get the projection of the point back in
                                the real xy plane. */
                        bsiDPoint4d_add2ScaledDPoint4d (&linePoint, NULL, &hPoint0, -h1 / dh, &hPoint1, h0 / dh);
                        xyzPoint.x = linePoint.x;
                        xyzPoint.y = linePoint.y;
                        xyzPoint.z = linePoint.z;
                        if (fabs (linePoint.w - 1.0) > s_normalizationTol)
                            {
                            double f = 1.0 / linePoint.w;
                            xyzPoint.x *= f;
                            xyzPoint.y *= f;
                            xyzPoint.z *= f;
                            }
                        /* Get the parameter on the polygon edge */
                        /* Remember that the stored vector is the perpendicular */
                        uv =  vector[m].y * (xyzPoint.x - basePoint[m].x)
                            - vector[m].x * (xyzPoint.y - basePoint[m].y);
                        if (uv >= 0.0 && uv < UU[m])
                            {
                            ds = h0 / dh;
                            /* printf(" intersection on edge (%d,%d) at %s\n", i-1, i, ds); */
                            jmdlGraphicsPointArray_addWithScratchParameter
                                        (
                                        pInstance,
                                        &linePoint,
                                        (double) (i - 1) + ds
                                        );
                            numAdded++;
                            }
                        }
                    }
                }

            j0 = j1;
            } while (j0 < numPolygonPoint);

        if (numAdded == 0)
            {
            /* Parity test on any point indicates whether the polyline is all in or
                all out */
            jmdlGraphicsPointArray_getDPoint4dWithMask (pInstance, &hPoint0, &mask, 0);
            bsiDPoint4d_normalize (&hPoint0, &testPoint);
            parity = bsiGeom_XYPolygonParity (&testPoint, pPolygonPoint, numPolygonPoint, hTol);
            if (clipType == 1)
                parity = - parity;
            if (parity < 0)
                jmdlGraphicsPointArray_empty (pInstance);
            }
        else
            {
            GraphicsPointArray *pOutHeader;
            double s0,sm;
            int k;
            /* Add the last polyline point as an artificial endpoint */
            jmdlGraphicsPointArray_getDPoint4dWithMask (pInstance, &hPoint1, &mask, numLinePoint - 1);
            jmdlGraphicsPointArray_addWithScratchParameter (pInstance, &hPoint1, (double)(numLinePoint - 1));
            numAdded++;

            qsort (jmdlGraphicsPointArray_getPtr (pInstance, numLinePoint), numAdded, sizeof (GraphicsPoint),
                            compareGraphicsPointA);

            pOutHeader = jmdlGraphicsPointArray_grab ();
            jmdlGraphicsPointArray_getDPoint4dWithMask (pInstance, &hPoint0, &mask, 0);

            s0 = 0.0;
            i0 = 0;
            m = 0;

            while (m < numAdded)
                {
                GraphicsPoint currGP;
                jmdlGraphicsPointArray_getGraphicsPoint (pInstance, &currGP, numLinePoint + m);
                sm = currGP.a;
                im = (int)sm;
                k = currGP.userData;
                jmdlGraphicsPointArray_getDPoint4dWithMask (pInstance, &hPoint1, &mask, k);

                if (sm <= s0)
                    {
                    /* repeated crossing -- no span to output */
                    m++;
                    }
                else if (im == i0)
                    {
                    /* crossing is within the current segment */
                    bsiDPoint4d_normalize (&hPoint0, &realPoint0);
                    bsiDPoint4d_normalize (&hPoint1, &realPoint1);
                    bsiDPoint3d_interpolate (&testPoint, &realPoint0, 0.5, &realPoint1);
                    parity = bsiGeom_XYPolygonParity (&testPoint, pPolygonPoint, numPolygonPoint, hTol);
                    if (clipType == 1)
                        parity = - parity;
                    if (parity > 0)
                        {
                        jmdlGraphicsPointArray_addDPoint4d (pOutHeader, &hPoint0);
                        jmdlGraphicsPointArray_addDPoint4d (pOutHeader, &hPoint1);
                        jmdlGraphicsPointArray_markBreak (pOutHeader);
                        }
                    s0 = sm;
                    hPoint0 = hPoint1;
                    m++;
                    i0 = im;
                    }
                else
                    {
                    int iTest;
                    i1 = (int)sm;
                    iTest = (i0 + 1 + i1) / 2;
                    jmdlGraphicsPointArray_getDPoint4dWithMask (pInstance, &hPoint0, &mask, iTest);
                    bsiDPoint4d_normalize (&hPoint0, &testPoint);
                    parity = bsiGeom_XYPolygonParity (&testPoint, pPolygonPoint, numPolygonPoint, hTol);
                    if (clipType == 1)
                        parity = - parity;
                    if (parity > 0)
                        {
                        jmdlGraphicsPointArray_addDPoint4d (pOutHeader, &hPoint0);
                        i = i0 + 1;
                        while (i <= im)
                            {
                            jmdlGraphicsPointArray_getDPoint4dWithMask (pInstance, &hPoint0, &mask, i);
                            jmdlGraphicsPointArray_addDPoint4d (pOutHeader, &hPoint0);
                            i++;
                            }
                        jmdlGraphicsPointArray_addDPoint4d (pOutHeader, &hPoint1);
                        jmdlGraphicsPointArray_markBreak (pOutHeader);
                        }
                    hPoint0 = hPoint1;
                    s0 = sm;
                    i0 = im;
                    m++;
                    }
                }

            jmdlGraphicsPointArray_swap (pInstance, pOutHeader);
            jmdlGraphicsPointArray_drop (pOutHeader);
            }
        }
    }


END_BENTLEY_GEOMETRY_NAMESPACE