/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/MstnOnly/GeomPrivateApi.h>
#include "mtgintrn.h"
#include "../memory/ptrcache.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*----------------------------------------------------------------------+
|                                                                       |
|   Local defines                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
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
|   Public Global variables                                             |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|   External variables                                                  |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|    Macro Definitions                                                  |
|                                                                       |
+----------------------------------------------------------------------*/
#define FRAGMENT_ARRAY_BRIDGE_EDGE_MASK MTG_CONSTU_MASK
/*----------------------------------------------------------------------+
|                                                                       |
|   Private Type Definitions                                            |
|                                                                       |
+----------------------------------------------------------------------*/

#define FRAGMENTGRAPH_CACHE_SIZE 8

static PtrCache_Functions cacheFunctions =
    {
    (void *(*)(void))jmdlGPFragments_alloc,
    (void (*)(void *))jmdlGPFragments_free,
    (void (*)(void *))jmdlGPFragments_empty,
    NULL
    };

static PPtrCacheHeader pCache =
    omdlPtrCache_new (&cacheFunctions, FRAGMENTGRAPH_CACHE_SIZE);

typedef struct
    {
    MTGNodeId nodeId;
    DPoint4d    point;
    double      sortParam;
    } NodeSortParam;

/*---------------------------------------------------------------------------------**//**
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGPFragments_gripe
(
GPFragments_hdr *pInstance,
const char              *pMessage
)
    {
    printf ("jmdlGPFragments_gripe: %s\n", pMessage);
    }


/*---------------------------------------------------------------------------------**//**
* Allocate a fragment header to the caller.  If possible, the header
* is taken from a cache of headers that were previously used and hence
* may have preallocated memory for connectivity and coordinate data.
* Use jmdlGPFragments_drop to return the header to the cache.
* @param void
* @see
* @return pointer to the borrowed fragment header.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/

Public GEOMDLLIMPEXP GPFragments_hdr * jmdlGPFragments_grab
(
void
)
    {
    return (GPFragments_hdr *)omdlPtrCache_grabFromCache (pCache);
    }


/*---------------------------------------------------------------------------------**//**
* Drop a fragment graph back to the cache.
* @param pHeader => header to return to cache.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP GPFragments_hdr *jmdlGPFragments_drop
(
GPFragments_hdr     *pHeader
)
    {
    jmdlGPFragments_empty (pHeader);
    omdlPtrCache_dropToCache (pCache, pHeader);
    return NULL;
    }


/*---------------------------------------------------------------------------------**//**
* Allocate a fragment header from the system heap.
* @return pointer to newly allocated and initialized header structure.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/

Public GEOMDLLIMPEXP GPFragments_hdr * jmdlGPFragments_alloc
(
void
)
    {
    GPFragments_hdr *pHeader =
        (GPFragments_hdr *)BSIBaseGeom::Malloc (sizeof (GPFragments_hdr));
    jmdlGPFragments_init (pHeader);
    return pHeader;

    }


/*---------------------------------------------------------------------------------**//**
* Initialize a facet header structure.   Prior contents ignored (hence destroyed).
* @param pFacetHeader <= initialized structure.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/

Public GEOMDLLIMPEXP void jmdlGPFragments_init
(
GPFragments_hdr *pInstance
)
    {
    jmdlMTGGraph_initGraph (&pInstance->graph_hdr);
    jmdlGraphicsPointArray_init (&pInstance->gpa_hdr);
    pInstance->danglingNodeId = MTG_NULL_NODEID;
    pInstance->danglingNodeCount = 0;
    pInstance->hpointsIndexOffset = jmdlMTGGraph_defineLabel (&pInstance->graph_hdr,
                                0, MTG_LabelMask_VertexProperty, -1);
    omdlVArray_init (&pInstance->sortArray_hdr, sizeof(NodeSortParam));
    }


/*---------------------------------------------------------------------------------**//**
* Empty all connectivity and point data, but preserve overall definitions and allocated memory
* for quick reuse.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/

Public GEOMDLLIMPEXP void jmdlGPFragments_empty
(
GPFragments_hdr *pInstance
)
    {
    jmdlMTGGraph_emptyNodes (&pInstance->graph_hdr, true);
    jmdlGraphicsPointArray_empty (&pInstance->gpa_hdr);
    omdlVArray_empty (&pInstance->sortArray_hdr);
    pInstance->danglingNodeId = MTG_NULL_NODEID;
    pInstance->danglingNodeCount = 0;
    }


/*---------------------------------------------------------------------------------**//**
* Return a fragment header to the system.
* @return always NULL
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/

Public GEOMDLLIMPEXP MTGFacets * jmdlGPFragments_free
(
GPFragments_hdr *     pInstance
)
    {
    jmdlGPFragments_releaseMem (pInstance);
    BSIBaseGeom::Free (pInstance);
    return NULL;
    }


/*---------------------------------------------------------------------------------**//**
* Return the associated memory (but not the facet header itself) to
* the sytem heap.
* @param pInstance    => header for facets to free.
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/

Public GEOMDLLIMPEXP void jmdlGPFragments_releaseMem
(
GPFragments_hdr *     pInstance     // => header for facets to free.
)
    {
    if (pInstance)
        {
        jmdlGraphicsPointArray_releaseMem (&pInstance->gpa_hdr);
        jmdlMTGGraph_releaseMem (&pInstance->graph_hdr);
        }
    }



/*---------------------------------------------------------------------------------**//**
* @param pNode0Id <= nodeId at start of new edge.
* @param pNode1Id <= nodeId at end of new edge.
* @param previousNodeId => node id to which start node is to be twisted.
* @param pPoint0 => coordinates at start of fragment.
* @param freeEnd0 => true if the start node is to be twisted into the dangler loop.
* @param pPoint1 => coordinates at the end of fragment.
* @param freeEnd1 => true if the end node is to be twisted into the dangler loop.
* @param index0 => label to apply to the start node.
* @param index1 => label to apply to the end node.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
static void jmdlGPFragments_addAndLabelFragment
(
GPFragments_hdr       *pFragmentGraph,
      MTGNodeId *pNode0Id,
      MTGNodeId *pNode1Id,
      MTGNodeId previousNodeId,
const DPoint4d          *pPoint0,
      bool              freeEnd0,
const DPoint4d          *pPoint1,
      bool              freeEnd1,
      int               index0,
      int               index1
)
    {
    static int s_noisy = 0;
    jmdlMTGGraph_createEdge (&pFragmentGraph->graph_hdr, pNode0Id, pNode1Id);

    if (freeEnd0)
        {
        if (pFragmentGraph->danglingNodeId != MTG_NULL_NODEID)
            jmdlMTGGraph_vertexTwist (&pFragmentGraph->graph_hdr, pFragmentGraph->danglingNodeId, *pNode0Id);
        pFragmentGraph->danglingNodeCount++;
        pFragmentGraph->danglingNodeId = *pNode0Id;
        }

    if (freeEnd1)
        {
        if (pFragmentGraph->danglingNodeId != MTG_NULL_NODEID)
            jmdlMTGGraph_vertexTwist (&pFragmentGraph->graph_hdr, pFragmentGraph->danglingNodeId, *pNode1Id);
        pFragmentGraph->danglingNodeCount++;
        pFragmentGraph->danglingNodeId = *pNode1Id;
        }

    if (previousNodeId != MTG_NULL_NODEID)
        {
        if (freeEnd0 && s_noisy)
            jmdlGPFragments_gripe (pFragmentGraph,
                    "addSegment -- should not connect to both a free list and previous chain");
        jmdlMTGGraph_vertexTwist (&pFragmentGraph->graph_hdr, previousNodeId, *pNode0Id);
        }

    jmdlMTGGraph_setLabel (&pFragmentGraph->graph_hdr, *pNode0Id, pFragmentGraph->hpointsIndexOffset, index0);
    jmdlMTGGraph_setLabel (&pFragmentGraph->graph_hdr, *pNode1Id, pFragmentGraph->hpointsIndexOffset, index1);
    }


/*---------------------------------------------------------------------------------**//**
* @param pNode0Id <= nodeId at start of new edge.
* @param pNode1Id <= nodeId at end of new edge.
* @param previousNodeId => node id to which start node is to be twisted.
* @param pPoint0 => coordinates at start of fragment.
* @param freeEnd0 => true if the start node is to be twisted into the dangler loop.
* @param pPoint1 => coordinates at the end of fragment.
* @param freeEnd1 => true if the end node is to be twisted into the dangler loop.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
static     void        jmdlGPFragments_addSegment
(
GPFragments_hdr       *pFragmentGraph,
      MTGNodeId *pNode0Id,
      MTGNodeId *pNode1Id,
      MTGNodeId previousNodeId,
const DPoint4d          *pPoint0,
      bool              freeEnd0,
const DPoint4d          *pPoint1,
      bool              freeEnd1
)
    {
    int index0, index1;
    index0 = jmdlGraphicsPointArray_getCount (&pFragmentGraph->gpa_hdr);
    jmdlGraphicsPointArray_addDPoint4dWithMask (
                            &pFragmentGraph->gpa_hdr,
                            pPoint0, 0);
    jmdlGraphicsPointArray_addDPoint4dWithMask (
                            &pFragmentGraph->gpa_hdr,
                            pPoint1, HPOINT_MASK_FRAGMENT_BREAK);
    index1 = index0 + 1;
    jmdlGPFragments_addAndLabelFragment (pFragmentGraph, pNode0Id, pNode1Id,
                            previousNodeId, pPoint0, freeEnd0, pPoint1, freeEnd1, index0, index1);

    }


/*---------------------------------------------------------------------------------**//**
* @param pNode0Id <= nodeId at start of new edge.
* @param pNode1Id <= nodeId at end of new edge.
* @param previousNodeId => node id to which start node is to be twisted.
* @param pPoint0 => coordinates at start of fragment.
* @param freeEnd0 => true if the start node is to be twisted into the dangler loop.
* @param pPoint1 => coordinates at the end of fragment.
* @param freeEnd1 => true if the end node is to be twisted into the dangler loop.
* @param pEllipse => ellipse curve.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
static     void        jmdlGPFragments_addEllipseSegment
(
GPFragments_hdr       *pFragmentGraph,
      MTGNodeId *pNode0Id,
      MTGNodeId *pNode1Id,
      MTGNodeId previousNodeId,
const DPoint4d          *pPoint0,
      bool              freeEnd0,
const DPoint4d          *pPoint1,
      bool              freeEnd1,
const DConic4d          *pConic
)
    {
    int index0, index1;
    index0 = jmdlGraphicsPointArray_getCount (&pFragmentGraph->gpa_hdr);
    jmdlGraphicsPointArray_addDConic4dWithIndices (
                            &pFragmentGraph->gpa_hdr, pConic, &index0, &index1);

    jmdlGPFragments_addAndLabelFragment (pFragmentGraph, pNode0Id, pNode1Id,
                            previousNodeId, pPoint0, freeEnd0, pPoint1, freeEnd1, index0, index1);
    }


/*---------------------------------------------------------------------------------**//**
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/
static     void        jmdlGPFragments_addBezierSegment
(
GPFragments_hdr       *pFragmentGraph,
      MTGNodeId *pNode0Id,
      MTGNodeId *pNode1Id,
      MTGNodeId previousNodeId,
      bool              freeEnd0,
      bool              freeEnd1,
const DPoint4d          *pPoleArray,
      int               numPole
)
    {
    int index0, index1;
    index0 = jmdlGraphicsPointArray_getCount (&pFragmentGraph->gpa_hdr);
    jmdlGraphicsPointArray_addDPoint4dBezierWithIndices (
                            &pFragmentGraph->gpa_hdr,
                            &index0, &index1,
                            pPoleArray, numPole, 1, false);

    jmdlGPFragments_addAndLabelFragment (pFragmentGraph, pNode0Id, pNode1Id,
                            previousNodeId,
                            &pPoleArray[0],
                            freeEnd0,
                            &pPoleArray[numPole - 1],
                            freeEnd1,
                            index0,
                            index1
                            );
    }




/*---------------------------------------------------------------------------------**//**
* Classify a point wrt a plane.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+---------------+------*/

