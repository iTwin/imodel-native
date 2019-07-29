/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/MstnOnly/BspPrivateApi.h>
#include "msbsplinemaster.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE
#define ASSEMBLE_MOST_LINEAR
#define FLAT_SURFACE_SSI
#define BOUNDARY_CLIPPING

#define COSINE_3PIOVER4     -0.7071

#define STOPCODE_KEEP_GOING                 0
#define STOPCODE_ON_EDGE                    1
#define STOPCODE_CROSSING_EDGE              2
#define STOPCODE_CLOSURE_LINK               3
#define STOPCODE_ZINGER                     4
#define STOPCODE_BADTANGENT                 5
#define STOPCODE_ABOUT_FACE                 6
#define STOPCODE_IN_PLACE                   7

#define APPEND_NONE                         0
#define APPEND_ORG_ORG                      1
#define APPEND_END_ORG                      2
#define APPEND_ORG_END                      3
#define APPEND_END_END                      4

#define CLOSED_TRUE_CLOSURE                 1
#define CLOSED_ON_EDGE                      -1

#define INTERSECTION_TOLERANCE              1.0
#define MAX_INTERSECTIONS                   200

#define DEGENERACY_TOLERANCE                0.2

#define CODE_ALL                            0

#if defined (debug)
static int    debugStop, debugStep, debugCovered, addLink, displayBox, debugGetNextPt,
               displayLinks, debugTan, debugReconcile, debugStartPt, before, after;
#endif

#if defined (BEZIER_ONLY_MARCH)
/* If we go for this option clean up these global variables. I just used them
    now to test a few concepts. */
static DRange2d        globalBezierRange0;
static DRange2d        globalBezierRange1;
#endif

#if defined (debug)
void    displaySurf(), dumpChain();
#endif


/*----------------------------------------------------------------------+
|                                                                       |
|   Private Code Section                                                |
|                                                                       |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|                                                                       |
| name          angleBetweenSegments                                       |
|                                                                       |
| author        BFP                                     7/91            |
|                                                                       |
+----------------------------------------------------------------------*/
static double   AbsCosineBetweenSegments
(
DPoint3d        *org1,
DPoint3d        *end1,
DPoint3d        *org2,
DPoint3d        *end2
)
    {
    DPoint3d    diff1, diff2;

    diff1.NormalizedDifference (*end1, *org1);
    diff2.NormalizedDifference (*end2, *org2);

    return (fabs(diff1.DotProduct (diff2)));
    }




