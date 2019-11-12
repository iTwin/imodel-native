/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "msbsplinemaster.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

typedef struct
    {
    DPoint3d dXYZdu;
    DPoint3d dXYZdv;
    DPoint3d d2XYZdudu;
    DPoint3d d2XYZdvdv;
    DPoint3d d2XYZdudv;
    } PlankDerivatives;
/*
Here is a cross section of a plank
<pre>
                    B
    XXXXXXXXXXXXXXXX|XXXXXXXXXXXXXXXX
   A----------------+----------------A
    XXXXXXXXXXXXXXXX|XXXXXXXXXXXXXXXX
                    B
</pre>

The planking problem is to find the path by which a plank can be attached to a surface,
such that the plank is permitted to have bending in the form of
<ul>
<li>axial torsion (which you might call "twist")</li>
<li>bending around axis AA of the cross section</li>
</ul>
but is NOT permitted to bend around axis BB of the cross section.

Note that the surface is prescribed.  We are told to go from point P on the surface to point Q
on the surface, using as much of the allowed bending modes as needed.

We need to trace a curve on the surface.  We will do so be establishing a series of points
on the curve, enforcing plank-like conditions on the points.

Consider three successive points indexed i, j, and k on the curve.
    (Xi, uvi, Ui, Vi)
    (Xj, uvj, Uj, Vj)
    (Xk, uvk, Uk, Vk)
Here X is an xyz space point, uv parameter space coordinates,
 and U and V are xyz vectors which are derivatives of X wrt u and v.

 Vectors from i to j and j to k are
    Rij = Xj - Xi
    Rjk = Xk - Xj

The vectors Rij and Rjk approximate the curve tangent.  The plane containing both these
vectors approximates the plane of the osculating circle of tangent to the curve at Xj.
The plank bending condition requires that the surface normal be in the plane of the osculating circle.

The middle point Xj is free to move around the surface in two parameter directions u and v.
Hence we need to enforce two equational conditions at this point.

The first condition is just spacing --- make the two vectors Rij and Rjk of equal length,
so all the points are spread out along the curve.
<pre>
    Rij dot Rij - Rjk dot Rjk = 0
</pre>

The second equation is to enforce the plank bending condition by making Rij and Rjk coplanar with Nj, i.e.
<pre>
        (Rij cross Rjk) dot (Uj cross Vj) = 0
</pre>

The first and last points of the curve are fixed.
These two conditions are enforced at each internal point on the curve.
Each such point has two degrees of freedom (u and v in surface) and two conditions to satisfy.
*/

/*
@description Compute a cross product and its derivative with respect
    to quantities that affect the vectors in the product.
@param pCross OUT cross product, pU cross pV.
@param pdCrossdQi OUT array of derivatives of pCross wrt Qi
@param pU IN first vector
@param pdUdQi IN array of derivatives of pU wrt independent variables Qi.
@param pV IN second vector
@param pdVdQi IN array of derivatives of pV wrt independent variables Qi.
@param numDerivative IN number of derivatives, i.e. dimension of pdUdQi, pdVdQi, and pdCrossdQi.
*/
static void differentiateCrossProduct
(
DPoint3d *pCross,
DPoint3d *pdCrossdQi,
DPoint3d *pU,
DPoint3d *pdUdQi,
DPoint3d *pV,
DPoint3d *pdVdQi,
int     numDerivative
)
    {
    int i;
    DPoint3d term0, term1;
    pCross->CrossProduct (*pU, *pV);
    for (i = 0; i < numDerivative; i++)
        {
        term0.CrossProduct (*pU, pdVdQi[i]);
        term1.CrossProduct (pdUdQi[i], *pV);
        pdCrossdQi[i].SumOf (term0, term1);
        }
    }

