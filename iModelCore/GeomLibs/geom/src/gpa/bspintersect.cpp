/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/gpa/bspintersect.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*---------------------------------------------------------------------------------**//**
*   BSpline Curve/Surface to GPA Conversion Routines
+---------------+---------------+---------------+---------------+---------------+------*/
#define MAX_POLE_BUFFER 1000

typedef void (*CBBezierSpanHandler)
    (
    DPoint4d    *pBezierPoles,
    int         order,
    double      s0,
    double      s1,
    int         m0,
    int         m1,
    void        *pUserData0,
    void        *pUserData1,
    void        *pUserData2,
    void        *pUserData3
    );
#ifdef CompileAll
/*---------------------------------------------------------------------------------**//**
* Return upper bound for #GP to be added for a B-spline curve.  Upper bound is
* realized for a curve with all interior knots of multiplicity one.
* @bsimethod                                                    DavidAssaf      05/02
+---------------+---------------+---------------+---------------+---------------+------*/
static int     numberBsplineCurveGraphicsPoints (BsplineParam const* pParam)
    {
    int order = pParam->order;
    int nIntKnots = bspknot_numberKnots (pParam->numPoles, order, pParam->closed) - 2 * order;

    // linears handled as GPA linestrings by addBsplineCurveSpecialCases
    if (order == 2)
        return nIntKnots + order;

    return (nIntKnots + 1) * order;
    }
