/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/bspline/bspcurv.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "../DeprecatedFunctions.h"
#include "msbsplinemaster.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#define MDLERR_NOPOLES          ERROR
#define MDLERR_INSFMEMORY       ERROR
#define MDLERR_NOWEIGHTS        ERROR
#define MDLERR_BADKNOTS         ERROR
#define MDLERR_TOOFEWPOLES      ERROR
#define MDLERR_BADPOLES         ERROR
#define MDLERR_BADORDER         ERROR
#define MDLERR_BADPARAMETER     ERROR
#define MDLERR_BADPERIODICITY   ERROR

/*----------------------------------------------------------------------+
|                                                                       |
|   BSPCURV.C : B-Spline Curve Routines                                 |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|  Include Files                                                        |
|                                                                       |
+----------------------------------------------------------------------*/


/* tools subsystem */


/*----------------------------------------------------------------------+
|                                                                       |
|   Local defines                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
#define LINEAR_CURVATURE                       1.0e-12
#define NALLOC                                 256
#define MAX_ORDER_PLUS_1                       (MAX_ORDER+1)


/*----------------------------------------------------------------------+
|                                                                       |
|   MajorPublic GEOMDLLIMPEXP Code Section                                           |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
|    Curve Utility Routines                                             |
|                                                                       |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_freeCurve                                       |
|                                                                       |
| author        RBB                                     3/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bspcurv_freeCurve
(
MSBsplineCurve *curve
)
    {
    if (curve->poles)
        {
        BSIBaseGeom::Free (curve->poles);
        curve->poles = NULL;
        }

    if (curve->knots)
        {
        BSIBaseGeom::Free (curve->knots);
        curve->knots = NULL;
        }

    if (curve->rational && curve->weights)
        {
        BSIBaseGeom::Free (curve->weights);
        curve->weights = NULL;
        }    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_freeInterpolationCurve                          |
|                                                                       |
| author        LuHan                                   01/01           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bspcurv_freeInterpolationCurve
(
MSInterpolationCurve *curve
)
    {
    if (curve->fitPoints)
        {
        BSIBaseGeom::Free (curve->fitPoints);
        curve->fitPoints = NULL;
        }

    if (curve->knots)
        {
        BSIBaseGeom::Free (curve->knots);
        curve->knots = NULL;
        }
    }
 /*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_allocateCurve                                   |
|                                                                       |
| author        RBB                                     3/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_allocateCurve
(
MSBsplineCurve  *curve
)
    {
    int         allocSize;

    /* Initialize all pointers to NULL */
    curve->poles = NULL;    curve->knots = curve->weights = NULL;

    if (curve->params.numPoles <= 0)
        return MDLERR_NOPOLES;

    /* Allocate poles memory */
    allocSize = curve->params.numPoles * sizeof (DPoint3d);
    if (NULL == (curve->poles = static_cast<DPoint3d *>(dlmSystem_mdlMallocWithDescr (allocSize, pHeapDescr))))
        return MDLERR_INSFMEMORY;

    /* Allocate knot vector memory */
    allocSize = bspknot_numberKnots (curve->params.numPoles, curve->params.order,
                                     curve->params.closed) * sizeof (double);

    if (NULL == (curve->knots = static_cast<double *>(dlmSystem_mdlMallocWithDescr (allocSize, pHeapDescr))))
        {
        bspcurv_freeCurve (curve);
        return MDLERR_INSFMEMORY;
        }

    /* Allocate weights (if necessary) */
    if (curve->rational)
        {
        allocSize = curve->params.numPoles * sizeof(double);
        if (NULL == (curve->weights = static_cast<double *>(dlmSystem_mdlMallocWithDescr (allocSize, pHeapDescr))))
            {
            bspcurv_freeCurve (curve);
            return MDLERR_INSFMEMORY;
            }
        }

    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_allocateInterpolationCurve                      |
|                                                                       |
| author        LuHan                                   01/01           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_allocateInterpolationCurve
(
MSInterpolationCurve    *curve
)
    {
    int         allocSize;

    /* Initialize all pointers to NULL */
    curve->fitPoints = NULL;    curve->knots = NULL;

    /* Allocate poles memory */
    allocSize = curve->params.numPoints * sizeof (DPoint3d);
    if (NULL == (curve->fitPoints = static_cast<DPoint3d *>(dlmSystem_mdlMallocWithDescr (allocSize, pHeapDescr))))
        return MDLERR_INSFMEMORY;

    /* Allocate knot vector memory */
    allocSize = curve->params.numKnots * sizeof (double);
    if (NULL == (curve->knots = static_cast<double *>(dlmSystem_mdlMallocWithDescr (allocSize, pHeapDescr))))
        {
        bspcurv_freeInterpolationCurve (curve);
        return MDLERR_INSFMEMORY;
        }
    curve->display.curveDisplay = true;
    curve->display.polygonDisplay = false;
    return SUCCESS;
    }

 /*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_copyCurve                                       |
|                                                                       |
| author        RBB                                     3/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_copyCurve
(
MSBsplineCurveP  output,
MSBsplineCurveCP input
)
    {
    int         status, numPoles, numKnots;

    if (output == input)
        return SUCCESS;
    *output = *input;

    if (SUCCESS != (status = bspcurv_allocateCurve (output)))
        return status;

    numPoles = output->params.numPoles;
    if (output->poles != NULL && input->poles != NULL)          //Defect #7321
        memcpy (output->poles, input->poles, numPoles * sizeof (DPoint3d));

    if (output->rational)
        memcpy (output->weights, input->weights, numPoles * sizeof(double));
    else
        output->weights = NULL;

    numKnots = bspknot_numberKnots (output->params.numPoles, output->params.order,
                                    output->params.closed);

    if (input->knots)
        memcpy (output->knots, input->knots, numKnots * sizeof (double));
    else
        /* If knot vector is missing then create it */
        bspknot_computeKnotVector (output->knots, &output->params, NULL);

    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_copyInterpolationCurve                          |