/*
@description Compute a dot product and its derivative with respect
    to quantities that affect the vectors in the product.
@param pDot OUT dot product, pU dot pV.
@param pdCrossdQi OUT array of derivatives of pCross wrt Qi.
@param pU IN first vector.
@param pdUdQi IN array of derivatives of pU wrt independent variables Qi.
@param pV IN second vector.
@param pdVdQi IN array of derivatives of pV wrt independent variables Qi.
@param numDerivative IN number of derivatives, i.e. dimension of pdUdQi, pdVdQi, and pdDotdQi.
*/
static void differentiateDotProduct
(
double *pDot,
double *pdDotdQi,
DPoint3d *pU,
DPoint3d *pdUdQi,
DPoint3d *pV,
DPoint3d *pdVdQi,
int     numDerivative
)
    {
    int i;
    double term0, term1;
    *pDot = pU->DotProduct (*pV);
    for (i = 0; i < numDerivative; i++)
        {
        term0 = pU->DotProduct (pdVdQi[i]);
        term1 = pdUdQi[i].DotProduct (*pV);
        pdDotdQi[i] = term0 + term1;
        }
    }

/*
@description Compute two (scalar) functions f0 and f1 which are to be zeroed at point j.
@param pf0 OUT the "equidistantly spaced points" condition, numerically equal to the
    difference of squared chord lengths.
@param pf1 OUT the "bending condition", numerically the dot product of the surface normal
    with the cross product of the incident chords.
@param pdf0 OUT derivatives of f0 wrt surface parameters u and v.
@param pdf1 OUT derivatives of f1 wrt surface parameters u and v.
@param pXi IN point i in space
@param pDi IN structure containing the two first derivatives of Xi wrt u, v, and three second
    derivatives wrt uu, vv, uv.
@param pXj IN point j in space.
@param pDj IN derivatives at j.
@param pXk IN point k in space.
@param pDk IN derivatives at k.
*/
static void plankingFunctions
(
double *pf0,
double *pf1,
double *pdf0,
double *pdf1,
DPoint3d *pXi,
PlankDerivatives *pDi,
DPoint3d *pXj,
PlankDerivatives *pDj,
DPoint3d *pXk,
PlankDerivatives *pDk
)
    {
    DPoint3d Rij, Rjk, cross0, cross1;
    DPoint3d dRij[6], dRjk[6];
    DPoint3d dCross0[6], dCross1[6];
    double g0, g1;
    int i;
    double dg0[6], dg1[6];
    DPoint3d Uj, Vj;
    DPoint3d dUj[6];
    DPoint3d dVj[6];

    Rij.DifferenceOf (*pXj, *pXi);
    Rjk.DifferenceOf (*pXk, *pXj);

    dRij[0].Negate (pDi->dXYZdu);
    dRij[1].Negate (pDi->dXYZdv);
    dRij[2] = pDj->dXYZdu;
    dRij[3] = pDj->dXYZdv;
    dRij[4].Zero ();
    dRij[5].Zero ();

    dRjk[0].Zero ();
    dRjk[1].Zero ();
    dRjk[2].Negate (pDj->dXYZdu);
    dRjk[3].Negate (pDj->dXYZdv);
    dRjk[4] = pDk->dXYZdu;
    dRjk[5] = pDk->dXYZdv;

    Uj = pDj->dXYZdu;
    Vj = pDj->dXYZdv;
    dUj[0].Zero ();
    dUj[1].Zero ();
    dUj[2] = pDj->d2XYZdudu;
    dUj[3] = pDj->d2XYZdudv;
    dUj[4].Zero ();
    dUj[5].Zero ();

    dVj[0].Zero ();
    dVj[1].Zero ();
    dVj[2] = pDj->d2XYZdudv;
    dVj[3] = pDj->d2XYZdvdv;
    dVj[4].Zero ();
    dVj[5].Zero ();

    /* f0 = Rij dot Rij - Rjk dot Rjk */
    differentiateDotProduct (&g0, dg0, &Rij, dRij, &Rij, dRij, 6);
    differentiateDotProduct (&g1, dg1, &Rjk, dRjk, &Rjk, dRjk, 6);
    *pf0 = g1 - g0;
    for (i = 0; i < 6; i++)
        pdf0[i] = dg1[i] - dg0[i];

    /* f1 = (Rij cross Rjk) dot (Uj cross Vj) */
    differentiateCrossProduct (&cross0, dCross0, &Rij, dRij, &Rjk, dRjk, 6);
    differentiateCrossProduct (&cross1, dCross1, &Uj, dUj, &Vj, dVj, 6);
    differentiateDotProduct (pf1, pdf1, &cross0, dCross0, &cross1, dCross1, 6);
    }