#ifdef CompileAll
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             03/91
+---------------+---------------+---------------+---------------+---------------+------*/
static double  bspssi_evaluatePlane
(
DPoint3d        *pointP,               /* => space point */
DPoint3d        *originP,              /* => plane origin */
DPoint3d        *normalP               /* => plane normal */
)
    {
    DPoint3d diff;
    diff.DifferenceOf (*pointP, *originP);
    return diff.DotProduct (*normalP);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     03/91
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspssi_reconcileNearPtsFast
(
DPoint3d        *newPt,                /* <= next approx intersection pt */
DPoint3d        *near0,                /* => near point on surf0 */
DPoint3d        *near1,                /* => near point on surf1 */
DPoint3d        *norm0,                /* => normal of surf0 @ near0 */
DPoint3d        *norm1                 /* => normal of surf1 @ near1 */
)
    {
    DPoint3d unitN0, unitN1, U;
    double b0, b1, n0n1;
    double alpha0, alpha1;
    double det, inverseDet;
    double detTol = 1.0e-12;
    unitN0 = *norm0;
    unitN1 = *norm1;

    unitN0.Normalize ();
    unitN1.Normalize ();
    n0n1 = unitN0.DotProduct (unitN1);

    U.x = 0.5 * (near1->x - near0->x);
    U.y = 0.5 * (near1->y - near0->y);
    U.z = 0.5 * (near1->z - near0->z);


    /* We want to project from midway between the near points to the intersection line.
       The linear system for the offset as a linear combination of the normals is
        [ n0n0  n0n1 ][a0]=[b0]
        [ n0n1  n1n1 ][a1]=[b1]
        n0n0=n1n1=1 because we normalized. The determinant 1-n0n1^2 should be positive.
    */
    det = 1.0 - n0n1 * n0n1;

    newPt->SumOf (*near0, U);
    if (det < detTol)
        return ERROR;

    b0 = -U.DotProduct (unitN0);
    b1 = U.DotProduct (unitN1);

    inverseDet = 1.0 / det;
    alpha0 = (b0 - n0n1 * b1) * inverseDet;
    alpha1 = (b1 - n0n1 * b0) * inverseDet;
    newPt->x += alpha0 * unitN0.x + alpha1 * unitN1.x;
    newPt->y += alpha0 * unitN0.y + alpha1 * unitN1.y;
    newPt->z += alpha0 * unitN0.z + alpha1 * unitN1.z;
    return SUCCESS;
    }

#ifdef CompileAll
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             03/91
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspssi_reconcileNearPts
(
DPoint3d        *newPt,                /* <= next approx intersection pt */
DPoint3d        *near0,                /* => near point on surf0 */
DPoint3d        *near1,                /* => near point on surf1 */
DPoint3d        *norm0,                /* => normal of surf0 @ near0 */
DPoint3d        *norm1                 /* => normal of surf1 @ near1 */
)
    {
    double          b0, b1, b2, denom, mag;
    DPoint3d        n0, n1, n2, c0, c1, c2, midPt, tmp;

    n0 = *norm0;
    n1 = *norm1;
    n0.Normalize ();
    n1.Normalize ();
    n2.CrossProduct (n0, n1);     /* normal of plane perp to first two */

    if ((mag = n2.Normalize ()) < fc_epsilon)
        return ERROR;

    c0.CrossProduct (n1, n2);
    denom = n0.DotProduct (c0);

    c1.CrossProduct (n2, n0);
    c2.CrossProduct (n0, n1);

    midPt.Interpolate (*near0, 0.5, *near1);

    b0 = n0.DotProduct (*near0);
    b1 = n1.DotProduct (*near1);
    b2 = n2.DotProduct (midPt);

    newPt->Scale (c0, b0);
    tmp.Scale (c1, b1);
    newPt->SumOf (*newPt, tmp);
    tmp.Scale (c2, b2);
    newPt->SumOf (*newPt, tmp);
    newPt->Scale (*newPt, 1.0/denom);
#if defined (debug)
    if (debugReconcile)
        {
        DPoint3d        line[3];
        ElementUnion    u;

        line[0] = *near0;
        line[1] = *newPt;
        line[2] = *near1;

        mdlLineString_directCreate (&u, NULL, line, NULL, 3, LINE_STRING_ELM, 0);
        u.line_3d.dhdr.symb.b.style = 2;
        mdlElement_display (&u, 2);
        }
#endif
    return SUCCESS;
    }
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    EDL             04/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP StatusInt    bspssi_projectToTangentSystem
(
DPoint3d            *paramP,            /* <= x,y are parameters of projection
                                                of pointP onto skewed plane.  z=0 */
double              *duduP,             /* <= squared magnitude of U vector */
double              *dvdvP,             /* <= squared magnitude of V vector */
DPoint3d            *vectorUP,          /* => parameter space U direction */
DPoint3d            *vectorVP,          /* => parameter space V direction */
DPoint3d            *originP,           /* => origin of parameter system */
DPoint3d            *pointP,            /* => space point to be projected */
double              shortVecTol         /* => tolerance to consider short vectors zero */
)
    {
    double a00, a01, a10, a11, b0, b1, det, p0, p1;
    DPoint3d vectorW;
    static double relTol = 1.0e-10;
    StatusInt status;
    double tol2 = shortVecTol * shortVecTol;
    vectorW.DifferenceOf (*pointP, *originP);

    a00 = *duduP = vectorUP->DotProduct (*vectorUP);
    a01 = a10    = vectorUP->DotProduct (*vectorVP);
    a11 = *dvdvP = vectorVP->DotProduct (*vectorVP);
    if (a00 <= tol2 || a11 <= tol2)
        return ERROR;

    b0  = vectorUP->DotProduct (vectorW);
    b1  = vectorVP->DotProduct (vectorW);

    p0 = a00 * a11; /* Must be positive */
    p1 = a01 * a10; /* Also must be positive !! */
    det = p0 - p1;
    if (det == 0.0 || fabs (det) < (p0 + p1) * relTol)
        {
        paramP->x = paramP->y = paramP->z = 0.0;
        status = ERROR;
        }
    else
        {
        double inverseDet = 1.0 / det;
        paramP->x = (b0 * a11 - b1 * a01) * inverseDet;
        paramP->y = -(b0 * a10 - b1 * a00) * inverseDet;
        paramP->z = 0.0;
        status = SUCCESS;
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             03/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspssi_relaxToDifficultSurface
(
DPoint3d            *nearPt,           /* <= closest point on surface */
DPoint3d            *normal,           /* <= normal to surf @ nearPt, scaled by sine angle */
double              *degeneracy,       /* <= sine of angle between surface partials */
DPoint2d            *uv,               /* <=> initial guess -> param of pt */
DPoint3d            *testPt,           /* => want closest pt to this pt */
Evaluator           *eval,
int                 iterations,        /* => number of iterations to try */
double              convTol            /* => convergence tolerance in UV space */
)
    {
    int             count;
    bool            bad_dPdu, bad_dPdv;
    double          mag_cross, mag_dPdu, mag_dPdv, mag_dCrossDu, mag_dCrossDv,
                    mag_cross2, prod;
    double          dudu, dvdv;
    DVec3d          du, dv, zeroVec, difference,
                    dNormDu, dNormDv, cross, dCrossDu, dCrossDv, tmp0, tmp1;
    RotMatrix       tmp;

    StatusInt paramStat;

    zeroVec.x = zeroVec.y = zeroVec.z =
    difference.x = difference.y = 0.0;

    for (count=0; count < iterations; count++)
        {
        uv->x += difference.x;
        uv->y += difference.y;

        /*---------------------------------------------------------------
        Add the correction to the uv pair. If I am crossing an edge of the
        [0,1] parameter square then if closed go around to the other side,
        or if open, just relax to boundary. This assumes that the new uv
        is no more than 1.0 outside the unit square; i.e. adding/subtracting
        one will wrap around to the opposite side of a closed surface.
        ---------------------------------------------------------------*/
        if (uv->x < 0.0)
            {
            if (eval->surf->uParams.closed)
                uv->x += 1.0;
            else
                uv->x = 0.0;
            }
        else if (uv->x > 1.0)
            {
            if (eval->surf->uParams.closed)
                uv->x -= 1.0;
            else
                uv->x = 1.0;
            }
        if (uv->y < 0.0)
            {
            if (eval->surf->vParams.closed)
                uv->y += 1.0;
            else
                uv->y = 0.0;
            }
        else if (uv->y > 1.0)
            {
            if (eval->surf->vParams.closed)
                uv->y -= 1.0;
            else
                uv->y = 1.0;
            }


        if (eval->offset)
            {
            DPoint3d duu, duv, dvv, safeNormal;

            bspsurf_computePartials (nearPt, NULL, &du, &dv, &duu, &dvv, &duv,
                                 &safeNormal, uv->x, uv->y, eval->surf);
            /*-----------------------------------------------------------
            Must correct the partials, see Farouki, R.T. in CAGD vol 3, pp. 15-45.
            -----------------------------------------------------------*/
            nearPt->SumOf (*nearPt, safeNormal, eval->distance / bsiDPoint3d_magnitude (&safeNormal));

            cross.CrossProduct (du, dv);
            tmp0.CrossProduct (duu, dv);
            tmp1.CrossProduct (du, duv);
            dCrossDu.SumOf (tmp0, tmp1);
            tmp0.CrossProduct (duv, dv);
            tmp1.CrossProduct (du, dvv);
            dCrossDv.SumOf (tmp0, tmp1);

            mag_cross    = cross.Magnitude ();
            mag_cross2   = mag_cross * mag_cross;
            mag_dCrossDu = cross.DotProduct (dCrossDu) / mag_cross;
            mag_dCrossDv = cross.DotProduct (dCrossDv) / mag_cross;

            dNormDu.Scale (dCrossDu, mag_cross);
            dNormDu.SumOf (dNormDu, cross, - mag_dCrossDu);
            dNormDu.Scale (dNormDu, 1.0 / mag_cross2);
            dNormDv.Scale (dCrossDv, mag_cross);
            dNormDv.SumOf (dNormDv, cross, - mag_dCrossDv);
            dNormDv.Scale (dNormDv, 1.0 / mag_cross2);

            du.SumOf (du, dNormDu, eval->distance);
            dv.SumOf (dv, dNormDv, eval->distance);
            }
        else
            {
            bspsurf_evaluateSurfacePoint (nearPt, NULL, &du, &dv,
                                 uv->x, uv->y, eval->surf);
            }
        mag_dPdu = du.Magnitude ();
        mag_dPdv = dv.Magnitude ();
        bad_dPdu = mag_dPdu < fc_epsilon;
        bad_dPdv = mag_dPdv < fc_epsilon;
        cross.CrossProduct (du, dv);
        mag_cross = cross.Magnitude ();

        if (bad_dPdu || bad_dPdv)
            {
            /* Use the safe normal vector. */
            DPoint3d safeNormal;
            bspsurf_computePartials (NULL, NULL, &du, &dv, NULL, NULL, NULL,
                                 &safeNormal, uv->x, uv->y, eval->surf);


            *degeneracy = 0.0;
            *normal = safeNormal;
            mag_cross = safeNormal.Magnitude ();

            if (mag_cross < fc_epsilon)
                return STATUS_NONCONVERGED;

            /* Fix the bad partials so the Jacobian doesn't go wild. */
            if (bad_dPdu && ! bad_dPdv)
                {
                du.CrossProduct (dv, *normal);
                du.Normalize ();
                du.Scale (du, mag_cross / mag_dPdv);
                }
            else if (bad_dPdv && ! bad_dPdu)
                {
                dv.CrossProduct (*normal, du);
                dv.Normalize ();
                dv.Scale (dv, mag_cross / mag_dPdu);
                }
            else
                {
                rotMatrix_orthogonalFromZRow (&tmp, (DVec3d*)normal);
                bsiRotMatrix_getRow ( &tmp, &du,  0);
                bsiRotMatrix_getRow ( &tmp, &dv,  1);
                du.Scale (du, sqrt (mag_cross));
                dv.Scale (dv, sqrt (mag_cross));
                }
            }
        else
            {
            prod = 1.0 / (mag_dPdu * mag_dPdv);
            mag_cross *= prod;
            *degeneracy = mag_cross;     /* sine of angle between dPdu & dPdv */
            normal->Scale (cross, prod);    /* scaled by sine angle btw partials */
            }

#if defined (debug_relax)
    {
    DPoint3d        line[2];
    ElementUnion    u;

    line[0] = *nearPt;
    line[1].SumOf (*line, *normal, fc_1000);
    mdlLine_directCreate (&u, NULL, line, NULL);
    u.line_3d.dhdr.symb.b.style = 3;
    mdlElement_display (&u, 2);
    }
#endif

        paramStat = bspssi_projectToTangentSystem (&difference, &dudu, &dvdv, &du, &dv, nearPt, testPt, fc_epsilon);

        if (fabs (difference.x) < convTol &&
            fabs (difference.y) < convTol &&
            SUCCESS == paramStat)
            return STATUS_CONVERGED;
        }

    /* Failed to converge within iterations */
    return STATUS_NONCONVERGED;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             03/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspssi_relaxToSurface
(
DPoint3d            *nearPt,           /* <= closest point on surface */
DPoint3d            *normal,           /* <= normal to surf @ nearPt, scaled by sine angle */
double              *degeneracy,       /* <= sine of angle between surface partials */
DPoint2d            *uv,               /* <=> initial guess -> param of pt */
DPoint3d            *testPt,           /* => want closest pt to this pt */
Evaluator           *eval,
int                 iterations,        /* => number of iterations to try */
double              convTol            /* => convergence tolerance in UV space */
)
    {
    int             count;
    double          prod;
    double          dudu, dvdv;
    DPoint3d        du, dv, zeroVec, difference,
                    cross;
    DPoint3d        localNormal;
    double          localDegeneracy;

    StatusInt paramStat;
    bool    converged;
    static int alwaysDifficult = 0;

    if (alwaysDifficult || eval->offset)
        return bspssi_relaxToDifficultSurface (nearPt,
                            normal ? normal : &localNormal,
                            degeneracy ? degeneracy : &localDegeneracy,
                            uv, testPt, eval, iterations, convTol);

    zeroVec.x = zeroVec.y = zeroVec.z =
    difference.x = difference.y = 0.0;

    for (count=0; count++ < iterations;)
        {
        uv->x += difference.x;
        uv->y += difference.y;

        /*---------------------------------------------------------------
        Add the correction to the uv pair. If I am crossing an edge of the
        [0,1] parameter square then if closed go around to the other side,
        or if open, just relax to boundary. This assumes that the new uv
        is no more than 1.0 outside the unit square; i.e. adding/subtracting
        one will wrap around to the opposite side of a closed surface.
        ---------------------------------------------------------------*/
        if (uv->x < 0.0)
            {
            if (eval->surf->uParams.closed)
                uv->x += 1.0;
            else
                uv->x = 0.0;
            }
        else if (uv->x > 1.0)
            {
            if (eval->surf->uParams.closed)
                uv->x -= 1.0;
            else
                uv->x = 1.0;
            }
        if (uv->y < 0.0)
            {
            if (eval->surf->vParams.closed)
                uv->y += 1.0;
            else
                uv->y = 0.0;
            }
        else if (uv->y > 1.0)
            {
            if (eval->surf->vParams.closed)
                uv->y -= 1.0;
            else
                uv->y = 1.0;
            }


        bspsurf_evaluateSurfacePoint (nearPt, NULL, &du, &dv,
                                 uv->x, uv->y, eval->surf);

        paramStat = bspssi_projectToTangentSystem (&difference, &dudu, &dvdv,
                            &du, &dv, nearPt, testPt, fc_epsilon);

        if (SUCCESS != paramStat)
                return bspssi_relaxToDifficultSurface (nearPt,
                            normal ? normal : &localNormal,
                            degeneracy ? degeneracy : &localDegeneracy,
                            uv, testPt, eval, iterations, convTol);

        converged = fabs (difference.x) < convTol && fabs (difference.y) < convTol;

        if (converged || count == iterations)
            {
            if (normal || degeneracy)
                {
                cross.CrossProduct (du, dv);
                prod = 1.0 / sqrt (dudu * dvdv);

                if (degeneracy)
                    {
                    double dwdw;
                    dwdw = cross.DotProduct (cross);
                    *degeneracy = sqrt (dwdw) * prod;   /* sine of angle btw partials */
                    }

                if (normal)
                    normal->Scale (cross, prod);   /* scaled by sine angle btw partials */
                }
            }

        if (converged)
            return STATUS_CONVERGED;
        }

    /* Failed to converge within iterations */
    return STATUS_NONCONVERGED;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             04/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspssi_relaxToPatch
(
DPoint3d            *nearPt,           /* <= closest point on surface */
DPoint3d            *normal,           /* <= normal to surf @ nearPt */
DPoint2d            *uv,               /* <=> initial guess -> param of pt */
DPoint3d            *testPt,           /* => want closest pt to this pt */
MSBsplineSurface    *surface,
int                 iterates
)
    {
    int                 i;
    double              knots[2 * MAX_ORDER], *kP;
    MSBsplineSurface    srf;
    Evaluator           eval;

    eval.surf     = &srf;
    eval.offset   = false;
    eval.distance = 0.0;

    memset (knots, 0, MAX_ORDER * sizeof(double));
    for (i = MAX_ORDER; i < 2 * MAX_ORDER; i++)
        knots[i] = 1.0;

    kP = knots + MAX_ORDER;

    memset (&srf, 0 , sizeof(srf));
    srf.rational = surface->rational;
    srf.uParams.numPoles = srf.uParams.order = surface->uParams.order;
    srf.vParams.numPoles = srf.vParams.order = surface->vParams.order;
    srf.uKnots = kP - srf.uParams.order;
    srf.vKnots = kP - srf.vParams.order;
    srf.poles = surface->poles;
    srf.weights = surface->weights;

    return bspssi_relaxToSurface (nearPt, normal, NULL, uv, testPt,
                                  &eval, iterates, fc_epsilon);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             04/91
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspssi_getNextPt
(
SsiPt               *iPt,              /* <= closest point on intersection */
DPoint3d            *tangent,          /* <= tangent of intersection curve */
double              *degeneracy,       /* <= sine of angle between surface normals */
                                       /*    time partial degeneracies for each surface */
SsiPt               *approx,           /* => next approx to intersection */
SsiTolerance        *ssiTolP,
Evaluator           *eval0,
Evaluator           *eval1,
int                 revLimit
)
    {
    int             i, status;
    double          dist, partialDegen0 = 0.0, partialDegen1 = 0.0;
    double          *partialDegen0P, *partialDegen1P;
    DPoint3d        near0, near1;
#if defined (debug)
    DPoint3d        pts[MAX_POLES];
    if (revLimit > MAX_POLES)
        {
        printf ("\n TROUBLE IF revLimit = %d > MAX_POLES!!!", revLimit);
        return  ERROR;
        }
#endif

    if (degeneracy)
        {
        partialDegen0P = &partialDegen0;
        partialDegen1P = &partialDegen1;
        }
    else
        {
        partialDegen0P = partialDegen1P = NULL;
        }

    iPt->uv0 = approx->uv0;    iPt->uv1 = approx->uv1;
    if (((status = bspssi_relaxToSurface (&near0, &iPt->norm0, NULL, &iPt->uv0,
                   &approx->xyz, eval0, 2, ssiTolP->uvSame)) == STATUS_BADNORMAL) ||
        ((status = bspssi_relaxToSurface (&near1, &iPt->norm1, NULL, &iPt->uv1,
                   &approx->xyz, eval1, 2, ssiTolP->uvSame)) == STATUS_BADNORMAL))
        return status;

    if (SUCCESS != (status = bspssi_reconcileNearPtsFast (&iPt->xyz, &near0, &near1,
                                                      &iPt->norm0, &iPt->norm1)))
        return STATUS_BADNORMAL; /* the two surfaces normals are parallel */

    for (i=0; i < revLimit; i++)
        {
#if defined (debug)
        pts[i] = iPt->xyz;
#endif
        bspssi_relaxToSurface (&near0, &iPt->norm0, partialDegen0P, &iPt->uv0, &iPt->xyz,
                               eval0, 2, ssiTolP->uvSame);
        bspssi_relaxToSurface (&near1, &iPt->norm1, partialDegen1P, &iPt->uv1, &iPt->xyz,
                               eval1, 2, ssiTolP->uvSame);
        if ((dist = near0.Distance (near1)) < ssiTolP->xyzSame)
            {
            iPt->xyz.Interpolate (near0, 0.5, near1);
            tangent->CrossProduct (iPt->norm0, iPt->norm1);
            if (degeneracy)
                {
                *degeneracy = tangent->Normalize ();
                *degeneracy *= partialDegen0;
                *degeneracy *= partialDegen1;
                }
#if defined (debug)
            before++;
            if (debugTan)
                {
                DPoint3d        line[2];
                ElementUnion    u;

                line[0] = near0;
                line[1].SumOf (*line, *tangent);
                line[1].SumOf (*line, *tangent, fc_10000);
                mdlLine_directCreate (&u, NULL, line, NULL);
                u.line_3d.dhdr.symb.b.color=1;
                mdlElement_display (&u, 2);
                }
            if (debugGetNextPt)
                {
                ElementUnion    u;

                mdlLineString_directCreate (&u, NULL, pts, NULL, i, LINE_STRING_ELM, 0);
                mdlElement_display (&u, 2);
                }
#endif
            return STATUS_CONVERGED;
            }

        if (SUCCESS != (status = bspssi_reconcileNearPtsFast (&iPt->xyz, &near0, &near1,
                                              &iPt->norm0, &iPt->norm1)))
            return STATUS_BADNORMAL;
        }

#if defined (debug)
    after++;
    if (debugGetNextPt)
        {
        ElementUnion    u;

        mdlLineString_directCreate (&u, NULL, pts, NULL, revLimit, LINE_STRING_ELM, 0);
        mdlElement_display (&u, 2);
        }
#endif
    tangent->CrossProduct (iPt->norm0, iPt->norm1);
    if (degeneracy)
        *degeneracy = tangent->Normalize () * partialDegen0 * partialDegen1;
    return STATUS_NONCONVERGED;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             04/91
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspssi_flushPts
(
SsiPt           *buffer,
int             bufSize,
DPoint3d        **destPt,
DPoint3d        **destNorm0,
DPoint3d        **destNorm1,
DPoint2d        **destUV0,
DPoint2d        **destUV1,
int             *destSize
)
    {
    int         i, allocSize;
    DPoint2d    *uv0P, *uv1P;
    DPoint3d    *xyzP, *n0P, *n1P;
    SsiPt       *iP;

    if (bufSize + *destSize < 2)
        return  SUCCESS;

    if (bufSize)
        {
        if (*destSize == 0)
            {
            allocSize = bufSize * sizeof(DPoint3d);
            if (NULL == (xyzP = (DPoint3d*)msbspline_malloc (allocSize, HEAPSIG_BSSI)) ||
                NULL == (n0P  = (DPoint3d*)msbspline_malloc (allocSize, HEAPSIG_BSSI)) ||
                NULL == (n1P  = (DPoint3d*)msbspline_malloc (allocSize, HEAPSIG_BSSI)))
                return ERROR;

            allocSize = bufSize * sizeof(DPoint2d);
            if (NULL == (uv0P = (DPoint2d*)msbspline_malloc (allocSize, HEAPSIG_BSSI)) ||
                NULL == (uv1P = (DPoint2d*)msbspline_malloc (allocSize, HEAPSIG_BSSI)))
                return ERROR;

            *destPt    = xyzP;
            *destNorm0 = n0P;
            *destNorm1 = n1P;
            *destUV0   = uv0P;
            *destUV1   = uv1P;
            *destSize  = bufSize;
            for (i=0, iP=buffer; i < bufSize;
                 i++, iP++, xyzP++, n0P++, n1P++, uv0P++, uv1P++)
                {
                *xyzP = iP->xyz;
                *n0P  = iP->norm0;
                *n1P  = iP->norm1;
                *uv0P = iP->uv0;
                *uv1P = iP->uv1;
                }
            }
        else
            {
            allocSize = (*destSize + bufSize) * sizeof(DPoint3d);
            if (NULL == (xyzP = (DPoint3d*)msbspline_realloc (*destPt,    allocSize)) ||
                NULL == (n0P  = (DPoint3d*)msbspline_realloc (*destNorm0, allocSize)) ||
                NULL == (n1P  = (DPoint3d*)msbspline_realloc (*destNorm1, allocSize)))
                return ERROR;

            allocSize = (*destSize + bufSize) * sizeof(DPoint2d);
            if (NULL == (uv0P = (DPoint2d*)msbspline_realloc (*destUV0, allocSize)) ||
                NULL == (uv1P = (DPoint2d*)msbspline_realloc (*destUV1, allocSize)))
                return ERROR;

            *destPt    = xyzP;
            *destNorm0 = n0P;
            *destNorm1 = n1P;
            *destUV0   = uv0P;
            *destUV1   = uv1P;
            xyzP += *destSize;
            n0P  += *destSize;
            n1P  += *destSize;
            uv0P += *destSize;
            uv1P += *destSize;
            for (i=0, iP=buffer; i < bufSize;
                 i++, iP++, xyzP++, n0P++, n1P++, uv0P++, uv1P++)
                {
                *xyzP = iP->xyz;
                *n0P  = iP->norm0;
                *n1P  = iP->norm1;
                *uv0P = iP->uv0;
                *uv1P = iP->uv1;
                }
            *destSize += bufSize;
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             04/91
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspssi_addPt
(
SsiPt           *pt,
SsiPt           *buffer,
int             *bufSize,
DPoint3d        **destPt,
DPoint3d        **destNorm0,
DPoint3d        **destNorm1,
DPoint2d        **destUV0,
DPoint2d        **destUV1,
int             *destSize
)
    {
    int     status;

    buffer[*bufSize] = *pt;
    (*bufSize)++;
    if (*bufSize >= MAX_BSPBATCH-1)
        {
        if (SUCCESS != (status = bspssi_flushPts (buffer, *bufSize, destPt, destNorm0,
                                                  destNorm1, destUV0, destUV1, destSize)))
            return status;
        *bufSize = 0;
        }
    return SUCCESS;
    }
#if defined (unused)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    08/93
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bsputil_trimToRange /* <= true if trimmed */
(
DPoint2d        *toPt,
DPoint2d        *fromPt,
DRange2d       *rangeIn,
int             uClosed,
int             vClosed,
double          tolerance
)
    {
    DPoint2d    delta, fabsDelta, a0, a1, b0, b1, rangeSize;
    DRange2d   range;

    if (rangeIn)
        {
        range = *rangeIn;
        rangeSize.x = range.high.x - range.low.x;
        rangeSize.y = range.high.y - range.low.y;
        }
    else
        {
        range.low.x = range.low.y = 0.0;
        range.high.x = range.high.y = rangeSize.x = rangeSize.y = 1.0;
        }

    a0.x = fromPt->x - range.low.x;
    a0.y = fromPt->y - range.low.y;
    a1.x = range.high.x - fromPt->x;
    a1.y = range.high.y - fromPt->y;

    b0.x = toPt->x - range.low.x;
    b0.y = toPt->y - range.low.y;
    b1.x = range.high.x - toPt->x;
    b1.y = range.high.y - toPt->y;

    fabsDelta.x = fabs (delta.x = toPt->x - fromPt->x);
    fabsDelta.y = fabs (delta.y = toPt->y - fromPt->y);

    if (b0.x < tolerance)
        {
        if (uClosed && a1.x < tolerance)
            {
            /* crossing u = 1.0 edge */
            toPt->x = range.high.x;
            delta.x += rangeSize.x;
            }
        else
            {
            /* approaching u = 0.0 edge */
            toPt->x = range.low.x;
            }
        if (fabsDelta.x > fc_1em15)
            toPt->y -= delta.y / delta.x * b0.x;
        return true;
        }
    else if (b1.x < tolerance)
        {
        if (uClosed && a0.x < tolerance)
            {
            /* crossing u = 0.0 edge */
            toPt->x = range.low.x;
            delta.x -= rangeSize.x;
            }
        else
            {
            /* approaching u = 1.0 edge */
            toPt->x = range.high.x;
            }
        if (fabsDelta.x > fc_1em15)
            toPt->y += delta.y / delta.x * b1.x;
        return true;
        }
    else if (b0.y < tolerance)
        {
        if (vClosed && a1.y < tolerance)
            {
            /* crossing v = 1.0 edge */
            toPt->y = range.high.y;
            delta.y += rangeSize.y;
            }
        else
            {
            /* approaching v = 0.0 edge */
            toPt->y = range.low.y;
            }
        if (fabsDelta.y > fc_1em15)
            toPt->x -= delta.x / delta.y * b0.y;
        return true;
        }
    else if (b1.y < tolerance)
        {
        if (vClosed && a0.y < tolerance)
            {
            /* crossing v = 0.0 edge */
            toPt->y = range.low.y;
            delta.y -= rangeSize.y;
            }
        else
            {
            /* approaching v = 1.0 edge */
            toPt->y = range.high.y;
            }
        if (fabsDelta.y > fc_1em15)
            toPt->x += delta.x / delta.y * b1.y;
        return true;
        }

    return false;
    }
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    07/93
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspssi_atEdge
(
SsiPt           *ptA,
SsiPt           *ptB,
int             code,            /* => choice of uv0 or uv1 */
SsiTolerance    *ssiTolP,
Evaluator       *eval0,
Evaluator       *eval1,
bool            adjustData
)
    {

    /*-------------------------------------------------------------------
    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    !!!!   DO NOT ALTER THIS ROUTINE IN ANY FASHION WITHOUT CHECKING !!!!
    !!!!   WITH BRIAN PETERS FIRST. THESE TESTS ARE CRUCIAL TO THE   !!!!
    !!!!   FUNCTIONING OF THE ENTIRE SURFACE-SURFACE INTERSECTION    !!!!
    !!!!   PROCESS.                                                  !!!!
    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    -------------------------------------------------------------------*/

    bool        approachingU0, approachingU1, approachingV0, approachingV1,
                crossingU0, crossingU1, crossingV0, crossingV1, closedU, closedV,
                alongU0, alongU1, alongV0, alongV1,
                a0xbelow, a1xbelow, b0xbelow, b1xbelow,
                a0ybelow, a1ybelow, b0ybelow, b1ybelow;
    int         stopAtEdge;
    DPoint2d    delta, fabsDelta, a0, a1, b0, b1, *BuvP = NULL, *AotherUvP, *BotherUvP = NULL;
    DPoint3d    dPdu, dPdv, tmp0, tmp1, *normP = NULL, *otherNormP = NULL;
    Evaluator   *evalP = NULL, *otherEvalP = NULL;

    a0.Zero ();
    b0.Zero ();
    switch (code)
        {
        case CODE_UV0:
            a0.x = ptA->uv0.x;
            a0.y = ptA->uv0.y;

            b0.x = ptB->uv0.x;
            b0.y = ptB->uv0.y;

            normP       = &ptB->norm0;
            otherNormP  = &ptB->norm1;
            BuvP        = &ptB->uv0;
            AotherUvP   = &ptA->uv1;
            BotherUvP   = &ptB->uv1;
            evalP       = eval0;
            otherEvalP  = eval1;
            break;

        case CODE_UV1:
            a0.x = ptA->uv1.x;
            a0.y = ptA->uv1.y;

            b0.x = ptB->uv1.x;
            b0.y = ptB->uv1.y;

            normP       = &ptB->norm1;
            otherNormP  = &ptB->norm0;
            BuvP        = &ptB->uv1;
            AotherUvP   = &ptA->uv0;
            BotherUvP   = &ptB->uv0;
            evalP       = eval1;
            otherEvalP  = eval0;
            break;
        }

    a1.x = 1.0 - a0.x;
    a1.y = 1.0 - a0.y;
    b1.x = 1.0 - b0.x;
    b1.y = 1.0 - b0.y;

    fabsDelta.x = fabs (delta.x = b0.x - a0.x);
    fabsDelta.y = fabs (delta.y = b0.y - a0.y);

    a0xbelow = a0.x < ssiTolP->uvTol;
    a1xbelow = a1.x < ssiTolP->uvTol;
    a0ybelow = a0.y < ssiTolP->uvTol;
    a1ybelow = a1.y < ssiTolP->uvTol;

    b0xbelow = b0.x < ssiTolP->uvTol;
    b1xbelow = b1.x < ssiTolP->uvTol;
    b0ybelow = b0.y < ssiTolP->uvTol;
    b1ybelow = b1.y < ssiTolP->uvTol;

    /* Determine case */
    bspsurf_isPhysicallyClosed (&closedU, &closedV, evalP->surf);

    if (closedU && (b0xbelow || b1xbelow) && fabsDelta.y > ssiTolP->uvTol)
        {
        alongU0 = a0xbelow;
        alongU1 = a1xbelow;
        }
    else
        alongU0 = alongU1 = false;

    crossingU0 = closedU && a0.x + b1.x < fabsDelta.x;
    crossingU1 = closedU && a1.x + b0.x < fabsDelta.x;

    approachingU0 = b0xbelow && ! a0xbelow && ! a1xbelow;
    approachingU1 = b1xbelow && ! a1xbelow && ! a0xbelow;

    if (closedV && (b0ybelow || b1ybelow) && fabsDelta.x > ssiTolP->uvTol)
        {
        alongV0 = a0ybelow;
        alongV1 = a1ybelow;
        }
    else
        alongV0 = alongV1 = false;

    crossingV0 = closedV && a0.y + b1.y < fabsDelta.y;
    crossingV1 = closedV && a1.y + b0.y < fabsDelta.y;

    approachingV0 = b0ybelow && ! a0ybelow && ! a1ybelow;
    approachingV1 = b1ybelow && ! a1ybelow && ! a0ybelow;

    /*-------------------------------------------------------------------
    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    !!!!   DO NOT ALTER THIS ROUTINE IN ANY FASHION WITHOUT CHECKING !!!!
    !!!!   WITH BRIAN PETERS FIRST. THESE TESTS ARE CRUCIAL TO THE   !!!!
    !!!!   FUNCTIONING OF THE ENTIRE SURFACE-SURFACE INTERSECTION    !!!!
    !!!!   PROCESS.                                                  !!!!
    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    -------------------------------------------------------------------*/

    if (approachingU0 || crossingU1 || alongU1)
        {
        if (crossingU1 || alongU1)
            {
            BuvP->x   = 1.0;
            if (crossingU1)
                delta.x += 1.0;
            else
                delta.x = 0.0;
            stopAtEdge = crossingU1 && ! alongU1;
            }
        else
            {
            BuvP->x = 0.0;
            stopAtEdge = true;
            }

        if (fabs (delta.x) > fc_1em15)
            BuvP->y -= delta.y / delta.x * b0.x;
        }
    else if (approachingU1 || crossingU0 || alongU0)
        {
        if (crossingU0 || alongU0)
            {
            BuvP->x = 0.0;
            if (crossingU0)
                delta.x -= 1.0;
            else
                delta.x = 0.0;
            stopAtEdge = crossingU0 && ! alongU0;
            }
        else
            {
            BuvP->x = 1.0;
            stopAtEdge = true;
            }

        if (fabs (delta.x) > fc_1em15)
            BuvP->y += delta.y / delta.x * b1.x;
        }
    else if (approachingV0 || crossingV1 || alongV1)
        {
        if (crossingV1 || alongV1)
            {
            BuvP->y = 1.0;
            if (crossingV1)
                delta.y += 1.0;
            else
                delta.y = 0.0;
            stopAtEdge = crossingV1 && ! alongV1;
            }
        else
            {
            BuvP->y = 0.0;
            stopAtEdge = true;
            }

        if (fabs (delta.y) > fc_1em15)
            BuvP->x -= delta.x / delta.y * b0.y;
        }
    else if (approachingV1 || crossingV0 || alongV0)
        {
        if (crossingV0 || alongV0)
            {
            BuvP->y = 0.0;
            if (crossingV0)
                delta.y -= 1.0;
            else
                delta.y = 0.0;
            stopAtEdge = crossingV0 && ! alongV0;
            }
        else
            {
            BuvP->y = 1.0;
            stopAtEdge = true;
            }

        if (fabs (delta.y) > fc_1em15)
            BuvP->x += delta.x / delta.y * b1.y;
        }
    else
        {
        stopAtEdge = adjustData = false;
        }

    if (adjustData)
        {
        /* correct ptB data for normP */
        bspsurf_computePartials (&tmp0, NULL, &dPdu, &dPdv, NULL, NULL, NULL,
                                 normP, BuvP->x, BuvP->y, evalP->surf);
        normP->Normalize ();
        if (evalP->offset)
            tmp0.SumOf (tmp0, *normP, evalP->distance);

        /* correct ptB data for BotherUvP, otherNormP */
        bspssi_relaxToSurface (&tmp1, otherNormP, NULL, BotherUvP, &tmp0, otherEvalP,
                               5, ssiTolP->uvSame);
        bspssi_atEdge (ptA, ptB, CODE_UV0 == code ? CODE_UV1 : CODE_UV0, ssiTolP,
                       eval0, eval1, false);

        /* correct ptB data for xyz. MAYBE I SHOULD LEAVE AS tmp0?? */
        ptB->xyz.Interpolate (tmp0, 0.5, tmp1);
        }

    return stopAtEdge;
    /*-------------------------------------------------------------------
    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    !!!!   DO NOT ALTER THIS ROUTINE IN ANY FASHION WITHOUT CHECKING !!!!
    !!!!   WITH BRIAN PETERS FIRST. THESE TESTS ARE CRUCIAL TO THE   !!!!
    !!!!   FUNCTIONING OF THE ENTIRE SURFACE-SURFACE INTERSECTION    !!!!
    !!!!   PROCESS.                                                  !!!!
    !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    -------------------------------------------------------------------*/
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             04/91
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspssi_finishedMarch
(
SsiPt           *p0,
SsiPt           *p1,
IntLink         *curr,
double          length,
double          zingerSize,
DPoint3d        *lastTan,
DPoint3d        *currTan,
DPoint3d        *lastStep,
SsiPt           *buffer,
int             *bufSize,
SsiTolerance    *ssiTolP,
Evaluator       *eval0,
Evaluator       *eval1
)
    {
    double      dist, dot;
    DPoint3d    step;

    /* Check for zingers */
    if ((dist = p0->xyz.Distance (p1->xyz)) > zingerSize)
        {
#if defined (debug)
        if (debugStop)
            printf ("\nStop march: zinger");
#endif
        return STOPCODE_ZINGER;
        }

    /* Check for edge intersection */
    if (bspssi_atEdge (p0, p1, CODE_UV0, ssiTolP, eval0, eval1, true))
        {
#if defined (debug)
        if (debugStop)
            {
            printf ("\nStop march: on edge  uv0 = (%3.4f, %3.4f)", p1->uv0.x, p1->uv0.y);
            printf ("\n                     uv1 = (%3.4f, %3.4f)", p1->uv1.x, p1->uv1.y);
            }
#endif
        return STOPCODE_ON_EDGE;
        }

    if (bspssi_atEdge (p0, p1, CODE_UV1, ssiTolP, eval0, eval1, true))
        {
#if defined (debug)
        if (debugStop)
            {
            printf ("\nStop march: on edge  uv0 = (%3.4f, %3.4f)", p1->uv0.x, p1->uv0.y);
            printf ("\n                     uv1 = (%3.4f, %3.4f)", p1->uv1.x, p1->uv1.y);
            }
#endif
        return STOPCODE_ON_EDGE;
        }

    /* Check for standing still */
    if (length < ssiTolP->xyzSame || dist < ssiTolP->xyzSame)
        {
#if defined (debug)
        if (debugStop)
            printf ("\nStop march: in place. length = %3.4f, dist = %3.4f", length, dist);
#endif
        return STOPCODE_IN_PLACE;
        }

    /* Check current link for closure */
    if (curr->number)
        if ((dist = p1->xyz.Distance (*(curr->xyz))) < length)
            {
#if defined (debug)
            if (debugStop)
                {
                printf ("\nStop march: closing current link");
                }
#endif
            return STOPCODE_CLOSURE_LINK;
            }

    /* Check for reversal of marching */
    step.DifferenceOf (p1->xyz, p0->xyz);
    if ((dot = step.DotProduct (*lastStep)) < 0.0)
        {
#if defined (debug)
        if (debugStop)
            {
            printf ("\nStop march: reversing direction");
            }
#endif
        /* I should really half the stepLength and converge exactly on the degeneracy,
            but for now  just stop short. */
        return STOPCODE_ABOUT_FACE;
        }

    *lastStep = step;
    return STOPCODE_KEEP_GOING;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    12/91
+---------------+---------------+---------------+---------------+---------------+------*/
static double  bspssi_stepLength
(
SsiPt           *lastPt,
DPoint3d        *thisTan,
SsiPt           *thisPt,
double          degeneracy,
SsiTolerance    *ssiTolP
)
    {
    double      dist, theta, twiceRadius, step, dot;
    DPoint3d    diff;

    dist = diff.NormalizedDifference (thisPt->xyz, lastPt->xyz);

    if ((dot = fabs (diff.DotProduct (*thisTan))) > 1.0)
        theta = 0.0;
    else
        theta = acos (dot);

    if (theta < fc_1em15)    /* set maximum step length */
        return ssiTolP->maxStep;

    if ((twiceRadius = dist / sin (theta)) < ssiTolP->xyzTol)
        return 0.0;

    step = twiceRadius * sin (acos (1.0 - ssiTolP->xyzTol * 2.0 / twiceRadius));

    /* Limit maximum step size */
    if (step > ssiTolP->maxStep)
        step = ssiTolP->maxStep;

    /* Check degeneracy. Multiply by four to prevent slowing march too much too soon */
    if (degeneracy < DEGENERACY_TOLERANCE)
        step *= degeneracy * fc_4;

    return step;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             03/91
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspssi_ssiMarch
(
IntLink             *link,             /* <= intersection curve link */
SsiPt               *start,            /* => starting point for march */
DPoint3d            *startTan,         /* => tangent of intersection */
Evaluator           *eval0,
Evaluator           *eval1,
#if defined (BEZIER_ONLY_MARCH)
DRange2d           *range0,
DRange2d           *range1,
#endif
int                 direction,         /* => 1 for forward, -1 for reverse */
SsiTolerance        *ssiTolP,
int                 showMarch
)
    {
    int             status, done, bufSize=0;
    double          stepLength, degeneracy, zingerSize;
    DPoint3d        tangent, lastTan, lastDiff;
    SsiPt           currPt, approxPt, nextPt, buffer[MAX_BSPBATCH];

    memset (link, 0, sizeof(IntLink));
    buffer[bufSize++] = currPt = *start;
    tangent = *startTan;
    lastDiff.x = lastDiff.y = lastDiff.z = 0.0;

    /* set first step default length */
    stepLength = ssiTolP->xyzTol;
    zingerSize = ssiTolP->maxStep * 50.0;
    done = false;
    do
        {

#if defined (debug)
        if (debugStep)
            printf ("\nstepLength = %f", stepLength);
#endif
        lastTan = tangent;
        approxPt = currPt;
        approxPt.xyz.SumOf (currPt.xyz, tangent, direction * stepLength);

        if (STATUS_BADNORMAL ==
            (status = bspssi_getNextPt (&nextPt, &tangent, &degeneracy, &approxPt,
                                        ssiTolP, eval0, eval1, REV_LIMIT)))
            {
            done = STOPCODE_BADTANGENT;
#if defined (debug)
            if (debugStop)
            printf ("\nStop march: Bad tangent in getNextPt");
#endif
            break;
            }

        /* Check if done */
        if (STOPCODE_ZINGER ==
            (done = bspssi_finishedMarch (&currPt, &nextPt, link, stepLength, zingerSize,
                                          &lastTan, &tangent, &lastDiff, buffer, &bufSize,
                                          ssiTolP, eval0, eval1)))
            break;

#if defined (BEZIER_ONLY_MARCH)
        /* Adjust parameters for the entire B-spline surfaces. */
        nextPt.uv0.x = range0->org.x + range0->end.x * nextPt.uv0.x;
        nextPt.uv0.y = range0->org.y + range0->end.y * nextPt.uv0.y;

        nextPt.uv1.x = range1->org.x + range1->end.x * nextPt.uv1.x;
        nextPt.uv1.y = range1->org.y + range1->end.y * nextPt.uv1.y;
#endif
        /* Add point to intersection curve link */
        if (SUCCESS != (status = bspssi_addPt (&nextPt, buffer, &bufSize,
                                               &link->xyz, &link->norm0, &link->norm1,
                                               &link->uv0, &link->uv1, &link->number)))
            return status;

#ifdef debug
        /* Display this step */
        if (showMarch)
            {
            DPoint3d line[2];
            ElementUnion    u;
            line[0]=currPt.xyz;
            line[1]=nextPt.xyz;
            mdlLine_directCreate (&u, NULL, line, ACTIVEMODEL_IS_3D, MASTERFILE);
            u.line_3d.dhdr.symb.weight=1;
            mdlElement_display (&u, 2);
            }
#endif
        /* Get next step length */
        stepLength = bspssi_stepLength (&currPt, &tangent, &nextPt, degeneracy, ssiTolP);

        currPt = nextPt;
        }
    while (! done);

    /* Do any necessary clean up of the marching */
    switch (done)
        {
        case STOPCODE_CLOSURE_LINK:
            {
            /* Close this link completely */
            if (link->number)
                {
                nextPt.xyz = link->xyz[0];
                nextPt.uv0 = link->uv0[0];
                nextPt.uv1 = link->uv1[0];
                }
            else
                nextPt = buffer[0];

            /* Check the assignment of uv0 & uv1 as this might cross an edge. */
            bspssi_atEdge (&currPt, &nextPt, CODE_UV0, ssiTolP, eval0, eval1, false);
            bspssi_atEdge (&currPt, &nextPt, CODE_UV1, ssiTolP, eval0, eval1, false);

            if (SUCCESS != (status = bspssi_addPt (&nextPt, buffer, &bufSize,
                                                   &link->xyz, &link->norm0, &link->norm1,
                                                   &link->uv0, &link->uv1, &link->number)))
                return status;

#ifdef debug
            if (showMarch)
                {
                line[0]=currPt.xyz;
                line[1]=nextPt.xyz;
                mdlLine_directCreate (&u, NULL, line, ACTIVEMODEL_IS_3D, MASTERFILE);
                u.line_3d.dhdr.symb.weight=1;
                mdlElement_display (&u, 2);
                }
#endif
            break;
            }

        case STOPCODE_ZINGER:
        case STOPCODE_BADTANGENT:
        case STOPCODE_ABOUT_FACE:
        case STOPCODE_IN_PLACE:
            {
            /* Do not accept the last point. */
            if (bufSize)
                bufSize--;
            else if (link->number)
                link->number--;
            break;
            }

        case STOPCODE_ON_EDGE:
            if (link->number == 0 && bufSize == 2)
                {
                /* Only two points in link so check for marching on an edge. I should
                    actually check in more detail to make sure that both points are on
                    the same edge, but this will do for now. */
                if ((bsputil_onEdge (&buffer[0].uv0, ssiTolP->uvTol) &&
                     bsputil_onEdge (&buffer[1].uv0, ssiTolP->uvTol)) ||
                    (bsputil_onEdge (&buffer[0].uv1, ssiTolP->uvTol) &&
                     bsputil_onEdge (&buffer[1].uv1, ssiTolP->uvTol)))
                    bufSize = 0;
                }

            break;

        default:
            break;
        }

    /* flush the buffer of any remaining SSI points */
    if (SUCCESS != (status = bspssi_flushPts (buffer, bufSize,
                                              &link->xyz, &link->norm0, &link->norm1,
                                              &link->uv0, &link->uv1, &link->number)))
        return status;

    return done;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             04/91
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspssi_pointCovered
(
SsiPt           *pt,
IntLink         *chain,
SsiTolerance    *ssiTolP
)
    {
    double      dist2, tol2, tol;
    DPoint3d    *pP, *endP;
    DPoint3d    xyz0;
    IntLink     *chnP;

    /* OK to have a fairly big tolerance here as some other startpoint will be
        used if the two SSI loops are truly different. */
    tol = ssiTolP->maxStep;
    tol2 = tol * tol;
    for (chnP=chain; chnP; chnP = chnP->next)
        {
        for (pP=endP=chnP->xyz, endP += chnP->number - 1; pP < endP; pP++)
            {
            DSegment3d segment;
            bsiDSegment3d_initFromDPoint3d (&segment, pP, pP + 1);
            if (bsiDSegment3d_projectPointBounded (&segment, &xyz0, NULL, &pt->xyz))
                {
                dist2 = xyz0.DistanceSquared (pt->xyz);
                if (dist2 < tol2)
                    return true;
                }
            }
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             04/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspssi_addLink
(
IntLink         **chain,
IntLink         *link,
int             addToBackwards
)
    {
    int         i, newNum, alloc2dSize, alloc3dSize;
    DPoint2d    *uv0P, *uv1P, *tuv0P, *tuv1P;
    DPoint3d    *xyzP, *n0P, *n1P, *txyzP, *tn0P, *tn1P;
    IntLink     *chnP, tmpLink;

    if (link->number < 2)
        return SUCCESS;

    /* If addToBackwards == true then this is the reverse of the last link
        so append this link to the last link */
    if (addToBackwards)
        {
        /* Get to end of existing chain */
        for (chnP = *chain; chnP->next; chnP = chnP->next)
            ;

        memset (&tmpLink, 0, sizeof(tmpLink));
        tmpLink.last = chnP->last;
        tmpLink.number = chnP->number + link->number - 1;

        alloc2dSize = tmpLink.number * sizeof(DPoint2d);
        alloc3dSize = tmpLink.number * sizeof(DPoint3d);
        if (NULL == (tmpLink.xyz   = (DPoint3d*)msbspline_malloc (alloc3dSize, HEAPSIG_BSSI)) ||
            NULL == (tmpLink.norm0 = (DPoint3d*)msbspline_malloc (alloc3dSize, HEAPSIG_BSSI)) ||
            NULL == (tmpLink.norm1 = (DPoint3d*)msbspline_malloc (alloc3dSize, HEAPSIG_BSSI)) ||
            NULL == (tmpLink.uv0   = (DPoint2d*)msbspline_malloc (alloc2dSize, HEAPSIG_BSSI)) ||
            NULL == (tmpLink.uv1   = (DPoint2d*)msbspline_malloc (alloc2dSize, HEAPSIG_BSSI)))
            {
            if (tmpLink.xyz)    msbspline_free (tmpLink.xyz);
            if (tmpLink.norm0)  msbspline_free (tmpLink.norm0);
            if (tmpLink.norm1)  msbspline_free (tmpLink.norm1);
            if (tmpLink.uv0)    msbspline_free (tmpLink.uv0);
            if (tmpLink.uv1)    msbspline_free (tmpLink.uv1);
            return ERROR;
            }

        newNum = link->number - 1;
        for (i=1,
             xyzP=link->xyz+newNum, n0P=link->norm0+newNum, n1P=link->norm1+newNum,
             uv0P=link->uv0+newNum, uv1P=link->uv1+newNum,
             txyzP=tmpLink.xyz, tn0P=tmpLink.norm0, tn1P=tmpLink.norm1,
             tuv0P=tmpLink.uv0, tuv1P=tmpLink.uv1;
             i < link->number; i++, xyzP--, n0P--, n1P--, uv0P--, uv1P--,
             txyzP++, tn0P++, tn1P++, tuv0P++, tuv1P++)
            {
            *txyzP = *xyzP; *tn0P = *n0P; *tn1P = *n1P; *tuv0P = *uv0P; *tuv1P = *uv1P;
            }

        alloc3dSize = chnP->number * sizeof(DPoint3d);
        alloc2dSize = chnP->number * sizeof(DPoint2d);
        memcpy (txyzP, chnP->xyz,   alloc3dSize);
        memcpy (tn0P,  chnP->norm0, alloc3dSize);
        memcpy (tn1P,  chnP->norm1, alloc3dSize);
        memcpy (tuv0P, chnP->uv0,   alloc2dSize);
        memcpy (tuv1P, chnP->uv1,   alloc2dSize);

        /* Free old link and replace with new combined link */
        if (chnP->xyz)      msbspline_free (chnP->xyz);
        if (chnP->norm0)    msbspline_free (chnP->norm0);
        if (chnP->norm1)    msbspline_free (chnP->norm1);
        if (chnP->uv0)      msbspline_free (chnP->uv0);
        if (chnP->uv1)      msbspline_free (chnP->uv1);
        memcpy (chnP, &tmpLink, sizeof(IntLink));

        /* Free link since the information was copied */
        if (link->xyz)      msbspline_free (link->xyz);
        if (link->norm0)    msbspline_free (link->norm0);
        if (link->norm1)    msbspline_free (link->norm1);
        if (link->uv0)      msbspline_free (link->uv0);
        if (link->uv1)      msbspline_free (link->uv1);
        }
    else
        {
        /* Add link to chain */
        if (! *chain)
            {
            if (NULL == (*chain = (IntLink*)msbspline_malloc (sizeof(IntLink), HEAPSIG_BSSI)))
                return ERROR;
            memcpy (*chain, link, sizeof(IntLink));
            (*chain)->last = NULL;
            (*chain)->next = NULL;
            }
        else
            {
            for (chnP = *chain; chnP->next; chnP = chnP->next)
                ;
            if (NULL == (chnP->next = (IntLink*)msbspline_malloc (sizeof(IntLink), HEAPSIG_BSSI)))
                return ERROR;
            memcpy (chnP->next, link, sizeof(IntLink));
            chnP->next->last = chnP;
            chnP->next->next = NULL;
            }
#if defined (debug)
        if ( addLink)
            {
            printf ("\nADDING LINK : number in link = %d\n", link->number);
            printf ("before = %d        after = %d\n", before, after);
            before=after=0;
            }
#endif
        }

    /* clear the old link array pointers */
    link->xyz = link->norm0 = link->norm1 = NULL;
    link->uv0 = link->uv1 = NULL;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             04/91
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspssi_processStartPt
(
IntLink         **chain,
SsiPt           *start,
Evaluator       *eval0,
Evaluator       *eval1,
#if defined (BEZIER_ONLY_MARCH)
DRange2d       *range0,
DRange2d       *range1,
#endif
SsiTolerance    *ssiTolP,
int             showMarch
)
    {
    int         i, status, code, resultsFromBackwards;
    DPoint3d    tangent;
    SsiPt       pt;
    IntLink     link;

    /* If we do not converge then don't bother marching from this
        points as it is a problem to begin with. */
    if (STATUS_CONVERGED != (status = bspssi_getNextPt (&pt, &tangent, NULL,
                             start, ssiTolP, eval0, eval1, 32)))
        {
        return SUCCESS;
        }

#if defined (debug)
    if (debugStartPt)
        {
        DPoint3d        line[2];
        ElementUnion    u;

        line[0]=line[1]=pt.xyz;
        mdlLine_directCreate (&u, NULL, line, NULL);
        u.line_3d.dhdr.symb.b.weight=5;
        mdlElement_display (&u, ERASE);
        }
#endif
    if (! bspssi_pointCovered (&pt, *chain, ssiTolP))
        {
        /* march both directions */
        for (i = resultsFromBackwards = 0; i < 2; i++, resultsFromBackwards = link.number)
            {
            if ((code = bspssi_ssiMarch (&link, &pt, &tangent, eval0, eval1,
#if defined (BEZIER_ONLY_MARCH)
                                         range0, range1,
#endif
                                         (i ? -1 : 1),
                                         ssiTolP, showMarch)) == ERROR)
                return ERROR;

            if (SUCCESS != (status = bspssi_addLink (chain, &link, resultsFromBackwards)))
                return status;

            if (code == STOPCODE_CLOSURE_LINK)
                return SUCCESS;    /* skip other direction */
            }
        }
    return SUCCESS;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             03/91
+---------------+---------------+---------------+---------------+---------------+------*/
static int      bspssi_ssiStopBez
(
BezierInfo          *infoP,
MSBsplineSurface    *bez0,
MSBsplineSurface    *bez1
)
    {
    DPoint2d    delta0, delta1;
    DPoint3d    tmp, center0, center1;
    SsiPt       stPt;
    Evaluator   eval0, eval1;


     if (bound_boxFlat (&infoP->ssi.b0, infoP->ssi.tol->xyzTol) &&
         bound_boxFlat (&infoP->ssi.b1, infoP->ssi.tol->xyzTol))
        {
#if defined (debug)
        if (displayBox)
            {
            bound_boxDisplay (&infoP->ssi.b0, 2);
            bound_boxDisplay (&infoP->ssi.b1, 2);
            }
#endif
        tmp = infoP->ssi.b0.extent;
        (((RotMatrix *) &infoP->ssi.b0.system))->MultiplyTranspose (tmp);
        center0.SumOf (*(&infoP->ssi.b0.origin), tmp, 0.5);
        tmp = infoP->ssi.b1.extent;
        (((RotMatrix *) &infoP->ssi.b1.system))->MultiplyTranspose (tmp);
        center1.SumOf (*(&infoP->ssi.b1.origin), tmp, 0.5);

        eval0 = *infoP->ssi.eval0;   eval0.surf = bez0;
        eval1 = *infoP->ssi.eval1;   eval1.surf = bez1;

        stPt.xyz.Interpolate (center0, 0.5, center1);

        /* Relax the start point to these Bezier patches. This helps in case the full surface
            has some sort of degeneracy; the patches are not degenerate in these cases. */
        stPt.uv0.x = stPt.uv0.y = stPt.uv1.x = stPt.uv1.y = 0.5;
        bspssi_relaxToSurface (&center0, &stPt.norm0, NULL, &stPt.uv0,
                               &stPt.xyz, &eval0, 10, infoP->ssi.tol->uvSame);
        bspssi_relaxToSurface (&center1, &stPt.norm1, NULL, &stPt.uv1,
                               &stPt.xyz, &eval1, 10, infoP->ssi.tol->uvSame);
        stPt.xyz.Interpolate (center0, 0.5, center1);

#if defined (BEZIER_ONLY_MARCH)
        globalBezierRange0.high.x = globalBezierRange0.high.x - globalBezierRange0.low.x;
        globalBezierRange0.high.y = globalBezierRange0.high.y - globalBezierRange0.low.y;
        globalBezierRange1.high.x = globalBezierRange1.high.x - globalBezierRange1.low.x;
        globalBezierRange1.high.y = globalBezierRange1.high.y - globalBezierRange1.low.y;
#else
        /* Adjust parameters for the entire B-spline surfaces. */
        delta0.x = infoP->ssi.range0.high.x - infoP->ssi.range0.low.x;
        delta0.y = infoP->ssi.range0.high.y - infoP->ssi.range0.low.y;
        delta1.x = infoP->ssi.range1.high.x - infoP->ssi.range1.low.x;
        delta1.y = infoP->ssi.range1.high.y - infoP->ssi.range1.low.y;

        stPt.uv0.x = infoP->ssi.range0.low.x + delta0.x * stPt.uv0.x;
        stPt.uv0.y = infoP->ssi.range0.low.y + delta0.y * stPt.uv0.y;

        stPt.uv1.x = infoP->ssi.range1.low.x + delta1.x * stPt.uv1.x;
        stPt.uv1.y = infoP->ssi.range1.low.y + delta1.y * stPt.uv1.y;
#endif

        bspssi_processStartPt (infoP->ssi.chainPP, &stPt,
                               infoP->ssi.eval0, infoP->ssi.eval1,
#if defined (BEZIER_ONLY_MARCH)
                               &globalBezierRange0, &globalBezierRange1,
#endif
                               infoP->ssi.tol, infoP->ssi.showMarch);

#if defined (debug)
        if (displayBox)
            {
            bound_boxDisplay (&infoP->ssi.b0, 1);
            bound_boxDisplay (&infoP->ssi.b1, 1);
            }
#endif
        return true;
        }
    else
        return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             03/91
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspssi_ssiSortBez
(
int                 *tags,
BezierInfo          **infoPtr,
MSBsplineSurface    **bezPtr0,
MSBsplineSurface    **bezPtr1
)
    {
    int             i, j, *tg, indx, iX4;
    Evaluator       eval0, eval1;


#if defined (needs_work)
    /* Would double indexing infoPtr array be faster? */
#endif
    for (i=indx=iX4=0, tg=tags; i < 4; i++, iX4 += 4)
        {
        eval0      = *(infoPtr[i]->ssi.eval0);
        eval0.surf = bezPtr0[i];
        eval1      = *(infoPtr[i]->ssi.eval1);
        eval1.surf = bezPtr1[i];

        bound_boxFromEval (&infoPtr[i]->ssi.b0,   &eval0);
        bound_boxFromEval (&infoPtr[iX4]->ssi.b1, &eval1);

        for (j=0; j < 4; j++, indx++, tg++)
            {
            infoPtr[j*4+i]->ssi.b0 = infoPtr[i]->ssi.b0;
            infoPtr[iX4+j]->ssi.b1 = infoPtr[iX4]->ssi.b1;
            *tg = indx;
            }
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             03/91
+---------------+---------------+---------------+---------------+---------------+------*/
static int      bspssi_ssiGoBez
(
BezierInfo          *infoP,
MSBsplineSurface    *bez0,
MSBsplineSurface    *bez1,
int                 sub0,
int                 sub1
)
    {
    bool    code;

#if defined (debug)
        if (displayBox)
            {
            bound_boxDisplay (&infoP->ssi.b0, 0);
            bound_boxDisplay (&infoP->ssi.b1, 0);
            }
#endif
    if (true == (code = bound_boxesIntersect (&infoP->ssi.b0, &infoP->ssi.b1)))
        {
        bsputil_selectQuarterPatch (&infoP->ssi.range0, sub0, (DRange2d*)&infoP->ssi.range0);
        bsputil_selectQuarterPatch (&infoP->ssi.range1, sub1, (DRange2d*)&infoP->ssi.range1);
        }
#if defined (debug)
        if (displayBox)
            {
            bound_boxDisplay (&infoP->ssi.b0, 1);
            bound_boxDisplay (&infoP->ssi.b1, 1);
            }
#endif
    return code;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             10/90
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    bspssi_flatSurfaceLineIntersect
(
double              *distance,
DPoint2d            *uvPoints,
DPoint3d            *linePoint,
DPoint3d            *normal,
DPoint3d            *surfaceNormal,
MSBsplineSurface    *surfP
)
    {
    int         i, iNext, n=0;
    static int  nextPole[4] = {1, 3, 0, 2};
    double      y[4], x, xNext, yMin, yMax, u, uNext, v, vNext, tmp;
    DPoint3d    delta[4], yVector;

    /* Compute vector perp to line and parallel to surface */
    yVector.CrossProduct (*normal, *surfaceNormal);
    yVector.Normalize ();

    for (i=0; i<4; i++)
        {
        delta[i].DifferenceOf (*(&surfP->poles[i]), *linePoint);
        y[i] = delta[i].DotProduct (yVector);
        }

    for (i=0; i<4; i++)
        {
        iNext = nextPole[i];
        if (y[i] < y[iNext])
            {
            yMin = y[i];
            yMax = y[iNext];
            }
        else
            {
            yMin = y[iNext];
            yMax = y[i];
            }
        if (yMin < -INTERSECTION_TOLERANCE && yMax > INTERSECTION_TOLERANCE)
            {
            x     =  delta[i].DotProduct (*normal);
            xNext =  delta[iNext].DotProduct (*normal);
            u     = (double) (i & 0x01);
            uNext = (double) (iNext & 0x01);
            v     = (double) (i >> 1);
            vNext = (double) (iNext >> 1);

            tmp = y[i] / (y[iNext] - y[i]);
            distance[n]   = x - tmp * (xNext - x);
            uvPoints[n].x = u - tmp * (uNext - u);
            uvPoints[n].y = v - tmp * (vNext - v);
            n++;
            }
        }
    return (n == 2 ? true : false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             09/91
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bspssi_interpolateFlatSurfParameter
(
DPoint2d            *uvResult,
double              distance,
DPoint2d            *uvOrigin,
double              distanceOrigin,
DPoint2d            *uvDelta,
double              distanceDelta,
int                 uStart,         /* => knot offset of bez0 in U */
int                 vStart,         /* => knot offset of bez0 in V */
double              *uKnotP,
double              *vKnotP
)
    {
    double      scale;
    DPoint2d    uv;

    uv = *uvOrigin;
    if (distanceDelta > 0.0)
        {
        scale = (distance - distanceOrigin)/distanceDelta;
        uv.x += uvDelta->x * scale;
        uv.y += uvDelta->y * scale;
        }
    uvResult->x = bspproc_setParameter (uv.x, uStart, uKnotP);
    uvResult->y = bspproc_setParameter (uv.y, vStart, vKnotP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             09/91
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspssi_ssiFlatSurfaces
(
MSBsplineSurface    *surf0P,
BoundBox            *box0P,
MSBsplineSurface    *surf1P,
BoundBox            *box1P,
BezierInfo          *infoP,
int                 uStart0,        /* => knot offset of bez0 in U */
int                 vStart0,        /* => knot offset of bez0 in V */
int                 uStart1,        /* => knot offset of bez1 in U */
int                 vStart1         /* => knot offset of bez1 in V */
)
    {
    double      d0[4], d1[4], d0Delta, d1Delta;
    DPoint2d    uv0Points[4], uv1Points[4], uv0Delta, uv1Delta;
    DVec3d      normal0, normal1, intNormal;
    DPoint3d    intPoint;
    SsiPt       ssiPts[4];
    IntLink     link;

    memset (&link, 0, sizeof(IntLink));
    bsiRotMatrix_getRow ( (RotMatrix *) &box0P->system, &normal0,  2);
    bsiRotMatrix_getRow ( (RotMatrix *) &box1P->system, &normal1,  2);

    if (!bsiGeom_planePlaneIntersection (&intPoint, &intNormal, &box0P->origin, &normal0,
                                       &box1P->origin, &normal1))
        return false;

    if (bspssi_flatSurfaceLineIntersect (d0, uv0Points, &intPoint, &intNormal, &normal0,
                                         surf0P) &&
        bspssi_flatSurfaceLineIntersect (d1, uv1Points, &intPoint, &intNormal, &normal1,
                                         surf1P))
        {
        if (d0[0] > d0[1])
            {
            DoubleOps::Swap (d0[0], d0[1]);
            uv0Points[0].Swap (uv0Points[1]);
            }
        if (d1[0] > d1[1])
            {
            DoubleOps::Swap (d1[0], d1[1]);
            uv1Points[0].Swap (uv1Points[1]);
            }
        if (d0[1] > d1[0] + INTERSECTION_TOLERANCE &&
            d0[0] < d1[1] - INTERSECTION_TOLERANCE)
            {
            ssiPts[0].norm0 = ssiPts[1].norm0 = normal0;
            ssiPts[0].norm1 = ssiPts[1].norm1 = normal1;

            uv0Delta.x = uv0Points[1].x - uv0Points[0].x;
            uv0Delta.y = uv0Points[1].y - uv0Points[0].y;
            d0Delta    = d0[1] - d0[0];

            uv1Delta.x = uv1Points[1].x - uv1Points[0].x;
            uv1Delta.y = uv1Points[1].y - uv1Points[0].y;
            d1Delta    = d1[1] - d1[0];
            memset (&link, 0, sizeof(link));
            if (d0[0] > d1[0])
                {
                ssiPts[0].uv0.x = bspproc_setParameter (uv0Points[0].x, uStart0,
                                                        infoP->ssi.uKts0);
                ssiPts[0].uv0.y = bspproc_setParameter (uv0Points[0].y, vStart0,
                                                        infoP->ssi.vKts0);
                bspssi_interpolateFlatSurfParameter (&ssiPts[0].uv1, d0[0],
                                                     &uv1Points[0],  d1[0],
                                                     &uv1Delta, d1Delta,
                                                     uStart1, vStart1,
                                                     infoP->ssi.uKts1, infoP->ssi.vKts1);

                ssiPts[0].xyz.SumOf (intPoint, intNormal, d0[0]);
                }
            else
                {
                ssiPts[0].uv1.x = bspproc_setParameter (uv1Points[0].x, uStart1,
                                                        infoP->ssi.uKts1);
                ssiPts[0].uv1.y = bspproc_setParameter (uv1Points[0].y, vStart1,
                                                        infoP->ssi.vKts1);
                bspssi_interpolateFlatSurfParameter (&ssiPts[0].uv0, d1[0],
                                                     &uv0Points[0], d0[0],
                                                     &uv0Delta, d0Delta,
                                                     uStart0, vStart0,
                                                     infoP->ssi.uKts0, infoP->ssi.vKts0);
                ssiPts[0].xyz.SumOf (intPoint, intNormal, d1[0]);
                }
            if (d0[1] < d1[1])
                {
                ssiPts[1].uv0.x = bspproc_setParameter (uv0Points[1].x, uStart0,
                                                        infoP->ssi.uKts0);
                ssiPts[1].uv0.y = bspproc_setParameter (uv0Points[1].y, vStart0,
                                                        infoP->ssi.vKts0);
                bspssi_interpolateFlatSurfParameter (&ssiPts[1].uv1, d0[1],
                                                     &uv1Points[0], d1[0],
                                                     &uv1Delta, d1Delta,
                                                     uStart1, vStart1,
                                                     infoP->ssi.uKts1, infoP->ssi.vKts1);


                ssiPts[1].xyz.SumOf (intPoint, intNormal, d0[1]);
                }
            else
                {
                ssiPts[1].uv1.x = bspproc_setParameter (uv1Points[1].x, uStart1,
                                                        infoP->ssi.uKts1);
                ssiPts[1].uv1.y = bspproc_setParameter (uv1Points[1].y, vStart1,
                                                        infoP->ssi.vKts1);
                bspssi_interpolateFlatSurfParameter (&ssiPts[1].uv0, d1[1],
                                                     &uv0Points[0], d0[0],
                                                     &uv0Delta, d0Delta,
                                                     uStart0, vStart0,
                                                     infoP->ssi.uKts0, infoP->ssi.vKts0);
                ssiPts[1].xyz.SumOf (intPoint, intNormal, d1[1]);
                }
            bspssi_flushPts (ssiPts, 2, &link.xyz, &link.norm0, &link.norm1,
                             &link.uv0, &link.uv1, &link.number);
            bspssi_addLink (infoP->ssi.chainPP, &link, false);
            }
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    02/92
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspssi_ssiSort
(
int                 *rank,
BezierInfo          *infoP,
int                 *uStart0,
int                 *vStart0,
int                 *uStart1,
int                 *vStart1,
int                 uNumSegs0,
int                 vNumSegs0,
int                 uNumSegs1,
int                 vNumSegs1,
MSBsplineSurface    *surface0,
MSBsplineSurface    *surface1
)
    {
#if defined (needs_work)
    /* When bspproc_doubleProcessBsplineSurface starts looking at the tags array
        this routine will need to assign values to rank */
#endif

    infoP->ssi.uKts0 = surface0->uKnots;
    infoP->ssi.vKts0 = surface0->vKnots;
    infoP->ssi.uKts1 = surface1->uKnots;
    infoP->ssi.vKts1 = surface1->vKnots;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             03/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int        bspssi_ssiGo
(
BezierInfo          *infoP,
MSBsplineSurface    *bez0,
MSBsplineSurface    *bez1,
int                 uSeg0,
int                 vSeg0,
int                 uSeg1,
int                 vSeg1,
int                 uStart0,        /* => knot offset of bez0 in U */
int                 vStart0,        /* => knot offset of bez0 in V */
int                 uStart1,        /* => knot offset of bez1 in U */
int                 vStart1,        /* => knot offset of bez1 in V */
int                 numU0,
int                 numV0,
int                 numU1,
int                 numV1
)
    {
    int         code;
    BoundBox    b0, b1;
    Evaluator   eval0, eval1;

    eval0      = *infoP->ssi.eval0;
    eval0.surf = bez0;
    eval1      = *infoP->ssi.eval1;
    eval1.surf = bez1;

    bound_boxFromEval (&b0, &eval0);
    bound_boxFromEval (&b1, &eval1);

#if defined (FLAT_SURFACE_SSI)
    if (bez0->uParams.order == 2 && bez0->vParams.order == 2 && b0.extent.z <= 1.01 &&
        bez1->uParams.order == 2 && bez1->vParams.order == 2 && b1.extent.z <= 1.01)
        {
        return bspssi_ssiFlatSurfaces (bez0, &b0, bez1, &b1,
                                       infoP, uStart0, vStart0, uStart1, vStart1);
        }
#endif

    if (true == (code = bound_boxesIntersect (&b0, &b1)))
        {
        infoP->ssi.range0.low.x = bspproc_setParameter (0.0, uStart0, infoP->ssi.uKts0);
        infoP->ssi.range0.low.y = bspproc_setParameter (0.0, vStart0, infoP->ssi.vKts0);
        infoP->ssi.range0.high.x = bspproc_setParameter (1.0,    uStart0, infoP->ssi.uKts0);
        infoP->ssi.range0.high.y = bspproc_setParameter (1.0,    vStart0, infoP->ssi.vKts0);

        infoP->ssi.range1.low.x = bspproc_setParameter (0.0, uStart1, infoP->ssi.uKts1);
        infoP->ssi.range1.low.y = bspproc_setParameter (0.0, vStart1, infoP->ssi.vKts1);
        infoP->ssi.range1.high.x = bspproc_setParameter (1.0,    uStart1, infoP->ssi.uKts1);
        infoP->ssi.range1.high.y = bspproc_setParameter (1.0,    vStart1, infoP->ssi.vKts1);
        }
    return code;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             03/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspssi_ssiBezier
(
BezierInfo          *infoP,
MSBsplineSurface    *bez0,
MSBsplineSurface    *bez1
)
    {
    Evaluator       eval0, eval1;


    eval0      = *infoP->ssi.eval0;
    eval0.surf = bez0;
    eval1      = *infoP->ssi.eval1;
    eval1.surf = bez1;

    bound_boxFromEval (&infoP->ssi.b0, &eval0);
    bound_boxFromEval (&infoP->ssi.b1, &eval1);

#if defined (BEZIER_ONLY_MARCH)
    infoP->ssi.eval0 = &eval0;
    infoP->ssi.eval1 = &eval1;

    globalBezierRange0 = infoP->ssi.range0;
    globalBezierRange1 = infoP->ssi.range1;
#endif

    return bspproc_doubleProcessBezierPatch (infoP, bez0, bez1,
                                             bspssi_ssiStopBez,
                                             bspssi_ssiSortBez,
                                             bspssi_ssiGoBez,
                                             NULLFUNC);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             07/91
+---------------+---------------+---------------+---------------+---------------+------*/
static int      bspssi_linkClosed
(
IntLink         *link,
IntLink         *chain,
int             code,
SsiTolerance    *ssiTolP
)
    {
    int         last = link->number - 1;

    switch (code)
        {
        case CODE_UV0:
            if (bsputil_onEdge (link->uv0, ssiTolP->uvTol) &&
                bsputil_onEdge (link->uv0 + last, ssiTolP->uvTol))
                return CLOSED_ON_EDGE;
            else if (link->uv0->Distance (*(link->uv0 + last)) < ssiTolP->uvTol)
                return CLOSED_TRUE_CLOSURE;
            else
                return false;
        case CODE_UV1:
            if (bsputil_onEdge (link->uv1, ssiTolP->uvTol) &&
                bsputil_onEdge (link->uv1 + last, ssiTolP->uvTol))
                return CLOSED_ON_EDGE;
            else if (link->uv1->Distance (*(link->uv1 + last)) < ssiTolP->uvTol)
                return CLOSED_TRUE_CLOSURE;
            else
                return false;
        case CODE_XYZ:
            if (link->xyz->Distance (*(link->xyz + last)) < ssiTolP->xyzSame)
                return CLOSED_TRUE_CLOSURE;
            else
                return false;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             07/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void     bspssi_freeLink
(
IntLink         **linkPP
)
    {
    IntLink     *tmpP, *endP=NULL;

    for (tmpP = *linkPP; tmpP; tmpP = tmpP->next)
        {
        endP = tmpP;
        if (tmpP->uv0)          msbspline_free (tmpP->uv0);
        if (tmpP->uv1)          msbspline_free (tmpP->uv1);
        if (tmpP->xyz)          msbspline_free (tmpP->xyz);
        if (tmpP->norm0)        msbspline_free (tmpP->norm0);
        if (tmpP->norm1)        msbspline_free (tmpP->norm1);
        if (tmpP->last)         msbspline_free (tmpP->last);
        }

    if (endP)                   msbspline_free (endP);
    *linkPP = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             06/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspssi_extractLinks
(
IntLink             **out,               /* <= link of only one uv0, uv1, or xyz */
IntLink             *chain,              /* => chain containing all info */
int                 code,                /* => choice of uv0, uv1, or xyz */
SsiTolerance        *ssiTolP,
MSBsplineSurface    *surf0,
MSBsplineSurface    *surf1
)
    {
    int         last, status=ERROR, allocSize;
    IntLink     *chnP, *tmpP, *endP = NULL;

    *out = NULL;
    for (chnP=chain; chnP; chnP = chnP->next)
        {
        /* Check that this is a relevant link. (for calls from bspbool.c) */
        if (code == CODE_UV0 && surf0 && surf0 != chnP->surf0 ||
            code == CODE_UV1 && surf1 && surf1 != chnP->surf1)
            continue;

        if (NULL == (tmpP = (IntLink*)msbspline_malloc (sizeof(IntLink), HEAPSIG_BSSI)))
            goto wrapup;
        memset (tmpP, 0, sizeof(IntLink));

        last = (tmpP->number = chnP->number) - 1;

        switch (code)
            {
            case CODE_UV0:
                allocSize = tmpP->number * sizeof(DPoint2d);
                if (NULL == (tmpP->uv0 = (DPoint2d*)msbspline_malloc (allocSize, HEAPSIG_BSSI)))
                    goto wrapup;
                memcpy (tmpP->uv0, chnP->uv0, allocSize);
                break;
            case CODE_UV1:
                allocSize = tmpP->number * sizeof(DPoint2d);
                if (NULL == (tmpP->uv1 = (DPoint2d*)msbspline_malloc (allocSize, HEAPSIG_BSSI)))
                    goto wrapup;
                memcpy (tmpP->uv1, chnP->uv1, allocSize);
                break;
            case CODE_XYZ:
                allocSize = tmpP->number * sizeof(DPoint3d);
                if (NULL == (tmpP->xyz = (DPoint3d*)msbspline_malloc (allocSize, HEAPSIG_BSSI)))
                    goto wrapup;
                memcpy (tmpP->xyz, chnP->xyz, allocSize);
                break;
            }

        tmpP->closed = bspssi_linkClosed (tmpP, chnP, code, ssiTolP);

        if (! *out)
            *out = tmpP;
        else
            {
            endP->next = tmpP;
            endP->next->last = endP;
            }

        endP = tmpP;
        }
    status = SUCCESS;

wrapup:
    if (status)     bspssi_freeLink (out);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             07/91
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bspssi_copyLink
(
IntLink         *to,
int             offsetTo,
IntLink         *from,
int             offsetFrom,
int             number
)
    {
    if (to->uv0)
        memcpy (to->uv0 + offsetTo, from->uv0 + offsetFrom, number * sizeof(DPoint2d));

    if (to->uv1)
        memcpy (to->uv1 + offsetTo, from->uv1 + offsetFrom, number * sizeof(DPoint2d));

    if (to->xyz)
        memcpy (to->xyz + offsetTo, from->xyz + offsetFrom, number * sizeof(DPoint3d));

    if (to->norm0)
        memcpy (to->norm0 + offsetTo, from->norm0 + offsetFrom, number * sizeof(DPoint3d));

    if (to->norm1)
        memcpy (to->norm1 + offsetTo, from->norm1 + offsetFrom, number * sizeof(DPoint3d));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             11/91
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bspssi_reverseCopy
(
Byte *to,
Byte *from,
int             number,
int             size
)
    {
    Byte *fromP, *toP;

    for (toP = to, fromP = from + (number - 1) * size;
         fromP >= from;
         toP += size, fromP -= size)
        memcpy (toP, fromP, size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             06/91
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bspssi_reverseLink
(
IntLink         *newLink,
IntLink         *old
)
    {
    if (newLink->uv0)   bspssi_reverseCopy ((Byte *)newLink->uv0, (Byte *)old->uv0,
                                        old->number, sizeof(DPoint2d));
    if (newLink->uv1)   bspssi_reverseCopy ((Byte *)newLink->uv1, (Byte *)old->uv1,
                                        old->number, sizeof(DPoint2d));
    if (newLink->xyz)   bspssi_reverseCopy ((Byte *)newLink->xyz, (Byte *)old->xyz,
                                        old->number, sizeof(DPoint3d));
    if (newLink->norm0) bspssi_reverseCopy ((Byte *)newLink->norm0, (Byte *)old->norm0,
                                        old->number, sizeof(DPoint3d));
    if (newLink->norm1) bspssi_reverseCopy ((Byte *)newLink->norm1, (Byte *)old->norm1,
                                        old->number, sizeof(DPoint3d));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             06/91
+---------------+---------------+---------------+---------------+---------------+------*/
static void    bspssi_removeLink
(
IntLink         *link,
IntLink         **next,
IntLink         **start
)
    {
    if (*next == link)
        *next = link->next;
    if (*start == link)
        *start = link->next;

    if (link->last)
        link->last->next = link->next;
    if (link->next)
        link->next->last = link->last;

    if (link->uv0)    msbspline_free (link->uv0);     link->uv0=NULL;
    if (link->uv1)    msbspline_free (link->uv1);     link->uv1=NULL;
    if (link->xyz)    msbspline_free (link->xyz);     link->xyz=NULL;
    if (link->norm0)  msbspline_free (link->norm0);   link->norm0=NULL;
    if (link->norm1)  msbspline_free (link->norm1);   link->norm0=NULL;
    if (link)         msbspline_free (link);        link=NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             05/91
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspssi_appendLinks
(
IntLink         *link0,
IntLink         *link1,
IntLink         **start,
IntLink         **end,
IntLink         **next,
int             appendCode,                    /* => how to do the append */
int             linkCode,
SsiTolerance    *ssiTolP
)
    {
    int         last, status=SUCCESS;
    IntLink     *newLink=NULL;

    if (NULL == (newLink = (IntLink*)msbspline_malloc (sizeof(IntLink), HEAPSIG_BSSI)))
        return ERROR;
    memset (newLink, 0, sizeof(IntLink));

    last = (newLink->number = link0->number + link1->number - 1) - 1;

    if (link0->uv0 &&
        NULL == (newLink->uv0 = (DPoint2d*)msbspline_malloc (newLink->number * sizeof(DPoint2d),
                                                HEAPSIG_BSSI)))
        status = ERROR;

    if (link0->uv1 &&
        NULL == (newLink->uv1 = (DPoint2d*)msbspline_malloc (newLink->number * sizeof(DPoint2d),
                                                HEAPSIG_BSSI)))
        status = ERROR;

    if (link0->xyz &&
        NULL == (newLink->xyz = (DPoint3d*)msbspline_malloc (newLink->number * sizeof(DPoint3d),
                                                HEAPSIG_BSSI)))
        status = ERROR;

    if (link0->norm0 &&
        NULL == (newLink->norm0 = (DPoint3d*)msbspline_malloc (newLink->number * sizeof(DPoint3d),
                                                  HEAPSIG_BSSI)))
        status = ERROR;

    if (link0->norm1 &&
        NULL == (newLink->norm1 = (DPoint3d*)msbspline_malloc (newLink->number * sizeof(DPoint3d),
                                                  HEAPSIG_BSSI)))
        status = ERROR;

    if (status)
        {
        if (newLink->uv0)   msbspline_free (newLink->uv0);
        if (newLink->uv1)   msbspline_free (newLink->uv1);
        if (newLink->xyz)   msbspline_free (newLink->xyz);
        if (newLink->norm0) msbspline_free (newLink->norm0);
        if (newLink->norm1) msbspline_free (newLink->norm1);
        if (newLink)        msbspline_free (newLink);
        return status;
        }

    switch (appendCode)
        {
        case APPEND_ORG_ORG:
            /* reverse link0, append link1, add to end, delete both */
            bspssi_reverseLink (newLink, link0);
            bspssi_copyLink (newLink, link0->number-1, link1, 0, link1->number);

            (*end)->next = newLink;
            newLink->last = *end;
            *end = newLink;

            *next = link0->next;
            bspssi_removeLink (link0, next, start);
            bspssi_removeLink (link1, next, start);
            break;
        case APPEND_END_ORG:
            /* append link1, add to end, delete both */
            bspssi_copyLink (newLink, 0, link0, 0, link0->number);
            bspssi_copyLink (newLink, link0->number-1, link1, 0, link1->number);

            (*end)->next = newLink;
            newLink->last = *end;
            *end = newLink;

            *next = link0->next;
            bspssi_removeLink (link0, next, start);
            bspssi_removeLink (link1, next, start);
            break;
        case APPEND_ORG_END:
            /* append link0 to link1, delete link0 */
            bspssi_copyLink (newLink, 0, link1, 0, link1->number);
            bspssi_copyLink (newLink, link1->number-1, link0, 0, link0->number);

            if (NULL != (newLink->next = link1->next))
                newLink->next->last = newLink;
            if (NULL != (newLink->last = link1->last))
                newLink->last->next = newLink;

            if (link1->uv0)    msbspline_free (link1->uv0);
            if (link1->uv1)    msbspline_free (link1->uv1);
            if (link1->xyz)    msbspline_free (link1->xyz);
            if (link1->norm0)  msbspline_free (link1->norm0);
            if (link1->norm1)  msbspline_free (link1->norm1);
            if (link1)         msbspline_free (link1);

            if (*end == link1)
                *end = newLink;
            *next = link0->next;
            bspssi_removeLink (link0, next, start);
            break;
        case APPEND_END_END:
            /* reverse link0, append to link1, delete link0 */
            bspssi_reverseLink (newLink, link0);
            bspssi_copyLink (link0, 0, newLink, 0, link0->number);
            bspssi_copyLink (newLink, 0, link1, 0, link1->number);
            bspssi_copyLink (newLink, link1->number-1, link0, 0, link0->number);

            if (NULL != (newLink->next = link1->next))
                newLink->next->last = newLink;
            if (NULL != (newLink->last = link1->last))
                newLink->last->next = newLink;

            if (link1->uv0)    msbspline_free (link1->uv0);
            if (link1->uv1)    msbspline_free (link1->uv1);
            if (link1->xyz)    msbspline_free (link1->xyz);
            if (link1->norm0)  msbspline_free (link1->norm0);
            if (link1->norm1)  msbspline_free (link1->norm1);
            if (link1)         msbspline_free (link1);

            if (*end == link1)
                *end = newLink;
            *next = link0->next;
            bspssi_removeLink (link0, next, start);
            break;
        default:
            break;
        }

    newLink->closed = bspssi_linkClosed (newLink, NULL, linkCode, ssiTolP);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             07/91
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspssi_appendCode
(
IntLink         *link0,
IntLink         *link1,
double          *angle,
int             code,
SsiTolerance    *ssiTolP
)
    {
    int         last0, last1, last0m1, last1m1;
    double      dist = 0.0;
    DPoint3d    a, b, c, d, e, f, g, h;

    last0 = link0->number - 1;    last0m1 = last0 - 1;
    last1 = link1->number - 1;    last1m1 = last1 - 1;
    switch (code)
        {
        case CODE_UV0:
            a.x = link0->uv0[0].x;          a.y = link0->uv0[0].y;
            b.x = link0->uv0[1].x;          b.y = link0->uv0[1].y;
            c.x = link0->uv0[last0m1].x;    c.y = link0->uv0[last0m1].y;
            d.x = link0->uv0[last0].x;      d.y = link0->uv0[last0].y;
            e.x = link1->uv0[0].x;          e.y = link1->uv0[0].y;
            f.x = link1->uv0[1].x;          f.y = link1->uv0[1].y;
            g.x = link1->uv0[last1m1].x;    g.y = link1->uv0[last1m1].y;
            h.x = link1->uv0[last1].x;      h.y = link1->uv0[last1].y;
            a.z = b.z = c.z = d.z = e.z = f.z = g.z = h.z = 0.0;
            dist = ssiTolP->uvTol;
            break;

        case CODE_UV1:
            a.x = link0->uv1[0].x;          a.y = link0->uv1[0].y;
            b.x = link0->uv1[1].x;          b.y = link0->uv1[1].y;
            c.x = link0->uv1[last0m1].x;    c.y = link0->uv1[last0m1].y;
            d.x = link0->uv1[last0].x;      d.y = link0->uv1[last0].y;
            e.x = link1->uv1[0].x;          e.y = link1->uv1[0].y;
            f.x = link1->uv1[1].x;          f.y = link1->uv1[1].y;
            g.x = link1->uv1[last1m1].x;    g.y = link1->uv1[last1m1].y;
            h.x = link1->uv1[last1].x;      h.y = link1->uv1[last1].y;
            a.z = b.z = c.z = d.z = e.z = f.z = g.z = h.z = 0.0;
            dist = ssiTolP->uvTol;
            break;

        case CODE_XYZ:
            a = link0->xyz[0];
            b = link0->xyz[1];
            c = link0->xyz[last0m1];
            d = link0->xyz[last0];
            e = link1->xyz[0];
            f = link1->xyz[1];
            g = link1->xyz[last1m1];
            h = link1->xyz[last1];
            dist = ssiTolP->xyzTol;
            break;
        }

    /* Do not allow links to attach to each other at the ends that are on the edges! */

    if (a.Distance (e) < dist &&
        !(code < CODE_XYZ &&
                (bsputil_onEdge ((DPoint2d *) &a, dist) ||
                 bsputil_onEdge ((DPoint2d *) &e, dist))))
        {
        *angle = AbsCosineBetweenSegments (&a, &b, &e, &f);
        return APPEND_ORG_ORG;
        }
    else if (d.Distance (e) < dist &&
        !(code < CODE_XYZ &&
                (bsputil_onEdge ((DPoint2d *) &d, dist) ||
                 bsputil_onEdge ((DPoint2d *) &e, dist))))
        {
        *angle = AbsCosineBetweenSegments  (&d, &c, &e, &f);
        return APPEND_END_ORG;
        }
    else if (a.Distance (h) < dist &&
        !(code < CODE_XYZ &&
                (bsputil_onEdge ((DPoint2d *) &a, dist) ||
                 bsputil_onEdge ((DPoint2d *) &h, dist))))
        {
        *angle = AbsCosineBetweenSegments (&a, &b, &h, &g);
        return APPEND_ORG_END;
        }
    else if (d.Distance (h) < dist &&
        !(code < CODE_XYZ &&
                (bsputil_onEdge ((DPoint2d *) &d, dist) ||
                 bsputil_onEdge ((DPoint2d *) &h, dist))))
        {
        *angle = AbsCosineBetweenSegments (&d, &c, &h, &g);
        return APPEND_END_END;
        }
    else
        return APPEND_NONE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             07/91
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspssi_selectLinkToAppend
(
IntLink         **bestLink,
double          *bestAngle, // Not angle -- COSINE of angle.
int             *bestCode,
IntLink         *link0,
IntLink         *link1,
int             linkCode,
SsiTolerance    *ssiTolP
)
    {
    int         code;
    double      angle;

    if (link1->closed)
        return SUCCESS;

    if (APPEND_NONE != (code = bspssi_appendCode (link0, link1, &angle, linkCode, ssiTolP)))
        {
#if defined (ASSEMBLE_MOST_LINEAR)
        if (angle > *bestAngle)
#elif defined (ASSEMBLE_LEAST_LINEAR)
        if (angle < *bestAngle)
#endif
            {
            *bestAngle = angle;
            *bestLink  = link1;
            *bestCode  = code;
            }
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             05/91
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspssi_appendBoundary
(
BsurfBoundary   **boundPP,
int             *numBounds,
DPoint2d        *pts,
int             numPts,
int             closed
)
    {
    int             length, status=SUCCESS;
    DPoint2d        path[7], *last;
    BsurfBoundary   bnd, *surfBndP;

    bspUtil_initializeBsurfBoundary (&bnd);
    last = pts + numPts - 1;
    if (closed == CLOSED_TRUE_CLOSURE)
        {
        bnd.numPoints = numPts + 1;
        if (NULL == (bnd.points = (DPoint2d*)BSIBaseGeom::Malloc (bnd.numPoints * sizeof(DPoint2d))))
            return ERROR;

        memcpy (bnd.points, pts, numPts * sizeof(DPoint2d));
        bnd.points[numPts] = bnd.points[0];
        }
    else if (closed == CLOSED_ON_EDGE)
        {
        bsputil_closestEdge (pts, pts);
        bsputil_closestEdge (last, last);
        bspsurf_shortPath (path, &length, pts, last);

        /* No need to repeat pts[0] in the boundary */
        length -= 1;

        bnd.numPoints = numPts + length;
        if (NULL == (bnd.points = (DPoint2d*)BSIBaseGeom::Malloc (bnd.numPoints * sizeof(DPoint2d))))
            return ERROR;

        memcpy (bnd.points, path, length * sizeof(DPoint2d));
        memcpy (bnd.points+length, pts, numPts * sizeof(DPoint2d));
        }
    else
        /* Ignore other cases for now */
        return SUCCESS;

    if (! *numBounds)
        {
        if (NULL == (*boundPP = (BsurfBoundary*)BSIBaseGeom::Malloc (sizeof(BsurfBoundary))))
            {
            status = ERROR;
            goto wrapup;
            }
        }
    else
        {
        if (NULL == (surfBndP = (BsurfBoundary*)BSIBaseGeom::Realloc (*boundPP,
                                          (*numBounds + 1) * sizeof(BsurfBoundary))))
            {
            status = ERROR;
            goto wrapup;
            }
        *boundPP = surfBndP;
        }

    // The newly allocated space will always be initialized by the following assignment
    *(*boundPP + (*numBounds)++) = bnd;

wrapup:
    if (status && bnd.points)
        BSIBaseGeom::Free (bnd.points);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             05/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int     bspssi_appendPointList
(
PointList       **listPP,           /* <=> point list to append to */
int             *numLists,          /* <=> number of lists */
PointList       *newList            /* =>  list to append */
)
    {
    int         status=SUCCESS;
    PointList   *listP;

    if (*numLists == 0)
        {
        if (NULL == (*listPP = (PointList*)BSIBaseGeom::Malloc (sizeof(PointList))))
            status = ERROR;
        }
    else
        {
        if (NULL == (listP = (PointList*)BSIBaseGeom::Realloc (*listPP,
                                                   (*numLists + 1) * sizeof(PointList))))
            status = ERROR;
        else
            *listPP = listP;
        }

    *(*listPP + (*numLists)++) = *newList;

    memset (newList, 0, sizeof(PointList));
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             05/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int     bspssi_appendPointList_clearInput
(
PointList       **listPP,           /* <=> point list to append to */
int             *numLists,          /* <=> number of lists */
bvector<DPoint3d> newPoints         /* new points.  This array is cleared after the copy.*/
)
    {
    int         status=SUCCESS;
    PointList   *listP;

    if (*numLists == 0)
        {
        if (NULL == (*listPP = (PointList*)BSIBaseGeom::Malloc (sizeof(PointList))))
            status = ERROR;
        }
    else
        {
        if (NULL == (listP = (PointList*)BSIBaseGeom::Realloc (*listPP,
                                                   (*numLists + 1) * sizeof(PointList))))
            status = ERROR;
        else
            *listPP = listP;
        }

    (*listPP)[*numLists].numPoints = BSIBaseGeom::MallocAndCopy (&(*listPP)[*numLists].points, newPoints);
    *numLists += 1;
    newPoints.clear ();
    return status;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             06/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int     bspssi_appendPointToList
(
PointList       *pointList,         /* <=> list to append to */
DPoint3d        *point              /* =>  point to append */
)
    {
    int         allocSize;
    void        *pointP;

    if (pointList->numPoints)
        {
        pointList->numPoints++;
        if (pointList->numPoints % MAX_BSPBATCH == 0)
            {
            allocSize = (pointList->numPoints + MAX_BSPBATCH) * sizeof(DPoint3d);
            if (NULL == (pointP = BSIBaseGeom::Realloc ((void *) pointList->points,
                                                        allocSize)))
                return ERROR;
            else
                pointList->points = (DPoint3d *) pointP;
            }
        }
    else
        {
        pointList->numPoints = 1;

        if (NULL ==
            (pointList->points = (DPoint3d*)BSIBaseGeom::Malloc (MAX_BSPBATCH * sizeof(DPoint3d))))
            return ERROR;
        }
    pointList->points[pointList->numPoints-1] = *point;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             03/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspssi_assignLinks
(
void            **linkPP,
int             *numLinks,
IntLink         **chain,
int             code,
SsiTolerance    *ssiTolP
)
    {
    int             status, appendCode;
    double          angle;
    DPoint3d        *xyzP, *xyzEndP;
    IntLink         *chnP, *tmpP, *endP, *nextChnP, *bestP = NULL;
    PointList       **plPP, pList;
    BsurfBoundary   **bPP;
    memset (&pList, 0, sizeof(PointList));
    if (! *chain)
        return SUCCESS;
    for (endP = *chain; endP->next; endP = endP->next)
        ;

    for (chnP = *chain; chnP; chnP = nextChnP)
        {
#if defined (debug)
        if ( displayLinks)
            dumpChain (*chain,
                (code == CODE_UV0 ? "uv0" : (code == CODE_UV1 ? "uv1" : "xyz")), code);
#endif
        nextChnP = chnP->next;
        appendCode = 0;

#if defined (ASSEMBLE_MOST_LINEAR)
        angle = -1.0 ;
#elif defined (ASSEMBLE_LEAST_LINEAR)
        angle = fc_hugeVal;
#endif

        if (! chnP->closed)
            {
            for (tmpP=chnP->next; tmpP; tmpP = tmpP->next)
                bspssi_selectLinkToAppend (&bestP, &angle, &appendCode, chnP, tmpP,
                                           code, ssiTolP);

            if (appendCode)
                bspssi_appendLinks (chnP, bestP, chain, &endP, &nextChnP, appendCode,
                                    code, ssiTolP);
            }
        }

    switch (code)
        {
        case CODE_UV0:
        case CODE_UV1:
            bPP  = (BsurfBoundary **) linkPP;
            for (tmpP = *chain; tmpP; tmpP = tmpP->next)
                {
                if (SUCCESS != (status = bspssi_appendBoundary (bPP, numLinks,
                                         (code == CODE_UV0 ? tmpP->uv0 : tmpP->uv1),
                                          tmpP->number, tmpP->closed)))
                    return status;
                }
            break;
        case CODE_XYZ:
            plPP = (PointList **) linkPP;
            for (tmpP = *chain; tmpP; tmpP = tmpP->next)
                {
                for (xyzP = tmpP->xyz, xyzEndP = xyzP + tmpP->number;
                        xyzP < xyzEndP;
                            xyzP++)
                    {
                    bspssi_appendPointToList (&pList, xyzP);
                    }
                if (pList.numPoints &&
                    SUCCESS != (status = bspssi_appendPointList (plPP, numLinks, &pList)))
                    return status;
                }
            break;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             06/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int     bspssi_compareIntersections
(
const BoundIntersect    *biP1,
const BoundIntersect    *biP2,
const void              *optArgsP
)
    {
    if (biP1->distance < biP2->distance)
        return -1;
    else if (biP1->distance > biP2->distance)
        return 1;
    else
        return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/93
+---------------+---------------+---------------+---------------+---------------+------*/
static void bspssi_computeIntersectNormal
(
DPoint3d            *normalP,
double              segDistance,
DRange2d           *uvSegmentP,
MSBsplineSurface    *surfaceP,
double              *offsetP
)
    {
    double          mag_cross, mag_cross2,
                    mag_dCrossDu, mag_dCrossDv, mag_dPdu, mag_dPdv, prod;
    DPoint2d        uv;
    DPoint3d        du, dv, duu, dvv, duv, safeNormal, cross, tmp0, tmp1,
                    nearPoint, dCrossDu, dCrossDv, dNormDu, dNormDv;

    uv.x = uvSegmentP->low.x * (1.0 - segDistance) + uvSegmentP->high.x * segDistance;
    uv.y = uvSegmentP->low.y * (1.0 - segDistance) + uvSegmentP->high.y * segDistance;

    bspsurf_computePartials (&nearPoint, NULL, &du, &dv, &duu, &dvv, &duv,
                             &safeNormal, uv.x, uv.y, surfaceP);

    if (offsetP)
        {
        /*-----------------------------------------------------------
        Must correct the partials, see Farouki, R.T. in CAGD vol 3, pp. 15-45.
        -----------------------------------------------------------*/
        nearPoint.SumOf (nearPoint, safeNormal, *offsetP / bsiDPoint3d_magnitude (&safeNormal));

        cross.CrossProduct (du, dv);
        tmp0.CrossProduct (duu, dv);
        tmp1.CrossProduct (du, duv);
        dCrossDu.SumOf (tmp0, tmp1);
        tmp0.CrossProduct (duv, dv);
        tmp1.CrossProduct (du, dvv);
        dCrossDv.SumOf (tmp0, tmp1);

        mag_cross    = cross.Magnitude ();
        mag_cross2   = mag_cross * mag_cross;
        mag_dCrossDu = cross.DotProduct (dCrossDu) / mag_cross;
        mag_dCrossDv = cross.DotProduct (dCrossDv) / mag_cross;

        dNormDu.Scale (dCrossDu, mag_cross);
        dNormDu.SumOf (dNormDu, cross, - mag_dCrossDu);
        dNormDu.Scale (dNormDu, 1.0 / mag_cross2);
        dNormDv.Scale (dCrossDv, mag_cross);
        dNormDv.SumOf (dNormDv, cross, - mag_dCrossDv);
        dNormDv.Scale (dNormDv, 1.0 / mag_cross2);

        du.SumOf (du, dNormDu, *offsetP);
        dv.SumOf (dv, dNormDv, *offsetP);
        }

    mag_dPdu = du.Magnitude ();
    mag_dPdv = dv.Magnitude ();
    cross.CrossProduct (du, dv);
    mag_cross = cross.Magnitude ();

    if (mag_dPdu < fc_epsilon || mag_dPdv < fc_epsilon)
        {
        *normalP = safeNormal;
        }
    else
        {
        prod = 1.0 / (mag_dPdu * mag_dPdv);
        normalP->Scale (cross, prod);    /* scaled by sine angle btw partials */
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             06/91
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspssi_getBoundaryIntersections
(
BoundIntersect      *boundIntersectP,
DRange2d           *uvSegmentP,
DRange2d           *otherUVSegmentP,
DRange3d            *xyzSegmentP,
MSBsplineSurface    *surfP,
MSBsplineSurface    *otherSurfaceP,
double              *offsetP,
double              *otherOffsetP,
bool                surface0
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
                                           (DRange2dP) uvSegmentP, (DRange2dP)  pntP, 1.0E-10);
            if (intersect0 & (INTERSECT_SPAN | INTERSECT_END) &&
                intersect1 & (INTERSECT_SPAN | INTERSECT_END))
                {
                if (boundIntersectP)
                    {
                    biP->distance = distance0 / segmentDistance;

                    biP->point.Interpolate (xyzSegmentP->low, biP->distance, xyzSegmentP->high);

                    if ((biP->surface0 = surface0) != false)
                        {
                        bspssi_computeIntersectNormal (&biP->normal0,  biP->distance,
                                                       uvSegmentP, surfP, offsetP);
                        bspssi_computeIntersectNormal (&biP->normal1,  biP->distance,
                                                       otherUVSegmentP, otherSurfaceP,
                                                       otherOffsetP);
                        }
                    else
                        {
                        bspssi_computeIntersectNormal (&biP->normal0, biP->distance,
                                                           otherUVSegmentP, otherSurfaceP,
                                                           otherOffsetP);
                        bspssi_computeIntersectNormal (&biP->normal1, biP->distance,
                                                       uvSegmentP, surfP, offsetP);
                        }
                    }
                biP++;
                }
            }
        }
#ifdef BEIJING_DGNGRAPHICS_WIP_64BIT
#endif
    return (int)(biP - boundIntersectP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RBB             06/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspssi_extractPointLists
(
PointList           **pointLists,
PointList           **off0ptLists,
PointList           **off1ptLists,
int                 *numPointLists,
IntLink             **chain,
SsiTolerance        *ssiTolP,
MSBsplineSurface    *surf0,
MSBsplineSurface    *surf1,
double              *offset0P,
double              *offset1P
)
    {
    int             nIsPnts, appendCode, numNorm0PtLists, numNorm1PtLists;
    bool            inside0, inside1;
    double          angle;
    DPoint2d        *uv0P, *uv1P;
    DPoint3d        *xyzP, *n0P, *n1P, *endP;
    IntLink         *tmpP, *chnP, *endLinkP, *nextChnP, *bestP = NULL;
    BoundIntersect  isPnts[MAX_INTERSECTIONS], *biP, *biPEnd;
    PointList       ptList, n0List, n1List;

    if (! *chain)
        return SUCCESS;

    for (endLinkP = *chain; endLinkP->next; endLinkP = endLinkP->next)
        ;

    for (chnP = *chain; chnP; chnP = nextChnP)
        {
#if defined (debug)
        if ( displayLinks)
            dumpChain (*chain, "xyz", CODE_XYZ);
#endif
        nextChnP = chnP->next;
        appendCode = APPEND_NONE;
#if defined (ASSEMBLE_MOST_LINEAR)
        angle = 0.0;
#elif defined (ASSEMBLE_LEAST_LINEAR)
        angle = fc_hugeVal;
#endif
        if (! chnP->closed)
            {
            for (tmpP=chnP->next; tmpP; tmpP = tmpP->next)
                bspssi_selectLinkToAppend (&bestP, &angle, &appendCode, chnP, tmpP,
                                           CODE_XYZ, ssiTolP);

            if (appendCode)
                bspssi_appendLinks (chnP, bestP, chain, &endLinkP, &nextChnP, appendCode,
                                    CODE_XYZ, ssiTolP);
            }
        }

    *numPointLists = numNorm0PtLists = numNorm1PtLists = 0;
    for (chnP = *chain; chnP; chnP = chnP->next)
        {
        memset (&ptList, 0, sizeof(ptList));
        memset (&n0List, 0, sizeof(n0List));
        memset (&n1List, 0, sizeof(n1List));
#if defined (BOUNDARY_CLIPPING)
        if ((surf0 && surf0->numBounds) || (surf1 && surf1->numBounds))
            {
            inside0 = bsputil_pointOnSurface (chnP->uv0, surf0);
            inside1 = bsputil_pointOnSurface (chnP->uv1, surf1);

            for (uv0P = chnP->uv0, uv1P = chnP->uv1,
                 xyzP = chnP->xyz, endP = xyzP + chnP->number - 1,
                 n0P = chnP->norm0, n1P = chnP->norm1;
                 xyzP < endP;
                 uv0P++, uv1P++, xyzP++, n0P++, n1P++)
                {
                nIsPnts = 0;
                nIsPnts += bspssi_getBoundaryIntersections (isPnts,
                                                            (DRange2d *) uv0P,
                                                            (DRange2d *) uv1P,
                                                            (DRange3d *) xyzP,
                                                             surf0, surf1,
                                                             offset0P, offset1P, true);
                nIsPnts += bspssi_getBoundaryIntersections (isPnts + nIsPnts,
                                                            (DRange2d *) uv1P,
                                                            (DRange2d *) uv0P,
                                                            (DRange3d *) xyzP,
                                                             surf1, surf0,
                                                             offset1P, offset0P, false);

                mdlUtil_dlmQuickSort (isPnts, isPnts + nIsPnts - 1,
                                sizeof(BoundIntersect),
                                (PFToolsSortCompare)bspssi_compareIntersections, NULL);

                if (inside0 && inside1)
                    {
                    bspssi_appendPointToList (&ptList, xyzP);
                    if (off0ptLists)    bspssi_appendPointToList (&n0List, n0P);
                    if (off1ptLists)    bspssi_appendPointToList (&n1List, n1P);
                    }

                for (biP=isPnts, biPEnd = biP + nIsPnts; biP < biPEnd; biP++)
                    {
                    if (biP->surface0)  inside0 = !inside0;
                    else                inside1 = !inside1;

                    if (inside0 && inside1)
                        {
                        bspssi_appendPointToList (&ptList, &biP->point);
                        if (off0ptLists) bspssi_appendPointToList (&n0List, &biP->normal0);
                        if (off1ptLists) bspssi_appendPointToList (&n1List, &biP->normal1);
                        }
                    else if (ptList.numPoints)
                        {
                        bspssi_appendPointToList (&ptList, &biP->point);
                        if (off0ptLists) bspssi_appendPointToList (&n0List, &biP->normal0);
                        if (off1ptLists) bspssi_appendPointToList (&n1List, &biP->normal1);

                        bspssi_appendPointList (pointLists, numPointLists, &ptList);
                        if (off0ptLists && n0List.numPoints)
                            bspssi_appendPointList (off0ptLists, &numNorm0PtLists, &n0List);
                        if (off1ptLists && n1List.numPoints)
                            bspssi_appendPointList (off1ptLists, &numNorm1PtLists, &n1List);
                        }
                    }
                }
            if (inside0 && inside1)
                {
                bspssi_appendPointToList (&ptList, xyzP);
                if (off0ptLists)        bspssi_appendPointToList (&n0List, n0P);
                if (off1ptLists)        bspssi_appendPointToList (&n1List, n1P);
                }
            }
        else
#endif
            {
            for (xyzP = chnP->xyz, endP = xyzP + chnP->number; xyzP < endP; xyzP++)
                bspssi_appendPointToList (&ptList, xyzP);

            if (off0ptLists)
                {
                for (n0P = chnP->norm0, endP = n0P + chnP->number; n0P < endP; n0P++)
                    bspssi_appendPointToList (&n0List, n0P);
                }
            if (off1ptLists)
                {
                for (n1P = chnP->norm1, endP = n1P + chnP->number; n1P < endP; n1P++)
                    bspssi_appendPointToList (&n1List, n1P);
                }
            }

        if (ptList.numPoints)
            bspssi_appendPointList (pointLists,  numPointLists, &ptList);
        if (off0ptLists && n0List.numPoints)
            bspssi_appendPointList (off0ptLists, &numNorm0PtLists, &n0List);
        if (off1ptLists && n1List.numPoints)
            bspssi_appendPointList (off1ptLists, &numNorm1PtLists, &n1List);
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brian.Peters    08/93
+---------------+---------------+---------------+---------------+---------------+------*/
static int     bspssi_extractWithFullInfo
(
IntLink             **chain,
SsiTolerance        *ssiTolP,
MSBsplineSurface    *surf0,
MSBsplineSurface    *surf1
)
    {
    int             appendCode;
    double          angle;
    IntLink         *tmpP, *chnP, *endLinkP, *nextChnP, *bestP = NULL;

    if (! *chain)
        return SUCCESS;

    for (endLinkP = *chain; endLinkP->next; endLinkP = endLinkP->next)
        ;

    for (chnP = *chain; chnP; chnP = nextChnP)
        {
#if defined (debug)
        if ( displayLinks)
            dumpChain (*chain, "xyz", CODE_XYZ);
#endif
        nextChnP = chnP->next;
        appendCode = APPEND_NONE;
#if defined (ASSEMBLE_MOST_LINEAR)
        angle = 0.0;
#elif defined (ASSEMBLE_LEAST_LINEAR)
        angle = fc_hugeVal;
#endif
        if (! chnP->closed)
            {
            for (tmpP=chnP->next; tmpP; tmpP = tmpP->next)
                bspssi_selectLinkToAppend (&bestP, &angle, &appendCode, chnP, tmpP,
                                           CODE_XYZ, ssiTolP);

            if (appendCode)
                bspssi_appendLinks (chnP, bestP, chain, &endLinkP, &nextChnP, appendCode,
                                    CODE_XYZ, ssiTolP);
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             06/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspssi_intersectSurfacesExtra
(
PointList           **pointLists,
PointList           **off0ptLists,
PointList           **off1ptLists,
int                 *numPointLists,
BsurfBoundary       **surf0Bounds,
int                 *numSurf0Bounds,
BsurfBoundary       **surf1Bounds,
int                 *numSurf1Bounds,
IntLink             **fullInfo,
SsiTolerance        *tolOut,
MSBsplineSurface    *surf0,
MSBsplineSurface    *surf1,
double              tolerance,
double              uor_resolution,
int                 displayFlag,
double              offset0,
double              offset1,
int                 tightTolerance
)
    {
    int             status, numBounds0, numBounds1, numLists;
    double          mag0, mag1, min, max, limit;
    BoundBox        b0, b1;
    BezierInfo      info;
    IntLink         *chainP, *uv0ChnP, *uv1ChnP;
    SsiTolerance    ssiTol;
    // ugh.  duelling tolerances.
    // bsp callers have traditional 1 uor resolution --
    //    we're going to supercede that with a fraction of the box size.
    //  But this is scary.
    static double s_minToleranceFraction = 1.0e-4;
    static double s_minUORFraction = 1.0e-5;
    static double s_maxUORFraction = 1.0e-5;
    double boxSize;
    double boxTol;
    Evaluator       eval0, eval1;

    if (tolerance == 0.0)
        return ERROR;

    status = SUCCESS;
    numBounds0 = numBounds1 = numLists = 0;
    chainP = uv0ChnP = uv1ChnP = NULL;
    if (fullInfo)       *fullInfo = NULL;

    eval0.surf     = surf0;
    eval0.offset   = ! DoubleOps::AlmostEqual (offset0, 0.0);
    eval0.distance = offset0;
    eval1.surf     = surf1;
    eval1.offset   = ! DoubleOps::AlmostEqual (offset1, 0.0);
    eval1.distance = offset1;

    /* Check surface intersection */
    bound_boxFromEval (&b0, &eval0);
    bound_boxFromEval (&b1, &eval1);
    boxSize = b0.extent.Magnitude () + b1.extent.Magnitude ();
    boxTol = s_minToleranceFraction * boxSize;
    if (tolerance < boxTol)
        tolerance = boxTol;
    boxTol = s_minUORFraction * boxSize;
    if (uor_resolution < boxTol)
        uor_resolution = boxTol;
    if (uor_resolution > s_maxUORFraction * boxSize)
        uor_resolution = s_maxUORFraction * boxSize;


    if (!bound_boxesIntersect (&b0, &b1))
        goto wrapup;

    mag0  = b0.extent.Magnitude ();
    mag1  = b1.extent.Magnitude ();
    if (mag0 < mag1)
        {
        min = mag0;
        max = mag1;
        }
    else
        {
        min = mag1;
        max = mag0;
        }
    mag0 = sqrt (mag0 * mag1);

    /* Load march-controlling constants */
    if (uor_resolution >= tolerance)
        tolerance = uor_resolution * 2;

    ssiTol.xyzSame = uor_resolution;
    ssiTol.xyzTol  = tolerance;
    ssiTol.uvSame  = ssiTol.xyzSame / (max * 10.0);
    ssiTol.uvTol   = ssiTol.xyzTol / (tightTolerance ? max: mag0);
    ssiTol.maxStep = ssiTol.xyzTol * 200;

    if (ssiTol.maxStep > (limit = min * fc_p01))
        ssiTol.maxStep =  limit;
    if (ssiTol.uvTol > fc_p001)
        ssiTol.uvTol = fc_p001;

    info.ssi.showMarch = displayFlag;
    info.ssi.tol       = &ssiTol;
    info.ssi.eval0     = &eval0;
    info.ssi.eval1     = &eval1;
    info.ssi.chainPP   = &chainP;

#if defined (debug)
    debugStartPt = displayLinks =debugStop = debugStep = addLink =
    debugTan = displayBox = debugCovered = debugReconcile = false;
    debugStop = displayLinks = true;
    before = after = 0;
    printf ("\n");
    printf ("ssiTol.xyzSame    = %f\n", ssiTol.xyzSame);
    printf ("ssiTol.uvSame     = %f\n", ssiTol.uvSame);
    printf ("ssiTol.xyzTol     = %f\n", ssiTol.xyzTol);
    printf ("ssiTol.uvTol      = %f\n", ssiTol.uvTol);
    printf ("ssiTol.maxStep    = %f\n", ssiTol.maxStep);
#endif

    if (SUCCESS != (status = bspproc_doubleProcessBsplineSurface (&info,
                                                                  eval0.surf,
                                                                  eval1.surf,
                                                                  bspssi_ssiSort,
                                                                  bspssi_ssiGo,
                                                                  bspssi_ssiBezier,
                                                                  NULLFUNC, NULLFUNC)))
        goto wrapup;

    /* Relax uv tolerance to accomodate assembling the intersection links */
    ssiTol.uvTol *= 10.0;

    /* If I need the full information for variable radius blending then assemble it by the
        xyz coordinates, but carry along all the parameter and differential information.
        Also pass out the ssiTol info so that it can be used later when extracting the
        PointList and BsurfBoundary. */
    if (tolOut)
        *tolOut = ssiTol;

    if (fullInfo)
        {
        if (SUCCESS == (status = bspssi_extractWithFullInfo (&chainP, &ssiTol, surf0, surf1)))
            {
            *fullInfo = chainP;
            chainP = NULL;
            }
        }
    else
        {
        if (surf0Bounds &&
            (SUCCESS != (status = bspssi_extractLinks (&uv0ChnP, chainP, CODE_UV0,
                                                       &ssiTol, NULL, NULL)) ||
            SUCCESS != (status = bspssi_assignLinks ((void **) surf0Bounds, &numBounds0,
                                                     &uv0ChnP, CODE_UV0, &ssiTol))))
            goto wrapup;

        if (surf1Bounds &&
            (SUCCESS != (status = bspssi_extractLinks (&uv1ChnP, chainP, CODE_UV1,
                                                       &ssiTol, NULL, NULL)) ||
            SUCCESS != (status = bspssi_assignLinks ((void **) surf1Bounds, &numBounds1,
                                                     &uv1ChnP, CODE_UV1, &ssiTol))))
            goto wrapup;

        if (pointLists)
            {
            if (SUCCESS != (status = bspssi_extractPointLists (pointLists,
                                                               off0ptLists, off1ptLists,
                                                               &numLists, info.ssi.chainPP,
                                                               &ssiTol, surf0, surf1,
                                                               eval0.offset ?
                                                               &eval0.distance : NULL,
                                                               eval1.offset ?
                                                               &eval1.distance : NULL)))
                goto wrapup;
            }
        }

wrapup:
    if (numSurf0Bounds) *numSurf0Bounds = numBounds0;
    if (numSurf1Bounds) *numSurf1Bounds = numBounds1;
    if (numPointLists)  *numPointLists  = numLists;


    bspssi_freeLink (&chainP);
    bspssi_freeLink (&uv0ChnP);
    bspssi_freeLink (&uv1ChnP);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             06/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspssi_intersectSurfaces
(
PointList           **pointLists,
PointList           **off0ptLists,
PointList           **off1ptLists,
int                 *numPointLists,
BsurfBoundary       **surf0Bounds,
int                 *numSurf0Bounds,
BsurfBoundary       **surf1Bounds,
int                 *numSurf1Bounds,
IntLink             **fullInfo,
SsiTolerance        *tolOut,
MSBsplineSurface    *surf0,
MSBsplineSurface    *surf1,
double              tolerance,
double              uor_resolution,
int                 displayFlag,
double              offset0,
double              offset1
)
    {
    return  bspssi_intersectSurfacesExtra (pointLists, off0ptLists, off1ptLists, numPointLists,
                                           surf0Bounds, numSurf0Bounds, surf1Bounds, numSurf1Bounds,
                                           fullInfo, tolOut, surf0, surf1, tolerance,
                                           uor_resolution, displayFlag, offset0, offset1, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             06/91
+---------------+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP int      bspssi_intersectSurfacesTight
(
PointList           **pointLists,
PointList           **off0ptLists,
PointList           **off1ptLists,
int                 *numPointLists,
BsurfBoundary       **surf0Bounds,
int                 *numSurf0Bounds,
BsurfBoundary       **surf1Bounds,
int                 *numSurf1Bounds,
IntLink             **fullInfo,
SsiTolerance        *tolOut,
MSBsplineSurface    *surf0,
MSBsplineSurface    *surf1,
double              tolerance,
double              uor_resolution,
int                 displayFlag,
double              offset0,
double              offset1
)
    {
    return  bspssi_intersectSurfacesExtra (pointLists, off0ptLists, off1ptLists, numPointLists,
                                           surf0Bounds, numSurf0Bounds, surf1Bounds, numSurf1Bounds,
                                           fullInfo, tolOut, surf0, surf1, tolerance,
                                           uor_resolution, displayFlag, offset0, offset1, true);
    }

#if defined (debug)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BFP             06/91
+---------------+---------------+---------------+---------------+---------------+------*/
void    dumpChain
(
IntLink         *chain,
char            *msg,
int             code
)
    {
    int         i, j;
    IntLink     *chnP;

    printf ("\n%s\n", msg);
    for (chnP=chain, i=0; chnP; chnP = chnP->next, i++)
        {
        j = chnP->number - 1;
        printf ("\nLink #%d, contains %d points, closed = %d\n",
                i, chnP->number, chnP->closed);
        switch (code)
            {
            case CODE_UV0:
                printf ("Start: %3.4f,      End: %3.4f\n", chnP->uv0[0].x, chnP->uv0[j].x);
                printf ("       %3.4f,           %3.4f\n", chnP->uv0[0].y, chnP->uv0[j].y);
                break;
            case CODE_UV1:
                printf ("Start: %3.4f,      End: %3.4f\n", chnP->uv1[0].x, chnP->uv1[j].x);
                printf ("       %3.4f,           %3.4f\n", chnP->uv1[0].y, chnP->uv1[j].y);
                break;
            case CODE_XYZ:
                printf ("Start: %3.4f,      End: %3.4f\n", chnP->xyz[0].x, chnP->xyz[j].x);
                printf ("       %3.4f,           %3.4f\n", chnP->xyz[0].y, chnP->xyz[j].y);
                printf ("       %3.4f,           %3.4f\n", chnP->xyz[0].z, chnP->xyz[j].z);
                break;
            }
        printf ("This: %x,    Next: %x,    Last: %x\n", chnP, chnP->next, chnP->last);
        }
    }
#endif


END_BENTLEY_GEOMETRY_NAMESPACE
