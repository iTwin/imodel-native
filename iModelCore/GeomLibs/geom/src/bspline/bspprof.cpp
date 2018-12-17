/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/bspline/bspprof.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/MstnOnly/BspPrivateApi.h>
#include "msbsplinemaster.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

typedef struct silhouettePt
    {
    DPoint3d    xyz;
    DPoint2d    uv;
    } SilhouettePt;

/*----------------------------------------------------------------------+
|                                                                       |
|   Immediate Defines                                                   |
|                                                                       |
+----------------------------------------------------------------------*/
#define     MAX_ITER                        5
#define     MAX_POINTS                      2
#define     fc_newtonTol                    0.0005
#define     TOLERANCE_SilhouetteTerms       1.0E-6
#define     TOLERANCE_TangentMagnitude      1.0E-6

#define     MAX_INTERSECTIONS               200

#define     APPEND_NONE                     0
#define     APPEND_ORG_ORG                  1
#define     APPEND_END_ORG                  2
#define     APPEND_ORG_END                  3
#define     APPEND_END_END                  4

static bool                 subdivide;
static int                  numListGP;
static DPoint3d             *cameraGP;
static PointList            **listGP;
static double               flatTol;

#define DEBUG_S_not
#ifdef DEBUG_S
static bool     debugDraw = 0;
extern debug_displaySurface (MSBsplineSurface*, int);
extern debug_displayCurve (MSBsplineCurve*, int);
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          08/92
+---------------+---------------+---------------+---------------+---------------+------*/
static void     bspprof_hornerSchemeBezier
(
DPoint3d        *ptOut,         /* point on Bezier curve */
double          *wgtOut,        /* NULL if non-rational */
DPoint3d        *polesP,        /* poles of Bezier curve */
double          *weightsP,      /* NULL if non-rational */
int             order,          /* order of Bezier */
double          u               /* value to evaluate at */
)
    {
    int         i, degree, nChoosei;
    double      u1, factor, *wPtr = NULL, sumWgt = 0.0;
    DPoint3d    *pPtr = NULL, sumPole;
    sumPole.Zero ();
    /* Initialize tmp variables */
    degree = order - 1;
    nChoosei = 1;
    u1 = 1.0 - u;
    factor = 1.0;
    sumPole.Scale (*polesP, u1);
    if (wgtOut)
        {
        wPtr = weightsP+1;
        sumWgt = *weightsP * u1;
        }

    /* Use Horner Scheme to evaluate Bezier, see Farin's book page 48 */
    for (i=1, pPtr=polesP+1; i<degree; i++, pPtr++)
        {
        factor *= u;
        nChoosei = nChoosei*(order-i)/i;
        if (wgtOut)
            {
            sumWgt = (sumWgt + factor * nChoosei * *wPtr) * u1;
            wPtr++;
            }
        sumPole.SumOf (sumPole, *pPtr, factor*nChoosei);
        sumPole.Scale (sumPole, u1);
        }

    ptOut->SumOf (sumPole, polesP[degree], factor*u);
    if (wgtOut)
        *wgtOut = sumWgt + factor * u * *(weightsP+degree);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          09/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bspprof_evaluateBezier
(
DPoint3d            *pointP,
DPoint3d            *dPdU,
DPoint3d            *dPdV,
DPoint3d            *dPdUU,
DPoint3d            *dPdVV,
DPoint3d            *dPdUV,
double              u,
double              v,
MSBsplineSurface    *patchBezP
)
    {
    int             i, j, uOrder, vOrder, uDegree, vDegree, uIndex, vIndex;
    bool            rational;
    double          rWgts[MAX_ORDER], rUParWgts[MAX_ORDER], rVParWgts[MAX_ORDER],
                    rUWgDiff[MAX_ORDER], rVWgDiff[MAX_ORDER],
                    *rWtr = NULL, tmpWgt = 1.0, *wgtP = NULL, *rUWtr = NULL, *wUDtr = NULL, *rUWDtr = NULL, tmpUWgt = 1.0, *rVWtr = NULL,
                    *wVDtr = NULL, *rVWDtr = NULL, tmpVWgt = 1.0, rUUParWgts[MAX_ORDER],
                    rVVParWgts[MAX_ORDER], rUVParWgts[MAX_ORDER],
                    rUUWgDiff[MAX_ORDER], rVVWgDiff[MAX_ORDER], rUVWgDiff[MAX_ORDER],
                    *rUUWtr = NULL, *wUUDtr = NULL, *rUUWDtr = NULL, tmpUUWgt = 1.0, *rVVWtr = NULL, *wVVDtr = NULL,
                    *rVVWDtr = NULL, tmpVVWgt = 1.0, *rUVWtr = NULL, *wUVDtr = NULL, *rUVWDtr = NULL, tmpUVWgt = 1.0;

    DPoint3d        rPoles[MAX_ORDER], rUPoDiff[MAX_ORDER], rVPoDiff[MAX_ORDER],
                    rUUPoDiff[MAX_ORDER], rVVPoDiff[MAX_ORDER], rUVPoDiff[MAX_ORDER],
                    rUParPoles[MAX_ORDER], rVParPoles[MAX_ORDER],
                    rUUParPoles[MAX_ORDER], rVVParPoles[MAX_ORDER], rUVParPoles[MAX_ORDER],
                    *poleP = NULL,  *rPtr = NULL, *rUPtr = NULL, *rUPDtr = NULL,
                    *pUDtr = NULL, *rVPtr = NULL, *pVDtr = NULL, *rVPDtr = NULL,
                    *rUUPtr = NULL, *rUUPDtr = NULL, *pUUDtr = NULL, *rVVPtr = NULL, *pVVDtr = NULL, *rVVPDtr = NULL,
                    *rUVPtr = NULL, *rUVPDtr = NULL, *pUVDtr = NULL;

    rational = (0 != patchBezP->rational);
    uOrder = patchBezP->uParams.order;
    vOrder = patchBezP->vParams.order;
    uDegree = uOrder - 1;
    vDegree = vOrder - 1;
    uIndex = uDegree - 1;
    vIndex = vDegree - 1;

    /* Compute point */
    if (rational)
        {
        wgtP=patchBezP->weights;
        rWtr=rWgts;
        }
    for (i=0, poleP=patchBezP->poles, rPtr=rPoles;
        i<vOrder; i++, poleP += uOrder, rPtr++)
        {
        bspprof_hornerSchemeBezier(rPtr, rational ? rWtr : NULL,
                               poleP, rational ? wgtP : NULL, uOrder, u);
        if (rational)
            {
            wgtP += uOrder;
            rWtr++;
            }
        }
    bspprof_hornerSchemeBezier(pointP, rational ? &tmpWgt : NULL,
                           rPoles, rational ? rWgts : NULL, vOrder, v);
    if (rational) pointP->Scale (*pointP, 1.0/tmpWgt);

    /* Compute u partial derivative */
    if (dPdU)
        {
        if (rational)
            {
            wgtP=patchBezP->weights;
            rUWtr=rUParWgts;
            }
        for (i=0, poleP=patchBezP->poles, rUPtr=rUParPoles;
            i<vOrder; i++, poleP += uOrder, rUPtr++)
            {
            /* Form the differences of poles along each row */
            if (rational)
                {
                wUDtr=wgtP;
                rUWDtr=rUWgDiff;
                }
            for (j=0, pUDtr=poleP, rUPDtr=rUPoDiff;
                 j<uDegree; j++, pUDtr++, rUPDtr++)
                {
                rUPDtr->DifferenceOf (pUDtr[1], *pUDtr);
                if (rational)
                    {
                    *rUWDtr = *(wUDtr+1) - *wUDtr;
                    wUDtr++;
                    rUWDtr++;
                    }
                }
            bspprof_hornerSchemeBezier (rUPtr, rational ? rUWtr : NULL,
                                        rUPoDiff, rational ? rUWgDiff : NULL,
                                        uDegree, u);
            if (rational)
                {
                wgtP += uOrder;
                rUWtr++;
                }
            }
        bspprof_hornerSchemeBezier (dPdU, rational ? &tmpUWgt : NULL,
                                    rUParPoles,
                                    rational ? rUParWgts : NULL,
                                    vOrder, v);
        }

    /* Compute v partial derivative */
    if (dPdV)
        {
        if (rational)
            {
            wgtP=patchBezP->weights;
            rVWtr=rVParWgts;
            }
        for (i=0, poleP=patchBezP->poles, rVPtr=rVParPoles;
            i<uOrder; i++, poleP++, rVPtr++)
            {
            /* Form the differences of poles along each row */
            if (rational)
                {
                wVDtr=wgtP;
                rVWDtr=rVWgDiff;
                }
            for (j=0, pVDtr=poleP, rVPDtr=rVPoDiff;
                j<vDegree; j++, pVDtr += uOrder, rVPDtr++)
                {
                rVPDtr->DifferenceOf (pVDtr[uOrder], *pVDtr);
                if (rational)
                    {
                    *rVWDtr = *(wVDtr+uOrder) - *wVDtr;
                    wVDtr += uOrder;
                    rVWDtr++;
                    }
                }
            bspprof_hornerSchemeBezier (rVPtr,
                                        rational ? rVWtr : NULL,
                                        rVPoDiff,
                                        rational ? rVWgDiff : NULL,
                                        vDegree, v);
            if (rational)
                {
                wgtP++;
                rVWtr++;
                }
            }
        bspprof_hornerSchemeBezier (dPdV, rational ? &tmpVWgt : NULL,
                                rVParPoles,
                                rational ? rVParWgts : NULL,
                                uOrder, u);
        }

    /* Compute second u partial derivative */
    if (dPdUU && uOrder>2)
        {
        if (rational)
            {
            wgtP=patchBezP->weights;
            rUUWtr=rUUParWgts;
            }
        for (i=0, poleP=patchBezP->poles, rUUPtr=rUUParPoles;
            i<vOrder; i++, poleP += uOrder, rUUPtr++)
            {
            /* Form the differences of poles along each row */
            if (rational)
                {
                wUUDtr=wgtP;
                rUUWDtr=rUUWgDiff;
                }
            for (j=0, pUUDtr=poleP, rUUPDtr=rUUPoDiff;
                 j<uIndex; j++, pUUDtr++, rUUPDtr++)
                {
                rUUPDtr->DifferenceOf (pUUDtr[2], pUUDtr[1]);
                rUUPDtr->DifferenceOf (*rUUPDtr, pUUDtr[1]);
                rUUPDtr->SumOf (*rUUPDtr, *pUUDtr);
                if (rational)
                    {
                    *rUUWDtr = *(wUUDtr+2) - 2.0 * *(wUUDtr+1) + *wUUDtr;
                    wUUDtr++;
                    rUUWDtr++;
                    }
                }

            bspprof_hornerSchemeBezier (rUUPtr, rational ? rUUWtr : NULL,
                                        rUUPoDiff, rational ? rUUWgDiff : NULL,
                                        uIndex, u);
            if (rational)
                {
                wgtP += uOrder;
                rUUWtr++;
                }
            }
        bspprof_hornerSchemeBezier (dPdUU, rational ? &tmpUUWgt : NULL,
                                    rUUParPoles,
                                    rational ? rUUParWgts : NULL,
                                    vOrder, v);
        }

    /* Compute second v partial derivative */
    if (dPdVV && vOrder>2)
        {
        if (rational)
            {
            wgtP=patchBezP->weights;
            rVVWtr=rVVParWgts;
            }
        for (i=0, poleP=patchBezP->poles, rVVPtr=rVVParPoles;
            i<uOrder; i++, poleP++, rVVPtr++)
            {
            /* Form the differences of poles along each row */
            if (rational)
                {
                wVVDtr=wgtP;
                rVVWDtr=rVVWgDiff;
                }
            for (j=0, pVVDtr=poleP, rVVPDtr=rVVPoDiff;
                j<vIndex; j++, pVVDtr += uOrder, rVVPDtr++)
                {
                rVVPDtr->DifferenceOf (pVVDtr[2*uOrder], pVVDtr[uOrder]);
                rVVPDtr->DifferenceOf (*rVVPDtr, pVVDtr[uOrder]);
                rVVPDtr->SumOf (*rVVPDtr, *pVVDtr);
                if (rational)
                    {
                    *rVVWDtr = *(wVVDtr+2*uOrder) - 2.0 * *(wVVDtr+uOrder) +
                               *wVVDtr;
                    wVVDtr += uOrder;
                    rVVWDtr++;
                    }
                }
            bspprof_hornerSchemeBezier (rVVPtr,
                                        rational ? rVVWtr : NULL,
                                        rVVPoDiff,
                                        rational ? rVVWgDiff : NULL,
                                        vIndex, v);
            if (rational)
                {
                wgtP++;
                rVVWtr++;
                }
            }
        bspprof_hornerSchemeBezier (dPdVV, rational ? &tmpVVWgt : NULL,
                                rVVParPoles,
                                rational ? rVVParWgts : NULL,
                                uOrder, u);
        }

    /* Compute second uv mixed partial derivative */
    if (dPdUV)
        {
        if (rational)
            {
            wgtP=patchBezP->weights;
            rUVWtr=rUVParWgts;
            }
        for (i=0, poleP=patchBezP->poles, rUVPtr=rUVParPoles;
            i<vDegree; i++, poleP += uOrder, rUVPtr++)
            {
            /* Form the differences of poles along each row */
            if (rational)
                {
                wUVDtr=wgtP;
                rUVWDtr=rUVWgDiff;
                }
            for (j=0, pUVDtr=poleP, rUVPDtr=rUVPoDiff;
                 j<uDegree; j++, pUVDtr++, rUVPDtr++)
                {
                rUVPDtr->DifferenceOf (pUVDtr[uOrder+1], pUVDtr[uOrder]);
                rUVPDtr->DifferenceOf (*rUVPDtr, pUVDtr[1]);
                rUVPDtr->SumOf (*rUVPDtr, *pUVDtr);
                if (rational)
                    {
                    *rUVWDtr = *(wUVDtr+uOrder+1) - *(wUVDtr+uOrder) -
                               *(wUVDtr+1) + *wUVDtr;
                    wUVDtr++;
                    rUVWDtr++;
                    }
                }

            bspprof_hornerSchemeBezier (rUVPtr, rational ? rUVWtr : NULL,
                                        rUVPoDiff, rational ? rUVWgDiff : NULL,
                                        uDegree, u);
            if (rational)
                {
                wgtP += uOrder;
                rUVWtr++;
                }
            }
        bspprof_hornerSchemeBezier (dPdUV, rational ? &tmpUVWgt : NULL,
                                    rUVParPoles,
                                    rational ? rUVParWgts : NULL,
                                    vDegree, v);
        }

    /* Output for all derivatives */
    if (dPdUU)
        {
        if (rational)
            {
            if (uOrder==2)
                {
                dPdUU->x=dPdUU->y=dPdUU->z=0.0;
                }
            else
                {
                dPdUU->SumOf (*dPdUU, *pointP, -1.0 *tmpUUWgt);
                dPdUU->Scale (*dPdUU, tmpWgt*uIndex);
                }
            dPdUU->SumOf (*dPdUU, *pointP, 2.0*uDegree*tmpUWgt*tmpUWgt);
            dPdUU->SumOf (*dPdUU, *dPdU, -2.0*uDegree*tmpUWgt);
            dPdUU->Scale (*dPdUU, (double)uDegree/(tmpWgt*tmpWgt));
            }
        else
            {
            if (uOrder == 2)
                {
                dPdUU->x=dPdUU->y=dPdUU->z=0.0;
                }
            else
                dPdUU->Scale (*dPdUU, (double)uDegree * (double)uIndex);
            }
        }

    if (dPdVV)
        {
        if (rational)
            {
            if (vOrder==2)
                {
                dPdVV->x=dPdVV->y=dPdVV->z=0.0;
                }
            else
                {
                dPdVV->SumOf (*dPdVV, *pointP, -1.0 *tmpVVWgt);
                dPdVV->Scale (*dPdVV, tmpWgt*vIndex);
                }
            dPdVV->SumOf (*dPdVV, *pointP, 2.0*vDegree*tmpVWgt*tmpVWgt);
            dPdVV->SumOf (*dPdVV, *dPdV, -2.0*vDegree*tmpVWgt);
            dPdVV->Scale (*dPdVV, (double)vDegree/(tmpWgt*tmpWgt));
            }
        else
            {
            if (vOrder == 2)
                {
                dPdVV->x=dPdVV->y=dPdVV->z=0.0;
                }
            else
                dPdVV->Scale (*dPdVV, (double)vDegree * (double)vIndex);
            }
        }

    if (dPdUV)
        {
        if (rational)
            {
            dPdUV->SumOf (*dPdUV, *pointP, -1.0 *tmpUVWgt);
            dPdUV->Scale (*dPdUV, tmpWgt);
            dPdUV->SumOf (*dPdUV, *pointP, 2.0*tmpUWgt*tmpVWgt);
            dPdUV->SumOf (*dPdUV, *dPdU, -1.0 *tmpVWgt);
            dPdUV->SumOf (*dPdUV, *dPdV, -1.0 *tmpUWgt);
            dPdUV->Scale (*dPdUV, (double)uDegree*(double)vDegree/(tmpWgt*tmpWgt));
            }
        else
            dPdUV->Scale (*dPdUV, (double)uDegree * (double)vDegree);
        }

    if (dPdU)
        {
        if (rational)
            dPdU->SumOf (*dPdU, *pointP, -1.0 *tmpUWgt);
        dPdU->Scale (*dPdU, rational ?
                                  (double)uDegree/tmpWgt : (double)uDegree);
        }

    if (dPdV)
        {
        if (rational)
            dPdV->SumOf (*dPdV, *pointP, -1.0 *tmpVWgt);
        dPdV->Scale (*dPdV, rational ?
                                  (double)vDegree/tmpWgt : (double)vDegree);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          09/92
+---------------+---------------+---------------+---------------+---------------+------*/
static double  bspprof_distance2dSquare
(
DPoint2d        *p1,
DPoint2d        *p2
)
    {
    double      xdist, ydist;

    xdist = (p2->x - p1->x);
    ydist = (p2->y - p1->y);

    return (xdist * xdist + ydist * ydist);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          09/92
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    bspprof_selectPoints
(
SilhouettePt    *silhPts,       /* three points at most */
int             num
)
    {
    int             i, itmp = 0;
    bool            status;
    double          max, tmp;
    SilhouettePt    sPt;

    if (num < 2 || (num == 2 &&
        bspprof_distance2dSquare (&(silhPts[0].uv), &(silhPts[1].uv)) < fc_epsilon))
        {
        status = false;
        }
    else if (num == 2)
        {
        status = true;
        }
    else
        {
        for (i = 0, max = 0.0; i < num; i++)
            {
            tmp = bspprof_distance2dSquare (&(silhPts[i].uv), &(silhPts[(i+1)%num].uv));
            if (max < tmp)
                {
                max = tmp;
                itmp = i;
                }
            }
        if (itmp == 1)
            {
            sPt = silhPts[1];
            silhPts[1] = silhPts[2];
            silhPts[0] = sPt;
            }
        else if (itmp == 2)
            {
            silhPts[1] = silhPts[2];
            }
        status = true;
        }
    return (status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          09/92
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bspprof_bracketPoint
(
double          *u,
double          *v
)
    {
    if (*u > 1.0)
        *u = 1.0;
    else if (*u < 0.0)
        *u = 0.0;

    if (*v > 1.0)
        *v = 1.0;
    else if (*v < 0.0)
        *v = 0.0;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          05/93
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    bspprof_relaxNewtonCam
(
double              *u1,
double              *v1,
DPoint3d            *pt,
MSBsplineSurface    *patch,
bool                vertical
)
    {
    int             i;
    bool            status = false;
    double          numerator, deno, numer1, numer2, numer3;
    DPoint3d        du, dv, duu, dvv, duv, diff, cross;
    duu.Zero ();
    duv.Zero ();
    dvv.Zero ();
    bspprof_evaluateBezier (pt, &du, &dv,
                            vertical?NULL:&duu, vertical?&dvv:NULL,
                            &duv, *u1, *v1, patch);

    diff.DifferenceOf (*cameraGP, *pt);
    if (diff.Normalize () < fc_tinyVal)
        return (status);

    numer1 = du.y * dv.z - du.z * dv.y;
    numer2 = du.z * dv.x - du.x * dv.z;
    numer3 = du.x * dv.y - du.y * dv.x;
    numerator = numer1 * diff.x + numer2 * diff.y + numer3 * diff.z;

    deno = vertical ?
            (duv.y*dv.z+du.y*dvv.z-duv.z*dv.y-du.z*dvv.y)*diff.x -
            dv.x * numer1 +
            (duv.z*dv.x+du.z*dvv.x-duv.x*dv.z-du.x*dvv.z)*diff.y -
            dv.y * numer2 +
            (duv.x*dv.y+du.x*dvv.y-duv.y*dv.x-du.y*dvv.x)*diff.z -
            dv.z * numer3
            :
            (duu.y*dv.z+du.y*duv.z-duu.z*dv.y-du.z*duv.y)*diff.x -
            du.x * numer1 +
            (duu.z*dv.x+du.z*duv.x-duu.x*dv.z-du.x*duv.z)*diff.y -
            du.y * numer2 +
            (duu.x*dv.y+du.x*duv.y-duu.y*dv.x-du.y*duv.x)*diff.z -
            du.z * numer3;

    /* Newton iteration */
    for (i = 0; i < MAX_ITER; i++)
        {
        if (fabs(deno) < fc_tinyVal)
            break;
        *u1 = vertical ? *u1 : *u1-numerator/deno;
        *v1 = vertical ? *v1-numerator/deno : *v1;
        bspprof_bracketPoint (u1, v1);
        bspprof_evaluateBezier (pt, &du, &dv,
                                vertical?NULL:&duu, vertical?&dvv:NULL,
                                &duv, *u1, *v1, patch);

        diff.DifferenceOf (*cameraGP, *pt);
        if (diff.Normalize () < fc_tinyVal)
            return (status);

        numer1 = du.y * dv.z - du.z * dv.y;
        numer2 = du.z * dv.x - du.x * dv.z;
        numer3 = du.x * dv.y - du.y * dv.x;

        numerator = numer1 * diff.x + numer2 * diff.y + numer3 * diff.z;

        deno = vertical ?
            (duv.y*dv.z+du.y*dvv.z-duv.z*dv.y-du.z*dvv.y)*diff.x -
            dv.x * numer1 +
            (duv.z*dv.x+du.z*dvv.x-duv.x*dv.z-du.x*dvv.z)*diff.y -
            dv.y * numer2 +
            (duv.x*dv.y+du.x*dvv.y-duv.y*dv.x-du.y*dvv.x)*diff.z -
            dv.z * numer3
            :
            (duu.y*dv.z+du.y*duv.z-duu.z*dv.y-du.z*duv.y)*diff.x -
            du.x * numer1 +
            (duu.z*dv.x+du.z*duv.x-duu.x*dv.z-du.x*duv.z)*diff.y -
            du.y * numer2 +
            (duu.x*dv.y+du.x*duv.y-duu.y*dv.x-du.y*duv.x)*diff.z -
            du.z * numer3;

        if (fabs(deno) > fc_tinyVal && fabs(numerator/deno) < fc_newtonTol)
            {
            cross.CrossProduct (du, dv);
            if (cross.Magnitude () > fc_p001)
                {
                status = true;
                break;
                }
            }
        }
    return (status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          09/92
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    bspprof_relaxNewton
(
double              *u1,
double              *v1,
DPoint3d            *pt,
MSBsplineSurface    *patch,
bool                vertical
)
    {
    int             i;
    bool            status = false;
    double          numerator, deno;
    DPoint3d        du, dv, duu, dvv, duv;
    duu.Zero ();
    duv.Zero ();
    dvv.Zero ();
    bspprof_evaluateBezier (pt, &du, &dv,
                            vertical?NULL:&duu, vertical?&dvv:NULL,
                            &duv, *u1, *v1, patch);

    numerator = du.x*dv.y-du.y*dv.x;
    deno = vertical ? dvv.y*du.x+dv.y*duv.x-duv.y*dv.x-du.y*dvv.x :
                      duu.x*dv.y+du.x*duv.y-duv.x*du.y-dv.x*duu.y;

    /* Newton iteration */
    for (i = 0; i < MAX_ITER; i++)
        {
        if (fabs(deno) < fc_tinyVal)
            break;
        *u1 = vertical ? *u1 : *u1 - numerator / deno;
        *v1 = vertical ? *v1 - numerator / deno : *v1;
        bspprof_bracketPoint (u1, v1);
        bspprof_evaluateBezier (pt, &du, &dv,
                                vertical?NULL:&duu, vertical?&dvv:NULL,
                                &duv, *u1, *v1, patch);

        numerator = du.x*dv.y-du.y*dv.x;
        deno = vertical ? dvv.y*du.x+dv.y*duv.x-duv.y*dv.x-du.y*dvv.x :
                          duu.x*dv.y+du.x*duv.y-duv.x*du.y-dv.x*duu.y;

        if (fabs(deno) > fc_tinyVal)
            {
            if (fabs(numerator / deno) < fc_newtonTol &&
                (fabs((du.y*dv.z-du.z*dv.y)/deno) > fc_newtonTol ||
                 fabs((du.z*dv.x-du.x*dv.z)/deno) > fc_newtonTol ))
                {
                status = true;
                break;
                }
            }
        }
    return (status);
    }

#if defined (IMPROVE_SEGMENT)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          09/92
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bspprof_improveSegment
(
SilhouettePt            *ptsList,
int                     *num,
SilhouettePt            *silhPts,
MSBsplineSurface        *patch,
BezierInfo              *infoP
)
    {
    int             tmp;
    bool            vertical;
    double          infoDiffx, infoDiffy,
                    xDiff, yDiff, step, u1, v1, uStart, vStart;
    DPoint3d        pt;
    SilhouettePt    *silhB, *silhE;

    infoDiffx = infoP->silhouette.range.end.x-infoP->silhouette.range.org.x;
    infoDiffy = infoP->silhouette.range.end.y-infoP->silhouette.range.org.y;

    xDiff = silhPts[1].uv.x-silhPts[0].uv.x;
    yDiff = silhPts[1].uv.y-silhPts[0].uv.y;
    if (fabs(xDiff) < fc_tinyVal)
        vertical = false;
    else
        vertical = (fabs(yDiff/xDiff) > 0.5) ? false : true;

    *num = MAX_POINTS;

    tmp = *num-1;
    step = vertical ? (xDiff/tmp) : (yDiff/tmp);

    for (silhB = silhE = ptsList+1, silhE += tmp-1, *num = 1,
         uStart = vertical ? silhPts[0].uv.x+step : 0.5,
         vStart = vertical ? 0.5 : silhPts[0].uv.y+step;
         silhB < silhE; silhB++)
        {
        u1 = uStart;
        v1 = vStart;
        if (!cameraGP)
            {
            if (bspprof_relaxNewton (&u1, &v1, &pt, patch, vertical))
                {
                silhB->uv.x = infoP->silhouette.range.org.x + u1 * infoDiffx;
                silhB->uv.y = infoP->silhouette.range.org.y + v1 * infoDiffy;
                silhB->xyz = pt;
                (*num)++;
                }
            }
        else
            {
            if (bspprof_relaxNewtonCam (&u1, &v1, &pt, patch, vertical))
                {
                silhB->uv.x = infoP->silhouette.range.org.x + u1 * infoDiffx;
                silhB->uv.y = infoP->silhouette.range.org.y + v1 * infoDiffy;
                silhB->xyz = pt;
                (*num)++;
                }
            }

        if (vertical)
            uStart += step;
        else
            vStart += step;
        }

    ptsList[0].xyz = silhPts[0].xyz;
    ptsList[0].uv.x = infoP->silhouette.range.org.x+silhPts[0].uv.x*infoDiffx;
    ptsList[0].uv.y = infoP->silhouette.range.org.y+silhPts[0].uv.y*infoDiffy;
    ptsList[*num].xyz = silhPts[1].xyz;
    ptsList[*num].uv.x = infoP->silhouette.range.org.x+silhPts[1].uv.x*infoDiffx;
    ptsList[(*num)++].uv.y = infoP->silhouette.range.org.y+silhPts[1].uv.y*infoDiffy;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          10/92
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    bspprof_isBoxSmall
(
BoundBox        *box,
double          tolerance
)
    {
     if (box->extent.z <= tolerance ||
        box->extent.y <= tolerance ||
        box->extent.x <= tolerance)
        return (true);

    return (false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          10/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspprof_fitPoints
(
MSBsplineCurve      *curve,
DPoint3d            *points,
int                 numPoints
)
    {
    int             i, status;

    if (numPoints < 2)
        {
        return ERROR;
        }
    else
        {
        curve->rational = false;
        curve->display.curveDisplay = true;
        curve->params.order = 2;
        curve->params.closed = false;
        curve->params.numPoles = numPoints;
        curve->params.numKnots = 0;

        if (status = bspcurv_allocateCurve (curve))
            return (status);

        bspknot_computeKnotVector (curve->knots, &(curve->params), NULL);

        for (i=0; i<numPoints; i++)
            curve->poles[i]=points[i];

        return SUCCESS;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             06/91
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspprof_getBoundaryIntersections
(
BoundIntersect      *boundIntersectP,
DRange2dP           uvSegmentP,
DSegment3d           *xyzSegmentP,
MSBsplineSurface    *surfP
)
    {
    int             intersect0, intersect1;
    double          segmentDistance, distance0, distance1;
    DPoint2d        *pntP, *endP, delta;
    BsurfBoundary   *boundP, *endB;
    BoundIntersect  *biP = boundIntersectP;

    delta.x = uvSegmentP->high.x - uvSegmentP->low.x;
    delta.y = uvSegmentP->high.y - uvSegmentP->low.y;
    segmentDistance = sqrt (delta.x * delta.x  + delta.y * delta.y);
    for (boundP = surfP->boundaries, endB = boundP + surfP->numBounds;
            boundP < endB;
                boundP++)
        {
        for (pntP = boundP->points, endP = pntP + boundP->numPoints - 1;
                pntP < endP;
                    pntP++)
            {
            bsputil_xySegmentIntersection (&intersect0, &intersect1,
                                           &distance0, &distance1,
                                           uvSegmentP, (DRange2dP) pntP, 1.0E-10);
            if (intersect0 & (INTERSECT_SPAN | INTERSECT_END) &&
                intersect1 & (INTERSECT_SPAN | INTERSECT_END))
                {
                if (boundIntersectP)
                    {
                    biP->distance = distance0 / segmentDistance;

                    biP->point.Scale (*(&xyzSegmentP->point[0]), 1.0 - biP->distance);
                    biP->point.SumOf (biP->point, *(&xyzSegmentP->point[1]), biP->distance);
                    }
                biP++;
                }
            }
        }
#ifdef TODO_EliminateAddressArithmetic
#endif
    return (int)(biP - boundIntersectP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/93
+---------------+---------------+---------------+---------------+---------------+------*/
static void bspprof_storeSilhouettePoints
(
SilhouettePt        *pointP,
int                 numPoints,
MSBsplineSurface    *patchP,
DRange2dCP          rangeP
)
    {
    int             nIsPnts;
    bool            inside;
    DRange2d        uvSegment;
    DSegment3d      xyzSegment;
    SilhouettePt    *endP;
    BoundIntersect  isPnts[MAX_INTERSECTIONS], *biP, *biPEnd;
    bvector<DPoint3d> points;
    if (patchP->numBounds)
        {
        uvSegment.high.x = rangeP->low.x + pointP[0].uv.x * (rangeP->high.x - rangeP->low.x);
        uvSegment.high.y = rangeP->low.y + pointP[0].uv.y * (rangeP->high.y - rangeP->low.y);
        inside = bsputil_pointOnSurface (&uvSegment.high, patchP);
        for (endP=pointP + numPoints - 1; pointP<endP; pointP++)
            {
            uvSegment.low = uvSegment.high;
            uvSegment.high.x = rangeP->low.x +
                              pointP[1].uv.x * (rangeP->high.x - rangeP->low.x);
            uvSegment.high.y = rangeP->low.y +
                              pointP[1].uv.y * (rangeP->high.y - rangeP->low.y);
            xyzSegment.point[0] = pointP[0].xyz;
            xyzSegment.point[1] = pointP[1].xyz;

            nIsPnts = bspprof_getBoundaryIntersections (isPnts, &uvSegment,
                                                       &xyzSegment, patchP);

            mdlUtil_dlmQuickSort (isPnts, isPnts + nIsPnts - 1, sizeof(BoundIntersect),
                            (PFToolsSortCompare)bspssi_compareIntersections, NULL);

            if (inside)
                points.push_back (pointP->xyz);

            for (biP=isPnts, biPEnd = biP + nIsPnts; biP < biPEnd; biP++)
                {
                points.push_back (biP->point);

                if (! (inside = !inside))
                    bspssi_appendPointList_clearInput (listGP, &numListGP, points);
                }
            }
        if (inside)
            points.push_back (pointP->xyz);
        }
    else
        {
        for (endP = pointP + numPoints; pointP<endP; pointP++)
            points.push_back (pointP->xyz);
        }
    if (points.size () > 0)
        bspssi_appendPointList_clearInput (listGP, &numListGP, points);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          10/92
+---------------+---------------+---------------+---------------+---------------+------*/
static double magnitudeSquared
(
DPoint3d    *vec
)
    {
    return  (vec->x * vec->x + vec->y * vec->y + vec->z * vec->z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          10/92
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    threePointsFlat
(
DPoint3d    *p0,
DPoint3d    *p1,
DPoint3d    *p2,
double      tol
)
    {
    double      d0, d1;
    DPoint3d    mid, diff;

    mid.SumOf (*p0, *p2);
    mid.Scale (mid, 0.5);
    diff.DifferenceOf (mid, *p1);
    d1 = magnitudeSquared (&diff);
    diff.DifferenceOf (*p0, *p2);
    d0 = magnitudeSquared (&diff);
    return (d1 > d0 * tol * 0.25) ? false : true;
    }

/*---------------------------------------------------------------------------------**//**
* p2 --- p3 | | p0 --- p1
* @bsimethod                                                    Lu.Han          10/92
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    fourPointsFlat
(
DPoint3d    *p0,
DPoint3d    *p1,
DPoint3d    *p2,
DPoint3d    *p3,
double      tol
)
    {
    double          d, d0, d1;
    DPoint3d        mid0, mid1, diff;

    mid0.SumOf (*p0, *p3);
    mid0.Scale (mid0, 0.5);
    mid1.SumOf (*p1, *p2);
    mid1.Scale (mid1, 0.5);
    diff.DifferenceOf (mid0, mid1);
    d = magnitudeSquared (&diff);
    diff.DifferenceOf (*p0, *p3);
    d0 = magnitudeSquared (&diff);
    diff.DifferenceOf (*p1, *p2);
    d1 = magnitudeSquared (&diff);
    if (d1 < d0)
        d0 = d1;

    return (d > 0.25 * tol * d0) ? false : true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          11/98
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    bspprof_isFlatBezPolygon
(
MSBsplineSurface    *pBez,
double              angleTolRadians
)
    {
    bool            isFlat = true;
    int             i, j, numU, numV;
    double          factor, factor0;
    DPoint3d        *pP, *pPole;

    factor0 = tan (angleTolRadians);

    numU = pBez->uParams.order;
    numV = pBez->vParams.order;
    if (pBez->rational)
        bsputil_unWeightPoles (pBez->poles, pBez->poles, pBez->weights, numU * numV);

    if (numU > 2)
        {
        /* Check u direction poles */
        factor = factor0 / (double) (numU - 2);
        factor *= factor;
        for (i = 0; i < numV; i++)
            {
            pP = pBez->poles + i * numU;
            for (j = 0, pPole = pP; j < numU-2; j++, pPole++)
                {
                if (!threePointsFlat (pPole, pPole+1, pPole+2, factor))
                    {
                    isFlat = false;
                    goto wrapup;
                    }
                }
            }
        }

    if (numV > 2)
        {
        /* Check v direction poles */
        factor = factor0 / (double) (numV - 2);
        factor *= factor;
        for (i = 0; i < numU; i++)
            {
            pP = pBez->poles + i;
            for (j = 0, pPole = pP; j < numV-2; j++)
                {
                if (!threePointsFlat (pPole, pPole+numU, pPole+2*numU, factor))
                    {
                    isFlat = false;
                    goto wrapup;
                    }
                pPole += numU;
                }
            }
        }
    /* Check each quadrilateral */
    /* Can't drop out for linear case because the quad can still twist */

    factor = factor0 / ((numU > numV) ? (double) (numU - 1) : (double) (numV - 1));
    factor *= factor;
    for (i = 0; i < numV-1; i++)
        {
        pP = pBez->poles + i * numU;
        for (j = 0, pPole = pP; j < numU-1; j++, pPole++)
            {
            if (!fourPointsFlat (pPole, pPole+1, pPole+numU, pPole+numU+1, factor))
                {
                isFlat = false;
                goto wrapup;
                }
            }
        }

wrapup:
    if (pBez->rational)
        bsputil_weightPoles (pBez->poles, pBez->poles, pBez->weights, numU * numV);

    return  isFlat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          01/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int      bspprof_silhouetteStopBez
(
BezierInfo          *infoP,
MSBsplineSurface    *patch
)
    {
    int             nPoints;
    bool            status = false;
    double          u0[4], v0[4], *uB, *uE, *vB;
    DPoint3d        pt;
    BoundBox        box;
    SilhouettePt    silhPts[3];
    static double   angleTolRadians = 0.10; /* About 5 degrees */

    bound_boxFromSurface (&box, patch);

    /* Flatness testing */
    if (subdivide && (bspprof_isBoxSmall (&box, flatTol) || bspprof_isFlatBezPolygon (patch, angleTolRadians)))
        {
        /* Assign initial guess for Newton iteration on four boundaries */
        u0[0] = u0[1] = v0[2] = v0[3] = 0.5;
        v0[0] = u0[2] = 0.0;
        v0[1] = u0[3] = 1.0;

        /* Find silhouette points from boundaries */
        for (uB = uE = u0, vB = v0, uE += 4, nPoints = 0;
             uB < uE && nPoints < 3; uB++, vB++)
            {
            if (!cameraGP)
                {
                if (bspprof_relaxNewton (uB, vB, &pt, patch,
                                     (fabs(*uB - 0.5) < fc_tinyVal) ?
                                      false : true))
                    {
                    silhPts[nPoints].uv.x = *uB;
                    silhPts[nPoints].uv.y = *vB;
                    silhPts[nPoints++].xyz=pt;
                    }
                }
            else
                {
                if (bspprof_relaxNewtonCam (uB, vB, &pt, patch,
                                     (fabs(*uB - 0.5) < fc_tinyVal) ?
                                      false : true))
                    {
                    silhPts[nPoints].uv.x = *uB;
                    silhPts[nPoints].uv.y = *vB;
                    silhPts[nPoints++].xyz=pt;
                    }
                }
            }

        /* Select only two different silhPts and improve the middle */
        if (bspprof_selectPoints (silhPts, nPoints))
            bspprof_storeSilhouettePoints (silhPts, nPoints,
                                           patch, &infoP->silhouette.range);

        status = true;
        }

    /* This means that always subdivide the patch once */
    subdivide = true;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     02/92
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    bspprof_computeProductInterval
(
double  *min,
double  *max,
double  min1,
double  max1,
double  min2,
double  max2
)
    {
    double  test;

    *min = *max = min1 * min2;

    test = min1 * max2;
    if (test < *min) *min = test;
    if (test > *max) *max = test;

    test = max1 * min2;
    if (test < *min) *min = test;
    if (test > *max) *max = test;

    test = max1 * max2;
    if (test < *min) *min = test;
    if (test > *max) *max = test;

    return (true);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          05/93
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bspprof_pointToSphereMinMax
(
DPoint3d        *ptMin,
DPoint3d        *ptMax,
DPoint3d        *tstPt,
DPoint3d        *center,
double          radius
)
    {
    DPoint3d        dist;

    dist.DifferenceOf (*tstPt, *center);

    ptMin->x = dist.x - radius;
    ptMin->y = dist.y - radius;
    ptMin->z = dist.z - radius;

    ptMax->x = dist.x + radius;
    ptMax->y = dist.y + radius;
    ptMax->z = dist.z + radius;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          05/93
+---------------+---------------+---------------+---------------+---------------+------*/
static int      bspprof_silhouetteGoBezCam
(
BezierInfo          *infoP,           /* =>                         */
MSBsplineSurface    *patch,           /* =>                         */
int                 sub
)
    {
    int             i, j, total;
    bool            first, status=false;
    double          dxduMin = 0, dxduMax = 0, dxdvMin = 0, dxdvMax = 0, dyduMin = 0, dyduMax = 0,
                    dydvMin = 0, dydvMax = 0, dzduMin = 0, dzduMax = 0, dzdvMin = 0, dzdvMax = 0,
                    dxdu, dydu, dzdu, dxdv, dydv, dzdv, tolerance,
                    xTerm1Min, xTerm1Max, xTerm2Min, xTerm2Max,
                    yTerm1Min, yTerm1Max, yTerm2Min, yTerm2Max,
                    zTerm1Min, zTerm1Max, zTerm2Min, zTerm2Max,
                    radius;

    DPoint3d        *poleP, *firstPoleP, center, ptMin, ptMax;

    /* Use bounding sphere to approx the min/max dist from camera to patch */
    bound_sphereFromSurface (&center, &radius, patch);
    bspprof_pointToSphereMinMax (&ptMin, &ptMax, cameraGP, &center, radius);

    total = patch->uParams.order * patch->vParams.order;
    if (patch->rational)
        bsputil_unWeightPoles (patch->poles, patch->poles, patch->weights, total);

    firstPoleP = patch->poles;

    first = true;
    for (j=0; j < patch->vParams.numPoles; j++ )
        {
        for (i = 1, poleP = firstPoleP + j * patch->uParams.numPoles;
                i < patch->uParams.numPoles;
                    i++, poleP++)
            {
            dxdu = (poleP+1)->x - poleP->x;
            dydu = (poleP+1)->y - poleP->y;
            dzdu = (poleP+1)->z - poleP->z;
            if (first)
                {
                first = false;
                dxduMin = dxduMax = dxdu;
                dyduMin = dyduMax = dydu;
                dzduMin = dzduMax = dzdu;
                }
            else
                {
                if (dxdu < dxduMin) dxduMin = dxdu;
                if (dxdu > dxduMax) dxduMax = dxdu;
                if (dydu < dyduMin) dyduMin = dydu;
                if (dydu > dyduMax) dyduMax = dydu;
                if (dzdu < dzduMin) dzduMin = dzdu;
                if (dzdu > dzduMax) dzduMax = dzdu;
                }
            }
        }


    first = true;
    for (i=0; i < patch->uParams.numPoles; i++ )
        {
        for (j = 0, poleP = firstPoleP+i;
                j < patch->vParams.numPoles - 1;
                    j++, poleP = poleP + patch->uParams.numPoles)
            {
            dxdv = (poleP+patch->uParams.numPoles)->x - poleP->x;
            dydv = (poleP+patch->uParams.numPoles)->y - poleP->y;
            dzdv = (poleP+patch->uParams.numPoles)->z - poleP->z;
            if (first)
                {
                first = false;
                dxdvMin = dxdvMax = dxdv;
                dydvMin = dydvMax = dydv;
                dzdvMin = dzdvMax = dzdv;
                }
            else
                {
                if (dxdv < dxdvMin) dxdvMin = dxdv;
                if (dxdv > dxdvMax) dxdvMax = dxdv;
                if (dydv < dydvMin) dydvMin = dydv;
                if (dydv > dydvMax) dydvMax = dydv;
                if (dzdv < dzdvMin) dzdvMin = dzdv;
                if (dzdv > dzdvMax) dzdvMax = dzdv;
                }
            }
        }

    if (patch->rational)
        bsputil_weightPoles (patch->poles, patch->poles, patch->weights, total);

    /* Min/Max for the two terms in cross product */
    bspprof_computeProductInterval (&xTerm1Min, &xTerm1Max,
                                    dyduMin, dyduMax, dzdvMin, dzdvMax);
    bspprof_computeProductInterval (&xTerm2Min, &xTerm2Max,
                                    dzduMin, dzduMax, dydvMin, dydvMax);
    bspprof_computeProductInterval (&yTerm1Min, &yTerm1Max,
                                    dzduMin, dzduMax, dxdvMin, dxdvMax);
    bspprof_computeProductInterval (&yTerm2Min, &yTerm2Max,
                                    dxduMin, dxduMax, dzdvMin, dzdvMax);
    bspprof_computeProductInterval (&zTerm1Min, &zTerm1Max,
                                    dxduMin, dxduMax, dydvMin, dydvMax);
    bspprof_computeProductInterval (&zTerm2Min, &zTerm2Max,
                                    dyduMin, dyduMax, dxdvMin, dxdvMax);

    /* Min/Max for the subtraction btw the two terms in cross product */
    xTerm1Min = xTerm1Min - xTerm2Max;
    xTerm1Max = xTerm1Max - xTerm2Min;
    yTerm1Min = yTerm1Min - yTerm2Max;
    yTerm1Max = yTerm1Max - yTerm2Min;
    zTerm1Min = zTerm1Min - zTerm2Max;
    zTerm1Max = zTerm1Max - zTerm2Min;

    /* Min/Max for the dot product */
    bspprof_computeProductInterval (&xTerm1Min, &xTerm1Max,
                                    xTerm1Min, xTerm1Max, ptMin.x, ptMax.x);
    bspprof_computeProductInterval (&yTerm1Min, &yTerm1Max,
                                    yTerm1Min, yTerm1Max, ptMin.y, ptMax.y);
    bspprof_computeProductInterval (&zTerm1Min, &zTerm1Max,
                                    zTerm1Min, zTerm1Max, ptMin.z, ptMax.z);

    tolerance = DoubleOps::MaxAbs (xTerm1Min, xTerm1Max,
                                 yTerm1Min, yTerm1Max,
                                 zTerm1Min, zTerm1Max) * TOLERANCE_SilhouetteTerms;

    if (fabs (xTerm1Min) < fc_epsilon && fabs (xTerm1Max) < fc_epsilon &&
        fabs (yTerm1Min) < fc_epsilon && fabs (yTerm1Max) < fc_epsilon &&
        fabs (zTerm1Min) < fc_epsilon && fabs (zTerm1Max) < fc_epsilon)
        return  false;

    if ((xTerm1Min + yTerm1Min + zTerm1Min) <= tolerance &&
        (xTerm1Max + yTerm1Max + zTerm1Max) >= -tolerance)
        {
        bsputil_selectQuarterPatch (&infoP->silhouette.range, sub,
                                   &infoP->silhouette.range);
        status=true;
        }

    return (status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          01/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int          bspprof_silhouetteGoBez
(
BezierInfo          *infoP,           /* =>                         */
MSBsplineSurface    *patch,           /* =>                         */
int                 sub
)
    {
    int             i, j, total;
    bool            first, status=false;
    double          dxduMin = 0, dxduMax = 0, dxdvMin = 0, dxdvMax = 0, dyduMin = 0, dyduMax = 0,
                    dydvMin = 0, dydvMax = 0, dxdu, dydu, dxdv, dydv,
                    term1Min, term1Max, term2Min, term2Max, tolerance;
    DPoint3d        *poleP, *firstPoleP;

#if defined (DEBUG_S)
    {
    bool        debug_on = false;

    if (debug_on == true)
        {
        patch->display.curveDisplay = true;
        patch->display.polygonDisplay = true;
        debug_displaySurface (patch, HILITE);
        debug_displaySurface (patch, ERASE);
        }
    }
#endif

    total = patch->uParams.order * patch->vParams.order;
    if (patch->rational)
        bsputil_unWeightPoles (patch->poles, patch->poles, patch->weights, total);

    firstPoleP = patch->poles;

    first = true;
    for (j=0; j < patch->vParams.numPoles; j++ )
        {
        for (i = 1, poleP = firstPoleP + j * patch->uParams.numPoles;
                i < patch->uParams.numPoles;
                    i++, poleP++)
            {
            dxdu = (poleP+1)->x - poleP->x;
            dydu = (poleP+1)->y - poleP->y;
            if (first)
                {
                first = false;
                dxduMin = dxduMax = dxdu;
                dyduMin = dyduMax = dydu;
                }
            else
                {
                if (dxdu < dxduMin) dxduMin = dxdu;
                if (dxdu > dxduMax) dxduMax = dxdu;
                if (dydu < dyduMin) dyduMin = dydu;
                if (dydu > dyduMax) dyduMax = dydu;
                }
            }
        }

    first = true;
    for (i=0; i < patch->uParams.numPoles; i++ )
        {
        for (j = 0, poleP = firstPoleP+i;
                j < patch->vParams.numPoles - 1;
                    j++, poleP = poleP + patch->uParams.numPoles)
            {
            dxdv = (poleP+patch->uParams.numPoles)->x - poleP->x;
            dydv = (poleP+patch->uParams.numPoles)->y - poleP->y;
            if (first)
                {
                first = false;
                dxdvMin = dxdvMax = dxdv;
                dydvMin = dydvMax = dydv;
                }
            else
                {
                if (dxdv < dxdvMin) dxdvMin = dxdv;
                if (dxdv > dxdvMax) dxdvMax = dxdv;
                if (dydv < dydvMin) dydvMin = dydv;
                if (dydv > dydvMax) dydvMax = dydv;
                }
            }
        }
    if (patch->rational)
        bsputil_weightPoles (patch->poles, patch->poles, patch->weights, total);

    bspprof_computeProductInterval (&term1Min, &term1Max,
                                    dxduMin, dxduMax, dydvMin, dydvMax);
    bspprof_computeProductInterval (&term2Min, &term2Max,
                                    dxdvMin, dxdvMax, dyduMin, dyduMax);

    tolerance = DoubleOps::MaxAbs (term1Min, term1Max, term2Min, term2Max)
                              * TOLERANCE_SilhouetteTerms;

    if (fabs (term1Min) < fc_epsilon && fabs (term1Max) < fc_epsilon &&
        fabs (term2Min) < fc_epsilon && fabs (term2Max) < fc_epsilon)
        return  false;

#if defined (ORIGINAL_TEST_FOR_ABSOLUTE_NONE_ZERO)
    if (term1Min-term2Max <= tolerance &&
        term1Max-term2Min >= -tolerance)
        {
        bsputil_selectQuarterPatch (&infoP->silhouette.range, sub,
                                   &infoP->silhouette.range);
        status=true;
        }
#elif defined (SECOND_TEST_FOR_ABSOLUTE_NONZERO)
    if (!((term1Min > tolerance && term2Max < -tolerance) ||
          (term1Max < -tolerance && term2Min > tolerance)))
        {
        bsputil_selectQuarterPatch (&infoP->silhouette.range, sub,
                                   &infoP->silhouette.range);
        status=true;
        }
#else
        {
        double maxDiff, minDiff;
        minDiff = term1Min - term2Max;
        maxDiff = term1Max - term2Min;
        if (minDiff > tolerance || maxDiff < -tolerance)
            {
            status = false;
            }
        else
            {
            bsputil_selectQuarterPatch (&infoP->silhouette.range, sub,
                                   &infoP->silhouette.range);
            status = true;
            }
        }
#endif

    return (status);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          01/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int          bspprof_silhouetteBezier
(
BezierInfo          *infoP,
MSBsplineSurface    *patch
)
    {
    /* For each Bezier patch, subdivide at least once */
    subdivide = false;

#if defined (DEBUG_S)
    {
    bool    debug_on;

    if (debug_on == true)
        {
        patch->display.curveDisplay = true;
        patch->display.polygonDisplay = true;
        debug_displaySurface (patch, HILITE);
        debug_displaySurface (patch, ERASE);
        }
    }
#endif

    bound_boxFromSurface (&infoP->silhouette.box, patch);

    return bspproc_processBezierPatch (infoP, patch,
                                       bspprof_silhouetteStopBez,
                                       NULLFUNC, cameraGP ?
                                       bspprof_silhouetteGoBezCam :
                                       bspprof_silhouetteGoBez, NULLFUNC);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    02/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspprof_silhouetteSort
(
int                 *rank,
BezierInfo          *infoP,
int                 *uStart,
int                 *vStart,
int                 uNumSegs,
int                 vNumSegs,
MSBsplineSurface    *surface
)
    {
    int             i, *rP, total;

    infoP->silhouette.uKts = surface->uKnots;
    infoP->silhouette.vKts = surface->vKnots;

    total = uNumSegs * vNumSegs;
    for (i=0, rP=rank; i < total; i++, rP++)
        *rP = i;

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          01/92
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspprof_silhouetteGo
(
BezierInfo          *infoP,
MSBsplineSurface    *bez,
int                 uSeg,
int                 vSeg,
int                 uStart,         /* => knot offset of bez in U */
int                 vStart,         /* => knot offset of bez in V */
int                 numU,
int                 numV
)
    {

    infoP->silhouette.range.low.x =
        bspproc_setParameter (0.0, uStart, infoP->silhouette.uKts);

    infoP->silhouette.range.low.y =
        bspproc_setParameter (0.0, vStart, infoP->silhouette.vKts);

    infoP->silhouette.range.high.x =
        bspproc_setParameter (1.0, uStart, infoP->silhouette.uKts);

    infoP->silhouette.range.high.y =
        bspproc_setParameter (1.0, vStart, infoP->silhouette.vKts);

    return (true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          04/93
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspprof_reverseList
(
PointList       *listP
)
    {
    int         i, status = SUCCESS, allocSize, num;
    PointList   copyList;

    num = listP->numPoints-1;

    allocSize = listP->numPoints * sizeof(DPoint3d);
    if (! (copyList.points = (DPoint3d*)msbspline_malloc (allocSize, 'BSSL')))
        {
        status = ERROR;
        goto wrapup;
        }

    memcpy (copyList.points, listP->points, allocSize);

    for (i = 0; i < listP->numPoints; i++)
        listP->points[i] = copyList.points[num-i];

wrapup:
    if (copyList.points) msbspline_free (copyList.points);

    return (status);
    }

/*---------------------------------------------------------------------------------**//**
* Append inList to outList, dropping the "first" point of inlist ("first" is point 0 after reversal)
* @bsimethod                                                    Lu.Han          04/93
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspprof_joinList
(
PointList       *outList,
PointList       *inList,
int             appendCode
)
    {
    int         status = SUCCESS, allocSize;
    int numAddedPoint;

    if (appendCode == APPEND_ORG_ORG)
        {
        if (status = bspprof_reverseList (outList))
            return status;
        }
    else if (appendCode == APPEND_ORG_END)
        {
        if (status = (bspprof_reverseList (outList) ||
                      bspprof_reverseList (inList)))
            return status;
        }
    else if (appendCode == APPEND_END_END)
        {
        if (status = bspprof_reverseList (inList))
            return status;
        }

    numAddedPoint = inList->numPoints - 1;
    allocSize = (outList->numPoints + numAddedPoint) * sizeof(DPoint3d);

    if (outList->points = (DPoint3d*)msbspline_realloc (outList->points, allocSize))
        {
        memcpy (outList->points + outList->numPoints,
                inList->points + 1,
                numAddedPoint * sizeof(DPoint3d));

        outList->numPoints += numAddedPoint;

        inList->numPoints = 0;
        msbspline_free (inList->points);
        return SUCCESS;
        }
    else
        {
        return ERROR;
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          04/93
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    bspprof_combineList
(
PointList       *outList,
PointList       *inList,
double          closureTol
)
    {
    int         appendCode, numIn, numOut;

    appendCode = APPEND_NONE;
    numIn = inList->numPoints-1;
    numOut = outList->numPoints-1;

    if (outList->points->Distance (*(inList->points)) < closureTol)
        appendCode = APPEND_ORG_ORG;
    else if (outList->points->Distance (*(inList->points+numIn)) < closureTol)
        appendCode = APPEND_ORG_END;
    else if ((outList->points+numOut)->Distance (*(inList->points)) < closureTol)
        appendCode = APPEND_END_ORG;
    else if ((outList->points+numOut)->Distance (*(inList->points+numIn)) < closureTol)
        appendCode = APPEND_END_END;

    if (!appendCode)
        return (false);
    else
        {
        if (bspprof_joinList (outList, inList, appendCode))
            return (false);
        }

    return (true);
    }
 #ifdef CompileAll
/*---------------------------------------------------------------------------------**//**
* Copied from acis\dlm\mdlmisc.c by LuHan & EarlinLutz 09/95 (And modified to return just a flag instead of arrays)
* @bsimethod                                                    Ray.Bentley     04/93
+---------------+---------------+---------------+---------------+---------------+------*/
static int     curveIsC1
(
MSBsplineCurve  *curveP
)
    {
    int         i, nKnots, numDistinct, *knotMultiplicityP;
    double      *distinctKnotP, leftValue, rightValue;
    DPoint3d    leftTangent, rightTangent;
    int         smooth = true;

    nKnots = bspknot_numberKnots (curveP->params.numPoles,
                                     curveP->params.order, curveP->params.closed);

    if (NULL == (distinctKnotP =
        (double *) BSIBaseGeom::Malloc (nKnots * sizeof(double))) ||
        NULL == (knotMultiplicityP =
        (int *)    BSIBaseGeom::Malloc (nKnots * sizeof(int))))
        return smooth;

    bspknot_getKnotMultiplicity (distinctKnotP, knotMultiplicityP, &numDistinct,
                                    curveP->knots, curveP->params.numPoles,
                                    curveP->params.order, curveP->params.closed,
                                    KNOT_TOLERANCE_BASIS);

    for (i = 0; i<numDistinct && smooth; i++)
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
                bspcurv_evaluateCurvePoint (NULL, &leftTangent, curveP, leftValue);
                bspcurv_evaluateCurvePoint (NULL, &rightTangent, curveP, rightValue);

                if (leftTangent.Normalize () < TOLERANCE_TangentMagnitude ||
                    rightTangent.Normalize () < TOLERANCE_TangentMagnitude ||
                    leftTangent.DotProduct (rightTangent) < COSINE_TOLERANCE)
                    smooth = false;
                }
            }

    BSIBaseGeom::Free (distinctKnotP);
    BSIBaseGeom::Free (knotMultiplicityP);

    return smooth;
    }
#endif
#ifdef CompileAll
/*---------------------------------------------------------------------------------**//**
* Copied from acis\dlm\mdlmisc.c by LuHan & EarlinLutz 09/95 (And modified to return just a count instead of arrays)
* @bsimethod                                                    Ray.Bentley     04/93
+---------------+---------------+---------------+---------------+---------------+------*/
static int     surfaceIsC1
(
MSBsplineSurface    *surfP,
int                 direction,          /* BSSURF_U or BSPCURV_V */
double              tolerance
)
    {
    int             i, nCurve,status;
    int             smooth = true;
    MSBsplineCurve  curve;


    nCurve = direction == BSSURF_U ? surfP->vParams.numPoles : surfP->uParams.numPoles;

    for (i = 0; smooth && i < nCurve; i++)
        {
        if (SUCCESS !=
            (status = bspconv_getCurveFromSurface (&curve, surfP, direction, i)))
            break;

        if (bspcurv_polygonLength (&curve) > tolerance
                && !curveIsC1 (&curve))
            {
            smooth = false;
            }
        bspcurv_freeCurve (&curve);
        }

    return smooth;
    }
#endif
/*---------------------------------------------------------------------------------**//**
* Test if the middle point of three successive points on a silhouette could be a cusp, i.e. the curve should break here.
* In flat-projection views, projection is along the z direction. Call it a cusp if the two successive xy vectors have a largish angle between
* them.
* In a camera, we would like to know the plane the curve projects to to do the same xy analysis. Unfortunately we only know the camera point.
* So... project all points back to the unit circle and use the (spatial) angle between successive displacement vectors on the unit circle.
* Remark: In the perspective case, all three points go through a normalization each call.
* @bsimethod                                                    Earlin.Lutz     02/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool        bspprof_possibleCusp
(
const   DPoint3d    *point0P,
const   DPoint3d    *point1P,
const   DPoint3d    *point2P,
const   DPoint3d    *cameraPosP
)
    {
    DPoint3d U, V;
    static double s_parallelAngleTol = 0.2;
    static double s_perspectiveAngleTol = 0.2;
    double angle;
    bool    possibleCusp = false;

    if (cameraPosP)
        {
        DPoint3d unit0, unit1, unit2;
        unit0.NormalizedDifference (*point0P, *cameraPosP);
        unit1.NormalizedDifference (*point1P, *cameraPosP);
        unit2.NormalizedDifference (*point2P, *cameraPosP);
        U.DifferenceOf (unit1, unit0);
        V.DifferenceOf (unit2, unit1);
        angle = bsiDPoint3d_angleBetweenVectors (&U, &V);
        if (fabs (angle) >= s_perspectiveAngleTol)
            {
            possibleCusp = true;
            }
        }
    else
        {
        U.DifferenceOf (*point1P, *point0P);
        V.DifferenceOf (*point2P, *point1P);
        angle = bsiDPoint3d_angleBetweenVectorsXY (&U, &V);

        if (fabs (angle) >= s_parallelAngleTol)
            {
            possibleCusp = true;
            }
        }

    return possibleCusp;
    }

/*---------------------------------------------------------------------------------**//**
* Scan forward from index i0 for index i1 which is either (a) the end of the array or (b) a point of sharp turn (possible cusp).
* @bsimethod                                                    Earlin.Lutz     02/99
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    bspprof_scanForSubcurveWithNoCusps
(
      int       *i1P,           /* <= index of last point in subcurve. */
      int       i0,             /* => start point of search */
const DPoint3d  *pPointArray,
      int       numPoint,
const DPoint3d  *cameraPosP
)
    {
    int i;
    if (i0 >= numPoint - 1)
        return false;
    if (i0 == numPoint - 2)
        {
        *i1P = numPoint - 1;
        return true;
        }

    for (i = i0 + 2; i < numPoint; i++)
        {
        if (bspprof_possibleCusp (
                    pPointArray + i - 2,
                    pPointArray + i - 1,
                    pPointArray + i,
                    cameraPosP))
            {
            *i1P = i - 1;
            return true;
            }
        }

    *i1P = numPoint - 1;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Lu.Han          02/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP  int     bspprof_extractSilhouette
(
bvector<MSBsplineCurvePtr>  &curves,            /* <= silhoutte curves */
MSBsplineSurfaceCP surfaceP,          /* => surface */
double                  tolerance,          /* => tolerance */
bool                    cubicFit,           /* => T: cubic, F: linear. */
DPoint3d                *cameraPos          /* => camera position or NULL */
)
    {
    int                 i, j, status;
    double              mag;
    DPoint3d            tol3d;
    BezierInfo          infoP;
    BoundBox            box;
    PointList           *listP;
    MSBsplineCurve      silhCurve;

    static int s_splitCusps = 1;

    bound_boxFromSurface (&box, const_cast <MSBsplineSurfaceP> (surfaceP));

    curves.clear ();

    /* If tolerance is 1/3 the size of bound box, no silhouette returned */
    if ((mag  = box.extent.Magnitude ()) <= tolerance * fc_3)
        return (true);

    memset (&silhCurve, 0, sizeof(silhCurve));

    listP = NULL;
    numListGP = 0;
    listGP = &listP;

    if (cameraPos)
        cameraGP = cameraPos;
    else
        cameraGP = NULL;

    flatTol = tolerance;
    tol3d.x = tol3d.y = tol3d.z = tolerance;

    status = bspproc_processBsplineSurface (&infoP,
                                            const_cast <MSBsplineSurfaceP> (surfaceP),
                                            bspprof_silhouetteSort,
                                            bspprof_silhouetteGo,
                                            bspprof_silhouetteBezier,
                                            NULLFUNC, NULLFUNC);

    /* Assemble listP to combine adjoint curves */
    for (i = 0; i < numListGP; i++)
        for (j = i+1; j < numListGP; j++)
            if (bspprof_combineList (listP+j, listP+i, tolerance/10.0))
                break;

    /* Search within each linestring for internal points that do not look smooth.
        Generate a curve for each smooth section.
    */
    for (i = 0; i < numListGP; i++)
        {
        int i0, i1;
        int num01;
        DPoint3d *basePointP;
        if (listP[i].numPoints)
            {
            for (i0 = 0;
                 bspprof_scanForSubcurveWithNoCusps
                            (&i1, i0, listP[i].points, listP[i].numPoints, cameraPos);
                 i0 = i1
                 )
                {
                if (!s_splitCusps)
                    {
                    i1 = listP[i].numPoints - 1;
                    }
                basePointP = listP[i].points + i0;
                num01 = i1 - i0 + 1;
                silhCurve.Zero ();
                if (cubicFit)
                    {
                    status = bspconv_cubicFitAndReducePoints (&silhCurve, basePointP, num01, tolerance);
                    }
                else
                    {
                    if (SUCCESS == (status = bspprof_fitPoints (&silhCurve,  basePointP, num01)))
                        {
                        status = bspcurv_curveDataReduction (&silhCurve, &silhCurve, &tol3d);
                        if (SUCCESS != status)
                            bspcurv_freeCurve (&silhCurve);
                        }
                    }

                if (status == SUCCESS)
                    {
                    silhCurve.display.polygonDisplay = false;
                    silhCurve.display.curveDisplay = true;
                    curves.push_back (silhCurve.CreateCapture ());

    #if defined (DEBUG_S)
                    if (debugDraw == true)
                        {
                        silhCurve.display.polygonDisplay = true;
                        silhCurve.display.curveDisplay = true;
                        debug_displayCurve (&silhCurve, NORMALDRAW);
                        }
    #endif
                    }
                }
            msbspline_free (listP[i].points);
            }
        }

    if (listP)
        BSIBaseGeom::Free (listP);

    return (status);
    }

END_BENTLEY_GEOMETRY_NAMESPACE