/*
@description Given an array of N points and derivatives, fill the vector of 2*N
    plank conditions, and the (block diagonal) jacobian matrix which gives
    derivatives of each plank condition wrt each surface parameter.
@param pA OUT jacobian matrix. A[i,j] is the derivative of condition i wrt unknown j.
        Indices 0,1 are at (internal) point 0 etc.
@param pf OUT function vector.
<ul>
<li>pf[2*p] is the "equidistant point spacing condition" at internal point index p.</li>
<li>pf[2*p+1] is the "plank bending condition" at internal point index p.</li>
</ul>
@param pXYZ IN array of numInternalPoint+2 points along curve.
@param pdXYZ IN array of derivatives at numInternalPoint+2 points along curve.
@param numInternalPoint IN number of internal points.

@remark the function vector is laid out point by point.
@remark the derivative matrix is "2x2 block tridiagonal". That is, there are nonzeros only
    in (a) each 2x2 diagonal block, and (b) the 2x2 blocks immediately to the right and left
        of the diagonal.  Hence there are exactly 6 nonzeros in each row.
        The matrix is formatted as 2*numInternalPoints rows, each containing only 6 values.
*/
static void fillPlankMatrix
(
double *pA,
double *pf,
DPoint3d *pXYZ,
PlankDerivatives *pdXYZ,
int     numInternalPoint
)
    {
    int i, j, k;
    int row0, row1;
    int matrixRow0, matrixRow1;

    for (i = 0; i < numInternalPoint; i++)
        {
        j = i + 1;
        k = j + 1;

        row0 = 2 * i;
        row1 = row0 + 1;
        matrixRow0 = 6 * row0;
        matrixRow1 = 6 * row1;
        plankingFunctions (&pf[row0], &pf[row1],
                    &pA[matrixRow0], &pA[matrixRow1],
                    &pXYZ[i], &pdXYZ[i],
                    &pXYZ[j], &pdXYZ[j],
                    &pXYZ[k], &pdXYZ[k]
                    );
        }
    }



/*
@description Compute a single Newton step, given points and derivatives.
@param pDeltaUV OUT array of (numPoint-2) parameter steps at internal points.
@param pXYZ IN array of numPoint points on the curve.
@param pdXYZ IN array of numPoint derivatives.
@remark note that points and derivatives are supplied at all points (including endpoints) but
    parameter updates are only at internal points.  Parameter update k applies to point k+1.
*/
static bool    plankingStep
(
DPoint2d *pDeltaUV,
DPoint3d *pXYZ,
PlankDerivatives *pdXYZ,
int  numPoint
)
    {
    int numInternalPoint = numPoint - 2;
    double *pff   = (double*)_alloca (2 * numInternalPoint * sizeof (double) );
    double *pAA   = (double*)_alloca (2 * numInternalPoint * 6 * sizeof (double) );

    fillPlankMatrix (pAA, pff, pXYZ, pdXYZ, numInternalPoint);
    return bsiLinalg_solveBlockTridiagonal ((double*)pDeltaUV, pAA, pff, numInternalPoint, 2);
    }