#endif
/*---------------------------------------------------------------------------------**//**
* Load B-Spline curve into a graphics point array -- only handles easy
* special cases.
* @bsimethod                                                    RayBentley      06/01
* @bsimethod                                                    DavidAssaf      06/01
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus   addBsplineCurveSpecialCases (GraphicsPointArrayP pGPA, MSBsplineCurveCP pCurve)
    {
    GraphicsPoint   gp;
    int             order = pCurve->params.order;

    // linear B-spline curves are stored as linestrings
    if (order == 2)
        {
        if (pCurve->params.closed)
            {
            if (pCurve->params.numKnots == pCurve->params.numPoles - 1)
                {
                /* In addition to the indicated interior knot count,
                    the knot array is sandwiched by one start/end knot and one
                    wraparound knot at each end. */
                double      *pKnotBuffer    = pCurve->knots + order - 1;
                DPoint3d    *pPoleBuffer    = pCurve->poles;
                double      *pWeightBuffer  = pCurve->weights;
                int         i;
                for (i = 0; i < pCurve->params.numPoles; i++)
                    {
                    gp.point.x  = pPoleBuffer[i].x;
                    gp.point.y  = pPoleBuffer[i].y;
                    gp.point.z  = pPoleBuffer[i].z;
                    gp.point.w  = pCurve->rational ? pWeightBuffer[i] : 1.0;
                    gp.mask     = 0;
                    gp.userData = 0;
                    gp.a        = pKnotBuffer[i];
                    jmdlGraphicsPointArray_addGraphicsPoint (pGPA, &gp);
                    }

                /* last pole is first */
                gp.point.x  = pPoleBuffer[0].x;
                gp.point.y  = pPoleBuffer[0].y;
                gp.point.z  = pPoleBuffer[0].z;
                gp.point.w  = pCurve->rational ? pWeightBuffer[0] : 1.0;
                gp.mask     = 0;
                gp.userData = 0;
                gp.a        = pKnotBuffer[pCurve->params.numPoles];
                jmdlGraphicsPointArray_addGraphicsPoint (pGPA, &gp);

                jmdlGraphicsPointArray_markBreak (pGPA);
                return SUCCESS;
                }
            }
        else /* open */
            {
            if (pCurve->params.numKnots == pCurve->params.numPoles - order)
                {
                /* In addition to the indicated interior knot count,
                    the knot array contains double knots at start and end. */
                double      *pKnotBuffer    = pCurve->knots + order - 1;
                DPoint3d    *pPoleBuffer    = pCurve->poles;
                double      *pWeightBuffer  = pCurve->weights;
                int         i;
                for (i = 0; i < pCurve->params.numPoles; i++)
                    {
                    gp.point.x  = pPoleBuffer[i].x;
                    gp.point.y  = pPoleBuffer[i].y;
                    gp.point.z  = pPoleBuffer[i].z;
                    gp.point.w  = pCurve->rational ? pWeightBuffer[i] : 1.0;
                    gp.mask     = 0;
                    gp.userData = 0;
                    gp.a        = pKnotBuffer[i];
                    jmdlGraphicsPointArray_addGraphicsPoint (pGPA, &gp);
                    }

                jmdlGraphicsPointArray_markBreak (pGPA);
                return SUCCESS;
                }
            }
        }
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     07/00
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus   emitBezierSpans
(
MSBsplineCurveCP        pCurve,
CBBezierSpanHandler     cbBezierSpanHandler,
void                    *pUserData0,
void                    *pUserData1,
void                    *pUserData2,
void                    *pUserData3
)
    {
    ExtractContext          Context;
    DPoint4d                bezSegmentPoles[MAX_BEZIER_CURVE_ORDER];
    DPoint3d*               pPoles = pCurve->poles;
    double*                 pWeights = (pCurve->rational && pCurve->weights) ? pCurve->weights : NULL;
    double                  s0, s1;
    BentleyStatus           status = ERROR;
    int                     i, numPoles = pCurve->params.numPoles, order = pCurve->params.order;
    bool                    closed = (0 != pCurve->params.closed);
    int                     m0, m1;
    bvector <DPoint4d>      pBspHPoles(numPoles);

     for (i = 0; i < numPoles; i++)
        {
        pBspHPoles[i].x = pPoles[i].x;
        pBspHPoles[i].y = pPoles[i].y;
        pBspHPoles[i].z = pPoles[i].z;
        pBspHPoles[i].w = pWeights ? pWeights[i] : 1.0;
        }

    if (bsiBezierDPoint4d_extractNextBezierFromBsplineInit (&Context, &pBspHPoles.front(), numPoles, pCurve->knots,
                                                             bspknot_numberKnots (numPoles, order, closed),
                                                             RELATIVE_BSPLINE_KNOT_TOLERANCE, order, closed))
        {
        while (bsiBezierDPoint4d_extractNextBezierFromBsplineExt2 (bezSegmentPoles, &s0, &s1, &m0, &m1, &Context))
            {
            if (!pWeights)
                {
                // last bit errors can introduce non-unit weights.  Force them to one if known so...
                for (i = 0; i < order; i++)
                    bezSegmentPoles[i].w = 1.0;
                }            
            cbBezierSpanHandler (bezSegmentPoles, order, s0, s1, m0, m1, pUserData0, pUserData1, pUserData2, pUserData3);
            }

        bsiBezierDPoint4d_extractNextBezierFromBsplineEnd (&Context);
        status = SUCCESS;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     07/00
+---------------+---------------+---------------+---------------+---------------+------*/
static void    cbAddBezierToGPA
(
DPoint4d            *pPoles,
int                 order,
double              knot0,
double              knot1,
int                 mult0,
int                 mult1,
GraphicsPointArrayP pGPA,
MSBsplineCurveP     pCurve,
void                *pVoid2,
void                *pVoid3
)
    {
    double  s0, s1;
    int     index0, index1;

    jmdlGraphicsPointArray_addDPoint4dBezierWithIndices (pGPA, &index0, &index1, pPoles, order, 1, false);
    s0 = mdlBspline_naturalParameterToFractionParameter (pCurve, knot0);
    s1 = mdlBspline_naturalParameterToFractionParameter (pCurve, knot1);
    jmdlGraphicsPointArray_setParameter (pGPA, index0, s0);
    jmdlGraphicsPointArray_setParameter (pGPA, index1, s1);
    jmdlGraphicsPointArray_setUserData (pGPA, index0, mult0);
    jmdlGraphicsPointArray_setUserData (pGPA, index1, mult1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2007
+---------------+---------------+---------------+---------------+---------------+------*/
GEOMDLLIMPEXP void jmdlGraphicsPointArray_addBsplineCurve (GraphicsPointArray *pGPA, MSBsplineCurveCP pCurve)
    {

    if (SUCCESS == addBsplineCurveSpecialCases (pGPA, pCurve))
        return;

    emitBezierSpans (pCurve, (CBBezierSpanHandler) cbAddBezierToGPA, pGPA, (void *) pCurve, NULL, NULL);
    }




/*---------------------------------------------------------------------------------**//**
* @description Return multiple intersections between two B-spline curves.
* @remarks While intersections are computed to machine precision, close approaches within
*       the given tolerance are also returned (to allow for small endpoint mismatches and
*       near-tangencies).
* @remarks The "a" field of each graphics point stores the <EM>global B-spline parameter</EM> of
*       the point (not the local parameter within the containing Bezier segment).  The "userData" field
*       stores the GPA index of the start of the containing Bezier segment.
* @param pIntersects0           OUT     intersection points with global parametrization on pCurve0 (or NULL)
* @param pIntersects1           OUT     intersection points with global parametrization on pCurve1 (or NULL)
* @param pCurve0                IN      first curve
* @param pCurve1                IN      second curve
* @param pWorldToLocalMatrix    IN      rotation by which to multiply geometry before xy-intersections are computed (or NULL for 3D intersections)
* @param distanceTolerance      IN      maximum absolute distance from either curve to a near-intersection point
* @param parametricTolerance    IN      maximum parametric distance between equal intersections (used in culling duplicate intersections)
* @return The number of intersections found (and returned in arrays, if given).
* @see intersection_allBetweenChainsExt
* @bsimethod                                                    DavidAssaf      08/04
+---------------+---------------+---------------+---------------+---------------+------*/
GEOMDLLIMPEXP int   jmdlGraphicsPointArray_intersectAllBetweenCurves
(
GraphicsPointArray* pIntersects0,
GraphicsPointArray* pIntersects1,
MSBsplineCurve const*       pCurve0,
MSBsplineCurve const*       pCurve1,
RotMatrix const*            pWorldToLocalMatrix,
double                      distanceTolerance,
double                      parametricTolerance
)
    {
    GraphicsPoint   gp;
    int             i, numOut = 0;

    if (!pCurve0 || !pCurve1)
        return 0;

    GraphicsPointArray* pGPA0 = jmdlGraphicsPointArray_grab();
    GraphicsPointArray* pGPA1 = jmdlGraphicsPointArray_grab();
    GraphicsPointArray* pGPA0X = pIntersects0 ? pIntersects0 : jmdlGraphicsPointArray_grab();
    GraphicsPointArray* pGPA1X = pIntersects1 ? pIntersects1 : jmdlGraphicsPointArray_grab();
    GraphicsPointArray* pGPA0World = NULL;
    GraphicsPointArray* pGPA1World = NULL;
    bool                        bApplyTransform = (pWorldToLocalMatrix && !bsiRotMatrix_isIdentity (pWorldToLocalMatrix));
    int                         workDim = pWorldToLocalMatrix ? 2 : 3;

    if (!pGPA0 || !pGPA1 || !pGPA0X || !pGPA1X)
        goto wrapup;

    jmdlGraphicsPointArray_empty (pGPA0X);
    jmdlGraphicsPointArray_empty (pGPA1X);

    jmdlGraphicsPointArray_addBsplineCurve (pGPA0, pCurve0);
    jmdlGraphicsPointArray_addBsplineCurve (pGPA1, pCurve1);

    // transform to plane but save copy of original GPAs
    if (bApplyTransform && (pIntersects0 || pIntersects1))
        {
        Transform   transform;

        pGPA0World = jmdlGraphicsPointArray_grab();
        pGPA1World = jmdlGraphicsPointArray_grab();
        if (!pGPA0World || !pGPA1World)
            goto wrapup;

        if (!jmdlGraphicsPointArray_appendArray (pGPA0World, pGPA0) ||
            !jmdlGraphicsPointArray_appendArray (pGPA1World, pGPA1))
            goto wrapup;

        bsiTransform_initFromMatrix (&transform, pWorldToLocalMatrix);

        jmdlGraphicsPointArray_multiplyByTransform (pGPA0, &transform);
        jmdlGraphicsPointArray_multiplyByTransform (pGPA1, &transform);
        }

    jmdlGraphicsPointArray_collectProjectedApproachPoints (pGPA0X, pGPA1X, pGPA0, pGPA1, false, distanceTolerance, workDim);

    if (!jmdlGraphicsPointArray_cullDuplicatePoints (pGPA0X, pGPA1X, parametricTolerance, -1.0))
        {
        jmdlGraphicsPointArray_empty (pGPA0X);
        jmdlGraphicsPointArray_empty (pGPA1X);
        goto wrapup;
        }

    numOut = jmdlGraphicsPointArray_getCount (pGPA0X);
    if (numOut <= 0 || (!pIntersects0 && !pIntersects1))
        goto wrapup;

    // re-evaluate intersections in terms of world coordinates
    if (bApplyTransform)
        {
        for (i = 0; i < numOut; i++)
            {
            jmdlGraphicsPointArray_getGraphicsPoint (pGPA0X, &gp, i);
            jmdlGraphicsPointArray_primitiveFractionToDPoint4d (pGPA0World, &gp.point, gp.userData, gp.a);
            jmdlGraphicsPointArray_setGraphicsPoint (pGPA0X, &gp, i);

            jmdlGraphicsPointArray_getGraphicsPoint (pGPA1X, &gp, i);
            jmdlGraphicsPointArray_primitiveFractionToDPoint4d (pGPA1World, &gp.point, gp.userData, gp.a);
            jmdlGraphicsPointArray_setGraphicsPoint (pGPA1X, &gp, i);
            }
        }

    // sort by Bezier index (userData), then Bezier parameter (a)
    jmdlGraphicsPointArray_sortParallelGPAs (pGPA0X, pGPA1X);

    // convert the local Bezier parameters of the sorted graphics points into global B-spline parameters
    for (i = 0; i < numOut; i++)
        {
        jmdlGraphicsPointArray_getGraphicsPoint (pGPA0X, &gp, i);
        jmdlGraphicsPointArray_primitiveFractionToApplicationParameter (pGPA0, &gp.a, gp.userData, gp.a);
        jmdlGraphicsPointArray_setGraphicsPoint (pGPA0X, &gp, i);

        jmdlGraphicsPointArray_getGraphicsPoint (pGPA1X, &gp, i);
        jmdlGraphicsPointArray_primitiveFractionToApplicationParameter (pGPA1, &gp.a, gp.userData, gp.a);
        jmdlGraphicsPointArray_setGraphicsPoint (pGPA1X, &gp, i);
        }

wrapup:
    jmdlGraphicsPointArray_drop (pGPA1World);
    jmdlGraphicsPointArray_drop (pGPA0World);
    if (!pIntersects1) jmdlGraphicsPointArray_drop (pGPA1X);
    if (!pIntersects0) jmdlGraphicsPointArray_drop (pGPA0X);
    jmdlGraphicsPointArray_drop (pGPA1);
    jmdlGraphicsPointArray_drop (pGPA0);
    return numOut;
    }

/*---------------------------------------------------------------------------------**//**
* @description Return multiple intersections between two B-spline curves.
* @remarks While intersections are computed to machine precision, close approaches within
*       the given tolerance are also returned (to allow for small endpoint mismatches and
*       near-tangencies).
* @remarks Output buffers allocated by this function must be freed by ~mBSIBaseGeom::Free (or ~mmemutil_free or ~mdlmSystem_mdlFree).
* @param ppIntersects0          OUT     intersection points on pCurve0 (or NULL)
* @param ppIntersects1          OUT     intersection points on pCurve1 (or NULL)
* @param ppParams0              OUT     intersection parameters on pCurve0 (or NULL)
* @param ppParams1              OUT     intersection parameters on pCurve1 (or NULL)
* @param pNumIntersects         OUT     the number of returned points/params
* @param pCurve0                IN      first curve
* @param pCurve1                IN      second curve
* @param pWorldToLocalMatrix    IN      rotation by which to multiply geometry before xy-intersections are computed (or NULL for 3D intersections)
* @param distanceTolerance      IN      maximum absolute distance from either curve to a near-intersection point
* @param parametricTolerance    IN      maximum parametric distance between equal intersections (used in culling duplicate intersections)
* @return SUCCESS unless invalid input or buffer allocations failed.
* @bsimethod                                                    DavidAssaf      08/04
+---------------+---------------+---------------+---------------+---------------+------*/
GEOMDLLIMPEXP StatusInt     bspcurv_intersectAllBetweenCurves
(
DPoint3d**              ppIntersects0,
DPoint3d**              ppIntersects1,
double**                ppParams0,
double**                ppParams1,
int*                    pNumIntersects,
MSBsplineCurve const*   pCurve0,
MSBsplineCurve const*   pCurve1,
RotMatrix const*        pWorldToLocalMatrix,
double                  distanceTolerance,
double                  parametricTolerance
)
    {
    StatusInt   status = ERROR;
    int         numIntersects;

    if (!pCurve0 || !pCurve1 || !pNumIntersects)
        return ERROR;

    if (ppIntersects0)
        *ppIntersects0 = NULL;
    if (ppIntersects1)
        *ppIntersects1 = NULL;
    if (ppParams0)
        *ppParams0 = NULL;
    if (ppParams1)
        *ppParams1 = NULL;

    GraphicsPointArray* pIntersects0 = jmdlGraphicsPointArray_grab();
    GraphicsPointArray* pIntersects1 = jmdlGraphicsPointArray_grab();

    numIntersects = jmdlGraphicsPointArray_intersectAllBetweenCurves (pIntersects0, pIntersects1, pCurve0, pCurve1, pWorldToLocalMatrix, distanceTolerance,
                                                      parametricTolerance);

    // allocate buffers
    if (numIntersects > 0)
        {
        int i, bufSize, numGot;

        if (ppIntersects0)
            {
            bufSize = numIntersects * sizeof (DPoint3d);
            if (!(*ppIntersects0 = static_cast <DPoint3d*> (BSIBaseGeom::Malloc (bufSize))))
                goto wrapup;
            if (!jmdlGraphicsPointArray_getDPoint3dArray (pIntersects0, *ppIntersects0, &numGot, 0, numIntersects) || numGot != numIntersects)
                goto wrapup;
            }
        if (ppIntersects1)
            {
            bufSize = numIntersects * sizeof (DPoint3d);
            if (!(*ppIntersects1 = static_cast <DPoint3d*> (BSIBaseGeom::Malloc (bufSize))))
                goto wrapup;
            if (!jmdlGraphicsPointArray_getDPoint3dArray (pIntersects1, *ppIntersects1, &numGot, 0, numIntersects) || numGot != numIntersects)
                goto wrapup;
            }
        if (ppParams0)
            {
            bufSize = numIntersects * sizeof (double);
            if (!(*ppParams0 = static_cast <double*> (BSIBaseGeom::Malloc (bufSize))))
                goto wrapup;
            for (i = 0; i < numIntersects; i++)
                jmdlGraphicsPointArray_getComplete (pIntersects0, NULL, *ppParams0 + i, NULL, NULL, i);
            }
        if (ppParams1)
            {
            bufSize = numIntersects * sizeof (double);
            if (!(*ppParams1 = static_cast <double*> (BSIBaseGeom::Malloc (bufSize))))
                goto wrapup;
            for (i = 0; i < numIntersects; i++)
                jmdlGraphicsPointArray_getComplete (pIntersects1, NULL, *ppParams1 + i, NULL, NULL, i);
            }
        }

    *pNumIntersects = numIntersects;

    status = SUCCESS;

wrapup:
    // if some allocations failed, clean up allocations that succeeded
    if (SUCCESS != status)
        {
        if (ppParams1 && *ppParams1)
            {
            BSIBaseGeom::Free (*ppParams1);
            *ppParams1 = NULL;
            }
        if (ppParams0 && *ppParams0)
            {
            BSIBaseGeom::Free (*ppParams0);
            *ppParams0 = NULL;
            }
        if (ppIntersects1 && *ppIntersects1)
            {
            BSIBaseGeom::Free (*ppIntersects1);
            *ppIntersects1 = NULL;
            }
        if (ppIntersects0 && *ppIntersects0)
            {
            BSIBaseGeom::Free (*ppIntersects0);
            *ppIntersects0 = NULL;
            }
        }

    jmdlGraphicsPointArray_drop (pIntersects1);
    jmdlGraphicsPointArray_drop (pIntersects0);
    return status;
    }

END_BENTLEY_GEOMETRY_NAMESPACE