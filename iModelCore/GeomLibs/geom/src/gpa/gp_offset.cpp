/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/gpa/gp_offset.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/*----------------------------------------------------------------------+
|                                                                       |
|   Local defines                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
#define CURVETYPE_NULL (-1)
#define CURVETYPE_LINE (0)
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
#define MAX_CONTEXT_POINT 5
#define HIGHEST_CONTEXT_POINT_INDEX 4

typedef struct
    {
    GraphicsPoint graphics;
    DPoint3d world;
    DPoint3d local;
    DPoint2d xy;
    DPoint2d U;         /* Unit tangent to outbound edge. */
    DPoint2d V;         /* Unit normal to output edge. */
    DPoint2d W;         /* Vector to connection point, from prior joint analysis */

    } GPAOCPoint;

typedef struct
    {
    GPAOCPoint    point[MAX_CONTEXT_POINT];
    int           numPoint;
    double        arcAngle;
    double        chamferAngle;
    double        signedOffset;
    double        absoluteOffset;
    DVec3d        normal;
    Transform  frame;
    Transform  inverseFrame;
    double        minDistance;
    GraphicsPointArrayCP pSource;
    GraphicsPointArray *pDest;
    } GPAOffsetContext;


/*---------------------------------------------------------------------------------**//**
* Stash control parameters for offset calculations.
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPAOC_init
(
        GPAOffsetContext            *pContext,
        GraphicsPointArray      *pDest,
GraphicsPointArrayCP pSource,
        double                      arcAngle,
        double                      chamferAngle,
        double                      offsetDistance,
        DVec3dCP                    pNormal
)
    {
    static double s_minDistanceFraction = 1.0e-4;
    pContext->pSource           = pSource;
    pContext->pDest             = pDest;
    pContext->numPoint          = 0;
    pContext->arcAngle          = arcAngle;
    pContext->chamferAngle      = chamferAngle;
    pContext->signedOffset      = offsetDistance;
    pContext->absoluteOffset    = fabs (offsetDistance);
    pContext->minDistance       = s_minDistanceFraction * offsetDistance;

    if (pContext->chamferAngle < 0.0)
        pContext->chamferAngle = 0.99 * msGeomConst_pi;

    if (pContext->arcAngle < 0.0)
        pContext->arcAngle = 10.0 * msGeomConst_pi;

    if (pNormal)
        {
        pContext->normal = *pNormal;
        }
    else
        {
        pContext->normal.Init ( 0.0, 0.0, 1.0);
        }

    DVec3d xColumn, yColumn, zColumn;
    if (!pContext->normal.GetNormalizedTriad (xColumn, yColumn, zColumn))
        {
        pContext->frame.InitIdentity ();
        }
   else
        {
        pContext->frame.InitFromOriginAndVectors (DPoint3d::FromZero (), xColumn, yColumn, zColumn);
        }
    }

/*---------------------------------------------------------------------------------**//**
* Start computations for a polyline offset.
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPAOC_polylineStart
(
        GPAOffsetContext            *pContext
)
    {
    pContext->numPoint = 0;
    }

/*---------------------------------------------------------------------------------**//**
* Drop the 0'th point, decrement count.
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPAOC_polylineDrop0
(
        GPAOffsetContext            *pContext
)
    {
    int i;
    if (pContext->numPoint > 0)
        {
        pContext->numPoint--;
        for (i = 0; i < pContext->numPoint; i++)
            {
            pContext->point[i] = pContext->point[i+1];
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Force the given point into the array at index.
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPAOC_polylineInsert
(
        GPAOffsetContext        *pContext,
const   GraphicsPoint           *pGPoint,
const   DPoint3d                *pXYZ,
        int                     index
)
    {
    GPAOCPoint *pPoint;
    if (index >= 0 && index < MAX_CONTEXT_POINT)
        {
        pPoint = &pContext->point[index];
        pPoint->graphics = *pGPoint;
        pPoint->world    = *pXYZ;
        pContext->inverseFrame.Multiply (&pPoint->local, pXYZ, 1);
        pPoint->xy.x = pPoint->local.x;
        pPoint->xy.y = pPoint->local.y;
        pPoint->U.x = pPoint->V.y = 1.0;
        pPoint->U.y = pPoint->V.x = 0.0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Set up tangent, normal at first point.
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPAOC_polylineAdvanceToPoint2
(
        GPAOffsetContext        *pContext
)
    {
    GPAOCPoint *p0, *p1;
    double distance;
    p0 = &pContext->point[0];
    p1 = &pContext->point[1];
    distance = p0->U.NormalizedDifferenceOf(p1->xy, p0->xy);

    if (distance < pContext->minDistance)
        {
        *p0 = *p1;
        pContext->numPoint = 1;
        }
    else
        {
        p0->V.Rotate90 (p0->U);
        p0->W.Scale (p0->V, pContext->signedOffset);
        }
    }

/*---------------------------------------------------------------------------------**//**
* Compute the world point at a specified local offset from a vertex.
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPAOC_getOffsetPoint
(
        GPAOffsetContext        *pContext,
        DPoint3d                *pWorldPoint,
const   GPAOCPoint              *pVert,
const   DPoint2d                *pOffset
)
    {
    DVec3d worldOffset;
    pContext->frame.MultiplyMatrixOnly (worldOffset,
                        pOffset->x, pOffset->y, 0.0);
    pWorldPoint->SumOf (pVert->world, worldOffset);
    }

/*---------------------------------------------------------------------------------**//**
* Compute the world vector corresponding to a local vector
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPAOC_getVector
(
        GPAOffsetContext        *pContext,
        DVec3d                  *pWorldVector,
const   DPoint2d                *pLocalVector
)
    {
    pContext->frame.MultiplyMatrixOnly (*pWorldVector,
                        pLocalVector->x, pLocalVector->y, 0.0);
    }

/*---------------------------------------------------------------------------------**//**
* Output a segment defined by two base points with local offset vectors.
* @param p0 = point0
* @param p
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPAOC_outputOffsetSegment
(
        GPAOffsetContext        *pContext,
const   GPAOCPoint              *pVert0,
const   DPoint2d                *pOffset0,
const   GPAOCPoint              *pVert1,
const   DPoint2d                *pOffset1
)
    {
    DPoint3d world0, world1;

    jmdlGPAOC_getOffsetPoint (pContext, &world0, pVert0, pOffset0);
    jmdlGPAOC_getOffsetPoint (pContext, &world1, pVert1, pOffset1);
    jmdlGraphicsPointArray_addDPoint3d (pContext->pDest, &world0);
    jmdlGraphicsPointArray_addDPoint3d (pContext->pDest, &world1);
    }

/*---------------------------------------------------------------------------------**//**
* Output an arc define by center,
* @param p0 = point0
* @param p
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPAOC_outputArc
(
        GPAOffsetContext        *pContext,
const   GPAOCPoint              *pVert,
const   DPoint2d                *pLocalVector0,
        double                  sweep
)
    {
    DPoint3d worldCenter;
    DVec3d worldVector0, worldVector90;
    DPoint2d localVector90;
    DEllipse3d ellipse;

    localVector90.Rotate90 (*pLocalVector0);

    worldCenter = pVert->world;
    jmdlGPAOC_getVector (pContext, &worldVector0,  pLocalVector0 );
    jmdlGPAOC_getVector (pContext, &worldVector90, &localVector90);

    bsiDEllipse3d_initFrom3dVectors
                        (
                        &ellipse,
                        &worldCenter,
                        &worldVector0,
                        &worldVector90,
                        0.0,
                        sweep
                        );
    jmdlGraphicsPointArray_addDEllipse3d (pContext->pDest, &ellipse);
    }

/*---------------------------------------------------------------------------------**//**
* Output from first point to current point.
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPAOC_polylineAdvanceToPoint3
(
        GPAOffsetContext        *pContext
)
    {
    GPAOCPoint *p0, *p1, *p2;
    double distance;
    double cos2Theta, sin2Theta;
    double cosTheta, sinTheta;
    double theta, adjTheta;
    double a;
    p0 = &pContext->point[0];
    p1 = &pContext->point[1];
    p2 = &pContext->point[2];
    distance = p1->U.NormalizedDifferenceOf(p2->xy, p1->xy);

    if (distance < pContext->minDistance)
        {
        /* Restart at the current point? */
        *p0 = *p2;
        pContext->numPoint = 1;
        }
    else
        {
        p1->V.Rotate90 (p1->U);
        cos2Theta = p0->U.DotProduct (p1->U);
        sin2Theta = p0->U.CrossProduct (p1->U);
        theta = 0.5 * bsiTrig_atan2 (sin2Theta, cos2Theta);
        cosTheta = cos (theta);
        sinTheta = sin (theta);
        adjTheta = pContext->signedOffset >= 0.0 ? theta : -theta;

        if (adjTheta > 0.0)
            {
            /* Offset to inside of turn */
            a = pContext->signedOffset / cosTheta;
            p1->W.SumOf(p1->U, a * sinTheta, p1->V, pContext->signedOffset);
            jmdlGPAOC_outputOffsetSegment (pContext, p0, &p0->W, p1, &p1->W);
            jmdlGPAOC_polylineDrop0 (pContext);
            }
        else if (fabs (adjTheta) > pContext->arcAngle)
            {
            DPoint2d W0, W1;
            a = pContext->signedOffset;
            W0.Scale (p0->V, a);
            W1.Scale (p1->V, a);
            jmdlGPAOC_outputOffsetSegment (pContext, p0, &p0->W, p1, &W0);
            jmdlGPAOC_outputArc (pContext, p1, &W0, theta);
            p1->W = W1;
            jmdlGPAOC_polylineDrop0 (pContext);
            }
        else if (fabs (adjTheta) > pContext->chamferAngle)
            {
            DPoint2d W, Wperp;
            DPoint2d W0, W1;
            double phi = msGeomConst_piOver2 - theta;
            double s;
            double cc = cos(phi);
            double ss = sin(phi);

            a = fabs (pContext->signedOffset);
            bsiTrig_safeDivide (&s, 1.0 - ss, cc, 1.0);

            W.SumOf(p1->U, pContext->signedOffset * cc, p1->V, pContext->signedOffset * ss);
            Wperp.Rotate90 (W);

            W0.SumOf (W, Wperp, -s);
            W1.SumOf (W, Wperp, s);
            jmdlGPAOC_outputOffsetSegment (pContext, p0, &p0->W, p1, &W0);
            jmdlGPAOC_outputOffsetSegment (pContext, p1, &W0,    p1, &W1);
            p1->W = W1;
            jmdlGPAOC_polylineDrop0 (pContext);
            }
        else
            {
            /* Safe outside offset */
            a = pContext->signedOffset / cosTheta;
            p1->W.SumOf(p1->U, a * sinTheta, p1->V, pContext->signedOffset);
            jmdlGPAOC_outputOffsetSegment (pContext, p0, &p0->W, p1, &p1->W);
            jmdlGPAOC_polylineDrop0 (pContext);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Construct a cap from 2 point final data.
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPAOC_polylineCap2
(
        GPAOffsetContext        *pContext
)
    {
    GPAOCPoint *p0, *p1;
    p0 = &pContext->point[0];
    p1 = &pContext->point[1];
    p1->U = p0->U;
    p1->V = p0->V;
    p1->W.Scale (p1->V, pContext->signedOffset);

    jmdlGPAOC_outputOffsetSegment (pContext, p0, &p0->W, p1, &p1->W);
    jmdlGPAOC_polylineDrop0 (pContext);
    }

/*---------------------------------------------------------------------------------**//**
* Advance to 4th point.
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPAOC_polylineAdvanceToPoint4
(
        GPAOffsetContext        *pContext
)
    {
    }
/*---------------------------------------------------------------------------------**//**
* Conclude the polyline.
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPAOC_polylineFinish
(
        GPAOffsetContext            *pContext
)
    {
    switch (pContext->numPoint)
        {
        case 1:
            break;
        case 2:
            jmdlGPAOC_polylineCap2 (pContext);
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Add a point to the polyline.
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPAOC_polylinePoint
(
GPAOffsetContext            *pContext,
GraphicsPoint               *pGPoint
)
    {
    DPoint3d xyz;
    /* Force the point into the array ... */
    if (!pGPoint->point.GetProjectedXYZ (xyz))
        {
        jmdlGPAOC_polylineFinish (pContext);
        jmdlGPAOC_polylineStart (pContext);
        }
    else
        {
        int i = pContext->numPoint;
        if (i > HIGHEST_CONTEXT_POINT_INDEX)
            {
            i = HIGHEST_CONTEXT_POINT_INDEX;
            pContext->numPoint = MAX_CONTEXT_POINT;
            }
        else
            {
            pContext->numPoint++;
            }

        jmdlGPAOC_polylineInsert (pContext, pGPoint, &xyz, i);
        if (i == 0)
            {
            /* First point -- nothing to do */
            }
        else if (i == 1)
            {
            jmdlGPAOC_polylineAdvanceToPoint2 (pContext);
            }
        else if (i == 2)
            {
            jmdlGPAOC_polylineAdvanceToPoint3 (pContext);
            }
        else
            {
            jmdlGPAOC_polylineAdvanceToPoint4 (pContext);
            }
        }
    }

static void trimTrailingDuplicates (GraphicsPointArrayP gpa, double tol)
    {
    int n = jmdlGraphicsPointArray_getCount (gpa) - 1;
    GraphicsPoint gpA, gpB;
    while (n >= 1)
        {
        jmdlGraphicsPointArray_getGraphicsPoint (gpa, &gpA, n-1);
        jmdlGraphicsPointArray_getGraphicsPoint (gpa, &gpB, n-2);
        if (gpA.point.RealDistance (gpB.point) <= tol)
            n--;
        else
            break;
        }
    jmdlGraphicsPointArray_trim (gpa, n);
    }



/*---------------------------------------------------------------------------------**//**
*
* Generate an offset of the lines and curves.
* @param arcAngle => If the turning angle at a vertex exceeds this angle (radians),
*                       an arc is created.   Passing a negative angle                   means no arcs.
* @param chamferAngle => If the turning angle at a vertex is smaller than
*                       arc angle but larger than chamfer angle, a chamfer is
*                       created.   This angle is always forced to .99pi or less.
* @param offsetDistance => the offset distance (uniform).  Positive is to
*                           left of curve relative to the normal.
* @param pNormal        => projection direction.
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_collectOffset
(
        GraphicsPointArray      *pCollector,
GraphicsPointArrayCP pSource,
        double                      arcAngle,
        double                      chamferAngle,
        double                      offsetDistance,
        DVec3dCP                    pNormal
)
    {
    int i0, i1, i;
    GPAOffsetContext context;
    GraphicsPoint gp;
    int curveType;
    static double sLengthFraction = 1.0e-10;
    double sourceLength = jmdlGraphicsPointArray_getLength (pSource);
    double tol = sLengthFraction * sourceLength;
    double originalArcAngle = arcAngle;
    jmdlGPAOC_init (&context, pCollector, pSource, arcAngle, chamferAngle,
                                offsetDistance, pNormal);

    if (arcAngle < 0.0)
        arcAngle = msGeomConst_2pi;
    static bool s_alwaysClose = true;
    for (i0 = 0;
        jmdlGraphicsPointArray_parseFragment (pSource, &i1, NULL, NULL, &curveType, i0);
         i0 = i1 + 1)
        {
        if (curveType == 0)
            {
            int collectorIndexA = jmdlGraphicsPointArray_getCount (pCollector);
            int numSave = 3;
            GraphicsPoint gpSave[3], gpLast;
            int jSave = 0;
            jmdlGPAOC_polylineStart (&context);
            for (i = i0; i <= i1; i++)
                {
                jmdlGraphicsPointArray_getGraphicsPoint (pSource, &gp, i);
                gpLast = gp;
                if (jSave < numSave)
                    gpSave[jSave++] = gp;
                jmdlGPAOC_polylinePoint (&context, &gp);
                }
            // If the world looks simple, force closure ...
            if ((s_alwaysClose || originalArcAngle <= 0.0)
                && i1 > i0 + 1
                && gpLast.point.RealDistance (*(&gpSave[0].point)) < tol)
                {
                GraphicsPoint gpA, gpB, gpC;
                int collectorIndexB, collectorIndexC;
                jmdlGPAOC_polylinePoint (&context, &gpSave[1]);
                jmdlGPAOC_polylineFinish (&context);
                // The output has wrapped around and made a "final" line that partially overlaps the "first"
                // Trim it off.
                // Move the true first point to match then end after the trim.
                collectorIndexC = jmdlGraphicsPointArray_getCount (pCollector) - 1;
                collectorIndexB = collectorIndexC - 1;
                jmdlGraphicsPointArray_getGraphicsPoint (pCollector, &gpA, collectorIndexA);
                jmdlGraphicsPointArray_getGraphicsPoint (pCollector, &gpB, collectorIndexB);
                jmdlGraphicsPointArray_getGraphicsPoint (pCollector, &gpC, collectorIndexC);
                gpA.point = gpB.point;
                jmdlGraphicsPointArray_setGraphicsPoint (pCollector, &gpA, collectorIndexA);
                gpC.point = gpB.point;
                jmdlGraphicsPointArray_setGraphicsPoint (pCollector, &gpC, collectorIndexC);
                trimTrailingDuplicates (pCollector, tol);
                }
            else
                jmdlGPAOC_polylineFinish (&context);
            }
        else
            {
            /* Just copy it?? */
            jmdlGraphicsPointArray_appendFragment (pCollector, pSource, i0, i1, 0);
            jmdlGraphicsPointArray_markBreak (pCollector);
            }
        }

    }

