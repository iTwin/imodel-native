/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/bspline/bspproc.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/MstnOnly/BspPrivateApi.h>
#include "msbsplinemaster.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#define     CALCULATE_NONE  0
#define     CALCULATE_HIGH  1
#define     CALCULATE_BOTH  2
#define     REV_LIMIT               10

typedef struct surfWireframeData
    {
    int     (*processFunc)();
    void    *args;
    } SurfWireframeData;


/*---------------------------------------------------------------------------------**//**
* rational deCasteljau algorithm
* @bsimethod                                                    BSI             06/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bspproc_refineLinearApprox
(
DPoint3d        *outPoles,             /* 0.0 to 0.5 approx */
double          *outWts,
DPoint3d        *inPoles,              /* <=> weighted poles of approx */
double          *inWts,                /*  on return is 0.5 to 1.0 approx */
int             order,                 /* => number of points <= MAX_ORDER */
int             rational,
int             inIncr,                /* => increment btw in poles/wts */
int             outIncr                /* => increment btw out poles/wts */
)
    {
    int         i, j, end;
    double      *iwP, *owP, *iwP1;
    DPoint3d    *ipP, *opP, *ipP1;

    for (i=0, end=order-1, opP=outPoles; i<order; i++, end--, opP += outIncr)
        {
        *opP = *inPoles;
        for (j=0, ipP=inPoles, ipP1=ipP+inIncr; j < end; j++, ipP += inIncr, ipP1 += inIncr)
            {
            ipP->x = (ipP->x + ipP1->x) / 2.0;
            ipP->y = (ipP->y + ipP1->y) / 2.0;
            ipP->z = (ipP->z + ipP1->z) / 2.0;
            }
        }

    if (rational)
        {
        for (i=0, end=order-1, owP=outWts; i<order; i++, end--, owP += outIncr)
            {
            *owP = *inWts;
            for (j=0, iwP=inWts, iwP1=iwP+inIncr; j < end;
                 j++, iwP += inIncr, iwP1 += inIncr)
                {
                *iwP = (*iwP + *iwP1) / 2.0;
                }
            }
        }
    }

