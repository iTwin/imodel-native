/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "msbsplinemaster.h"
//#include "../memory/msvarray.fdf"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#define LINEAR_CURVATURE    1.0e-8
#define NAT_LOG_4           (1.386294361)
#define MAX_ORDER_PLUS_1    (MAX_ORDER+1)
#define NEAR_ZERO           0.0001
#define BUFFER_SIZE         1024
#define MAX_ITER            15
#define MAX_CHORD_ITER      25
#define MAX_DEPTH           10
#define TOLERANCE_ParameterSpace 1.0e-5
#define TOLERANCE_Relative  0.0005

/*---------------------------------------------------------------------------------**//**
* Note: See Farin's book (3rd edition, section 9.6) for ref.
* @bsimethod                                                    Lu.Han          03/93
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_c2CubicInterpolatePoles
(
DPoint3d        *outPts,            /* <= poles */
double          *outWts,            /* <= weights or NULL */
double          *knots,             /* <= knots or NULL */
double          *inParams,          /* => u parameters or NULL */
DPoint3d        *points,            /* => points to be interpolated (first = last if closed) */
DPoint3d        *endTangents,       /* => end tangents or NULL */
double          *weights,           /* => weights or NULL */
BsplineParam    *bsplineParams,     /* => B-Spline parameters */
int             numPoints           /* => number of points (incl redundant endpt if closed) */
)
    {
    /*
    Old function used bessel end cdx to compute/scale zero/nonzero endTangents,
    and did NOT force colinearity of computed end tangents of curves that are
    geometrically closed (it was supposed to, but didn't!).  The last four
    parameters to the following function were essentially false.

    Now we produce interpolants that, if exported to AutoCAD fit-point based
    splines, will display exactly the same.  I.e., we use chord-length knot
    sequence, scale given end tangent(s) by chord length, and compute non-given
    end tangent(s) using the natural end condition.  Such "dumbing down" of our
    end conditions is a sad but necessary decision.
    */
    return bspcurv_c2CubicInterpolatePolesExt
            (outPts, outWts, knots, inParams, points, endTangents, weights,
                bsplineParams, numPoints, true, false, true, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          03/93
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_c2CubicInterpolateCurve
(
MSBsplineCurve  *curve,        /* <= cubic spline curve */
DPoint3d        *inPts,        /* => points to be interpolated */
double          *inParams,     /* => u parameters starting with 0, or NULL */
int             numPts,        /* => number of points */
bool            remvData,      /* => T: remove coincide points */
double          tolerance,     /* => max dist betw coincide pts or closed curve */
DPoint3d        *endTangents,  /* => end tangents or NULL */
bool            closedCurve    /* => if true, closed Bspline is created */
)
    {
    /*
    Old function used bessel end cdx to compute/scale zero/nonzero endTangents,
    and did NOT force colinearity of computed end tangents of curves that are
    geometrically closed (it was supposed to, but didn't!).  The last four
    parameters to the following function were essentially false.

    Now we produce interpolants that, if exported to AutoCAD fit-point based
    splines, will display exactly the same.  I.e., we use chord-length knot
    sequence, scale given end tangent(s) by chord length, and compute non-given
    end tangent(s) using the natural end condition.  Such "dumbing down" of our
    end conditions is a sad but necessary decision.
    */
    return bspcurv_c2CubicInterpolateCurveExt
            (curve, inPts, inParams, numPts, remvData, tolerance, endTangents,
                closedCurve, true, false, true, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    09/92
+---------------+---------------+---------------+---------------+---------------+------*/
static void    scaleVectorRn
(
double          *outVec,
double          *inVec,
double          factor,
int             dimension
)
    {
    double      *dp0, *dp1, *endP;

    if (dimension == 0)     return;
    for (dp0=endP=outVec, dp1=inVec, endP+=dimension; dp0 < endP; dp0++, dp1++)
        *dp0 = *dp1 * factor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    09/92
+---------------+---------------+---------------+---------------+---------------+------*/
static void    offsetPointRn
(
double          *outVec,
double          *inVec,
double          *dirVec,
double          factor,
int             dimension
)
    {
    double      *dp0, *dp1, *dp2, *endP;

    if (dimension == 0)     return;
    for (dp0=endP=outVec, dp1=inVec, dp2=dirVec, endP+=dimension;
         dp0 < endP; dp0++, dp1++, dp2++)
        *dp0 = *dp1 + *dp2 * factor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          04/92
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bspknotrm_getLeftMultiplicity
(
int             *l,                 /* <= left multiplicity */
int             *v,                 /* <= the index of the knot on the right of jth knot */
double          *newKnots,          /* <= after delete jth knots with left multiplicity l */
int             j,                  /* => the index of jth knot in old sequence*/
int             numKnots,           /* => old num knots */
double          *gKnotsP,           /* => old knots sequence */
int             order,              /* => curve order */
int             muLeft
)
    {
    int         i, num;
    double      *nKP, *gKP, *endP, knotVal;

    num = numKnots + 2*order - muLeft;
    knotVal = gKnotsP[j];
    *l = -1;
    *v = -1;
    for (nKP=endP=newKnots, gKP=gKnotsP, endP+=num; nKP < endP; nKP++, gKP++)
        *nKP = (*gKP < knotVal) ? *gKP : *(gKP+muLeft);

    for (i = order, *l = 1, nKP=newKnots+order; i < num; i++, nKP++)
        {
        if (fabs (*nKP - knotVal) < fc_1em15)
            {
            *l = *l + 1;
            }
        }

    num -= order;
    for (i = order - 1; i < num; i++)
        {
        if ((newKnots[i] <= knotVal) && (knotVal < newKnots[i+1]))
            {
            *v = i + 1;
            }
        else if (knotVal < newKnots[i])
            {
            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          03/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspknotrm_getKnotFactor
(
double          *factor,           /* <= knot factor for jth knot */
DPoint3d        **newPoles,        /* <= after delete jth knot */
double          **auxNewPoles,
int             *newNumPoles,      /* => number of poles after delete jth knot */
int             j,                 /* => the index of the jth knot */
int             *gNumKnots,        /* => the number of knots */
double          *gKnotsP,          /* => the knot sequence */
DPoint3d        *oldPoles,         /* => old poles used in calculating newPoles array */
double          *auxOldPoles,
int             muLeft,            /* => the left multiplicity of jth knot */
int             order,             /* => the order of the spline */
int             dimension
)
    {
    int         i, l, v, n, status=SUCCESS, index, incr, d;
    double      *newKnots, *dP0, *dP1, *dP2, abs0, abs1,
                *auxTmpPoles, *auxXtmp, matrix[MAX_ORDER_PLUS_1*MAX_ORDER_PLUS_1],
                *rhsSides, kt0, *kt1P, *kt2P, *muP, *lamdaP, *betaP;
    DPoint3d    *tmpPoles, xtmp[MAX_ORDER_PLUS_1], *poleP;

    newKnots = auxXtmp = rhsSides = auxTmpPoles = NULL;    tmpPoles =NULL;

    if (NULL ==
        (newKnots = (double*)BSIBaseGeom::Malloc ((*gNumKnots+2*order-muLeft)*sizeof(double))))
        {
        status = ERROR;
        goto wrapup;
        }

    bspknotrm_getLeftMultiplicity (&l, &v, newKnots, j, *gNumKnots, gKnotsP, order, muLeft);
    if (l < 0 || v < 0)
        {
        status = ERROR;
        goto wrapup;
        }
    *newNumPoles = *newNumPoles - 1;
    index = v - order;
    n = order - l + 2;
    if (n < 1)
        return ERROR;
    if (NULL ==
        (tmpPoles = (DPoint3d*)BSIBaseGeom::Malloc (*newNumPoles * sizeof(DPoint3d))) ||
        NULL ==
        (rhsSides = (double*)msbspline_malloc (n * (3+dimension) * sizeof(double), HEAPSIG_BCRV)))
        {
        status = ERROR;
        goto wrapup;
        }

    if (dimension &&
       (NULL ==
        (auxTmpPoles = (double*)BSIBaseGeom::Malloc (*newNumPoles * dimension * sizeof(double))) ||
        NULL ==
        (auxXtmp     = (double*)msbspline_malloc (n * dimension * sizeof(double), HEAPSIG_BCRV))))
        {
        status = ERROR;
        goto wrapup;
        }

    *newPoles = tmpPoles;
    tmpPoles = NULL;
    if (dimension)
        *auxNewPoles = auxTmpPoles;

    /* Solve the matrix system */
    memset (matrix, 0, sizeof(matrix));
    muP = matrix;
    lamdaP = matrix + n;
    betaP = matrix+n-1;
    incr = n+1;

    *muP = 1.0;
    *betaP = n % 2 ? 1.0 : -1.0 ;

    kt0  = gKnotsP[j];
    kt1P = newKnots + v - order + 1;
    kt2P = newKnots + v;

    for (i=2, muP += incr, betaP += n;
         i < n;
         i++, muP += incr, lamdaP += incr, betaP += n, kt1P++, kt2P++)
        {
        *muP = (kt0 - *kt1P) / (*kt2P - *kt1P);
        *lamdaP = 1.0 - *muP;
        *betaP = -1.0  * *(betaP-n);
        }
    *lamdaP = *betaP = 1.0;

#if defined (debug)
    {
    int     d;

    printf ("\n Matrix dump: n = %d\n", n);
    for (i=0, dP0=matrix; i < n; i++)
        {
        for (d=0; d < n; d++)
            printf ("   %3.4f", *dP0++);
        printf ("\n");
        }
    }
#endif

    for (i=0, dP0=rhsSides, dP1=dP0+n, dP2=dP1+n, poleP=oldPoles+index; i < n;
         i++, dP0++, dP1++, dP2++, poleP++)
        {
        *dP0 = poleP->x;
        *dP1 = poleP->y;
        *dP2 = poleP->z;
        }

    if (dimension)
        {
        for (i=0, dP0=rhsSides+3*n, dP1=auxOldPoles+index*dimension; i < n; i++, dP0++)
            for (d=0, dP2=dP0; d < dimension; d++, dP2 += n, dP1++)
                *dP2 = *dP1;
        }

    if (SUCCESS != (status = (bsiLinAlg_solveLinearGaussPartialPivot (matrix, n, rhsSides, 3+dimension) ? SUCCESS : ERROR)))
        goto wrapup;

    for (i=0, dP0=rhsSides, dP1=dP0+n, dP2=dP1+n, poleP=xtmp; i < n;
         i++, dP0++, dP1++, dP2++, poleP++)
        {
        poleP->x = *dP0;
        poleP->y = *dP1;
        poleP->z = *dP2;
        }

    if (dimension)
        {
        for (i=0, dP0=rhsSides+3*n, dP1=auxXtmp; i < n; i++, dP0++)
            for (d=0, dP2=dP0; d < dimension; d++, dP2 += n, dP1++)
                *dP1 = *dP2;
        }

    /* Assign value to newPoles array */
    memcpy (*newPoles, oldPoles, index * sizeof (DPoint3d));
    memcpy (*newPoles+index, xtmp, (v-l-index+1) * sizeof (DPoint3d));
    memcpy (*newPoles+v-l+1, oldPoles+v-l, (*newNumPoles - (v-l+1)) * sizeof (DPoint3d));
    if (dimension)
        {
        d = dimension * sizeof(double);
        memcpy (*auxNewPoles, auxOldPoles, index * d);
        memcpy (*auxNewPoles+index*dimension, auxXtmp, (v-l-index+1) * d);
        memcpy (*auxNewPoles+(v-l+1)*dimension, auxOldPoles+(v-l)*dimension,
                (*newNumPoles - (v-l+1)) * d);
        }

    abs0 = fabs (xtmp[n-1].x);
    abs1 = fabs (xtmp[n-1].y);
    *factor = abs0 > abs1 ? abs0 : abs1;
    abs0 = fabs (xtmp[n-1].z);
    if (*factor < abs0)    *factor = abs0;

    for (d=0, dP0 = auxXtmp+(n-1)*dimension; d < dimension; d++, dP0++)
        {
        abs0 = fabs (*dP0);
        if (*factor < abs0)    *factor = abs0;
        }

wrapup:
    if (rhsSides) msbspline_free (rhsSides);
    if (auxXtmp)  msbspline_free (auxXtmp);

    if (newKnots)   BSIBaseGeom::Free (newKnots);
    if (status)
        {
        if (tmpPoles)      BSIBaseGeom::Free(tmpPoles);
        if (auxTmpPoles)   BSIBaseGeom::Free(auxTmpPoles);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          03/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspknotrm_knotSignificantFactors
(
double          *gFactorsP,                 /* <= the significant factors */
int             gNumKnots,                  /* => the number of knots */
double          *gKnotsP,                   /* => knots sequence */
DPoint3d        *gPolesP,                   /* => the poles */
double          *auxGPolesP,
 int            order,                      /* => the order of the spline */
int             dimension
)
    {
    int         i, j, tmpNum, leftMultiplicity, newNumPoles, status = SUCCESS;
    double      factor, *kiP, *kjP, *auxNewPoles, *auxOldPoles;
    DPoint3d    *newPoles, *oldPoles;

    newPoles = oldPoles = NULL;     auxNewPoles = auxOldPoles = NULL;
    tmpNum = gNumKnots + order;

    /* assign values to significant factors */
    for (i = order; i < tmpNum; i++)
        {
        leftMultiplicity = 1;
        for (j = order, kiP=gKnotsP+i, kjP=gKnotsP+j; j < i; j++, kjP++)
            {
            if (*kjP == *kiP)
                {
                leftMultiplicity++;
                }
            }

        /* A newPoles array is calculated from the original curve poles when
           leftMultiplicity == 1. If the next knot has the same value then we enter
           the else branch of the if statement. Another newPoles array is calculated,
           this time from the array calculated in round one. The round one array is freed
           since a pointer to it was saved as oldPoles. If the next knot has a new value,
           the round one array is freed in the if branch and a new round one array is
           calculated from the original poles. */
        if (leftMultiplicity == 1)
            {
            newNumPoles =  gNumKnots + order;
            if (newPoles)       BSIBaseGeom::Free(newPoles);
            if (auxNewPoles)    BSIBaseGeom::Free(auxNewPoles);
            newPoles = NULL;    auxNewPoles = NULL;

            if (SUCCESS != (status = bspknotrm_getKnotFactor (&factor, &newPoles,
                                                              &auxNewPoles, &newNumPoles,
                                                              i, &gNumKnots, gKnotsP, gPolesP,
                                                              auxGPolesP, leftMultiplicity,
                                                              order, dimension)))
                goto wrapup;
            gFactorsP[i] = factor;
            }
        else
            {
            oldPoles    = newPoles;
            auxOldPoles = auxNewPoles;
            if (SUCCESS != (status = bspknotrm_getKnotFactor (&factor, &newPoles,
                                                              &auxNewPoles, &newNumPoles,
                                                              i, &gNumKnots, gKnotsP,
                                                              oldPoles, auxOldPoles,
                                                              leftMultiplicity,
                                                              order, dimension)))
                goto wrapup;

            if (oldPoles)       BSIBaseGeom::Free (oldPoles);
            if (auxOldPoles)    BSIBaseGeom::Free (auxOldPoles);
            oldPoles = NULL;    auxOldPoles = NULL;

            gFactorsP[i] = gFactorsP[i-1] + factor;
            }
        }

wrapup:
    if (oldPoles)       BSIBaseGeom::Free (oldPoles);
    if (auxOldPoles)    BSIBaseGeom::Free (auxOldPoles);
    if (newPoles)       BSIBaseGeom::Free(newPoles);
    if (auxNewPoles)    BSIBaseGeom::Free(auxNewPoles);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          03/92
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bspknotrm_rankSignificantFactors
(
int             *rank,                /* <= ranking index for the significant factors */
double          *gFactorsP,           /* => significant factors */
int             gNumKnots,            /* => number of new interior knots */
int             order                 /* the order of the bspline */
)
    {
    int         i, itmp, iPlus, sorted, *rP, *endP;
    double      temp, ranktmp;

    for (i=1, rP=endP=rank, endP += gNumKnots; rP < endP; i++, rP++)
        *rP = i;

    do
        {
        sorted = true;
        itmp = gNumKnots + order - 2;
        for (i=order; i < itmp; i++)
            {
            if (gFactorsP[i] > gFactorsP[i+1])
                {
                iPlus = i + 1;
                sorted = false;
                temp = gFactorsP[i];
                gFactorsP[i] = gFactorsP[iPlus];
                gFactorsP[iPlus] = temp;
                ranktmp = rank[i-order];
                rank[i-order] = rank[iPlus-order];
                rank[iPlus-order] = (int)ranktmp;
                }
            }
        }
    while (!sorted);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          03/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspknotrm_locateKnotInNewSequence   /* <= index of knot in new knots */
                                                    /*    on the left of jth knot */
(
int             newNumKnots,    /* => number of new knots */
double          *gKnotsP,       /* => new knot sequence */
double          curveKnotsPj,   /* => the jth knot in the old knots sequence */
int             order           /* => the order of the spline */
)
    {
    int         i, tmpNum, u = 0;
    double      *k0P, *k1P;

    tmpNum = newNumKnots + order;
    for (i=0, k0P=k1P=gKnotsP, k1P+=1; i < tmpNum; i++, k0P++, k1P++)
        {
        if ((*k0P <= curveKnotsPj) && (*k1P > curveKnotsPj))
            {
            u = i;
            break;
            }
        }
    return u;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          03/92
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bspknotrm_discretBsplinesMatrix
(
double          *matrix,           /* <= discret B-spline matrix */
int             *uRight,           /* <= index for the leftmost nonzero entry */
int             *uLeft,            /* <= index for the rightmost nonzero entry */
int             newNumKnots,       /* => number of new knots */
double          *gKnotsP,          /* => new knots */
int             numCurveKnots,     /* => number of old knots */
double          *curveKnots,       /* => old knots */
int             order              /* => the order of the spline */
)
    {
    int         i, j, jtmp, index, tmpIndex, newIndex, r, rtmp, u = 0,
                utmp, u2, num, dumIndex;
    double      tmp[MAX_ORDER][MAX_ORDER], beta, beta1, d1, d2, tj;

    index = MAX_ORDER - 1;
    num = numCurveKnots + order;
    memset (tmp, 0, sizeof(tmp));

    for (j = 0; j < num; j++)
        {
        u = bspknotrm_locateKnotInNewSequence (newNumKnots, gKnotsP, curveKnots[j], order);
        uLeft[j] = u - order + 1;
        uRight[j] = u;
        tmp[index][1] = 1.0;
        u2 = u;
        dumIndex = index - u;
        newIndex = dumIndex - 1;
        for (r = 1; r < order; r++)
            {
            beta1 = 0.0;
            rtmp = r + 1;
            tj = curveKnots[j + r];
            for (i = u2; i <= u; i++)
                {
                d1 = tj - gKnotsP[i];
                d2 = gKnotsP[i+r] - tj;
                beta = tmp[dumIndex + i][r] / (d1 + d2);
                tmpIndex = newIndex + i;
                tmp[tmpIndex][rtmp] = d2 * beta + beta1;
                beta1 = d1 * beta;
                }
            tmp[index][rtmp] = beta1;
            u2 = u2 - 1;
            }
        jtmp = j * order;
        utmp = index - order + 1;
        for (i = 0; i < order; i++)
            {
            matrix[jtmp + i] = tmp[utmp + i][order];
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          03/92
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bspknotrm_systemSolver
(
DPoint3d        *gPolesP,          /* <= solutions                      */
double          *auxGPolesP,
int             dim,               /* => size of matrix a               */
int             order,             /* => the order of the bspline       */
int             uWidth,            /* => upper band width               */
int             lWidth,            /* => lower band width               */
double          *product,          /* => coefficient matrix             */
DPoint3d        *tmpPoles,         /* => right hand constant vector     */
double          *auxTmpPoles,
int             dimension
)
    {
    int         i, j, k, l, jtmp, jNum, ktmp, index, tmpIndex,
                newIndex0 = 0, newIndex1, minp, maxp, tmpdim, degree, num,
                tmpnum, tmplWidth;
    double      factor, *prod0P, *prod1P, *dP0, *dP1;

    degree = order - 1;
    num = 2 * order - 1;
    tmpnum = num - 1;
    tmpdim = dim - 1;

    /* LU decomposition */
    for (k = 0, dP1=auxTmpPoles; k < tmpdim; k++, dP1 += dimension)
        {
        ktmp = degree + k;
        index = k * num;
        tmpIndex = index + degree;
        minp = ((tmplWidth = k + 1 + lWidth) < dim) ? tmplWidth : dim;

        factor = product[tmpIndex];
        for (i=k+1, prod0P=product+i*tmpnum+ktmp;
             i < minp; i++, prod0P+= tmpnum)
            *prod0P /= factor;

        for (j = 0; j < uWidth; j++)
            {
            jtmp = order + k + j;
            newIndex0 = degree + k;
            newIndex1 = index + order + j;
            for (i=k+1,
                 prod0P=product+i*tmpnum+jtmp,
                 prod1P=product+i*tmpnum+newIndex0,
                 factor = product[newIndex1];
                 i < minp;
                 i++, prod0P += tmpnum, prod1P += tmpnum)
                *prod0P -= *prod1P * factor;
            }

        for (i = k+1, prod0P = product+i*tmpnum+newIndex0, dP0=auxTmpPoles+i*dimension;
             i < minp; i++, prod0P += tmpnum, dP0 += dimension)
            {
            tmpPoles[i].SumOf (tmpPoles[i], tmpPoles[k], -1.0  * (*prod0P));
            offsetPointRn (dP0, dP0, dP1, -1.0  * (*prod0P), dimension);
            }
        }

    /* back substitution for the solutions */
    for (j = tmpdim, dP0=auxTmpPoles+tmpdim*dimension; j > -1; j--, dP0 -= dimension)
        {
        maxp = ((tmplWidth = j - lWidth) > 0) ? tmplWidth : 0;
        jNum = j * num + degree;
        factor = 1.0/product[jNum];
        tmpPoles[j].Scale (tmpPoles[j], factor);
        scaleVectorRn (dP0, dP0, factor, dimension);

        l = (j < uWidth) ? j : uWidth;
        for (i = maxp, dP1=auxTmpPoles+i*dimension; i < j; i++, l--, dP1 += dimension)
            {
            factor = -product[i*num + degree + l];
            tmpPoles[i].SumOf (tmpPoles[i], tmpPoles[j], factor);
            offsetPointRn (dP1, dP1, dP0, factor, dimension);
            }
        }

    /* assign the new poles to gPolesP */
    memcpy (gPolesP, tmpPoles, dim*sizeof(DPoint3d));
    if (dimension)
        memcpy (auxGPolesP, auxTmpPoles, dim*dimension*sizeof(double));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          03/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspknotrm_leastSquare
(
DPoint3d        *gPolesP,            /* <= new poles with fewer knots */
double          *auxGPolesP,
DPoint3d        *curvePoles,         /* => old poles */
double          *auxCurvePoles,
double          *matrix,             /* => discret B-spline matrix */
int             *uRight,             /* => index for leftmost nonzero entry */
int             *uLeft,              /* => index for rightmost nonzero entry */
int             numRows,             /* => num of rows in matrix */
int             numCols,             /* => num of cols in matrix */
int             order,               /* => the order of the bspline */
int             dimension
)
    {
// EDL 7/14/18
// static analysis issues a complaint about C6385, names tmpPoles as part of
//    possible out-of-bounds expressions.
// A sensible thing to be suspicious of, indeed!
// This is old, complex code and ripping it apart seems more dangerous than ignoring it.
// 
PUSH_MSVC_IGNORE(6385)
    int         i, j, itmp, jtmp, ltmp, k, num, numProduct,
                allocSize, index, newIndex, status = SUCCESS;
    double      *product, *prodP, *endP, *mP, factor, *auxTmpPoles, *dP0, *dP1;
    DPoint3d    *tmpPoles;

    tmpPoles = NULL;    auxTmpPoles = product = NULL;
    k = order - 1;
    num = 2 * order - 1;

    numProduct = num * numCols;
    allocSize = numProduct * sizeof(double);

    /* malloc the product of transpose of matrix times matrix */
    if (NULL == (tmpPoles = (DPoint3d*)BSIBaseGeom::Malloc (numCols * sizeof(DPoint3d))) ||
        NULL == (product  = (double*)BSIBaseGeom::Malloc (allocSize)))
        {
        status = ERROR;
        goto wrapup;
        }
    if (dimension &&
        NULL == (auxTmpPoles = (double*)BSIBaseGeom::Malloc (numCols * dimension * sizeof(double))))
        {
        status = ERROR;
        goto wrapup;
        }
    memset (product, 0, allocSize);

    allocSize = dimension * sizeof(double);
    /* compute the product matrix and the right hand vector */
    for (j = 0, dP0=auxTmpPoles; j < numCols; j++, dP0+=dimension)
        {
        memset (tmpPoles+j, 0, sizeof(DPoint3d));
        if (dimension)
            memset (auxTmpPoles+j*dimension, 0, allocSize);
        jtmp = j * num + k;
        for (i = 0, dP1=auxCurvePoles; i < numRows; i++, dP1 += dimension)
            {
            if ((uLeft[i] <= j) && (j <= uRight[i]))
                {
                itmp = j - uLeft[i];
                index = i * order;
                ltmp =jtmp - itmp;
                for (prodP=endP=product+ltmp, mP=matrix+index,
                     factor=matrix[index+itmp], endP+=order;
                     prodP < endP; prodP++, mP++)
                    *prodP += *mP * factor;

                newIndex = index + itmp;
                tmpPoles[j].SumOf (tmpPoles[j], curvePoles[i], matrix[newIndex]);
                offsetPointRn (dP0, dP0, dP1, matrix[newIndex], dimension);
                }
            }
        }

    /* solve the above normal equation to find the new poles gPolesP */
    bspknotrm_systemSolver (gPolesP, auxGPolesP, numCols, order, k, k, product,
                            tmpPoles, auxTmpPoles, dimension);

wrapup:
    if (product)     BSIBaseGeom::Free(product);
    if (tmpPoles)    BSIBaseGeom::Free(tmpPoles);
    if (auxTmpPoles) BSIBaseGeom::Free(auxTmpPoles);
    return status;
POP_MSVC_IGNORE
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          04/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspknotrm_computeTolerance
(
bool            *code,                 /* <= result of test */
DPoint3d        *gPolesP,              /* => new poles */
double          *auxGPolesP,
double          *matrix,               /* => discret B-spline matrix */
int             *uLeft,                /* => index for leftmost nonzero entry */
DPoint3d        *curvePoles,           /* => old poles */
double          *auxCurvePoles,
int             order,                 /* => the order of the spline */
int             oldNumPoles,           /* => number of old poles */
DPoint3d        *tolerance,            /* => tolerance in three coordinates */
double          *auxTolerance,
int             dimension
)
    {
    int         i, j, d, allocSize, status=SUCCESS;
    double      *mP, *dP0, *dP1, *dP2, factor, *auxTol, *auxTmp,
                *auxMaxdiff, *auxGPP, *auxPoleP;
    DPoint3d    tol, tmp, maxdiff, *gPP, *poleP;

    auxTol = auxTmp = auxMaxdiff = NULL;

    factor = 1.0 / sqrt ((double) order);
    tol.Scale (*tolerance, factor);
    maxdiff.x = maxdiff.y = maxdiff.z = 0.0;
    allocSize = 0;
    if (dimension)
        {
        allocSize = dimension * sizeof(double);
        if (NULL == (auxTol     = (double*)msbspline_malloc (allocSize, HEAPSIG_BCRV)) ||
            NULL == (auxTmp     = (double*)msbspline_malloc (allocSize, HEAPSIG_BCRV)) ||
            NULL == (auxMaxdiff = (double*)msbspline_malloc (allocSize, HEAPSIG_BCRV)))
            {
            status = ERROR;
            goto wrapup;
            }
        scaleVectorRn (auxTol, auxTolerance, factor, dimension);
        memset (auxMaxdiff, 0, allocSize);
        }

    for (j = 0, poleP = curvePoles, auxPoleP = auxCurvePoles;
         j < oldNumPoles; j++, poleP++, auxPoleP += dimension)
        {
        tmp.x = tmp.y = tmp.z = 0.0;
        for (i = 0, mP=matrix+j*order, gPP=gPolesP+uLeft[j]; i < order; i++, mP++, gPP++)
            tmp.SumOf (tmp, *gPP, *mP);

        if (dimension)
            {
            memset (auxTmp, 0, allocSize);
            for (i = 0, mP=matrix+j*order, dP0=auxTmp, auxGPP=auxGPolesP+uLeft[j]*dimension;
                 i < order; i++, mP++, auxGPP+=dimension)
                offsetPointRn (dP0, dP0, auxGPP, *mP, dimension);
            }

        if ((factor = fabs(tmp.x - poleP->x)) > maxdiff.x)
            maxdiff.x = factor;
        if ((factor = fabs(tmp.y - poleP->y)) > maxdiff.y)
            maxdiff.y = factor;
        if ((factor = fabs(tmp.z - poleP->z)) > maxdiff.z)
            maxdiff.z = factor;

        for (d=0, dP0=auxMaxdiff, dP1=auxTmp, dP2=auxPoleP;
             d < dimension; d++, dP0++, dP1++, dP2++)
            {
            if ((factor = fabs(*dP1 - *dP2)) > *dP0)
                *dP0 = factor;
            }
        }

    if ((maxdiff.x < tol.x) && (maxdiff.y < tol.y) && (maxdiff.z < tol.z))
        *code = true;
    else
        *code = false;

    for (d=0, dP0=auxMaxdiff, dP1=auxTol; *code && d < dimension; d++, dP0++, dP1++)
        *code = *dP0 < *dP1;

wrapup:
    if (auxTol)     msbspline_free (auxTol);
    if (auxTmp)     msbspline_free (auxTmp);
    if (auxMaxdiff) msbspline_free (auxMaxdiff);
    return status;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          03/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspknotrm_knotsRemoval
(
int             *gNumKnots,      /* <=> number of new interior knots */
int             *gNumPoles,      /* <=> number of new poles */
int             gRational,       /* <=> if rational, weights are one */
double          **gKnotsP,       /* <=> new knot sequence */
DPoint3d        **gPolesP,       /* <=> new poles */
double          **auxGPolesP,
double          **gWeightsP,     /* <=> new weights or NULL if nonrational */
int             **rank,          /* <=> ranking number for significant factors */
int             order,           /*  => order of the spline */
int             mid,             /*  => index of knot where binary deleting take place */
int             numCurveKnots,   /*  => number of old knots */
double          *curveKnots,     /*  => old knots */
DPoint3d        *curvePoles,     /*  => old poles */
double          *auxCurvePoles,
DPoint3d        *tolerance,      /*  => tolerance in three coordinates */
double          *auxTolerance,
int             dimension
)
    {
    bool        code;
    int         i, j, itmp, sorted, status, numCols, numRows,
                newNumKnots, newNumPoles, *tmpRank, *oldRank,
                allocSize, *uRight, *uLeft;
    double      *matrix = NULL, temp, *tmpKnots, *tmpWeights, *oldKnots, *oldWeights = NULL,
                *dP, *d1P, *oldP, *old1P, *endP, *auxTmpPoles, *auxOldPoles;
    DPoint3d    *tmpPoles, *oldPoles;

    tmpKnots = tmpWeights = auxTmpPoles = auxOldPoles = NULL;   tmpRank = NULL;
    uRight = uLeft = NULL;      tmpPoles = NULL;
    newNumKnots = *gNumKnots - mid;
    newNumPoles = newNumKnots + order;
    if (NULL == (tmpKnots = (double*)BSIBaseGeom::Malloc((newNumKnots + 2*order) * sizeof(double))) ||
        NULL == (tmpPoles = (DPoint3d*)BSIBaseGeom::Malloc(newNumPoles * sizeof(DPoint3d))))
        {
        status = ERROR;
        goto wrapup;
        }

    if (dimension &&
        NULL == (auxTmpPoles = (double*)BSIBaseGeom::Malloc(newNumPoles * dimension * sizeof(double))))
        {
        status = ERROR;
        goto wrapup;
        }

    if (newNumKnots)
        {
        if (NULL == (tmpRank  = (int*)msbspline_malloc(newNumKnots * sizeof(int), HEAPSIG_BCRV)))
            {
            status = ERROR;
            goto wrapup;
            }
        }

    oldRank = *rank;
    oldKnots = *gKnotsP;
    oldPoles = *gPolesP;
    *rank = tmpRank;
    *gKnotsP = tmpKnots;
    *gPolesP = tmpPoles;

    if (dimension)
        {
        auxOldPoles = *auxGPolesP;
        *auxGPolesP = auxTmpPoles;
        }

    if (gRational)
        {
        if (NULL == (tmpWeights = (double*)BSIBaseGeom::Malloc(newNumPoles * sizeof(double))))
            {
            status = ERROR;
            goto wrapup;
            }

        oldWeights = *gWeightsP;
        *gWeightsP = tmpWeights;
        for (dP=endP = *gWeightsP, endP += newNumPoles; dP < endP; dP++)
            *dP = 1.0;
        }

    /* assign new knots by deleting mid knots from gKnots sequence according to rank */
    for (dP=d1P=endP = *gKnotsP, d1P += newNumKnots+order,
         oldP=old1P=oldKnots, old1P += *gNumKnots+order, endP += order;
         dP < endP; dP++, d1P++, oldP++, old1P++)
        {
        *dP = *oldP;
        *d1P = *old1P;
        }

    for (i = order; i < newNumKnots+order; i++)
        {
        (*gKnotsP)[i] = oldKnots[order + oldRank[mid + i - order] - 1];
        }

    /* sort gKnotsP[j], from j = 1 to newNum */
    do
        {
        sorted = true;
        itmp = newNumKnots + order - 2;
        for (j = order; j < itmp; j++)
            {
            if ((*gKnotsP)[j] > (*gKnotsP)[j+1])
                {
                sorted = false;
                temp = (*gKnotsP)[j];
                (*gKnotsP)[j] = (*gKnotsP)[j+1];
                (*gKnotsP)[j+1] = temp;
                }
            }
        }
    while (!sorted);

    /* assign new rank */
    if (newNumKnots)
        {
        for (j = 0, i = 0; j < *gNumKnots; j++)
            {
            if (oldRank[j] > mid)
                {
                (*rank)[i] = oldRank[j] - mid;
                i++;
                }
            }
        }

    /* assign new poles by first computing discrete Bsplines using Oslo Algorithm */
    numRows = numCurveKnots + order;
    numCols = newNumKnots + order;

    allocSize = order * numRows * sizeof(double);
    if (NULL == (matrix = (double*)msbspline_malloc(allocSize,           HEAPSIG_BCRV)) ||
        NULL == (uLeft  = (int*)msbspline_malloc(numRows * sizeof(int), HEAPSIG_BCRV)) ||
        NULL == (uRight = (int*)msbspline_malloc(numRows * sizeof(int), HEAPSIG_BCRV)))
        {
        status = ERROR;
        goto wrapup;
        }
    memset (matrix, 0, allocSize);

    bspknotrm_discretBsplinesMatrix (matrix, uRight, uLeft, newNumKnots,
                                     *gKnotsP, numCurveKnots, curveKnots,
                                     order);

    /* use least square to get new poles by solving normal equation */
    if (SUCCESS !=
        (status = bspknotrm_leastSquare (*gPolesP, dimension ? *auxGPolesP : NULL,
                                         curvePoles, auxCurvePoles,
                                         matrix, uRight, uLeft,
                                         numRows, numCols, order, dimension)))
        goto wrapup;

    /* fix two end points and their weights, consider rational case */
    (*gPolesP)[0] = curvePoles[0];
    (*gPolesP)[newNumPoles-1] = curvePoles[numCurveKnots+order-1];

    if (dimension)
        {
        allocSize = dimension * sizeof(double);
        memcpy (*auxGPolesP, auxCurvePoles, allocSize);
        memcpy (*auxGPolesP + (newNumPoles-1)*dimension,
                auxCurvePoles + (numCurveKnots+order-1)*dimension, allocSize);
        }

    /* compute the norm of the error of new approximation */
    if (SUCCESS !=
        (status = bspknotrm_computeTolerance (&code, *gPolesP,
                                              dimension ? *auxGPolesP : NULL,
                                              matrix, uLeft,
                                              curvePoles, auxCurvePoles,
                                              order, numRows, tolerance, auxTolerance,
                                              dimension)))
        goto wrapup;

    if (code)
        {
        /* Knots removed, so free original data */
        *gNumKnots = newNumKnots;
        *gNumPoles = newNumPoles;
        if (oldRank)     msbspline_free (oldRank);
        if (oldKnots)    BSIBaseGeom::Free (oldKnots);
        if (oldPoles)    BSIBaseGeom::Free (oldPoles);
        if (auxOldPoles) BSIBaseGeom::Free (auxOldPoles);
        if (gRational && oldWeights)
            BSIBaseGeom::Free (oldWeights);
        }
    else
        {
        /* Knots not removed, so restore original data */
        if (tmpKnots)    BSIBaseGeom::Free(tmpKnots);
        if (tmpPoles)    BSIBaseGeom::Free(tmpPoles);
        if (auxTmpPoles) BSIBaseGeom::Free(auxTmpPoles);
        if (tmpRank)     msbspline_free(tmpRank);
        *rank = oldRank;
        *gKnotsP = oldKnots;
        *gPolesP = oldPoles;
        if (dimension)
            *auxGPolesP = auxOldPoles;

        if (gRational)
            {
            if (tmpWeights) BSIBaseGeom::Free(tmpWeights);
            *gWeightsP = oldWeights;
            }
        }

wrapup:
    if (matrix) msbspline_free(matrix);
    if (uRight) msbspline_free(uRight);
    if (uLeft)  msbspline_free(uLeft);
    if (status)
        {
        if (tmpPoles)       BSIBaseGeom::Free(tmpPoles);
        if (auxTmpPoles)    BSIBaseGeom::Free(auxTmpPoles);
        if (tmpKnots)       BSIBaseGeom::Free(tmpKnots);
        if (tmpWeights)     BSIBaseGeom::Free(tmpWeights);
        if (tmpRank)        msbspline_free(tmpRank);
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    09/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspknotrm_reduceKnots
(
MSBsplineCurve  *curve,                /* <=> open curve to simplify */
double          **auxPoles,            /* => extra dimensional poles */
DPoint3d        *tolerance,            /* => tolerance in each coordinates */
double          *auxTolerance,         /* => extra dimensional tolerances */
int             dimension              /* => number of extra dimensions */
)
    {
    int             i, numIter, mid, status, *rankP, allocSize;
    double          *factorsP, *auxOrigPoles;
    MSBsplineCurve  origCurve;

    if (curve->params.numKnots < 1)
        return SUCCESS;

    memset (&origCurve, 0, sizeof(origCurve));
    rankP = NULL;       factorsP = auxOrigPoles = NULL;

    /* Make copy of the original curve for later use */
    if (SUCCESS != (status = bspcurv_copyCurve (&origCurve, curve)))
        goto wrapup;
    if (dimension)
        {
        allocSize = curve->params.numPoles * dimension * sizeof(double);
        if (NULL == (auxOrigPoles = (double*)BSIBaseGeom::Malloc (allocSize)))
            {
            status = ERROR;
            goto wrapup;
            }
        memcpy (auxOrigPoles, *auxPoles, allocSize);
        }

    /* number of iterations suggested by Tom Lyche */
    numIter = (int) (log10 ((double) curve->params.numKnots) / log10(2.0)) + 2;

    /* malloc the memory needed for the knot factors and rank arrays */
    if (NULL ==
        (factorsP = (double*)msbspline_malloc ((curve->params.numKnots + curve->params.order)
                                    * sizeof(double),                     HEAPSIG_BCRV)) ||
        NULL ==
        (rankP    = (int*)msbspline_malloc (curve->params.numKnots * sizeof(int), HEAPSIG_BCRV)))
        {
        status = ERROR;
        goto wrapup;
        }

    /* determine the significancy factor for each interior knot */
    bspknotrm_knotSignificantFactors (factorsP, curve->params.numKnots, curve->knots,
                                      curve->poles, dimension ? *auxPoles : NULL,
                                      curve->params.order, dimension);

    /* rank the significancy factors */
    bspknotrm_rankSignificantFactors (rankP, factorsP, curve->params.numKnots,
                                      curve->params.order);

    /* remove interior knots by binary search to satisfy the tolerance */
    for (i = 0, mid = (curve->params.numKnots + 1) / 2;
         i < numIter; i++)
        {
        /* mid is the number of knots to be removed */
        if (curve->params.numKnots == 0)
            break;

        /* if knots can be removed, return new knots */
        if ((status = bspknotrm_knotsRemoval (&curve->params.numKnots,
                                              &curve->params.numPoles,
                                              curve->rational, &curve->knots,
                                              &curve->poles, auxPoles,
                                              &curve->weights,
                                              &rankP, curve->params.order,
                                              mid, origCurve.params.numKnots,
                                              origCurve.knots,
                                              origCurve.poles, auxOrigPoles,
                                              tolerance, auxTolerance,
                                              dimension)) == ERROR)
            goto wrapup;

        mid = (curve->params.numKnots % 2) ? (mid + 1) / 2 : mid / 2;
        /* if no knots to remove, go out */
        if (mid == 0)
            break;
        }

wrapup:
    if (factorsP)       msbspline_free (factorsP);
    if (rankP)          msbspline_free (rankP);
    if (auxOrigPoles)   BSIBaseGeom::Free (auxOrigPoles);
    bspcurv_freeCurve (&origCurve);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* NOTE The terminologies follow the ones in Lyche and Morken "A data-reduction strategy for splines with applications to the approximation of
* functions and data, IMA J. of Num. Ana, 1988, 8, 185-208.
* @bsimethod                                                    Lu.Han          03/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_curveDataReduction
(
MSBsplineCurve  *outCurve,        /* <=>  new curve with fewer knots and poles */
MSBsplineCurve  *inCurve,         /* => input curve */
DPoint3d        *tolerance        /* => tolerance in each coordinates */
)
    {
    int             status;
    double          start, end;
    MSBsplineCurve  curve;

    memset (&curve, 0, sizeof(curve));

    /* if curve is closed, make it open */
    if (inCurve->params.closed)
        {
        if (SUCCESS != (status = bspcurv_openCurve (&curve, inCurve, 0.0)))
            goto wrapup;
        }
    else
        {
        if (SUCCESS != (status = bspcurv_copyCurve (&curve, inCurve)))
            goto wrapup;
        }

    mdlBspline_normalizeCurveKnots (&start, &end, &curve);

    /* find the number of interior knots */
    curve.params.numKnots = bspknot_numberKnots (curve.params.numPoles,
                                                 curve.params.order,
                                                 curve.params.closed) -
                                                 2 * curve.params.order;

    /* Do the knot reduction on the open curve */
    if (SUCCESS != (status = bspknotrm_reduceKnots (&curve, NULL, tolerance, NULL, 0)))
        goto wrapup;

    /* Load the knot vectors of the curve structure */
    if (inCurve->params.closed)
        bspcurv_closeCurve (&curve, &curve);

    if (outCurve == inCurve)
        bspcurv_freeCurve (outCurve);

    *outCurve = curve;
    mdlBspline_unNormalizeCurveKnots (outCurve, start, end);
wrapup:
    if (status)
        bspcurv_freeCurve (&curve);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          05/93
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bspcurv_newKnots
(
double          *knots,
int             *num,
int             order
)
    {
    int         i, j, numOld, numNew, numTmp[10], jump, count,
                index0, index1;

    /* Absolute Heuristic Approch */
    numOld = *num + order;
    memset (numTmp, 0, 10 * sizeof(int));

    /* Find the distribution of knots in eack sub-interval */
    for (i = order; i < numOld; i++)
        {
        if (knots[i] < 0.1)
            numTmp[0]++;
        else if (knots[i] < 0.2)
            numTmp[1]++;
        else if (knots[i] < 0.3)
            numTmp[2]++;
        else if (knots[i] < 0.4)
            numTmp[3]++;
        else if (knots[i] < 0.5)
            numTmp[4]++;
        else if (knots[i] < 0.6)
            numTmp[5]++;
        else if (knots[i] < 0.7)
            numTmp[6]++;
        else if (knots[i] < 0.8)
            numTmp[7]++;
        else if (knots[i] < 0.9)
            numTmp[8]++;
        else if (knots[i] < 1.0)
            numTmp[9]++;
        }

    /* Reassign the knots according to the above distribution */
    for (i = *num = count = 0; i < 10; i++)
        {
        if ((numNew = (int) sqrt((double) numTmp[i])) > 0)
            {
            jump = numTmp[i] / numNew;
            index0 = order + *num;
            index1 = order + count;
            for (j = 0; j < numNew; j++)
                knots[index0 + j] = knots[index1+j*jump];
            count += numTmp[i];
            *num += numNew;
            }
        }

    for (i = 0; i < 4; i++)
        knots[order + *num + i] = 1.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          05/93
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspcurv_cubicDataReduce
(
MSBsplineCurve  *outCurve,          /* cubic non-rational approx curve */
MSBsplineCurve  *inCurve,           /* input curve */
double          tolerance           /* max deviation bwt two curves */
)
    {
    int         status, allocSize, numKnots, *uRight, *uLeft, order,
                numRows, numCols;
    double      *matrix;
    MSBsplineCurve  curve;

    matrix = NULL;      uRight = uLeft = NULL;
    memset (&curve, 0, sizeof(MSBsplineCurve));

    /* If curve is closed, make it open */
    if (inCurve->params.closed)
        {
        if (SUCCESS != (status = bspcurv_openCurve (&curve, inCurve, 0.0)))
            goto wrapup;
        }
    else
        {
        if (SUCCESS != (status = bspcurv_copyCurve (&curve, inCurve)))
            goto wrapup;
        }

    /* Find the number of interior knots */
    curve.params.numKnots =
    numKnots =  bspknot_numberKnots (curve.params.numPoles,
                                     curve.params.order,
                                     curve.params.closed) - 2 * curve.params.order;
    /* Check if there are less than six poles */
    if (curve.params.numKnots <= 2)
        {
        *outCurve = curve;
        return SUCCESS;
        }

    /* Determine the number of knots to be left */
    order = curve.params.order;
    bspcurv_newKnots (curve.knots, &curve.params.numKnots, order);
    curve.params.numPoles = order + curve.params.numKnots;

    numRows = numKnots + order;
    numCols = curve.params.numKnots + order;
    allocSize = order * numRows * sizeof(double);
    if (NULL == (matrix = (double*)msbspline_malloc(allocSize,           HEAPSIG_BCRV)) ||
        NULL == (uLeft  = (int*)msbspline_malloc(numRows * sizeof(int), HEAPSIG_BCRV)) ||
        NULL == (uRight = (int*)msbspline_malloc(numRows * sizeof(int), HEAPSIG_BCRV)))
        {
        status = ERROR;
        goto wrapup;
        }

    memset (matrix, 0, allocSize);
    bspknotrm_discretBsplinesMatrix (matrix, uRight, uLeft, curve.params.numKnots,
                                     curve.knots, numKnots, inCurve->knots, order);

    if (SUCCESS != (status = bspknotrm_leastSquare (curve.poles, NULL,
                                                    inCurve->poles, NULL,
                                                    matrix, uRight, uLeft,
                                                    numRows, numCols, order, 0)))
        goto wrapup;

    /* fix two end points and their weights, consider rational case */
    curve.poles[0] = inCurve->poles[0];
    curve.poles[curve.params.numPoles-1] =
    inCurve->poles[inCurve->params.numPoles-1];

    /* Load the knot vectors of the curve structure */
    if (inCurve->params.closed)
        bspcurv_closeCurve (&curve, &curve);

    if (outCurve == inCurve)
        bspcurv_freeCurve (outCurve);
    *outCurve = curve;

wrapup:
    if (matrix) msbspline_free(matrix);
    if (uRight) msbspline_free(uRight);
    if (uLeft)  msbspline_free(uLeft);

    if (status)
        bspcurv_freeCurve (&curve);

    return status;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          05/95
+---------------+---------------+---------------+---------------+---------------+------*/
bool MSBsplineCurve::CloneArcLengthCompatibleCurves
(
bvector<MSBsplineCurvePtr> &outputCurves,/* <= array of output curves */
bvector<MSBsplineCurvePtr> const &inputCurves,           /* => array of input curves */
bool enableReverse,         /* => allows reversing output */
bool openAll                /* => forces opening */
)
    {
    int             i, j, k, highestOrder, highestDegree, bezier, numSegments,
                    rational, sameNum, sameKnots, sameMult, newMult = 0, *starts=NULL,
                    index, done, sum, status = SUCCESS, allocSize,
                    *numDistinct = NULL, **knotMultiplicity = NULL,
                    *tags = NULL, allClosed, *maxMult, maxNumKnots = 0;
    double          curveTolerance, knotTolerance, avgKnot,
                    **distinctKnots = NULL, *knots = NULL,
                    *length = NULL;
    MSBsplineCurve  *cvP;
    size_t numCurves = inputCurves.size ();
    outputCurves.clear ();
    if (numCurves == 1) 
        outputCurves.push_back (inputCurves[0]->CreateCopy ());
    if (numCurves < 2)
        return true;

    if (numCurves > MAX_POLES)      return false;
    
    numDistinct      = (int*)BSIBaseGeom::Malloc (numCurves * sizeof(int));
    knotMultiplicity = (int**)BSIBaseGeom::Malloc (numCurves * sizeof(int*));
    tags             = (int*)BSIBaseGeom::Malloc (numCurves * sizeof(int));
    distinctKnots    = (double**)BSIBaseGeom::Malloc (numCurves * sizeof(double*));
    knots            = (double*)BSIBaseGeom::Malloc (numCurves * sizeof(double));
    length           = (double*)BSIBaseGeom::Malloc (numCurves * sizeof(double));

    memset(distinctKnots, 0, numCurves * sizeof(double*));
    memset(knotMultiplicity, 0, numCurves * sizeof(int*));
    maxMult = NULL;
    knotTolerance = fc_hugeVal;

    /* We need to find the curve with the highest degree, because the others
        will have to be raised to that degree. Also check periodicity and
        rationality of all the curves. If all the curves are Bezier then we
        will not have to do the refining part, just up the degree */
    highestOrder = inputCurves[0]->params.order;
    allClosed    = inputCurves[0]->params.closed;
    rational     = inputCurves[0]->rational;
    bezier       = inputCurves[0]->params.numPoles == inputCurves[0]->params.order;

    for (i=1; i < numCurves; i++)
        {
        cvP = inputCurves[i].get ();
        highestOrder = cvP->params.order > highestOrder ? cvP->params.order : highestOrder;
        allClosed = allClosed && cvP->params.closed;
        rational  = rational || cvP->rational;
        bezier    = bezier && (cvP->params.numPoles == cvP->params.order);
        }
    highestDegree = highestOrder - 1;

    for (i=0; i < numCurves; i++)
        {
        outputCurves.push_back (MSBsplineCurve::CreatePtr ());
        cvP = outputCurves.back ().get ();
        /* prepare curves into chain of beziers */
        if (SUCCESS != (status = bspproc_prepareCurve (cvP, &numSegments, &starts,
            inputCurves[i].get ())))
            goto wrapup;
        // We make no further use of the starts array ...
        if (starts != nullptr)
            BSIBaseGeom::Free (starts);
        /* Raise degree if necessary */
        if (cvP->params.order < highestOrder)
            {
            if (SUCCESS != (status = bspcurv_elevateDegree (cvP, cvP, highestDegree)))
                goto wrapup;
            }

        /* Make rational if necessary */
        if (rational && ! cvP->rational)
            {
            if (SUCCESS != (status = bspcurv_makeRational (cvP, cvP)))
                goto wrapup;
            }

        if (!bezier)
            {
            curveTolerance = bspknot_knotTolerance (cvP);
            sameKnots = cvP->params.numPoles + cvP->params.order; /*OPEN*/

            /* we want to find the smallest knot tolerance of all curves */
            if ( i == 0  ||  curveTolerance < knotTolerance)
                knotTolerance = curveTolerance;

            /* and the maximum number of knots in any curve */
            if ( i == 0  ||  maxNumKnots < sameKnots)
                maxNumKnots = sameKnots;
            }
        }

    if (bezier)
        {
        status = SUCCESS;
        goto wrapup;
        }

    /* refine all the knot vectors, so curve have the same number of knots */

    /* find the distinct knots of all the OPEN(!!) curves */

    allocSize = maxNumKnots * sizeof(int);
    if (NULL == (maxMult = (int*)msbspline_malloc (allocSize, HEAPSIG_BCRV)))
        {
        status = ERROR;
        goto wrapup;
        }
    memset (maxMult, 0, allocSize);
    for (i=0; i < numCurves; i++)
        {
        cvP = outputCurves[i].get ();
        allocSize = cvP->params.numPoles + cvP->params.order;
        if (NULL ==
            (distinctKnots[i]    = (double*)msbspline_malloc (allocSize * sizeof(double), HEAPSIG_BCRV)) ||
            NULL ==
            (knotMultiplicity[i] = (int*)msbspline_malloc (allocSize * sizeof(int),    HEAPSIG_BCRV)))
            {
            status = ERROR;
            goto wrapup;
            }
        bspknot_getKnotMultiplicity (distinctKnots[i], knotMultiplicity[i], numDistinct + i,
                                     cvP->knots, cvP->params.numPoles, cvP->params.order,
                                     cvP->params.closed, knotTolerance);
        for (j=0; j < numDistinct[i]; j++)
            maxMult[j] = maxMult[j] > knotMultiplicity[i][j] ?
                         maxMult[j] : knotMultiplicity[i][j];
        }

    /* find out if they all have the same number of distinct knots and
       if they are the same distinct knots */
    sameNum = true;
    sameKnots = true;
    sameMult = true;
    for (i=1; i < numCurves; i++)
        {
        if (!sameNum || numDistinct[i-1] != numDistinct[i])
            {
            sameNum = false;
            break;
            }
        else
            {
            if (sameKnots && sameMult)
                {
                /* only check if all previous have same distinct knots
                   NOTE: this loop assumes all curves are open !!! */
                for (j=1; j < numDistinct[0]-1; j++)
                    {
                    sameKnots =
                        fabs (distinctKnots[i-1][j] - distinctKnots[i][j]) <= knotTolerance;
                    sameMult =
                        knotMultiplicity[i-1][j] == knotMultiplicity[i][j];
                    if (!sameKnots || !sameMult)
                        break;
                    }
                }
            }
        }

    if (sameNum)
        {
        if (!sameKnots || !sameMult)
            {
            /* make all interior knots of maximum multiplicity */
            for (i=0; i < numCurves; i++)
                {
                cvP = outputCurves[i].get ();
                for (j=1; j < numDistinct[i]-1; j++)
                    {
                    bspknot_addKnot (cvP, distinctKnots[i][j], knotTolerance, maxMult[j],
                                     false);
                    }
                }
            }

        index = highestOrder; /* assumes all curves are open */
        for (i=1; i < numDistinct[0] - 1; i++)
            {
            /* find the average of each distinct knot between curves */
            for (j=0, avgKnot=0.0; j < numCurves; j++)
                 avgKnot += distinctKnots[j][i];
            avgKnot /= numCurves;

            for (j=0; j < numCurves; j++)
                {
                cvP = outputCurves[j].get ();
                newMult = (sameKnots && sameMult) ? knotMultiplicity[j][i] : maxMult[i];
                for (k=0; k < newMult; k++)
                    cvP->knots[index + k] = avgKnot;
                }
            index += newMult;
            }
        }

    else /* general case */
        {
        done = false;
        index = highestOrder;
        newMult = index - 1;
        for (i = 0; i < numCurves; i++)
            {
            length[i] = outputCurves[i]->Length ();
            }
        do
            {
            sum = 0;
            for (i = 0; i < numCurves; i++)
                {
                cvP = outputCurves[i].get ();
                if (cvP->params.numPoles == index)
                    sum += 1;
                knots[i] = cvP->LengthBetweenKnots (cvP->knots[0], cvP->knots[index]);
                knots[i] /= length[i];
                }
            if (sum == numCurves) /* been through all knots of all curves */
                done = true;
            else
                {
                /* sort the knots from smallest to largest */
                util_tagSort (tags, knots, (int)numCurves);

                /* add the knot to the curves to full multiplicity  */
                for (i = 0; i < numCurves; i++)
                    {
                    if (i != tags[0])
                        {
                        double actualDistance;
                        cvP = outputCurves[i].get ();
                        cvP->FractionAtSignedDistance (0.0, knots[tags[0]] * length[i],
                                        avgKnot, actualDistance);
                        bspknot_addKnot (cvP, avgKnot, knotTolerance, newMult, false);
                        }
                    }
                index += newMult;
                }
            }
        while (!done);
        }

    /* close all curves if allClosed == true */
    if (allClosed && !openAll)
        for (i=0; i< numCurves; i++)
            {
            cvP = outputCurves[i].get ();
            if (SUCCESS != (status = bspcurv_closeCurve (cvP, cvP)))
                goto wrapup;
            }

wrapup:
    if (maxMult)        msbspline_free (maxMult);
    for (i=0; i < numCurves; i++)
        {
        if (distinctKnots[i])       msbspline_free (distinctKnots[i]);
        if (knotMultiplicity[i])    msbspline_free (knotMultiplicity[i]);
        }
    
    BSIBaseGeom::Free (knotMultiplicity);
    BSIBaseGeom::Free (distinctKnots);
    BSIBaseGeom::Free (length);
    BSIBaseGeom::Free (numDistinct);
    BSIBaseGeom::Free (tags);
    BSIBaseGeom::Free (knots);

    if (status)
        outputCurves.clear ();
    return status == SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* To solve for x[0], ..., x[dimension-1] from lower[i] * x[i-1] + diagonal[i] * x[i] + upper[i] * x[i+1] = right[i] i=0,..., dimension-1, with
* lower[0] = upper[dimension-1] = 0.0
* @bsimethod                                                    Lu.Han          05/95
+---------------+---------------+---------------+---------------+---------------+------*/
static int     tridiagonalSolver
(
double  *solutionP,
double  *lowerP,
double  *diagonalP,
double  *upperP,
double  *rightSideP,
int     dimension
)
    {
    int     k;
    double  m;

    if (dimension <= 0)
        return (ERROR);

    if (dimension == 1)
        {
        if (fabs(diagonalP[0]) < 1.0e-14)
            return (ERROR);
        else
            solutionP[0] = rightSideP[0] / diagonalP[0];
        return (SUCCESS);
        }

    ScopedArray <double>bArray (dimension, rightSideP);
    ScopedArray <double>dArray (dimension, diagonalP);
    double *b = bArray.GetData ();
    double *d = dArray.GetData ();
    
    for (k = 1; k < dimension; k++)
        {
        if (fabs(d[k-1]) < 1.0e-14)
            return (ERROR);
        else
            {
            m = lowerP[k] / d[k-1];
            d[k] -= m * upperP[k-1];
            b[k] -= m * b[k-1];
            }
        }
    if (fabs(d[dimension-1]) < 1.0e-14)
        return (ERROR);
    else
        {
        solutionP[dimension-1] = b[dimension-1] / d[dimension-1];
        for (k = dimension-2; k >= 0; k--)
            solutionP[k] = (b[k] - upperP[k] * solutionP[k+1]) / d[k];
        }

    return (SUCCESS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          05/95
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt computeEqualChordByNumber
(
bvector<DPoint3d> &outpoints,  //!< [out] stroke points
bvector<double> &outparams,    //!< [out] fraction parameters for the strokes
MSBsplineCurve  *curveP,    /* => input B-spline curve */
size_t          numSeg,     /* => number of chords: number of pointsP - 1 */
int             initMethod      /* => 0 for equal parameter step, 1 for equal arc length */
)
    {
    bool        converge;
    int         status = ERROR, i, j, allocSize, numParams;
    double      paramTol, step, deltaPlus = 0, deltaMinus;
    DPoint3d    ends[2], tmp;
    outparams.clear ();
    outpoints.clear ();
    if (numSeg <= 0)
        return ERROR;

    if (numSeg <= 1)
        {
        outparams.push_back(0.0);
        outparams.push_back(1.0);
        DPoint3d xyz0, xyz1;
        bspcurv_extractEndPoints (&xyz0, &xyz1, curveP);
        outpoints.push_back (xyz0);
        outpoints.push_back (xyz1);
        return (SUCCESS);
        }

    allocSize = ((numParams = (int)numSeg - 1)) * sizeof(double);
    ScopedArray <double>upDiagArray (numParams);    double   *upDiag     = upDiagArray.GetData ();
    ScopedArray <double>lowDiagArray (numParams);   double   *lowDiag    = lowDiagArray.GetData ();
    ScopedArray <double>midDiagArray (numParams);   double   *midDiag    = midDiagArray.GetData ();
    ScopedArray <double>oldParamsArray (numParams); double   *oldParams  = oldParamsArray.GetData ();
    ScopedArray <double>solutionsArray (numParams); double   *solutions  = solutionsArray.GetData ();
    ScopedArray <double>rightSideArray (numParams); double   *rightSide  = rightSideArray.GetData ();
    ScopedArray <DPoint3d>pointsArray (numParams);  DPoint3d *points     = pointsArray.GetData ();
    ScopedArray <DVec3d>tangentsArray (numParams);  DVec3d   *tangents   = tangentsArray.GetData ();
    ScopedArray <DVec3d>diffArray   (numParams+1);  DVec3d   *diff       = diffArray.GetData ();

    /* Set initial guess parameters */
    //static bool s_initByArcLength = true;
    if (initMethod == 1)
        {
        double totalLength;
        totalLength = curveP->Length ();
        step = totalLength / numSeg;
        double previousParameter = 0.0;
        for (i = 0; i < numParams; i++)
            {
            curveP->FractionAtSignedDistance (previousParameter, step, oldParams[i], totalLength);
            previousParameter = oldParams[i];
            }
        }
    else
        {
        step = 1.0 / numSeg;
        for (i = 0; i < numParams; i++)
            oldParams[i] = (i + 1) * step;
        }

    /* Multi variate Newton iterations */
    paramTol = TOLERANCE_ParameterSpace;
    bspcurv_evaluateCurvePoint (&ends[0], NULL, curveP, 0.0);
    bspcurv_evaluateCurvePoint (&ends[1], NULL, curveP, 1.0);
    for (i = 0; i < MAX_CHORD_ITER; i++)
        {
        converge = true;
        for (j = 0; j < numParams; j++)
            {
            bspcurv_evaluateCurvePoint (&points[j], &tangents[j], curveP, oldParams[j]);
            if (j == 0)
                diff[0].DifferenceOf (points[0], ends[0]);
            else
                diff[j].DifferenceOf (points[j], points[j-1]);
            }
        diff[numParams].DifferenceOf (ends[1], points[numParams-1]);

        /* Compute the Jacobian matrix */
        lowDiag[0] = upDiag[numParams-1] = 0.0;
        tmp.SumOf (diff[0], diff[1]);
        midDiag[0] = 2.0 * tangents[0].DotProduct (tmp);
        upDiag[0] = (numParams == 1) ? 0.0 : -2.0 * tangents[1].DotProduct (diff[1]);
        rightSide[0] = diff[0].DotProduct (diff[0]) - diff[1].DotProduct (diff[1]);

        if (numParams > 1)
            {
            lowDiag[numParams-1] = -2.0 * tangents[numParams-2].DotProduct (diff[numParams-1]);
            tmp.SumOf (diff[numParams-1], diff[numParams]);
            midDiag[numParams-1] = 2.0 * tangents[numParams-1].DotProduct (tmp);
            rightSide[numParams-1] = diff[numParams-1].DotProduct (diff[numParams-1]) -
                diff[numParams].DotProduct (diff[numParams]);
            }

        for (j = 1; j < numParams-1; j++)
            {
            rightSide[j] = diff[j].DotProduct (diff[j]) -        diff[j+1].DotProduct (diff[j+1]);
            upDiag[j]  = -2.0 * tangents[j+1].DotProduct (diff[j+1]);
            tmp.SumOf (diff[j], diff[j+1]);
            midDiag[j] = 2.0 * tangents[j].DotProduct (tmp);
            lowDiag[j] = -2.0 * tangents[j-1].DotProduct (diff[j]);
            }

        /* Solve the tridiagonal system */
        if (SUCCESS != tridiagonalSolver (solutions, lowDiag, midDiag, upDiag,
            rightSide, numParams))
            {
            status = ERROR;
            goto wrapup;
            }

        /* Check if the current params converge */
        double maxDelta = DoubleOps::MaxAbs (solutions, (size_t)numParams);
        converge = maxDelta <= paramTol;

        if (converge || i == MAX_CHORD_ITER - 1)
            {
            outparams.push_back (0.0);
            for (int i = 0; i < numParams; i++)
                outparams.push_back (oldParams[i]);
            outparams.push_back (1.0);
            outpoints.push_back (ends[0]);
            for (int i = 0; i < numParams; i++)
                outpoints.push_back (points[i]);
            outpoints.push_back (ends[1]);
            status = converge ? SUCCESS : ERROR;
            goto wrapup;
            }
        else
            {
            /* Also need to restrict the divergence of the oldParams */
            for (j = 0; j < numParams; j++)
                {
                deltaMinus = (j == 0) ? oldParams[j] / 3.0 : deltaPlus;
                deltaPlus = (j == numParams - 1) ? (1.0 - oldParams[j]) / 3.0 :
                            (oldParams[j+1] - oldParams[j]) / 3.0;

                if (solutions[j] < -1.0 * deltaPlus)
                    solutions[j] = -1.0 * deltaPlus;
                else if (solutions[j] > deltaMinus)
                    solutions[j] = deltaMinus;
                oldParams[j] -= solutions[j];
                if (oldParams[j] > 1.0)
                    oldParams[j] = 1.0;
                else if (oldParams[j] < 0.0)
                    oldParams[j] = 0.0;
                }
            }
        }

wrapup:
    return status;
    }

bool MSBsplineCurve::StrokeFixedNumberWithEqualChordLength
(
bvector<DPoint3d> &points,  //!< [out] stroke points
bvector<double> &params,    //!< [out] fraction parameters for the strokes
size_t          numSeg
)
    {
    if (SUCCESS == computeEqualChordByNumber (points, params, this, numSeg, 0))
        return true;
    if (SUCCESS == computeEqualChordByNumber (points, params, this, numSeg, 1))
        return true;
    return false;
    }

//! Compute points along the bspline, spaced to have equal fraction spacing.
//! The first point will be the start point of the curve.
//! The last point is the end of the curve.
void MSBsplineCurve::StrokeFixedNumberWithEqualFractionLength
(
bvector<DPoint3d> &points,  //!< [out] stroke points
bvector<double> &params,    //!< [out] fraction parameters for the strokes
size_t          numSeg      //!< [in] segment count
)
    {
    points.clear ();
    params.clear ();
    if (numSeg == 0)
        numSeg = 1;
    for (size_t i = 0; i <= numSeg; i++)
        {
        double f = (double)i / (double)numSeg;
        DPoint3d xyz;
        FractionToPoint (xyz, f);
        points.push_back (xyz);
        params.push_back (f);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          09/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int     bspcurv_convertConicToBspline
(
MSBsplineCurve  *pCurve,    /* <= rational Bspline with three poles */
DPoint3d        *pStart,    /* => starting point */
DPoint3d        *pEnd,      /* => ending point */
DPoint3d        *pInt,      /* => tangent intersection point */
double          paramS,     /* => barry centric cood of another point */
double          paramE,     /* => barry centric cood of another point */
double          paramI      /* => barry centric cood of another point */
)
    {
    int             status;

    /* Initialize parameters */
    memset (pCurve, 0, sizeof(MSBsplineCurve));
    pCurve->rational = true;
    pCurve->params.order = pCurve->params.numPoles = 3;

    /* Alocate memory */
    if (SUCCESS == (status = bspcurv_allocateCurve (pCurve)))
        {
        /* Assign weights */
        pCurve->weights[0] = pCurve->weights[2] = 1.0;
        pCurve->weights[1] = 0.5 * paramI / sqrt (paramS * paramE);
        if (pCurve->weights[1] > 1.0)
            {
            pCurve->weights[0] /= pCurve->weights[1];
            pCurve->weights[2] = pCurve->weights[0];
            pCurve->weights[1] = 1.0;
            }

        /* Assign knots */
        pCurve->knots[0] = pCurve->knots[1] = pCurve->knots[2] = 0.0;
        pCurve->knots[3] = pCurve->knots[4] = pCurve->knots[5] = 1.0;

        /* Assign poles */
        pCurve->poles[0] = *pStart;
        pCurve->poles[2] = *pEnd;
        pCurve->poles[1].Scale (*pInt, pCurve->weights[1]);
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     07/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt curveParallelToSegmentFunc
(
MSBsplineCurve *pCurve,
double *pf,             /* Objective function -- 0 when tangent at X(s)
                            is perpendicular to vector from X(s) to the given segment. */
double *pdf,            /* Derivative of objective function wrt s */
DPoint3d *pPoint2,      /* Point X(s) */
DPoint3d *pPoint3,      /* Projection of X(s) onto the segment */
double s,
DPoint3d *pPoint0,
DPoint3d *pPoint1
)
    {
    DPoint3d xyz[4];
    DPoint3d xyz0[4];
    DPoint3d tangent2, curvature2, vector32;
    double   w0[4];
    int numDeriv = 3;
    int numArrayEntry = 4;
    double lambda, dlambda;
    double UdotU, UdotV;
    DPoint3d vector01, vector02;
    DPoint3d dvector32;
    /********************************************************************
                          4
                    2            *
                *                      *
             *                            *
            0-------3-----------------------1
        0,1 = start and end points
        2 = point on curve at parameter s.
        3 = projection of 2 on chord.
        vector01 = vector from 0 to 1 = P1 - P0
        vector02 = vector from 0 to 2 = P2 - P0
        vector32 = vector from 3 to 2 = P2 - P3
        tangent2 = curve tangent at 2
        Note that P3 is controlled by P2; the projection equation is
            P3 = P0 + lambda (P1 - P0)
                where lambda = (vector02 dot vector01) / (vector01 dot vector01)
                      lambda' = tangent2 / (vector01 dot vector01)


    The function to compute is
            f = vector32 dot tangent2
              = (P2 - P3) dot tangent2
            f' = (P2' - P3') dot tangent2 + (P2 - P3) dot tangent2'
            P2' is just tangent2.
            P3' is lambda' * vector01
    If P2 happens to be at the very top of the curve (i.e. at P4) the tangent vector
        is perpendicular to vector32, and f==0.
    *************************************************************************/

    if (SUCCESS != bspcurv_computeDerivatives (xyz0, w0, pCurve, numDeriv, s, false))
        return ERROR;

    if (pCurve->rational)
        {
        bspcurv_chainRule (xyz, xyz0, w0);
        }
    else
        {
        memcpy (xyz, xyz0, numArrayEntry * sizeof(DPoint3d));
        }
    *pPoint2   = xyz[0];
    tangent2   = xyz[1];
    curvature2 = xyz[2];

    vector01.DifferenceOf (*pPoint1, *pPoint0);
    vector02.DifferenceOf (*pPoint2, *pPoint0);

    UdotU = vector01.DotProduct (vector01);
    UdotV = vector01.DotProduct (vector02);

    if (UdotV <= 0.0 || UdotV >= UdotU)
        return ERROR;

    lambda = UdotV / UdotU;
    dlambda = tangent2.DotProduct (vector01) / UdotU;
    dvector32.SumOf (tangent2, vector01, -dlambda);

    pPoint3->SumOf (*pPoint0, vector01, lambda);
    vector32.DifferenceOf (*pPoint2, *pPoint3);

    *pf = vector32.DotProduct (tangent2);

    *pdf = dvector32.DotProduct (tangent2)
         + vector32.DotProduct (curvature2);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* Search between two parameter values for the parameter at which the curve attains maximum deviation from its chord.
* @bsimethod                                                    Earlin.Lutz     07/01
+---------------+---------------+---------------+---------------+---------------+------*/
static StatusInt mdlBspline_findChordErrorPoint
(
MSBsplineCurve *pCurve, /* => curve to search */
DPoint3d *pPoint2,      /* <= Point X(*pParam) */
DPoint3d *pPoint3,      /* <= Projection of X(*pParam) onto the segment */
double *pParam,         /* parameter of max deviation */
double s0,              /* => start of interval */
double s1               /* => end of interval */
)
    {
    DPoint3d point0, point1;

    double deltas, s, f, df, step;
    double limit0, limit1;
    //static double limitFraction = 0.01;
    DPoint3d point2, point3;
    int iter;
    static int maxIter = 10;
    static double s_stepTol = 1.0e-5;
    double tStart, t, dCurr, dMax;
    static double t0 = 0.075;
    static double t1 = 0.935;
    static double dt = 0.05;

    bspcurv_evaluateCurvePoint (&point0, NULL, pCurve, s0);
    bspcurv_evaluateCurvePoint (&point1, NULL, pCurve, s1);
    deltas = s1 - s0;


    /* Brute force search for starting parameter */
    tStart = 0.0;
    dMax = 0.0;
    for (t = t0; t <= t1; t += dt)
        {
        s = s0 + t * deltas;
        if (SUCCESS != curveParallelToSegmentFunc
                    (pCurve, &f, &df, &point2, &point3, s, &point0, &point1))
            return ERROR;
        dCurr = point2.Distance (point3);
        if (tStart == 0.0 || dCurr > dMax)
            {
            dMax = dCurr;
            tStart = t;
            }
        }

    s = s0 + tStart * deltas;

    if (deltas > 0.0)
        {
        limit0 = s - dt * deltas;
        limit1 = s + dt * deltas;
        }
    else
        {
        limit0 = s + dt * deltas;
        limit1 = s - dt * deltas;
        }

    for (iter = 0; iter < maxIter; iter++)
        {
        if (SUCCESS != curveParallelToSegmentFunc
                        (pCurve, &f, &df, &point2, &point3, s, &point0, &point1))
            return ERROR;

        /* punt if we found a linear segment */
        if (!df)
            return ERROR;

        step = f / df;
        s -= step;

        if (fabs (step) < s_stepTol)
            {
            *pPoint2 = point2;
            *pPoint3 = point3;
            *pParam = s;
            return SUCCESS;
            }
        if (s < limit0)
            s = limit0;
        if (s > limit1)
            s = limit1;
        }

    return ERROR;
    }


//! Compute points along the bspline, spaced to have equal chord error (true perpendicular from chord to curve)
//! Stroke length adapts to the count and shape.
//! The first point will be the start point of the curve.
//! The last point is the end of the curve.
bool MSBsplineCurve::StrokeFixedNumberWithEqualChordError
(
bvector<DPoint3d> &points,  //!< [out] stroke points
bvector<double> &params,    //!< [out] fraction parameters for the strokes
size_t          numSeg      //!< [in] segment count
)
    {
    points.clear ();
    points.resize (numSeg);
    params.resize (numSeg);
    double minDist, maxDist;
    auto status = mdlBspline_computeEqualDeviationChordByNumber (&points[0], &params[0], &minDist, &maxDist, this, (int)numSeg);
    return status == SUCCESS;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     07/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      mdlBspline_computeEqualDeviationChordByNumber
(
DPoint3d        *pointsP,   /* <= chord points with length of numSeg+1.  CALLER MUST ALLOCATE */
double          *paramsP,   /* <= parameters for equal chord error   CALLER MUST ALLOCATE*/
double          *minDistP,  /* <= smallest distance to chord */
double          *maxDistP,  /* <= largest distance to chord */
MSBsplineCurve  *curveP,    /* => input B-spline curve */
int             numSeg     /* => number of chords: number of pointsP - 1 */
)
    {

    int numFunc  = numSeg - 1;
    if (numFunc < 1)
        return ERROR;
    numSeg = numFunc + 1;       // This is exactly what came in.  But seeing the "plus 1" convinces the bounds checker.
    if (numSeg < 1)
        return ERROR;
    int numPoint = numFunc + 2;
    if (numPoint < 1)
        return ERROR;
    DPoint3d *pPointBuffer = (DPoint3d*)_alloca ((numFunc + 2) *sizeof (DPoint3d));
    double *pParamBuffer    = (double*)_alloca ((numFunc + 2) * sizeof(double));
    double *pDistanceBuffer = (double*)_alloca ((numFunc + 1) * sizeof(double));
    double *pLeftDeriv      = (double*)_alloca (numFunc * sizeof(double));
    double *pRightDeriv     = (double*)_alloca (numFunc * sizeof(double));
    double *pDiag           = (double*)_alloca (numFunc * sizeof(double));
    double *pUpper          = (double*)_alloca (numFunc * sizeof(double));
    double *pLower          = (double*)_alloca (numFunc * sizeof(double));
    double *pStep           = (double*)_alloca (numFunc * sizeof(double));
    double *pFuncBuffer     = (double*)_alloca (numFunc * sizeof (double));
    int i, k;
    int pass;
    static int s_maxPass = 10;
    static double s_stepTol = 1.0e-6;
    double maxStep;
    DPoint3d point2, point3;
    double midParam;
    StatusInt status = ERROR;

    for (i = 0; i < numPoint; i++)
        {
        pParamBuffer[i] = (double)i / (double)numSeg;
        }

    bspcurv_evaluateCurvePoint (&pPointBuffer[0], NULL, curveP, 0.0);
    bspcurv_evaluateCurvePoint (&pPointBuffer[numPoint - 1], NULL, curveP, 1.0);
    for (pass = 0; pass < s_maxPass; pass++)
        {
        for (i = 1; i < numSeg; i++)
            {
            bspcurv_evaluateCurvePoint (&pPointBuffer[i], NULL, curveP, pParamBuffer[i]);
            }

        for (i = 0; i < numSeg; i++)
            {
            if (SUCCESS != mdlBspline_findChordErrorPoint
                            (curveP,
                            &point2, &point3,
                            &midParam,
                            pParamBuffer[i], pParamBuffer[i+1]
                            ))
                goto cleanup;
            pDistanceBuffer[i] = point2.Distance (point3);
#ifdef NOISY
            printf (" Interval %lf %lf parallel param %lf distance %lf\n",
                                pParamBuffer[i], pParamBuffer[i+1],
                                midParam, pDistanceBuffer[i]);
#endif
            }

        for (k = 0; k < numFunc; k++)
            {
            double shift = 0.001;
            double shiftedParameter = pParamBuffer[k+1] + shift;
            double dist0, dist1;
            if (SUCCESS != mdlBspline_findChordErrorPoint
                            (curveP,
                            &point2, &point3,
                            &midParam,
                            pParamBuffer[k], shiftedParameter
                            ))
                goto cleanup;
            dist0 = point2.Distance (point3);
            if (SUCCESS != mdlBspline_findChordErrorPoint
                            (curveP,
                            &point2, &point3,
                            &midParam,
                            shiftedParameter, pParamBuffer[k+2]
                            ))
                goto cleanup;
            dist1 = point2.Distance (point3);
            pLeftDeriv[k] = (dist0 - pDistanceBuffer[k]) / shift;
            pRightDeriv[k] = (dist1 - pDistanceBuffer[k + 1]) / shift;
            }

        for (k = 0; k < numFunc; k++)
            {
            pFuncBuffer[k] = pDistanceBuffer[k+1] - pDistanceBuffer[k];

            if (k > 0)
                pLower[k] = -pRightDeriv[k - 1];
            else
                pLower[k] = 0.0;

            pDiag[k] = pRightDeriv[k] - pLeftDeriv[k];

            if (k + 1 < numFunc)
                pUpper[k] = pLeftDeriv[k + 1];
            else
                pUpper[k] = 0.0;
            }

#ifdef NOISY
        printf ("                  %15lf %15lf\n", pDiag[0], pUpper[0]);
        for (k = 1; k < numFunc - 1; k++)
            printf (" %15lf %15lf %15lf\n", pLower[k], pDiag[k], pUpper[k]);
        printf (" %15lf %15lf\n", pLower[numFunc - 1], pDiag[numFunc - 1]);
#endif

        if (SUCCESS != tridiagonalSolver
                        (pStep, pLower, pDiag, pUpper, pFuncBuffer, numFunc))
            goto cleanup;

        maxStep = 0.0;
        for (k = 0; k < numFunc; k++)
            {
            double aStep = fabs (pStep[k]);
            pParamBuffer[k + 1] -= pStep[k];
            if (aStep > maxStep)
                maxStep = aStep;
            }
        if (maxStep < s_stepTol)
            {
            status = SUCCESS;
            goto cleanup;
            }
        }

cleanup:
    if (SUCCESS == status)
        {
        if (pointsP)
            {
            for (i = 0; i < numPoint; i++)
                {
                bspcurv_evaluateCurvePoint
                    (&pPointBuffer[i], NULL, curveP, pParamBuffer[i]);
                pointsP[i] = pPointBuffer[i];
                }
            }
        if (paramsP)
            {
            for (i = 0; i < numPoint; i++)
                {
                paramsP[i] = pParamBuffer[i];
                }
            }
        if (minDistP || maxDistP)
            {
            double currDist;
            double minDist, maxDist;
            minDist = 1.0e100;
            maxDist = 0.0;
            for (i = 0; i < numSeg; i++)
                {
                if (SUCCESS == mdlBspline_findChordErrorPoint
                                (curveP,
                                &point2, &point3,
                                &midParam,
                                pParamBuffer[i], pParamBuffer[i+1]
                                ))
                    {
                    currDist = point2.Distance (point3);
                    if (i == 0)
                        {
                        minDist = maxDist = currDist;
                        }
                    else
                        {
                        if (currDist < minDist)
                            minDist = currDist;
                        if (currDist > maxDist)
                            maxDist = currDist;
                        }
                    }
                }
            if (minDistP)
                *minDistP = minDist;
            if (maxDistP)
                *maxDistP = maxDist;
            }
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* Find the middle pole for a quadratic Bezier segment from coplanar tangents at
* the end poles.
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    intersectTangents
(
DPoint3d*       pPoint,
double*         pParam0,
double*         pParam1,
bool*           pbLinear,
const DRay3d*   pTan0,
const DRay3d*   pTan1,
double          tolerance
)
    {
    DPoint3d    pt0, pt1;
    double      p0, p1;
    bool        bIntersect = false;

    if (DRay3d::ClosestApproachUnboundedRayUnboundedRay (p0, p1, pt0, pt1, *pTan0, *pTan1))
        {
        double size0 = fabs(p0);
        double size1 = fabs(p1);

        if ((size0 >= fc_nearZero || size1 >= fc_nearZero)                  // rule out coincidence
            && p0 > -fc_nearZero
            && p1 < fc_nearZero
            && pt0.IsEqual (pt1, tolerance))    // planarity check
            {
            bIntersect = true;
            if (pPoint)
                *pPoint = pt0;
            if (pParam0)
                *pParam0 = p0;
            if (pParam1)
                *pParam1 = p1;
            if (pbLinear)
                *pbLinear = (size0 < fc_nearZero || size1 < fc_nearZero);
            }
        }

    return bIntersect;
    }

/*---------------------------------------------------------------------------------**//**
* @description    Creates a planar B-spline curve that simultaneously interpolates the given
* data points and tangent directions at those points, allowing for linear segments.
* Successive tangents at the same point are optionally removed before processing.
* The resulting curve is always planar, quadratic, and C1 continuous almost everywhere
* (it is only C0 at the junctions of linear segments, and only G1 at a closed curve's
* start/end point).  This routine allocates memory for the returned curve as necessary.
* @param    pCurve      OUT planar quadratic B-spline curve
* @param    pTangents   IN  tangents to interpolate
* @param    numPts      IN  number of tangents supplied
* @param    bClosed     IN  true to create a closed interpolant
* @param    bCompress   IN  true to remove successive tangents rooted at the same point
* @param    tolerance   IN  max relative error for duplicate points
* @return   SUCCESS indicates successful completion; ERROR indicates there is
* not enough memory for allocation; ERROR indicates too few points
* were passed in; ERROR indicates that successive tangents were parallel,
* rooted at the same point, or didn't intersect as expected.
* @alinkjoin   usmthmdlBspline_catmullRomCurve usmthmdlBspline_leastSquaresToCurve
*              usmthmdlBspline_cubicInterpolation usmthmdlBspline_cubicInterpolationExt
*              usmthmdlBspline_cubicInterpolationExt2
* @group        "B-spline Creation"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt    mdlBspline_interpolateCoplanarTangents
(
MSBsplineCurve*     pCurve,
const DRay3d*       pTangents,
int                 numTangents,
bool                bClosed,
bool                bCompress,
double              tolerance
)
    {
    // We construct a "fake" periodic curve (cf. mdlBspline_curveShouldBeOpened)
    // if bClosed == true.  Reasoning: there is no chord length parametrization
    // for a periodic interpolant that delivers C1 continuity at the start/end
    // point unless each pair of end/start Bezier polygon segments at the interp
    // points have the same length.  Thus, to nail the first interpolation point,
    // we have to saturate the first (or last) knot.  But ustn sees this knot
    // saturated *inside* the interior knot vector and assumes a "fake" periodic
    // curve.  To validate this assumption, we must also saturate the other knot
    // inside the interior knot vector.

    int         i, j, nPole, nKnot, nPrevPole, nPrevKnot, order = 3, degree = 2;
    int*        pIndices;
    double      param0, param1, d0, inc, scale, *pKnot;
    DPoint3d    pt, *pPole;
    DRay3d      tan0, tan1;
    bool        bLinear;
    StatusInt   status;

    if (!pCurve)
        return ERROR;
    if (!pTangents || numTangents < 2)
        return ERROR;

    pIndices = (int*)msbspline_malloc ((numTangents + 1) * sizeof (*pIndices), HEAPSIG_BCRV);
    if (NULL == pIndices)
        return ERROR;
    for (i = 0; i < numTangents; i++)
        pIndices[i] = i;

    // don't reference tangents rooted at the same point
    if (bCompress)
        {
        DPoint3d* pOrigins = (DPoint3d*)msbspline_malloc (numTangents * sizeof (*pOrigins), HEAPSIG_BCRV);
        if (NULL == pOrigins)
            {
            msbspline_free (pIndices);
            return ERROR;
            }
        for (i = 0; i < numTangents; i++)
            pOrigins[i] = pTangents[i].origin;

        bsiPolygon_compressDuplicateVertices (NULL, pIndices, NULL, &numTangents, pOrigins, pIndices, numTangents, 0.0, tolerance);

        msbspline_free (pOrigins);
        }

    // ensure last ray is duplicated in periodic case
    tan0 = pTangents[pIndices[0]];
    tan1 = pTangents[pIndices[numTangents - 1]];
    if (bClosed)
        {
        if (!tan0.origin.IsEqual (tan1.origin, tolerance))
            pIndices[numTangents++] = pIndices[0];      // add end tangent
        else if (!tan0.direction.IsEqual (tan1.direction, tolerance))
            pIndices[numTangents - 1] = pIndices[0];    // prefer tan0 over tan1

        tan1 = pTangents[pIndices[numTangents - 1]];    // get the new endTangent
        }

    if ((bClosed && numTangents < 4) || (!bClosed && numTangents < 2))
        {
        msbspline_free (pIndices);
        return ERROR;
        }

    // allocate more than enough for worst case (all linear segments)
    memset (pCurve, 0, sizeof (*pCurve));
    pCurve->params.order = order;
    pCurve->params.closed = bClosed;
    pCurve->params.numPoles = 2 * numTangents;
    pCurve->display.curveDisplay = true;
    if (SUCCESS != (status = bspcurv_allocateCurve (pCurve)))
        {
        msbspline_free (pIndices);
        return status;
        }

    pPole = pCurve->poles;
    pKnot = &pCurve->knots[degree];     // point to "0" knot

    // set start knot (always saturated)
    *pKnot++ = 0.0;
    if (bClosed)
        {
        double previousKnot = pKnot[-1];   // for "fake" periodic curve
        *pKnot++ = previousKnot;
        }
    nPrevKnot = degree;

    // add start pole
    *pPole++ = tan0.origin;
    nPrevPole = 1;

    // compute interior pole from successive tangent intersections
    // compute interior knot (+ end knot) from previous 2 knots and previous 3 Bezier poles
    for (i = 0; i < numTangents - 1; i++)
        {
        tan0 = pTangents[pIndices[i]];
        tan1 = pTangents[pIndices[i + 1]];
        if (intersectTangents (&pt, &param0, &param1, &bLinear, &tan0, &tan1, tolerance))
            {
            // make a linear Bezier segment if intersection at ray origin
            if (bLinear)
                {
                // add start pole if none there
                if (0 == nPrevPole)
                    *pPole++ = tan0.origin;

                // saturate start knot
                for (j = 0; j < degree - nPrevKnot; j++)
                    {
                    double previousKnot = pKnot[-1];
                    *pKnot++ = previousKnot;
                    }

                pt.Interpolate (tan0.origin, 0.5, tan1.origin);
                *pPole++ = pt;              // midpoint
                double previousKnot = pKnot[-1];
                *pKnot++ = previousKnot + 1;   // arbitrary end knot

                // add end pole and saturate end knot
                if (i < numTangents - 2)
                    {
                    *pPole++ = tan1.origin;
                    double previousKnot = pKnot[-1];
                    *pKnot++ = previousKnot;
                    nPrevPole = 1;
                    nPrevKnot = degree;
                    }
                }
            else
                {
                // set knot at ith tangent
                if (i == 0)
                    *pKnot++ = 1.0;                 // arbitrary
                else
                    {
                    d0 = tan0.origin.Distance (pPole[-1]);
                    inc = d0 ? (pKnot[-1] - pKnot[-2]) * (pt.Distance (tan0.origin) / d0) : 1.0;
                    double previousKnot = pKnot[-1];
                    *pKnot++ = previousKnot + inc;
                    }

                *pPole++ = pt;
                nPrevPole = 0;
                nPrevKnot = 1;
                }
            }
        else
            {
            // tangents were parallel or didn't intersect as expected
            msbspline_free (pIndices);
            bspcurv_freeCurve (pCurve);
            return ERROR;
            }
        }

    // saturate end knot inside interior knot vector for "fake" periodic curve
    if (bClosed)
        {
        double previousKnot = pKnot[-1];
        *pKnot++ = previousKnot;
        }

    // add end pole
    *pPole++ = tan1.origin;

    // get exact sizes
#ifdef TODO_EliminateAddressArithmetic
#endif
    nPole = (int)(pPole - pCurve->poles);
    pCurve->params.numPoles = nPole;
    nKnot = bspknot_numberKnots (nPole, order, bClosed);
    pCurve->params.numKnots = nKnot - 2 * order;

    // normalize interior + end knots
    pKnot = pCurve->knots;
    scale = 1.0 / pKnot[nKnot - order];
    for (i = order; i < nKnot - order; i++)
        pKnot[i] *= scale;
    pKnot[nKnot - order] = 1.0;

    // fill in exterior knots
    for (i = 0; i < degree; i++)
        {
        pKnot[i] = bClosed ? pKnot[nKnot - order - degree + i] - 1.0 : 0.0;
        pKnot[nKnot - degree + i] = bClosed ? 1.0 + pKnot[order + i] : 1.0;
        }

    msbspline_free (pIndices);
    return SUCCESS;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