/*
@description Compute a single Newton step, given parameters and a surface evaluation function.
@param pDeltaUV OUT array of (numPoint-2) parameter updates.
@param pUV IN array of surface parameters along the curve
@param pXYZ OUT array of surface points evaluated at parameters pUV
@param numPoint IN number of points
@param surfaceFunc IN function to call to evaluate surface
@param pSurfaceData IN context data for surface function.
*/
static bool    planking_computePathChange
(
DPoint2d *pDeltaUV,
DPoint2d *pUV,
DPoint3d *pXYZ,
int      numPoint,
PlankingSurfaceEvaluator surfaceFunc,
void *pSurfaceData
)
    {
    if (numPoint < 1)
        return false;
    PlankDerivatives *pD = (PlankDerivatives*)_alloca (numPoint * sizeof(PlankDerivatives));
    int i;

    /* Load up derivative arrays. */
    for (i = 0; i < numPoint; i++)
        {
        if (!surfaceFunc
                (
                pSurfaceData,
                &pUV[i],
                &pXYZ[i],
                &pD[i].dXYZdu,
                &pD[i].dXYZdv,
                &pD[i].d2XYZdudu,
                &pD[i].d2XYZdvdv,
                &pD[i].d2XYZdudv
                ))
            return false;
        }

    if (!plankingStep (pDeltaUV, pXYZ, pD, numPoint))
        return false;

    return true;
    }


static double periodicDistance
(
DPoint2d *pUV0,
DPoint2d *pUV1,
double xPeriod,
double yPeriod
)
    {
    double dx = pUV1->x - pUV0->x;
    double dy = pUV1->y - pUV0->y;
    double a;
    if (xPeriod != 0.0)
        {
        a = 0.5 * xPeriod;
        if (dx < -a)
            dx += xPeriod;
        if (dx > a)
            dx -= xPeriod;
        }

    if (yPeriod != 0.0)
        {
        a = 0.5 * yPeriod;
        if (dy < -a)
            dy += yPeriod;
        if (dy > a)
            dy -= yPeriod;
        }
    return sqrt (dx * dx + dy * dy);
    }

/*
@description Scale a parameter update so it is not too large compared to point spacing.
@param pDeltaOut OUT returned step.
@param pUV0 IN parameters at point 0.
@param pUV1 IN parameters at point 1.
@param pUV2 IN parameters at point 2.
@param pDelta IN proprosed parameter step.
@param localFraction IN largest permitted step, expressed as fraction of smaller parameter difference.
@return the scale factor applied.  Factor of 1.0 is the full step (which is what we really want to happen in converging
        iterations).
*/
static double restrictUpdate
(
DPoint2d *pDeltaOut,
DPoint2d *pUV0,
DPoint2d *pUV1,
DPoint2d *pUV2,
DPoint2d *pDelta,
double localFraction,
double xPeriod,
double yPeriod
)
    {
    double r0   = periodicDistance (pUV0, pUV1, xPeriod, yPeriod);
    double r1   = periodicDistance (pUV1, pUV2, xPeriod, yPeriod);
    double r2   = pDelta->Magnitude ();
    double rMin = localFraction * (r0 < r1 ? r0 : r1);
    double a;

    if (r2 < rMin)
        {
        *pDeltaOut = *pDelta;
        a = 1.0;
        }
    else
        {
        DoubleOps::SafeDivide (a, rMin, r2, 1.0e-4);
        pDeltaOut->Scale (*pDelta, a);
        }
    return a;
    }


static void updateParameter
(
double *pvalue,
double fraction,
double delta,
double period
)
    {
    double a = *pvalue - fraction * delta;
    if (period != 0.0)
        {
        if (a < 0.0)
            a += period;
        if (a > period)
            a -= period;
        }
    *pvalue = a;
    }