|                                                                       |
| author        LuHan                                   01/01           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_copyInterpolationCurve
(
MSInterpolationCurve    *output,
MSInterpolationCurve    *input
)
    {
    int         status;

    if (output == input)
        return SUCCESS;
    *output = *input;

    if (SUCCESS != (status = bspcurv_allocateInterpolationCurve (output)))
        return status;

    memcpy (output->fitPoints, input->fitPoints, output->params.numPoints * sizeof (DPoint3d));
    memcpy (output->knots, input->knots, output->params.numKnots * sizeof (double));

    return SUCCESS;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_isPhysicallyClosedBCurve                        |
|                                                                       |
| author        LuHan                                   9/96            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool   bspcurv_isPhysicallyClosedBCurve
(
MSBsplineCurveCP    pCurve,        /* => input curve to be checked */
double              tolerance       /* => tolerance used to check dist in 3d space */
)
    {
    int         index;
    DPoint3d    sPole, ePole;

    index = pCurve->params.numPoles - 1;
    if (pCurve->rational)
        {
        sPole.Scale (*(&pCurve->poles[0]), 1.0 / pCurve->weights[0]);
        ePole.Scale (*(&pCurve->poles[index]), 1.0 / pCurve->weights[index]);
        }
    else
        {
        sPole = pCurve->poles[0];
        ePole = pCurve->poles[index];
        }

    return  (fabs(sPole.x - ePole.x) < tolerance &&
             fabs(sPole.y - ePole.y) < tolerance &&
             fabs(sPole.z - ePole.z) < tolerance);
    }




/*----------------------------------------------------------------------+
|                                                                       |
|   Curve Compatiblity Routines                                         |
|                                                                       |
+----------------------------------------------------------------------*/
#if !defined (FONTIMPORTER)

/*----------------------------------------------------------------------+
|                                                                       |
|   Curve Routines - acting on single curve                             |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_chainRule                                       |
|                                                                       |
| author        BFP                                     2/91            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void    bspcurv_chainRule
(
DPoint3d        *out,
DPoint3d        *inPoles,
double          *inWeights
)
    {
    double      dWt0, c0, c1, c2, c3;
    DPoint3d    dPoles[4], derivatives[4];

    dWt0 = inWeights[0];
    memcpy (dPoles, inPoles, sizeof(dPoles));

    derivatives[0].x = dPoles[0].x / dWt0;
    derivatives[0].y = dPoles[0].y / dWt0;
    derivatives[0].z = dPoles[0].z / dWt0;

    /* first derivative - apply chain rule */
    dWt0 *= inWeights[0];
    derivatives[1].x = ( dPoles[1].x * inWeights[0]
                           - dPoles[0].x * inWeights[1]) / dWt0;
    derivatives[1].y = ( dPoles[1].y * inWeights[0]
                           - dPoles[0].y * inWeights[1])        / dWt0;
    derivatives[1].z = ( dPoles[1].z * inWeights[0]
                           - dPoles[0].z * inWeights[1])        / dWt0;

    /* second derivative - apply chain rule */
    dWt0 *= inWeights[0];
    c0 = inWeights[1] * inWeights[1];
    c1 = inWeights[0] * inWeights[2];
    c2 = inWeights[0] * inWeights[1];
    c3 = inWeights[0] * inWeights[0];
    derivatives[2].x = (2*dPoles[0].x * c0 - dPoles[0].x * c1
                     -  2*dPoles[1].x * c2 + dPoles[2].x * c3) / dWt0;
    derivatives[2].y = (2*dPoles[0].y * c0 - dPoles[0].y * c1
                     -  2*dPoles[1].y * c2 + dPoles[2].y * c3) / dWt0;
    derivatives[2].z = (2*dPoles[0].z * c0 - dPoles[0].z * c1
                     -  2*dPoles[1].z * c2 + dPoles[2].z * c3) / dWt0;

    /* third derivative - apply chain rule */
    dWt0 *= inWeights[0];
    derivatives[3].x = (6*dPoles[0].x * c2 * inWeights[2]
                     -  6*dPoles[0].x * c0 * inWeights[1]
                     +    dPoles[0].x * c3 * inWeights[3]
                     +  6*dPoles[1].x * c0 * inWeights[0]
                     -  3*dPoles[1].x * c3 * inWeights[2]
                     -  3*dPoles[2].x * c3 * inWeights[1]
                     +    dPoles[3].x * c3 * inWeights[0]) / dWt0;
    derivatives[3].y = (6*dPoles[0].y * c2 * inWeights[2]
                     -  6*dPoles[0].y * c0 * inWeights[1]
                     +    dPoles[0].y * c3 * inWeights[3]
                     +  6*dPoles[1].y * c0 * inWeights[0]
                     -  3*dPoles[1].y * c3 * inWeights[2]
                     -  3*dPoles[2].y * c3 * inWeights[1]
                     +    dPoles[3].y * c3 * inWeights[0]) / dWt0;
    derivatives[3].z = (6*dPoles[0].z * c2 * inWeights[2]
                     -  6*dPoles[0].z * c0 * inWeights[1]
                     +    dPoles[0].z * c3 * inWeights[3]
                     +  6*dPoles[1].z * c0 * inWeights[0]
                     -  3*dPoles[1].z * c3 * inWeights[2]
                     -  3*dPoles[2].z * c3 * inWeights[1]
                     +    dPoles[3].z * c3 * inWeights[0]) / dWt0;

    memcpy (out, derivatives, sizeof(dPoles));
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_computeDerivatives                              |
|                                                                       |
| author        BFP                                     5/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_computeDerivatives
(
DPoint3d        *dervPoles,            /* must contain numDervs+1 Dpoint3ds */
double          *dervWts,              /* must contain numDervs+1 doubles */
MSBsplineCurveCP curve,
int             numDervs,              /* does not include point itself */
double          u,
int             leftHand               /* give left derivatives at knots */
)
    {
    int         i, j, degree, derivsToFind, left, rght, start;
    double      leftDiff[MAX_ORDER], rightDiff[MAX_ORDER], tmpWt, knotTol,
                minKnot=0.0, maxKnot=1.0, alpha, beta, denom, dconstant,
                *wP0, *wP1, *kP, dWts[MAX_ORDER];
    DPoint3d    tmpPole, *pP0, *pP1, dPoles[MAX_ORDER];
    int         numKnots;
    double      min, max;
    bool        bCorrect;

    if (curve->rational && !dervWts)
        return (MDLERR_NOWEIGHTS);

    mdlBspline_getParameterRange (&minKnot, &maxKnot, curve);
    numKnots = bspknot_numberKnots (curve->params.numPoles, curve->params.order, curve->params.closed);
    min = curve->knots[curve->params.order - 1];
    max = curve->knots[numKnots - curve->params.order];

    u = mdlBspline_fractionParameterToNaturalParameter (curve, u);

    if (!curve->params.closed)
        {
        if (u < min)
            u = min;
        else if (u > max)
            u = max;
        }
    else
        {
        double period = max - min;
        if (u < min)
            {
            while (u < min)
                u += period;
            }
        else if (u > max)
            {
            while (u > max)
                u -= period;
            }
        }

    degree = curve->params.order - 1;
    knotTol = bspknot_knotTolerance (curve);

    /* compensate for fake periodic knot vectors */
    bCorrect = curve->params.closed ? mdlBspline_curveShouldBeOpened (curve) : false;

    /* Zero out the function and all the derivatives */
    memset (dPoles, 0, (numDervs+1) * sizeof(DPoint3d));

    /* Initialize weights to one, weight derivatives to zero if non-rational but to zero if rational
        because they will be calculated below */
    memset (dWts, 0, (numDervs+1) * sizeof(double));
    if (!curve->rational)
        {
        *dWts = 1.0;
        for (i=1, wP0=dWts + 1; i <= numDervs; i++, wP0++)
            *wP0 = 0.0;
        }
    /* Load the proper poles and weights into output arrays,
        based on leftHand and u */
    for (kP=curve->knots + curve->params.order;
         minKnot >= *kP || *kP < u; kP++)
        ;
    left = (int)(kP - curve->knots - 1);

    for (kP=curve->knots + left; u >= *kP && *kP < maxKnot; kP++)
        ;
    rght = (int)(kP - curve->knots);

    start = leftHand ? left - degree : rght - curve->params.order;
    if (bCorrect)
        start -= curve->params.order/2;

    bsputil_loadPoles (dPoles, dWts, curve->poles, curve->weights,
                       start, curve->params.numPoles, curve->params.order,
                       curve->params.closed, curve->rational);

    /* Only degree derivatives are non-zero */
    derivsToFind = (numDervs > degree) ? degree : numDervs;

    /* Compute differences */
    if (bCorrect)
        start += curve->params.order/2;
    for (i=1; i < curve->params.order; i++)
        {
        leftDiff[i]  = u - curve->knots[start + i];
        rightDiff[i] = curve->knots[start + degree + i] - u;
        }

    /* Branch depending on whether we are approaching this parameter value
        from the right or from the left side */
    if (rightDiff[1] > leftDiff[degree])              /* from right? */
        {
        /* Compute the point */
        for (j=1; j < curve->params.order; j++)
            for (i=0, pP0=dPoles, pP1=pP0+1, wP0=dWts, wP1=wP0+1;
                 i <= (degree - j); i++, pP0++, pP1++, wP0++, wP1++)
                {
                alpha = leftDiff[i + j];
                beta = rightDiff[i + 1];
                denom = alpha + beta;
                if (denom < knotTol)
                    return (MDLERR_BADKNOTS);
                pP0->x = (alpha * pP1->x + beta * pP0->x) / denom;
                pP0->y = (alpha * pP1->y + beta * pP0->y) / denom;
                pP0->z = (alpha * pP1->z + beta * pP0->z) / denom;
                if (curve->rational)
                    *wP0 = (alpha* *wP1 + beta* *wP0)/denom;
                }

        /* Compute the derivatives */
        for (j=1; j <= derivsToFind; j++)
            {
            dconstant = degree - j + 1;
            for (i = derivsToFind,
                 pP0=dPoles+i, pP1=pP0-1, wP0=dWts+i, wP1=wP0-1;
                 i >= j; i--, pP0--, pP1--, wP0--, wP1--)
                {
                denom = rightDiff[i - j + 1] / dconstant;
                if (denom < knotTol)
                    return (MDLERR_BADKNOTS);

                pP0->x = (pP0->x - pP1->x) / denom;
                pP0->y = (pP0->y - pP1->y) / denom;
                pP0->z = (pP0->z - pP1->z) / denom;
                if (curve->rational)
                    *wP0 = (*wP0 - *wP1) / denom;
                }
            }
        }
    else                               /* from left */
        {
        /* Reverse poles and weights */
        for (i=0, j=degree/2+1, pP0=dPoles, pP1=dPoles+degree;
             i < j; i++, pP0++, pP1--)
            {
            tmpPole = *pP1; *pP1 = *pP0; *pP0 = tmpPole;
            }
        if (curve->rational)
            {
            for (i=0, wP0=dWts, wP1=dWts+degree; i<j; i++, wP0++, wP1--)
                {
                tmpWt = *wP1; *wP1 = *wP0; *wP0 = tmpWt;
                }
            }

        /* Compute the point */
        for (j=1; j < curve->params.order; j++)
            for (i=0, pP0=dPoles, pP1=pP0+1, wP0=dWts, wP1=wP0+1;
                 i <= (degree - j); i++, pP0++, pP1++, wP0++, wP1++)
                {
                alpha = leftDiff[degree - i];
                beta = rightDiff[degree - j + 1 - i];
                denom = alpha + beta;
                if (denom < knotTol)
                    return (MDLERR_BADKNOTS);

                pP0->x = (alpha * pP0->x + beta * pP1->x) / denom;
                pP0->y = (alpha * pP0->y + beta * pP1->y) / denom;
                pP0->z = (alpha * pP0->z + beta * pP1->z) / denom;
                if (curve->rational)
                    *wP0 = (alpha * *wP0 + beta * *wP1)/denom;
                }

        /* Compute the derivatives */
        for (j=1; j <= derivsToFind; j++)
            {
            dconstant = degree - j + 1;
            for (i = derivsToFind,
                 pP0=dPoles+i, pP1=pP0-1, wP0=dWts+i, wP1=wP0-1;
                 i >= j; i--, pP0--, pP1--, wP0--, wP1--)
                {
                denom = leftDiff[degree + j - i] / dconstant;
                if (denom < knotTol)
                    return (MDLERR_BADKNOTS);

                pP0->x = (pP1->x - pP0->x) / denom;
                pP0->y = (pP1->y - pP0->y) / denom;
                pP0->z = (pP1->z - pP0->z) / denom;
                if (curve->rational)
                    *wP0 = (*wP1 - *wP0) / denom;
                }
            }
        }

    if (dervPoles)
        memcpy (dervPoles, dPoles, (numDervs+1) * sizeof(DPoint3d));

    if (dervWts)
        memcpy (dervWts, dWts, (numDervs+1) * sizeof(double));

    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_getTangent                                      |
|                                                                       |
| author        BFP                                     8/91            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_getTangent
(
DPoint3d        *tangent,
double          param,
MSBsplineCurve  *curve,
int             leftHand
)
    {
    int         status;
    double      dWeights[2], dWtSqrd;
    DPoint3d    dPoles[2];

    if (SUCCESS != (status = bspcurv_computeDerivatives (dPoles, dWeights, curve, 1,
                                                         param, leftHand)))
        return status;

    if (curve->rational)
        {
        dWtSqrd = dWeights[0] * dWeights[0];
        tangent->x = (dPoles[1].x * dWeights[0]
                            - dPoles[0].x * dWeights[1]) / dWtSqrd;
        tangent->y = (dPoles[1].y * dWeights[0]
                            - dPoles[0].y * dWeights[1]) / dWtSqrd;
        tangent->z = (dPoles[1].z * dWeights[0]
                            - dPoles[0].z * dWeights[1]) / dWtSqrd;
        }
    else
        {
        *tangent = dPoles[1];
        }

    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bsputil_polygonNormal                                   |
|                                                                       |
| author        BFP                                     11/91           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bsputil_polygonNormal
(
DPoint3d        *normal,                        /* <= normal of polygon */
DPoint3d        *vertices,
int             nvertices
)
    {
    int         i;
    DPoint3d    *prev;
    DPoint3d    *current;

    prev = vertices+nvertices-1;
    normal->x = normal->y = normal->z = 0.0;

    for (i=0; i<nvertices; i++)
        {
        current = vertices + i;
        normal->x += (prev->y - current->y) * (prev->z + current->z);
        normal->y += (prev->z - current->z) * (prev->x + current->x);
        normal->z += (prev->x - current->x) * (prev->y + current->y);

        prev = current;
        }
    if (normal->Normalize () < fc_epsilon)
        return ERROR;
    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_frenetFrame                                     |
|                                                                       |
| author        BFP                                     5/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_frenetFrame
(
DPoint3d        *frame,                /* <= Frenet frame t,m,b */
DPoint3d        *point,                /* <= origin of frame (on curve) */
double          *curvature,
double          *torsion,
MSBsplineCurveCP curve,                /* => curve structure */
double          u,                     /* => parameter value */
DPoint3d        *defaultM              /* => normal for linear, or NULL */
)
    {
    int             status, linear=false;
    double          dWeights[4], tMag, dotMag, det, crvt;
    DPoint3d        t, m, b, dPoles[4], derivatives[4], tmp;

    t.Init (1,0,0);
    m.Init (0,1,0);
    b.Init (0,0,1);
    if (SUCCESS != (status = bspcurv_computeDerivatives (dPoles, dWeights, curve, 3, u,
                                                         false)))
        return status;

    if (curve->rational)
        bspcurv_chainRule (derivatives, dPoles, dWeights);
    else
        memcpy (derivatives, dPoles, 4 * sizeof(DPoint3d));

    if (point)
        *point = derivatives[0];

    /* Calculate Frenet frame from derivatives */
    t = derivatives[1];
    t.Normalize ();
    if (derivatives[ 2].Magnitude () < fc_epsilon)
        linear = true;
    else
        {
        b.CrossProduct (derivatives[1], derivatives[2]);
        linear = (b.Normalize () < fc_epsilon);
        m.CrossProduct (b, t);
        }

    /* Calculate curvature */
    if (curve->params.order < 3 || linear)
        crvt = 0.0;
    else
        {
        tMag = derivatives[1].Magnitude ();
        tmp.CrossProduct (derivatives[1], derivatives[2]);
        dotMag = tmp.Magnitude ();

        crvt = dotMag / tMag / tMag / tMag;
        }

    linear = linear || crvt < LINEAR_CURVATURE;

    /* Check to see that second derivative is non-zero */
    if (linear)
        {
        if (defaultM)
            {
            m = *defaultM;
            b.CrossProduct (t, m);
            b.Normalize ();
            m.CrossProduct (b, t);
            }
        else
            {
            if (curve->rational)
                bsputil_unWeightPoles (curve->poles, curve->poles,
                           curve->weights, curve->params.numPoles);
            if (bsputil_polygonNormal (&b, curve->poles,
                                       curve->params.numPoles))
                {
                tmp.x = tmp.y = tmp.z = 0.0;
                if ((fabs (t.x) < fc_p01) && (fabs (t.y) < fc_p01))
                    tmp.y = 1.0;
                else
                    tmp.z = 1.0;
                m.CrossProduct (tmp, t);
                m.Normalize ();
                b.CrossProduct (t, m);
                }
            else
                {
                m.CrossProduct (b, t);
                m.Normalize ();
                b.CrossProduct (t, m);
                }

            if (curve->rational)
                bsputil_weightPoles (curve->poles, curve->poles,
                           curve->weights, curve->params.numPoles);
            }
        }

    if (curvature)
        *curvature = linear ? 0.0 : crvt;

     if (torsion)
        {
        if (curve->params.order < 4 || linear)
            {
            *torsion = 0.0;
            }
        else
            {
            tmp.CrossProduct (derivatives[1], derivatives[2]);
            tMag = tmp.Magnitude ();
            tmp.CrossProduct (derivatives[2], derivatives[3]);
            det = derivatives->DotProduct (tmp);

            *torsion = det / tMag / tMag;
            }
        }

    if (frame)
        {
        frame[0] = t;
        frame[1] = m;
        frame[2] = b;
        }
    double determinant = t.TripleProduct (m, b);

    return fabs (determinant - 1.0) < Angle::SmallAngle () ? SUCCESS : ERROR;
    }
#endif /* !FONTIMPORTER */

/*----------------------------------------------------------------------+
|                                                                       |
| name          calculateCenter3pts                                     |
|                                                                       |
| author        RBB                                     6/86            |
|                                                                       |
+----------------------------------------------------------------------*/
static int     calculateCenter3pts
(
DPoint3d        *center,            /* <= center */
const DPoint3d  *pt                 /* => three points */
)
    {
    DPoint2d    md12, md23;
    double      b12, m12, b23, m23;

    md12.x = (pt[0].x + pt[1].x) / 2.0;
    md12.y = (pt[0].y + pt[1].y) / 2.0;
    md23.x = (pt[1].x + pt[2].x) / 2.0;
    md23.y = (pt[1].y + pt[2].y) / 2.0;

    center->z=pt[0].z;

    if ((fabs(pt[1].y-pt[0].y) < .2) && (fabs(pt[2].y-pt[1].y) < .2))
        {                               /* both are horizontal */
        return(2058);
        }
    else if (fabs(pt[1].y-pt[0].y) < .2) /* first bisector is horizontal */
        {
        center->x = md12.x;
        center->y = md23.y + ((pt[2].x-pt[1].x)/(pt[1].y-pt[2].y))*
                                (center->x-md23.x);
        }
    else if (fabs(pt[2].y-pt[1].y) < .2) /* second bisector is horizontal */
        {
        center->x = md23.x;
        center->y = md12.y + ((pt[1].x-pt[0].x)/(pt[0].y-pt[1].y))*
                                (center->x-md12.x);
        }
    else                                /* neither are horizontal */
        {
        m12 = (pt[1].x-pt[0].x)/(pt[0].y-pt[1].y);
        b12 = md12.y - m12*md12.x;
        m23 = (pt[2].x-pt[1].x)/(pt[1].y-pt[2].y);
        b23 = md23.y - m23*md23.x;

        if (fabs (m12-m23) < fc_epsilon) /* points are colinear */
            {
            return 2058;
            }
        else
            {
            center->x = (b12-b23)/(m23-m12);
            center->y = m12*center->x + b12;
            }
        }

    return (SUCCESS);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_inflectionPoints                                |
|                                                                       |
| author        BFP                                     2/91            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_inflectionPoints
(
double          **params,
int             *numPoints,
MSBsplineCurve  *curve
)
    {
    int         previous=false;
    DPoint3d    *p0, *p1, *p2, *endP, dif1, dif2, center, lastB, thisB;

    *numPoints = 0;
    if (params)    *params = NULL;
    if (curve->params.numPoles < 4)
        return SUCCESS;

    if (curve->rational)
        bsputil_unWeightPoles (curve->poles, curve->poles,
                   curve->weights, curve->params.numPoles);

    double tol = bsiTrig_smallAngle () * bsiDPoint3d_getLargestCoordinate (curve->poles, curve->params.numPoles);

    for (p0 = p1 = p2 = endP = curve->poles, p1 += 1, p2 += 2,
         endP += curve->params.numPoles;
         p2 < endP; p0++, p1++, p2++)
        {
        if (!(   p0->IsEqual (*p2, tol)
              || p0->IsEqual (*p1, tol)
              || p1->IsEqual (*p2, tol)
              || calculateCenter3pts (&center, p0))
             )
            {
            if (center.Distance (*p0) < 1.0 / LINEAR_CURVATURE)
                {
                dif1.DifferenceOf (*p1, *p0);
                dif2.DifferenceOf (*p2, *p1);
                thisB.CrossProduct (dif1, dif2);
                thisB.Normalize ();
                if (previous)
                    {
                    if ((thisB.DotProduct (lastB) < -0.7071))
                        *numPoints += 1;
                    lastB = thisB;
                    }
                else
                    {
                    lastB = thisB;
                    previous = true;
                    }
                }
            }
        }

    if (curve->rational)
        bsputil_weightPoles (curve->poles, curve->poles,
                   curve->weights, curve->params.numPoles);

    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_openCurve                                       |
|                                                                       |
| author        BFP                                     9/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_openCurve
(
MSBsplineCurveP  outCurve,
MSBsplineCurveCP inCurve,
double          u
)
    {
    int             i, j, status, initKnot, initKnot1, initPole, initPole1, maxKnotIndex, polesToEnd, numKnots;
    double          knotTolerance, *kP1, *end;
    MSBsplineCurve  curve;
    double          startKnot, endKnot, knotSpan;
    bool            bCorrect, bKeepUniform;

    if (!inCurve->params.closed)
        return bspcurv_copyCurve (outCurve, inCurve);

    /* Copy the input curve so output may be same */
    if (SUCCESS != (status = bspcurv_copyCurve (&curve, inCurve)))
        return status;

    // remember start/end knots and span
    startKnot = curve.knots[curve.params.order - 1];
    endKnot   = curve.knots[curve.params.numPoles + curve.params.order - 1];    // closed
    knotSpan  = endKnot - startKnot;

    /* compensate for fake periodic knot vectors */
    bCorrect = curve.params.closed ? mdlBspline_curveShouldBeOpened (&curve) : false;

    // If we're near either end of parameter space, open at 0.0.
    // Opening at 1.0 introduces slop in the last digits of precision that can cause problems with knot merging:
    //  e.g., -1/3 0 0 0 1/3 1/3 2/3 2/3 1 1 1 4/3  -->  0 0 0 1/3+e 1/3+e 2/3+e 2/3+e 1 1 1
    if (u < startKnot + KNOT_TOLERANCE_BASIS * knotSpan || u > endKnot - KNOT_TOLERANCE_BASIS * knotSpan)
        u = 0.0;

    // most likely scenario (e.g., 7-pole circles)
    if (bCorrect && (u == startKnot || u == endKnot))
        {
        // open up in place by shifting past initial mangled knots (poles/weights remain unchanged!)
        if (inCurve == outCurve)
            bspcurv_freeCurve (outCurve);
        *outCurve = curve;
        outCurve->params.closed = false;
        outCurve->params.numKnots = outCurve->params.numPoles - outCurve->params.order;
        memmove (outCurve->knots, &outCurve->knots[outCurve->params.order/2], (outCurve->params.numPoles + outCurve->params.order) * sizeof (double));
        return SUCCESS;
        }

    /* preserve uniform flag if interior knots will be preserved */
    bKeepUniform = (!curve.params.numKnots && (u == startKnot || u == endKnot));

    // generate intermediate curve
    knotTolerance = bspknot_knotTolerance (&curve);
    if (SUCCESS !=
        (status = bspknot_addKnot (&curve, u, knotTolerance, curve.params.order, false)))
        goto wrapup;

    if (inCurve == outCurve)
        bspcurv_freeCurve (outCurve);
    *outCurve = curve;

    // We assume that fake closed curves have same first/last poles so that we can sew the two end basis functions into one C0 function,
    // by removing the duplicate knot/pole.  This eliminates the order multiplicity interior knot that would otherwise result.
    if (bCorrect)
        outCurve->params.numPoles--;
    outCurve->params.closed = false;
    outCurve->params.numKnots = bKeepUniform ? 0 : outCurve->params.numPoles - outCurve->params.order;

    if (SUCCESS != (status = bspcurv_allocateCurve (outCurve)))
        goto wrapup;

    // point to first multiple of newly saturated knot
    numKnots = curve.params.numPoles + 2 * curve.params.order - 1;
    for (initKnot=0, kP1=end=curve.knots, end += numKnots; kP1 < end && (*kP1 - u <= knotTolerance); initKnot++, kP1++);
    initKnot -= curve.params.order;

    // point to end knot
    maxKnotIndex = curve.params.numPoles + curve.params.order - 1;

    // copy all multiples of newly saturated knot and following interior knots
    for (i = initKnot, j = 0; i < maxKnotIndex; i++, j++)
        outCurve->knots[j] = curve.knots[i];

    // point to the start knot
    initKnot1 = outCurve->params.order - 1;
    if (bCorrect)
        initKnot1++;    // but not for fake closed curves, as this would result in an interior knot with order multiplicity!

    // append preceding knots up to orderth multiple of newly saturated knot (ensure increasing vector by adding knotSpan)
    for (i = initKnot1; j < outCurve->params.numPoles + outCurve->params.order; j++, i++)
        outCurve->knots[j] = curve.knots[i] + knotSpan;

    bspknot_normalizeKnotVector (outCurve->knots, outCurve->params.numPoles, outCurve->params.order, outCurve->params.closed);

    // poles of fake closed curves are lined up with knots so that they remain invariant when opened
    initPole = initKnot;
    if (bCorrect)
        initPole -= curve.params.order/2;

    /* Safety check */
    if (initPole < 0)
        initPole = 0;
    polesToEnd = curve.params.numPoles - initPole;

    // only append the start pole if periodic: for fake closed curves, the start and end poles are assumed to be equal
    initPole1 = bCorrect ? 1 : 0;

    // shift poles/weights
    memcpy (outCurve->poles,              curve.poles + initPole,  polesToEnd * sizeof(DPoint3d));
    memcpy (outCurve->poles + polesToEnd, curve.poles + initPole1, (initPole - initPole1) * sizeof(DPoint3d));
    if (outCurve->rational)
        {
        memcpy (outCurve->weights,              curve.weights + initPole,  polesToEnd * sizeof(double));
        memcpy (outCurve->weights + polesToEnd, curve.weights + initPole1, (initPole - initPole1) * sizeof(double));
        }

wrapup:
    bspcurv_freeCurve (&curve);
    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_closeCurveWrapAround                            |
|                                                                       |
| author        DavidAssaf                              05/01           |
|                                                                       |
+----------------------------------------------------------------------*/
static int     bspcurv_closeCurveWrapAround
(
MSBsplineCurve  *outCurve,
MSBsplineCurve  *inCurve
)
    {
    /*
    Algorithm from Piegl and Tiller's _The NURBS Book_.
    Note: interior knots unchanged; exterior knots/poles are formulated to
            produce periodic (wrap-around) poles if the open curve has degree-1
            continuity at its endpoints.
    */
    MSBsplineCurve  curve;
    DPoint3d *P;
    double *U, *w, a, b, absMax = 1.0e-10, max = 0.0;
    bool    r;
    int i, j, k, n, p, status;

    if (SUCCESS != (status = bspcurv_copyCurve (&curve, inCurve)))
        return status;

    P = curve.poles;
    U = curve.knots;
    w = curve.weights;
    n = curve.params.numPoles - 1;
    p = curve.params.order - 1;
    r = (0 != curve.rational);

    /* not enough poles to go around */
    if (n - p < p)
        {
        bspcurv_freeCurve (&curve);
        return MDLERR_TOOFEWPOLES;
        }

    /* unclamp left end */
    for (i=0; i<=p-2; i++)
        {
        U[p-i-1] = U[p-i] - (U[n-i+1]-U[n-i]);
        for (k=p-1, j=i; j>=0; j--, k--)
            {
            a = (U[p]-U[k])/(U[p+j+1]-U[k]);

            if (1.0 == a)
                {
                // too many start knots
                bspcurv_freeCurve (&curve);
                return MDLERR_BADKNOTS;
                }

            b = 1.0/(1.0-a);
            a *= -b;
            P[j].Scale (P[j], b);
            P[j].SumOf (P[j], P[j+1], a);
            if (r)
                w[j] = b*w[j] + a*w[j+1];
            }
        }

    /* set first knot */
    U[0] = U[1] - (U[n-p+2]-U[n-p+1]);

    /* unclamp at right end */
    for (i=0; i<=p-2; i++)
        {
        U[n+i+2] = U[n+i+1] + (U[p+i+1]-U[p+i]);
        for (j=i; j>=0; j--)
            {
            a = (U[n+1]-U[n-j])/(U[n-j+i+2]-U[n-j]);

            if (0.0 == a)
                {
                // too many end knots
                bspcurv_freeCurve (&curve);
                return MDLERR_BADKNOTS;
                }

            b = 1.0/a;
            a = (a-1)*b;
            P[n-j].Scale (P[n-j], b);
            P[n-j].SumOf (P[n-j], P[n-j-1], a);
            if (r)
                w[n-j] = b*w[n-j] + a*w[n-j-1];
            }
        }

    /* set last knot */
    U[n+p+1] = U[n+p] + (U[2*p]-U[2*p-1]);

    /* set pole tolerance */
    for (i = 0; i <= n; i++)
        {
        if (max < P[i].x)
            max = P[i].x;
        if (max < P[i].y)
            max = P[i].y;
        if (max < P[i].z)
            max = P[i].z;
        }
    max *= absMax;

    /* leave "open" if resulting formulation doesn't wrap around, i.e., have
    homogeneous p-1 continuity */
    for (i = 0; i < p; i++)
        if (P[i].Distance (P[n-p+1+i]) > max || (r && (w[i]-w[n-p+1+i] > absMax)))
            {
            bspcurv_freeCurve (&curve);
            return ERROR;
            }

    /* curve can now be considered "closed" (it's truly periodic) */
    curve.params.closed = true;
    curve.params.numPoles -= p;

    if (outCurve == inCurve)
        bspcurv_freeCurve (outCurve);

    status = bspcurv_copyCurve (outCurve, &curve);

    bspcurv_freeCurve (&curve);
    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_closeCurve                                      |
|                                                                       |
| author        BFP                                     10/90           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_closeCurve
(
MSBsplineCurve  *outCurve,
MSBsplineCurve  *inCurve
)
    {
    int             status, numPoles;
    double          wt0, wt1;
    MSBsplineCurve  curve;

    if (inCurve->params.closed)
        return bspcurv_copyCurve (outCurve, inCurve);

    if (inCurve->params.order == 2 && inCurve->params.numPoles == 2)
        return bspcurv_copyCurve (outCurve, inCurve);

    numPoles = inCurve->params.numPoles;

    wt0 = inCurve->rational ? inCurve->weights[0] : 1.0;
    wt1 = inCurve->rational ? inCurve->weights[numPoles-1] : 1.0;

    if (! bsputil_isSameRationalPoint (inCurve->poles, wt0, inCurve->poles+numPoles-1, wt1))
        return (MDLERR_BADPOLES);

    if (SUCCESS != (status = bspcurv_copyCurve (&curve, inCurve)))
        return status;

    /* revert to v7 knots */
    mdlBspline_normalizeCurveKnots (NULL, NULL, &curve);

    if (curve.params.order == 2)
        {
        /* set first/last knot intervals to penultimate/second knot intervals */
        curve.knots[0] = curve.knots[1] - (curve.knots[numPoles] - curve.knots[numPoles-1]);
        curve.knots[numPoles+1] = curve.knots[numPoles] + (curve.knots[2] - curve.knots[1]);

        curve.params.numPoles--;
        curve.params.closed = true;
        }
    else
        {
        /* try to close the curve with degree continuity */
        if (SUCCESS != bspcurv_closeCurveWrapAround (&curve, &curve))
            {
            int     numKnots = bspknot_numberKnots (curve.params.numPoles, curve.params.order, curve.params.closed);
            double* pNormalizedKnots = static_cast<double *>(_alloca (numKnots * sizeof (double)));
            double* pKnots = NULL;

            /* need copy of old (normalized) full knot vector for bspknot_computeKnotVector */
            if (NULL == pNormalizedKnots)
                {
                status = MDLERR_INSFMEMORY;
                goto wrapup;
                }
            memcpy (pNormalizedKnots, curve.knots, numKnots * sizeof (double));

            /* Fake it!  This produces a problematic knot vector with order
            multiplicity start and end knots---both strictly *inside* the knot
            vector!  Use mdlBspline_curveShouldBeOpened to detect these curves. */
            curve.params.closed = true;
            curve.params.numKnots = curve.params.numPoles - 1;
            numKnots = bspknot_numberKnots (curve.params.numPoles, curve.params.order, curve.params.closed);

            if (NULL == (pKnots = static_cast<double *>(dlmSystem_mdlReallocWithDescr (curve.knots, numKnots * sizeof (double), pHeapDescr))))
                {
                status = MDLERR_INSFMEMORY;
                goto wrapup;
                }
            curve.knots = pKnots;

            bspknot_computeKnotVector (curve.knots, &curve.params, pNormalizedKnots + (curve.params.order + 1) / 2);
            }
        }

    if (outCurve == inCurve)
        bspcurv_freeCurve (outCurve);

    status = bspcurv_copyCurve (outCurve, &curve);

wrapup:
    bspcurv_freeCurve (&curve);
    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_makeBezier                                      |
|                                                                       |
| author        BFP                                     6/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_makeBezier
(
MSBsplineCurve  *outCurve,
MSBsplineCurve const *inCurve
)
    {
    int             i, status = SUCCESS, degree, numDist, bezier, *knotMult = NULL;
    double          knotTolerance, *distKnot = NULL;
    MSBsplineCurve  curve;

    // Bezier spline curve is always open
    if (SUCCESS != (status = bspcurv_openCurve (&curve, inCurve, 0.0)))
        return status;

    // Check if already Bezier curve
    if (curve.params.order == 2 || curve.params.numPoles == curve.params.order)
        goto wrapup;

    // Get distinct knots
    numDist = bspknot_numberKnots (curve.params.numPoles, curve.params.order, curve.params.closed);
    if (NULL == (knotMult = static_cast<int *>(msbspline_malloc (numDist * sizeof(int), HEAPSIG_BCRV))) ||
        NULL == (distKnot = static_cast<double *>(msbspline_malloc (numDist * sizeof(double), HEAPSIG_BCRV))))
        {
        status = MDLERR_INSFMEMORY;
        goto wrapup;
        }
    knotTolerance = bspknot_knotTolerance (&curve);
    bspknot_getKnotMultiplicity (distKnot, knotMult, &numDist, curve.knots, curve.params.numPoles, curve.params.order, curve.params.closed,
                                 knotTolerance);

    // Check if already Bezier spline curve
    degree = curve.params.order - 1;
    for (i=0, bezier=true; bezier && i < numDist-1; bezier = *(knotMult+i++) >= degree);
    if (bezier)
        goto wrapup;

    // Increase knot multiplicity
    for (i=0; i < numDist; i++)
        {
        if ((knotMult[i] < degree) &&
            SUCCESS != (status = bspknot_addKnot (&curve, distKnot[i], knotTolerance, degree, false)))
            goto wrapup;
        }

wrapup:
    if (distKnot)   msbspline_free (distKnot);
    if (knotMult)   msbspline_free (knotMult);
    if (SUCCESS == status)
        {
        if (outCurve == inCurve)
            bspcurv_freeCurve (outCurve);

        *outCurve = curve;
        }
    else
        bspcurv_freeCurve (&curve);

    return status;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_makeRational                                    |
|                                                                       |
| author        RBB                                     3/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_makeRational
(
MSBsplineCurve  *outCurve,
MSBsplineCurve  *inCurve
)
    {
    int               status;
    double            *wP, *endP;
    MSBsplineCurve    curve;

    if (inCurve->rational)
        return bspcurv_copyCurve (outCurve, inCurve);

    if (SUCCESS != (status = bspcurv_copyCurve (&curve, inCurve)))
        return status;

    curve.rational = true;
    if (NULL == (curve.weights = static_cast<double *>(dlmSystem_mdlMalloc (curve.params.numPoles * sizeof(double)))))
        {
        status = MDLERR_INSFMEMORY;
        goto wrapup;
        }
    else
        {
        for (wP=endP=curve.weights, endP += curve.params.numPoles; wP < endP; wP++)
            *wP = 1.0;
        }

    if (outCurve == inCurve)
        bspcurv_freeCurve (outCurve);

    *outCurve = curve;

wrapup:
    return status;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_reverseCurve                                    |
|                                                                       |
| author        RBB                                     3/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_reverseCurve
(
MSBsplineCurve  *inCurve,
MSBsplineCurve  *outCurve
)
    {
    int         i, k, status;
    bool        bClose = (0 != inCurve->params.closed);
    double      temp;
    DPoint3d    tempPoint;

    // TR #165344: lose the v7-era pole shifting on even order closed curves;
    //             preserve start point of closed curves by opening first, then re-closing after reversal
    if (SUCCESS != (status = bspcurv_openCurve (outCurve, inCurve, 0.0)))
        return status;

    k = outCurve->params.numPoles - 1;

    // reverse the poles
    for (i=0; i < k/2 + 1; i++)
        {
        tempPoint = outCurve->poles[k-i];
        outCurve->poles[k-i] = outCurve->poles[i];
        outCurve->poles[i] = tempPoint;
        }

    if (outCurve->rational)
        {
        // reverse the weights
        for(i=0; i < k/2 + 1; i++)
            {
            temp = outCurve->weights[k-i];
            outCurve->weights[k-i] = outCurve->weights[i];
            outCurve->weights[i] = temp;
            }
        }

    // reverse knot vector
    k = bspknot_numberKnots (outCurve->params.numPoles, outCurve->params.order, outCurve->params.closed) - 1;
    for (i=0; i < k/2 + 1; i++)
        {
        temp =  outCurve->knots[k-i];
        outCurve->knots[k-i] = outCurve->knots[i];
        outCurve->knots[i] = temp;
        }
    bspknot_normalizeKnotVector (outCurve->knots, outCurve->params.numPoles, outCurve->params.order, outCurve->params.closed);

    return bClose ? bspcurv_closeCurve (outCurve, outCurve) : SUCCESS;
    }



/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_knotToPointAndTangent                           |
|                                                                       |
| author        RBB                                     12/87           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bspcurv_knotToPointAndTangent
(
DPoint3d        *point,
DPoint3d        *tangent,
double          *weight,
double          x,
DPoint3d        *poles,
int             order,
int             numPoles,
double          *t,
double          *weights,
int             rational,
int             closed
)
    {
    int         left, p, numKnots;
    double      b[MAX_BSORDER], dB[MAX_BSORDER], maxKnot;
    double      h, dWeightDu, *bPtr, *bEnd, *dBLoc;
    DPoint3d    *pPtr;

    dBLoc = tangent ? dB:NULL;

    numKnots = bspknot_numberKnots (numPoles, order, closed);
    maxKnot = t[numKnots - order];
    bsputil_knotToBlendingFuncs (b, dBLoc, &left, t, x, maxKnot, order, closed);

    point->x = point->y = point->z = h = 0.0;

    p = left - order;
    if (closed && p < 0) p += numPoles;
    if (rational)
        {
        for (bPtr=b, bEnd=b+order; bPtr<bEnd; p++, bPtr++)
            {
            p %= numPoles;
            h += *(weights + p) * *bPtr;
            pPtr = poles + p;
            point->x += *bPtr * pPtr->x;
            point->y += *bPtr * pPtr->y;
            point->z += *bPtr * pPtr->z;
            }
        if (weight)
            *weight = h;
        if (h == 0.0)
            h = 1.0;
        point->x /= h;
        point->y /= h;
        point->z /= h;
        }
    else
        {
        for (bPtr=b, bEnd=b+order; bPtr<bEnd; p++, bPtr++)
            {
            p %= numPoles;
            pPtr = poles + p;
            point->x += *bPtr * pPtr->x;
            point->y += *bPtr * pPtr->y;
            point->z += *bPtr * pPtr->z;
            }
        if (weight)
            *weight = 1.0;
        }
    /* Compute the tangent */
    if (tangent)
        {
        tangent->x = tangent->y = tangent->z = dWeightDu = 0.0;
        p = left - order;
        if (closed && p < 0) p += numPoles;
        for (bPtr=dB,bEnd=dB+order; bPtr<bEnd;  p++, bPtr++)
            {
            p %= numPoles;
            if (rational) dWeightDu += *(weights + p) * *bPtr;
            pPtr = poles + p;
            tangent->x += *bPtr * pPtr->x;
            tangent->y += *bPtr * pPtr->y;
            tangent->z += *bPtr * pPtr->z;
            }

        if (rational)
            {
            /* Use quotient rule for finding dpdu.
                d(f/g) = (df * g - f * dg) / (g * g) */

            tangent->x = (tangent->x - point->x * dWeightDu)/h;
            tangent->y = (tangent->y - point->y * dWeightDu)/h;
            tangent->z = (tangent->z - point->z * dWeightDu)/h;
            }
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_computeCurvePoint                               |
|                                                                       |
| author        RBB                                     12/87           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void     bspcurv_computeCurvePoint
(
DPoint3d        *point,
DPoint3d        *tangent,
double          *weight,
double          fraction,
DPoint3d        *poles,
int             order,
int             numPoles,
double          *t,
double          *weights,
int             rational,
int             closed
)
    {
    int numKnots = bspknot_numberKnots (numPoles, order, closed);
    double x = fraction * t[numKnots-order] + t[order-1] * (1.0 - fraction);
    return bspcurv_knotToPointAndTangent (point, tangent, weight, x, poles,
                        order, numPoles, t, weights, rational, closed);
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_evaluateCurvePoint                              |
|                                                                       |
| author        RayBentley                               12/92          |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bspcurv_evaluateCurvePoint
(
DPoint3d        *pointP,            /* <= point */
DPoint3d        *tangentP,          /* <= tangent */
MSBsplineCurve const *curveP,            /* => curve */
double          u                   /* => u */
)
    {
    DPoint3d    point, tangent;

    bspcurv_computeCurvePoint (&point, &tangent, NULL, u,
                               curveP->poles,
                               curveP->params.order,
                               curveP->params.numPoles,
                               curveP->knots,
                               curveP->weights,
                               curveP->rational,
                               curveP->params.closed);

    if (pointP)     *pointP     = point;
    if (tangentP)   *tangentP   = tangent;
    }


#if !defined (FONTIMPORTER)
/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_generateSymmetricFunctions                      |
|                                                                       |
| author        RBB                                     3/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int     bspcurv_generateSymmetricFunctions
(
double          *f,
double          *x,
int             degree
)
    {
    int         i, j, order;

    order = degree + 1;

    for( i=0; i <= degree; i++)
        f[i*order] = 1;

    for (i=0; i < degree; i++)
        f[i*order +i +1] = 0;

    for (i=1; i <= degree; i++)
        {
        for (j=1; j <= i; j++)
            {
            f[i*order +j] = x[i -1] * f[(i-1)*order+j-1] + f[(i-1)*order+j];
            }
        }
    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_elevateDegree                                   |
|                                                                       |
| author        RBB                                     3/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_elevateDegree
(
MSBsplineCurve  *outCurve,
MSBsplineCurve  *inCurve,
int             newDegree
)
    {
    int             i,j,k, newN, newOrder, f, numDerivatives,
                    itmp, iitmp, jtmp, temp, status;

    double          firstWeight = 1.0,
                    sumWts,             /* summation                         */
                    *symi = NULL,       /* for calculation functions,        */
                    symiMem[NALLOC],    /* stores the i(th) one              */
                    psi[MAX_ORDER],     /* multiplying factor                */
                    *dervWts = NULL,    /* for storing all the derivatives   */
                    dervWtsMem[NALLOC], /* calculated for several parameters */
                    delta[MAX_ORDER],   /* an array of real values found from knot */
                    *newKnotVector = NULL,
                    knotTolerance;
    double          start = 0.0, end = 1.0;
    bool            bWasClosed = false,
                    symiAlloc = false,
                    dervAlloc = false;

    DPoint3d        firstPoint,           /* original first point of curve  */
                    sumPoles,
                    *dervPoles = NULL,
                    dervPolesMem[NALLOC];
    MSBsplineCurve  curve;

    if (newDegree > MAX_BSORDER-1)
        return (MDLERR_BADORDER);

    /* we only increase the degree, not decrease */
    if (newDegree <= inCurve->params.order - 1)
        {
        if(newDegree == inCurve->params.order - 1)
            return bspcurv_copyCurve (outCurve, inCurve);
        else
            return (MDLERR_BADORDER);
        }

    /* Make copy of input curve so input and output can be same */
    if (SUCCESS != (status = bspcurv_copyCurve (&curve, inCurve)))
        return status;

    if (curve.params.closed)
        {
        if (SUCCESS != (status = bspcurv_openCurve (&curve, &curve, 0.0)))
            goto wrapup;

        bWasClosed = true;
        }

    mdlBspline_normalizeCurveKnots (&start, &end, &curve);

    firstPoint = curve.poles[0];
    if (curve.rational)
        firstWeight = curve.weights[0];

    knotTolerance = bspknot_knotTolerance (&curve);
    if (SUCCESS !=
        (status = bspknot_increaseKnotDegree (&newKnotVector, &newN, newDegree, curve.knots,
                                              curve.params.numPoles, curve.params.order-1,
                                              curve.params.closed, knotTolerance)))
        goto wrapup;

    newOrder = newDegree + 1;
    temp = (newN + 1) * newOrder;
    if (temp > NALLOC)
        {
        dervAlloc = true;
        dervPoles = static_cast<DPoint3d *>(msbspline_malloc ((unsigned)(temp*sizeof(DPoint3d)), HEAPSIG_BCRV));
        dervWts =   static_cast<double *>(msbspline_malloc ((unsigned)(temp * sizeof(double)), HEAPSIG_BCRV));
        if (NULL == dervPoles || NULL == dervWts)
            {
            status = MDLERR_INSFMEMORY;
            goto wrapup;
            }
        }
    else
        {
        dervPoles = &dervPolesMem[0];
        dervWts = &dervWtsMem[0];
        }

   temp = newOrder * newOrder * (newN + 1);
   if (temp > NALLOC)
        {
        symiAlloc = true;
        symi = static_cast<double *>(msbspline_malloc ((unsigned)(temp * sizeof(double)), HEAPSIG_BCRV));
        if (NULL == symi)
            {
            status = MDLERR_INSFMEMORY;
            goto wrapup;
            }
        }
    else
        {
        symi = &symiMem[0];
        }

    numDerivatives = newDegree;

    for (i=0 ; i <= newN; i++)
        {
        /* we now use the old curve stuff to calculate derivatives at the
         new knot values */
        itmp = i * newOrder;
        iitmp = i+newDegree;
        if (SUCCESS !=
            (status = bspcurv_computeDerivatives (dervPoles+itmp, dervWts+itmp, &curve,
                                                 numDerivatives, newKnotVector[iitmp], true)))
            goto wrapup;

        /* we now form a real valued array from the new knot values */
        for (j=0; j < newDegree; j++)
            delta[j] = newKnotVector[i+j] - newKnotVector[iitmp];

        itmp *= newOrder;
        if (SUCCESS != (status = bspcurv_generateSymmetricFunctions (&symi[itmp],
                                                                     delta, newDegree)))
            goto wrapup;
        }

    if (outCurve == inCurve)
        bspcurv_freeCurve (outCurve);
    *outCurve = curve;
    outCurve->params.order = newOrder;
    outCurve->params.numPoles = newN + 1;
    outCurve->params.numKnots =
        bspknot_numberKnots (outCurve->params.numPoles,
                             outCurve->params.order,
                             outCurve->params.closed) - 2 * newOrder;

    if (SUCCESS != (status = bspcurv_allocateCurve (outCurve)))
         goto wrapup;

    outCurve->poles[newN] = curve.poles[curve.params.numPoles - 1];
    if (outCurve->rational)
        outCurve->weights[newN] = curve.weights[curve.params.numPoles - 1];

    for (i=1; i <= newN; i++)
        {
        itmp = i*newOrder;
        temp = newOrder*(itmp+newDegree)+newDegree;
        for (j=0; j <= newDegree; j++)
            {
            /*
                        (-1)**j  * (j!) * symi[i][deg -j][deg]
            psi[j] =   --------------------------------------
                                  deg !
            */
            f =  (j%2 == 1) ? -1 : 1;
            psi[j] = f*symi[temp-j];

            for (k=j+1; k <= newDegree; k++)
                psi[j] /= k;
            }

        /* Sum the poles and weights (if rational) */
        sumPoles.x = sumPoles.y = sumPoles.z = sumWts = 0.0;
        for (j=0; j <= newDegree; j++)
            {
            jtmp = newDegree - j;
            iitmp = itmp+j;
            f = ((newDegree - j) % 2 == 1) ? -1 : 1;
            sumPoles.x += f * psi[jtmp] * dervPoles[iitmp].x;
            sumPoles.y += f * psi[jtmp] * dervPoles[iitmp].y;
            sumPoles.z += f * psi[jtmp] * dervPoles[iitmp].z;
            if (outCurve->rational)
                sumWts += f * psi[jtmp] * dervWts[iitmp];
            }

        outCurve->poles[i-1] = sumPoles;
        if (outCurve->rational)
            outCurve->weights[i-1] = sumWts;
        }

    /* Reassign original first pole */
    if (!outCurve->params.closed)
        {
        outCurve->poles[0] = firstPoint;
        if (outCurve->rational)
            outCurve->weights[0] = firstWeight;
        }

    /* Load the knot vectors of the curve structure */
    memcpy (outCurve->knots, newKnotVector, bspknot_numberKnots (outCurve->params.numPoles, outCurve->params.order, outCurve->params.closed) * sizeof(double));

    if (bWasClosed)
        status = bspcurv_closeCurve (outCurve, outCurve);

wrapup:
    bspcurv_freeCurve (&curve);

    if (dervAlloc && dervPoles) msbspline_free (dervPoles);
    if (dervAlloc && dervWts)   msbspline_free (dervWts);
    if (symiAlloc && symi)      msbspline_free (symi);
    if (newKnotVector)          msbspline_free (newKnotVector);

    mdlBspline_unNormalizeCurveKnots (outCurve, start, end);
    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_segmentCurve                                    |
|                                                                       |
| author        BFP                                     9/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_segmentCurve
(
MSBsplineCurve  *segment,
MSBsplineCurveCP inCurve,
double          uInitial,
double          uFinal
)
    {
    int                 size, order, initKnot, finalKnot, numKnots,
                        status, reverseSegment;
    double              u1, u2, uDiff, knotTolerance, minKnot, maxKnot, range,
                        *kP1;
    MSBsplineCurve      curve;

    knotTolerance = bspknot_knotTolerance (inCurve);

    uInitial = mdlBspline_fractionParameterToNaturalParameter (inCurve, uInitial);
    uFinal = mdlBspline_fractionParameterToNaturalParameter (inCurve, uFinal);

    u1 = uInitial;      u2 = uFinal;
    uDiff = u2 - u1;
    reverseSegment = (uDiff < 0.0);
    numKnots = bspknot_numberKnots (inCurve->params.numPoles, inCurve->params.order,
                                    inCurve->params.closed);
    minKnot = inCurve->knots[inCurve->params.order - 1];
    maxKnot = inCurve->knots[numKnots - inCurve->params.order];
    range = maxKnot - minKnot;

    if (fabs (uDiff) < knotTolerance)
        return (MDLERR_BADPARAMETER);

    if (fabs (uDiff) >= range)
        {
        // copying entire curve -- possibly reversed or with new periodic origin
        status = SUCCESS;
        if (segment != inCurve)
            status = bspcurv_copyCurve (segment, inCurve);
        if (reverseSegment)
            status = bspcurv_reverseCurve (segment, segment);
        if (segment->params.closed
            && fabs (u1 - minKnot) > knotTolerance
            && fabs (u1 - maxKnot) > knotTolerance
            )
            return bspcurv_openCurve (segment, segment, u1);
        return status;
        }

    /* Copy the input curve so output may be same */
    size = sizeof (curve);
    memset (&curve, 0, size);
    if (SUCCESS != (status = bspcurv_copyCurve (&curve, inCurve)))
        return status;

    if (curve.params.closed)
        {
        while (u1 < minKnot)    u1 += range;
        while (u1 > maxKnot)    u1 -= range;
        if (SUCCESS != (status = bspcurv_openCurve (&curve, &curve, u1)))
            goto wrapup;
        if (reverseSegment && SUCCESS != (status = bspcurv_reverseCurve (&curve, &curve)))
            goto wrapup;
        u1 = 0.0;
        u2 = fabs (uDiff);
        }
    else
        {
        if (u1 < minKnot)       u1 = minKnot;
        else if (u1 > maxKnot)  u1 = maxKnot;
        if (u2 < minKnot)       u2 = minKnot;
        else if (u2 > maxKnot)  u2 = maxKnot;
        }

    if (SUCCESS !=
        (status = bspknot_addKnot (&curve, u1, knotTolerance, curve.params.order, false)) ||
        SUCCESS !=
        (status = bspknot_addKnot (&curve, u2, knotTolerance, curve.params.order, false)))
        goto wrapup;

    /* Set segment specifications and allocate */
    if (inCurve == segment)
        bspcurv_freeCurve (segment);
    memset (segment, 0, size);

    segment->params.closed = false;
    segment->rational = curve.rational;
    segment->params.order = order = curve.params.order;

    if (u2 < u1)
        {
        uDiff = u1;    u1 = u2;     u2 = uDiff;
        }

    for (initKnot=0, kP1 = curve.knots; fabs (*kP1 - u1) > knotTolerance; initKnot++, kP1++)
        ;
    for (finalKnot = initKnot+1, kP1 = curve.knots+finalKnot;
         fabs (*kP1 - u2) > knotTolerance; finalKnot++, kP1++)
        ;

    segment->params.numPoles = finalKnot - initKnot;
    segment->params.numKnots = segment->params.numPoles - order;

    if (segment->params.numPoles < order)
        {
        status = MDLERR_BADELEMENT;
        goto wrapup;
        }

    bspcurv_allocateCurve (segment);

    size = segment->params.numPoles * sizeof (DPoint3d);
    memcpy (segment->poles, curve.poles + initKnot, size);

    size = (segment->params.numPoles + order) * sizeof (double);
    memcpy (segment->knots, curve.knots + initKnot, size);

    if (SUCCESS !=
        (status = bspknot_normalizeKnotVector (segment->knots, segment->params.numPoles,
                                               segment->params.order, segment->params.closed)))
        {
        bspcurv_freeCurve (segment);
        goto wrapup;
        }

    if (segment->rational)
        {
        size = segment->params.numPoles * sizeof (double);
        memcpy (segment->weights, curve.weights + initKnot, size);
        }

    segment->display = curve.display;
    switch (curve.type)
        {
        case BSCURVE_LINE:
            {
            segment->type = BSCURVE_LINE;
            break;
            }
        case BSCURVE_CIRCLE:
        case BSCURVE_CIRCULAR_ARC:
            {
            segment->type = BSCURVE_CIRCULAR_ARC;
            break;
            }
        case BSCURVE_ELLIPSE:
        case BSCURVE_ELLIPTICAL_ARC:
            {
            segment->type = BSCURVE_ELLIPTICAL_ARC;
            break;
            }
        default : segment->type = BSCURVE_GENERAL;
        }

    if (reverseSegment)
        bspcurv_reverseCurve (segment, segment);

wrapup:
    bspcurv_freeCurve (&curve);
    return status;
    }


/*----------------------------------------------------------------------+
|                                                                       |
|   Curve Routines - acting on multiple curves                          |
|                                                                       |
+----------------------------------------------------------------------*/
/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_combineCurves                                   |
|                                                                       |
|   contiguous determines how the pole arrays are combined              |
|                                                                       |
|   CONTINUITY_NONE:  all poles of curve1, all poles of curve2.         |
|                                                                       |
|   In all other cases, outCurve has one less pole than the two inputs  |
|   CONTINUITY_LEFT:  all poles of curve1, lose the first pole of curve2|
|                                                                       |
|   CONTINUITY_MID:   bisect the last pole of curve1 and the first      |
|                     pole of curve2                                    |
|   CONTINUITY_RIGHT: lose the last pole of curve1, all poles of curve2 |
|                                                                       |
|                                                                       |
| author        RBB                                     3/90            |
|                                                                       |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_combineCurves                                   |
|                                                                       |
|   contiguous determines how the pole arrays are combined              |
|                                                                       |
|   CONTINUITY_NONE:  all poles of curve1, all poles of curve2.         |
|                                                                       |
|   In all other cases, outCurve has one less pole than the two inputs  |
|   CONTINUITY_LEFT:  all poles of curve1, lose the first pole of curve2|
|                                                                       |
|   CONTINUITY_MID:   bisect the last pole of curve1 and the first      |
|                     pole of curve2                                    |
|   CONTINUITY_RIGHT: lose the last pole of curve1, all poles of curve2 |
|                                                                       |
|                                                                       |
| author        RBB                                     3/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_combineCurves
(
MSBsplineCurve  *outCurve,
MSBsplineCurve  *curve1,
MSBsplineCurve  *curve2,
bool            contiguous,
bool            reparam
)
    {
    bool            dist1IsZero, dist2IsZero;
    int             i, j, status, numKnots1, outNumKnots;
    double          dist1, dist2;
#if defined (contiguousModes)
    DPoint3d        tmp1, tmp2;
#endif

    /* check the two input curves */
    if (curve1->params.closed || curve2->params.closed)
        return (MDLERR_BADPERIODICITY);
    if (curve1->params.order != curve2->params.order)
        return (MDLERR_BADORDER);

    memset (outCurve, 0, sizeof (*outCurve));
    outCurve->display.curveDisplay =
        curve1->display.curveDisplay || curve2->display.curveDisplay;
    outCurve->display.polygonDisplay =
        curve1->display.polygonDisplay || curve2->display.polygonDisplay;
    outCurve->rational = curve1->rational || curve2->rational;
    outCurve->params.order = curve1->params.order;

    outCurve->params.numPoles = curve1->params.numPoles +
                                curve2->params.numPoles;
    if (contiguous) outCurve->params.numPoles--;

    outCurve->params.numKnots = outCurve->params.numPoles - outCurve->params.order;

    if (SUCCESS != (status = bspcurv_allocateCurve (outCurve)))
        return status;

    /* Compute the actual number of knots in each of the curve */
    outNumKnots = outCurve->params.numPoles + outCurve->params.order;
    numKnots1   = curve1->params.numPoles + curve1->params.order;

    /* arrange the knot vector for outCurve */
    j = numKnots1 - (contiguous ? 1 : 0);
    if (reparam)
        {
#if defined (future_anyScale)
        /* NOTE: If the spline libraries are to be re-written to accept
           poles at any scale we fc_epsilon may be too big a test for zero. Everything is
           fine if we stick to UOR coordinates. */
#endif

        dist1 = bsputil_cumulativeDistance (curve1->poles, curve1->weights,
                      curve1->rational, curve1->params.numPoles);
        dist2 = bsputil_cumulativeDistance (curve2->poles, curve2->weights,
                      curve2->rational, curve2->params.numPoles);

        dist1IsZero = (dist1 < 1.0e-12) || (dist2 /dist1 > 1.0e10);
        dist2IsZero = (dist2 < 1.0e-12) || (dist1 /dist2 > 1.0e10);


        if (dist1IsZero && !dist2IsZero)
            {
            bspcurv_freeCurve (outCurve);
            bspcurv_copyCurve (outCurve, curve2);
            return SUCCESS;
            }
        else if (dist2IsZero && !dist1IsZero)
            {
            bspcurv_freeCurve (outCurve);
            bspcurv_copyCurve (outCurve, curve1);
            return SUCCESS;
            }
        else if (dist1IsZero && dist2IsZero)
            {
            bspcurv_freeCurve (outCurve);
            return MDLERR_BADPOLES;
            }

        for (i=0; i < j; i++)
            outCurve->knots[i] = curve1->knots[i] * dist1;
        for (j=curve2->params.order; i < outNumKnots; i++, j++)
            {
            outCurve->knots[i] = curve1->knots[curve1->params.numPoles] * dist1
                        + (curve2->knots[j] - curve2->knots[curve2->params.order - 1])
                        * dist2;
            }
        }
    else
        {
        memcpy (outCurve->knots, curve1->knots, j*sizeof(double));
        for (i=j, j=curve2->params.order; i < outNumKnots; i++, j++)
            {
            outCurve->knots[i] = curve1->knots[curve1->params.numPoles]
                        + (curve2->knots[j] - curve2->knots[curve2->params.order - 1]);
            }
        }

    bspknot_normalizeKnotVector (outCurve->knots, outCurve->params.numPoles,
                                 outCurve->params.order, outCurve->params.closed);

    /* arrange the poles for outCurve */
    memcpy (outCurve->poles, curve1->poles, curve1->params.numPoles * sizeof (DPoint3d));
    memcpy (outCurve->poles + curve1->params.numPoles, curve2->poles + (contiguous ? 1 : 0),
            (outCurve->params.numPoles - curve1->params.numPoles) * sizeof (DPoint3d));

    /* assign the weights and reweight the poles */
    if (outCurve->rational)
        {
        if (curve1->rational)
            {
            memcpy (outCurve->weights, curve1->weights,
                    curve1->params.numPoles * sizeof (double));
            }
        else
            {
            for (i=0; i < curve1->params.numPoles; outCurve->weights[i++] = 1.0)
                ;
            }
        if (curve2->rational)
            {
            memcpy (outCurve->weights + curve1->params.numPoles,
                    curve2->weights + (contiguous ? 1 : 0),
                    (outCurve->params.numPoles - curve1->params.numPoles) * sizeof (double));
            }
        else
            {
            for (i=curve1->params.numPoles; i < outCurve->params.numPoles;
                 outCurve->weights[i++] = 1.0)
                ;
            }
        }
#if defined (contiguousModes)
    /* adjust based on contiguous */
    midPole = curve1->params.numPoles - 1;
    if (contiguous == CONTINUITY_RIGHT)
        {
        outCurve->poles[midPole] = curve2->poles[0];
        if (outCurve->rational)
            {
            if (curve2->rational)
                outCurve->weights[midPole] = curve2->weights[0];
            else
                outCurve->weights[midPole] = 1.0;
            }
        }
    else if (contiguous == CONTINUITY_MID)
        {
        last = curve1->params.numPoles - 1;
        if (curve1->rational)
            tmp1.Scale (*(curve1->poles + last), 1.0 / curve1->weights[last]);
        else
            tmp1 = curve1->poles[last];
        if (curve1->rational)
            tmp1.Scale (*(curve1->poles + last), 1.0 / curve1->weights[last]);
        else
            tmp1 = curve1->poles[last];
        (outCurve->poles + midPole)->Interpolate (tmp1, 0.5, tmp2);
        if (outCurve->rational)
            outCurve->weights[midPole] = 1.0;
        }
#endif
    return SUCCESS;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_contiguousCurves                                |
|                                                                       |
| author        BFP                                     10/90           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool     bspcurv_contiguousCurves
(
MSBsplineCurveCP curve1,
MSBsplineCurveCP curve2
)
    {
    DPoint3d end1, start2;
    bspcurv_extractEndPoints (NULL, &end1, curve1);
    bspcurv_extractEndPoints (&start2, NULL, curve2);
    return bsputil_isSamePoint (&end1, &start2);
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_appendCurves                                    |
|                                                                       |
| author        BFP                                     5/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_appendCurves
(
MSBsplineCurve  *combinedCurve,
MSBsplineCurve  *inCurve1,
MSBsplineCurve  *inCurve2,
bool            forceContinuity,
bool            reparam,
void         *pHeapDescr
)
    {
    int             status;
    bool            contiguous;
    MSBsplineCurve  curve1, curve2;

    if (SUCCESS != (status = bspcurv_copyCurve (&curve1, inCurve1)) ||
        SUCCESS != (status = bspcurv_copyCurve (&curve2, inCurve2)))
        return status;

    if ((curve1.params.closed &&
        SUCCESS != (status = bspcurv_openCurve (&curve1, &curve1, 0.0))) ||
        (curve2.params.closed &&
        SUCCESS != (status = bspcurv_openCurve (&curve2, &curve2, 0.0))))
        goto wrapup;

    if (((curve1.params.order < curve2.params.order) &&
         SUCCESS !=
        (status = bspcurv_elevateDegree (&curve1, &curve1, curve2.params.order-1))) ||
        ((curve2.params.order < curve1.params.order) &&
         SUCCESS !=
        (status = bspcurv_elevateDegree (&curve2, &curve2, curve1.params.order-1))))
        goto wrapup;

    if (combinedCurve == inCurve1 || combinedCurve == inCurve2)
        bspcurv_freeCurve (combinedCurve);

    if (forceContinuity)
        contiguous = forceContinuity;
    else
        contiguous = bspcurv_contiguousCurves (&curve1, &curve2);
    status = bspcurv_combineCurves (combinedCurve, &curve1, &curve2, contiguous, reparam);

wrapup:
    bspcurv_freeCurve (&curve1);
    bspcurv_freeCurve (&curve2);
    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_appendCurves                                    |
|                                                                       |
| author        BFP                                     5/90            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      bspcurv_appendCurves
(
MSBsplineCurve  *combinedCurve,
MSBsplineCurve  *inCurve1,
MSBsplineCurve  *inCurve2,
bool            forceContinuity,
bool            reparam
)
    {
    bool condition1, condition2;

    condition1 = inCurve1 && NULL == inCurve1->poles;
    condition2 = inCurve2 && NULL == inCurve2->poles;
    if (condition1 && !condition2)
        return bspcurv_copyCurve (combinedCurve, inCurve2);
    else if (condition2 && !condition1)
        return bspcurv_copyCurve (combinedCurve, inCurve1);
    else
        return bspcurv_appendCurves (combinedCurve, inCurve1, inCurve2, forceContinuity, reparam, NULL);
    }


void MSBsplineCurve::GetDisjointCurves
(
bvector<MSBsplineCurvePtr> &curves
) const
    {
    MSBsplineCurve *curveArray;
    int numCurves;
    curves.clear ();
    if (SUCCESS == bspcurv_segmentDisjointCurve (&curveArray, &numCurves, this))
        {
        for (int i = 0; i < numCurves; i++)
            curves.push_back (curveArray[i].CreateCapture ());
        BSIBaseGeom::Free (curveArray);
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_segmentDisjointCurve                            |
|                                                                       |
| author        RBB                                      6/91           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int bspcurv_segmentDisjointCurve
(
MSBsplineCurve    **segCurves,          /* <= nondisjoint curves */
int               *nSegCurves,          /* <= number of nondisjoint curves */
MSBsplineCurveCP   curveP               /* => input curve */
)
    {
    KnotData knots;
    knots.LoadCurveKnots (*curveP);
    bvector<double> knotBreaks;
    knots.CollectHighMultiplicityActiveKnots ((size_t)curveP->params.order, true, knotBreaks);

    size_t numIn = knotBreaks.size () - 1;
    *segCurves = static_cast<MSBsplineCurve *>(BSIBaseGeom::Malloc (numIn * sizeof(MSBsplineCurve)));

    int numOut = 0;
    if (knotBreaks.size () == 2)
        {
        numOut = 1;
        bspcurv_copyCurve (*segCurves, curveP);
        }
    else
        {
        for (size_t i = 0; i < knotBreaks.size (); i++)
            knotBreaks[i] = curveP->KnotToFraction (knotBreaks[i]);

        for (size_t i = 0; i < numIn; i++)
            {
            if (SUCCESS == bspcurv_segmentCurve (&((*segCurves)[numOut]), curveP,
                            knotBreaks[i], knotBreaks[i+1]))
                numOut++;
            }
        }
    *nSegCurves = numOut;
    return SUCCESS;
    }



/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_extractEndPoints                                |
|                                                                       |
| author        RayBentley                               1/93           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void bspcurv_extractEndPoints
(
DPoint3d        *startP,
DPoint3d        *endP,
MSBsplineCurveCP curveP
)
    {
    if (curveP->params.closed)
        {
        if (startP)
            bspcurv_evaluateCurvePoint (startP, NULL, curveP, 0.0);
        if (endP)
            bspcurv_evaluateCurvePoint (endP, NULL, curveP, 1.0);
        }
    else if (curveP->rational)
        {
        if (startP)
            bsputil_unWeightPoles (startP, &curveP->poles[0], &curveP->weights[0], 1);

        if (endP)
            bsputil_unWeightPoles (endP, &curveP->poles[curveP->params.numPoles-1],
                                         &curveP->weights[curveP->params.numPoles-1], 1);
        }
    else
        {
        if (startP)
            *startP = curveP->poles[0];
        if (endP)
            *endP   = curveP->poles[curveP->params.numPoles-1];
        }
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_extractNormal                                   |
|                                                                       |
| author        RBB                                      6/91           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int bspcurv_extractNormal
(
DPoint3d          *normalP,               /* <= curve normal */
DPoint3d          *positionP,             /* <= average pole position */
double            *planarDeviationP,      /* <= deviation from planar */
MSBsplineCurveCP  curveP
)
    {
    int         numPoles = curveP->params.numPoles;

    if (numPoles < 2)
        return ERROR;

    if (curveP->rational)
        bsputil_unWeightPoles (curveP->poles, curveP->poles,
                                  curveP->weights, numPoles);

    bsputil_extractNormal (normalP, positionP, planarDeviationP,
                           curveP->poles,  (bspcurv_isPhysicallyClosed (curveP) && curveP->params.order > 2) ?
                           curveP->params.numPoles - 1 : curveP->params.numPoles, NULL);

    if (curveP->rational)
        bsputil_weightPoles (curveP->poles, curveP->poles, curveP->weights, numPoles);

    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_rotateCurve                                     |
|                                                                       |
| author        RayBentley                               1/92           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int bspcurv_rotateCurve
(
MSBsplineCurve          *outCurveP,         /* <= rotated curve */
MSBsplineCurve          *inCurveP,          /* => input curve */
RotMatrixCP             rMatrixP           /* => rotation matrix */
)
    {
    int         status;

    if (outCurveP != inCurveP)
        if (SUCCESS != (status = bspcurv_copyCurve (outCurveP, inCurveP)))
            return status;

    if (NULL != rMatrixP)
        {
        if (outCurveP->rational)
            bsputil_unWeightPoles (outCurveP->poles, outCurveP->poles,
                                   outCurveP->weights, outCurveP->params.numPoles);

        rMatrixP->Multiply (outCurveP->poles, outCurveP->poles, outCurveP->params.numPoles);
        if (outCurveP->rational)
            bsputil_weightPoles (outCurveP->poles, outCurveP->poles,
                                 outCurveP->weights, outCurveP->params.numPoles);
        }
    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_transformCurve                                  |
|                                                                       |
| author        EarlinLutz                               02/01          |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int bspcurv_transformCurve
(
MSBsplineCurveP         outCurveP,         /* <= transformed curve */
MSBsplineCurveCP        inCurveP,          /* => input curve */
Transform const         *transformP         /* => transform */
)
    {
    int         status;

    if (outCurveP != inCurveP)
        if (SUCCESS != (status = bspcurv_copyCurve (outCurveP, inCurveP)))
            return status;

    if (transformP && ! transformP->IsIdentity ())
        {
        if (outCurveP->rational)
            {
            transformP->MultiplyWeighted (outCurveP->poles, outCurveP->poles, outCurveP->weights, outCurveP->params.numPoles);
            }
        else
            {
            transformP->Multiply (outCurveP->poles, outCurveP->params.numPoles);
            }
        }
    return SUCCESS;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_transformCurve4d                                |
|                                                                       |
| author        EarlinLutz                               05/06          |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int bspcurv_transformCurve4d
(
MSBsplineCurveP         outCurveP,         /* <= transformed curve */
MSBsplineCurveCP        inCurveP,          /* => input curve */
DMatrix4dCP             transform4dP         /* => transform */
)
    {
    Transform transform;
    int i;
    DPoint4d xyzw;

    if (outCurveP == inCurveP && (NULL == transform4dP || transform4dP->IsIdentity ()))
        return SUCCESS;

    // If the DMatrix4d has no perspective effects, just apply as a
    // regular transform.
    if (transform.InitFrom (*transform4dP))
        return bspcurv_transformCurve (outCurveP, inCurveP, &transform);

    if (outCurveP != inCurveP)
        bspcurv_copyCurve (outCurveP, inCurveP);

    // Force the output to have weights ...

    if (SUCCESS != bspcurv_makeRational (outCurveP, outCurveP))
        return ERROR;

    for (i = 0; i < outCurveP->params.numPoles; i++)
        {
        bsiDMatrix4d_multiplyWeightedDPoint3dArray
                (
                transform4dP,
                &xyzw,
                &outCurveP->poles[i],
                &outCurveP->weights[i],
                1
                );
        outCurveP->poles[i].x  = xyzw.x;
        outCurveP->poles[i].y  = xyzw.y;
        outCurveP->poles[i].z  = xyzw.z;
        outCurveP->weights[i]  = xyzw.w;
        }

    return SUCCESS;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_isPhysicallyClosed                              |
|                                                                       |
| author        BrianPeters                             4/93            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    bspcurv_isPhysicallyClosed
(
MSBsplineCurveCP  curve
)
    {
    return bspcurv_contiguousCurves (curve, curve);
    }

// REMOVE BBB HERE

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_c1Discontinuities                               |
|                                                                       |
| author        RayBentley                               5/93           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int bspcurv_c1Discontinuities
(
bvector<double>&params,
MSBsplineCurveCP inCurveP
)
    {
    int         i, nKnots, numDistinct, *knotMultiplicityP;
    double      *distinctKnotP, leftValue, rightValue, start, end;
    DPoint3d    leftTangent, rightTangent;
    MSBsplineCurveP curveP = (MSBsplineCurveP)inCurveP; // const
    mdlBspline_normalizeCurveKnots (&start, &end, curveP);
    nKnots = bspknot_numberKnots (curveP->params.numPoles,
                                  curveP->params.order, curveP->params.closed);

    if (NULL ==
        (distinctKnotP = (double *) msbspline_malloc (nKnots * sizeof(double), HEAPSIG_BCRV)) ||
        NULL ==
        (knotMultiplicityP  = (int *) msbspline_malloc (nKnots * sizeof(int),  HEAPSIG_BCRV)))
        return MDLERR_INSFMEMORY;

    bspknot_getKnotMultiplicity (distinctKnotP, knotMultiplicityP, &numDistinct,
                                 curveP->knots, curveP->params.numPoles,
                                 curveP->params.order, curveP->params.closed,
                                 KNOT_TOLERANCE_BASIS);

    for (i = 0; i<numDistinct; i++)
        if (knotMultiplicityP[i] >= curveP->params.order - 1)
            {
            if (curveP->params.closed ||
                (distinctKnotP[i] >= KNOT_TOLERANCE_BASIS &&
                 distinctKnotP[i] <= 1.0 - KNOT_TOLERANCE_BASIS))
                {
                leftValue  = distinctKnotP[i] - KNOT_TOLERANCE_BASIS;
                rightValue = distinctKnotP[i] + KNOT_TOLERANCE_BASIS;
                if (leftValue  < 0.0) leftValue  += 1.0;
                if (rightValue > 1.0) rightValue -= 1.0;
                bspcurv_getTangent (&leftTangent,  leftValue, curveP, false);
                bspcurv_getTangent (&rightTangent, rightValue, curveP, true);

                if (leftTangent.Normalize () < 1.0 ||
                    rightTangent.Normalize () < 1.0 ||
                    leftTangent.DotProduct (rightTangent) < COSINE_TOLERANCE)
                    params.push_back (distinctKnotP[i]);
                }
            }
    msbspline_free (distinctKnotP);
    msbspline_free (knotMultiplicityP);

    mdlBspline_unNormalizeCurveKnots (curveP, start, end);
    return SUCCESS;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_c1Discontinuities                               |
|                                                                       |
| author        RayBentley                               5/93           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int bspcurv_c1Discontinuities
(
double          **paramPP,
int             *nParamsP,
MSBsplineCurve  *curveP
)
    {
    bvector<double> params;
    StatusInt stat = bspcurv_c1Discontinuities (params, curveP);
    *nParamsP = BSIBaseGeom::MallocAndCopy (paramPP, params);
    return stat;
    }





/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_polygonLength                                   |
|                                                                       |
| author        RayBentley                               5/93           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP double bspcurv_polygonLength
(
MSBsplineCurve const *curveP
)
    {
    return curveP->PolygonLength ();
    }



/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_createBsplineFromPointsAndOrder
|                                                                       |
| author        LuHan                                   1/99            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP StatusInt mdlBspline_createBsplineFromPointsAndOrder
(
MSBsplineCurve  *pCurve,
DPoint3d        *pPoints,
int             numPoints,
int             order
)
    {
    int     status = ERROR;

    if (pCurve != NULL && pPoints != NULL && numPoints > 1)
        {
        memset (pCurve, 0, sizeof(MSBsplineCurve));
        pCurve->params.order = order;
        pCurve->params.numPoles = numPoints;
        if (SUCCESS == (status = bspcurv_allocateCurve (pCurve)))
            {
            memcpy (pCurve->poles, pPoints, numPoints*sizeof(DPoint3d));
            status = bspknot_computeKnotVector (pCurve->knots, &pCurve->params, NULL);
            }
        }
    return  status;
    }


#endif /* !FONTIMPORTER */


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             12/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  int     bspproc_prepareCurveOld
(
MSBsplineCurve  *bezier,               /* <= Bezier curve */
int             *numSegments,          /* <= number of segs */
int             **starts,              /* <= offsets of segments */
MSBsplineCurve  *bspline               /* => Bspline curve */
)
    {
    int         i, *iP, status, numDist;
    double      *distKnot, knotTol;

    memset (bezier, 0, sizeof(*bezier));
    if (SUCCESS != (status = bspcurv_openCurve (bezier, bspline, 0.0)) ||
        SUCCESS != (status = bspcurv_makeBezier (bezier, bezier)))
        return status;

    numDist = bspknot_numberKnots (bezier->params.numPoles, bezier->params.order,
                                   bezier->params.closed);
    distKnot = NULL;    *starts = NULL;
    if (NULL == (*starts  = (int*)msbspline_malloc (numDist * sizeof(int),    HEAPSIG_BPRC)) ||
        NULL == (distKnot = (double*)msbspline_malloc (numDist * sizeof(double), HEAPSIG_BPRC)))
        {
        status = ERROR;
        goto wrapup;
        }

    knotTol = bspknot_knotTolerance (bezier);
    bspknot_getKnotMultiplicity (distKnot, *starts, numSegments, bezier->knots,
                                 bezier->params.numPoles, bezier->params.order,
                                 bezier->params.closed, knotTol);
    *numSegments -= 1;

    /* Find offset of start of segs from knot multiplicity array */
    (*starts)[0] = 0;
    for (i=1, iP=(*starts)+1; i < *numSegments; i++, iP++)
        *(iP) += *(iP-1);
wrapup:
    if (distKnot)       msbspline_free (distKnot);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             12/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  int     bspproc_prepareCurve
(
MSBsplineCurve  *bezier,               /* <= Bezier curve */
int             *numSegments,          /* <= number of segs */
int             **starts,              /* <= offsets of segments */
MSBsplineCurve  *bspline               /* => Bspline curve */
)
    {
    BsplineDisplay display = bspline->display;
    BCurveSegment segment;
    StatusInt status = ERROR;
    int order = bspline->params.order;
    size_t numSegment = bspline->CountDistinctBeziers ();
    size_t numPoles = numSegment * (order - 1) + 1;
    size_t numKnots = 2 * order + (numSegment - 1) * (order - 1);
    DPoint3d *poles = (DPoint3d*)BSIBaseGeom::Malloc (numPoles * sizeof (DPoint3d));
    bool isRational = bspline->rational != 0;
    double   *knots = (double*)BSIBaseGeom::Malloc (numKnots * sizeof (double));
    double   *weights = isRational ? (double*)BSIBaseGeom::Malloc (numPoles * sizeof (double)) : NULL;

    *numSegments = (int)numSegment;

    if (starts)
        *starts = (int*)BSIBaseGeom::Malloc (numPoles * sizeof (int));

    if (bezier != bspline)
        bezier->Zero ();

    size_t iPole = 0;
    size_t iKnot = 0;
    size_t numSegmentHere = 0;
    if (starts != nullptr)
        (*starts)[0] = 0;
    for (size_t i = 0; bspline->AdvanceToBezier (segment, i);)
        {
        numSegmentHere++;
        // Really shouldn't happen but we'll make sure ...
        if (numSegmentHere > numSegment)
            goto cleanup;
        double u0 = segment.UMin ();
        double u1 = segment.UMax ();

        if (iPole == 0)
            {
            // Leading pole ...
            segment.m_poles[0].GetXYZ (poles[0]);
            if (isRational)
                weights[0] = segment.m_poles[0].w;
            iPole = 1;
            // leading knots....
            for (int k = 0; k < order; k++)
                knots[iKnot++] = u0;
            }

        // First pole (should) already be in place.
        for (int k = 1; k < order; k++, iPole++)
            {
            segment.m_poles[k].GetXYZ (poles[iPole]);
            if (isRational)
                weights[iPole]= segment.m_poles[k].w;
            }

        if (NULL != starts)
            (*starts)[numSegmentHere] = (int)iPole - 1;
        // {order-1} copies of upper knot.
        for (int k = 1; k < order; k++, iKnot++)
            knots[iKnot] = u1;
        }
    // extra knot per classic storage convention
    knots[iKnot] = knots[iKnot-1];
    if (bezier == bspline)
        bspline->ReleaseMem ();

    bezier->poles = poles;
    bezier->knots = knots;
    bezier->weights = weights;
    bezier->params.order = order;
    bezier->rational = isRational;
    bezier->params.numPoles = (int)numPoles;
    bezier->params.numKnots = (int)numKnots - 2 * order;  // only the internal count is recorded.  Very strange.
    bezier->params.closed = 0;
    bezier->display = display;
    return SUCCESS;

cleanup:
    if (poles)
        BSIBaseGeom::Free (poles);
    if (knots)
        BSIBaseGeom::Free (knots);
    if (weights)
        BSIBaseGeom::Free (weights);
    if (starts)
        {
        BSIBaseGeom::Free (*starts);
        *starts = NULL;
        }
    return status;
    }


/*----------------------------------------------------------------------+
|                                                                       |
| name          mdlBspline_removeRedundantKnotsFromCurve                |
|                                                                       |
| author        LuHan                                   6/99            |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP int      mdlBspline_removeRedundantKnotsFromCurve
(
MSBsplineCurve  *out,
MSBsplineCurve  *in
)
    {
    int     i, j, status=SUCCESS, numKnots, numDistinct, numRemove, index, size, numPoles, order,
            *removeMultsP=NULL, *knotMultiplicityP=NULL, *knotIndexP=NULL;
    double  *distinctKnotP=NULL, *removeKnotsP=NULL;

    bspcurv_openCurve (out, in, 0.0);
    order = in->params.order;
    numKnots = bspknot_numberKnots (out->params.numPoles, out->params.order, out->params.closed);
    if (NULL == (distinctKnotP = (double *)BSIBaseGeom::Malloc (numKnots * sizeof(double))) ||
        NULL == (removeKnotsP = (double *)BSIBaseGeom::Malloc (numKnots * sizeof(double))) ||
        NULL == (removeMultsP = (int *)BSIBaseGeom::Malloc (numKnots * sizeof(int))) ||
        NULL == (knotIndexP = (int *)BSIBaseGeom::Malloc (numKnots * sizeof(int))) ||
        NULL == (knotMultiplicityP = (int *)BSIBaseGeom::Malloc (numKnots * sizeof(int))))
        return ERROR;

    bspknot_getKnotMultiplicity (distinctKnotP, knotMultiplicityP, &numDistinct,
        out->knots, out->params.numPoles, out->params.order, out->params.closed,
        KNOT_TOLERANCE_BASIS);

    for (i = 0, numRemove = 0, index = 0, knotIndexP[0] = 0; i < numDistinct; i++)
        {
        index += knotMultiplicityP[i];
        if (i == 0 || i == numDistinct-1)
            {
            if (knotMultiplicityP[i] > out->params.order)
                {
                removeMultsP[numRemove] = knotMultiplicityP[i] - out->params.order;
                removeKnotsP[numRemove] = distinctKnotP[i];
                knotIndexP[numRemove] = index - removeMultsP[numRemove];
                numRemove++;
                }
            }
        else
            {
            if (knotMultiplicityP[i] >= out->params.order)
                {
                removeMultsP[numRemove] = knotMultiplicityP[i] - out->params.order + 1;
                removeKnotsP[numRemove] = distinctKnotP[i];
                knotIndexP[numRemove] = index - removeMultsP[numRemove];
                numRemove++;
                }
            }
        }

    /* Remove redundant knots */
    numPoles = out->params.numPoles;
    for (i = 0; i < numRemove; i++)
        {
        size = numKnots - knotIndexP[i] - removeMultsP[i];
        if(size==0) 
            break;
        memcpy (out->knots+knotIndexP[i], out->knots+knotIndexP[i]+removeMultsP[i], size*sizeof(double));

        size =  knotIndexP[i]+removeMultsP[i]-order+1;
        memcpy (out->poles+knotIndexP[i]-order+1, out->poles+size, (numPoles-size)*sizeof(DPoint3d));
        if (out->rational)
            memcpy (out->weights+knotIndexP[i]-order+1, out->weights+size, (numPoles-size)*sizeof(double));

        for (j = i+1; j < numRemove; j++)
            knotIndexP[j] -= removeMultsP[i];
        out->params.numPoles -= removeMultsP[i];
        out->params.numKnots -= removeMultsP[i];
        }

    if (removeMultsP)
        BSIBaseGeom::Free (removeMultsP);
    if (knotMultiplicityP)
        BSIBaseGeom::Free (knotMultiplicityP);
    if (distinctKnotP)
        BSIBaseGeom::Free (distinctKnotP);
    if (removeKnotsP)
        BSIBaseGeom::Free (removeKnotsP);
    if (knotIndexP)
        BSIBaseGeom::Free (knotIndexP);

    return  status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
| name          bspcurv_areCurvesIdentical                              |
|                                                                       |
| author        RayBentley                               5/93           |
|                                                                       |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP bool    bspcurv_areCurvesIdentical
(
MSBsplineCurve  *curve0,
MSBsplineCurve  *curve1
)
    {
    return curve0->AlmostEqual (*curve1);
    }

/*---------------------------------------------------------------------------------**//**
* Four ways to use this function: 1). numPts != 0 && data != NULL -> evaluate at given parametrs #). numPts != 0 && data == NULL -> evaluate
* at numPts evenly 2). rulesByLength == true ... spaced along arc length 3). rulesByLength == false ... spaced from 0.0 to 1.0 4). numPts == 0
* && -> stroke curve according to data[0] = chord tol chord tolerance, set numPts to number of points returned
* @bsimethod                                                    BFP             09/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_evaluateCurve
(
DPoint3d        **pts,         /* <= evaluated points */
double          *data,         /* => params to evaluate at */
int             *numPts,       /* <=> number evaluated points */
MSBsplineCurve  *curve         /* => curve structure */
)
    {
    int         i, status;
    double      *dPtr, x, delta, tol, *endP;
    DPoint3d    *pPtr;

    if (*numPts)
        {
        if (NULL == (*pts = static_cast<DPoint3d *>(BSIBaseGeom::Malloc (*numPts * sizeof (DPoint3d)))))
            return ERROR;
        pPtr = *pts;
        if (data)                               /* case 1 */
            {
            for (dPtr=endP=data, endP += *numPts; dPtr < endP; dPtr++, pPtr++)
                bspcurv_evaluateCurvePoint (pPtr, NULL, curve, *dPtr);

            status = SUCCESS;
            }
/*      else if (curve->display.rulesByLength)   case 2
            {
            Not currently supported
            }*/
        else                                    /* case 3 */
            {
            delta = (double) (curve->params.closed ? *numPts : *numPts - 1);
            if (delta > 0.0)
                delta = 1.0 / delta;
            for (i=0, x=0.0; i < *numPts; i++, pPtr++, x += delta)
                bspcurv_evaluateCurvePoint (pPtr, NULL, curve, x);

            status = SUCCESS;
            }
        }
    else if (data)                              /* case 4 */
        {
        tol = *data;
        bvector <DPoint3d> points;
        // TFS1226
        curve->AddStrokes (points, tol, 0.0 /* no angle tol */);
        *numPts = (int)points.size ();
        *pts = DPoint3dOps::MallocAndCopy (points);
        return *numPts > 0 ? SUCCESS : ERROR;
        }
    else
        status = ERROR;

    return status;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