#if !defined (FONTIMPORTER)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             12/90
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bspproc_refineBezier
(
BezierInfo      *infoA,
BezierInfo      *infoB,
BezierInfo      *infoP,
MSBsplineCurve  *bezierA,
MSBsplineCurve  *bezierB
)
    {
    memcpy (infoA, infoP, sizeof(BezierInfo));
    memcpy (infoB, infoP, sizeof(BezierInfo));

    bezierA->rational = bezierB->rational;
    bezierA->params   = bezierB->params;

    bezierA->knots = bezierB->knots;

    bspproc_refineLinearApprox (bezierA->poles, bezierA->weights,
                                bezierB->poles, bezierB->weights,
                                bezierB->params.order, bezierB->rational, 1, 1);
#if defined (debug)
    {
    ElementUnion    u;
    DPoint3d        local[MAX_ORDER];

    if (bezierA->rational)
        bsputil_unWeightPoles (local, bezierA->poles, bezierA->weights,
                               bezierA->params.order);
    else
        memcpy (local, bezierA->poles, bezierA->params.order * sizeof(DPoint3d));
    create_lineStringD (&u, NULL, local, NULL, bezierA->params.order, LINE_STRING_ELM, 0);
    displayElement (&u, 0, -1L);

    if (bezierB->rational)
        bsputil_unWeightPoles (local, bezierB->poles, bezierB->weights,
                               bezierB->params.order);
    else
        memcpy (local, bezierB->poles, bezierB->params.order * sizeof(DPoint3d));
    create_lineStringD (&u, NULL, local, NULL, bezierB->params.order, LINE_STRING_ELM, 0);
    displayElement (&u, 0, -1L);
    }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* 1.0 + -------------- + ---------------- + | | | | outPolesA | inPoles | | | | v + -------------- + ---------------- + | | | | outPolesB |
* outPolesC | | | | 0.0 + -------------- + ---------------- + 0.0 u 1.0
* @bsimethod                                                    BSI             11/90
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bspproc_refineNetApprox
(
DPoint3d        *outPolesA,
double          *outWtsA,
DPoint3d        *outPolesB,
double          *outWtsB,
DPoint3d        *outPolesC,
double          *outWtsC,
DPoint3d        *inPoles,              /* <=> weighted poles of approx */
double          *inWts,
int             uOrder,                /* => U number of points <= MAX_ORDER */
int             vOrder,                /* => V number of points <= MAX_ORDER */
int             rational
)
    {
    int         i;
    double      *iwP, *awP, *bwP, *cwP;
    DPoint3d    *ipP, *apP, *bpP, *cpP;

    for (i=0, ipP=inPoles, iwP=inWts, apP=outPolesA, awP=outWtsA;
         i < vOrder;
         i++, ipP+=uOrder, iwP+=uOrder, apP+=uOrder, awP+=uOrder)
        {
        bspproc_refineLinearApprox (apP, awP, ipP, iwP, uOrder, rational, 1, 1);
        }

    for (i=0,
         ipP=inPoles, iwP=inWts,
         apP=outPolesA, awP=outWtsA,
         bpP=outPolesB, bwP=outWtsB,
         cpP=outPolesC, cwP=outWtsC;
         i < uOrder;
         i++,
         ipP++, iwP++,
         apP++, awP++,
         bpP++, bwP++,
         cpP++, cwP++)
        {
        bspproc_refineLinearApprox (cpP, cwP, ipP, iwP, vOrder, rational, uOrder, uOrder);
        bspproc_refineLinearApprox (bpP, bwP, apP, awP, vOrder, rational, uOrder, uOrder);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             12/90
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bspproc_refineBezierPatch
(
BezierInfo          **infoPtr,
BezierInfo          *infoP,
MSBsplineSurface    **patchPtr,
int                 numberPtr
)
    {
    int         i;

    for (i=0; i < numberPtr; i++)
        memcpy (infoPtr[i], infoP, sizeof(BezierInfo));

   patchPtr[0]->rational     = patchPtr[1]->rational =
   patchPtr[2]->rational     = patchPtr[3]->rational;
   patchPtr[0]->uParams      = patchPtr[1]->uParams =
   patchPtr[2]->uParams      = patchPtr[3]->uParams;
   patchPtr[0]->vParams      = patchPtr[1]->vParams =
   patchPtr[2]->vParams      = patchPtr[3]->vParams;
   patchPtr[0]->uKnots       = patchPtr[1]->uKnots =
   patchPtr[2]->uKnots       = patchPtr[3]->uKnots;
   patchPtr[0]->vKnots       = patchPtr[1]->vKnots =
   patchPtr[2]->vKnots       = patchPtr[3]->vKnots;
   patchPtr[0]->holeOrigin   = patchPtr[1]->holeOrigin =
   patchPtr[2]->holeOrigin   = patchPtr[3]->holeOrigin;
   patchPtr[0]->numBounds    = patchPtr[1]->numBounds =
   patchPtr[2]->numBounds    = patchPtr[3]->numBounds;
   patchPtr[0]->boundaries   = patchPtr[1]->boundaries =
   patchPtr[2]->boundaries   = patchPtr[3]->boundaries;

   bspproc_refineNetApprox (patchPtr[2]->poles, patchPtr[2]->weights,
                            patchPtr[0]->poles, patchPtr[0]->weights,
                            patchPtr[1]->poles, patchPtr[1]->weights,
                            patchPtr[3]->poles, patchPtr[3]->weights,
                            patchPtr[3]->uParams.order,
                            patchPtr[3]->vParams.order,
                            patchPtr[3]->rational);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             12/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   bspproc_setParameter
(
double          in,                    /* => parameter in segment 0.0 to 1.0 */
int             start,                 /* => knot offset of segment */
double          *knots                 /* => full knot vector */
)
    {
    int         offset;
    double      uValue, *kP;

    for (kP=knots; (KNOT_TOLERANCE_BASIS >= *kP && *kP < 1.0); kP++)
        ;
    /* kP pointing to first knot > 0.0 */

#ifdef TODO_EliminateAddressArithmetic
#endif
    offset = (int)(kP - knots);
    offset += start;

    uValue = (1.0 - in) * knots[offset-1] + in * knots[offset];
    return (uValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     09/97
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double   bspproc_setParameterWithOrder
(
double          in,                    /* => parameter in segment 0.0 to 1.0 */
int             start,                 /* => knot offset of segment */
double          *knots,                /* => full knot vector */
int             order
)
    {
    int         offset = start + order;

    return  (1.0 - in) * knots[offset-1] + in * knots[offset];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             12/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bspproc_setPatchParameter
(
DPoint2d        *out,
int             code,
DPoint2d        *in
)
    {
    switch (code)
        {
        case 0:                        /* 0 <= u <= 1/2 : 0 <= v <= 1/2 */
            out->x = 0.5 * in[0].x;
            out->y = 0.5 * in[0].y;
            break;
        case 1:                        /* 1/2 <= u <= 1 : 0 <= v <= 1/2 */
            out->x = 0.5 + 0.5 * in[1].x;
            out->y = 0.5 * in[1].y;
            break;
        case 2:                        /* 0 <= u <= 1/2 : 1/2 <= v <= 1 */
            out->x = 0.5 * in[2].x;
            out->y = 0.5 + 0.5 * in[2].y;
            break;
        case 3:                        /* 1/2 <= u <= 1 : 1/2 <= v <= 1 */
            out->x = 0.5 + 0.5 * in[3].x;
            out->y = 0.5 + 0.5 * in[3].y;
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    earlinLutz      11/94
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bspproc_freePreparedCurve
(
MSBsplineCurve  *bezier,               /*  => Bezier curve */
int             **starts               /* <=> offsets of segments */
)
    {
    msbspline_free(*starts);
    bspcurv_freeCurve(bezier);
    *starts = 0;
    }

/*---------------------------------------------------------------------------------**//**
* Free the surface and arrays previously allocated by bspproc_prepareSurface
* @bsimethod                                                    earlinLutz      11/94
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bspproc_freePreparedSurface
(
MSBsplineSurface    *bezierP,          /* <=> bezier surface */
int                 **uStartsPP,       /* <=> offsets of segments U */
int                 **vStartsPP        /* <=> offsets of segments V */
)
    {
    if(bezierP)bspsurf_freeSurface(bezierP);
    if(*uStartsPP)msbspline_free(*uStartsPP);
    if(*vStartsPP)msbspline_free(*vStartsPP);
    *uStartsPP = *vStartsPP = 0;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             12/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int     bspproc_prepareSurface
(
MSBsplineSurface    *bezier,           /* <= bezier surface */
int                 *uNumSegs,         /* <= number of segs U */
int                 *vNumSegs,         /* <= number of segs V */
int                 **uStarts,         /* <= offsets of segments U */
int                 **vStarts,         /* <= offsets of segments V */
MSBsplineSurface    *bspline           /* => Bspline surface */
)
    {
    int         i, *iP, status, numDist;
    double      *distKnot, knotTol=KNOT_TOLERANCE_BASIS;

    memset (bezier, 0, sizeof(*bezier));

    if (SUCCESS != (status = bsprsurf_openSurface (bezier, bspline, 0.0, 0)) ||
        SUCCESS != (status = bsprsurf_openSurface (bezier, bezier, 0.0, 1)) ||
        SUCCESS != (status = bsprsurf_makeBezierSurface (bezier, bezier)))
        return status;

    numDist = bspknot_numberKnots (bezier->uParams.numPoles, bezier->uParams.order,
                                   bezier->uParams.closed);
    distKnot = NULL;    *uStarts = NULL;
    if (NULL == (*uStarts = (int*)msbspline_malloc (numDist * sizeof(int),    HEAPSIG_BPRC)) ||
        NULL == (distKnot = (double*)msbspline_malloc (numDist * sizeof(double), HEAPSIG_BPRC)))
        {
        status = ERROR;
        goto wrapup;
        }

    bspknot_getKnotMultiplicity (distKnot, *uStarts, uNumSegs, bezier->uKnots,
                                 bezier->uParams.numPoles, bezier->uParams.order,
                                 bezier->uParams.closed, knotTol);
    *uNumSegs -= 1;

    numDist = bspknot_numberKnots (bezier->vParams.numPoles, bezier->vParams.order,
                                   bezier->vParams.closed);
    *vStarts = NULL;
    if (NULL == (*vStarts = (int*)msbspline_malloc  (numDist * sizeof(int), HEAPSIG_BPRC)) ||
        NULL == (distKnot = (double*)msbspline_realloc (distKnot, numDist*sizeof(double))))
        {
        status = ERROR;
        goto wrapup;
        }
    bspknot_getKnotMultiplicity (distKnot, *vStarts, vNumSegs, bezier->vKnots,
                                 bezier->vParams.numPoles, bezier->vParams.order,
                                 bezier->vParams.closed, knotTol);
    *vNumSegs -= 1;

    /* Find offset of start of segs from knot multiplicity array */
    (*uStarts)[0] = 0;
    for (i=1, iP=(*uStarts)+1; i < *uNumSegs; i++, iP++)
        *(iP) += *(iP-1);
    (*vStarts)[0] = 0;
    for (i=1, iP=(*vStarts)+1; i < *vNumSegs; i++, iP++)
        *(iP) += *(iP-1);

wrapup:
    if (distKnot)       msbspline_free (distKnot);

    return (SUCCESS);
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   Bezier Processing Routines                                          |
|                                                                       |
+----------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------**//**
* stopFunc : Return true if converged after setting output information else return false goFunc : Return true if this Bezier is still worth
* considering else return false sortFunc : Return 1 or 0 depending on which bezier should be processed first selectFunc : Set output
* information based on information from the 2 halves
* @bsimethod                                                    BSI             06/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_processBezier
(
BezierInfo      *infoP,                /* <=> information desired of curve */
MSBsplineCurve  *bezier,               /* => Bezier curve to process */
PFBezCurve_Stop stopFunc,     /* => ends recursion & loads output */
PFBezCurve_Sort sortFunc,     /* => sorting function, or NULL */
PFBezCurve_Go   goFunc,       /* => optional, to check half */
PFBezCurve_Select selectFunc  /* => loads ouput based on halves */
)
    {
    int             status;
    double          weightsA[MAX_ORDER], weightsB[MAX_ORDER];
    DPoint3d        polesA[MAX_ORDER], polesB[MAX_ORDER];
    BezierInfo      infoA, infoB;
    MSBsplineCurve  bezierA, bezierB;

    if (stopFunc && (*stopFunc) (infoP, bezier))        return (SUCCESS);

    /* If not stop, subdivide this Bezier curve and recurse */
    bezierB = *bezier;
    bezierA.poles   = polesA;       bezierB.poles   = polesB;
    bezierA.weights = weightsA;     bezierB.weights = weightsB;

    memcpy (bezierB.poles, bezier->poles, bezierB.params.order * sizeof(DPoint3d));
    if (bezierB.rational)
        memcpy (bezierB.weights, bezier->weights, bezierB.params.order * sizeof(double));

    /* OK to use same knot vector as the subdivision routines do not change knots */
    bezierB.knots = bezier->knots;

    bspproc_refineBezier (&infoA, &infoB, infoP, &bezierA, &bezierB);

    if (sortFunc && (*sortFunc) (&infoA, &infoB, &bezierA, &bezierB))
        {
        /* Process second half first */
        if (! goFunc || (*goFunc) (&infoB, &bezierB, 1))
            {
            if (SUCCESS != (status = bspproc_processBezier (&infoB, &bezierB, stopFunc,
                                                            sortFunc, goFunc, selectFunc)))
                return status;
            }

        if (! goFunc || (*goFunc) (&infoA, &bezierA, 0))
            {
            if (SUCCESS != (status = bspproc_processBezier (&infoA, &bezierA, stopFunc,
                                                            sortFunc, goFunc, selectFunc)))
                return status;
            }
        }
    else
        {
        /* Process first half first */
        if (! goFunc || (*goFunc) (&infoA, &bezierA, 0))
            {
            if (SUCCESS != (status = bspproc_processBezier (&infoA, &bezierA, stopFunc,
                                                            sortFunc, goFunc, selectFunc)))
                return status;
            }

        if (! goFunc || (*goFunc) (&infoB, &bezierB, 1))
            {
            if (SUCCESS != (status = bspproc_processBezier (&infoB, &bezierB, stopFunc,
                                                            sortFunc, goFunc, selectFunc)))
                return status;
            }
        }

    if (selectFunc)
        return ((*selectFunc)(infoP, &infoA, &infoB, &bezierA, &bezierB));
    else
        return (SUCCESS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             12/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_doubleProcessBezier
(
BezierInfo      *infoP,                /* <=> information desired of curve */
MSBsplineCurve  *bezier0,              /* => Bezier curve to process */
MSBsplineCurve  *bezier1,              /* => Bezier curve to process */
PFBezCurveBezCurve_Stop   stopFunc,      /* => ends recursion & loads output */
PFBezCurveBezCurve_Sort   sortFunc,      /* => sorting function, or NULL */
PFBezCurveBezCurve_Go     goFunc,        /* => optional, to check half */
PFBezCurveBezCurve_Select selectFunc     /* => loads ouput based on halves */
)
    {
    int             i, status, tags[4], *tg, indx0, indx1;
    double          weights0A[MAX_ORDER], weights0B[MAX_ORDER],
                    weights1A[MAX_ORDER], weights1B[MAX_ORDER];
    DPoint3d        poles0A[MAX_ORDER], poles0B[MAX_ORDER],
                    poles1A[MAX_ORDER], poles1B[MAX_ORDER];
    BezierInfo      *infoPtr[4], infoAA, infoAB, infoBA, infoBB;
    MSBsplineCurve  *bezPtr[4], bezier0A, bezier0B, bezier1A, bezier1B;

    if ((*stopFunc) (infoP, bezier0, bezier1))
        return (SUCCESS);

    /* If not stop, subdivide both Bezier curves and recurse */
    bezier0B = *bezier0;    bezier1B = *bezier1;

    bezier0A.poles = poles0A; bezier0A.weights = weights0A;
    bezier0B.poles = poles0B; bezier0B.weights = weights0B;
    bezier1A.poles = poles1A; bezier1A.weights = weights1A;
    bezier1B.poles = poles1B; bezier1B.weights = weights1B;

    /* OK to use same knot vector as the subdivision routines do not change knots */
    bezier0B.knots = bezier0->knots;
    bezier1B.knots = bezier1->knots;

    memcpy (bezier0B.poles, bezier0->poles, bezier0B.params.order * sizeof(DPoint3d));
    if (bezier0B.rational)
        memcpy (bezier0B.weights, bezier0->weights, bezier0B.params.order * sizeof(double));

    memcpy (bezier1B.poles, bezier1->poles, bezier1B.params.order * sizeof(DPoint3d));
    if (bezier1B.rational)
        memcpy (bezier1B.weights, bezier1->weights, bezier1B.params.order * sizeof(double));

    bspproc_refineBezier (&infoAA, &infoAB, infoP, &bezier0A, &bezier0B);
    bspproc_refineBezier (&infoBA, &infoBB, infoP, &bezier1A, &bezier1B);

    /* This ordering is crucial so that the selectFunc can call
        bspproc_setPatchParameter to set the parameters of the 2 Bezier's */

    bezPtr[0] = &bezier0A;
    bezPtr[1] = &bezier0B;
    bezPtr[2] = &bezier1A;
    bezPtr[3] = &bezier1B;

    infoPtr[0] = &infoAA;    /* info about first halves of 0 & 1 */
    infoPtr[1] = &infoBA;    /* info about second of 0, first of 1 */
    infoPtr[2] = &infoAB;    /* info about first of 0, second of 1 */
    infoPtr[3] = &infoBB;    /* info about second halves of 0 & 1 */

    if (sortFunc)
        {
        (*sortFunc) (tags, infoPtr, bezPtr);
        }
    else
        {
        tags[0]=0;      tags[1]=1;      tags[2]=2;      tags[3]=3;
        }

    for (i=0, tg=tags; i < 4; i++, tg++)
        {
        indx0 = *tg % 2;        indx1 = *tg / 2 + 2;

        if (! goFunc || (*goFunc) (infoPtr[*tg], bezPtr[indx0], bezPtr[indx1], indx0, indx1))
            {
            if (SUCCESS != (status = bspproc_doubleProcessBezier (infoPtr[*tg],
                                             bezPtr[indx0], bezPtr[indx1],
                                             stopFunc, sortFunc, goFunc, selectFunc)))
                return status;
            }
        }

    if (selectFunc)
        return ((*selectFunc) (infoP, infoPtr, bezPtr));
    else
        return (SUCCESS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             12/90
+---------------+---------------+---------------+---------------+---------------+------*/
static     int      bspproc_processPreallocatedBezierPatch
(
BezierInfo          *infoP,            /* <=> information desired of patch */
DPoint3d            *poles,            /* point buffer. */
double              *weights,          /* weight buffer */
int                 poleCount,         /* => number of poles allocated */
MSBsplineSurface    *patch,            /* => Bezier patch to process */
PFBezPatch_Stop  stopFunc,  /* => ends recursion & loads output */
PFBezPatch_Sort  sortFunc,  /* => sorting function, or NULL */
PFBezPatch_Go    goFunc,    /* => to check half, or NULL */
PFBezPatch_Select selectFunc /* => loads ouput, or NULL */
)
    {
    int                 i, status, tags[4], *tg, polesPerQuadrant;
    double              *wP;
    DPoint3d            *pP;
    BezierInfo          infoX[4], *infoPtr[4];
    MSBsplineSurface    patchX[4], *patchPtr[4];

    polesPerQuadrant = (patch->uParams.order * patch->vParams.order);

    if (poleCount < 4 * polesPerQuadrant)
        return ERROR;


    /* Assign pointers to pole arrays */
    for (i=0, pP=poles; i < 4; i++, pP += polesPerQuadrant)
        patchX[i].poles = pP;
    if (patch->rational)
        for (i=0, wP=weights; i < 4; i++, wP += polesPerQuadrant)
            patchX[i].weights = wP;

    /* Ordering here is crucial so that selectFunc can call bspproc_setPatchParameter */
    patchPtr[0] = &patchX[0];   /* 0 <= u <= 1/2 : 0 <= v <= 1/2 */
    patchPtr[1] = &patchX[1];   /* 1/2 <= u <= 1 : 0 <= v <= 1/2 */
    patchPtr[2] = &patchX[2];   /* 0 <= u <= 1/2 : 1/2 <= v <= 1 */
    patchPtr[3] = &patchX[3];   /* 1/2 <= u <= 1 : 1/2 <= v <= 1 */
    infoPtr[0] = &infoX[0];
    infoPtr[1] = &infoX[1];
    infoPtr[2] = &infoX[2];
    infoPtr[3] = &infoX[3];

    patchX[3].rational = patch->rational;
    patchX[3].uParams  = patch->uParams;
    patchX[3].vParams  = patch->vParams;

    /* OK to use same knot vector as the subdivision routines do not change knots */
    patchX[3].uKnots   = patch->uKnots;
    patchX[3].vKnots   = patch->vKnots;

    /* copy boundary information */
    patchX[3].numBounds  = patch->numBounds;
    patchX[3].boundaries = patch->boundaries;
    patchX[3].holeOrigin = patch->holeOrigin;

    memcpy (patchX[3].poles, patch->poles, polesPerQuadrant * sizeof(DPoint3d));
    if (patch->rational)
        memcpy (patchX[3].weights, patch->weights, polesPerQuadrant * sizeof(double));

    bspproc_refineBezierPatch (infoPtr, infoP, patchPtr, 4);

    if (sortFunc)
        (*sortFunc) (tags, infoPtr, patchPtr);
    else
        {
        tags[0]=0;      tags[1]=1;      tags[2]=2;      tags[3]=3;
        }

    for (i=0, tg=tags; i < 4; i++, tg++)
        {
        if (! goFunc || (*goFunc) (infoPtr[*tg], patchPtr[*tg], *tg))
            {
            if (SUCCESS != (status = bspproc_processBezierPatch (infoPtr[*tg], patchPtr[*tg],
                                             stopFunc, sortFunc, goFunc, selectFunc)))
                return status;
            }
        }

    if (selectFunc)
        status = (*selectFunc) (infoP, infoPtr, patchPtr);
    else
        status = SUCCESS;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             12/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_processBezierPatchFromStack
(
StatusInt           *processStatusP,    /* Result of processing.  Set to ERROR if
                                                this function is unable to start processing */
BezierInfo          *infoP,            /* <=> information desired of patch */
MSBsplineSurface    *patch,            /* => Bezier patch to process */
PFBezPatch_Stop  stopFunc,  /* => ends recursion & loads output */
PFBezPatch_Sort  sortFunc,  /* => sorting function, or NULL */
PFBezPatch_Go    goFunc,    /* => to check half, or NULL */
PFBezPatch_Select selectFunc /* => loads ouput, or NULL */
)
    {
/* Allow local allocation for cubics */
#define LOCAL_ALLOC_LIMIT 64
    DPoint3d poles[LOCAL_ALLOC_LIMIT];
    double   weights[LOCAL_ALLOC_LIMIT];
    StatusInt   myStatus = ERROR;
    int allocSize = (patch->uParams.order * patch->vParams.order) * 4;

    if (allocSize < LOCAL_ALLOC_LIMIT)
        {
        myStatus = SUCCESS;
        *processStatusP = bspproc_processPreallocatedBezierPatch (
                        infoP, poles, weights, LOCAL_ALLOC_LIMIT, patch,
                        stopFunc, sortFunc, goFunc, selectFunc);
        }
    else
        {
        *processStatusP = ERROR;
        }

    return myStatus;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             12/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_processBezierPatch
(
BezierInfo          *infoP,            /* <=> information desired of patch */
MSBsplineSurface    *patch,            /* => Bezier patch to process */
PFBezPatch_Stop  stopFunc,  /* => ends recursion & loads output */
PFBezPatch_Sort  sortFunc,  /* => sorting function, or NULL */
PFBezPatch_Go    goFunc,    /* => to check half, or NULL */
PFBezPatch_Select selectFunc /* => loads ouput, or NULL */
)
    {
    int         allocSize;
    StatusInt status = ERROR;
    static int allocateFromHeap = false;

    if ((*stopFunc) (infoP, patch))     return (SUCCESS);

    if (allocateFromHeap || SUCCESS != bspproc_processBezierPatchFromStack (
                        &status,
                        infoP,
                        patch,
                        stopFunc,
                        sortFunc,
                        goFunc,
                        selectFunc))
        {
        double      *weights = NULL;
        DPoint3d    *poles = NULL;
        int         failures = 0;

        allocSize = (patch->uParams.order * patch->vParams.order) * 4;

        poles = (DPoint3d*)msbspline_malloc (allocSize * sizeof(DPoint3d), HEAPSIG_BPRC);
        if (!poles)
            failures++;
        if (patch->rational)
            {
            weights = (double*)msbspline_malloc (allocSize * sizeof(double), HEAPSIG_BPRC);
            if (!weights)
                failures++;
            }

        if (failures > 0)
            {
            status = ERROR;
            }
        else
            status = bspproc_processPreallocatedBezierPatch (infoP, poles, weights, allocSize, patch, stopFunc, sortFunc, goFunc, selectFunc);

        if (poles)
            msbspline_free (poles);
        if (weights)
            msbspline_free (weights);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             03/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_doubleProcessPreallocatedBezierPatch
(
BezierInfo          *infoP,            /* <=> information desired of patch */
MSBsplineSurface    *patch0,           /* => Bezier patch to process */
DPoint3d            *poles0,           /* => pole buffer for surface 0 */
double              *weights0,         /* => weight buffer for surface 0 */
int                 count0,             /* => buffer size for surface 0 */
MSBsplineSurface    *patch1,           /* => Bezier patch to process */
DPoint3d            *poles1,           /* => pole buffer for surface 1 */
double              *weights1,         /* => weight buffer for surface 1 */
int                 count1,             /* => buffer size for surface 1 */
BezierInfo          *infoX,             /* => buffer of bezier info structs */
PFBezPatchBezPatch_Stop  stopFunc,  /* => ends recursion & loads output */
PFBezPatchBezPatch_Sort  sortFunc,  /* => sorting function, or NULL */
PFBezPatchBezPatch_Go    goFunc,    /* => to check half, or NULL */
PFBezPatchBezPatch_Select selectFunc /* => loads ouput, or NULL */
)
    {
    int                 i, j, status, tags[16], *tg, total0, total1,
                        allocSize0, allocSize1, sub0, sub1, indx;
    double              *wP;
    DPoint3d            *pP0, *pP1;
    BezierInfo          *infoPtr[16], *iP;
    MSBsplineSurface    patch0X[4], patch1X[4], *patch0Ptr[4], *patch1Ptr[4];


    allocSize0 = (total0 = patch0->uParams.order * patch0->vParams.order) * 4;
    allocSize1 = (total1 = patch1->uParams.order * patch1->vParams.order) * 4;

    if (allocSize0 > count0 || allocSize1 > count1)
        {
        return ERROR;
        }

    /* Assign pointers to arrays */
    for (i=0, pP0=poles0, pP1=poles1; i < 4; i++, pP0 += total0, pP1 += total1)
        {
        patch0X[i].poles = pP0;
        patch1X[i].poles = pP1;
        }
    if (patch0->rational)
        for (i=0, wP=weights0; i < 4; i++, wP += total0)
            patch0X[i].weights = wP;
    if (patch1->rational)
        for (i=0, wP=weights1; i < 4; i++, wP += total1)
            patch1X[i].weights = wP;

    for (i=0, iP=infoX; i < 16; i++, iP++)
        infoPtr[i] = iP;

    /* Ordering here is crucial so that selectFunc can call
        bspproc_setPatchParameter */
    patch0Ptr[0] = &patch0X[0]; /* 0 <= u <= 1/2 : 0 <= v <= 1/2 */
    patch0Ptr[1] = &patch0X[1]; /* 1/2 <= u <= 1 : 0 <= v <= 1/2 */
    patch0Ptr[2] = &patch0X[2]; /* 0 <= u <= 1/2 : 1/2 <= v <= 1 */
    patch0Ptr[3] = &patch0X[3]; /* 1/2 <= u <= 1 : 1/2 <= v <= 1 */

    patch1Ptr[0] = &patch1X[0]; /* 0 <= u <= 1/2 : 0 <= v <= 1/2 */
    patch1Ptr[1] = &patch1X[1]; /* 1/2 <= u <= 1 : 0 <= v <= 1/2 */
    patch1Ptr[2] = &patch1X[2]; /* 0 <= u <= 1/2 : 1/2 <= v <= 1 */
    patch1Ptr[3] = &patch1X[3]; /* 1/2 <= u <= 1 : 1/2 <= v <= 1 */

    patch0X[3].rational = patch0->rational;
    patch0X[3].uParams  = patch0->uParams;
    patch0X[3].vParams  = patch0->vParams;
    patch0X[3].uKnots   = patch0->uKnots;
    patch0X[3].vKnots   = patch0->vKnots;
    patch1X[3].rational = patch1->rational;
    patch1X[3].uParams  = patch1->uParams;
    patch1X[3].vParams  = patch1->vParams;

    /* OK to use same knot vector as the subdivision routines do not change knots */
    patch1X[3].uKnots   = patch1->uKnots;
    patch1X[3].vKnots   = patch1->vKnots;

    memcpy (patch0X[3].poles, patch0->poles, total0 * sizeof(DPoint3d));
    if (patch0->rational)
        memcpy (patch0X[3].weights, patch0->weights, total0 * sizeof(double));
    memcpy (patch1X[3].poles, patch1->poles, total1 * sizeof(DPoint3d));
    if (patch1->rational)
        memcpy (patch1X[3].weights, patch1->weights, total1 * sizeof(double));

    bspproc_refineBezierPatch (infoPtr, infoP, patch0Ptr, 8);
    bspproc_refineBezierPatch (infoPtr+8, infoP, patch1Ptr, 8);

    if (sortFunc)
        {
        if (SUCCESS != (status = (*sortFunc) (tags, infoPtr, patch0Ptr, patch1Ptr)))
            goto wrapup;
        tg = tags;
        }
    else
        tg = NULL;

    for (j=0; j < 4; j++)
        for (i=0; i < 4; i++)
            {
            if (tg)
                {
                sub0 = *tg % 4;
                sub1 = *tg / 4;
                tg++;
                }
            else
                {
                sub0 = i;    sub1 = j;
                }

            indx = sub1*4 + sub0;
            if (! goFunc || (*goFunc) (infoPtr[indx], patch0Ptr[sub0], patch1Ptr[sub1],
                                       sub0, sub1))
                {
                if (SUCCESS != (status = bspproc_doubleProcessBezierPatch (infoPtr[indx],
                                                 patch0Ptr[sub0], patch1Ptr[sub1],
                                                 stopFunc, sortFunc, goFunc, selectFunc)))
                    goto wrapup;
                }
            }

    if (selectFunc)
        status = (*selectFunc) (infoP, infoPtr, patch0Ptr, patch1Ptr);
    else
        status = SUCCESS;

wrapup:
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             03/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_doubleProcessBezierPatchFromHeap
(
BezierInfo          *infoP,            /* <=> information desired of patch */
MSBsplineSurface    *patch0,           /* => Bezier patch to process */
MSBsplineSurface    *patch1,           /* => Bezier patch to process */
PFBezPatchBezPatch_Stop  stopFunc,  /* => ends recursion & loads output */
PFBezPatchBezPatch_Sort  sortFunc,  /* => sorting function, or NULL */
PFBezPatchBezPatch_Go    goFunc,    /* => to check half, or NULL */
PFBezPatchBezPatch_Select selectFunc /* => loads ouput, or NULL */
)
    {
    int                 status, total0, total1,
                        allocSize0, allocSize1;
    double              *weights0, *weights1;
    DPoint3d            *poles0, *poles1;
    BezierInfo          *infoX;

    /* If not stop, subdivide Bezier patches and recurse */
    allocSize0 = (total0 = patch0->uParams.order * patch0->vParams.order) * 4;
    allocSize1 = (total1 = patch1->uParams.order * patch1->vParams.order) * 4;
    weights0 = weights1 = NULL;    poles0 = poles1 = NULL;    infoX = NULL;

    /* Do big malloc's to save time */
    if (NULL == (poles0 = (DPoint3d*)msbspline_malloc (allocSize0 * sizeof(DPoint3d), HEAPSIG_BPRC)) ||
        NULL == (poles1 = (DPoint3d*)msbspline_malloc (allocSize1 * sizeof(DPoint3d), HEAPSIG_BPRC)))
        {
        status = ERROR;
        goto wrapup;
        }
    if ((patch0->rational &&
        NULL == (weights0 = (double*)msbspline_malloc (allocSize0 * sizeof(double), HEAPSIG_BPRC))) ||
        (patch1->rational &&
        NULL == (weights1 = (double*)msbspline_malloc (allocSize1 * sizeof(double), HEAPSIG_BPRC))))
        {
        status = ERROR;
        goto wrapup;
        }
    if (NULL == (infoX = (BezierInfo*)msbspline_malloc (16 * sizeof(BezierInfo), HEAPSIG_BPRC)))
        {
        status = ERROR;
        goto wrapup;
        }

    status = bspproc_doubleProcessPreallocatedBezierPatch (
                    infoP,
                    patch0, poles0, weights0, allocSize0,
                    patch1, poles1, weights1, allocSize1,
                    infoX,
                    stopFunc, sortFunc, goFunc, selectFunc );
wrapup:
    if (poles0)         msbspline_free (poles0);
    if (weights0)       msbspline_free (weights0);
    if (poles1)         msbspline_free (poles1);
    if (weights1)       msbspline_free (weights1);
    if (infoX)          msbspline_free (infoX);
    return status;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             03/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt    bspproc_doubleProcessBezierPatchFromStack   /* ERROR if stack not big enough */
(
StatusInt           *procStatusP,      /* <= status of the real processing. */
BezierInfo          *infoP,            /* <=> information desired of patch */
MSBsplineSurface    *patch0,           /* => Bezier patch to process */
MSBsplineSurface    *patch1,           /* => Bezier patch to process */
PFBezPatchBezPatch_Stop  stopFunc,  /* => ends recursion & loads output */
PFBezPatchBezPatch_Sort  sortFunc,  /* => sorting function, or NULL */
PFBezPatchBezPatch_Go    goFunc,    /* => to check half, or NULL */
PFBezPatchBezPatch_Select selectFunc /* => loads ouput, or NULL */
)
    {
#define DOUBLE_RECURSION_STACK_BUFFER_COUNT 128
    DPoint3d poles[DOUBLE_RECURSION_STACK_BUFFER_COUNT];
    double weights[DOUBLE_RECURSION_STACK_BUFFER_COUNT];
    DPoint3d *poles0,   *poles1;
    double   *weights0, *weights1;
    int         allocSize0, allocSize1;
    BezierInfo  infoArray[16];
    StatusInt   myStatus = ERROR;
    *procStatusP = ERROR;

    allocSize0 = (patch0->uParams.order * patch0->vParams.order) * 4;
    allocSize1 = (patch1->uParams.order * patch1->vParams.order) * 4;

    if (allocSize0 + allocSize1 <= DOUBLE_RECURSION_STACK_BUFFER_COUNT)
        {
        poles0   = poles;
        poles1   = poles0 + allocSize0;
        weights0 = weights;
        weights1 = weights0 + allocSize0;

        *procStatusP = bspproc_doubleProcessPreallocatedBezierPatch
                            (
                            infoP,
                            patch0, poles0, weights0, allocSize0,
                            patch1, poles1, weights1, allocSize1,
                            infoArray,
                            stopFunc, sortFunc, goFunc, selectFunc
                            );
        myStatus = SUCCESS;
        }
    return myStatus;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             03/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_doubleProcessBezierPatch
(
BezierInfo          *infoP,            /* <=> information desired of patch */
MSBsplineSurface    *patch0,           /* => Bezier patch to process */
MSBsplineSurface    *patch1,           /* => Bezier patch to process */
PFBezPatchBezPatch_Stop  stopFunc,  /* => ends recursion & loads output */
PFBezPatchBezPatch_Sort  sortFunc,  /* => sorting function, or NULL */
PFBezPatchBezPatch_Go    goFunc,    /* => to check half, or NULL */
PFBezPatchBezPatch_Select selectFunc /* => loads ouput, or NULL */
)
    {
    StatusInt status = ERROR;
    static int alwaysUseHeap = false;

    if ((*stopFunc) (infoP, patch0, patch1))    return (SUCCESS);

    if (alwaysUseHeap || SUCCESS != bspproc_doubleProcessBezierPatchFromStack (&status,
                        infoP, patch0, patch1, stopFunc, sortFunc, goFunc, selectFunc))
        status = bspproc_doubleProcessBezierPatchFromHeap (
                        infoP, patch0, patch1, stopFunc, sortFunc, goFunc, selectFunc);
    return status;
    }

/*----------------------------------------------------------------------+
|                                                                       |
|   B-spline Processing Functions                                       |
|                                                                       |
+----------------------------------------------------------------------*/
/*---------------------------------------------------------------------------------**//**
* Process a B-spline curve by dividing it into Bezier segments and recursively processing the segments. The following functions control the
* recursion. sortFunc : Order the Bezier segments for further processing goFunc : Return true if this Bezier is still worth considering else
* return false selectFunc : Assigns the output for _processBspline by selecting from the results on the segments cleanFunc : Called before
* _processBspline returns
* @bsimethod                                                    BSI             06/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_processBspline
(
BezierInfo      *infoP,        /* <=> information desired of curve */
MSBsplineCurve  *bspline,      /* => Bspline curve to process */
FPBsplineSort    sortFunc,     /* => sorting function, or NULL */
FPBsplineGo      goFunc,       /* => go function, or NULL */
FPBsplineProcess processFunc,  /* => acts on Bezier segments */
FPBsplineSelect  selectFunc,   /* => loads ouput based on segments */
FPBsplineClean   cleanFunc     /* => called before returning */
)
    {
    int             i, status, numSegments, *tg, *tags=NULL, *start=NULL;
    double          weights[MAX_ORDER];
    DPoint3d        poles[MAX_ORDER];
    BezierInfo      segInfo;
    MSBsplineCurve  curve, bezier;

    if (SUCCESS != (status = bspproc_prepareCurve (&curve, &numSegments, &start, bspline)))
        goto wrapup;

    if (NULL == (tags = (int*)msbspline_malloc (numSegments * sizeof(int), HEAPSIG_BPRC)))
        {
        status = ERROR;
        goto wrapup;
        }

    if (sortFunc)
        {
        if (SUCCESS != (status = (*sortFunc) (tags, infoP, start, numSegments, &curve)))
            goto wrapup;
        }
    else
        {
        for (i=0, tg=tags; i < numSegments; i++, tg++)
            *tg = i;
        }

    /* Make a local copy of poles and weights so that processFunc
        does not have to conserve them. */
    bezier.rational        = curve.rational;
    bezier.params.numPoles =
    bezier.params.order    = curve.params.order;
    bezier.params.numKnots =
    bezier.params.closed   = 0;
    bezier.knots           = bspknot_bezierKnotVector (bezier.params.order);
    bezier.poles           = poles;
    bezier.weights         = weights;

    for (i=0, tg=tags; i < numSegments; i++, tg++)
        {
        memcpy (bezier.poles, curve.poles + start[*tg],
                bezier.params.order * sizeof(DPoint3d));
        if (bezier.rational)
            memcpy (bezier.weights, curve.weights + start[*tg],
                    bezier.params.order * sizeof(double));
        memcpy (&segInfo, infoP, sizeof(segInfo));

        if (! goFunc || (*goFunc) (&segInfo, &bezier, *tg, start[*tg], numSegments))
            {
            if (processFunc &&
                SUCCESS != (status = (*processFunc) (&segInfo, &bezier)))
                goto wrapup;

            if (selectFunc &&
                SUCCESS != (status = (*selectFunc) (infoP, &segInfo, &bezier,
                                                   *tg, start[*tg], numSegments)))
                goto wrapup;
            }
        }

wrapup:
    if (cleanFunc)
        (*cleanFunc) (infoP, numSegments);
    if (tags)       msbspline_free (tags);
    if (start)      msbspline_free (start);
    bspcurv_freeCurve (&curve);
    return status;
    }
/*---------------------------------------------------------------------------------**//**
* Initialize a bezier curve struct to hold a bezier of same order as a given curve.
* @bsimethod                                                    earlinLutz      10/94
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_initializeBezierSegment
(
MSBsplineCurve  *segP,           /* <= bezier segment of curve */
MSBsplineCurve  *curveP          /* => curve (bezier prepped) */
)
    {
    int                 numSegmentPoles;

    /* bezier is the patch under consideration */
    numSegmentPoles = curveP->params.order;
    if (NULL ==
        (segP->poles = (DPoint3d*)msbspline_malloc (numSegmentPoles * sizeof(DPoint3d), HEAPSIG_BPRC)))
        return ERROR;

    if (curveP->rational &&
        NULL ==
        (segP->weights = (double*)msbspline_malloc (numSegmentPoles * sizeof(double), HEAPSIG_BPRC)))
        return ERROR;

    segP->rational         = curveP->rational;
    segP->params.numPoles  =
    segP->params.order     = curveP->params.order;
    segP->params.closed    =
    segP->params.numKnots  = 0;
    segP->knots            =  bspknot_bezierKnotVector (segP->params.order);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    earlinLutz      10/94
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void      bspproc_freeBezierSegment
(
MSBsplineCurve  *segP                    /* <= bezier segment of curve */
)
    {
    if( segP->poles   ) msbspline_free( segP->poles );
    if( segP->weights ) msbspline_free( segP->weights );
    segP->poles   = 0;
    segP->weights = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    earlinLutz      10/94
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void      bspproc_freeBezierPatch
(
MSBsplineSurface        *surfaceP                /* <= bezier patch of surface */
)
    {
    if( surfaceP->poles )   msbspline_free( surfaceP->poles );
    if( surfaceP->weights ) msbspline_free( surfaceP->weights );
    surfaceP->poles   = 0;
    surfaceP->weights = 0;
    }

/*---------------------------------------------------------------------------------**//**
* Analogy to mdlBspline_netPoles. Copy poles and weights from a 'prepped curve' index to working arrays
* @bsimethod                                                    earlinLutz      12/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bspproc_segmentPoles
(
DPoint3d            *poles,
double              *weights,
int                 uIndex,
MSBsplineCurve   *curveP
)
    {
    int         offset = uIndex;

    if (curveP->rational)
        {
        memcpy (poles,   curveP->poles + offset, curveP->params.order * sizeof(DPoint3d));
        memcpy (weights, curveP->weights + offset, curveP->params.order * sizeof(double));
        }
    else
        {
        memcpy (poles, curveP->poles + offset, curveP->params.order * sizeof(DPoint3d));
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             12/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_processBsplineSurface
(
BezierInfo          *infoP,           /* <=> information desired of surf */
MSBsplineSurface    *bspline,         /* => Bspline surface to process */
PFBSurf_Sort       sortFunc,          /* => sorting function, or NULL */
PFBSurf_Go         goFunc,            /* => go function, or NULL */
PFBSurf_Process    processFunc,       /* => acts on Bezier patches */
PFBSurf_Select     selectFunc,        /* => loads ouput based on patches */
PFBSurf_Clean      cleanFunc          /* => called before returning */
)
    {
    int                 i, j, status, uNumSegs, vNumSegs, uSeg, vSeg, *tags=NULL,
                        *uStart=NULL, *vStart=NULL;
    BezierInfo          segInfo;
    MSBsplineSurface    surf, bezier;

    memset (&bezier, 0, sizeof(bezier));

    if (SUCCESS != (status = bspproc_prepareSurface (&surf, &uNumSegs, &vNumSegs,
                                                    &uStart, &vStart, bspline)))
        goto wrapup;

    if (sortFunc)
        {
        if (NULL == (tags = (int*)msbspline_malloc (uNumSegs * vNumSegs * sizeof(int), HEAPSIG_BPRC)))
            {
            status = ERROR;
            goto wrapup;
            }
        if (SUCCESS != (status = (*sortFunc) (tags, infoP, uStart, vStart,
                                              uNumSegs, vNumSegs, &surf)))
            goto wrapup;
        }
    if (SUCCESS != (status = bspproc_initializeBezierPatch (&bezier, &surf)))
        goto wrapup;

    for (j=0; j < vNumSegs; j++)
        for (i=0; i < uNumSegs; i++)
            {
            if (tags)
                {
                uSeg = tags[j*uNumSegs+i] % uNumSegs;
                vSeg = tags[j*uNumSegs+i] / uNumSegs;
                }
            else
                {
                uSeg = i;
                vSeg = j;
                }
            segInfo = *infoP;
            bsprsurf_netPoles (bezier.poles, bezier.weights, uStart[uSeg], vStart[vSeg],
                               &surf);
            if (! goFunc ||  (*goFunc) (&segInfo, &bezier, uSeg, vSeg,
                                        uStart[uSeg], vStart[vSeg], uNumSegs, vNumSegs))
                {
                if (SUCCESS != (status = (*processFunc) (&segInfo, &bezier)))
                    goto wrapup;

                if (selectFunc &&
                    SUCCESS != (status = (*selectFunc) (infoP, &segInfo, &bezier, uSeg, vSeg,
                                                        uStart[uSeg], vStart[vSeg],
                                                        uNumSegs, vNumSegs)))
                    goto wrapup;
                }
            }

wrapup:
    if (cleanFunc)
        (*cleanFunc) (infoP, uNumSegs, vNumSegs);

#if defined (NO_BOUNDARY_FREE)
    /* clear the copied boundary information before freeing surf */
    surf.numBounds = 0;
    surf.boundaries = NULL;
#endif
    bspsurf_freeSurface (&surf);

    if (bezier.poles)       msbspline_free (bezier.poles);
    if (bezier.weights)     msbspline_free (bezier.weights);
    if (tags)               msbspline_free (tags);
    if (uStart)             msbspline_free (uStart);
    if (vStart)             msbspline_free (vStart);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             12/90
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_doubleProcessBsplineSurface
(
BezierInfo          *infoP,           /* <=> information desired of surf */
MSBsplineSurface    *bspline0,        /* => Bspline surface to process */
MSBsplineSurface    *bspline1,        /* => Bspline surface to process */
PFBSurfBSurf_Sort       sortFunc,          /* => sorting function, or NULL */
PFBSurfBSurf_Go         goFunc,            /* => go function, or NULL */
PFBSurfBSurf_Process    processFunc,       /* => acts on Bezier patches */
PFBSurfBSurf_Select     selectFunc,        /* => loads ouput based on patches */
PFBSurfBSurf_Clean      cleanFunc          /* => called before returning */
)
    {
    int                 i, j, k, l, status, uNumSegs0, vNumSegs0, uNumSegs1 = 0, vNumSegs1 = 0,
                        uSeg0, vSeg0, uSeg1, vSeg1, *tags, numPatchPoles0,
                        numPatchPoles1, *uStart0, *vStart0, *uStart1, *vStart1;
    BezierInfo          segInfo;
    MSBsplineSurface    surf0, surf1, bezier0, bezier1;

    tags = uStart0 = vStart0 = uStart1 = vStart1 = NULL;
    bezier0.poles = bezier1.poles = NULL;    bezier0.weights = bezier1.weights = NULL;
    memset (&surf0, 0, sizeof(surf0));
    memset (&surf1, 0, sizeof(surf1));

    if (SUCCESS != (status = bspproc_prepareSurface (&surf0, &uNumSegs0, &vNumSegs0,
                                                     &uStart0, &vStart0, bspline0)) ||
        SUCCESS != (status = bspproc_prepareSurface (&surf1, &uNumSegs1, &vNumSegs1,
                                                     &uStart1, &vStart1, bspline1)))
        goto wrapup;

    if (sortFunc)
        {
        if (NULL ==
            (tags = (int*)msbspline_malloc (uNumSegs0 * vNumSegs0 * uNumSegs1 * vNumSegs1
                                    * sizeof(int), HEAPSIG_BPRC)))
            {
            status = ERROR;
            goto wrapup;
            }
        if (SUCCESS != (status = (*sortFunc) (tags, infoP, uStart0, vStart0, uStart1, vStart1,
                                              uNumSegs0, vNumSegs0, uNumSegs1, vNumSegs1,
                                              &surf0, &surf1)))
            goto wrapup;
        }

    numPatchPoles0 = surf0.uParams.order * surf0.vParams.order;
    numPatchPoles1 = surf1.uParams.order * surf1.vParams.order;
    if (NULL ==
        (bezier0.poles = (DPoint3d*)msbspline_malloc (numPatchPoles0 * sizeof(DPoint3d), HEAPSIG_BPRC)) ||
        NULL ==
        (bezier1.poles = (DPoint3d*)msbspline_malloc (numPatchPoles1 * sizeof(DPoint3d), HEAPSIG_BPRC)))
        {
        status = ERROR;
        goto wrapup;
        }

    if ((surf0.rational &&
        NULL ==
        (bezier0.weights = (double*)msbspline_malloc (numPatchPoles0 * sizeof(double), HEAPSIG_BPRC))) ||
        (surf1.rational &&
        NULL ==
        (bezier1.weights = (double*)msbspline_malloc (numPatchPoles1 * sizeof(double), HEAPSIG_BPRC))))
        {
        status = ERROR;
        goto wrapup;
        }

    bezier0.rational         = surf0.rational;
    bezier0.uParams.numPoles =
    bezier0.uParams.order    = surf0.uParams.order;
    bezier0.vParams.numPoles =
    bezier0.vParams.order    = surf0.vParams.order;
    bezier0.uParams.closed   =
    bezier0.vParams.closed   =
    bezier0.uParams.numKnots =
    bezier0.vParams.numKnots = 0;
    bezier0.uKnots           = bspknot_bezierKnotVector (bezier0.uParams.order);
    bezier0.vKnots           = bspknot_bezierKnotVector (bezier0.vParams.order);
    bezier0.holeOrigin       = surf0.holeOrigin;
    bezier0.numBounds        = surf0.numBounds;
    bezier0.boundaries       = surf0.boundaries;

    bezier1.rational         = surf1.rational;
    bezier1.uParams.numPoles =
    bezier1.uParams.order    = surf1.uParams.order;
    bezier1.vParams.numPoles =
    bezier1.vParams.order    = surf1.vParams.order;
    bezier1.uParams.closed   =
    bezier1.vParams.closed   =
    bezier1.uParams.numKnots =
    bezier1.vParams.numKnots = 0;
    bezier1.uKnots           = bspknot_bezierKnotVector (bezier1.uParams.order);
    bezier1.vKnots           = bspknot_bezierKnotVector (bezier1.vParams.order);
    bezier1.holeOrigin       = surf1.holeOrigin;
    bezier1.numBounds        = surf1.numBounds;
    bezier1.boundaries       = surf1.boundaries;

    for (l=0; l < vNumSegs1; l++)
        for (k=0; k < uNumSegs1; k++)
            for (j=0; j < vNumSegs0; j++)
                for (i=0; i < uNumSegs0; i++)
            {
#if defined (needs_work)
            /* If tags use them to order the patches */
            if (tags)
                {}
#endif
            uSeg0 = i;    vSeg0 = j;    uSeg1 = k;      vSeg1 = l;
            segInfo = *infoP;
            bsprsurf_netPoles (bezier0.poles, bezier0.weights,
                              uStart0[uSeg0], vStart0[vSeg0], &surf0);
            bsprsurf_netPoles (bezier1.poles, bezier1.weights,
                              uStart1[uSeg1], vStart1[vSeg1], &surf1);

            if (! goFunc ||  (*goFunc) (&segInfo, &bezier0, &bezier1,
                                        uSeg0, vSeg0, uSeg1, vSeg1,
                                        uStart0[uSeg0], vStart0[vSeg0],
                                        uStart1[uSeg1], vStart1[vSeg1],
                                        uNumSegs0, vNumSegs0, uNumSegs1, vNumSegs1))
                {
                if (SUCCESS != (status = (*processFunc) (&segInfo, &bezier0, &bezier1)))
                    goto wrapup;

                if (selectFunc &&
                    SUCCESS != (status = (*selectFunc)(infoP, &segInfo, &bezier0, &bezier1,
                                                       uSeg0, vSeg0, uSeg1, vSeg1,
                                                       uStart0[uSeg0], vStart0[vSeg0],
                                                       uStart1[uSeg1], vStart1[vSeg1],
                                                       uNumSegs0, vNumSegs0,
                                                       uNumSegs1, vNumSegs1)))
                    goto wrapup;
                }
            }

wrapup:
    if (cleanFunc)
        (*cleanFunc) (infoP, uNumSegs0, vNumSegs0, uNumSegs1, vNumSegs1);

#if defined (NO_BOUNDARY_FREE)
    /* clear the copied boundary information before freeing surf */
    surf0.numBounds  = surf1.numBounds  = 0;
    surf0.boundaries = surf1.boundaries = NULL;
#endif
    bspsurf_freeSurface (&surf0);
    bspsurf_freeSurface (&surf1);

    if (bezier0.poles)      msbspline_free (bezier0.poles);
    if (bezier0.weights)    msbspline_free (bezier0.weights);
    if (bezier1.poles)      msbspline_free (bezier1.poles);
    if (bezier1.weights)    msbspline_free (bezier1.weights);
    if (tags)               msbspline_free (tags);
    if (uStart0)            msbspline_free (uStart0);
    if (vStart0)            msbspline_free (vStart0);
    if (uStart1)            msbspline_free (uStart1);
    if (vStart1)            msbspline_free (vStart1);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BSI             09/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_doubleProcessSurfaceByCurves
(
MSBsplineSurface    *outSurface,   /* <= != bspline0 && != bspline1 */
MSBsplineSurface    *bspline0,
MSBsplineSurface    *bspline1,
PFBCurveBCurveBCurveVoidInt processFunc,   /* => acts on row/column curves of surface */
PFBSurfBSurfBSurfVoidInt    boundaryFunc,  /* => acts on boundaries of input surfaces */
void                *args,
int                 direction
)
    {
    int             i, numCurves, major = 1, minor = 1, status = SUCCESS, numPoles = 0, numBounds;
    double          **knotP, *surfWtP = NULL, *srfWP, *crvWP, *endW;
    DPoint3d        *surfPoleP = NULL, *srfPP, *crvPP, *endP;
    MSBsplineCurve  crv0, crv1, crv2;

    memset (&crv0, 0, sizeof(crv0));
    memset (&crv1, 0, sizeof(crv1));
    memset (&crv2, 0, sizeof(crv2));
    memset (outSurface, 0, sizeof(*outSurface));

    numCurves = direction == BSSURF_U ?
                bspline0->vParams.numPoles :
                bspline0->uParams.numPoles;

    outSurface->rational = bspline0->rational;
    outSurface->display  = bspline0->display;
    outSurface->uParams  = bspline0->uParams;
    outSurface->vParams  = bspline0->vParams;

    for (i=0; i < numCurves; i++, surfPoleP += major, surfWtP += major)
        {
        if (SUCCESS !=
            (status = bspconv_getCurveFromSurface (&crv1, bspline0, direction, i)) ||
            SUCCESS !=
            (status = bspconv_getCurveFromSurface (&crv2, bspline1, direction, i)))
            goto wrapup;

        if (SUCCESS != (status = (*processFunc) (&crv0, &crv1, &crv2, args, i)))
            goto wrapup;

        if (i==0)
            {
            numPoles = crv0.params.numPoles;
            if (direction == BSSURF_U)
                {
                outSurface->uParams = crv0.params;
                knotP = &outSurface->uKnots;
                major = outSurface->uParams.numPoles;
                minor = 1;
                }
            else
                {
                outSurface->vParams = crv0.params;
                knotP = &outSurface->vKnots;
                major = 1;
                minor = outSurface->uParams.numPoles;
                }

            if (SUCCESS != (status = bspsurf_allocateSurface (outSurface)))
                goto wrapup;

            surfPoleP = outSurface->poles;
            surfWtP   = outSurface->weights;

            /* Only copy new knot vector first time */
            memcpy (*knotP, crv0.knots,
                    bspknot_numberKnots (crv0.params.numPoles, crv0.params.order,
                                         crv0.params.closed) * sizeof(double));
            }

        /* copy poles & weights to surf */
        for (crvPP = endP = crv0.poles, srfPP = surfPoleP,
             endP += numPoles;
             crvPP < endP; srfPP += minor, crvPP++)
            *srfPP = *crvPP;
        if (outSurface->rational)
            for (crvWP = endW = crv0.weights, srfWP=surfWtP,
                 endW += numPoles;
                 crvWP < endW; srfWP += minor, crvWP++)
                *srfWP = *crvWP;

        bspcurv_freeCurve (&crv0);
        bspcurv_freeCurve (&crv1);
        bspcurv_freeCurve (&crv2);
        }

    /* Copy unchanged knot vector */
    if (direction == BSSURF_U)
        {
        memcpy (outSurface->vKnots, bspline0->vKnots,
                bspknot_numberKnots (outSurface->vParams.numPoles, outSurface->vParams.order,
                                     outSurface->vParams.closed) * sizeof(double));
        }
    else
        {
        memcpy (outSurface->uKnots, bspline0->uKnots,
                bspknot_numberKnots (outSurface->uParams.numPoles, outSurface->uParams.order,
                                     outSurface->uParams.closed) * sizeof(double));
        }

    /* Process boundaries of surface */
    if (boundaryFunc &&
        0 != (numBounds = bspline0->numBounds + bspline1->numBounds))
        {
        if (NULL == (outSurface->boundaries =
                        (BsurfBoundary*)BSIBaseGeom::Malloc (numBounds * sizeof (BsurfBoundary))))
            {
            status = ERROR;
            goto wrapup;
            }
        outSurface->numBounds = numBounds;

        if (SUCCESS != (status = (*boundaryFunc) (outSurface, bspline0, bspline1,
                                                  args, direction)))
            goto wrapup;
        }

wrapup:
    bspcurv_freeCurve (&crv0);
    bspcurv_freeCurve (&crv1);
    bspcurv_freeCurve (&crv2);
    return status;
    }


/*---------------------------------------------------------------------------------**//**
* x1Hi x2Hi yHi +--------------+ / \ / \ yLo +---------------------+ x1Lo x2Lo
* @bsimethod                                                    RBB             09/89
+---------------+---------------+---------------+---------------+---------------+------*/
static int      bspproc_processSurfaceMeshPatch
(
double              vHi,
double              vLo,
double              u1Hi,
double              u1Lo,
double              u2Hi,
double              u2Lo,
MSBsplineSurface    *surfP,
double              strokeTolerance,
PFDPoint3dPDPoint3dPDPoint2dPVoidP processFunc,
int                 (*stopFunc)(),
void                *args
)
    {
    int         i, j, k, l, status, uInsertNum, vInsertNum, iTmp,
                uMinIndex, uMaxIndex, vMinIndex, vMaxIndex;
    double      scanMin, scanMax, u1Min, u1Max, u2Min, u2Max, m1, m2,
                u, v, vDelta, uMinimum, uMaximum, uDelta, uMin, uMax,
                u1SpMax, u2SpMax, u1SpMin, u2SpMin, uSpanMinimum, uSpanMaximum,
                vSpanMinimum, vSpanMaximum;
    DPoint3d    ll, lr, ul, ur, dPdU, dPdV, points[5], normals[5], verts1[MAX_INSERT],
                verts2[MAX_INSERT], norms1[MAX_INSERT], norms2[MAX_INSERT];
    DPoint2d    uvPoints[5],
                uv1[MAX_INSERT], uv2[MAX_INSERT];

    if (vLo == vHi || (u1Hi == u2Hi && u1Lo == u2Lo)) return (SUCCESS);

    m1 = (u1Hi - u1Lo)/(vHi-vLo);
    m2 = (u2Hi - u2Lo)/(vHi-vLo);

    bspknot_findSpan (&vMinIndex, surfP->vKnots,  surfP->vParams.numPoles,
                                  surfP->vParams.order, surfP->vParams.closed, vLo);

    bspknot_findSpan (&vMaxIndex, surfP->vKnots,  surfP->vParams.numPoles,
                                  surfP->vParams.order, surfP->vParams.closed, vHi);

    for (i = vMinIndex; i <= vMaxIndex && (! stopFunc || ! (*stopFunc)()); i++)
        {
        if (surfP->vKnots[i + 1] - surfP->vKnots[i] < 1.0E-9)
            continue;

        vSpanMinimum = surfP->vKnots[i] + 1.0E-9;
        vSpanMaximum = surfP->vKnots[i + 1] - 1.0E-9;
        scanMin = vSpanMinimum < vLo ? vLo : vSpanMinimum;
        scanMax = vSpanMaximum > vHi ? vHi : vSpanMaximum;
        u1Min = u1Lo + m1 * (scanMin - vLo);
        u1Max = u1Lo + m1 * (scanMax - vLo);
        u2Min = u2Lo + m2 * (scanMin - vLo);
        u2Max = u2Lo + m2 * (scanMax - vLo);

        bspknot_findSpan (&uMinIndex, surfP->uKnots,  surfP->uParams.numPoles,
                                      surfP->uParams.order, surfP->uParams.closed,
                                      u1Min < u1Max ? u1Min : u1Max);

        bspknot_findSpan (&uMaxIndex, surfP->uKnots,  surfP->uParams.numPoles,
                                      surfP->uParams.order, surfP->uParams.closed,
                                      u2Min < u2Max ? u2Min : u2Max);

        for (j = uMinIndex; j <= uMaxIndex; j++)
            {
            if (surfP->uKnots[j + 1] - surfP->uKnots[j] < 1.0E-9)
                continue;

            uSpanMinimum = surfP->uKnots[j] + 1.0E-9;
            uSpanMaximum = surfP->uKnots[j + 1] - 1.0E-9;
            u1SpMin = uSpanMinimum < u1Min ? u1Min : uSpanMinimum;
            u1SpMax = uSpanMinimum < u1Max ? u1Max : uSpanMinimum;
            u2SpMin = uSpanMaximum > u2Min ? u2Min : uSpanMaximum;
            u2SpMax = uSpanMaximum > u2Max ? u2Max : uSpanMaximum;

            uMin = u1SpMin < u1SpMax ? u1SpMin : u1SpMax;
            uMax = u2SpMin < u2SpMax ? u2SpMin : u2SpMax;

            bspsurf_evaluateSurfacePoint (&ll, NULL, &dPdU, &dPdV, uMin, scanMin, surfP);
            bspsurf_evaluateSurfacePoint (&ul, NULL, NULL, NULL, uMin, scanMax, surfP);
            bspsurf_evaluateSurfacePoint (&lr, NULL, NULL, NULL, uMax, scanMin, surfP);

            uInsertNum = bspline_spansReqd (&dPdU, &ll, &lr, strokeTolerance);
            vInsertNum = bspline_spansReqd (&dPdV, &ll, &ul, strokeTolerance);

            bspsurf_evaluateSurfacePoint (&ur, NULL, &dPdU, &dPdV, uMax, scanMax, surfP);

            dPdU.Scale (dPdU, -1.0);
            dPdV.Scale (dPdV, -1.0);
            if ((iTmp = bspline_spansReqd (&dPdU, &ur, &ul, strokeTolerance)) >
                 uInsertNum) uInsertNum = iTmp;
            if ((iTmp = bspline_spansReqd (&dPdV, &ur, &lr, strokeTolerance)) >
                 vInsertNum) vInsertNum = iTmp;

            uInsertNum += 2;    vInsertNum += 2;
            if (uInsertNum > MAX_INSERT)
                uInsertNum = MAX_INSERT;
            if (vInsertNum > MAX_INSERT)
                vInsertNum = MAX_INSERT;

            vDelta = (scanMax - scanMin)/(double) (vInsertNum - 1);

            for (k = 0, v = scanMin; k < vInsertNum; k++, v += vDelta)
                {
                uMinimum = u1SpMin + m1 * (v - scanMin);
                uMaximum = u2SpMin + m2 * (v - scanMin);
                uDelta = (uMaximum - uMinimum)/(double) (uInsertNum - 1);
                for (l=0, u = uMinimum; l < uInsertNum; l++, u += uDelta)
                    {
                    uv2[l].x = u;
                    uv2[l].y = v;
                    bspsurf_evaluateSurfacePoint (verts2 + l, NULL, &dPdU, &dPdV,
                                                  u, v, surfP);

                    norms2[l].CrossProduct (dPdU, dPdV);
                    norms2[l].Normalize ();
                    if (k && l)
                        {
                        points[0] = points[4] = *(verts1 + l - 1);
                        points[1] = *(verts1 + l);
                        points[2] = *(verts2 + l);
                        points[3] = *(verts2 + l - 1);

                        normals[0] = normals[4] = *(norms1 + l - 1);
                        normals[1] = *(norms1 + l);
                        normals[2] = *(norms2 + l);
                        normals[3] = *(norms2 + l - 1);

                        uvPoints[0] = uvPoints[4] = *(uv1 + l - 1);
                        uvPoints[1] = *(uv1 + l);
                        uvPoints[2] = *(uv2 + l);
                        uvPoints[3] = *(uv2 + l - 1);

                        if (SUCCESS !=
                            (status = (*processFunc)(points, normals, uvPoints, args)))
                            return status;
                        }
                    }
                memcpy (verts1, verts2, uInsertNum*sizeof(DPoint3d));
                memcpy (norms1, norms2, uInsertNum*sizeof(DPoint3d));
                memcpy (uv1, uv2, uInsertNum * sizeof(DPoint2d));
                }
            }
        }
    return (SUCCESS);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             01/88
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bspline_computeMeshSpans
(
double              *iHi,
double              *iLo,
int                 *nHi,
int                 *nLo,
MSBsplineSurface    *surfP,
double              y
)
    {
    int             i, k, prev, curr, next, numVerts, index;
    double          value;
    DPoint2d        *pointP;

    *nLo = *nHi = 0;
    if (! surfP->holeOrigin)
        {
        if (y != 0.0)
            {
            iLo[(*nLo)++] = 0.0;
            iLo[(*nLo)++] = 1.0;
            }
        if (y != 1.0)
            {
            iHi[(*nHi)++] = 0.0;
            iHi[(*nHi)++] = 1.0;
            }
        }

    for (k=0; k < surfP->numBounds; k++)
        {
        numVerts = surfP->boundaries[k].numPoints;
        pointP   = surfP->boundaries[k].points;

        curr = util_compareDoubles (y, pointP[0].y);
        next = util_compareDoubles (y, pointP[1].y);
        for (i=1; i < numVerts; i++)
            {
            prev = curr;
            curr = next;
            index = i % (numVerts-1);
            next = util_compareDoubles (y, pointP[(i+1) % (numVerts-1)].y);
            if (curr)
                {
                if (curr*prev < 0)
                    {
                    value = (pointP[i-1].x +
                          (y - pointP[i-1].y)*(pointP[index].x - pointP[i-1].x) /
                                              (pointP[index].y - pointP[i-1].y));

                    iLo[(*nLo)++] = value;
                    iHi[(*nHi)++] = value;
                    }
                }
            else
                {
                if (prev < 0)
                    iLo[(*nLo)++] = pointP[index].x;
                else if (prev > 0)
                    iHi[(*nHi)++] = pointP[index].x;
                if (next < 0)
                    iLo[(*nLo)++] = pointP[index].x;
                else if (next > 0)
                    iHi[(*nHi)++] = pointP[index].x;
                }
            }
        }

    /* now bubble sort the intersection lists */
    if (*nLo)       mdlUtil_sortDoubles (iLo, *nLo, true);
    if (*nHi)       mdlUtil_sortDoubles (iHi, *nHi, true);
    }

/*---------------------------------------------------------------------------------**//**
* Process a B-spline surface by operating on polygonal mesh. processFunc : receives 4 corners of polygons and normals.
* @bsimethod                                                    RBB             12/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_processSurfaceMesh
(
MSBsplineSurface    *surfP,            /* => surface to process */
double              tolerance,         /* => stroke tolerance */
PFDPoint3dPDPoint3dPDPoint2dPVoidP processFunc,
int                 (*stopFunc)(),     /* => stop function */
void                *args              /* => arguments */
)
    {
    int                 i, j, numBoundVerts, nLo, nHi, nTmp, status = SUCCESS;
    bool                workBufferAllocated = false;
    double              yLo, yHi, *yVerts, *intLo, *intHi, *tmp, *workBuffer;
    BsurfBoundary       *boundP, *endBoundP;

    if (surfP->numBounds)
        {
        numBoundVerts = 0;
        endBoundP = surfP->boundaries + surfP->numBounds;
        for (boundP = surfP->boundaries; boundP < endBoundP; boundP++)
            numBoundVerts += boundP->numPoints;

        if (! surfP->holeOrigin) numBoundVerts += 2;
        if (NULL == (workBuffer =
                     (double*)BSIBaseGeom::Malloc (5*numBoundVerts * sizeof(double))))
            return ERROR;
        workBufferAllocated = true;

        intLo  = workBuffer;
        intHi  = intLo + numBoundVerts;
        tmp    = intHi + numBoundVerts;
        yVerts = tmp + numBoundVerts;

        for (numBoundVerts = 0, boundP = surfP->boundaries; boundP < endBoundP; boundP++)
            for (j = 0; j < boundP->numPoints; j++)
                yVerts[numBoundVerts++] = boundP->points[j].y;

        if (! surfP->holeOrigin)
            {
            yVerts[numBoundVerts++] = 0.0;
            yVerts[numBoundVerts++] = 1.0;
            }
        mdlUtil_sortDoubles (yVerts, numBoundVerts, false);

        yHi = yVerts[0];
        bspline_computeMeshSpans (intHi, intLo, &nHi, &nLo, surfP, yHi);

        for (i = 1; i < numBoundVerts && (! stopFunc || ! (*stopFunc)()); i++)
            {
            while (yVerts[i] == yVerts[i-1] && i<numBoundVerts) i++;
            if (i >= numBoundVerts) break;
            yLo = yVerts[i];
            bspline_computeMeshSpans (intHi, tmp, &nHi, &nTmp, surfP, yLo);

            for (j = 0;
                    j < nHi - 1 && j < nLo - 1 && (! stopFunc || ! (*stopFunc)());
                        j+=2)
                {
                if (SUCCESS != (status = bspproc_processSurfaceMeshPatch (yHi, yLo,
                                                 intLo[j], intHi[j],intLo[j+1], intHi[j+1],
                                                 surfP, tolerance,
                                                 processFunc, stopFunc, args)))
                    goto wrapup;
                }
            yHi = yLo;
            memcpy (intLo, tmp, nTmp*sizeof(double));
            nLo = nTmp;
            }
        }
    else
        {
        return bspproc_processSurfaceMeshPatch (1.0, 0.0, 0.0, 0.0, 1.0, 1.0, surfP,
                                                tolerance, processFunc, stopFunc, args);
        }
wrapup:

    if (workBufferAllocated)
        BSIBaseGeom::Free (workBuffer);

    return status;
    }
#ifdef COMPILE_OBS_STROKERS
/*---------------------------------------------------------------------------------**//**
* Process a B-spline curve by operating on line segments
* @bsimethod                                                    RBB             12/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_processCurveAsLines
(
MSBsplineCurve  *curveP,                /* => curve to process */
double          tolerance,              /* => stroke tolerance */
RotMatrix       *matrix,                /* => of view to stroke in, or NULL */
int             (*processFunc)(),       /* => process function */
int             (*stopFunc)(),          /* => stop function */
void            *args                   /* => arguments */
)
    {
    return bspstrok_strokeCurveByFunction (curveP, tolerance, matrix,
                                            stopFunc, processFunc, args);
    }

#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             12/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_processCurvePolygon
(
MSBsplineCurve  *curveP,                /* => curve to process */
PFDPoint3dPIntVoidP processFunc,       /* => process function */
void            *args                   /* => arguments */
)
    {
    int                 numPoles = curveP->params.numPoles, status;
    bool                polesAllocated = false;
    DPoint3d            *poleP = NULL, closure[2];

    if (curveP->rational)
        {
        if (NULL == (poleP = (DPoint3d*)BSIBaseGeom::Malloc (numPoles * sizeof(DPoint3d))))
            return ERROR;
        
        polesAllocated = true;
        bsputil_unWeightPoles (poleP, curveP->poles, curveP->weights, numPoles);
        }
    else
        {
        poleP = curveP->poles;
        }
    status = (*processFunc)(poleP, numPoles, args);
    if (SUCCESS == status && curveP->params.closed)
        {
        closure[0] = poleP[numPoles-1];
        closure[1] = poleP[0];
        status = (*processFunc)(closure, 2, args);
        }

    if (polesAllocated)
        BSIBaseGeom::Free (poleP);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          01/01
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_processInterpolationCurvePolygon
(
MSInterpolationCurve    *curveP,                /* => curve to process */
PFDPoint3dPIntVoidP processFunc,       /* => process function */
void            *args                   /* => arguments */
)
    {
    int         numPoints = curveP->params.numPoints, status = ERROR;
    DPoint3d    startPoints[2], endPoints[2];

    if (!curveP->params.isPeriodic)
        {
        mdlBspline_extractInterpolationTangentPoints (&startPoints[1], &endPoints[1], curveP);
        startPoints[0] = curveP->fitPoints[0];
        endPoints[0] = curveP->fitPoints[numPoints-1];
        status = (*processFunc)(startPoints, 2, args);
        status = (*processFunc)(endPoints, 2, args);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* 1 +--+--+--+--+ |\ |\ |\ |\ | This routine checks the triangles of | \| \| \| \| the polygon control net. If the lower +--+--+--+--+ left
* triangle is not hit (i.e. the v |\ |\ |\ |\ | intersection's barycentric coordinates | \| \| \| \| add up to more than 1.0 for the triangle)
* +--+--+--+--+ then the upper right triangles is |\ |\ |\ |\ | checked. | \| \| \| \| 0 +--+--+--+--+ 0 u 1
* @bsimethod                                                    Brian.Peters    02/93
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspproc_processControlNet
(
DPoint3d        *points,               /* => points defining net */
int             uNum,                  /* => number of points in u (varies fastest) */
int             vNum,                  /* => number of points in v */
PFDPoint3dPDPoint3dPDPoint3dPVoidPBool_IntIntIntInt goFunc,      /* => decides whether or not to process triangle */
PFDPoint3dPDPoint3dPDPoint3dPVoidPBool_IntIntIntInt processFunc, /* => preocesses triangle */
PFDPoint3dPDPoint3dPDPoint3dPVoidPBool_IntIntIntInt selectFunc,  /* => accumulates results */
void            *argsP                 /* => passed through to processFunc */
)
    {
    int             i, j, altI, altJ, status=SUCCESS;
    DPoint3d        *basePt, *vEnd, *uEnd, *altBasePt, *rowP, *endRow, *endP;

    for (rowP = endRow = points, endRow += uNum * (vNum - 1), i=0;
         rowP < endRow; rowP += uNum, i++)
        for (basePt = uEnd = vEnd = altBasePt = endP = rowP,
             endP += uNum - 1,
             uEnd += 1,
             vEnd += uNum,
             altBasePt += uNum + 1,
             j=0;
             basePt < endP;
             j++, basePt++, uEnd++, vEnd++, altBasePt++)
            {
            /* Check lower left triangle of this quadrilateral */
            if (! goFunc || (*goFunc) (basePt, uEnd, vEnd, argsP,
                                       false, i, j, uNum, vNum))
                {
                if (processFunc &&
                    SUCCESS != (status = (*processFunc) (basePt, uEnd, vEnd, argsP,
                                                         false, i, j, uNum, vNum)))
                    return status;

                if (selectFunc &&
                    SUCCESS != (status = (*selectFunc) (basePt, uEnd, vEnd, argsP,
                                                        false, i, j, uNum, vNum)))
                    return status;
                }

            /* Check upper right triangle of this quadrilateral */
            altI = i+1;
            altJ = j+1;
            if (! goFunc || (*goFunc) (altBasePt, vEnd, uEnd, argsP,
                                       true, altI, altJ, uNum, vNum))
                {
                if (processFunc &&
                    SUCCESS != (status = (*processFunc) (altBasePt, vEnd, uEnd, argsP,
                                                         true, altI, altJ, uNum, vNum)))
                    return status;

                if (selectFunc &&
                    SUCCESS != (status = (*selectFunc) (altBasePt, vEnd, uEnd, argsP,
                                                        true, altI, altJ, uNum, vNum)))
                    return status;
                }
            }

    return status;
    }

#endif /* !FONTIMPORTER */




END_BENTLEY_GEOMETRY_NAMESPACE