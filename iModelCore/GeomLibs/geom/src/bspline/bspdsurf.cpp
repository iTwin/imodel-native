/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/bspline/bspdsurf.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/MstnOnly/BspPrivateApi.h>
#include "msbsplinemaster.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#define BOUNDARY_JUMP_TOLERANCE 0.90
#ifdef compile_in_bspdsurf
/*----------------------------------------------------------------------+
|                                                                       |
| Local type definition                                                 |
|                                                                       |
+----------------------------------------------------------------------*/
typedef struct filletargs
    {
    double      deltaRad;
    double      radius0;
    double      radius1;
    } FilletArgs;

/*----------------------------------------------------------------------+
|                                                                       |
|    Derived Surface Routines                                           |
|                                                                       |
+----------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             04/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspsurf_leastSquaresToSurface
(
MSBsplineSurface    *surface,
double              *avgDistance,
double              *maxDistance,
DPoint3d            *points,
DPoint2d            *uvValues,
int                 *uNumPoints,
int                 vNumPoints
)
    {
    int             i, j, status = ERROR, allocSize;
    double          *values;
    DPoint2d        *vPtr;
    DPoint3d        *tmpMesh, *pPtr;
    MSBsplineCurve  curve;

    tmpMesh = NULL;     values = NULL;
    memset (&curve, 0, sizeof(MSBsplineCurve));

    /* Check ok params for number of data points */
    if (surface->uParams.numPoles < surface->uParams.order ||
        surface->vParams.numPoles < surface->vParams.order)
        return ERROR;

    if (vNumPoints < surface->vParams.numPoles)
        return ERROR;
    for (i=0; i < vNumPoints; i++)
        if (uNumPoints[i] < surface->uParams.numPoles)
            return ERROR;

    if (surface->uParams.numKnots && !surface->uKnots ||
        surface->vParams.numKnots && !surface->vKnots)
        return ERROR;

    /* Allocate memory */
    allocSize = surface->uParams.numPoles * surface->vParams.numPoles * sizeof (DPoint3d);
    if (NULL == (surface->poles = (DPoint3d*)BSIBaseGeom::Malloc (allocSize)))
        return ERROR;

    allocSize = surface->uParams.numPoles * vNumPoints * sizeof (DPoint3d);
    if (NULL == (tmpMesh = (DPoint3d*)BSIBaseGeom::Malloc (allocSize)))
        {
        status = ERROR;
        goto wrapup;
        }

    /* If uvValues are passed in, malloc something so that values can be realloc'ed below */
    if (uvValues && NULL == (values = (double*)BSIBaseGeom::Malloc (1)))
        {
        status = ERROR;
        goto wrapup;
        }

    /* Calculate intermediate poles for each curve in v */
    curve.params = surface->uParams;
    if (surface->uKnots)
        curve.knots = surface->uKnots;

    for (i=0, pPtr=points, vPtr = uvValues ; i < vNumPoints; i++)
        {
        if (uvValues)
            {
            if (NULL == (values = (double*)BSIBaseGeom::Realloc (values, uNumPoints[i] * sizeof (double))))
                {
                status = ERROR;
                goto wrapup;
                }
            for (j=0; j < uNumPoints[i]; j++)
                values[j] = vPtr[j].x;
            }
        if (SUCCESS != (status = bspcurv_leastSquaresToCurve (&curve, NULL, NULL,
                                                              pPtr, values, uNumPoints[i])))
            goto wrapup;

#if defined (debug_lsqt)
        {
        ElementDescr    *u;

        curve.display.curveDisplay = true;
        bspcurv_createCurve (&u, NULL, &curve);
        mdlElmdscr_display (u, 0, 0);
        }
#endif
        /* copy intermediate poles into tmpMesh */
        for (j=0; j < surface->uParams.numPoles; j++)
             tmpMesh[j*vNumPoints + i] = curve.poles[j];

        BSIBaseGeom::Free (curve.poles); curve.poles = NULL;

        /* Not freeing curve.knots this will assure that all rows use the same knot vector */

        pPtr += uNumPoints[i];
        vPtr += uNumPoints[i];
        }

    surface->uKnots = curve.knots;
    if (curve.knots)
        curve.knots = NULL;

    /* Calculate actual poles from intermediate poles */
    curve.params = surface->vParams;
    if (surface->vKnots)
        curve.knots = surface->vKnots;

    if (uvValues)
        {
        if (NULL == (values = (double*)BSIBaseGeom::Realloc (values, vNumPoints * sizeof (double))))
            {
            status = ERROR;
            goto wrapup;
            }
        for (i=0, vPtr=uvValues; i < vNumPoints; vPtr += uNumPoints[i++])
            values[i] = vPtr->y;
        }

    for (i=0, pPtr=tmpMesh; i < surface->uParams.numPoles;
         i++, pPtr += vNumPoints)
        {
        if (SUCCESS != (status = bspcurv_leastSquaresToCurve (&curve, NULL, NULL,
                                                              pPtr, values, vNumPoints)))
            goto wrapup;

        for (j=0; j < surface->vParams.numPoles; j++)
            surface->poles[j*surface->uParams.numPoles + i] = curve.poles[j];

        BSIBaseGeom::Free (curve.poles); curve.poles = NULL;
        }

    surface->vKnots = curve.knots;
    curve.knots = NULL;

    /* Return B-Spline surface */
    surface->type = BSSURF_GENERAL;
    surface->rational = false;
    surface->display.curveDisplay = true;

    surface->uParams.numRules = surface->vParams.numPoles;
    surface->vParams.numRules = surface->uParams.numPoles;

    surface->weights = NULL;
    surface->numBounds = 0;
    surface->boundaries = NULL;

    wrapup:
        if (tmpMesh)            BSIBaseGeom::Free(tmpMesh);
        if (values)             BSIBaseGeom::Free(values);
        bspcurv_freeCurve (&curve);
        return status;
    }
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             03/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspsurf_surfaceOfRevolution
(
MSBsplineSurface    *surface,       /* <= Surface of revolution */
MSBsplineCurveCP    curve,         /* => Boundary curve */
DPoint3dCP          center,        /* => Center axis point */
DPoint3dCP          axis,          /* => Axis direction vector */
double              start,          /* => Start angle */
double              sweep           /* => Sweep angle */
)
    {
    int         i, j, status, index, numArcPoles;
    double      arcWeights[MAX_ARCPOLES], arcKnots[MAX_ARCKNOTS], radius, arcStart;
    DPoint3d    curvePole, arcPoles[MAX_ARCPOLES];
    RotMatrix   rotMatrix, tempMatrix;

    /* Clear the surface structure */
    memset (surface, 0, sizeof(*surface));

    /* Calculate the number of knots and poles required for
        the sweep arc here so the surface data size can be calculated */

    numArcPoles = 3;
    if (fabs (sweep) > msGeomConst_2pi /fc_3)
        {
        numArcPoles += 2;
        if (fabs (sweep) > 2.0* msGeomConst_2pi /fc_3)
            numArcPoles += 2;
        }
    surface->vParams.order = 3;
    surface->vParams.numPoles = numArcPoles;

    surface->vParams.closed = Angle::IsFullCircle (sweep) ? 1 : 0;
    if (surface->vParams.closed)
        surface->vParams.numKnots = MAX_ARCKNOTS;
    else
        surface->vParams.numKnots = numArcPoles - 3;

    surface->uParams = curve->params;
    surface->rational = true;

    /* Allocate arrays for surface poles, weights, and knots */
    if (SUCCESS != (status = bspsurf_allocateSurface (surface)))
        return status;

    rotMatrix_orthogonalFromZRow (&rotMatrix, (DVec3d const*)axis);
    tempMatrix = rotMatrix;
    rotMatrix.TransposeOf (tempMatrix);
    for (i=0; i < curve->params.numPoles; i++)
        {
        if (curve->rational)
            curvePole.Scale (*(curve->poles+i), 1.0/curve->weights[i]);
        else
            curvePole = curve->poles[i];

        curvePole.DifferenceOf (curvePole, *center);
        rotMatrix.MultiplyTranspose (curvePole);
        radius = sqrt (curvePole.x*curvePole.x + curvePole.y*curvePole.y);
        arcStart = start + Angle::Atan2 (curvePole.y, curvePole.x);
        bspconv_computeCurveFromArc (arcPoles, arcKnots, arcWeights, &surface->vParams,
                                     arcStart, sweep, radius, radius);

        for (j=0; j < numArcPoles; j++)
            {
            index = j*curve->params.numPoles + i;
            surface->poles[index] = arcPoles[j];
            surface->poles[index].z = curvePole.z;
            if (curve->rational)
                surface->weights[index] = curve->weights[i]*arcWeights[j];
            else
                surface->weights[index] = arcWeights[j];
            rotMatrix.Multiply (*(surface->poles+index));
            (surface->poles+index)->SumOf (*(surface->poles+index), *center);

            (surface->poles+index)->Scale (*(surface->poles+index), surface->weights[index]);
            }
        }

    /* Set the knot vectors */
    memcpy (surface->uKnots, curve->knots,
            bspknot_numberKnots (curve->params.numPoles, curve->params.order,
                                 curve->params.closed) * sizeof (double));
    bspknot_computeKnotVector (surface->vKnots, &surface->vParams, arcKnots);

    /* Set the number of rule lines equal to poles in opposite direction */
    surface->type = BSSURF_REVOLUTION;
    surface->display.curveDisplay = true;
    surface->uParams.numRules = surface->vParams.numPoles;
    surface->vParams.numRules = surface->uParams.numPoles;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             03/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspsurf_surfaceOfProjection
(
MSBsplineSurface    *surface,
MSBsplineCurveCP     curve,
DPoint3dCP          delta
)
    {
    int         status, uPoles;

    /* Clear the entire structure so only the nonZero values need be set */
    memset (surface, 0, sizeof(*surface));

    surface->uParams = curve->params;
    surface->vParams.order = 2;
    surface->vParams.numPoles = 2;
    surface->rational = curve->rational;

    if (SUCCESS != (status = bspsurf_allocateSurface (surface)))
        goto wrapup;

    uPoles = surface->uParams.numPoles;
    for (int i=0; i < curve->params.numPoles; i++)
        {
        surface->poles[i] = curve->poles[i];
        if (surface->rational)
            {
            surface->poles[i + uPoles].SumOf (curve->poles[i], *delta, curve->weights[i]);
            surface->weights[i]=surface->weights[uPoles+i] = curve->weights[i];
            }
        else
            {
            surface->poles[i + uPoles].SumOf (curve->poles[i], *delta);
            }
        }

    memcpy (surface->uKnots, curve->knots,
            bspknot_numberKnots (uPoles, surface->uParams.order,
                                 surface->uParams.closed) * sizeof(double));
    surface->vKnots[0] = surface->vKnots[1] = 0.0;
    surface->vKnots[2] = surface->vKnots[3] = 1.0;

    /* Set the number of rule lines equal to poles in opposite direction */
    surface->type = BSSURF_TAB_CYLINDER;
    surface->display.curveDisplay = true;

    surface->uParams.numRules = surface->vParams.numPoles;
    surface->vParams.numRules = surface->uParams.numPoles;
wrapup:
    return status;
    }
#ifdef compile_in_bspdsurf
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             03/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int     bspsurf_orderCoonsCurves
(
MSBsplineCurve  *curves,
int             numCv
)
    {
    int             i, j, index, reverse;
    double          d, len, min;
    DPoint3d        startPnt, endPnt, start, end;
    MSBsplineCurve  *cv;
// EDL 1-16-2012 initialize reverse=false on each curve i (because compiler complained)
//  Is this right? Maybe the reverseal is supposed to be modal for later curves?
    /* Coon's patch is always open in both directions */
    for (i=0, cv=curves; i < numCv; i++, cv++)
        bspcurv_openCurve (cv, cv, 0.0);

    // bubble sort curves head to tail after first
    for (i=1; i<numCv; i++)
        {
        bspcurv_extractEndPoints (&startPnt, &endPnt, &curves[i-1]);
        len = startPnt.Distance (endPnt);
        min = fc_hugeVal;
        reverse = false;
        index = i;
        for (j=i; j<numCv; j++)
            {
            bspcurv_extractEndPoints (&start, &end, &curves[j]);
            if ((d = endPnt.Distance (start)) < min)
                {
                min = d;
                reverse = false;
                index = j;
                }
            if ((d = endPnt.Distance (end)) < min)
                {
                min = d;
                reverse = true;
                index = j;
                }

            // TR #54588: trivial curves at the end of a nontrivial comparison curve should come next
            if (len > fc_nearZero && d <= fc_nearZero && start.Distance (end) <= fc_nearZero)
                {
                min = d;
                reverse = false;
                index = j;
                }
            }

        if (min > fc_5)
            return (ERROR);

        bsputil_swap (curves+i, curves+index);
        if (reverse)
            bspcurv_reverseCurve (curves+i, curves+i);
        }

    bspcurv_extractEndPoints (&start, NULL, &curves[0]);
    bspcurv_extractEndPoints (NULL, &end, &curves[numCv-1]);
    if (start.Distance (end) > fc_5)
        return (ERROR);

    /* They are now ordered head to tail, reorder to coons patch order */
    if (numCv == 4)
        {
        bsputil_swap (curves+1, curves+2);
        bsputil_swap (curves+2, curves+3);
        bspcurv_reverseCurve (curves+1, curves+1);
        bspcurv_reverseCurve (curves+2, curves+2);
        }

    /* Set corner weights all to one */
    for (i=0, cv=curves; i < numCv; i++, cv++)
        {
        if (cv->rational)
            {
            if (! MSBsplineCurve::AreSameWeights (cv->weights[0], 1.0))
                {
                cv->poles->Scale (*(cv->poles), 1.0/cv->weights[0]);
                cv->weights[0] = 1.0;
                }
            if (! MSBsplineCurve::AreSameWeights (cv->weights[(index=cv->params.numPoles-1)], 1.0))
                {
                (cv->poles+index)->Scale (*(cv->poles+index), 1.0/cv->weights[index]);
                cv->weights[index] = 1.0;
                }
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             03/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspsurf_coonsPatch
(
MSBsplineSurface    *surface,          /* B-spline Coons patch surface */
MSBsplineCurve      *curves            /* bounding B-spline curve */
)
    {
    int             i, j, k, l, m, n, index, status, rationalSurface;
    double          pKnotTolerance, qKnotTolerance, pNode, pNodeM1, qNode, qNodeM1,
                    pWts0i, pWts1i, qWts0j, qWts1j, *p0Nodes, *q0Nodes, *weights;
    DPoint3d        pPoles0i, pPoles1i, qPoles0j, qPoles1j,
                    corner00, corner01, corner10, corner11, *controlNet;
    MSBsplineCurve  *p, *q;

    /* Clear the surface structure */
    memset (surface, 0, sizeof(MSBsplineSurface));
    p0Nodes = q0Nodes = weights = NULL;     controlNet = NULL;

    if (SUCCESS != (status = bspsurf_orderCoonsCurves (curves, 4)))
        return status;
    p = curves;
    q = curves + 2;

    /* Check for cases that can be done as ruled surfaces */
    if (p[0].params.numPoles == 2 && p[1].params.numPoles == 2)
        {
        MSBsplineCurve  curve1, curve2;

        memset (&curve1, 0, sizeof(curve1));
        memset (&curve2, 0, sizeof(curve2));

        status =
            bspcurv_copyCurve (&curve1, &q[0]) ||
            bspcurv_copyCurve (&curve2, &q[1]) ||
            bspcurv_make2CurvesCompatible (&curve1, &curve2) ||
            bspsurf_ruledSurface (surface, &curve1, &curve2);

        bspcurv_freeCurve (&curve1);
        bspcurv_freeCurve (&curve2);
        return status;
        }
    else if (q[0].params.numPoles == 2 && q[1].params.numPoles == 2)
        {
        MSBsplineCurve  curve1, curve2;

        memset (&curve1, 0, sizeof(curve1));
        memset (&curve2, 0, sizeof(curve2));

        status =
            bspcurv_copyCurve (&curve1, &p[0]) ||
            bspcurv_copyCurve (&curve2, &p[1]) ||
            bspcurv_make2CurvesCompatible (&curve1, &curve2) ||
            bspsurf_ruledSurface (surface, &curve1, &curve2);

        bspcurv_freeCurve (&curve1);
        bspcurv_freeCurve (&curve2);
        return status;
        }

#if defined (NEEDS_WORK)
    /* What about cusps */
#endif

    if (SUCCESS != (status = bspcurv_make2CurvesCompatible (&p[0], &p[1])) ||
        SUCCESS != (status = bspcurv_make2CurvesCompatible (&q[0], &q[1])))
        return status;

    /* Initialize internal variables */
    k = p[0].params.order;
    l = q[0].params.order;
    n = p[0].params.numPoles;
    m = q[0].params.numPoles;


    i = bspknot_numberKnots (p[0].params.numPoles, p[0].params.order, false);
    j = bspknot_numberKnots (q[0].params.numPoles, q[0].params.order, false);
    if (NULL == (p0Nodes = (double*)msbspline_malloc (i*sizeof(double), HEAPSIG_BSRF)) ||
        NULL == (q0Nodes = (double*)msbspline_malloc (j*sizeof(double), HEAPSIG_BSRF)))
        {
        status = ERROR;
        goto wrapup;
        }


    /***** find u,v knot_tol ****/
    pKnotTolerance = bspknot_knotTolerance (&p[0]);
    qKnotTolerance = bspknot_knotTolerance (&q[0]);

    bspknot_computeGrevilleAbscissa (p0Nodes, p[0].knots, p[0].params.numPoles,
                                     p[0].params.order, p[0].params.closed, pKnotTolerance);
    bspknot_computeGrevilleAbscissa (q0Nodes, q[0].knots, q[0].params.numPoles,
                                     q[0].params.order, q[0].params.closed, qKnotTolerance);

    /* Allocate weights if either curve is rational */
    if (true == (rationalSurface = p[0].rational || q[0].rational))
        {
        if (NULL == (weights = (double*)BSIBaseGeom::Malloc (m*n*sizeof(double))))
            {
            status = ERROR;
            goto wrapup;
            }
        }

    /* Calculate interior control net and weights if rational */
    if (NULL == (controlNet = (DPoint3d*)BSIBaseGeom::Malloc (n*m*sizeof(DPoint3d))))
        {
        status = ERROR;
        goto wrapup;
        }

    /* Assign corners by averaging end poles of p's & q's */
    if (p[0].rational)
        {
        corner00.Scale (*(&p[0].poles[0]), 1.0/p[0].weights[0]);
        corner10.Scale (*(&p[1].poles[0]), 1.0/p[1].weights[0]);
        corner01.Scale (*(&p[0].poles[n-1]), 1.0/p[0].weights[n-1]);
        corner11.Scale (*(&p[1].poles[n-1]), 1.0/p[1].weights[n-1]);
        }
    else if (q[0].rational)
        {
        corner00.Scale (*(&q[0].poles[0]), 1.0/q[0].weights[0]);
        corner10.Scale (*(&q[0].poles[m-1]), 1.0/q[0].weights[m-1]);
        corner01.Scale (*(&q[1].poles[0]), 1.0/q[1].weights[0]);
        corner11.Scale (*(&q[1].poles[m-1]), 1.0/q[1].weights[m-1]);
        }
    else
        {
        corner00 = p[0].poles[0];       corner10 = p[1].poles[0];
        corner01 = p[0].poles[n-1];     corner11 = p[1].poles[n-1];
        }

    for (i=0; i < n; i++)
        {
        pNode = p0Nodes[i];
        pNodeM1 = 1.0 - pNode;
        pWts0i = p[0].rational ? p[0].weights[i] : 1.0;
        pWts1i = p[1].rational ? p[1].weights[i] : 1.0;
        pPoles0i.Scale (*(&p[0].poles[i]), 1.0/pWts0i);
        pPoles1i.Scale (*(&p[1].poles[i]), 1.0/pWts1i);
        for (j=0; j < m; j++)
            {
            qNode = q0Nodes[j];
            qNodeM1 = 1.0 - qNode;
            qWts0j = q[0].rational ? q[0].weights[j] : 1.0;
            qWts1j = q[1].rational ? q[1].weights[j] : 1.0;
            qPoles0j.Scale (*(&q[0].poles[j]), 1.0/qWts0j);
            qPoles1j.Scale (*(&q[1].poles[j]), 1.0/qWts1j);
            index = i*m + j;
            controlNet[index].x = qNodeM1*pPoles0i.x + qNode*pPoles1i.x
                                + pNodeM1*qPoles0j.x + pNode*qPoles1j.x
                                - pNodeM1*qNodeM1 * corner00.x
                                - pNodeM1*qNode   * corner10.x
                                - qNodeM1*pNode   * corner01.x
                                - pNode*qNode     * corner11.x;

            controlNet[index].y = qNodeM1*pPoles0i.y + qNode*pPoles1i.y
                                + pNodeM1*qPoles0j.y + pNode*qPoles1j.y
                                - pNodeM1*qNodeM1 * corner00.y
                                - pNodeM1*qNode   * corner10.y
                                - qNodeM1*pNode   * corner01.y
                                - pNode*qNode     * corner11.y;

            controlNet[index].z = qNodeM1*pPoles0i.z + qNode*pPoles1i.z
                                + pNodeM1*qPoles0j.z + pNode*qPoles1j.z
                                - pNodeM1*qNodeM1 * corner00.z
                                - pNodeM1*qNode   * corner10.z
                                - qNodeM1*pNode   * corner01.z
                                - pNode*qNode     * corner11.z;

            if (rationalSurface)
                weights[index]  = (qNodeM1*pWts0i + qNode*pWts1i)
                                * (pNodeM1*qWts0j + pNode*qWts1j);
            }
        }

    /* Set the surface parameters and allocate the surface */
    surface->uParams = q[0].params;
    surface->vParams = p[0].params;

    /* Set number or interior knots (always nonuniform) */
    surface->uParams.numKnots = m - l;
    surface->vParams.numKnots = n - k;

    /* The coons surface is never closed */
    surface->uParams.closed = surface->vParams.closed = false;
    surface->rational = rationalSurface;
    surface->display.curveDisplay = true;

    /* Set the number of rule lines to the number of poles in the opposition
        direction */
    surface->uParams.numRules = surface->vParams.numPoles;
    surface->vParams.numRules = surface->uParams.numPoles;

    if (SUCCESS != (status = bspsurf_allocateSurface (surface)))
        goto wrapup;

    /* Load knot vectors */
    memcpy (surface->vKnots, p[0].knots, (n+k) * sizeof(double));
    memcpy (surface->uKnots, q[0].knots, (m+l) * sizeof(double));

    if (rationalSurface)
        {
        memcpy (surface->weights, weights, n*m*sizeof(double));
        bsputil_weightPoles (surface->poles, controlNet, weights, n*m);
        }
    else
        memcpy (surface->poles, controlNet, n*m*sizeof(DPoint3d));

wrapup:
    if (p0Nodes)        msbspline_free (p0Nodes);
    if (q0Nodes)        msbspline_free (q0Nodes);
    if (controlNet)     BSIBaseGeom::Free (controlNet);
    if (weights)        BSIBaseGeom::Free (weights);
    return status;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          03/92
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bspsurf_cubicHermitBlends
(
double          *h0,
double          *h1,
double          *h2,
double          *h3,
double          u
)
    {
    double      uSquared = u*u, uTwo = u*2.0;

    *h0 = uSquared * (uTwo - fc_3) + 1.0;
    *h1 = u * (uSquared - uTwo + 1.0);
    *h2 = uSquared * (u - 1.0);
    *h3 = uSquared * (fc_3 - uTwo);
    }

/*---------------------------------------------------------------------------------**//**
* (1,0)---q[1]---(1,1) | | (u) p[0] p[1] | | | | (0,0)---q[0]---(0,1) (v)
* @bsimethod                                                    Lu.Han          03/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspsurf_gregoryPatch
(
MSBsplineSurface    *surface,          /* B-spline Coons patch surface */
MSBsplineCurve      *curves            /* bounding B-spline curve */
)
    {
    int             i, j, k, l, uk, vl, m, n, index, status, rationalSurface;
    double          pKnotTolerance, qKnotTolerance, pNode, qNode, pNodeM1, qNodeM1,
                    pWts0i, pWts1i, qWts0j, qWts1j, *p0Nodes, *q0Nodes, *weights;
    DPoint3d        pPoles0i, pPoles1i, qPoles0j, qPoles1j,
                    corner00, corner01, corner10, corner11, *controlNet;
    MSBsplineCurve  *p, *q;
    DPoint3d        twist[4], xu00, xu01, xu10, xu11, xv00, xv01, xv10, xv11,
                    tmp[8], uPar0v, uPar1v, vParu0, vParu1;
    double          hu0, hu1, hu2, hu3, hv0, hv1, hv2, hv3;

    /* Clear the surface structure */
    memset (surface, 0, sizeof(*surface));
    p0Nodes = q0Nodes = weights = NULL;     controlNet = NULL;

    if (SUCCESS != (status = bspsurf_orderCoonsCurves (curves, 4)))
        return status;
    p = curves;
    q = curves + 2;

    if (SUCCESS != (status = bspcurv_make2CurvesCompatible (&p[0], &p[1])) ||
        SUCCESS != (status = bspcurv_make2CurvesCompatible (&q[0], &q[1])))
        return status;

    /* Initialize internal variables */
    k = p[0].params.order;
    l = q[0].params.order;
    n = p[0].params.numPoles;
    m = q[0].params.numPoles;


    i = bspknot_numberKnots (p[0].params.numPoles, p[0].params.order, false);
    j = bspknot_numberKnots (q[0].params.numPoles, q[0].params.order, false);
    if (NULL == (p0Nodes = (double*)msbspline_malloc (i*sizeof(double), HEAPSIG_BSRF)) ||
        NULL == (q0Nodes = (double*)msbspline_malloc (j*sizeof(double), HEAPSIG_BSRF)))
        {
        status = ERROR;
        goto wrapup;
        }


    /***** find u,v knot_tol ****/
    pKnotTolerance = bspknot_knotTolerance (&p[0]);
    qKnotTolerance = bspknot_knotTolerance (&q[0]);

    bspknot_computeGrevilleAbscissa (p0Nodes, p[0].knots, p[0].params.numPoles,
                                     p[0].params.order, p[0].params.closed, pKnotTolerance);
    bspknot_computeGrevilleAbscissa (q0Nodes, q[0].knots, q[0].params.numPoles,
                                     q[0].params.order, q[0].params.closed, qKnotTolerance);

    /* Allocate weights if either curve is rational */
    if (true == (rationalSurface = p[0].rational || q[0].rational))
        {
        if (NULL == (weights = (double*)BSIBaseGeom::Malloc (m*n*sizeof(double))))
            {
            status = ERROR;
            goto wrapup;
            }
        }

    /* Calculate interior control net and weights if rational */
    if (NULL == (controlNet = (DPoint3d*)BSIBaseGeom::Malloc (n*m*sizeof(DPoint3d))))
        {
        status = ERROR;
        goto wrapup;
        }

    /* Assign corners by averaging end poles of p's & q's */
    if (p[0].rational)
        {
        corner00.Scale (*(&p[0].poles[0]), 1.0/p[0].weights[0]);
        corner01.Scale (*(&p[1].poles[0]), 1.0/p[1].weights[0]);
        corner10.Scale (*(&p[0].poles[n-1]), 1.0/p[0].weights[n-1]);
        corner11.Scale (*(&p[1].poles[n-1]), 1.0/p[1].weights[n-1]);
        }
    else if (q[0].rational)
        {
        corner00.Scale (*(&q[0].poles[0]), 1.0/q[0].weights[0]);
        corner01.Scale (*(&q[0].poles[m-1]), 1.0/q[0].weights[m-1]);
        corner10.Scale (*(&q[1].poles[0]), 1.0/q[1].weights[0]);
        corner11.Scale (*(&q[1].poles[m-1]), 1.0/q[1].weights[m-1]);
        }
    else
        {
        corner00 = p[0].poles[0];       corner01 = p[1].poles[0];
        corner10 = p[0].poles[n-1];     corner11 = p[1].poles[n-1];
        }

    /* Compute derivatives at the corner */
    if (p[0].rational)
        {
        tmp[0].Scale (*(&p[0].poles[1]), 1.0/p[0].weights[1]);
        tmp[2].Scale (*(&p[1].poles[1]), 1.0/p[1].weights[1]);
        tmp[4].Scale (*(&p[0].poles[n-2]), 1.0/p[0].weights[n-2]);
        tmp[6].Scale (*(&p[1].poles[n-2]), 1.0/p[1].weights[n-2]);
        }
    else
        {
        tmp[0] = p[0].poles[1];         tmp[2] = p[1].poles[1];
        tmp[4] = p[0].poles[n-2];       tmp[6] = p[1].poles[n-2];
        }

    if (q[0].rational)
        {
        tmp[1].Scale (*(&q[0].poles[1]), 1.0/q[0].weights[1]);
        tmp[3].Scale (*(&q[0].poles[m-2]), 1.0/q[0].weights[m-2]);
        tmp[5].Scale (*(&q[1].poles[1]), 1.0/q[1].weights[1]);
        tmp[7].Scale (*(&q[1].poles[m-2]), 1.0/q[1].weights[m-2]);
        }
    else
        {
        tmp[1] = q[0].poles[1];         tmp[3] = q[0].poles[m-2];
        tmp[5] = q[1].poles[1];         tmp[7] = q[1].poles[m-2];
        }

    uk = k - 1;
    xu00.x = uk * (tmp[0].x - corner00.x);
    xu00.y = uk * (tmp[0].y - corner00.y);
    xu00.z = uk * (tmp[0].z - corner00.z);
    xu10.x = uk * (corner10.x - tmp[4].x);
    xu10.y = uk * (corner10.y - tmp[4].y);
    xu10.z = uk * (corner10.z - tmp[4].z);
    xu01.x = uk * (tmp[2].x - corner01.x);
    xu01.y = uk * (tmp[2].y - corner01.y);
    xu01.z = uk * (tmp[2].z - corner01.z);
    xu11.x = uk * (corner11.x - tmp[6].x);
    xu11.y = uk * (corner11.y - tmp[6].y);
    xu11.z = uk * (corner11.z - tmp[6].z);

    vl = l - 1;
    xv00.x = vl * (tmp[1].x - corner00.x);
    xv00.y = vl * (tmp[1].y - corner00.y);
    xv00.z = vl * (tmp[1].z - corner00.z);
    xv01.x = vl * (corner01.x - tmp[3].x);
    xv01.y = vl * (corner01.y - tmp[3].y);
    xv01.z = vl * (corner01.z - tmp[3].z);
    xv10.x = vl * (tmp[5].x - corner10.x);
    xv10.y = vl * (tmp[5].y - corner10.y);
    xv10.z = vl * (tmp[5].z - corner10.z);
    xv11.x = vl * (corner11.x - tmp[7].x);
    xv11.y = vl * (corner11.y - tmp[7].y);
    xv11.z = vl * (corner11.z - tmp[7].z);

    /* Compute the corner twists, need try other choices */
    for (i=0; i<4; i++)
        {
        twist[i].x = 0.0;
        twist[i].y = 0.0;
        twist[i].z = 0.0;
        }

    for (i=0; i < n; i++)
        {
        pNode = p0Nodes[i];
        pNodeM1 = 1.0 - pNode;
        pWts0i = p[0].rational ? p[0].weights[i] : 1.0;
        pWts1i = p[1].rational ? p[1].weights[i] : 1.0;
        pPoles0i.Scale (*(&p[0].poles[i]), 1.0/pWts0i);
        pPoles1i.Scale (*(&p[1].poles[i]), 1.0/pWts1i);
        for (j=0; j < m; j++)
            {
            qNode = q0Nodes[j];
            qNodeM1 = 1.0 - qNode;
            qWts0j = q[0].rational ? q[0].weights[j] : 1.0;
            qWts1j = q[1].rational ? q[1].weights[j] : 1.0;
            qPoles0j.Scale (*(&q[0].poles[j]), 1.0/qWts0j);
            qPoles1j.Scale (*(&q[1].poles[j]), 1.0/qWts1j);
            index = i*m + j;

            bspsurf_cubicHermitBlends (&hu0, &hu1, &hu2, &hu3, pNode);
            bspsurf_cubicHermitBlends (&hv0, &hv1, &hv2, &hv3, qNode);

            vParu0.x = xv00.x * hu0 + xv10.x * hu3 + twist[0].x * hu1 + twist[2].x * hu2;
            vParu0.y = xv00.y * hu0 + xv10.y * hu3 + twist[0].y * hu1 + twist[2].y * hu2;
            vParu0.z = xv00.z * hu0 + xv10.z * hu3 + twist[0].z * hu1 + twist[2].z * hu2;
            vParu1.x = xv01.x * hu0 + xv11.x * hu3 + twist[1].x * hu1 + twist[3].x * hu2;
            vParu1.y = xv01.y * hu0 + xv11.y * hu3 + twist[1].y * hu1 + twist[3].y * hu2;
            vParu1.z = xv01.z * hu0 + xv11.z * hu3 + twist[1].z * hu1 + twist[3].z * hu2;

            uPar0v.x = xu00.x * hv0 + xu01.x * hv3 + twist[0].x * hv1 + twist[1].x * hv2;
            uPar0v.y = xu00.y * hv0 + xu01.y * hv3 + twist[0].y * hv1 + twist[1].y * hv2;
            uPar0v.z = xu00.z * hv0 + xu01.z * hv3 + twist[0].z * hv1 + twist[1].z * hv2;
            uPar1v.x = xu10.x * hv0 + xu11.x * hv3 + twist[2].x * hv1 + twist[3].x * hv2;
            uPar1v.y = xu10.y * hv0 + xu11.y * hv3 + twist[2].y * hv1 + twist[3].y * hv2;
            uPar1v.z = xu10.z * hv0 + xu11.z * hv3 + twist[2].z * hv1 + twist[3].z * hv2;

            controlNet[index].x = hu0*qPoles0j.x + hu3*qPoles1j.x
                                + hu1*uPar0v.x + hu2*uPar1v.x
                                + hv0*pPoles0i.x + hv3*pPoles1i.x
                                + hv1*vParu0.x + hv2*vParu1.x
                                - hv0*(hu0*corner00.x + hu1*xu00.x + hu2*xu10.x + hu3*corner10.x)
                                - hv1*(hu0*xv00.x + hu1*twist[0].x + hu2*twist[2].x + hu3*xv10.x)
                                - hv2*(hu0*xv01.x + hu1*twist[1].x + hu2*twist[3].x + hu3*xv11.x)
                                - hv3*(hu0*corner01.x + hu1*xu01.x + hu2*xu11.x + hu3*corner11.x);

            controlNet[index].y = hu0*qPoles0j.y + hu3*qPoles1j.y
                                + hu1*uPar0v.y + hu2*uPar1v.y
                                + hv0*pPoles0i.y + hv3*pPoles1i.y
                                + hv1*vParu0.y + hv2*vParu1.y
                                - hv0*(hu0*corner00.y + hu1*xu00.y + hu2*xu10.y + hu3*corner10.y)
                                - hv1*(hu0*xv00.y + hu1*twist[0].y + hu2*twist[2].y + hu3*xv10.y)
                                - hv2*(hu0*xv01.y + hu1*twist[1].y + hu2*twist[3].y + hu3*xv11.y)
                                - hv3*(hu0*corner01.y + hu1*xu01.y + hu2*xu11.y + hu3*corner11.y);

            controlNet[index].z = hu0*qPoles0j.z + hu3*qPoles1j.z
                                + hu1*uPar0v.z + hu2*uPar1v.z
                                + hv0*pPoles0i.z + hv3*pPoles1i.z
                                + hv1*vParu0.z + hv2*vParu1.z
                                - hv0*(hu0*corner00.z + hu1*xu00.z + hu2*xu10.z + hu3*corner10.z)
                                - hv1*(hu0*xv00.z + hu1*twist[0].z + hu2*twist[2].z + hu3*xv10.z)
                                - hv2*(hu0*xv01.z + hu1*twist[1].z + hu2*twist[3].z + hu3*xv11.z)
                                - hv3*(hu0*corner01.z + hu1*xu01.z + hu2*xu11.z + hu3*corner11.z);

            if (rationalSurface)
                weights[index]  = (qNodeM1*pWts0i + qNode*pWts1i)
                                * (pNodeM1*qWts0j + pNode*qWts1j);
            }
        }

    /* Set the surface parameters and allocate the surface */
    surface->uParams = q[0].params;
    surface->vParams = p[0].params;

    /* Set number or interior knots (always nonuniform) */
    surface->uParams.numKnots = m - l;
    surface->vParams.numKnots = n - k;

    /* The coons surface is never closed */
    surface->uParams.closed = surface->vParams.closed = false;
    surface->rational = rationalSurface;
    surface->display.curveDisplay = true;

    /* Set the number of rule lines to the number of poles in the opposition
        direction */
    surface->uParams.numRules = surface->vParams.numPoles;
    surface->vParams.numRules = surface->uParams.numPoles;

    if (SUCCESS != (status = bspsurf_allocateSurface (surface)))
        goto wrapup;

    /* Load knot vectors */
    memcpy (surface->vKnots, p[0].knots, (n+k) * sizeof(double));
    memcpy (surface->uKnots, q[0].knots, (m+l) * sizeof(double));

    if (rationalSurface)
        {
        memcpy (surface->weights, weights, n*m*sizeof(double));
        bsputil_weightPoles (surface->poles, controlNet, weights, n*m);
        }
    else
        memcpy (surface->poles, controlNet, n*m*sizeof(DPoint3d));

wrapup:
    if (p0Nodes)        msbspline_free (p0Nodes);
    if (q0Nodes)        msbspline_free (q0Nodes);
    if (controlNet)     BSIBaseGeom::Free (controlNet);
    if (weights)        BSIBaseGeom::Free (weights);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          04/93
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    bspsurf_degenerateBoundary
(
MSBsplineSurface    *surf
)
    {
    bool        degen = true;
    int         i, numU, numV;
    DPoint3d    *pP;

    numU = surf->uParams.numPoles;
    numV = surf->vParams.numPoles - 1;
    for (i = 0, pP = surf->poles; i < numV; pP += numU, i++)
         if (pP->Distance (pP[numU]) > fc_epsilon)
            {
            degen = false;
            break;
            }

    return (degen);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          05/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspsurf_gordonSurface
(
MSBsplineSurface    *surface,          /* <= B-spline Gordon surface */
MSBsplineCurve      *uCurves,          /* => network of B-spline curves */
MSBsplineCurve      *vCurves,          /* => network of B-spline curves */
int                 numUCurves,        /* => number of curves in U direction */
int                 numVCurves         /* => number of curves in V direction */
)
    {
    bool                useSubdivision;
    int                 i, j, k, itmp, jtmp, iitmp, allocSize, status,
                        index, index0, index1, index2, index3, numInts;
    double              *u, *v, tol, *intParams0P, *intParams1P;
    DPoint3d            pt0, pt1;
    DVec3d              normal0, normal1;
    MSBsplineCurve      *p, *q, seg[4];
    MSBsplineSurface    patch, uSurf, vSurf;
    RotMatrix           rotMatrix;

    /* try other tolerance */
    tol = 0.1;

    u = v = NULL;
    /* Clear surfaces structures */
    memset (surface, 0, sizeof(*surface));
    memset (&uSurf, 0, sizeof(uSurf));
    memset (&vSurf, 0, sizeof(vSurf));

    p = uCurves;
    q = vCurves;

    /* malloc memary for u and v parameters of the intersections */
    allocSize = numUCurves * numVCurves * sizeof(double);
    if (NULL == (u = (double*)msbspline_malloc (allocSize, HEAPSIG_BSRF)) ||
        NULL == (v = (double*)msbspline_malloc (allocSize, HEAPSIG_BSRF)))
        {
        status = ERROR;
        goto wrapup;
        }

    /* find all the u and v parameters at intersections */
    for (i = 0; i < numUCurves; i++)
        {
        itmp = i * numVCurves;
        for (j = 0; j < numVCurves; j++)
            {
            index = itmp + j;

            /* Need work in cv-cv intsection code to cover point cv case */
            if (p[i].poles->Distance (*(q[j].poles)) < fc_epsilon)
                {
                u[index] = v[index] = 0.0;
                }
            else if (p[i].poles->Distance (*(q[j].poles +
                     q[j].params.numPoles - 1)) < fc_epsilon)
                {
                u[index] = 0.0;
                v[index] = 1.0;
                }
            else if ((p[i].poles+p[i].params.numPoles-1)->Distance (*(q[j].poles)) < fc_epsilon)
                {
                u[index] = 1.0;
                v[index] = 0.0;
                }
            else if ((p[i].poles+p[i].params.numPoles-1)->Distance (*(q[j].poles+q[j].params.numPoles-1)) < fc_epsilon)
                {
                u[index] = v[index] = 1.0;
                }
            else
                {
                bspcurv_extractNormal (&normal0, NULL, NULL, p+i);
                bspcurv_extractNormal (&normal1, NULL, NULL, q+j);
                normal0.SumOf (normal0, normal1);
                normal0.Normalize ();
                rotMatrix_orthogonalFromZRow (&rotMatrix, &normal0);

                useSubdivision = p[i].params.order > 5 || q[j].params.order > 5;
                if (SUCCESS == (status = bspcci_allIntersectsBtwCurves (NULL, &intParams0P,
                        &intParams1P, &numInts, p+i, q+j, &tol, &rotMatrix, useSubdivision)) &&
                        numInts > 0)
                    {
                    u[index] = intParams0P[0];
                    v[index] = intParams1P[0];
                    bspcurv_evaluateCurvePoint (&pt0, NULL, p+i, u[index]);
                    bspcurv_evaluateCurvePoint (&pt1, NULL, q+j, v[index]);
                    BSIBaseGeom::Free (intParams0P);
                    BSIBaseGeom::Free (intParams1P);
                    }
                else
                    {
                    status = ERROR;
                    goto wrapup;
                    }
                }
            }
        }

    /* make each boundary curve a bspline curve and create patches */
    for (i = 1; i < numUCurves; i++)
        {
        itmp = (i - 1) * numVCurves;
        iitmp = itmp + numVCurves;
        for (j = 1; j < numVCurves; j++)
            {
            jtmp = j - 1;
            /* create four boundary curves */
            index0 = itmp + jtmp;
            index1 = index0 + 1;
            index2 = iitmp + jtmp;
            index3 = index2 + 1;

            bspcurv_segmentCurve (seg,   p+i-1, u[index0], u[index1]);
            bspcurv_segmentCurve (seg+1, p+i,   u[index2], u[index3]);
            bspcurv_segmentCurve (seg+2, q+j-1, v[index0], v[index2]);
            bspcurv_segmentCurve (seg+3, q+j,   v[index1], v[index3]);

            /* To force the end points meet */
            for (k=0; k < 4; k++)
                {
                if (seg[k].rational)
                    bsputil_unWeightPoles (seg[k].poles, seg[k].poles,
                                  seg[k].weights, seg[k].params.numPoles);
                }

            seg[2].poles[0] = seg[0].poles[0];
            seg[2].poles[seg[2].params.numPoles-1] = seg[1].poles[0];
            seg[3].poles[0] = seg[0].poles[seg[0].params.numPoles-1];
            seg[3].poles[seg[3].params.numPoles-1] =
                                seg[1].poles[seg[1].params.numPoles-1];

            for (k=0; k < 4; k++)
                {
                if (seg[k].rational)
                    {
                    seg[k].weights[0] =
                    seg[k].weights[seg[k].params.numPoles-1] = 1.0;
                    bsputil_weightPoles (seg[k].poles, seg[k].poles,
                                  seg[k].weights, seg[k].params.numPoles);
                    }
                }

            for (k=0; k<4; k++)
                {
#if defined (debug_draw)
            {
            ElementDescr        *edpDraw = NULL;
            MSElementUnion      elUDraw;

            seg[k].display.polygonDisplay = false;
            seg[k].display.curveDisplay = true;
            bspcurv_createCurve (&edpDraw, NULL, &seg[k]);
            mdlElmdscr_display (edpDraw, MASTERFILE, NORMALDRAW);
            mdlElmdscr_freeAll (&edpDraw);

            if (seg[k].rational)
                bsputil_unWeightPoles (seg[k].poles, seg[k].poles, seg[k].weights, 2);
            create_lineStringD (&elUDraw, NULL, seg[k].poles, NULL, 2, LINE_STRING_ELM, 0);
            if (seg[k].rational)
                bsputil_weightPoles (seg[k].poles, seg[k].poles, seg[k].weights, 2);
            elUDraw.hdr.dhdr.symb.b.weight = 5;
            elUDraw.hdr.dhdr.symb.b.color = 4;
            displayElement (&elUDraw, 0, NORMALDRAW);
            }
#endif
                }

            /* make patch out of the above four curves */
            if (status = bspsurf_gregoryPatch (&patch, seg))
                return (status);

#if defined (debug_draw)
            {
            ElementDescr        *edpDraw = NULL;

            patch.display.polygonDisplay = false;
            patch.display.curveDisplay = true;
            patch.uParams.numRules = patch.vParams.numPoles;
            patch.vParams.numRules = patch.uParams.numPoles;
            bspsurf_createSurface (&edpDraw, NULL, &patch);
            mdlElmdscr_display (edpDraw, MASTERFILE, HILITE);
            mdlElmdscr_freeAll (&edpDraw);
            }
#endif

            /* join patches in U direction */
            if (j == 1)
                bspsurf_copySurface (&uSurf, &patch);
            else
                bsprsurf_appendSurfaces (&uSurf, &uSurf, &patch, BSSURF_U, true, true);

            bspcurv_freeCurve (seg);
            bspcurv_freeCurve (seg+1);
            bspcurv_freeCurve (seg+2);
            bspcurv_freeCurve (seg+3);
            bspsurf_freeSurface (&patch);
            }

        /* join patches in V direction */
        if (i == 1)
            {
            bspsurf_copySurface (&vSurf, &uSurf);
            if (bspsurf_degenerateBoundary (&vSurf))
                bspsurf_reverseSurface (&vSurf, &vSurf, BSSURF_V);
            }
        else
            bsprsurf_appendSurfaces (&vSurf, &vSurf, &uSurf, BSSURF_V, true, true);

#if defined (debug_draw)
            {
            ElementDescr        *edpDraw = NULL;

            vSurf.display.polygonDisplay = false;
            vSurf.display.curveDisplay = true;
            vSurf.uParams.numRules = vSurf.vParams.numPoles;
            vSurf.vParams.numRules = vSurf.uParams.numPoles;
            bspsurf_createSurface (&edpDraw, NULL, &vSurf);
            mdlElmdscr_display (edpDraw, MASTERFILE, HILITE);
            mdlElmdscr_freeAll (&edpDraw);
            }
#endif

        bspsurf_freeSurface (&uSurf);
        }

    *surface = vSurf;
    surface->display.polygonDisplay = false;
    surface->display.curveDisplay = true;
    status = SUCCESS;

wrapup:
    if (u)  msbspline_free(u);
    if (v)  msbspline_free(v);
    if (status)
        bspsurf_freeSurface (&vSurf);

    return status;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          07/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspsurf_approxByCubicNonRational
(
MSBsplineCurve      *outCv,             /* => non-rational curve of order 4 */
MSBsplineCurve      *inCv               /* => rational curve of order 3 */
)
    {
    int         i, numPoles, status = SUCCESS, num;
    double      data, length0, length1, kx, deno, factor;
    DPoint3d    *evalPt, midPt0, midPt1, diff0, diff1;

    /* initialize variables */
    outCv->type = BSCURVE_GENERAL;
    outCv->rational = false;
    outCv->params.order = 4;
    outCv->params.closed = false;
    outCv->params.numPoles = 4;
    outCv->params.numKnots = 0;
    outCv->display.curveDisplay = true;

    if (SUCCESS != (status = bspcurv_allocateCurve (outCv)))
        return status;

    for (i = 0; i < outCv->params.order; i++)
        {
        outCv->knots[i] = 0.0;
        outCv->knots[i+outCv->params.order] = 1.0;
        }

    if (inCv->params.order == 2)
        {
        outCv->poles[0] = inCv->poles[0];
        outCv->poles[3] = inCv->poles[1];
        (outCv->poles+1)->Scale (*(inCv->poles), 2.0);
        (outCv->poles+1)->SumOf (*(outCv->poles+1), *(inCv->poles+1));
        (outCv->poles+1)->Scale (*(outCv->poles+1), 1.0/fc_3);
        (outCv->poles+2)->Scale (*(inCv->poles+1), 2.0);
        (outCv->poles+2)->SumOf (*(outCv->poles+2), *(inCv->poles));
        (outCv->poles+2)->Scale (*(outCv->poles+2), 1.0/fc_3);
        return SUCCESS;
        }
    else if (inCv->params.order == 3 && !inCv->rational)
        {
        status = bspcurv_elevateDegree (outCv, inCv, 3);
        return status;
        }
    else if (inCv->params.order == 3 && inCv->rational)
        {
        numPoles = inCv->params.numPoles - 1;

        ((outCv->poles[0])).Scale (*(&(inCv->poles[0])), 1.0/inCv->weights[0]);
        ((outCv->poles[3])).Scale (*(&(inCv->poles[numPoles])), 1.0/inCv->weights[numPoles]);
        midPt0.Scale (*(&(inCv->poles[1])), 1.0/inCv->weights[1]);
        midPt1.Scale (*(&(inCv->poles[numPoles-1])), 1.0/inCv->weights[numPoles-1]);

        diff0.DifferenceOf (midPt0, *(&(outCv->poles[0])));
        diff1.DifferenceOf (midPt1, *(&(outCv->poles[3])));

        length0 = diff0.Magnitude ();
        length1 = diff1.Magnitude ();
        factor = length1 / length0;

        diff0.Normalize ();
        diff1.Normalize ();

        data = 0.5;
        num = 1;
        bspcurv_evaluateCurve (&evalPt, &data, &num, inCv);


        deno = diff0.x + diff1.x * factor;
        if (fabs(deno) < fc_epsilon)
            {
            deno = diff0.y + diff1.y * factor;
            if (fabs(deno) < fc_epsilon)
                {
                deno = diff0.z + diff1.z * factor;
                kx = (evalPt->z - 0.5*(outCv->poles[0].z + outCv->poles[3].z)) /
                     (deno * fc_3/8.0);
                }
            else
                kx = (evalPt->y - 0.5*(outCv->poles[0].y + outCv->poles[3].y)) /
                     (deno * fc_3/8.0);
            }
        else
            kx = (evalPt->x - 0.5*(outCv->poles[0].x + outCv->poles[3].x)) /
                 (deno * fc_3/8.0);

        ((outCv->poles[1])).SumOf (*(&(outCv->poles[0])), diff0, kx);
        ((outCv->poles[2])).SumOf (*(&(outCv->poles[3])), diff1, kx*factor);

        return SUCCESS;
        }
    else if (inCv->params.numPoles == 4)
        {
        bspcurv_copyCurve (outCv, inCv);
        return SUCCESS;
        }
    else
        return ERROR;
    }

// REMOVE AAA
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/93
+---------------+---------------+---------------+---------------+---------------+------*/
static void bspdsurf_updateTubeMatrix
(
RotMatrix   *newMatrixP,
RotMatrix   *prevMatrixP,
bool        rigidSweep,
DVec3d      *newTangentP
)
    {
    DVec3d    prevTangent;

    bsiRotMatrix_getRow ( prevMatrixP, &prevTangent,  2);

    if (newTangentP->DotProduct (prevTangent) > .99999)
        {
        *newMatrixP = *prevMatrixP;
        }
    else
        {
        double      twistAngle;
        DVec3d    pivotX, pivotY, prevX, prevY;
        RotMatrix   pivotMatrix, twistMatrix;

        bsiRotMatrix_getRow (     prevMatrixP, &prevX,  0);
        bsiRotMatrix_getRow (     prevMatrixP, &prevY,  1);
        if (rigidSweep == true)
            {
            prevX.CrossProduct (prevY, *newTangentP);
            prevX.Normalize ();
            newMatrixP->InitFromRowVectors (prevX, prevY, *newTangentP);
            }
        else
            {
            pivotX.CrossProduct (prevTangent, *newTangentP);
            pivotX.Normalize ();
            twistAngle = Angle::Atan2 (pivotX.DotProduct (prevY),
                            pivotX.DotProduct (prevX));

            bsiRotMatrix_rotateByPrincipleAngles (&twistMatrix, NULL, 0.0, 0.0, twistAngle);

            /* Now compute the new frame by computing the pivotted system
            and then applying same twist */
            pivotY.CrossProduct (*newTangentP, pivotX);
            pivotY.Normalize ();
            pivotMatrix.InitFromRowVectors (pivotX, pivotY, *newTangentP);
            newMatrixP->InitProduct (twistMatrix, pivotMatrix);
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* ----------- | | NOTE : SEGMENT IS A BEZIER SEGMENT (u) section | I.E. IT HAS AT MOST MAX_ORDER POLES! | | | | - segment - (v)
* @bsimethod                                                    BFP             10/90
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspsurf_tubePatch
(
MSBsplineSurface    *surface,          /* <= surface */
MSBsplineCurve      *segment,          /* => Bezier segment!!!! */
MSBsplineCurve      *section,
RotMatrix           *rotMatrixP,
bool                rigidSweep
)
    {
    int             i, j, index, status, rationalSurface;
    double          knotTolerance, nodes[2*MAX_ORDER], vWtsi, *weights, factor;
    DPoint3d        vPolesi, *poles, *pP, line[2];
    DVec3d          traceDervs[3];
    MSBsplineCurve  curve;

    if (SUCCESS != (status = bspcurv_copyCurve (&curve, section)))
        return status;

    /* Initialize variables */
    memset (surface, 0, sizeof (*surface));

    /* Set the surface parameters and allocate the surface */
    surface->rational = rationalSurface = curve.rational || segment->rational;
    surface->uParams  = curve.params;
    surface->vParams  = segment->params;

    if (SUCCESS != (status = bspsurf_allocateSurface (surface)))
        goto wrapup;

    poles   = surface->poles;
    weights = surface->weights;

    /* Un-weight poles before aligning & blending */
    if (curve.rational)
        bsputil_unWeightPoles (curve.poles, curve.poles, curve.weights,
                               curve.params.numPoles);

    /* Align poles at origin according to orientation */
    for (i=0, pP=curve.poles; i < curve.params.numPoles; i++, pP++)
        pP->z = 0.0;

#if defined (needs_work)
/* get rid of this as we know that segment is a bezier curve */
#endif
    /* Get Greville abscissa of the trace curve */
    knotTolerance = bspknot_knotTolerance (segment);

    bspknot_computeGrevilleAbscissa (nodes, segment->knots, segment->params.numPoles,
                                     segment->params.order, segment->params.closed,
                                     knotTolerance);

    /* Calculate the poles and weights (if rational):
        Loop over the nodes of the trace.  For each value, align poles
        with the Frenet frame of the trace evaluated at the current node.
        Note optional case check using lastBvector to make sure that surface
        does not cross itself when trace has an inflection point.
        Form a translational surface with the trace; i.e., add
        segment->poles[i], multiply segment->weights[i] (if rational). */

    for (i=0; i < segment->params.numPoles; i++)
        {
        vWtsi = segment->rational ? segment->weights[i] : 1.0;
        bsiDPoint3d_scale (&vPolesi, segment->poles+i, (factor = 1.0/vWtsi));
        for (j=0; j < curve.params.numPoles; j++)
            {
            index = i*curve.params.numPoles + j;
            poles[index] = curve.poles[j];
            if (rationalSurface)
                weights[index] = (curve.rational ? curve.weights[j] : 1.0) * vWtsi;

            /* Scale to compensate for segment weights */
            if (!MSBsplineCurve::AreSameWeights  (factor, 1.0))
                poles[index].x *= factor;
            }

        /* Position the last poles to get translational surface at this node */
        /* Calculate Frenet frame at this node */
        bspcurv_frenetFrame (traceDervs, line, NULL, NULL, segment, nodes[i], NULL);

        if (i)
            bspdsurf_updateTubeMatrix (rotMatrixP, rotMatrixP, rigidSweep, &traceDervs[0]);

        /* Rotate blended poles at origin */
        index = i*curve.params.numPoles;
        rotMatrixP->MultiplyTranspose (poles+index, poles+index, curve.params.numPoles);

        /* Translate into place */
        DPoint3d::AddToArray (poles+index, curve.params.numPoles, vPolesi);
        }

    /* Set the number of rule lines to the number of poles in the opposition
        direction */
    surface->uParams.numRules = surface->vParams.numPoles;
    surface->vParams.numRules = surface->uParams.numPoles;

    /* Load knot vectors */
    memcpy (surface->vKnots, segment->knots,
            bspknot_numberKnots (segment->params.numPoles, segment->params.order,
                                 segment->params.closed) * sizeof(double));
    memcpy (surface->uKnots, curve.knots,
            bspknot_numberKnots (curve.params.numPoles, curve.params.order,
                                 curve.params.closed) * sizeof(double));

    if (rationalSurface)
        bsputil_weightPoles (poles, poles, weights,
                             segment->params.numPoles * curve.params.numPoles);

    surface->display.curveDisplay = true;

wrapup:
    bspcurv_freeCurve (&curve);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             01/91
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspsurf_tubeProcess
(
BezierInfo      *infoP,
MSBsplineCurve  *bezier
)
    {
    int               status;
    double            weights[MAX_ORDER], knots[2*MAX_ORDER];
    DPoint3d          poles[MAX_ORDER], line[2];
    DVec3d            frame[3];
    MSBsplineCurve    curve;

    memset (&curve, 0, sizeof(curve));
    curve.rational        = bezier->rational;
    curve.params.numPoles =
    curve.params.order    = bezier->params.order;
    curve.poles           = poles;
    curve.knots           = knots;
    curve.weights         = weights;

    memcpy (curve.poles, bezier->poles, curve.params.numPoles * sizeof(DPoint3d));
    if (curve.rational)
        memcpy (curve.weights, bezier->weights, curve.params.numPoles * sizeof(double));

    bspknot_computeKnotVector (curve.knots, &curve.params, NULL);
    bspcurv_frenetFrame (frame, line, NULL, NULL, &curve, 0.0, NULL);

    bspdsurf_updateTubeMatrix (infoP->tube.matrix, infoP->tube.matrix,
        infoP->tube.rigidSweep, &frame[0]);

    status = bspsurf_tubePatch (infoP->tube.patch, &curve, infoP->tube.section,
                                infoP->tube.matrix, infoP->tube.rigidSweep);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             01/91
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspsurf_tubeAssemble
(
BezierInfo      *infoP,
BezierInfo      *segInfo,
MSBsplineCurve  *curve,
int             segNumber,
int             knotIndex,
int             numberOfSegments
)
    {
    int                 status, code0, code1, index;
    double              u0, u1;
    DPoint3d            *p0, *p1, *p2, *p3, *endP;
    DSegment3d           seg0, seg1;
    MSBsplineSurface    *patch, *surface;
    double              pointTol, intersectionTol;

    static double       s_relTol = 1.0e-4;

    if (!segNumber)
        {
        status = bspsurf_copySurface (infoP->tube.surface, segInfo->tube.patch);
        bspsurf_freeSurface (segInfo->tube.patch);
        return status;
        }

    /* check if mitre is necessary at this joint */
    patch   = segInfo->tube.patch;
    surface = infoP->tube.surface;

    // old tols in UORs (intersectionTol = 1.0, pointTol = 0.25), but VBA calls in master units, so must base tols on fraction of range
    intersectionTol = bsiDPoint3d_getLargestWeightedCoordinateDifference (surface->poles, surface->rational ? surface->weights : NULL, surface->uParams.numPoles * 2);
    intersectionTol *= s_relTol;
    pointTol = intersectionTol * 0.25;

    /* WARNING: this code assumes that the weights of the last row of surface
        are always 1.0. index is used to save unweighting and reweighting all
        the poles of surface. */
    index = surface->uParams.numPoles * (surface->vParams.numPoles - 2);

    if (patch->rational)
        bsputil_unWeightPoles (patch->poles, patch->poles, patch->weights, patch->uParams.numPoles * 2);

    if (surface->rational)
        bsputil_unWeightPoles (surface->poles + index, surface->poles + index, surface->weights + index, surface->uParams.numPoles * 2);

    /* p0 = first pole in second to last row of surface.
       p1 = first pole in last row of surface.
       p2 = first pole in first row of patch.
       p3 = first pole in second row of patch. */
    for (p0 = p1 = endP = surface->poles + index,
         p1 += surface->uParams.numPoles,
         p2 = p3 = patch->poles,
         p3 += patch->uParams.numPoles,
         endP += surface->uParams.numPoles;
         p0 < endP;
         p0++, p1++, p2++, p3++)
        {
        if (!p1->IsEqual (*p2, pointTol))
            {
            seg0.point[0] = *p0;     seg0.point[1] = *p1;
            seg1.point[0] = *p2;     seg1.point[1] = *p3;

            bsputil_segmentIntersection (&code0, &code1, p1, p2, &u0, &u1, (DPoint3d *) &seg0, (DPoint3d *) &seg1, intersectionTol);
            }
        }

    if (patch->rational)
        bsputil_weightPoles (patch->poles, patch->poles, patch->weights, patch->uParams.numPoles * 2);

    if (surface->rational)
        bsputil_weightPoles (surface->poles + index, surface->poles + index, surface->weights + index, surface->uParams.numPoles * 2);

    /* Scale so that the rule lines will fall at the mitre joints */
    bspknot_scaleKnotVector (surface->vKnots, &surface->vParams, (double) segNumber);

    if (SUCCESS != (status = bsprsurf_combineSurfaces (surface, surface, patch, BSSURF_U, true, false)))
        return status;

    if (segNumber == numberOfSegments-1 && infoP->tube.closedTrace)
        {
        /* insure mitred closure - kludged place to store this info but
            it works for now. This patch has already been added to surface.*/
        if (surface->rational)
            bsputil_unWeightPoles (surface->poles, surface->poles, surface->weights, surface->uParams.numPoles * surface->vParams.numPoles);

        /* p0 = first pole in second to last row of surface.
        p1 = first pole in last row of surface.
        p2 = first pole in first row of surface.
        p3 = first pole in second row of surface. */
        index = surface->uParams.numPoles * (surface->vParams.numPoles - 2);
        for (p0 = p1 = endP = surface->poles + index,
             p1 += surface->uParams.numPoles,
             p2 = p3 = surface->poles,
             p3 += surface->uParams.numPoles,
             endP += surface->uParams.numPoles;
             p0 < endP;
             p0++, p1++, p2++, p3++)
            {
            if (!p1->IsEqual (*p2, pointTol))
                {
                seg0.point[0] = *p0;     seg0.point[1] = *p1;
                seg1.point[0] = *p2;     seg1.point[1] = *p3;

                if (SUCCESS == bsputil_segmentIntersection (&code0, &code1, p1, p2, &u0, &u1, (DPoint3d*) &seg0, (DPoint3d*) &seg1, intersectionTol))
                    {
                    p1->Interpolate (*p1, 0.5, *p2);
                    *p2 = *p1;
                    }
                }
            }

        if (surface->rational)
            bsputil_weightPoles (surface->poles, surface->poles, surface->weights, surface->uParams.numPoles * surface->vParams.numPoles);
        }

    bspsurf_freeSurface (segInfo->tube.patch);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             10/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspsurf_tubeSurface
(
MSBsplineSurface    *surface,          /* <= surface */
MSBsplineCurveCP     trace,            /* => B-spline curve */
MSBsplineCurveCP     section,
DSegment3d          *orientationUnused,
double              *cuspRadiusUnused,
bool                rigidSweep
)
    {
    int                 status;
    DVec3d              frame[3];
    RotMatrix           rotMatrix;
    BezierInfo          info;
    MSBsplineSurface    patch;

    memset (&patch, 0, sizeof(patch));
    memset (surface, 0, sizeof(*surface));

    info.tube.section = const_cast <MSBsplineCurveP> (section);
    info.tube.patch   = &patch;
    info.tube.surface = surface;
    info.tube.matrix  = &rotMatrix;
    info.tube.rigidSweep = rigidSweep;

    bspcurv_frenetFrame (frame, NULL, NULL, NULL, trace, 0.0, NULL);
    rotMatrix.InitFromRowVectors (frame[1], frame[2], *frame);
#ifdef RemvoeCusps
    if (cuspRadius)
        {
        /* remove cusps from trace curve */
        }
#endif
    info.tube.closedTrace = trace->params.closed != 0;

    if (SUCCESS != (status =  bspproc_processBspline (&info, 
          const_cast <MSBsplineCurveP> (trace), NULLFUNC, NULLFUNC, bspsurf_tubeProcess, bspsurf_tubeAssemble, NULLFUNC)))
        return status;

    if (false && trace->params.closed)
        bsprsurf_closeSurface (surface, surface, BSSURF_V);

    surface->display.curveDisplay = true;
    surface->uParams.numRules = surface->vParams.numPoles;
    surface->vParams.numRules = surface->uParams.numPoles;
    return SUCCESS;
    }

MSBsplineSurfacePtr MSBsplineSurface::CreateTubeSurface (MSBsplineCurveCR baseCurve, bool translateBaseCurve, MSBsplineCurveCR traceCurve)
    {
    MSBsplineSurfacePtr result = MSBsplineSurface::CreatePtr ();
    bspsurf_tubeSurface (result.get (), &traceCurve, &baseCurve, NULL, NULL, translateBaseCurve);
    return result;
    }
#endif
#ifdef compile_bspsurf_skinSurface
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             02/91
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspsurf_prepareSkin
(
MSBsplineCurve  *segment,
MSBsplineCurve  *segment1,
MSBsplineCurve  *section0,
MSBsplineCurve  *section1,
MSBsplineCurve  *trace,
MSBsplineCurve  *trace1,
double          initial,
double          final
)
    {
    int         status;
    double      dotM;
    DPoint3d    lastM;
    DVec3d      frame[3];
    RotMatrix   rMA, rMB;

    if (SUCCESS != (status = bspcurv_segmentCurve (segment, trace, initial, final)))
        return status;
    if (trace1)
        {
        if (SUCCESS != (status = bspcurv_segmentCurve (segment1, trace1, initial, final)))
            return status;
        }

    if (! trace1)
        {
        bspcurv_frenetFrame (frame, NULL, NULL, NULL, trace, initial, NULL);
        rMB.InitFromRowVectors (frame[1], frame[2], *frame);
        lastM = frame[1];
        bspcurv_frenetFrame (frame, NULL, NULL, NULL, segment, 0.0, NULL);
        dotM = lastM.DotProduct (frame[1]);
        if (dotM < COSINE_TOLERANCE)
            {
            rMA.InitFromColumnVectors (frame[1], frame[2], *frame);
            rMA.InitProduct (rMB, rMA);
            rMA.MultiplyTranspose (section0->poles, section0->poles, section0->params.numPoles);
            }

        bspcurv_frenetFrame (frame, NULL, NULL, NULL, trace, final, NULL);
        rMB.InitFromRowVectors (frame[1], frame[2], *frame);
        lastM = frame[1];
        bspcurv_frenetFrame (frame, NULL, NULL, NULL, segment, 1.0, NULL);
        dotM = lastM.DotProduct (frame[1]);
        if (dotM < COSINE_TOLERANCE)
            {
            rMA.InitFromColumnVectors (frame[1], frame[2], *frame);
            rMA.InitProduct (rMB, rMA);
            rMA.MultiplyTranspose (section1->poles, section1->poles, section1->params.numPoles);
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* ----------- | | (u) section0 | | section1 | | -- trace0 - (v)
* @bsimethod                                                    BFP             10/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspsurf_skinPatch
(
MSBsplineSurface    *surface,
MSBsplineCurve      *trace0,
MSBsplineCurve      *trace1,
MSBsplineCurve      *section0,
MSBsplineCurve      *section1,
DSegment3d          *orientation0,
DSegment3d          *orientation1,
int                 checkInflectionPt
)
    {
    int             i, j, k, l, m, n, index, rationalSurface, numInflections, status;
    double          knotTolerance, blend, blendM1, *nodes=NULL, weight, dist0,
                    uWts0j, uWts1j, vWtsi, *weights, factor, openAtParam;
    DPoint3d        uPoles0j, uPoles1j, vPolesi, *poles, minPt;
    DVec3d          lastBvector, traceDervs[3];
    DSegment3d       orient0, orient1;
    RotMatrix       rotMatrix;
    MSBsplineCurve  curve0, curve1;

    /* Initialize variables */
    memset (surface, 0, sizeof (*surface));
    memset (&curve0, 0, sizeof (curve0));
    memset (&curve1, 0, sizeof (curve1));

    orient0 = *orientation0;
    orient1 = *orientation1;

    if (SUCCESS != (status = bspcurv_openCurve (&curve0, section0, 0.0)) ||
        SUCCESS != (status = bspcurv_openCurve (&curve1, section1, 0.0)))
        goto wrapup;

    if (NULL == (nodes =
        (double*)msbspline_malloc (trace0->params.numPoles * sizeof(double), HEAPSIG_BSRF)))
        {
        status = ERROR;
        goto wrapup;
        }

    /* Check for inflection pts in the trace */
    if (checkInflectionPt)
        {
        bspcurv_inflectionPoints (NULL, &numInflections, trace0);
        if (numInflections % 2)
            {
            lastBvector.DifferenceOf (*(&orient1.point[1]), *(&orient1.point[0]));
            orient1.point[1].DifferenceOf (*(&orient1.point[0]), lastBvector);
            }
        }

    /* Un-weight poles before aligning */
    if (curve0.rational)
        bsputil_unWeightPoles (curve0.poles, curve0.poles, curve0.weights,
                               curve0.params.numPoles);
    if (curve1.rational)
        bsputil_unWeightPoles (curve1.poles, curve1.poles, curve1.weights,
                               curve1.params.numPoles);

    /* Align poles at origin according to orientation */
    traceDervs[2].x = traceDervs[2].y = 0.0; traceDervs[2].z = 1.0;

    /*-------------------------------------------------------------------
    Do not flatten so that sections can be out of the plane perpendicular to end tangent

    for (poles=endP=curve0.poles, endP += curve0.params.numPoles; poles < endP; poles++)
        poles->z = 0.0;
    for (poles=endP=curve1.poles, endP += curve1.params.numPoles; poles < endP; poles++)
        poles->z = 0.0;
    orient0.point[0].z = orient0.point[1].z = orient1.point[0].z = orient1.point[1].z = 0.0;
    -------------------------------------------------------------------*/

    bsiDPoint3d_subtractDPoint3dArray (curve0.poles, &orient0.point[0], curve0.params.numPoles);
    bsiDPoint3d_subtractDPoint3dArray (curve1.poles, &orient1.point[0], curve1.params.numPoles);

    traceDervs->NormalizedDifference (*(&orient0.point[1]), *(&orient0.point[0]));
    traceDervs[1].CrossProduct (traceDervs[2], *traceDervs);
    rotMatrix.InitFromColumnVectors (*traceDervs, traceDervs[1], traceDervs[2]);
    rotMatrix.MultiplyTranspose (curve0.poles, curve0.poles, curve0.params.numPoles);

    traceDervs->NormalizedDifference (*(&orient1.point[1]), *(&orient1.point[0]));
    traceDervs[1].CrossProduct (traceDervs[2], *traceDervs);
    rotMatrix.InitFromColumnVectors (*traceDervs, traceDervs[1], traceDervs[2]);
    rotMatrix.MultiplyTranspose (curve1.poles, curve1.poles, curve1.params.numPoles);

    if (curve0.rational)
        bsputil_weightPoles (curve0.poles, curve0.poles, curve0.weights,
                             curve0.params.numPoles);
    if (curve1.rational)
        bsputil_weightPoles (curve1.poles, curve1.poles, curve1.weights,
                             curve1.params.numPoles);

    /* Open second section at appropriate point */
    if (curve1.params.closed)
        {
        if (curve0.rational)
            minPt.Scale (*(curve0.poles), 1.0 / curve0.weights[0]);
        else
            minPt = curve0.poles[0];
        bsprcurv_minDistToCurve (&dist0, &minPt, &openAtParam, &minPt, &curve1, NULL, NULL);
        if (SUCCESS != (status = bspcurv_openCurve (&curve1, &curve1, openAtParam)) ||
            SUCCESS != (status = bspcurv_openCurve (section1, section1, openAtParam)))
            goto wrapup;
        bspcurv_closeCurve (&curve1, &curve1);
        }

#if defined (CHECK_SKIN_DIRECTION)
    /* Make sure sections are running the same direction */
    tan0.NormalizedDifference (*(curve0.poles), *(curve0.poles+1));
    tan1.NormalizedDifference (*(curve1.poles), *(curve1.poles+1));
    dot = tan0.DotProduct (tan1);
    dist0 = curve0.poles->Distance (*(curve1.poles));
    dist1 = curve0.poles->Distance (*(curve1.poles + curve1.params.numPoles - 1));
    if ((dot < 0.0) || (dist1 < dist0))
        {
        bspcurv_reverseCurve (&curve1, &curve1);
        bspcurv_reverseCurve (section1, section1);
        }
#endif

    if (SUCCESS != (status = bspcurv_make2CurvesCompatible (&curve0, &curve1)))
        goto wrapup;

    /* Set the surface parameters and allocate the surface */
    k = trace0->params.order;
    l = curve0.params.order;
    n = trace0->params.numPoles;
    m = curve0.params.numPoles;

    surface->rational = (rationalSurface = curve0.rational || trace0->rational);
    surface->uParams  = curve0.params;
    surface->vParams  = trace0->params;

    if (bspsurf_allocateSurface (surface))
        return ERROR;
    poles = surface->poles;
    weights = surface->weights;

    /* Get Greville abscissa of the trace curve */
    knotTolerance = bspknot_knotTolerance (trace0);
    bspknot_computeGrevilleAbscissa (nodes, trace0->knots, trace0->params.numPoles,
                           trace0->params.order, trace0->params.closed, knotTolerance);

    /* Un-weight poles before blending */
    if (curve0.rational)
        bsputil_unWeightPoles (curve0.poles, curve0.poles, curve0.weights,
                               curve0.params.numPoles);
    if (curve1.rational)
        bsputil_unWeightPoles (curve1.poles, curve1.poles, curve1.weights,
                               curve1.params.numPoles);

    /* Calculate the poles and weights (if rational):
        Loop over the nodes of the trace.  For each value, blend curve0 &
        curve1, and align thess blended poles with the Frenet frame of the
        trace evaluated at the current node.
        Form a translational surface with the trace; i.e., add
        trace0->poles[i], multiply trace0->weights[i] (if rational). */


#if ! defined (OLD_FLIP_ALGORITHUM)
    if (trace1 == NULL && checkInflectionPt)
        {
        double      twistAngle;
        DVec3d      orientX, orientY;
        RotMatrix   twistMatrix;

        for (i=0; i < n; i++)
            {
            bspcurv_frenetFrame (traceDervs, NULL, NULL, NULL, trace0, nodes[i], NULL);
            if (i)
                bspdsurf_updateTubeMatrix (&rotMatrix, &rotMatrix, false, &traceDervs[0]);
            else
                rotMatrix.InitFromRowVectors (traceDervs[1], traceDervs[2], *traceDervs);
            }
        /* At this point, rotMatrix represents the end orientation of the section
           and traceDervs[] represents the original frenet frame */

        bsiRotMatrix_getRow ( &rotMatrix, &orientX,  0);
        bsiRotMatrix_getRow ( &rotMatrix, &orientY,  1);
        twistAngle = Angle::Atan2 (traceDervs[1].DotProduct (orientY),
                            traceDervs[1].DotProduct (orientX));

        bsiRotMatrix_rotateByPrincipleAngles (&twistMatrix, NULL, 0.0, 0.0, twistAngle);
        twistMatrix.Multiply (curve1.poles, curve1.poles, curve1.params.numPoles);
        }
#endif



    for (i=0; i < n; i++)
        {
        blend = nodes[i];
        blendM1 = 1.0 - blend;
        vWtsi = trace0->rational ? trace0->weights[i] : 1.0;
        bsiDPoint3d_scale (&vPolesi, trace0->poles+i, (factor = 1.0 / vWtsi));
        for (j=0; j < m; j++)
            {
            uPoles0j = curve0.poles[j];
            uPoles1j = curve1.poles[j];
            index = i*m + j;

            /* Blend curve0 & curve1 */
            poles[index].x = blendM1*uPoles0j.x + blend*uPoles1j.x;
            poles[index].y = blendM1*uPoles0j.y + blend*uPoles1j.y;
            poles[index].z = blendM1*uPoles0j.z + blend*uPoles1j.z;
            if (rationalSurface)
                {
                uWts0j = curve0.rational ? curve0.weights[j] : 1.0;
                uWts1j = curve1.rational ? curve1.weights[j] : 1.0;
                weights[index]  = (blendM1*uWts0j + blend*uWts1j) * vWtsi;
                }

            /* Scale to compensate for segment weights */
            if (!MSBsplineCurve::AreSameWeights (factor, 1.0))
                poles[index].x *= factor;
            }

        /* Position the last m values of poles to get the poles of the
           translational surface at this node */
        /* Calculate orientation frame at this node */
        if (trace1)
            {
            bspcurv_computeCurvePoint (traceDervs+1, traceDervs, &weight, nodes[i],
                                       trace0->poles, trace0->params.order,
                                       trace0->params.numPoles, trace0->knots,
                                       trace0->weights, trace0->rational,
                                       trace0->params.closed);
            if (trace0->rational)
                traceDervs[1].Scale (traceDervs[1], weight);

            bspcurv_computeCurvePoint (traceDervs+2, NULL, &weight, nodes[i],
                                       trace1->poles, trace1->params.order,
                                       trace1->params.numPoles, trace1->knots,
                                       trace1->weights, trace1->rational,
                                       trace1->params.closed);
            if (trace1->rational)
                traceDervs[2].Scale (traceDervs[2], weight);

            traceDervs->Normalize ();
            traceDervs[1].NormalizedDifference (traceDervs[2], traceDervs[1]);
            traceDervs[2].CrossProduct (*traceDervs, traceDervs[1]);
            traceDervs[1].CrossProduct (traceDervs[2], *traceDervs);
            }
        else
            {
            bspcurv_frenetFrame (traceDervs, NULL, NULL, NULL, trace0, nodes[i], NULL);

            if (checkInflectionPt)
                {
#if !defined (OLD_FLIP_ALGORITHUM)
                if (i)
                    {
                    bspdsurf_updateTubeMatrix (&rotMatrix, &rotMatrix, false, &traceDervs[0]);
                    bsiRotMatrix_getRow ( &rotMatrix, traceDervs + 1,  0);
                    bsiRotMatrix_getRow ( &rotMatrix, traceDervs + 2,  1);
                    }
#else
                /* Test to prevent surface from reversing its orientation */
                if (i == 0)
                    lastBvector = traceDervs[2];
                else
                    {
                    if (lastBvector.DotProduct (traceDervs[2]) < 0.0)
                        {
                        traceDervs[1].Scale (traceDervs[1], -1.0);
                        traceDervs[2].Scale (traceDervs[2], -1.0);
                        }
                    lastBvector = traceDervs[2];
                    }
#endif
                }
            }

        rotMatrix.InitFromRowVectors (traceDervs[1], traceDervs[2], *traceDervs);

        /* Rotate blended poles at origin */
        rotMatrix.MultiplyTranspose (poles+i*m, poles+i*m, m);

        /* Translate into place */
        DPoint3d::AddToArray (poles+i*m, m, vPolesi);
        }

    /* Set the number of rule lines to the number of poles in the opposition
        direction */
    surface->uParams.numRules = surface->vParams.numPoles;
    surface->vParams.numRules = surface->uParams.numPoles;

    /* Load knot vectors */
    memcpy (surface->vKnots, trace0->knots,
            bspknot_numberKnots (n, k, trace0->params.closed) * sizeof(double));
    memcpy (surface->uKnots, curve0.knots,
            bspknot_numberKnots (m, l, curve0.params.closed) * sizeof(double));

    if (rationalSurface)
        bsputil_weightPoles (poles, poles, weights, n*m);

    surface->display.curveDisplay = true;

wrapup:
    bspcurv_freeCurve (&curve0);
    bspcurv_freeCurve (&curve1);
    if (nodes)      msbspline_free (nodes);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             02/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspsurf_skinSurface
(
MSBsplineSurface    *surface,
MSBsplineCurve      *trace,
MSBsplineCurve      *trace1,       /* => second trace, or NULL */
MSBsplineCurve      **sections,    /* => pre-aligned in the xy plane */
double              *params,
int                 numSections
)
    {
    int                 status = ERROR, i, allClosed, flipped;
    double              *u0, *u1;
    DPoint3d            site;
    DVec3d              frame[3];
    DSegment3d           dVec;
    RotMatrix           rMA;
    MSBsplineCurve      segment, segment1, *seg1Ptr, *x0, *x1, tmp1, tmp2;
    MSBsplineSurface    patch, tmpSurf;

    if (numSections > MAX_POLES)
        return (ERROR);
    if (numSections < 2)
        return ERROR;

    memset (&tmpSurf, 0, sizeof(tmpSurf));
    memset (&segment, 0, sizeof(segment));
    memset (&segment1, 0, sizeof(segment1));
    memset (&patch, 0, sizeof(patch));
    memset (&tmp1, 0, sizeof(tmp1));
    memset (&tmp2, 0, sizeof(tmp2));
    memset (&dVec, 0, sizeof(dVec));
    dVec.point[1].x = 1.0;

    seg1Ptr = trace1 ? &segment1 : NULL;

    for (i=0, allClosed = true; allClosed && (i < numSections); i++)
        allClosed = allClosed && sections[i]->params.closed;

    for (i=1, u0=params, u1=u0+1; i < numSections; i++, u0++, u1++)
        {
        x0 = sections[i-1];
        x1 = sections[i];
        if (SUCCESS != (status = bspsurf_prepareSkin (&segment, &segment1, x0, x1,
                                                      trace, trace1, *u0, *u1)))
            {
            if (!(MSBsplineCurve::AreSameKnots (*u0, *u1)))
                goto wrapup;

            /* make ruled surface at site */
            if (SUCCESS != (status = bspcurv_copyCurve (&tmp1, x0)) ||
                SUCCESS != (status = bspcurv_copyCurve (&tmp2, x1)))
                goto wrapup;

            bspcurv_frenetFrame (frame, &site, NULL, NULL, trace, *u0, NULL);
            rMA.InitFromRowVectors (frame[1], frame[2], *frame);

            /* align copies of sections at site */
            if (tmp1.rational)
                bsputil_unWeightPoles (tmp1.poles, tmp1.poles, tmp1.weights,
                                       tmp1.params.numPoles);
            rMA.MultiplyTranspose (tmp1.poles, tmp1.poles, tmp1.params.numPoles);
            DPoint3d::AddToArray (tmp1.poles, tmp1.params.numPoles, site);
            if (tmp1.rational)
                bsputil_weightPoles (tmp1.poles, tmp1.poles, tmp1.weights,
                                     tmp1.params.numPoles);
            if (tmp2.rational)
                bsputil_unWeightPoles (tmp2.poles, tmp2.poles, tmp2.weights,
                                       tmp2.params.numPoles);
            rMA.MultiplyTranspose (tmp2.poles, tmp2.poles, tmp2.params.numPoles);
            DPoint3d::AddToArray (tmp2.poles, tmp2.params.numPoles, site);
            if (tmp2.rational)
                bsputil_weightPoles (tmp2.poles, tmp2.poles, tmp2.weights,
                                     tmp2.params.numPoles);

            if (SUCCESS != (status = bspcurv_make2CurvesCompatible (&tmp1, &tmp2)) ||
                SUCCESS != (status = bspsurf_ruledSurface (&patch, &tmp1, &tmp2)))
                goto wrapup;
            bspcurv_freeCurve (&tmp1);
            bspcurv_freeCurve (&tmp2);
            }
        else
            {
            /* make skin patch from sections */
            if (SUCCESS !=
                (status = bspsurf_skinPatch (&patch, &segment, seg1Ptr, x0, x1,
                                             &dVec, &dVec, (trace1 ? false : true))))
                goto wrapup;

            /* Check for flipping in last section */
            if (!seg1Ptr)
                {
                bspcurv_inflectionPoints (NULL, &flipped, &segment);
                if (flipped % 2)
                    {
                    rMA.InitIdentity ();
                    rMA.form3d[0][0] = -1.0 ;
                    rMA.Multiply (x1->poles, x1->poles, x1->params.numPoles);
                    }
                }
            }

        if (i == 1)
            {
            if (SUCCESS != (status = bspsurf_copySurface (surface, &patch)))
                goto wrapup;
            }
        else
            {
            if (SUCCESS != (status = bsprsurf_appendSurfaces (&tmpSurf, surface, &patch,
                                                              BSSURF_U, true, true)))
                goto wrapup;
            bspsurf_freeSurface (surface);
            *surface = tmpSurf;
            memset (&tmpSurf, 0, sizeof(tmpSurf));
            }

        bspsurf_freeSurface (&patch);
        bspcurv_freeCurve (&segment);
        bspcurv_freeCurve (&segment1);
#if defined (debug_skin)
    {
    ElementDescr  *edP;
    bspsurf_createSurface (&edP, NULL, surface);
    mdlElmdscr_display (edP,0,0);
    mdlElmdscr_freeAll (&edP);
    }
#endif
        }

    if (allClosed)
        status = bsprsurf_closeSurface (surface, surface, BSSURF_U);

    surface->display.curveDisplay = true;
    surface->uParams.numRules = surface->vParams.numPoles;
    surface->vParams.numRules = surface->uParams.numPoles;

wrapup:
    bspcurv_freeCurve (&tmp1);
    bspcurv_freeCurve (&tmp2);
    bspcurv_freeCurve (&segment);
    bspcurv_freeCurve (&segment1);
    bspsurf_freeSurface (&patch);
    bspsurf_freeSurface (&tmpSurf);

    return status;
    }
#endif
#ifdef compile_bsproll
/*----------------------------------------------------------------------+
|                                                                       |
|   Rolling Ball Fillet routines                                        |
|                                                                       |
+----------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------**//**
* Calculates the poles for the arc section of the specified fillet. !!! ORDER OF CALCUALTION IS CRITICAL !!! !!! SEE COMMENT WHERE CALLED
* BELOW !!!
* @bsimethod                                                    Brian.Peters    03/93
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bsproll_calculateArcPoles
(
DPoint3d        *org,
DPoint3d        *mid,
DPoint3d        *end,
DPoint3d        *root,
DPoint3d        *norm0,
DPoint3d        *norm1
)
    {
    DPoint3d    cross, tan0, tan1;
    DPoint3d approach2;

    tan0.NormalizedDifference (*root, *norm0);
    tan1.NormalizedDifference (*root, *norm1);
    cross.CrossProduct (tan0, tan1);
    cross.Normalize ();

    tan0.CrossProduct (tan0, cross);
    tan1.CrossProduct (tan1, cross);
    *org = *norm0;
    *end = *norm1;

    if (SUCCESS != SegmentSegmentClosestApproachPoints (
                            *mid, approach2,
                            *org, tan0, *end, tan1))
        mid->Interpolate (*org, 0.5, *end);
    }

/*---------------------------------------------------------------------------------**//**
* Replace the point arrays in the PointLists with arrays holding two more points each (from c2Cubic fit). The number of list does not change.
* @bsimethod                                                    Brian.Peters    08/93
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsproll_generateArcPoles
(
PointList       *ptList,               /* <= middle pole of arc, => center of arc */
PointList       *n0List,               /* <=> first pole of arc */
PointList       *n1List,               /* <=> last pole of arc */
int             numList,               /* => number of list */
int             (*rhoFunc) (),         /* => varying rho function, or NULL */
void            *userDataP             /* => arguments passed through to rhoFunc */
)
    {
    int             status = ERROR, allocSize, numPoles, numKnots, numPoints;
    DPoint3d        *endP, *pP, *n0P, *n1P, *arc0P, *arc1P, *arc2P;
    PointList       *plP, *n0lP, *n1lP, *endListP;
    BsplineParam    bsplineParams;


    arc0P = arc1P = arc2P = NULL;

    for (plP = endListP = ptList, endListP += numList, n0lP = n0List, n1lP = n1List;
         plP < endListP; plP++, n0lP++, n1lP++)
        {
        /* Check to make sure data is present */
        if (NULL == n0lP->points || NULL == n1lP->points)
            {
            status = ERROR;
            goto wrapup;
            }

        /* Calculate the poles for the filleting arc in the same arrays that hold the
            SSI data. This is ok because of the order of calculations in
            bsproll_calculateArcPoles. */
        numPoints = plP->numPoints;
        numPoles  = plP->numPoints + 2;
        numKnots  = plP->numPoints - 2;

        for (pP = endP = plP->points, endP += numPoints,
             n0P = n0lP->points,
             n1P = n1lP->points;
             pP < endP; pP++, n0P++, n1P++)
            {
            bsproll_calculateArcPoles (n0P, pP, n1P, pP, n0P, n1P);
            }

#if defined (debug_pre)
        /* The arrays in the point lists hold the poles of a series of arcs for
            all the points of SSI. This would essentialy give a 3x2 order fillet
            surface; i.e. a series of ruled surfaces betweens filleting arcs. */
        {
        DPoint3d        arcPts[3];
        ElementUnion    arc;

        for (pP  = plP->points, endP = pP + plP->numPoints,
             n0P = n0lP->points,
             n1P = n1lP->points;
             pP < endP; pP++, n0P++, n1P++)
            {
            arcPts[0] = *n0P;
            arcPts[1] = *pP;
            arcPts[2] = *n1P;
            mdlLineString_create (&arc, NULL, arcPts, 3);
            displayElement (&arc, HILITE, -1L);
            }
        }
#endif
        /* Allocate memory to hold c2Cubic fitting poles for these data so that the
            filleting surface will be smooth along the direction of the arc. */
        allocSize = numPoles * sizeof(DPoint3d);
        if (NULL == (arc0P = (DPoint3d*)BSIBaseGeom::Malloc (allocSize)) ||
            NULL == (arc1P = (DPoint3d*)BSIBaseGeom::Malloc (allocSize)) ||
            NULL == (arc2P = (DPoint3d*)BSIBaseGeom::Malloc (allocSize)))
            {
            status = ERROR;
            goto wrapup;
            }

        bsplineParams.closed = false;
        bsplineParams.numKnots = numKnots;
        bsplineParams.numPoles = numPoles;
        if (SUCCESS != (status = bspcurv_c2CubicInterpolatePoles (arc0P, NULL, NULL, NULL,
                                 n0lP->points, NULL, NULL, &bsplineParams, numPoints)) ||
            SUCCESS != (status = bspcurv_c2CubicInterpolatePoles (arc1P, NULL, NULL, NULL,
                                 plP->points, NULL, NULL,  &bsplineParams, numPoints)) ||
            SUCCESS != (status = bspcurv_c2CubicInterpolatePoles (arc2P, NULL, NULL, NULL,
                                 n1lP->points, NULL, NULL, &bsplineParams, numPoints)))
            goto wrapup;

        /* Replace the original arrays in the PointLists with the larger, smooth,
            c2Cubic fit data. */
        BSIBaseGeom::Free (n0lP->points);
        BSIBaseGeom::Free (plP->points);
        BSIBaseGeom::Free (n1lP->points);

        n0lP->points = arc0P;
        plP->points  = arc1P;
        n1lP->points = arc2P;
        arc0P = arc1P = arc2P = NULL;

        n0lP->numPoints = plP->numPoints = n1lP->numPoints = numPoles;

#if defined (debug_post)
        {
        DPoint3d        arcPts[3];
        ElementUnion    arc;

        for (pP  = plP->points, endP = pP + plP->numPoints,
             n0P = n0lP->points,
             n1P = n1lP->points;
             pP < endP; pP++, n0P++, n1P++)
            {
            arcPts[0] = *n0P;
            arcPts[1] = *pP;
            arcPts[2] = *n1P;
            mdlLineString_create (&arc, NULL, arcPts, 3);
            displayElement (&arc, HILITE, -1L);
            }
        }
#endif
        } /* for all point lists */

wrapup:
    if (arc0P)      BSIBaseGeom::Free (arc0P);
    if (arc1P)      BSIBaseGeom::Free (arc1P);
    if (arc2P)      BSIBaseGeom::Free (arc2P);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    08/93
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  int     bsproll_extractListsFromData
(
PointList           **ptList,          /* <=> OSIC points */
PointList           **n0List,          /* <=> normals to surface0 */
PointList           **n1List,          /* <=> normals to surface1 */
int                 *numList,          /* <=> number of lists returned */
BsurfBoundary       **bnd0,            /* <=> trim boiundaries for surface0, or NULL */
int                 *numBnd0,          /* <=> number of trim boundaries for surface0 */
BsurfBoundary       **bnd1,            /* <=> trim boiundaries for surface1, or NULL */
int                 *numBnd1,          /* <=> number of trim boundaries for surface1 */
IntLink             *chainP,           /* => data adjusted for varying radius */
MSBsplineSurface    *surface0,         /* => first surface */
MSBsplineSurface    *surface1,         /* => second surface */
SsiTolerance        *ssiTolP           /* => tolerance used in SSI calculations */
)
    {
    int                 status;
    IntLink             *uv0ChnP, *uv1ChnP;

    uv0ChnP = uv1ChnP = NULL;

    if (bnd0 &&
        (SUCCESS != (status = bspssi_extractLinks (&uv0ChnP, chainP, CODE_UV0,
                                                   ssiTolP, NULL, NULL)) ||
         SUCCESS != (status = bspssi_assignLinks ((void **) bnd0, numBnd0,
                                                  &uv0ChnP, CODE_UV0, ssiTolP))))
        goto wrapup;

    if (bnd1 &&
        (SUCCESS != (status = bspssi_extractLinks (&uv1ChnP, chainP, CODE_UV1,
                                                   ssiTolP, NULL, NULL)) ||
         SUCCESS != (status = bspssi_assignLinks ((void **) bnd1, numBnd1,
                                                  &uv1ChnP, CODE_UV1, ssiTolP))))
        goto wrapup;

    if (SUCCESS != (status = bspssi_extractPointLists (ptList, n0List, n1List,
                                                       numList, &chainP,
                                                       ssiTolP, surface0, surface1,
                                                       NULL, NULL)))
        goto wrapup;

wrapup:
    if (status)
        {
        if (*numList != 0)    bsputil_freePointList (ptList, *numList);
        if (*numList != 0)    bsputil_freePointList (n0List, *numList);
        if (*numList != 0)    bsputil_freePointList (n1List, *numList);
        if (*numBnd0 != 0)    bsputil_freeBoundary (bnd0, *numBnd0);
        if (*numBnd1 != 0)    bsputil_freeBoundary (bnd1, *numBnd1);
        }
    bspssi_freeLink (&uv0ChnP);
    bspssi_freeLink (&uv1ChnP);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* notation form B.K. Choi page 277.
* @bsimethod                                                    Brian.Peters    08/93
+---------------+---------------+---------------+---------------+---------------+------*/
static void    pntutil_intersect3Planes
(
DPoint3d        *G,
DPoint3d        *m,
DPoint3d        *n,
DPoint3d        *f,
DPoint3d        *fo,
DPoint3d        *go
)
    {
    double          a, b, c, divisor;
    DPoint3d        p, nxp, pxm, mxn;    /* 'x' denotes cross product */

    mxn.CrossProduct (*m, *n);
    p = mxn;
    p.Normalize ();

    a = m->DotProduct (*fo);
    b = n->DotProduct (*go);
    c = p.DotProduct (*f);

    nxp.CrossProduct (*n, p);
    pxm.CrossProduct (p, *m);

    G->Scale (nxp, a);
    G->SumOf (*G, pxm, b);
    G->SumOf (*G, mxn, c);

    divisor = m->DotProduct (nxp);
    if (divisor > fc_1em15)
        G->Scale (*G, 1.0 / divisor);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    08/93
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bsproll_relaxToOSIC
(
DPoint3d            *xyz,
DPoint3d            *norm0,
DPoint3d            *norm1,
DPoint2d            *uv0,
DPoint2d            *uv1,
double              radius,
double              avgRadius,
Evaluator           *eval0,
Evaluator           *eval1,
int                 iterations,
double              convTol
)
    {
    int             count;
    double          deltaRad, dist, degen;
    DPoint3d        f, foff, goff;

    if (fabs (deltaRad = avgRadius - radius) < fc_1em15)
        {
        /* Put the normal point lists on the surfaces at the contact points. */
        norm0->Normalize ();
        norm1->Normalize ();

        norm0->SumOf (*xyz, *norm0, - eval0->distance);
        norm1->SumOf (*xyz, *norm1, - eval1->distance);
        return STATUS_CONVERGED;
        }

    norm0->Normalize ();
    norm1->Normalize ();
    f.SumOf (*xyz, *norm0, -avgRadius);
    foff = goff = *xyz;

    for (count = 0; count < iterations; count++)
        {
        pntutil_intersect3Planes (xyz, norm0, norm1, &f, &foff, &goff);

        /* xyz is a guess point for the variable radius OSIC.
            Now relax it to the two surfaces. */

        bspssi_relaxToSurface (&foff, norm0, &degen, uv0, xyz, eval0, iterations, 1e-7);
        bspssi_relaxToSurface (&goff, norm1, &degen, uv1, xyz, eval1, iterations, 1e-7);


#if defined (debug_osic)
        {
        DPoint3d        line[3];
        ElementUnion    u;

        line[0] = foff;
        line[1] = *xyz;
        line[2] = goff;

        mdlLineString_create (&u, NULL, line, 3);
        displayElement (&u, HILITE, -1L);
        }
#endif
        if ((dist = foff.Distance (goff)) < convTol)
            {
            /* Put the normal point lists on the surfaces at the contact points. */
            xyz->Interpolate (foff, 0.5, goff);
            norm0->Normalize ();
            norm1->Normalize ();

            norm0->SumOf (*xyz, *norm0, - eval0->distance);
            norm1->SumOf (*xyz, *norm1, - eval1->distance);
            return STATUS_CONVERGED;
            }

        norm0->Normalize ();
        norm1->Normalize ();

        f.SumOf (foff, *norm0, -eval0->distance);
        }

    /* Failed to converge within iterations */
    return STATUS_NONCONVERGED;
    }

typedef int (*PFDoublePDoubleVoidP)(double*,double,void*);
/*---------------------------------------------------------------------------------**//**
* see Choi, B.K. Surface Modeling for CAD/CAM, pp275-278.
* The data comes in in the IntLink as follows: xyz = the actual intersection point at avgRadius norm0 = normal direction of eval0 (NOT
* normalized) norm1 = normal direction of eval1 (NOT normalized) uv0 = parameters of xyz on eval0->surface uv1 = parameters of xyz on
* eval1->surface
* The data goes out in the IntLink as follows: xyz = the actual ball center point norm0 = ball contact point on eval0->surface norm1 = ball
* contact point on eval1->surface uv0 = parameters of norm0 on eval0->surface uv1 = parameters of norm1 on eval1->surface
* @bsimethod                                                    Brian.Peters    08/93
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsproll_adjustOSIC
(
IntLink         *chain,                /* <=> full OSIC data */
Evaluator       *eval0,                /* => first surface */
Evaluator       *eval1,                /* => second surface */
int             (*radFunc) (),         /* => varying radius function, or NULL */
void            *userDataP,            /* => arguments passed trhough to radFunc */
double          avgRadius,             /* => average radius */
double          tolerance              /* => 3d convergence tolerance */
)
    {
    int             status;
    double          radius, u, incr;
    DPoint2d        *uv0P, *uv1P;
    DPoint3d        *endP, *xyzP, *n0P, *n1P;
    IntLink         *linkP;
    PFDoublePDoubleVoidP radFunc1 = (PFDoublePDoubleVoidP)radFunc;
    for (linkP = chain; linkP; linkP = linkP->next)
        {
        if (radFunc)
            {
            /* Adjust according to the radius function */
            incr = 1.0 / (linkP->number - 1);
            for (xyzP = endP = linkP->xyz, endP += linkP->number,
                 uv0P = linkP->uv0,          uv1P = linkP->uv1,
                 n0P  = linkP->norm0,    n1P  = linkP->norm1,
                 u = 0.0;
                 xyzP < endP; xyzP++, n0P++, n1P++, uv0P++, uv1P++, u += incr)
                {
                if (SUCCESS != (status = radFunc1(&radius, u, userDataP)))
                    return status;

                eval0->distance = eval1->distance = radius;
                bsproll_relaxToOSIC (xyzP, n0P, n1P, uv0P, uv1P, radius, avgRadius,
                                     eval0, eval1, 10, tolerance);
                }
            }
        else
            {
            /* Offset the norm0 & norm1 data to the actual surfaces. */
            if (eval0->offset)
                {
                if (eval1->offset)
                    {
                    for (xyzP = endP = linkP->xyz, endP += linkP->number,
                         n0P  = linkP->norm0, n1P  = linkP->norm1;
                         xyzP < endP; xyzP++, n0P++, n1P++)
                        {
                        n0P->SumOf (*xyzP, *n0P, - eval0->distance);
                        n1P->SumOf (*xyzP, *n1P, - eval1->distance);
                        }
                    }
                else
                    {
                    for (xyzP = endP = linkP->xyz, endP += linkP->number,
                         n0P  = linkP->norm0;
                         xyzP < endP; xyzP++, n0P++)
                        n0P->SumOf (*xyzP, *n0P, - eval0->distance);
                    }
                }
            else
                {
                for (xyzP = endP = linkP->xyz, endP += linkP->number,
                     n1P  = linkP->norm1;
                     xyzP < endP; xyzP++, n1P++)
                    n1P->SumOf (*xyzP, *n1P, - eval1->distance);
                }
            }
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    09/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bsproll_generateRawFilletData
(
IntLink             **chainPP,         /* <= full OSIC info for avgRadius offsets */
MSBsplineSurface    *surface0,         /* => first surface */
MSBsplineSurface    *surface1,         /* => second surface */
double              radius0,           /* => offset for surface0 */
double              radius1,           /* => offset for surface1 */
double              tolerance,         /* => tolerance to use in calculation */
double              uor_res            /* => UOR resolution */
)
    {
    int                 status, save0, save1;
    Evaluator           eval0, eval1;
    SsiTolerance        ssiTolSave;

    memset (&eval0, 0, sizeof (Evaluator));
    memset (&eval1, 0, sizeof (Evaluator));

    eval0.surf = surface0;
    eval1.surf = surface1;
    eval0.offset = eval1.offset = true;

    save0 = surface0->numBounds;
    save1 = surface1->numBounds;
    surface0->numBounds = surface1->numBounds = 0;

    status = bspssi_intersectSurfaces (NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                                       chainPP, &ssiTolSave, surface0, surface1,
                                       tolerance, uor_res,
#if defined (debug)
                                       true, radius0, radius1);
#else
                                       false, radius0, radius1);
#endif

    surface0->numBounds = save0;
    surface1->numBounds = save1;

#if defined (debug_raw)
    {
    int                     addCurves=false;
    MSBsplineCurve          curve;
    ElementDescr            *edP=NULL;
    IntLink                 *linkP;

    memset (&curve, 0, sizeof(MSBsplineCurve));
    curve.display.curveDisplay =
    curve.display.polygonDisplay = true;
    curve.params.order = 2;

    for (linkP = *chainPP; linkP; linkP = linkP->next)
        {
        curve.params.numPoles = linkP->number;
        curve.poles               = linkP->xyz;

        bspcurv_createCurve (&edP, NULL, &curve);
        mdlElmdscr_display (edP, MASTERFILE, HILITE);
        if (addCurves)
        mdlElmdscr_add (edP);
        mdlElmdscr_freeAll (&edP);
        }
    }
#endif
    return status;
    }
#endif
END_BENTLEY_GEOMETRY_NAMESPACE