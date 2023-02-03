/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#ifdef DEBUG_DISPLAY
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public void debug_displayCurve
(
MSBsplineCurve      *curveP,
int                 displayMode
)
    {
#ifdef TRANSFORM_FOR_DISPLAY
    MSBsplineCurve      tmpCurve;
    MSElementDescr      *edP;

    if (! bspcurv_transformCurve (&tmpCurve, curveP, &gInverseTransform))
        {
        tmpCurve.display.curveDisplay = true;
        if (! mdlBspline_createCurve (&edP, NULL, &tmpCurve))
            {
            mdlElmdscr_display (edP, MASTERFILE, displayMode);
            if (storeDebugGeometry) mdlElmdscr_add (edP);
            mdlElmdscr_freeAll (&edP);
            }
        mdlBspline_freeCurve (&tmpCurve);
        }
#else
    MSElementDescr      *edP;

    if (! mdlBspline_createCurve (&edP, NULL, curveP)
        {
        curveP->display.curveDisplay = true;
        curveP->display.polygonDisplay = true;
        mdlElmdscr_display (edP, MASTERFILE, displayMode);
        mdlElmdscr_freeAll (&edP);
        }
#endif
    }
#endif

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
// Intersect xy parts of an ellipse and a line segment.   Announce
// the intersection (or proximity) to an intersection list.
/*---------------------------------------------------------------------------------**//**
* @param   pDistance <= 2 x 2 array of distances between corresponding points
* @param   pClose    <= 2 x 2 array of tolerance comparison results
* @param   pPointA   => array of two points
* @param   pPointB   => array of two points
* @param    tol2     => squared tolerance
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int jmdlRIMSBS_computeSquaredXYDistancesExt
(
double      pDistance[2][2],
bool        pClose[2][2],
DPoint3d    *pPointA,
bool        bMoveAIfClose,
DPoint3d    *pPointB,
bool        bMoveBIfClose,
double      tol2
)
    {
    int i, j;
    double dx, dy, d2;
    int     numClose = 0;

    for (i = 0; i < 2; i++ )
        {
        for (j = 0; j < 2; j++)
            {
            dx = pPointB[j].x - pPointA[i].x;
            dy = pPointB[j].y - pPointA[i].y;
            d2 = pDistance[i][j] = dx * dx + dy * dy;
            if (d2 <= tol2)
                {
                pClose[i][j] = true;
                if (bMoveAIfClose)
                    pPointA[i] = pPointB[j];
                if (bMoveBIfClose )
                    pPointB[j] = pPointA[i];
                numClose++;
                }
            else
                {
                pClose[i][j] = false;
                }
            }
        }
    return numClose;
    }

/*---------------------------------------------------------------------------------**//**
* @param   pDistance <= 2 x 2 array of distances between corresponding points
* @param   pClose    <= 2 x 2 array of tolerance comparison results
* @param   pPointA   => array of two points
* @param   pPointB   => array of two points
* @param    tol2     => squared tolerance
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static int jmdlRIMSBS_computeSquaredXYDistances
(
double      pDistance[2][2],
bool        pClose[2][2],
DPoint3d    *pPointA,
DPoint3d    *pPointB,
double      tol2
)
    {
    return jmdlRIMSBS_computeSquaredXYDistancesExt (pDistance, pClose, pPointA, false, pPointB, false, tol2);
    }
/*---------------------------------------------------------------------------------**//**
* declare vertex-vertex coincidences from prior classifications
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlRIMSBS_declareEndPointIntersections
(
RG_Header           *pRG,                   /* => region context */
RG_IntersectionList *pRGIL,                 /* <=> list of intersection parameters */
bool                isClose[2][2],
MTGNodeId           nodeA,
MTGNodeId           nodeB
)
    {
    int i, j;
    for (i = 0; i < 2; i++)
        {
        for (j = 0; j < 2; j++)
            {
            if (isClose[i][j])
                jmdlRGIL_declareVertexOnVertex (pRG, pRGIL,
                            nodeA, i,
                            nodeB, j
                            );

            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* declare vertex-vertex coincidences for a curve's own start and end.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlRIMSBS_declareSelfIntersectionsAtEndPoints
(
RG_Header           *pRG,                   /* => region context */
RG_IntersectionList *pRGIL,                 /* <=> list of intersection parameters */
DPoint3d            *pStartEnd,
MTGNodeId           nodeA,
double              tol2
)
    {
    double d2 = pStartEnd->DistanceSquaredXY (pStartEnd[ 1]);

    if (d2 < tol2)
        jmdlRGIL_declareVertexOnVertex (pRG, pRGIL, nodeA, 0, nodeA, 1);
    }

/*---------------------------------------------------------------------------------**//**
* declare vertexSegment and SegmentSegment intersections from prior classifications
* @pIntersection => intersection coordinates.
* @param nodeA => base node on edge A
* @param paramA => intersection parameter on edge A
* @param pEndA => array of the two endpoint coordinates for edge A.
* @param nodeB => base node on edge B
* @param paramB => intersection parameter on edge B
* @param pEndB => array of the two endpoint coordinates for edge B.
* @param
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void        jmdlRIMSBS_declareInteriorIntersections
(
RG_Header           *pRG,
RG_IntersectionList *pRGIL,
DPoint3d            *pIntersection,
MTGNodeId           nodeA,
double              paramA,
DPoint3d            *pEndA,
MTGNodeId           nodeB,
double              paramB,
DPoint3d            *pEndB,
double              tol2
)
    {
    bool    withinA, withinB;
    int     endIndexA, endIndexB;
    double  d2A, d2B;

    withinA       = paramA > 0.0 && paramA < 1.0;

    withinB = paramB > 0.0 && paramB < 1.0;

    endIndexA   = paramA < 0.5 ? 0 : 1;
    endIndexB   = paramB < 0.5 ? 0 : 1;
    /* NO!!! Don't try to do a quick out for parameter values outside
        0..1.  A parameter value just outside the range could be an
        end-on-interior case (which will get caught by the tolerance checks).
    */

    d2A = pIntersection->DistanceSquaredXY (pEndA[endIndexA]);
    d2B = pIntersection->DistanceSquaredXY (pEndB[endIndexB]);

    if (   d2A > tol2
        && d2B > tol2
        && withinA
        && withinB
        )
        {
        jmdlRGIL_declareSimpleIntersection
                            (
                            pRG, pRGIL,
                            nodeA,
                            paramA,
                            nodeB,
                            paramB
                            );
        }
    else if (  d2A < tol2
            && d2B > tol2
            && withinB
            )
        {
        jmdlRGIL_declareVertexOnSegment
                            (
                            pRG, pRGIL,
                            nodeA,
                            endIndexA,
                            nodeB,
                            paramB
                            );
        }
    else if (  d2B < tol2
            && d2A > tol2
            && withinA
            )
        {
        jmdlRGIL_declareVertexOnSegment
                            (
                            pRG, pRGIL,
                            nodeB,
                            endIndexB,
                            nodeA,
                            paramA
                            );
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public bool    jmdlRIMSBS_segmentEllipseIntersection
(
RIMSBS_Context      *pContext,              /* => application context */
RG_Header           *pRG,                   /* => region context */
RG_IntersectionList *pRGIL,                 /* <=> list of intersection parameters */
RG_EdgeData         *pEdgeData0,            /* => first edge data.  Assumed linear */
RG_EdgeData         *pEdgeData1,            /* => second edge data. */
DEllipse3d          *pEllipse               /* => ellipse geometry from second curve. */
)
    {
    double      lineParam[2];
    double      ellipseAngle[2];
    double      ellipseParam[2];
    DPoint3d    ellipseCoff[2];
    DPoint3d    intersectionPoint[2];
    DPoint3d    linePoint[2];
    DPoint3d    ellipsePoint[2];
    double      endDist2[2][2];
    bool        isClose[2][2];
    int         i;
    double      tol = jmdlRG_getTolerance (pRG);
    double      tol2 = tol * tol;

    int lineNode, ellipseNode;
    int numIntersection;

    jmdlRGEdge_getXYZ (pEdgeData0, &linePoint[0], 0);
    jmdlRGEdge_getXYZ (pEdgeData0, &linePoint[1], 1);
    lineNode = jmdlRGEdge_getNodeId (pEdgeData0, 0);
    ellipseNode = jmdlRGEdge_getNodeId (pEdgeData1, 0);

    pEllipse->EvaluateEndPoints (ellipsePoint[0], ellipsePoint[1]);

    jmdlRIMSBS_computeSquaredXYDistancesExt (endDist2, isClose, linePoint, true, ellipsePoint, false, tol2);

    jmdlRIMSBS_declareEndPointIntersections
                        (
                        pRG, pRGIL,
                        isClose,
                        lineNode,
                        ellipseNode
                        );

    numIntersection = pEllipse->IntersectXYLine (
                            intersectionPoint,
                            lineParam,
                            ellipseCoff,
                            ellipseAngle,
                            linePoint[0],
                            linePoint[1]
                            );

    for (i = 0; i < numIntersection; i++)
        {
        ellipseParam[i] = bsiTrig_normalizeAngleToSweep (ellipseAngle[i],
                            pEllipse->start, pEllipse->sweep);
        /* interior test with no tolerance.   If near end, it should show up
            in raw distance tests from endpoint */
        jmdlRIMSBS_declareInteriorIntersections
                        (
                        pRG, pRGIL,
                        &intersectionPoint[i],
                        lineNode,
                        lineParam[i],
                        linePoint,
                        ellipseNode,
                        ellipseParam[i],
                        ellipsePoint,
                        tol2
                        );
        }

    return  true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public bool    jmdlRIMSBS_ellipseEllipseIntersection
(
RIMSBS_Context      *pContext,              /* => application context */
RG_Header           *pRG,                   /* => region context */
RG_IntersectionList *pRGIL,                 /* <=> list of intersection parameters */
RG_EdgeData         *pEdgeData0,            /* => first edge data.  Assumed linear */
DEllipse3d          *pEllipse0,             /* => ellipse geometry from first curve. */
RG_EdgeData         *pEdgeData1,            /* => second edge data. */
DEllipse3d          *pEllipse1              /* => ellipse geometry from second curve. */
)
    {
    DPoint3d    intersectionPoint[4];

    double      ellipse0Angle[4];
    DPoint3d    ellipse0Coffs[4];
    DPoint3d    ellipse0End[2];
    MTGNodeId  ellipse0NodeId;

    double      ellipse1Angle[4];
    DPoint3d    ellipse1Coffs[4];
    DPoint3d    ellipse1End[2];

    MTGNodeId  ellipse1NodeId;

    bool        isClose[2][2];
    double      endDist2[2][2];

    int numIntersection;
    int i;
    double      tol = jmdlRG_getTolerance (pRG);
    double      tol2 = tol * tol;
    double      f0, f1;

    jmdlRGEdge_getXYZ (pEdgeData0, &ellipse0End[0], 0);
    jmdlRGEdge_getXYZ (pEdgeData0, &ellipse0End[1], 1);

    jmdlRGEdge_getXYZ (pEdgeData1, &ellipse1End[0], 0);
    jmdlRGEdge_getXYZ (pEdgeData1, &ellipse1End[1], 1);

    ellipse0NodeId = jmdlRGEdge_getNodeId (pEdgeData0, 0);
    ellipse1NodeId = jmdlRGEdge_getNodeId (pEdgeData1, 0);

    jmdlRIMSBS_computeSquaredXYDistances (endDist2, isClose, ellipse0End, ellipse1End, tol2);

    jmdlRIMSBS_declareEndPointIntersections
                        (
                        pRG, pRGIL,
                        isClose,
                        ellipse0NodeId,
                        ellipse1NodeId
                        );


    numIntersection = pEllipse0->IntersectXYDEllipse3dBounded (
                            intersectionPoint,
                            ellipse0Coffs,
                            ellipse0Angle,
                            ellipse1Coffs,
                            ellipse1Angle,
                            *pEllipse1
                            );

    for (i = 0; i < numIntersection; i++)
        {
        f0 = pEllipse0->AngleToFraction (ellipse0Angle[i]);
        f1 = pEllipse1->AngleToFraction (ellipse1Angle[i]);

        jmdlRIMSBS_declareInteriorIntersections
                            (
                            pRG, pRGIL,
                            &intersectionPoint[i],
                            ellipse0NodeId,
                            f0,
                            ellipse0End,
                            ellipse1NodeId,
                            f1,
                            ellipse1End,
                            tol2
                            );
        }
    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public bool    jmdlRIMSBS_segmentMSBSplineCurveIntersection
(
RIMSBS_Context      *pContext,              /* => application context */
RG_Header           *pRG,                   /* => region context */
RG_IntersectionList *pRGIL,                 /* <=> list of intersection parameters */
RG_EdgeData         *pEdgeData0,            /* => first edge data.  Assumed linear */
RG_EdgeData         *pEdgeData1,            /* => second edge data. */
MSBsplineCurve      *pCurve                 /* => curve data from second edge */
)
    {
    MTGNodeId     lineNodeId, curveNodeId;

    DPoint3d        linePoint[2], curvePoint[2];

    double          endDist2[2][2];
    bool            isClose[2][2];
    int             i;
    double          tol = jmdlRG_getTolerance (pRG);
    double          tol2 = tol * tol;

    int             numIntersection;
    double          *pSegmentParam;
    double          *pCurveParam;
    DPoint3d        intersectionPoint;
    RotMatrix z0Matrix;

    /* Sputter and gag.... no way to force z=0 in standard rmatrix package */
    z0Matrix.InitFromScaleFactors (1.0, 1.0, 0.0);

    jmdlRGEdge_getXYZ (pEdgeData0, &linePoint[0], 0);
    jmdlRGEdge_getXYZ (pEdgeData0, &linePoint[1], 1);

    jmdlRGEdge_getXYZ (pEdgeData1, &curvePoint[0], 0);
    jmdlRGEdge_getXYZ (pEdgeData1, &curvePoint[1], 1);

    lineNodeId = jmdlRGEdge_getNodeId (pEdgeData0, 0);
    curveNodeId = jmdlRGEdge_getNodeId (pEdgeData1, 0);

    jmdlRIMSBS_computeSquaredXYDistances (endDist2, isClose, linePoint, curvePoint, tol2);

    jmdlRIMSBS_declareEndPointIntersections
                        (
                        pRG, pRGIL,
                        isClose,
                        lineNodeId,
                        curveNodeId
                        );

    bspcurv_segmentCurveIntersection (
                    &numIntersection, &pSegmentParam, &pCurveParam,
                    &linePoint[0], &linePoint[1],
                    pCurve,
                    &z0Matrix, tol);

    if (numIntersection > 0)
        {
        for (i = 0; i < numIntersection; i++)
            {
            /* interior test with no tolerance.   If near end, it should show up
                in raw distance tests from endpoint */
            intersectionPoint.Interpolate (linePoint[0], pSegmentParam[i], linePoint[1]);
            jmdlRIMSBS_declareInteriorIntersections
                            (
                            pRG, pRGIL,
                            &intersectionPoint,
                            lineNodeId,
                            pSegmentParam[i],
                            linePoint,
                            curveNodeId,
                            pCurveParam[i],
                            curvePoint,
                            tol2
                            );
            }

        bsputil_free (pSegmentParam);
        bsputil_free (pCurveParam);
        }

    return true;

    }
//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
// Apply Newton-Raphson step to improve a tentative intersection
// in the xy coordinates of two curves.
static int bspcci_improveIntersections
(
DPoint3d    *pIntersectionPoint,
double      *pParam0,
double      *pParam1,
int         *pNumIntersection,
double      *pMaxStep0,
double      *pMaxStep1,
MSBsplineCurve  *pCurve0,
MSBsplineCurve  *pCurve1,
double          geomTol
)
    {
    int i;
    DPoint3d dX0[2], dX1[2];
    double   w0[2],  w1[2];
    int numDeriv = 1;
    double f0, f1;
    double d00, d01, d10, d11;
    double s0, s1, denom;
    double ds0, ds1;
    double s_smallGrossStep = 0.01;
    double divTest;
    StatusInt status = SUCCESS;
    bool    converged;
    int iter;
    static int s_maxIter = 3;
    static double s_parameterRelTol = 1.0e-8;
    static double stepSign = 1.0;
    static double s_functionRelTol = 1.0e-8;

    *pMaxStep0 = 0.0;
    *pMaxStep1 = 0.0;

    for (i = 0; i < *pNumIntersection; i++)
        {
        s0 = pParam0[i];
        s1 = pParam1[i];
        converged = false;
        for (iter = 0; iter < s_maxIter && !converged; iter++)
            {
            if (   SUCCESS == bspcurv_computeDerivatives (dX0, w0, pCurve0, numDeriv, s0, false)
                && SUCCESS == bspcurv_computeDerivatives (dX1, w1, pCurve1, numDeriv, s1, false)
               )
                {
                f0 = dX0[0].x * w1[0]  - dX1[0].x * w0[0];
                d00 = dX0[1].x * w1[0] - dX1[0].x * w0[1];
                d01 = dX0[0].x * w1[1] - dX1[1].x * w0[0];

                f1 = dX0[0].y * w1[0]  - dX1[0].y * w0[0];
                d10 = dX0[1].y * w1[0] - dX1[0].y * w0[1];
                d11 = dX0[0].y * w1[1] - dX1[1].y * w0[0];

                denom = d00 * d11 - d01 * d10;
                ds0 = (  d11 * f0 - d01 * f1);
                ds1 = (- d10 * f0 + d00 * f1);
                divTest = s_smallGrossStep * fabs (denom);
                if (   fabs (ds0) < divTest
                    && fabs (ds1) < divTest)
                    {
                    ds0 /= denom;
                    ds1 /= denom;

                    if (fabs (ds0) > *pMaxStep0)
                        *pMaxStep0 = fabs (ds0);

                    if (fabs (ds1) > *pMaxStep1)
                        *pMaxStep1 = fabs (ds1);

                    s0 -= stepSign * ds0;
                    s1 -= stepSign * ds1;
                    converged =    (   fabs (ds0) < s_parameterRelTol
                                    && fabs (ds1) < s_parameterRelTol)
                                || (   fabs (f0) < s_functionRelTol * fabs (dX0[0].x)
                                    && fabs (f1) < s_functionRelTol * fabs (dX0[0].y));
                    }
                else
                    {
                    status = ERROR;
                    break;
                    }
                }
            }

        if (converged)
            {
            pParam0[i] = s0;
            pParam1[i] = s1;
            }

        }
    return status;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
// Intersect xy parts of two curves.   Announce
// the intersection (or proximity) to an intersection list.
Public bool    jmdlRIMSBS_MSBsplineCurveMSBsplineCurveIntersection
(
RIMSBS_Context      *pContext,              /* => application context */
RG_Header           *pRG,                   /* => region context */
RG_IntersectionList *pRGIL,                 /* <=> list of intersection parameters */
RG_EdgeData         *pEdgeData0,            /* => first edge data.  Assumed linear */
MSBsplineCurve      *pCurve0,               /* => curve data from first edge */
RG_EdgeData         *pEdgeData1,            /* => second edge data. */
MSBsplineCurve      *pCurve1                /* => curve data from second edge */
)
    {
    DPoint3d    curve0End[2], curve1End[2];
    MTGNodeId  curve0NodeId;
    MTGNodeId  curve1NodeId;

    bool        isClose[2][2];
    double      endDist2[2][2];
    DPoint3d    *pIntersectionPoint;
    double      *pParam0, *pParam1;

    int numIntersection;
    int i;
    double      tol = jmdlRG_getTolerance (pRG);
    double      intersectionTol = tol;
    double      tol2 = tol * tol;
    double      f0, f1;
    double      delta0, delta1;
    static int s_improveIntersections = 1;

    RotMatrix z0Matrix;

    /* Sputter and gag.... no way to force z=0 in standard rmatrix package */
    z0Matrix.InitFromScaleFactors (1.0, 1.0, 0.0);

    jmdlRGEdge_getXYZ (pEdgeData0, &curve0End[0], 0);
    jmdlRGEdge_getXYZ (pEdgeData0, &curve0End[1], 1);

    jmdlRGEdge_getXYZ (pEdgeData1, &curve1End[0], 0);
    jmdlRGEdge_getXYZ (pEdgeData1, &curve1End[1], 1);

    curve0NodeId = jmdlRGEdge_getNodeId (pEdgeData0, 0);
    curve1NodeId = jmdlRGEdge_getNodeId (pEdgeData1, 0);

    jmdlRIMSBS_computeSquaredXYDistances (endDist2, isClose, curve0End, curve1End, tol2);

    jmdlRIMSBS_declareEndPointIntersections
                        (
                        pRG, pRGIL,
                        isClose,
                        curve0NodeId,
                        curve1NodeId
                        );

    bspcci_allIntersectsBtwCurves
                    (
                    &pIntersectionPoint, &pParam0, &pParam1, &numIntersection,
                    pCurve0, pCurve1, &intersectionTol, &z0Matrix, false);

    if (s_improveIntersections)
        bspcci_improveIntersections
                    (
                    pIntersectionPoint, pParam0, pParam1, &numIntersection,
                    &delta0, &delta1,
                    pCurve0, pCurve1, tol
                    );

    if (numIntersection > 0)
        {
        for (i = 0; i < numIntersection; i++)
            {
            f0 = pParam0[i];
            f1 = pParam1[i];

            jmdlRIMSBS_declareInteriorIntersections
                                (
                                pRG, pRGIL,
                                &pIntersectionPoint[i],
                                curve0NodeId,
                                f0,
                                curve0End,
                                curve1NodeId,
                                f1,
                                curve1End,
                                tol2
                                );
            }

        bsputil_free (pParam0);
        bsputil_free (pParam1);
        bsputil_free (pIntersectionPoint);
        }
    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public bool    jmdlRIMSBS_MSBsplineCurveDEllipse3dIntersection
(
RIMSBS_Context      *pContext,              /* => application context */
RG_Header           *pRG,                   /* => region context */
RG_IntersectionList *pRGIL,                 /* <=> list of intersection parameters */
RG_EdgeData         *pEdgeData0,            /* => first edge data.  Assumed linear */
MSBsplineCurve      *pCurve0,               /* => curve data from first edge */
RG_EdgeData         *pEdgeData1,            /* => second edge data. */
DEllipse3d          *pEllipse1,             /* => ellipse */
MSBsplineCurve      *pCurve1                /* => optional curve rep of ellipse. */

)
    {
    DPoint3d    curve0End[2], curve1End[2];
    MTGNodeId  curve0NodeId;
    MTGNodeId  curve1NodeId;

    bool        isClose[2][2];
    double      endDist2[2][2];
    DPoint3d    *pIntersectionPoint;
    DPoint3d    ellipseCoffs;
    double      *pParam0, *pParam1, theta1;
    Transform ellipseFrame, inverseFrame;

    int numIntersection;
    int i;
    double      tol = jmdlRG_getTolerance (pRG);
    double      tol2 = tol * tol;
    double      f0, f1;
    MSBsplineCurve curve1;
    RotMatrix z0Matrix;

    /* Sputter and gag.... no way to force z=0 in standard rmatrix package */
    z0Matrix.InitFromScaleFactors (1.0, 1.0, 0.0);

    jmdlRGEdge_getXYZ (pEdgeData0, &curve0End[0], 0);
    jmdlRGEdge_getXYZ (pEdgeData0, &curve0End[1], 1);

    jmdlRGEdge_getXYZ (pEdgeData1, &curve1End[0], 0);
    jmdlRGEdge_getXYZ (pEdgeData1, &curve1End[1], 1);

    curve0NodeId = jmdlRGEdge_getNodeId (pEdgeData0, 0);
    curve1NodeId = jmdlRGEdge_getNodeId (pEdgeData1, 0);


    jmdlRIMSBS_computeSquaredXYDistances (endDist2, isClose, curve0End, curve1End, tol2);

    jmdlRIMSBS_declareEndPointIntersections
                        (
                        pRG, pRGIL,
                        isClose,
                        curve0NodeId,
                        curve1NodeId
                        );

    if (!pCurve1)
        {
        pCurve1 = &curve1;
        bspconv_convertDEllipse3dToCurve (&curve1, pEllipse1);
        }

#define CHECK_ELLIPSEnot 1
#ifdef CHECK_ELLIPSE
                {
                DPoint3d ellipsePoint, curvePoint, ellipseTangent, curveTangent;
                DPoint3d    dX;

                double s, theta, dot, cross;
                for (s = 0.0; s <= 1.000001; s += 0.5)
                    {
                    theta = pEllipse1->FractionToAngle, s);
                    pEllipse1->EvaluateDerivatives
                            (&ellipsePoint, &ellipseTangent, NULL, theta);
                    bspcurv_evaluateCurvePoint (
                                    &curvePoint, &curveTangent, pCurve1, s);
                    dX.DifferenceOf (ellipsePoint, curvePoint);
                    dot = ellipseTangent.DotProductXY (curveTangent);
                    cross = ellipseTangent.CrossProductXY (curveTangent);
                    printf(" s = %lf theta = %lf dx = %lf, %lf, %lf   (%lf, %lf)\n",
                                        s, theta,
                                        dX.x, dX.y, dX.z,
                                        Angle::Atan2 (ellipseTangent.y, ellipseTangent.x),
                                        Angle::Atan2 (curveTangent.y, curveTangent.x)
                                        );
                    }
                }
#endif



    pEllipse1->GetXYLocalFrame (ellipseFrame, inverseFrame);

    bspcci_allIntersectsBtwCurves
                    (
                    &pIntersectionPoint, &pParam0, &pParam1, &numIntersection,
                    pCurve0, pCurve1, &tol, &z0Matrix, false);

    if (numIntersection > 0)
        {
        for (i = 0; i < numIntersection; i++)
            {
            f0 = pParam0[i];

            inverseFrame.Multiply (&ellipseCoffs, &pIntersectionPoint[i], 1);
            theta1 = Angle::Atan2 (ellipseCoffs.y, ellipseCoffs.x);
            f1 = pEllipse1->AngleToFraction (theta1);

            jmdlRIMSBS_declareInteriorIntersections
                                (
                                pRG, pRGIL,
                                &pIntersectionPoint[i],
                                curve0NodeId,
                                f0,
                                curve0End,
                                curve1NodeId,
                                f1,
                                curve1End,
                                tol2
                                );
            }

        bsputil_free (pParam0);
        bsputil_free (pParam1);
        bsputil_free (pIntersectionPoint);
        }

    if (pCurve1 == &curve1)
        bspcurv_freeCurve (&curve1);

    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public bool    jmdlRIMSBS_checkClosedEdge
(
RIMSBS_Context      *pContext,              /* => application context */
RG_Header           *pRG,                   /* => region context */
RG_IntersectionList *pRGIL,                 /* <=> list of intersection parameters */
RG_EdgeData         *pEdgeData0             /* => Edge data. */
)
    {
    DPoint3d    curve0End[2];
    MTGNodeId  curve0NodeId;

    double      tol = jmdlRG_getTolerance (pRG);
    double      tol2 = tol * tol;

    jmdlRGEdge_getXYZ (pEdgeData0, &curve0End[0], 0);
    jmdlRGEdge_getXYZ (pEdgeData0, &curve0End[1], 1);

    curve0NodeId = jmdlRGEdge_getNodeId (pEdgeData0, 0);

    jmdlRIMSBS_declareSelfIntersectionsAtEndPoints
                        (
                        pRG, pRGIL,
                        curve0End,
                        curve0NodeId,
                        tol2
                        );

    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Public bool    jmdlRIMSBS_MSBsplineCurveSelfIntersection
(
RIMSBS_Context      *pContext,              /* => application context */
RG_Header           *pRG,                   /* => region context */
RG_IntersectionList *pRGIL,                 /* <=> list of intersection parameters */
RG_EdgeData         *pEdgeData0,            /* => first edge data.  Assumed spline */
MSBsplineCurve      *pCurve0                /* => curve data from first edge */
)
    {
    DPoint3d    curve0End[2];
    MTGNodeId  curve0NodeId;

    DPoint3d    point;
    double      *pParam0;

    int numIntersection;
    int i;
    double      tol = jmdlRG_getTolerance (pRG);
    double      tol2 = tol * tol;
    double      f0, f1;

    jmdlRGEdge_getXYZ (pEdgeData0, &curve0End[0], 0);
    jmdlRGEdge_getXYZ (pEdgeData0, &curve0End[1], 1);

    curve0NodeId = jmdlRGEdge_getNodeId (pEdgeData0, 0);

    jmdlRIMSBS_declareSelfIntersectionsAtEndPoints
                        (
                        pRG, pRGIL,
                        curve0End,
                        curve0NodeId,
                        tol2
                        );

    bspcci_selfIntersections
                    (
                    &pParam0, &numIntersection,
                    pCurve0, &tol, NULL, false);

    if (numIntersection > 0)
        {
        for (i = 0; i < numIntersection; i+=2)
            {
            f0 = pParam0[i];
            f1 = pParam0[i+1];
            bspcurv_evaluateCurvePoint (&point, NULL, pCurve0, f0);

            jmdlRIMSBS_declareInteriorIntersections
                                (
                                pRG, pRGIL,
                                &point,
                                curve0NodeId,
                                f0,
                                curve0End,
                                curve0NodeId,
                                f1,
                                curve0End,
                                tol2
                                );
            }

        bsputil_free (pParam0);
        }
    return true;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