/*
@description Update points 1..numPoint-1 by subtracting off a (fraction of) an update.
   Each move is restricted to localFraction of its adjacent intervals.
@param pUV IN OUT numPoint parametric coordinates.   Parameters 1..numPoint-2 are to be updated.
@param pDeltaUV IN (numPoint-2) nominal updates at internal points
        (i.e. update suggested by Newton Raphson linear system.)
@param numPoint IN total number of points.
@param localFraction IN upper limit on update, as a fraction of spacing to next parameter.
@param pMaxDelta OUT largest parameter step found in input pDeltaUV
@param pMinFraction IN smallest fractional update applied (1.0 is prefered output suggesting converging step).
@param globalFraction IN additional fraction to apply to update.  In typical use, send 1.0 for early updates,
    smaller fractions if things don't appear to be converging.
*/
static void updateParameters
(
DPoint2d *pUV,
DPoint2d *pDeltaUV,
int numPoint,
double localFraction,
double *pMaxDelta,
double *pMinFraction,
double  globalFraction,
double xPeriod,
double yPeriod
)
    {
    double aaMax = 0.0;
    double bbMin = 1.0;

    double aa, bb;
    int i;
    DPoint2d delta;
    for (i = 1; i < numPoint - 1; i++)
        {

        aa = fabs (pDeltaUV[i].x);
        if (aa > aaMax)
            aaMax = aa;
        aa = fabs (pDeltaUV[i].y);
        if (aa > aaMax)
            aaMax = aa;
        bb = restrictUpdate (&delta, &pUV[i-1], &pUV[i], &pUV[i+1], &pDeltaUV[i], localFraction, xPeriod, yPeriod);
        if (bb < bbMin)
            bbMin = bb;
        updateParameter (&pUV[i].x, globalFraction, delta.x, xPeriod);
        updateParameter (&pUV[i].y, globalFraction, delta.y, yPeriod);
//        pUV[i].x -= globalFraction * delta.x;
//        correctPeriod (&pUV[i].x, xPeriod);
//        pUV[i].y -= globalFraction * delta.y;
        }
    *pMaxDelta = aaMax;
    *pMinFraction = bbMin;
    }

/*
@description Compute a planking path by iteratively improving an initial path.
@param pUV IN OUT parametric coordinates of points on curve.
@param pXYZ OUT xyz coordinates.
@param numPoint IN number of points
@param surfaceFunc IN function to evaluate surface.
@param pSurfaceData IN surface context, contents known to surfaceFunc.
@param uvTol IN parameter-space tolerance.
*/
Public GEOMDLLIMPEXP bool    planking_computePathExt
(
DPoint2d *pUV,
DPoint3d *pXYZ,
int      numPoint,
PlankingSurfaceEvaluator surfaceFunc,
void *pSurfaceData,
double uvTol,
double xPeriod,
double yPeriod
)
    {
    double maxChange;
    double minFraction;
    int pass;
    static int s_maxPass = 40;
    double s_localFraction = 1.1;
    double globalFraction;
    DPoint2d* pDeltaUV = (DPoint2d*)_alloca (numPoint * sizeof (DPoint2d));

    for (pass = 0; pass < s_maxPass; pass++)
        {
        if (!planking_computePathChange (pDeltaUV, pUV, pXYZ, numPoint, surfaceFunc, pSurfaceData))
            return false;
        //printDPoint2dArray (pUV, numPoint, "UV");
        //printDPoint2dArray (pDeltaUV, numPoint, "delta UV");
        globalFraction = 1.0;
        if (pass > 10)
            globalFraction = 0.5;
        if (pass > 20)
            globalFraction = 0.25;
        updateParameters (pUV, pDeltaUV, numPoint, s_localFraction, &maxChange,
                        &minFraction,
                        globalFraction, xPeriod, yPeriod);
        //printf (" PASS %d n=%d max change = %8.1le min fraction %8.1le\n", pass, numPoint, maxChange, minFraction);
        if (maxChange <= uvTol)
            return true;
        }
    return false;
    }


/*
@description Compute a planking path by iteratively improving an initial path.
@param pUV IN OUT parametric coordinates of points on curve.
@param pXYZ OUT xyz coordinates.
@param numPoint IN number of points
@param surfaceFunc IN function to evaluate surface.
@param pSurfaceData IN surface context, contents known to surfaceFunc.
@param uvTol IN parameter-space tolerance.
*/
Public GEOMDLLIMPEXP bool    planking_computePath
(
DPoint2d *pUV,
DPoint3d *pXYZ,
int      numPoint,
PlankingSurfaceEvaluator surfaceFunc,
void *pSurfaceData,
double uvTol
)
    {
    return planking_computePathExt (pUV, pXYZ, numPoint, surfaceFunc, pSurfaceData, uvTol, 0.0, 0.0);
    }
END_BENTLEY_GEOMETRY_NAMESPACE