static void jmdlGraphicsPointArray_classifyPointWRTPlane
(
      int         *pSign,
      double      *pAltitude,
const DPoint4d    *pPoint,
const DPoint4d    *pPlane,
      double      tolerance
)
    {
    *pAltitude = bsiDPoint4d_dotProduct (pPoint, pPlane);
    if (*pAltitude > tolerance)
        {
        *pSign = 1;
        }
    else if (*pAltitude < - tolerance)
        {
        *pSign = -1;
        }
    else
        {
        *pSign = 0;
        }
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/

static void    jmdlGPFragments_addClippedEllipse
(
GPFragments_hdr   *pFragmentGraph,
        GraphicsPointArray      *pSourceHeader,
        int         i0,
        int         i1,
const   DPoint4d    *pStartPoint,
const   DPoint4d    *pEndPoint,
const   DPoint4d    *pPlane
)
    {
    DPoint3d trigPoint[2];
    DPoint4d midPoint, point0, point1;
    double clipFraction[4];
    int numBound = 0;
    double fraction;
    double theta0, sweep, f, f0, f1;
    double alpha0, alpha1, alphaMid;
    MTGNodeId node0Id, node1Id;

    double tolerance;
    static double s_relTol = 1.0e-8;
    int i;
    DEllipse4d ellipse;
    int numIntersection;
    /* I know that this only returns a single sweep sector ... */
    jmdlGraphicsPointArray_getDEllipse4d (pSourceHeader, &i0, &ellipse, &theta0, &sweep, NULL, NULL);
    numIntersection = bsiDEllipse4d_intersectPlane
                    (
                    trigPoint,
                    &ellipse.center,
                    &ellipse.vector0,
                    &ellipse.vector90,
                    pPlane
                    );

    tolerance = s_relTol *
                ( fabs (bsiDPoint4d_dotProduct (&ellipse.center,   pPlane))
                + fabs (bsiDPoint4d_dotProduct (&ellipse.vector0,  pPlane))
                + fabs (bsiDPoint4d_dotProduct (&ellipse.vector90, pPlane))
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

    bsiDEllipse4d_evaluateDPoint4d (&ellipse, &point0, theta0);
    alpha0 = theta0;
    f0 = bsiDPoint4d_dotProduct (&point0, pPlane);

    /* ARGHH.  How to tell if endpoint is on? */
    for (i = 1; i < numBound; i++, f0 = f1, alpha0 = alpha1, point0 = point1)
        {
        alpha1 = theta0 + sweep * clipFraction[i];
        alphaMid = 0.5 * (alpha0 + alpha1);
        DConic4d conic;
        conic.center = ellipse.center;
        conic.vector0 = ellipse.vector0;
        conic.vector90 = ellipse.vector90;
        conic.start = alpha0;
        conic.sweep = alpha1 - alpha0;

        bsiDEllipse4d_evaluateDPoint4d (&ellipse, &midPoint, alphaMid);
        f = bsiDPoint4d_dotProduct (&midPoint, pPlane);
        if (bsiTrig_equalAngles (alpha0, alpha1))
            point1 = point0;
        else
            bsiDEllipse4d_evaluateDPoint4d (&ellipse, &point1, alpha1);
        f1 = bsiDPoint4d_dotProduct (&point1, pPlane);
        if (f < 0.0)
            {
            jmdlGPFragments_addEllipseSegment (pFragmentGraph, &node0Id, &node1Id,
                            MTG_NULL_NODEID,
                            &point0, fabs (f0) <= tolerance,
                            &point1, fabs (f1) <= tolerance,
                            &conic
                            );
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPFragments_addClippedBezier
(
GPFragments_hdr   *pFragmentGraph,
        GraphicsPointArray      *pSourceHeader,
        int         i0,
        int         i1,
const   DPoint4d    *pStartPoint,
const   DPoint4d    *pEndPoint,
const   DPoint4d    *pPlane
)
    {
    DPoint4d poleArray      [MAX_BEZIER_CURVE_ORDER];
    DPoint4d leftPoleArray  [MAX_BEZIER_CURVE_ORDER];
    double   altitudeArray  [MAX_BEZIER_CURVE_ORDER];
    double   rootArray      [MAX_BEZIER_CURVE_ORDER];
    int numPole;
    int numRoot;
    double s0, s1, sMax, t, f0, f1, fTest;
    MTGNodeId node0Id, node1Id;

    double tolerance;
    static double s_relTol = 1.0e-8;
    static double s_smallInterval = 1.0e-8;
    int i;

    if (!jmdlGraphicsPointArray_getBezier
                (
                pSourceHeader,
                &i0,
                poleArray,
                &numPole,
                MAX_BEZIER_CURVE_ORDER
                ))
        {
        tolerance = 0.0;
        }
    else
        {

        tolerance = 0.0;
        for (i = 0; i < numPole; i++)
            {
            altitudeArray[i] = bsiDPoint4d_dotProduct (poleArray + i, pPlane);
            f0 = fabs (altitudeArray[i]);
            if (f0 > tolerance)
                {
                tolerance = f0;
                }
            }
        tolerance *= s_relTol;

        if (bsiBezier_univariateRoots (rootArray, &numRoot, altitudeArray, numPole))
            {
            if (numRoot == numPole)
                {
                /* The bezier is entirely ON the plane */
                f0 = altitudeArray[0];
                f1 = altitudeArray[numPole - 1];
                jmdlGPFragments_addBezierSegment
                                (
                                pFragmentGraph, &node0Id, &node1Id,
                                MTG_NULL_NODEID,
                                fabs (f0) <= tolerance,
                                fabs (f1) <= tolerance,
                                poleArray,
                                numPole
                                );
                }
            else if (numRoot == 0)
                {
                /* The whole thing is on one side or the other.   Just look at the start point to
                            classify everything */
                f0 = altitudeArray[0];
                f1 = altitudeArray[numPole - 1];
                if (f0 < 0.0)
                    {
                    jmdlGPFragments_addBezierSegment (pFragmentGraph, &node0Id, &node1Id,
                                MTG_NULL_NODEID,
                                fabs (f0) <= tolerance,
                                fabs (f1) <= tolerance,
                                poleArray,
                                numPole
                                );
                    }
                }
            else
                {
                s0 = 0.0;
                sMax = 1.0 - s_smallInterval;
                i  = 0;
                f0 = altitudeArray[0];

                /* Invariant: poleArray containsthe poles from s0 to 1 */
                /* Remark: double roots will cause repeated subdivision at local parameter 0.
                    Just let it happen.  The small interval test will ignore the repetition, and
                    doing the 'extra' computation ensures that the interval really creeps to the right
                    if roots are close but not exact */
                for (i = 0; i < numRoot && s0 < sMax; i++, f0 = f1)
                    {
                    s1 = rootArray[i];
                    t = (s1 - s0) / (1.0 - s0);
                    f1 = 0.0;   /* It's a root, right? */
                    bsiBezier_subdivisionPolygons(
                                        (double *)leftPoleArray,
                                        (double *)poleArray,
                                        (double *)poleArray,
                                        numPole,
                                        4,
                                        t);

                    if (s1 - s0 > s_smallInterval)
                        {
                        bsiBezier_functionAndDerivative
                                    (&fTest, NULL, altitudeArray, numPole, 1, 0.5 * (s0 + s1));
                        if (fTest <= 0.0)
                            {
                            /* Hm ... these are at least nominally zero, EXCEPT for the
                                    first pass f0 (which might be an endpoint */
                            jmdlGPFragments_addBezierSegment
                                        (
                                        pFragmentGraph,
                                        &node0Id, &node1Id,
                                        MTG_NULL_NODEID,
                                        fabs (f0) <= tolerance,
                                        fabs (f1) <= tolerance,
                                        leftPoleArray,
                                        numPole
                                        );
                            }
                        }
                    s0 = s1;
                    }

                if (s0 < sMax)
                    {
                    /* The residual pole array is the final interval */
                    bsiBezier_functionAndDerivative
                                (&fTest, NULL, altitudeArray, numPole, 1, 0.5 * (s0 + 1.0));
                    if (fTest <= 0.0)
                        {
                        f1 = altitudeArray[numPole - 1];
                        jmdlGPFragments_addBezierSegment
                                    (
                                    pFragmentGraph,
                                    &node0Id, &node1Id,
                                    MTG_NULL_NODEID,
                                    fabs (f0) <= tolerance,
                                    fabs (f1) <= tolerance,
                                    poleArray,
                                    numPole
                                    );
                        }
                    }
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPFragments_addClippedLinear
(
GPFragments_hdr   *pFragmentGraph,
        GraphicsPointArray      *pSourceHeader,
        int         i0,
        int         i1,
const   DPoint4d    *pStartPoint,
const   DPoint4d    *pEndPoint,
const   DPoint4d    *pPlane
)
    {
    double h0, h1;
    DPoint4d point0, point1;
    DPoint4d paramPoint;
    GraphicsPoint currPoint;
    MTGNodeId previousNodeId;
    MTGNodeId node0Id, node1Id;

    double tolerance = 1.0e-4;
    double param;
    int i;
    int branchSelector;
    bool    connect;
    int sign0, sign1, startSign, iStart;

    point0 = *pStartPoint;
    jmdlGraphicsPointArray_classifyPointWRTPlane (&sign0, &h0, &point0, pPlane, tolerance);

    iStart = i0;
    startSign = sign0;
    previousNodeId = MTG_NULL_NODEID;
    for (i = i0 + 1;i <= i1; i++)
        {
        jmdlGraphicsPointArray_getGraphicsPoint (pSourceHeader, &currPoint, i);
        point1 = currPoint.point;
        jmdlGraphicsPointArray_classifyPointWRTPlane (&sign1, &h1, &point1, pPlane, tolerance);
        branchSelector = 3 * (sign0  + 1) + (sign1 + 1);
        switch (branchSelector)
            {
            case 0:     /* -1, -1 */
                jmdlGPFragments_addSegment (pFragmentGraph, &node0Id, &node1Id,
                            previousNodeId, &point0, false, &point1, false);
                connect = true;
                break;
            case 1:     /* -1,  0 */
                jmdlGPFragments_addSegment (pFragmentGraph, &node0Id, &node1Id,
                            previousNodeId, &point0, false, &point1, true);
                connect = true;
                break;
            case 2:     /* -1,  1 */
                param = - h0 / (h1 - h0);
                bsiDPoint4d_interpolate (&paramPoint, &point0, param, &point1);

                jmdlGPFragments_addSegment (pFragmentGraph, &node0Id, &node1Id,
                            previousNodeId, &point0, false, &paramPoint, true);
                connect = false;
                break;

            case 3:     /*  0, -1 */
                jmdlGPFragments_addSegment (pFragmentGraph, &node0Id, &node1Id,
                            previousNodeId, &point0, true, &point1, false);
                connect = true;
                break;
            case 4:     /*  0,  0 */
                jmdlGPFragments_addSegment (pFragmentGraph, &node0Id, &node1Id,
                            previousNodeId, &point0, true, &point1, true);
                connect = true;
                break;
            case 5:     /*  0,  1 */
                connect = false;
                break;

            case 6:     /*  1, -1 */
                param = - h0 / (h1 - h0);
                bsiDPoint4d_interpolate (&paramPoint, &point0, param, &point1);

                jmdlGPFragments_addSegment (pFragmentGraph, &node0Id, &node1Id,
                            previousNodeId, &paramPoint, true, &point1, false);
                connect = false;
                break;
            case 7:     /*  1,  0 */
                connect = false;
                break;
            case 8:     /*  1,  1 */
                connect = false;
                break;
            }

        if (connect)
            {
            previousNodeId = node1Id;
            }
        else
            {
            previousNodeId = MTG_NULL_NODEID;
            }
        point0 = point1;
        sign0 = sign1;
        h0 = h1;
        }
    }


/*---------------------------------------------------------------------------------**//**
* Branch into fragment-specific clipper.
* Copy fragments to output array, and record their boundary indices in a fragment array.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPFragments_clipFragment
(
        GraphicsPointArray      *pSourceHeader,
GPFragments_hdr   *pFragmentGraph,
        int         i0,
        int         i1,
const   DPoint4d    *pPoint0,
const   DPoint4d    *pPoint1,
        int         curveType,
const   DPoint4d    *pPlane
)
    {
    if (curveType == 0)
        {
        jmdlGPFragments_addClippedLinear (pFragmentGraph, pSourceHeader, i0, i1, pPoint0, pPoint1, pPlane);
        }
    else if (curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
        {
        jmdlGPFragments_addClippedEllipse (pFragmentGraph, pSourceHeader, i0, i1, pPoint0, pPoint1, pPlane);
        }
    else
        {
        jmdlGPFragments_addClippedBezier (pFragmentGraph, pSourceHeader, i0, i1, pPoint0, pPoint1, pPlane);
        }
    }
/*---------------------------------------------------------------------------------**//**
* Find cut points for a plane wrt fragments in an array.
* Copy fragments to output array, and record their boundary indices in a fragment array.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGPFragments_createPlaneClipFragments
(
GPFragments_hdr   *pFragmentGraph,
int             *piReadPos,
GraphicsPointArray              *pSourceHeader,
const DPoint4d      *pPlane
)
    {
    int i0, i1;
    int     curveType;
    DPoint4d point0, point1;
    int numFragment = 0;

    for (i0 = *piReadPos;
         jmdlGraphicsPointArray_parseFragment (pSourceHeader, &i1, &point0, &point1, &curveType, i0);
         i0 = i1 + 1)
        {
        jmdlGPFragments_clipFragment
                                (
                                pSourceHeader,
                                pFragmentGraph,
                                i0, i1,
                                &point0, &point1,
                                curveType,
                                pPlane
                                );
        numFragment++;
        if (jmdlGraphicsPointArray_getPointMask (pSourceHeader, i1, HPOINT_MASK_MAJOR_BREAK))
            {
            i0 = i1 + 1;
            break;
            }
        }
    *piReadPos = i0;
    return numFragment > 0;
    }

/*---------------------------------------------------------------------------------**//**
* qsort-compatible comparison function
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
static int jmdlGPFragments_compareSortParameters
(
const void *pAddr0,
const void *pAddr1
)
    {

    double param0 = ((NodeSortParam*)pAddr0)->sortParam;
    double param1 = ((NodeSortParam*)pAddr1)->sortParam;

    if (param0 < param1)
        {
        return -1;
        }
    else if (param0 > param1)
        {
        return 1;
        }

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGPFragments_sortAndJoinAlongMaximalAxis
(
GPFragments_hdr   *pFragmentGraph
)
    {
    MTGGraph   *pGraph     = &pFragmentGraph->graph_hdr;
    GraphicsPointArray     *pHPoints   = &pFragmentGraph->gpa_hdr;
    EmbeddedStructArray     *pSortArray = &pFragmentGraph->sortArray_hdr;
    int         index, i;
    int         offset = pFragmentGraph->hpointsIndexOffset;
    int         pointMask;
    MTGNodeId   firstDanglingNodeId = pFragmentGraph->danglingNodeId;
    MTGNodeId  node0Id, node1Id, node2Id, node3Id, node4Id;

    NodeSortParam sortParam;

    int numParam = 0;
    DRange3d    range;

    if (pFragmentGraph->danglingNodeCount == 0)
        {
        }
    else if (pFragmentGraph->danglingNodeCount == 2)
        {
        node0Id = firstDanglingNodeId;
        node1Id = jmdlMTGGraph_getVSucc (pGraph, node0Id);
        node2Id = jmdlMTGGraph_getVSucc (pGraph, node1Id);

        if (node0Id != node2Id || node0Id == node1Id)
            {
            jmdlGPFragments_gripe (pFragmentGraph, "dangling node count mismatch");
            }
        else
            {
            jmdlMTGGraph_vertexTwist (pGraph, node0Id, node1Id);
            jmdlMTGGraph_join  (pGraph,
                            &node3Id, &node4Id,
                            node0Id,  node1Id,
                            FRAGMENT_ARRAY_BRIDGE_EDGE_MASK,
                            FRAGMENT_ARRAY_BRIDGE_EDGE_MASK
                            );
            }
        }
    else
        {
        NodeSortParam *pBase;
        bsiDRange3d_init (&range);

        /* Find the xyz range of all the dangling ends */
        MTGARRAY_VERTEX_LOOP (nodeId, pGraph, firstDanglingNodeId)
            {
            jmdlMTGGraph_getLabel (pGraph, &index, nodeId, offset);
            sortParam.nodeId = nodeId;
            jmdlGraphicsPointArray_getDPoint4dWithMask (pHPoints, &sortParam.point, &pointMask, index);
            bsiDPoint4d_initWithNormalizedWeight (&sortParam.point, &sortParam.point);
            bsiDRange3d_extendByDPoint4d (&range, &sortParam.point);
            omdlVArray_insert (pSortArray, &sortParam, -1);
            numParam++;
            }
        MTGARRAY_END_VERTEX_LOOP (nodeId, pGraph, firstDanglingNodeId)

        /* Find the index of the axis with the largest variation among the danglers */
        index = bsiDRange3d_indexOfMaximalAxis (&range);

        /* Extract the large-variation component for sorting */
        pBase = (NodeSortParam *)omdlVArray_getPtr (pSortArray, 0);
        for (i = 0; i < numParam; i++)
          {
          pBase[i].sortParam = ((double *)(&pBase[i].point))[index];
          }

        omdlVArray_sort (pSortArray, jmdlGPFragments_compareSortParameters);

        /* Join in pairs as per the sort */
        for (i = 0; i + 1 < numParam; i += 2)
            {
            node0Id = pBase[i].nodeId;
            node1Id = pBase[i+1].nodeId;
            jmdlMTGGraph_yankEdgeFromVertex (pGraph, node0Id);
            jmdlMTGGraph_yankEdgeFromVertex (pGraph, node1Id);

            jmdlMTGGraph_join  (pGraph,
                        &node2Id, &node3Id,
                        node0Id,  node1Id,
                        FRAGMENT_ARRAY_BRIDGE_EDGE_MASK,
                        FRAGMENT_ARRAY_BRIDGE_EDGE_MASK
                        );
            }

        }
    }

/*---------------------------------------------------------------------------------**//**
* Access the fragment referenced by a given node of the fragment graph.  Copy the fragment
* to a destination GraphicsPointArray.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP bool     jmdlGPFragments_copyFragment
(
GPFragments_hdr   *pFragmentGraph,
GraphicsPointArray              *pDest,
MTGNodeId           baseNodeId
)
    {
    MTGGraph   *pGraph     = &pFragmentGraph->graph_hdr;
    GraphicsPointArray     *pSource    = &pFragmentGraph->gpa_hdr;
    int         offset = pFragmentGraph->hpointsIndexOffset;
    MTGNodeId  mateNodeId, baseVSuccNodeId, mateVSuccNodeId;
    int         baseIndex, mateIndex;
    MTGMask    baseMask, mateMask;
    bool        myStat = true;

    mateNodeId = jmdlMTGGraph_getEdgeMate (pGraph, baseNodeId);
    jmdlMTGGraph_getLabel (pGraph, &baseIndex, baseNodeId, offset);
    jmdlMTGGraph_getLabel (pGraph, &mateIndex, mateNodeId, offset);
    baseMask = jmdlMTGGraph_getMask (pGraph, baseNodeId, FRAGMENT_ARRAY_BRIDGE_EDGE_MASK);
    mateMask = jmdlMTGGraph_getMask (pGraph, mateNodeId, FRAGMENT_ARRAY_BRIDGE_EDGE_MASK);

    if (baseIndex >= 0 && mateIndex >= 0 && !baseMask && !mateMask)
        {
        jmdlGraphicsPointArray_appendFragment (pDest, pSource, baseIndex, mateIndex, HPOINT_MASK_BREAK);
        }
    else if (baseMask && mateMask)
        {
        baseVSuccNodeId = jmdlMTGGraph_getVSucc (pGraph, baseNodeId);
        mateVSuccNodeId = jmdlMTGGraph_getVSucc (pGraph, mateNodeId);
        jmdlMTGGraph_getLabel (pGraph, &baseIndex, baseVSuccNodeId, offset);
        jmdlMTGGraph_getLabel (pGraph, &mateIndex, mateVSuccNodeId, offset);
        jmdlGraphicsPointArray_appendSegment (pDest, pSource, baseIndex, 0, mateIndex, 0);
        }
    else
        {
        jmdlGPFragments_gripe (pFragmentGraph, "bridge masks, index ids out of sync");
        myStat = false;
        }
    return myStat;
    }

/*---------------------------------------------------------------------------------**//**
* Search a graphics point array for a point that matches x,y,z (unweighted) of test point.
* If none found, add it.  If found, return its label data and copy the final point into its place.
* @param pScratch <=> scratch header for point searches.
* @return MTG_NULL_NODEID if no match found; userData label if found.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP MTGNodeId   jmdlGPFragments_stashOrMatch
(
GraphicsPointArray              *pSearchHeader,
GraphicsPointArray              *pGeometryHeader,
MTGNodeId           currNodeId,
int                 currIndex
)
    {
    int numCandidate = jmdlGraphicsPointArray_getCount (pSearchHeader);
    MTGNodeId matchedId = MTG_NULL_NODEID;
    int i;
    GraphicsPoint   currPoint;
    GraphicsPoint   *pTest;
    double      dist;
    double      tol = 1.0e-6;

    if (   jmdlGraphicsPointArray_getGraphicsPoint (pGeometryHeader, &currPoint, currIndex)
        && bsiDPoint4d_initWithNormalizedWeight (&currPoint.point, &currPoint.point))
        {
        pTest = jmdlGraphicsPointArray_getPtr (pSearchHeader, 0);
        for (i = 0; i < numCandidate; i++, pTest++)
            {
            dist =    fabs (currPoint.point.x - pTest->point.x)
                    + fabs (currPoint.point.y - pTest->point.y)
                    + fabs (currPoint.point.z = pTest->point.z);

            if (dist < tol)
                {
                matchedId = pTest->userData;
                jmdlGraphicsPointArray_pop (pSearchHeader, NULL);
                return matchedId;
                }
            }
        currPoint.userData = currNodeId;
        jmdlGraphicsPointArray_addGraphicsPoint (pSearchHeader, &currPoint);
        }
    return matchedId;
    }
/*---------------------------------------------------------------------------------**//**
* Find and join dangling edges in a fragment graph.
* @param pScratch <=> scratch header for point searches.
* @bsimethod                                                    EarlinLutz      08/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGPFragments_joinDanglers
(
GPFragments_hdr   *pFragmentGraph,
GraphicsPointArray              *pSearchHeader
)
    {
    int currLabel;
    MTGNodeId   matchNodeId;
    MTGGraph   *pGraph     = &pFragmentGraph->graph_hdr;
    int         offset = pFragmentGraph->hpointsIndexOffset;

    jmdlGraphicsPointArray_empty (pSearchHeader);
    MTGARRAY_SET_LOOP (currNodeId, pGraph)
        {
        if (jmdlMTGGraph_getVSucc (pGraph, currNodeId) == currNodeId)
            {
            jmdlMTGGraph_getLabel (pGraph, &currLabel, currNodeId, offset);
            matchNodeId = jmdlGPFragments_stashOrMatch
                        (pSearchHeader, &pFragmentGraph->gpa_hdr, currNodeId, currLabel);
            if (matchNodeId != MTG_NULL_NODEID)
                {
                jmdlMTGGraph_vertexTwist (pGraph, currNodeId, matchNodeId);
                }
            }
        }
    MTGARRAY_END_SET_LOOP (currNodeId, pGraph)

    }

static void jmdlGPFragments_derefNodeId
(
GPFragments_hdr   *pFragmentGraph,
DPoint4d        *pPoint,
int             *pIndex,
int             *pMask,
MTGNodeId       nodeId,
const char      *pMessage
)
    {
    int         offset = pFragmentGraph->hpointsIndexOffset;
    jmdlMTGGraph_getLabel (&pFragmentGraph->graph_hdr, pIndex, nodeId, offset);
    jmdlGraphicsPointArray_getDPoint4dWithMask (&pFragmentGraph->gpa_hdr, pPoint, pMask, *pIndex);
    if (pMessage)
        {
        printf("%s node %d index %d  (%lf,%lf,lf,%lf) mask %d\n",
                        pMessage,
                        nodeId, *pIndex, pPoint->x, pPoint->y, pPoint->z, pPoint->w, *pMask);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      08/98
*
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGPFragments_extractEdgesToGraphicsPointArray
(
GPFragments_hdr   *pFragmentGraph,
GraphicsPointArray              *pDest
)
    {

    MTGGraph   *pGraph     = &pFragmentGraph->graph_hdr;


    MTGNodeId  mateNodeId, currNodeId;
    MTGMask    visitMask;
    int         count;
    static int s_noisy = 1;

    visitMask = jmdlMTGGraph_grabMask (pGraph);
    jmdlMTGGraph_clearMaskInSet (pGraph, visitMask);

    /* Trim off danglers first */
    MTGARRAY_SET_LOOP (seedNodeId, pGraph)
        {
        currNodeId = seedNodeId;
        if (jmdlMTGGraph_getVSucc (pGraph, seedNodeId) == seedNodeId)
            {
            count = 0;
            for (currNodeId = seedNodeId;
                !jmdlMTGGraph_getMask (pGraph, currNodeId, visitMask);
                currNodeId = jmdlMTGGraph_getFSucc (pGraph, currNodeId))
                {
                count++;
                mateNodeId = jmdlMTGGraph_getEdgeMate (pGraph, currNodeId);

                jmdlGPFragments_copyFragment (pFragmentGraph, pDest, currNodeId);
                jmdlMTGGraph_setMask (pGraph, currNodeId, visitMask);
                jmdlMTGGraph_setMask (pGraph, mateNodeId, visitMask);
                }
            jmdlGraphicsPointArray_markBreak (pDest);
            }
        }
    MTGARRAY_END_SET_LOOP (seedNodeId, pGraph)

    /* Extract true loops */
    MTGARRAY_SET_LOOP (seedNodeId, pGraph)
        {
        currNodeId = seedNodeId;
        count = 0;
        for (currNodeId = seedNodeId;
            !jmdlMTGGraph_getMask (pGraph, currNodeId, visitMask);
            currNodeId = jmdlMTGGraph_getFSucc (pGraph, currNodeId))
            {
            count++;
            mateNodeId = jmdlMTGGraph_getEdgeMate (pGraph, currNodeId);

            jmdlGPFragments_copyFragment (pFragmentGraph, pDest, currNodeId);
            jmdlMTGGraph_setMask (pGraph, currNodeId, visitMask);
            jmdlMTGGraph_setMask (pGraph, mateNodeId, visitMask);
            }
        if (count > 0)
            jmdlGraphicsPointArray_markBreak (pDest);
        }
    MTGARRAY_END_SET_LOOP (seedNodeId, pGraph)


    jmdlMTGGraph_dropMask (pGraph, visitMask);
    }


END_BENTLEY_GEOMETRY_NAMESPACE