typedef struct
    {
    double arcAngle;
    double chamferAngle;
    double offsetDistance;
    GraphicsPointArrayCP pSource;
    GraphicsPointArray  *pCollector;
    double tolerance;
    int    sourceCount;
    } GPAXYOffsetContext;

typedef struct
    {
    int i0;
    int curveType;
    double offsetDistance;
    double finalOffsetDistance;
    double fraction0;
    double fraction1;
    DSegment4d segment;

    DConic4d conic;

    DPoint4d poleArray[MAX_BEZIER_CURVE_ORDER];
    int     numPole;
    } GPAPrimitive;

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPAPrimitive_initCommon
(
GPAPrimitive    *pPrim,
int             curveType,
int             i0,
double          fraction0,
double          fraction1,
double          offsetDistance
)
    {
    pPrim->curveType = curveType;
    pPrim->i0 = i0;
    pPrim->fraction0 = fraction0;
    pPrim->fraction1 = fraction1;
    pPrim->offsetDistance = offsetDistance;
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPAPrimitive_initFromDSegment4d
(
GPAPrimitive    *pPrim,
int             i0,
DSegment4d      *pSegment,
double          offsetDistance
)
    {
    jmdlGPAPrimitive_initCommon (pPrim, 0, i0, 0.0, 1.0, offsetDistance);
    pPrim->segment = *pSegment;
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPAPrimitive_initFromDSegment3d
(
GPAPrimitive    *pPrim,
int             i0,
DSegment3d      *pSegment,
double          offsetDistance
)
    {
    jmdlGPAPrimitive_initCommon (pPrim, 0, i0, 0.0, 1.0, offsetDistance);
    bsiDSegment4d_initFromDPoint3d
                    (&pPrim->segment, &pSegment->point[0], &pSegment->point[1]);
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPAPrimitive_initFromBezierDPoint4d
(
GPAPrimitive    *pPrim,
int             i0,
DPoint4d        *pPoleArray,
int             order,
double          currOffsetDistance,
double          finalOffsetDistance
)
    {
    int i;
    jmdlGPAPrimitive_initCommon (pPrim, HPOINT_MASK_CURVETYPE_BEZIER, i0, 0.0, 1.0, currOffsetDistance);// BSPLINE_AT_NEXT_LEVEL

    for (i = 0; i < order; i++)
        {
        pPrim->poleArray[i] = pPoleArray[i];
        }
    pPrim->numPole = order;
    pPrim->finalOffsetDistance = finalOffsetDistance;
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPAPrimitive_initNull
(
GPAPrimitive    *pPrim
)
    {
    jmdlGPAPrimitive_initCommon (pPrim, CURVETYPE_NULL, -1, 0.0, 1.0, 0.0);
    }


/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPAPrimitive_initFromDConic4d
(
GPAPrimitive    *pPrim,
int             i0,
DConic4d        *pConic,
double          offsetDistance
)
    {
    jmdlGPAPrimitive_initCommon (pPrim, HPOINT_MASK_CURVETYPE_ELLIPSE, i0, 0.0, 1.0, offsetDistance);
    pPrim->conic = *pConic;
    }
#ifdef CompileAll
/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        jmdlGPAPrimitive_isNullCurve
(
GPAPrimitive    *pPrim
)
    {
    return pPrim->curveType == CURVETYPE_NULL;
    }
#endif
/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        jmdlGPAPrimitive_fractionToDPoint4dTangent
(
GPAPrimitive    *pPrim,
DPoint4d        *pPoint,
DPoint4d        *pTangent,
double          fraction
)
    {
    bool    boolstat = false;
    int curveType = pPrim->curveType;
    if (curveType == CURVETYPE_LINE)
        {
        DPoint4d tangent;
        bsiDPoint4d_subtractDPoint4dDPoint4d (&tangent, &pPrim->segment.point[1], &pPrim->segment.point[0]);
        if (pPoint)
            bsiDPoint4d_addScaledDPoint4d (pPoint, &pPrim->segment.point[0], &tangent, fraction);
        if (pTangent)
            *pTangent = tangent;
        boolstat = true;
        }
    else if (curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
        {
        bsiDConic4d_angleParameterToDPoint4dDerivatives
                (
                &pPrim->conic,
                pPoint,
                pTangent,
                NULL,
                bsiDConic4d_fractionToAngle (&pPrim->conic, fraction)
                );
        boolstat = true;
        }
    else if (curveType == HPOINT_MASK_CURVETYPE_BEZIER) // BSPLINE_NOT_REQUIRED (exploded to prims)
        {
        bsiBezierDPoint4d_evaluateArray
                (
                pPoint,
                pTangent,
                pPrim->poleArray,
                pPrim->numPole,
                &fraction,
                1
                );
        boolstat = true;
        }
    return boolstat;
    }


/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPAXYOffset_init
(
        GPAXYOffsetContext          *pContext,
        GraphicsPointArray      *pCollector,
GraphicsPointArrayCP pSource,
        double                      arcAngle,
        double                      chamferAngle,
        double                      offsetDistance
)
    {
    pContext->pCollector     = pCollector;
    pContext->pSource        = pSource;
    pContext->arcAngle       = arcAngle;
    pContext->chamferAngle   = chamferAngle;
    pContext->offsetDistance = offsetDistance;
    pContext->tolerance      = 1.0e-8;
    pContext->sourceCount    = jmdlGraphicsPointArray_getCount (pSource);
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     jmdlGPAXYOffset_initOffsetFromDSegment4d
(
GPAXYOffsetContext          *pContext,
GPAPrimitive     *pOffset,
DSegment4d      *pOriginalSegment,
double          distance,
int             i0
)
    {
    DSegment3d localSegment;
    DSegment3d offsetSegment;
    DPoint3d tangent, perp;

    pOriginalSegment->point[0].GetProjectedXYZ (*(&localSegment.point[0]));
    pOriginalSegment->point[1].GetProjectedXYZ (*(&localSegment.point[1]));
    tangent.DifferenceOf (*(&localSegment.point[1]), *(&localSegment.point[0]));
    perp.Init ( -tangent.y, tangent.x, 0.0);
    if (bsiDPoint3d_scaleToLength (&perp, &perp, pContext->offsetDistance) > 0.0)
        {
        offsetSegment.point[0].SumOf (*(&localSegment.point[0]), perp);
        offsetSegment.point[1].SumOf (*(&localSegment.point[1]), perp);
        jmdlGPAPrimitive_initFromDSegment3d (pOffset, i0, &offsetSegment, distance);
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* Note that a circle is the only conic that offsets to another conic.
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     jmdlGPAXYOffset_initOffsetFromDConic4d
(
GPAXYOffsetContext          *pContext,
GPAPrimitive        *pOffset,
DConic4d        *pOriginalConic,
double          distance,
int             i0
)
    {
    DEllipse3d localEllipse;

    if (    bsiDEllipse3d_initFromDConic4d (&localEllipse, pOriginalConic)
        &&  fabs (localEllipse.vector0.z) <= pContext->tolerance
        &&  fabs (localEllipse.vector90.z) <= pContext->tolerance
        &&  bsiDEllipse3d_isCircular (&localEllipse)
        )
        {
        double cc = cos (localEllipse.start);
        double ss = sin (localEllipse.start);
        double r0, r1, lambda;
        DPoint3d radialVector, tangentVector, perpVector, offsetRadiusVector;
        bsiDPoint3d_add2ScaledDPoint3d (&radialVector, NULL,
                                        &localEllipse.vector0, cc,
                                        &localEllipse.vector90, ss);
        bsiDPoint3d_add2ScaledDPoint3d (&tangentVector, NULL,
                                        &localEllipse.vector0, -ss,
                                        &localEllipse.vector90, cc);
        perpVector.Init ( -tangentVector.y, tangentVector.x, 0.0);
        bsiDPoint3d_scaleToLength (&perpVector, &perpVector, pContext->offsetDistance);
        offsetRadiusVector.SumOf (radialVector, perpVector);
        r0 = radialVector.Magnitude ();
        r1 = offsetRadiusVector.Magnitude ();
        if (bsiTrig_safeDivide (&lambda, r1, r0, 0.0))
            {
            DConic4d offsetConic;
            offsetConic = *pOriginalConic;
            bsiDPoint4d_scaleInPlace (&offsetConic.vector0,  lambda);
            bsiDPoint4d_scaleInPlace (&offsetConic.vector90, lambda);
            jmdlGPAPrimitive_initFromDConic4d (pOffset, i0, &offsetConic, distance);
            return true;
            }
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     jmdlGPAXYOffset_initOffsetFromBezierDPoint4d
(
GPAXYOffsetContext  *pContext,
GPAPrimitive        *pOffset,
DPoint4d            *pPoleArray,
int                 order,
double              distance,
int                 i0
)
    {
    jmdlGPAPrimitive_initFromBezierDPoint4d (pOffset, i0, pPoleArray, order,
                                0.0, distance);

    return true;
    }


/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool                jmdlGPAXYOffset_readAndOffsetPrimitive
(
    GPAXYOffsetContext      *pContext,
    int                     *pNextStart,
    GPAPrimitive            *pMidline,
    GPAPrimitive            *pLeftOffset,
    int                     i0
)
    {
    DSegment4d segment;
    DConic4d conic;
    DPoint4d poleArray[MAX_BEZIER_ORDER];
    int numPole, maxPole;
    maxPole = MAX_BEZIER_ORDER;
    *pNextStart = i0;
    static double s_absTol = 1.0e-10;

    if (jmdlGraphicsPointArray_getDSegment4d (pContext->pSource, pNextStart, &segment))
        {
        if (segment.point[0].RealDistance (segment.point[1]) < s_absTol) // 0 length segment --- skip it ... really should never recurse more than once....
            {
            return jmdlGPAXYOffset_readAndOffsetPrimitive (pContext, pNextStart, pMidline, pLeftOffset, 1 + (*pNextStart));
            }
        jmdlGPAPrimitive_initFromDSegment4d (pMidline, i0, &segment, 0.0);
        return jmdlGPAXYOffset_initOffsetFromDSegment4d
                                (
                                pContext,
                                pLeftOffset,
                                &segment,
                                pContext->offsetDistance,
                                i0
                                );
        }
    else if (jmdlGraphicsPointArray_getDConic4d
                    (pContext->pSource, pNextStart, &conic, NULL, NULL, NULL, NULL))
        {
        jmdlGPAPrimitive_initFromDConic4d (pMidline, i0, &conic, 0.0);
        return jmdlGPAXYOffset_initOffsetFromDConic4d
                                (
                                pContext,
                                pLeftOffset,
                                &conic,
                                pContext->offsetDistance,
                                i0
                                );
        }
    else if (jmdlGraphicsPointArray_getBezier
                    (pContext->pSource, pNextStart, poleArray, &numPole, maxPole))
        {
        return jmdlGPAXYOffset_initOffsetFromBezierDPoint4d
                                (
                                pContext,
                                pLeftOffset,
                                poleArray,
                                numPole,
                                pContext->offsetDistance,
                                i0
                                );
        }
    else
        {
        size_t tailIndex;
        if (pContext->pSource->IsBsplineCurve (*pNextStart, tailIndex))
            {
            *pNextStart = (int)tailIndex;
            TaggedBezierDPoint4d bezier (pContext->pSource);
            // Design problem -- We have a full bspline.
            //  The offset context is only read for a single bspline.
 #ifdef CompileAll
            size_t readIndex = (size_t)*pNextStart;
            for (size_t k = 0; bezier.LoadBsplineSpan (readIndex, k); k++)
                {
                return jmdlGPAXYOffset_initOffsetFromBezierDPoint4d
                                (
                                pContext,
                                pLeftOffset,
                                bezier.m_poles,
                                bezier.m_order,
                                pContext->offsetDistance,
                                i0
                                );
                }
#else
            return false;   // fail
#endif                
            }
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* Construct an offset bezier of specified order over a subinterval.  Caller responsible
*   for picking good order and interval!!!
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool       jmdlGPAXYOffset_offsetBezierXYDPoint4dInterval
(
      DPoint4d              *pOffsetPoleArray,
      int                   offsetOrder,
const DPoint4d              *pBasePoleArray,
      int                   baseOrder,
      double                s0,
      double                s1,
      double                distance,
      bool                  cusp0,
      bool                  cusp1
)
    {
    DPoint3d offsetPoint[MAX_BEZIER_ORDER];
    DPoint3d baseTangent[MAX_BEZIER_ORDER];
    DPoint3d basePoint[MAX_BEZIER_ORDER];
    double baseParam[MAX_BEZIER_ORDER];
    double interpA[MAX_BEZIER_ORDER];
    double interpF[MAX_BEZIER_ORDER];
    DPoint3d normal, unitNormal;
    int i, k;
    double ds;
    DPoint4d refPoint;
    double t0, dt;
    ds = (s1 - s0);
    bsiDPoint4d_setComponents (&refPoint, 0.0, 0.0, 0.0, 1.0);

    baseParam[0] = s0;
    baseParam[offsetOrder - 1] = s1;

    /* We don't want poles clumped up at the cusps.  Crude way to do this
        is to have more points in the grid over 0..1, but skip the ones
        immediately adjacent to the cusps. */
    if (cusp0 && cusp1)
        {
        dt = ds / (offsetOrder + 1);
        t0 = s0 + dt;
        }
    else if (cusp0)
        {
        dt = ds / (offsetOrder);
        t0 = s0 + dt;
        }
    else if (cusp1)
        {
        dt = ds / (offsetOrder);
        t0 = s0;
        }
    else
        {
        dt = ds / (offsetOrder - 1);
        t0 = s0;
        }

    for (i = 1; i < offsetOrder - 1; i++)
        {
        baseParam[i] = t0 + i * dt;
        }

    bsiBezierDPoint4d_evaluateDPoint3dArray (basePoint, baseTangent, pBasePoleArray, baseOrder,
                        baseParam, offsetOrder);
    for (i = 0; i < offsetOrder; i++)
        {
        normal.Init ( -baseTangent[i].y, baseTangent[i].x, 0.0);
        unitNormal.Normalize (normal);
        offsetPoint[i].SumOf (basePoint[i], unitNormal, distance);
        /* get a unit weight in the offset point. */
        pOffsetPoleArray[i] = refPoint;
        }
    for (k = 0; k < 3; k++)
        {
        bsiBezier_copyComponent (interpF, 0, 1, (double*)offsetPoint, k, 3, offsetOrder);
        if (!bsiBezier_univariateInterpolationPoles (interpA, interpF, offsetOrder))
            return false;
        bsiBezier_copyComponent ((double*)pOffsetPoleArray, k, 4, interpA, 0, 1, offsetOrder);
        }
    return true;
    }
/*---------------------------------------------------------------------------------**//**
*
* @param pDegenerate <= true if the curvature is identically R everywhere.
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool       bsiBezierDPoint4d_xyCurvatureCriticalPoints
(
      double                *pParams,
      int                   *pNumParam,
      bool                  *pDegenerate,
      int                   maxParam,
const DPoint4d              *pPoles,
      int                   order,
      double                distance
)
    {
    DPoint4d dPoles[MAX_BEZIER_CURVE_ORDER];
    DPoint4d ddPoles[MAX_BEZIER_CURVE_ORDER];
    double f[MAX_BEZIER_ORDER];
    double ff[MAX_BEZIER_ORDER];
    double fff[MAX_BEZIER_ORDER];
    double g[MAX_BEZIER_ORDER];
    double gg[MAX_BEZIER_ORDER];
    double h[MAX_BEZIER_ORDER];
    double roots[MAX_BEZIER_ORDER];

    int dOrder = order - 1;
    int ddOrder = order - 2;
    int orderF, orderG, orderFF, orderFFF, orderGG;
    int orderH;

    *pDegenerate = false;
    *pNumParam = 0;

    if (order > MAX_BEZIER_CURVE_ORDER)
        return false;

    bsiBezier_derivativePoles ((double*)dPoles, (double*)pPoles, order, 4);
    bsiBezier_derivativePoles ((double*)ddPoles, (double*)dPoles, dOrder, 4);
    /* Squared curvature is R^2 = (x'^2 + y'^2)^3 / ( x'y''-x''y')^2 = f^3 / g^2 */
    if (   bsiBezier_dotProduct (f, &orderF, MAX_BEZIER_ORDER,
                            (double*)dPoles, dOrder, 0, 4,
                            (double*)dPoles, dOrder, 0, 4, 2)
       && bsiBezier_crossProductComponent (g, &orderG, 0, 1, MAX_BEZIER_ORDER,
                            (double*)dPoles, dOrder, 0, 1, 4,
                            (double*)ddPoles, ddOrder, 0, 1, 4)
       && bsiBezier_univariateProductExt (ff, &orderFF,MAX_BEZIER_ORDER, 0, 1,
                            f, orderF, 0, 1,
                            f, orderF, 0, 1)
       && bsiBezier_univariateProductExt (fff, &orderFFF, MAX_BEZIER_ORDER, 0, 1,
                            f,  orderF,  0, 1,
                            ff, orderFF, 0, 1)
       && bsiBezier_univariateProductExt (gg, &orderGG, MAX_BEZIER_ORDER, 0, 1,
                            g,  orderG, 0, 1,
                            g,  orderG, 0, 1)
        )
        {
        int i;
        double rr = distance * distance;
        double absH, maxH, currAbsH;
        int numRoot;
        static double s_relTol = 1.0e-12;
        orderH = orderFFF;

        maxH = absH = 0.0;
        bsiBezier_raiseDegreeInPlace (gg, orderGG, orderH, 1);
        for (i = 0; i < orderH; i++)
            {
            h[i] = fff[i] - rr * gg[i];
            currAbsH = fabs (h[i]);
            absH += fabs (fff[i]) + rr * fabs (gg[i]);
            if (currAbsH > maxH)
                maxH = currAbsH;
            }

        if (maxH < s_relTol * absH)
            {
            *pDegenerate = true;
            *pNumParam = 0;
            return true;
            }

        bsiBezier_univariateRoots (roots, &numRoot, h, orderH);
        if (numRoot == orderH)
            {
            *pDegenerate = true;
            *pNumParam = 0;
            }
        else
            {
            int numReturn = numRoot;
            if (numRoot > maxParam)
                numReturn = maxParam;
            *pNumParam = numReturn;
            for (i = 0; i < numReturn; i++)
                {
                pParams[i] = roots[i];
                }
            if (numRoot != numReturn)
                return false;
            }
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool       jmdlGPAXYOffset_offsetAndOutputBezierDPoint4d
(
    GPAXYOffsetContext      *pContext,
const DPoint4d              *pBasePoleArray,
      int                   baseOrder,
      double                distance,
      int                   numSub,
      int                   outputOrder
)
    {
#define BREAK_CUSP 1
#define BREAK_NOCUSP 2
    DPoint4d offsetPoleArray[MAX_BEZIER_ORDER];
    bool    degenerate;
    double s0, s1, ds;
    double u0, u1;
    int numCrit;
    int i, k;
    double criticalParameters[MAX_BEZIER_ORDER + 2];
    int    breakType[MAX_BEZIER_ORDER + 2];

    if (numSub < 1)
        numSub = 1;

    bsiBezierDPoint4d_xyCurvatureCriticalPoints (criticalParameters+1, &numCrit, &degenerate,
                    MAX_BEZIER_ORDER, pBasePoleArray, baseOrder, distance);

    criticalParameters[0] = 0.0;
    breakType[0] = BREAK_NOCUSP;

    for (i = 1; i <= numCrit; i++)
        breakType[i] = BREAK_CUSP;
    numCrit++;

    if (criticalParameters[numCrit - 1] < 1.0)
        {
        criticalParameters[numCrit] = 1.0;
        breakType[numCrit]=BREAK_NOCUSP;
        numCrit++;
        }

    for (s0 = 0.0, i = 1; i < numCrit; i++, s0 = s1)
        {
        s1 = criticalParameters[i];
        ds = (s1 - s0) / numSub;
        if (ds > 0.0)
            {
            for (k = 0; k < numSub; k++)
                {
                u0 = s0 + k * ds;
                u1 = s0 + (k + 1) * ds;
                if (jmdlGPAXYOffset_offsetBezierXYDPoint4dInterval
                                (
                                offsetPoleArray,
                                outputOrder,
                                pBasePoleArray,
                                baseOrder, u0, u1, distance,
                                k == 0 && breakType[i-1] == BREAK_CUSP,
                                k == numSub - 1 && breakType[i] == BREAK_CUSP
                                ))
                    {
#ifdef NOISYOFFSETS
                    if (i > 0 || k > 0)
                        {
                        DPoint4d basePoint;
                        bsiBezierDPoint4d_evaluateArray (&basePoint, NULL, pBasePoleArray, baseOrder, &u0, 1);
                        jmdlGraphicsPointArray_addDPoint4d (pContext->pCollector, &offsetPoleArray[0]);
                        jmdlGraphicsPointArray_addDPoint4d (pContext->pCollector, &basePoint);
                        jmdlGraphicsPointArray_markBreak (pContext->pCollector);
                        }

                        {
                        int m;
                        printf ("******** i=%d   k=%d\n", i, k);
                        for (m = 0; m < outputOrder; m++)
                            {
                            jmdlGraphicsPointArray_addDPoint4d (pContext->pCollector,
                                                    &offsetPoleArray[m]);
                            jmdlGraphicsPointArray_markPoint (pContext->pCollector);
                            printf ("   %23.16le %23.16le\n",
                                            offsetPoleArray[m].x,
                                            offsetPoleArray[m].y);
                            }
                        }
#endif
                    jmdlGraphicsPointArray_addDPoint4dBezier
                            (pContext->pCollector, offsetPoleArray, outputOrder, 1, false);
                    }
                }
            }
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
*
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool       jmdlGPAXYOffset_outputPrimitive
(
    GPAXYOffsetContext      *pContext,
    GPAPrimitive            *pPrim
)
    {
    bool    boolstat = false;
    if (pPrim->curveType == 0)
        {
        DPoint4d point;
        bsiDSegment4d_fractionParameterToDPoint4d (&pPrim->segment, &point, pPrim->fraction0);
        jmdlGraphicsPointArray_addDPoint4d (pContext->pCollector, &point);
        bsiDSegment4d_fractionParameterToDPoint4d (&pPrim->segment, &point, pPrim->fraction1);
        jmdlGraphicsPointArray_addDPoint4d (pContext->pCollector, &point);
        jmdlGraphicsPointArray_markBreak (pContext->pCollector);
        boolstat = true;
        }
    else if (pPrim->curveType == HPOINT_MASK_CURVETYPE_ELLIPSE)
        {
        DConic4d newConic = pPrim->conic;
        double theta, sweep;
        sweep = (pPrim->fraction1 - pPrim->fraction0) * newConic.sweep;
        theta = newConic.start + pPrim->fraction0 * newConic.sweep;
        newConic.start = theta;
        newConic.sweep = sweep;
        jmdlGraphicsPointArray_addDConic4d (pContext->pCollector, &newConic);
        boolstat = true;
        }
    else if (pPrim->curveType == HPOINT_MASK_CURVETYPE_BEZIER)  // NO BSPLINE NEEDED
        {
        if (pPrim->offsetDistance == pPrim->finalOffsetDistance)
            jmdlGraphicsPointArray_addDPoint4dBezier (pContext->pCollector,
                            pPrim->poleArray,
                            pPrim->numPole,
                            1,
                            false);
        else
            {
            static int numSpan = 2;
            static int s_offsetFixedOrder = 4;
            jmdlGPAXYOffset_offsetAndOutputBezierDPoint4d (pContext,
                            pPrim->poleArray,
                            pPrim->numPole,
                            pPrim->finalOffsetDistance - pPrim->offsetDistance,
                            numSpan,
                            s_offsetFixedOrder > 0 ? s_offsetFixedOrder : pPrim->numPole
                            );
            }
        boolstat = true;
        }
    else if (pPrim->curveType == CURVETYPE_NULL)
        {
        boolstat = true;
        }
    return boolstat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    jmdlGPAXYOffset_needJoint
(
GPAXYOffsetContext      *pContext,
GPAPrimitive            *pMidline0,
GPAPrimitive            *pMidline1
)
    {
    DPoint4d point0, point1;
    double distanceSquared;
    bool    boolstat = false;
    if (   jmdlGPAPrimitive_fractionToDPoint4dTangent (pMidline0, &point0, NULL, 1.0)
        && jmdlGPAPrimitive_fractionToDPoint4dTangent (pMidline1, &point1, NULL, 0.0)
        )
        {
        point0.z = point1.z;
        if (point0.RealDistanceSquared (&distanceSquared, point1))
            {
            boolstat = distanceSquared <= pContext->tolerance * pContext->tolerance;
            }
        }
    return boolstat;
    }

/*---------------------------------------------------------------------------------**//**
* Convert an angle to a fraction which is 0..1 within the conic, negative if outside and
*  closer to start (in angle space), positive if outside and closer to end.
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    fixOffsetParameters
(
GPAPrimitive    *pPrim0,
GPAPrimitive    *pPrim1,
const double    *pFraction0,
const double    *pFraction1,
int             numFraction
)
    {
    int i, iBest;
    double d, dBest = DBL_MAX;
    static double s_tangencyTol = 1.0e-4;
    bool    boolstat = false;
    iBest = -1;
    for (i = 0; i < numFraction; i++)
        {
        double f0 = pFraction0[i];
	double f1 = pFraction1[i];
        d = fabs (f0 - 1.0) + fabs (f1);
	if (iBest < 0 || (((f0 - 0.5) * (f1 - 0.5) < 0.0 || d < s_tangencyTol) && d < dBest))
            {
            dBest = d;
            iBest = i;
            }
        }

    if (iBest >= 0)
        {
        pPrim0->fraction1 = pFraction0[iBest];
        pPrim1->fraction0 = pFraction1[iBest];
        boolstat = true;
        }
    return boolstat;
    }

/*---------------------------------------------------------------------------------**//**
* Convert an angle to a fraction which is 0..1 within the conic, negative if outside and
*  closer to start (in angle space), positive if outside and closer to end.
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static double  jmdlDConic4d_angleToSignedFraction
(
const DConic4d  *pConic,
double          angle
)
    {
    double forwardFraction = bsiDConic4d_angleParameterToFraction
                                (pConic, angle);
    double result = forwardFraction;
    if (forwardFraction > 1.0)
        {
        double reverseFraction = bsiTrig_normalizeAngleToSweep
                                    (
                                    angle,
                                    pConic->start + pConic->sweep,
                                    -pConic->sweep
                                    );
        /* REMARK: reverseFraction > 1.0 should be assertable, but let's be really sure: */
        if (reverseFraction > 1.0 && forwardFraction > reverseFraction)
            result = 1.0 - reverseFraction;
        }
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
static void    jmdlGPAXYOffset_trimAndJoin
(
GPAXYOffsetContext      *pContext,
GPAPrimitive            *pJoint,
GPAPrimitive            *pMidline0,
GPAPrimitive            *pOffset0,
GPAPrimitive            *pMidline1,
GPAPrimitive            *pOffset1
)
    {
    int type0 = pOffset0->curveType;
    int type1 = pOffset1->curveType;
    jmdlGPAPrimitive_initNull (pJoint);
    if (jmdlGPAXYOffset_needJoint (pContext, pMidline0, pMidline1))
        {
        if (type0 == CURVETYPE_LINE && type1 == CURVETYPE_LINE)
            {
            DPoint4d point0, point1;
            double param0, param1;
            if (bsiDSegment4d_intersectXYDSegment4dDSegment4d (&point0, &param0, &point1, &param1,
                                    &pOffset0->segment, &pOffset1->segment))
                {
                pOffset0->fraction1 = param0;
                pOffset1->fraction0 = param1;
                }
            }
        else if (type0 == CURVETYPE_LINE && type1 == HPOINT_MASK_CURVETYPE_ELLIPSE)
            {
            int numIntersection;
            double lineFraction[2], conicAngle[2], conicFraction[2];
            DPoint4d linePoint[2], conicPoint[2];
            int i;
            numIntersection = bsiDConic4d_intersectDSegment4dXYW
                        (
                        &pOffset1->conic,
                        conicPoint,
                        conicAngle,
                        linePoint,
                        lineFraction,
                        &pOffset0->segment
                        );

            for (i = 0; i < numIntersection; i++)
                {
                conicFraction[i] = jmdlDConic4d_angleToSignedFraction (&pOffset1->conic, conicAngle[i]);
                }
            fixOffsetParameters (pOffset0, pOffset1, lineFraction, conicFraction, numIntersection);
            }
        else if (type0 == HPOINT_MASK_CURVETYPE_ELLIPSE && type1 == CURVETYPE_LINE)
            {
            int numIntersection;
            DPoint4d linePoint[2], conicPoint[2];
            double lineFraction[2], conicAngle[2], conicFraction[2];
            int i;
            numIntersection = bsiDConic4d_intersectDSegment4dXYW
                        (
                        &pOffset0->conic,
                        conicPoint,
                        conicAngle,
                        linePoint,
                        lineFraction,
                        &pOffset1->segment
                        );

            for (i = 0; i < numIntersection; i++)
                {
                conicFraction[i] = jmdlDConic4d_angleToSignedFraction (&pOffset0->conic, conicAngle[i]);
                }
            fixOffsetParameters (pOffset0, pOffset1, conicFraction, lineFraction, numIntersection);
            }
        else if (type0 == HPOINT_MASK_CURVETYPE_ELLIPSE && type1 == HPOINT_MASK_CURVETYPE_ELLIPSE)
            {
            int numIntersection;
            DPoint4d point0[2], point1[2];
            double angle0[2], angle1[2], fraction0[2], fraction1[2];
            int i;
            numIntersection = bsiDConic4d_intersectDConic4dXYW
                        (
                        &pOffset0->conic,
                        point0,
                        angle0,
                        point1,
                        angle1,
                        &pOffset1->conic
                        );

            for (i = 0; i < numIntersection; i++)
                {
                fraction0[i] = jmdlDConic4d_angleToSignedFraction (&pOffset0->conic, angle0[i]);
                fraction1[i] = jmdlDConic4d_angleToSignedFraction (&pOffset1->conic, angle1[i]);
                }
            fixOffsetParameters (pOffset0, pOffset1, fraction0, fraction1, numIntersection);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
*
* Generate an offset of the lines and curves.
* @param arcAngle => If the turning angle at a vertex exceeds this angle (radians),
*                       an arc is created.   Passing a negative angle                   means no arcs.
* @param chamferAngle => If the turning angle at a vertex is smaller than
*                       arc angle but larger than chamfer angle, a chamfer is
*                       created.   This angle is always forced to .99pi or less.
* @param offsetDistance => the offset distance (uniform).  Positive is to
*                           left of curve relative to the normal.
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_collectOffsetXY
(
        GraphicsPointArray      *pCollector,
GraphicsPointArrayCP pSource,
        double                      arcAngle,
        double                      chamferAngle,
        double                      offsetDistance,
        bool                        enforceClosure
)
    {
    GPAXYOffsetContext context;

    double smallAngle = bsiTrig_smallAngle ();

    if (enforceClosure && !jmdlGraphicsPointArray_isClosed (pSource, smallAngle, smallAngle))
        enforceClosure = false;

    jmdlGPAXYOffset_init (
                            &context,
                            pCollector,
                            pSource,
                            arcAngle,
                            chamferAngle,
                            offsetDistance
                            );

    bvector<GPAPrimitive> basePrim;
    bvector<GPAPrimitive> offsetPrim;
    bvector<GPAPrimitive> jointPrim;
    GPAPrimitive currBase, currOffset, currJoint;

    int i0, i1;
    int numGP = jmdlGraphicsPointArray_getCount (pSource);
    for (i0 = 0, i1 = 0;jmdlGPAXYOffset_readAndOffsetPrimitive (&context, &i1, &currBase, &currOffset, i0); i0 = i1)
        /* Shift old primitives away. Bah humbug, who cares about all the data this moves. */
            {
            /* First prim --just leave it accumulate */
        basePrim.push_back (currBase);
        offsetPrim.push_back (currOffset);

        if (jmdlGraphicsPointArray_isMajorBreak (pSource, i1 - 1)
            || i1 >= numGP)
            {

            // Compute joint primitives between each offset and its successor (wrapping around as needed)
            size_t numPrim = basePrim.size ();
            if (numPrim > 0)
                {
                size_t lastPrim = enforceClosure ? numPrim : numPrim - 1;
                for (size_t k0 = 0; k0 < lastPrim; k0++)
                    {
                    size_t k1 = (k0 + 1) % numPrim;
            jmdlGPAXYOffset_trimAndJoin (&context,
	                    &currJoint,
                        &basePrim[k0], &offsetPrim[k0],
	                    &basePrim[k1], &offsetPrim[k1]);
                    jointPrim.push_back (currJoint);
                }

                size_t numJoint = enforceClosure ? numPrim : numPrim - 1;
                for (size_t k = 0; k < numJoint; k++)
                {
                    jmdlGPAXYOffset_outputPrimitive (&context, &offsetPrim[k]);
                    jmdlGPAXYOffset_outputPrimitive (&context, &jointPrim[k]);
                }
                if (!enforceClosure)
                    jmdlGPAXYOffset_outputPrimitive (&context, &offsetPrim.back ());
            }

            jmdlGraphicsPointArray_markMajorBreak (pCollector);
            basePrim.clear ();
            offsetPrim.clear ();
            jointPrim.clear ();
			}
        }
    }

typedef struct
    {
    double distance;
    bool    createCircles;
    DPoint3d keyPoint[2];
    bool     keyKnown[2];
    GraphicsPointArray *pCollector;
    double tolerance;
    } PrimitiveOffsetContext;

/*---------------------------------------------------------------------------------**//**
* Stuff a circle into the output
+---------------+---------------+---------------+---------------+---------------+------*/
static void    announcePrimitiveOffsetCircle
(
        PrimitiveOffsetContext      *pContext,
const   DPoint3d                    *pStart
)
    {
    DEllipse3d ellipse;
    double r = pContext->distance;
    bsiDEllipse3d_init
            (
            &ellipse,
            pStart->x, pStart->y, pStart->z,
            r, 0.0, 0.0,
            0.0, r, 0.0,
            0.0, 2.0 * msGeomConst_2pi
            );
    jmdlGraphicsPointArray_addDEllipse3d (pContext->pCollector, &ellipse);
    jmdlGraphicsPointArray_markBreak (pContext->pCollector);
    jmdlGraphicsPointArray_markMajorBreak (pContext->pCollector);
    }

/*---------------------------------------------------------------------------------**//**
* Test endpoints against prior points and record circles.
+---------------+---------------+---------------+---------------+---------------+------*/
static void    announcePrimitiveOffsetStartEnd
(
        PrimitiveOffsetContext      *pContext,
const   DPoint3d                    *pStart,
const   DPoint3d                    *pEnd
)
    {
    if (  pContext->keyKnown[1]
       && pStart->DistanceSquaredXY (*(&pContext->keyPoint[1])) <= pContext->tolerance)
        {
        /* Start point duplicates an old end point. */
        if (  pContext->keyKnown[0]
            && pEnd->DistanceSquaredXY (*(&pContext->keyPoint[0])) <= pContext->tolerance)
            {
            /* We have closed a loop.  Both points are already out, just clear away
                the logged points */
            pContext->keyKnown[0] = pContext->keyKnown[1] = false;
            }
        else
            {
            announcePrimitiveOffsetCircle (pContext, pEnd);
            pContext->keyPoint[1] = *pEnd;
            }
        }
    else
        {
        announcePrimitiveOffsetCircle (pContext, pStart);
        announcePrimitiveOffsetCircle (pContext, pEnd);
        pContext->keyKnown[0] = true;
        pContext->keyPoint[0] = *pStart;
        pContext->keyKnown[1] = true;
        pContext->keyPoint[1] = *pEnd;
        }
    }

/*---------------------------------------------------------------------------------**//**
* "Process" function for DSegment4d hatch block boundaries.
* conic and dispatch to more generic integrators.  (Still a couple levels away
* from evaluating the curve!!)
+---------------+---------------+---------------+---------------+---------------+------*/
static bool                    cbPrimitiveOffsetDSegment4d
(
        PrimitiveOffsetContext      *pContext,
GraphicsPointArrayCP pSource,
        int                         index,
const   DSegment4d                  *pSegment
)
    {
    DSegment3d segment;
    DPoint3d unitNormal, fullTangent;
    DPoint3d polygon[5];

    if (    pSegment->point[0].GetProjectedXYZ (*(&segment.point[0]))
        &&  pSegment->point[1].GetProjectedXYZ (*(&segment.point[1])))
        {
        fullTangent.DifferenceOf (*(&segment.point[1]), *(&segment.point[0]));
	    if (bsiDPoint3d_unitPerpendicularXY (&unitNormal, &fullTangent))
            {
            polygon[0].SumOf (*(&segment.point[0]), unitNormal, pContext->distance);
            polygon[1].SumOf (*(&segment.point[0]), unitNormal, -pContext->distance);
            polygon[2].SumOf (*(&segment.point[1]), unitNormal, -pContext->distance);
            polygon[3].SumOf (*(&segment.point[1]), unitNormal, pContext->distance);
            polygon[4] = polygon[0];
            jmdlGraphicsPointArray_addDPoint3dArray (pContext->pCollector, polygon, 5);
            jmdlGraphicsPointArray_markBreak (pContext->pCollector);
            jmdlGraphicsPointArray_markMajorBreak (pContext->pCollector);
            announcePrimitiveOffsetStartEnd (pContext, &segment.point[0], &segment.point[1]);
            }
        }
    return true;
    }
#ifdef CompileAll
/*---------------------------------------------------------------------------------**//**
* "Process" function for DSegment4d hatch block boundaries.
* conic and dispatch to more generic integrators.  (Still a couple levels away
* from evaluating the curve!!)
+---------------+---------------+---------------+---------------+---------------+------*/
static bool                    cbPrimitiveOffsetDConic4d
(
        PrimitiveOffsetContext      *pContext,
GraphicsPointArrayCP pSource,
        int                         index,
const   DConic4d                    *pConic
)
    {
    DEllipse3d ellipse0, ellipse1, ellipse2;
    DPoint3d startPoint, startTangent, startVector;
    DPoint3d endPoint;
    double zz, f, f1, f2;
    DPoint3d point10, point11, point20, point21;

    if (   bsiDEllipse3d_initFromDConic4d (&ellipse0, pConic)
        && bsiDEllipse3d_isCircularXY (&ellipse0))
        {
        ellipse0.vector0.z = ellipse0.vector90.z = 0.0;
        bsiDEllipse3d_fractionParameterToDerivatives (&ellipse0,
                    &startPoint, &startTangent, NULL, 0.0);
        bsiDEllipse3d_fractionParameterToDerivatives (&ellipse0,
                    &endPoint, NULL, NULL, 1.0);
        announcePrimitiveOffsetStartEnd (pContext, &startPoint, &endPoint);

        startVector.DifferenceOf (startPoint, ellipse0.center);
        zz = startVector.CrossProductXY (startTangent);
        if (zz < 0.0)
            bsiDEllipse3d_initReversed (&ellipse0, &ellipse0);

        if (bsiTrig_safeDivide
                            (&f,
                            pContext->distance,
                            startVector.Magnitude (),
                            0.0
                            ))
            {
            f = fabs (f);
            if (zz < 0.0)
                {
                f1 = 1.0 + f;
                f2 = 1.0 - f;
                }
            ellipse1 = ellipse0;
            bsiDPoint3d_scaleInPlace (&ellipse1.vector0, f1);
            bsiDPoint3d_scaleInPlace (&ellipse1.vector90, f1);
            bsiDEllipse3d_fractionParameterToDPoint3d (&ellipse1, &point10, 0.0);
            bsiDEllipse3d_fractionParameterToDPoint3d (&ellipse1, &point11, 1.0);
            if (f2 <= 0.0)
                {
                jmdlGraphicsPointArray_addDEllipse3d
                                (
                                pContext->pCollector,
                                &ellipse1
                                );
                jmdlGraphicsPointArray_markBreak (pContext->pCollector);
                jmdlGraphicsPointArray_addDPoint3d (pContext->pCollector, &point11);
                jmdlGraphicsPointArray_addDPoint3d (pContext->pCollector, &ellipse0.center);
                jmdlGraphicsPointArray_addDPoint3d (pContext->pCollector, &point10);
                jmdlGraphicsPointArray_markBreak (pContext->pCollector);
                jmdlGraphicsPointArray_markMajorBreak (pContext->pCollector);
                }
            else
                {
                bsiDEllipse3d_initReversed (&ellipse2, &ellipse0);
                bsiDPoint3d_scaleInPlace (&ellipse2.vector0, f2);
                bsiDPoint3d_scaleInPlace (&ellipse2.vector90, f2);
                bsiDEllipse3d_fractionParameterToDPoint3d (&ellipse2, &point20, 0.0);
                bsiDEllipse3d_fractionParameterToDPoint3d (&ellipse2, &point21, 1.0);

                jmdlGraphicsPointArray_addDEllipse3d
                                (
                                pContext->pCollector,
                                &ellipse1
                                );
                jmdlGraphicsPointArray_markBreak (pContext->pCollector);
                jmdlGraphicsPointArray_addDPoint3d (pContext->pCollector, &point11);
                jmdlGraphicsPointArray_addDPoint3d (pContext->pCollector, &point20);
                jmdlGraphicsPointArray_markBreak (pContext->pCollector);

                jmdlGraphicsPointArray_addDEllipse3d
                                (
                                pContext->pCollector,
                                &ellipse1
                                );
                jmdlGraphicsPointArray_markBreak (pContext->pCollector);
                jmdlGraphicsPointArray_addDPoint3d (pContext->pCollector, &point21);
                jmdlGraphicsPointArray_addDPoint3d (pContext->pCollector, &point10);
                jmdlGraphicsPointArray_markBreak (pContext->pCollector);
                jmdlGraphicsPointArray_markMajorBreak (pContext->pCollector);
                }
            }
        }
    return true;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* For each primitive in the array, generate a closed area by perpendicular
*   offset.   Add these to the collector.
* The "strips" of offset area are created one-by-one for the primitives -- incidence
*   with the following primitive is not considered.
* @param pCollector <= destination array.  This will contain one closed
*       area for each primitive.   The collector is NOT emptied.
* @param pSource => midline geoemtry.
* @param offsetDistance =>distance to offset.
* @param createCircles => if true, create a circle at each start and end point.
* @bsimethod                                                    EarlinLutz      12/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     jmdlGraphicsPointArray_collectOffsetPrimitiveAreasXY
(
        GraphicsPointArray      *pCollector,
GraphicsPointArrayCP pSource,
        double                      offsetDistance,
        bool                        createCircles
)
    {
    PrimitiveOffsetContext context;
    memset (&context, 0, sizeof (PrimitiveOffsetContext));
    context.distance = offsetDistance;
    context.createCircles = createCircles;
    context.pCollector = pCollector;
    context.tolerance = offsetDistance * 1.0e-8;
    jmdlGraphicsPointArray_processPrimitives (&context, pSource,
                    (GPAFunc_DSegment4d)cbPrimitiveOffsetDSegment4d,
                    NULL, NULL
                    //cbPrimitiveOffsetDConic4d,
                    //cbPrimitiveOffsetBezierDPoint4d
                    );
    }



END_BENTLEY_GEOMETRY_NAMESPACE
